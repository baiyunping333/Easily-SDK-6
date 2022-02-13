/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc clipboard document

	@module	impclip.c | implement file

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

#include "impclip.h"
#include "xdcinit.h"

#ifdef XDU_SUPPORT_CLIPBOARD


bool_t clipboard_put(res_win_t win, int fmt, const byte_t* data, dword_t size)
{
	if_clipboard_t* pif;

	pif = PROCESS_CLIPBOARD_INTERFACE;

	return (pif->pf_clipboard_put)(win, fmt, data, size);
}

dword_t clipboard_get(res_win_t win, int fmt, byte_t* buf, dword_t max)
{
	if_clipboard_t* pif;

	pif = PROCESS_CLIPBOARD_INTERFACE;

	return (pif->pf_clipboard_get)(win, fmt, buf, max);
}


#endif //XDU_SUPPORT_CLIPBOARD
