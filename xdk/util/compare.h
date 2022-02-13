/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk utility document

	@module	compare.h | interface file

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

#ifndef _COMPARE_H
#define _COMPARE_H

#include "../xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API int compare_date(const xdate_t* pmd1, const xdate_t* pmd2);

EXP_API int compare_datetime(const xdate_t* pmd1, const xdate_t* pmd2);

EXP_API int compare_time(const xdate_t* pmd1, const xdate_t* pmd2);

EXP_API int compare_numeric(const tchar_t* szSrc, const tchar_t* szDes, int digi);

EXP_API int compare_float(float f1, float f2, int prec);

EXP_API int compare_double(double f1, double f2, int prec);

EXP_API int compare_text(const tchar_t* src, int srclen, const tchar_t* dest, int destlen, int nocase);


#ifdef	__cplusplus
}
#endif

#endif