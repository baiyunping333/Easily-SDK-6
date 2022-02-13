/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc region document

	@module	if_region.c | windows implement file

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

#ifdef XDU_SUPPORT_CONTEXT_REGION

#ifndef WINCE
#pragma comment(lib, "Msimg32.lib")
#endif

res_rgn_t _create_region(const tchar_t* shape, const xrect_t* pxr)
{
	RECT rt;

	rt.left = pxr->x;
	rt.top = pxr->y;
	rt.right = pxr->x + pxr->w;
	rt.bottom = pxr->y + pxr->h;

	if (xscmp(shape, GDI_ATTR_REGION_RECT) == 0)
		return CreateRectRgn(rt.left, rt.top, rt.right, rt.bottom);
	else if (xscmp(shape, GDI_ATTR_REGION_ROUND) == 0)
		return CreateRoundRectRgn(rt.left, rt.top, rt.right, rt.bottom, 10, 10);
	else if (xscmp(shape, GDI_ATTR_REGION_ELLIPSE) == 0)
		return CreateEllipticRgn(rt.left, rt.top, rt.right, rt.bottom);
	else
		return NULL;
}

void _delete_region(res_rgn_t rgn)
{
	DeleteObject(rgn);
}

bool_t _pt_in_region(res_rgn_t rgn, const xpoint_t* ppt)
{
	return (PtInRegion(rgn, ppt->x, ppt->y)) ? 1 : 0;
}


#endif //XDU_SUPPORT_CONTEXT_REGION
