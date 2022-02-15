/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc glyph pattern document

	@module	gly_pattern.c | implement file

	@devnote 张文权 2021.01 - 2021.12 v6.0
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

#include "gly.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"

void xfont_from_glyph_info(xfont_t* pxf, const glyph_info_t* pgi)
{
	xscpy(pxf->family, pgi->name);

	if (xsicmp(pgi->weight, _T("bold")) == 0)
		xscpy(pxf->weight, _T("700"));
	else if (xsicmp(pgi->weight, _T("medium")) == 0)
		xscpy(pxf->weight, _T("500"));
	else
		xscpy(pxf->weight, _T("300"));

	if (xsicmp(pgi->style, _T("oblique")) == 0)
		xscpy(pxf->style, GDI_ATTR_FONT_STYLE_OBLIQUE);
	else if(xsicmp(pgi->style, _T("italic")) == 0)
		xscpy(pxf->style, GDI_ATTR_FONT_STYLE_ITALIC);
	else
		xscpy(pxf->style, GDI_ATTR_FONT_STYLE_REGULAR);

	xscpy(pxf->size, pgi->size);
}

void xfont_to_glyph_info(const xfont_t* pxf, glyph_info_t* pgi)
{
	xscpy(pgi->name, pxf->family);

	if (xstol(pxf->weight) >= 700)
		xscpy(pgi->weight, _T("bold"));
	else if (xstol(pxf->weight) >= 400)
		xscpy(pgi->weight, _T("medium"));
	else
		xscpy(pgi->weight, _T("light"));

	if (xsicmp(pxf->style, GDI_ATTR_FONT_STYLE_OBLIQUE) == 0)
		xscpy(pgi->style, _T("oblique"));
	else if (xsicmp(pxf->style, GDI_ATTR_FONT_STYLE_ITALIC) == 0)
		xscpy(pgi->style, _T("italic"));
	else
		xscpy(pgi->style, _T("regular"));

	xscpy(pgi->size, pxf->size);
}

void parse_glyph_pattern(xfont_t* pxf, const tchar_t* buf, int len)
{
	tchar_t fname[RES_LEN + 1] = { 0 };
	tchar_t fweight[NUM_LEN + 1] = { 0 };
	tchar_t fstyle[INT_LEN + 1] = { 0 };
	tchar_t fsize[INT_LEN + 1] = { 0 };
	int n = 0;
	const tchar_t* pre;

	if (len < 0) len = xslen(buf);
	if (!len) 
		return;

	n = 0;
	pre = buf;
	while (*buf != _T('-') && *buf != _T('\0') && n < len)
	{
		buf++;
		n++;
	}
	xsncpy(fname, pre, n);

	if (*buf == _T('-'))
	{
		buf++;
		n++;
	}
	len -= n;

	n = 0;
	pre = buf;
	while (*buf != _T('-') && *buf != _T('\0') && n < len)
	{
		buf++;
		n++;
	}
	xsncpy(fweight, pre, n);

	if (*buf == _T('-'))
	{
		buf++;
		n++;
	}
	len -= n;

	n = 0;
	pre = buf;
	while (*buf != _T('-') && *buf != _T('\0') && n < len)
	{
		buf++;
		n++;
	}
	xsncpy(fstyle, pre, n);

	n = 0;
	pre = buf;
	while (*buf != _T('-') && *buf != _T('\0') && n < len)
	{
		buf++;
		n++;
	}
	xsncpy(fweight, pre, n);

	if (*buf == _T('-'))
	{
		buf++;
		n++;
	}
	len -= n;

	n = 0;
	pre = buf;
	while (*buf != _T('-') && *buf != _T('\0') && n < len)
	{
		buf++;
		n++;
	}
	xsncpy(fsize, pre, n);

	xscpy(pxf->family, fname);
	
	if (xsicmp(fweight, _T("bold")) == 0)
		xscpy(pxf->weight, _T("700"));
	else if (xsicmp(fweight, _T("medium")) == 0)
		xscpy(pxf->weight, _T("500"));
	else
		xscpy(pxf->weight, _T("300"));

	if (xsicmp(pxf->style, _T("oblique")) == 0)
		xscpy(pxf->style, GDI_ATTR_FONT_STYLE_OBLIQUE);
	else if (xsicmp(pxf->style, _T("italic")) == 0)
		xscpy(pxf->style, GDI_ATTR_FONT_STYLE_ITALIC);
	else
		xscpy(pxf->style, GDI_ATTR_FONT_STYLE_REGULAR);

	xscpy(pxf->size, fsize);
}

int format_glyph_pattern(const xfont_t* pxf, tchar_t* buf, int max)
{
	tchar_t fname[RES_LEN + 1], fweight[NUM_LEN + 1], fstyle[INT_LEN + 1], fsize[INT_LEN + 1];
	int n;

	if (is_null(pxf->family))
		xscpy(fname, _T("*"));
	else
		xsncpy(fname, pxf->family, RES_LEN);

	n = xstol(pxf->weight);
	if (n < 400)
		xscpy(fweight, _T("light"));
	else if (n <= 500)
		xscpy(fweight, _T("medium"));
	else
		xscpy(fweight, _T("bold"));

	if (xsicmp(pxf->style, GDI_ATTR_FONT_STYLE_OBLIQUE) == 0)
		xscpy(fstyle, _T("oblique"));
	else if (xsicmp(pxf->style, GDI_ATTR_FONT_STYLE_ITALIC) == 0)
		xscpy(fstyle, _T("italic"));
	else
		xscpy(fstyle, _T("regular"));

	if (is_null(pxf->size))
		xscpy(fsize, _T("12"));
	else
		xsncpy(fsize, pxf->size, INT_LEN);

	return xsprintf(buf, _T("%s-%s-%s-%s"), fname, fweight, fstyle, fsize);
}

void calc_glyph_size(xfont_t* pxf, int* pw, int* ph)
{
	float fs;

	fs = (float)xstof(pxf->size);
	if (fs <= 5.5f)
	{
		if (pw) *pw = 8;
		if (ph) *ph = 8;
	}
	else if (fs <= 12.0f)
	{
		if (pw) *pw = 16;
		if (ph) *ph = 16;
	}
	else if (fs <= 18.0f)
	{
		if (pw) *pw = 24;
		if (ph) *ph = 24;
	}
	else if (fs <= 24.0f)
	{
		if (pw) *pw = 32;
		if (ph) *ph = 32;
	}
	else if (fs <= 26.0f)
	{
		if (pw) *pw = 40;
		if (ph) *ph = 40;
	}
	else if (fs <= 36.0f)
	{
		if (pw) *pw = 48;
		if (ph) *ph = 48;
	}
	else if (fs <= 42.0f)
	{
		if (pw) *pw = 56;
		if (ph) *ph = 56;
	}
	else
	{
		if (pw) *pw = 64;
		if (ph) *ph = 64;
	}
}