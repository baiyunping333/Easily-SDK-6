/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc error system call document

	@module	_if_error.c | linux implement file

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

#include "../xdkloc.h"

#ifdef XDK_SUPPORT_ERROR

int _error_text(tchar_t* buf, int max)
{
	strncpy(buf, strerror(errno), max);
	return (int)strlen(buf);;
}

void _error_exit(void)
{
	exit(-1);
}

void _error_debug(const char* src, const char* func, unsigned int line, const char* str)
{
	if(str) perror(str);
}

void _error_print(const tchar_t* str)
{
    if(str) perror(str);
}
#endif //XDK_SUPPORT_ERROR