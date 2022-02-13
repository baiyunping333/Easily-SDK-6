/*
*  HKDF implementation -- RFC 5869
*
*  Copyright (C) 2016-2018, ARM Limited, All Rights Reserved
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

#include "hkdf.h"

#include "../xdkimp.h"


int hkdf(const md_info_t *md, const unsigned char *salt,
	dword_t salt_len, const unsigned char *ikm, dword_t ikm_len,
	const unsigned char *info, dword_t info_len,
	unsigned char *okm, dword_t okm_len)
{
	int ret;
	unsigned char prk[MD_MAX_SIZE];

	ret = hkdf_extract(md, salt, salt_len, ikm, ikm_len, prk);

	if (ret == 0)
	{
		ret = hkdf_expand(md, prk, md->size,
			info, info_len, okm, okm_len);
	}

	xmem_zero(prk, sizeof(prk));

	return(ret);
}

int hkdf_extract(const md_info_t *md,
	const unsigned char *salt, dword_t salt_len,
	const unsigned char *ikm, dword_t ikm_len,
	unsigned char *prk)
{
	unsigned char null_salt[MD_MAX_SIZE] = { '\0' };

	if (salt == NULL)
	{
		dword_t hash_len;

		if (salt_len != 0)
		{
			return ERR_HKDF_BAD_INPUT_DATA;
		}

		hash_len = md->size;

		if (hash_len == 0)
		{
			return ERR_HKDF_BAD_INPUT_DATA;
		}

		salt = null_salt;
		salt_len = hash_len;
	}

	return(md_hmac(md, salt, salt_len, ikm, ikm_len, prk));
}

int hkdf_expand(const md_info_t *md, const unsigned char *prk,
	dword_t prk_len, const unsigned char *info,
	dword_t info_len, unsigned char *okm, dword_t okm_len)
{
	dword_t hash_len;
	dword_t where = 0;
	dword_t n;
	dword_t t_len = 0;
	dword_t i;
	int ret = 0;
	void* ctx = NULL;
	unsigned char t[MD_MAX_SIZE] = { 0 };

	if (okm == NULL)
	{
		return(ERR_HKDF_BAD_INPUT_DATA);
	}

	hash_len = md->size;

	if (prk_len < hash_len || hash_len == 0)
	{
		return(ERR_HKDF_BAD_INPUT_DATA);
	}

	if (info == NULL)
	{
		info = (const unsigned char *) "";
		info_len = 0;
	}

	n = okm_len / hash_len;

	if ((okm_len % hash_len) != 0)
	{
		n++;
	}

	/*
	* Per RFC 5869 Section 2.3, okm_len must not exceed
	* 255 times the hash length
	*/
	if (n > 255)
	{
		return(ERR_HKDF_BAD_INPUT_DATA);
	}

	ctx = md_alloc(md);
	if (!ctx)
	{
		return(ERR_HKDF_BAD_INPUT_DATA);
	}

	/*
	* Compute T = T(1) | T(2) | T(3) | ... | T(N)
	* T(i) = HMAC-Hash(PRK, T(i - 1) | info | i) 
	* Where T(N) is defined in RFC 5869 Section 2.3
	*/
	for (i = 1; i <= n; i++)
	{
		dword_t num_to_copy;
		unsigned char c = i & 0xff;

		ret = md_hmac_starts(md, ctx, prk, prk_len);
		if (ret != 0)
		{
			goto exit;
		}

		ret = md_hmac_update(md, ctx, t, t_len);
		if (ret != 0)
		{
			goto exit;
		}

		ret = md_hmac_update(md, ctx, info, info_len);
		if (ret != 0)
		{
			goto exit;
		}

		/* The constant concatenated to the end of each T(n) is a single octet.
		* */
		ret = md_hmac_update(md, ctx, &c, 1);
		if (ret != 0)
		{
			goto exit;
		}

		ret = md_hmac_finish(md, ctx, t);
		if (ret != 0)
		{
			goto exit;
		}

		num_to_copy = (i != n) ? hash_len : (okm_len - where);
		memcpy(okm + where, t, num_to_copy);
		where += hash_len;
		t_len = hash_len;
	}

exit:
	md_free(md, ctx);

	return(ret);
}

#ifdef XDK_SUPPORT_TEST

typedef struct _test_octet{
	int size;
	byte_t octet[512];
}test_octet;

typedef struct _test_case{
	md_type_t md;
	test_octet ikm;
	test_octet salt;
	test_octet info;
	test_octet prk;
	test_octet okm;
}test_case;

