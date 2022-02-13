/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc grid view document

	@module	gridview.h | interface file

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

#ifndef _GRIDVIEW_H
#define _GRIDVIEW_H

#include "xdldef.h"
#include "hintner.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	GRID_HINT_NONE,
	GRID_HINT_MENU,
	GRID_HINT_TITLE,
	GRID_HINT_NULBAR,
	GRID_HINT_COLBAR,
	GRID_HINT_ROWBAR,
	GRID_HINT_CELL,
	GRID_HINT_VERT_SPLIT,
	GRID_HINT_HORZ_SPLIT,
}GRID_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void hint_grid_item(link_t_ptr ptr, int page, PF_HINT_DESIGNER_CALLBACK pf, void* pp);

EXP_API float calc_grid_page_width(link_t_ptr ptr);

EXP_API float calc_grid_page_height(link_t_ptr ptr, int page);

EXP_API int calc_grid_row_scope(link_t_ptr ptr, int page, link_t_ptr* pfirst, link_t_ptr* plast);

EXP_API int calc_grid_row_page(link_t_ptr ptr, link_t_ptr rlk);

EXP_API int calc_grid_pages(link_t_ptr ptr);

EXP_API int calc_grid_cell_rect(link_t_ptr ptr, int page, link_t_ptr rlk, link_t_ptr clk, xrect_t* pxr);

EXP_API int calc_grid_row_rect(link_t_ptr ptr, int page, link_t_ptr rlk, xrect_t* pxr);

EXP_API int calc_grid_col_rect(link_t_ptr ptr, int page, link_t_ptr rlk, link_t_ptr clk, xrect_t* pxr);

EXP_API int calc_grid_hint(const xpoint_t* ppt, link_t_ptr ptr, int page, link_t_ptr* prlk, link_t_ptr* pclk);

EXP_API void draw_grid_page(const drawing_interface* pcanv, link_t_ptr ptr, int page);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*GRIDVIEW_H*/