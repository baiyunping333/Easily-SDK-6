/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc timer service document

	@module	srvtimer.h | interface file

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

#ifndef _SRVTIMER_H
#define _SRVTIMER_H

#include "../xdkdef.h"
#include "../loginf.h"

typedef struct _timer_block_t{
	tchar_t name[RES_LEN + 1];
	tchar_t path[PATH_LEN + 1];

	const loged_interface* plg;
}timer_block_t;

typedef enum{
	TIMER_INVOKE_SUCCEED = 0,
	TIMER_INVOKE_WITHINFO = 1,
	TIMER_INVOKE_WITHERROR = 2,
	TIMER_INVOKE_PENDING = 100
}TIMER_INVOKE_STATE;

typedef int(STDCALL *PF_TIMER_INVOKE)(const timer_block_t* pt);

#endif /*_SRVTIMER_H*/