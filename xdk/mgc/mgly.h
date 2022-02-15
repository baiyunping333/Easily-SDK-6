/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc glyph driver document

	@module	mgly.h | interface file

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

#ifndef _MGLY_H
#define _MGLY_H

#include "mdef.h"
#include "mpix.h"
#include "../gly/gly.h"

#define GLY_DRIVER_FIXED		_T("Fixed")

typedef struct _gly_driver_t* gly_driver_ptr;

typedef struct _gly_driver_t{
	tchar_t glyph_name[MAX_GLYPH_NAME];

	glyph_t(*createGlyph)(const xfont_t* pxf);
	void(*destroyGlyph)(glyph_t gly);
	void(*getGlyphMetrix)(glyph_t gly, const tchar_t *pch, glyph_metrix_t* pemtr);
	int(*getGlyphPixmap)(glyph_t gly, const tchar_t* pch, mem_pixmap_ptr ppixmap);
} gly_driver_t;

extern gly_driver_t glyph_Fixed;

#ifdef	__cplusplus
extern "C" {
#endif
	

#ifdef	__cplusplus
}
#endif

#endif /*_MGLY_H*/
