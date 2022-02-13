/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc png document

	@module	png.h | interface file

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
#ifndef _PNG_H
#define	_PNG_H

#include "../xdkdef.h"

#define GPL_SUPPORT_PNG

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API	dword_t xpng_decompress(const byte_t* png_buf, dword_t png_len, byte_t* bmp_buf, dword_t bmp_size);

	EXP_API	dword_t xpng_compress(const byte_t* bmp_buf, dword_t bmp_len, byte_t* png_buf, dword_t png_size);

#ifdef	__cplusplus
}
#endif


#endif	/*OEMPNG_H */

