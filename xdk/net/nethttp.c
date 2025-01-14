﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdn http document

	@module	nethttp.c | implement file

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

#include "nethttp.h"

#include "../xdknet.h"
#include "../xdkimp.h"
#include "../xdkoem.h"
#include "../xdkstd.h"
#include "../xdkutil.h"

#if defined(XDK_SUPPORT_SOCK)

typedef struct _http_context{
	handle_head head;		//head for xhand_t
	int type;

	int secu;
	
	bio_interface* pif;

	stream_t send_stream;
	stream_t recv_stream;

	link_t_ptr st_request,st_response;
	bool_t b_request,b_response;

	unsigned short port;
	tchar_t addr[ADDR_LEN + 1];

	tchar_t method[RES_LEN + 1];
	tchar_t poto[RES_LEN + 1];
	tchar_t version[NUM_LEN + 1];
	tchar_t host[RES_LEN + 1];
	tchar_t object[PATH_LEN + 1];

	byte_t *query;
	dword_t len_query;

	tchar_t code[NUM_LEN + 1];
	tchar_t text[ERR_LEN + 1];
}http_context;

#define IS_XHTTP_METHOD(token)		(a_xsnicmp(token,"GET",3) == 0 ||  a_xsnicmp(token,"PUT",3) == 0 || a_xsnicmp(token,"POST",4) == 0 || a_xsnicmp(token,"DELETE",6) == 0 || a_xsnicmp(token,"HEAD",4) == 0 || a_xsnicmp(token,"LIST",4) == 0 || a_xsnicmp(token,"GRANT",5) == 0) 

/***********************************************************************************************/
static void _xhttp_parse_request(http_context* phttp, byte_t* data, int len)
{
	byte_t *token = data;
	byte_t *key, *val;
	int klen, vlen;
	tchar_t *tkey, *tval;
	int tlen;

	//get method
	klen = 0;
	key = token;
	while (*token != ' ' && *token != '\0')
	{
		token++;
		klen++;
	}
#ifdef _UNICODE
	utf8_to_ucs(key, klen, phttp->method, RES_LEN);
#else
	utf8_to_mbs(key, klen, phttp->method, RES_LEN);
#endif

	if (data + len < token)
		return;

	//skip black
	while (*token == ' ')
		token++;

	//get object
	klen = 0;
	key = token;
	while (*token != ' ' && *token != '?' && *token != '\0')
	{
		token++;
		klen++;
	}
#ifdef _UNICODE
	utf8_to_ucs(key, klen, phttp->object, RES_LEN);
#else
	utf8_to_mbs(key, klen, phttp->object, RES_LEN);
#endif

	if (data + len < token)
		return;

	//get query
	if (*token == _T('?'))
	{
		//skip '?'
		token++;

		key = token;
		klen = 0;
		while (*token != ' ' && *token != '\0')
		{
			token++;
			klen++;
		}

		phttp->len_query = klen;
		phttp->query = (byte_t*)xmem_realloc(phttp->query, phttp->len_query + 1);
		xmem_copy((void*)phttp->query, (void*)key, phttp->len_query);
	}

	//skip black
	while (*token == ' ')
		token++;

	//get proto
	klen = 0;
	key = token;
	while (*token != '/' && *token != '\0')
	{
		token++;
		klen++;
	}
#ifdef _UNICODE
	utf8_to_ucs(key, klen, phttp->poto, RES_LEN);
#else
	utf8_to_mbs(key, klen, phttp->poto, RES_LEN);
#endif

	if (data + len < token)
		return;

	if (*token != '\0')
		token++;

	//get version
	klen = 0;
	key = token;
	while (*token != '\r' && *token != '\0')
	{
		token++;
		klen++;
	}
#ifdef _UNICODE
	utf8_to_ucs(key, klen, phttp->version, NUM_LEN);
#else
	utf8_to_mbs(key, klen, phttp->version, NUM_LEN);
#endif

	if (data + len < token)
		return;

	//skip '\r\n'
	if (*token == '\r')
		token++;
	if (*token == '\n')
		token++;

	//parse key and val
	while (1)
	{
		if (*token == '\r' || *token == '\n' || *token == '\0')
			break;

		key = token;
		klen = 0;
		while (*token != ':' && *token != '\0')
		{
			token++;
			klen++;
		}
		//skip ':'
		if (*token != '\0')
			token++;

		//skip black
		while (*token == ' ')
			token++;

		val = token;
		vlen = 0;
		while (*token != '\r' && *token != '\n' && *token != '\0')
		{
			token++;
			vlen++;
		}
#ifdef _UNICODE
		tlen = utf8_to_ucs(key, klen, NULL, MAX_LONG);
		tkey = xsalloc(tlen + 1);
		utf8_to_ucs(key, klen, tkey, tlen);

		tlen = utf8_to_ucs(val, vlen, NULL, MAX_LONG);
		tval = xsalloc(tlen + 1);
		utf8_to_ucs(val, vlen, tval, tlen);

		write_string_entity(phttp->st_request, tkey, -1, tval, -1);

		xsfree(tkey);
		xsfree(tval);
#else
		tlen = utf8_to_mbs(key, klen, NULL, MAX_LONG);
		tkey = xsalloc(tlen + 1);
		utf8_to_mbs(key, klen, tkey, tlen);

		tlen = utf8_to_mbs(val, vlen, NULL, MAX_LONG);
		tval = xsalloc(tlen + 1);
		utf8_to_mbs(val, vlen, tval, tlen);

		write_string_entity(phttp->st_request, tkey, -1, tval, -1);

		xsfree(tkey);
		xsfree(tval);
#endif
		//skip '\r\n'
		if (*token == '\r')
			token++;
		if (*token == '\n')
			token++;

		if (data + len < token)
			return;
	}
}

