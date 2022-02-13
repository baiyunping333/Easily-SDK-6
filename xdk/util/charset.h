/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk utility document

	@module	charset.h | interface file

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

#ifndef _CHARSET_H
#define _CHARSET_H

#include "../xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void bytes_turn(byte_t* ba, int n);

EXP_API int parse_encode(const tchar_t* enstr);

EXP_API void format_encode(int encode, tchar_t* buf);

EXP_API int parse_charset(const tchar_t* enstr);

EXP_API void format_charset(int encode, tchar_t* buf);

EXP_API int parse_utfbom(const byte_t* buf, int len);

EXP_API int format_utfbom(int encode, byte_t* buf);

EXP_API int skip_utfbom(const byte_t* buf);


#ifdef	__cplusplus
}
#endif

#endif