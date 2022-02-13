/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc clipboard document

	@module	if_clipboard_win.c | windows implement file

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

#include "../xduloc.h"

#ifdef XDU_SUPPORT_CLIPBOARD

bool_t _clipboard_put(res_win_t win, int fmt, const byte_t* data, dword_t size)
{
	HGLOBAL glb;
	void* buf;

	if (!OpenClipboard(win))
		return bool_false;

	EmptyClipboard();

	glb = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE | GMEM_ZEROINIT, (SIZE_T)size);
	if (!glb) return bool_false;

	buf = GlobalLock(glb);
	CopyMemory(buf, (void*)data, size);
	GlobalUnlock(glb);

	return (SetClipboardData(fmt, (HANDLE)glb)) ? bool_true : bool_false;
}

dword_t _clipboard_get(res_win_t win, int fmt, byte_t* buf, dword_t max)
{
	HGLOBAL glb;
	void* data;
	dword_t len;

	if (!OpenClipboard(win))
		return 0;

	if (!IsClipboardFormatAvailable(fmt))
		return 0;

	glb = (HGLOBAL)GetClipboardData(fmt);
	if (!glb) return 0;

	len = (dword_t)GlobalSize(glb);

	if (!buf) return len;

	data = GlobalLock(glb);
	if (!data) return 0;

	len = (len < max) ? len : max;
	CopyMemory((void*)data, (void*)buf, len);
	GlobalUnlock(glb);

	return len;
}

#endif //XDU_SUPPORT_CLIPBOARD