/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc cert document

	@module	cert.c | implement file

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

#include "certdoc.h"

link_t_ptr create_cert_doc(void)
{
	link_t_ptr ptr;

	ptr = create_dom_doc();
	set_dom_node_name(ptr, DOC_CERT, -1);

	return ptr;
}

void destroy_cert_doc(link_t_ptr ptr)
{
	destroy_dom_doc(ptr);
}

void clear_cert_doc(link_t_ptr ptr)
{
	delete_dom_child_nodes(ptr);
}

bool_t is_cert_doc(link_t_ptr ptr)
{
	return (compare_text(get_dom_node_name_ptr(ptr), -1, DOC_CERT, -1, 0) == 0) ? 1 : 0;
}

link_t_ptr get_cert_tbs_certificate(link_t_ptr ptr, bool_t add)
{
	link_t_ptr tbs;

	tbs = find_dom_node_by_name(ptr, 0, DOC_CERT_TBS_CERTIFICATE, -1);
	if (!tbs && add)
	{
		tbs = insert_dom_node(ptr, LINK_FIRST);
		set_dom_node_name(tbs, DOC_CERT_TBS_CERTIFICATE, -1);
	}

	return tbs;
}

int get_tbs_version(link_t_ptr tbs, bool_t add)
{
	link_t_ptr nlk;
	const tchar_t* txt;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_VERSION, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(tbs, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_VERSION, -1);
	}

	txt = (nlk) ? get_dom_node_text_ptr(nlk) : NULL;

	return xstol(txt);
}

void set_tbs_version(link_t_ptr tbs, int ver)
{
	link_t_ptr nlk;
	tchar_t txt[NUM_LEN + 1] = { 0 };

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_VERSION, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(tbs, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_VERSION, -1);
	}

	ltoxs(ver, txt, NUM_LEN);
	set_dom_node_text(nlk, txt, -1);
}

int get_tbs_serial_number(link_t_ptr tbs, tchar_t* sn, int max, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SERIAL_NUMBER, -1);
	if (!nlk && add)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_VERSION, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_SERIAL_NUMBER, -1);
	}

	if (!nlk) return 0;

	return get_dom_node_text(nlk, sn, max);
}

void set_tbs_serial_number(link_t_ptr tbs, const tchar_t* sn, int n)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SERIAL_NUMBER, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_VERSION, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_SERIAL_NUMBER, -1);
	}

	set_dom_node_text(nlk, sn, n);
}

link_t_ptr get_tbs_signature_algorithm(link_t_ptr tbs, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SIGNATURE_ALGORITHM, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SERIAL_NUMBER, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_SIGNATURE_ALGORITHM, -1);
	}

	return nlk;
}

bool_t get_tbs_signature_algorithm_identifier(link_t_ptr alg, tchar_t* oid, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alg, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, oid, MAX_LONG);

	return 1;
}

void set_tbs_signature_algorithm_identifier(link_t_ptr alg, const tchar_t* oid, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alg, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	}
	
	set_dom_node_text(nlk, oid, olen);
}

bool_t get_tbs_signature_algorithm_parameters(link_t_ptr alg, tchar_t* pss, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alg, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, pss, MAX_LONG);

	return 1;
}

void set_tbs_signature_algorithm_parameters(link_t_ptr alg, const tchar_t* pss, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alg, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	}

	set_dom_node_text(nlk, pss, olen);
}

link_t_ptr get_tbs_issuer(link_t_ptr tbs, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_ISSUER, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SIGNATURE_ALGORITHM, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_ISSUER, -1);
	}

	return nlk;
}

link_t_ptr get_tbs_next_issuer_name(link_t_ptr iss, link_t_ptr rdn, bool_t add)
{
	link_t_ptr nlk;

	if (rdn == LINK_FIRST || rdn == LINK_LAST)
		nlk = NULL;
	else
		nlk = get_dom_next_sibling_node(rdn);

	if (!nlk && add)
	{
		nlk = insert_dom_node(iss, rdn);
		set_dom_node_name(nlk, DOC_CERT_RDN, -1);
	}

	return nlk;
}

link_t_ptr get_tbs_next_issuer_name_attribute(link_t_ptr rdn, link_t_ptr alk, bool_t add)
{
	link_t_ptr nlk;

	if (alk == LINK_FIRST || alk == LINK_LAST)
		nlk = NULL;
	else
		nlk = get_dom_next_sibling_node(alk);

	if (!nlk && add)
	{
		nlk = insert_dom_node(rdn, alk);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE, -1);
	}

	return nlk;
}