static dword_t _xhttp_format_request(http_context* phttp, byte_t* buf, dword_t max)
{
	dword_t total = 0;
	link_t_ptr ent;
	const tchar_t *key, *val;

	//method
#ifdef _UNICODE
	total += ucs_to_utf8(phttp->method, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->method, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = ' ';
	}
	total++;
	if (total >= max)
		return total;

	//object
#ifdef _UNICODE
	total += ucs_to_utf8(phttp->object, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->object, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	//query
	if (phttp->len_query)
	{
		if (buf)
		{
			buf[total] = '?';
		}
		total++;
		if (total >= max)
			return total;

		if (buf)
		{
			xmem_copy((void*)(buf + total), (void*)phttp->query, phttp->len_query);
		}
		total += phttp->len_query;
		if (total >= max)
			return total;
	}

	if (buf)
	{
		buf[total] = ' ';
	}
	total++;
	if (total >= max)
		return total;

	//proto
#ifdef _UNICODE
	total += ucs_to_utf8(phttp->poto, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->poto, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = '/';
	}
	total++;
	if (total >= max)
		return total;

	//version
#ifdef _UNICODE
	total += ucs_to_utf8(phttp->version, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->version, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = '\r';
		buf[total + 1] = '\n';
	}
	total += 2;
	if (total >= max)
		return total;

	//entity
	ent = get_string_next_entity(phttp->st_request, LINK_FIRST);
	while (ent)
	{
		key = get_string_entity_key_ptr(ent);
		val = get_string_entity_val_ptr(ent);

#ifdef _UNICODE
		total += ucs_to_utf8(key, -1, ((buf) ? buf + total : NULL), max - total);
#else
		total += mbs_to_utf8(key, -1, ((buf) ? buf + total : NULL), max - total);
#endif
		if (total >= max)
			return total;

		if (buf)
		{
			buf[total] = ':';
			buf[total + 1] = ' ';
		}
		total += 2;
		if (total >= max)
			return total;

#ifdef _UNICODE
		total += ucs_to_utf8(val, -1, ((buf) ? buf + total : NULL), max - total);
#else
		total += mbs_to_utf8(val, -1, ((buf) ? buf + total : NULL), max - total);
#endif
		if (total >= max)
			return total;

		if (buf)
		{
			buf[total] = '\r';
			buf[total + 1] = '\n';
		}
		total += 2;
		if (total >= max)
			return total;

		ent = get_string_next_entity(phttp->st_request, ent);
	}

	if (buf)
	{
		buf[total] = '\r';
		buf[total + 1] = '\n';
	}
	total += 2;

	return total;
}

static void _xhttp_parse_response(http_context* phttp, byte_t* data, dword_t len)
{
	byte_t *token = data;
	byte_t *key, *val;
	int klen, vlen;
	tchar_t *tkey, *tval;
	int tlen;

	//get proto
	klen = 0;
	key = token;
	while (*token != '/' && *token != '\0')
	{
		token++;
		klen++;
	}
#ifdef _UNICODE
	utf8_to_ucs(key, klen, phttp->poto, RES_LEN);
#else
	utf8_to_mbs(key, klen, phttp->poto, RES_LEN);
#endif

	if (data + len < token)
		return;

	//skip '/'
	if (*token != '\0')
		token++;

	//get version
	klen = 0;
	key = token;
	while (*token != ' ' && *token != '\0')
	{
		token++;
		klen++;
	}
#ifdef _UNICODE
	utf8_to_ucs(key, klen, phttp->version, NUM_LEN);
#else
	utf8_to_mbs(key, klen, phttp->version, NUM_LEN);
#endif

	if (data + len < token)
		return;

	//skip black
	while (*token == ' ')
		token++;

	//get code
	klen = 0;
	key = token;
	while (*token != ' ' && *token != '\0')
	{
		token++;
		klen++;
	}
#ifdef _UNICODE
	utf8_to_ucs(key, klen, phttp->code, NUM_LEN);
#else
	utf8_to_mbs(key, klen, phttp->code, NUM_LEN);
#endif

	if (data + len < token)
		return;

	//skip black
	while (*token == ' ')
		token++;

	//get text
	if (*token != '\r' && *token != '\0')
	{
		vlen = 0;
		val = token;
		while (*token != '\r' && *token != '\0')
		{
			token++;
			vlen++;
		}
#ifdef _UNICODE
		utf8_to_ucs(val, vlen, phttp->text, ERR_LEN);
#else
		utf8_to_mbs(val, vlen, phttp->text, ERR_LEN);
#endif
	}

	//skip '\r\n'
	if (*token == '\r')
		token++;
	if (*token == '\n')
		token++;

	if (data + len < token)
		return;

	//parse key and val
	while (1)
	{
		if (*token == '\r' || *token == '\n' || *token == '\0')
			break;

		key = token;
		klen = 0;
		while (*token != ':' && *token != '\0')
		{
			token++;
			klen++;
		}
		//skip ':'
		if (*token != '\0')
			token++;

		//skip black
		while (*token == ' ')
			token++;

		val = token;
		vlen = 0;
		while (*token != '\r' && *token != '\n' && *token != '\0')
		{
			token++;
			vlen++;
		}

#ifdef _UNICODE
		tlen = utf8_to_ucs(key, klen, NULL, MAX_LONG);
		tkey = xsalloc(tlen + 1);
		utf8_to_ucs(key, klen, tkey, tlen);

		tlen = utf8_to_ucs(val, vlen, NULL, MAX_LONG);
		tval = xsalloc(tlen + 1);
		utf8_to_ucs(val, vlen, tval, tlen);

		write_string_entity(phttp->st_response, tkey, -1, tval, -1);

		xsfree(tkey);
		xsfree(tval);
#else
		tlen = utf8_to_mbs(key, klen, NULL, MAX_LONG);
		tkey = xsalloc(tlen + 1);
		utf8_to_mbs(key, klen, tkey, tlen);

		tlen = utf8_to_mbs(val, vlen, NULL, MAX_LONG);
		tval = xsalloc(tlen + 1);
		utf8_to_mbs(val, vlen, tval, tlen);

		write_string_entity(phttp->st_response, tkey, -1, tval, -1);

		xsfree(tkey);
		xsfree(tval);
#endif
		//skip '\r\n'
		if (*token == '\r')
			token++;
		if (*token == '\n')
			token++;

		if (data + len < token)
			return;
	}
}

static dword_t _xhttp_format_response(http_context* phttp, byte_t* buf, dword_t max)
{
	dword_t total = 0;
	link_t_ptr ent;
	const tchar_t *key, *val;

#ifdef _UNICODE
	total += ucs_to_utf8(phttp->poto, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->poto, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = '/';
	}
	total++;
	if (total >= max)
		return total;

#ifdef _UNICODE
	total += ucs_to_utf8(phttp->version, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->version, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = ' ';
	}
	total++;
	if (total >= max)
		return total;

#ifdef _UNICODE
	total += ucs_to_utf8(phttp->code, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->code, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = ' ';
	}
	total++;
	if (total >= max)
		return total;

#ifdef _UNICODE
	total += ucs_to_utf8(phttp->text, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->text, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = '\r';
		buf[total + 1] = '\n';
	}
	total += 2;
	if (total >= max)
		return total;

	ent = get_string_next_entity(phttp->st_response, LINK_FIRST);
	while (ent)
	{
		key = get_string_entity_key_ptr(ent);
		val = get_string_entity_val_ptr(ent);

#ifdef _UNICODE
		total += ucs_to_utf8(key, -1, ((buf) ? buf + total : NULL), max - total);
#else
		total += mbs_to_utf8(key, -1, ((buf) ? buf + total : NULL), max - total);
#endif
		if (total >= max)
			return total;

		if (buf)
		{
			buf[total] = ':';
			buf[total + 1] = ' ';
		}
		total += 2;
		if (total >= max)
			return total;

#ifdef _UNICODE
		total += ucs_to_utf8(val, -1, ((buf) ? buf + total : NULL), max - total);
#else
		total += mbs_to_utf8(val, -1, ((buf) ? buf + total : NULL), max - total);
#endif
		if (total >= max)
			return total;

		if (buf)
		{
			buf[total] = '\r';
			buf[total + 1] = '\n';
		}
		total += 2;
		if (total >= max)
			return total;

		ent = get_string_next_entity(phttp->st_response, ent);
	}

	if (buf)
	{
		buf[total] = '\r';
		buf[total + 1] = '\n';
	}
	total += 2;

	return total;
}

static int _xhttp_format_continue(http_context* phttp, byte_t* buf, dword_t max)
{
	dword_t total = 0;

#ifdef _UNICODE
	total += ucs_to_utf8(phttp->poto, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->poto, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = '/';
	}
	total++;
	if (total >= max)
		return total;

#ifdef _UNICODE
	total += ucs_to_utf8(phttp->version, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(phttp->version, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = ' ';
	}
	total++;
	if (total >= max)
		return total;

#ifdef _UNICODE
	total += ucs_to_utf8(HTTP_CODE_100, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(HTTP_CODE_100, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = ' ';
	}
	total++;
	if (total >= max)
		return total;

#ifdef _UNICODE
	total += ucs_to_utf8(HTTP_CODE_100_TEXT, -1, ((buf) ? buf + total : NULL), max - total);
#else
	total += mbs_to_utf8(HTTP_CODE_100_TEXT, -1, ((buf) ? buf + total : NULL), max - total);
#endif
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = '\r';
		buf[total + 1] = '\n';
	}
	total += 2;
	if (total >= max)
		return total;

	if (buf)
	{
		buf[total] = '\r';
		buf[total + 1] = '\n';
	}
	total += 2;

	return total;
}

/*****************************************************************************************************/

void xhttp_split_object(const tchar_t* sz_object, tchar_t* sz_site, tchar_t* sz_file)
{
	int fat = 0;
	int lat = 0;
	int len;
	tchar_t* token;
	
	token = (tchar_t*)sz_object;
	len = 0;

	if (*token == _T('/') || *token == _T('\\'))
	{
		token++;
		len++;
	}

	while (*token != _T('/') && *token != _T('\\') && *token != _T('\0'))
	{
		token++;
		len++;
	}

	if (*token == _T('\0'))
	{
		if (sz_site)
		{
			xscpy(sz_site, _T("/"));
		}

		if (sz_file)
		{
			xsncpy(sz_file, sz_object, PATH_LEN);
			if (sz_file[0] == _T('\\'))
				sz_file[0] = _T('/');
		}
		return;
	}
	
	if (sz_site)
	{
		len = (len < RES_LEN) ? len : RES_LEN;
		xsncpy(sz_site, sz_object, len);
		if (sz_site[0] == _T('\\'))
			sz_site[0] = _T('/');
	}

	if (sz_file)
	{
		xsncpy(sz_file, token, PATH_LEN);
		token = sz_file;
		while (*token != _T('\0'))
		{
			if (*token == _T('\\'))
				*token = _T('/');
			token++;
		}
	}
}

int xhttp_format_content_type(const tchar_t* filter, tchar_t* buf, int max)
{
	int len;

	if (compare_text(filter, -1, _T("html"), -1, 1) == 0)
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_TEXTHTML);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_TEXTHTML, len);
		}
		return len;
	}
	else if (compare_text(filter, -1, _T("css"), -1, 1) == 0)
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_TEXTCSS);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_TEXTCSS, len);
		}
		return len;
	}
	else if (compare_text(filter, -1, _T("js"), -1, 1) == 0)
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_JAVASCRIPT);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_JAVASCRIPT, len);
		}
		return len;
	}
	else if (compare_text(filter, -1, _T("jpg"), -1, 1) == 0)
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_IMAGEJPG);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_IMAGEJPG, len);
		}
		return len;
	}
	else if (compare_text(filter, -1, _T("jpeg"), -1, 1) == 0)
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_IMAGEJPG);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_IMAGEJPG, len);
		}
		return len;
	}
	else if (compare_text(filter, -1, _T("png"), -1, 1) == 0)
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_IMAGEPNG);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_IMAGEPNG, len);
		}
		return len;
	}
	else if (compare_text(filter, -1, _T("gif"), -1, 1) == 0)
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_IMAGEGIF);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_IMAGEGIF, len);
		}
		return len;
	}
	else if (compare_text(filter, -1, _T("tiff"), -1, 1) == 0)
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_IMAGETIFF);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_IMAGETIFF, len);
		}
		return len;
	}
	else
	{
		len = xslen(HTTP_HEADER_CONTENTTYPE_TEXTPLAIN);
		len = (len < max) ? len : max;
		if (buf)
		{
			xsncpy(buf, HTTP_HEADER_CONTENTTYPE_TEXTPLAIN, len);
		}
		return len;
	}
}

