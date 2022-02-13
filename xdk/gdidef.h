/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc graphic defination document

	@module	xdkdef.h | interface file

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


#ifndef _GDIDEF_H
#define	_GDIDEF_H

//#define INCHPERTM	0.003937
#define INCHPERMM	0.03937f
//#define TMPERINCH	254
#define MMPERINCH	25.4f
//#define PTPERINCH	1440
//#define PTPERMM	56.693f
#define MMPERLOGPT	0.1f
#define MMPERPD		0.3527f
#define PDPERMM		2.835f
#define PDPERINCH	72.0f
#define PTPERMM		3.779f
#define MMPERPT		0.2646f

#define BDPI		96
#define SDPI		120
#define MDPI		160
#define LDPI		240
#define HDPI		320


typedef struct _dev_cap_t{
	int horz_res, vert_res;
	int horz_pixels, vert_pixels;
	int horz_feed, vert_feed;
	int horz_size, vert_size;
}dev_cap_t;

#define MAX_FORM_NAME		32
#define MAX_GLYPH_NAME		64
#define MAX_DEVICE_NAME		256
#define MAX_FONT_NAME		512

typedef struct _dev_prn_t{
	tchar_t devname[MAX_DEVICE_NAME];
	short paper_width;
	short paper_height;
	short landscape;
	short duplex;
}dev_prn_t;

//color darkeness
#define DEF_SOFT_DARKEN		-3
#define DEF_MIDD_DARKEN		-6
#define DEF_HARD_DARKEN		-10
#define DEF_SOFT_LIGHTEN	3
#define DEF_MIDD_LIGHTEN	6
#define DEF_HARD_LIGHTEN	10

/* palette quad table*/
typedef struct _pal_quad_t{
	unsigned char blue;		//blue lighten(0-255)
	unsigned char green;	//green lighten(0-255)
	unsigned char red;		//red lighten(0-255)
	unsigned char reserved;	//set to zero
}pal_quad_t;

typedef enum{ _RGB_COLOR, HSL_COLOR, _HEX_COLOR }CLRFMT;

typedef struct _yuv_color_t{
	unsigned char y, u, v;
}yuv_color_t;

typedef struct _xcolor_t{
	unsigned char r, g, b, a;
}xcolor_t;

typedef struct _clr_mod_t{
	xcolor_t clr_bkg;
	xcolor_t clr_frg;
	xcolor_t clr_txt;
	xcolor_t clr_msk;
	xcolor_t clr_ico;
}clr_mod_t;


typedef struct _xrect_t{
	union{
		int x;
		float fx;
	};
	union{
		int y;
		float fy;
	};
	union{
		int w;
		float fw;
	};
	union{
		int h;
		float fh;
	};
}xrect_t;

typedef struct _xpoint_t{
	union{
		int x;
		float fx;
	};
	union{
		int y;
		float fy;
	};
}xpoint_t;

typedef struct _xsize_t{
	union{
		int w;
		float fw;
	};
	union{
		int h;
		float fh;
	};
}xsize_t;

typedef struct _xspan_t{
	union{
		int r;
		float fr;
	};
}xspan_t;

#define RECTPOINT(pxr)	((xpoint_t*)pxr)
#define RECTSIZE(pxr)	((xsize_t*)pxr + 1)

typedef struct _border_t{
	int title;
	int edge;
	int hscroll;
	int vscroll;
	int menu;
	int icon;
}border_t;

typedef struct _scroll_t{
	int pos;
	int min;
	int max;
	int page;
	int track;
}scroll_t;

/*define shadow feed*/
#define DEF_MIN_SHADOW		5
#define DEF_MAX_SHADOW		10

typedef struct _shadow_t{
	int offx;
	int offy;
}shadow_t;

typedef struct _adorn_t{
	int feed;
	int size;
}adorn_t;

typedef struct _xbrush_t{
	tchar_t style[RES_LEN + 1];
	tchar_t opacity[INT_LEN + 1];
	tchar_t color[CLR_LEN + 1];
	tchar_t linear[CLR_LEN + 1];
	tchar_t gradient[RES_LEN + 1];
	shadow_t shadow;
}xbrush_t;

typedef struct _xpen_t{
	tchar_t style[RES_LEN + 1];
	tchar_t size[INT_LEN + 1];
	tchar_t opacity[INT_LEN + 1];
	tchar_t color[CLR_LEN + 1];
	adorn_t adorn;
}xpen_t;

typedef struct _xfont_t{
	tchar_t style[RES_LEN + 1];
	tchar_t decorate[RES_LEN + 1];
	tchar_t size[INT_LEN + 1];
	tchar_t weight[INT_LEN + 1];
	tchar_t family[RES_LEN + 1];
	tchar_t color[CLR_LEN + 1];
}xfont_t;

typedef struct _xface_t{
	tchar_t text_wrap[RES_LEN + 1];
	tchar_t text_align[RES_LEN + 1];
	tchar_t line_align[RES_LEN + 1];
	tchar_t line_height[INT_LEN + 1];
}xface_t;

typedef struct _ximage_t{
	tchar_t style[RES_LEN + 1];
	tchar_t type[RES_LEN + 1];
	tchar_t color[CLR_LEN + 1];

	const tchar_t* source;
}ximage_t;



#endif	/* _GDIDEF_H */

