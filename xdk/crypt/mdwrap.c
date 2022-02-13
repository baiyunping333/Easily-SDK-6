/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc md document

	@module	md.c | implement file

	@devnote 张文权 2021.01 - 2021.12	v6.0
***********************************************************************/

/**********************************************************************
This program is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
LICENSE.GPL3 for more details.
***********************************************************************/


#include "mdwrap.h"
#include "md2.h"
#include "md4.h"
#include "md5.h"
#include "ripe160.h"
#include "sha1.h"
#include "sha2.h"
#include "sha4.h"

#include "../xdkimp.h"

const md_info_t md2_info = {
	MD_MD2,
	"MD2",
	16,
	64
};

const md_info_t md4_info = {
	MD_MD4,
	"MD4",
	16,
	64
};

const md_info_t md5_info = {
	MD_MD5,
	"MD5",
	16,
	64
};

const md_info_t ripemd160_info = {
	MD_RIPEMD160,
	"RIPEMD160",
	20,
	64
};

const md_info_t sha1_info = {
	MD_SHA1,
	"SHA1",
	20,
	64
};

const md_info_t sha224_info = {
	MD_SHA224,
	"SHA224",
	28,
	64
};

const md_info_t sha256_info = {
	MD_SHA256,
	"SHA256",
	32,
	64
};

const md_info_t sha384_info = {
	MD_SHA384,
	"SHA384",
	48,
	128
};

const md_info_t sha512_info = {
	MD_SHA512,
	"SHA512",
	64,
	128
};

const md_info_t* md_info_from_type(md_type_t md)
{
	switch (md)
	{
	case MD_MD2:
		return &md2_info;
	case MD_MD4:
		return &md4_info;
	case MD_MD5:
		return &md5_info;
	case MD_RIPEMD160:
		return &ripemd160_info;
	case MD_SHA1:
		return &sha1_info;
	case MD_SHA224:
		return &sha224_info;
	case MD_SHA256:
		return &sha256_info;
	case MD_SHA384:
		return &sha384_info;
	case MD_SHA512:
		return &sha512_info;
	default:
		return NULL;
	}
}

int md_starts(const md_info_t *md_info, void *ctx)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2_starts((md2_context*)ctx);
	case MD_MD4:
		return md4_starts((md4_context*)ctx);
	case MD_MD5:
		return md5_starts((md5_context*)ctx);
	case MD_RIPEMD160:
		return ripemd160_starts((ripemd160_context*)ctx);
	case MD_SHA1:
		return sha1_starts((sha1_context*)ctx);
	case MD_SHA224:
		return sha256_starts((sha256_context*)ctx, 1);
	case MD_SHA256:
		return sha256_starts((sha256_context*)ctx, 0);
	case MD_SHA384:
		return sha512_starts((sha512_context*)ctx, 1);
	case MD_SHA512:
		return sha512_starts((sha512_context*)ctx, 1);
	default:
		return C_ERR;
	}
}

int md_update(const md_info_t *md_info, void *ctx, const byte_t *input, dword_t ilen)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2_update((md2_context*)ctx, input, ilen);
	case MD_MD4:
		return md4_update((md4_context*)ctx, input, ilen);
	case MD_MD5:
		return md5_update((md5_context*)ctx, input, ilen);
	case MD_RIPEMD160:
		return ripemd160_update((ripemd160_context*)ctx, input, ilen);
	case MD_SHA1:
		return sha1_update((sha1_context*)ctx, input, ilen);
	case MD_SHA224:
		return sha256_update((sha256_context*)ctx, input, ilen);
	case MD_SHA256:
		return sha256_update((sha256_context*)ctx, input, ilen);
	case MD_SHA384:
		return sha512_update((sha512_context*)ctx, input, ilen);
	case MD_SHA512:
		return sha512_update((sha512_context*)ctx, input, ilen);
	default:
		return C_ERR;
	}
}

int md_finish(const md_info_t *md_info, void *ctx, byte_t *output)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2_finish((md2_context*)ctx, output);
	case MD_MD4:
		return md4_finish((md4_context*)ctx, output);
	case MD_MD5:
		return md5_finish((md5_context*)ctx, output);
	case MD_RIPEMD160:
		return ripemd160_finish((ripemd160_context*)ctx, output);
	case MD_SHA1:
		return sha1_finish((sha1_context*)ctx, output);
	case MD_SHA224:
		return sha256_finish((sha256_context*)ctx, output);
	case MD_SHA256:
		return sha256_finish((sha256_context*)ctx, output);
	case MD_SHA384:
		return sha512_finish((sha512_context*)ctx, output);
	case MD_SHA512:
		return sha512_finish((sha512_context*)ctx, output);
	default:
		return C_ERR;
	}
}

