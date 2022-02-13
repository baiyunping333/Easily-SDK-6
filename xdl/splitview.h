/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc split view

	@module	splitview.h | interface file

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

#ifndef _SPLITVIEW_H
#define _SPLITVIEW_H

#include "xdldef.h"


#ifdef XDL_SUPPORT_VIEW

typedef enum{
	SPLIT_HINT_NONE,
	SPLIT_HINT_BAR,
}SPLIT_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void calc_split_item_rect(link_t_ptr ptr, link_t_ptr cur, xrect_t* pxr);

EXP_API void calc_split_span_rect(link_t_ptr ptr, link_t_ptr cur, xrect_t* pxr);

EXP_API void resize_split_item(link_t_ptr ilk);

EXP_API void adjust_split_item(link_t_ptr ilk, float off);

EXP_API int calc_split_hint(link_t_ptr ptr, const xpoint_t* ppt, link_t_ptr* pilk);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*_SPLITVIEW_H*/
