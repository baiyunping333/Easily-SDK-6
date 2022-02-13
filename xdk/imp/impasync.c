/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc async document

	@module	impasync.c | implement file

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

#include "impasync.h"

#include "../xdkimp.h"
#include "../xdkstd.h"


#ifdef XDK_SUPPORT_ASYNC

void async_init(async_t* pas, int type, int ms, res_file_t fd)
{
	if_async_t *pif;

	pif = PROCESS_ASYNC_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_async_init)(pas, type, ms, fd);
}

void async_uninit(async_t* pas)
{
	if_async_t *pif;

	pif = PROCESS_ASYNC_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_async_uninit)(pas);
}


#endif //XDK_SUPPORT_ASYNC
