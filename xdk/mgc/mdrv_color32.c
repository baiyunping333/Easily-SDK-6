/***********************************************************************
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

typedef struct _color32_driver_t{
	handle_head head;

	int	width;		/* X real reslution */
	int	height;		/* Y real reslution */

	dword_t	size;		/* bytes of frame buffer */
	byte_t* addr;		/* address of frame buffer */

	int	line_dwords;	/* line length in dobule words */
}color32_driver_t;

#define VALID_COORDINATE(x, y) (x >= 0 && x < pdrv->width && y >= 0 && y < pdrv->height)

#define BLUE_MASK		0x000000FF
#define GREEN_MASK		0x0000FF00
#define RED_MASK		0x00FF0000

#define GET_COLOR32_R(c) (byte_t)(((c) & RED_MASK) >> 16)
#define GET_COLOR32_G(c) (byte_t)(((c) & GREEN_MASK) >> 8)
#define GET_COLOR32_B(c) (byte_t)((c) & BLUE_MASK)

#define PUT_COLOR32(r, g, b)	((((dword_t)r << 16) & RED_MASK) | (((dword_t)g << 8) & GREEN_MASK) | ((dword_t)b & BLUE_MASK))

static driver_t open_driver(int width, int height)
{
	color32_driver_t* pdrv;

	pdrv = (color32_driver_t*)xmem_alloc(sizeof(color32_driver_t));
	pdrv->head.tag = _DRIVER_COLOR8888;

	pdrv->width = width;
	pdrv->height = height;
	pdrv->line_dwords = BMP_PIXEL_SIZE(pdrv->width, 32) / 4;

	pdrv->size = pdrv->line_dwords * 4 * height;
	pdrv->addr = (byte_t*)xmem_alloc(pdrv->size);

	return &(pdrv->head);
}

static void close_driver(driver_t drv)
{
	color32_driver_t* pdrv = (color32_driver_t*)drv;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);

	xmem_free(pdrv->addr);
	xmem_free(pdrv);
}

static int get_pixels(driver_t drv, int x, int y, int w, int h, PIXELVAL* val, int rop)
{
	color32_driver_t* pdrv = (color32_driver_t*)drv;

	register ADDR32 addr;
	byte_t r, g, b;
	PIXELVAL c;
	int dx, dy, total = 0;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);
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

			addr = ((ADDR32)pdrv->addr) + dx + dy * pdrv->line_dwords;

			r = GET_COLOR32_R(*addr);
			g = GET_COLOR32_G(*addr);
			b = GET_COLOR32_B(*addr);

			c = PUT_PIXVAL(0, r, g, b);
			val[total] = raster_opera(rop, val[total], c);

			total++;
			dx++;
		}
		dy--;
	}

	return total;
}

static void set_pixels(driver_t drv, int x, int y, int w, int h, const PIXELVAL* val, int rop)
{
	color32_driver_t* pdrv = (color32_driver_t*)drv;

	register ADDR32 addr;
	byte_t r, g, b;
	int dx, dy, total = 0;
	PIXELVAL c;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);
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

			addr = ((ADDR32)pdrv->addr) + dx + dy * pdrv->line_dwords;

			r = GET_COLOR32_R(*addr);
			g = GET_COLOR32_G(*addr);
			b = GET_COLOR32_B(*addr);

			c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), val[total]);

			r = GET_PIXVAL_R(c);
			g = GET_PIXVAL_G(c);
			b = GET_PIXVAL_B(c);

			*addr = PUT_COLOR32(r, g, b);

			total++;
			dx++;
		}
		dy--;
	}
}

static void draw_horzline(driver_t drv, int x1, int x2, int y, PIXELVAL c, int rop)
{
	color32_driver_t* pdrv = (color32_driver_t*)drv;

	register ADDR32 addr;
	byte_t r, g, b;
	int x, w;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);

	y = (pdrv->height - 1 - y);
	x = (x1 < x2) ? x1 : x2;
	w = (x1 < x2) ? (x2 - x1) : (x1 - x2);

	addr = ((ADDR32)pdrv->addr) + x + y * pdrv->line_dwords;

	while (VALID_COORDINATE(x1, y) && w--) {
		r = GET_COLOR32_R(*addr);
		g = GET_COLOR32_G(*addr);
		b = GET_COLOR32_B(*addr);

		c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), c);

		r = GET_PIXVAL_R(c);
		g = GET_PIXVAL_G(c);
		b = GET_PIXVAL_B(c);

		*addr = PUT_COLOR32(r, g, b);

		addr++;
		x++;
	}
}

