/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory context device document

	@module	mdev.h | interface file

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

#ifndef _MEMDEV_H
#define _MEMDEV_H

#include "mdef.h"
#include "mdrv.h"

#define MGC_DEVICE_BITMAP_MONOCHROME		_T("Monochrome Bitmap Device")
#define MGC_DEVICE_BITMAP_GRAYSCALE			_T("Grayscale Bitmap Device")
#define MGC_DEVICE_BITMAP_TRUECOLOR16		_T("TrueColor16 Bitmap Device")
#define MGC_DEVICE_BITMAP_TRUECOLOR24		_T("TrueColor24 Bitmap Device")
#define MGC_DEVICE_BITMAP_TRUECOLOR32		_T("TrueColor32 Bitmap Device")

typedef struct _mem_device_t* mem_device_ptr;

typedef struct _mem_device_t{
	tchar_t dev_name[MAX_DEVICE_NAME];

	mem_driver_ptr driver;

	int depth;	/*color depth*/

	device_t(*openDevice)(const dev_prn_t* devPrint, int dpi);
	void(*closeDevice)(device_t dev);
	void(*getPoint)(device_t dev, const xpoint_t* ppt, xcolor_t* pxc);
	void(*setPoint)(device_t dev, const xpoint_t* ppt, const xcolor_t* pxc, int rop);
	void(*drawPoints)(device_t dev, const xpoint_t* ppt, int n, const xcolor_t* pxc, int c, int rop);
	void(*fillPoints)(device_t dev, int x, int y, int w, int h, const xcolor_t* pxc, int rop);
	void(*drawPixmap)(device_t dev, int dstx, int dsty, int w, int h, mem_pixmap_ptr pxm, int srcx, int srcy, int rop);
	void(*stretchPixmap)(device_t dev, int dstx, int dsty, int dstw, int dsth, mem_pixmap_ptr pxm, int srcx, int srcy, int srcw, int srch, int rop);
	void(*getSize)(device_t dev, dword_t* pTotal, dword_t* pPixel);
	dword_t(*getBitmap)(device_t dev, byte_t* buf, dword_t max);
	void(*horzLine)(device_t dev, const xpoint_t* ppt1, const xpoint_t* ppt2, const xcolor_t* pxc, int rop);
	void(*vertLine)(device_t dev, const xpoint_t* ppt1, const xpoint_t* ppt2, const xcolor_t* pxc, int rop);
} mem_device_t;

extern mem_device_t monochrome_bitmap_device;
extern mem_device_t grayscale_bitmap_device;
extern mem_device_t truecolor16_bitmap_device;
extern mem_device_t truecolor24_bitmap_device;
extern mem_device_t truecolor32_bitmap_device;

#ifdef	__cplusplus
extern "C" {
#endif



#ifdef	__cplusplus
}
#endif

#endif /*_MDEV_H*/
