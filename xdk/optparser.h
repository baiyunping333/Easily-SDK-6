/***********************************************************************
	Easily SDK 6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc options parser document

	@module	optparser.h | interface file

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


#ifndef _OPTPARSER_H
#define	_OPTPARSER_H

#include "xdkdef.h"

typedef bool_t(*PF_OPTIONS_PARSE)(void* pp, const tchar_t* key, int klen, const tchar_t* val, int vlen);
typedef bool_t(*PF_OPTIONS_FORMAT)(void* fp, const tchar_t** pkey, int* pklen, const tchar_t** pval, int* pvlen);

#ifdef	__cplusplus
extern "C" {
#endif
	
	LOC_API int parse_options(const tchar_t* str, int len, tchar_t itemfeed, tchar_t linefeed, void* param, PF_OPTIONS_PARSE pf_parse);

	LOC_API int format_options(tchar_t* buf, int max, tchar_t itemfeed, tchar_t linefeed, void* param, PF_OPTIONS_FORMAT pf_format);

#ifdef	__cplusplus
}
#endif


#endif	/* _OPTPARSER_H */


