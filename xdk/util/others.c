/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc utility document

	@module	others.c | implement file

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

#include "others.h"

#include "../xdkimp.h"
#include "../xdkstd.h"

void bytes_turn(byte_t* ba, int n)
{
	byte_t b;
	int i;

	for (i = 0; i < n / 2; i++)
	{
		b = ba[i];
		ba[i] = ba[n - 1 - i];
		ba[n - 1 - i] = b;
	}
}

int format_password(const tchar_t* sz, tchar_t* buf, int max)
{
	int len;

	len = xslen(sz);
	len = (len < max) ? len : max;
	max = len;

	while (len--)
	{
		if (buf)
		{
			buf[len] = _T('*');
		}
	}

	if (buf)
		buf[max] = _T('\0');

	return max;
}


static int _next_book_mark(tchar_t* book_mark)
{
	int t1, t2, t3;

	if (!book_mark)
		return 0;

	if (xslen(book_mark) == 0)
	{
		book_mark[0] = 42;
		book_mark[1] = 42;
		book_mark[2] = 42;
		return 1;
	}

	t1 = (int)book_mark[0];
	t2 = (int)book_mark[1];
	t3 = (int)book_mark[2];

	t3++;
	if (t3 == 49)
	{
		t3 = 42;
		if (t2 < 91)
			t2 += 7;
		else
		{
			t2 = 42;
			if (t1 < 91)
				t1 += 7;
			else
				return 0;
		}
	}

	book_mark[0] = t1;
	book_mark[1] = t2;
	book_mark[2] = t3;

	return 1;
}


int peek_word(const tchar_t* str, tchar_t* pch)
{
	int n;

	if (str == NULL || *str == _T('\0'))
		return 0;

#if defined(_UNICODE) || defined(UNICODE)
	n = ucs_sequence(*str);
#else
	n = (int)mbs_sequence(*str);
#endif

	if (pch) xsncpy(pch, str, n);
	return n;
}

int words_count(const tchar_t* str, int len)
{
	int n = 0, total = 0;

	if (len < 0)
		len = xslen(str);
	if (is_null(str) || !len)
		return 0;

	while (total < len)
	{
		n += peek_word((str + n), NULL);
		total ++;
	}

	return total;
}

void split_path(const tchar_t* pathfile, tchar_t* path, tchar_t* file, tchar_t* ext)
{
	const tchar_t* token;
	int n, extlen = 0;

	if (is_null(pathfile))
		return;

	token = pathfile + xslen(pathfile);
	while (token != pathfile && *token != _T('\\') && *token != _T('/'))
	{
		token--;

		if (*token == _T('.'))
		{
			extlen = xslen(token);
			if (ext)
			{
				n = (extlen - 1 < RES_LEN) ? (extlen - 1) : RES_LEN;
				xsncpy(ext, token + 1, n);
			}
			break;
		}
	}

	while (token != pathfile && *token != _T('\\') && *token != _T('/'))
	{
		token--;
	}

	if (file)
	{
		if (*token == _T('\\') || *token == _T('/'))
		{
			n = xslen(token + 1) - extlen;
			if (n > PATH_LEN) n = PATH_LEN;

			xsncpy(file, token + 1, n);
		}
		else
		{
			n = xslen(token) - extlen;
			if (n > PATH_LEN) n = PATH_LEN;

			xsncpy(file, token, n);
		}
	}

	if (path)
	{
		n = (int)(token - pathfile);
		if (n > PATH_LEN) n = PATH_LEN;

		xsncpy(path, pathfile, n);
	}
}

void split_file(const tchar_t* pathfile, tchar_t* path, tchar_t* file)
{
	const tchar_t* token;
	int n;

	if (is_null(pathfile))
		return;

	token = pathfile + xslen(pathfile);

	while (token != pathfile && *token != _T('\\') && *token != _T('/'))
	{
		token--;
	}

	if (file)
	{
		if (*token == _T('\\') || *token == _T('/'))
		{
			n = xslen(token + 1);
			if (n > PATH_LEN) n = PATH_LEN;

			xsncpy(file, token + 1, n);
		}
		else
		{
			n = xslen(token);
			if (n > PATH_LEN) n = PATH_LEN;

			xsncpy(file, token, n);
		}
	}

	if (path)
	{
		n = (int)(token - pathfile);
		if (n > PATH_LEN) n = PATH_LEN;

		xsncpy(path, pathfile, n);
	}
}

bool_t is_ip(const tchar_t* addr)
{
	if (addr[0] >= _T('0') && addr[0] <= _T('9'))
	{
		if ((addr[1] >= _T('0') && addr[1] <= _T('9')) || addr[1] == _T('.'))
		{
			if ((addr[2] >= _T('0') && addr[2] <= _T('9')) || addr[2] == _T('.'))
				return 1;
		}
	}

	return 0;
}

