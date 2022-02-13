/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc coding document

	@module	coding.h | interface file

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

#ifndef _CODING_H
#define _CODING_H

#include "xdldef.h"

#if defined(XDL_SUPPORT_GDI)

#ifdef	__cplusplus
extern "C" {
#endif

	
/*
@FUNCTION draw_code128: draw CODE128 code in canvas using millimeter coordinate.
@INPUT drawing_interface* pif: the canvas interface.
@INPUT const xcolor_t* pxc: the foreground color struct.
@INPUT const xrect_t* pxr: the rect struct using float member.
@INPUT const tchar_t* text: the text for coding.
@INPUT int len: the length in characters, -1 indicate zero terminated.
@RETURN void: none.
*/
EXP_API void 	draw_code128(const drawing_interface* pif, const xcolor_t* pxc, xrect_t* pxr, const tchar_t* text, int len);

/*
@FUNCTION draw_pdf417: draw PDF417 code in canvas using millimeter coordinate.
@INPUT drawing_interface* pif: the canvas interface.
@INPUT const xcolor_t* pxc: the foreground color struct.
@INPUT const xrect_t* pxr: the rect struct using float member.
@INPUT const tchar_t* text: the text for coding.
@INPUT int len: the length in characters, -1 indicate zero terminated.
@RETURN void: none.
*/
EXP_API void 	draw_pdf417(const drawing_interface* pif, const xcolor_t* pxc, xrect_t* pxr, const tchar_t* text, int len);

/*
@FUNCTION draw_qrcode: draw QR code in canvas using millimeter coordinate.
@INPUT drawing_interface* pif: the canvas interface.
@INPUT const xcolor_t* pxc: the foreground color struct.
@INPUT const xrect_t* pxr: the rect struct using float member.
@INPUT const tchar_t* text: the text for coding.
@INPUT int len: the length in characters, -1 indicate zero terminated.
@RETURN void: none.
*/
EXP_API void 	draw_qrcode(const drawing_interface* pif, const xcolor_t* pxc, xrect_t* pxr, const tchar_t* text, int len);



#ifdef	__cplusplus
}
#endif

#endif

#endif /*SHAPING_H*/