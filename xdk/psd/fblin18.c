﻿/*
* Copyright (c) 2000, 2001, 2010 Greg Haerr <greg@censoft.com>
* Portions Copyright (c) 2002 by Koninklijke Philips Electronics N.V.
*
* 18bpp Linear Video Driver for Microwindows
*
* M. BRACH V. (v.brach@smie.com)
* This file is based on the fblin24.c module with adaptation of the framebuffer
* dialog to drive a 18bpp (666) framebuffer color memory structure.
* In framebuffer memory map, a pixel is adressing by 3 bytes (24bits) with
* the color structure |000000R5R4|R3R2R1R0V5V4V3V2|V1V0B5B4B3B2B1B0
*
* This driver work with nano-X with SCREEN_PIXTYPE=MWPF_TRUECOLOR888
* This adaptation was developed for the ARM PXA270 with a 18bit LCD panel
*/

#define MWINCLUDECOLORS

#include "fb.h"
#include "mwdevice.h"

#include <assert.h>

/* Calc linelen and mmap size, return 0 on fail*/
static int
linear18_init(PSD psd)
{
	if (!psd->size) {
		psd->size = psd->yres * psd->linelen;
		/* convert linelen from byte to pixel len for bpp 16, 24, 18, 32*/
		psd->linelen /= 3;
	}
	return 1;
}

/* Convert a colormap 24bpp RGB888 to 24bpp RGB666 (with only 18 LSB bits useful)*/
/* Convert the 24bpp color represented by
* "R7 R6 R5 R4 R3 R2 R1 R0 | V7 V6 V5 V4 V3 V2 V1 V0 | B7 B6 B5 B4 B3 B2 B1 B2"
* to an 24bpp containing
* "0  0  0  0  0  0  R7 R6 | R5 R4 R3 R2 V7 V6 V5 V4 | V3 V2 B7 B6 B5 B4 B3 B2"
*/
static void convert24_888to24_666(MWPIXELVAL *c)
{
	MWPIXELVAL newc;
	// A l'appel de la fonction on a c = R7 R6 R5 R4 R3 R2 R1 R0 | V7 V6 V5 V4 V3 V2 V1 V0 | B7 B6 B5 B4 B3 B2 B1 B2
	// On veut obtenir					 0  0  0  0  0  0  R7 R6 | R5 R4 R3 R2 V7 V6 V5 V4 | V3 V2 B7 B6 B5 B4 B3 B2
	// 
	newc = (((*c) >> 6) & 0x03F000); // 0  0  0  0  0  0  R7 R6 | R5 R4 R3 R2 0  0  0  0  | 0  0  0  0  0  0  0  0
	newc |= (((*c) >> 4) & 0x000FC0); // 0  0  0  0  0  0  R7 R6 | R5 R4 R3 R2 V7 V6 V5 V4 | V3 V2 0  0  0  0  0  0
	newc |= (((*c) >> 2) & 0x00003F); // 0  0  0  0  0  0  R7 R6 | R5 R4 R3 R2 V7 V6 V5 V4 | V3 V2 B7 B6 B5 B4 B3 B2
	*c = newc;
}

/* Invert of the 'convert24_888to24_666' function */
/* Convert a colormap 24bpp RGB666 (with only 18 LSB bits useful) to 24bpp RGB888 */
static MWPIXELVAL convert24_666to24_888(MWPIXELVAL c)
{
	MWPIXELVAL newc;
	// A l'appel de la fonction on a c =0  0  0  0  0  0  R7 R6 | R5 R4 R3 R2 V7 V6 V5 V4 | V3 V2 B7 B6 B5 B4 B3 B2
	// On veut obtenir					R7 R6 R5 R4 R3 R2 0  0  | V7 V6 V5 V4 V3 V2 0  0  | B7 B6 B5 B4 B3 B2 0  0
	// 
	newc = ((c << 6) & 0xFC0000);	//	R7 R6 R5 R4 R3 R2 0  0  | 0  0  0  0  0  0  0  0  | 0  0  0  0  0  0  0  0
	newc |= ((c << 4) & 0x00FC00);	//	R7 R6 R5 R4 R3 R2 0  0  | V7 V6 V5 V4 V3 V2 0  0  | 0  0  0  0  0  0  0  0
	newc |= ((c << 2) & 0x0000FC);	//	R7 R6 R5 R4 R3 R2 0  0  | V7 V6 V5 V4 V3 V2 0  0  | B7 B6 B5 B4 B3 B2 0  0
	return newc;
}

/* Set pixel at x, y, to pixelval c*/
static void
linear18_drawpixel(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;
	MWUCHAR	r, g, b;

	assert(addr != 0);
	assert(x >= 0 && x < psd->xres);
	assert(y >= 0 && y < psd->yres);
	convert24_888to24_666(&c);
	assert(c < psd->ncolors);
	r = PIXEL888RED(c);
	g = PIXEL888GREEN(c);
	b = PIXEL888BLUE(c);

	addr += (x + y * psd->linelen) * 3;
	DRAWON;
	if (gr_mode == MWROP_COPY) {
		*addr++ = b;
		*addr++ = g;
		*addr = r;
	}
	else {
		applyOp(gr_mode, b, addr, ADDR8); ++addr;
		applyOp(gr_mode, g, addr, ADDR8); ++addr;
		applyOp(gr_mode, r, addr, ADDR8);
	}
	DRAWOFF;
}

