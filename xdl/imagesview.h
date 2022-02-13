/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc imagelist view document

	@module	imageview.h | interface file

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

#ifndef _IMAGESVIEW_H
#define _IMAGESVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	IMAGE_HINT_NONE,
	IMAGE_HINT_ITEM,
	IMAGE_HINT_CHECK,
	IMAGE_HINT_TEXT,
}IMAGE_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API float calc_images_height(link_t_ptr ptr);

	EXP_API float calc_images_width(link_t_ptr ptr);

	EXP_API bool_t calc_images_item_rect(link_t_ptr ptr, link_t_ptr ilk, xrect_t* pxr);

	EXP_API bool_t calc_images_image_rect(link_t_ptr ptr, link_t_ptr ilk, xrect_t* pxr);

	EXP_API bool_t calc_images_text_rect(link_t_ptr ptr, link_t_ptr ilk, xrect_t* pxr);

	EXP_API int	calc_images_hint(const xpoint_t* ppt, link_t_ptr ptr, link_t_ptr* pilk);

	EXP_API void draw_images(const drawing_interface* pcanv, link_t_ptr ptr);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_VIEW*/

#endif /*IMAGEVIEW_H*/