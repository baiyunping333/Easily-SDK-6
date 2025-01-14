﻿/*
*  RFC 1321 compliant MD5 implementation
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
*  The MD5 algorithm was designed by Ron Rivest in 1991.
*
*  http://www.ietf.org/rfc/rfc1321.txt
*/

#include "md5.h"

#include "../xdkimp.h"

/*
* 32-bit integer manipulation macros (little endian)
*/
#ifndef GET_UINT32_LE
#define GET_UINT32_LE(n,b,i)                            \
{                                                       \
    (n) = ( (dword_t) (b)[(i)    ]       )             \
        | ( (dword_t) (b)[(i) + 1] <<  8 )             \
        | ( (dword_t) (b)[(i) + 2] << 16 )             \
        | ( (dword_t) (b)[(i) + 3] << 24 );            \
}
#endif

#ifndef PUT_UINT32_LE
#define PUT_UINT32_LE(n,b,i)                                    \
{                                                               \
    (b)[(i)    ] = (byte_t) ( ( (n)       ) & 0xFF );    \
    (b)[(i) + 1] = (byte_t) ( ( (n) >>  8 ) & 0xFF );    \
    (b)[(i) + 2] = (byte_t) ( ( (n) >> 16 ) & 0xFF );    \
    (b)[(i) + 3] = (byte_t) ( ( (n) >> 24 ) & 0xFF );    \
}
#endif

void md5_init(md5_context *ctx)
{
	xmem_zero(ctx, sizeof(md5_context));
}

void md5_free(md5_context *ctx)
{
	if (ctx == NULL)
		return;

	xmem_zero(ctx, sizeof(md5_context));
}

void md5_clone(md5_context *dst,
	const md5_context *src)
{
	*dst = *src;
}

/*
* MD5 context setup
*/
int md5_starts(md5_context *ctx)
{
	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x67452301;
	ctx->state[1] = 0xEFCDAB89;
	ctx->state[2] = 0x98BADCFE;
	ctx->state[3] = 0x10325476;

	return(0);
}

int internal_md5_process(md5_context *ctx,
	const byte_t data[64])
{
	dword_t X[16], A, B, C, D;

	GET_UINT32_LE(X[0], data, 0);
	GET_UINT32_LE(X[1], data, 4);
	GET_UINT32_LE(X[2], data, 8);
	GET_UINT32_LE(X[3], data, 12);
	GET_UINT32_LE(X[4], data, 16);
	GET_UINT32_LE(X[5], data, 20);
	GET_UINT32_LE(X[6], data, 24);
	GET_UINT32_LE(X[7], data, 28);
	GET_UINT32_LE(X[8], data, 32);
	GET_UINT32_LE(X[9], data, 36);
	GET_UINT32_LE(X[10], data, 40);
	GET_UINT32_LE(X[11], data, 44);
	GET_UINT32_LE(X[12], data, 48);
	GET_UINT32_LE(X[13], data, 52);
	GET_UINT32_LE(X[14], data, 56);
	GET_UINT32_LE(X[15], data, 60);

#define S(x,n)                                                          \
    ( ( (x) << (n) ) | ( ( (x) & 0xFFFFFFFF) >> ( 32 - (n) ) ) )

#define P(a,b,c,d,k,s,t)                                        \
    do                                                          \
	    {                                                           \
        (a) += F((b),(c),(d)) + X[(k)] + (t);                   \
        (a) = S((a),(s)) + (b);                                 \
	    } while( 0 )

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];

