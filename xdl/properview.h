/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc proper view

	@module	properview.h | interface file

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

#ifndef _PROPERVIEW_H
#define _PROPERVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	PROPER_HINT_NONE,
	PROPER_HINT_SECTION,
	PROPER_HINT_ENTITY,
	PROPER_HINT_HORZ_SPLIT,
	PROPER_HINT_VERT_SPLIT,
}PROPER_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API float calc_proper_height(link_t_ptr ptr);

	EXP_API float calc_proper_width(link_t_ptr ptr);

	EXP_API void calc_proper_section_rect(link_t_ptr ptr, link_t_ptr sec, xrect_t* pxr);

	EXP_API void calc_proper_entity_rect(link_t_ptr ptr, link_t_ptr ent, xrect_t* pxr);

	EXP_API void calc_proper_entity_text_rect(link_t_ptr ptr, link_t_ptr ent, xrect_t* pxr);

	EXP_API int calc_proper_hint(const xpoint_t* ppt, link_t_ptr ptr, link_t_ptr* psec, link_t_ptr* pent);

	EXP_API void draw_proper(const drawing_interface* pcanv, link_t_ptr ptr);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*PROPERVIEW_H*/