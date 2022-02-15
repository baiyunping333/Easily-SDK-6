/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc glyph document

	@module	gly.h | interface file

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

#ifndef _GLYDEF_H
#define _GLYDEF_H

#include "../xdkdef.h"

#define GB2312_GLYPH_INDEX(pch)		 ((pch[0] - 161) * 94 + pch[1] - 161)

#define a_alyph_list_length		16
#define c_alyph_list_length		16

typedef struct _glyph_metrix_t {
	int width;		/* Maximum advance width of any character */
	int height;		/* Height of the glyph, Always equal to (baseline+descent) */
	int ascent;		/* The ascent (height above the baseline) of the glyph */
	int descent;	/* The descent (height below the baseline) of the glyph */
	int maxascent;	/* Maximum height of any character above the baseline */
	int maxdescent; /* Maximum height of any character below the baseline */
}glyph_metrix_t;

typedef struct _glyph_info_t{
	tchar_t			charset[32];	/*glyph charset*/
	tchar_t			name[32];	/* glyph name*/
	tchar_t			weight[32];		/* glyph weight*/
	tchar_t			style[32];		/* Glyph style*/
	tchar_t			size[32];		/* Glyph size*/

	int				width;		/* max width in pixels*/
	int				height;		/* height in pixels*/
	int				ascent;		/* The ascent (height above the baseline) of the glyph */
	int				descent;	/* The descent (height below the baseline) of the glyph */

	int				firstchar;	/* first character in bitmap*/
	int				defaultchar;/* default character*/
	int				characters;		/* characters count*/
	int				bytesperline;		/* pixmap bytes per character, eg: 1, 2, 3, 4, 5, 6, 7, 8 */

	xhand_t			glyph;	/* pixmap and width array */
} glyph_info_t;


#ifdef __cplusplus
extern "C" {
#endif

	EXP_API glyph_info_t a_glyph_list[a_alyph_list_length];
	EXP_API glyph_info_t c_glyph_list[c_alyph_list_length];

	EXP_API void xfont_from_glyph_info(xfont_t* pxf, const glyph_info_t* pgi);

	EXP_API void xfont_to_glyph_info(const xfont_t* pxf, glyph_info_t* pgi);

	EXP_API int format_glyph_pattern(const xfont_t* pxf, tchar_t* buf, int max);

	EXP_API void parse_glyph_pattern(xfont_t* pxf, const tchar_t* buf, int len);

	EXP_API void calc_glyph_size(xfont_t* pxf, int* pw, int* ph);

	EXP_API const glyph_info_t* find_glyph_info(const tchar_t* charset, const xfont_t* pxf);

	EXP_API bool_t gly_init(void);

	EXP_API void gly_uninit(void);

#ifdef __cplusplus
}
#endif

#endif /*_FNTDEF_H*/
