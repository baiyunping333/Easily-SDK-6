/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc clipboard document

	@module	impclip.h | interface file

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

#ifndef _IMPCLIP_H
#define _IMPCLIP_H

#include "xdcdef.h"

#ifdef XDU_SUPPORT_CLIPBOARD

#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION clipboard_put: put the content into clipboard.
@INPUT res_win_t win: the clipboard to ownered widget.
@INPUT int fmt: the data format, it can be mutiple characters(CB_FORMAT_MBS), unicode characers(CB_FORMAT_UCS), or binary bitmap(CB_FORMAT_DIB).
@INPUT const byte_t* data: the buffer for input.
@INPUT dword_t size: the data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t	clipboard_put(res_win_t win, int fmt, const byte_t* data, dword_t size);

/*
@FUNCTION clipboard_get: get the content from clipboard.
@INPUT res_win_t win: the clipboard to ownered widget.
@INPUT int fmt: the data format, it can be mutiple characters(CB_FORMAT_MBS), unicode characers(CB_FORMAT_UCS), or binary bitmap(CB_FORMAT_DIB).
@INPUT byte_t* buf: the buffer for output.
@INPUT dword_t max: the buffer size in bytes.
@RETURN dword_t: if succeeds return bytes copyed, otherwise return zero.
*/
EXP_API dword_t clipboard_get(res_win_t win, int fmt, byte_t* buf, dword_t max);


#ifdef	__cplusplus
}
#endif

#endif /*XDU_SUPPORT_CLIPBOARD*/

#endif /*IMPCLIP_H*/