/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory font document

	@module	mfnt_fixed.c | implement file

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
#include "mgly.h"
#include "mpix.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"
#include "../xdkmgc.h"

typedef struct _Fixed_font_t{
	handle_head head;

	gly_driver_ptr driver;
	glyph_t handle;

}Fixed_font_t;

static const gly_driver_ptr select_driver(const tchar_t* fntPattern)
{
	return &glyph_Fixed;
}

static font_t create_font(const xfont_t* pxf)
{
	Fixed_font_t* pfnt;
	tchar_t font_pattern[MAX_FONT_NAME + 1] = { 0 };

	TRY_CATCH;

	pfnt = (Fixed_font_t*)xmem_alloc(sizeof(Fixed_font_t));
	pfnt->head.tag = _HANDLE_FONT;

	format_glyph_pattern(pxf, font_pattern, MAX_FONT_NAME);
	
	pfnt->driver = select_driver(font_pattern);
	if (!pfnt->driver)
	{
		raise_user_error(_T("open_font"), _T("create_font"));
	}

	pfnt->handle = (*(pfnt->driver->createGlyph))(pxf);
	if (!pfnt->handle)
	{
		raise_user_error(_T("open_font"), _T("create_font"));
	}
	
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

	(*(pfnt->driver->destroyGlyph))(pfnt->handle);

	xmem_free(pfnt);
}

static void get_font_info(font_t fnt, xfont_t* pxf)
{
	Fixed_font_t* pfnt = (Fixed_font_t*)fnt;

	XDK_ASSERT(fnt && fnt->tag == _HANDLE_FONT);

	
}

static void get_font_metrix(font_t fnt, const tchar_t* pch, font_metrix_t* pmetrix)
{
	Fixed_font_t* pfnt = (Fixed_font_t*)fnt;
	glyph_metrix_t gm = { 0 };
	tchar_t ch = 0;

	XDK_ASSERT(fnt && fnt->tag == _HANDLE_FONT);

	(*pfnt->driver->getGlyphMetrix)(pfnt->handle, ((pch) ? pch : &ch), &gm);

	pmetrix->width = gm.width;
	pmetrix->height = gm.height;
}

static void get_char_size(font_t fnt, const tchar_t *pch, xsize_t* pse)
{
	Fixed_font_t* pfnt = (Fixed_font_t*)fnt;
	glyph_metrix_t gm = { 0 };

	XDK_ASSERT(fnt && fnt->tag == _HANDLE_FONT);

	(*pfnt->driver->getGlyphMetrix)(pfnt->handle, pch, &gm);

	if (pse)
	{
		pse->w = gm.width;
		pse->h = gm.height;
	}
}

static int get_char_pixmap(font_t fnt, const tchar_t* pch, mem_pixmap_ptr ppixmap)
{
	Fixed_font_t* pfnt = (Fixed_font_t*)fnt;

	XDK_ASSERT(fnt && fnt->tag == _HANDLE_FONT);
	XDK_ASSERT(pch != NULL);

	return (*pfnt->driver->getGlyphPixmap)(pfnt->handle, pch, ppixmap);
}

/*****************************************************************************************************************/

mem_font_t font_Fixed = {
	MGC_FONT_FIXED, /*the memroy font*/

	create_font,
	destroy_font,
	get_font_info,
	get_font_metrix,
	get_char_size,
	get_char_pixmap,
};

