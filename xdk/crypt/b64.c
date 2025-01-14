﻿/*
*  RFC 1521 base64 encoding/decoding
*
*  Copyright (C) 2006-2007  Christophe Devine
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License aint
*  with this program; if not, write to the Free Software Foundation, Inc.,
*  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "b64.h"

#include "../xdkimp.h"

static const byte_t base64_enc_map[64] =
{
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
	'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',
	'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',
	'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
	'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', '+', '/'
};

static const byte_t base64_dec_map[128] =
{
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 62, 127, 127, 127, 63, 52, 53,
	54, 55, 56, 57, 58, 59, 60, 61, 127, 127,
	127, 64, 127, 127, 127, 0, 1, 2, 3, 4,
	5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
	25, 127, 127, 127, 127, 127, 127, 26, 27, 28,
	29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
	39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
	49, 50, 51, 127, 127, 127, 127, 127
};

#define BASE64_SIZE_T_MAX   ( (dword_t) -1 ) /* SIZE_T_MAX is not standard */

/*
* Encode a buffer into base64 format
*/
int base64_encode(byte_t *dst, dword_t dlen, dword_t *olen,
	const byte_t *src, dword_t slen)
{
	dword_t i, n;
	int C1, C2, C3;
	byte_t *p;

	if (slen == 0)
	{
		*olen = 0;
		return(0);
	}

	n = slen / 3 + (slen % 3 != 0);

	if (n > (BASE64_SIZE_T_MAX - 1) / 4)
	{
		*olen = BASE64_SIZE_T_MAX;
		set_last_error(_T("base64_encode"), _T("ERR_BASE64_BUFFER_TOO_SMALL"), -1);
		return C_ERR;
	}

	n *= 4;

	if ((dlen < n + 1) || (NULL == dst))
	{
		*olen = n + 1;
		set_last_error(_T("base64_encode"), _T("ERR_BASE64_BUFFER_TOO_SMALL"), -1);
		return C_ERR;
	}

	n = (slen / 3) * 3;

	for (i = 0, p = dst; i < n; i += 3)
	{
		C1 = *src++;
		C2 = *src++;
		C3 = *src++;

		*p++ = base64_enc_map[(C1 >> 2) & 0x3F];
		*p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];
		*p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
		*p++ = base64_enc_map[C3 & 0x3F];
	}

	if (i < slen)
	{
		C1 = *src++;
		C2 = ((i + 1) < slen) ? *src++ : 0;

		*p++ = base64_enc_map[(C1 >> 2) & 0x3F];
		*p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];

		if ((i + 1) < slen)
			*p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
		else *p++ = '=';

		*p++ = '=';
	}

	*olen = p - dst;
	*p = 0;

	return(0);
}