#define F(x,y,z) ((z) ^ ((x) & ((y) ^ (z))))

	P(A, B, C, D, 0, 7, 0xD76AA478);
	P(D, A, B, C, 1, 12, 0xE8C7B756);
	P(C, D, A, B, 2, 17, 0x242070DB);
	P(B, C, D, A, 3, 22, 0xC1BDCEEE);
	P(A, B, C, D, 4, 7, 0xF57C0FAF);
	P(D, A, B, C, 5, 12, 0x4787C62A);
	P(C, D, A, B, 6, 17, 0xA8304613);
	P(B, C, D, A, 7, 22, 0xFD469501);
	P(A, B, C, D, 8, 7, 0x698098D8);
	P(D, A, B, C, 9, 12, 0x8B44F7AF);
	P(C, D, A, B, 10, 17, 0xFFFF5BB1);
	P(B, C, D, A, 11, 22, 0x895CD7BE);
	P(A, B, C, D, 12, 7, 0x6B901122);
	P(D, A, B, C, 13, 12, 0xFD987193);
	P(C, D, A, B, 14, 17, 0xA679438E);
	P(B, C, D, A, 15, 22, 0x49B40821);

#undef F

#define F(x,y,z) ((y) ^ ((z) & ((x) ^ (y))))

	P(A, B, C, D, 1, 5, 0xF61E2562);
	P(D, A, B, C, 6, 9, 0xC040B340);
	P(C, D, A, B, 11, 14, 0x265E5A51);
	P(B, C, D, A, 0, 20, 0xE9B6C7AA);
	P(A, B, C, D, 5, 5, 0xD62F105D);
	P(D, A, B, C, 10, 9, 0x02441453);
	P(C, D, A, B, 15, 14, 0xD8A1E681);
	P(B, C, D, A, 4, 20, 0xE7D3FBC8);
	P(A, B, C, D, 9, 5, 0x21E1CDE6);
	P(D, A, B, C, 14, 9, 0xC33707D6);
	P(C, D, A, B, 3, 14, 0xF4D50D87);
	P(B, C, D, A, 8, 20, 0x455A14ED);
	P(A, B, C, D, 13, 5, 0xA9E3E905);
	P(D, A, B, C, 2, 9, 0xFCEFA3F8);
	P(C, D, A, B, 7, 14, 0x676F02D9);
	P(B, C, D, A, 12, 20, 0x8D2A4C8A);

#undef F

#define F(x,y,z) ((x) ^ (y) ^ (z))

	P(A, B, C, D, 5, 4, 0xFFFA3942);
	P(D, A, B, C, 8, 11, 0x8771F681);
	P(C, D, A, B, 11, 16, 0x6D9D6122);
	P(B, C, D, A, 14, 23, 0xFDE5380C);
	P(A, B, C, D, 1, 4, 0xA4BEEA44);
	P(D, A, B, C, 4, 11, 0x4BDECFA9);
	P(C, D, A, B, 7, 16, 0xF6BB4B60);
	P(B, C, D, A, 10, 23, 0xBEBFBC70);
	P(A, B, C, D, 13, 4, 0x289B7EC6);
	P(D, A, B, C, 0, 11, 0xEAA127FA);
	P(C, D, A, B, 3, 16, 0xD4EF3085);
	P(B, C, D, A, 6, 23, 0x04881D05);
	P(A, B, C, D, 9, 4, 0xD9D4D039);
	P(D, A, B, C, 12, 11, 0xE6DB99E5);
	P(C, D, A, B, 15, 16, 0x1FA27CF8);
	P(B, C, D, A, 2, 23, 0xC4AC5665);

#undef F

#define F(x,y,z) ((y) ^ ((x) | ~(z)))

	P(A, B, C, D, 0, 6, 0xF4292244);
	P(D, A, B, C, 7, 10, 0x432AFF97);
	P(C, D, A, B, 14, 15, 0xAB9423A7);
	P(B, C, D, A, 5, 21, 0xFC93A039);
	P(A, B, C, D, 12, 6, 0x655B59C3);
	P(D, A, B, C, 3, 10, 0x8F0CCC92);
	P(C, D, A, B, 10, 15, 0xFFEFF47D);
	P(B, C, D, A, 1, 21, 0x85845DD1);
	P(A, B, C, D, 8, 6, 0x6FA87E4F);
	P(D, A, B, C, 15, 10, 0xFE2CE6E0);
	P(C, D, A, B, 6, 15, 0xA3014314);
	P(B, C, D, A, 13, 21, 0x4E0811A1);
	P(A, B, C, D, 4, 6, 0xF7537E82);
	P(D, A, B, C, 11, 10, 0xBD3AF235);
	P(C, D, A, B, 2, 15, 0x2AD7D2BB);
	P(B, C, D, A, 9, 21, 0xEB86D391);

