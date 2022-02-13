/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc code page document

	@module	acp_gbk.c | implement file

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

#include "acp.h"

#include "../xdkimp.h"

int gb2312_code_sequence(unsigned char b)
{
	if ((b & ~0x7F) == 0)
		return 1;

	//head
	if (0xa1 <= b && b <= 0xf7)
		return 2;

	//tail
	//if(0xa1 <= b && b <= 0xfe)
	//return 1;

	return 2;
}

int gb2312_byte_to_unicode(const byte_t* src, wchar_t* dest)
{
#ifdef XDK_SUPPORT_ACP_TABLE
	return table_gb2312_seek_unicode(src, (unsigned short*)dest);
#else
	return share_gb2312_seek_unicode(src, (unsigned short*)dest);
#endif
}

int gb2312_to_unicode(const byte_t* src, dword_t slen, wchar_t* dest, int dlen)
{
	int len = 0, total = 0;

	while (total < slen && len < dlen)
	{
#ifdef XDK_SUPPORT_ACP_TABLE
		len += table_gb2312_seek_unicode((src + total), ((dest)? (unsigned short*)(dest + len) : NULL));
#else
		len += share_gb2312_seek_unicode((src + total), ((dest) ? (unsigned short*)(dest + len) : NULL));
#endif
		total += gb2312_code_sequence((unsigned char)(src[total]));
	}

	return len;
}

int unicode_byte_to_gb2312(wchar_t ch, byte_t* buf)
{
#ifdef XDK_SUPPORT_ACP_TABLE
	return table_unicode_seek_gb2312((unsigned short)ch, (unsigned char*)buf);
#else
	return share_unicode_seek_gb2312((unsigned short)ch, (unsigned char*)buf);
#endif
}

int unicode_to_gb2312(const wchar_t* src, int slen, byte_t* dest, dword_t dlen)
{
	int len = 0, total = 0;

	while (total < slen && len < dlen)
	{
#ifdef XDK_SUPPORT_ACP_TABLE
		len += table_unicode_seek_gb2312((unsigned short)(src[total]), ((dest) ? (unsigned char*)(dest + len) : NULL));
#else
		len += share_unicode_seek_gb2312((unsigned short)(src[total]), ((dest) ? (unsigned char*)(dest + len) : NULL));
#endif
		total++;
	}

	return len;
}

int gb2312_code_count(void)
{
	return (0xFE - 0xA1 + 1) * (0xFE - 0xA1 + 1);
}

int ascii_code_count(void)
{
	return (0xFE - 0x20 + 1);
}

bool_t next_ascii_char(byte_t* pch)
{
	if (*pch == 0)
		*pch = 0x20;
	else if (*pch == 0xFE)
		*pch = 0;
	else
		*pch = *pch + 1;

	return (*pch) ? bool_true : bool_false;
}

bool_t next_gb2312_char(byte_t* pch)
{
	byte_t h, l;

	h = pch[0];
	l = pch[1];

	if (!h && !l)
	{
		pch[0] = 0xA1;
		pch[1] = 0xA1;
		return bool_true;
	}
	else if (h == 0xFE && l == 0xFE)
	{
		pch[0] = 0x00;
		pch[1] = 0x00;
		return bool_false;
	}

	if (l == 0xFE)
	{
		h++;
		pch[0] = h;
		pch[1] = 0xA1;
	}
	else
	{
		l++;
		pch[1] = l;
	}

	return bool_true;
}