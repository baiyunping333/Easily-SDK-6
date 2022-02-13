/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc x509 cert document

	@module	certoid.h | interface file

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

#ifndef _CERTOID_H
#define _CERTOID_H

#include "certdef.h"

#define OID_MAX_SIZE		512

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API bool_t x520_attr_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t x520_attr_short_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t x520_attr_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t x509_ext_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t x509_ext_type_from_oid(const byte_t* oid, dword_t olen, int* type);

	EXP_API bool_t x509_ext_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t ext_key_usage_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t ext_key_usage_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t sig_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t sig_alg_type_from_oid(const byte_t* oid, dword_t olen, md_type_t* md, pk_type_t* pk);

	EXP_API bool_t sig_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t pk_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t pk_alg_type_from_oid(const byte_t* oid, dword_t olen, pk_type_t* pk);

	EXP_API bool_t pk_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t ecp_grp_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t ecp_grp_type_from_oid(const byte_t* oid, dword_t olen, ecp_group_id* grp_id);

	EXP_API bool_t ecp_grp_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t cipher_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t cipher_alg_type_from_oid(const byte_t* oid, dword_t olen, int* cipher);

	EXP_API bool_t cipher_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t md_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t md_alg_type_from_oid(const byte_t* oid, dword_t olen, md_type_t* md);

	EXP_API bool_t md_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t md_hmac_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t md_hmac_type_from_oid(const byte_t* oid, dword_t olen, md_type_t* md);

	EXP_API bool_t md_hmac_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API bool_t pkcs12_pbe_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen);

	EXP_API bool_t pkcs12_pbe_alg_type_from_oid(const byte_t* oid, dword_t olen, int* md, int* cipher);

	EXP_API bool_t pkcs12_pbe_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen);

	EXP_API int oid_to_string(const byte_t* oid, dword_t len, tchar_t* buf, int max);

	EXP_API dword_t oid_from_string(byte_t* oid, dword_t max, const tchar_t* str, int len);

#ifdef	__cplusplus
}
#endif

#endif /*_CERTOID_H*/