xhand_t xhttp_client(const tchar_t* method,const tchar_t* url)
{
	tchar_t *potoat,*hostat,*portat,*objat,*qryat;
	int potolen,hostlen,portlen,objlen,qrylen;

	http_context* phttp = NULL;

	TRY_CATCH;

	if (is_null(url))
	{
		raise_user_error(_T("xhttp_client"), _T("invalid url"));
	}

	phttp = (http_context*)xmem_alloc(sizeof(http_context));
	phttp->head.tag = _HANDLE_INET;

	phttp->type = _XHTTP_TYPE_CLI;
	xsncpy(phttp->method, method, RES_LEN);
	xsupr(phttp->method);

	parse_url(url,&potoat,&potolen,&hostat,&hostlen,&portat,&portlen,&objat,&objlen,&qryat,&qrylen);

	if (potolen)
	{
		xsncpy(phttp->poto, potoat, potolen);
		xsupr(phttp->poto);

		if (compare_text(phttp->poto, -1, _T("HTTPS"), -1, 1) == 0)
		{
			phttp->secu = _SECU_SSL;
		}
		else if (compare_text(phttp->poto, -1, _T("SSH"), -1, 1) == 0)
		{
			phttp->secu = _SECU_SSH;
		}
	}

	xscpy(phttp->poto, _T("HTTP"));
	xscpy(phttp->version, _T("1.1"));

	if(hostlen)
	{
		xsncpy(phttp->host, hostat, hostlen);

		if(is_ip(phttp->host))
		{
			xscpy(phttp->addr,phttp->host);
		}else
		{
			host_addr(phttp->host,phttp->addr);
		}
	}

	if(is_null(phttp->addr))
	{
		raise_user_error(_T("xhttp_client"), _T("invalid host address"));
	}

	if (portlen)
	{
		phttp->port = xsntos(portat, portlen);
	}
	else
	{
		switch (phttp->secu)
		{
		case _SECU_SSL:
			phttp->port = DEF_HTTPS_PORT;
			break;
		case _SECU_SSH:
			phttp->port = DEF_SSH_PORT;
			break;
		default:
			phttp->port = DEF_HTTP_PORT;
			break;
		}
	}

	if (objlen)
		xsncpy(phttp->object, objat, objlen);
	else
		xscpy(phttp->object,_T("/"));

	if(qrylen)
	{
		phttp->len_query = xhttp_url_encoding(qryat, qrylen, NULL, MAX_LONG);
		phttp->query = (byte_t*)xmem_alloc(phttp->len_query + 1);
		xhttp_url_encoding(qryat, qrylen, phttp->query, phttp->len_query);
	}

	phttp->pif = (bio_interface*)xmem_alloc(sizeof(bio_interface));

	switch (phttp->secu)
	{
	case _SECU_SSL:
		phttp->pif->fd = xssl_cli(phttp->port, phttp->addr);
		break;
	case _SECU_SSH:
		phttp->pif->fd = xssh_cli(phttp->port, phttp->addr);
		break;
	default:
		phttp->pif->fd = xtcp_cli(phttp->port, phttp->addr);
		break;
	}
	
	if(!phttp->pif->fd)
	{
		raise_user_error(_T("xhttp_client"), _T("create bio failed"));
	}

	get_bio_interface(phttp->pif->fd, phttp->pif);

	phttp->st_request = create_string_table(ORDER_ASCEND);
	phttp->st_response = create_string_table(ORDER_ASCEND);

	END_CATCH;

	return &phttp->head;
ONERROR:
	XDK_TRACE_LAST;


	if (phttp)
	{
		xhttp_close(&phttp->head);
	}

	return NULL;
}

xhand_t xhttp_server(xhand_t bio)
{
	unsigned short port;
	
	http_context* phttp = NULL;

	TRY_CATCH;

	if (!bio)
	{
		raise_user_error(_T("xhttp_server"), _T("invalid bio"));
	}

	phttp = (http_context*)xmem_alloc(sizeof(http_context));
	phttp->head.tag = _HANDLE_INET;
	phttp->type = _XHTTP_TYPE_SRV;

	phttp->pif = (bio_interface*)xmem_alloc(sizeof(bio_interface));

	get_bio_interface(bio, phttp->pif);

	switch (bio->tag)
	{
	case _HANDLE_SSL:
		phttp->secu = _SECU_SSL;
		break;
	case _HANDLE_SSH:
		phttp->secu = _SECU_SSH;
		break;
	default:
		phttp->secu = _SECU_NONE;
		break;
	}

	port = (*(phttp->pif->pf_peer))(phttp->pif->fd, phttp->addr);

	xscpy(phttp->poto, _T("HTTP"));
	xscpy(phttp->version, _T("1.1"));

	phttp->st_request = create_string_table(ORDER_ASCEND);
	phttp->st_response = create_string_table(ORDER_ASCEND);

	END_CATCH;

	return &phttp->head;
ONERROR:
	XDK_TRACE_LAST;

	if (phttp)
	{
		xhttp_close(&phttp->head);
	}

	return NULL;
}

void xhttp_close(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->type == _XHTTP_TYPE_CLI || phttp->type == _XHTTP_TYPE_SRV);

	if (phttp->st_request)
		destroy_string_table(phttp->st_request);

	if (phttp->st_response)
		destroy_string_table(phttp->st_response);

	if (phttp->query)
		xmem_free(phttp->query);

	if (phttp->send_stream)
	{
		stream_free(phttp->send_stream);
	}

	if (phttp->recv_stream)
	{
		stream_free(phttp->recv_stream);
	}

	if (phttp->pif)
	{
		if (phttp->pif->fd && phttp->type == _XHTTP_TYPE_CLI)
		{
			if (phttp->pif->pf_close)
			{
				(*phttp->pif->pf_close)(phttp->pif->fd);
				phttp->pif->fd = NULL;
			}
		}

		xmem_free(phttp->pif);
	}

	xmem_free(phttp);
}


int xhttp_type(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return phttp->type;
}

int xhttp_secu(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return phttp->secu;
}

xhand_t xhttp_bio(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return (phttp->pif)? phttp->pif->fd : NULL;
}

unsigned short xhttp_addr_port(xhand_t xhttp, tchar_t* addr)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->pif)
	{
		return (*(phttp->pif->pf_addr))(phttp->pif->fd, addr);
	}

	return 0;
}

unsigned short xhttp_peer_port(xhand_t xhttp, tchar_t* addr)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->pif)
	{
		return (*(phttp->pif->pf_peer))(phttp->pif->fd, addr);
	}

	return 0;
}

dword_t xhttp_format_request(xhand_t xhttp, byte_t* buf, dword_t max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return _xhttp_format_request(phttp, buf, max);
}

stream_t xhttp_get_send_stream(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return phttp->send_stream;
}

stream_t xhttp_get_recv_stream(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return phttp->recv_stream;
}

int	xhttp_get_url_method(xhand_t xhttp, tchar_t* buf, int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	int len;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	len = xslen(phttp->method);
	len = (len < max) ? len : max;
	xsncpy(buf, phttp->method, len);
	return len;
}

int	xhttp_get_url_host(xhand_t xhttp,tchar_t* buf,int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	int len;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	len = xslen(phttp->host);
	len = (len < max) ? len : max;
	xsncpy(buf, phttp->host, len);
	return len;
}

int	xhttp_get_url_port(xhand_t xhttp,tchar_t* buf,int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return stoxs(phttp->port, buf, max);
}

int	xhttp_get_url_object(xhand_t xhttp,tchar_t* buf,int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	int len;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	len = xslen(phttp->object);
	len = (len < max) ? len : max;
	xsncpy(buf, phttp->object, len);
	return len;
}

dword_t xhttp_get_encoded_query(xhand_t xhttp, byte_t* buf, dword_t max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	max = (max < phttp->len_query) ? max : phttp->len_query;
	if (buf)
	{
		xmem_copy((void*)buf, (void*)phttp->query, max);
	}

	return max;
}

void xhttp_set_encoded_query(xhand_t xhttp, const byte_t* query, dword_t len)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (!query || !len)
	{
		xmem_free(phttp->query);
		phttp->query = NULL;
		phttp->len_query = 0;
		return;
	}

	phttp->len_query = len;
	phttp->query = (byte_t*)xmem_realloc(phttp->query, phttp->len_query + 1);
	xmem_copy((void*)phttp->query, (void*)query, phttp->len_query);
}

dword_t xhttp_url_encoding(const tchar_t* url, int len, byte_t* buf, dword_t max)
{
	dword_t total = 0;
	tchar_t *kstr, *vstr;
	int klen, vlen;
	int n, m = 0;

	byte_t* utf;
	dword_t ulen;

	byte_t* esc;
	dword_t elen;

	if (len < 0)
		len = xslen(url);

	while (n = parse_options_token((url + m), (len - m), _T('='), _T('&'), &kstr, &klen, &vstr, &vlen))
	{
		m += n;

		//encoding the key
#ifdef _UNICODE
		ulen = ucs_to_utf8(kstr, klen, NULL, MAX_LONG);
		utf = (byte_t*)xmem_alloc(ulen);
		ucs_to_utf8(kstr, klen, utf, ulen);
#else
		ulen = mbs_to_utf8(kstr, klen, NULL, MAX_LONG);
		utf = (byte_t*)xmem_alloc(ulen);
		mbs_to_utf8(kstr, klen, utf, ulen);
#endif

		elen = url_byte_encode(utf, ulen, NULL, MAX_LONG);
		esc = (byte_t*)xmem_alloc(elen + 1);
		url_byte_encode(utf, ulen, esc, elen);

		xmem_free(utf);

		if (total + elen > max)
		{
			xmem_free(esc);
			return total;
		}
		if (buf)
		{
			xmem_copy((void*)(buf + total), (void*)esc, elen);
		}
		total += elen;

		xmem_free(esc);

		if (total + 1 > max)
			return total;

		if (buf)
		{
			buf[total] = ('=');
		}
		total++;

		//encoding the value
#ifdef _UNICODE
		ulen = ucs_to_utf8(vstr, vlen, NULL, MAX_LONG);
		utf = (byte_t*)xmem_alloc(ulen);
		ucs_to_utf8(vstr, vlen, utf, ulen);
#else
		ulen = mbs_to_utf8(vstr, vlen, NULL, MAX_LONG);
		utf = (byte_t*)xmem_alloc(ulen);
		mbs_to_utf8(vstr, vlen, utf, ulen);
#endif

		elen = url_byte_encode(utf, ulen, NULL, MAX_LONG);
		esc = (byte_t*)xmem_alloc(elen + 1);
		url_byte_encode(utf, ulen, esc, elen);

		xmem_free(utf);

		if (total + elen > max)
		{
			xmem_free(esc);
			return total;
		}
		if (buf)
		{
			xmem_copy((void*)(buf + total), (void*)esc, elen);
		}
		total += elen;

		xmem_free(esc);

		if (total + 1 > max)
			return total;

		if (buf)
		{
			buf[total] = ('&');
		}
		total++;
	}

	//remove last '&'
	if (total)
	{
		total--;

		if (buf)
		{
			buf[total] = ('\0');
		}
	}

	return total;
}

