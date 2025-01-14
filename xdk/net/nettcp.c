﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc tcp document

	@module	nettcp.c | implement file

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

#include "nettcp.h"

#include "../xdknet.h"
#include "../xdkimp.h"
#include "../xdkoem.h"
#include "../xdkstd.h"

#ifdef XDK_SUPPORT_SOCK

#define MAX_LISTEN		5
#define TRY_MAX			3

typedef struct _tcp_context{
	handle_head head;		//reserved for xhand_t

	int type;
	res_file_t so;

	async_t* pov;
}tcp_context;

/****************************************************************************************************/

xhand_t xtcp_cli(unsigned short port, const tchar_t* addr)
{
	net_addr_t sin;
	struct linger li = { 1,10 };

	tcp_context* ptcp = NULL;
	res_file_t so = 0;
    int n;

	TRY_CATCH;

	so = socket_tcp(0, 0);
	if (so == INVALID_FILE)
	{
		raise_user_error(NULL, NULL);
	}

	xmem_zero((void*)&sin, sizeof(sin));

	fill_addr(&sin, port, addr);
	
    n = TRY_MAX;
	while(n && !socket_connect(so,(res_addr_t)&sin,sizeof(sin)))
	{
        n--;
	}
    
    if(!n)
    {
        raise_user_error(NULL, NULL);
    }

	socket_setopt(so, SO_LINGER, (const char*)&li, sizeof(struct linger));

	ptcp = (tcp_context*)xmem_alloc(sizeof(tcp_context));
	ptcp->head.tag = _HANDLE_TCP;

	ptcp->so = so;
	ptcp->type = _XTCP_TYPE_CLI;

	ptcp->pov = (async_t*)xmem_alloc(sizeof(async_t));
#if defined(_DEBUG) || defined(DEBUG)
	async_init(ptcp->pov, ASYNC_BLOCK, -1, INVALID_FILE);
#else
	async_init(ptcp->pov, ASYNC_BLOCK, TCP_BASE_TIMO, INVALID_FILE);
#endif

	END_CATCH;

	return &ptcp->head;
ONERROR:

	if (so)
		socket_close(so);

	if (ptcp)
	{
		if (ptcp->pov)
		{
			async_uninit(ptcp->pov);
			xmem_free(ptcp->pov);
		}

		xmem_free(ptcp);
	}

	XDK_TRACE_LAST;

	return NULL;
}

xhand_t xtcp_srv(res_file_t so)
{
	struct linger li = { 1, 10 };
	int zo = 0;

	tcp_context* ptcp = NULL;

	TRY_CATCH;

	if (so == INVALID_FILE)
	{
		raise_user_error(_T("xssl_srv"), _T("invalid socket handle"));
	}

	ptcp = (tcp_context*)xmem_alloc(sizeof(tcp_context));
	ptcp->head.tag = _HANDLE_TCP;

	ptcp->so = so;
	ptcp->type = _XTCP_TYPE_SRV;

	socket_setopt(ptcp->so, SO_LINGER, (const char*)&li, sizeof(struct linger));
	socket_setopt(ptcp->so, SO_SNDBUF, (const char*)&zo, sizeof(int));
	socket_setopt(ptcp->so, SO_RCVBUF, (const char*)&zo, sizeof(int));

	ptcp->pov = (async_t*)xmem_alloc(sizeof(async_t));
#if defined(_DEBUG) || defined(DEBUG)
	socket_set_nonblk(ptcp->so, 0);
	async_init(ptcp->pov, ASYNC_BLOCK, -1, INVALID_FILE);
#else
	async_init(ptcp->pov, ASYNC_EVENT, TCP_BASE_TIMO, INVALID_FILE);
#endif

	END_CATCH;

	return &ptcp->head;
ONERROR:

	if (ptcp)
	{
		if (ptcp->pov)
		{
			async_uninit(ptcp->pov);
			xmem_free(ptcp->pov);
		}

		xmem_free(ptcp);
	}

	XDK_TRACE_LAST;

	return NULL;
}

void  xtcp_close(xhand_t tcp)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	XDK_ASSERT(ptcp->type == _XTCP_TYPE_CLI || ptcp->type == _XTCP_TYPE_SRV);

	if (ptcp->type == _XTCP_TYPE_CLI)
	{
		//disable recive
		socket_shutdown(ptcp->so, 0);
	}
	else if (ptcp->type == _XTCP_TYPE_SRV)
	{
		//disable send
		socket_shutdown(ptcp->so, 1);
	}

	if (ptcp->type == _XTCP_TYPE_CLI)
	{
		socket_close(ptcp->so);
	}

	if (ptcp->pov)
	{
		async_uninit(ptcp->pov);
		xmem_free(ptcp->pov);
	}

	xmem_free(ptcp);
}

res_file_t xtcp_socket(xhand_t tcp)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	return ptcp->so;
}

int xtcp_type(xhand_t tcp)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	return ptcp->type;
}

bool_t xtcp_write(xhand_t tcp, const byte_t* buf, dword_t* pcb)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);
	dword_t size;

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	size = *pcb;
	ptcp->pov->size = 0;

	if (!socket_send(ptcp->so, (void*)buf, size, ptcp->pov))
	{
		*pcb = (dword_t)(ptcp->pov->size);
		return 0;
	}

	*pcb = (dword_t)(ptcp->pov->size);
	return 1;
}

bool_t xtcp_read(xhand_t tcp, byte_t* buf, dword_t* pcb)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);
	dword_t size;

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	size = *pcb;
	ptcp->pov->size = 0;

	if (!socket_recv(ptcp->so, buf, size, ptcp->pov))
	{
		*pcb = (dword_t)(ptcp->pov->size);
		return (*pcb)? 1 : 0;
	}

	*pcb = (dword_t)(ptcp->pov->size);
	return 1;
}

unsigned short xtcp_addr_port(xhand_t tcp, tchar_t* addr)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);
	net_addr_t na = { 0 };
	unsigned short port;

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	socket_addr(ptcp->so, &na);
	conv_addr(&na, &port, addr);

	return port;
}

unsigned short xtcp_peer_port(xhand_t tcp, tchar_t* addr)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);
	net_addr_t na = { 0 };
	unsigned short port;

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	socket_peer(ptcp->so, &na);
	conv_addr(&na, &port, addr);

	return port;
}

bool_t xtcp_setopt(xhand_t tcp, int oid, void* opt, int len)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	switch (oid)
	{
	case SOCK_OPTION_SNDBUF:
		socket_set_sndbuf(xtcp_socket(tcp), *(int*)opt);
		return 1;
	case SOCK_OPTION_RCVBUF:
		socket_set_rcvbuf(xtcp_socket(tcp), *(int*)opt);
		return 1;
	case SOCK_OPTION_NONBLK:
		socket_set_nonblk(xtcp_socket(tcp), *(bool_t*)opt);
		return 1;
	}

	return 0;
}

void xtcp_settmo(xhand_t tcp, dword_t tmo)
{
	tcp_context* ptcp = TypePtrFromHead(tcp_context, tcp);

	XDK_ASSERT(tcp && tcp->tag == _HANDLE_TCP);

	if (ptcp->pov)
	{
		ptcp->pov->timo = tmo;
	}
}

#endif //XDK_SUPPORT_SOCK
