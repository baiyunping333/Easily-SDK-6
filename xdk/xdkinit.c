/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc initialize document

	@module	xdkinit.c | implement file

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


#include "xdkinit.h"

#include "xdkimp.h"
#include "xdkstd.h"

#ifdef XDK_SUPPORT_ACP
#include "acp/acp.h"
#endif

#ifdef XDK_SUPPORT_GLYPH
#include "gly/gly.h"
#endif

xdk_mou_t g_xdk_mou = { 0 };

jmp_buf* thread_jump_buff(void)
{
	if_jump_t* pju;

	pju = THREAD_JUMP_INTERFACE;
	if (!pju)
		return NULL;

	return &(pju->if_buf[pju->if_index]);
}

#ifdef XDK_SUPPORT_THREAD

static void _action_pipe(int sig)
{
    
}

void xdk_thread_init(int master)
{
	if_zone_t* pzn;
	if_jump_t* pju;
	if_dump_t* pdu;

	if_memo_t* piv;
	if_thread_t* pit;
#ifdef XDK_SUPPORT_MEMO_HEAP
	void* heap = NULL;
#endif
    dword_t tid;

    XDK_ASSERT(XDK_MOUNTED);

	piv = PROCESS_MEMO_INTERFACE;
	pit = PROCESS_THREAD_INTERFACE;

    if(!master)
    {
        (*pit->pf_thread_safe)();
    }
    
    tid = (*pit->pf_thread_get_id)();

#ifdef XDK_SUPPORT_MEMO_HEAP  
	if (g_xdk_mou.if_opt & XDK_APARTMENT_THREAD)
	{
		heap = (*piv->pf_heap_create)();
	}
	else
	{
		heap = (*piv->pf_process_heap)();
	}
	
	XDK_ASSERT(heap != NULL);

	pzn = (if_zone_t*)(*piv->pf_heap_alloc)(heap, sizeof(if_zone_t));
#else
	pzn = (if_zone_t*)(*piv->pf_local_alloc)(sizeof(if_zone_t));
#endif /*XDK_SUPPORT_MEMO_HEAP*/

#ifdef XDK_SUPPORT_MEMO_HEAP 
	pzn->if_heap = heap;
#endif

	XDK_ASSERT(g_xdk_mou.tls_thr_zone != 0);
	(*pit->pf_thread_set_tls)(g_xdk_mou.tls_thr_zone, (void*)pzn);
#ifdef XDK_SUPPORT_MEMO_DUMP
	init_root_link(&pzn->if_dump);

	(*pit->pf_criti_enter)(g_xdk_mou.dump_crit);

	pzn->if_trak.tag = (int)(*pit->pf_thread_get_id)();
	pzn->if_trak.next = pzn->if_trak.prev = NULL;
	insert_link_after(&g_xdk_mou.dump_link, LINK_LAST, &pzn->if_trak);

	(*pit->pf_criti_leave)(g_xdk_mou.dump_crit);
#endif
#ifdef XDK_SUPPORT_MEMO_HEAP
	pju = (if_jump_t*)(*piv->pf_heap_alloc)(heap, sizeof(if_jump_t));
	pdu = (if_dump_t*)(*piv->pf_heap_alloc)(heap, sizeof(if_dump_t));
#else
	pju = (if_jump_t*)(*piv->pf_local_alloc)(sizeof(if_jump_t));
	pdu = (if_dump_t*)(*piv->pf_local_alloc)(sizeof(if_dump_t));
#endif

	pju->if_buf = NULL;
	pju->if_index = -1;
	pju->if_size = 0;

	pdu->err_enable = 1;

	XDK_ASSERT(g_xdk_mou.tls_thr_jump != 0);
	(*pit->pf_thread_set_tls)(g_xdk_mou.tls_thr_jump, (void*)pju);

	XDK_ASSERT(g_xdk_mou.tls_thr_dump != 0);
	(*pit->pf_thread_set_tls)(g_xdk_mou.tls_thr_dump, (void*)pdu);
}

