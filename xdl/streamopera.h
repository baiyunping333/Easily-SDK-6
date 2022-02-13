/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc stream operator document

	@module	streamopera.h | interface file

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

#ifndef _STREAMOPERA_H
#define _STREAMOPERA_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_DOC

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API bool_t call_stream_can_escape(void* p_obj);

	EXP_API bool_t call_stream_with_eof(void* p_obj);

	EXP_API int call_stream_read_escape(void* p_obj, int max, int pos, int encode, tchar_t* pch);

	EXP_API int call_stream_write_escape(void* p_obj, int max, int pos, int encode, tchar_t ch);

	EXP_API int call_stream_read_char(void* p_obj, int max, int pos, int encode, tchar_t* pch);

	EXP_API int call_stream_write_char(void* p_obj, int max, int pos, int encode, const tchar_t* pch);

	EXP_API int call_stream_read_token(void* p_obj, int max, int pos, int encode, tchar_t* pch, int len);

	EXP_API int call_stream_write_token(void* p_obj, int max, int pos, int encode, const tchar_t* pch, int len);

	EXP_API void call_stream_set_encode(void* p_obj, int encode);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_DOC*/

#endif /*_STREAMOPERA_H*/
