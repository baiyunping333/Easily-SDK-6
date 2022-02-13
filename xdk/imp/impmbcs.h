/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk mbcs document

	@module	impmbcs.h | interface file

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

#ifndef _IMPMBCS_H
#define _IMPMBCS_H

#include "../xdkdef.h"

#ifdef XDK_SUPPORT_MBCS

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API int gbk_code_sequence(byte_t b);

	EXP_API int gbk_byte_to_ucs(const byte_t* src, wchar_t* dest);

	EXP_API int gbk_to_ucs(const byte_t* src, dword_t slen, wchar_t* dest, int dlen);

	EXP_API int ucs_byte_to_gbk(wchar_t ch, byte_t* dest);

	EXP_API int ucs_to_gbk(const wchar_t* src, int slen, byte_t* dest, dword_t dlen);

	EXP_API int utf_code_sequence(byte_t b);

	EXP_API int utf_byte_to_ucs(const byte_t* src, wchar_t* dest);

	EXP_API int utf_to_ucs(const byte_t* src, dword_t slen, wchar_t* dest, int dlen);

	EXP_API int ucs_byte_to_utf(wchar_t ch, byte_t* dest);

	EXP_API int ucs_to_utf(const wchar_t* src, int slen, byte_t* dest, dword_t dlen);


#ifdef	__cplusplus
}
#endif

#endif /*XDK_SUPPORT_MBCS*/

#endif /*IMPMBCS_H*/
