/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc sorting document

	@module	sort.h | interface file

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

#ifndef _SORT_H
#define _SORT_H

#include "xdkdef.h"

typedef struct _xsort_t{
	int fact;
	void* data;
}xsort_t;

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void bubble_xsort(xsort_t* pxs, int count);

#ifdef	__cplusplus
}
#endif

#endif /*SORTING_H*/