int xhttp_url_decoding(const byte_t* url, dword_t len, tchar_t* buf, int max)
{
	int total = 0;
	schar_t *kstr, *vstr;
	int klen, vlen;
	int n, m = 0;

	tchar_t* ucs;
	int ulen;

	byte_t* esc;
	dword_t elen;

	while (n = a_parse_options_token((url + m), (len - m), ('='), ('&'), &kstr, &klen, &vstr, &vlen))
	{
		m += n;

		//decoding the key
		elen = url_byte_decode((byte_t*)kstr, klen, NULL, MAX_LONG);
		esc = (byte_t*)xmem_alloc(elen + 1);
		url_byte_decode(kstr, klen, esc, elen);

#ifdef _UNICODE
		ulen = utf8_to_ucs(esc, elen, NULL, MAX_LONG);
		ucs = xsalloc(ulen + 1);
		utf8_to_ucs(esc, elen, ucs, ulen);
#else
		ulen = utf8_to_mbs(esc, elen, NULL, MAX_LONG);
		ucs = xsalloc(ulen + 1);
		utf8_to_mbs(esc, elen, ucs, ulen);
#endif

		xmem_free(esc);

		if (total + ulen > max)
		{
			xsfree(ucs);
			return total;
		}
		if (buf)
		{
			xsncpy(buf + total, ucs, ulen);
		}
		total += ulen;

		xsfree(ucs);

		if (total + 1 > max)
			return total;

		if (buf)
		{
			buf[total] = ('=');
		}
		total++;

		//decoding the value
		elen = url_byte_decode((byte_t*)vstr, vlen, NULL, MAX_LONG);
		esc = (byte_t*)xmem_alloc(elen + 1);
		url_byte_decode(vstr, vlen, esc, elen);

#ifdef _UNICODE
		ulen = utf8_to_ucs(esc, elen, NULL, MAX_LONG);
		ucs = xsalloc(ulen + 1);
		utf8_to_ucs(esc, elen, ucs, ulen);
#else
		ulen = utf8_to_mbs(esc, elen, NULL, MAX_LONG);
		ucs = xsalloc(ulen + 1);
		utf8_to_mbs(esc, elen, ucs, ulen);
#endif

		xmem_free(esc);

		if (total + ulen > max)
		{
			xsfree(ucs);
			return total;
		}
		if (buf)
		{
			xsncpy(buf + total, ucs, ulen);
		}
		total += ulen;

		xsfree(ucs);

		if (total + 1 > max)
			return total;

		if (buf)
		{
			buf[total] = ('&');
		}
		total++;
	}

	//remove last '&'
	if (total)
	{
		total--;

		if (buf)
		{
			buf[total] = ('\0');
		}
	}

	return total;
}

void xhttp_set_url_query(xhand_t xhttp, const tchar_t* query, int len)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (len < 0) len = xslen(query);

	phttp->len_query = xhttp_url_encoding(query, len, NULL, MAX_LONG);
	phttp->query = (byte_t*)xmem_realloc(phttp->query, phttp->len_query + 1);
	xhttp_url_encoding(query, len, phttp->query, phttp->len_query);
}

int xhttp_get_url_query(xhand_t xhttp, tchar_t* buf, int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return xhttp_url_decoding(phttp->query, phttp->len_query, buf, max);
}

void xhttp_set_url_query_entity(xhand_t xhttp, const tchar_t* key, int klen, const tchar_t* val, int vlen)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	byte_t *utf_key, *utf_val;
	dword_t ulen_key, ulen_val;

	byte_t *esc_key, *esc_val;
	dword_t elen_key, elen_val;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->type == _XHTTP_TYPE_CLI);

	if (klen < 0) klen = xslen(key);

	if (!klen)
		return;

	if (vlen < 0) vlen = xslen(val);

	//encoding the key
#ifdef _UNICODE
	ulen_key = ucs_to_utf8(key, klen, NULL, MAX_LONG);
	utf_key = (byte_t*)xmem_alloc(ulen_key);
	ucs_to_utf8(key, klen, utf_key, ulen_key);
#else
	ulen_key = mbs_to_utf8(key, klen, NULL, MAX_LONG);
	utf_key = (byte_t*)xmem_alloc(ulen_key);
	mbs_to_utf8(key, klen, utf_key, ulen_key);
#endif

	elen_key = url_byte_encode(utf_key, ulen_key, NULL, MAX_LONG);
	esc_key = (byte_t*)xmem_alloc(elen_key + 1);
	url_byte_encode(utf_key, ulen_key, esc_key, elen_key);

	xmem_free(utf_key);

	//encoding the value
#ifdef _UNICODE
	ulen_val = ucs_to_utf8(val, vlen, NULL, MAX_LONG);
	utf_val = (byte_t*)xmem_alloc(ulen_val);
	ucs_to_utf8(val, vlen, utf_val, ulen_val);
#else
	ulen_val = mbs_to_utf8(val, vlen, NULL, MAX_LONG);
	utf_val = (byte_t*)xmem_alloc(ulen_val);
	mbs_to_utf8(val, vlen, utf_val, ulen_val);
#endif

	elen_val = url_byte_encode(utf_val, ulen_val, NULL, MAX_LONG);
	esc_val = (byte_t*)xmem_alloc(elen_val + 1);
	url_byte_encode(utf_val, ulen_val, esc_val, elen_val);

	xmem_free(utf_val);

	//append '&' key '=' val
	phttp->query = (byte_t*)xmem_realloc(phttp->query, phttp->len_query + 1 + elen_key + 1 + elen_val + 1);

	//if not the first entity
	if (phttp->len_query)
	{
		phttp->query[phttp->len_query] = ('&');
		phttp->len_query++;
	}

	xmem_copy((void*)(phttp->query + phttp->len_query), (void*)esc_key, elen_key);
	phttp->len_query += elen_key;

	phttp->query[phttp->len_query] = ('=');
	phttp->len_query++;

	xmem_copy((void*)(phttp->query + phttp->len_query), (void*)esc_val, elen_val);
	phttp->len_query += elen_val;

	xmem_free(esc_key);
	xmem_free(esc_val);
}

int xhttp_get_url_query_entity(xhand_t xhttp, const tchar_t* key, int len, tchar_t* buf, int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	tchar_t* url;
	int url_len;

	tchar_t *kstr, *vstr;
	int klen, vlen;
	int n, total = 0;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (!phttp->len_query)
	{
		if (buf) buf[0] = _T('\0');
		return 0;
	}

	url_len = xhttp_url_decoding(phttp->query, phttp->len_query, NULL, MAX_LONG);
	url = xsalloc(url_len + 1);
	xhttp_url_decoding(phttp->query, phttp->len_query, url, url_len);

	while (n = parse_options_token((url + total), (url_len - total), _T('='), _T('&'), &kstr, &klen, &vstr, &vlen))
	{
		total += n;

		if (compare_text(key, len, kstr, klen, 1) == 0)
		{
			break;
		}
	}

	if (buf)
	{
		vlen = (vlen < max) ? vlen : max;
		xsncpy(buf, vstr, vlen);
	}

	xsfree(url);

	return vlen;
}

void xhttp_set_request_header(xhand_t xhttp, const tchar_t* hname, int nlen, const tchar_t* hvalue, int vlen)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	write_string_entity(phttp->st_request,hname,nlen,hvalue,vlen);
}

int xhttp_get_request_header(xhand_t xhttp, const tchar_t* hname, int nlen, tchar_t* hvalue, int hmax)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	link_t_ptr ent;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	ent = find_string_entity(phttp->st_request, hname, nlen);
	if (ent)
	{
		return get_string_entity_val(ent, hvalue, hmax);
	}
	else
	{
		if (hvalue) hvalue[0] = _T('\0');
		return 0;
	}
}

void xhttp_set_request_default_header(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t szHost[PATH_LEN + 1] = { 0 };
	tchar_t szPort[NUM_LEN + 1] = { 0 };
	tchar_t szDate[NUM_LEN + 1] = { 0 };
	xdate_t dt = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	get_utc_date(&dt);
	format_gmttime(&dt, szDate);

	xhttp_get_url_host(xhttp, szHost, PATH_LEN);
	xhttp_get_url_port(xhttp, szPort, NUM_LEN);
	
	if (!is_null(szPort) && xstos(szPort) != 80)
	{
		xscat(szHost, _T(":"));
		xscat(szHost, szPort);
	}

	clear_string_table(phttp->st_request);

	xhttp_set_request_header(xhttp, HTTP_HEADER_HOST, -1, szHost, -1);
	xhttp_set_request_header(xhttp, HTTP_HEADER_DATE, -1, szDate, -1);
	xhttp_set_request_header(xhttp, HTTP_HEADER_CONNECTION, -1, HTTP_HEADER_CONNECTION_CLOSE, -1);
}

dword_t xhttp_format_response(xhand_t xhttp, byte_t* buf, dword_t max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return _xhttp_format_response(phttp, buf, max);
}

void xhttp_get_response_code(xhand_t xhttp, tchar_t* buf)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	
	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	xsncpy(buf,phttp->code,NUM_LEN);
}

void xhttp_set_response_code(xhand_t xhttp, const tchar_t* code)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	xsncpy(phttp->code, code, NUM_LEN);
}

int	xhttp_get_response_message(xhand_t xhttp, tchar_t* buf, int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	int len;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	len = xslen(phttp->text);
	len = (len < max) ? len : max;
	if (buf)
		xsncpy(buf, phttp->text, len);

	return len;
}

void xhttp_set_response_message(xhand_t xhttp, const tchar_t* msg, int len)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (len < 0)
		len = xslen(msg);
	len = (len < ERR_LEN) ? len : ERR_LEN;
	xsncpy(phttp->text, msg, len);
}

int xhttp_get_response_header(xhand_t xhttp, const tchar_t* hname, int nlen, tchar_t* hvalue, int vmax)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	link_t_ptr ent;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	ent = find_string_entity(phttp->st_response, hname, nlen);
	if (ent)
	{
		return get_string_entity_val(ent, hvalue, vmax);
	}
	else
	{
		if (hvalue) hvalue[0] = _T('\0');
		return 0;
	}
}

