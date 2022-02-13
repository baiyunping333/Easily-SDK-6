/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc http bag document

	@module	httpbag.c | implement file

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

#include "httpbag.h"
#include "misce.h"

#include "xdldoc.h"
#include "xdlutil.h"

#ifdef XDL_SUPPORT_DOC

dword_t xhttp_format_error(bool_t b_json, const tchar_t* encoding, const tchar_t* errcode, const tchar_t* errtext, int slen, byte_t* buf, dword_t max)
{
	link_t_ptr ptr_xml, ptr_dom;
	link_t_ptr nlk;

	dword_t nlen = 0;
	byte_t* sz_buf = NULL;

	if (b_json)
	{
		ptr_dom = create_json_doc();
	}
	else
	{
		ptr_xml = create_xml_doc();

		if (!is_null(encoding))
			set_xml_encoding(ptr_xml, encoding, -1);

		ptr_dom = get_xml_dom_node(ptr_xml);
		set_dom_node_name(ptr_dom, HTTP_FAULT, -1);
	}

	nlk = insert_dom_node(ptr_dom, LINK_LAST);
	set_dom_node_name(nlk, HTTP_FAULT_CODE, -1);
	set_dom_node_text(nlk, errcode, -1);

	nlk = insert_dom_node(ptr_dom, LINK_LAST);
	set_dom_node_name(nlk, HTTP_FAULT_STRING, -1);
	set_dom_node_text(nlk, errtext, slen);

	if (b_json)
	{
		nlen = format_json_doc_to_bytes(ptr_dom, buf, max, parse_charset(encoding));

		destroy_json_doc(ptr_dom);
	}
	else
	{
		nlen = format_xml_doc_to_bytes(ptr_xml, buf, max);

		destroy_xml_doc(ptr_xml);
	}

	return nlen;
}

bool_t xhttp_parse_error(bool_t b_json, const tchar_t* encoding, const byte_t* buf, dword_t len, tchar_t* errcode, tchar_t* errtext, int max)
{
	link_t_ptr nlk, ptr_dom, ptr_xml;
	bool_t b_rt;

	if (b_json)
	{
		ptr_dom = create_json_doc();
		b_rt = parse_json_doc_from_bytes(ptr_dom, buf, len, parse_charset(encoding));
		if (!b_rt)
		{
			destroy_json_doc(ptr_dom);
			return 0;
		}
	}
	else
	{
		ptr_xml = create_xml_doc();
		b_rt = parse_xml_doc_from_bytes(ptr_xml, buf, len);
		if (!b_rt)
		{
			destroy_xml_doc(ptr_xml);
			return 0;
		}

		ptr_dom = get_xml_dom_node(ptr_xml);
		if (compare_text(get_dom_node_name_ptr(ptr_dom), -1, HTTP_FAULT, -1, 1) != 0)
		{
			destroy_xml_doc(ptr_xml);
			return 0;
		}
	}

	nlk = get_dom_first_child_node(ptr_dom);
	while (nlk)
	{
		if (compare_text(get_dom_node_name_ptr(nlk), -1, HTTP_FAULT_CODE, -1, 1) == 0)
		{
			if (errcode)
			{
				get_dom_node_text(nlk, errcode, NUM_LEN);
			}
		}
		else if (compare_text(get_dom_node_name_ptr(nlk), -1, HTTP_FAULT_STRING, -1, 1) == 0)
		{
			if (errtext)
			{
				get_dom_node_text(nlk, errtext, max);
			}
		}
		nlk = get_dom_next_sibling_node(nlk);
	}

	if (b_json)
		destroy_json_doc(ptr_dom);
	else
		destroy_xml_doc(ptr_xml);

	return 1;
}

