/*
*  FIPS-180-1 compliant SM3 implementation
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
*  The SM3 standard was published by NIST in 1993.
*
*  http://www.itl.nist.gov/fipspubs/fip180-1.htm
*/


#include "sm3.h"

#include "../xdkimp.h"

#define HASH_PADDING_PATTERN 	0x80
#define HASH_ROUND_NUM			64

/*
* 32-bit integer manipulation macros (big endian)
*/
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                            \
do {                                                    \
    (n) = ( (dword_t) (b)[(i)    ] << 24 )             \
        | ( (dword_t) (b)[(i) + 1] << 16 )             \
        | ( (dword_t) (b)[(i) + 2] <<  8 )             \
        | ( (dword_t) (b)[(i) + 3]       );            \
} while( 0 )
#endif

#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                            \
do {                                                    \
    (b)[(i)    ] = (byte_t) ( (n) >> 24 );       \
    (b)[(i) + 1] = (byte_t) ( (n) >> 16 );       \
    (b)[(i) + 2] = (byte_t) ( (n) >>  8 );       \
    (b)[(i) + 3] = (byte_t) ( (n)       );       \
} while( 0 )
#endif

/* SM3 Constants */
static dword_t T[2] =
{
	0x79CC4519, 0x7A879D8A
};

/* ROTate Left (circular left shift) */
static dword_t ROTL(dword_t x, byte_t shift)
{
	shift %= 32;
	return (x << shift) | (x >> (32 - shift));
}

static dword_t FF(dword_t x, dword_t y, dword_t z, dword_t j)
{
	if (j<16) /* 0 <= j <= 15 */
	{
		return x ^ y ^ z;
	}
	else /* 16 <= j <= 63 */
	{
		return (x & y) | (x & z) | (y & z);
	}
}

static dword_t GG(dword_t x, dword_t y, dword_t z, dword_t j)
{
	if (j<16) /* 0 <= j <= 15 */
	{
		return x ^ y ^ z;
	}
	else /* 16 <= j <= 63 */
	{
		return (x & y) | (~x & z);
	}
}

/* P0, Permutation 0 */
static dword_t P0(dword_t x)
{
	return x ^ ROTL(x, 9) ^ ROTL(x, 17);
}

/* P1, Permutation 1 */
static dword_t P1(dword_t x)
{
	return x ^ ROTL(x, 15) ^ ROTL(x, 23);
}

void sm3_init(sm3_context *ctx)
{
	XDK_ASSERT(ctx != NULL);

	xmem_zero(ctx, sizeof(sm3_context));
}

void sm3_free(sm3_context *ctx)
{
	if (ctx == NULL)
		return;

	xmem_zero(ctx, sizeof(sm3_context));
}

void sm3_clone(sm3_context *dst,
	const sm3_context *src)
{
	XDK_ASSERT(dst != NULL && src != NULL);

	xmem_copy((void*)dst, (void*)src, sizeof(sm3_context));
}

/*
* SM3 context setup
*/
int sm3_starts(sm3_context *ctx)
{
	XDK_ASSERT(ctx != NULL);

	ctx->total[0] = 0;
	ctx->total[1] = 0;

	ctx->state[0] = 0x7380166f;
	ctx->state[1] = 0x4914b2b9;
	ctx->state[2] = 0x172442d7;
	ctx->state[3] = 0xda8a0600;
	ctx->state[4] = 0xa96f30bc;
	ctx->state[5] = 0x163138aa;
	ctx->state[6] = 0xe38dee4d;
	ctx->state[7] = 0xb0fb0e4e;

	return(0);
}

