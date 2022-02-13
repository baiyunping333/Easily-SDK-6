/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc cert document

	@module	cert.h | interface file

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

#ifndef _CERTDOC_H
#define _CERTDOC_H

#include "certdef.h"
#include "certattr.h"


#ifdef	__cplusplus
extern "C" {
#endif

EXP_API link_t_ptr create_cert_doc(void);

EXP_API void destroy_cert_doc(link_t_ptr ptr);

EXP_API void clear_cert_doc(link_t_ptr ptr);

EXP_API bool_t is_cert_doc(link_t_ptr ptr);

EXP_API link_t_ptr get_cert_tbs_certificate(link_t_ptr ptr, bool_t add);

EXP_API int get_tbs_version(link_t_ptr tbs, bool_t add);

EXP_API void set_tbs_version(link_t_ptr tbs, int ver);

EXP_API int get_tbs_serial_number(link_t_ptr tbs, tchar_t* sn, int max, bool_t add);

EXP_API void set_tbs_serial_number(link_t_ptr tbs, const tchar_t* sn, int n);

EXP_API link_t_ptr get_tbs_signature_algorithm(link_t_ptr tbs, bool_t add);

EXP_API bool_t get_tbs_signature_algorithm_identifier(link_t_ptr alg, tchar_t* oid, int* olen, bool_t add);

EXP_API void set_tbs_signature_algorithm_identifier(link_t_ptr alg, const tchar_t* oid, int olen);

EXP_API bool_t get_tbs_signature_algorithm_parameters(link_t_ptr alg, tchar_t* pss, int* olen, bool_t add);

EXP_API void set_tbs_signature_algorithm_parameters(link_t_ptr alg, const tchar_t* pss, int olen);

EXP_API link_t_ptr get_tbs_issuer(link_t_ptr tbs, bool_t add);

EXP_API link_t_ptr get_tbs_next_issuer_name(link_t_ptr iss, link_t_ptr rdn, bool_t add);

EXP_API link_t_ptr get_tbs_next_issuer_name_attribute(link_t_ptr rdn, link_t_ptr alk, bool_t add);

EXP_API bool_t get_tbs_issuer_name_attribute_type(link_t_ptr alk, tchar_t* type, int* tlen, bool_t add);

EXP_API void set_tbs_issuer_name_attribute_type(link_t_ptr alk, const tchar_t* type, int tlen);

EXP_API bool_t get_tbs_issuer_name_attribute_value(link_t_ptr alk, tchar_t* value, int* vlen, bool_t add);

EXP_API void set_tbs_issuer_name_attribute_value(link_t_ptr alk, const tchar_t* value, int vlen);

EXP_API link_t_ptr get_tbs_validity(link_t_ptr tbs, bool_t add);

EXP_API bool_t get_tbs_validity_notbefore(link_t_ptr vlk, xdate_t* not_before, bool_t add);

EXP_API void set_tbs_validity_notbefore(link_t_ptr vlk, const xdate_t* not_before);

EXP_API bool_t get_tbs_validity_notafter(link_t_ptr vlk, xdate_t* not_after, bool_t add);

EXP_API void set_tbs_validity_notafter(link_t_ptr vlk, const xdate_t* not_after);

EXP_API link_t_ptr get_tbs_subject(link_t_ptr tbs, bool_t add);

EXP_API link_t_ptr get_tbs_next_subject_name(link_t_ptr sub, link_t_ptr rdn, bool_t add);

EXP_API link_t_ptr get_tbs_next_subject_name_attribute(link_t_ptr rdn, link_t_ptr alk, bool_t add);

EXP_API bool_t get_tbs_subject_name_attribute_type(link_t_ptr alk, tchar_t* type, int* tlen, bool_t add);

EXP_API void set_tbs_subject_name_attribute_type(link_t_ptr alk, const tchar_t* type, int tlen);

EXP_API bool_t get_tbs_subject_name_attribute_value(link_t_ptr alk, tchar_t* value, int* vlen, bool_t add);

EXP_API void set_tbs_subject_name_attribute_value(link_t_ptr alk, const tchar_t* value, int vlen);

EXP_API link_t_ptr get_tbs_subject_publickey_info(link_t_ptr tbs, bool_t add);

EXP_API link_t_ptr get_tbs_subject_publickey_algorithm(link_t_ptr pki, bool_t add);

EXP_API bool_t get_tbs_subject_publickey_algorithm_identifier(link_t_ptr alg, tchar_t* oid, int* olen, bool_t add);

EXP_API void set_tbs_subject_publickey_algorithm_identifier(link_t_ptr alg, const tchar_t* oid, int olen);

EXP_API bool_t get_tbs_subject_publickey_algorithm_parameters(link_t_ptr alg, tchar_t* curve, int* olen, bool_t add);

EXP_API void set_tbs_subject_publickey_algorithm_parameters(link_t_ptr alg, const tchar_t* curve, int olen);

EXP_API bool_t get_tbs_subject_publickey(link_t_ptr pki, tchar_t* key, int* olen, bool_t add);

EXP_API void set_tbs_subject_publickey(link_t_ptr pki, const tchar_t* key, int olen);

EXP_API bool_t get_tbs_issuer_uniqueID(link_t_ptr tbs, tchar_t* id, int* olen, bool_t add);

EXP_API void set_tbs_issuer_uniqueID(link_t_ptr tbs, const tchar_t* id, int olen);

EXP_API bool_t get_tbs_subject_uniqueID(link_t_ptr tbs, tchar_t* id, int* olen, bool_t add);

EXP_API void set_tbs_subject_uniqueID(link_t_ptr tbs, const tchar_t* id, int olen);

EXP_API link_t_ptr get_tbs_extensions(link_t_ptr tbs, bool_t add);

EXP_API link_t_ptr get_tbs_next_extensions_extn(link_t_ptr exts, link_t_ptr elk, bool_t add);

EXP_API bool_t get_tbs_extensions_extn_id(link_t_ptr elk, tchar_t* id, int* olen, bool_t add);

EXP_API void set_tbs_extensions_extn_id(link_t_ptr elk, const tchar_t* id, int len);

EXP_API bool_t get_tbs_extensions_extn_critical(link_t_ptr elk, bool_t* criti, bool_t add);

EXP_API void set_tbs_extensions_extn_critical(link_t_ptr elk, bool_t criti);

EXP_API bool_t get_tbs_extensions_extn_value(link_t_ptr elk, tchar_t* val, int* olen, bool_t add);

EXP_API void set_tbs_extensions_extn_value(link_t_ptr elk, const tchar_t* val, int len);

EXP_API link_t_ptr get_cert_signature_algorithm(link_t_ptr ptr, bool_t add);

EXP_API bool_t get_cert_signature_algorithm_identifier(link_t_ptr alg, tchar_t* oid, int* olen, bool_t add);

EXP_API void set_cert_signature_algorithm_identifier(link_t_ptr alg, const tchar_t* oid, int olen);

EXP_API bool_t get_cert_signature_algorithm_parameters(link_t_ptr alg, tchar_t* pss, int* olen, bool_t add);

EXP_API void set_cert_signature_algorithm_parameters(link_t_ptr alg, const tchar_t* pss, int olen);

EXP_API bool_t get_cert_signature(link_t_ptr ptr, tchar_t* sig, int* olen, bool_t add);

EXP_API void set_cert_signature(link_t_ptr ptr, const tchar_t* sig, int olen);

#if defined(XDK_SUPPORT_TEST)
EXP_API void test_x509(void);
#endif

#ifdef	__cplusplus
}
#endif

#endif /*_X509_H*/
