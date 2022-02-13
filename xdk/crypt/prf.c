/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc ssl interface document

	@module	sslprf.c | implement file

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

#include "prf.h"
#include "md5.h"
#include "sha1.h"
#include "sha2.h"
#include "sha4.h"
#include "mdwrap.h"
#include "hkdf.h"

#include "../xdkimp.h"

//master_secret =
//MD5(pre_master_secret + SHA('A' + pre_master_secret + ClientHello.random + ServerHello.random)) +
//MD5(pre_master_secret + SHA('BB' + pre_master_secret + ClientHello.random + ServerHello.random)) +
//MD5(pre_master_secret + SHA('CCC' + pre_master_secret + ClientHello.random + ServerHello.random));
void ssl_prf0(byte_t *secret, int slen, byte_t *random, int rlen, byte_t *dstbuf, int dlen)
{
	int i, mul;
	byte_t padding[16] = { 0 };
	byte_t sha1sum[20] = { 0 };

	sha1_context sha1 = { 0 };
	md5_context md5 = { 0 };

	mul = dlen / 16;
	for (i = 0; i < mul; i++)
	{
		xmem_set((void*)padding, 'A' + i, 1 + i);

		sha1_starts(&sha1);
		sha1_update(&sha1, padding, 1 + i);
		sha1_update(&sha1, secret, slen);
		sha1_update(&sha1, random, rlen);
		sha1_finish(&sha1, sha1sum);

		md5_starts(&md5);
		md5_update(&md5, secret, slen);
		md5_update(&md5, sha1sum, 20);
		md5_finish(&md5, dstbuf + i * 16);
	}
}

/*
PRF1(secret, label, seed) = P_MD5(S1, label + seed) XOR P_SHA-1(S2, label + seed);
L_S = length in bytes of secret;
L_S1 = L_S2 = ceil(L_S / 2);

P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) + HMAC_hash(secret, A(2) + seed) + HMAC_hash(secret, A(3) + seed) + ...
A(0) = seed
A(i) = HMAC_hash(secret, A(i-1))
*/

void ssl_prf1(byte_t *secret, int slen, char *label, byte_t *random, int rlen, byte_t *dstbuf, int dlen)
{
	int nb, hs;
	int i, j, k;
	byte_t *S1, *S2;
	byte_t seed[128] = { 0 };
	byte_t hash[20] = { 0 };

	hs = (slen + 1) / 2;
	S1 = secret;
	S2 = secret + slen - hs;

	nb = strlen(label);
	xmem_copy(seed + 20, label, nb);
	xmem_copy(seed + 20 + nb, random, rlen);
	nb += rlen;

	// First compute P_md5(secret,label+random)[0..dlen]
	//the MD5 output length is 16 bytes
	//A(0)
	md5_hmac(S1, hs, seed + 20, nb, seed + 4);

	for (i = 0; i < dlen; i += 16)
	{
		md5_hmac(S1, hs, seed + 4, 16 + nb, hash);
		//A(i)
		md5_hmac(S1, hs, seed + 4, 16, seed + 4);

		k = (i + 16 > dlen) ? dlen % 16 : 16;

		for (j = 0; j < k; j++)
			dstbuf[i + j] = hash[j];
	}

	// XOR out with P_sha1(secret,label+random)[0..dlen]
	//A(0)
	sha1_hmac(S2, hs, seed + 20, nb, seed);

	for (i = 0; i < dlen; i += 20)
	{
		sha1_hmac(S2, hs, seed, 20 + nb, hash);
		//A(i)
		sha1_hmac(S2, hs, seed, 20, seed);

		k = (i + 20 > dlen) ? dlen % 20 : 20;

		for (j = 0; j < k; j++)
			dstbuf[i + j] = (byte_t)(dstbuf[i + j] ^ hash[j]);
	}
}

/*
PRF2(secret, label, seed) = P_<hash>(secret, label + seed)
L_S = length in bytes of secret;
L_S1 = L_S2 = ceil(L_S / 2);

P_hash(secret, seed) = HMAC_hash(secret, A(1) + seed) + HMAC_hash(secret, A(2) + seed) + HMAC_hash(secret, A(3) + seed) + ...
A(0) = seed
A(i) = HMAC_hash(secret, A(i-1))
*/

void ssl_prf2(byte_t *secret, int slen, char *label, byte_t *random, int rlen, byte_t *dstbuf, int dlen)
{
	int nb, hs;
	int i, j, k;
	byte_t *S1;
	byte_t seed[128] = { 0 };
	byte_t hash[32] = { 0 };

	S1 = secret;
	hs = slen;

	nb = strlen(label);
	xmem_copy(seed + 32, label, nb);
	xmem_copy(seed + 32 + nb, random, rlen);
	nb += rlen;

	// compute P_sha256(secret,label+random)[0..dlen]
	//the sha256 output length is 32 bytes
	//A(0)
	sha256_hmac(S1, hs, seed + 32, nb, seed, 0);

	for (i = 0; i < dlen; i += 32)
	{
		sha256_hmac(S1, hs, seed, 32 + nb, hash, 0);
		//A(i)
		sha256_hmac(S1, hs, seed, 32, seed, 0);

		k = (i + 32 > dlen) ? dlen % 32 : 32;

		for (j = 0; j < k; j++)
			dstbuf[i + j] = hash[j];
	}
}

void ssl_extract(int md_alg, const byte_t *ikm, int ilen, const byte_t *salt, int slen, byte_t *prk, int* plen)
{
	const md_info_t* md;
	byte_t zeros[MD_MAX_SIZE] = { 0 };

	md = md_info_from_type(md_alg);
	if (!md)
	{
		*plen = 0;
		return;
	}

	if (ikm == NULL)
	{
		ikm = zeros;
		ilen = md->size;
	}

	if (hkdf_extract(md, salt, slen, ikm, ilen, prk) != 0)
	{
		*plen = 0;
		return;
	}

	*plen = md->size;
}

void ssl_expand(int md_alg, const byte_t* prk, int klen, const schar_t* label, const byte_t* hash, int hlen, byte_t* okm, int olen)
{
	const md_info_t* md;
	byte_t hkdf_label[512] = { 0 };
	int n, m, total = 0;
	static byte_t label_hdr[] = "tls13 ";

	md = md_info_from_type(md_alg);
	if (!md)
	{
		return;
	}

	n = strlen(label);
	m = strlen(label_hdr);
	/*
	* struct {
	*	uint16 length = Length;
	*	opaque label<7..255> = "tls13 " + Label;
	*	opaque context<0..255> = Context;
	* } HkdfLabel;
	*/
	PUT_SWORD_NET(hkdf_label, total, 0);
	total += 2;

	PUT_BYTE(hkdf_label, total, (m + n));
	total++;

	xmem_copy((void*)(hkdf_label + total), (void*)label_hdr, m);
	total += m;

	xmem_copy((void*)(hkdf_label + total), (void*)label, n);
	total += n;

	PUT_BYTE(hkdf_label, total, hlen);
	total++;

	xmem_copy((void*)(hkdf_label + total), (void*)hash, hlen);
	total += hlen;
	//reset length
	PUT_SWORD_NET(hkdf_label, 0, total);

	hkdf_expand(md, prk, klen, hkdf_label, total, okm, olen);
}