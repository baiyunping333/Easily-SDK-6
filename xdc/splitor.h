/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc splitor document

	@module	splitor.h | interface file

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

#ifndef _SPLITOR_H
#define _SPLITOR_H

#include "xdcdef.h"



#ifdef	__cplusplus
extern "C" {
#endif

EXP_API bool_t hand_splitor_mouse_move(splitor_t* ptd, dword_t dw, const xpoint_t* pxp);

EXP_API bool_t hand_splitor_lbutton_down(splitor_t* ptd, const xpoint_t* pxp);

EXP_API bool_t hand_splitor_lbutton_up(splitor_t* ptd, const xpoint_t* pxp);

EXP_API void hand_splitor_size(splitor_t* ptd, const xrect_t* pxr);

EXP_API void hand_splitor_paint(splitor_t* ptd, visual_t rdc);

#ifdef	__cplusplus
}
#endif


#endif /*SPLITOR_H*/