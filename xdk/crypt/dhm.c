/*
 *  Diffie-Hellman-Merkle key exchange
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
 *  The following sources were referenced in the design of this implementation
 *  of the Diffie-Hellman-Merkle algorithm:
 *
 *  [1] Handbook of Applied Cryptography - 1997, Chapter 12
 *      Menezes, van Oorschot and Vanstone
 *
 */


#include "dhm.h"
#include "asn1.h"
#include "pem.h"

#include "../xdkimp.h"

#if !defined(DHM_ALT)

/*
* helper to validate the mpi size and import it
*/
static int dhm_read_bignum(mpi *X,
	byte_t **p,
	const byte_t *end)
{
	int ret, n;

	if (end - *p < 2)
		return(ERR_DHM_BAD_INPUT_DATA);

	n = ((*p)[0] << 8) | (*p)[1];
	(*p) += 2;

	if ((int)(end - *p) < n)
		return(ERR_DHM_BAD_INPUT_DATA);

	if ((ret = mpi_read_binary(X, *p, n)) != 0)
		return(ERR_DHM_READ_PARAMS_FAILED + ret);

	(*p) += n;

	return(0);
}

/*
* Verify sanity of parameter with regards to P
*
* Parameter should be: 2 <= public_param <= P - 2
*
* This means that we need to return an error if
*              public_param < 2 or public_param > P-2
*
* For more information on the attack, see:
*  http://www.cl.cam.ac.uk/~rja14/Papers/psandqs.pdf
*  http://web.nvd.nist.gov/view/vuln/detail?vulnId=CVE-2005-2643
*/
static int dhm_check_range(const mpi *param, const mpi *P)
{
	mpi L, U;
	int ret = 0;

	mpi_init(&L); mpi_init(&U);

	MPI_CHK(mpi_lset(&L, 2));
	MPI_CHK(mpi_sub_int(&U, P, 2));

	if (mpi_cmp_mpi(param, &L) < 0 ||
		mpi_cmp_mpi(param, &U) > 0)
	{
		ret = ERR_DHM_BAD_INPUT_DATA;
	}

cleanup:
	mpi_free(&L); mpi_free(&U);
	return(ret);
}

void dhm_init(dhm_context *ctx)
{
	XDK_ASSERT(ctx != NULL);

	xmem_zero(ctx, sizeof(dhm_context));
}

/*
* Parse the ServerKeyExchange parameters
*/
int dhm_read_params(dhm_context *ctx,
	byte_t **p,
	const byte_t *end)
{
	int ret;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(p != NULL && *p != NULL);
	XDK_ASSERT(end != NULL);

	if ((ret = dhm_read_bignum(&ctx->P, p, end)) != 0 ||
		(ret = dhm_read_bignum(&ctx->G, p, end)) != 0 ||
		(ret = dhm_read_bignum(&ctx->GY, p, end)) != 0)
		return(ret);

	if ((ret = dhm_check_range(&ctx->GY, &ctx->P)) != 0)
		return(ret);

	ctx->len = mpi_size(&ctx->P);

	return(0);
}

/*
* Setup and write the ServerKeyExchange parameters
*/
int dhm_make_params(dhm_context *ctx, int x_size,
	byte_t *output, dword_t *olen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret, count = 0;
	dword_t n1, n2, n3;
	byte_t *p;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(output != NULL);
	XDK_ASSERT(olen != NULL);
	XDK_ASSERT(f_rng != NULL);

	if (mpi_cmp_int(&ctx->P, 0) == 0)
		return(ERR_DHM_BAD_INPUT_DATA);

	/*
	* Generate X as large as possible ( < P )
	*/
	do
	{
		MPI_CHK(mpi_fill_random(&ctx->X, x_size, f_rng, p_rng));

		while (mpi_cmp_mpi(&ctx->X, &ctx->P) >= 0)
			MPI_CHK(mpi_shift_r(&ctx->X, 1));

		if (count++ > 10)
			return(ERR_DHM_MAKE_PARAMS_FAILED);
	} while (dhm_check_range(&ctx->X, &ctx->P) != 0);

	/*
	* Calculate GX = G^X mod P
	*/
	MPI_CHK(mpi_exp_mod(&ctx->GX, &ctx->G, &ctx->X,
		&ctx->P, &ctx->RP));

	if ((ret = dhm_check_range(&ctx->GX, &ctx->P)) != 0)
		return(ret);

	/*
	* export P, G, GX
	*/
#define DHM_MPI_EXPORT( X, n )                                          \
    do {                                                                \
        MPI_CHK( mpi_write_binary( ( X ),               \
                                                   p + 2,               \
                                                   ( n ) ) );           \
        *p++ = (byte_t)( ( n ) >> 8 );                           \
        *p++ = (byte_t)( ( n )      );                           \
        p += ( n );                                                     \
	    } while( 0 )

	n1 = mpi_size(&ctx->P);
	n2 = mpi_size(&ctx->G);
	n3 = mpi_size(&ctx->GX);

	p = output;
	DHM_MPI_EXPORT(&ctx->P, n1);
	DHM_MPI_EXPORT(&ctx->G, n2);
	DHM_MPI_EXPORT(&ctx->GX, n3);

	*olen = p - output;

	ctx->len = n1;

cleanup:

	if (ret != 0)
		return(ERR_DHM_MAKE_PARAMS_FAILED + ret);

	return(0);
}