static void draw_vertline(driver_t drv, int x, int y1, int y2, PIXELVAL c, int rop)
{
	color32_driver_t* pdrv = (color32_driver_t*)drv;

	register ADDR32 addr;
	byte_t r, g, b;
	int y, h;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);

	y = (y1 < y2) ? y1 : y2;
	h = (y1 < y2) ? (y2 - y1) : (y1 - y2);
	y = (pdrv->height - 1 - y);

	addr = ((ADDR32)pdrv->addr) + x + y * pdrv->line_dwords;

	while (VALID_COORDINATE(x, y1) && h--) {
		r = GET_COLOR32_R(*addr);
		g = GET_COLOR32_G(*addr);
		b = GET_COLOR32_B(*addr);

		c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), c);

		r = GET_PIXVAL_R(c);
		g = GET_PIXVAL_G(c);
		b = GET_PIXVAL_B(c);

		*addr = PUT_COLOR32(r, g, b);

		addr -= pdrv->line_dwords;
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
	color32_driver_t* pdrv = (color32_driver_t*)drv;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);

	ADDR32	addr, d;
	ADDR8	src, s;
	int		i, dx, sx, sp;
	int		dlinelen = pdrv->line_dwords;
	int		slinelen = (pxm->width + 7) / 8;
	byte_t	r, g, b;
	PIXELVAL c;

	dsty = pdrv->height - 1 - dsty;
	addr = ((ADDR32)pdrv->addr) + dstx + dsty * dlinelen;
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
				r = GET_COLOR32_R(*d);
				g = GET_COLOR32_G(*d);
				b = GET_COLOR32_B(*d);

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), pxm->fg_color);

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				*d = PUT_COLOR32(r, g, b);
			}
			else if (pxm->bg_used)
			{
				r = GET_COLOR32_R(*d);
				g = GET_COLOR32_G(*d);
				b = GET_COLOR32_B(*d);

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), pxm->bg_color);

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				*d = PUT_COLOR32(r, g, b);
			}

			++d;
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
	color32_driver_t* pdrv = (color32_driver_t*)drv;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);

	ADDR32	addr, d;
	int		i, j, dx, dy, sx, sy;
	int		linelen = pdrv->line_dwords;
	int		bit, bit1, bit2, bit3, bit4;
	byte_t	r, g, b;
	PIXELVAL c;

	float bw, bh;

	bw = (float)srcw / (float)dstw;
	bh = (float)srch / (float)dsth;

	dsty = pdrv->height - 1 - dsty;
	addr = ((ADDR32)pdrv->addr) + dstx + dsty * linelen;
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
				r = GET_COLOR32_R(*d);
				g = GET_COLOR32_G(*d);
				b = GET_COLOR32_B(*d);

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), pxm->fg_color);

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				*d = PUT_COLOR32(r, g, b);
			}
			else if (pxm->bg_used)
			{
				r = GET_COLOR32_R(*d);
				g = GET_COLOR32_G(*d);
				b = GET_COLOR32_B(*d);

				c = raster_opera(rop, PUT_PIXVAL(0, r, g, b), pxm->bg_color);

				r = GET_PIXVAL_R(c);
				g = GET_PIXVAL_G(c);
				b = GET_PIXVAL_B(c);

				*d = PUT_COLOR32(r, g, b);
			}

			dx++;
			d++;
		}

		addr -= linelen;
		dy--;
	}
}

static dword_t get_size(driver_t drv)
{
	color32_driver_t* pdrv = (color32_driver_t*)drv;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);

	return pdrv->size;
}

static dword_t get_bytes(driver_t drv, byte_t* buf, dword_t max)
{
	color32_driver_t* pdrv = (color32_driver_t*)drv;
	dword_t total = 0;

	XDK_ASSERT(drv && drv->tag == _DRIVER_COLOR8888);

	if (buf)
	{
		xmem_copy((void*)(buf + total), (void*)(pdrv->addr), pdrv->size);
	}
	total += pdrv->size;

	return total;
}
/*****************************************************************************************************************/

mem_driver_t color8888_driver = {
	MGC_DRIVER_COLOR8888, /*the driver name*/

	1,		/* planes */
	32,		/* pixel depth 1,2,4,8, 16, 18, 24, 32 */
	0,		/* summary colors */
	PIXEL_DEPTH_COLOR32,	/* format of pixel value */

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


