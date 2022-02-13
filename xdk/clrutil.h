/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xds color document

	@module	xdscolor.h | interface file

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

#ifndef _CLRUTIL_H
#define _CLRUTIL_H

#include "xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API bool_t find_color(const tchar_t* en_clr, tchar_t* cn_clr, tchar_t* rgb_clr, tchar_t* bin_clr);

	EXP_API bool_t next_color(const tchar_t* en, tchar_t* en_clr, tchar_t* cn_clr, tchar_t* rgb_clr, tchar_t* bin_clr);

#ifdef	__cplusplus
}
#endif


#endif