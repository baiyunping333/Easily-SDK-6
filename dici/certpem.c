/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc cert document

	@module	certpem.c | implement file

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

#include "certpem.h"

/*
* Read a 16-byte hex string and convert it to binary
*/
static int pem_get_iv(const byte_t *s, byte_t *iv,
	dword_t iv_len)
{
	dword_t i, j, k;

	xmem_zero(iv, iv_len);

	for (i = 0; i < iv_len * 2; i++, s++)
	{
		if (*s >= '0' && *s <= '9') j = *s - '0'; else
			if (*s >= 'A' && *s <= 'F') j = *s - '7'; else
				if (*s >= 'a' && *s <= 'f') j = *s - 'W'; else
				{
					set_last_error(_T("pem_get_iv"), _T("ERR_PEM_INVALID_ENC_IV"), -1);
					return C_ERR;
				}

		k = ((i & 1) != 0) ? j : j << 4;

		iv[i >> 1] = (byte_t)(iv[i >> 1] | k);
	}

	return(0);
}

static int pem_pbkdf1(byte_t *key, dword_t keylen,
	byte_t *iv,
	const byte_t *pwd, dword_t pwdlen)
{
	md5_context md5_ctx;
	byte_t md5sum[16];
	dword_t use_len;
	int ret;

	md5_init(&md5_ctx);

	/*
	* key[ 0..15] = MD5(pwd || IV)
	*/
	if ((ret = md5_starts(&md5_ctx)) != 0)
		goto exit;
	if ((ret = md5_update(&md5_ctx, pwd, pwdlen)) != 0)
		goto exit;
	if ((ret = md5_update(&md5_ctx, iv, 8)) != 0)
		goto exit;
	if ((ret = md5_finish(&md5_ctx, md5sum)) != 0)
		goto exit;

	if (keylen <= 16)
	{
		xmem_copy(key, md5sum, keylen);
		goto exit;
	}

	xmem_copy(key, md5sum, 16);

	/*
	* key[16..23] = MD5(key[ 0..15] || pwd || IV])
	*/
	if ((ret = md5_starts(&md5_ctx)) != 0)
		goto exit;
	if ((ret = md5_update(&md5_ctx, md5sum, 16)) != 0)
		goto exit;
	if ((ret = md5_update(&md5_ctx, pwd, pwdlen)) != 0)
		goto exit;
	if ((ret = md5_update(&md5_ctx, iv, 8)) != 0)
		goto exit;
	if ((ret = md5_finish(&md5_ctx, md5sum)) != 0)
		goto exit;

	use_len = 16;
	if (keylen < 32)
		use_len = keylen - 16;

	xmem_copy(key + 16, md5sum, use_len);

exit:
	md5_free(&md5_ctx);
	xmem_zero(md5sum, 16);

	return(ret);
}

/*
* Decrypt with DES-CBC, using PBKDF1 for key derivation
*/
static int pem_des_decrypt(byte_t des_iv[8],
	byte_t *buf, dword_t buflen,
	const byte_t *pwd, dword_t pwdlen)
{
	des_context des_ctx;
	byte_t des_key[8];
	int ret;

	des_init(&des_ctx);

	if ((ret = pem_pbkdf1(des_key, 8, des_iv, pwd, pwdlen)) != 0)
		goto exit;

	if ((ret = des_setkey_dec(&des_ctx, des_key)) != 0)
		goto exit;
	ret = des_crypt_cbc(&des_ctx, DES_DECRYPT, buflen,
		des_iv, buf, buf);

exit:
	des_free(&des_ctx);
	xmem_zero(des_key, 8);

	return(ret);
}

/*
* Decrypt with 3DES-CBC, using PBKDF1 for key derivation
*/
static int pem_des3_decrypt(byte_t des3_iv[8],
	byte_t *buf, dword_t buflen,
	const byte_t *pwd, dword_t pwdlen)
{
	des3_context des3_ctx;
	byte_t des3_key[24];
	int ret;

	des3_init(&des3_ctx);

	if ((ret = pem_pbkdf1(des3_key, 24, des3_iv, pwd, pwdlen)) != 0)
		goto exit;

	if ((ret = des3_set3key_dec(&des3_ctx, des3_key)) != 0)
		goto exit;

	ret = des3_crypt_cbc(&des3_ctx, DES_DECRYPT, buflen,
		des3_iv, buf, buf);

exit:
	des3_free(&des3_ctx);
	xmem_zero(des3_key, 24);

	return(ret);
}

