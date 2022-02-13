/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdl export document

	@module	svgbag.h | interface file

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

#ifndef _SVGBAG_H
#define _SVGBAG_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_GDI

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void svg_print_form(link_t_ptr svg, link_t_ptr form, int page);

EXP_API void svg_print_grid(link_t_ptr svg, link_t_ptr grid, int page);

EXP_API void svg_print_statis(link_t_ptr svg, link_t_ptr statis, int page);

EXP_API void svg_print_topog(link_t_ptr svg, link_t_ptr topog);

EXP_API void svg_print_dialog(link_t_ptr svg, link_t_ptr dialog);

EXP_API void svg_print_diagram(link_t_ptr svg, link_t_ptr diagram);

EXP_API void svg_print_plot(link_t_ptr svg, link_t_ptr plot);

EXP_API void svg_print_memo(link_t_ptr svg, const xfont_t* pxf, const xface_t* pxa, link_t_ptr memo, int page);

EXP_API void svg_print_rich(link_t_ptr svg, const xfont_t* pxf, const xface_t* pxa, link_t_ptr rich, int page);


#ifdef	__cplusplus
}
#endif

#endif

#endif /*SVGBAG_H*/