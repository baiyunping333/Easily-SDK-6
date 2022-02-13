/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc base64 document

	@module	base64.h | interface file

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
#ifndef _BASE64_H
#define _BASE64_H

#include "xdkdef.h"

#define BASE64_LENGTH(len)		((len % 3)? (len / 3 + 1) * 4 : len / 3 * 4)

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API dword_t a_xbas_decode(const schar_t* src, int slen, byte_t* dest, dword_t dlen);

EXP_API dword_t w_xbas_decode(const wchar_t* src, int slen, byte_t* dest, dword_t dlen);

EXP_API int a_xbas_encode(const byte_t* src, dword_t slen, schar_t* dest, int dlen);

EXP_API int w_xbas_encode(const byte_t* src, dword_t slen, wchar_t* dest, int dlen);

#if defined(UNICODE) || defined(_UNICODE)
#define xbas_decode		w_xbas_decode
#define xbas_encode		w_xbas_encode
#else
#define xbas_decode		a_xbas_decode
#define xbas_encode		a_xbas_encode
#endif

#ifdef	__cplusplus
}
#endif


#endif /*OEMBAS_H*/