bool_t get_tbs_issuer_name_attribute_type(link_t_ptr alk, tchar_t* type, int* tlen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alk, 0, DOC_CERT_ATTRIBUTE_TYPE, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alk, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE_TYPE, -1);
	}
	if (!nlk) return 0;

	*tlen = get_dom_node_text(nlk, type, MAX_LONG);

	return 1;
}

void set_tbs_issuer_name_attribute_type(link_t_ptr alk, const tchar_t* type, int tlen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alk, 0, DOC_CERT_ATTRIBUTE_TYPE, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alk, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE_TYPE, -1);
	}
	set_dom_node_text(nlk, type, tlen);
}

bool_t get_tbs_issuer_name_attribute_value(link_t_ptr alk, tchar_t* value, int* vlen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alk, 0, DOC_CERT_ATTRIBUTE_VALUE, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alk, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE_VALUE, -1);
	}
	if (!nlk) return 0;

	*vlen = get_dom_node_text(nlk, value, MAX_LONG);

	return 1;
}

void set_tbs_issuer_name_attribute_value(link_t_ptr alk, const tchar_t* value, int vlen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alk, 0, DOC_CERT_ATTRIBUTE_VALUE, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alk, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE_VALUE, -1);
	}
	set_dom_node_text(nlk, value, vlen);
}

link_t_ptr get_tbs_validity(link_t_ptr tbs, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_VALIDITY, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_ISSUER, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_VALIDITY, -1);
	}

	return nlk;
}

bool_t get_tbs_validity_notbefore(link_t_ptr vlk, xdate_t* not_before, bool_t add)
{
	link_t_ptr nlk;
	const tchar_t* txt;

	nlk = find_dom_node_by_name(vlk, 0, DOC_CERT_NOTBEFORE, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(vlk, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_NOTBEFORE, -1);
	}
	if (!nlk) return 0;

	txt = get_dom_node_text_ptr(nlk);
	parse_date(not_before, txt);

	return 1;
}

void set_tbs_validity_notbefore(link_t_ptr vlk, const xdate_t* not_before)
{
	link_t_ptr nlk;
	tchar_t txt[DATE_LEN] = { 0 };

	nlk = find_dom_node_by_name(vlk, 0, DOC_CERT_NOTBEFORE, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(vlk, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_NOTBEFORE, -1);
	}

	format_date(not_before, txt);
	set_dom_node_text(nlk, txt, -1);
}

bool_t get_tbs_validity_notafter(link_t_ptr vlk, xdate_t* not_after, bool_t add)
{
	link_t_ptr nlk;
	const tchar_t* txt;

	nlk = find_dom_node_by_name(vlk, 0, DOC_CERT_NOTAFTER, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(vlk, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_NOTAFTER, -1);
	}
	if (!nlk) return 0;

	txt = get_dom_node_text_ptr(nlk);
	parse_date(not_after, txt);

	return 1;
}

void set_tbs_validity_notafter(link_t_ptr vlk, const xdate_t* not_after)
{
	link_t_ptr nlk;
	tchar_t txt[DATE_LEN] = { 0 };

	nlk = find_dom_node_by_name(vlk, 0, DOC_CERT_NOTAFTER, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(vlk, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_NOTAFTER, -1);
	}

	format_date(not_after, txt);
	set_dom_node_text(nlk, txt, -1);
}

link_t_ptr get_tbs_subject(link_t_ptr tbs, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SUBJECT, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_VALIDITY, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_SUBJECT, -1);
	}

	return nlk;
}

link_t_ptr get_tbs_next_subject_name(link_t_ptr sub, link_t_ptr rdn, bool_t add)
{
	link_t_ptr nlk;

	if (rdn == LINK_FIRST || rdn == LINK_LAST)
		nlk = NULL;
	else
		nlk = get_dom_next_sibling_node(rdn);

	if (!nlk && add)
	{
		nlk = insert_dom_node(sub, rdn);
		set_dom_node_name(nlk, DOC_CERT_RDN, -1);
	}

	return nlk;
}

link_t_ptr get_tbs_next_subject_name_attribute(link_t_ptr rdn, link_t_ptr alk, bool_t add)
{
	link_t_ptr nlk;

	if (alk == LINK_FIRST || alk == LINK_LAST)
		nlk = NULL;
	else
		nlk = get_dom_next_sibling_node(alk);

	if (!nlk && add)
	{
		nlk = insert_dom_node(rdn, alk);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE, -1);
	}

	return nlk;
}

bool_t get_tbs_subject_name_attribute_type(link_t_ptr alk, tchar_t* type, int* tlen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alk, 0, DOC_CERT_ATTRIBUTE_TYPE, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alk, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE_TYPE, -1);
	}
	if (!nlk) return 0;

	*tlen = get_dom_node_text(nlk, type, MAX_LONG);

	return 1;
}

