﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory driver for Gray image document

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

#include "mdrv.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"
#include "../xdkoem.h"

typedef struct _color24_driver_t{
	handle_head head;

	int	width;		/* X real reslution */
	int	height;		/* Y real reslution */

	dword_t	size;		/* bytes of frame buffer */
	byte_t* addr;		/* address of frame buffer */

	int	line_bytes;	/* line length in bytes */
}color24_driver_t;

#define VALID_COORDINATE(x, y) (x >= 0 && x < pdrv->width && y >= 0 && y < pdrv->height)

static driver_t open_driver(int width, int height)
{
	color24_driver_t* pdrv;

	pdrv = (color24_driver_t*)xmem_alloc(sizeof(color24_driver_t));
	pdrv->head.tag = _DRIVER_COLOR888;

	pdrv->width = width;
	pdrv->height = height;
	pdrv->line_bytes = BMP_PIXEL_SIZE(pdrv->width, 24);

	pdrv->size = pdrv->line_bytes * height;
	pdrv->addr = (byte_t*)xmem_alloc(pdrv->size);

	return &(pdrv->head);
}

static void close_driver(driver_t drv)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);

	xmem_free(pdrv->addr);
	xmem_free(pdrv);
}

static int get_pixels(driver_t drv, int x, int y, int w, int h, PIXELVAL* val, int n, int rop)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;

	register ADDR8 addr;
	byte_t r, g, b;
	PIXELVAL c;
	int dx, dy, total = 0;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);
	XDK_ASSERT(val != NULL);

	dy = (pdrv->height - 1 - y);
	while (dy > (pdrv->height - 1 - y - h))
	{
		if (dy < 0)
		{
			total += (y + h - pdrv->height) * w;
			break;
		}

		dx = x;
		while (dx < x + w)
		{
			if (dx >= pdrv->width)
			{
				total += (x + w - pdrv->width);
				break;
			}

			if (VALID_COORDINATE(dx, dy))
			{
				addr = ((ADDR8)pdrv->addr) + dx * 3 + dy * pdrv->line_bytes;
				if (total < n)
				{
					b = addr[0];
					g = addr[1];
					r = addr[2];

					c = PUT_PIXVAL(0, r, g, b);
					val[total] = raster_opera(rop, val[total], c);
				}
			}
			total++;
			dx++;
		}
		dy--;
	}

	return total;
}

static void set_pixels(driver_t drv, int x, int y, int w, int h, const PIXELVAL* val, int n, int rop)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;

	register ADDR8 addr;
	byte_t r, g, b;
	int dx, dy, total = 0;
	PIXELVAL c;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);
	XDK_ASSERT(val != NULL);

	dy = (pdrv->height - 1 - y);
	while (dy > (pdrv->height - 1 - y - h))
	{
		if (dy < 0)
		{
			total += (y + h - pdrv->height) * w;
			break;
		}

		dx = x;
		while (dx < x + w)
		{
			if (dx >= pdrv->width)
			{
				total += (x + w - pdrv->width);
				break;
			}

			if (VALID_COORDINATE(dx, dy))
			{
				addr = ((ADDR8)pdrv->addr) + dx * 3 + dy * pdrv->line_bytes;

				b = addr[0];
				g = addr[1];
				r = addr[2];

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), ((total < n) ? val[total] : val[n - 1]));

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				addr[0] = b;
				addr[1] = g;
				addr[2] = r;
			}
			total++;
			dx++;
		}
		dy--;
	}
}

static void draw_horzline(driver_t drv, int x1, int x2, int y, PIXELVAL c, int rop)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;

	register ADDR8 addr;
	byte_t r, g, b;
	int x, w;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);

	y = (pdrv->height - 1 - y);
	x = (x1 < x2) ? x1 : x2;
	w = (x1 < x2) ? (x2 - x1) : (x1 - x2);

	addr = ((ADDR8)pdrv->addr) + x * 3 + y * pdrv->line_bytes;

	while (VALID_COORDINATE(x1, y) && w--) {
		b = addr[0];
		g = addr[1];
		r = addr[2];

		c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), c);
		r = GET_PIXVAL_R(c);
		g = GET_PIXVAL_G(c);
		b = GET_PIXVAL_B(c);

		addr[0] = b;
		addr[1] = g;
		addr[2] = r;

		addr += 3;
		x++;
	}
}

static void draw_vertline(driver_t drv, int x, int y1, int y2, PIXELVAL c, int rop)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;

	register ADDR8 addr;
	byte_t r, g, b;
	int y, h;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);

	y = (y1 < y2) ? y1 : y2;
	h = (y1 < y2) ? (y2 - y1) : (y1 - y2);
	y = (pdrv->height - 1 - y);

	addr = ((ADDR8)pdrv->addr) + x * 3 + y * pdrv->line_bytes;

	while (VALID_COORDINATE(x, y1) && h--) {
		b = addr[0];
		g = addr[1];
		r = addr[2];

		c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), c);

		r = GET_PIXVAL_R(c);
		g = GET_PIXVAL_G(c);
		b = GET_PIXVAL_B(c);

		addr[0] = b;
		addr[1] = g;
		addr[2] = r;

		addr -= pdrv->line_bytes;
		y--;
	}
}