#undef F

	ctx->state[0] += A;
	ctx->state[1] += B;
	ctx->state[2] += C;
	ctx->state[3] += D;

	return(0);
}

/*
* MD5 process buffer
*/
int md5_update(md5_context *ctx,
	const byte_t *input,
	dword_t ilen)
{
	int ret;
	dword_t fill;
	dword_t left;

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
		if ((ret = internal_md5_process(ctx, ctx->buffer)) != 0)
			return(ret);

		input += fill;
		ilen -= fill;
		left = 0;
	}

	while (ilen >= 64)
	{
		if ((ret = internal_md5_process(ctx, input)) != 0)
			return(ret);

		input += 64;
		ilen -= 64;
	}

	if (ilen > 0)
	{
		xmem_copy((void *)(ctx->buffer + left), input, ilen);
	}

	return(0);
}

/*
* MD5 final digest
*/
int md5_finish(md5_context *ctx,
	byte_t output[16])
{
	int ret;
	dword_t used;
	dword_t high, low;

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

		if ((ret = internal_md5_process(ctx, ctx->buffer)) != 0)
			return(ret);

		xmem_zero(ctx->buffer, 56);
	}

	/*
	* Add message length
	*/
	high = (ctx->total[0] >> 29)
		| (ctx->total[1] << 3);
	low = (ctx->total[0] << 3);

	PUT_UINT32_LE(low, ctx->buffer, 56);
	PUT_UINT32_LE(high, ctx->buffer, 60);

	if ((ret = internal_md5_process(ctx, ctx->buffer)) != 0)
		return(ret);

	/*
	* Output final state
	*/
	PUT_UINT32_LE(ctx->state[0], output, 0);
	PUT_UINT32_LE(ctx->state[1], output, 4);
	PUT_UINT32_LE(ctx->state[2], output, 8);
	PUT_UINT32_LE(ctx->state[3], output, 12);

	return(0);
}

/*
* output = MD5( input buffer )
*/
int md5(const byte_t *input,
	dword_t ilen,
	byte_t output[16])
{
	int ret;
	md5_context ctx;

	md5_init(&ctx);

	if ((ret = md5_starts(&ctx)) != 0)
		goto exit;

	if ((ret = md5_update(&ctx, input, ilen)) != 0)
		goto exit;

	if ((ret = md5_finish(&ctx, output)) != 0)
		goto exit;

exit:
	md5_free(&ctx);

	return(ret);
}

/*
* MD2 HMAC context setup
*/
int md5_hmac_starts(md5_context *ctx, const byte_t *key, dword_t keylen)
{
	int i;
	byte_t sum[16];

	if (keylen > 64)
	{
		md5(key, keylen, sum);
		keylen = 16;
		key = sum;
	}

	xmem_set(ctx->ipad, 0x36, 64);
	xmem_set(ctx->opad, 0x5C, 64);

	for (i = 0; i < keylen; i++)
	{
		ctx->ipad[i] = (byte_t)(ctx->ipad[i] ^ key[i]);
		ctx->opad[i] = (byte_t)(ctx->opad[i] ^ key[i]);
	}

	md5_starts(ctx);
	md5_update(ctx, ctx->ipad, 64);

	xmem_zero(sum, sizeof(sum));

	return (0);
}

/*
* MD2 HMAC process buffer
*/
int md5_hmac_update(md5_context *ctx, const byte_t *input, dword_t ilen)
{
	return md5_update(ctx, input, ilen);
}

