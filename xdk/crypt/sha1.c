/*
*  FIPS-180-1 compliant SHA-1 implementation
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
*  The SHA-1 standard was published by NIST in 1993.
*
*  http://www.itl.nist.gov/fipspubs/fip180-1.htm
*/


#include "sha1.h"

#include "../xdkimp.h"

/*
* 32-bit integer manipulation macros (big endian)
*/
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                            \
{                                                       \
    (n) = ( (dword_t) (b)[(i)    ] << 24 )             \
        | ( (dword_t) (b)[(i) + 1] << 16 )             \
        | ( (dword_t) (b)[(i) + 2] <<  8 )             \
        | ( (dword_t) (b)[(i) + 3]       );            \
}
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
{                                                       \
    (b)[(i)    ] = (byte_t) ( (n) >> 24 );       \
    (b)[(i) + 1] = (byte_t) ( (n) >> 16 );       \
    (b)[(i) + 2] = (byte_t) ( (n) >>  8 );       \
    (b)[(i) + 3] = (byte_t) ( (n)       );       \
}
#endif

void sha1_init(sha1_context *ctx)
{
	XDK_ASSERT(ctx != NULL);

	xmem_zero(ctx, sizeof(sha1_context));
}

void sha1_free(sha1_context *ctx)
{
	if (ctx == NULL)
		return;

	xmem_zero(ctx, sizeof(sha1_context));
}

void sha1_clone(sha1_context *dst,
	const sha1_context *src)
{
	XDK_ASSERT(dst != NULL && src != NULL);

	xmem_copy((void*)dst, (void*)src, sizeof(sha1_context));
}

/*
* SHA-1 context setup
*/
int sha1_starts(sha1_context *ctx)
{
	XDK_ASSERT(ctx != NULL);

	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;
	ctx->state[4] = 0xC3D2E1F0;

	return(0);
}

int internal_sha1_process(sha1_context *ctx,
	const byte_t data[64])
{
	dword_t temp, W[16], A, B, C, D, E;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT((const byte_t *)data != NULL);

	GET_UINT32_BE(W[0], data, 0);
	GET_UINT32_BE(W[1], data, 4);
	GET_UINT32_BE(W[2], data, 8);
	GET_UINT32_BE(W[3], data, 12);
	GET_UINT32_BE(W[4], data, 16);
	GET_UINT32_BE(W[5], data, 20);
	GET_UINT32_BE(W[6], data, 24);
	GET_UINT32_BE(W[7], data, 28);
	GET_UINT32_BE(W[8], data, 32);
	GET_UINT32_BE(W[9], data, 36);
	GET_UINT32_BE(W[10], data, 40);
	GET_UINT32_BE(W[11], data, 44);
	GET_UINT32_BE(W[12], data, 48);
	GET_UINT32_BE(W[13], data, 52);
	GET_UINT32_BE(W[14], data, 56);
	GET_UINT32_BE(W[15], data, 60);

#define S(x,n) (((x) << (n)) | (((x) & 0xFFFFFFFF) >> (32 - (n))))

#define R(t)                                                    \
    (                                                           \
        temp = W[( (t) -  3 ) & 0x0F] ^ W[( (t) - 8 ) & 0x0F] ^ \
               W[( (t) - 14 ) & 0x0F] ^ W[  (t)       & 0x0F],  \
        ( W[(t) & 0x0F] = S(temp,1) )                           \
    )

#define P(a,b,c,d,e,x)                                          \
    do                                                          \
	    {                                                           \
        (e) += S((a),5) + F((b),(c),(d)) + K + (x);             \
        (b) = S((b),30);                                        \
	    } while( 0 )

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];

