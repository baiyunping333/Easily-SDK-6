﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory cache document

	@module	impcache.c | implement file

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

#include "impcache.h"

#include "../xdkimp.h"
#include "../xdkstd.h"


/////////////////////////////////////////////////////////////////////////////////////////
#ifdef XDK_SUPPORT_MEMO_CACHE

typedef struct _cache_context{
	handle_head head;		//reserved for xhand_t

	void* cache;
	dword_t write_bytes;
	dword_t read_bytes;
}cache_context;


xhand_t xcache_open()
{
	cache_context* ppi;
	if_memo_t* pif;
	void* bh;

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	bh = (*pif->pf_cache_open)();

	if (!bh)
	{
		set_system_error(_T("pf_cache_open"));
		return NULL;
	}

	ppi = (cache_context*)xmem_alloc(sizeof(cache_context));
	ppi->head.tag = _HANDLE_CACHE;
	ppi->cache = bh;
	ppi->read_bytes = 0;
	ppi->write_bytes = 0;

	return &ppi->head;
}

void* xcache_handle(xhand_t cache)
{
	cache_context* ppi = TypePtrFromHead(cache_context, cache);

	XDK_ASSERT(cache && cache->tag == _HANDLE_CACHE);

	return ppi->cache;
}

void xcache_close(xhand_t cache)
{
	cache_context* ppi = TypePtrFromHead(cache_context, cache);
	if_memo_t* pif;

	XDK_ASSERT(cache && cache->tag == _HANDLE_CACHE);

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_cache_close)(ppi->cache);

	xmem_free(ppi);
}

bool_t xcache_read(xhand_t cache, byte_t* buf, dword_t* pb)
{
	cache_context* ppt = TypePtrFromHead(cache_context, cache);
	if_memo_t* pif;
	dword_t size;
	bool_t rt;

	XDK_ASSERT(cache && cache->tag == _HANDLE_CACHE);

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	size = (*pb < (ppt->write_bytes - ppt->read_bytes)) ? (*pb) : (ppt->write_bytes - ppt->read_bytes);

	if (size)
		rt = (*pif->pf_cache_read)(ppt->cache, 0, ppt->read_bytes, buf, size, &size);
	else
		rt = 1;

	if (rt)
	{
		ppt->read_bytes += (dword_t)size;
		*pb = (dword_t)size;
	}
	else
	{
		set_system_error(_T("pf_cache_read"));
		*pb = 0;
	}

	return rt;
}

bool_t xcache_write(xhand_t cache, const byte_t* buf, dword_t* pb)
{
	cache_context* ppt = TypePtrFromHead(cache_context, cache);
	if_memo_t* pif;
	dword_t size;
	bool_t rt;

	XDK_ASSERT(cache && cache->tag == _HANDLE_CACHE);

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	size = *pb;
	rt = (*pif->pf_cache_write)(ppt->cache, 0, ppt->write_bytes, (void*)buf, size, &size);
	if (rt)
	{
		ppt->write_bytes += (dword_t)size;
		*pb = (dword_t)size;
	}
	else
	{
		set_system_error(_T("pf_cache_write"));
		*pb = 0;
	}

	return rt;
}

#endif //XDK_SUPPORT_MEMO_CACHE