static test_case hkdf_case[6] = {
	{ MD_SHA256, 
		{ 22, "0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b" },
		{ 13, "0x000102030405060708090a0b0c" },
		{ 10, "0xf0f1f2f3f4f5f6f7f8f9"},
		{ 32, "0x077709362c2e32df0ddc3f0dc47bba6390b6c73bb50f9c3122ec844ad7c2b3e5" },
		{ 42, "0x3cb25f25faacd57a90434f64d0362f2a2d2d0a90cf1a5a4c5db02d56ecc4c5bf34007208d5b887185865" },
	},
	{ MD_SHA256,
		{ 80, "0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f" },
		{ 80, "0x606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeaf" },
		{ 80, "0xb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff" },
		{ 32, "0x06a6b88c5853361a06104c9ceb35b45cef760014904671014a193f40c15fc244" },
		{ 82, "0xb11e398dc80327a1c8e7f78c596a49344f012eda2d4efad8a050cc4c19afa97c59045a99cac7827271cb41c65e590e09da3275600c2f09b8367793a9aca3db71cc30c58179ec3e87c14c01d5c1f3434f1d87" },
	},
	{ MD_SHA256,
		{ 22, "0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b" },
		{ 0, "" },
		{ 0, "" },
		{ 32, "0x19ef24a32c717b167f33a91d6f648bdf96596776afdb6377ac434c1c293ccb04" },
		{ 42, "0x8da4e775a563c18f715f802a063c5a31b8a11f5c5ee1879ec3454e5f3c738d2d9d201395faa4b61a96c8" },
	},
	{ MD_SHA1,
		{ 11, "0x0b0b0b0b0b0b0b0b0b0b0b" },
		{ 13, "0x000102030405060708090a0b0c" },
		{ 10, "0xf0f1f2f3f4f5f6f7f8f9" },
		{ 20, "0x9b6c18c432a7bf8f0e71c8eb88f4b30baa2ba243" },
		{ 42, "0x085a01ea1b10f36933068b56efa5ad81a4f14b822f5b091568a9cdd4f155fda2c22e422478d305f3f896" },
	},
	{ MD_SHA1,
		{ 80, "0x000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f" },
		{ 80, "0x606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeaf" },
		{ 80, "0xb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff" },
		{ 20, "0x8adae09a2a307059478d309b26c4115a224cfaf6" },
		{ 82, "0x0bd770a74d1160f7c9f12cd5912a06ebff6adcae899d92191fe4305673ba2ffe8fa3f1a4e5ad79f3f334b3b202b2173c486ea37ce3d397ed034c7f9dfeb15c5e927336d0441f4c4300e2cff0d0900b52d3b4" },
	},
	{ MD_SHA1,
		{ 22, "0x0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b" },
		{ 0, "" },
		{ 0, "" },
		{ 20, "0xda8c8a73c7fa77288ec6f5e7c297786aa0d32d01" },
		{ 42, "0x0ac1af7002b3d761d1e55298da9d0506b9ae52057220a306e07b6b87e8df21d0ea00033de03984d34918" },
	},
};

/*
void test_hkdf(int verbos)
{
	int ikm_len, salt_len, info_len, prk_len, okm_len;
	unsigned char ikm[128] = { 0 };
	unsigned char salt[128] = { 0 };
	unsigned char info[128] = { 0 };
	unsigned char prk[128] = { 0 };
	unsigned char prk_octet[257] = { 0 };
	unsigned char okm[128] = { 0 };
	unsigned char okm_octet[257] = { 0 };

	int i, ret;
	const md_info_t *md;

	for (i = 0; i < 6; i++)
	{
		md = md_info_from_type(hkdf_case[i].md);
		if (!md)
		{
			printf("test hkdf_case%d: unknown md type\n", i);
			continue;
		}

		ikm_len = a_parse_octet_string(hkdf_case[i].ikm.octet, (hkdf_case[i].ikm.size + 1) * 2, ikm, 128);
		salt_len = a_parse_octet_string(hkdf_case[i].salt.octet, (hkdf_case[i].salt.size + 1) * 2, salt, 128);
		info_len = a_parse_octet_string(hkdf_case[i].info.octet, (hkdf_case[i].info.size + 1) * 2, info, 128);

		xmem_zero((void*)prk, 128);

		ret = hkdf_extract(md, salt, salt_len, ikm, ikm_len, prk);
		if (ret != 0)
		{
			printf("test hkdf_case%d: hkdf_extract falied\n", i);
			continue;
		}

		prk_octet[0] = '0';
		prk_octet[1] = 'x';
		prk_len = a_format_octet_string(prk, hkdf_case[i].prk.size, 0, (prk_octet + 2), 256);

		ret = xmem_comp((void*)hkdf_case[i].prk.octet, (void*)prk_octet, prk_len + 2);
		if (ret != 0)
		{
			printf("test hkdf_case%d: prm not matched\n", i);
			continue;
		}

		ret = hkdf_expand(md, prk, md->size, info, info_len, okm, hkdf_case[i].okm.size);
		if (ret != 0)
		{
			printf("test hkdf_case%d: hkdf_expand falied\n", i);
			continue;
		}

		okm_octet[0] = '0';
		okm_octet[1] = 'x';
		okm_len = a_format_octet_string(okm, hkdf_case[i].okm.size, 0, (okm_octet + 2), 256);

		ret = xmem_comp((void*)hkdf_case[i].okm.octet, (void*)okm_octet, okm_len + 2);
		if (ret != 0)
		{
			printf("test hkdf_case%d: okm not matched\n", i);
			continue;
		}

		printf("test hkdf_case%d: succeed\n", i);
	}

}
*/

#endif