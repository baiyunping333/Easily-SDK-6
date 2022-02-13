/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk utility document

	@module	styles.h | interface file

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

#ifndef _STYLES_H
#define _STYLES_H

#include "../xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void rgb_to_hsl(unsigned char r, unsigned char g, unsigned char b, short* ph, short* ps, short* pl);

EXP_API void hsl_to_rgb(short h, short s, short l, unsigned char* pr, unsigned char* pg, unsigned char* pb);

EXP_API bool_t is_whiteness_xcolor(const xcolor_t* pxc);

EXP_API bool_t is_blackness_xcolor(const xcolor_t* pxc);

EXP_API bool_t is_grayness_xcolor(const xcolor_t* pxc);

EXP_API void parse_xcolor(xcolor_t* pxc, const tchar_t* token);

EXP_API void format_xcolor(const xcolor_t* pxc, tchar_t* buf);

EXP_API void lighten_xcolor(xcolor_t* clr, int n);

EXP_API bool_t is_null_xpen(const xpen_t* pxp);

EXP_API bool_t is_null_xbrush(const xbrush_t* pxb);

EXP_API bool_t is_null_xfont(const xfont_t* pxf);

EXP_API bool_t is_null_xface(const xface_t* pxa);

EXP_API void default_xpen(xpen_t* pxp);

EXP_API void default_xbrush(xbrush_t* pxb);

EXP_API void default_xfont(xfont_t* pxf);

EXP_API void default_xface(xface_t* pxa);

EXP_API void lighten_xpen(xpen_t* pxp, int n);

EXP_API void merge_xpen(xpen_t* pxp_dst, const xpen_t* pxp_src);

EXP_API void merge_xbrush(xbrush_t* pxb_dst, const xbrush_t* pxb_src);

EXP_API void merge_xfont(xfont_t* pxf_dst, const xfont_t* pxf_src);

EXP_API void merge_xface(xface_t* pxa_dst, const xface_t* pxa_src);

EXP_API void lighten_xbrush(xbrush_t* pxb, int n);

EXP_API void lighten_xfont(xfont_t* pxf, int n);

EXP_API void parse_xpen_from_style(xpen_t* pxp, const tchar_t* token);

EXP_API int format_xpen_to_style(const xpen_t* pxp, tchar_t* buf, int len);

EXP_API void parse_xbrush_from_style(xbrush_t* pxb, const tchar_t* token);

EXP_API int format_xbrush_to_style(const xbrush_t* pxb, tchar_t* buf, int len);

EXP_API void parse_xfont_from_style(xfont_t* pxf, const tchar_t* token);

EXP_API int format_xfont_to_style(const xfont_t* pxf, tchar_t* buf, int len);

EXP_API void parse_xface_from_style(xface_t* pxa, const tchar_t* token);

EXP_API int format_xface_to_style(const xface_t* pxa, tchar_t* buf, int len);

EXP_API void parse_ximage_from_source(ximage_t* pxi, const tchar_t* token);

EXP_API int format_ximage_to_source(const ximage_t* pxi, tchar_t* buf, int len);


#ifdef	__cplusplus
}
#endif

#endif