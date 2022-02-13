/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc string operator document

	@module	stringopera.c | implement file

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

#include "stringopera.h"


#ifdef XDL_SUPPORT_DOC

bool_t call_string_can_escape(void* p_obj)
{
	return 0;
}

bool_t call_string_with_eof(void* p_obj)
{
	return 1;
}

int call_string_read_char(void* p_obj, int max, int pos, int encode, tchar_t* pch)
{
	STRINGOBJECT* ptt = (STRINGOBJECT*)p_obj;

	if (string_len(ptt->var) == ptt->pos)
		return 0;

	string_get_chars(ptt->var, ptt->pos, pch, 1);
	ptt->pos = ptt->pos + 1;

	return 1;
}

int call_string_read_token(void* p_obj, int max, int pos, int encode, tchar_t* pch, int len)
{
	int i = 0;

	while (call_string_read_char(p_obj, max, pos + i, encode, pch + i))
	{
		i++;
	}

	return i;
}

int call_string_write_char(void* p_obj, int max, int pos, int encode, const tchar_t* pch)
{
	STRINGOBJECT* ptt = (STRINGOBJECT*)p_obj;

	string_ins_chars(ptt->var, ptt->pos, pch, 1);

	ptt->pos = ptt->pos + 1;

	return 1;
}

int call_string_write_indent(void* p_obj, int max, int pos, int encode)
{
	tchar_t pch[2] = { 0 };

	pch[0] = _T('\t');
	return call_string_write_char(p_obj, max, pos, encode, pch);
}

int call_string_write_carriage(void* p_obj, int max, int pos, int encode)
{
	tchar_t pch[2] = { 0 };

	pch[0] = _T('\n');
	return call_string_write_char(p_obj, max, pos, encode, pch);
}

int call_string_write_token(void* p_obj, int max, int pos, int encode, const tchar_t* pch, int len)
{
	int i = 0;

	if (len < 0)
		len = xslen(pch);

	for (i = 0; i < len; i++)
	{
		call_string_write_char(p_obj, max, pos + i, encode, pch + i);
	}

	return i;
}


#endif
