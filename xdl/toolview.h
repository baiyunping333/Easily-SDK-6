/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc tool view

	@module	toolview.h | interface file

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

#ifndef _TOOLVIEW_H
#define _TOOLVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	TOOL_HINT_NONE,
	TOOL_HINT_GROUP,
	TOOL_HINT_TITLE,
	TOOL_HINT_ITEM,
}TOOL_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API float calc_tool_group_width(link_t_ptr ptr, link_t_ptr glk);

EXP_API float calc_tool_group_height(link_t_ptr ptr, link_t_ptr glk);

EXP_API float calc_tool_width(link_t_ptr ptr);

EXP_API void calc_tool_group_entire_rect(link_t_ptr ptr, link_t_ptr glk, xrect_t* pxr);

EXP_API void calc_tool_group_title_rect(link_t_ptr ptr, link_t_ptr glk, xrect_t* pxr);

EXP_API void calc_tool_group_item_rect(link_t_ptr ptr, link_t_ptr ilk, xrect_t* pxr);

EXP_API int calc_tool_point_hint(const xpoint_t* ppt, link_t_ptr ptr, link_t_ptr* pglk, link_t_ptr* pplk);

EXP_API void draw_tool(const drawing_interface* pcanv, link_t_ptr ptr);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*TOOLVIEW_H*/