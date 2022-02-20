/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc fit document

	@module	fit.h | interface file

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

#ifndef _FIT_H
#define _FIT_H

#include "../xdkdef.h"

#define PEN_DOT			1
#define PEN_DASH		2
#define PEN_SOLID		0

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API int fit_line(int ps, int dashdot, const xpoint_t* ppt1, const xpoint_t* ppt2, xpoint_t* ppt, int max);

#ifdef	__cplusplus
}
#endif

#endif /*_FIT_H*/
