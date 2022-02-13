/*
*  Elliptic curve DSA
*
*  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
*  SPDX-License-Identifier: Apache-2.0
*
*  Licensed under the Apache License, Version 2.0 (the "License"); you may
*  not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*  http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
*  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*  This file is part of mbed TLS (https://tls.mbed.org)
*/

/*
* References:
*
* SEC1 http://www.secg.org/index.php?action=secg,docs_secg
*/

#include "ecdsa.h"
#include "asn1.h"
#include "dbrg_hmac.h"

#include "../xdkimp.h"

#define ECDSA_RS_ECP    NULL

#define ECDSA_BUDGET( ops )   /* no-op; for compatibility */

#define ECDSA_RS_ENTER( SUB )   (void) rs_ctx
#define ECDSA_RS_LEAVE( SUB )   (void) rs_ctx

/*
* Derive a suitable integer for group grp from a buffer of length len
* SEC1 4.1.3 step 5 aka SEC1 4.1.4 step 3
*/
static int derive_mpi(const ecp_group *grp, mpi *x,
	const byte_t *buf, dword_t blen)
{
	int ret;
	dword_t n_size = (grp->nbits + 7) / 8;
	dword_t use_size = blen > n_size ? n_size : blen;

	MPI_CHK(mpi_read_binary(x, buf, use_size));
	if (use_size * 8 > grp->nbits)
		MPI_CHK(mpi_shift_r(x, use_size * 8 - grp->nbits));

	/* While at it, reduce modulo N */
	if (mpi_cmp_mpi(x, &grp->N) >= 0)
		MPI_CHK(mpi_sub_mpi(x, x, &grp->N));

cleanup:
	return(ret);
}

