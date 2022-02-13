/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc der document

	@module	der.c | implement file

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

#include "der.h"

#include "xdkimp.h"
#include "xdkoem.h"
#include "xdkstd.h"

/*
* ASN.1 DER decoding routines
*/

dword_t der_read_tag(const byte_t *buf, byte_t *ptag, dword_t* plen)
{
	dword_t total = 0;

	XDK_ASSERT(buf != NULL);

	if (ptag)
	{
		*ptag = buf[total];
	}
	total++;

	if (!(buf[total] & 0x80))
	{
		if (plen)
		{
			*plen = buf[total];
		}
		total++;

		return total;
	}
	else
	{
		switch (buf[total] & 0x7F)
		{
		case 1:
			if (plen)
			{
				*plen = (int)GET_BYTE(buf, (total + 1));
			}
			total += 2;
			break;
		case 2:
			if (plen)
			{
				*plen = (int)GET_SWORD_NET(buf, (total + 1));
			}
			total += 3;
			break;
		case 3:
			if (plen)
			{
				*plen = (int)GET_THREEBYTE_NET(buf, (total + 1));
			}
			total += 4;
			break;
		case 4:
			if (plen)
			{
				*plen = (int)GET_DWORD_NET(buf, (total + 1));
			}
			total += 5;
			break;
		default:
			set_last_error(_T("der_read_tag"), _T("ERR_DER_OUT_OF_DATA"), -1);
			total = 0;
			break;
		}
	}

	return total;
}

dword_t der_write_tag(byte_t *buf, byte_t tag, dword_t len)
{
	dword_t total = 0;

	if (buf)
	{
		PUT_BYTE(buf, total, tag);
	}
	total++;

	if (len < 0x80)
	{
		if (buf)
		{
			PUT_BYTE(buf, total, (byte_t)len);
		}
		total++;
	}
	else if (len <= 0xFF)
	{
		if (buf)
		{
			PUT_BYTE(buf, total, 0x81);
			PUT_BYTE(buf, (total + 1), (byte_t)len);
		}
		total += 2;
	}
	else if (len <= 0xFFFF)
	{
		if (buf)
		{
			PUT_BYTE(buf, total, 0x82);
			PUT_SWORD_NET(buf, (total + 1), (sword_t)len);
		}
		total += 3;
	}
	else if (len <= 0xFFFFFF)
	{
		if (buf)
		{
			PUT_BYTE(buf, total, 0x83);
			PUT_THREEBYTE_NET(buf, (total + 1), (dword_t)len);
		}
		total += 4;
	}
	else if (len <= 0xFFFFFFFF)
	{
		if (buf)
		{
			PUT_BYTE(buf, total, 0x84);
			PUT_DWORD_NET(buf, (total + 1), (dword_t)len);
		}
		total += 5;
	}
	else
	{
		total = 0;
	}

	return total;
}

