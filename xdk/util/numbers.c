/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc utility document

	@module	numbers.c | implement file

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

#include "numbers.h"

#include "../xdkimp.h"
#include "../xdkstd.h"

bool_t is_zero_size(const tchar_t* fsize)
{
	dword_t hdw, ldw;

	parse_long(&hdw, &ldw, fsize);

	return (!hdw && !ldw) ? 1 : 0;
}

bool_t is_huge_size(const tchar_t* fsize)
{
	dword_t hdw, ldw;

	parse_long(&hdw, &ldw, fsize);

	if (hdw)
		return 1;
	else if (ldw > MAX_LONG)
		return 1;

	return 0;
}

int format_long(unsigned int hl, unsigned int ll, tchar_t* buf)
{
	unsigned long long li;
	int len = 0;

	li = (((unsigned long long)hl) << 32) | (unsigned long long)ll;
	do
	{
		if (buf)
		{
			buf[len] = (int)(li % 10) + _T('0');
		}
		li /= 10;
		len++;
	} while (li);

	if (buf)
	{
		buf[len] = _T('\0');

		xsnrev(buf, len);
	}

	return len;
}

void parse_long(unsigned int* phl, unsigned int* pll, const tchar_t* str)
{
	unsigned long long li = 0;
	int len = 0;

	if (phl)
		*phl = 0;
	if (pll)
		*pll = 0;

	if (is_null(str))
		return;

	while (str[len] != _T('\0'))
	{
		li *= 10;
		li += str[len++] - _T('0');
	}

	if (phl)
		*phl = (unsigned int)((li & 0xFFFFFFFF00000000) >> 32);
	if (pll)
		*pll = (unsigned int)(li & 0x00000000FFFFFFFF);
}

unsigned int parse_hexnum(const tchar_t* token, int len)
{
	unsigned int k = 0;
	int c = 0;
	int pos = 0;

	if (len < 0)
		len = xslen(token);

	if (is_null(token) || !len)
		return 0;

	if (token[0] == _T('0') && (token[1] == _T('x') || token[1] == _T('X')))
		pos += 2;

	while (pos < len)
	{
		k *= 16;

		if (token[pos] >= _T('a') && token[pos] <= _T('z'))
			c = (token[pos] - _T('a')) + 10;
		else if (token[pos] >= _T('A') && token[pos] <= _T('Z'))
			c = (token[pos] - _T('A')) + 10;
		else if (token[pos] >= _T('0') && token[pos] <= _T('9'))
			c = (token[pos] - _T('0'));
		else if (token[pos] == _T('\0'))
			break;
		else
			return 0;

		k += c;

		pos++;
	}

	return k;
}

int format_hexnum(unsigned int n, tchar_t* buf, int max)
{
	int c = 0;
	int pos = 0;

	do
	{
		c = n % 16;

		if (buf)
		{
			if (c >= 10)
				buf[pos] = _T('A') + (byte_t)(c - 10);
			else
				buf[pos] = _T('0') + (byte_t)c;
		}

		n /= 16;
		pos++;

		if (pos % 2)
			continue;
	} while (pos < max);

	if (buf)
	{
		buf[pos] = _T('\0');
		xsnrev(buf, pos);
	}

	return pos;
}

int fill_integer(int ln, tchar_t* buf, int max)
{
	int len;
	bool_t b = 0;

	if (ln < 0)
	{
		b = 1;
		ln = 0 - ln;
		max--;
		buf[0] = _T('-');
		buf++;
	}

	len = ltoxs(ln, NULL, max);

	if (len < max)
	{
		len = max - len;
		ltoxs(ln, buf + len, max - len);
		while (len--)
		{
			(*buf) = _T('0');
			buf++;
		}

		if (b)
			max++;

		return max;
	}
	else
	{
		return ltoxs(ln, buf, max);
	}
}

