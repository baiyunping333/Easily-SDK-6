/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk jump document

	@module	impjmp.h | interface file

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

#ifndef _IMPJMP_H
#define _IMPJMP_H

#include "../xdkdef.h"


#define TRY_CATCH		do{push_jump();if(setjmp(*thread_jump_buff())){pop_jump();goto _XDK_ERROR;}
#define END_CATCH		pop_jump();}while(0)
#define CLN_CATCH		do{pop_jump();}while(0)
#define ONERROR			_XDK_ERROR


#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void push_jump(void);

EXP_API void pop_jump(void);

EXP_API void clear_jump(void);

EXP_API void raise_user_error(const tchar_t* errcode, const tchar_t* errtext);

EXP_API void raise_system_error(const tchar_t* errcode);

#ifdef	__cplusplus
}
#endif


#endif /*IMPJMP_H*/