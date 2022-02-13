/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory document

	@module	impmem.c | implement file

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

#include "impmem.h"

#include "../xdkimp.h"
#include "../xdkstd.h"

#if defined(XDK_SUPPORT_MEMO_DUMP)

void* xmem_alloc_dump(dword_t size, const char* src, const char* func, unsigned int line)
{
	if_zone_t* pif;
	void* p;
	dword_t dlen;
	char* dump;
	
	pif = THREAD_ZONE_INTERFACE;

	p = xmem_alloc_nodump((size + sizeof(vword_t) + sizeof(link_t)));

	((link_t_ptr)p)->tag = lkDebug;
	insert_link_after(&pif->if_dump, LINK_LAST, (link_t_ptr)p);

	dlen = a_xslen(src) + a_xslen(func) + 2 * NUM_LEN;
	dump = (char*)xmem_alloc_nodump(dlen);
	a_xsprintf(dump, "_FILE: %s, _FUNC: %s, _LINE: %d, _SIZE: %d", src, func, line, size);

	*((vword_t*)((byte_t*)p + sizeof(link_t))) = (vword_t)dump;
	p = (void*)((byte_t*)p + sizeof(link_t) + sizeof(vword_t));

	return p;
}

void* xmem_realloc_dump(void* p, dword_t size, const char* src, const char* func, unsigned int line)
{
	if_zone_t* pif;
	char* dump;
	dword_t dlen;

	pif = THREAD_ZONE_INTERFACE;

	if (!size)
	{
		xmem_free_dump(p);
		return NULL;
	}

	if (!p)
	{
		return xmem_alloc_dump(size, src, func, line);
	}

	p = (void*)((byte_t*)p - sizeof(vword_t) - sizeof(link_t));
	delete_link(&pif->if_dump, (link_t_ptr)p);
	dump =(char*)(*((vword_t*)((byte_t*)p + sizeof(link_t))));
	xmem_free_nodump((void*)dump);

	p = xmem_realloc_nodump(p, (size + sizeof(link_t) + sizeof(vword_t)));

	((link_t_ptr)p)->tag = lkDebug;
	insert_link_after(&pif->if_dump, LINK_LAST, (link_t_ptr)p);

	dlen = a_xslen(src) + a_xslen(func) +  2 * NUM_LEN;
	dump = (char*)xmem_alloc_nodump(dlen);
	a_xsprintf(dump, "%s %s:%d size:%d\n", src, func, line, size);

	*((vword_t*)((byte_t*)p + sizeof(link_t))) = (vword_t)dump;
	p = (void*)((byte_t*)p + sizeof(link_t) + sizeof(vword_t));

	return p;
}

void xmem_free_dump(void* p)
{
	if_zone_t* pif;
	vword_t dump;

	pif = THREAD_ZONE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	if (!p) return;

	p = (void*)((byte_t*)p - sizeof(vword_t) - sizeof(link_t));
	XDK_ASSERT(((link_t_ptr)p)->tag == lkDebug);

	delete_link(&pif->if_dump, (link_t_ptr)p);

	dump = *((vword_t*)((byte_t*)p + sizeof(link_t)));
	xmem_free_nodump((void*)dump);

	xmem_free_nodump(p);
}

#endif //XDK_SUPPORT_MEMO_DUMP

void* xmem_alloc_nodump(dword_t size)
{
	void* p;

#ifdef XDK_SUPPORT_MEMO_HEAP
	if_zone_t* pif;
	if_memo_t* piv;

	if(!size)
		return NULL;

	pif = THREAD_ZONE_INTERFACE;
	piv = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL && piv != NULL);

	p = (*piv->pf_heap_alloc)(pif->if_heap, size);
	
	XDK_ASSERT(p != NULL);

#else

	if_memo_t* piv;

	piv = PROCESS_MEMO_INTERFACE;
	XDK_ASSERT(piv != NULL);

	p = (*piv->pf_local_alloc)(size);

	XDK_ASSERT(p != NULL);

#endif //XDK_SUPPORT_MEMO_HEAP

	return p;
}

void* xmem_realloc_nodump(void* p, dword_t size)
{
	if (!size)
	{
		xmem_free_nodump(p);
		return NULL;
	}

	if (!p)
	{
		return xmem_alloc_nodump(size);
	}

#ifdef XDK_SUPPORT_MEMO_HEAP
	if_zone_t* pif;
	if_memo_t* piv;

	pif = THREAD_ZONE_INTERFACE;
	piv = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL && piv != NULL);

	p = (*piv->pf_heap_realloc)(pif->if_heap, p, size);
	XDK_ASSERT(p != NULL);
