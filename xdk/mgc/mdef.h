/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory context document

	@module	mdef.h | interface file

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

#ifndef _MDEF_H
#define _MDEF_H

#include "../xdkdef.h"

/*the frame buffer pixel unit*/
typedef byte_t*		ADDR8;
typedef sword_t*	ADDR16;
typedef dword_t*	ADDR32;


#define PIXEL_DEPTH_PALETTE1	1	/* pixel is packed 1 bits with 2 palette index*/
#define PIXEL_DEPTH_PALETTE2	2	/* pixel is packed 2 bits with 4 palette index*/
#define PIXEL_DEPTH_PALETTE4	4	/* pixel is packed 4 bits with 16 palette index*/
#define PIXEL_DEPTH_PALETTE8	8	/* pixel is packed 8 bits with 256 palette index*/
#define PIXEL_DEPTH_COLOR16		16	/* pixel is packed 16 bits 5/5/5 RGB truecolor*/
#define PIXEL_DEPTH_COLOR24		24	/* pixel is packed 24 bits R/G/B RGB truecolor*/
#define PIXEL_DEPTH_COLOR32		32	/* pixel is packed 32 bits A/R/G/B ARGB truecolor with alpha */

typedef dword_t PIXELVAL;

#define PUT_PIXVAL(a,r,g,b)	((dword_t)a | ((dword_t)r << 8) | ((dword_t)g << 16) | ((dword_t)b << 24))
#define GET_PIXVAL_B(c)		(unsigned char)(((dword_t)c >> 24) & 0xFF) 
#define GET_PIXVAL_G(c)		(unsigned char)(((dword_t)c >> 16) & 0xFF) 
#define GET_PIXVAL_R(c)		(unsigned char)(((dword_t)c >> 8) & 0xFF) 
#define GET_PIXVAL_A(c)		(unsigned char)(c & 0xFF) 

typedef enum{
	ROP_COPY = 0,	/* src*/
	ROP_XOR	= 1,	/* src ^ dst*/
	ROP_OR	= 2,	/* src | dst*/
	ROP_AND	= 3,	/* src & dst*/
	ROP_CLR = 4,	/* 0*/
	ROP_SET	= 5,	/* ~0, was ROP_SETTO1*/
	ROP_EQUIV =	6,	/* ~(src ^ dst)*/
	ROP_NOR = 7,	/* ~(src | dst)*/
	ROP_NAND = 8,	/* ~(src & dst)*/
	ROP_INVERT = 9,	/* ~dst*/
	ROP_COPYINVERTED = 10,	/* ~src*/
	ROP_ORINVERTED = 11,	/* ~src | dst*/
	ROP_ANDINVERTED	= 12,	/* ~src & dst*/
	ROP_ORREVERSE = 13,	/* src | ~dst*/
	ROP_ANDREVERSE = 14,	/* src & ~dst*/
	ROP_NOOP = 15,	/* dst*/
	ROP_XOR_FGBG = 16,	/* src ^ background ^ dst (Java XOR mode)*/
	ROP_BLENDCONSTANT = 32,	/* alpha blend src -> dst with constant alpha*/
	ROP_BLENDFGBG = 33	/* alpha blend fg/bg color -> dst with src alpha channel*/
}RASTER_MODE;

#define RASTER_COPY(dst, src)	(src)
#define RASTER_XOR(dst, src)	(dst ^ src)
#define RASTER_OR(dst, src)		(dst | src)
#define RASTER_AND(dst, src)	(dst & src)
#define RASTER_CLR(dst)			(dst & 0)


#endif /*_MDEF_H*/