bool_t xhttp_send_error(xhand_t xhttp, const tchar_t* http_code, const tchar_t* http_info, const tchar_t* errcode, const tchar_t* errtext, int slen)
{
	int type;
	byte_t* sz_buf = NULL;
	dword_t nlen;
	bool_t b_rt;
	tchar_t encoding[RES_LEN + 1] = { 0 };
	tchar_t fsize[NUM_LEN + 1] = { 0 };
	int encode = 0;
	bool_t b_json = 0;

	type = xhttp_type(xhttp);

	XDK_ASSERT(type == _XHTTP_TYPE_SRV);

	xhttp_get_request_content_type(xhttp, encoding, RES_LEN);

	b_json = (compare_text(encoding, xslen(HTTP_HEADER_CONTENTTYPE_APPJSON), HTTP_HEADER_CONTENTTYPE_APPJSON, -1, 1) == 0) ? 1 : 0;

	xhttp_get_request_accept_charset(xhttp, encoding, RES_LEN);
	if (is_null(encoding))
	{
		xhttp_get_request_content_type_charset(xhttp, encoding, RES_LEN);
	}

	encode = parse_charset(encoding);
	if (encode)
	{
		format_charset(encode, encoding);
	}
	else
	{
		xscpy(encoding, CHARSET_UTF8);
	}

	nlen = xhttp_format_error(b_json, encoding, errcode, errtext, slen, NULL, MAX_LONG);
	sz_buf = (byte_t*)xmem_alloc(nlen);
	nlen = xhttp_format_error(b_json, encoding, errcode, errtext, slen, sz_buf, nlen);

	xsprintf(fsize, _T("%d"), nlen);
	xhttp_set_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);
	if (b_json)
		xhttp_set_response_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	else
		xhttp_set_response_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPXML, -1);
	xhttp_set_response_content_type_charset(xhttp, encoding, -1);

	if (!is_null(http_code))
		xhttp_set_response_code(xhttp, http_code);
	if (!is_null(http_info))
		xhttp_set_response_message(xhttp, http_info, -1);

	b_rt = xhttp_send_full(xhttp, sz_buf, nlen);

	xmem_free(sz_buf);
	sz_buf = NULL;

	return b_rt;
}

bool_t xhttp_recv_error(xhand_t xhttp, tchar_t* http_code, tchar_t* http_info, tchar_t* errcode, tchar_t* errtext, int max)
{
	int type;
	byte_t** pbuf = NULL;
	tchar_t encoding[RES_LEN + 1] = { 0 };
	dword_t nlen = 0;
	bool_t b_rt;
	bool_t b_json = 0;

	type = xhttp_type(xhttp);

	XDK_ASSERT(type == _XHTTP_TYPE_CLI);

	xhttp_get_response_content_type(xhttp, encoding, RES_LEN);

	b_json = (compare_text(encoding, xslen(HTTP_HEADER_CONTENTTYPE_APPJSON), HTTP_HEADER_CONTENTTYPE_APPJSON, -1, 1) == 0) ? 1 : 0;

	xhttp_get_response_content_type_charset(xhttp, encoding, RES_LEN);
	if (is_null(encoding))
	{
		xscpy(encoding, CHARSET_UTF8);
	}

	if (http_code)
	{
		xhttp_get_response_code(xhttp, http_code);
	}
	if (http_info)
	{
		xhttp_get_response_message(xhttp, http_info, META_LEN);
	}

	pbuf = bytes_alloc();

	b_rt = xhttp_recv_full(xhttp, pbuf, &nlen);

	if (b_rt)
	{
		b_rt = xhttp_parse_error(b_json, encoding, *pbuf, nlen, errcode, errtext, max);
	}

	bytes_free(pbuf);
	pbuf = NULL;

	return b_rt;
}