/* takes a pixmap, each line is byte aligned, and copies it
* to the screen using fg_color and bg_color to replace a 1
* and 0 in the pixmap.
*
* The bitmap is ordered how you'd expect, with the MSB used
* for the leftmost of the 8 pixels controlled by each byte.
*
* Variables used in the gc:
*       dstx, dsty, dsth, dstw   Destination rectangle
*       srcx, srcy               Source rectangle
*       src_linelen              Linesize in bytes of source
*       data					 Pixmap data
*       fg_color                 Color of a '1' bit
*       bg_color                 Color of a '0' bit
*       usebg					If set, bg_color is used.  If zero,
*                                then '0' bits are transparentz.
*/
static void draw_pixmap(driver_t drv, int dstx, int dsty, int w, int h, mem_pixmap_ptr pxm, int srcx, int srcy, int rop)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);

	ADDR8	addr, d;
	ADDR8	src, s;
	int		i, dx, sx, sp;
	int		dlinelen = pdrv->line_bytes;
	int		slinelen = (pxm->width + 7) / 8;
	byte_t	r, g, b;
	PIXELVAL c;

	dsty = pdrv->height - 1 - dsty;
	addr = ((ADDR8)pdrv->addr) + dstx * 3 + dsty * dlinelen;
	src = ((ADDR8)pxm->data) + (srcx >> 3) + srcy * slinelen;

	while (h-- > 0) {
		d = addr;
		s = src;
		dx = dstx;
		sx = srcx;

		for (i = 0; (i < w && VALID_COORDINATE(dx, dsty)); ++i) {
			sp = sx & 7;
			if ((*s >> (7 - sp)) & 01)
			{
				b = d[0];
				g = d[1];
				r = d[2];

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), pxm->fg_color);

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				d[0] = b;
				d[1] = g;
				d[2] = r;
			}
			else if (pxm->bg_used)
			{
				b = d[0];
				g = d[1];
				r = d[2];

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), pxm->bg_color);

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				d[0] = b;
				d[1] = g;
				d[2] = r;
			}

			d += 3;
			if ((++sx & 7) == 0)
				++s;
		}

		addr -= dlinelen;
		src += slinelen;
		dsty--;
	}
}

static void stretch_pixmap(driver_t drv, int dstx, int dsty, int dstw, int dsth, mem_pixmap_ptr pxm, int srcx, int srcy, int srcw, int srch, int rop)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);

	ADDR8	addr, d;
	int		i, j, dx, dy, sx, sy;
	int		linelen = pdrv->line_bytes;
	int		bit, bit1, bit2, bit3, bit4;
	byte_t	r, g, b;
	PIXELVAL c;

	float bw, bh;

	bw = (float)srcw / (float)dstw;
	bh = (float)srch / (float)dsth;

	dsty = pdrv->height - 1 - dsty;
	addr = ((ADDR8)pdrv->addr) + dstx * 3 + dsty * linelen;
	dy = dsty;

	for (j = dsth - 1; j >= 0; j--){
		dx = dstx;
		d = addr;

		for (i = 0; (i < dstw && VALID_COORDINATE(dx, dy)); i++) {

			sx = srcx + (int)((float)(dx - dstx) * bw + 0.5);
			sy = srcy + (int)((float)(dsty - dy) * bh + 0.5);

			bit1 = pxm->getPixbit(pxm, sx, sy);
			if (sx == pxm->width - 1)
				bit2 = bit1;
			else
				bit2 = pxm->getPixbit(pxm, sx + 1, sy);
			if (sy == pxm->height - 1)
				bit3 = bit1;
			else
				bit3 = pxm->getPixbit(pxm, sx, sy + 1);
			if (sx == pxm->width - 1)
				bit4 = bit3;
			else if (sy == pxm->height - 1)
				bit4 = bit2;
			else
				bit4 = pxm->getPixbit(pxm, sx + 1, sy + 1);

			//Double Linear Interpolate: f(x,y) = f(0,0)(1-x)(1-y) + f(1,0)x(1-y) + f(0,1)(1-x)y + f(1,1)xy
			bit = bit1 * (1 - (dx - sx)) * (1 - (dy - sy)) + bit2 * (dx - sx) * (1 - (dy - sy)) + bit3 * (1 - (dx - sx)) * (dy - sy) + bit4 * (dx - sx) * (dy - sy);
			if (bit) bit = 1;

			if (bit & 0x01)
			{
				b = d[0];
				g = d[1];
				r = d[2];

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), pxm->fg_color);

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				d[0] = b;
				d[1] = g;
				d[2] = r;
			}
			else if (pxm->bg_used)
			{
				b = d[0];
				g = d[1];
				r = d[2];

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), pxm->bg_color);

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				d[0] = b;
				d[1] = g;
				d[2] = r;
			}

			dx++;
			d += 3;
		}

		addr -= linelen;
		dy--;
	}
}

static dword_t get_size(driver_t drv)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);

	return pdrv->size;
}

static dword_t get_bytes(driver_t drv, byte_t* buf, dword_t max)
{
	color24_driver_t* pdrv = (color24_driver_t*)drv;
	dword_t total = 0;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR888);

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)(pdrv->addr), pdrv->size);
	}
	total += pdrv->size;

	return total;
}
/*****************************************************************************************************************/

mem_driver_t color888_driver = {
	MGC_DRIVER_COLOR888, /*the driver name*/

	1,		/* planes */
	24,		/* pixel depth 1,2,4,8, 16, 18, 24, 32 */
	0,		/* summary colors */
	PIXEL_DEPTH_COLOR24,	/* format of pixel value */

	open_driver,
	close_driver,
	get_pixels,
	set_pixels,
	draw_horzline,
	draw_vertline,
	draw_pixmap,
	stretch_pixmap,
	get_size,
	get_bytes,
};


