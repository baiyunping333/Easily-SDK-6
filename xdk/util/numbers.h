/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk utility document

	@module	numbers.h | interface file

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

#ifndef _NUMBERS_H
#define _NUMBERS_H

#include "../xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API bool_t is_zero_size(const tchar_t* fsize);

EXP_API bool_t is_huge_size(const tchar_t* fsize);

EXP_API unsigned int parse_hexnum(const tchar_t* token, int len);

EXP_API int format_hexnum(unsigned int n, tchar_t* buf, int max);

EXP_API int fill_integer(int ln, tchar_t* buf, int max);

EXP_API int format_integer_ex(int n, const tchar_t* fmt, tchar_t* buf, int max);

EXP_API int parse_intset(const tchar_t* str, int len, int* sa, int max);

EXP_API bool_t is_zero_numeric(double dbl, int scale);

EXP_API double parse_numeric(const tchar_t* token, int len);

EXP_API int format_numeric(double dbl, const tchar_t* fmt, tchar_t* buf, int max);

EXP_API int format_ages(const xdate_t* bday, const xdate_t* tday, tchar_t* buf);

EXP_API int format_long(unsigned int hl, unsigned int ll, tchar_t* buf);

EXP_API void parse_long(unsigned int* phl, unsigned int* pll, const tchar_t* str);

EXP_API int mul_div_int(int m1, int m2, int d);

EXP_API short mul_div_short(short m1, short m2, short d);

#ifdef	__cplusplus
}
#endif

#endif