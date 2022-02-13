/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc spin lock document

	@module	spinlock.h | interface file

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

#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include "xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION alloc_spinlock: alloc a lock table.
@INPUT const tchar_t* guid: the lock global id.
@INPUT int nums_permap: the single map items count.
@RETURN spinlock_t: return the lock table object.
*/
EXP_API spinlock_t alloc_spinlock(const tchar_t* guid, int nums_permap);

/*
@FUNCTION free_spinlock: free a lock table.
@INPUT spinlock_t pt: the lock table object.
@RETURN void: none.
*/
EXP_API void free_spinlock(spinlock_t pt);

/*
@FUNCTION enter_spinlock: enter the lock table.
@INPUT spinlock_t pt: the lock table object.
@INPUT int map_ind: the zero based map index.
@INPUT int map_pos: the zero based map item index.
@RETURN bool_t: if succeeded return none zero.
*/
EXP_API bool_t enter_spinlock(spinlock_t pt, int map_ind, int map_pos);

/*
@FUNCTION leave_lock_table: leave the lock table.
@INPUT spinlock_t pt: the lock table object.
@INPUT int map_ind: the zero based map index.
@INPUT int map_pos: the zero based map item index.
@RETURN bool_t: if succeeded return none zero.
*/
EXP_API void leave_spinlock(spinlock_t pt, int map_ind, int map_pos);

#if defined(XDK_SUPPORT_TEST)
EXP_API void test_spinlock(void);
#endif

#ifdef	__cplusplus
}
#endif

#endif /*_LOCKTABLE_H*/