/*
* Compute ECDSA signature of a hashed message (SEC1 4.1.3)
* Obviously, compared to SEC1 4.1.3, we skip step 4 (hash message)
*/
static int ecdsa_sign_restartable(ecp_group *grp,
	mpi *r, mpi *s,
	const mpi *d, const byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t), void *p_rng,
	int(*f_rng_blind)(void *, byte_t *, dword_t),
	void *p_rng_blind,
	ecdsa_restart_ctx *rs_ctx)
{
	int ret, key_tries, sign_tries;
	int *p_sign_tries = &sign_tries, *p_key_tries = &key_tries;
	ecp_point R;
	mpi k, e, t;
	mpi *pk = &k, *pr = r;

	/* Fail cleanly on curves such as Curve25519 that can't be used for ECDSA */
	if (grp->N.p == NULL)
	{
		set_last_error(_T("ecdsa_sign_restartable"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/* Make sure d is in range 1..n-1 */
	if (mpi_cmp_int(d, 1) < 0 || mpi_cmp_mpi(d, &grp->N) >= 0)
	{
		set_last_error(_T("ecdsa_sign_restartable"), _T("ERR_ECP_INVALID_KEY"), -1);
		return C_ERR;
	}

	ecp_point_init(&R);
	mpi_init(&k); mpi_init(&e); mpi_init(&t);

	ECDSA_RS_ENTER(sig);

	*p_sign_tries = 0;
	do
	{
		if ((*p_sign_tries)++ > 10)
		{
			set_last_error(_T("ecdsa_sign_restartable"), _T("ERR_ECP_RANDOM_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}

		/*
		* Steps 1-3: generate a suitable ephemeral keypair
		* and set r = xR mod n
		*/
		*p_key_tries = 0;
		do
		{
			if ((*p_key_tries)++ > 10)
			{
				set_last_error(_T("ecdsa_sign_restartable"), _T("ERR_ECP_RANDOM_FAILED"), -1);
				ret = C_ERR;
				goto cleanup;
			}

			MPI_CHK(ecp_gen_privkey(grp, pk, f_rng, p_rng));

			MPI_CHK(ecp_mul_restartable(grp, &R, pk, &grp->G,
				f_rng_blind,
				p_rng_blind,
				ECDSA_RS_ECP));
			MPI_CHK(mpi_mod_mpi(pr, &R.X, &grp->N));
		} while (mpi_cmp_int(pr, 0) == 0);

		/*
		* Accounting for everything up to the end of the loop
		* (step 6, but checking now avoids saving e and t)
		*/
		ECDSA_BUDGET(ECP_OPS_INV + 4);

		/*
		* Step 5: derive MPI from hashed message
		*/
		MPI_CHK(derive_mpi(grp, &e, buf, blen));

		/*
		* Generate a random value to blind inv_mod in next step,
		* avoiding a potential timing leak.
		*/
		MPI_CHK(ecp_gen_privkey(grp, &t, f_rng_blind,
			p_rng_blind));

		/*
		* Step 6: compute s = (e + r * d) / k = t (e + rd) / (kt) mod n
		*/
		MPI_CHK(mpi_mul_mpi(s, pr, d));
		MPI_CHK(mpi_add_mpi(&e, &e, s));
		MPI_CHK(mpi_mul_mpi(&e, &e, &t));
		MPI_CHK(mpi_mul_mpi(pk, pk, &t));
		MPI_CHK(mpi_mod_mpi(pk, pk, &grp->N));
		MPI_CHK(mpi_inv_mod(s, pk, &grp->N));
		MPI_CHK(mpi_mul_mpi(s, s, &e));
		MPI_CHK(mpi_mod_mpi(s, s, &grp->N));
	} while (mpi_cmp_int(s, 0) == 0);

cleanup:
	ecp_point_free(&R);
	mpi_free(&k); mpi_free(&e); mpi_free(&t);

	ECDSA_RS_LEAVE(sig);

	return(ret);
}

/*
* Compute ECDSA signature of a hashed message
*/
int ecdsa_sign(ecp_group *grp, mpi *r, mpi *s,
	const mpi *d, const byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t), void *p_rng)
{
	XDK_ASSERT(grp != NULL);

	if (r == NULL || s == NULL || d == NULL || f_rng == NULL || buf == NULL)
	{
		set_last_error(_T("ecdsa_sign"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	/* Use the same RNG for both blinding and ephemeral key generation */
	return(ecdsa_sign_restartable(grp, r, s, d, buf, blen,
		f_rng, p_rng, f_rng, p_rng, NULL));
}

/*
* Deterministic signature wrapper
*/
static int ecdsa_sign_det_restartable(ecp_group *grp,
	mpi *r, mpi *s,
	const mpi *d, const byte_t *buf, dword_t blen,
	md_type_t md_alg,
	int(*f_rng_blind)(void *, byte_t *, dword_t),
	void *p_rng_blind,
	ecdsa_restart_ctx *rs_ctx)
{
	int ret;
	hmac_drbg_context rng_ctx;
	hmac_drbg_context *p_rng = &rng_ctx;
	byte_t data[2 * ECP_MAX_BYTES];
	dword_t grp_len = (grp->nbits + 7) / 8;
	mpi h;

	mpi_init(&h);
	hmac_drbg_init(&rng_ctx);

	ECDSA_RS_ENTER(det);

	/* Use private key and message hash (reduced) to initialize HMAC_DRBG */
	MPI_CHK(mpi_write_binary(d, data, grp_len));
	MPI_CHK(derive_mpi(grp, &h, buf, blen));
	MPI_CHK(mpi_write_binary(&h, data + grp_len, grp_len));
	hmac_drbg_seed_buf(p_rng, md_alg, data, 2 * grp_len);

	if (f_rng_blind != NULL)
		ret = ecdsa_sign_restartable(grp, r, s, d, buf, blen,
		hmac_drbg_random, p_rng,
		f_rng_blind, p_rng_blind, rs_ctx);
	else
	{
		hmac_drbg_context *p_rng_blind_det;

		/*
		* To avoid reusing rng_ctx and risking incorrect behavior we seed a
		* second HMAC-DRBG with the same seed. We also apply a label to avoid
		* reusing the bits of the ephemeral key for blinding and eliminate the
		* risk that they leak this way.
		*/
		const char* blind_label = "BLINDING CONTEXT";
		hmac_drbg_context rng_ctx_blind;

		hmac_drbg_init(&rng_ctx_blind);
		p_rng_blind_det = &rng_ctx_blind;

		hmac_drbg_seed_buf(p_rng_blind_det, md_alg,
			data, 2 * grp_len);
		ret = hmac_drbg_update(p_rng_blind_det,
			(const byte_t*)blind_label,
			strlen(blind_label));
		if (ret != 0)
		{
			hmac_drbg_free(&rng_ctx_blind);
			goto cleanup;
		}

		/*
		* Since the output of the RNGs is always the same for the same key and
		* message, this limits the efficiency of blinding and leaks information
		* through side channels. After ecdsa_sign_det() is removed NULL
		* won't be a valid value for f_rng_blind anymore. Therefore it should
		* be checked by the caller and this branch and check can be removed.
		*/
		ret = ecdsa_sign_restartable(grp, r, s, d, buf, blen,
			hmac_drbg_random, p_rng,
			hmac_drbg_random, p_rng_blind_det,
			rs_ctx);

		hmac_drbg_free(&rng_ctx_blind);
	}

cleanup:
	hmac_drbg_free(&rng_ctx);
	mpi_free(&h);

	ECDSA_RS_LEAVE(det);

	return(ret);
}

/*
* Deterministic signature wrappers
*/
int ecdsa_sign_det(ecp_group *grp, mpi *r,
	mpi *s, const mpi *d,
	const byte_t *buf, dword_t blen,
	md_type_t md_alg)
{
	XDK_ASSERT(grp != NULL);

	if (r == NULL || s == NULL || d == NULL || buf == NULL)
	{
		set_last_error(_T("ecdsa_sign_det"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	return(ecdsa_sign_det_restartable(grp, r, s, d, buf, blen, md_alg,
		NULL, NULL, NULL));
}

int ecdsa_sign_det_ext(ecp_group *grp, mpi *r,
	mpi *s, const mpi *d,
	const byte_t *buf, dword_t blen,
	md_type_t md_alg,
	int(*f_rng_blind)(void *, byte_t *,
	dword_t),
	void *p_rng_blind)
{
	XDK_ASSERT(grp != NULL);

	if (r == NULL || s == NULL || d == NULL || buf == NULL || f_rng_blind == NULL)
	{
		set_last_error(_T("ecdsa_sign_det_ext"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	return(ecdsa_sign_det_restartable(grp, r, s, d, buf, blen, md_alg,
		f_rng_blind, p_rng_blind, NULL));
}

/*
* Verify ECDSA signature of hashed message (SEC1 4.1.4)
* Obviously, compared to SEC1 4.1.3, we skip step 2 (hash message)
*/
static int ecdsa_verify_restartable(ecp_group *grp,
	const byte_t *buf, dword_t blen,
	const ecp_point *Q,
	const mpi *r, const mpi *s,
	ecdsa_restart_ctx *rs_ctx)
{
	int ret;
	mpi e, s_inv, u1, u2;
	ecp_point R;
	mpi *pu1 = &u1, *pu2 = &u2;

	ecp_point_init(&R);
	mpi_init(&e); mpi_init(&s_inv);
	mpi_init(&u1); mpi_init(&u2);

	/* Fail cleanly on curves such as Curve25519 that can't be used for ECDSA */
	if (grp->N.p == NULL)
	{
		set_last_error(_T("ecdsa_verify_restartable"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	ECDSA_RS_ENTER(ver);

	/*
	* Step 1: make sure r and s are in range 1..n-1
	*/
	if (mpi_cmp_int(r, 1) < 0 || mpi_cmp_mpi(r, &grp->N) >= 0 ||
		mpi_cmp_int(s, 1) < 0 || mpi_cmp_mpi(s, &grp->N) >= 0)
	{
		set_last_error(_T("ecdsa_verify_restartable"), _T("ERR_ECP_VERIFY_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	/*
	* Step 3: derive MPI from hashed message
	*/
	MPI_CHK(derive_mpi(grp, &e, buf, blen));

	/*
	* Step 4: u1 = e / s mod n, u2 = r / s mod n
	*/
	ECDSA_BUDGET(ECP_OPS_CHK + ECP_OPS_INV + 2);

	MPI_CHK(mpi_inv_mod(&s_inv, s, &grp->N));

	MPI_CHK(mpi_mul_mpi(pu1, &e, &s_inv));
	MPI_CHK(mpi_mod_mpi(pu1, pu1, &grp->N));

	MPI_CHK(mpi_mul_mpi(pu2, r, &s_inv));
	MPI_CHK(mpi_mod_mpi(pu2, pu2, &grp->N));
	/*
	* Step 5: R = u1 G + u2 Q
	*/
	MPI_CHK(ecp_muladd_restartable(grp,
		&R, pu1, &grp->G, pu2, Q, ECDSA_RS_ECP));

	if (ecp_is_zero(&R))
	{
		set_last_error(_T("ecdsa_verify_restartable"), _T("ERR_ECP_VERIFY_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	/*
	* Step 6: convert xR to an integer (no-op)
	* Step 7: reduce xR mod n (gives v)
	*/
	MPI_CHK(mpi_mod_mpi(&R.X, &R.X, &grp->N));

	/*
	* Step 8: check if v (that is, R.X) is equal to r
	*/
	if (mpi_cmp_mpi(&R.X, r) != 0)
	{
		set_last_error(_T("ecdsa_verify_restartable"), _T("ERR_ECP_VERIFY_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

cleanup:
	ecp_point_free(&R);
	mpi_free(&e); mpi_free(&s_inv);
	mpi_free(&u1); mpi_free(&u2);

	ECDSA_RS_LEAVE(ver);

	return(ret);
}

/*
* Verify ECDSA signature of hashed message
*/
int ecdsa_verify(ecp_group *grp,
	const byte_t *buf, dword_t blen,
	const ecp_point *Q,
	const mpi *r,
	const mpi *s)
{
	XDK_ASSERT(grp != NULL);

	if (Q == NULL || r == NULL || s == NULL || buf == NULL)
	{
		set_last_error(_T("ecdsa_verify"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	return(ecdsa_verify_restartable(grp, buf, blen, Q, r, s, NULL));
}

/*
* Convert a signature (given by context) to ASN.1
*/
static int ecdsa_signature_to_asn1(const mpi *r, const mpi *s,
	byte_t *sig, dword_t *slen)
{
	int ret;
	byte_t buf[ECDSA_MAX_LEN];
	byte_t *p = buf + sizeof(buf);
	dword_t len = 0;

	ASN1_CHK_ADD(len, asn1_write_mpi(&p, buf, s));
	ASN1_CHK_ADD(len, asn1_write_mpi(&p, buf, r));

	ASN1_CHK_ADD(len, asn1_write_len(&p, buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&p, buf,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE));

	xmem_copy(sig, p, len);
	*slen = len;

	return(0);
}

/*
* Compute and write signature
*/
int ecdsa_write_signature_restartable(ecdsa_context *ctx,
	md_type_t md_alg,
	const byte_t *hash, dword_t hlen,
	byte_t *sig, dword_t *slen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	ecdsa_restart_ctx *rs_ctx)
{
	int ret;
	mpi r, s;
	XDK_ASSERT(ctx != NULL);

	if (hash == NULL || sig == NULL || slen == NULL)
	{
		set_last_error(_T("ecdsa_write_signature_restartable"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	mpi_init(&r);
	mpi_init(&s);

	MPI_CHK(ecdsa_sign_det_restartable(&ctx->grp, &r, &s, &ctx->d,
		hash, hlen, md_alg, f_rng,
		p_rng, rs_ctx));

	MPI_CHK(ecdsa_signature_to_asn1(&r, &s, sig, slen));

cleanup:
	mpi_free(&r);
	mpi_free(&s);

	return(ret);
}

/*
* Compute and write signature
*/
int ecdsa_write_signature(ecdsa_context *ctx,
	md_type_t md_alg,
	const byte_t *hash, dword_t hlen,
	byte_t *sig, dword_t *slen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	XDK_ASSERT(ctx != NULL);

	if (hash == NULL || sig == NULL || slen == NULL)
	{
		set_last_error(_T("ecdsa_write_signature"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	return(ecdsa_write_signature_restartable(
		ctx, md_alg, hash, hlen, sig, slen, f_rng, p_rng, NULL));
}

int ecdsa_write_signature_det(ecdsa_context *ctx,
	const byte_t *hash, dword_t hlen,
	byte_t *sig, dword_t *slen,
	md_type_t md_alg)
{
	XDK_ASSERT(ctx != NULL);

	if (hash == NULL || sig == NULL || slen == NULL)
	{
		set_last_error(_T("ecdsa_write_signature_det"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	return(ecdsa_write_signature(ctx, md_alg, hash, hlen, sig, slen,
		NULL, NULL));
}

/*
* Read and check signature
*/
int ecdsa_read_signature(ecdsa_context *ctx,
	const byte_t *hash, dword_t hlen,
	const byte_t *sig, dword_t slen)
{
	XDK_ASSERT(ctx != NULL);

	if (hash == NULL || sig == NULL)
	{
		set_last_error(_T("ecdsa_read_signature"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	return(ecdsa_read_signature_restartable(
		ctx, hash, hlen, sig, slen, NULL));
}

/*
* Restartable read and check signature
*/
int ecdsa_read_signature_restartable(ecdsa_context *ctx,
	const byte_t *hash, dword_t hlen,
	const byte_t *sig, dword_t slen,
	ecdsa_restart_ctx *rs_ctx)
{
	int ret;
	byte_t *p = (byte_t *)sig;
	const byte_t *end = sig + slen;
	dword_t len;
	mpi r, s;
	XDK_ASSERT(ctx != NULL);

	if (hash == NULL || sig == NULL)
	{
		set_last_error(_T("ecdsa_read_signature_restartable"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	mpi_init(&r);
	mpi_init(&s);

	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		set_last_error(_T("ecdsa_read_signature_restartable"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	if (p + len != end)
	{
		set_last_error(_T("ecdsa_read_signature_restartable"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	if ((ret = asn1_get_mpi(&p, end, &r)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &s)) != 0)
	{
		set_last_error(_T("ecdsa_read_signature_restartable"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	if ((ret = ecdsa_verify_restartable(&ctx->grp, hash, hlen,
		&ctx->Q, &r, &s, rs_ctx)) != 0)
		goto cleanup;

	/* At this point we know that the buffer starts with a valid signature.
	* Return 0 if the buffer just contains the signature, and a specific
	* error code if the valid signature is followed by more data. */
	if (p != end)
	{
		set_last_error(_T("ecdsa_read_signature_restartable"), _T("ERR_ECP_SIG_LEN_MISMATCH"), -1);
		ret = C_ERR;
	}

cleanup:
	mpi_free(&r);
	mpi_free(&s);

	return(ret);
}

/*
* Generate key pair
*/
int ecdsa_genkey(ecdsa_context *ctx, ecp_group_id gid,
	int(*f_rng)(void *, byte_t *, dword_t), void *p_rng)
{
	int ret = 0;
	XDK_ASSERT(ctx != NULL);

	if (f_rng == NULL)
	{
		set_last_error(_T("ecdsa_genkey"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	ret = ecp_group_load(&ctx->grp, gid);
	if (ret != 0)
		return(ret);

	return(ecp_gen_keypair(&ctx->grp, &ctx->d,
		&ctx->Q, f_rng, p_rng));
}

/*
* Set context from an ecp_keypair
*/
int ecdsa_from_keypair(ecdsa_context *ctx, const ecp_keypair *key)
{
	int ret;
	XDK_ASSERT(ctx != NULL);

	if (key == NULL)
	{
		set_last_error(_T("ecdsa_from_keypair"), _T("ERR_INVALID_PARAMETERS"), -1);
		return C_ERR;
	}

	if ((ret = ecp_group_copy(&ctx->grp, &key->grp)) != 0 ||
		(ret = mpi_copy(&ctx->d, &key->d)) != 0 ||
		(ret = ecp_copy(&ctx->Q, &key->Q)) != 0)
	{
		ecdsa_free(ctx);
	}

	return(ret);
}

/*
* Initialize context
*/
void ecdsa_init(ecdsa_context *ctx)
{
	XDK_ASSERT(ctx != NULL);

	ecp_keypair_init(ctx);
}

/*
* Free context
*/
void ecdsa_free(ecdsa_context *ctx)
{
	if (ctx == NULL)
		return;

	ecp_keypair_free(ctx);
}