/*
* Set prime modulus and generator
*/
int dhm_set_group(dhm_context *ctx,
	const mpi *P,
	const mpi *G)
{
	int ret;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(P != NULL);
	XDK_ASSERT(G != NULL);

	if ((ret = mpi_copy(&ctx->P, P)) != 0 ||
		(ret = mpi_copy(&ctx->G, G)) != 0)
	{
		return(ERR_DHM_SET_GROUP_FAILED + ret);
	}

	ctx->len = mpi_size(&ctx->P);
	return(0);
}

/*
* Import the peer's public value G^Y
*/
int dhm_read_public(dhm_context *ctx,
	const byte_t *input, dword_t ilen)
{
	int ret;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(input != NULL);

	if (ilen < 1 || ilen > ctx->len)
		return(ERR_DHM_BAD_INPUT_DATA);

	if ((ret = mpi_read_binary(&ctx->GY, input, ilen)) != 0)
		return(ERR_DHM_READ_PUBLIC_FAILED + ret);

	return(0);
}

/*
* Create own private value X and export G^X
*/
int dhm_make_public(dhm_context *ctx, int x_size,
	byte_t *output, dword_t olen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret, count = 0;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(output != NULL);
	XDK_ASSERT(f_rng != NULL);

	if (olen < 1 || olen > ctx->len)
		return(ERR_DHM_BAD_INPUT_DATA);

	if (mpi_cmp_int(&ctx->P, 0) == 0)
		return(ERR_DHM_BAD_INPUT_DATA);

	/*
	* generate X and calculate GX = G^X mod P
	*/
	do
	{
		MPI_CHK(mpi_fill_random(&ctx->X, x_size, f_rng, p_rng));

		while (mpi_cmp_mpi(&ctx->X, &ctx->P) >= 0)
			MPI_CHK(mpi_shift_r(&ctx->X, 1));

		if (count++ > 10)
			return(ERR_DHM_MAKE_PUBLIC_FAILED);
	} while (dhm_check_range(&ctx->X, &ctx->P) != 0);

	MPI_CHK(mpi_exp_mod(&ctx->GX, &ctx->G, &ctx->X,
		&ctx->P, &ctx->RP));

	if ((ret = dhm_check_range(&ctx->GX, &ctx->P)) != 0)
		return(ret);

	MPI_CHK(mpi_write_binary(&ctx->GX, output, olen));

cleanup:

	if (ret != 0)
		return(ERR_DHM_MAKE_PUBLIC_FAILED + ret);

	return(0);
}

int dhm_make_public_size(dhm_context *ctx, int x_size)
{
	return x_size / (sizeof(mpi_uint));
}

/*
* Use the blinding method and optimisation suggested in section 10 of:
*  KOCHER, Paul C. Timing attacks on implementations of Diffie-Hellman, RSA,
*  DSS, and other systems. In : Advances in Cryptology-CRYPTO'96. Springer
*  Berlin Heidelberg, 1996. p. 104-113.
*/
static int dhm_update_blinding(dhm_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t), void *p_rng)
{
	int ret, count;

	/*
	* Don't use any blinding the first time a particular X is used,
	* but remember it to use blinding next time.
	*/
	if (mpi_cmp_mpi(&ctx->X, &ctx->pX) != 0)
	{
		MPI_CHK(mpi_copy(&ctx->pX, &ctx->X));
		MPI_CHK(mpi_lset(&ctx->Vi, 1));
		MPI_CHK(mpi_lset(&ctx->Vf, 1));

		return(0);
	}

	/*
	* Ok, we need blinding. Can we re-use existing values?
	* If yes, just update them by squaring them.
	*/
	if (mpi_cmp_int(&ctx->Vi, 1) != 0)
	{
		MPI_CHK(mpi_mul_mpi(&ctx->Vi, &ctx->Vi, &ctx->Vi));
		MPI_CHK(mpi_mod_mpi(&ctx->Vi, &ctx->Vi, &ctx->P));

		MPI_CHK(mpi_mul_mpi(&ctx->Vf, &ctx->Vf, &ctx->Vf));
		MPI_CHK(mpi_mod_mpi(&ctx->Vf, &ctx->Vf, &ctx->P));

		return(0);
	}

	/*
	* We need to generate blinding values from scratch
	*/

	/* Vi = random( 2, P-1 ) */
	count = 0;
	do
	{
		MPI_CHK(mpi_fill_random(&ctx->Vi, mpi_size(&ctx->P), f_rng, p_rng));

		while (mpi_cmp_mpi(&ctx->Vi, &ctx->P) >= 0)
			MPI_CHK(mpi_shift_r(&ctx->Vi, 1));

		if (count++ > 10)
			return(ERR_MPI_NOT_ACCEPTABLE);
	} while (mpi_cmp_int(&ctx->Vi, 1) <= 0);

	/* Vf = Vi^-X mod P */
	MPI_CHK(mpi_inv_mod(&ctx->Vf, &ctx->Vi, &ctx->P));
	MPI_CHK(mpi_exp_mod(&ctx->Vf, &ctx->Vf, &ctx->X, &ctx->P, &ctx->RP));

cleanup:
	return(ret);
}

