/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc prim document

	@module	prim.h | interface file

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
#ifndef _PRIM_H
#define _PRIM_H

#include "xdkdef.h"

#define MIN_PRIM		2
#define MAX_PRIM		2147483647

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API dword_t next_prim(dword_t prim);

#if defined(_DEBUG) || defined(DEBUG)
	EXP_API void test_prim(void);
#endif

#ifdef	__cplusplus
}
#endif

#endif /*OEMPRIM_H*/