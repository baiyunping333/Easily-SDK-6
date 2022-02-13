/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc tree view

	@module	treeview.h | interface file

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

#ifndef _TREEVIEW_H
#define _TREEVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	TREE_HINT_NONE,
	TREE_HINT_EXPAND,
	TREE_HINT_CHECK,
	TREE_HINT_ITEM,
	TREE_HINT_TITLE,
}TREE_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API float calc_tree_height(link_t_ptr ptr);

	EXP_API float calc_tree_width(const measure_interface* pif, link_t_ptr ptr);

	EXP_API bool_t calc_tree_item_rect(link_t_ptr ptr, link_t_ptr cur, xrect_t* pxr);

	EXP_API bool_t calc_tree_item_text_rect(link_t_ptr ptr, link_t_ptr cur, xrect_t* pxr);

	EXP_API bool_t calc_tree_item_entity_rect(link_t_ptr ptr, link_t_ptr cur, xrect_t* pxr);

	EXP_API bool_t calc_tree_item_expand_rect(link_t_ptr ptr, link_t_ptr cur, xrect_t* pxr);

	EXP_API int calc_tree_hint(const xpoint_t* ppt, link_t_ptr ptr, link_t_ptr* pilk);

	EXP_API void draw_tree(const drawing_interface* pcanv, link_t_ptr ptr);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*_TREEVIEW_H*/