/*
* Derive and export the shared secret (G^Y)^X mod P
*/
int dhm_calc_secret(dhm_context *ctx,
	byte_t *output, dword_t output_size, dword_t *olen,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	mpi GYb;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(output != NULL);
	XDK_ASSERT(olen != NULL);

	if (output_size < ctx->len)
		return(ERR_DHM_BAD_INPUT_DATA);

	if ((ret = dhm_check_range(&ctx->GY, &ctx->P)) != 0)
		return(ret);

	mpi_init(&GYb);

	/* Blind peer's value */
	if (f_rng != NULL)
	{
		MPI_CHK(dhm_update_blinding(ctx, f_rng, p_rng));
		MPI_CHK(mpi_mul_mpi(&GYb, &ctx->GY, &ctx->Vi));
		MPI_CHK(mpi_mod_mpi(&GYb, &GYb, &ctx->P));
	}
	else
		MPI_CHK(mpi_copy(&GYb, &ctx->GY));

	/* Do modular exponentiation */
	MPI_CHK(mpi_exp_mod(&ctx->K, &GYb, &ctx->X,
		&ctx->P, &ctx->RP));

	/* Unblind secret value */
	if (f_rng != NULL)
	{
		MPI_CHK(mpi_mul_mpi(&ctx->K, &ctx->K, &ctx->Vf));
		MPI_CHK(mpi_mod_mpi(&ctx->K, &ctx->K, &ctx->P));
	}

	*olen = mpi_size(&ctx->K);

	MPI_CHK(mpi_write_binary(&ctx->K, output, *olen));

cleanup:
	mpi_free(&GYb);

	if (ret != 0)
		return(ERR_DHM_CALC_SECRET_FAILED + ret);

	return(0);
}

/*
* Free the components of a DHM key
*/
void dhm_free(dhm_context *ctx)
{
	if (ctx == NULL)
		return;

	mpi_free(&ctx->pX);
	mpi_free(&ctx->Vf);
	mpi_free(&ctx->Vi);
	mpi_free(&ctx->RP);
	mpi_free(&ctx->K);
	mpi_free(&ctx->GY);
	mpi_free(&ctx->GX);
	mpi_free(&ctx->X);
	mpi_free(&ctx->G);
	mpi_free(&ctx->P);

	xmem_zero(ctx, sizeof(dhm_context));
}

/*
* Parse DHM parameters
*/
int dhm_parse_dhm(dhm_context *dhm, const byte_t *dhmin,
	dword_t dhminlen)
{
	int ret;
	dword_t len;
	byte_t *p, *end;
#if defined(PEM_PARSE_C)
	pem_context pem;
#endif /* PEM_PARSE_C */

	XDK_ASSERT(dhm != NULL);
	XDK_ASSERT(dhmin != NULL);

#if defined(PEM_PARSE_C)
	pem_init(&pem);

	/* Avoid calling pem_read_buffer() on non-null-terminated string */
	if (dhminlen == 0 || dhmin[dhminlen - 1] != '\0')
		ret = ERR_PEM_NO_HEADER_FOOTER_PRESENT;
	else
		ret = pem_read_buffer(&pem,
		"-----BEGIN DH PARAMETERS-----",
		"-----END DH PARAMETERS-----",
		dhmin, NULL, 0, &dhminlen);

	if (ret == 0)
	{
		/*
		* Was PEM encoded
		*/
		dhminlen = pem.buflen;
	}
	else if (ret != ERR_PEM_NO_HEADER_FOOTER_PRESENT)
		goto exit;

	p = (ret == 0) ? pem.buf : (byte_t *)dhmin;
#else
	p = (byte_t *)dhmin;
#endif /* PEM_PARSE_C */
	end = p + dhminlen;

	/*
	*  DHParams ::= SEQUENCE {
	*      prime              INTEGER,  -- P
	*      generator          INTEGER,  -- g
	*      privateValueLength INTEGER OPTIONAL
	*  }
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		ret = ERR_DHM_INVALID_FORMAT + ret;
		goto exit;
	}

	end = p + len;

	if ((ret = asn1_get_mpi(&p, end, &dhm->P)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &dhm->G)) != 0)
	{
		ret = ERR_DHM_INVALID_FORMAT + ret;
		goto exit;
	}

	if (p != end)
	{
		/* This might be the optional privateValueLength.
		* If so, we can cleanly discard it */
		mpi rec;
		mpi_init(&rec);
		ret = asn1_get_mpi(&p, end, &rec);
		mpi_free(&rec);
		if (ret != 0)
		{
			ret = ERR_DHM_INVALID_FORMAT + ret;
			goto exit;
		}
		if (p != end)
		{
			ret = ERR_DHM_INVALID_FORMAT;
			goto exit;
		}
	}

	ret = 0;

	dhm->len = mpi_size(&dhm->P);