/* Read pixel at x, y*/
static MWPIXELVAL
linear18_readpixel(PSD psd, MWCOORD x, MWCOORD y)
{
	ADDR8	addr = psd->addr;

	assert(addr != 0);
	assert(x >= 0 && x < psd->xres);
	assert(y >= 0 && y < psd->yres);

	addr += (x + y * psd->linelen) * 3;
	return convert24_666to24_888(RGB2PIXEL888(addr[2], addr[1], addr[0]));
}

/* Draw horizontal line from x1,y to x2,y including final point*/
static void
linear18_drawhorzline(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;
	MWUCHAR	r, g, b;

	assert(addr != 0);
	assert(x1 >= 0 && x1 < psd->xres);
	assert(x2 >= 0 && x2 < psd->xres);
	assert(x2 >= x1);
	assert(y >= 0 && y < psd->yres);
	convert24_888to24_666(&c);
	assert(c < psd->ncolors);

	r = PIXEL888RED(c);
	g = PIXEL888GREEN(c);
	b = PIXEL888BLUE(c);
	addr += (x1 + y * psd->linelen) * 3;
	DRAWON;
	if (gr_mode == MWROP_COPY) {
		while (x1++ <= x2) {
			*addr++ = b;
			*addr++ = g;
			*addr++ = r;
		}
	}
	else {
		while (x1++ <= x2) {
			applyOp(gr_mode, b, addr, ADDR8); ++addr;
			applyOp(gr_mode, g, addr, ADDR8); ++addr;
			applyOp(gr_mode, r, addr, ADDR8); ++addr;
		}
	}
	DRAWOFF;
}

/* Draw a vertical line from x,y1 to x,y2 including final point*/
static void
linear18_drawvertline(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2, MWPIXELVAL c)
{
	ADDR8	addr = psd->addr;
	int	linelen = psd->linelen * 3;
	MWUCHAR	r, g, b;

	assert(addr != 0);
	assert(x >= 0 && x < psd->xres);
	assert(y1 >= 0 && y1 < psd->yres);
	assert(y2 >= 0 && y2 < psd->yres);
	assert(y2 >= y1);
	convert24_888to24_666(&c);
	assert(c < psd->ncolors);

	r = PIXEL888RED(c);
	g = PIXEL888GREEN(c);
	b = PIXEL888BLUE(c);
	addr += (x + y1 * psd->linelen) * 3;
	DRAWON;
	if (gr_mode == MWROP_COPY) {
		while (y1++ <= y2) {
			addr[0] = b;
			addr[1] = g;
			addr[2] = r;
			addr += linelen;
		}
	}
	else {
		while (y1++ <= y2) {
			applyOp(gr_mode, b, &addr[0], ADDR8);
			applyOp(gr_mode, g, &addr[1], ADDR8);
			applyOp(gr_mode, r, &addr[2], ADDR8);
			addr += linelen;
		}
	}
	DRAWOFF;
}

/* srccopy bitblt, opcode is currently ignored*/
static void
linear18_blit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD w, MWCOORD h,
PSD srcpsd, MWCOORD srcx, MWCOORD srcy, int op)
{
	ADDR8	dst = dstpsd->addr;
	ADDR8	src = srcpsd->addr;
	int	i;
	int	dlinelen = dstpsd->linelen * 3;
	int	slinelen = srcpsd->linelen * 3;
	int	dlinelen_minus_w = (dstpsd->linelen - w) * 3;
	int	slinelen_minus_w = (srcpsd->linelen - w) * 3;

	assert(dst != 0);
	assert(dstx >= 0 && dstx < dstpsd->xres);
	assert(dsty >= 0 && dsty < dstpsd->yres);
	assert(w > 0);
	assert(h > 0);
	assert(src != 0);
	assert(srcx >= 0 && srcx < srcpsd->xres);
	assert(srcy >= 0 && srcy < srcpsd->yres);
	assert(dstx + w <= dstpsd->xres);
	assert(dsty + h <= dstpsd->yres);
	assert(srcx + w <= srcpsd->xres);
	assert(srcy + h <= srcpsd->yres);

	DRAWON;
	dst += (dstx + dsty * dstpsd->linelen) * 3;
	src += (srcx + srcy * srcpsd->linelen) * 3;

	if (op == MWROP_BLENDCONSTANT) {
		uint32_t alpha = 150, pd;

		while (--h >= 0) {
			for (i = 0; i<w; ++i) {
				if (alpha != 0) {
					pd = *dst;
					*dst++ = muldiv255(alpha, *src++ - pd) + pd;
					pd = *dst;
					*dst++ = muldiv255(alpha, *src++ - pd) + pd;
					pd = *dst;
					*dst++ = muldiv255(alpha, *src++ - pd) + pd;
				}
				else {
					dst += 3;
					src += 3;
				}
			}
			dst += dlinelen_minus_w;
			src += slinelen_minus_w;
		}
	}
	else if (op == MWROP_COPY) {
		/* copy from bottom up if dst in src rectangle*/
		/* memmove is used to handle x case*/
		if (srcy < dsty) {
			src += (h - 1) * slinelen;
			dst += (h - 1) * dlinelen;
			slinelen *= -1;
			dlinelen *= -1;
		}
		while (--h >= 0) {
			/* a _fast_ memcpy is a _must_ in this routine*/
			memmove(dst, src, w * 3);
			dst += dlinelen;
			src += slinelen;
		}
	}
	else {
		while (--h >= 0) {
			applyOp4(w * 3, op, src, dst, ADDR8);
			dst += dlinelen_minus_w;
			src += slinelen_minus_w;
		}
	}
	DRAWOFF;
}

