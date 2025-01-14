﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc global/heap/virtual memory system call document

	@module	_if_memo.c | windows implement file

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


#include "../xdkloc.h"

#ifdef XDK_SUPPORT_MEMO

void* _local_alloc(dword_t size)
{
	return calloc(1, (SIZE_T)size);
}

void* _local_realloc(void* p, dword_t size)
{
	if (!p)
		return calloc(1, (SIZE_T)size);
	else
		return realloc(p, (SIZE_T)size);
}

void _local_free(void* p)
{
	if(p) free(p);
}
/******************************************************************************/
#ifdef XDK_SUPPORT_MEMO_HEAP
res_heap_t _process_heapo(void)
{
	return GetProcessHeap();
}

res_heap_t _heapo_create(void)
{
	return HeapCreate(HEAP_NO_SERIALIZE, 0, 0);
}

void  _heapo_destroy(res_heap_t heap)
{
	BOOL b;
	b = HeapDestroy(heap);
}

void* _heapo_alloc(res_heap_t heap, dword_t size)
{
	return HeapAlloc(heap, HEAP_ZERO_MEMORY, (SIZE_T)size);
}

void* _heapo_realloc(res_heap_t heap, void* p, dword_t size)
{
	if(!p)
		return HeapAlloc(heap, HEAP_ZERO_MEMORY, (SIZE_T)size);
	else
		return HeapReAlloc(heap, HEAP_ZERO_MEMORY, p, (SIZE_T)size);
}

void _heapo_zero(res_heap_t heap, void* p, dword_t size)
{
	if (p)
	{
#ifdef WINCE
		ZeroMemory(p, (SIZE_T)size);	
#else
		SecureZeroMemory(p, (SIZE_T)size);
#endif
	}
}

void _heapo_free(res_heap_t heap, void* p)
{
	 HeapFree(heap, HEAP_NO_SERIALIZE, p);
}

void _heapo_clean(res_heap_t heap)
{
	SIZE_T s = 0;

	/*PROCESS_HEAP_ENTRY Entry = { 0 };
	BOOL b;

	Entry.lpData = NULL;
	while (HeapWalk(heap, &Entry) != FALSE) 
	{
		if ((Entry.wFlags & PROCESS_HEAP_ENTRY_BUSY) != 0) 
		{
			if ((Entry.wFlags & PROCESS_HEAP_ENTRY_MOVEABLE) != 0) 
			{
			}
			if ((Entry.wFlags & PROCESS_HEAP_ENTRY_DDESHARE) != 0) 
			{
			}
		}
		else if ((Entry.wFlags & PROCESS_HEAP_REGION) != 0) 
		{
			b = HeapFree(heap, HEAP_NO_SERIALIZE, Entry.Region.lpFirstBlock);
		}
		else if ((Entry.wFlags & PROCESS_HEAP_UNCOMMITTED_RANGE) != 0) 
		{
		}
	}*/
#ifdef WINCE
	(void*)heap;
#else
	s = HeapCompact(heap, 0);
#endif
}
#endif
/******************************************************************************/
#ifdef XDK_SUPPORT_MEMO_PAGE
void* _paged_alloc(dword_t size)
{
	DWORD dw;
	void* p;
	DWORD page_size;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	page_size = si.dwPageSize;

	dw = (DWORD)(size / page_size);
	if (size % page_size)
		dw++;
	
	p = VirtualAlloc(NULL, dw * page_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (p)
	{
		ZeroMemory(p, dw * page_size);
	}

	return p;
}

void* _paged_realloc(void* p, dword_t size)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	void* pnew;
	dword_t n;
	bool_t b;
	DWORD page_size;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	page_size = si.dwPageSize;

	if (!p)
		return _paged_alloc(size);

	VirtualQuery(p, &mbi, sizeof(mbi));

	if (mbi.RegionSize < size)
	{
		pnew = _paged_alloc(size);

		n = _paged_size(p);
		n = (n < size) ? n : size;

		CopyMemory(pnew, p, n);

		_paged_free(p);
		return pnew;
	}
	else
	{
		n = size / page_size;
		if (size % page_size)
			n++;

		if (mbi.RegionSize > n * page_size)
		{
			b = VirtualFree((LPBYTE)mbi.AllocationBase + n * page_size, mbi.RegionSize - n * page_size, MEM_DECOMMIT);
		}
		return mbi.AllocationBase;
	}
}

void _paged_free(void* p)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	bool_t b;
	DWORD page_size;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	page_size = si.dwPageSize;

	if (!p)
		return;

	VirtualQuery(p, &mbi, sizeof(mbi));

	if (mbi.State == MEM_FREE)
	{
		return;
	}
	else if (mbi.State == MEM_COMMIT)
	{
		b = VirtualFree(mbi.AllocationBase, mbi.RegionSize, MEM_DECOMMIT);
		b = VirtualFree(mbi.AllocationBase, 0, MEM_RELEASE);
	}
	else if (mbi.State == MEM_RESERVE)
	{
		b = VirtualFree(mbi.AllocationBase, 0, MEM_RELEASE);
	}
}

