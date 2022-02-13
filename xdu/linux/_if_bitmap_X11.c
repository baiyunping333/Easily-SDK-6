/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc bitmap document

	@module	if_bitmap_x11.c | linux implement file

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

#ifdef XDU_SUPPORT_CONTEXT_BITMAP


static void _CenterRect(XRectangle* pRect, int src_width, int src_height)
{
	if (pRect->width > (unsigned short)src_width)
	{
		pRect->x = pRect->x + (pRect->width - (unsigned short)src_width) / 2;
		pRect->width = (unsigned short)src_width;
	}
	if (pRect->height > (unsigned short)src_height)
	{
		pRect->y = pRect->y + (pRect->height - (unsigned short)src_height) / 2;
		pRect->height = (unsigned short)src_height;
	}
}

void _destroy_bitmap(bitmap_t rbm)
{
    X11_bitmap_t* bmp = (X11_bitmap_t*)rbm;
    
	XDestroyImage(bmp->image);
	free(bmp);
}

void _get_bitmap_size(bitmap_t rb, int* pw, int* ph)
{
   X11_bitmap_t* bmp = (X11_bitmap_t*)bmp;

    if(pw) *pw = bmp->image->width;
    
    if(ph) *ph = bmp->image->height;
}

bitmap_t _create_context_bitmap(visual_t rdc)
{
	X11_context_t* ctx = (X11_context_t*)rdc;
	X11_bitmap_t* bmp;

	bmp = (X11_bitmap_t*)xmem_alloc(sizeof(X11_bitmap_t));
	
	bmp->image = XGetImage(g_display, ctx->device, 0, 0, ctx->width, ctx->height, AllPlanes, ZPixmap);

	return &(bmp->head);
}

bitmap_t _create_color_bitmap(visual_t rdc, const xcolor_t* pxc, int w, int h)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

    int row,col;
	unsigned long pix;
    X11_bitmap_t* bmp;

	bmp = (X11_bitmap_t*)xmem_alloc(sizeof(X11_bitmap_t));

    bmp->image = (XImage*)calloc(1, sizeof(XImage));
	
	bmp->image->width = w;
	bmp->image->height = h;
	bmp->image->xoffset = 0;
	bmp->image->format = ZPixmap;
	bmp->image->data = (char*)calloc(1, w * h * 4);
	bmp->image->byte_order = LSBFirst;
	bmp->image->bitmap_unit = 32;
	bmp->image->bitmap_bit_order = LSBFirst;
	bmp->image->bitmap_pad = 32;
	bmp->image->depth = ctx->depth;
	bmp->image->bytes_per_line = w * 4;
	bmp->image->bits_per_pixel = 32;
	bmp->image->red_mask = ctx->visual->red_mask;
	bmp->image->green_mask = ctx->visual->green_mask;
	bmp->image->blue_mask = ctx->visual->blue_mask;

	XInitImage(bmp->image);
    
	pix = 0;
	pix |= ((pxc->r << 16) & bmp->image->red_mask);
	pix |= ((pxc->g << 8) & bmp->image->green_mask);
	pix |= ((pxc->b) & bmp->image->blue_mask);
	
    for(row = 0; row < h; row ++)
    {
        for(col = 0; col < w; col++)
        {
			XPutPixel(bmp->image, col, row, pix);
        }
    }

    return &(bmp->head);
}

bitmap_t _create_pattern_bitmap(visual_t rdc, const xcolor_t* pxc_front, const xcolor_t* pxc_back, int w, int h, const tchar_t* lay)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

    return NULL;
}

bitmap_t _create_gradient_bitmap(visual_t rdc, const xcolor_t* pxc_near, const xcolor_t* pxc_center, int w, int h, const tchar_t* type)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	return NULL;
}

bitmap_t _create_code128_bitmap(visual_t rdc, int w, int h, const unsigned char* bar_buf, dword_t bar_len, const tchar_t* text)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	return NULL;
}

bitmap_t _create_pdf417_bitmap(visual_t rdc, int w, int h, const unsigned char* bar_buf, dword_t bar_len, int rows, int cols)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	return NULL;
}

