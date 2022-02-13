/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdl utility document

	@module	xdlutil.h | interface file

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

#ifndef _MISCE_H
#define _MISCE_H

#include "xdldef.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API int compare_data(const tchar_t* szSrc, const tchar_t* szDes, const tchar_t* datatype);

EXP_API int verify_text(const tchar_t* str, const tchar_t* datatype, bool_t nullable, int len, const tchar_t* min, const tchar_t* max);

EXP_API int format_shield(const tchar_t* sz, tchar_t* buf, int max);

EXP_API void cn_date_token(const xdate_t* pdt, tchar_t* year, tchar_t* month, tchar_t* day, tchar_t* week, tchar_t* solar);

EXP_API bool_t get_param_item(const tchar_t* sz_param, const tchar_t* key, tchar_t* val, int max);

EXP_API int split_line(const tchar_t* token, int len);

EXP_API bool_t split_xmlns(tchar_t* str, int* kat, int* klen, int* vat, int* vlen);

EXP_API const tchar_t* skip_xmlns(const tchar_t* str, int slen);

EXP_API int trim_xmlns(tchar_t* str, int slen);

EXP_API int compare_nons(const tchar_t* src, int srclen, const tchar_t* dest, int destlen);

EXP_API int printf_path(tchar_t* fpath, const tchar_t* strfmt, ...);


#ifdef	__cplusplus
}
#endif

#endif