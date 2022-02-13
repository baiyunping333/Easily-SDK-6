/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdl string operator document

	@module	stringopera.h | interface file

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

#ifndef STRINGOPERA_H
#define STRINGOPERA_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_DOC

typedef struct _STRINGOBJECT{
	string_t var;
	int pos;
}STRINGOBJECT;

#ifdef	__cplusplus
extern "C" {
#endif

	LOC_API bool_t call_string_can_escape(void* p_obj);

	LOC_API bool_t call_string_with_eof(void* p_obj);

	LOC_API int call_string_read_char(void* p_obj, int max, int pos, int encode, tchar_t* pch);

	LOC_API int call_string_read_token(void* p_obj, int max, int pos, int encode, tchar_t* pch, int len);

	LOC_API int call_string_write_char(void* p_obj, int max, int pos, int encode, const tchar_t* pch);

	LOC_API int call_string_write_indent(void* p_obj, int max, int pos, int encode);

	LOC_API int call_string_write_carriage(void* p_obj, int max, int pos, int encode);

	LOC_API int call_string_write_token(void* p_obj, int max, int pos, int encode, const tchar_t* pch, int len);


#ifdef	__cplusplus
}
#endif

#endif

#endif /*STRINGOPERA_H*/
