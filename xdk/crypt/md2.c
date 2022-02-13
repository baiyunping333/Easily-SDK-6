/*
*  RFC 1115/1319 compliant MD2 implementation
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
*  The MD2 algorithm was designed by Ron Rivest in 1989.
*
*  http://www.ietf.org/rfc/rfc1115.txt
*  http://www.ietf.org/rfc/rfc1319.txt
*/


#include "md2.h"

#include "../xdkimp.h"

static const byte_t PI_SUBST[256] =
{
	0x29, 0x2E, 0x43, 0xC9, 0xA2, 0xD8, 0x7C, 0x01, 0x3D, 0x36,
	0x54, 0xA1, 0xEC, 0xF0, 0x06, 0x13, 0x62, 0xA7, 0x05, 0xF3,
	0xC0, 0xC7, 0x73, 0x8C, 0x98, 0x93, 0x2B, 0xD9, 0xBC, 0x4C,
	0x82, 0xCA, 0x1E, 0x9B, 0x57, 0x3C, 0xFD, 0xD4, 0xE0, 0x16,
	0x67, 0x42, 0x6F, 0x18, 0x8A, 0x17, 0xE5, 0x12, 0xBE, 0x4E,
	0xC4, 0xD6, 0xDA, 0x9E, 0xDE, 0x49, 0xA0, 0xFB, 0xF5, 0x8E,
	0xBB, 0x2F, 0xEE, 0x7A, 0xA9, 0x68, 0x79, 0x91, 0x15, 0xB2,
	0x07, 0x3F, 0x94, 0xC2, 0x10, 0x89, 0x0B, 0x22, 0x5F, 0x21,
	0x80, 0x7F, 0x5D, 0x9A, 0x5A, 0x90, 0x32, 0x27, 0x35, 0x3E,
	0xCC, 0xE7, 0xBF, 0xF7, 0x97, 0x03, 0xFF, 0x19, 0x30, 0xB3,
	0x48, 0xA5, 0xB5, 0xD1, 0xD7, 0x5E, 0x92, 0x2A, 0xAC, 0x56,
	0xAA, 0xC6, 0x4F, 0xB8, 0x38, 0xD2, 0x96, 0xA4, 0x7D, 0xB6,
	0x76, 0xFC, 0x6B, 0xE2, 0x9C, 0x74, 0x04, 0xF1, 0x45, 0x9D,
	0x70, 0x59, 0x64, 0x71, 0x87, 0x20, 0x86, 0x5B, 0xCF, 0x65,
	0xE6, 0x2D, 0xA8, 0x02, 0x1B, 0x60, 0x25, 0xAD, 0xAE, 0xB0,
	0xB9, 0xF6, 0x1C, 0x46, 0x61, 0x69, 0x34, 0x40, 0x7E, 0x0F,
	0x55, 0x47, 0xA3, 0x23, 0xDD, 0x51, 0xAF, 0x3A, 0xC3, 0x5C,
	0xF9, 0xCE, 0xBA, 0xC5, 0xEA, 0x26, 0x2C, 0x53, 0x0D, 0x6E,
	0x85, 0x28, 0x84, 0x09, 0xD3, 0xDF, 0xCD, 0xF4, 0x41, 0x81,
	0x4D, 0x52, 0x6A, 0xDC, 0x37, 0xC8, 0x6C, 0xC1, 0xAB, 0xFA,
	0x24, 0xE1, 0x7B, 0x08, 0x0C, 0xBD, 0xB1, 0x4A, 0x78, 0x88,
	0x95, 0x8B, 0xE3, 0x63, 0xE8, 0x6D, 0xE9, 0xCB, 0xD5, 0xFE,
	0x3B, 0x00, 0x1D, 0x39, 0xF2, 0xEF, 0xB7, 0x0E, 0x66, 0x58,
	0xD0, 0xE4, 0xA6, 0x77, 0x72, 0xF8, 0xEB, 0x75, 0x4B, 0x0A,
	0x31, 0x44, 0x50, 0xB4, 0x8F, 0xED, 0x1F, 0x1A, 0xDB, 0x99,
	0x8D, 0x33, 0x9F, 0x11, 0x83, 0x14
};

