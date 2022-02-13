/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk error document

	@module	imperr.h | interface file

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

#ifndef _IMPERROR_H
#define _IMPERROR_H

#include "../xdkdef.h"
#include "../loginf.h"

#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION set_system_error: set system error.
@INPUT const tchar_t* errcode: the error code.
@RETURN void: none.
*/
EXP_API void set_system_error(const tchar_t* errcode);

/*
@FUNCTION set_last_error: set last error.
@INPUT const tchar_t* errcode: the error code.
@INPUT const tchar_t* errtext: the error text.
@INPUT int len: the error text length in characters, not include terminate character.
@RETURN void: none.
*/
EXP_API void set_last_error(const tchar_t* errcode, const tchar_t* errtext, int len);

/*
@FUNCTION get_last_error: get last error.
@INPUT const tchar_t* errcode: the error code.
@INPUT const tchar_t* errtext: the error text.
@INPUT int len: the error text length in characters, not include terminate character.
@RETURN void: none.
*/
EXP_API void get_last_error(tchar_t* code, tchar_t* text, int max);

/*
@FUNCTION xdk_trace: set and trace error.
@INPUT const tchar_t* code: the error code.
@INPUT const tchar_t* info: the error text.
@RETURN void: none.
*/
EXP_API void xdk_trace(const tchar_t* code, const tchar_t* info);

EXP_API void xdk_set_track(PF_TRACK_ERROR pf, void* pa);

/*
@FUNCTION xdk_trace_last: trace last error.
@RETURN void: none.
*/
EXP_API void xdk_trace_last(void);

#if defined(DEBUG) || defined(_DEBUG)
#define XDK_TRACE(code, token)	xdk_trace(code, token)
#define XDK_TRACE_LAST			xdk_trace_last()
#else
#define XDK_TRACE(code, token)	
#define XDK_TRACE_LAST
#endif

#ifdef	__cplusplus
}
#endif

#endif /*IMPERR_H*/