void xdk_thread_uninit(int error)
{
	if_zone_t* pzn;
	if_jump_t* pju;
	if_dump_t* pdu;
	err_dump_t* per;
	if_memo_t* piv;
	if_thread_t* pit;
#ifdef XDK_SUPPORT_MEMO_HEAP
	void* heap = NULL;
#endif

	if (!error)
	{
		XDK_ASSERT(XDK_MOUNTED);
	}

	if (!XDK_MOUNTED)
		return;

	piv = PROCESS_MEMO_INTERFACE;
	pit = PROCESS_THREAD_INTERFACE;

	if (!error)
	{
		XDK_ASSERT(g_xdk_mou.tls_thr_zone != 0);
	}

	if (!g_xdk_mou.tls_thr_zone)
		return;

	pzn = (if_zone_t*)(*pit->pf_thread_get_tls)(g_xdk_mou.tls_thr_zone);
	
	if (!error)
	{
		XDK_ASSERT(pzn != NULL);
	}

	if (!pzn)
		return;

#ifdef XDK_SUPPORT_MEMO_HEAP
	heap = pzn->if_heap;

	if (!error)
	{
		XDK_ASSERT(heap != NULL);
	}

	if (!heap)
		return;
#endif /*XDK_SUPPORT_MEMO_HEAP*/

	if (g_xdk_mou.tls_thr_dump != 0)
	{
		pdu = (if_dump_t*)(*pit->pf_thread_get_tls)(g_xdk_mou.tls_thr_dump);
		while (pdu && pdu->err_dump)
		{
			pdu->err_enable = 0;

			per = pdu->err_dump;
			pdu->err_dump = per->err_next;

#ifdef XDK_SUPPORT_MEMO_HEAP
			(*piv->pf_heap_free)(heap, per->err_code);
			(*piv->pf_heap_free)(heap, per->err_text);
			(*piv->pf_heap_free)(heap, per);
#else
			(*piv->pf_local_free)(per->err_code);
			(*piv->pf_local_free)(per->err_text);
			(*piv->pf_local_free)(per);
#endif
		}
#ifdef XDK_SUPPORT_MEMO_HEAP
		if (pdu)
			(*piv->pf_heap_free)(heap, (void*)pdu);
#else
		if (pdu)
			(*piv->pf_local_free)((void*)pdu);
#endif

		(*pit->pf_thread_set_tls)(g_xdk_mou.tls_thr_dump, 0);
	}

	if (g_xdk_mou.tls_thr_jump != 0)
	{
		pju = (if_jump_t*)(*pit->pf_thread_get_tls)(g_xdk_mou.tls_thr_jump);

		if (!error)
		{
			XDK_ASSERT(pju->if_buf == NULL);
		}

		if (pju)
		{
			clear_jump();
		}

#ifdef XDK_SUPPORT_MEMO_HEAP
		if(pju)
			(*piv->pf_heap_free)(heap, (void*)pju);
#else
		if(pju)
			(*piv->pf_local_free)((void*)pju);
#endif

		(*pit->pf_thread_set_tls)(g_xdk_mou.tls_thr_jump, 0);
	}

#ifdef XDK_SUPPORT_MEMO_DUMP
	(*pit->pf_criti_enter)(g_xdk_mou.dump_crit);

	if (!error)
	{
		xmem_dump();
	}

	delete_link(&g_xdk_mou.dump_link, &pzn->if_trak);

	(*pit->pf_criti_leave)(g_xdk_mou.dump_crit);
#endif

#ifdef XDK_SUPPORT_MEMO_HEAP
	(*piv->pf_heap_free)(heap, (void*)pzn);

	if (g_xdk_mou.if_opt & XDK_APARTMENT_THREAD)
		(*piv->pf_heap_destroy)(heap);
	else
		(*piv->pf_heap_clean)(heap);
		
#else /*XDK_SUPPORT_MEMO_HEAP*/
	(*piv->pf_local_free)((void*)pzn);
#endif

	(*pit->pf_thread_set_tls)(g_xdk_mou.tls_thr_zone, 0);
}

#endif //XDK_SUPPORT_THREAD

static int _is_big_endian()
{
	union cu
	{
		int i;
		char j;
	} n;

	n.i = 1;

	return (n.j == 1) ? 0 : 1;
}

