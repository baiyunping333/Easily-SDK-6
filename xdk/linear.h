/***********************************************************************
	Easily SDK 6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc linear buffer document

	@module	linear.h | interface file

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

#ifndef _LINEAR_H
#define _LINEAR_H

#include "xdkdef.h"


#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API linear_t alloc_linear(int wins);

	EXP_API void clear_linear(linear_t lin);

	EXP_API void free_linear(linear_t lin);

	EXP_API byte_t* insert_linear_frame(linear_t lin, int seqnum, dword_t frmlen);

	EXP_API bool_t delete_linear_frame(linear_t lin, int seqnum);

	EXP_API void clean_linear_frame(linear_t lin, int seqnum);

	EXP_API byte_t* get_linear_frame(linear_t lin, int seqnum, dword_t* pb);

	EXP_API int get_linear_window(linear_t lin);

	EXP_API int get_linear_top(linear_t lin);

#if defined(XDK_SUPPORT_TEST)
	EXP_API void test_linear();
#endif

#ifdef	__cplusplus
}
#endif

#endif /*LINEAR_H*/
