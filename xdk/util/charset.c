/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc utility document

	@module	charset.c | implement file

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

#include "charset.h"

#include "../xdkimp.h"
#include "../xdkstd.h"

int parse_charset(const tchar_t* enstr)
{
	if (xsnicmp(enstr, CHARSET_GB2312, xslen(CHARSET_GB2312)) == 0)
		return _GB2312;
	else if (xsnicmp(enstr, CHARSET_UTF8, xslen(CHARSET_UTF8)) == 0)
		return _UTF8;
	else if (xsnicmp(enstr, CHARSET_UTF16, xslen(CHARSET_UTF16)) == 0)
		return _UCS2;
	else
		return _UNKNOWN;
}

void format_charset(int encode, tchar_t* buf)
{
	switch (encode)
	{
	case _GB2312:
		xscpy(buf, CHARSET_GB2312);
		break;
	case _UTF8:
		xscpy(buf, CHARSET_UTF8);
		break;
	case _UTF16_BIG:
		xscpy(buf, CHARSET_UTF16);
		break;
	case _UTF16_LIT:
		xscpy(buf, CHARSET_UTF16);
		break;
	default:
		buf[0] = _T('\0');
	}
}

int parse_encode(const tchar_t* enstr)
{
	if (xsnicmp(enstr, _T("gb2312"), xslen(_T("gb2312"))) == 0)
		return _GB2312;
	else if (xsnicmp(enstr, _T("utf-8"), xslen(_T("utf-8"))) == 0)
		return _UTF8;
	else if (xsnicmp(enstr, _T("utf-16-lit"), xslen(_T("utf-16-lit"))) == 0)
		return _UTF16_LIT;
	else if (xsnicmp(enstr, _T("utf-16-big"), xslen(_T("utf-16-big"))) == 0)
		return _UTF16_BIG;
	else
		return _UNKNOWN;
}

void format_encode(int encode, tchar_t* buf)
{
	switch (encode)
	{
	case _GB2312:
		xscpy(buf, _T("gb2312"));
		break;
	case _UTF8:
		xscpy(buf, _T("utf-8"));
		break;
	case _UTF16_BIG:
		xscpy(buf, _T("utf-16-big"));
		break;
	case _UTF16_LIT:
		xscpy(buf, _T("utf-16-lit"));
		break;
	default:
		buf[0] = _T('\0');
	}
}

int parse_utfbom(const byte_t* buf, int len)
{
	if (len > 1 && buf[0] == 0xFF && buf[1] == 0xFE)
		return _UTF16_LIT;

	if (len > 1 && buf[0] == 0xFE && buf[1] == 0xFF)
		return _UTF16_BIG;

	if (len > 2 && buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
		return _UTF8;

	return _GB2312;
}

int format_utfbom(int encode, byte_t* buf)
{
	if (encode == _UTF16_LIT)
	{
		if (buf)
		{
			buf[0] = 0xFF;
			buf[1] = 0xFE;
		}
		return 2;
	}
	else if (encode == _UTF16_BIG)
	{
		if (buf)
		{
			buf[0] = 0xFE;
			buf[1] = 0xFF;
		}
		return 2;
	}
	else if (encode == _UTF8)
	{
		if (buf)
		{
			buf[0] = 0xEF;
			buf[1] = 0xBB;
			buf[2] = 0xBF;
		}
		return 3;
	}

	return 0;
}

int skip_utfbom(const byte_t* buf)
{
	if (buf[0] == 0xFF && buf[1] == 0xFE)
		return 2;

	if (buf[0] == 0xFE && buf[1] == 0xFF)
		return 2;

	if (buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF)
		return 3;

	return 0;
}

