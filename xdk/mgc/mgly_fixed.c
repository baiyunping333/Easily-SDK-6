/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc font glyph document

	@module	fdrv_fixed.c | implement file

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

#include "mgly.h"
#include "mpix.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"
#include "../xdkmgc.h"

typedef struct _Fixed_driver_t{
	handle_head head;

	const glyph_info_t* a_font_glyph;
	const glyph_info_t* c_font_glyph;
}Fixed_driver_t;

static glyph_t create_glyph(const xfont_t* pxf)
{
	Fixed_driver_t* pfd = NULL;

	pfd = (Fixed_driver_t*)xmem_alloc(sizeof(Fixed_driver_t));
	pfd->head.tag = _HANDLE_GLYPH;

	pfd->a_font_glyph = find_glyph_info(_T("ASCII"), pxf);
	pfd->c_font_glyph = find_glyph_info(_T("GB2312"), pxf);
	
	return &(pfd->head);
}

static void destroy_glyph(glyph_t gly)
{
	Fixed_driver_t* pfd = (Fixed_driver_t*)gly;

	XDK_ASSERT(gly && gly->tag == _HANDLE_GLYPH);

	xmem_free(pfd);
}

static void get_glyph_metrix(glyph_t gly, const tchar_t *pch, glyph_metrix_t* pmetr)
{
	Fixed_driver_t* pfd = (Fixed_driver_t*)gly;

	int j, b = 0, h = 0, w = 0;
	byte_t chs[CHS_LEN + 1] = { 0 };
	dword_t n, m;
	byte_t* pb;

	XDK_ASSERT(gly && gly->tag == _HANDLE_GLYPH);

	XDK_ASSERT(pfd->a_font_glyph != NULL && pfd->c_font_glyph != NULL);

	if (!(*pch))
	{
		w = (pfd->c_font_glyph->width > pfd->a_font_glyph->width) ? pfd->c_font_glyph->width : pfd->a_font_glyph->width;
		h = (pfd->c_font_glyph->height > pfd->a_font_glyph->height) ? pfd->c_font_glyph->height : pfd->a_font_glyph->height;
		b = (pfd->c_font_glyph->ascent > pfd->a_font_glyph->ascent) ? pfd->c_font_glyph->ascent : pfd->a_font_glyph->ascent;

		if (pmetr) pmetr->width = w;
		if (pmetr) pmetr->height = h;
		if (pmetr) pmetr->ascent = b;

		return;
	}

#if defined(_UNICODE) || defined(UNICODE)
	ucs_byte_to_gb2312(pch[0], chs);
	n = ucs_sequence(pch[0]);
#else
	utf8_byte_to_gb2312(pch, chs);
	n = utf8_sequence(pch[0]);
#endif

	if (is_ascii(chs[0]) && pfd->a_font_glyph->glyph)
	{
		m = 2 + pfd->a_font_glyph->bytesperline * pfd->a_font_glyph->height;
		j = chs[0] - pfd->a_font_glyph->firstchar;
		if (j<0 || j >= pfd->a_font_glyph->characters)
		{
			j = pfd->a_font_glyph->defaultchar - pfd->a_font_glyph->firstchar;
		}
		pb = (pfd->a_font_glyph->glyph)? xshare_lock(pfd->a_font_glyph->glyph, j * m, m) : NULL;
		if (pb)
		{
			w = GET_SWORD_LOC(pb, 0);
			xshare_unlock(pfd->a_font_glyph->glyph, j * m, m, pb);
		}
		else
		{
			w = pfd->a_font_glyph->width;
		}

		if (h < pfd->a_font_glyph->height)
			h = pfd->a_font_glyph->height;

		if (b < pfd->a_font_glyph->ascent)
			b = pfd->a_font_glyph->ascent;
	}
	else
	{
		m = 2 + pfd->c_font_glyph->bytesperline * pfd->c_font_glyph->height;
		j = GB2312_GLYPH_INDEX(chs);
		if (j<0 || j >= pfd->c_font_glyph->characters)
		{
			j = pfd->c_font_glyph->defaultchar - pfd->c_font_glyph->firstchar;
		}
		pb = (pfd->c_font_glyph->glyph)? xshare_lock(pfd->c_font_glyph->glyph, j * m, m) : NULL;
		if (pb)
		{
			w = GET_SWORD_LOC(pb, 0);
			xshare_unlock(pfd->c_font_glyph->glyph, j * m, m, pb);
		}
		else
		{
			w = pfd->c_font_glyph->width;
		}

		if (h < pfd->c_font_glyph->height)
			h = pfd->c_font_glyph->height;

		if (b < pfd->c_font_glyph->ascent)
			b = pfd->c_font_glyph->ascent;
	}

	if (pmetr) pmetr->width = w;
	if (pmetr) pmetr->height = h;
	if (pmetr) pmetr->ascent = b;
}

static int get_glyph_pixmap(glyph_t gly, const tchar_t* pch, mem_pixmap_ptr ppixmap)
{
	Fixed_driver_t* pfd = (Fixed_driver_t*)gly;

	int i, w;
	dword_t m;
	byte_t chs[CHS_LEN + 1] = { 0 };
	byte_t* pb;

	XDK_ASSERT(gly && gly->tag == _HANDLE_GLYPH);
	
	XDK_ASSERT(pfd->a_font_glyph != NULL && pfd->c_font_glyph != NULL);

#if defined(_UNICODE) || defined(UNICODE)
	ucs_byte_to_gb2312(*pch, chs);
#else
	utf8_byte_to_gb2312(pch, chs);
#endif

	if (is_ascii(chs[0]))
	{
		m = 2 + pfd->a_font_glyph->bytesperline * pfd->a_font_glyph->height;
		i = chs[0] - pfd->a_font_glyph->firstchar;
		if (i<0 || i >= pfd->a_font_glyph->characters)
		{
			i = pfd->a_font_glyph->defaultchar - pfd->a_font_glyph->firstchar;
		}
		pb = (pfd->a_font_glyph->glyph)? xshare_lock(pfd->a_font_glyph->glyph, i * m, m) : NULL;
		if (pb)
		{
			w = GET_SWORD_LOC(pb, 0);
			if (ppixmap)
			{
				xmem_copy((void*)(ppixmap->data), (void*)(pb + 2), (m - 2));
			}
			xshare_unlock(pfd->a_font_glyph->glyph, i * m, m, pb);
		}
		else
		{
			w = pfd->a_font_glyph->width;
		}
	}
	else
	{
		m = 2 + pfd->c_font_glyph->bytesperline * pfd->c_font_glyph->height;
		i = GB2312_GLYPH_INDEX(chs);
		if (i<0 || i >= pfd->c_font_glyph->characters)
		{
			i = pfd->c_font_glyph->defaultchar - pfd->c_font_glyph->firstchar;
		}
		pb = (pfd->c_font_glyph->glyph)? xshare_lock(pfd->c_font_glyph->glyph, i * m, m) : NULL;
		if (pb)
		{
			w = GET_SWORD_LOC(pb, 0);
			if (ppixmap)
			{
				xmem_copy((void*)(ppixmap->data), (void*)(pb + 2), (m - 2));
			}
			xshare_unlock(pfd->c_font_glyph->glyph, i * m, m, pb);
		}
		else
		{
			w = pfd->c_font_glyph->width;
		}
	}

	return w;
}

/*****************************************************************************************************************/

gly_driver_t glyph_Fixed = {
	GLY_DRIVER_FIXED, /*the memroy font*/

	create_glyph,
	destroy_glyph,
	get_glyph_metrix,
	get_glyph_pixmap
};


