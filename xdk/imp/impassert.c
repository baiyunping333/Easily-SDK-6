/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc assert document

	@module	impassert.c | implement file

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

#include "impassert.h"

#include "../xdkimp.h"
#include "../xdkstd.h"


void xdk_assert(const char* _Expr, const char* _File, const char* _Func, unsigned int _Line)
{
	if_error_t* pie;
#ifdef XDK_SUPPORT_THREAD
	if_thread_t* pit;
#endif

	pie = PROCESS_ERROR_INTERFACE;

#if defined(_DEBUG) || defined(DEBUG)
	if (pie)
	{
		(*pie->pf_error_debug)(_File, _Func, _Line, _Expr);
	}
#endif
#ifdef XDK_SUPPORT_THREAD
    
	pit = PROCESS_THREAD_INTERFACE;

	if (pit)
	{
		clear_jump();

		if ((*pit->pf_thread_get_id)() == g_xdk_mou.thread_id)
		{//the primary thread
			xdk_thread_uninit(-1);
			if(pie)
			{
				(*pie->pf_error_exit)();
			}
		}
		else
		{//the working thread
			xdk_thread_uninit(-1);
			if (pit)
			{
				(*pit->pf_thread_end)();
			}
		}
	}
#else
	pie = PROCESS_ERROR_INTERFACE;

	clear_jump();

	if (pie)
	{
		(*pie->pf_error_exit)();
	}
#endif
}

