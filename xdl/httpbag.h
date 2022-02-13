/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc http bag document

	@module	httpbag.h | interface file

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

#ifndef _HTTPBAG_H
#define _HTTPBAG_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_DOC


#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION xhttp_send_error: send a http error.
@INPUT xhand_t xhttp: the http handle.
@INPUT const tchar_t* http_code: the http code token.
@INPUT const tchar_t* http_info: the http code information.
@INPUT const tchar_t* errcode: the user error code.
@INPUT const tchar_t* errtext: the user error text.
@INPUT int len: the user error text length in characters.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t		xhttp_send_error(xhand_t xhttp, const tchar_t* http_code, const tchar_t* http_info, const tchar_t* errcode, const tchar_t* errtext, int len);

/*
@FUNCTION xhttp_recv_error: recv a http error.
@INPUT xhand_t xhttp: the http handle.
@OUTPUT tchar_t* http_code: the string buffer for returning http code.
@OUTPUT tchar_t* http_info: the string buffer for returning http code information.
@OUTPUT tchar_t* errcode: the string buffer for returning user error code.
@OUTPUT tchar_t* errtext: the string buffer for returning user error text.
@INPUT int max: the user error text string buffer size in characters.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t		xhttp_recv_error(xhand_t xhttp, tchar_t* http_code, tchar_t* http_info, tchar_t* errcode, tchar_t* errtext, int max);

/*
@FUNCTION xhttp_send_xml: send a http xml document.
@INPUT xhand_t xhttp: the http handle.
@INPUT link_t_ptr xml: the xml document.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t		xhttp_send_xml(xhand_t xhttp, link_t_ptr xml);

/*
@FUNCTION xhttp_recv_xml: recv a http xml document.
@INPUT xhand_t xhttp: the http handle.
@OUTPUT link_t_ptr xml: the xml document for receiving.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t		xhttp_recv_xml(xhand_t xhttp, link_t_ptr xml);

/*
@FUNCTION xhttp_send_json: send a http json document.
@INPUT xhand_t xhttp: the http handle.
@INPUT link_t_ptr json: the json document.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t		xhttp_send_json(xhand_t xhttp, link_t_ptr json);

/*
@FUNCTION xhttp_recv_json: recv a http json document.
@INPUT xhand_t xhttp: the http handle.
@OUTPUT link_t_ptr json: the json document for receiving.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t		xhttp_recv_json(xhand_t xhttp, link_t_ptr json);

/*
@FUNCTION xhttp_invoke_soap: call soap action.
@INPUT xhand_t xhttp: the http handle.
@INPUT link_t_ptr send_soap: the input soap document.
@OUTPUT link_t_ptr recv_soap: the output soap document.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xhttp_invoke_soap(xhand_t xhttp, link_t_ptr send_soap, link_t_ptr recv_soap);

/*
@FUNCTION xhttp_invoke_wsdl: get soap wsdl.
@INPUT const tchar_t* sz_url: the soap service url.
@OUTPUT link_t_ptr wsdl: the output wsdl document.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xhttp_invoke_wsdl(const tchar_t* sz_url, link_t_ptr wsdl);

EXP_API bool_t get_wsdl_soap_info(link_t_ptr wsdl, const tchar_t* sz_srv, const tchar_t* sz_rpc, tchar_t* sz_loc, tchar_t* sz_act, tchar_t* sz_tns, link_t_ptr req_sch, link_t_ptr res_sch);

EXP_API void set_wsdl_soap_info(link_t_ptr wsdl, const tchar_t* sz_srv, const tchar_t* sz_rpc, const tchar_t* sz_loc, const tchar_t* sz_act, const tchar_t* sz_tns, link_t_ptr req_sch, link_t_ptr res_sch);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_DOC*/

#endif /*GRIDBAG_H*/