#define F(x,y,z) ((z) ^ ((x) & ((y) ^ (z))))
#define K 0x5A827999

	P(A, B, C, D, E, W[0]);
	P(E, A, B, C, D, W[1]);
	P(D, E, A, B, C, W[2]);
	P(C, D, E, A, B, W[3]);
	P(B, C, D, E, A, W[4]);
	P(A, B, C, D, E, W[5]);
	P(E, A, B, C, D, W[6]);
	P(D, E, A, B, C, W[7]);
	P(C, D, E, A, B, W[8]);
	P(B, C, D, E, A, W[9]);
	P(A, B, C, D, E, W[10]);
	P(E, A, B, C, D, W[11]);
	P(D, E, A, B, C, W[12]);
	P(C, D, E, A, B, W[13]);
	P(B, C, D, E, A, W[14]);
	P(A, B, C, D, E, W[15]);
	P(E, A, B, C, D, R(16));
	P(D, E, A, B, C, R(17));
	P(C, D, E, A, B, R(18));
	P(B, C, D, E, A, R(19));

#undef K
#undef F

#define F(x,y,z) ((x) ^ (y) ^ (z))
#define K 0x6ED9EBA1

	P(A, B, C, D, E, R(20));
	P(E, A, B, C, D, R(21));
	P(D, E, A, B, C, R(22));
	P(C, D, E, A, B, R(23));
	P(B, C, D, E, A, R(24));
	P(A, B, C, D, E, R(25));
	P(E, A, B, C, D, R(26));
	P(D, E, A, B, C, R(27));
	P(C, D, E, A, B, R(28));
	P(B, C, D, E, A, R(29));
	P(A, B, C, D, E, R(30));
	P(E, A, B, C, D, R(31));
	P(D, E, A, B, C, R(32));
	P(C, D, E, A, B, R(33));
	P(B, C, D, E, A, R(34));
	P(A, B, C, D, E, R(35));
	P(E, A, B, C, D, R(36));
	P(D, E, A, B, C, R(37));
	P(C, D, E, A, B, R(38));
	P(B, C, D, E, A, R(39));

#undef K
#undef F

#define F(x,y,z) (((x) & (y)) | ((z) & ((x) | (y))))
#define K 0x8F1BBCDC

	P(A, B, C, D, E, R(40));
	P(E, A, B, C, D, R(41));
	P(D, E, A, B, C, R(42));
	P(C, D, E, A, B, R(43));
	P(B, C, D, E, A, R(44));
	P(A, B, C, D, E, R(45));
	P(E, A, B, C, D, R(46));
	P(D, E, A, B, C, R(47));
	P(C, D, E, A, B, R(48));
	P(B, C, D, E, A, R(49));
	P(A, B, C, D, E, R(50));
	P(E, A, B, C, D, R(51));
	P(D, E, A, B, C, R(52));
	P(C, D, E, A, B, R(53));
	P(B, C, D, E, A, R(54));
	P(A, B, C, D, E, R(55));
	P(E, A, B, C, D, R(56));
	P(D, E, A, B, C, R(57));
	P(C, D, E, A, B, R(58));
	P(B, C, D, E, A, R(59));

#undef K
#undef F

#define F(x,y,z) ((x) ^ (y) ^ (z))
#define K 0xCA62C1D6

	P(A, B, C, D, E, R(60));
	P(E, A, B, C, D, R(61));
	P(D, E, A, B, C, R(62));
	P(C, D, E, A, B, R(63));
	P(B, C, D, E, A, R(64));
	P(A, B, C, D, E, R(65));
	P(E, A, B, C, D, R(66));
	P(D, E, A, B, C, R(67));
	P(C, D, E, A, B, R(68));
	P(B, C, D, E, A, R(69));
	P(A, B, C, D, E, R(70));
	P(E, A, B, C, D, R(71));
	P(D, E, A, B, C, R(72));
	P(C, D, E, A, B, R(73));
	P(B, C, D, E, A, R(74));
	P(A, B, C, D, E, R(75));
	P(E, A, B, C, D, R(76));
	P(D, E, A, B, C, R(77));
	P(C, D, E, A, B, R(78));
	P(B, C, D, E, A, R(79));