int md(const md_info_t *md_info, const byte_t *input, dword_t ilen,
	byte_t *output)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2(input, ilen, output);
	case MD_MD4:
		return md4(input, ilen, output);
	case MD_MD5:
		return md5(input, ilen, output);
	case MD_RIPEMD160:
		return ripemd160(input, ilen, output);
	case MD_SHA1:
		return sha1(input, ilen, output);
	case MD_SHA224:
		return sha256(input, ilen, output, 1);
	case MD_SHA256:
		return sha256(input, ilen, output, 0);
	case MD_SHA384:
		return sha512(input, ilen, output, 1);
	case MD_SHA512:
		return sha512(input, ilen, output, 0);
	default:
		return C_ERR;
	}
}

int md_hmac(const md_info_t *md_info, const byte_t *key, dword_t keylen,
	const byte_t *input, dword_t ilen,
	byte_t *output)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2_hmac(key, keylen, input, ilen, output);
	case MD_MD4:
		return md4_hmac(key, keylen, input, ilen, output);
	case MD_MD5:
		return md5_hmac(key, keylen, input, ilen, output);
	case MD_RIPEMD160:
		return ripemd160_hmac(key, keylen, input, ilen, output);
	case MD_SHA1:
		return sha1_hmac(key, keylen, input, ilen, output);
	case MD_SHA224:
		return sha256_hmac(key, keylen, input, ilen, output, 1);
	case MD_SHA256:
		return sha256_hmac(key, keylen, input, ilen, output, 0);
	case MD_SHA384:
		return sha512_hmac(key, keylen, input, ilen, output, 1);
	case MD_SHA512:
		return sha512_hmac(key, keylen, input, ilen, output, 0);
	default:
		return C_ERR;
	}
}


int md_hmac_starts(const md_info_t *md_info, void *ctx, const byte_t *key, dword_t keylen)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2_hmac_starts((md2_context*)ctx, key, keylen);
	case MD_MD4:
		return md4_hmac_starts((md4_context*)ctx, key, keylen);
	case MD_MD5:
		return md5_hmac_starts((md5_context*)ctx, key, keylen);
	case MD_RIPEMD160:
		return ripemd160_hmac_starts((ripemd160_context*)ctx, key, keylen);
	case MD_SHA1:
		return sha1_hmac_starts((sha1_context*)ctx, key, keylen);
	case MD_SHA224:
		return sha256_hmac_starts((sha256_context*)ctx, key, keylen, 1);
	case MD_SHA256:
		return sha256_hmac_starts((sha256_context*)ctx, key, keylen, 0);
	case MD_SHA384:
		return sha512_hmac_starts((sha512_context*)ctx, key, keylen, 1);
	case MD_SHA512:
		return sha512_hmac_starts((sha512_context*)ctx, key, keylen, 1);
	default:
		return C_ERR;
	}
}

int md_hmac_update(const md_info_t *md_info, void *ctx, const byte_t *input, dword_t ilen)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2_hmac_update((md2_context*)ctx, input, ilen);
	case MD_MD4:
		return md4_hmac_update((md4_context*)ctx, input, ilen);
	case MD_MD5:
		return md5_hmac_update((md5_context*)ctx, input, ilen);
	case MD_RIPEMD160:
		return ripemd160_hmac_update((ripemd160_context*)ctx, input, ilen);
	case MD_SHA1:
		return sha1_hmac_update((sha1_context*)ctx, input, ilen);
	case MD_SHA224:
		return sha256_hmac_update((sha256_context*)ctx, input, ilen);
	case MD_SHA256:
		return sha256_hmac_update((sha256_context*)ctx, input, ilen);
	case MD_SHA384:
		return sha512_hmac_update((sha512_context*)ctx, input, ilen);
	case MD_SHA512:
		return sha512_hmac_update((sha512_context*)ctx, input, ilen);
	default:
		return C_ERR;
	}
}