int internal_sm3_process(sm3_context *ctx,
	const byte_t data[64])
{
	dword_t W[HASH_ROUND_NUM + 4], Wp[HASH_ROUND_NUM];
	int j;
	dword_t A, B, C, D, E, F, G, H;
	dword_t SS1, SS2;
	dword_t TT1, TT2;

	/* Array W */
	for (j = 0; j<(HASH_ROUND_NUM + 4); j++)
	{
		if (j <= 15) /*  0 <= j <= 15 */
			GET_UINT32_BE(W[j], data, 4 * j);
		else	   /* 16 <= j <= 67 */
			W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROTL(W[j - 3], 15)) ^ ROTL(W[j - 13], 7) ^ W[j - 6];
	}

	/* Array W Prime */
	for (j = 0; j<HASH_ROUND_NUM; j++)
	{
		Wp[j] = W[j] ^ W[j + 4];
	}

	A = ctx->state[0];
	B = ctx->state[1];
	C = ctx->state[2];
	D = ctx->state[3];
	E = ctx->state[4];
	F = ctx->state[5];
	G = ctx->state[6];
	H = ctx->state[7];

	for (j = 0; j<HASH_ROUND_NUM; j++)
	{
		SS1 = ROTL(ROTL(A, 12) + E + ROTL(T[j<16 ? 0 : 1], j), 7);
		SS2 = SS1 ^ ROTL(A, 12);
		TT1 = FF(A, B, C, j) + D + SS2 + Wp[j];
		TT2 = GG(E, F, G, j) + H + SS1 + W[j];
		D = C;
		C = ROTL(B, 9);
		B = A;
		A = TT1;
		H = G;
		G = ROTL(F, 19);
		F = E;
		E = P0(TT2);
	}

	ctx->state[0] ^= A;
	ctx->state[1] ^= B;
	ctx->state[2] ^= C;
	ctx->state[3] ^= D;
	ctx->state[4] ^= E;
	ctx->state[5] ^= F;
	ctx->state[6] ^= G;
	ctx->state[7] ^= H;

	return(0);
}

/*
* SM3 process buffer
*/
int sm3_update(sm3_context *ctx,
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

		if ((ret = internal_sm3_process(ctx, ctx->buffer)) != 0)
			return(ret);

		input += fill;
		ilen -= fill;
		left = 0;
	}

	while (ilen >= 64)
	{
		if ((ret = internal_sm3_process(ctx, input)) != 0)
			return(ret);

		input += 64;
		ilen -= 64;
	}

	if (ilen > 0)
		xmem_copy((void *)(ctx->buffer + left), input, ilen);

	return(0);
}

/*
* SM3 final digest
*/
int sm3_finish(sm3_context *ctx,
	byte_t output[32])
{
	int ret;
	dword_t used;
	dword_t high, low;

	XDK_ASSERT(ctx != NULL);

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

		if ((ret = internal_sm3_process(ctx, ctx->buffer)) != 0)
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

	if ((ret = internal_sm3_process(ctx, ctx->buffer)) != 0)
		return(ret);

	/*
	* Output final state
	*/
	PUT_UINT32_BE(ctx->state[0], output, 0);
	PUT_UINT32_BE(ctx->state[1], output, 4);
	PUT_UINT32_BE(ctx->state[2], output, 8);
	PUT_UINT32_BE(ctx->state[3], output, 12);
	PUT_UINT32_BE(ctx->state[4], output, 16);
	PUT_UINT32_BE(ctx->state[5], output, 20);
	PUT_UINT32_BE(ctx->state[6], output, 24);
	PUT_UINT32_BE(ctx->state[7], output, 28);

	return(0);
}

/*
* output = SM3( input buffer )
*/
int sm3(const byte_t *input,
	dword_t ilen,
	byte_t output[32])
{
	int ret;
	sm3_context ctx;

	sm3_init(&ctx);

	if ((ret = sm3_starts(&ctx)) != 0)
		goto exit;

	if ((ret = sm3_update(&ctx, input, ilen)) != 0)
		goto exit;

	if ((ret = sm3_finish(&ctx, output)) != 0)
		goto exit;

exit:
	sm3_free(&ctx);

	return(ret);
}

/*
* MD2 HMAC context setup
*/
int sm3_hmac_starts(sm3_context *ctx, const byte_t *key, dword_t keylen)
{
	int i;
	byte_t sum[32];

	if (keylen > 64)
	{
		sm3(key, keylen, sum);
		key = sum;
	}

	xmem_set(ctx->ipad, 0x36, 64);
	xmem_set(ctx->opad, 0x5C, 64);

	for (i = 0; i < keylen; i++)
	{
		ctx->ipad[i] = (byte_t)(ctx->ipad[i] ^ key[i]);
		ctx->opad[i] = (byte_t)(ctx->opad[i] ^ key[i]);
	}

	sm3_starts(ctx);
	sm3_update(ctx, ctx->ipad, 64);

	xmem_zero(sum, sizeof(sum));

	return (0);
}

/*
* MD2 HMAC process buffer
*/
int sm3_hmac_update(sm3_context *ctx, const byte_t *input, dword_t ilen)
{
	return sm3_update(ctx, input, ilen);
}

