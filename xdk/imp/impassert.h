/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk assert document

	@module	impassert.h | interface file

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

#ifndef _IMPASSERT_H
#define _IMPASSERT_H

#include "../xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void xdk_assert(const char* _Expr, const char* _File, const char* _Func, unsigned int _Line);

#if defined(_DEBUG) || defined(DEBUG)
#define XDK_ASSERT(token) (void)( (!!(token)) || (xdk_assert(#token, __FILE__, __FUNCTION__, __LINE__), 0) )
#else
#define XDK_ASSERT(token) (void)( (!!(token)) || (xdk_assert(NULL, NULL, NULL, 0), 0) )
#endif

#ifdef	__cplusplus
}
#endif

#endif /*IMPASSERT_H*/

