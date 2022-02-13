/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc raster operation for image document

	@module	mdrv_raster.c | implement file

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

#include "mdrv.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"
#include "../xdkoem.h"

PIXELVAL raster_opera(RASTER_MODE rop, PIXELVAL dst, PIXELVAL src)
{
	switch (rop)
	{
	case ROP_COPY:
		return RASTER_COPY(dst, src);
	case ROP_XOR:
		return RASTER_XOR(dst, src);
	case ROP_OR:
		return RASTER_OR(dst, src);
	case ROP_AND:
		return RASTER_AND(dst, src);
	case ROP_CLR:
		return RASTER_CLR(dst);
	default:
		return RASTER_COPY(dst, src);
	}
}