/*
* MD2 HMAC final digest
*/
int md5_hmac_finish(md5_context *ctx, byte_t output[16])
{
	byte_t tmpbuf[16];

	md5_finish(ctx, tmpbuf);
	md5_starts(ctx);
	md5_update(ctx, ctx->opad, 64);
	md5_update(ctx, tmpbuf, 16);
	md5_finish(ctx, output);

	xmem_zero(tmpbuf, sizeof(tmpbuf));

	return (0);
}

int md5_hmac_reset(md5_context *ctx)
{
	md5_starts(ctx);
	md5_update(ctx, ctx->ipad, 64);

	return (0);
}

/*
* output = HMAC-MD2( hmac key, input buffer )
*/
int md5_hmac(const byte_t *key, dword_t keylen, const byte_t *input, dword_t ilen,
	byte_t output[16])
{
	md5_context ctx;

	md5_hmac_starts(&ctx, key, keylen);
	md5_hmac_update(&ctx, input, ilen);
	md5_hmac_finish(&ctx, output);

	xmem_zero(&ctx, sizeof(md5_context));

	return (0);
}

#if defined(XDK_SUPPORT_TEST)
/*
* RFC 1321 test vectors
*/
static const byte_t md5_test_buf[7][81] =
{
	{ "" },
	{ "a" },
	{ "abc" },
	{ "message digest" },
	{ "abcdefghijklmnopqrstuvwxyz" },
	{ "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789" },
	{ "12345678901234567890123456789012345678901234567890123456789012"
	"345678901234567890" }
};

static const dword_t md5_test_buflen[7] =
{
	0, 1, 3, 14, 26, 62, 80
};

static const byte_t md5_test_sum[7][16] =
{
	{ 0xD4, 0x1D, 0x8C, 0xD9, 0x8F, 0x00, 0xB2, 0x04,
	0xE9, 0x80, 0x09, 0x98, 0xEC, 0xF8, 0x42, 0x7E },
	{ 0x0C, 0xC1, 0x75, 0xB9, 0xC0, 0xF1, 0xB6, 0xA8,
	0x31, 0xC3, 0x99, 0xE2, 0x69, 0x77, 0x26, 0x61 },
	{ 0x90, 0x01, 0x50, 0x98, 0x3C, 0xD2, 0x4F, 0xB0,
	0xD6, 0x96, 0x3F, 0x7D, 0x28, 0xE1, 0x7F, 0x72 },
	{ 0xF9, 0x6B, 0x69, 0x7D, 0x7C, 0xB7, 0x93, 0x8D,
	0x52, 0x5A, 0x2F, 0x31, 0xAA, 0xF1, 0x61, 0xD0 },
	{ 0xC3, 0xFC, 0xD3, 0xD7, 0x61, 0x92, 0xE4, 0x00,
	0x7D, 0xFB, 0x49, 0x6C, 0xCA, 0x67, 0xE1, 0x3B },
	{ 0xD1, 0x74, 0xAB, 0x98, 0xD2, 0x77, 0xD9, 0xF5,
	0xA5, 0x61, 0x1C, 0x2C, 0x9F, 0x41, 0x9D, 0x9F },
	{ 0x57, 0xED, 0xF4, 0xA2, 0x2B, 0xE3, 0xC9, 0x55,
	0xAC, 0x49, 0xDA, 0x2E, 0x21, 0x07, 0xB6, 0x7A }
};

/*
* Checkup routine
*/
int md5_self_test(int verbose)
{
	int i, ret = 0;
	byte_t md5sum[16];

	for (i = 0; i < 7; i++)
	{
		if (verbose != 0)
			printf("  MD5 test #%d: ", i + 1);

		ret = md5(md5_test_buf[i], md5_test_buflen[i], md5sum);
		if (ret != 0)
			goto fail;

		if (xmem_comp(md5sum, md5_test_sum[i], 16) != 0)
		{
			ret = 1;
			goto fail;
		}

		if (verbose != 0)
			printf("passed\n");
	}

	if (verbose != 0)
		printf("\n");

	return(0);

fail:
	if (verbose != 0)
		printf("failed\n");

	return(ret);
}

#endif /* SELF_TEST */