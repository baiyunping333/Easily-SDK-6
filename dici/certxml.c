/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc cert document

	@module	certdom.c | implement file

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

#include "certxml.h"
#include "certdoc.h"
#include "certoid.h"
#include "certctx.h"
#include "certpem.h"

typedef struct _CERT_PARSE_PARAM{
	link_t_ptr plk;
	link_t_ptr nlk;
}CERT_PARSE_PARAM;

static bool_t on_read_cert_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = NULL;
	clear_cert_doc(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_begin(void* pa, const byte_t* tbs_raw, dword_t raw_len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_cert_tbs_certificate(pcp->plk, 1);

	return 1;
}

static bool_t on_read_tbs_version(void* pa, int ver)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	set_tbs_version(pcp->nlk, ver);

	return 1;
}

static bool_t on_read_tbs_serial_number(void* pa, const byte_t* num, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 3] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(num, len, 1, token + 2, OID_MAX_SIZE);

	set_tbs_serial_number(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_signature_algorithm_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_signature_algorithm(pcp->plk, 1);

	return 1;
}

static bool_t on_read_tbs_signature_algorithm_identifier(void* pa, const byte_t* oid, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 1] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = oid_to_string(oid, len, token, OID_MAX_SIZE);

	set_tbs_signature_algorithm_identifier(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_signature_algorithm_parameters(void* pa, const byte_t* pss, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 3] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(pss, len, 1, token, OID_MAX_SIZE);

	set_tbs_signature_algorithm_parameters(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_signature_algorithm_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_issuer_begin(void* pa, const byte_t* issuer_raw, dword_t raw_len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_issuer(pcp->plk, 1);

	return 1;
}

static bool_t on_read_tbs_issuer_name_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_next_issuer_name(pcp->plk, LINK_LAST, 1);

	return 1;
}

static bool_t on_read_tbs_issuer_attribute_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_next_issuer_name_attribute(pcp->plk, LINK_LAST, 1);

	return 1;
}

static bool_t on_read_tbs_issuer_attribute_type(void* pa, const byte_t* oid, int len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 1] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = oid_to_string(oid, len, token, OID_MAX_SIZE);

	set_tbs_issuer_name_attribute_type(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_issuer_attribute_value(void* pa, const byte_t* attr, int alen)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t* token;
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = format_octet_string(attr, alen, 1, NULL, MAX_LONG);
	token = xsalloc(n + 3);
	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(attr, alen, 1, token + 2, n);

	set_tbs_issuer_name_attribute_value(pcp->nlk, token, n);
	xsfree(token);

	return 1;
}

static bool_t on_read_tbs_issuer_attribute_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_issuer_name_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_issuer_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_validity_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_validity(pcp->plk, 1);

	return 1;
}

static bool_t on_read_tbs_validity_notbefore(void* pa, const xdate_t* not_before)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	set_tbs_validity_notbefore(pcp->nlk, not_before);

	return 1;
}

static bool_t on_read_tbs_validity_notafter(void* pa, const xdate_t* not_after)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	set_tbs_validity_notafter(pcp->nlk, not_after);

	return 1;
}

static bool_t on_read_tbs_validity_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_subject_begin(void* pa, const byte_t* subject_raw, dword_t raw_len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_subject(pcp->plk, 1);

	return 1;
}

static bool_t on_read_tbs_subject_name_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_next_subject_name(pcp->plk, LINK_LAST, 1);

	return 1;
}

static bool_t on_read_tbs_subject_attribute_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_next_subject_name_attribute(pcp->plk, LINK_LAST, 1);

	return 1;
}

static bool_t on_read_tbs_subject_attribute_type(void* pa, const byte_t* oid, int len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 1] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = oid_to_string(oid, len, token, OID_MAX_SIZE);

	set_tbs_subject_name_attribute_type(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_subject_attribute_value(void* pa, const byte_t* attr, int alen)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t* token;
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = format_octet_string(attr, alen, 1, NULL, MAX_LONG);
	token = xsalloc(n + 3);
	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(attr, alen, 1, token + 2, n);

	set_tbs_subject_name_attribute_value(pcp->nlk, token, n);
	xsfree(token);

	return 1;
}

static bool_t on_read_tbs_subject_attribute_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_subject_name_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_subject_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_subject_publickey_info_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_subject_publickey_info(pcp->plk, 1);

	return 1;
}

static bool_t on_read_tbs_subject_publickey_algorithm_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_subject_publickey_algorithm(pcp->plk, 1);

	return 1;
}

