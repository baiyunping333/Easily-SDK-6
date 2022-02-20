/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory device for bitmap document

	@module	mdev_bitmap.c | implement file

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

#include "mdev.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"
#include "../xdkimg.h"

typedef struct _bitmap_device_t{
	handle_head head;

	mem_driver_ptr driver;
	driver_t handle;

	int dpi;
	bitmap_info_head_t bitmap_info;
	
}bitmap_device_t;

static const mem_driver_ptr select_driver(const tchar_t* devName)
{
	if (xsicmp(devName, MGC_DEVICE_BITMAP_MONOCHROME) == 0)
		return &monochrome_driver;
	else if (xsicmp(devName, MGC_DEVICE_BITMAP_GRAYSCALE) == 0)
		return &grayscale_driver;
	else if (xsicmp(devName, MGC_DEVICE_BITMAP_TRUECOLOR16) == 0)
		return &color555_driver;
	else if (xsicmp(devName, MGC_DEVICE_BITMAP_TRUECOLOR24) == 0)
		return &color888_driver;
	else if (xsicmp(devName, MGC_DEVICE_BITMAP_TRUECOLOR32) == 0)
		return &color8888_driver;
	else
	{
		set_last_error(_T("select_driver"), _T("unknown memory device"), -1);
		return NULL;
	}
}

static device_t open_device(const dev_prn_t* devPrint, int dpi)
{
	bitmap_device_t* pdev;

	int resx, resy;

	XDK_ASSERT(devPrint != NULL);

	TRY_CATCH;

	resx = (int)(((float)devPrint->paper_width / (10.0 * MMPERINCH)) * (float)dpi);
	resy = (int)(((float)devPrint->paper_height / (10.0 * MMPERINCH)) * (float)dpi);

	pdev = (bitmap_device_t*)xmem_alloc(sizeof(bitmap_device_t));
	pdev->head.tag = _DEVICE_BITMAP;

	pdev->driver = select_driver(devPrint->devname);
	if (!pdev->driver)
	{
		raise_user_error(_T("open_device"), _T("openDriver"));
	}

	pdev->handle = (*(pdev->driver->openDriver))(resx, resy);
	if (!pdev->handle)
	{
		raise_user_error(_T("open_device"), _T("openDriver"));
	}

	pdev->bitmap_info.isize = BMP_INFOHEADER_SIZE;
	pdev->bitmap_info.width = resx;
	pdev->bitmap_info.height = resy;
	pdev->bitmap_info.planes = 1;
	pdev->bitmap_info.clrbits = pdev->driver->bits_per_pixel;
	pdev->bitmap_info.compress = 0;
	pdev->bitmap_info.bytes = BMP_PIXEL_SIZE(resx, pdev->bitmap_info.clrbits) * resy;
	pdev->bitmap_info.xpelsperm = 0;
	pdev->bitmap_info.ypelsperm = 0;
	pdev->bitmap_info.clrused = (pdev->bitmap_info.clrbits <= 8)? (1 << pdev->bitmap_info.clrbits) : 0;
	pdev->bitmap_info.clrimport = 0;

	END_CATCH;

	return &(pdev->head);
ONERROR:
	XDK_TRACE_LAST;

	if (pdev)
	{
		xmem_free(pdev);
	}

	return NULL;
}

void close_device(device_t dev)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);

	if (pdev->handle)
	{
		(*(pdev->driver->closeDriver))(pdev->handle);
	}

	xmem_free(pdev);
}

static void get_point(device_t dev, const xpoint_t* ppt, xcolor_t* pxc)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);
	XDK_ASSERT(pdev->handle);

	PIXELVAL c;

	c = (*(pdev->driver->getPixels))(pdev->handle, ppt->x, ppt->y, 1, 1, &c, 1, ROP_COPY);

	pxc->a = GET_PIXVAL_A(c);
	pxc->b = GET_PIXVAL_B(c);
	pxc->g = GET_PIXVAL_G(c);
	pxc->r = GET_PIXVAL_R(c);
}

static void set_point(device_t dev, const xpoint_t* ppt, const xcolor_t* pxc, int rop)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);
	XDK_ASSERT(pdev->handle);

	PIXELVAL c;

	c = PUT_PIXVAL(pxc->a, pxc->r, pxc->g, pxc->b);

	(*(pdev->driver->setPixels))(pdev->handle, ppt->x, ppt->y, 1, 1, &c, 1, rop);
}

static void draw_points(device_t dev, const xpoint_t* ppt, int n, const xcolor_t* pxc, int m, int rop)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);
	XDK_ASSERT(pdev->handle);

	PIXELVAL c;
	int i, j = 0;

	for (i = 0; i < n; i++)
	{
		c = PUT_PIXVAL(pxc[j].a, pxc[j].r, pxc[j].g, pxc[j].b);
		if (j < m - 1) j++;

		(*(pdev->driver->setPixels))(pdev->handle, ppt[i].x, ppt[i].y, 1, 1, &c, 1, rop);
	}
}

static void fill_points(device_t dev, int x, int y, int w, int h, const xcolor_t* pxc, int rop)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);
	XDK_ASSERT(pdev->handle);

	PIXELVAL c;

	c = PUT_PIXVAL(pxc[0].a, pxc[0].r, pxc[0].g, pxc[0].b);

	(*(pdev->driver->setPixels))(pdev->handle, x, y, w, h, &c, 1, rop);
}

