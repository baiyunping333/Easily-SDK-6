/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc https service document

	@module	srvhttp.h | interface file

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

#ifndef _SRVHTTP_H
#define _SRVHTTP_H

#include "../xdkdef.h"
#include "../loginf.h"

#include "netdef.h"
#include "srvtcp.h"

typedef void(*PF_HTTPS_DISPATCH)(xhand_t http, void* param);

typedef struct _http_listen_t{

	tcp_listen_t* lis_tcp;
	
	void *pf_param;
	PF_HTTPS_DISPATCH pf_dispatch;
}http_listen_t;

#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION xhttp_start_thread: create http thread service routing.
@INPUT unsigned short port: the service port.
@INPUT NET_SECU secu: the security type, it can be _SECU_SSH, _SECU_SSL.
@INPUT PF_HTTPS_DISPATCH pf_dispatch: the callback service dispatch function.
@INPUT void* param: the user parameter transback into dispath function.
@RETURN http_listen_t*: if succeeds return listen struct, fails return NULL.
*/
EXP_API http_listen_t*	xhttp_start_thread(unsigned short port, NET_SECU secu, PF_HTTPS_DISPATCH pf_dispatch, void* pf_param);

/*
@FUNCTION xhttp_start_process: create http process service routing.
@INPUT NET_SECU secu: the security type, it can be _SECU_SSH, _SECU_SSL.
@INPUT unsigned short port: the service port.
@INPUT const tchar_t* sz_moudle: the process service moudle name.
@INPUT const tchar_t* sz_cmdline: the process execute command line.
@RETURN pnp_listen_t*: if succeeds return listen struct, fails return NULL.
*/
EXP_API http_listen_t*	xhttp_start_process(unsigned short port, NET_SECU secu, const tchar_t* sz_module, tchar_t* sz_cmdline);

/*
@FUNCTION xhttp_stop: stop the http service routing.
@INPUT http_listen_t* plis: the service listen struct.
@RETURN void: none.
*/
EXP_API void xhttp_stop(http_listen_t* plis);

#ifdef	__cplusplus
}
#endif

typedef struct _https_block_t{
	int cbs;

	bool_t is_thread;
	xhand_t http;

	tchar_t site[RES_LEN + 1];
	tchar_t object[PATH_LEN + 1];
	tchar_t path[PATH_LEN + 1];

	const loged_interface* plg;
}https_block_t;

typedef enum{
	HTTPS_INVOKE_SUCCEED = 0,
	HTTPS_INVOKE_WITHINFO = 1,
	HTTPS_INVOKE_WITHERROR = 2,
	HTTPS_INVOKE_PENDING = 100
}HTTPS_INVOKE_STATE;

typedef int(STDCALL *PF_HTTPS_INVOKE)(const tchar_t* method, const https_block_t* pb);

typedef struct _soap_block_t{
	link_t_ptr soap_request;
	link_t_ptr soap_response;

	tchar_t soap_action[PATH_LEN + 1];
	tchar_t soap_path[PATH_LEN + 1];
}soap_block_t;

typedef int(STDCALL *PF_SOAP_INVOKE)(const soap_block_t* pb);

typedef struct _wsdl_block_t{
	link_t_ptr soap_wsdl;

	tchar_t soap_path[PATH_LEN + 1];
}wsdl_block_t;

typedef int(STDCALL *PF_WSDL_INVOKE)(const wsdl_block_t* pb);

#endif /*_XDSHTTP_H*/