bitmap_t _create_qrcode_bitmap(visual_t rdc, int w, int h, const unsigned char* bar_buf, dword_t bar_len, int rows, int cols)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	return NULL;
} 

bitmap_t _create_storage_bitmap(visual_t rdc, const tchar_t* fname)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	struct stat st = {0};
	int fd = 0;

	tchar_t itype[10] = {0};
	int len;
	byte_t* file_buf;
	dword_t file_len;
	byte_t* bmp_buf;
	dword_t bmp_len;

	xsize_t xs = {0};
	xcolor_t xc = {0};
	int i, j;
	unsigned long pix;

	X11_bitmap_t* bmp;

    if(stat(fname, &st) < 0)
        return 0;

	len = xslen(fname);
	if(len < 4) return (bitmap_t)0;

	if (xsicmp((fname + len - 4), _T(".jpg")) == 0)
	{
		xscpy(itype, GDI_ATTR_IMAGE_TYPE_JPG);
	}
	else if (xsicmp((fname + len - 4), _T(".png")) == 0)
	{
		xscpy(itype, GDI_ATTR_IMAGE_TYPE_PNG);
	}
	else if (xsicmp((fname + len - 4), _T(".bmp")) == 0)
	{
		xscpy(itype, GDI_ATTR_IMAGE_TYPE_BMP);
	}
	else
		return 0;

	if(stat(fname, &st) != 0)
		return 0;

	fd = open(fname, O_RDONLY, S_IRWXU | S_IXGRP | S_IROTH | S_IXOTH);
    if(fd < 0)
        return 0;

	file_len = (dword_t)(st.st_size);
	file_buf = (byte_t*)calloc(1, file_len);

	len = (int)read(fd, file_buf, file_len);
	if(len < 0)
	{
		close(fd);
		free(file_buf);
		return 0;
	}

	close(fd);

	if (xsicmp(itype, GDI_ATTR_IMAGE_TYPE_JPG) == 0)
	{
		bmp_len = xjpg_decompress(file_buf, file_len, NULL, MAX_LONG);
		if (!bmp_len)
		{
			free(file_buf);
			return 0;
		}

		bmp_buf = (byte_t*)calloc(1, bmp_len);

		xjpg_decompress(file_buf, file_len, bmp_buf, bmp_len);

		free(file_buf);
	}
	else if (xsicmp(itype, GDI_ATTR_IMAGE_TYPE_PNG) == 0)
	{
		bmp_len = xpng_decompress(file_buf, file_len, NULL, MAX_LONG);
		if (!bmp_len)
		{
			free(file_buf);
			return 0;
		}

		bmp_buf = (byte_t*)calloc(1, bmp_len);

		xpng_decompress(file_buf, file_len, bmp_buf, bmp_len);

		free(file_buf);
	}
	else if (xsicmp(itype, GDI_ATTR_IMAGE_TYPE_BMP) == 0)
	{
		bmp_buf = file_buf;
		bmp_len = file_len;
	}

	if(!xbmp_get_size(&xs, bmp_buf, bmp_len))
	{
		free(bmp_buf);
		return 0;
	}

	bmp = (X11_bitmap_t*)xmem_alloc(sizeof(X11_bitmap_t));
	bmp->image = (XImage*)calloc(1, sizeof(XImage));
	
	bmp->image->width = xs.w;
	bmp->image->height = xs.h;
	bmp->image->xoffset = 0;
	bmp->image->format = ZPixmap;
	bmp->image->data = (char*)calloc(1, xs.w * xs.h * 4);
	bmp->image->byte_order = LSBFirst;
	bmp->image->bitmap_unit = 32;
	bmp->image->bitmap_bit_order = LSBFirst;
	bmp->image->bitmap_pad = 32;
	bmp->image->depth = ctx->depth;
	bmp->image->bytes_per_line = xs.w * 4;
	bmp->image->bits_per_pixel = 32;
	bmp->image->red_mask = ctx->visual->red_mask;
	bmp->image->green_mask = ctx->visual->green_mask;
	bmp->image->blue_mask = ctx->visual->blue_mask;

	XInitImage(bmp->image);
	
	for(i=0;i<xs.h;i++)
	{
		for(j=0;j<xs.w;j++)
		{
			xbmp_get_rgb(&xc, i, j, bmp_buf, bmp_len);

			pix = 0;
			pix |= ((xc.r << 16) & bmp->image->red_mask);
			pix |= ((xc.g << 8) & bmp->image->green_mask);
			pix |= ((xc.b) & bmp->image->blue_mask);

			XPutPixel(bmp->image, j, i, pix);
		}
	}

	free(bmp_buf);

	return (bitmap_t)&(bmp->head);
}