void set_tbs_subject_name_attribute_type(link_t_ptr alk, const tchar_t* type, int tlen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alk, 0, DOC_CERT_ATTRIBUTE_TYPE, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alk, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE_TYPE, -1);
	}
	set_dom_node_text(nlk, type, tlen);
}

bool_t get_tbs_subject_name_attribute_value(link_t_ptr alk, tchar_t* value, int* vlen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alk, 0, DOC_CERT_ATTRIBUTE_VALUE, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alk, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE_VALUE, -1);
	}
	if (!nlk) return 0;

	*vlen = get_dom_node_text(nlk, value, MAX_LONG);

	return 1;
}

void set_tbs_subject_name_attribute_value(link_t_ptr alk, const tchar_t* value, int vlen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alk, 0, DOC_CERT_ATTRIBUTE_VALUE, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alk, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ATTRIBUTE_VALUE, -1);
	}
	set_dom_node_text(nlk, value, vlen);
}

link_t_ptr get_tbs_subject_publickey_info(link_t_ptr tbs, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SUBJECT_PUBLICKEY_INFO, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SUBJECT, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_SUBJECT_PUBLICKEY_INFO, -1);
	}

	return nlk;
}

link_t_ptr get_tbs_subject_publickey_algorithm(link_t_ptr pki, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(pki, 0, DOC_CERT_PUBLICKEY_ALGORITHM, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(pki, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_PUBLICKEY_ALGORITHM, -1);
	}

	return nlk;
}

bool_t get_tbs_subject_publickey_algorithm_identifier(link_t_ptr alg, tchar_t* oid, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alg, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, oid, MAX_LONG);

	return 1;
}

void set_tbs_subject_publickey_algorithm_identifier(link_t_ptr alg, const tchar_t* oid, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alg, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	}

	set_dom_node_text(nlk, oid, olen);
}

bool_t get_tbs_subject_publickey_algorithm_parameters(link_t_ptr alg, tchar_t* curve, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alg, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, curve, MAX_LONG);

	return 1;
}

void set_tbs_subject_publickey_algorithm_parameters(link_t_ptr alg, const tchar_t* curve, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alg, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	}

	set_dom_node_text(nlk, curve, olen);
}

bool_t get_tbs_subject_publickey(link_t_ptr pki, tchar_t* key, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(pki, 0, DOC_CERT_SUBJECT_PUBLICKEY, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(pki, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_SUBJECT_PUBLICKEY, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, key, MAX_LONG);

	return 1;
}

void set_tbs_subject_publickey(link_t_ptr pki, const tchar_t* key, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(pki, 0, DOC_CERT_SUBJECT_PUBLICKEY, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(pki, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_SUBJECT_PUBLICKEY, -1);
	}
	set_dom_node_text(nlk, key, olen);
}

bool_t get_tbs_issuer_uniqueID(link_t_ptr tbs, tchar_t* id, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_ISSUER_UNIQUEID, -1);
	if (!nlk && add)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SUBJECT_PUBLICKEY_INFO, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_ISSUER_UNIQUEID, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, id, MAX_LONG);

	return 1;
}

void set_tbs_issuer_uniqueID(link_t_ptr tbs, const tchar_t* id, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_ISSUER_UNIQUEID, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SUBJECT_PUBLICKEY_INFO, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_ISSUER_UNIQUEID, -1);
	}

	set_dom_node_text(nlk, id, olen);
}

bool_t get_tbs_subject_uniqueID(link_t_ptr tbs, tchar_t* id, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SUBJECT_UNIQUEID, -1);
	if (!nlk && add)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_ISSUER_UNIQUEID, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_SUBJECT_UNIQUEID, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, id, MAX_LONG);

	return 1;
}

void set_tbs_subject_uniqueID(link_t_ptr tbs, const tchar_t* id, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_SUBJECT_UNIQUEID, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_ISSUER_UNIQUEID, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(tbs, nlk);
		set_dom_node_name(nlk, DOC_CERT_SUBJECT_UNIQUEID, -1);
	}

	set_dom_node_text(nlk, id, olen);
}

link_t_ptr get_tbs_extensions(link_t_ptr tbs, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(tbs, 0, DOC_CERT_EXTENSIONS, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(tbs, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_EXTENSIONS, -1);
	}

	return nlk;
}

link_t_ptr get_tbs_next_extensions_extn(link_t_ptr exts, link_t_ptr elk, bool_t add)
{
	link_t_ptr nlk;

	if (elk == LINK_FIRST || elk == LINK_LAST)
		nlk = NULL;
	else
		nlk = get_dom_next_sibling_node(elk);

	if (!nlk && add)
	{
		nlk = insert_dom_node(exts, elk);
		set_dom_node_name(nlk, DOC_CERT_EXTENSION, -1);
	}

	return nlk;
}