#else

	if_memo_t* piv;

	piv = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(piv != NULL);

	p = (*piv->pf_local_realloc)(p, size);
	XDK_ASSERT(p != NULL);

#endif //XDK_SUPPORT_MEMO_HEAP

	return p;
}

void xmem_free_nodump(void* p)
{
#ifdef XDK_SUPPORT_MEMO_HEAP

	if_zone_t* pif;
	if_memo_t* piv;

	pif = THREAD_ZONE_INTERFACE;
	piv = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL && piv != NULL);

	if (!p) return;

	(*piv->pf_heap_free)(pif->if_heap, p);
#else

	if_memo_t* piv;

	piv = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(piv != NULL);

	if (!p) return;

	(*piv->pf_local_free)(p);

#endif //XDK_SUPPORT_MEMO_HEAP
}

void xmem_zero(void* p, dword_t size)
{
	byte_t* pb = (byte_t*)p;

	if(!p || !size)
		return;

	while (size--)
	{
		*(pb++) = 0;
	}
}

void xmem_set(void* p, byte_t c, dword_t size)
{
	byte_t* pb = (byte_t*)p;

	if (!p || !size)
		return;

	while (size--)
	{
		*(pb++) = c;
	}
}

void* xmem_clone(void* src,dword_t bytes)
{
	void* p;

	if(!src || bytes <= 0)
		return NULL;

	p = xmem_alloc_nodump(bytes);

	xmem_copy(p,src,bytes);

	return p;
}

void xmem_copy(void* dest, void* src, dword_t size)
{
	byte_t* ps = (byte_t*)src;
	byte_t* pd = (byte_t*)dest;

	if (!dest || !src)
		return;

	while (size--)
	{
		*(pd++) = *(ps++);
	}
}

int xmem_comp(void* mem1, void* mem2, dword_t size)
{
	byte_t *p1, *p2;
	dword_t len;

	if (!size)
		return 0;
	else if (!mem1 && mem2)
		return -1;
	else if (mem1 && !mem2)
		return 1;

	p1 = (byte_t*)mem1;
	p2 = (byte_t*)mem2;

	len = size;
	while (len--)
	{
		if (*p1 > *p2)
			return 1;
		else if (*p1 < *p2)
			return -1;

		p1++; p2++;
	}
	
	return 0;
}

void xmem_move(void* p, dword_t len, int off)
{
	byte_t *p1, *p2;

	if (!p || !off)
		return;

	if (off > 0)
	{
		p1 = (byte_t*)p + len - 1 + off;
		p2 = (byte_t*)p + len - 1;

		while (len--)
		{
			*p1 = *p2;
			p1--;
			p2--;
		}
	}
	else
	{
		p1 = (byte_t*)p + off;
		p2 = (byte_t*)p;

		while (len--)
		{
			*p1 = *p2;
			p1++;
			p2++;
		}
	}
}

#ifdef XDK_SUPPORT_MEMO_DUMP
void xmem_assert(void* p)
{
	if (!p)
		return;

	p = (void*)((byte_t*)p - sizeof(dword_t) - sizeof(link_t));

	XDK_ASSERT(((link_t_ptr)p)->tag == lkDebug);
}

dword_t	xmem_size(void* p)
{
	if (!p)
		return 0;

	XDK_ASSERT(((link_t_ptr)((byte_t*)p - sizeof(dword_t) - sizeof(link_t)))->tag == lkDebug);

	p = (void*)((byte_t*)p - sizeof(dword_t));
	return *((dword_t*)p);
}

#endif

//////////////////////////////////////////////////////////////////////////////////////////
#ifdef XDK_SUPPORT_MEMO_PAGE

void* pmem_alloc(dword_t size)
{
	if_memo_t *pif;
	void* p;

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	p = (*pif->pf_page_alloc)(size);

	XDK_ASSERT(p != NULL);

	return p;
}

void* pmem_realloc(void* p, dword_t size)
{
	if_memo_t *pif;

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	p = (*pif->pf_page_realloc)(p, size);

	XDK_ASSERT(p != NULL);

	return p;
}

void pmem_free(void* p)
{
	if_memo_t *pif;

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_page_free)(p);
}

dword_t	pmem_size(void* p)
{
	if_memo_t *pif;

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (dword_t)(*pif->pf_page_size)(p);
}

void* pmem_lock(void* p)
{
	if_memo_t *pif;

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (*pif->pf_page_lock)(p);
}

void pmem_unlock(void* p)
{
	if_memo_t *pif;

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_page_unlock)(p);
}

bool_t	pmem_protect(void* p, bool_t b)
{
	if_memo_t *pif;

	pif = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (*pif->pf_page_protect)(p, b);
}

#endif

