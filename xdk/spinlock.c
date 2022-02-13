/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc spin lock document

	@module	spinlock.c | implement file

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

#include "spinlock.h"

#include "xdkimp.h"
#include "xdkoem.h"
#include "xdkstd.h"


#define GUID_MEM_PREFIX		_T("mem-")

#define SPINLOCK_MAPBITS		2

#define SPINLOCK_TAG_WAIT		0x01
#define SPINLOCK_TAG_LOCK		0x02
#define SPINLOCK_TAG_BITMASK	0x03

typedef struct _spinlock_context{
	memobj_head head;

	xhand_t share;

	map_t map;
	dword_t map_size;
	tchar_t guid[NUID_TOKEN_SIZE + 1];
}spinlock_context;

/************************************************************************************/

spinlock_t alloc_spinlock(const tchar_t* guid, int map_nums)
{
	spinlock_context* ppt = NULL;

	tchar_t token[NUID_TOKEN_SIZE + 5] = { 0 };

	TRY_CATCH;

	ppt = (spinlock_context*)xmem_alloc(sizeof(spinlock_context));
	ppt->head.tag = MEM_SPINLOCK;

	xsncpy(ppt->guid, guid, NUID_TOKEN_SIZE);

	ppt->map = map_alloc(map_nums, SPINLOCK_MAPBITS);
	if (!ppt->map)
	{
		raise_user_error(_T("create_spinlock"), _T("alloc map object failed"));
	}
	ppt->map_size = map_size(ppt->map);

	xsprintf(token, _T("%s%s"), GUID_MEM_PREFIX, ppt->guid);
	ppt->share = xshare_cli(token, MAX_LONG, FILE_OPEN_CREATE);
	if (!ppt->share)
	{
		raise_user_error(_T("create_spinlock"), _T("alloc share object failed"));
	}

	END_CATCH;

	return &ppt->head;
ONERROR:

	if (ppt->share)
		xshare_close(ppt->share);

	if (ppt->map)
		map_free(ppt->map);

	if (ppt)
		xmem_free(ppt);

	return NULL;
}

void free_spinlock(spinlock_t pt)
{
	spinlock_context* ppt = TypePtrFromHead(spinlock_context, pt);
	
	tchar_t token[NUID_TOKEN_SIZE + 5] = { 0 };

	XDK_ASSERT(pt && pt->tag == MEM_SPINLOCK);

	xshare_close(ppt->share);

	map_free(ppt->map);

	xmem_free(ppt);
}

bool_t enter_spinlock(spinlock_t pt, int map_ind, int map_pos)
{
	spinlock_context* ppt = TypePtrFromHead(spinlock_context, pt);

	dword_t offs;
	byte_t *buff;
	byte_t tag;
	bool_t b;
	void* data;

	XDK_ASSERT(pt && pt->tag == MEM_SPINLOCK);

	offs = map_ind * ppt->map_size;
	buff = xshare_lock(ppt->share, offs, ppt->map_size);

	XDK_ASSERT(buff != NULL);

	data = map_detach(ppt->map);
	map_attach(ppt->map, (void*)buff);

	tag = map_get_bit(ppt->map, map_pos);
	tag &= SPINLOCK_TAG_BITMASK;
	
	b = (tag & SPINLOCK_TAG_WAIT) ? 0 : 1;

	tag >>= 1;
	map_set_bit(ppt->map, map_pos, tag);

	if (b)
	{
		while ((tag & SPINLOCK_TAG_WAIT))
		{
			tag = map_get_bit(ppt->map, map_pos);
			tag &= SPINLOCK_TAG_BITMASK;
		}

		map_set_bit(ppt->map, map_pos, SPINLOCK_TAG_LOCK);
	}

	buff = (byte_t*)map_detach(ppt->map);
	map_attach(ppt->map, data);

	xshare_unlock(ppt->share, offs, ppt->map_size, buff);

	return b;
}

void leave_spinlock(spinlock_t pt, int map_ind, int map_pos)
{
	spinlock_context* ppt = TypePtrFromHead(spinlock_context, pt);

	dword_t offs;
	byte_t *buff;
	byte_t tag;
	void* data;

	XDK_ASSERT(pt && pt->tag == MEM_SPINLOCK);

	offs = map_ind * ppt->map_size;
	buff = xshare_lock(ppt->share, offs, ppt->map_size);

	XDK_ASSERT(buff != NULL);

	data = map_detach(ppt->map);
	map_attach(ppt->map, (void*)buff);

	tag = map_get_bit(ppt->map, map_pos);
	tag &= SPINLOCK_TAG_BITMASK;

	if (tag)
	{
		tag = 0;
		map_set_bit(ppt->map, map_pos, tag);
	}

	buff = (byte_t*)map_detach(ppt->map);
	map_attach(ppt->map, data);

	xshare_unlock(ppt->share, offs, ppt->map_size, buff);
}

#if defined(XDK_SUPPORT_TEST)

void test_spinlock()
{
	lword_t tms;
	nuid_t nuid = { 0 };
	tchar_t token[NUID_TOKEN_SIZE + 1] = { 0 };

	tms = get_timestamp();
	nuid_from_timestamp(&nuid, tms);
	nuid_format_string(&nuid, token);

	int nums = 4096;
	spinlock_t lt = alloc_spinlock(token, nums);
	bool_t rt;
	int i, k, j;

	for (k = 0; k < 1024; k++)
	{
		for (i = 0; i < nums; i++)
		{
			for (j = 0; j < 2; j++)
			{
				rt = enter_spinlock(lt, k, i);
				_tprintf(_T("map:%d pos:%d return:%d\n"), k, i, rt);
				//if (j % 2 && rt)
					//goto err;
				leave_spinlock(lt, k, i);
			}
		}
	}

//err:

	free_spinlock(lt);
}
#endif