int md_hmac_finish(const md_info_t *md_info, void *ctx, byte_t *output)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2_hmac_finish((md2_context*)ctx, output);
	case MD_MD4:
		return md4_hmac_finish((md4_context*)ctx, output);
	case MD_MD5:
		return md5_hmac_finish((md5_context*)ctx, output);
	case MD_RIPEMD160:
		return ripemd160_hmac_finish((ripemd160_context*)ctx, output);
	case MD_SHA1:
		return sha1_hmac_finish((sha1_context*)ctx, output);
	case MD_SHA224:
		return sha256_hmac_finish((sha256_context*)ctx, output);
	case MD_SHA256:
		return sha256_hmac_finish((sha256_context*)ctx, output);
	case MD_SHA384:
		return sha512_hmac_finish((sha512_context*)ctx, output);
	case MD_SHA512:
		return sha512_hmac_finish((sha512_context*)ctx, output);
	default:
		return C_ERR;
	}
}

int md_hmac_reset(const md_info_t *md_info, void *ctx)
{
	switch (md_info->type)
	{
	case MD_MD2:
		return md2_hmac_reset((md2_context*)ctx);
	case MD_MD4:
		return md4_hmac_reset((md4_context*)ctx);
	case MD_MD5:
		return md5_hmac_reset((md5_context*)ctx);
	case MD_RIPEMD160:
		return ripemd160_hmac_reset((ripemd160_context*)ctx);
	case MD_SHA1:
		return sha1_hmac_reset((sha1_context*)ctx);
	case MD_SHA224:
		return sha256_hmac_reset((sha256_context*)ctx);
	case MD_SHA256:
		return sha256_hmac_reset((sha256_context*)ctx);
	case MD_SHA384:
		return sha512_hmac_reset((sha512_context*)ctx);
	case MD_SHA512:
		return sha512_hmac_reset((sha512_context*)ctx);
	default:
		return C_ERR;
	}
}

void* md_alloc(const md_info_t *md_info)
{
	void* ctx;

	switch (md_info->type)
	{
	case MD_MD2:
		ctx = xmem_alloc(sizeof(md2_context));
		md2_init((md2_context*)ctx);
		return ctx;
	case MD_MD4:
		ctx = xmem_alloc(sizeof(md4_context));
		md4_init((md4_context*)ctx);
		return ctx;
	case MD_MD5:
		ctx = xmem_alloc(sizeof(md5_context));
		md5_init((md5_context*)ctx);
		return ctx;
	case MD_RIPEMD160:
		ctx = xmem_alloc(sizeof(ripemd160_context));
		ripemd160_init((ripemd160_context*)ctx);
		return ctx;
	case MD_SHA1:
		ctx = xmem_alloc(sizeof(sha1_context));
		sha1_init((sha1_context*)ctx);
		return ctx;
	case MD_SHA224:
		ctx = xmem_alloc(sizeof(sha256_context));
		sha256_init((sha256_context*)ctx);
		return ctx;
	case MD_SHA256:
		ctx = xmem_alloc(sizeof(sha256_context));
		sha256_init((sha256_context*)ctx);
		return ctx;
	case MD_SHA384:
		ctx = xmem_alloc(sizeof(sha512_context));
		sha512_init((sha512_context*)ctx);
		return ctx;
	case MD_SHA512:
		ctx = xmem_alloc(sizeof(sha512_context));
		sha512_init((sha512_context*)ctx);
		return ctx;
	default:
		return NULL;
	}
}

void md_free(const md_info_t *md_info, void* ctx)
{
	switch (md_info->type)
	{
	case MD_MD2:
		md2_free((md2_context*)ctx);
		xmem_free(ctx);
		break;
	case MD_MD4:
		md4_free((md4_context*)ctx);
		xmem_free(ctx);
		break;
	case MD_MD5:
		md5_free((md5_context*)ctx);
		xmem_free(ctx);
		break;
	case MD_RIPEMD160:
		ripemd160_free((ripemd160_context*)ctx);
		xmem_free(ctx);
		break;
	case MD_SHA1:
		sha1_free((sha1_context*)ctx);
		xmem_free(ctx);
		break;
	case MD_SHA224:
		sha256_free((sha256_context*)ctx);
		xmem_free(ctx);
		break;
	case MD_SHA256:
		sha256_free((sha256_context*)ctx);
		xmem_free(ctx);
		break;
	case MD_SHA384:
		sha512_free((sha512_context*)ctx);
		xmem_free(ctx);
		break;
	case MD_SHA512:
		sha512_free((sha512_context*)ctx);
		xmem_free(ctx);
		break;
	}
}