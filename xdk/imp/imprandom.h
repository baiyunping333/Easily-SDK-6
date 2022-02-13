/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc random document

	@module	imprandom.h | interface file

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

#ifndef _IMPRANDOM_H
#define _IMPRANDOM_H

#include "../xdkdef.h"

#ifdef XDK_SUPPORT_RANDOM

#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION system_random: get system random.
@OUTPUT byte_t* buf: the buffer for return random bytes.
@INPUT dword_t size: the buffer size in bytes.
@RETURN bool_t: if succeeds return non zero, fails return zero.
*/
EXP_API bool_t system_random(byte_t* buf, dword_t size);


#ifdef	__cplusplus
}
#endif


#endif /*XDK_SUPPORT_RANDOM*/

#endif /*_IMPRANDOM_H*/