int parse_intset(const tchar_t* str, int len, int* sa, int max)
{
	const tchar_t* key;
	int klen;
	int n = 0, total = 0;
	int k1, k2;
	bool_t b = 0;

	if (len < 0)
		len = xslen(str);

	if (*str == _T('['))
	{
		str++;
		total++;
	}

	while (*str != _T('\0') && (*str < _T('0') || *str > _T('9')) && total < len)
	{
		str++;
		total++;
	}

	k1 = k2 = 0;
	while (*str != _T(']') && *str != _T('\0') && total < len)
	{
		key = str;
		klen = 0;
		while (*str >= _T('0') && *str <= _T('9'))
		{
			str++;
			total++;
			klen++;
		}

		if (klen)
		{
			k2 = xsntol(key, klen);
			if (!b)
				k1 = k2;

			for (; k1 <= k2; k1++)
			{
				if (n + 1 > max)
					return n;

				if (sa)
				{
					sa[n] = k1;
				}
				n++;
			}
		}

		if (*str == _T('-'))
			b = 1;
		else
			b = 0;

		while (*str == _T('-') || *str == _T(' ') || *str == _T(','))
		{
			str++;
			total++;
		}
	}

	return n;
}

int format_integer_ex(int ln, const tchar_t* fmt, tchar_t* buf, int max)
{
	int n_split, b_negat;
	int n, len, total = 0;
	tchar_t sz_num[INT_LEN + 1] = { 0 };
	tchar_t* token;

	if (is_null(fmt))
		return ltoxs(ln, buf, max);

	n_split = 0;

	if (ln < 0)
		b_negat = 1;
	else
		b_negat = 0;

	if (b_negat && *fmt == _T('('))
	{
		b_negat++;
	}

	while (*fmt == _T('#'))
	{
		fmt++;
	}

	if (*fmt == _T(','))
	{
		fmt++;
		n_split = 0;
		while (*fmt == _T('#'))
		{
			n_split++;
			fmt++;
		}
	}

	while (*fmt >= _T('0') && *fmt <= _T('9'))
		fmt++;

	if (b_negat)
		ln = 0 - ln;

	ltoxs(ln, sz_num, INT_LEN);

	len = xslen(sz_num);

	if (len && n_split)
	{
		if (len % n_split)
			n = (len / n_split);
		else
			n = (len / n_split) - 1;
	}
	else
	{
		n = 0;
	}

	if (len + n + b_negat + xslen(fmt) > max)
		return 0;

	total = 0;

	if (b_negat == 1)
	{
		if (buf)
		{
			buf[total] = _T('-');
		}
		total++;
	}
	else if (b_negat == 2)
	{
		if (buf)
		{
			buf[total] = _T('(');
		}
		total++;
	}

	token = sz_num;
	while (*token != _T('\0'))
	{
		if (token > sz_num && n && n_split && !(len % n_split))
		{
			if (buf)
			{
				buf[total] = _T(',');
			}
			total++;
			n--;
		}

		if (buf)
		{
			buf[total] = *token;
		}
		total++;
		len--;

		token++;
	}

	if (b_negat == 2)
	{
		if (buf)
		{
			buf[total] = _T(')');
		}
		total++;
	}

	len = xslen(fmt);
	if (buf)
	{
		xsncpy(buf + total, fmt, len);
	}
	total += len;

	return total;
}

bool_t is_zero_numeric(double dbl, int scale)
{
	if (!scale || scale > MAX_DOUBLE_DIGI)
		scale = MAX_DOUBLE_DIGI;
	else if (scale < 0)
		scale = 0;

	while (scale--)
	{
		dbl *= 10;
	}

	return ((int)dbl) ? 0 : 1;
}

double parse_numeric(const tchar_t* token, int len)
{
	double f = 0;
	int b_negat = 0;
	int sz_len = 0;
	tchar_t sz_num[NUM_LEN + 1] = { 0 };

	if (len < 0)
		len = xslen(token);

	if (is_null(token) || !len)
		return 0;

	while (*token != _T('\0') && len && sz_len < NUM_LEN)
	{
		if (*token == _T('('))
		{
			b_negat = 1;
		}
		else if (*token == _T('+') || *token == _T('-') || (*token >= _T('0') && *token <= _T('9')) || *token == _T('.'))
		{
			sz_num[sz_len] = *token;
			sz_len++;
		}
		else if (*token != _T(','))
		{
			break;
		}

		token++;
		len--;
	}

	f = xsntonum(sz_num, sz_len);

	if (b_negat)
	{
		f = 0 - f;
	}

	if (*token == _T('%'))
		return f / 100;
	else if (xsncmp(token, _T("万"), xslen(_T("万"))) == 0)
		return f * 10000;
	else
		return f;
}

