/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, China ZheJiang HangZhou JianDe, Mail: powersuite@hotmaol.com

	@doc tkb defination document

	@module	tkv.h | definition interface file

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


#ifndef _TKV_H
#define	_TKV_H

#include "tdef.h"

#define TKV_MASK_PERSIST	0x00000001

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API t_tk_t tkv_create(tchar_t key_feed);

EXP_API void tkv_destroy(t_tk_t tkv);

EXP_API bool_t tkv_write(t_tk_t tkv, const tchar_t* key, object_t val);

EXP_API bool_t tkv_read(t_tk_t tkv, const tchar_t* key, object_t val);

EXP_API bool_t tkv_update(t_tk_t tkv, const tchar_t* key, object_t val);

EXP_API bool_t tkv_delete(t_tk_t tkv, const tchar_t* key);

EXP_API void tkv_bind(t_tk_t tkv, t_db_t tdb, bool_t laze);

EXP_API bool_t tkv_load(t_tk_t tkv);

EXP_API bool_t tkv_flush(t_tk_t tkv);

#ifdef XDK_SUPPORT_TEST
EXP_API void test_tkv();
#endif

#ifdef	__cplusplus
}
#endif


#endif	/* _TKV_H */

