﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdn udp document

	@module	netudp.c | implement file

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

#include "netudp.h"

#include "../xdknet.h"
#include "../xdkimp.h"
#include "../xdkoem.h"
#include "../xdkstd.h"

#if defined(XDK_SUPPORT_SOCK)

typedef struct _udp_context{
	handle_head head;		//head for xhand_t

	int type;
	res_file_t so;
	async_t* pov;

	unsigned short port;
	tchar_t addr[ADDR_LEN + 1];

	dword_t pkg_size;

	byte_t* snd_pdu;
	dword_t snd_bys;

	byte_t* rcv_pdu;
	dword_t rcv_bys;
	dword_t rcv_ret;
}udp_context;

/*********************************************************************************************/

xhand_t xudp_cli(unsigned short port, const tchar_t* addr)
{
	udp_context* pudp = NULL;
	int zo;

	pudp = (udp_context*)xmem_alloc(sizeof(udp_context));
	pudp->head.tag = _HANDLE_UDP;
	pudp->type = _XUDP_TYPE_CLI;

	pudp->so = INVALID_FILE;
	pudp->port = port;
	xscpy(pudp->addr, addr);

	pudp->pkg_size = UDP_PKG_SIZE;

	pudp->so = socket_udp(0, FILE_OPEN_OVERLAP);
	if (pudp->so == INVALID_FILE)
	{
		xmem_free(pudp);
		return NULL;
	}
	zo = 1;
	socket_setopt(pudp->so, SO_REUSEADDR, (const char*)&zo, sizeof(int));

	pudp->pov = (async_t*)xmem_alloc(sizeof(async_t));
	async_init(pudp->pov, ASYNC_EVENT, UDP_BASE_TIMO, INVALID_FILE);

	pudp->snd_pdu = (byte_t*)xmem_alloc(pudp->pkg_size);
	pudp->snd_bys = 0;

	pudp->rcv_pdu = (byte_t*)xmem_alloc(pudp->pkg_size);
	pudp->rcv_bys = 0;
	pudp->rcv_ret = 0;

	return &(pudp->head);
}

xhand_t xudp_srv(unsigned short port, const tchar_t* addr, const byte_t* pack, dword_t size)
{
	udp_context* pudp = NULL;
	int zo;

	if (size > MTU_MAX_SIZE)
	{
		set_last_error(_T("xudp_srv"), _T("too large package size"), -1);
		return NULL;
	}

	pudp = (udp_context*)xmem_alloc(sizeof(udp_context));
	pudp->head.tag = _HANDLE_UDP;
	pudp->type = _XUDP_TYPE_SRV;

	pudp->so = INVALID_FILE;
	pudp->port = port;
	xscpy(pudp->addr, addr);

	pudp->pkg_size = (size > UDP_PKG_SIZE)? size : UDP_PKG_SIZE;

	pudp->so = socket_udp(0, FILE_OPEN_OVERLAP);
	if (pudp->so == INVALID_FILE)
	{
		xmem_free(pudp);
		return NULL;
	}
	zo = 1;
	socket_setopt(pudp->so, SO_REUSEADDR, (const char*)&zo, sizeof(int));

	pudp->pov = (async_t*)xmem_alloc(sizeof(async_t));
	async_init(pudp->pov, ASYNC_EVENT, UDP_BASE_TIMO, INVALID_FILE);

	pudp->snd_pdu = (byte_t*)xmem_alloc(pudp->pkg_size);
	pudp->snd_bys = 0;

	pudp->rcv_pdu = (byte_t*)xmem_alloc(pudp->pkg_size);
	pudp->rcv_bys = 0;
	pudp->rcv_ret = 0;

	if (size)
	{
		xmem_copy((void*)(pudp->rcv_pdu), (void*)pack, size);
		pudp->rcv_bys = size;
		pudp->rcv_ret = 0;
	}

	return &pudp->head;
}

bool_t  xudp_connect(xhand_t udp, unsigned short port, const tchar_t* addr)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);
	net_addr_t sin = { 0 };

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	pudp->port = port;
	xscpy(pudp->addr, addr);

	fill_addr(&sin, port, addr);

	return socket_connect(pudp->so, (res_addr_t)&sin, sizeof(sin));
}

bool_t  xudp_bind(xhand_t udp, unsigned short bind)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);
	net_addr_t sin = { 0 };

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	fill_addr(&sin, bind, NULL);

	return socket_bind(pudp->so, (res_addr_t)&sin, sizeof(sin));
}

void  xudp_close(xhand_t udp)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	if (pudp->so)
	{
		socket_close(pudp->so);
	}

	if (pudp->rcv_pdu)
		xmem_free(pudp->rcv_pdu);

	if (pudp->snd_pdu)
		xmem_free(pudp->snd_pdu);

	if (pudp->pov)
	{
		async_uninit(pudp->pov);
		xmem_free(pudp->pov);
	}

	xmem_free(pudp);
}

int xudp_type(xhand_t udp)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	return pudp->type;
}

res_file_t xudp_socket(xhand_t udp)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	return pudp->so;
}