exit:
#if defined(PEM_PARSE_C)
	pem_free(&pem);
#endif
	if (ret != 0)
		dhm_free(dhm);

	return(ret);
}

#endif /* DHM_ALT */

#if defined(XDK_SUPPORT_TEST)

#if defined(PEM_PARSE_C)
static const char test_dhm_params[] =
"-----BEGIN DH PARAMETERS-----\r\n"
"MIGHAoGBAJ419DBEOgmQTzo5qXl5fQcN9TN455wkOL7052HzxxRVMyhYmwQcgJvh\r\n"
"1sa18fyfR9OiVEMYglOpkqVoGLN7qd5aQNNi5W7/C+VBdHTBJcGZJyyP5B3qcz32\r\n"
"9mLJKudlVudV0Qxk5qUJaPZ/xupz0NyoVpviuiBOI1gNi8ovSXWzAgEC\r\n"
"-----END DH PARAMETERS-----\r\n";
#else /* PEM_PARSE_C */
static const char test_dhm_params[] = {
	0x30, 0x81, 0x87, 0x02, 0x81, 0x81, 0x00, 0x9e, 0x35, 0xf4, 0x30, 0x44,
	0x3a, 0x09, 0x90, 0x4f, 0x3a, 0x39, 0xa9, 0x79, 0x79, 0x7d, 0x07, 0x0d,
	0xf5, 0x33, 0x78, 0xe7, 0x9c, 0x24, 0x38, 0xbe, 0xf4, 0xe7, 0x61, 0xf3,
	0xc7, 0x14, 0x55, 0x33, 0x28, 0x58, 0x9b, 0x04, 0x1c, 0x80, 0x9b, 0xe1,
	0xd6, 0xc6, 0xb5, 0xf1, 0xfc, 0x9f, 0x47, 0xd3, 0xa2, 0x54, 0x43, 0x18,
	0x82, 0x53, 0xa9, 0x92, 0xa5, 0x68, 0x18, 0xb3, 0x7b, 0xa9, 0xde, 0x5a,
	0x40, 0xd3, 0x62, 0xe5, 0x6e, 0xff, 0x0b, 0xe5, 0x41, 0x74, 0x74, 0xc1,
	0x25, 0xc1, 0x99, 0x27, 0x2c, 0x8f, 0xe4, 0x1d, 0xea, 0x73, 0x3d, 0xf6,
	0xf6, 0x62, 0xc9, 0x2a, 0xe7, 0x65, 0x56, 0xe7, 0x55, 0xd1, 0x0c, 0x64,
	0xe6, 0xa5, 0x09, 0x68, 0xf6, 0x7f, 0xc6, 0xea, 0x73, 0xd0, 0xdc, 0xa8,
	0x56, 0x9b, 0xe2, 0xba, 0x20, 0x4e, 0x23, 0x58, 0x0d, 0x8b, 0xca, 0x2f,
	0x49, 0x75, 0xb3, 0x02, 0x01, 0x02 };
#endif /* PEM_PARSE_C */

static const dword_t test_dhm_params_len = sizeof(test_dhm_params);

/*
* Checkup routine
*/
int dhm_self_test(int verbose)
{
	int ret;
	dhm_context dhm;

	dhm_init(&dhm);

	if (verbose != 0)
		printf("  DHM parameter load: ");

	if ((ret = dhm_parse_dhm(&dhm,
		(const byte_t *)test_dhm_params,
		test_dhm_params_len)) != 0)
	{
		if (verbose != 0)
			printf("failed\n");

		ret = 1;
		goto exit;
	}

	if (verbose != 0)
		printf("passed\n\n");

exit:
	dhm_free(&dhm);

	return(ret);
}

#endif /* SELF_TEST */
