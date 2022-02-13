/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, China ZheJiang HangZhou JianDe, Mail: powersuite@hotmaol.com

	@doc trieDB defination document

	@module	tdef.h | definition interface file

	@devnote 张文权 2021.01 - 2021.12 v6.0
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


#ifndef _TDEF_H
#define	_TDEF_H

#include "../xdkdef.h"


typedef enum{
	T_OBJ_DB = 1,
	T_OBJ_TK = 2,
	T_OBJ_HK = 3
}TOBJ;

typedef struct _t_db_hdr{
	sword_t tag;
	sword_t lru;
}t_db_hdr;

typedef struct _t_tk_hdr{
	sword_t tag;
	sword_t lru;
}t_tk_hdr;

typedef struct _t_hk_hdr{
	sword_t tag;
	sword_t lru;
}t_hk_hdr;

typedef t_db_hdr*	t_db_t;
typedef t_tk_hdr*	t_tk_t;
typedef t_hk_hdr*	t_hk_t;

#endif	/* _TDEF_H */