/*
* MD2 HMAC final digest
*/
int sm3_hmac_finish(sm3_context *ctx, byte_t output[32])
{
	byte_t tmpbuf[32];

	sm3_finish(ctx, tmpbuf);
	sm3_starts(ctx);
	sm3_update(ctx, ctx->opad, 64);
	sm3_update(ctx, tmpbuf, 32);
	sm3_finish(ctx, output);

	xmem_zero(tmpbuf, sizeof(tmpbuf));

	return (0);
}

int sm3_hmac_reset(sm3_context *ctx)
{
	sm3_starts(ctx);
	sm3_update(ctx, ctx->ipad, 64);

	return (0);
}

/*
* output = HMAC-MD2( hmac key, input buffer )
*/
int sm3_hmac(const byte_t *key, dword_t keylen, const byte_t *input, dword_t ilen,
	byte_t output[32])
{
	sm3_context ctx;

	sm3_hmac_starts(&ctx, key, keylen);
	sm3_hmac_update(&ctx, input, ilen);
	sm3_hmac_finish(&ctx, output);

	xmem_zero(&ctx, sizeof(sm3_context));

	return (0);
}

#if defined(XDK_SUPPORT_TEST)
typedef struct SM3_TEST_ITEM {
	char        *str;
	uint32_t    len;
	unsigned char md[64];
};

static struct SM3_TEST_ITEM sm3_items[] =
{
	{ /* 0 */
		"",
		0,
		"1ab21d8355cfa17f8e61194831e81a8f22bec8c728fefb747ed035eb5082aa2b"
	},
	{ /* 1 */
		"a",
		1,
		"623476ac18f65a2909e43c7fec61b49c7e764a91a18ccb82f1917a29c86c5e88"
	},
	{ /* 2 */
		"abc",
		3,
		"66c7f0f462eeedd9d1f2d46bdc10e4e24167c4875cf2f7a2297da02b8f4ba8e0"
	},
	{ /* 3 */
		"message digest",
		14,
		"c522a942e89bd80d97dd666e7a5531b36188c9817149e9b258dfe51ece98ed77"
	},
	{ /* 4 */
		"abcdefghijklmnopqrstuvwxyz",
		26,
		"b80fe97a4da24afc277564f66a359ef440462ad28dcc6d63adb24d5c20a61595"
	},
	{ /* 5 */
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
		62,
		"2971d10c8842b70c979e55063480c50bacffd90e98e2e60d2512ab8abfdfcec5"
	},
	{ /* 6 */
		"12345678901234567890123456789012345678901234567890123456789012345678901234567890",
		80,
		"ad81805321f3e69d251235bf886a564844873b56dd7dde400f055b7dde39307a"
	},
	{ /* 7 */
		"abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd",
		64,
		"debe9ff92275b8a138604889c18e5a4d6fdb70e5387e5765293dcba39c0c5732"
	},
};


/*
* Checkup routine
*/
int sm3_self_test(int verbose)
{
	int i, j, ret = 0;
	byte_t sm3sum[32];
	unsigned char sm3str[64+1];
	sm3_context ctx;

	sm3_init(&ctx);

	/*
	* SM3
	*/
	for (i = 0; i < sizeof(sm3_items) / sizeof(struct SM3_TEST_ITEM); i++)
	{
		if (verbose != 0)
			printf("  SM3 test #%d: ", i + 1);

		if ((ret = sm3_starts(&ctx)) != 0)
			goto fail;

		if((ret = sm3_update(&ctx, sm3_items[i].str, sm3_items[i].len)) != 0)
			goto fail;

		if ((ret = sm3_finish(&ctx, sm3sum)) != 0)
			goto fail;

		for (j = 0; j < 32; j++)
		{
			sprintf((char*)(sm3str+j*2),"%02x", sm3sum[j]);
		}

		if (xmem_comp(sm3str, sm3_items[i].md, 64) != 0)
		{
			ret = 1;
			if (verbose != 0)
				printf("failed\n");
		}
		else if (verbose != 0)
			printf("passed\n");
	}

	if (verbose != 0)
		printf("\n");

	goto exit;

fail:
	if (verbose != 0)
		printf("failed\n");

exit:
	sm3_free(&ctx);

	return(ret);
}

#endif /* SELF_TEST */