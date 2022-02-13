/*
*  Elliptic curve Diffie-Hellman
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
* RFC 4492
*/

#include "ecdh.h"

#include "../xdkimp.h"

static ecp_group_id ecdh_grp_id(
	const ecdh_context *ctx)
{
	return(ctx->grp.id);
}

/*
* Generate public key
*/
int ecdh_gen_public(ecp_group *grp, mpi *d, ecp_point *Q,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;

	/* If multiplication is in progress, we already generated a privkey */

	MPI_CHK(ecp_gen_privkey(grp, d, f_rng, p_rng));

	MPI_CHK(ecp_mul_restartable(grp, Q, d, &grp->G, f_rng, p_rng, NULL));

cleanup:
	return(ret);
}

/*
* Compute shared secret (SEC1 3.3.1)
*/
int ecdh_compute_shared(ecp_group *grp, mpi *z,
	const ecp_point *Q, const mpi *d,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	ecp_point P;

	ecp_point_init(&P);

	MPI_CHK(ecp_mul_restartable(grp, &P, d, Q, f_rng, p_rng, NULL));

	if (ecp_is_zero(&P))
	{
		set_last_error(_T("ecdh_compute_shared_restartable"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		goto cleanup;
	}

	MPI_CHK(mpi_copy(z, &P.X));

cleanup:
	ecp_point_free(&P);

	return(ret);
}

/*
* Initialize context
*/
void ecdh_init(ecdh_context *ctx)
{
	ecp_group_init(&ctx->grp);

	mpi_init(&ctx->d);
	ecp_point_init(&ctx->Q);
	ecp_point_init(&ctx->Qp);
	mpi_init(&ctx->z);

	ecp_point_init(&ctx->Vi);
	ecp_point_init(&ctx->Vf);
	mpi_init(&ctx->_d);

	ctx->point_format = ECP_PF_UNCOMPRESSED;
}

/*
* Setup context
*/
int ecdh_setup(ecdh_context *ctx, ecp_group_id grp_id)
{
	int ret;

	ret = ecp_group_load(&ctx->grp, grp_id);
	if (ret != 0)
	{
		set_last_error(_T("ecdh_setup_internal"), _T("ERR_ECP_FEATURE_UNAVAILABLE"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Free context
*/
void ecdh_free(ecdh_context *ctx)
{
	if (ctx == NULL)
		return;

	ecp_point_free(&ctx->Vi);
	ecp_point_free(&ctx->Vf);
	mpi_free(&ctx->_d);
	
	ecp_group_free(&ctx->grp);
	mpi_free(&ctx->d);
	ecp_point_free(&ctx->Q);
	ecp_point_free(&ctx->Qp);
	mpi_free(&ctx->z);
}

/*
* Setup and write the ServerKeyExhange parameters (RFC 4492)
*      struct {
*          ECParameters    curve_params;
*          ECPoint         public;
*      } ServerECDHParams;
*/
int ecdh_make_params(ecdh_context *ctx, dword_t *olen,
	byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	dword_t grp_len, pt_len;

	if (ctx->grp.pbits == 0)
	{
		set_last_error(_T("ecdh_make_params_internal"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if ((ret = ecdh_gen_public(&ctx->grp, &ctx->d, &ctx->Q, f_rng, p_rng)) != 0)
		return(ret);

	if ((ret = ecp_tls_write_group(&ctx->grp, &grp_len, buf, blen)) != 0)
		return(ret);

	buf += grp_len;
	blen -= grp_len;

	if ((ret = ecp_tls_write_point(&ctx->grp, &ctx->Q, ctx->point_format, &pt_len, buf, blen)) != 0)
		return(ret);

	*olen = grp_len + pt_len;
	return(0);
}

/*
* Read the ServerKeyExhange parameters (RFC 4492)
*      struct {
*          ECParameters    curve_params;
*          ECPoint         public;
*      } ServerECDHParams;
*/
int ecdh_read_params(ecdh_context *ctx,
	const byte_t **buf,
	const byte_t *end,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	ecp_group_id grp_id;

	if ((ret = ecp_tls_read_group_id(&grp_id, buf, end - *buf)) != 0)
		return(ret);

	if ((ret = ecdh_setup(ctx, grp_id)) != 0)
		return(ret);

	if ((ret = ecp_tls_read_point(&ctx->grp, &ctx->Qp, buf, end - *buf)) != 0)
		return(ret);

	return (0);
}

/*
* Get parameters from a keypair
*/
int ecdh_get_params(ecdh_context *ctx,
	const ecp_keypair *key,
	ecdh_side side)
{
	int ret;

	if (ecdh_grp_id(ctx) == ECP_DP_NONE)
	{
		/* This is the first call to get_params(). Set up the context
		* for use with the group. */
		if ((ret = ecdh_setup(ctx, key->grp.id)) != 0)
			return(ret);
	}
	else
	{
		/* This is not the first call to get_params(). Check that the
		* current key's group is the same as the context's, which was set
		* from the first key's group. */
		if (ecdh_grp_id(ctx) != key->grp.id)
		{
			set_last_error(_T("ecdh_get_params"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}
	}

	/* If it's not our key, just import the public part as Qp */
	if (side == ECDH_THEIRS)
		return(ecp_copy(&ctx->Qp, &key->Q));

	/* Our key: import public (as Q) and private parts */
	if (side != ECDH_OURS)
	{
		set_last_error(_T("ecdh_get_params_internal"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if ((ret = ecp_copy(&ctx->Q, &key->Q)) != 0 ||
		(ret = mpi_copy(&ctx->d, &key->d)) != 0)
		return(ret);

	return(0);
}


/*
* Setup and export the client public value
*/
int ecdh_make_public(ecdh_context *ctx, dword_t *olen,
	byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;

	if (ctx->grp.pbits == 0)
	{
		set_last_error(_T("ecdh_make_public_internal"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if ((ret = ecdh_gen_public(&ctx->grp, &ctx->d, &ctx->Q, f_rng, p_rng)) != 0)
		return(ret);

	return ecp_tls_write_point(&ctx->grp, &ctx->Q, ctx->point_format, olen, buf, blen);
}

/*
* Parse and import the client's public value
*/
int ecdh_read_public(ecdh_context *ctx,
	const byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	const byte_t *p = buf;

	if ((ret = ecp_tls_read_point(&ctx->grp, &ctx->Qp, &p, blen)) != 0)
		return(ret);

	if ((dword_t)(p - buf) != blen)
	{
		set_last_error(_T("ecdh_read_public_internal"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	return (0);
}

int ecdh_make_params_tls13(ecdh_context *ctx, 
	ecp_group_id grp_id,
	dword_t *olen, byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;

	if ((ret = ecdh_setup(ctx, grp_id)) != 0)
		return(ret);

	if (ctx->grp.pbits == 0)
	{
		set_last_error(_T("ecdh_make_params_internal"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if ((ret = ecdh_gen_public(&ctx->grp, &ctx->d, &ctx->Q, f_rng, p_rng)) != 0)
		return(ret);

	if ((ret = ecp_point_write_binary(&ctx->grp, &ctx->Q, ctx->point_format, olen, buf, blen)) != 0)
		return (ret);

	return(0);
}

int ecdh_read_params_tls13(ecdh_context *ctx,
	ecp_group_id grp_id,
	const byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	
	if ((ret = ecdh_setup(ctx, grp_id)) != 0)
		return(ret);

	if ((ret = ecp_point_read_binary(&ctx->grp, &ctx->Qp, buf, blen)) != 0)
		return(ret);

	return (0);
}

int ecdh_make_public_tls13(ecdh_context *ctx, dword_t *olen,
	byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;

	if (ctx->grp.pbits == 0)
	{
		set_last_error(_T("ecdh_make_public_internal"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if ((ret = ecdh_gen_public(&ctx->grp, &ctx->d, &ctx->Q, f_rng, p_rng)) != 0)
		return(ret);

	if ((ret = ecp_point_write_binary(&ctx->grp, &ctx->Q, ctx->point_format, olen, buf, blen)) != 0)
		return (ret);

	return (0);
}

int ecdh_read_public_tls13(ecdh_context *ctx,
	const byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;

	if ((ret = ecp_point_read_binary(&ctx->grp, &ctx->Qp, buf, blen)) != 0)
		return(ret);

	return (0);
}

/*
* Derive and export the shared secret
*/
int ecdh_calc_secret(ecdh_context *ctx, dword_t *olen,
	byte_t *buf, dword_t blen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;

	if (ctx == NULL || ctx->grp.pbits == 0)
	{
		set_last_error(_T("ecdh_calc_secret_internal"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if ((ret = ecdh_compute_shared(&ctx->grp, &ctx->z, &ctx->Qp, &ctx->d, f_rng, p_rng)) != 0)
	{
		return(ret);
	}

	if (mpi_size(&ctx->z) > blen)
	{
		set_last_error(_T("ecdh_calc_secret_internal"), _T("ERR_ECP_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	*olen = ctx->grp.pbits / 8 + ((ctx->grp.pbits % 8) != 0);
	return mpi_write_binary(&ctx->z, buf, *olen);
}

#if defined(XDK_SUPPORT_TEST)

#include "dbrg_ctr.h"
#include "entropy.h"

int ecdh_x25519_test(int verbose)
{
	int exit_code, n, ret = 1;
	ecdh_context ctx_cli, ctx_srv;
	entropy_context entropy;
	ctr_drbg_context ctr_drbg;
	unsigned char cli_to_srv[64], srv_to_cli[64];
	const char pers[] = "ecdh";

	ecdh_init(&ctx_cli);
	ecdh_init(&ctx_srv);
	ctr_drbg_init(&ctr_drbg);

	/*
	* Initialize random number generation
	*/
	entropy_init(&entropy);
	if ((ret = ctr_drbg_seed(&ctr_drbg, entropy_func, &entropy, (const unsigned char *)pers, sizeof pers)) != 0)
	{
		printf(" failed\n  ! ctr_drbg_seed returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	* Client: inialize context and generate keypair
	*/
	if (verbose == 2)
		ret = ecp_group_load(&ctx_cli.grp, ECP_DP_CURVE448);
	else
		ret = ecp_group_load(&ctx_cli.grp, ECP_DP_CURVE25519);
	if (ret != 0)
	{
		printf(" failed\n  ! ecp_group_load returned %d\n", ret);
		goto exit;
	}

	ret = ecdh_gen_public(&ctx_cli.grp, &ctx_cli.d, &ctx_cli.Q,
		ctr_drbg_random, &ctr_drbg);
	if (ret != 0)
	{
		printf(" failed\n  ! ecdh_gen_public returned %d\n", ret);
		goto exit;
	}

	n = mpi_size(&(ctx_cli.grp.P));
	ret = mpi_write_binary(&ctx_cli.Q.X, cli_to_srv, n);
	if (ret != 0)
	{
		printf(" failed\n  ! mpi_write_binary returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	* Server: initialize context and generate keypair
	*/
	if (verbose == 2)
		ret = ecp_group_load(&ctx_srv.grp, ECP_DP_CURVE448);
	else
		ret = ecp_group_load(&ctx_srv.grp, ECP_DP_CURVE25519);
	if (ret != 0)
	{
		printf(" failed\n  ! ecp_group_load returned %d\n", ret);
		goto exit;
	}

	ret = ecdh_gen_public(&ctx_srv.grp, &ctx_srv.d, &ctx_srv.Q,
		ctr_drbg_random, &ctr_drbg);
	if (ret != 0)
	{
		printf(" failed\n  ! ecdh_gen_public returned %d\n", ret);
		goto exit;
	}

	n = mpi_size(&(ctx_srv.grp.P));
	ret = mpi_write_binary(&ctx_srv.Q.X, srv_to_cli, n);
	if (ret != 0)
	{
		printf(" failed\n  ! mpi_write_binary returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	* Server: read peer's key and generate shared secret
	*/
	printf("  . Server reading client key and computing secret...");

	ret = mpi_lset(&ctx_srv.Qp.Z, 1);
	if (ret != 0)
	{
		printf(" failed\n  ! mpi_lset returned %d\n", ret);
		goto exit;
	}

	n = mpi_size(&(ctx_srv.grp.P));
	ret = mpi_read_binary(&ctx_srv.Qp.X, cli_to_srv, n);
	if (ret != 0)
	{
		printf(" failed\n  ! mpi_read_binary returned %d\n", ret);
		goto exit;
	}

	ret = ecdh_compute_shared(&ctx_srv.grp, &ctx_srv.z,
		&ctx_srv.Qp, &ctx_srv.d,
		ctr_drbg_random, &ctr_drbg);
	if (ret != 0)
	{
		printf(" failed\n  ! ecdh_compute_shared returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	* Client: read peer's key and generate shared secret
	*/
	printf("  . Client reading server key and computing secret...");

	ret = mpi_lset(&ctx_cli.Qp.Z, 1);
	if (ret != 0)
	{
		printf(" failed\n  ! mpi_lset returned %d\n", ret);
		goto exit;
	}

	n = mpi_size(&(ctx_cli.grp.P));
	ret = mpi_read_binary(&ctx_cli.Qp.X, srv_to_cli, n);
	if (ret != 0)
	{
		printf(" failed\n  ! mpi_read_binary returned %d\n", ret);
		goto exit;
	}

	ret = ecdh_compute_shared(&ctx_cli.grp, &ctx_cli.z,
		&ctx_cli.Qp, &ctx_cli.d,
		ctr_drbg_random, &ctr_drbg);
	if (ret != 0)
	{
		printf(" failed\n  ! ecdh_compute_shared returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	/*
	* Verification: are the computed secrets equal?
	*/
	printf("  . Checking if both computed secrets are equal...");

	ret = mpi_cmp_mpi(&ctx_cli.z, &ctx_srv.z);
	if (ret != 0)
	{
		printf(" failed\n  ! ecdh_compute_shared returned %d\n", ret);
		goto exit;
	}

	printf(" ok\n");

	exit_code = EXIT_SUCCESS;

exit:

#if defined(_WIN32)
	printf("  + Press Enter to exit this program.\n");
#endif

	ecdh_free(&ctx_srv);
	ecdh_free(&ctx_cli);
	ctr_drbg_free(&ctr_drbg);
	entropy_free(&entropy);

	return(exit_code);
}

int ecdh_test(int verbose)
{
	havege_state rng;
	int ret, len_cli, len_srv;
	ecdh_context ecdh_cli, ecdh_srv;
	byte_t buf_cli[256] = { 0 };
	byte_t buf_srv[256] = { 0 };
	byte_t prm_cli[256] = { 0 };
	byte_t prm_srv[256] = { 0 };

	havege_init(&rng);

	//client 
	ecdh_init(&ecdh_cli);

	//write params to server
	if (verbose == 2)
	{
		ecdh_setup(&ecdh_cli, ECP_DP_CURVE448);
		ret = ecdh_make_params(&ecdh_cli, &len_cli, buf_cli, 256, havege_random, &rng);
	}
	else
	{
		ecdh_setup(&ecdh_cli, ECP_DP_SECP256K1);
		ret = ecdh_make_params(&ecdh_cli, &len_cli, buf_cli, 256, havege_random, &rng);
	}
	if (ret != 0)
	{
		printf(" client failed\n  ! ecdh_setup\n");
		goto exit;
	}
	
	//read params from client
	ecdh_init(&ecdh_srv);
	byte_t* p = buf_cli;
	if (verbose == 2)
	{
		ret = ecdh_read_params(&ecdh_srv, &p, (buf_cli + len_cli), havege_random, &rng);
	}
	else
	{
		ret = ecdh_read_params(&ecdh_srv, &p, (buf_cli + len_cli), havege_random, &rng);
	}
	if (ret != 0)
	{
		printf(" client failed...ecdh_setup\n");
		goto exit;
	}

	//write public to client
	ret = ecdh_make_public(&ecdh_srv, &len_srv, buf_srv, 256, havege_random, &rng);
	if (ret != 0)
	{
		printf(" server failed...ecdh_read_public\n");
		goto exit;
	}
	ret = ecdh_calc_secret(&ecdh_srv, &len_cli, prm_srv, 256, havege_random, &rng);
	if (ret != 0)
	{
		printf(" server failed...ecdh_calc_secret\n");
		goto exit;
	}

	//read public from server
	ret = ecdh_read_public(&ecdh_cli, buf_srv, len_srv, havege_random, &rng);
	if (ret != 0)
	{
		printf(" client failed...ecdh_read_public\n");
		goto exit;
	}
	ret = ecdh_calc_secret(&ecdh_cli, &len_srv, prm_cli, 256, havege_random, &rng);
	if (ret != 0)
	{
		printf(" client failed...ecdh_calc_secret\n");
		goto exit;
	}

	ret = mpi_cmp_mpi(&ecdh_cli.z, &ecdh_srv.z);
	if (ret != 0)
	{
		printf(" secret compare failed...mpi mistatch\n");
		goto exit;
	}
	if (len_cli != len_srv)
	{
		printf(" secret compare failed...length mistatch\n");
		goto exit;
	}
	if (xmem_comp(prm_cli, prm_srv, len_cli) != 0)
		printf(" secret compare failed...context mistatch\n");
	else
		printf(" secret compare success\n");

exit:

	ecdh_free(&ecdh_cli);
	ecdh_free(&ecdh_srv);

	return ret;
}

#endif