int format_numeric(double dbl, const tchar_t* fmt, tchar_t* buf, int max)
{
	int n_split, n_prec, b_percent, b_negat;
	int n, len, total = 0;
	tchar_t sz_num[NUM_LEN + 1] = { 0 };
	tchar_t* token;

	if (is_null(fmt))
		return numtoxs(dbl, buf, max);

	n_split = n_prec = b_percent = 0;

	if (dbl < 0)
		b_negat = 1;
	else
		b_negat = 0;

	if (*fmt == _T('('))
	{
		fmt++;

		if (b_negat)
			b_negat++;
	}

	while (*fmt == _T('#'))
		fmt++;

	if (*fmt == _T(','))
	{
		fmt++;
		n_split = 0;
		while (*fmt == _T('#'))
		{
			n_split++;
			fmt++;
		}
	}

	if (*fmt == _T('.'))
	{
		fmt++;
		n_prec = 0;
		while (*fmt == _T('#'))
		{
			n_prec++;
			fmt++;
		}
	}

	if (*fmt == _T('%'))
	{
		b_percent = 1;
		fmt++;
	}
	else if (xsncmp(fmt, _T("万"), xslen(_T("万"))) == 0)
	{
		b_percent = 2;
		fmt += xslen(_T("万"));
	}

	if (*fmt == _T(')'))
		fmt++;

	if (b_negat)
		dbl = 0 - dbl;

	if (b_percent == 1)
		dbl *= 100;
	else if (b_percent == 2)
		dbl /= 10000;

	numtoxs_dig(dbl, n_prec, sz_num, NUM_LEN);

	len = 0;
	token = sz_num;
	while (*token != _T('.') && *token != _T('\0'))
	{
		token++;
		len++;
	}

	if (n_prec)
	{
		if (*token == _T('.'))
		{
			token++;
		}
		else
		{
			*token = _T('.');
			*(token + 1) = _T('\0');
			token++;
		}

		while (n_prec--)
		{
			if (*token == _T('\0'))
			{
				*token = _T('0');
				*(token + 1) = _T('\0');
			}

			token++;
		}
	}
	else
	{
		*token = _T('\0');
	}

	n_prec = (int)(token - sz_num) - len;

	if (len && n_split)
	{
		if (len % n_split)
			n = (len / n_split);
		else
			n = (len / n_split) - 1;
	}
	else
	{
		n = 0;
	}

	if (len + n + n_prec + b_negat + b_percent + xslen(fmt) > max)
		return 0;

	total = 0;

	if (b_negat == 1)
	{
		if (buf)
		{
			buf[total] = _T('-');
		}
		total++;
	}
	else if (b_negat == 2)
	{
		if (buf)
		{
			buf[total] = _T('(');
		}
		total++;
	}

	token = sz_num;
	while (*token != _T('\0'))
	{
		if (token > sz_num && n && n_split && !(len % n_split))
		{
			if (buf)
			{
				buf[total] = _T(',');
			}
			total++;
			n--;
		}

		if (buf)
		{
			buf[total] = *token;
		}
		total++;
		len--;

		token++;
	}

	if (b_percent == 1)
	{
		if (buf)
		{
			buf[total] = _T('%');
		}
		total++;
	}
	else if (b_percent == 2)
	{
		if (buf)
		{
			xscpy(buf + total, _T("万"));
		}
		total += xslen(_T("万"));
	}

	if (b_negat == 2)
	{
		if (buf)
		{
			buf[total] = _T(')');
		}
		total++;
	}

	len = xslen(fmt);
	if (buf)
	{
		xsncpy(buf + total, fmt, len);
	}
	total += len;

	if (buf)
	{
		buf[total] = _T('\0');
	}

	return total;
}

int mul_div_int(int m1, int m2, int d)
{
	return (int)((double)(m1 * m2) / (double)d);
}

short mul_div_short(short m1, short m2, short d)
{
	return (short)((float)(m1 * m2) / (float)d);
}
