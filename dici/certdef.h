/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, China ZheJiang HangZhou JianDe, Mail: powersuite@hotmaol.com

	@doc cert defination document

	@module	certdef.h | definition interface file

	@devnote 张文权 2021.01 - 2021.12 v6.0
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


#ifndef _CERTDEF_H
#define	_CERTDEF_H

#include <xdl.h>

/**
* Maximum number of intermediate CAs in a verification chain.
* That is, maximum length of the chain, excluding the end-entity certificate
* and the trusted root certificate.
*
* Set this to a low value to prevent an adversary from making you waste
* resources verifying an overlong certificate chain.
*/
#define CERT_MAX_INTERMEDIATE_CA   8

/**
* \name CERT Verify codes
* \{
*/
/* Reminder: update x509_crt_verify_strings[] in library/x509_crt.c */
#define CERT_BAD_EXPIRED             0x01  /**< The certificate validity has expired. */
#define CERT_BAD_REVOKED             0x02  /**< The certificate has been revoked (is on a CRL). */
#define CERT_BAD_CN_MISMATCH         0x04  /**< The certificate Common Name (CN) does not match with the expected CN. */
#define CERT_BAD_NOT_TRUSTED         0x08  /**< The certificate is not correctly signed by the trusted CA. */
#define CERT_BADCRL_NOT_TRUSTED          0x10  /**< The CRL is not correctly signed by the trusted CA. */
#define CERT_BADCRL_EXPIRED              0x20  /**< The CRL is expired. */
#define CERT_BAD_MISSING             0x40  /**< Certificate was missing. */
#define CERT_BAD_SKIP_VERIFY         0x80  /**< Certificate verification was skipped. */
#define CERT_BAD_OTHER             0x0100  /**< Other reason (can be used by verify callback) */
#define CERT_BAD_FUTURE            0x0200  /**< The certificate validity starts in the future. */
#define CERT_BADCRL_FUTURE             0x0400  /**< The CRL is from the future */
#define CERT_BAD_KEY_USAGE         0x0800  /**< Usage does not match the keyUsage extension. */
#define CERT_BAD_EXT_KEY_USAGE     0x1000  /**< Usage does not match the extendedKeyUsage extension. */
#define CERT_BAD_NS_CERT_TYPE      0x2000  /**< Usage does not match the nsCertType extension. */
#define CERT_BAD_BAD_MD            0x4000  /**< The certificate is signed with an unacceptable hash. */
#define CERT_BAD_BAD_PK            0x8000  /**< The certificate is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define CERT_BAD_BAD_KEY         0x010000  /**< The certificate is signed with an unacceptable key (eg bad curve, RSA too short). */
#define CERT_BADCRL_BAD_MD           0x020000  /**< The CRL is signed with an unacceptable hash. */
#define CERT_BADCRL_BAD_PK           0x040000  /**< The CRL is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define CERT_BADCRL_BAD_KEY          0x080000  /**< The CRL is signed with an unacceptable key (eg bad curve, RSA too short). */

/*
* X.509 v3 Key Usage Extension flags
* Reminder: update x509_info_key_usage() when adding new flags.
*/
#define CERT_KU_DIGITAL_SIGNATURE            (0x80)  /* bit 0 */
#define CERT_KU_NON_REPUDIATION              (0x40)  /* bit 1 */
#define CERT_KU_KEY_ENCIPHERMENT             (0x20)  /* bit 2 */
#define CERT_KU_DATA_ENCIPHERMENT            (0x10)  /* bit 3 */
#define CERT_KU_KEY_AGREEMENT                (0x08)  /* bit 4 */
#define CERT_KU_KEY_CERT_SIGN                (0x04)  /* bit 5 */
#define CERT_KU_CRL_SIGN                     (0x02)  /* bit 6 */
#define CERT_KU_ENCIPHER_ONLY                (0x01)  /* bit 7 */
#define CERT_KU_DECIPHER_ONLY              (0x8000)  /* bit 8 */

/*
* Netscape certificate types
* (http://www.mozilla.org/projects/security/pki/nss/tech-notes/tn3.html)
*/

#define CERT_NS_CERT_TYPE_SSL_CLIENT         (0x80)  /* bit 0 */
#define CERT_NS_CERT_TYPE_SSL_SERVER         (0x40)  /* bit 1 */
#define CERT_NS_CERT_TYPE_EMAIL              (0x20)  /* bit 2 */
#define CERT_NS_CERT_TYPE_OBJECT_SIGNING     (0x10)  /* bit 3 */
#define CERT_NS_CERT_TYPE_RESERVED           (0x08)  /* bit 4 */
#define CERT_NS_CERT_TYPE_SSL_CA             (0x04)  /* bit 5 */
#define CERT_NS_CERT_TYPE_EMAIL_CA           (0x02)  /* bit 6 */
#define CERT_NS_CERT_TYPE_OBJECT_SIGNING_CA  (0x01)  /* bit 7 */

/*
* X.509 extension types
*
* Comments refer to the status for using certificates. Status can be
* different for writing certificates or reading CRLs or CSRs.
*/
#define CERT_EXT_AUTHORITY_KEY_IDENTIFIER    (1 << 0)
#define CERT_EXT_SUBJECT_KEY_IDENTIFIER      (1 << 1)
#define CERT_EXT_KEY_USAGE                   (1 << 2)
#define CERT_EXT_CERTIFICATE_POLICIES        (1 << 3)
#define CERT_EXT_POLICY_MAPPINGS             (1 << 4)
#define CERT_EXT_SUBJECT_ALT_NAME            (1 << 5)    /* Supported (DNS) */
#define CERT_EXT_ISSUER_ALT_NAME             (1 << 6)
#define CERT_EXT_SUBJECT_DIRECTORY_ATTRS     (1 << 7)
#define CERT_EXT_BASIC_CONSTRAINTS           (1 << 8)    /* Supported */
#define CERT_EXT_NAME_CONSTRAINTS            (1 << 9)
#define CERT_EXT_POLICY_CONSTRAINTS          (1 << 10)
#define CERT_EXT_EXTENDED_KEY_USAGE          (1 << 11)
#define CERT_EXT_CRL_DISTRIBUTION_POINTS     (1 << 12)
#define CERT_EXT_INIHIBIT_ANYPOLICY          (1 << 13)
#define CERT_EXT_FRESHEST_CRL                (1 << 14)

#define CERT_EXT_NS_CERT_TYPE                (1 << 16)



#endif	/* _CERTDEF_H */

