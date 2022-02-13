/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc async document

	@module	impasync.h | interface file

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

#ifndef _IMPASYNC_H
#define	_IMPASYNC_H

#include "../xdkdef.h"

#ifdef XDK_SUPPORT_ASYNC


#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION async_init: init async operation resource.
@INOUTPUT int type: the async mode, it can be ASYNC_BLOCK, ASYNC_EVENT, ASYNC_QUEUE.
@INPUT int ms: timeout in millisecond, -1 indicate INFINITE
@INPUT res_file_t fd: if type is ASYNC_QUEUE, this param is the input output handle for creating queue.
@RETURN  void: none.
*/
EXP_API void async_init(async_t* pas, int type, int ms, res_file_t fd);

/*
@FUNCTION async_uninit: uninit async operation resource.
@INPUT async_t* pas: the async struct for releasing background resource.
@RETURN void: none.
*/
EXP_API void async_uninit(async_t* pas);


#ifdef	__cplusplus
}
#endif

#endif /*XDK_SUPPORT_ASYNC*/

#endif	/*_IMPASYNC_H*/