void xhttp_set_response_header(xhand_t xhttp, const tchar_t* hname, int nlen, const tchar_t* hvalue, int vlen)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	write_string_entity(phttp->st_response, hname, nlen, hvalue, vlen);
}

void xhttp_set_response_default_header(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t szDate[DATE_LEN + 1] = { 0 };
	xdate_t dt = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	get_utc_date(&dt);
	format_gmttime(&dt, szDate);

	clear_string_table(phttp->st_response);

	xhttp_set_response_header(xhttp, HTTP_HEADER_CONNECTION, -1, HTTP_HEADER_CONNECTION_CLOSE, -1);
	xhttp_set_response_header(xhttp, HTTP_HEADER_DATE, -1, szDate, -1);
	xhttp_set_response_header(xhttp, HTTP_HEADER_SERVER, -1, HTTP_HEADER_SERVER_XSERVICE, -1);
}

bool_t xhttp_get_response_state(xhand_t xhttp)
{
	tchar_t sz_code[NUM_LEN + 1] = { 0 };

	xhttp_get_response_code(xhttp, sz_code);

	return IS_XHTTP_SUCCEED(sz_code);
}

void xhttp_set_response_content_length(xhand_t xhttp, dword_t len)
{
	tchar_t sz_size[NUM_LEN + 1] = { 0 };

	xsprintf(sz_size, _T("%d"), len);

	xhttp_set_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, sz_size, -1);
}

dword_t xhttp_get_response_content_length(xhand_t xhttp)
{
	tchar_t sz_size[NUM_LEN + 1] = { 0 };
	int n_size;

	xhttp_get_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, sz_size, NUM_LEN);

	n_size = xstol(sz_size);
	return (n_size < 0) ? 0 : n_size;
}

void xhttp_set_request_content_length(xhand_t xhttp, dword_t len)
{
	tchar_t sz_size[NUM_LEN + 1] = { 0 };

	xsprintf(sz_size, _T("%d"), len);

	xhttp_set_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, sz_size, -1);
}

dword_t xhttp_get_request_content_length(xhand_t xhttp)
{
	tchar_t sz_size[NUM_LEN + 1] = { 0 };

	xhttp_get_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, sz_size, NUM_LEN);

	return xstol(sz_size);
}

int xhttp_get_response_content_type_charset(xhand_t xhttp, tchar_t* buf, int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	const tchar_t* str;
	tchar_t *key, *val;
	int klen, vlen;
	link_t_ptr ent;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (buf)
	{
		xszero(buf, max);
	}

	ent = find_string_entity(phttp->st_response, HTTP_HEADER_CONTENTTYPE, -1);
	if (!ent)
		return 0;

	str = get_string_entity_val_ptr(ent);
	if (is_null(str))
		return 0;

	str = xsistr(str, HTTP_HEADER_CONTENTTYPE_ENTITY_CHARSET);
	if (is_null(str))
		return 0;

	parse_options_token(str, -1, _T('='), _T(';'), &key, &klen, &val, &vlen);

	if (!vlen)
		return 0;

	vlen = (vlen < max) ? vlen : max;

	if (buf)
	{
		xsncpy(buf, val, vlen);
	}

	return vlen;
}

void xhttp_set_response_content_type_charset(xhand_t xhttp, const tchar_t* token, int len)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	tchar_t* str;
	tchar_t ctype[META_LEN + 1] = { 0 };
	link_t_ptr ent;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	ent = find_string_entity(phttp->st_response, HTTP_HEADER_CONTENTTYPE, -1);
	if (!ent)
		return;

	get_string_entity_val(ent, ctype, META_LEN);

	str = (tchar_t*)xsistr(ctype, HTTP_HEADER_CONTENTTYPE_ENTITY_CHARSET);
	if (str)
	{
		*str = _T('\0');
	}

	str = (tchar_t*)xsistr(ctype, _T(";"));
	if (str)
	{
		*str = _T('\0');
	}

	if (len < 0)
		len = xslen(token);

	if (len)
	{
		xscat(ctype, _T("; "));
		xscat(ctype, HTTP_HEADER_CONTENTTYPE_ENTITY_CHARSET);
		xscat(ctype, _T("="));
		xsncat(ctype, token, len);
	}

	set_string_entity_val(ent, ctype, -1);
}

int xhttp_get_request_content_type_charset(xhand_t xhttp, tchar_t* buf, int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	const tchar_t* str;
	tchar_t *key, *val;
	int klen, vlen;
	link_t_ptr ent;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (buf)
	{
		xszero(buf, max);
	}

	ent = find_string_entity(phttp->st_request, HTTP_HEADER_CONTENTTYPE, -1);
	if (!ent)
		return 0;

	str = get_string_entity_val_ptr(ent);
	if (is_null(str))
		return 0;

	str = xsistr(str, HTTP_HEADER_CONTENTTYPE_ENTITY_CHARSET);
	if (is_null(str))
		return 0;

	parse_options_token(str, -1, _T('='), _T(';'), &key, &klen, &val, &vlen);

	if (!vlen)
		return 0;

	vlen = (vlen < max) ? vlen : max;

	if (buf)
	{
		xsncpy(buf, val, vlen);
	}

	return vlen;
}

void xhttp_set_request_content_type_charset(xhand_t xhttp, const tchar_t* token, int len)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	tchar_t* str;
	tchar_t ctype[META_LEN + 1] = { 0 };
	link_t_ptr ent;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	ent = find_string_entity(phttp->st_request, HTTP_HEADER_CONTENTTYPE, -1);
	if (!ent)
		return;

	get_string_entity_val(ent, ctype, META_LEN);

	str = (tchar_t*)xsistr(ctype, HTTP_HEADER_CONTENTTYPE_ENTITY_CHARSET);
	if (str)
	{
		*str = _T('\0');
	}

	str = (tchar_t*)xsistr(ctype, _T(";"));
	if (str)
	{
		*str = _T('\0');
	}

	if (len < 0)
		len = xslen(token);

	if (len)
	{
		xscat(ctype, _T("; "));
		xscat(ctype, HTTP_HEADER_CONTENTTYPE_ENTITY_CHARSET);
		xscat(ctype, _T("="));
		xsncat(ctype, token, len);
	}

	set_string_entity_val(ent, ctype, -1);
}

void xhttp_set_request_cookie(xhand_t xhttp, const tchar_t* key, const tchar_t* val, int len)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t cookies[PATH_LEN + 1] = { 0 };
	link_t_ptr ent;
	link_t_ptr st, ilk;
	int total = 0;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	ent = find_string_entity(phttp->st_request, HTTP_HEADER_COOKIE, -1);

	if (ent)
	{
		get_string_entity_val(ent, cookies, PATH_LEN);
	}

	if (len < 0)
		len = xslen(val);

	if (xslen(cookies) + xslen(key) + len + 3 >= PATH_LEN)
	{
		return;
	}

	st = create_string_table(0);

	string_table_format_options(st, cookies, -1, _T('='), _T(';'));

	ilk = get_string_next_entity(st, LINK_FIRST);

	while (ilk)
	{
		if (compare_text(get_string_entity_key_ptr(ilk), -1, key, -1, 1) == 0)
		{
			set_string_entity_val(ilk, val, len);
			break;
		}

		ilk = get_string_next_entity(st, ilk);
	}

	if (!ilk)
	{
		write_string_entity(st, key, -1, val, len);
	}

	xszero(cookies, PATH_LEN);

	total = 0;
	ilk = get_string_next_entity(st, LINK_FIRST);
	while (ilk)
	{
		total += xsprintf(cookies + total, _T("%s=%s; "), get_string_entity_key_ptr(ilk), get_string_entity_val_ptr(ilk));

		ilk = get_string_next_entity(st, ilk);
	}

	destroy_string_table(st);

	write_string_entity(phttp->st_request, HTTP_HEADER_COOKIE, -1, cookies, total);
}

int xhttp_get_request_cookie(xhand_t xhttp, const tchar_t* key, tchar_t* val, int max)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t cookies[PATH_LEN + 1] = { 0 };
	link_t_ptr ent;
	link_t_ptr st, ilk;
	int n;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	ent = find_string_entity(phttp->st_request, HTTP_HEADER_COOKIE, -1);

	if (ent)
	{
		get_string_entity_val(ent, cookies, PATH_LEN);
	}

	st = create_string_table(0);

	string_table_format_options(st, cookies, -1, _T('='), _T(';'));

	n = 0;
	ilk = get_string_next_entity(st, LINK_FIRST);
	while (ilk)
	{
		if (compare_text(get_string_entity_key_ptr(ilk), -1, key, -1, 1) == 0)
		{
			n = get_string_entity_val(ilk, val, max);
			break;
		}

		ilk = get_string_next_entity(st, ilk);
	}

	destroy_string_table(st);

	return n;
}

void xhttp_get_authorization(xhand_t xhttp, tchar_t* sz_mode, tchar_t* sz_sid, int slen, tchar_t* sz_sign, int max)
{
	tchar_t sz_auth[META_LEN + 1] = { 0 };
	int len;
	tchar_t *token;

	xhttp_get_request_header(xhttp, HTTP_HEADER_AUTHORIZATION, -1, sz_auth, META_LEN);

	if (compare_text(sz_auth, 3, _T("XDS"), 3, 1) != 0)
		return;

	token = sz_auth + 3;
	while (*token == _T(' ') && *token != _T('\0'))
		token++;

	len = 0;
	while (*token != _T(':') && *token != _T('\0'))
	{
		token++;
		len++;
	}

	slen = (slen < len) ? slen : len;
	if (sz_sid)
	{
		xsncpy(sz_sid, token - len, slen);
	}

	if (*token == _T(':'))
		token++;

	len = 0;
	while (*token != _T('\0'))
	{
		token++;
		len++;
	}

	max = (max < len) ? max : len;
	if (sz_sign)
	{
		xsncpy(sz_sign, token - len, max);
	}
}

