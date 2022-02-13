/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, China ZheJiang HangZhou JianDe, Mail: powersuite@hotmaol.com

	@doc hkv defination document

	@module	hkv.h | definition interface file

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


#ifndef _HKV_H
#define	_HKV_H

#include "tdef.h"

#define HKV_MASK_PERSIST		0x00000001

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API t_hk_t hkv_create(void);

EXP_API void hkv_destroy(t_hk_t hkv);

EXP_API bool_t hkv_write(t_hk_t hkv, variant_t key, object_t val);

EXP_API bool_t hkv_read(t_hk_t hkv, variant_t key, object_t val);

EXP_API bool_t hkv_update(t_hk_t hkv, variant_t key, object_t val);

EXP_API bool_t hkv_delete(t_hk_t hkv, variant_t key);

EXP_API void hkv_bind(t_hk_t hkv, t_db_t tdb, bool_t laze);

EXP_API bool_t hkv_load(t_hk_t hkv);

EXP_API bool_t hkv_flush(t_hk_t hkv);

#ifdef XDK_SUPPORT_TEST
EXP_API void test_hkv();
#endif

#ifdef	__cplusplus
}
#endif


#endif	/* _HKV_H */