bool_t xhttp_send_xml(xhand_t xhttp, link_t_ptr xml)
{
	int type;
	tchar_t trans[RES_LEN + 1] = { 0 };
	tchar_t fsize[NUM_LEN + 1] = { 0 };
	dword_t size = 0;
	stream_t stm;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	type = xhttp_type(xhttp);

	if (type == _XHTTP_TYPE_CLI && !xhttp_is_requested(xhttp))
	{
		xhttp_get_request_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, trans, RES_LEN);
		xhttp_get_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, NUM_LEN);

		if (compare_text(trans, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) != 0 && xstol(fsize) == 0)
		{
			size = format_xml_doc_to_bytes(xml, NULL, MAX_LONG);
			ltoxs(size, fsize, NUM_LEN);
			xhttp_set_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);
		}

		if (!xhttp_send_request(xhttp))
			return 0;
	}

	if (type == _XHTTP_TYPE_SRV && !xhttp_is_responsed(xhttp))
	{
		xhttp_get_response_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, trans, RES_LEN);
		xhttp_get_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, NUM_LEN);

		if (compare_text(trans, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) != 0 && xstol(fsize) == 0)
		{
			size = format_xml_doc_to_bytes(xml, NULL, MAX_LONG);
			ltoxs(size, fsize, NUM_LEN);
			xhttp_set_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);
		}

		if (!xhttp_send_response(xhttp))
			return 0;
	}

	stm = xhttp_get_send_stream(xhttp);
	XDK_ASSERT(stm != NULL);

	if (!format_xml_doc_to_stream(xml, stm))
		return 0;

	return stream_flush(stm);
}

bool_t xhttp_recv_xml(xhand_t xhttp, link_t_ptr xml)
{
	int type;
	stream_t stm;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	type = xhttp_type(xhttp);

	if (type == _XHTTP_TYPE_CLI && !xhttp_is_responsed(xhttp))
	{
		if (!xhttp_recv_response(xhttp))
			return 0;
	}

	if (type == _XHTTP_TYPE_SRV && !xhttp_is_requested(xhttp))
	{
		if (!xhttp_recv_request(xhttp))
			return 0;
	}

	stm = xhttp_get_recv_stream(xhttp);
	XDK_ASSERT(stm != NULL);

	return parse_xml_doc_from_stream(xml, stm);
}

bool_t xhttp_send_json(xhand_t xhttp, link_t_ptr json)
{
	int type;
	tchar_t trans[RES_LEN + 1] = { 0 };
	tchar_t fsize[NUM_LEN + 1] = { 0 };
	dword_t size = 0;
	tchar_t charset[RES_LEN + 1] = { 0 };
	stream_t stm;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	type = xhttp_type(xhttp);

	if (type == _XHTTP_TYPE_CLI && !xhttp_is_requested(xhttp))
	{
		xhttp_get_request_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, trans, RES_LEN);
		xhttp_get_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, NUM_LEN);
		xhttp_get_request_content_type_charset(xhttp, charset, INT_LEN);

		if (compare_text(trans, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) != 0 && xstol(fsize) == 0)
		{
			size = format_json_doc_to_bytes(json, NULL, MAX_LONG, parse_charset(charset));
			ltoxs(size, fsize, NUM_LEN);
			xhttp_set_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);
		}

		if (!xhttp_send_request(xhttp))
			return 0;
	}

	if (type == _XHTTP_TYPE_SRV && !xhttp_is_responsed(xhttp))
	{
		xhttp_get_response_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, trans, RES_LEN);
		xhttp_get_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, NUM_LEN);
		xhttp_get_response_content_type_charset(xhttp, charset, INT_LEN);

		if (compare_text(trans, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) != 0 && xstol(fsize) == 0)
		{
			size = format_json_doc_to_bytes(json, NULL, MAX_LONG, parse_charset(charset));
			ltoxs(size, fsize, NUM_LEN);
			xhttp_set_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);
		}

		if (!xhttp_send_response(xhttp))
			return 0;
	}

	stm = xhttp_get_send_stream(xhttp);
	XDK_ASSERT(stm != NULL);

	if (!format_json_doc_to_stream(json, stm))
		return 0;

	return stream_flush(stm);
}

