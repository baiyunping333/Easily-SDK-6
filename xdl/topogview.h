/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc topog document

	@module	topogview.h | interface file

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

#ifndef _TOPOGVIEW_H
#define _TOPOGVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	TOPOG_HINT_NONE,
	TOPOG_HINT_SPOT
}TOPOG_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void calc_topog_spot_rect(link_t_ptr ptr, link_t_ptr ilk, xrect_t* pxr);

EXP_API int calc_topog_hint(const xpoint_t* ppt, link_t_ptr ptr, link_t_ptr* pilk, int* prow, int* pcol);

EXP_API void draw_topog(const drawing_interface* pcanv, link_t_ptr ptr);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*TOPOGVIEW_H*/
