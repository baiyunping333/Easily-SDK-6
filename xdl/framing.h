/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdl frame document

	@module	framing.h | interface file

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

#ifndef _FRAMING_H
#define _FRAMING_H

#include "xdldef.h"

#ifdef	__cplusplus
extern "C" {
#endif

#if defined(XDL_SUPPORT_GDI)

/*
@FUNCTION draw_progress: draw the progress in canvas using millimeter coordinate.
@INPUT drawing_interface* pif: the canvas interface.
@INPUT cont xcolor_t* pxc: the color struct.
@INPUT const xrect_t* prt: the rect struct using float member.
@INPUT int steps: the steps of progress.
@RETURN void: none.
*/
EXP_API void	draw_progress(const drawing_interface* pif, const xcolor_t* pxc, const xrect_t* prt, int steps);

/*
@FUNCTION draw_ruler: draw the ruler in canvas using millimeter coordinate.
@INPUT drawing_interface* pif: the canvas interface.
@INPUT cont xcolor_t* pxc: the color struct.
@INPUT const xrect_t* prt: the rect struct using float member.
@RETURN void: none.
*/
EXP_API void	draw_ruler(const drawing_interface* pif, const xcolor_t* pxc, const xrect_t* prt);

/*
@FUNCTION draw_corner: draw the conner in canvas using millimeter coordinate.
@INPUT drawing_interface* pif: the canvas interface.
@INPUT cont xcolor_t* pxc: the color struct.
@INPUT const xrect_t* prt: the rect struct using float member.
@RETURN void: none.
*/
EXP_API void	draw_corner(const drawing_interface* pif, const xcolor_t* pxc, const xrect_t* prt);



#endif /*XDL_SUPPORT_VIEW*/

#ifdef	__cplusplus
}
#endif

#endif /*PRINTBAG_H*/