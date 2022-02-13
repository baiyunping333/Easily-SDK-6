/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdc print document

	@module	printbag.h | interface file

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

#ifndef _PRINTBAG_H
#define _PRINTBAG_H

#include "xdcdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

#if defined(XDU_SUPPORT_CONTEXT_PRINTER)

EXP_API void print_form(const dev_prn_t* pdev, link_t_ptr form);

EXP_API void print_grid(const dev_prn_t* pdev, link_t_ptr grid);

EXP_API void print_statis(const dev_prn_t* pdev, link_t_ptr statis);

EXP_API void print_topog(const dev_prn_t* pdev, link_t_ptr topog);

EXP_API void print_dialog(const dev_prn_t* pdev, link_t_ptr dialog);

EXP_API void print_diagram(const dev_prn_t* pdev, link_t_ptr diagram);

EXP_API void print_memo(const dev_prn_t* pdev, const xfont_t* pxf, const xface_t* pxa, link_t_ptr memo);

EXP_API void print_rich(const dev_prn_t* pdev, const xfont_t* pxf, const xface_t* pxa, link_t_ptr rich);

EXP_API void print_svg(const dev_prn_t* pdev, link_t_ptr svg);



#endif /*XDU_SUPPORT_CONTEXT_PRINTER*/

#ifdef	__cplusplus
}
#endif

#endif /*PRINTBAG_H*/