static void draw_pixmap(device_t dev, int dstx, int dsty, int w, int h, mem_pixmap_ptr pxm, int srcx, int srcy, int rop)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);

	(*(pdev->driver->drawPixmap))(pdev->handle, dstx, dsty, w, h, pxm, srcx, srcy, rop);
}

static void stretch_pixmap(device_t dev, int dstx, int dsty, int dstw, int dsth, mem_pixmap_ptr pxm, int srcx, int srcy, int srcw, int srch, int rop)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);

	(*(pdev->driver->stretchPixmap))(pdev->handle, dstx, dsty, dstw, dsth, pxm, srcx, srcy, srcw, srch, rop);
}

static void get_size(device_t dev, dword_t* pTotal, dword_t* pPixel)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;
	
	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);

	*pPixel = (*(pdev->driver->getSize))(pdev->handle);
	*pTotal = BMP_INFOHEADER_SIZE + pdev->bitmap_info.clrused * BMP_RGBQUAD_SIZE + *pPixel;
}

static dword_t get_bitmap(device_t dev, byte_t* buf, dword_t max)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;
	dword_t total = 0;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);

	total += xbmp_set_info(&pdev->bitmap_info, ((buf) ? (buf + total) : NULL), (max - total));
	total += xbmp_fill_quad(pdev->bitmap_info.clrbits, pdev->bitmap_info.clrused, ((buf) ? (buf + total) : NULL), (max - total));
	total += (*(pdev->driver->getBytes))(pdev->handle, ((buf) ? (buf + total) : NULL), (max - total));
	
	return total;
}

void horz_line(device_t dev, const xpoint_t* ppt1, const xpoint_t* ppt2, const xcolor_t* pxc, int rop)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);
	XDK_ASSERT(ppt1 != NULL && ppt2 != NULL);

	PIXELVAL c;

	if (pxc)
		c = PUT_PIXVAL(pxc->a, pxc->r, pxc->g, pxc->b);
	else
		c = PUT_PIXVAL(0, 255, 255, 255);

	(*(pdev->driver->drawHorzline))(pdev->handle, ppt1->x, ppt2->x, ppt1->y, c, rop);
}

void vert_line(device_t dev, const xpoint_t* ppt1, const xpoint_t* ppt2, const xcolor_t* pxc, int rop)
{
	bitmap_device_t* pdev = (bitmap_device_t*)dev;

	XDK_ASSERT(dev && dev->tag == _DEVICE_BITMAP);
	XDK_ASSERT(ppt1 != NULL && ppt2 != NULL);

	PIXELVAL c;

	if (pxc)
		c = PUT_PIXVAL(pxc->a, pxc->r, pxc->g, pxc->b);
	else
		c = PUT_PIXVAL(0, 255, 255, 255);

	(*(pdev->driver->drawVertline))(pdev->handle, ppt1->x, ppt1->y, ppt2->y, c, rop);
}

/**************************************************************************************************/
mem_device_t monochrome_bitmap_device = {
	MGC_DEVICE_BITMAP_MONOCHROME,

	&monochrome_driver,
	PIXEL_DEPTH_PALETTE1,

	open_device,
	close_device,
	get_point,
	set_point,
	draw_points,
	fill_points,
	draw_pixmap,
	stretch_pixmap,
	get_size,
	get_bitmap,
	horz_line,
	vert_line
};

mem_device_t grayscale_bitmap_device = {
	MGC_DEVICE_BITMAP_GRAYSCALE,

	&grayscale_driver,
	PIXEL_DEPTH_PALETTE8,

	open_device,
	close_device,
	get_point,
	set_point,
	draw_points,
	fill_points,
	draw_pixmap,
	stretch_pixmap,
	get_size,
	get_bitmap,
	horz_line,
	vert_line
};

mem_device_t truecolor16_bitmap_device = {
	MGC_DEVICE_BITMAP_TRUECOLOR16,

	&color555_driver,
	PIXEL_DEPTH_COLOR16,

	open_device,
	close_device,
	get_point,
	set_point,
	draw_points,
	fill_points,
	draw_pixmap,
	stretch_pixmap,
	get_size,
	get_bitmap,
	horz_line,
	vert_line
};

mem_device_t truecolor24_bitmap_device = {
	MGC_DEVICE_BITMAP_TRUECOLOR24,

	&color888_driver,
	PIXEL_DEPTH_COLOR24,

	open_device,
	close_device,
	get_point,
	set_point,
	draw_points,
	fill_points,
	draw_pixmap,
	stretch_pixmap,
	get_size,
	get_bitmap,
	horz_line,
	vert_line
};

mem_device_t truecolor32_bitmap_device = {
	MGC_DEVICE_BITMAP_TRUECOLOR32,

	&color8888_driver,
	PIXEL_DEPTH_COLOR32,

	open_device,
	close_device,
	get_point,
	set_point,
	draw_points,
	fill_points,
	draw_pixmap,
	stretch_pixmap,
	get_size,
	get_bitmap,
	horz_line,
	vert_line
};