//ZPixmap width * height * ((depth + 7) / 8) width * ((depth + 7) / 8)  
//XYPixmap ((width + 7) / 8) * height * depth (width + 7) / 8  
//XYBitmap ((width + 7) / 8) * height * 1   (width + 7) / 8  
/*******************************************************************************/
#pragma pack (2)
typedef struct _bitmap_filehead_t
{
    unsigned short type;		//bitmap file type
    unsigned int size;		//bitmap file size
    unsigned short reserved1;
    unsigned short reserved2;
    unsigned int offset;		//bitmap data offset from file header
}bitmap_filehead_t;		//14 bytes
#pragma pack ()

typedef struct _bitmap_infohead_t{
    unsigned int size;		//struct size
    unsigned int width;		//bitmap point width
    unsigned int height;		//bitmap point height
    unsigned short planes;		//number of planes for the target device, set to 1
    unsigned short bitcount;	//the number of bits-per-pixel. 1:is monochrome; 4:maximum of 16 colors; 8:maximum of 256 colors; 16:maximum of 2^16 colors; 24~; 32~;
    unsigned int compression; //type of compression.0: uncompressed format; 1: RLE format for bitmaps with 8 bpp; 2:RLE format for bitmaps with 4 bpp.
    unsigned int imagesize;	// the size, in bytes, of the image
    unsigned int horzpixels;	//the horizontal resolution, in pixels-per-meter
    unsigned int vertpixels;	//the vertical resolution, in pixels-per-meter
    unsigned int clrused;	// the number of color indexes in the color table  that are actually used by the bitmap
    unsigned int clrimportant;//the number of color indexes that are required for displaying the bitmap
}bitmap_infohead_t;			//40 bytes

typedef struct _bitmap_rgbquad_t{
    unsigned char blue;		//blue lighten(0-255)
    unsigned char green;		//green lighten(0-255)
    unsigned char red;			//red lighten(0-255)
    unsigned char reserved;	//set to zero
}bitmap_rgbquad_t;


dword_t _get_bitmap_bytes(bitmap_t rb)
{
    X11_bitmap_t* bmp = (X11_bitmap_t*)rb;
    
	unsigned short cClrBits;
	unsigned int dwClrUsed;
	unsigned int dwSizeImage;
	unsigned int dwTotal;

    cClrBits = bmp->image->bitmap_unit;

	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else
		cClrBits = 32;

	if (cClrBits < 24)
		dwClrUsed = (1 << cClrBits);
	else
		dwClrUsed = 0;

    dwSizeImage = ((bmp->image->width * cClrBits + 31) & ~31) / 8 * bmp->image->height;

	dwTotal = (unsigned int)(sizeof(bitmap_filehead_t) + sizeof(bitmap_infohead_t) + dwClrUsed * sizeof(bitmap_rgbquad_t) + dwSizeImage);

	return dwTotal;
}

bitmap_t _load_bitmap_from_bytes(visual_t rdc, const unsigned char* pb, dword_t bytes)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	bitmap_infohead_t* pbmi;
	bitmap_filehead_t bfh;
	char* lpBits;
    
    Visual* pvi;
    X11_bitmap_t* bmp;

	if (!pb)
		return NULL;

	if ((unsigned int)bytes < sizeof(bitmap_filehead_t) + sizeof(bitmap_infohead_t))
		return NULL;

	memcpy((void*)&bfh, (void*)pb, sizeof(bitmap_filehead_t));

	if (bfh.type != 0x4d42)
		return NULL;

	if ((unsigned int)bytes < bfh.size)
		return NULL;

	pbmi = (bitmap_infohead_t*)(pb + sizeof(bitmap_filehead_t));
    
	lpBits = (char*)(pb + bfh.offset);
    
    pvi = DefaultVisual(g_display, DefaultScreen(g_display));
    
    bmp->image = XCreateImage(g_display, pvi, 24, ZPixmap, 0, lpBits, pbmi->width, pbmi->height, 32, pbmi->width * sizeof(int));
    
    return bmp->image;
}

