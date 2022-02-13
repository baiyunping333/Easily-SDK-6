/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc label document

	@module	labelview.h | interface file

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

#ifndef _LABELVIEW_H
#define _LABELVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	LABEL_HINT_NONE,
	LABEL_HINT_ITEM,
}LABEL_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API int calc_label_pages(link_t_ptr ptr);

EXP_API void calc_label_item_scope(link_t_ptr ptr, int page, link_t_ptr* firstitem, link_t_ptr* lastitem);

EXP_API int calc_label_item_page(link_t_ptr ptr, link_t_ptr ilk);

EXP_API bool_t calc_label_item_rect(link_t_ptr ptr, int page, link_t_ptr ilk, xrect_t* pxr);

EXP_API int	calc_label_hint(const xpoint_t* ppt, link_t_ptr ptr, int page, link_t_ptr* pilk);

EXP_API void draw_label(const drawing_interface* pcanv, link_t_ptr ptr, int page);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*LABELVIEW_H*/