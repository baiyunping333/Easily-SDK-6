/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk utility document

	@module	others.h | interface file

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

#ifndef _OTHERS_H
#define _OTHERS_H

#include "../xdkdef.h"

//define file proto type
#define _PROTO_UNKNOWN		0x00
#define _PROTO_LOC			0x01
#define _PROTO_NFS			0x02
#define _PROTO_HTTP			0x04
#define _PROTO_SSH			0x06
#define _PROTO_TFTP			0x08
#define _PROTO_COAP			0x09

#define IS_INET_FILE(n)	(n == _PROTO_HTTP || n == _PROTO_SSH || n == _PROTO_TFTP || n == _PROTO_COAP)

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void bytes_turn(byte_t* ba, int n);

EXP_API int format_password(const tchar_t* sz, tchar_t* buf, int max);

EXP_API int peek_word(const tchar_t* str, tchar_t* pch);

EXP_API int words_count(const tchar_t* str, int len);

EXP_API void split_path(const tchar_t* pathfile, tchar_t* path, tchar_t* file, tchar_t* ext);

EXP_API void split_file(const tchar_t* pathfile, tchar_t* path, tchar_t* file);

EXP_API bool_t is_ip(const tchar_t* addr);

EXP_API void parse_bytes_range(tchar_t* sz_range, dword_t* phoff, dword_t* ploff, dword_t* psize, long long* ptotal);

EXP_API void format_bytes_range(tchar_t* sz_range, dword_t hoff, dword_t loff, dword_t size, long long total);

EXP_API byte_t parse_proto(const tchar_t* file);

EXP_API void parse_url(const tchar_t* url, tchar_t** proat, int* prolen, tchar_t** addrat, int* addrlen, tchar_t** portat, int* portlen, tchar_t** objat, int* objlen, tchar_t** qryat, int* qrylen);


#ifdef	__cplusplus
}
#endif

#endif