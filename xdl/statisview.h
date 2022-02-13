/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc statis document

	@module	statisview.h | interface file

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

#ifndef _STATISVIEW_H
#define _STATISVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	STATIS_HINT_NONE,
	STATIS_HINT_MENU,
	STATIS_HINT_TITLE,
	STATIS_HINT_NULBAR,
	STATIS_HINT_GAXBAR,
	STATIS_HINT_YAXBAR,
	STATIS_HINT_XAXBAR,
	STATIS_HINT_COOR,
	STATIS_HINT_VERT_SPLIT,
	STATIS_HINT_HORZ_SPLIT,
	STATIS_HINT_MIDD_SPLIT,
}STATIS_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API float calc_statis_page_width(link_t_ptr ptr, int page);

	EXP_API int calc_statis_pages(link_t_ptr ptr);

	EXP_API void calc_statis_xax_scope(link_t_ptr ptr, int page, link_t_ptr* firstxax, link_t_ptr* lastxax);

	EXP_API int calc_statis_xax_page(link_t_ptr ptr, link_t_ptr xlk);

	EXP_API void calc_statis_gax_rect(link_t_ptr ptr, link_t_ptr ylk, xrect_t* pxr);

	EXP_API void calc_statis_yax_rect(link_t_ptr ptr, link_t_ptr glk, xrect_t* pxr);

	EXP_API int calc_statis_coor_rect(link_t_ptr ptr, int page, link_t_ptr xlk, link_t_ptr ylk, xrect_t* pxr);

	EXP_API void calc_statis_xax_rect(link_t_ptr ptr, int page, link_t_ptr xlk, xrect_t* pxr);

	EXP_API int calc_statis_hint(const xpoint_t* ppt, link_t_ptr ptr, int page, link_t_ptr* pxlk, link_t_ptr* pylk, link_t_ptr* pglk);

	EXP_API void draw_statis_page(const drawing_interface* pcanv, link_t_ptr ptr, int page);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*STATISVIEW_H*/