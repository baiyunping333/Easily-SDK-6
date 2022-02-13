﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory graphic context document

	@module	mgc.h | interface file

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

#ifndef _MEMGC_H
#define _MEMGC_H

#include "mdef.h"
#include "mdrv.h"


#ifdef	__cplusplus
extern "C" {
#endif

EXP_API visual_t mgc_create(const tchar_t* devName, const tchar_t* formName, int width, int height, int dpi);

EXP_API void mgc_destroy(visual_t gc);

EXP_API void mgc_get_point(visual_t gc, xcolor_t* pxc, const xpoint_t* ppt);

EXP_API void mgc_set_point(visual_t gc, const xcolor_t* pxc, const xpoint_t* ppt, int rop);

EXP_API bool_t mgc_text_size(visual_t gc, const xfont_t* pxf, const tchar_t* str, int len, xsize_t* pxs);

EXP_API bool_t mgc_text_out(visual_t gc, const xfont_t* pxf, const xpoint_t* ppt, int rop, const tchar_t* str, int len);

EXP_API dword_t mgc_save_bytes(visual_t gc, byte_t* buf, dword_t max);

#ifdef	__cplusplus
}
#endif

#endif /*_MDEV_H*/