bool_t xhttp_recv_json(xhand_t xhttp, link_t_ptr json)
{
	int type;
	stream_t stm;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	type = xhttp_type(xhttp);

	if (type == _XHTTP_TYPE_CLI && !xhttp_is_responsed(xhttp))
	{
		if (!xhttp_recv_response(xhttp))
			return 0;
	}

	if (type == _XHTTP_TYPE_SRV && !xhttp_is_requested(xhttp))
	{
		if (!xhttp_recv_request(xhttp))
			return 0;
	}

	stm = xhttp_get_recv_stream(xhttp);
	XDK_ASSERT(stm != NULL);

	return parse_json_doc_from_stream(json, stm);
}

bool_t xhttp_invoke_soap(xhand_t xhttp, link_t_ptr send_soap, link_t_ptr recv_soap)
{
	link_t_ptr xml;
	link_t_ptr nlk_req;

	tchar_t sz_action[PATH_LEN + 1] = { 0 };
	tchar_t sz_xmlns[PATH_LEN + 1] = { 0 };
	tchar_t sz_code[NUM_LEN + 1] = { 0 };
	tchar_t sz_text[ERR_LEN + 1] = { 0 };
	tchar_t fsize[NUM_LEN + 1] = { 0 };
	int encode = 0;

	const tchar_t *sz_method, *sz_nsurl;
	int nlen;
	bool_t b_rt;

	xhttp_get_request_header(xhttp, HTTP_HEADER_SOAPACTION, -1, sz_action, PATH_LEN);

	if (is_null(sz_action))
	{
		nlk_req = get_soap_request_node(send_soap);
		if (!nlk_req)
		{
			return 0;
		}

		sz_method = get_dom_node_name_ptr(nlk_req);
		sz_nsurl = get_dom_node_nsurl_ptr(nlk_req);

		if (!is_null(sz_nsurl))
		{
			nlen = xslen(sz_nsurl);
			if (sz_nsurl[nlen - 1] == L'/')
				xsprintf(sz_action, _T("%s%s"), sz_nsurl, sz_method);
			else
				xsprintf(sz_action, _T("%s/%s"), sz_nsurl, sz_method);
		}
		else
		{
			xscpy(sz_action, _T(""));
		}

		xhttp_set_request_header(xhttp, HTTP_HEADER_SOAPACTION, -1, sz_action, -1);
	}

	xml = upcast_dom_to_xml(send_soap);

	nlen = format_xml_doc_to_bytes(xml, NULL, MAX_LONG);
	xsprintf(fsize, _T("%d"), nlen);
	xhttp_set_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);

	b_rt = xhttp_send_xml(xhttp, xml);

	send_soap = downcast_xml_to_dom(xml);

	if (!b_rt)
	{
		return 0;
	}

	xml = upcast_dom_to_xml(recv_soap);

	b_rt = xhttp_recv_xml(xhttp, xml);

	recv_soap = downcast_xml_to_dom(xml);

	return b_rt;
}

bool_t xhttp_invoke_wsdl(const tchar_t* sz_url, link_t_ptr wsdl)
{
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;
	dword_t n_size = 0;
	link_t_ptr ptr_xml;
	bool_t rt;

	xhttp = xhttp_client(_T("GET"), sz_url);
	if (!xhttp)
		return 0;

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_accept_type(xhttp, HTTP_HEADER_CONTENTTYPE_TEXTXML, -1);

	if (!xhttp_send_request(xhttp))
	{
		xhttp_close(xhttp);
		return 0;
	}

	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &n_size))
	{
		bytes_free(pbuf);

		xhttp_close(xhttp);

		return 0;
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	ptr_xml = upcast_dom_to_xml(wsdl);

	rt = parse_xml_doc_from_bytes(ptr_xml, *pbuf, n_size);

	bytes_free(pbuf);
	pbuf = NULL;

	wsdl = downcast_xml_to_dom(ptr_xml);

	return rt;
}