void xhttp_set_authorization(xhand_t xhttp, const tchar_t* sz_mode, const tchar_t* sz_sid, int slen, const tchar_t* sz_sign, int max)
{
	tchar_t sz_auth[META_LEN + 1] = { 0 };

	xscat(sz_auth, _T("XDS"));

	if (slen < 0)
		slen = xslen(sz_sid);

	if (!slen)
		return;

	xscat(sz_auth, _T(" "));
	xsncat(sz_auth, sz_sid, slen);

	if (max < 0)
		max = xslen(sz_sign);

	if (!max)
		return;

	xscat(sz_auth, _T(":"));
	xsncat(sz_auth, sz_sign, max);

	xhttp_set_request_header(xhttp, HTTP_HEADER_AUTHORIZATION, -1, sz_auth, -1);
}

int xhttp_request_signature(xhand_t xhttp, const tchar_t* auth, const tchar_t* skey, tchar_t* buf, int max)
{
	tchar_t sz_verb[RES_LEN + 1] = { 0 };
	tchar_t sz_type[RES_LEN + 1] = { 0 };
	tchar_t sz_date[DATE_LEN + 1] = { 0 };
	tchar_t sz_object[PATH_LEN + 1] = { 0 };
	tchar_t sz_signature[HMAC_LEN + 1] = { 0 };

	int acc_len, utf8_len, bas_len;
	int pos;

	byte_t* acc_buf = NULL;
	byte_t* utf8_buf = NULL;

	unsigned char dig[20] = { 0 };

	xhttp_get_url_method(xhttp, sz_verb, RES_LEN);
	xhttp_get_url_object(xhttp, sz_object, PATH_LEN);
	xhttp_get_request_header(xhttp, HTTP_HEADER_CONTENTTYPE, -1, sz_type, RES_LEN);
	xhttp_get_request_header(xhttp, HTTP_HEADER_DATE, -1, sz_date, DATE_LEN);

#ifdef _UNICODE
	acc_len = ucs_to_utf8(skey, -1, NULL, MAX_LONG);
#else
	acc_len = mbs_to_utf8(skey, -1, NULL, MAX_LONG);
#endif

	acc_buf = (byte_t*)xmem_alloc(acc_len + 1);

#ifdef _UNICODE
	ucs_to_utf8(skey, -1, acc_buf, acc_len);
#else
	mbs_to_utf8(skey, -1, acc_buf, acc_len);
#endif

#ifdef _UNICODE
	utf8_len = ucs_to_utf8(sz_verb, -1, NULL, MAX_LONG) + 1 \
		+ ucs_to_utf8(sz_type, -1, NULL, MAX_LONG) + 1 \
		+ ucs_to_utf8(sz_date, -1, NULL, MAX_LONG) + 1 \
		+ ucs_to_utf8(sz_object, -1, NULL, MAX_LONG);
#else
	utf8_len = mbs_to_utf8(sz_verb, -1, NULL, MAX_LONG) + 1 \
		+ mbs_to_utf8(sz_type, -1, NULL, MAX_LONG) + 1 \
		+ mbs_to_utf8(sz_date, -1, NULL, MAX_LONG) + 1 \
		+ mbs_to_utf8(sz_object, -1, NULL, MAX_LONG);
#endif

	utf8_buf = (byte_t*)xmem_alloc(utf8_len + 1);

	pos = 0;
#ifdef _UNICODE
	pos += ucs_to_utf8(sz_verb, -1, utf8_buf + pos, utf8_len - pos);
#else
	pos += mbs_to_utf8(sz_verb, -1, utf8_buf + pos, utf8_len - pos);
#endif
	utf8_buf[pos++] = '\n';

#ifdef _UNICODE
	pos += ucs_to_utf8(sz_type, -1, utf8_buf + pos, utf8_len - pos);
#else
	pos += mbs_to_utf8(sz_type, -1, utf8_buf + pos, utf8_len - pos);
#endif
	utf8_buf[pos++] = '\n';

#ifdef _UNICODE
	pos += ucs_to_utf8(sz_date, -1, utf8_buf + pos, utf8_len - pos);
#else
	pos += mbs_to_utf8(sz_date, -1, utf8_buf + pos, utf8_len - pos);
#endif
	utf8_buf[pos++] = '\n';

#ifdef _UNICODE
	pos += ucs_to_utf8(sz_object, -1, utf8_buf + pos, utf8_len - pos);
#else
	pos += mbs_to_utf8(sz_object, -1, utf8_buf + pos, utf8_len - pos);
#endif

	sha1_hmac((unsigned char*)acc_buf, acc_len, (unsigned char*)utf8_buf, pos, dig);

	bas_len = xbas_encode(dig, 20, sz_signature, RES_LEN);

	xmem_free(acc_buf);
	xmem_free(utf8_buf);

	bas_len = (bas_len < max) ? bas_len : max;
	if (buf)
	{
		xsncpy(buf, sz_signature, bas_len);
	}

	return bas_len;
}

bool_t	xhttp_is_requested(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return  phttp->b_request;
}

bool_t	xhttp_is_responsed(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	return phttp->b_response;
}

void xhttp_reset_request(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	clear_string_table(phttp->st_request);

	if (phttp->type == _XHTTP_TYPE_CLI)
	{
		if (phttp->send_stream)
		{
			stream_free(phttp->send_stream);
			phttp->send_stream = NULL;
		}
	}

	phttp->b_request = 0;
}

void xhttp_reset_response(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	clear_string_table(phttp->st_response);

	if (phttp->type == _XHTTP_TYPE_SRV)
	{
		if (phttp->send_stream)
		{
			stream_free(phttp->send_stream);
			phttp->send_stream = NULL;
		}
	}

	phttp->b_response = 0;

	xszero(phttp->code, NUM_LEN);
	xszero(phttp->text, ERR_LEN);
}

void xhttp_send_continue(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	byte_t *buf_response = NULL;
	dword_t len_response = 0;
	int opt;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->type == _XHTTP_TYPE_SRV);
	
	TRY_CATCH;

	len_response = _xhttp_format_continue(phttp, NULL, MAX_LONG);
	buf_response = (byte_t*)xmem_alloc(len_response + 1);

	len_response = _xhttp_format_continue(phttp, buf_response, len_response);

	if (phttp->pif->pf_setopt)
	{
		opt = 0;
		(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_SNDBUF, (void*)&opt, sizeof(int));
	}

	if (phttp->pif->pf_write)
	{
		if (!(*phttp->pif->pf_write)(phttp->pif->fd, buf_response, &len_response))
		{
			raise_user_error(NULL, NULL);
		}
	}

	if (phttp->pif->pf_flush)
	{
		if (!(*phttp->pif->pf_flush)(phttp->pif->fd))
		{
			raise_user_error(NULL, NULL);
		}
	}

	xmem_free(buf_response);
	buf_response = NULL;

	END_CATCH;

	return;
ONERROR:

	if (buf_response)
		xmem_free(buf_response);

	XDK_TRACE_LAST;

	return;
}

bool_t xhttp_send_response(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t token[RES_LEN + 1] = { 0 };
	byte_t *buf_response = NULL;
	dword_t len_response = 0;
	int opt;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->type == _XHTTP_TYPE_SRV);
	
	TRY_CATCH;

	if (!phttp->b_response)
	{
		len_response = _xhttp_format_response(phttp, NULL, MAX_LONG);
		buf_response = (byte_t*)xmem_alloc(len_response + 1);

		len_response = _xhttp_format_response(phttp, buf_response, len_response);

		if (phttp->pif->pf_setopt)
		{
			opt = len_response;
			(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_SNDBUF, (void*)&opt, sizeof(int));
		}

		if (!(*phttp->pif->pf_write)(phttp->pif->fd, buf_response, &len_response))
		{
			raise_user_error(NULL, NULL);
		}

		if (phttp->pif->pf_flush)
		{
			if (!(*phttp->pif->pf_flush)(phttp->pif->fd))
			{
				raise_user_error(NULL, NULL);
			}
		}

		phttp->b_response = 1;

		if (phttp->b_response)
		{
			phttp->send_stream = stream_alloc(phttp->pif);

			xhttp_get_response_content_type_charset(xhttp, token, INT_LEN);
			if (!is_null(token))
			{
				stream_set_encode(phttp->send_stream, parse_charset(token));
			}

			if (xhttp_is_chunked_send(xhttp))
			{
				stream_set_mode(phttp->send_stream, CHUNK_OPERA);
				stream_opera_reset(phttp->send_stream);
				opt = TCP_MAX_SNDBUFF;
			}
			else if (xhttp_is_lined_send(xhttp))
			{
				stream_set_mode(phttp->send_stream, LINE_OPERA);
				stream_opera_reset(phttp->send_stream);
				opt = TCP_MAX_SNDBUFF;
			}
			else
			{
				len_response = xhttp_get_response_content_length(xhttp);
				stream_set_size(phttp->send_stream, len_response);
				opt = ((len_response) ? len_response : TCP_MAX_SNDBUFF);
			}

			if (phttp->pif->pf_setopt)
			{
				(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_SNDBUF, (void*)&opt, sizeof(int));
			}
		}

		xmem_free(buf_response);
		buf_response = NULL;
	}

	END_CATCH;

	return phttp->b_response;
ONERROR:

	if (buf_response)
		xmem_free(buf_response);

	XDK_TRACE_LAST;

	return phttp->b_response;
}