#if 0000 /* DEPRECATED*/
/* srccopy stretchblt*/
static void
linear18_stretchblit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD dstw,
MWCOORD dsth, PSD srcpsd, MWCOORD srcx, MWCOORD srcy, MWCOORD srcw,
MWCOORD srch, int op)
{
	ADDR8	dst;
	ADDR8	src;
	int	dlinelen = dstpsd->linelen;
	int	slinelen = srcpsd->linelen;
	int	i, ymax;
	int	row_pos, row_inc;
	int	col_pos, col_inc;
	unsigned char r = 0;
	unsigned char g = 0;
	unsigned char b = 0;

	assert(dstpsd->addr != 0);
	assert(dstx >= 0 && dstx < dstpsd->xres);
	assert(dsty >= 0 && dsty < dstpsd->yres);
	assert(dstw > 0);
	assert(dsth > 0);
	assert(srcpsd->addr != 0);
	assert(srcx >= 0 && srcx < srcpsd->xres);
	assert(srcy >= 0 && srcy < srcpsd->yres);
	assert(srcw > 0);
	assert(srch > 0);
	assert(dstx + dstw <= dstpsd->xres);
	assert(dsty + dsth <= dstpsd->yres);
	assert(srcx + srcw <= srcpsd->xres);
	assert(srcy + srch <= srcpsd->yres);

	DRAWON;
	row_pos = 0x10000;
	row_inc = (srch << 16) / dsth;

	/* stretch blit using integer ratio between src/dst height/width*/
	for (ymax = dsty + dsth; dsty<ymax; ++dsty) {

		/* find source y position*/
		while (row_pos >= 0x10000L) {
			++srcy;
			row_pos -= 0x10000L;
		}

		dst = ((ADDR8)dstpsd->addr) + (dstx + dsty*dlinelen) * 3;
		src = ((ADDR8)srcpsd->addr) + (srcx + (srcy - 1)*slinelen) * 3;

		/* copy a row of pixels*/
		col_pos = 0x10000;
		col_inc = (srcw << 16) / dstw;
		for (i = 0; i<dstw; ++i) {
			/* get source x pixel*/
			while (col_pos >= 0x10000L) {
				b = *src++;
				g = *src++;
				r = *src++;
				col_pos -= 0x10000L;
			}
			*dst++ = b;
			*dst++ = g;
			*dst++ = r;
			col_pos += col_inc;
		}

		row_pos += row_inc;
	}
	DRAWOFF;
}
#endif /* DEPRECATED*/

/*
* This stretchblit code was originally written for the TriMedia
* VLIW CPU.  Therefore it uses RESTRICT pointers, and the special
* one-assembler-opcode pseudo-functions SIGN and ABS.
*
* (The 'restrict' extension is in C99, so for a C99 compiler you
* could "#define RESTRICT restrict" or put
* "CFLAGS += -DRESTRICT=restrict" in the makefile).
*
* Compatibility definitions:
*/
#ifndef RESTRICT
#define RESTRICT
#endif
#ifndef SIGN
#define SIGN(x) (((x) > 0) ? 1 : (((x) == 0) ? 0 : -1))
#endif
#ifndef ABS
#define ABS(x) (((x) >= 0) ? (x) : -(x))
#endif

