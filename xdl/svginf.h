/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc svg interface document

	@module	svginf.h | interface file

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

#ifndef _SVGINF_H
#define _SVGINF_H

#include "xdldef.h"

#if defined(XDL_SUPPORT_GDI)

#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION create_svg_interface: create svg canvas interface.
@INPUT canvas_t canv: the svg canvas object.
@RETURN drawing_interface*: if succeeds return svg canvas interface struct, fails return NULL.
*/
EXP_API void svg_get_canvas_interface(canvas_t canv, drawing_interface* pif);

/*
@FUNCTION create_visual_interface: create svg view interface.
@INPUT visual_t view: the context object.
@RETURN if_viewING_t*: if succeeds return view interface struct, fails return NULL.
*/
EXP_API void svg_get_visual_interface(visual_t visu, drawing_interface* pif);


#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*SVGINF_H*/