bool_t xhttp_recv_response(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	dword_t len_header, len_response,len_one;
	tchar_t token[RES_LEN + 1] = { 0 };
	byte_t* buf_response = NULL;
	int opt;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->type == _XHTTP_TYPE_CLI);
	
	TRY_CATCH;

	if (!phttp->b_response)
	{
		if (phttp->pif->pf_setopt)
		{
			opt = TCP_MIN_RCVBUFF;
			(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_RCVBUF, (void*)&opt, sizeof(int));
		}

		len_header = XHTTP_HEADER_SIZE;
		buf_response = (byte_t*)xmem_alloc(len_header + 1);
		len_response = 0;

		while (1)
		{
			len_one = 1;
			if (!(*phttp->pif->pf_read)(phttp->pif->fd, buf_response + len_response, &len_one))
			{
				raise_user_error(NULL, NULL);
			}

			if (!len_one)
            {
                raise_user_error(_T("xhttp_recv_response"), _T("recv response breaked"));
            }

			len_response++;

			if (len_response > XHTTP_HEADER_MAX)
			{
				raise_user_error(_T("xhttp_recv_response"), _T("invalid http header size"));
			}

			if (len_response > len_header)
			{
				len_header += XHTTP_HEADER_SIZE;
				buf_response = (byte_t*)xmem_realloc(buf_response, len_header + 1);
			}

			if (len_response >= 4)
			{
				if (buf_response[len_response - 1] == '\n' && buf_response[len_response - 2] == '\r' && buf_response[len_response - 3] == '\n' && buf_response[len_response - 4] == '\r')
				{
					phttp->b_response = 1;
					break;
				}
			}
		}

		if (phttp->b_response)
		{
			phttp->recv_stream = stream_alloc(phttp->pif);

			_xhttp_parse_response(phttp, buf_response, len_response);

			xhttp_get_response_content_type_charset(xhttp, token, INT_LEN);
			if (!is_null(token))
			{
				stream_set_encode(phttp->recv_stream, parse_charset(token));
			}

			if (xhttp_is_chunked_recv(xhttp))
			{
				stream_set_mode(phttp->recv_stream, CHUNK_OPERA);
				stream_opera_reset(phttp->recv_stream);
				opt = TCP_MAX_RCVBUFF;
			}
			else if (xhttp_is_lined_recv(xhttp))
			{
				stream_set_mode(phttp->recv_stream, LINE_OPERA);
				stream_opera_reset(phttp->recv_stream);
				opt = TCP_MAX_RCVBUFF;
			}
			else
			{
				len_one = xhttp_get_response_content_length(xhttp);
				stream_set_size(phttp->recv_stream, len_one);
				opt = ((len_one) ? len_one : TCP_MAX_RCVBUFF);
			}

			if (phttp->pif->pf_setopt)
			{
				(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_RCVBUF, (void*)&opt, sizeof(int));
			}
		}

		xmem_free(buf_response);
		buf_response = NULL;
	}

	END_CATCH;

	return phttp->b_response;
ONERROR:

	if (buf_response)
		xmem_free(buf_response);

	XDK_TRACE_LAST;

	return phttp->b_response;
}

bool_t xhttp_send_request(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t charset[INT_LEN + 1] = { 0 };
	byte_t *buf_request = NULL;
	dword_t len_request = 0;
	int opt;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->type == _XHTTP_TYPE_CLI);

	TRY_CATCH;

	if (!phttp->b_request)
	{
		len_request = _xhttp_format_request(phttp, NULL, MAX_LONG);
		buf_request = (byte_t*)xmem_alloc(len_request + 1);

		len_request = _xhttp_format_request(phttp, buf_request, len_request);

		if (phttp->pif->pf_setopt)
		{			
			opt = TCP_MIN_SNDBUFF;
			(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_SNDBUF, (void*)&opt, sizeof(int));
		}

		if (!(*phttp->pif->pf_write)(phttp->pif->fd, buf_request, &len_request))
		{
			raise_user_error(NULL, NULL);
		}

		if (phttp->pif->pf_flush)
		{
			if (!(*phttp->pif->pf_flush)(phttp->pif->fd))
			{
				raise_user_error(NULL, NULL);
			}
		}

		phttp->b_request = 1;

		if (phttp->b_request)
		{
			phttp->send_stream = stream_alloc(phttp->pif);

			xhttp_get_request_content_type_charset(xhttp, charset, INT_LEN);
			if (!is_null(charset))
			{
				stream_set_encode(phttp->send_stream, parse_charset(charset));
			}

			if (xhttp_is_chunked_send(xhttp))
			{
				stream_set_mode(phttp->send_stream, CHUNK_OPERA);
				stream_opera_reset(phttp->send_stream);
				opt = TCP_MAX_SNDBUFF;
			}
			else if (xhttp_is_lined_send(xhttp))
			{
				stream_set_mode(phttp->send_stream, LINE_OPERA);
				stream_opera_reset(phttp->send_stream);
				opt = TCP_MAX_SNDBUFF;
			}
			else
			{
				len_request = xhttp_get_request_content_length(xhttp);
				stream_set_size(phttp->send_stream, len_request);
				opt = ((len_request) ? len_request : TCP_MAX_SNDBUFF);
			}

			if (phttp->pif->pf_setopt)
			{
				(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_SNDBUF, (void*)&opt, sizeof(int));
			}
		}

		xmem_free(buf_request);
		buf_request = NULL;
	}

	END_CATCH;

	return phttp->b_request;
ONERROR:

	if (buf_request)
		xmem_free(buf_request);

	XDK_TRACE_LAST;

	return phttp->b_request;
}

bool_t xhttp_recv_request(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	dword_t len_header, len_request,len_one;
	tchar_t charset[INT_LEN + 1] = { 0 };
	byte_t* buf_request = NULL;
	int opt;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->type == _XHTTP_TYPE_SRV);

	TRY_CATCH;

	if (!phttp->b_request)
	{
		if (phttp->pif->pf_setopt)
		{			
			opt = TCP_MIN_RCVBUFF;
			(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_RCVBUF, (void*)&opt, sizeof(int));
		}

		len_header = XHTTP_HEADER_SIZE;
		buf_request = (byte_t*)xmem_alloc(len_header + 1);
		len_request = 0;

		while (1)
		{
			len_one = 1;
			if (!(*phttp->pif->pf_read)(phttp->pif->fd, buf_request + len_request, &len_one))
			{
				raise_user_error(NULL, NULL);
			}

			if (!len_one)
            {
                raise_user_error(_T("xhttp_recv_request"), _T("recv request beaked"));
            }

			len_request++;

			if (len_request > XHTTP_HEADER_MAX)
			{
				raise_user_error(_T("xhttp_recv_request"), _T("invalid http header size"));
			}

			if (len_request > len_header)
			{
				len_header += XHTTP_HEADER_SIZE;
				buf_request = (byte_t*)xmem_realloc(buf_request, len_header + 1);
			}

			if (len_request == 6)
			{
				if (!IS_XHTTP_METHOD((schar_t*)buf_request))
				{
					raise_user_error(_T("xhttp_recv_request"), _T("invalid http method"));
				}
			}

			if (len_request == 2)
			{
				if ((buf_request[len_request - 1] == '\n' && buf_request[len_request - 2] == '\r') || (buf_request[len_request - 1] == '\n' && buf_request[len_request - 2] == '\n'))
				{
					phttp->b_request = 1;
					break;
				}
			}

			if (len_request >= 4)
			{
				if (buf_request[len_request - 1] == '\n' && buf_request[len_request - 2] == '\r' && buf_request[len_request - 3] == '\n' && buf_request[len_request - 4] == '\r')
				{
					phttp->b_request = 1;
					break;
				}
			}
		}

		if (phttp->b_request)
		{
			phttp->recv_stream = stream_alloc(phttp->pif);

			_xhttp_parse_request(phttp, buf_request, len_request);

			xhttp_get_request_content_type_charset(xhttp, charset, INT_LEN);
			if (!is_null(charset))
			{
				stream_set_encode(phttp->recv_stream, parse_charset(charset));
			}

			if (xhttp_is_chunked_recv(xhttp))
			{
				stream_set_mode(phttp->recv_stream, CHUNK_OPERA);
				stream_opera_reset(phttp->recv_stream);
				opt = TCP_MAX_RCVBUFF;
			}
			else if (xhttp_is_lined_recv(xhttp))
			{
				stream_set_mode(phttp->recv_stream, LINE_OPERA);
				stream_opera_reset(phttp->recv_stream);
				opt = TCP_MAX_RCVBUFF;
			}
			else
			{
				len_one = xhttp_get_request_content_length(xhttp);
				stream_set_size(phttp->recv_stream, len_one);
				opt = ((len_one) ? len_one : TCP_MAX_RCVBUFF);
			}

			if (phttp->pif->pf_setopt)
			{
				(*phttp->pif->pf_setopt)(phttp->pif->fd, SOCK_OPTION_RCVBUF, (void*)&opt, sizeof(int));
			}
		}

		xmem_free(buf_request);
		buf_request = NULL;
	}

	END_CATCH;

	return phttp->b_request;
ONERROR:

	if (buf_request)
		xmem_free(buf_request);

	XDK_TRACE_LAST;

	return phttp->b_request;
}

bool_t xhttp_is_lined_recv(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	tchar_t token[RES_LEN + 1] = { 0 };
	dword_t size = 0;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->type == _XHTTP_TYPE_CLI && !phttp->b_response)
		return 0;

	if (phttp->type == _XHTTP_TYPE_SRV && !phttp->b_request)
		return 0;

	if (phttp->type == _XHTTP_TYPE_CLI)
	{
		xhttp_get_response_content_type(xhttp, token, RES_LEN);
		size = xhttp_get_response_content_length(xhttp);
	}
	else
	{
		xhttp_get_request_content_type(xhttp, token, RES_LEN);
		size = xhttp_get_request_content_length(xhttp);
	}

	return (CONTENTTYPE_IS_TEXT(token) && (!size)) ? 1 : 0;
}