//mount system call
void xdk_process_init(dword_t opt)
{
#if defined(XDK_SUPPORT_PROCESS)
    if_process_t* pro;
#endif
#if defined(XDK_SUPPORT_THREAD)
	if_thread_t* pit;
#endif

#ifdef XDK_SUPPORT_MEMO_HEAP
	void* heap = NULL;
#endif

	if (g_xdk_mou.if_ok)
		return;

    g_xdk_mou.if_ok = 1;
	g_xdk_mou.if_opt = opt;

	//g_xdk_mou.if_big = _is_big_endian();
    
#ifdef XDK_SUPPORT_PROCESS
    xdk_impl_process(&g_xdk_mou.if_process);
    
    pro = PROCESS_PROCESS_INTERFACE;
    (*pro->pf_process_safe)();
#endif

	xdk_impl_memo_local(&g_xdk_mou.if_memo);

#ifdef XDK_SUPPORT_MEMO_HEAP
	xdk_impl_memo_heap(&g_xdk_mou.if_memo);
#endif

#ifdef XDK_SUPPORT_MEMO_PAGE
	xdk_impl_memo_page(&g_xdk_mou.if_memo);
#endif

#ifdef XDK_SUPPORT_MEMO_CACHE
	xdk_impl_memo_cache(&g_xdk_mou.if_memo);
#endif

#ifdef XDK_SUPPORT_ERROR
	xdk_impl_error(&g_xdk_mou.if_error);
#endif

#ifdef XDK_SUPPORT_MBCS
	xdk_impl_mbcs(&g_xdk_mou.if_mbcs);
#endif

#ifdef XDK_SUPPORT_ASYNC
	xdk_impl_async(&g_xdk_mou.if_async);
#endif

#ifdef XDK_SUPPORT_THREAD
	xdk_impl_thread(&g_xdk_mou.if_thread);

#ifdef XDK_SUPPORT_THREAD_EVENT
	xdk_impl_thread_event(&g_xdk_mou.if_thread);
#endif

#ifdef XDK_SUPPORT_THREAD_CRITI
	xdk_impl_thread_criti(&g_xdk_mou.if_thread);
#endif

#ifdef XDK_SUPPORT_THREAD_MUTEX
	xdk_impl_thread_mutex(&g_xdk_mou.if_thread);
#endif

#ifdef XDK_SUPPORT_THREAD_SEMAP
	xdk_impl_thread_semap(&g_xdk_mou.if_thread);
#endif

#ifdef XDK_SUPPORT_THREAD_QUEUE
	xdk_impl_thread_queue(&g_xdk_mou.if_thread);
#endif

#endif //XDK_SUPPORT_THREAD

#ifdef XDK_SUPPORT_TIMER
	xdk_impl_timer(&g_xdk_mou.if_timer);
#endif

#ifdef XDK_SUPPORT_RANDOM
	xdk_impl_random(&g_xdk_mou.if_random);
#endif

#ifdef XDK_SUPPORT_FILE
	xdk_impl_file(&g_xdk_mou.if_file);
#endif

#ifdef XDK_SUPPORT_FILE_FIND
	xdk_impl_file_find(&g_xdk_mou.if_file);
#endif

#ifdef XDK_SUPPORT_SHARE
	xdk_impl_share(&g_xdk_mou.if_share);
#endif

#ifdef XDK_SUPPORT_PIPE
	xdk_impl_pipe(&g_xdk_mou.if_pipe);
#endif

#ifdef XDK_SUPPORT_COMM
	xdk_impl_comm(&g_xdk_mou.if_comm);
#endif

#ifdef XDK_SUPPORT_CONS
	xdk_impl_cons(&g_xdk_mou.if_cons);
#endif

#ifdef XDK_SUPPORT_SOCK
	xdk_impl_socket(&g_xdk_mou.if_socket);
#endif 

#ifdef XDK_SUPPORT_DATE
	xdk_impl_date(&g_xdk_mou.if_date);
#endif

#ifdef XDK_SUPPORT_THREAD
	pit = PROCESS_THREAD_INTERFACE;
	g_xdk_mou.thread_id = (*pit->pf_thread_get_id)();

	//create thread id, heap, jump local storage index
	(*pit->pf_thread_create_tls)(&g_xdk_mou.tls_thr_zero);
	(*pit->pf_thread_create_tls)(&g_xdk_mou.tls_thr_zone);
	(*pit->pf_thread_create_tls)(&g_xdk_mou.tls_thr_jump);
	(*pit->pf_thread_create_tls)(&g_xdk_mou.tls_thr_dump);
#else
	pim = PROCESS_MEMO_INTERFACE;

#ifdef XDK_SUPPORT_MEMO_HEAP
	heap = (*pim->pf_process_heap)();

	g_xdk_mou.pif_zone = (if_zone_t*)(*pim->pf_heap_alloc)(heap, sizeof(if_zone_t));
	g_xdk_mou.pif_zone->if_heap = heap;

	g_xdk_mou.pif_jump = (if_jump_t*)(*pim->pf_heap_alloc)(heap, sizeof(if_jump_t));
	g_xdk_mou.pif_dump = (if_dump_t*)(*pim->pf_heap_alloc)(heap, sizeof(if_dump_t));
#else
	g_xdk_mou.pif_zone = (if_zone_t*)(*pim->pf_local_alloc)(sizeof(if_heap_t));

	g_xdk_mou.pif_jump = (if_jump_t*)(*pim->pf_local_alloc)(sizeof(if_jump_t));
	g_xdk_mou.pif_dump = (if_dump_t*)(*pim->pf_local_alloc)(sizeof(if_dump_t));
#endif //XDK_SUPPORT_MEMO_HEAP

	g_xdk_mou.pif_jump->if_buf = NULL;
	g_xdk_mou.pif_jump->if_index = -1;
	g_xdk_mou.pif_jump->if_size = 0;

#endif //XDK_SUPPORT_TRHEAD

#if defined(XDK_SUPPORT_MEMO_DUMP) && defined(XDK_SUPPORT_THREAD)
	pit = PROCESS_THREAD_INTERFACE;
	g_xdk_mou.dump_crit = (*pit->pf_criti_create)();
	init_root_link(&g_xdk_mou.dump_link);
#endif

#ifdef XDK_SUPPORT_THREAD
	//init the master thread local heap, error handler
	xdk_thread_init(1);
#endif
    
#ifdef XDK_SUPPORT_SOCK
	if (g_xdk_mou.if_socket.pf_socket_startup)
	{
		(*g_xdk_mou.if_socket.pf_socket_startup)();
	}
#endif

#ifdef XDK_SUPPORT_ACP
	acp_init();
#endif

#ifdef XDK_SUPPORT_GLYPH
	gly_init();
#endif
}