bool_t get_wsdl_soap_info(link_t_ptr wsdl, const tchar_t* sz_srv, const tchar_t* sz_rpc, tchar_t* sz_loc, tchar_t* sz_act, tchar_t* sz_tns, link_t_ptr req_sch, link_t_ptr res_sch)
{
	link_t_ptr nlk_service, nlk_port, nlk_binding, nlk_porttype, nlk_message, nlk_types;
	link_t_ptr nlk_schema, nlk_element, nlk_child, nlk;
	link_t_ptr nlk_sch;

	tchar_t sz_porttype[RES_LEN + 1] = { 0 };
	tchar_t sz_binding[RES_LEN + 1] = { 0 };
	tchar_t sz_type[RES_LEN + 1] = { 0 };
	tchar_t sz_inmsg[RES_LEN + 1] = { 0 };
	tchar_t sz_outmsg[RES_LEN + 1] = { 0 };
	tchar_t sz_inparam[RES_LEN + 1] = { 0 };
	tchar_t sz_outparam[RES_LEN + 1] = { 0 };
	tchar_t sz_paname[RES_LEN + 1] = { 0 };
	tchar_t sz_patype[RES_LEN + 1] = { 0 };

	bool_t b_break = 0;

	get_dom_node_attr(wsdl, WSDL_ATTR_TARGETNAMESPACE, -1, sz_tns, PATH_LEN);

	nlk_service = get_wsdl_next_service_node(wsdl, LINK_FIRST);
	while (nlk_service)
	{
		if (compare_nons(get_dom_node_attr_ptr(nlk_service, WSDL_ATTR_NAME, -1), -1, sz_srv, -1) == 0)
			break;

		nlk_service = get_wsdl_next_service_node(wsdl, nlk_service);
	}

	if (!nlk_service)
	{
		return 0;
	}

	nlk_port = get_dom_first_child_node(nlk_service);
	while (nlk_port)
	{
		if (compare_nons(get_dom_node_name_ptr(nlk_port), -1, WSDL_NODE_PORT, -1) != 0)
		{
			nlk_port = get_dom_next_sibling_node(nlk_port);
			continue;
		}

		get_dom_node_attr(nlk_port, WSDL_ATTR_BINDING, -1, sz_binding, RES_LEN);
		trim_xmlns(sz_binding, -1);

		b_break = 0;
		nlk_binding = get_wsdl_next_binding_node(wsdl, LINK_FIRST);
		while (nlk_binding)
		{
			if (compare_nons(get_dom_node_attr_ptr(nlk_binding, WSDL_ATTR_NAME, -1), -1, sz_binding, -1) != 0)
			{
				nlk_binding = get_wsdl_next_binding_node(wsdl, nlk_binding);
				continue;
			}

			nlk_child = get_dom_first_child_node(nlk_binding);
			while (nlk_child)
			{
				if (compare_nons(get_dom_node_name_ptr(nlk_child), -1, WSDL_NODE_OPERATION, -1) == 0 && compare_nons(get_dom_node_attr_ptr(nlk_child, WSDL_ATTR_NAME, -1), -1, sz_rpc, -1) == 0)
				{
					nlk = get_dom_first_child_node(nlk_child);
					while (nlk)
					{
						if (compare_nons(get_dom_node_name_ptr(nlk), -1, WSDL_NODE_OPERATION, -1) == 0)
						{
							get_dom_node_attr(nlk, WSDL_ATTR_SOAPACTION, -1, sz_act, PATH_LEN);
						}

						nlk = get_dom_next_sibling_node(nlk);
					}

					b_break = 1;
					break;
				}

				nlk_child = get_dom_next_sibling_node(nlk_child);
			}

			if (b_break)
			{
				get_dom_node_attr(nlk_binding, WSDL_ATTR_TYPE, -1, sz_porttype, RES_LEN);
				trim_xmlns(sz_porttype, -1);

				break;
			}

			nlk_binding = get_wsdl_next_binding_node(wsdl, nlk_binding);
		}

		b_break = 0;
		nlk_porttype = get_wsdl_next_porttype_node(wsdl, LINK_FIRST);
		while (nlk_porttype)
		{
			if (compare_nons(get_dom_node_attr_ptr(nlk_porttype, WSDL_ATTR_NAME, -1), -1, sz_porttype, -1) != 0)
			{
				nlk_porttype = get_wsdl_next_porttype_node(wsdl, nlk_porttype);
				continue;
			}

			nlk_child = get_dom_first_child_node(nlk_porttype);
			while (nlk_child)
			{
				if (compare_nons(get_dom_node_name_ptr(nlk_child), -1, WSDL_NODE_OPERATION, -1) == 0 && compare_nons(get_dom_node_attr_ptr(nlk_child, WSDL_ATTR_NAME, -1), -1, sz_rpc, -1) == 0)
				{
					nlk = get_dom_first_child_node(nlk_child);
					while (nlk)
					{
						if (compare_nons(get_dom_node_name_ptr(nlk), -1, WSDL_NODE_INPUT, -1) == 0)
						{
							get_dom_node_attr(nlk, WSDL_ATTR_MESSAGE, -1, sz_inmsg, RES_LEN);
							trim_xmlns(sz_inmsg, -1);
						}
						else if (compare_nons(get_dom_node_name_ptr(nlk), -1, WSDL_NODE_OUTPUT, -1) == 0)
						{
							get_dom_node_attr(nlk, WSDL_ATTR_MESSAGE, -1, sz_outmsg, RES_LEN);
							trim_xmlns(sz_outmsg, -1);
						}

						nlk = get_dom_next_sibling_node(nlk);
					}

					b_break = 1;
					break;
				}

				nlk_child = get_dom_next_sibling_node(nlk_child);
			}

			if (b_break)
				break;
			nlk_porttype = get_wsdl_next_porttype_node(wsdl, nlk_porttype);
		}

		nlk = get_dom_first_child_node(nlk_port);
		while (nlk)
		{
			if (compare_nons(get_dom_node_name_ptr(nlk), -1, WSDL_NODE_ADDRESS, -1) == 0)
			{
				get_dom_node_attr(nlk, WSDL_ATTR_LOCATION, -1, sz_loc, PATH_LEN);
				break;;
			}

			nlk = get_dom_next_sibling_node(nlk);
		}

		if (!is_null(sz_inmsg) || !is_null(sz_outmsg))
			break;

		nlk_port = get_dom_next_sibling_node(nlk_port);
	}

	nlk_message = get_wsdl_next_message_node(wsdl, LINK_FIRST);
	while (nlk_message)
	{
		if (compare_nons(get_dom_node_attr_ptr(nlk_message, WSDL_ATTR_NAME, -1), -1, sz_inmsg, -1) == 0)
		{
			nlk_child = get_dom_first_child_node(nlk_message);
			while (nlk_child)
			{
				if (compare_nons(get_dom_node_name_ptr(nlk_child), -1, WSDL_NODE_PART, -1) == 0)
				{
					get_dom_node_attr(nlk_child, WSDL_ATTR_ELEMENT, -1, sz_inparam, RES_LEN);
					trim_xmlns(sz_inparam, -1);

					if (!is_null(sz_inparam))
						break;

					get_dom_node_attr(nlk_child, WSDL_ATTR_NAME, -1, sz_paname, RES_LEN);
					trim_xmlns(sz_paname, -1);

					get_dom_node_attr(nlk_child, WSDL_ATTR_TYPE, -1, sz_patype, RES_LEN);
					trim_xmlns(sz_patype, -1);

					nlk_sch = insert_schema_element(req_sch, LINK_LAST);
					set_schema_element_name(nlk_sch, sz_paname);
					set_schema_element_type(nlk_sch, sz_patype);
				}

				nlk_child = get_dom_next_sibling_node(nlk_child);
			}
		}
		else if (compare_nons(get_dom_node_attr_ptr(nlk_message, WSDL_ATTR_NAME, -1), -1, sz_outmsg, -1) == 0)
		{
			nlk_child = get_dom_first_child_node(nlk_message);
			while (nlk_child)
			{
				if (compare_nons(get_dom_node_name_ptr(nlk_child), -1, WSDL_NODE_PART, -1) == 0)
				{
					get_dom_node_attr(nlk_child, WSDL_ATTR_ELEMENT, -1, sz_outparam, RES_LEN);
					trim_xmlns(sz_outparam, -1);

					if (!is_null(sz_outparam))
						break;

					get_dom_node_attr(nlk_child, WSDL_ATTR_NAME, -1, sz_paname, RES_LEN);
					trim_xmlns(sz_paname, -1);

					get_dom_node_attr(nlk_child, WSDL_ATTR_TYPE, -1, sz_patype, RES_LEN);
					trim_xmlns(sz_patype, -1);

					nlk_sch = insert_schema_element(res_sch, LINK_LAST);
					set_schema_element_name(nlk_sch, sz_paname);
					set_schema_element_type(nlk_sch, sz_patype);
				}

				nlk_child = get_dom_next_sibling_node(nlk_child);
			}
		}

		if (!is_null(sz_inparam) && !is_null(sz_outparam))
			break;

		nlk_message = get_wsdl_next_message_node(wsdl, nlk_message);
	}

	if (is_null(sz_inparam) && is_null(sz_outparam))
		return 1;

	nlk_types = get_wsdl_types_node(wsdl);
	if (!nlk_types)
		return 1;

	nlk_schema = get_dom_first_child_node(nlk_types);
	while (nlk_schema)
	{
		if (!is_null(sz_inparam) || !is_null(sz_outparam))
		{
			nlk_element = find_dom_node_by_name(nlk_schema, 1, WSDL_ATTR_ELEMENT, -1);
			while (nlk_element)
			{
				if (compare_nons(get_dom_node_attr_ptr(nlk_element, WSDL_ATTR_NAME, -1), -1, sz_inparam, -1) == 0)
				{
					nlk_child = find_dom_node_by_name(nlk_element, 1, WSDL_ATTR_ELEMENT, -1);
					while (nlk_child)
					{
						if (compare_nons(get_dom_node_name_ptr(nlk_child), -1, WSDL_ATTR_ELEMENT, -1) == 0)
						{
							get_dom_node_attr(nlk_child, WSDL_ATTR_NAME, -1, sz_paname, RES_LEN);
							trim_xmlns(sz_paname, -1);

							get_dom_node_attr(nlk_child, WSDL_ATTR_TYPE, -1, sz_patype, RES_LEN);
							trim_xmlns(sz_patype, -1);

							nlk_sch = insert_schema_element(req_sch, LINK_LAST);
							set_schema_element_name(nlk_sch, sz_paname);
							set_schema_element_type(nlk_sch, sz_patype);
						}
						nlk_child = get_dom_next_sibling_node(nlk_child);
					}

					xscpy(sz_inparam, _T(""));
				}
				else if (compare_nons(get_dom_node_attr_ptr(nlk_element, WSDL_ATTR_NAME, -1), -1, sz_outparam, -1) == 0)
				{
					nlk_child = find_dom_node_by_name(nlk_element, 1, WSDL_ATTR_ELEMENT, -1);
					while (nlk_child)
					{
						if (compare_nons(get_dom_node_name_ptr(nlk_child), -1, WSDL_ATTR_ELEMENT, -1) == 0)
						{
							get_dom_node_attr(nlk_child, WSDL_ATTR_NAME, -1, sz_paname, RES_LEN);
							trim_xmlns(sz_paname, -1);

							get_dom_node_attr(nlk_child, WSDL_ATTR_TYPE, -1, sz_patype, RES_LEN);
							trim_xmlns(sz_patype, -1);

							nlk_sch = insert_schema_element(res_sch, LINK_LAST);
							set_schema_element_name(nlk_sch, sz_paname);
							set_schema_element_type(nlk_sch, sz_patype);
						}
						nlk_child = get_dom_next_sibling_node(nlk_child);
					}

					xscpy(sz_inparam, _T(""));
				}

				nlk_element = get_dom_next_sibling_node(nlk_element);
			}
		}
		nlk_schema = get_dom_next_sibling_node(nlk_schema);
	}

	return 1;
}