#undef K
#undef F

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;
	ctx->state[4] += E;

	return(0);
}

/*
* SHA-1 process buffer
*/
int sha1_update(sha1_context *ctx,
	const byte_t *input,
	dword_t ilen)
{
	int ret;
	dword_t fill;
	dword_t left;

	XDK_ASSERT(ctx != NULL);

	if (ilen == 0)
		return(0);

	left = ctx->total[0] & 0x3F;
	fill = 64 - left;

	ctx->total[0] += (dword_t)ilen;
	ctx->total[0] &= 0xFFFFFFFF;

	if (ctx->total[0] < (dword_t)ilen)
		ctx->total[1]++;

	if (left && ilen >= fill)
	{
		xmem_copy((void *)(ctx->buffer + left), input, fill);

		if ((ret = internal_sha1_process(ctx, ctx->buffer)) != 0)
			return(ret);

		input += fill;
		ilen -= fill;
		left = 0;
	}

	while (ilen >= 64)
	{
		if ((ret = internal_sha1_process(ctx, input)) != 0)
			return(ret);

		input += 64;
		ilen -= 64;
	}

	if (ilen > 0)
		xmem_copy((void *)(ctx->buffer + left), input, ilen);

	return(0);
}

/*
* SHA-1 final digest
*/
int sha1_finish(sha1_context *ctx,
	byte_t output[20])
{
	int ret;
	dword_t used;
	dword_t high, low;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT((byte_t *)output != NULL);

	/*
	* Add padding: 0x80 then 0x00 until 8 bytes remain for the length
	*/
	used = ctx->total[0] & 0x3F;

	ctx->buffer[used++] = 0x80;

	if (used <= 56)
	{
		/* Enough room for padding + length in current block */
		xmem_zero(ctx->buffer + used, 56 - used);
	}
	else
	{
		/* We'll need an extra block */
		xmem_zero(ctx->buffer + used, 64 - used);

		if ((ret = internal_sha1_process(ctx, ctx->buffer)) != 0)
			return(ret);

		xmem_zero(ctx->buffer, 56);
	}

	/*
	* Add message length
	*/
	high = (ctx->total[0] >> 29)
		| (ctx->total[1] << 3);
	low = (ctx->total[0] << 3);

	PUT_UINT32_BE(high, ctx->buffer, 56);
	PUT_UINT32_BE(low, ctx->buffer, 60);

	if ((ret = internal_sha1_process(ctx, ctx->buffer)) != 0)
		return(ret);

	/*
	* Output final state
	*/
	PUT_UINT32_BE(ctx->state[0], output, 0);
	PUT_UINT32_BE(ctx->state[1], output, 4);
	PUT_UINT32_BE(ctx->state[2], output, 8);
	PUT_UINT32_BE(ctx->state[3], output, 12);
	PUT_UINT32_BE(ctx->state[4], output, 16);

	return(0);
}

/*
* output = SHA-1( input buffer )
*/
int sha1(const byte_t *input,
	dword_t ilen,
	byte_t output[20])
{
	int ret;
	sha1_context ctx;

	sha1_init(&ctx);

	if ((ret = sha1_starts(&ctx)) != 0)
		goto exit;

	if ((ret = sha1_update(&ctx, input, ilen)) != 0)
		goto exit;

	if ((ret = sha1_finish(&ctx, output)) != 0)
		goto exit;

exit:
	sha1_free(&ctx);

	return(ret);
}

/*
* MD2 HMAC context setup
*/
int sha1_hmac_starts(sha1_context *ctx, const byte_t *key, dword_t keylen)
{
	int i;
	byte_t sum[20];

	if (keylen > 64)
	{
		sha1(key, keylen, sum);
		keylen = 20;
		key = sum;
	}

	xmem_set(ctx->ipad, 0x36, 64);
	xmem_set(ctx->opad, 0x5C, 64);

	for (i = 0; i < keylen; i++)
	{
		ctx->ipad[i] = (byte_t)(ctx->ipad[i] ^ key[i]);
		ctx->opad[i] = (byte_t)(ctx->opad[i] ^ key[i]);
	}

	sha1_starts(ctx);
	sha1_update(ctx, ctx->ipad, 64);

	xmem_zero(sum, sizeof(sum));

	return (0);
}

