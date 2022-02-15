/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc glyph finder document

	@module	gly_loader.c | implement file

	@devnote 张文权 2021.01 - 2021.12 v6.0
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

#include "gly.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"


const glyph_info_t* find_glyph_info(const tchar_t* charset, const xfont_t* pxf)
{
	int n, i;

	if (xsicmp(charset, _T("GB2312")) == 0)
	{
		n = sizeof(c_glyph_list) / sizeof(glyph_info_t);

		for (i = 0; i < n; i++)
		{
			if (xstof(pxf->size) <= xstof(c_glyph_list[i].size))
				return &(c_glyph_list[i]);
		}
	}
	else
	{
		n = sizeof(a_glyph_list) / sizeof(glyph_info_t);

		for (i = 0; i < n; i++)
		{
			if (xstof(pxf->size) <= xstof(a_glyph_list[i].size))
				return &(a_glyph_list[i]);
		}
	}

	return NULL;
}