dword_t _save_bitmap_to_bytes(visual_t rdc, bitmap_t rb, unsigned char* buf, dword_t max)
{
	X11_context_t* ctx = (X11_context_t*)rdc;
	X11_bitmap_t* bmp = (X11_bitmap_t*)rb;
    
	bitmap_infohead_t* pbmi;
	unsigned short    cClrBits;
	bitmap_filehead_t bfh;
	char* lpBits;
	unsigned int dwTotal;

	cClrBits = (unsigned short)(bmp->image->bitmap_unit);

	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else
		cClrBits = 32;

	if (cClrBits < 24)
		pbmi = (bitmap_infohead_t*)calloc(1, sizeof(bitmap_infohead_t) + sizeof(bitmap_rgbquad_t) * (unsigned int)(1 << cClrBits));
	else
		pbmi = (bitmap_infohead_t*)calloc(1, sizeof(bitmap_infohead_t));

	pbmi->size = sizeof(bitmap_infohead_t);
	pbmi->width = bmp->image->width;
	pbmi->height = bmp->image->height;
	pbmi->planes = 1;
	pbmi->bitcount = bmp->image->bits_per_pixel;
	if (cClrBits < 24)
		pbmi->clrused = (1 << cClrBits);
	else
		pbmi->clrused = 0;
	pbmi->compression = 0;
	pbmi->imagesize = ((pbmi->width * cClrBits + 31) & ~31) / 8 * pbmi->height;
	pbmi->clrimportant = 0;

	bfh.type = 0x4d42;        // 0x42 = "B" 0x4d = "M"
	bfh.size = (unsigned int)(sizeof(bitmap_filehead_t) + sizeof(bitmap_infohead_t) + pbmi->clrused * sizeof(bitmap_rgbquad_t) + pbmi->imagesize);
	bfh.reserved1 = 0;
	bfh.reserved2 = 0;
	bfh.offset = (unsigned int)(sizeof(bitmap_filehead_t) + sizeof(bitmap_infohead_t) + pbmi->clrused * sizeof(bitmap_rgbquad_t));

	if (pbmi->imagesize > (unsigned int)max)
	{
		free(pbmi);
		return 0;
	}

	dwTotal = 0;
	if (buf)
	{
		memcpy((void*)(buf + dwTotal), (void*)&bfh, sizeof(bitmap_filehead_t));
	}
	dwTotal += sizeof(bitmap_filehead_t);

	if (buf)
	{
		memcpy((void*)(buf + dwTotal), (void*)pbmi, sizeof(bitmap_infohead_t) + pbmi->clrused * sizeof(bitmap_rgbquad_t));
	}
	dwTotal += sizeof(bitmap_infohead_t) + pbmi->clrused * sizeof(bitmap_rgbquad_t);

	if (buf)
	{
		lpBits = (char*)(buf + dwTotal);
	}
	else
	{
		lpBits = NULL;
	}
	dwTotal += pbmi->imagesize;

	if (buf)
	{
        memcpy(lpBits, bmp->image->data, pbmi->imagesize);
	}

	free(pbmi);

	return dwTotal;
}

#ifdef XDU_SUPPORT_SHELL
bitmap_t _load_bitmap_from_icon(visual_t rdc, const tchar_t* iname)
{
    return NULL;
}

bitmap_t _load_bitmap_from_thumb(visual_t rdc, const tchar_t* file)
{    
    return NULL;
}
#endif //XDU_SUPPORT_SHELL

#endif //XDU_SUPPORT_CONTEXT_BITMAP
