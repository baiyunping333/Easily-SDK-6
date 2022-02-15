/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc system mbcs call document

	@module	_if_mbcs_win.c | windows implement file

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

#include "../xdkloc.h"

#ifdef XDK_SUPPORT_MBCS

int _gbk_to_ucs(const schar_t* gbk, int len, wchar_t* ucs, int max)
{
	if (!ucs)
		return MultiByteToWideChar(CP_ACP, 0, gbk, len, NULL, 0);
	else
		return MultiByteToWideChar(CP_ACP, 0, gbk, len, ucs, max);
}

int _ucs_to_gbk(const wchar_t* ucs, int len, schar_t* gbk, int max)
{
	if (!gbk)
		return WideCharToMultiByte(CP_ACP, 0, ucs, len, NULL, 0, NULL, NULL);
	else
		return WideCharToMultiByte(CP_ACP, 0, ucs, len, gbk, max, NULL, NULL);
}

int _utf_to_ucs(const schar_t* utf, int len, wchar_t* ucs, int max)
{
	if (!ucs)
		return MultiByteToWideChar(CP_UTF8, 0, utf, len, NULL, 0);
	else
		return MultiByteToWideChar(CP_UTF8, 0, utf, len, ucs, max);
}

int _ucs_to_utf(const wchar_t* ucs, int len, schar_t* utf, int max)
{
	if (!utf)
		return WideCharToMultiByte(CP_UTF8, 0, ucs, len, NULL, 0, NULL, NULL);
	else
		return WideCharToMultiByte(CP_UTF8, 0, ucs, len, utf, max, NULL, NULL);
}


#endif //XDK_SUPPORT_MBCS