static bool_t on_read_tbs_subject_publickey_algorithm_identifier(void* pa, const byte_t* oid, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 1] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = oid_to_string(oid, len, token, OID_MAX_SIZE);

	set_tbs_subject_publickey_algorithm_identifier(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_subject_publickey_algorithm_parameters(void* pa, const byte_t* oid, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 1] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = oid_to_string(oid, len, token, OID_MAX_SIZE);

	set_tbs_subject_publickey_algorithm_parameters(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_subject_publickey_algorithm_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_subject_publickey(void* pa, const byte_t* key, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	tchar_t *token;
	int n;

	n = format_octet_string(key, len, 1, NULL, MAX_LONG);
	token = xsalloc(n + 3);
	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(key, len, 1, token + 2, n);

	set_tbs_subject_publickey(pcp->nlk, token, n);
	xsfree(token);

	return 1;
}

static bool_t on_read_tbs_subject_publickey_info_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_issuer_uuid(void* pa, const byte_t* uuid, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 3] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(uuid, len, 1, token + 2, OID_MAX_SIZE);

	set_tbs_issuer_uniqueID(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_subject_uuid(void* pa, const byte_t* uuid, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 3] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(uuid, len, 1, token + 2, OID_MAX_SIZE);

	set_tbs_subject_uniqueID(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_extensions_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_extensions(pcp->plk, 1);

	return 1;
}

static bool_t on_read_tbs_extensions_extn_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_tbs_next_extensions_extn(pcp->plk, LINK_LAST, 1);

	return 1;
}

static bool_t on_read_tbs_extensions_extn_id(void* pa, const byte_t* oid, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 1] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = oid_to_string(oid, len, token, OID_MAX_SIZE);

	set_tbs_extensions_extn_id(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_extensions_critical(void* pa, bool_t criti)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	set_tbs_extensions_extn_critical(pcp->nlk, criti);

	return 1;
}

static bool_t on_read_tbs_extensions_extn_value(void* pa, const byte_t* val, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 3] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(val, len, 1, token + 2, OID_MAX_SIZE);

	set_tbs_extensions_extn_value(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_tbs_extensions_extn_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_extensions_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_tbs_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_cert_signature_algorithm_begin(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = pcp->nlk;
	pcp->nlk = get_cert_signature_algorithm(pcp->plk, 1);

	return 1;
}

static bool_t on_read_cert_signature_algorithm_identifier(void* pa, const byte_t* oid, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 1] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = oid_to_string(oid, len, token, OID_MAX_SIZE);

	set_cert_signature_algorithm_identifier(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_cert_signature_algorithm_parameters(void* pa, const byte_t* pss, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t token[OID_MAX_SIZE + 3] = { 0 };
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(pss, len, 1, token + 2, OID_MAX_SIZE);

	set_cert_signature_algorithm_parameters(pcp->nlk, token, n);

	return 1;
}

static bool_t on_read_cert_signature_algorithm_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->nlk = get_dom_parent_node(pcp->nlk);
	pcp->plk = get_dom_parent_node(pcp->nlk);

	return 1;
}

static bool_t on_read_cert_signature(void* pa, const byte_t* sig, dword_t len)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	tchar_t *token;
	int n;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	n = format_octet_string(sig, len, 1, NULL, MAX_LONG);
	token = xsalloc(n + 3);
	xscpy(token, _T("0X"));
	n = 2 + format_octet_string(sig, len, 1, token + 2, n);

	set_cert_signature(pcp->nlk, token, n);
	xsfree(token);

	return 1;
}

static bool_t on_read_cert_end(void* pa)
{
	CERT_PARSE_PARAM* pcp = (CERT_PARSE_PARAM*)pa;

	XDK_ASSERT(pcp != NULL && pcp->nlk != NULL);

	pcp->plk = get_dom_parent_node(pcp->nlk);

	XDK_ASSERT(pcp->plk == NULL);

	return 1;
}

/************************************************************************************************/

bool_t parse_cert_doc_from_bytes(link_t_ptr ptr, const byte_t* buf, dword_t len)
{
	cert_reader cr = { 0 };
	bool_t b_pem = 0;
	byte_t *der = NULL;
	dword_t n;
	CERT_PARSE_PARAM cp = { 0 };
	int ret;

	cr.read_cert_begin = on_read_cert_begin;
	cr.read_tbs_begin = on_read_tbs_begin;
	cr.read_tbs_version = on_read_tbs_version;
	cr.read_tbs_serial_number = on_read_tbs_serial_number;
	cr.read_tbs_signature_algorithm_begin = on_read_tbs_signature_algorithm_begin;
	cr.read_tbs_signature_algorithm_identifier = on_read_tbs_signature_algorithm_identifier;
	cr.read_tbs_signature_algorithm_parameters = on_read_tbs_signature_algorithm_parameters;
	cr.read_tbs_signature_algorithm_end = on_read_tbs_signature_algorithm_end;
	cr.read_tbs_issuer_begin = on_read_tbs_issuer_begin;
	cr.read_tbs_issuer_name_begin = on_read_tbs_issuer_name_begin;
	cr.read_tbs_issuer_attribute_begin = on_read_tbs_issuer_attribute_begin;
	cr.read_tbs_issuer_attribute_type = on_read_tbs_issuer_attribute_type;
	cr.read_tbs_issuer_attribute_value = on_read_tbs_issuer_attribute_value;
	cr.read_tbs_issuer_attribute_end = on_read_tbs_issuer_attribute_end;
	cr.read_tbs_issuer_name_end = on_read_tbs_issuer_name_end;
	cr.read_tbs_issuer_end = on_read_tbs_issuer_end;
	cr.read_tbs_validity_begin = on_read_tbs_validity_begin;
	cr.read_tbs_validity_notbefore = on_read_tbs_validity_notbefore;
	cr.read_tbs_validity_notafter = on_read_tbs_validity_notafter;
	cr.read_tbs_validity_end = on_read_tbs_validity_end;
	cr.read_tbs_subject_begin = on_read_tbs_subject_begin;
	cr.read_tbs_subject_name_begin = on_read_tbs_subject_name_begin;
	cr.read_tbs_subject_attribute_begin = on_read_tbs_subject_attribute_begin;
	cr.read_tbs_subject_attribute_type = on_read_tbs_subject_attribute_type;
	cr.read_tbs_subject_attribute_value = on_read_tbs_subject_attribute_value;
	cr.read_tbs_subject_attribute_end = on_read_tbs_subject_attribute_end;
	cr.read_tbs_subject_name_end = on_read_tbs_subject_name_end;
	cr.read_tbs_subject_end = on_read_tbs_subject_end;
	cr.read_tbs_subject_publickey_info_begin = on_read_tbs_subject_publickey_info_begin;
	cr.read_tbs_subject_publickey_algorithm_begin = on_read_tbs_subject_publickey_algorithm_begin;
	cr.read_tbs_subject_publickey_algorithm_identifier = on_read_tbs_subject_publickey_algorithm_identifier;
	cr.read_tbs_subject_publickey_algorithm_parameters = on_read_tbs_subject_publickey_algorithm_parameters;
	cr.read_tbs_subject_publickey_algorithm_end = on_read_tbs_subject_publickey_algorithm_end;
	cr.read_tbs_subject_publickey = on_read_tbs_subject_publickey;
	cr.read_tbs_subject_publickey_info_end = on_read_tbs_subject_publickey_info_end;
	cr.read_tbs_issuer_uuid = on_read_tbs_issuer_uuid;
	cr.read_tbs_subject_uuid = on_read_tbs_subject_uuid;
	cr.read_tbs_extensions_begin = on_read_tbs_extensions_begin;
	cr.read_tbs_extensions_extn_begin = on_read_tbs_extensions_extn_begin;
	cr.read_tbs_extensions_extn_id = on_read_tbs_extensions_extn_id;
	cr.read_tbs_extensions_critical = on_read_tbs_extensions_critical;
	cr.read_tbs_extensions_extn_value = on_read_tbs_extensions_extn_value;
	cr.read_tbs_extensions_extn_end = on_read_tbs_extensions_extn_end;
	cr.read_tbs_extensions_end = on_read_tbs_extensions_end;
	cr.read_tbs_end = on_read_tbs_end;
	cr.read_cert_signature_algorithm_begin = on_read_cert_signature_algorithm_begin;
	cr.read_cert_signature_algorithm_identifier = on_read_cert_signature_algorithm_identifier;
	cr.read_cert_signature_algorithm_parameters = on_read_cert_signature_algorithm_parameters;
	cr.read_cert_signature_algorithm_end = on_read_cert_signature_algorithm_end;
	cr.read_cert_signature = on_read_cert_signature;
	cr.read_cert_end = on_read_cert_end;

	if (a_xsnstr((schar_t*)buf, len, PEM_CRT_BEGIN) != NULL)
		b_pem = 1;

	if (b_pem)
	{
		n = pem_decode(buf, len, NULL, MAX_LONG, PEM_CRT_BEGIN, PEM_CRT_END, NULL, 0);
		if (!n)
		{
			set_last_error(_T("parse_cert_doc_from_der"), _T("pem_decoding"), -1);
			goto clean;
		}
		
		der = (byte_t*)xmem_alloc(n);

		pem_decode(buf, len, der, n, PEM_CRT_BEGIN, PEM_CRT_END, NULL, 0);
	}
	else
	{
		n = len;
		der = (byte_t*)xmem_alloc(n);
		xmem_copy((void*)der, (void*)buf, n);
	}

	cp.nlk = ptr;
	ret = cert_read_der(&cr, (void*)&cp, der, n);

	xmem_free(der);

	return (ret == 0) ? 1 : 0;
clean:

	if(der)
	{
		xmem_free(der);
	}

	return (0);
}