void md2_init(md2_context *ctx)
{
	xmem_zero(ctx, sizeof(md2_context));
}

void md2_free(md2_context *ctx)
{
	if (ctx == NULL)
		return;

	xmem_zero(ctx, sizeof(md2_context));
}

void md2_clone(md2_context *dst,
	const md2_context *src)
{
	*dst = *src;
}

/*
* MD2 context setup
*/
int md2_starts(md2_context *ctx)
{
	xmem_zero(ctx->cksum, 16);
	xmem_zero(ctx->state, 46);
	xmem_zero(ctx->buffer, 16);
	ctx->left = 0;

	return(0);
}

int internal_md2_process(md2_context *ctx)
{
	int i, j;
	byte_t t = 0;

	for (i = 0; i < 16; i++)
	{
		ctx->state[i + 16] = ctx->buffer[i];
		ctx->state[i + 32] =
			(byte_t)(ctx->buffer[i] ^ ctx->state[i]);
	}

	for (i = 0; i < 18; i++)
	{
		for (j = 0; j < 48; j++)
		{
			ctx->state[j] = (byte_t)
				(ctx->state[j] ^ PI_SUBST[t]);
			t = ctx->state[j];
		}

		t = (byte_t)(t + i);
	}

	t = ctx->cksum[15];

	for (i = 0; i < 16; i++)
	{
		ctx->cksum[i] = (byte_t)
			(ctx->cksum[i] ^ PI_SUBST[ctx->buffer[i] ^ t]);
		t = ctx->cksum[i];
	}

	return(0);
}

/*
* MD2 process buffer
*/
int md2_update(md2_context *ctx,
	const byte_t *input,
	dword_t ilen)
{
	int ret;
	dword_t fill;

	while (ilen > 0)
	{
		if (ilen > 16 - ctx->left)
			fill = 16 - ctx->left;
		else
			fill = ilen;

		xmem_copy(ctx->buffer + ctx->left, input, fill);

		ctx->left += fill;
		input += fill;
		ilen -= fill;

		if (ctx->left == 16)
		{
			ctx->left = 0;
			if ((ret = internal_md2_process(ctx)) != 0)
				return(ret);
		}
	}

	return(0);
}

/*
* MD2 final digest
*/
int md2_finish(md2_context *ctx,
	byte_t output[16])
{
	int ret;
	dword_t i;
	byte_t x;

	x = (byte_t)(16 - ctx->left);

	for (i = ctx->left; i < 16; i++)
		ctx->buffer[i] = x;

	if ((ret = internal_md2_process(ctx)) != 0)
		return(ret);

	xmem_copy(ctx->buffer, ctx->cksum, 16);
	if ((ret = internal_md2_process(ctx)) != 0)
		return(ret);

	xmem_copy(output, ctx->state, 16);

	return(0);
}

/*
* output = MD2( input buffer )
*/
int md2(const byte_t *input,
	dword_t ilen,
	byte_t output[16])
{
	int ret;
	md2_context ctx;

	md2_init(&ctx);

	if ((ret = md2_starts(&ctx)) != 0)
		goto exit;

	if ((ret = md2_update(&ctx, input, ilen)) != 0)
		goto exit;

	if ((ret = md2_finish(&ctx, output)) != 0)
		goto exit;

exit:
	md2_free(&ctx);

	return(ret);
}