bool_t get_tbs_extensions_extn_id(link_t_ptr elk, tchar_t* id, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(elk, 0, DOC_CERT_EXTEN_ID, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(elk, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_EXTEN_ID, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, id, MAX_LONG);

	return 1;
}

void set_tbs_extensions_extn_id(link_t_ptr elk, const tchar_t* id, int len)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(elk, 0, DOC_CERT_EXTEN_ID, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(elk, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_EXTEN_ID, -1);
	}

	set_dom_node_text(nlk, id, len);
}

bool_t get_tbs_extensions_extn_critical(link_t_ptr elk, bool_t* criti, bool_t add)
{
	link_t_ptr nlk;
	const tchar_t* txt;

	nlk = find_dom_node_by_name(elk, 0, DOC_CERT_CRITICAL, -1);
	if (!nlk && add)
	{
		nlk = find_dom_node_by_name(elk, 0, DOC_CERT_EXTEN_ID, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(elk, nlk);
		set_dom_node_name(nlk, DOC_CERT_CRITICAL, -1);
	}
	if (!nlk) return 0;

	txt = get_dom_node_text_ptr(nlk);
	*criti = (xstol(txt)) ? 1 : 0;

	return 1;
}

void set_tbs_extensions_extn_critical(link_t_ptr elk, bool_t criti)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(elk, 0, DOC_CERT_CRITICAL, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(elk, 0, DOC_CERT_EXTEN_ID, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(elk, nlk);
		set_dom_node_name(nlk, DOC_CERT_CRITICAL, -1);
	}

	set_dom_node_text(nlk, ((criti)? _T("1") : _T("0")), 1);
}

bool_t get_tbs_extensions_extn_value(link_t_ptr elk, tchar_t* val, int* len, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(elk, 0, DOC_CERT_EXTEN_VALUE, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(elk, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_EXTEN_VALUE, -1);
	}
	if (!nlk) return 0;

	*len = get_dom_node_text(nlk, val, MAX_LONG);

	return 1;
}

void set_tbs_extensions_extn_value(link_t_ptr elk, const tchar_t* val, int len)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(elk, 0, DOC_CERT_EXTEN_VALUE, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(elk, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_EXTEN_VALUE, -1);
	}

	set_dom_node_text(nlk, val, len);
}

link_t_ptr get_cert_signature_algorithm(link_t_ptr ptr, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(ptr, 0, DOC_CERT_SIGNATURE_ALGORITHM, -1);
	if (!nlk)
	{
		nlk = find_dom_node_by_name(ptr, 0, DOC_CERT_TBS_CERTIFICATE, -1);
		if (!nlk) nlk = LINK_FIRST;
		nlk = insert_dom_node(ptr, nlk);
		set_dom_node_name(nlk, DOC_CERT_SIGNATURE_ALGORITHM, -1);
	}

	return nlk;
}

bool_t get_cert_signature_algorithm_identifier(link_t_ptr alg, tchar_t* oid, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alg, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, oid, MAX_LONG);

	return 1;
}

void set_cert_signature_algorithm_identifier(link_t_ptr alg, const tchar_t* oid, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alg, LINK_FIRST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_IDENTIFIER, -1);
	}

	set_dom_node_text(nlk, oid, olen);
}

bool_t get_cert_signature_algorithm_parameters(link_t_ptr alg, tchar_t* pss, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(alg, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, pss, MAX_LONG);

	return 1;
}

void set_cert_signature_algorithm_parameters(link_t_ptr alg, const tchar_t* pss, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(alg, 0, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(alg, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_ALGORITHM_PARAMETERS, -1);
	}

	set_dom_node_text(nlk, pss, olen);
}

bool_t get_cert_signature(link_t_ptr ptr, tchar_t* sig, int* olen, bool_t add)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(ptr, 0, DOC_CERT_SIGNATURE, -1);
	if (!nlk && add)
	{
		nlk = insert_dom_node(ptr, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_SIGNATURE, -1);
	}
	if (!nlk) return 0;

	*olen = get_dom_node_text(nlk, sig, MAX_LONG);

	return 1;
}

void set_cert_signature(link_t_ptr ptr, const tchar_t* sig, int olen)
{
	link_t_ptr nlk;

	nlk = find_dom_node_by_name(ptr, 0, DOC_CERT_SIGNATURE, -1);
	if (!nlk)
	{
		nlk = insert_dom_node(ptr, LINK_LAST);
		set_dom_node_name(nlk, DOC_CERT_SIGNATURE, -1);
	}

	set_dom_node_text(nlk, sig, olen);
}