dword_t der_read_bool(const byte_t *buf, bool_t *pval)
{
	byte_t tag;
	dword_t len;
	int n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_bool"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != DER_BOOLEAN)
	{
		set_last_error(_T("der_read_bool"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	if (pval)
	{
		*pval = (buf[total]) ? 1 : 0;
	}
	total += len;

	return total;
}

dword_t der_write_bool(byte_t *buf, bool_t b)
{
	dword_t n, total = 0;
	byte_t c;

	n = der_write_tag((buf + total), DER_BOOLEAN, 1);
	if (!n)
	{
		set_last_error(_T("der_write_bool"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	if (buf)
	{
		c = (b) ? 255 : 0;
		PUT_BYTE(buf, total, c);
	}
	total++;

	return total;
}

dword_t der_read_integer(const byte_t *buf, int *pval)
{
	byte_t tag;
	dword_t len;
	int n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_integer"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != (DER_CONTEXT_SPECIFIC | DER_PRIMITIVE | DER_INTEGER) && tag != DER_INTEGER)
	{
		set_last_error(_T("der_read_integer"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	switch (len)
	{
	case 1:
		if (pval)
		{
			*pval = (int)GET_BYTE(buf, total);
		}
		break;
	case 2:
		if (pval)
		{
			*pval = (int)GET_SWORD_NET(buf, total);
		}
		break;
	case 3:
		if (pval)
		{
			*pval = (int)GET_THREEBYTE_NET(buf, total);
		}
		break;
	case 4:
		if (pval)
		{
			*pval = (int)GET_DWORD_NET(buf, total);
		}
		break;
	default:
		if (pval)
		{
			*pval = 0;
		}
		break;
	}
	
	total += len;

	return total;
}

dword_t der_write_integer(byte_t *buf, int val)
{
	dword_t len, n, total = 0;
	byte_t c[4] = { 0 };

	PUT_DWORD_NET(c, 0, (dword_t)val);

	if (val < 0)
	{
		len = 4;
	}
	else
	{
		len = 0;
		while (!c[len] && len < 4)
			len++;

		if (c[len] & 0x80 && len < 4)
			len--;

		len = 4 - len;
	}

	if (!len)
		len++;

	n = der_write_tag((buf + total), DER_INTEGER, len);
	if (!n)
	{
		set_last_error(_T("der_write_integer"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)(c + 4 - len), len);
	}
	total += len;

	return total;
}

dword_t der_read_bit_string(const byte_t *buf, byte_t** pstr, dword_t* plen, dword_t* pbit)
{
	byte_t tag;
	dword_t len;
	dword_t n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_bit_string"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != DER_BIT_STRING)
	{
		set_last_error(_T("der_read_bit_string"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	if (plen) *plen = (len - 1);
	if (pbit) *pbit = buf[total];
	if (pstr) *pstr = buf + total + 1;

	total += len;

	return total;
}

dword_t der_write_bit_string(byte_t *buf, const byte_t* str, dword_t bits)
{
	dword_t len;
	dword_t n, total = 0;
	byte_t unu, c;

	len = (bits + 7) / 8;
	n = der_write_tag((buf + total), DER_BIT_STRING, (len + 1));
	if (!n)
	{
		set_last_error(_T("der_write_bit_string"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	if (buf)
	{
		unu = (byte_t)(len * 8 - bits);
		PUT_BYTE(buf, total, unu);

		c = ~((0x01 << unu) - 1);
	}
	total++;

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)str, len);
		buf[total - 1] &= c;
	}
	total += len;

	return total;
}

dword_t der_read_octet_string(const byte_t *buf, byte_t **poct, dword_t* plen)
{
	byte_t tag;
	dword_t len;
	dword_t n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_octet_string"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != DER_OCTET_STRING)
	{
		set_last_error(_T("der_read_octet_string"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	if (plen) *plen = len;
	if (poct) *poct = buf + total;
	
	total += len;

	return total;
}

dword_t der_write_octet_string(byte_t *buf, const byte_t* oct, dword_t len)
{
	dword_t n, total = 0;

	n = der_write_tag((buf + total), DER_OCTET_STRING, len);
	if (!n)
	{
		set_last_error(_T("der_write_octet_string"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)oct, len);
	}
	total += len;

	return total;
}

dword_t der_read_null(const byte_t *buf)
{
	byte_t tag;
	dword_t len;
	dword_t n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_null"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != DER_NULL)
	{
		set_last_error(_T("der_read_null"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	return total;
}

dword_t der_write_null(byte_t *buf)
{
	dword_t n, total = 0;

	n = der_write_tag((buf + total), DER_NULL, 0);
	if (!n)
	{
		set_last_error(_T("der_write_null"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	return total;
}

dword_t der_read_oid(const byte_t *buf, byte_t **poid, dword_t* plen)
{
	byte_t tag;
	dword_t len;
	dword_t n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_oid"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != DER_OID)
	{
		set_last_error(_T("der_read_oid"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	if (plen) *plen = len;
	if (poid) *poid = buf + total;
	
	total += len;

	return total;
}

dword_t der_write_oid(byte_t *buf, const byte_t* oid, dword_t len)
{
	dword_t n, total = 0;

	n = der_write_tag((buf + total), DER_OID, len);
	if (!n)
	{
		set_last_error(_T("der_write_oid"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)oid, len);
	}
	total += len;

	return total;
}

dword_t der_read_utf8_string(const byte_t *buf, byte_t **putf, dword_t* plen)
{
	byte_t tag;
	dword_t len;
	dword_t n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_utf8_string"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != DER_UTF8_STRING)
	{
		set_last_error(_T("der_read_utf8_string"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	if (plen) *plen = len;
	if (putf) *putf = buf + total;

	total += len;

	return total;
}

dword_t der_write_utf8_string(byte_t *buf, const byte_t* utf, dword_t len)
{
	dword_t n, total = 0;

	n = der_write_tag((buf + total), DER_UTF8_STRING, len);
	if (!n)
	{
		set_last_error(_T("der_write_utf8_string"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)utf, len);
	}
	total += len;

	return total;
}

dword_t der_read_printable_string(const byte_t *buf, char **pstr, dword_t* plen)
{
	byte_t tag;
	dword_t len;
	dword_t n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_printable_string"), _T("der_read_tag"), -1);
		return C_ERR;
	}
	if (tag != DER_PRINTABLE_STRING)
	{
		set_last_error(_T("der_read_printable_string"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	if (plen) *plen = len;
	if (pstr) *pstr = buf + total;
	
	total += len;

	return total;
}

dword_t der_write_printable_string(byte_t *buf, const char* str, dword_t len)
{
	dword_t n, total = 0;

	n = der_write_tag((buf + total), DER_PRINTABLE_STRING, len);
	if (!n)
	{
		set_last_error(_T("der_write_printable_string"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)str, len);
	}
	total += len;

	return total;
}

dword_t der_read_ia5_string(const byte_t *buf, char **pstr, dword_t* plen)
{
	byte_t tag;
	dword_t len;
	dword_t n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_ia5_string"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != DER_IA5_STRING)
	{
		set_last_error(_T("der_read_ia5_string"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	if (plen) *plen = len;
	if (pstr) *pstr = buf + total;
	
	total += len;

	return total;
}

dword_t der_write_ia5_string(byte_t *buf, const char* str, dword_t len)
{
	dword_t n, total = 0;

	n = der_write_tag((buf + total), DER_IA5_STRING, len);
	if (!n)
	{
		set_last_error(_T("der_write_ia5_string"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)str, len);
	}
	total += len;

	return total;
}

dword_t der_read_sequence_of(const byte_t *buf, dword_t* plen)
{
	byte_t tag;
	dword_t len;
	dword_t n, total = 0;

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_sequence_of"), _T("der_read_tag"), -1);
		return total;
	}
	if (tag != (DER_CONSTRUCTED | DER_SEQUENCE))
	{
		set_last_error(_T("der_read_sequence_of"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	return total;
}

dword_t der_write_sequence_of(byte_t *buf, dword_t len)
{
	dword_t n, total = 0;

	n = der_write_tag((buf + total), (DER_CONSTRUCTED | DER_SEQUENCE), len);
	if (!n)
	{
		set_last_error(_T("der_write_sequence_of"), _T("der_write_tag"), -1);
		return total;
	}
	total += n;

	return total;
}

dword_t der_read_time(const byte_t *buf, xdate_t *pdt)
{
	dword_t len, total = 0;
	dword_t n;
	byte_t tag;
	int year_len, mon_len, day_len, hour_len, min_len, sec_len;

	if (pdt)
	{
		xmem_zero((void*)pdt, sizeof(xdate_t));
	}

	n = der_read_tag((buf + total), &tag, &len);
	if (!n)
	{
		set_last_error(_T("der_read_time"), _T("der_read_tag"), -1);
		return total;
	}

	if (tag == DER_UTC_TIME)
	{
		year_len = 2;
		mon_len = 2;
		day_len = 2;
		hour_len = 2;
		min_len = 2;
		sec_len = 2;
	}
	else if (tag == DER_GENERALIZED_TIME)
	{
		year_len = 4;
		mon_len = 2;
		day_len = 2;
		hour_len = 2;
		min_len = 2;
		sec_len = 2;
	}
	else
	{
		set_last_error(_T("der_read_time"), _T("ERR_DER_TAG_MISMATCH"), -1);
		return total;
	}
	total += n;

	while (len && year_len)
	{
		if (pdt)
		{
			pdt->year *= 10;
			pdt->year += (*(buf + total) - '0');
		}
		total++;
		year_len--;
		len--;
	}

	if (pdt->year < 50)
		pdt->year += 100;
	pdt->year += 1900;

	while (len && mon_len)
	{
		if (pdt)
		{
			pdt->mon *= 10;
			pdt->mon += (*(buf + total) - '0');
		}
		total++;
		mon_len--;
		len--;
	}

	while (len && day_len)
	{
		if (pdt)
		{
			pdt->day *= 10;
			pdt->day += (*(buf + total) - '0');
		}
		total++;
		day_len--;
		len--;
	}

	while (len && hour_len)
	{
		if (pdt)
		{
			pdt->hour *= 10;
			pdt->hour += (*(buf + total) - '0');
		}
		total++;
		hour_len--;
		len--;
	}

	while (len && min_len)
	{
		if (pdt)
		{
			pdt->min *= 10;
			pdt->min += (*(buf + total) - '0');
		}
		total++;
		min_len--;
		len--;
	}

	while (len && sec_len)
	{
		if (pdt)
		{
			pdt->sec *= 10;
			pdt->sec += (*(buf + total) - '0');
		}
		total++;
		sec_len--;
		len--;
	}

	while (len)
	{
		total++;
		len--;
	}

	return total;
}



#if defined(XDK_SUPPORT_TEST)

void test_der()
{
	int n;
	int total = 0;
	byte_t tmp[1024];

	n = MAX_BYTE - 1;
	total += der_write_integer(tmp + total, n);
	printf("write: %d\n", n);

	n = MAX_SHORT - 1;
	total += der_write_integer(tmp + total, n);
	printf("write: %d\n", n);

	n = MAX_LONG - 1;
	total += der_write_integer(tmp + total, n);
	printf("write: %d\n", n);

	total = 0;

	total += der_read_integer(tmp + total, &n);
	if (n == MAX_BYTE - 1)
		printf("read: %d\n", n);
	else
		printf("read error: %d\n", n);

	total += der_read_integer(tmp + total, &n);
	if (n == MAX_SHORT - 1)
		printf("read: %d\n", n);
	else
		printf("read error: %d\n", n);

	total += der_read_integer(tmp + total, &n);
	if (n == MAX_LONG - 1)
		printf("read: %d\n", n);
	else
		printf("read error: %d\n", n);

	total = 0;

	total += der_write_null(tmp);
	printf("write: NULL\n", n);

	const char str[] = "OCTET TOKEN";
	total += der_write_octet_string((tmp + total), str, a_xslen(str));
	printf("write: %s\n", str);

	total = 0;

	total += der_read_null((tmp + total));
	if (total)
		printf("rad: NULL\n", n);
	else
		printf("read error: NULL\n", n);

	byte_t* buf = { 0 };
	total += der_read_octet_string((tmp + total), &buf, &n);
	printf("rad: %s\n", buf);
}

#endif