void parse_bytes_range(tchar_t* sz_range, dword_t* phoff, dword_t* ploff, dword_t* psize, long long* ptotal)
{
	int len, step = 0;
	const tchar_t* token = sz_range;
	long long ll_from = 0;
	long long ll_to = 0;
	long long ll_total = 0;

	*phoff = *ploff = *psize = 0;

	while (*token != _T('\0'))
	{
		while ((*token < _T('0') || *token > _T('9')) && *token != _T('\0'))
			token++;

		len = 0;
		while (*token >= _T('0') && *token <= _T('9'))
		{
			token++;
			len++;
		}

		if (step == 0)
		{
			ll_from = xsntoll(token - len, len);
			step++;
		}
		else if (step == 1)
		{
			ll_to = xsntoll(token - len, len);
			step++;
		}
		else
		{
			ll_total = xsntoll(token - len, len);
			step++;
		}

		if (*token != _T('\0'))
			token++;
	}

	*phoff = GETHDWORD(ll_from);
	*ploff = GETLDWORD(ll_from);
	*psize = (dword_t)(ll_to - ll_from + 1);
	*ptotal = ll_total;
}

void format_bytes_range(tchar_t* sz_range, dword_t hoff, dword_t loff, dword_t size, long long total)
{
	tchar_t sz_from[NUM_LEN + 1] = { 0 };
	tchar_t sz_to[NUM_LEN + 1] = { 0 };
	tchar_t sz_total[NUM_LEN + 1] = { 0 };

	unsigned long long ll = 0;

	ll = MAKELWORD(loff, hoff);
	lltoxs(ll, sz_from, NUM_LEN);

	ll += (size - 1);
	lltoxs(ll, sz_to, NUM_LEN);

	lltoxs(total, sz_total, NUM_LEN);

	xsprintf(sz_range, _T("%s-%s/%s"), sz_from, sz_to, sz_total);
}

byte_t parse_proto(const tchar_t* file)
{
	if (!file)
		return -1;

	if (xsnicmp(file, _T("http:"), xslen(_T("http:"))) == 0)
	{
		return _PROTO_HTTP;
	}
	else if (xsnicmp(file, _T("https:"), xslen(_T("https:"))) == 0)
	{
		return _PROTO_HTTP;
	}
	else if (xsnicmp(file, _T("ssh:"), xslen(_T("ssh:"))) == 0)
	{
		return _PROTO_SSH;
	}
	else if (xsnicmp(file, _T("tftp:"), xslen(_T("tftp:"))) == 0)
	{
		return _PROTO_TFTP;
	}
	else if (xsnicmp(file, _T("tftps:"), xslen(_T("tftps:"))) == 0)
	{
		return _PROTO_TFTP;
	}
	else if (xsnicmp(file, _T("\\\\"), xslen(_T("\\\\"))) == 0 || xsnicmp(file, _T("//"), xslen(_T("//"))) == 0)
	{
		return _PROTO_NFS;
	}
	else if ((file[0] == _T('/') && file[1] != _T('/')) || file[1] == _T(':'))
	{
		return _PROTO_LOC;
	}

	return _PROTO_UNKNOWN;
}

void parse_url(const tchar_t* url, tchar_t** proat, int* prolen, tchar_t** addrat, int* addrlen, tchar_t** portat, int* portlen, tchar_t** objat, int* objlen, tchar_t** qryat, int* qrylen)
{
	tchar_t* token = (tchar_t*)url;

	*proat = *addrat = *portat = *objat = *qryat = NULL;
	*prolen = *addrlen = *portlen = *objlen = *qrylen = 0;

	if (!token)
		return;

	/*skip http://*/
	if (token = (tchar_t*)xsstr(url, _T("://")))
	{
		*proat = (tchar_t*)url;
		*prolen = (int)(token - url);
		token += xslen(_T("://"));
	}
	else
	{
		token = (tchar_t*)url;
	}

	/*get www.aaa.bbb*/
	*addrat = token;
	while (*token != _T('\0') && *token != _T(':') && *token != _T('/') && *token != _T('\\'))
	{
		*addrlen = *addrlen + 1;
		token++;
	}
	if (*token == _T('\0'))
		return;

	if (*token == _T(':'))
	{
		/*skip ':'*/
		token++;
		/*get 80*/
		*portat = token;
		while (*token != _T('\0') && *token != _T('/') && *token != _T('\\'))
		{
			*portlen = *portlen + 1;
			token++;
		}
		if (*token == _T('\0'))
			return;
	}

	/*get somfile.html*/
	*objat = token;
	while (*token != _T('\0') && *token != _T('?'))
	{
		*objlen = *objlen + 1;
		token++;
	}
	if (*token == _T('\0'))
		return;

	/*skip '?'*/
	token++;
	/*skip blank*/
	while ((*token == _T(' ') || *token == _T('\t')) && *token != _T('\0'))
		token++;

	/*get key=val...*/
	*qryat = token;
	while (*token != _T('#') && *token != _T('\0'))
	{
		*qrylen = *qrylen + 1;
		token++;
	}
}