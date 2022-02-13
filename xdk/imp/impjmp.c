/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc jump document

	@module	impjmp.c | implement file

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

#include "impjmp.h"

#include "../xdkimp.h"
#include "../xdkstd.h"


//#define ONERROR		setjmp(thread_jump_interface()->if_buf)


void push_jump()
{
	if_jump_t* pju;

	pju = THREAD_JUMP_INTERFACE;
	XDK_ASSERT(pju != NULL);

	if (pju->if_index + 1 == pju->if_size)
	{
		pju->if_size++;

		pju->if_buf = (jmp_buf*)xmem_realloc(pju->if_buf, pju->if_size * sizeof(jmp_buf));
	}

	pju->if_index++;
}

void pop_jump()
{
	if_jump_t* pju;

	pju = THREAD_JUMP_INTERFACE;
	XDK_ASSERT(pju != NULL);

	XDK_ASSERT(pju->if_index >= 0);

	xmem_zero((void*)&(pju->if_buf[pju->if_index]), sizeof(jmp_buf));
	pju->if_index--;

	if (pju->if_index < 0)
	{
		xmem_free(pju->if_buf);

		pju->if_buf = NULL;
		pju->if_size = 0;
	}
}

void clear_jump()
{
	if_jump_t* pju;

	pju = THREAD_JUMP_INTERFACE;
	if (pju == NULL)
		return;

	if (pju->if_buf)
	{
		xmem_free(pju->if_buf);

		pju->if_buf = NULL;
		pju->if_size = 0;
		pju->if_index = -1;
	}
}

void raise_user_error(const tchar_t* errcode, const tchar_t* errtext)
{
	if_jump_t* pju;

	pju = THREAD_JUMP_INTERFACE;
	
	XDK_ASSERT(pju != NULL);

	if (errcode && errtext)
	{
		set_last_error(errcode, errtext, -1);
	}

	XDK_ASSERT(pju->if_index >= 0);

	longjmp(pju->if_buf[pju->if_index], 1);
}

void raise_system_error(const tchar_t* errcode)
{
	tchar_t errtext[ERR_LEN + 1] = { 0 };

#ifdef XDK_SUPPORT_ERROR
	if_error_t* pie;

	pie = PROCESS_ERROR_INTERFACE;

	XDK_ASSERT(pie != NULL);

	(*pie->pf_error_text)(errtext, ERR_LEN);
#endif
	
	raise_user_error(errcode, errtext);
}
