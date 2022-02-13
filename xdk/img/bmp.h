/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc bitmap document

	@module	bmp.h | interface file

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
#ifndef _BMP_H
#define _BMP_H

#include "../xdkdef.h"

typedef struct _bitmap_file_head_t{
	/*file info header*/
	unsigned short flag; /*file flag*/
	unsigned int fsize; /*file size*/
	unsigned short rev1;
	unsigned short rev2;
	unsigned int offset; /*image bytes position*/
}bitmap_file_head_t;

typedef struct _bitmap_info_head_t{
	/*bmp info header*/
	unsigned int isize; /*bitmap info struct size, set to 40 bytes*/
	unsigned int width; /*bitmap cols*/
	unsigned int height; /*bitmap rows*/
	unsigned short planes; /*number of planes for the target device, set to 1*/
	unsigned short clrbits; /*number of bits-per-pixel*/
	unsigned int compress; /*type of compression, set to 0*/
	unsigned int bytes; /*image bytes*/
	unsigned int xpelsperm; /*the horizontal resolution, in pixels-per-meter*/
	unsigned int ypelsperm; /*the vertical resolution, in pixels-per-meter*/
	unsigned int clrused; /*the number of color indexes in the color table*/
	unsigned int clrimport; /*the number of color indexes that are required for displaying the bitmap*/
}bitmap_info_head_t;

typedef struct _bitmap_quad_t{
	unsigned char blue;		//blue lighten(0-255)
	unsigned char green;		//green lighten(0-255)
	unsigned char red;			//red lighten(0-255)
	unsigned char reserved;	//set to zero
}bitmap_quad_t;

#define BMP_FLAG		0x4d42 /*"B","M"*/
#define BMP_FILEHEADER_SIZE		14
#define BMP_INFOHEADER_SIZE		40
#define BMP_RGBQUAD_SIZE		4

#define BMPFILEHEADERPTR(p)			((unsigned char*)p)
#define BMPINFOHEADERPTR(p)			((unsigned char*)p + BMP_FILEHEADER_SIZE)
#define BMPQUADHEADERPTR(p)			((unsigned char*)p + BMP_FILEHEADER_SIZE + BMP_INFOHEADER_SIZE)

#define BMP_FILL_QUAD(p, a, r, g, b) do{PUT_BYTE(p, 0, b);PUT_BYTE(p, 1, g);PUT_BYTE(p, 2, r);PUT_BYTE(p, 3, a);}while(0);
#define BMP_FILL_RGB(p, r, g, b) do{PUT_BYTE(p, 0, b);PUT_BYTE(p, 1, g);PUT_BYTE(p, 2, r);}while(0);

#define RGB_GRAY(r,g,b) (unsigned char)(0.299 * (double)r + 0.587 * (double)g + 0.114 * (double)b)

#define BMP_PIXEL_SIZE(width, depth)	(((width * depth + 31) & ~31) / 8)

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API dword_t xbmp_set_head(const bitmap_file_head_t* pbf, unsigned char* buf, unsigned int max);

	EXP_API dword_t xbmp_get_head(bitmap_file_head_t* pbf, const unsigned char* src, unsigned int len);

	EXP_API dword_t xbmp_set_info(const bitmap_info_head_t* pbi, unsigned char* buf, unsigned int max);

	EXP_API dword_t xbmp_get_info(bitmap_info_head_t* pbi, const unsigned char* src, unsigned int len);

	EXP_API dword_t xbmp_fill_quad(int clrbits, int clrused, unsigned char* buf, unsigned int max);

	EXP_API bool_t xbmp_get_size(xsize_t* ps, const unsigned char* src, unsigned int len);
	
	EXP_API bool_t xbmp_get_rgb(xcolor_t* pc, int row, int col, const unsigned char* src, unsigned int len);

	EXP_API	dword_t xbmp_convgray(const byte_t* src, dword_t len, byte_t* buf, dword_t max);

	EXP_API	dword_t xbmp_convbina(const byte_t* src, dword_t len, byte_t* buf, dword_t max);

#ifdef	__cplusplus
}
#endif


#endif /*OEMBMP_H*/