bool_t xhttp_is_lined_send(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t token[RES_LEN + 1] = { 0 };
	dword_t size = 0;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->type == _XHTTP_TYPE_CLI && !phttp->b_request)
		return 0;

	if (phttp->type == _XHTTP_TYPE_SRV && !phttp->b_response)
		return 0;

	if (phttp->type == _XHTTP_TYPE_CLI)
	{
		xhttp_get_request_content_type(xhttp, token, RES_LEN);
		size = xhttp_get_request_content_length(xhttp);
	}
	else
	{
		xhttp_get_response_content_type(xhttp, token, RES_LEN);
		size = xhttp_get_response_content_length(xhttp);
	}

	return (CONTENTTYPE_IS_TEXT(token) && (!size)) ? 1 : 0;
}

bool_t xhttp_is_chunked_recv(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	tchar_t token[RES_LEN + 1] = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->type == _XHTTP_TYPE_CLI && !phttp->b_response)
		return 0;

	if (phttp->type == _XHTTP_TYPE_SRV && !phttp->b_request)
		return 0;

	if (phttp->type == _XHTTP_TYPE_CLI)
		xhttp_get_response_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, token, RES_LEN);
	else
		xhttp_get_request_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, token, RES_LEN);

	return (compare_text(token, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) == 0) ? 1 : 0;
}

bool_t xhttp_is_chunked_send(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t token[RES_LEN + 1] = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->type == _XHTTP_TYPE_CLI && !phttp->b_request)
		return 0;

	if (phttp->type == _XHTTP_TYPE_SRV && !phttp->b_response)
		return 0;

	if (phttp->type == _XHTTP_TYPE_CLI)
		xhttp_get_request_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, token, RES_LEN);
	else
		xhttp_get_response_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, token, RES_LEN);

	return (compare_text(token, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) == 0) ? 1 : 0;
}

bool_t xhttp_is_zipped_recv(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	tchar_t token[RES_LEN + 1] = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->type == _XHTTP_TYPE_CLI && !phttp->b_response)
		return 0;

	if (phttp->type == _XHTTP_TYPE_SRV && !phttp->b_request)
		return 0;

	if (phttp->type == _XHTTP_TYPE_CLI)
		xhttp_get_response_header(xhttp, HTTP_HEADER_CONTENTENCODING, -1, token, RES_LEN);
	else
		xhttp_get_request_header(xhttp, HTTP_HEADER_CONTENTENCODING, -1, token, RES_LEN);

	return (compare_text(token, -1, HTTP_HEADER_CONTENTENCODING_DEFLATE, -1, 1) == 0 || compare_text(token, -1, HTTP_HEADER_CONTENTENCODING_GZIP, -1, 1) == 0) ? 1 : 0;
}

bool_t xhttp_is_zipped_send(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t token[RES_LEN + 1] = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->type == _XHTTP_TYPE_CLI && !phttp->b_request)
		return 0;

	if (phttp->type == _XHTTP_TYPE_SRV && !phttp->b_response)
		return 0;

	if (phttp->type == _XHTTP_TYPE_CLI)
		xhttp_get_request_header(xhttp, HTTP_HEADER_CONTENTENCODING, -1, token, RES_LEN);
	else
		xhttp_get_response_header(xhttp, HTTP_HEADER_CONTENTENCODING, -1, token, RES_LEN);

	return (compare_text(token, -1, HTTP_HEADER_CONTENTENCODING_DEFLATE, -1, 1) == 0 || compare_text(token, -1, HTTP_HEADER_CONTENTENCODING_GZIP, -1, 1) == 0) ? 1 : 0;
}

bool_t xhttp_need_expect(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	tchar_t token[RES_LEN + 1] = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	xhttp_get_request_header(xhttp, HTTP_HEADER_EXPECT, -1, token, RES_LEN);
	
	return (compare_text(phttp->method, -1, HTTP_METHOD_POST,-1,1) == 0 && compare_text(token, -1, HTTP_HEADER_EXPECT_CONTINUE, -1, 1) == 0) ? 1 : 0;
}

bool_t xhttp_send(xhand_t xhttp, const byte_t* data, dword_t len)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->send_stream != NULL);

	TRY_CATCH;

	if (phttp->type == _XHTTP_TYPE_CLI && !phttp->b_request)
	{
		raise_user_error(_T("xhttp_send"), _T("xhttp not send request"));
	}

	if (phttp->type == _XHTTP_TYPE_SRV && !phttp->b_response)
	{
		raise_user_error(_T("xhttp_send"), _T("xhttp not recv response"));
	}

	if (!stream_write_bytes(phttp->send_stream, data, len))
	{
		raise_user_error(NULL, NULL);
	}

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	return 0;
}

bool_t xhttp_fush(xhand_t xhttp)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->send_stream != NULL);

	return stream_flush(phttp->send_stream);
}

bool_t xhttp_recv(xhand_t xhttp,byte_t* buf,dword_t *pb)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);
	XDK_ASSERT(phttp->recv_stream != NULL);

	TRY_CATCH;

	if (phttp->type == _XHTTP_TYPE_CLI && !phttp->b_response)
	{
		raise_user_error(_T("xhttp_recv"), _T("http not recv response"));
	}

	if (phttp->type == _XHTTP_TYPE_SRV && !phttp->b_request)
	{
		raise_user_error(_T("xhttp_recv"), _T("http not send request"));
	}

	if (!stream_read_bytes(phttp->recv_stream, buf, pb))
	{
		raise_user_error(NULL, NULL);
	}

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	return 0;
}

bool_t xhttp_send_full(xhand_t xhttp, const byte_t* buf, dword_t size)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	tchar_t trans[RES_LEN + 1] = { 0 };
	tchar_t fsize[NUM_LEN + 1] = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->type == _XHTTP_TYPE_CLI && !xhttp_is_requested(xhttp))
	{
		xhttp_get_request_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, trans, RES_LEN);
		xhttp_get_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, NUM_LEN);

		if (compare_text(trans, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) != 0 && xstol(fsize) == 0)
		{
			ltoxs(size, fsize, NUM_LEN);
			xhttp_set_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);
		}

		if (!xhttp_send_request(xhttp))
			return 0;
	}
	
	if (phttp->type == _XHTTP_TYPE_SRV && !xhttp_is_responsed(xhttp))
	{
		xhttp_get_response_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, trans, RES_LEN);
		xhttp_get_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, NUM_LEN);

		if (compare_text(trans, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) != 0 && xstol(fsize) == 0)
		{
			ltoxs(size, fsize, NUM_LEN);
			xhttp_set_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);
		}

		if (!xhttp_send_response(xhttp))
			return 0;
	}

	XDK_ASSERT(phttp->send_stream != NULL);

	if (!stream_write_bytes(phttp->send_stream, buf, size))
	{
		return 0;
	}

	return stream_flush(phttp->send_stream);
}

bool_t xhttp_recv_full(xhand_t xhttp, byte_t** pbuf, dword_t* plen)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	dword_t npos, nbys;
	bool_t rt = 0;

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	if (phttp->type == _XHTTP_TYPE_CLI && !xhttp_is_responsed(xhttp))
	{
		if (!xhttp_recv_response(xhttp))
			return 0;
	}
	
	if (phttp->type == _XHTTP_TYPE_SRV && !xhttp_is_requested(xhttp))
	{
		if (!xhttp_recv_request(xhttp))
			return 0;
	}

	XDK_ASSERT(phttp->recv_stream != NULL);

	if (xhttp_is_chunked_recv(xhttp))
	{
		npos = 0;
		do{
			nbys = 0;
			if (!(rt = stream_read_chunk_size(phttp->recv_stream, &nbys)))
				break;

			if (!nbys)
				break;

			bytes_realloc(pbuf, npos + nbys);

			if (!(rt = stream_read_bytes(phttp->recv_stream, *pbuf + npos, &nbys)))
				break;

			npos += nbys;
		} while (rt);

		*plen = npos;
		return rt;
	}
	else
	{
		if (phttp->type == _XHTTP_TYPE_CLI)
			npos = xhttp_get_response_content_length(xhttp);
		else
			npos = xhttp_get_request_content_length(xhttp);

		bytes_realloc(pbuf, npos);

		rt = stream_read_bytes(phttp->recv_stream, *pbuf, &npos);

		*plen = npos;
		return 1;
	}
}

bool_t xhttp_send_string(xhand_t xhttp, string_t var)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);
	int type;
	tchar_t trans[RES_LEN + 1] = { 0 };
	tchar_t fsize[NUM_LEN + 1] = { 0 };
	dword_t size = 0;
	tchar_t charset[RES_LEN + 1] = { 0 };

	XDK_ASSERT(xhttp && xhttp->tag == _HANDLE_INET);

	type = xhttp_type(xhttp);

	if (type == _XHTTP_TYPE_CLI && !xhttp_is_requested(xhttp))
	{
		xhttp_get_request_header(xhttp, HTTP_HEADER_TRANSFERENCODING, -1, trans, RES_LEN);
		xhttp_get_request_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, NUM_LEN);
		xhttp_get_request_content_type_charset(xhttp, charset, INT_LEN);

		if (compare_text(trans, -1, HTTP_HEADER_TRANSFERENCODING_CHUNKED, -1, 1) != 0 && xstol(fsize) == 0)
		{
			size = string_encode(var, parse_charset(charset), NULL, MAX_LONG);
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
			size = string_encode(var, parse_charset(charset), NULL, MAX_LONG);
			ltoxs(size, fsize, NUM_LEN);
			xhttp_set_response_header(xhttp, HTTP_HEADER_CONTENTLENGTH, -1, fsize, -1);
		}

		if (!xhttp_send_response(xhttp))
			return 0;
	}

	XDK_ASSERT(phttp->send_stream != NULL);

	if (!stream_write_line(phttp->send_stream, var, NULL))
		return 0;

	return stream_flush(phttp->send_stream);
}

bool_t xhttp_recv_string(xhand_t xhttp, string_t var)
{
	http_context* phttp = TypePtrFromHead(http_context, xhttp);

	int type;

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

	XDK_ASSERT(phttp->recv_stream != NULL);

	return stream_read_line(phttp->recv_stream, var, NULL);
}


#endif /*XDK_SUPPORT_SOCK*/
