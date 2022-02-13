/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc form document

	@module	formview.h | interface file

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

#ifndef _FORMVIEW_H
#define _FORMVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	FORM_HINT_NONE,
	FORM_HINT_FIELD,
	FORM_HINT_GROUP,
	FORM_HINT_VERT_SPLIT,
	FORM_HINT_HORZ_SPLIT,
	FORM_HINT_CROSS_SPLIT,
}FORM_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API int calc_form_pages(const drawing_interface* pif, link_t_ptr form);;

	EXP_API void calc_form_field_rect(link_t_ptr ptr, link_t_ptr flk, xrect_t* pxr);

	EXP_API void calc_form_group_rect(link_t_ptr ptr, link_t_ptr alk, xrect_t* pxr);

	EXP_API int calc_form_hint(const xpoint_t* ppt, link_t_ptr ptr, link_t_ptr* pflk);

	EXP_API void draw_form_page(const drawing_interface* pcanv, link_t_ptr ptr, int page);

#ifdef	__cplusplus
}
#endif

#endif //XDLVIEW

#endif
