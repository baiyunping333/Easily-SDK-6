/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory font document

	@module	mfnt_SansSerif.c | implement file

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

#include "mfnt.h"
#include "mpix.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"
#include "../xdkmgc.h"

typedef struct _Fixed_font_t{
	handle_head head;

	const font_glyph_t* a_font_glyph;
	const font_glyph_t* c_font_glyph;
}Fixed_font_t;

static font_t create_font(int width, int height, dword_t attr)
{
	Fixed_font_t* pfnt;

	TRY_CATCH;

	pfnt = (Fixed_font_t*)xmem_alloc(sizeof(Fixed_font_t));
	pfnt->head.tag = _HANDLE_FONT;

	pfnt->a_font_glyph = &System14x16_glyph;
	pfnt->c_font_glyph = &Song16x16_glyph;
	
	END_CATCH;

	return &(pfnt->head);
ONERROR:
	XDK_TRACE_LAST;

	if (pfnt)
	{
		xmem_free(pfnt);
	}

	return NULL;
}

static void destroy_font(font_t fnt)
{
	Fixed_font_t* pfnt = (Fixed_font_t*)fnt;

	XDK_ASSERT(fnt && fnt->tag == _HANDLE_FONT);

	xmem_free(pfnt);
}

static void get_fontinfo(font_t fnt, font_info_t* pfontinfo)
{
	Fixed_font_t* pfnt = (Fixed_font_t*)fnt;

	XDK_ASSERT(fnt && fnt->tag == _HANDLE_FONT);

	pfontinfo->width = (pfnt->a_font_glyph->width > pfnt->c_font_glyph->width) ? pfnt->a_font_glyph->width : pfnt->c_font_glyph->width;
	pfontinfo->height = (pfnt->a_font_glyph->height > pfnt->c_font_glyph->height) ? pfnt->a_font_glyph->height : pfnt->c_font_glyph->height;
	pfontinfo->baseline = (pfnt->a_font_glyph->ascent > pfnt->c_font_glyph->ascent) ? pfnt->a_font_glyph->ascent : pfnt->c_font_glyph->ascent;
	pfontinfo->firstchar = pfnt->a_font_glyph->firstchar;
	pfontinfo->lastchar = 0;
}

static void get_textsize(font_t fnt, const tchar_t *str, int len, int *pwidth, int *pheight, int *pbase)
{
	Fixed_font_t* pfnt = (Fixed_font_t*)fnt;

	int i, j, b = 0, h = 0, w = 0;
	byte_t chs[CHS_LEN + 1] = { 0 };
	dword_t n, total = 0;

	XDK_ASSERT(fnt && fnt->tag == _HANDLE_FONT);

	if (len < 0) len = xslen(str);

	for (i = 0; i < len; i++)
	{
#if defined(_UNICODE) || defined(UNICODE)
		ucs_byte_to_gb2312(str[total], chs);
		n = ucs_sequence(str[total]);
#else
		utf8_byte_to_gb2312((str + total), chs);
		n = utf8_sequence(str[total]);
#endif
		total += n;

		if (is_ascii(chs[0]))
		{
			if (pfnt->a_font_glyph->widthused)
			{
				j = chs[0] - pfnt->a_font_glyph->firstchar;
				w += pfnt->a_font_glyph->widthused[j];
			}
			else
			{
				w += pfnt->a_font_glyph->width;
			}

			if (h < pfnt->a_font_glyph->height)
				h = pfnt->a_font_glyph->height;

			if (b < pfnt->a_font_glyph->ascent)
				b = pfnt->a_font_glyph->ascent;
		}
		else
		{
			j = (chs[0] - 161) * 94 + chs[1] - 161;
			w += pfnt->c_font_glyph->width;

			if (h < pfnt->c_font_glyph->height)
				h = pfnt->c_font_glyph->height;

			if (b < pfnt->c_font_glyph->ascent)
				b = pfnt->c_font_glyph->ascent;
		}
	}

	if (pwidth) *pwidth = w;
	if (pheight) *pheight = h;
	if (pbase) *pbase = b;
}

static int get_charpixmap(font_t fnt, const tchar_t* pch, mem_pixmap_ptr ppixmap)
{
	Fixed_font_t* pfnt = (Fixed_font_t*)fnt;

	const sword_t* addr;
	int i, w;
	byte_t chs[CHS_LEN + 1] = { 0 };

	XDK_ASSERT(fnt && fnt->tag == _HANDLE_FONT);
	XDK_ASSERT(pch != NULL);

#if defined(_UNICODE) || defined(UNICODE)
	ucs_byte_to_gb2312(*pch, chs);
#else
	utf8_byte_to_gb2312(pch, chs);
#endif

	if (is_ascii(chs[0]))
	{
		i = chs[0] - pfnt->a_font_glyph->firstchar;
		if (i >= pfnt->a_font_glyph->count)
		{
			i = pfnt->a_font_glyph->defaultchar - pfnt->a_font_glyph->firstchar;
		}
		addr = pfnt->a_font_glyph->pixmap + i * pfnt->a_font_glyph->height;

		if (ppixmap)
		{
			for (i = 0; i < ppixmap->height; i++)
			{
				ppixmap->data[2 * i] = (byte_t)(addr[i] >> 8) & 0xFF;
				ppixmap->data[2 * i + 1] = (byte_t)(addr[i] & 0x00FF);
			}
		}

		if (pfnt->a_font_glyph->widthused)
		{
			i = chs[0] - pfnt->a_font_glyph->firstchar;
			w = pfnt->a_font_glyph->widthused[i];
		}
		else
		{
			w = pfnt->a_font_glyph->width;
		}
	}
	else
	{
		i = (chs[0] - 161) * 94 + chs[1] - 161;
		addr = pfnt->c_font_glyph->pixmap + i * pfnt->c_font_glyph->height;

		if (ppixmap)
		{
			for (i = 0; i < ppixmap->height; i++)
			{
				ppixmap->data[2 * i] = (byte_t)(addr[i] >> 8) & 0xFF;
				ppixmap->data[2 * i + 1] = (byte_t)(addr[i] & 0x00FF);
			}
		}

		w = pfnt->c_font_glyph->width;
	}

	return w;
}

/*****************************************************************************************************************/

mem_font_t font_Fixed = {
	MGC_FONT_FIXED, /*the memroy font*/

	16,	/* font width in pixels*/
	16,	/* font height in pixels*/
	0, /* font rotation*/
	0, /* font attributes: kerning/antialias*/

	create_font,
	destroy_font,
	get_fontinfo,
	get_textsize,
	get_charpixmap
};