//unmount system call
void xdk_process_uninit()
{
#ifdef XDK_SUPPORT_THREAD
	if_thread_t* pit;
#endif

	if (!g_xdk_mou.if_ok)
		return;

#ifdef XDK_SUPPORT_GLYPH
	gly_uninit();
#endif

#ifdef XDK_SUPPORT_ACP
	acp_uninit();
#endif

#ifdef XDK_SUPPORT_SOCK
	(*g_xdk_mou.if_socket.pf_socket_cleanup)();
#endif

#ifdef XDK_SUPPORT_THREAD
	pit = PROCESS_THREAD_INTERFACE;

	//uninit the master thread local heap, error handler
	xdk_thread_uninit(0);

	//destroy thread id, heap, jump local storage index
	(*pit->pf_thread_destroy_tls)(g_xdk_mou.tls_thr_zero);
	g_xdk_mou.tls_thr_zero = 0;
	(*pit->pf_thread_destroy_tls)(g_xdk_mou.tls_thr_zone);
	g_xdk_mou.tls_thr_zone = 0;
	(*pit->pf_thread_destroy_tls)(g_xdk_mou.tls_thr_jump);
	g_xdk_mou.tls_thr_jump = 0;
	(*pit->pf_thread_destroy_tls)(g_xdk_mou.tls_thr_dump);
	g_xdk_mou.tls_thr_dump = 0;
#ifdef XDK_SUPPORT_MEMO_DUMP
	(*pit->pf_criti_enter)(g_xdk_mou.dump_crit);

	thread_dump();

	(*pit->pf_criti_leave)(g_xdk_mou.dump_crit);

	(*pit->pf_criti_destroy)(g_xdk_mou.dump_crit);
#endif

#else

	pim = PROCESS_MEMO_INTERFACE;

#ifdef XDK_SUPPORT_MEMO_HEAP
	(*pim->pf_heap_free)(g_xdk_mou.pif_zone->if_heap, g_xdk_mou.pif_dump);
	g_xdk_mou.pif_dump = NULL;
	(*pim->pf_heap_free)(g_xdk_mou.pif_zone->if_heap, g_xdk_mou.pif_jump);
	g_xdk_mou.pif_jump = NULL;
	(*pim->pf_heap_free)(g_xdk_mou.pif_zone->if_heap, g_xdk_mou.pif_zone);
	g_xdk_mou.pif_zone = NULL;
#else
	(*pim->pf_local_free)(g_xdk_mou.pif_dump);
	g_xdk_mou.pif_dump = NULL;
	(*pim->pf_local_free)(g_xdk_mou.pif_jump);
	g_xdk_mou.pif_jump = NULL;
	(*pim->pf_local_free)(g_xdk_mou.pif_zone);
	g_xdk_mou.pif_heap = NULL;
#endif //XDK_SUPPORT_MEMO_HEAP

#endif //XDK_SUPPORT_THREAD

	memset((void*)&g_xdk_mou, 0, sizeof(xdk_mou_t));
}