dword_t _paged_size(void* p)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };

	if (!p)
		return 0;

	VirtualQuery(p, &mbi, sizeof(mbi));
	
	return (dword_t)mbi.RegionSize;
}

void* _paged_lock(void* p)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };

	if (!p)
		return NULL;

	VirtualQuery(p, &mbi, sizeof(mbi));

	if (mbi.State == MEM_FREE)
	{
		return NULL;
	}
	else if (mbi.State == MEM_RESERVE)
	{
		VirtualAlloc(p, mbi.RegionSize, MEM_COMMIT, PAGE_READWRITE);
		ZeroMemory(p, mbi.RegionSize);
		return VirtualLock(p, mbi.RegionSize) ? p : NULL;
	}
	else if (mbi.State == MEM_COMMIT)
	{
		return VirtualLock(p, mbi.RegionSize) ? p : NULL;
	}

	return NULL;
}

void _paged_unlock(void* p)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };

	if (!p)
		return;

	VirtualQuery(p, &mbi, sizeof(mbi));

	if (mbi.State == MEM_COMMIT && (mbi.Protect & PAGE_READONLY))
	{
		VirtualUnlock(p, mbi.RegionSize);
	}
}

bool_t _paged_protect(void* p, bool_t b)
{
	MEMORY_BASIC_INFORMATION mbi = { 0 };
	DWORD dw = 0;

	if (!p)
		return 0;

	VirtualQuery(p, &mbi, sizeof(mbi));

	if (mbi.State == MEM_FREE)
	{
		return 0;
	}
	else if (mbi.State == MEM_RESERVE)
	{
		VirtualAlloc(p, mbi.RegionSize, MEM_COMMIT, PAGE_READWRITE);
		ZeroMemory(p, mbi.RegionSize);
		return (bool_t)VirtualProtect(p, mbi.RegionSize, ((b) ? PAGE_READONLY : PAGE_READWRITE), &dw);
	}
	else if (mbi.State == MEM_COMMIT)
	{
		return (bool_t)VirtualProtect(p, mbi.RegionSize, ((b) ? PAGE_READONLY : PAGE_READWRITE), &dw);
	}

	return 0;
}
#endif

/*****************************************************************************************/
#ifdef XDK_SUPPORT_MEMO_CACHE

void* _cache_open()
{
	SIZE_T psize = 0;
	DWORD page_gran;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	page_gran = si.dwAllocationGranularity;

	return VirtualAlloc(NULL, (page_gran * 1024), MEM_RESERVE | MEM_TOP_DOWN, PAGE_NOACCESS);
}

void _cache_close(void* fh)
{
	VirtualFree(fh, 0, MEM_RELEASE);
}

bool_t _cache_write(void* fh, dword_t hoff, dword_t loff, void* buf, dword_t size, dword_t* pb)
{
	void* pBase = NULL;
	DWORD poff;
	DWORD dwh, dwl;
	SIZE_T dlen, flen;

	DWORD page_gran;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	page_gran = si.dwAllocationGranularity;

	poff = (loff % page_gran);
	loff = (loff / page_gran) * page_gran;
	dlen = poff + size;

	flen = MAKESIZE(loff, hoff) + dlen;
	dwh = GETSIZEH(flen);
	dwl = GETSIZEL(flen);

	pBase = VirtualAlloc((LPBYTE)fh + (flen - dlen), dlen, MEM_COMMIT, PAGE_READWRITE);
	if (!pBase)
	{
		if (pb) *pb = 0;

		return 0;
	}

	CopyMemory((void*)((char*)pBase + poff), buf, (SIZE_T)size);

	if (pb) *pb = size;

	return 1;
}

bool_t _cache_read(void* fh, dword_t hoff, dword_t loff, void* buf, dword_t size, dword_t* pb)
{
	void* pBase = NULL;
	DWORD poff;
	DWORD dwh, dwl;
	SIZE_T dlen, flen;
	DWORD page_size, page_gran;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	page_size = si.dwPageSize;
	page_gran = si.dwAllocationGranularity;

	poff = (loff % page_gran);
	loff = (loff / page_gran) * page_gran;
	dlen = poff + size;

	flen = MAKESIZE(loff, hoff) + dlen;
	dwh = GETSIZEH(flen);
	dwl = GETSIZEL(flen);

	pBase = VirtualAlloc((LPBYTE)fh + (flen - dlen), dlen, MEM_COMMIT, PAGE_READONLY);
	if (!pBase)
	{
		if (pb) *pb = 0;

		return 0;
	}

	CopyMemory(buf, (void*)((char*)pBase + poff), size);

	dlen = (flen / page_size) * page_size;
	if (dlen)
	{
		VirtualFree(pBase, dlen, MEM_DECOMMIT);
	}

	if (pb) *pb = size;

	return 1;
}
#endif

#endif //XDK_SUPPORT_MEMO