void xudp_set_package(xhand_t udp, dword_t size)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	pudp->pkg_size = (size > UDP_PKG_SIZE) ? size : UDP_PKG_SIZE;

	pudp->snd_pdu = (byte_t*)xmem_realloc(pudp->snd_pdu, pudp->pkg_size);
	pudp->rcv_pdu = (byte_t*)xmem_realloc(pudp->rcv_pdu, pudp->pkg_size);

	socket_set_rcvbuf(pudp->so, 32 * 1024);
	socket_set_sndbuf(pudp->so, 16 * 1024);
}

dword_t xudp_get_package(xhand_t udp)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	return pudp->pkg_size;
}

bool_t xudp_write(xhand_t udp, const byte_t* buf, dword_t* pb)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);
	dword_t bys, dw;
	net_addr_t sin = { 0 };
	int addr_len;

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	dw = *pb;
	while (dw)
	{
		bys = (pudp->snd_bys + dw > pudp->pkg_size) ? (pudp->pkg_size - pudp->snd_bys) : dw;
		xmem_copy((void*)(pudp->snd_pdu + pudp->snd_bys), (void*)buf, bys);
		pudp->snd_bys += bys;

		if (pudp->snd_bys == pudp->pkg_size)
		{
			fill_addr(&sin, pudp->port, pudp->addr);
			addr_len = sizeof(net_addr_t);

			bys = pudp->snd_bys;
			pudp->pov->size = 0;
			if (!socket_sendto(pudp->so, (res_addr_t)&sin, addr_len, (void*)(pudp->snd_pdu), bys, pudp->pov))
			{
				*pb -= dw;
				return 0;
			}

			if (!(pudp->pov->size))
			{
				break;
			}

			pudp->snd_bys = bys - (dword_t)(pudp->pov->size);
			if (pudp->snd_bys && (pudp->snd_bys < pudp->pkg_size))
			{
				xmem_copy((void*)(pudp->snd_pdu), (void*)(pudp->snd_pdu + pudp->pkg_size - pudp->snd_bys), pudp->snd_bys);
			}
		}

		dw -= bys;
	}

	*pb -= dw;

	return 1;
}

bool_t xudp_flush(xhand_t udp)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);
	dword_t dw = 0;
	net_addr_t sin = { 0 };
	int addr_len;

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	if (pudp->snd_bys)
	{
		fill_addr(&sin, pudp->port, pudp->addr);
		addr_len = sizeof(net_addr_t);

		dw = pudp->snd_bys;
		pudp->pov->size = 0;
		if (!socket_sendto(pudp->so, (res_addr_t)&sin, addr_len, (void*)(pudp->snd_pdu), dw, pudp->pov))
		{
			return 0;
		}

		dw = (dword_t)(pudp->pov->size);
		pudp->snd_bys = 0;
	}

	return (0 == pudp->snd_bys) ? 1 : 0;
}

bool_t xudp_read(xhand_t udp, byte_t* buf, dword_t* pb)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);
	dword_t bys, dw;
	net_addr_t na = { 0 };
	int addr_len;

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	dw = *pb;
	*pb = 0;
	while (dw)
	{
		if (!pudp->rcv_bys)
		{
			bys = pudp->pkg_size;
			addr_len = sizeof(net_addr_t);
			pudp->pov->size = 0;
			if (!socket_recvfrom(pudp->so, (res_addr_t)&na, &addr_len, (void*)(pudp->rcv_pdu), bys, pudp->pov))
			{
				*pb = pudp->pov->size;
				return 0;
			}

			if (addr_len)
			{
				conv_addr(&na, &pudp->port, pudp->addr);
			}

			pudp->rcv_bys = (dword_t)(pudp->pov->size);
			pudp->rcv_ret = 0;
		}
		
		bys = (pudp->rcv_bys < dw) ? pudp->rcv_bys : dw;
		xmem_copy((void*)(buf + *pb), (void*)(pudp->rcv_pdu + pudp->rcv_ret), bys);

		pudp->rcv_ret += bys;
		pudp->rcv_bys -= bys;
		dw -= bys;
		*pb += bys;

		if (bys)
			break;
	}

	return (*pb)? 1: 0;
}

unsigned short xudp_addr_port(xhand_t udp, tchar_t* addr)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);
	net_addr_t na = { 0 };
	unsigned short port;

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	socket_addr(pudp->so, &na);
	conv_addr(&na, &port, addr);

	return port;
}

unsigned short xudp_peer_port(xhand_t udp, tchar_t* addr)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);
	net_addr_t na = { 0 };
	unsigned short port;

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	socket_peer(pudp->so, &na);
	conv_addr(&na, &port, addr);

	return port;
}

bool_t xudp_setopt(xhand_t udp, int oid, void* opt, int len)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	switch (oid)
	{
	case SOCK_OPTION_SNDBUF:
		socket_set_sndbuf(pudp->so, *(int*)opt);
		return 1;
	case SOCK_OPTION_RCVBUF:
		socket_set_rcvbuf(pudp->so, *(int*)opt);
		return 1;
	case SOCK_OPTION_NONBLK:
		socket_set_nonblk(pudp->so, *(bool_t*)opt);
		return 1;
	}

	return 0;
}

void xudp_settmo(xhand_t udp, dword_t tmo)
{
	udp_context* pudp = TypePtrFromHead(udp_context, udp);

	XDK_ASSERT(udp && udp->tag == _HANDLE_UDP);

	if (pudp->pov)
	{
		pudp->pov->timo = tmo;
	}
}


#endif /*XDK_SUPPORT_SOCK*/
