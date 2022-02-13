/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc http service document

	@module	srvhttp.c | implement file

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

#include "srvhttp.h"

#include "../xdknet.h"
#include "../xdkimp.h"
#include "../xdkoem.h"
#include "../xdkstd.h"

/*****************************************************************************************************/

static void _http_bio_dispatch(xhand_t bio, void* param)
{
	http_listen_t* plis = (http_listen_t*)param;
	xhand_t http = NULL;

	XDK_ASSERT(plis != NULL);

	http = xhttp_server(bio);

	if (plis->pf_dispatch)
	{
		(*plis->pf_dispatch)((xhand_t)http, plis->pf_param);
	}

	xhttp_close(http);
}

http_listen_t* xhttp_start_thread(unsigned short port, NET_SECU secu, PF_HTTPS_DISPATCH pf_dispatch, void* pf_param)
{
	http_listen_t* plis;

	plis = (http_listen_t*)xmem_alloc(sizeof(http_listen_t));
	
	plis->pf_dispatch = pf_dispatch;
	plis->pf_param = (void*)pf_param;

	plis->lis_tcp = xtcp_start_thread(port, secu, _http_bio_dispatch, (void*)plis);

	if (!plis->lis_tcp)
	{
		xmem_free(plis);
		return 0;
	}

	return plis;
}

http_listen_t* xhttp_start_process(unsigned short port, NET_SECU secu, const tchar_t* sz_module, tchar_t* sz_cmdline)
{
	http_listen_t* plis;

	plis = (http_listen_t*)xmem_alloc(sizeof(http_listen_t));
	
	plis->lis_tcp = xtcp_start_process(port, secu, sz_module, sz_cmdline);

	if (!plis->lis_tcp)
	{
		xmem_free(plis);
		return 0;
	}

	return plis;
}

void xhttp_stop(http_listen_t* plis)
{
	if (plis->lis_tcp)
		xtcp_stop(plis->lis_tcp);

	xmem_free(plis);
}

