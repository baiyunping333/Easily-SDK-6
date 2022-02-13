/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc stream operator document

	@module	streamopera.c | implement file

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

#include "streamopera.h"

#ifdef XDL_SUPPORT_DOC

bool_t call_stream_can_escape(void* p_obj)
{
	return 1;
}

bool_t call_stream_with_eof(void* p_obj)
{
	stream_t stm = (stream_t)p_obj;

	if (stream_get_mode(stm) == CHUNK_OPERA)
		return 1;
	else
		return stream_get_size(stm) ? 1 : 0;
}

int call_stream_read_escape(void* p_obj, int max, int pos, int encode, tchar_t* pch)
{
	stream_t stm = (stream_t)p_obj;
	bool_t rt = 0;
	dword_t dw = 0;

	rt = stream_read_escape(stm, pch, &dw);

	return (rt) ? (int)dw : C_ERR;
}

int call_stream_write_escape(void* p_obj, int max, int pos, int encode, tchar_t ch)
{
	stream_t stm = (stream_t)p_obj;
	bool_t rt = 0;
	dword_t dw = 0;

	rt = stream_write_escape(stm, ch, &dw);

	return (rt) ? (int)dw : C_ERR;
}

int call_stream_read_char(void* p_obj, int max, int pos, int encode, tchar_t* pch)
{
	stream_t stm = (stream_t)p_obj;
	int rt = 0;
	dword_t dw = 0;

	rt = stream_read_char(stm, pch, &dw);

	return (rt) ? (int)dw : C_ERR;
}

int call_stream_write_char(void* p_obj, int max, int pos, int encode, const tchar_t* pch)
{
	stream_t stm = (stream_t)p_obj;
	int rt = 0;
	dword_t dw = 0;

	rt = stream_write_char(stm, pch, &dw);

	return (rt) ? (int)dw : C_ERR;
}

int call_stream_read_token(void* p_obj, int max, int pos, int encode, tchar_t* pch, int len)
{
	stream_t stm = (stream_t)p_obj;
	bool_t rt = 0;
	dword_t dw = 0;
	dword_t bs = 0;

	max -= pos;
	pos = 0;

	while (len > 0)
	{
		if (pos >= max)
		{
			*pch = _T('\0');
			return 0;
		}

		rt = stream_read_char(stm, pch, &dw);
		if (!rt || !dw)
			break;

		pos += (int)dw;
#ifdef _UNICODE
		bs = ucs_sequence(*pch);
#else
		bs = mbs_sequence(*pch);
#endif
		pch += bs;
		len -= bs;
	}

	return (rt) ? pos : C_ERR;
}

int call_stream_write_token(void* p_obj, int max, int pos, int encode, const tchar_t* pch, int len)
{
	stream_t stm = (stream_t)p_obj;
	int rt = 0;
	dword_t dw = 0;

	if (len < 0)
		len = xslen(pch);

	if (!len)
		return 0;

	rt = stream_write(stm, pch, len, &dw);

	return (rt) ? (int)dw : C_ERR;
}

void call_stream_set_encode(void* p_obj, int encode)
{
	stream_t stm = (stream_t)p_obj;

	stream_set_encode(stm, encode);
}


#endif 