/*
* Decode a base64-formatted buffer
*/
int base64_decode(byte_t *dst, dword_t dlen, dword_t *olen,
	const byte_t *src, dword_t slen)
{
	dword_t i, n;
	dword_t j, x;
	byte_t *p;

	/* First pass: check for validity and get output length */
	for (i = n = j = 0; i < slen; i++)
	{
		/* Skip spaces before checking for EOL */
		x = 0;
		while (i < slen && src[i] == ' ')
		{
			++i;
			++x;
		}

		/* Spaces at end of buffer are OK */
		if (i == slen)
			break;

		if ((slen - i) >= 2 &&
			src[i] == '\r' && src[i + 1] == '\n')
			continue;

		if (src[i] == '\n')
			continue;

		/* Space inside a line is an error */
		if (x != 0)
		{
			set_last_error(_T("base64_decode"), _T("ERR_BASE64_INVALID_CHARACTER"), -1);
			return C_ERR;
		}

		if (src[i] == '=' && ++j > 2)
		{
			set_last_error(_T("base64_decode"), _T("ERR_BASE64_INVALID_CHARACTER"), -1);
			return C_ERR;
		}

		if (src[i] > 127 || base64_dec_map[src[i]] == 127)
		{
			set_last_error(_T("base64_decode"), _T("ERR_BASE64_INVALID_CHARACTER"), -1);
			return C_ERR;
		}

		if (base64_dec_map[src[i]] < 64 && j != 0)
		{
			set_last_error(_T("base64_decode"), _T("ERR_BASE64_INVALID_CHARACTER"), -1);
			return C_ERR;
		}

		n++;
	}

	if (n == 0)
	{
		*olen = 0;
		return(0);
	}

	/* The following expression is to calculate the following formula without
	* risk of integer overflow in n:
	*     n = ( ( n * 6 ) + 7 ) >> 3;
	*/
	n = (6 * (n >> 3)) + ((6 * (n & 0x7) + 7) >> 3);
	n -= j;

	if (dst == NULL)
	{
		*olen = n;
		return (0);
	} else if (dlen < n)
	{
		*olen = n;
		set_last_error(_T("base64_decode"), _T("ERR_BASE64_BUFFER_TOO_SMALL"), -1);
		return C_ERR;
	}

	for (j = 3, n = x = 0, p = dst; i > 0; i--, src++)
	{
		if (*src == '\r' || *src == '\n' || *src == ' ')
			continue;

		j -= (base64_dec_map[*src] == 64);
		x = (x << 6) | (base64_dec_map[*src] & 0x3F);

		if (++n == 4)
		{
			n = 0;
			if (j > 0) *p++ = (byte_t)(x >> 16);
			if (j > 1) *p++ = (byte_t)(x >> 8);
			if (j > 2) *p++ = (byte_t)(x);
		}
	}

	*olen = p - dst;

	return(0);
}

#if defined(XDK_SUPPORT_TEST)

static const byte_t base64_test_dec[64] =
{
	0x24, 0x48, 0x6E, 0x56, 0x87, 0x62, 0x5A, 0xBD,
	0xBF, 0x17, 0xD9, 0xA2, 0xC4, 0x17, 0x1A, 0x01,
	0x94, 0xED, 0x8F, 0x1E, 0x11, 0xB3, 0xD7, 0x09,
	0x0C, 0xB6, 0xE9, 0x10, 0x6F, 0x22, 0xEE, 0x13,
	0xCA, 0xB3, 0x07, 0x05, 0x76, 0xC9, 0xFA, 0x31,
	0x6C, 0x08, 0x34, 0xFF, 0x8D, 0xC2, 0x6C, 0x38,
	0x00, 0x43, 0xE9, 0x54, 0x97, 0xAF, 0x50, 0x4B,
	0xD1, 0x41, 0xBA, 0x95, 0x31, 0x5A, 0x0B, 0x97
};

static const byte_t base64_test_enc[] =
"JEhuVodiWr2/F9mixBcaAZTtjx4Rs9cJDLbpEG8i7hPK"
"swcFdsn6MWwINP+Nwmw4AEPpVJevUEvRQbqVMVoLlw==";

/*
* Checkup routine
*/
int base64_self_test(int verbose)
{
	dword_t len;
	const byte_t *src;
	byte_t buffer[128];

	if (verbose != 0)
		printf("  Base64 encoding test: ");

	src = base64_test_dec;

	if (base64_encode(buffer, sizeof(buffer), &len, src, 64) != 0 ||
		xmem_comp(base64_test_enc, buffer, 88) != 0)
	{
		if (verbose != 0)
			printf("failed\n");

		return(1);
	}

	if (verbose != 0)
		printf("passed\n  Base64 decoding test: ");

	src = base64_test_enc;

	if (base64_decode(buffer, sizeof(buffer), &len, src, 88) != 0 ||
		xmem_comp(base64_test_dec, buffer, 64) != 0)
	{
		if (verbose != 0)
			printf("failed\n");

		return(1);
	}

	if (verbose != 0)
		printf("passed\n\n");

	return(0);
}

#endif /* SELF_TEST */