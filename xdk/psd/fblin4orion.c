﻿/*
* Copyright(c) 1999, 2000, 2003 Greg Haerr <greg@censoft.com>
*
* 4bpp Packed Linear Video Driver for Microwindows(nibble - flipped)
* 	This driver is written for the 4access Communications Orion machines
* 	which use a pxa210 and a hantronix lcd panel.
*	This combination requires a nibble - flip.
*
* 	In this driver, psd->linelen is line byte length, not line pixel length
*/

/*
*	Currently MODE_XOR is not correctly implemented.
*/

#include "fb.h"
#include "mwdevice.h"

#include <assert.h>


//#define REMAP(c)	((c) = (remap[c]))
#define REMAP(c)	((c) = (~c & 0x0f))
static const unsigned char remap[16] = { 15, 13, 11, 9, 7, 5, 3, 1, 14, 12, 10, 8, 6, 4, 2, 0 };

//#define REMAP_BACK(c)	((c) = (remap_back[c]))
#define REMAP_BACK(c)	((c) = (~c & 0x0f))
static const unsigned char remap_back[16] = { 15, 7, 14, 6, 13, 5, 12, 4, 11, 3, 10, 2, 9, 1, 8, 0 };

static const unsigned char notmask[2] = { 0x0f, 0xf0 };

/* Calc linelen and mmap size, return 0 on fail*/
static int
linear4_init(PSD psd)
{
	if (!psd->size)
		psd->size = psd->yres * psd->linelen;
	/* linelen in bytes for bpp 1, 2, 4, 8 so no change*/
	return 1;
}

/* Set pixel at x, y, to pixelval c*/
static void
linear4_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;

	assert(addr != 0);
	assert(x >= 0 && x < psd->xres);
	assert(y >= 0 && y < psd->yres);
	assert(c < psd->ncolors);

	REMAP(c);
	DRAWON;
	addr += (x >> 1) + y * psd->linelen;
	if (gr_mode == MWROP_XOR)
		*addr ^= c << ((1 - (x & 1)) << 2);
	else
		*addr = (*addr & notmask[!(x & 1)]) | (c << ((1 - (!(x & 1))) << 2));
	DRAWOFF;
}

/* Read pixel at x, y*/
static MWPIXELVAL
linear4_readpixel(PSD psd, MWCOORD x, MWCOORD y)
{
	ADDR8		addr = psd->addr;
	MWPIXELVAL	c;

	assert(addr != 0);
	assert(x >= 0 && x < psd->xres);
	assert(y >= 0 && y < psd->yres);

	c = (addr[(x >> 1) + y * psd->linelen] >> ((1 - (!(x & 1))) << 2)) & 0x0f;
	REMAP_BACK(c);
	return c;

}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void
linear4_drawhorzline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;

	assert(addr != 0);
	assert(x1 >= 0 && x1 < psd->xres);
	assert(x2 >= 0 && x2 < psd->xres);
	assert(x2 >= x1);
	assert(y >= 0 && y < psd->yres);
	assert(c < psd->ncolors);


	REMAP(c);
	DRAWON;
	addr += (x1 >> 1) + y * psd->linelen;
	if (gr_mode == MWROP_XOR) {
		while (x1 <= x2) {
			*addr ^= c << ((1 - (x1 & 1)) << 2);
			if ((++x1 & 1) == 0)
				++addr;
		}
	}
	else {
		while (x1 <= x2) {
			*addr = (*addr & notmask[!(x1 & 1)]) | (c << ((1 - (!(x1 & 1))) << 2));
			if ((++x1 & 1) == 0)
				++addr;
		}
	}
	DRAWOFF;
}

/* Draw a vertical line from x,y1 to x,y2 including final point*/
static void
linear4_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;
	int	linelen = psd->linelen;

	assert(addr != 0);
	assert(x >= 0 && x < psd->xres);
	assert(y1 >= 0 && y1 < psd->yres);
	assert(y2 >= 0 && y2 < psd->yres);
	assert(y2 >= y1);
	assert(c < psd->ncolors);

	REMAP(c);
	DRAWON;
	addr += (x >> 1) + y1 * linelen;
	if (gr_mode == MWROP_XOR)
		while (y1++ <= y2) {
			*addr ^= c << ((1 - (x & 1)) << 2);
			addr += linelen;
		}
	else
		while (y1++ <= y2) {
			*addr = (*addr & notmask[!(x & 1)]) | (c << ((1 - (!(x & 1))) << 2));
			addr += linelen;
		}
	DRAWOFF;
}

/* srccopy bitblt, opcode is currently ignored*/
static void
linear4_blit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD w, MWCOORD h,
PSD srcpsd, MWCOORD srcx, MWCOORD srcy, int op)
{
	ADDR8	dst;
	ADDR8	src;
	int	i;
	int	dlinelen = dstpsd->linelen;
	int	slinelen = srcpsd->linelen;

	assert(dstpsd->addr != 0);
	assert(dstx >= 0 && dstx < dstpsd->xres);
	assert(dsty >= 0 && dsty < dstpsd->yres);
	assert(w > 0);
	assert(h > 0);
	assert(srcpsd->addr != 0);
	assert(srcx >= 0 && srcx < srcpsd->xres);
	assert(srcy >= 0 && srcy < srcpsd->yres);
	assert(dstx + w <= dstpsd->xres);
	assert(dsty + h <= dstpsd->yres);
	assert(srcx + w <= srcpsd->xres);
	assert(srcy + h <= srcpsd->yres);

	DRAWON;
	dst = (ADDR8)(dstpsd->addr) + (dstx >> 1) + dsty * dlinelen;
	src = (ADDR8)(srcpsd->addr) + (srcx >> 1) + srcy * slinelen;
	while (--h >= 0) {
		ADDR8	d = dst;
		ADDR8	s = src;
		MWCOORD	dx = dstx;
		MWCOORD	sx = srcx;
		for (i = 0; i<w; ++i) {
			unsigned char c = *s;
			REMAP(c);
			*d = (*d & notmask[!(dx & 1)]) |
				((c >> ((1 - (!(sx & 1))) << 2) & 0x0f) << ((1 - (!(dx & 1))) << 2));
			if ((++dx & 1) == 0)
				++d;
			if ((++sx & 1) == 0)
				++s;
		}
		dst += dlinelen;
		src += slinelen;
	}
	DRAWOFF;
}

SUBDRIVER fblinear4 = {
	linear4_init,
	linear4_drawpixel,
	linear4_readpixel,
	linear4_drawhorzline,
	linear4_drawvertline,
	gen_fillrect,
	linear4_blit
};