#ifdef XDK_SUPPORT_MEMO_HEAP
void xdk_process_clean()
{
	if_memo_t* piv;
	res_heap_t heap;

	piv = PROCESS_MEMO_INTERFACE;

	XDK_ASSERT(piv != NULL);

	heap = (*piv->pf_process_heap)();
	(*piv->pf_heap_clean)(heap);
}
#else
void xdk_process_clean()
{
	return;
}
#endif //XDK_SUPPORT_MEMO_HEAP


#if defined(XDK_SUPPORT_MEMO_DUMP) && defined(XDK_SUPPORT_THREAD)

void thread_dump()
{
	link_t_ptr plk, nlk;
	tchar_t token[ERR_LEN + 1];
	dword_t dw;

	if_error_t* pie;
#ifdef XDK_SUPPORT_ERROR
	pie = PROCESS_ERROR_INTERFACE;
	XDK_ASSERT(pie != NULL);
#endif

	plk = get_first_link(&g_xdk_mou.dump_link);
	while (plk)
	{
		nlk = get_next_link(plk);

#ifdef XDK_SUPPORT_ERROR
		dw = xsprintf(token, _T("thread dump:[thread id: %d]"), plk->tag);
		(*pie->pf_error_print)(token);
#endif

		delete_link(&g_xdk_mou.dump_link, plk);
		plk = nlk;
	}
}

void xmem_dump()
{
	link_t_ptr plk, nlk;
	dword_t tid,len;
	vword_t dump;
	tchar_t token[4096];

	if_thread_t* pit;
	if_error_t* pie;
	if_zone_t* pih;

	pih = THREAD_ZONE_INTERFACE;
	pit = PROCESS_THREAD_INTERFACE;

#ifdef XDK_SUPPORT_ERROR
	pie = PROCESS_ERROR_INTERFACE;
	XDK_ASSERT(pie != NULL);
#endif

	XDK_ASSERT(pih != NULL && pit != NULL);

	plk = get_first_link(&pih->if_dump);
	while (plk)
	{
		nlk = get_next_link(plk);

		dump = *((vword_t*)((byte_t*)plk + sizeof(link_t)));
		
		tid = (*pit->pf_thread_get_id)();

#ifdef XDK_SUPPORT_ERROR
#if defined(UNICODE) || defined(_UNICODE)
		len = xsprintf(token, _T("memory leak:[thread id: %d, %S]\n"), tid, (char*)dump);
#else
		len = xsprintf(token, _T("memory leak:[thread id: %d, %s]\n"), tid, (char*)dump);
#endif
		(*pie->pf_error_print)(token);
#endif

		delete_link(&pih->if_dump, plk);
		plk = nlk;
	}
}

#else //XDK_SUPPORT_MEMO_DUMP

void thread_dump()
{
	return;
}

void xmem_dump()
{
	return;
}

#endif //XDK_SUPPORT_THREAD

