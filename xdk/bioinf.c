/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc bio interface document

	@module	bioinf.c | implement file

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

#include "bioinf.h"

#include "xdkimp.h"
#include "xdkoem.h"
#include "xdkstd.h"
#include "xdknet.h"

bool_t get_bio_interface(xhand_t io, bio_interface* pio)
{
	pio->fd = io;
	
	switch (io->tag)
	{
	case _HANDLE_BLOCK:
		pio->pf_read = xblock_read;
		pio->pf_write = xblock_write;
		return 1;
		break;
#ifdef XDK_SUPPORT_SOCK
	case _HANDLE_UDP:
		pio->pf_read = xudp_read;
		pio->pf_write = xudp_write;
		pio->pf_flush = xudp_flush;
		pio->pf_close = xudp_close;
		pio->pf_setopt = xudp_setopt;
		pio->pf_addr = xudp_addr_port;
		pio->pf_peer = xudp_peer_port;
		return 1;
		break;
	case _HANDLE_DTLS:
		pio->pf_read = xdtls_read;
		pio->pf_write = xdtls_write;
		pio->pf_flush = xdtls_flush;
		pio->pf_close = xdtls_close;
		pio->pf_setopt = xdtls_setopt;
		pio->pf_addr = xdtls_addr_port;
		pio->pf_peer = xdtls_peer_port;
		return 1;
		break;
	case _HANDLE_TCP:
		pio->pf_read = xtcp_read;
		pio->pf_write = xtcp_write;
		pio->pf_close = xtcp_close;
		pio->pf_setopt = xtcp_setopt;
		pio->pf_addr = xtcp_addr_port;
		pio->pf_peer = xtcp_peer_port;
		return 1;
		break;
	case _HANDLE_SSL:
		pio->pf_read = xssl_read;
		pio->pf_write = xssl_write;
		pio->pf_flush = xssl_flush;
		pio->pf_close = xssl_close;
		pio->pf_setopt = xssl_setopt;
		pio->pf_addr = xssl_addr_port;
		pio->pf_peer = xssl_peer_port;
		return 1;
		break;
	case _HANDLE_SSH:
		pio->pf_read = xssh_read;
		pio->pf_write = xssh_write;
		pio->pf_flush = xssh_flush;
		pio->pf_close = xssh_close;
		pio->pf_setopt = xssh_setopt;
		pio->pf_addr = xssh_addr_port;
		pio->pf_peer = xssh_peer_port;
		return 1;
		break;
	case _HANDLE_INET:
		pio->pf_read = xnetf_read_file;
		pio->pf_write = xnetf_write_file;
		return 1;
		break;
#endif
#ifdef XDK_SUPPORT_PIPE
	case _HANDLE_PIPE:
		pio->pf_read = xpipe_read;
		pio->pf_write = xpipe_write;
		pio->pf_flush = xpipe_flush;
		return 1;
		break;
#endif
#ifdef XDK_SUPPORT_COMM
	case _HANDLE_COMM:
		pio->pf_read = xcomm_read;
		pio->pf_write = xcomm_write;
		pio->pf_flush = xcomm_flush;
		return 1;
		break;
#endif
#ifdef XDK_SUPPORT_CONS
	case _HANDLE_CONS:
		pio->pf_read = xcons_read;
		pio->pf_write = xcons_write;
		pio->pf_flush = xcons_flush;
		return 1;
		break;
#endif
#ifdef XDK_SUPPORT_MEMO_CACHE
	case _HANDLE_CACHE:
		pio->pf_read = xcache_read;
		pio->pf_write = xcache_write;
		return 1;
		break;
#endif
#ifdef XDK_SUPPORT_SHARE
	case _HANDLE_SHARE:
		pio->pf_read = xshare_read;
		pio->pf_write = xshare_write;
		return 1;
		break;
#endif
	case _HANDLE_UNCF:
#ifdef XDK_SUPPORT_FILE
		pio->pf_read = xuncf_read_file;
		pio->pf_write = xuncf_write_file;
		pio->pf_flush = xuncf_flush_file;
		return 1;
		break;
#endif
	}

	return 0;
}