/*
* MD2 HMAC context setup
*/
int md2_hmac_starts(md2_context *ctx, const byte_t *key, dword_t keylen)
{
	int i;
	byte_t sum[16];

	if (keylen > 64)
	{
		md2(key, keylen, sum);
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

	md2_starts(ctx);
	md2_update(ctx, ctx->ipad, 64);

	xmem_zero(sum, sizeof(sum));

	return (0);
}

/*
* MD2 HMAC process buffer
*/
int md2_hmac_update(md2_context *ctx, const byte_t *input, dword_t ilen)
{
	return md2_update(ctx, input, ilen);
}

/*
* MD2 HMAC final digest
*/
int md2_hmac_finish(md2_context *ctx, byte_t output[16])
{
	byte_t tmpbuf[16];

	md2_finish(ctx, tmpbuf);
	md2_starts(ctx);
	md2_update(ctx, ctx->opad, 64);
	md2_update(ctx, tmpbuf, 16);
	md2_finish(ctx, output);

	xmem_zero(tmpbuf, sizeof(tmpbuf));

	return (0);
}

int md2_hmac_reset(md2_context *ctx)
{
	md2_starts(ctx);
	md2_update(ctx, ctx->ipad, 64);

	return (0);
}

/*
* output = HMAC-MD2( hmac key, input buffer )
*/
int md2_hmac(const byte_t *key, dword_t keylen, const byte_t *input, dword_t ilen,
	byte_t output[16])
{
	md2_context ctx;

	md2_hmac_starts(&ctx, key, keylen);
	md2_hmac_update(&ctx, input, ilen);
	md2_hmac_finish(&ctx, output);

	xmem_zero(&ctx, sizeof(md2_context));

	return (0);
}

#if defined(XDK_SUPPORT_TEST)

/*
* RFC 1319 test vectors
*/
static const byte_t md2_test_str[7][81] =
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

static const dword_t md2_test_strlen[7] =
{
	0, 1, 3, 14, 26, 62, 80
};

static const byte_t md2_test_sum[7][16] =
{
	{ 0x83, 0x50, 0xE5, 0xA3, 0xE2, 0x4C, 0x15, 0x3D,
	0xF2, 0x27, 0x5C, 0x9F, 0x80, 0x69, 0x27, 0x73 },
	{ 0x32, 0xEC, 0x01, 0xEC, 0x4A, 0x6D, 0xAC, 0x72,
	0xC0, 0xAB, 0x96, 0xFB, 0x34, 0xC0, 0xB5, 0xD1 },
	{ 0xDA, 0x85, 0x3B, 0x0D, 0x3F, 0x88, 0xD9, 0x9B,
	0x30, 0x28, 0x3A, 0x69, 0xE6, 0xDE, 0xD6, 0xBB },
	{ 0xAB, 0x4F, 0x49, 0x6B, 0xFB, 0x2A, 0x53, 0x0B,
	0x21, 0x9F, 0xF3, 0x30, 0x31, 0xFE, 0x06, 0xB0 },
	{ 0x4E, 0x8D, 0xDF, 0xF3, 0x65, 0x02, 0x92, 0xAB,
	0x5A, 0x41, 0x08, 0xC3, 0xAA, 0x47, 0x94, 0x0B },
	{ 0xDA, 0x33, 0xDE, 0xF2, 0xA4, 0x2D, 0xF1, 0x39,
	0x75, 0x35, 0x28, 0x46, 0xC3, 0x03, 0x38, 0xCD },
	{ 0xD5, 0x97, 0x6F, 0x79, 0xD8, 0x3D, 0x3A, 0x0D,
	0xC9, 0x80, 0x6C, 0x3C, 0x66, 0xF3, 0xEF, 0xD8 }
};

/*
* Checkup routine
*/
int md2_self_test(int verbose)
{
	int i, ret = 0;
	byte_t md2sum[16];

	for (i = 0; i < 7; i++)
	{
		if (verbose != 0)
			printf("  MD2 test #%d: ", i + 1);

		ret = md2(md2_test_str[i], md2_test_strlen[i], md2sum);
		if (ret != 0)
			goto fail;

		if (xmem_comp(md2sum, md2_test_sum[i], 16) != 0)
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
