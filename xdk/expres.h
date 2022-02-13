/***********************************************************************
	Easily SDK 6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc expression document

	@module	expres.h | interface file

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

#ifndef _ESPRES_H
#define _ESPRES_H

#include "xdkdef.h"
#include "dlink.h"

/*data compare callback function*/
typedef int (*expr_compare_ptr)(const tchar_t* key,const tchar_t* sin,const tchar_t* val,void* parm);

#ifdef	__cplusplus
extern "C" {
#endif

/*parse expression token*/
EXP_API bool_t expr_parse(link_t_ptr ptr,const tchar_t* str);

/*format expression token size*/
EXP_API int expr_format_length(link_t_ptr ptr);

/*format expression token*/
EXP_API int expr_format(link_t_ptr ptr,tchar_t* buf,int max);

/*execute expression and return result*/
EXP_API bool_t expr_exec(link_t_ptr ptr,expr_compare_ptr pf,void* parm);

#ifdef	__cplusplus
}
#endif

#endif /*EXPR_H*/