/*
* Decrypt with AES-XXX-CBC, using PBKDF1 for key derivation
*/
static int pem_aes_decrypt(byte_t aes_iv[16], unsigned int keylen,
	byte_t *buf, dword_t buflen,
	const byte_t *pwd, dword_t pwdlen)
{
	aes_context aes_ctx;
	byte_t aes_key[32];
	int ret;

	aes_init(&aes_ctx);

	if ((ret = pem_pbkdf1(aes_key, keylen, aes_iv, pwd, pwdlen)) != 0)
		goto exit;

	if ((ret = aes_setkey_dec(&aes_ctx, aes_key, keylen * 8)) != 0)
		goto exit;

	ret = aes_crypt_cbc(&aes_ctx, AES_DECRYPT, buflen,
		aes_iv, buf, buf);

exit:
	aes_free(&aes_ctx);
	xmem_zero(aes_key, keylen);

	return(ret);
}

dword_t pem_decode(const byte_t *pem, dword_t len, byte_t* der, dword_t max, const char *header, const char *footer, const byte_t *pwd, dword_t pwdlen)
{
	int ret, enc;
	const byte_t *s1, *s2, *end;
	byte_t pem_iv[16];
	cipher_type_t enc_alg = CIPHER_NONE;

	s1 = (byte_t *)a_xsstr((const char *)pem, header);

	if (s1 == NULL)
	{
		set_last_error(_T("pem_decode"), _T("ERR_PEM_NO_HEADER_FOOTER_PRESENT"), -1);
		return 0;
	}

	s2 = (byte_t *)a_xsstr((const char *)pem, footer);

	if (s2 == NULL || s2 <= s1)
	{
		set_last_error(_T("pem_decode"), _T("ERR_PEM_NO_HEADER_FOOTER_PRESENT"), -1);
		return 0;
	}

	s1 += a_xslen(header);
	if (*s1 == ' ') s1++;
	if (*s1 == '\r') s1++;
	if (*s1 == '\n') s1++;
	else 
	{
		set_last_error(_T("pem_decode"), _T("ERR_PEM_NO_HEADER_FOOTER_PRESENT"), -1);
		return 0;
	}

	end = s2;
	end += a_xslen(footer);
	if (*end == ' ') end++;
	if (*end == '\r') end++;
	if (*end == '\n') end++;

	enc = 0;

	if (s2 - s1 >= 22 && xmem_comp(s1, "Proc-Type: 4,ENCRYPTED", 22) == 0)
	{
		enc++;

		s1 += 22;
		if (*s1 == '\r') s1++;
		if (*s1 == '\n') s1++;
		else 
		{
			set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_DATA"), -1);
			return 0;
		}

		if (s2 - s1 >= 23 && xmem_comp(s1, "DEK-Info: DES-EDE3-CBC,", 23) == 0)
		{
			enc_alg = CIPHER_DES_EDE3_CBC;

			s1 += 23;
			if (s2 - s1 < 16 || pem_get_iv(s1, pem_iv, 8) != 0)
			{
				set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_ENC_IV"), -1);
				return 0;
			}

			s1 += 16;
		}
		else if (s2 - s1 >= 18 && xmem_comp(s1, "DEK-Info: DES-CBC,", 18) == 0)
		{
			enc_alg = CIPHER_DES_CBC;

			s1 += 18;
			if (s2 - s1 < 16 || pem_get_iv(s1, pem_iv, 8) != 0)
			{
				set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_ENC_IV"), -1);
				return 0;
			}

			s1 += 16;
		}

		if (s2 - s1 >= 14 && xmem_comp(s1, "DEK-Info: AES-", 14) == 0)
		{
			if (s2 - s1 < 22)
			{
				set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_UNKNOWN_ENC_ALG"), -1);
				return 0;
			}
			else if (xmem_comp(s1, "DEK-Info: AES-128-CBC,", 22) == 0)
				enc_alg = CIPHER_AES_128_CBC;
			else if (xmem_comp(s1, "DEK-Info: AES-192-CBC,", 22) == 0)
				enc_alg = CIPHER_AES_192_CBC;
			else if (xmem_comp(s1, "DEK-Info: AES-256-CBC,", 22) == 0)
				enc_alg = CIPHER_AES_256_CBC;
			else
			{
				set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_UNKNOWN_ENC_ALG"), -1);
				return 0;
			}

			s1 += 22;
			if (s2 - s1 < 32 || pem_get_iv(s1, pem_iv, 16) != 0)
			{
				set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_ENC_IV"), -1);
				return 0;
			}

			s1 += 32;
		}

		if (enc_alg == CIPHER_NONE)
		{
			set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_UNKNOWN_ENC_ALG"), -1);
			return 0;
		}

		if (*s1 == '\r') s1++;
		if (*s1 == '\n') s1++;
		else 
		{
			set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_DATA"), -1);
			return 0;
		}
	}

	if (s1 >= s2)
	{
		set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_DATA"), -1);
		return 0;
	}

	if((ret = base64_decode(NULL, 0, &len, s1, s2 - s1)) != 0)
	{
		set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_DATA"), -1);
		return 0;
	}

	if (len > max)
	{
		set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_DATA"), -1);
		return 0;
	}

	if (!der)
		return len;

	if ((ret = base64_decode(der, len, &len, s1, s2 - s1)) != 0)
	{
		set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_INVALID_DATA"), -1);
		return 0;
	}

	if (enc != 0)
	{
		if (pwd == NULL)
		{
			set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_PASSWORD_REQUIRED"), -1);
			return 0;
		}

		ret = 0;

		if (enc_alg == CIPHER_DES_EDE3_CBC)
			ret = pem_des3_decrypt(pem_iv, der, len, pwd, pwdlen);
		else if (enc_alg == CIPHER_DES_CBC)
			ret = pem_des_decrypt(pem_iv, der, len, pwd, pwdlen);

		if (enc_alg == CIPHER_AES_128_CBC)
			ret = pem_aes_decrypt(pem_iv, 16, der, len, pwd, pwdlen);
		else if (enc_alg == CIPHER_AES_192_CBC)
			ret = pem_aes_decrypt(pem_iv, 24, der, len, pwd, pwdlen);
		else if (enc_alg == CIPHER_AES_256_CBC)
			ret = pem_aes_decrypt(pem_iv, 32, der, len, pwd, pwdlen);

		if (ret != 0)
		{
			return 0;
		}

		/*
		* The result will be ASN.1 starting with a SEQUENCE tag, with 1 to 3
		* length bytes (allow 4 to be sure) in all known use cases.
		*
		* Use that as a heuristic to try to detect password mismatches.
		*/
		if (len <= 2 || der[0] != 0x30 || der[1] > 0x83)
		{
			set_last_error(_T("pem_read_buffer"), _T("ERR_PEM_PASSWORD_MISMATCH"), -1);
			return 0;
		}
	}

	return len;
}

