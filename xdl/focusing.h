/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdl focus document

	@module	focusing.h | interface file

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

#ifndef _FOCUSING_H
#define _FOCUSING_H

#include "xdldef.h"

#ifdef	__cplusplus
extern "C" {
#endif

#if defined(XDL_SUPPORT_GDI)


/*
@FUNCTION draw_select_raw: draw select frame in memory or device context using points coordinate.
@INPUT drawing_interface* piv: the context interface.
@INPUT const xcolor_t* pxc: the color struct.
@INPUT const xrect_t* pxr: the rect struct using integer member.
@INPUT int deep: the alphablend level: 0~255, the predefined value is ALPHA_SOLID, ALPHA_SOFT, ALPHA_TRANS.
@RETURN void: none.
*/
EXP_API void	draw_select_raw(const drawing_interface* piv, const xcolor_t* pxc, const xrect_t* prt, int deep);

/*
@FUNCTION draw_focus_raw: draw focus frame in memory or device context using points coordinate.
@INPUT drawing_interface* piv: the context interface.
@INPUT const xcolor_t* pxc: the color struct.
@INPUT const xrect_t* pxr: the rect struct using integer member.
@INPUT int deep: the alphablend level: 0~255, the predefined value is ALPHA_SOLID, ALPHA_SOFT, ALPHA_TRANS.
@RETURN void: none.
*/
EXP_API void	draw_focus_raw(const drawing_interface* piv, const xcolor_t* pxc, const xrect_t* prt, int deep);

/*
@FUNCTION draw_feed_raw: draw feed frame in memory or device context using points coordinate.
@INPUT drawing_interface* piv: the context interface.
@INPUT const xcolor_t* pxc: the color struct.
@INPUT const xrect_t* pxr: the rect struct using integer member.
@INPUT int deep: the alphablend level: 0~255, the predefined value is ALPHA_SOLID, ALPHA_SOFT, ALPHA_TRANS.
@RETURN void: none.
*/
EXP_API void	draw_feed_raw(const drawing_interface* piv, const xcolor_t* pxc, const xrect_t* prt, int deep);

/*
@FUNCTION draw_sizing_raw: draw feed frame in memory or device context using points coordinate.
@INPUT drawing_interface* piv: the context interface.
@INPUT const xcolor_t* pxc: the color struct.
@INPUT const xrect_t* pxr: the rect struct using integer member.
@INPUT int deep: the alphablend level: 0~255, the predefined value is ALPHA_SOLID, ALPHA_SOFT, ALPHA_TRANS.
@RETURN void: none.
*/
EXP_API void	draw_sizing_raw(const drawing_interface* piv, const xcolor_t* pxc, const xrect_t* prt, int deep, dword_t pos);



#endif /*XDL_SUPPORT_VIEW*/

#ifdef	__cplusplus
}
#endif

#endif /*PRINTBAG_H*/