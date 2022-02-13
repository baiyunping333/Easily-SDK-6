/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc table view

	@module	tableview.h | interface file

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

#ifndef _TABLEVIEW_H
#define _TABLEVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	TABLE_HINT_NONE,
	TABLE_HINT_KEY,
	TABLE_HINT_VAL,
	TABLE_HINT_SPLIT,
}TABLE_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API float calc_table_height(const measure_interface* pif, const xfont_t* pxf, const xface_t* pxa, link_t_ptr ptr);

	EXP_API float calc_table_width(const measure_interface* pif, const xfont_t* pxf, const xface_t* pxa,  link_t_ptr ptr);

	EXP_API void calc_table_item_rect(const measure_interface* pif, const xfont_t* pxf, const xface_t* pxa, link_t_ptr ptr, link_t_ptr plk, xrect_t* pxr);

	EXP_API void calc_table_item_key_rect(const measure_interface* pif, const xfont_t* pxf, const xface_t* pxa, link_t_ptr ptr, float ratio, link_t_ptr plk, xrect_t* pxr);

	EXP_API void calc_table_item_val_rect(const measure_interface* pif, const xfont_t* pxf, const xface_t* pxa, link_t_ptr ptr, float ratio, link_t_ptr plk, xrect_t* pxr);

	EXP_API int	calc_table_hint(const measure_interface* pif, const xfont_t* pxf, const xface_t* pxa, const xpoint_t* ppt, link_t_ptr ptr, float ratio, link_t_ptr* pilk);

	EXP_API void draw_table(const drawing_interface* pif, const xfont_t* pxf, const xface_t* pxa, const xpen_t* pxp, const xbrush_t* pxb, link_t_ptr ptr, float ratio);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*TABLEVIEW_H*/