dword_t pem_encode(byte_t *pem, dword_t max, const byte_t *der, dword_t len, const char *header, const char *footer)
{
	int ret;
	byte_t *buf = NULL;
	byte_t *c, *p = pem;
	dword_t use_len, add_len = 0;

	base64_encode(NULL, 0, &use_len, der, len);
	add_len = a_xslen(header) + a_xslen(footer) + (use_len / 64) + 1;

	if (use_len + add_len > max)
	{
		set_last_error(_T("pem_write_buffer"), _T("ERR_PEM_ALLOC_FAILED"), -1);
		return 0;
	}

	if (!pem)
	{
		return (use_len + add_len);
	}

	buf = (byte_t*)xmem_alloc(use_len);

	if ((ret = base64_encode(buf, use_len, &use_len, der, len)) != 0)
	{
		xmem_free(buf);
		return (0);
	}

	xmem_copy(p, header, a_xslen(header));
	p += a_xslen(header);
	c = buf;

	while (use_len)
	{
		len = (use_len > 64) ? 64 : use_len;
		xmem_copy(p, c, len);
		use_len -= len;
		p += len;
		c += len;
		*p++ = '\n';
	}

	xmem_copy(p, footer, a_xslen(footer));
	p += strlen(footer);

	*p++ = '\0';
	len = p - pem;

	xmem_free(buf);

	return (len);
}