/*
* MD2 HMAC process buffer
*/
int sha1_hmac_update(sha1_context *ctx, const byte_t *input, dword_t ilen)
{
	return sha1_update(ctx, input, ilen);
}

/*
* MD2 HMAC final digest
*/
int sha1_hmac_finish(sha1_context *ctx, byte_t output[20])
{
	byte_t tmpbuf[20];

	sha1_finish(ctx, tmpbuf);
	sha1_starts(ctx);
	sha1_update(ctx, ctx->opad, 64);
	sha1_update(ctx, tmpbuf, 20);
	sha1_finish(ctx, output);

	xmem_zero(tmpbuf, sizeof(tmpbuf));

	return (0);
}

int sha1_hmac_reset(sha1_context *ctx)
{
	sha1_starts(ctx);
	sha1_update(ctx, ctx->ipad, 64);

	return (0);
}

/*
* output = HMAC-MD2( hmac key, input buffer )
*/
int sha1_hmac(const byte_t *key, dword_t keylen, const byte_t *input, dword_t ilen,
	byte_t output[20])
{
	sha1_context ctx;

	sha1_hmac_starts(&ctx, key, keylen);
	sha1_hmac_update(&ctx, input, ilen);
	sha1_hmac_finish(&ctx, output);

	xmem_zero(&ctx, sizeof(sha1_context));

	return (0);
}

#if defined(XDK_SUPPORT_TEST)
/*
* FIPS-180-1 test vectors
*/
static const byte_t sha1_test_buf[3][57] =
{
	{ "abc" },
	{ "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq" },
	{ "" }
};

static const dword_t sha1_test_buflen[3] =
{
	3, 56, 1000
};

static const byte_t sha1_test_sum[3][20] =
{
	{ 0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E,
	0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D },
	{ 0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E, 0xBA, 0xAE,
	0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5, 0xE5, 0x46, 0x70, 0xF1 },
	{ 0x34, 0xAA, 0x97, 0x3C, 0xD4, 0xC4, 0xDA, 0xA4, 0xF6, 0x1E,
	0xEB, 0x2B, 0xDB, 0xAD, 0x27, 0x31, 0x65, 0x34, 0x01, 0x6F }
};

/*
* Checkup routine
*/
int sha1_self_test(int verbose)
{
	int i, j, buflen, ret = 0;
	byte_t buf[1024];
	byte_t sha1sum[20];
	sha1_context ctx;

	sha1_init(&ctx);

	/*
	* SHA-1
	*/
	for (i = 0; i < 3; i++)
	{
		if (verbose != 0)
			printf("  SHA-1 test #%d: ", i + 1);

		if ((ret = sha1_starts(&ctx)) != 0)
			goto fail;

		if (i == 2)
		{
			xmem_set(buf, 'a', buflen = 1000);

			for (j = 0; j < 1000; j++)
			{
				ret = sha1_update(&ctx, buf, buflen);
				if (ret != 0)
					goto fail;
			}
		}
		else
		{
			ret = sha1_update(&ctx, sha1_test_buf[i],
				sha1_test_buflen[i]);
			if (ret != 0)
				goto fail;
		}

		if ((ret = sha1_finish(&ctx, sha1sum)) != 0)
			goto fail;

		if (xmem_comp(sha1sum, sha1_test_sum[i], 20) != 0)
		{
			ret = 1;
			goto fail;
		}

		if (verbose != 0)
			printf("passed\n");
	}

	if (verbose != 0)
		printf("\n");

	goto exit;

fail:
	if (verbose != 0)
		printf("failed\n");

exit:
	sha1_free(&ctx);

	return(ret);
}

#endif /* SELF_TEST */