void set_wsdl_soap_info(link_t_ptr wsdl, const tchar_t* sz_srv, const tchar_t* sz_rpc, const tchar_t* sz_loc, const tchar_t* sz_act, const tchar_t* sz_tns, link_t_ptr req_sch, link_t_ptr res_sch)
{
	link_t_ptr nlk_sch, nlk_types, nlk_message, nlk_porttype, nlk_binding, nlk_service;
	link_t_ptr nlk_operation, nlk_port;

	tchar_t sz_rpcres[RES_LEN + 1] = { 0 };
	tchar_t sz_porttype[RES_LEN + 1] = { 0 };
	tchar_t sz_inmsg[RES_LEN + 1] = { 0 };
	tchar_t sz_outmsg[RES_LEN + 1] = { 0 };
	tchar_t sz_binding[RES_LEN + 1] = { 0 };
	tchar_t sz_portname[RES_LEN + 1] = { 0 };

	if (!is_null(sz_tns))
	{
		set_wsdl_default_xmlns(wsdl);

		set_dom_node_xmlns(wsdl, XMLNS_TNS, -1, sz_tns, -1);
		set_dom_node_attr(wsdl, WSDL_ATTR_TARGETNAMESPACE, -1, sz_tns, -1);
	}

	xsprintf(sz_rpcres, _T("%sResponse"), sz_rpc);

	nlk_types = get_wsdl_types_node(wsdl);
	if (!nlk_types)
	{
		nlk_types = add_wsdl_types_node(wsdl);
	}

	nlk_sch = add_wsdl_types_schema(nlk_types, sz_tns);

	add_wsdl_schema_element(nlk_sch, sz_rpc, req_sch);
	add_wsdl_schema_element(nlk_sch, sz_rpcres, res_sch);

	xsprintf(sz_inmsg, _T("%sSoapIn"), sz_rpc);
	xsprintf(sz_outmsg, _T("%sSoapOut"), sz_rpc);

	nlk_message = add_wsdl_message_node(wsdl, sz_inmsg);
	add_wsdl_message_part_element(nlk_message, WSDL_ATTR_ELEMENT_PARAMETERS, sz_rpc);

	nlk_message = add_wsdl_message_node(wsdl, sz_outmsg);
	add_wsdl_message_part_element(nlk_message, WSDL_ATTR_ELEMENT_PARAMETERS, sz_rpcres);

	xsprintf(sz_porttype, _T("%sSoapPortType"), sz_srv);
	nlk_porttype = add_wsdl_porttype_node(wsdl, sz_porttype);

	nlk_operation = add_wsdl_porttype_operation(nlk_porttype, sz_rpc, sz_inmsg, sz_outmsg);

	xsprintf(sz_binding, _T("%sSoapBinding"), sz_srv);
	nlk_binding = add_wsdl_binding_node(wsdl, sz_binding, sz_porttype);

	nlk_operation = add_wsdl_binding_operation(nlk_binding, sz_rpc, sz_act);

	nlk_service = get_wsdl_service_node(wsdl);
	if (!nlk_service)
	{
		nlk_service = add_wsdl_service_node(wsdl, sz_srv);
	}

	xsprintf(sz_portname, _T("%sPort"), sz_srv);
	nlk_port = add_wsdl_service_port(nlk_service, sz_portname, sz_binding, sz_loc);
}

#endif //XDLDOC