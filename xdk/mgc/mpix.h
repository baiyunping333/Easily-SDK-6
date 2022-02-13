/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory pixmap document

	@module	mpix.h | interface file

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

#ifndef _MPIX_H
#define _MPIX_H

#include "mdef.h"

typedef struct _mem_pixmap_t*	mem_pixmap_ptr;

typedef struct _mem_pixmap_t{
	int width;	/*the pixmap row size*/
	int height; /*the pixmap col size*/
	int bytes_per_line; /* (width + 7) / 8*/
	int size;	/*the total bytes of data*/
	byte_t *data; /*the pixmap data*/

	PIXELVAL fg_color; /*foreground color*/
	PIXELVAL bg_color; /*background color*/
	bool_t bg_used; /*will use bg_color*/

	int(*getPixbit)(mem_pixmap_ptr pmp, int x, int y);
	void(*setPixbit)(mem_pixmap_ptr pmp, int x, int y, int bit);
}mem_pixmap_t;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API mem_pixmap_ptr alloc_pixmap(int width, int height);
	
	EXP_API void clean_pixmap(mem_pixmap_ptr pmp);

	EXP_API void free_pixmap(mem_pixmap_ptr pmp);

#ifdef	__cplusplus
}
#endif

#endif /*_MPIX_H*/
