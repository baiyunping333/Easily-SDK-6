/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, China ZheJiang HangZhou JianDe, Mail: powersuite@hotmaol.com

	@doc tdb defination document

	@module	tdb.h | definition interface file

	@devnote 张文权 2018.01 - 2018.12	v1.0
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


#ifndef _TDB_H
#define	_TDB_H

#include "tdef.h"

#include "../bplustree.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API t_db_t tdb_create(const tchar_t* dpath, const tchar_t* dname, dword_t tmode);

EXP_API void tdb_destroy(t_db_t hdb);

EXP_API bool_t tdb_save(t_db_t hdb, variant_t key, object_t val);

EXP_API bool_t tdb_load(t_db_t hdb, variant_t key, object_t val);

EXP_API bool_t tdb_clean(t_db_t hdb, variant_t key);

EXP_API void tdb_enum(t_db_t hdb, ENUM_BPLUSTREE_ENTITY pf, void* param);

#ifdef	__cplusplus
}
#endif


#endif	/* _TDB_H */

