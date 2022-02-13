/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc der document

	@module	der.h | interface file

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

#ifndef _DER_H
#define _DER_H

#include "xdkdef.h"

/**
* \name DER constants
* These constants comply with the DER encoded ASN.1 type tags.
* DER encoding uses hexadecimal representation.
* An example DER sequence is:\n
* - 0x02 -- tag indicating INTEGER
* - 0x01 -- length in octets
* - 0x05 -- value
* Such sequences are typically read into \c ::x509_buf.
* \{
*/
#define DER_BOOLEAN                 0x01
#define DER_INTEGER                 0x02
#define DER_BIT_STRING              0x03
#define DER_OCTET_STRING            0x04
#define DER_NULL                    0x05
#define DER_OID                     0x06
#define DER_UTF8_STRING             0x0C
#define DER_SEQUENCE                0x10
#define DER_SET                     0x11
#define DER_PRINTABLE_STRING        0x13
#define DER_T61_STRING              0x14
#define DER_IA5_STRING              0x16
#define DER_UTC_TIME                0x17
#define DER_GENERALIZED_TIME        0x18
#define DER_UNIVERSAL_STRING        0x1C
#define DER_BMP_STRING              0x1E
#define DER_PRIMITIVE               0x00
#define DER_CONSTRUCTED             0x20
#define DER_CONTEXT_SPECIFIC        0x80

/*
* Bit masks for each of the components of an ASN.1 tag as specified in
* ITU X.690 (08/2015), section 8.1 "General rules for encoding",
* paragraph 8.1.2.2:
*
* Bit  8     7   6   5          1
*     +-------+-----+------------+
*     | Class | P/C | Tag number |
*     +-------+-----+------------+
*/
#define DER_TAG_CLASS_MASK          0xC0
#define DER_TAG_PC_MASK             0x20
#define DER_TAG_VALUE_MASK          0x1F


#ifdef	__cplusplus
extern "C" {
#endif

EXP_API dword_t der_read_tag(const byte_t *buf, byte_t *ptag, dword_t* plen);

EXP_API dword_t der_write_tag(byte_t *buf, byte_t tag, dword_t len);

EXP_API dword_t der_read_bool(const byte_t *buf, bool_t *pval);

EXP_API dword_t der_write_bool(byte_t *buf, bool_t b);

EXP_API dword_t der_read_integer(const byte_t *buf, int *pval);

EXP_API dword_t der_write_integer(byte_t *buf, int val);

EXP_API dword_t der_read_bit_string(const byte_t *buf, byte_t** pstr, dword_t* plen, dword_t* pbit);

EXP_API dword_t der_write_bit_string(byte_t *buf, const byte_t* bstr, dword_t bits);

EXP_API dword_t der_read_octet_string(const byte_t *buf, byte_t** poct, dword_t* plen);

EXP_API dword_t der_write_octet_string(byte_t *buf, const byte_t* oct, dword_t len);

EXP_API dword_t der_read_null(const byte_t *buf);

EXP_API dword_t der_write_null(byte_t *buf);

EXP_API dword_t der_read_time(const byte_t *buf, xdate_t* pdt);

EXP_API dword_t der_write_time(byte_t *buf, const xdate_t* pdt);

EXP_API dword_t der_read_oid(const byte_t *buf, byte_t** poid, dword_t* plen);

EXP_API dword_t der_write_oid_string(byte_t *buf, const byte_t* oid, dword_t len);

EXP_API dword_t der_read_utf8_string(const byte_t *buf, byte_t** putf, dword_t* plen);

EXP_API dword_t der_write_utf8_string(byte_t *buf, const byte_t* utf, dword_t len);

EXP_API dword_t der_read_printable_string(const byte_t *buf, char** pstr, dword_t* plen);

EXP_API dword_t der_write_printable_string(byte_t *buf, const char* str, dword_t len);

EXP_API dword_t der_read_ia5_string(const byte_t *buf, char** pstr, dword_t* plen);

EXP_API dword_t der_write_ia5_string(byte_t *buf, const char* str, dword_t len);

EXP_API dword_t der_read_sequence_of(const byte_t *buf, dword_t* plen);

EXP_API dword_t der_write_sequence_of(byte_t *buf, dword_t len);


#if defined(XDK_SUPPORT_TEST)
EXP_API void test_der(void);
#endif

#ifdef	__cplusplus
}
#endif

#endif /*_DER_H*/
