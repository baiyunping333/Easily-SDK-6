/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc x509 cert document

	@module	certctx.h | interface file

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

#ifndef _CERTCTX_H
#define _CERTCTX_H

#include "certdef.h"


typedef struct _cert_reader{
	bool_t(*read_cert_begin)(void* pa);

	bool_t(*read_tbs_begin)(void* pa, const byte_t* tbs_raw, dword_t raw_len);

	bool_t(*read_tbs_version)(void* pa, int ver);
	bool_t(*read_tbs_serial_number)(void* pa, const byte_t* num, dword_t len);

	bool_t(*read_tbs_signature_algorithm_begin)(void* pa);
	bool_t(*read_tbs_signature_algorithm_identifier)(void* pa, const byte_t* oid, dword_t len);
	bool_t(*read_tbs_signature_algorithm_parameters)(void* pa, const byte_t* pss, dword_t len);
	bool_t(*read_tbs_signature_algorithm_end)(void* pa);

	bool_t(*read_tbs_issuer_begin)(void* pa, const byte_t* issuer_raw, dword_t raw_len);
	bool_t(*read_tbs_issuer_name_begin)(void* pa);
	bool_t(*read_tbs_issuer_attribute_begin)(void* pa);
	bool_t(*read_tbs_issuer_attribute_type)(void* pa, const byte_t* oid, int alen);
	bool_t(*read_tbs_issuer_attribute_value)(void* pa, const byte_t* value, int vlen);
	bool_t(*read_tbs_issuer_attribute_end)(void* pa);
	bool_t(*read_tbs_issuer_name_end)(void* pa);
	bool_t(*read_tbs_issuer_end)(void* pa);

	bool_t(*read_tbs_validity_begin)(void* pa);
	bool_t(*read_tbs_validity_notbefore)(void* pa, const xdate_t* not_before);
	bool_t(*read_tbs_validity_notafter)(void* pa, const xdate_t* not_after);
	bool_t(*read_tbs_validity_end)(void* pa);

	bool_t(*read_tbs_subject_begin)(void* pa, const byte_t* subject_raw, dword_t raw_len);
	bool_t(*read_tbs_subject_name_begin)(void* pa);
	bool_t(*read_tbs_subject_attribute_begin)(void* pa);
	bool_t(*read_tbs_subject_attribute_type)(void* pa, const byte_t* oid, int alen);
	bool_t(*read_tbs_subject_attribute_value)(void* pa, const byte_t* value, int vlen);
	bool_t(*read_tbs_subject_attribute_end)(void* pa);
	bool_t(*read_tbs_subject_name_end)(void* pa);
	bool_t(*read_tbs_subject_end)(void* pa);

	bool_t(*read_tbs_subject_publickey_info_begin)(void* pa);
	bool_t(*read_tbs_subject_publickey_algorithm_begin)(void* pa);
	bool_t(*read_tbs_subject_publickey_algorithm_identifier)(void* pa, const byte_t* oid, dword_t len);
	bool_t(*read_tbs_subject_publickey_algorithm_parameters)(void* pa, const byte_t* oid, dword_t len);
	bool_t(*read_tbs_subject_publickey_algorithm_end)(void* pa);
	bool_t(*read_tbs_subject_publickey)(void* pa, const byte_t* key, dword_t len);
	bool_t(*read_tbs_subject_publickey_info_end)(void* pa);

	bool_t(*read_tbs_issuer_uuid)(void* pa, const byte_t* uuid, dword_t len);
	bool_t(*read_tbs_subject_uuid)(void* pa, const byte_t* uuid, dword_t len);

	bool_t(*read_tbs_extensions_begin)(void* pa);
	bool_t(*read_tbs_extensions_extn_begin)(void* pa);
	bool_t(*read_tbs_extensions_extn_id)(void* pa, const byte_t* id, dword_t len);
	bool_t(*read_tbs_extensions_critical)(void* pa, bool_t criti);
	bool_t(*read_tbs_extensions_extn_value)(void* pa, const byte_t* val, dword_t len);
	bool_t(*read_tbs_extensions_extn_end)(void* pa);
	bool_t(*read_tbs_extensions_end)(void* pa);

	bool_t(*read_tbs_end)(void* pa);

	bool_t(*read_cert_signature_algorithm_begin)(void* pa);
	bool_t(*read_cert_signature_algorithm_identifier)(void* pa, const byte_t* oid, dword_t len);
	bool_t(*read_cert_signature_algorithm_parameters)(void* pa, const byte_t* pss, dword_t len);
	bool_t(*read_cert_signature_algorithm_end)(void* pa);

	bool_t(*read_cert_signature)(void* pa, const byte_t* sig, dword_t len);

	bool_t(*read_cert_end)(void* pa);
}cert_reader;

#ifdef	__cplusplus
extern "C" {
#endif


EXP_API int cert_read_der(cert_reader *cr, void* pa, const byte_t *buf, dword_t size);


#ifdef	__cplusplus
}
#endif

#endif /*_X509_CRT_H*/