/* Blit a 24-bit/18-bit image.
* Can stretch the image by any X and/or Y scale factor.
* Can flip the image in the X and/or Y axis.
*
* This is the faster version with no per-pixel multiply and a single
* decision tree for the inner loop, by Jon.  Based on Alex's original
* all-integer version.
*
* Paramaters:
* srf              - Dest surface
* dest_x_start
* dest_y_start    - Top left corner of dest rectangle
* width, height   - Size in dest co-ordinates.
* x_denominator   - Denominator for source X value fractions.  Note that
*                   this must be even, and all the numerators must also be
*                   even, so we can easily divide by 2.
* y_denominator   - Denominator for source Y value fractions.  Note that
*                   this must be even, and all the numerators must also be
*                   even, so we can easily divide by 2.
* src_x_fraction  -
* src_y_fraction  - Point in source that corresponds to the top left corner
*                   of the pixel (dest_x_start, dest_y_start).  This is a
*                   fraction - to get a float, divide by y_denominator.
* x_step_fraction - X step in src for an "x++" step in dest.  May be negative
*                   (for a flip).  Expressed as a fraction - divide it by
*                   x_denominator for a float.
* y_step_fraction - Y step in src for a  "y++" step in dest.  May be negative
*                   (for a flip).  Expressed as a fraction - divide it by
*                   y_denominator for a float.
* image           - Source image.
* op              - Raster operation, currently ignored.
*/
static void
linear18_stretchblitex(PSD dstpsd,
PSD srcpsd,
MWCOORD dest_x_start,
MWCOORD dest_y_start,
MWCOORD width,
MWCOORD height,
int x_denominator,
int y_denominator,
int src_x_fraction,
int src_y_fraction,
int x_step_fraction,
int y_step_fraction,
int op)
{
	/* Pointer to the current pixel in the source image */
	unsigned char *RESTRICT src_ptr;

	/* Pointer to x=xs1 on the next line in the source image */
	unsigned char *RESTRICT next_src_ptr;

	/* Pointer to the current pixel in the dest image */
	unsigned char *RESTRICT dest_ptr;

	/* Pointer to x=xd1 on the next line in the dest image */
	unsigned char *next_dest_ptr;

	/* Keep track of error in the source co-ordinates */
	int x_error;
	int y_error;

	/* 1-unit steps "forward" through the source image, as steps in the image
	* byte array.
	*/
	int src_x_step_one;
	int src_y_step_one;

	/* normal steps "forward" through the source image, as steps in the image
	* byte array.
	*/
	int src_x_step_normal;
	int src_y_step_normal;

	/* 1-unit steps "forward" through the source image, as steps in the image
	* byte array.
	*/
	int x_error_step_normal;
	int y_error_step_normal;

	/* Countdown to the end of the destination image */
	int x_count;
	int y_count;

	/* Start position in source, in whole pixels */
	int src_x_start;
	int src_y_start;

	/* Error values for start X position in source */
	int x_error_start;

	/* 1-unit step down dest, in bytes. */
	int dest_y_step;

	/*DPRINTF("Nano-X: linear18_stretchflipblit( dest=(%d,%d) %dx%d )\n",
	dest_x_start, dest_y_start, width, height);*/

	/* We add half a dest pixel here so we're sampling from the middle of
	* the dest pixel, not the top left corner.
	*/
	src_x_fraction += (x_step_fraction >> 1);
	src_y_fraction += (y_step_fraction >> 1);

	/* Seperate the whole part from the fractions.
	*
	* Also, We need to do lots of comparisons to see if error values are
	* >= x_denominator.  So subtract an extra x_denominator for speed - then
	* we can just check if it's >= 0.
	*/
	src_x_start = src_x_fraction / x_denominator;
	src_y_start = src_y_fraction / y_denominator;
	x_error_start = src_x_fraction - (src_x_start + 1) * x_denominator;
	y_error = src_y_fraction - (src_y_start + 1) * y_denominator;

	/* precalculate various deltas */

	src_x_step_normal = x_step_fraction / x_denominator;
	src_x_step_one = SIGN(x_step_fraction);
	x_error_step_normal = ABS(x_step_fraction) - ABS(src_x_step_normal) * x_denominator;

	src_y_step_normal = y_step_fraction / y_denominator;
	src_y_step_one = SIGN(y_step_fraction) * srcpsd->linelen;
	y_error_step_normal = ABS(y_step_fraction) - ABS(src_y_step_normal) * y_denominator;
	src_y_step_normal *= srcpsd->linelen;

	/* DPRINTF("linear18_stretchblitex: X: One step=%d, err-=%d; normal step=%d, err+=%d\n"
	"Y: One step=%d, err-=%d; normal step=%d, err+=%d\n",
	src_x_step_one, x_denominator, src_x_step_normal, x_error_step_normal,
	src_y_step_one, y_denominator, src_y_step_normal, y_error_step_normal);
	*/

	/* Pointer to the first source pixel */
	next_src_ptr = ((unsigned char *)srcpsd->addr) + 3 * (src_y_start * srcpsd->linelen + src_x_start);

	/* Cache the width of a scanline in dest */
	dest_y_step = dstpsd->linelen;

	/* Pointer to the first dest pixel */
	next_dest_ptr = ((unsigned char *)dstpsd->addr) + 3 * (dest_y_start * dest_y_step + dest_x_start);

	/* Convert from pixels to bytes (only for this 24bpp/18bpp mode) */
	src_x_step_normal *= 3;
	src_x_step_one *= 3;
	src_y_step_normal *= 3;
	src_y_step_one *= 3;

	/*
	* Note: The MWROP_SRC case below is a simple expansion of the
	* default case.  It can be removed without significant speed
	* penalty if you need to reduce code size.
	*
	* The MWROP_CLEAR case could be removed.  But it is a large
	* speed increase for a small quantity of code.
	*/
	switch (op) {
	case MWROP_SRC:
		/* Benchmarking shows that this while loop is faster than the equivalent
		* for loop: for(y_count=0; y_count<height; y_count++) { ... }
		*/
		y_count = height;
		while (y_count-- > 0) {
			src_ptr = next_src_ptr;
			dest_ptr = next_dest_ptr;

			x_error = x_error_start;

			x_count = width;
			while (x_count-- > 0) {
				*dest_ptr++ = src_ptr[0];
				*dest_ptr++ = src_ptr[1];
				*dest_ptr++ = src_ptr[2];

				src_ptr += src_x_step_normal;
				x_error += x_error_step_normal;

				if (x_error >= 0) {
					src_ptr += src_x_step_one;
					x_error -= x_denominator;
				}
			}

			next_dest_ptr += dest_y_step;
			next_src_ptr += src_y_step_normal;
			y_error += y_error_step_normal;

			if (y_error >= 0) {
				next_src_ptr += src_y_step_one;
				y_error -= y_denominator;
			}
		}
		break;

	case MWROP_CLEAR:
		y_count = height;
		while (y_count-- > 0) {
			dest_ptr = next_dest_ptr;
			x_count = width * 3;
			while (x_count-- > 0) {
				*dest_ptr++ = 0;
			}
			next_dest_ptr += dest_y_step;
		}
		break;

	default:
		y_count = height;
		while (y_count-- > 0) {
			src_ptr = next_src_ptr;
			dest_ptr = next_dest_ptr;

			x_error = x_error_start;

			x_count = width;
			while (x_count-- > 0) {
				applyOp(op, src_ptr[0], dest_ptr, ADDR8);
				dest_ptr++;
				applyOp(op, src_ptr[1], dest_ptr, ADDR8);
				dest_ptr++;
				applyOp(op, src_ptr[2], dest_ptr, ADDR8);
				dest_ptr++;

				src_ptr += src_x_step_normal;
				x_error += x_error_step_normal;

				if (x_error >= 0) {
					src_ptr += src_x_step_one;
					x_error -= x_denominator;
				}
			}

			next_dest_ptr += dest_y_step;
			next_src_ptr += src_y_step_normal;
			y_error += y_error_step_normal;

			if (y_error >= 0) {
				next_src_ptr += src_y_step_one;
				y_error -= y_denominator;
			}
		}
		break;

	}
}

#if MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST
/* psd->DrawArea operation PSDOP_BITMAP_BYTES_LSB_FIRST which
* takes a pixmap, each line is byte aligned, and copies it
* to the screen using fg_color and bg_color to replace a 1
* and 0 in the pixmap.  This pixmap is ordered the wrong
* way around; it has the leftmost pixel (on the screen) in
* LSB (Bit 0) of the bytes.
*
* The reason why this non-intuitive bit ordering is used is
* to match the bit ordering used in the T1lib font rendering
* library.
*
* Variables used in the gc:
*       dstx, dsty, dsth, dstw   Destination rectangle
*       srcx, srcy               Source rectangle
*       src_linelen              Linesize in bytes of source
*       pixels                   Pixmap data
*       fg_color                 Color of a '1' bit
*       bg_color                 Color of a '0' bit
*       usebg                 If set, bg_color is used.  If zero,
*                                then '0' bits are transparent.
*/
static void
linear18_drawarea_bitmap_bytes_lsb_first(PSD psd, driver_gc_t * gc)
{
	/*
	* The difference between the MSB_FIRST and LSB_FIRST variants of
	* this function is simply the definition of these three #defines.
	*
	* MWI_IS_BIT_BEFORE_OR_EQUAL(A,B) returns true if bit A is before
	*     (i.e. to the left of) bit B.
	* MWI_ADVANCE_BIT(X) advances X on to the next bit to the right,
	*     and stores the result back in X.
	* MWI_BIT_NO(N), where 0<=n<=7, gives the Nth bit, where 0 is the
	*     leftmost bit and 7 is the rightmost bit.  This is a constant
	*     iff N is a constant.
	*/
#define MWI_IS_BIT_BEFORE_OR_EQUAL(a,b) ((a) <= (b))
#define MWI_ADVANCE_BIT(target) ((target) <<= 1)
#define MWI_BIT_NO(n) (0x01 << (n))

	/*
	* Two convenience defines - these are the same for MSB_FIRST and
	* LSB_FIRST.
	*/
#define MWI_FIRST_BIT MWI_BIT_NO(0)
#define MWI_LAST_BIT  MWI_BIT_NO(7)

	unsigned char prefix_first_bit;
	unsigned char postfix_first_bit = MWI_FIRST_BIT;
	unsigned char postfix_last_bit;
	unsigned char bitmap_byte;
	unsigned char mask;
	uint32_t fg, bg;
	unsigned char fg_r, fg_g, fg_b, bg_r, bg_g, bg_b;
	int first_byte, last_byte;
	int size_main;
	int t, y;
	unsigned int advance_src, advance_dst;
	ADDR8 src;
	ADDR8 dst;

	/* The bit in the first byte, which corresponds to the leftmost pixel. */
	prefix_first_bit = MWI_BIT_NO(gc->srcx & 7);

	/* The bit in the last byte, which corresponds to the rightmost pixel. */
	postfix_last_bit = MWI_BIT_NO((gc->srcx + gc->width - 1) & 7);

	/* The index into each scanline of the first byte to use. */
	first_byte = gc->srcx >> 3;

	/* The index into each scanline of the last byte to use. */
	last_byte = (gc->srcx + gc->width - 1) >> 3;

	src = ((ADDR8)gc->data) + gc->src_linelen * gc->srcy + first_byte;
	dst = ((ADDR8)psd->addr) + (psd->linelen * gc->dsty + gc->dstx) * 3;

	fg = gc->fg_color;
	convert24_888to24_666(&fg);
	fg_r = PIXEL888RED(fg);
	fg_g = PIXEL888GREEN(fg);
	fg_b = PIXEL888BLUE(fg);
	bg = gc->bg_color;
	convert24_888to24_666(&bg);
	bg_r = PIXEL888RED(bg);
	bg_g = PIXEL888GREEN(bg);
	bg_b = PIXEL888BLUE(bg);
	advance_src = gc->src_linelen - last_byte + first_byte - 1;
	advance_dst = (psd->linelen - gc->width) * 3;

	if (first_byte != last_byte) {
		/* The total number of bytes to use, less the two special-cased
		* bytes (first and last).
		*/
		size_main = last_byte - first_byte + 1 - 2;

		if (prefix_first_bit == MWI_FIRST_BIT) {
			/* No need to special case. */
			prefix_first_bit = 0;
			size_main++;
		}
		if (postfix_last_bit == MWI_LAST_BIT) {
			/* No need to special case. */
			postfix_last_bit = 0;
			size_main++;
		}
	}
	else if ((prefix_first_bit == MWI_FIRST_BIT) && (postfix_last_bit == MWI_LAST_BIT)) {
		/* Exactly one byte wide. */
		prefix_first_bit = 0;
		postfix_last_bit = 0;
		size_main = 1;
	}
	else {
		/* Very narrow pixmap, fits in single first byte. */
		/* Do everything in 'postfix' loop. */
		postfix_first_bit = prefix_first_bit;
		prefix_first_bit = 0;
		size_main = 0;
	}

	DRAWON;

	if (gc->usebg) {
		for (y = 0; y < gc->height; y++) {

			/* Do pixels of partial first byte */
			if (prefix_first_bit) {
				bitmap_byte = *src++;
				for (mask = prefix_first_bit; mask; MWI_ADVANCE_BIT(mask)) {
					if (mask & bitmap_byte) {
						*dst++ = fg_b;
						*dst++ = fg_g;
						*dst++ = fg_r;
					}
					else {
						*dst++ = bg_b;
						*dst++ = bg_g;
						*dst++ = bg_r;
					}
				}
			}

			/* Do all pixels of main part one byte at a time */
			for (t = size_main; t != 0; t--) {
				bitmap_byte = *src++;

				if (MWI_BIT_NO(0) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(1) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(2) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(3) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(4) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(5) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(6) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(7) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
			}

			/* Do last few bits of line */
			if (postfix_last_bit) {
				bitmap_byte = *src++;
				for (mask = postfix_first_bit;
					MWI_IS_BIT_BEFORE_OR_EQUAL(mask, postfix_last_bit); MWI_ADVANCE_BIT(mask)) {
					if (mask & bitmap_byte) {
						*dst++ = fg_b;
						*dst++ = fg_g;
						*dst++ = fg_r;
					}
					else {
						*dst++ = bg_b;
						*dst++ = bg_g;
						*dst++ = bg_r;
					}
				}
			}

			src += advance_src;
			dst += advance_dst;
		}
	}
	else {
		for (y = 0; y < gc->height; y++) {

			/* Do pixels of partial first byte */
			if (prefix_first_bit) {
				bitmap_byte = *src++;
				for (mask = prefix_first_bit; mask; MWI_ADVANCE_BIT(mask)) {
					if (mask & bitmap_byte) {
						dst[0] = fg_b;
						dst[1] = fg_g;
						dst[2] = fg_r;
					}
					dst += 3;
				}
			}

			/* Do all pixels of main part one byte at a time */
			for (t = size_main; t != 0; t--) {
				bitmap_byte = *src++;

				if (MWI_BIT_NO(0) & bitmap_byte) {
					dst[0 * 3 + 0] = fg_b;
					dst[0 * 3 + 1] = fg_g;
					dst[0 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(1) & bitmap_byte) {
					dst[1 * 3 + 0] = fg_b;
					dst[1 * 3 + 1] = fg_g;
					dst[1 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(2) & bitmap_byte) {
					dst[2 * 3 + 0] = fg_b;
					dst[2 * 3 + 1] = fg_g;
					dst[2 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(3) & bitmap_byte) {
					dst[3 * 3 + 0] = fg_b;
					dst[3 * 3 + 1] = fg_g;
					dst[3 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(4) & bitmap_byte) {
					dst[4 * 3 + 0] = fg_b;
					dst[4 * 3 + 1] = fg_g;
					dst[4 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(5) & bitmap_byte) {
					dst[5 * 3 + 0] = fg_b;
					dst[5 * 3 + 1] = fg_g;
					dst[5 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(6) & bitmap_byte) {
					dst[6 * 3 + 0] = fg_b;
					dst[6 * 3 + 1] = fg_g;
					dst[6 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(7) & bitmap_byte) {
					dst[7 * 3 + 0] = fg_b;
					dst[7 * 3 + 1] = fg_g;
					dst[7 * 3 + 2] = fg_r;
				}
				dst += 8 * 3;
			}

			/* Do last few bits of line */
			if (postfix_last_bit) {
				bitmap_byte = *src++;
				for (mask = postfix_first_bit;
					MWI_IS_BIT_BEFORE_OR_EQUAL(mask, postfix_last_bit); MWI_ADVANCE_BIT(mask)) {
					if (mask & bitmap_byte) {
						dst[0] = fg_b;
						dst[1] = fg_g;
						dst[2] = fg_r;
					}
					dst += 3;
				}
			}

			src += advance_src;
			dst += advance_dst;
		}
	}

	DRAWOFF;

#undef MWI_IS_BIT_BEFORE_OR_EQUAL
#undef MWI_ADVANCE_BIT
#undef MWI_BIT_NO
#undef MWI_FIRST_BIT
#undef MWI_LAST_BIT
}
#endif /* MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST */

#if MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST
/* psd->DrawArea operation PSDOP_BITMAP_BYTES_MSB_FIRST which
* takes a pixmap, each line is byte aligned, and copies it
* to the screen using fg_color and bg_color to replace a 1
* and 0 in the pixmap.
*
* The bitmap is ordered how you'd expect, with the MSB used
* for the leftmost of the 8 pixels controlled by each byte.
*
* Variables used in the gc:
*       dstx, dsty, height, width   Destination rectangle
*       srcx, srcy               Source rectangle
*       src_linelen              Linesize in bytes of source
*       data                   Pixmap data
*       fg_color                 Color of a '1' bit
*       bg_color                 Color of a '0' bit
*       usebg                 If set, bg_color is used.  If zero,
*                                then '0' bits are transparent.
*/
static void
linear18_drawarea_bitmap_bytes_msb_first(PSD psd, driver_gc_t * gc)
{
	/*
	* The difference between the MSB_FIRST and LSB_FIRST variants of
	* this function is simply the definition of these three #defines.
	*
	* MWI_IS_BIT_BEFORE_OR_EQUAL(A,B) returns true if bit A is before
	*     (i.e. to the left of) bit B.
	* MWI_ADVANCE_BIT(X) advances X on to the next bit to the right,
	*     and stores the result back in X.
	* MWI_BIT_NO(N), where 0<=n<=7, gives the Nth bit, where 0 is the
	*     leftmost bit and 7 is the rightmost bit.  This is a constant
	*     iff N is a constant.
	*/
#define MWI_IS_BIT_BEFORE_OR_EQUAL(a,b) ((a) >= (b))
#define MWI_ADVANCE_BIT(target) ((target) >>= 1)
#define MWI_BIT_NO(n) (0x80 >> (n))

	/*
	* Two convenience defines - these are the same for MSB_FIRST and
	* LSB_FIRST.
	*/
#define MWI_FIRST_BIT MWI_BIT_NO(0)
#define MWI_LAST_BIT  MWI_BIT_NO(7)

	unsigned char prefix_first_bit;
	unsigned char postfix_first_bit = MWI_FIRST_BIT;
	unsigned char postfix_last_bit;
	unsigned char bitmap_byte;
	unsigned char mask;
	uint32_t fg, bg;
	unsigned char fg_r, fg_g, fg_b, bg_r, bg_g, bg_b;
	int first_byte, last_byte;
	int size_main;
	int t, y;
	unsigned int advance_src, advance_dst;
	ADDR8 src;
	ADDR8 dst;

	/* The bit in the first byte, which corresponds to the leftmost pixel. */
	prefix_first_bit = MWI_BIT_NO(gc->srcx & 7);

	/* The bit in the last byte, which corresponds to the rightmost pixel. */
	postfix_last_bit = MWI_BIT_NO((gc->srcx + gc->width - 1) & 7);

	/* The index into each scanline of the first byte to use. */
	first_byte = gc->srcx >> 3;

	/* The index into each scanline of the last byte to use. */
	last_byte = (gc->srcx + gc->width - 1) >> 3;

	src = ((ADDR8)gc->data) + gc->src_linelen * gc->srcy + first_byte;
	dst = ((ADDR8)psd->addr) + (psd->linelen * gc->dsty + gc->dstx) * 3;
	fg = gc->fg_color;
	convert24_888to24_666(&fg);
	fg_r = PIXEL888RED(fg);
	fg_g = PIXEL888GREEN(fg);
	fg_b = PIXEL888BLUE(fg);
	bg = gc->bg_color;
	convert24_888to24_666(&bg);
	bg_r = PIXEL888RED(bg);
	bg_g = PIXEL888GREEN(bg);
	bg_b = PIXEL888BLUE(bg);
	advance_src = gc->src_linelen - last_byte + first_byte - 1;
	advance_dst = (psd->linelen - gc->width) * 3;

	if (first_byte != last_byte) {
		/* The total number of bytes to use, less the two special-cased
		* bytes (first and last).
		*/
		size_main = last_byte - first_byte + 1 - 2;

		if (prefix_first_bit == MWI_FIRST_BIT) {
			/* No need to special case. */
			prefix_first_bit = 0;
			size_main++;
		}
		if (postfix_last_bit == MWI_LAST_BIT) {
			/* No need to special case. */
			postfix_last_bit = 0;
			size_main++;
		}
	}
	else if ((prefix_first_bit == MWI_FIRST_BIT) && (postfix_last_bit == MWI_LAST_BIT)) {
		/* Exactly one byte wide. */
		prefix_first_bit = 0;
		postfix_last_bit = 0;
		size_main = 1;
	}
	else {
		/* Very narrow pixmap, fits in single first byte. */
		/* Do everything in 'postfix' loop. */
		postfix_first_bit = prefix_first_bit;
		prefix_first_bit = 0;
		size_main = 0;
	}

	DRAWON;

	if (gc->usebg) {
		for (y = 0; y < gc->height; y++) {

			/* Do pixels of partial first byte */
			if (prefix_first_bit) {
				bitmap_byte = *src++;
				for (mask = prefix_first_bit; mask; MWI_ADVANCE_BIT(mask)) {
					if (mask & bitmap_byte) {
						*dst++ = fg_b;
						*dst++ = fg_g;
						*dst++ = fg_r;
					}
					else {
						*dst++ = bg_b;
						*dst++ = bg_g;
						*dst++ = bg_r;
					}
				}
			}

			/* Do all pixels of main part one byte at a time */
			for (t = size_main; t != 0; t--) {
				bitmap_byte = *src++;

				if (MWI_BIT_NO(0) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(1) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(2) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(3) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(4) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(5) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(6) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
				if (MWI_BIT_NO(7) & bitmap_byte) {
					*dst++ = fg_b;
					*dst++ = fg_g;
					*dst++ = fg_r;
				}
				else {
					*dst++ = bg_b;
					*dst++ = bg_g;
					*dst++ = bg_r;
				}
			}

			/* Do last few bits of line */
			if (postfix_last_bit) {
				bitmap_byte = *src++;
				for (mask = postfix_first_bit;
					MWI_IS_BIT_BEFORE_OR_EQUAL(mask, postfix_last_bit); MWI_ADVANCE_BIT(mask)) {
					if (mask & bitmap_byte) {
						*dst++ = fg_b;
						*dst++ = fg_g;
						*dst++ = fg_r;
					}
					else {
						*dst++ = bg_b;
						*dst++ = bg_g;
						*dst++ = bg_r;
					}
				}
			}

			src += advance_src;
			dst += advance_dst;
		}
	}
	else {
		for (y = 0; y < gc->height; y++) {

			/* Do pixels of partial first byte */
			if (prefix_first_bit) {
				bitmap_byte = *src++;
				for (mask = prefix_first_bit; mask; MWI_ADVANCE_BIT(mask)) {
					if (mask & bitmap_byte) {
						dst[0] = fg_b;
						dst[1] = fg_g;
						dst[2] = fg_r;
					}
					dst += 3;
				}
			}

			/* Do all pixels of main part one byte at a time */
			for (t = size_main; t != 0; t--) {
				bitmap_byte = *src++;

				if (MWI_BIT_NO(0) & bitmap_byte) {
					dst[0 * 3 + 0] = fg_b;
					dst[0 * 3 + 1] = fg_g;
					dst[0 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(1) & bitmap_byte) {
					dst[1 * 3 + 0] = fg_b;
					dst[1 * 3 + 1] = fg_g;
					dst[1 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(2) & bitmap_byte) {
					dst[2 * 3 + 0] = fg_b;
					dst[2 * 3 + 1] = fg_g;
					dst[2 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(3) & bitmap_byte) {
					dst[3 * 3 + 0] = fg_b;
					dst[3 * 3 + 1] = fg_g;
					dst[3 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(4) & bitmap_byte) {
					dst[4 * 3 + 0] = fg_b;
					dst[4 * 3 + 1] = fg_g;
					dst[4 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(5) & bitmap_byte) {
					dst[5 * 3 + 0] = fg_b;
					dst[5 * 3 + 1] = fg_g;
					dst[5 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(6) & bitmap_byte) {
					dst[6 * 3 + 0] = fg_b;
					dst[6 * 3 + 1] = fg_g;
					dst[6 * 3 + 2] = fg_r;
				}
				if (MWI_BIT_NO(7) & bitmap_byte) {
					dst[7 * 3 + 0] = fg_b;
					dst[7 * 3 + 1] = fg_g;
					dst[7 * 3 + 2] = fg_r;
				}
				dst += 8 * 3;
			}

			/* Do last few bits of line */
			if (postfix_last_bit) {
				bitmap_byte = *src++;
				for (mask = postfix_first_bit;
					MWI_IS_BIT_BEFORE_OR_EQUAL(mask, postfix_last_bit); MWI_ADVANCE_BIT(mask)) {
					if (mask & bitmap_byte) {
						dst[0] = fg_b;
						dst[1] = fg_g;
						dst[2] = fg_r;
					}
					dst += 3;
				}
			}

			src += advance_src;
			dst += advance_dst;
		}
	}

	DRAWOFF;

#undef MWI_IS_BIT_BEFORE_OR_EQUAL
#undef MWI_ADVANCE_BIT
#undef MWI_BIT_NO
#undef MWI_FIRST_BIT
#undef MWI_LAST_BIT
}
#endif /* MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST */

#if MW_FEATURE_PSDOP_ALPHACOL
static void
linear18_drawarea_alphacol(PSD psd, driver_gc_t * gc)
{
	ADDR8 dst;
	ADDR8 alpha;
	uint32_t ps, pd;
	int as;
	int psr, psg, psb;
	int x, y;
	int src_row_step, dst_row_step;

	alpha = ((ADDR8)gc->data) + gc->src_linelen * gc->srcy + gc->srcx;
	dst = ((ADDR8)psd->addr) + (psd->linelen * gc->dsty + gc->dstx) * 3;
	ps = gc->fg_color;
	convert24_888to24_666(&ps);
	psr = PIXEL888RED(ps);
	psg = PIXEL888GREEN(ps);
	psb = PIXEL888BLUE(ps);
	src_row_step = gc->src_linelen - gc->width;
	dst_row_step = (psd->linelen - gc->width) * 3;

	DRAWON;
	for (y = 0; y < gc->height; y++) {
		for (x = 0; x < gc->width; x++) {
			if ((as = *alpha++) == 255) {
				*dst++ = psb;
				*dst++ = psg;
				*dst++ = psr;
			}
			else if (as != 0) {
				pd = *dst;
				*dst++ = muldiv255(as, psb - pd) + pd;
				pd = *dst;
				*dst++ = muldiv255(as, psg - pd) + pd;
				pd = *dst;
				*dst++ = muldiv255(as, psr - pd) + pd;
			}
			else if (gc->usebg)	{	/* alpha 0 - draw bkgnd*/
				*dst++ = PIXEL888BLUE(gc->bg_color);
				*dst++ = PIXEL888GREEN(gc->bg_color);
				*dst++ = PIXEL888RED(gc->bg_color);
			}
			else
				dst += 3;
		}
		alpha += src_row_step;
		dst += dst_row_step;
	}
	DRAWOFF;
}
#endif /* MW_FEATURE_PSDOP_ALPHACOL */

static void
linear18_drawarea(PSD psd, driver_gc_t * gc)
{
	assert(psd->addr != 0);
	/*assert(gc->width <= gc->srcw); */
	assert(gc->dstx >= 0 && gc->dstx + gc->width <= psd->xres);
	/*assert(gc->dsty >= 0 && gc->dsty+gc->height <= psd->yres); */
	/*assert(gc->srcx >= 0 && gc->srcx+gc->width <= gc->srcw); */
	assert(gc->srcy >= 0);
	/*DPRINTF("linear32_drawarea op=%d dstx=%d dsty=%d\n", op, gc->dstx, gc->dsty);*/

	switch (gc->op) {
#if MW_FEATURE_PSDOP_ALPHACOL
	case PSDOP_ALPHACOL:
		linear18_drawarea_alphacol(psd, gc);
		break;
#endif

#if MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST
	case PSDOP_BITMAP_BYTES_LSB_FIRST:
		linear18_drawarea_bitmap_bytes_lsb_first(psd, gc);
		break;
#endif

#if MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST
	case PSDOP_BITMAP_BYTES_MSB_FIRST:
		linear18_drawarea_bitmap_bytes_msb_first(psd, gc);
		break;
#endif

	}
}

SUBDRIVER fblinear18 = {
	linear18_init,
	linear18_drawpixel,
	linear18_readpixel,
	linear18_drawhorzline,
	linear18_drawvertline,
	gen_fillrect,
	linear18_blit,
	linear18_drawarea,
	linear18_stretchblitex
};

