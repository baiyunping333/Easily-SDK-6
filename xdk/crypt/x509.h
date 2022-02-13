/**
 * \file x509.h
 *
 * \brief X.509 generic defines and structures
 */
/*
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#ifndef X509_H
#define X509_H


#include "../xdkdef.h"
#include "rsa.h"
#include "ecp.h"
#include "asn1.h"
#include "mdwrap.h"
#include "pkwrap.h"

/**
* \addtogroup x509_module
* \{
*/

/**
* Maximum number of intermediate CAs in a verification chain.
* That is, maximum length of the chain, excluding the end-entity certificate
* and the trusted root certificate.
*
* Set this to a low value to prevent an adversary from making you waste
* resources verifying an overlong certificate chain.
*/
#define X509_MAX_INTERMEDIATE_CA   8

/**
* \name X509 Verify codes
* \{
*/
/* Reminder: update x509_crt_verify_strings[] in library/x509_crt.c */
#define X509_BADCERT_EXPIRED             0x01  /**< The certificate validity has expired. */
#define X509_BADCERT_REVOKED             0x02  /**< The certificate has been revoked (is on a CRL). */
#define X509_BADCERT_CN_MISMATCH         0x04  /**< The certificate Common Name (CN) does not match with the expected CN. */
#define X509_BADCERT_NOT_TRUSTED         0x08  /**< The certificate is not correctly signed by the trusted CA. */
#define X509_BADCRL_NOT_TRUSTED          0x10  /**< The CRL is not correctly signed by the trusted CA. */
#define X509_BADCRL_EXPIRED              0x20  /**< The CRL is expired. */
#define X509_BADCERT_MISSING             0x40  /**< Certificate was missing. */
#define X509_BADCERT_SKIP_VERIFY         0x80  /**< Certificate verification was skipped. */
#define X509_BADCERT_OTHER             0x0100  /**< Other reason (can be used by verify callback) */
#define X509_BADCERT_FUTURE            0x0200  /**< The certificate validity starts in the future. */
#define X509_BADCRL_FUTURE             0x0400  /**< The CRL is from the future */
#define X509_BADCERT_KEY_USAGE         0x0800  /**< Usage does not match the keyUsage extension. */
#define X509_BADCERT_EXT_KEY_USAGE     0x1000  /**< Usage does not match the extendedKeyUsage extension. */
#define X509_BADCERT_NS_CERT_TYPE      0x2000  /**< Usage does not match the nsCertType extension. */
#define X509_BADCERT_BAD_MD            0x4000  /**< The certificate is signed with an unacceptable hash. */
#define X509_BADCERT_BAD_PK            0x8000  /**< The certificate is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define X509_BADCERT_BAD_KEY         0x010000  /**< The certificate is signed with an unacceptable key (eg bad curve, RSA too short). */
#define X509_BADCRL_BAD_MD           0x020000  /**< The CRL is signed with an unacceptable hash. */
#define X509_BADCRL_BAD_PK           0x040000  /**< The CRL is signed with an unacceptable PK alg (eg RSA vs ECDSA). */
#define X509_BADCRL_BAD_KEY          0x080000  /**< The CRL is signed with an unacceptable key (eg bad curve, RSA too short). */

/*
* X.509 v3 Key Usage Extension flags
* Reminder: update x509_info_key_usage() when adding new flags.
*/
#define X509_KU_DIGITAL_SIGNATURE            (0x80)  /* bit 0 */
#define X509_KU_NON_REPUDIATION              (0x40)  /* bit 1 */
#define X509_KU_KEY_ENCIPHERMENT             (0x20)  /* bit 2 */
#define X509_KU_DATA_ENCIPHERMENT            (0x10)  /* bit 3 */
#define X509_KU_KEY_AGREEMENT                (0x08)  /* bit 4 */
#define X509_KU_KEY_CERT_SIGN                (0x04)  /* bit 5 */
#define X509_KU_CRL_SIGN                     (0x02)  /* bit 6 */
#define X509_KU_ENCIPHER_ONLY                (0x01)  /* bit 7 */
#define X509_KU_DECIPHER_ONLY              (0x8000)  /* bit 8 */

/*
* Netscape certificate types
* (http://www.mozilla.org/projects/security/pki/nss/tech-notes/tn3.html)
*/

#define X509_NS_CERT_TYPE_SSL_CLIENT         (0x80)  /* bit 0 */
#define X509_NS_CERT_TYPE_SSL_SERVER         (0x40)  /* bit 1 */
#define X509_NS_CERT_TYPE_EMAIL              (0x20)  /* bit 2 */
#define X509_NS_CERT_TYPE_OBJECT_SIGNING     (0x10)  /* bit 3 */
#define X509_NS_CERT_TYPE_RESERVED           (0x08)  /* bit 4 */
#define X509_NS_CERT_TYPE_SSL_CA             (0x04)  /* bit 5 */
#define X509_NS_CERT_TYPE_EMAIL_CA           (0x02)  /* bit 6 */
#define X509_NS_CERT_TYPE_OBJECT_SIGNING_CA  (0x01)  /* bit 7 */

/*
* X.509 extension types
*
* Comments refer to the status for using certificates. Status can be
* different for writing certificates or reading CRLs or CSRs.
*/
#define X509_EXT_AUTHORITY_KEY_IDENTIFIER    (1 << 0)
#define X509_EXT_SUBJECT_KEY_IDENTIFIER      (1 << 1)
#define X509_EXT_KEY_USAGE                   (1 << 2)
#define X509_EXT_CERTIFICATE_POLICIES        (1 << 3)
#define X509_EXT_POLICY_MAPPINGS             (1 << 4)
#define X509_EXT_SUBJECT_ALT_NAME            (1 << 5)    /* Supported (DNS) */
#define X509_EXT_ISSUER_ALT_NAME             (1 << 6)
#define X509_EXT_SUBJECT_DIRECTORY_ATTRS     (1 << 7)
#define X509_EXT_BASIC_CONSTRAINTS           (1 << 8)    /* Supported */
#define X509_EXT_NAME_CONSTRAINTS            (1 << 9)
#define X509_EXT_POLICY_CONSTRAINTS          (1 << 10)
#define X509_EXT_EXTENDED_KEY_USAGE          (1 << 11)
#define X509_EXT_CRL_DISTRIBUTION_POINTS     (1 << 12)
#define X509_EXT_INIHIBIT_ANYPOLICY          (1 << 13)
#define X509_EXT_FRESHEST_CRL                (1 << 14)

#define X509_EXT_NS_CERT_TYPE                (1 << 16)


/**
 * \name Structures for parsing X.509 certificates, CRLs and CSRs
 * \{
 */

/**
 * Type-length-value structure that allows for ASN1 using DER.
 */
typedef asn1_buf x509_buf;

/**
 * Container for ASN1 bit strings.
 */
typedef asn1_bitstring x509_bitstring;

/**
 * Container for ASN1 named information objects.
 * It allows for Relative Distinguished Names (e.g. cn=localhost,ou=code,etc.).
 */
typedef asn1_named_data x509_name;

/**
 * Container for a sequence of ASN.1 items
 */
typedef asn1_sequence x509_sequence;

/** Container for date and time (precision in seconds). */
typedef struct x509_time
{
    int year, mon, day;         /**< Date. */
    int hour, min, sec;         /**< Time. */
}
x509_time;


#define X509_SAFE_SNPRINTF                          \
    do {                                                    \
        if( ret < 0 || (dword_t) ret >= n )                  \
            return( C_ERR );    \
                                                            \
        n -= (dword_t) ret;                                  \
        p += (dword_t) ret;                                  \
    } while( 0 )

/*
* Storage format identifiers
* Recognized formats: PEM and DER
*/
#define X509_FORMAT_DER                 1
#define X509_FORMAT_PEM                 2

#define X509_MAX_DN_NAME_SIZE         256 /**< Maximum value size of a DN entry */




#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Store the certificate DN in printable form into buf;
 *                 no more than size characters will be written.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param dn       The X509 name to represent
 *
 * \return         The length of the string written (not including the
 *                 terminated nul byte), or a negative error code.
 */
EXP_API int x509_dn_gets(char *buf, dword_t size, const x509_name *dn);

/**
 * \brief          Store the certificate serial in printable form into buf;
 *                 no more than size characters will be written.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param serial   The X509 serial to represent
 *
 * \return         The length of the string written (not including the
 *                 terminated nul byte), or a negative error code.
 */
EXP_API int x509_serial_gets(char *buf, dword_t size, const x509_buf *serial);

/**
 * \brief          Check a given x509_time against the system time
 *                 and tell if it's in the past.
 *
 * \note           Intended usage is "if( is_past( valid_to ) ) ERROR".
 *                 Hence the return value of 1 if on internal errors.
 *
 * \param to       x509_time to check
 *
 * \return         1 if the given time is in the past or an error occurred,
 *                 0 otherwise.
 */
EXP_API int x509_time_is_past(const x509_time *to);

/**
 * \brief          Check a given x509_time against the system time
 *                 and tell if it's in the future.
 *
 * \note           Intended usage is "if( is_future( valid_from ) ) ERROR".
 *                 Hence the return value of 1 if on internal errors.
 *
 * \param from     x509_time to check
 *
 * \return         1 if the given time is in the future or an error occurred,
 *                 0 otherwise.
 */
EXP_API int x509_time_is_future(const x509_time *from);

/*
 * Internal module functions. You probably do not want to use these unless you
 * know you do.
 */
EXP_API int x509_get_name(byte_t **p, const byte_t *end,
                   x509_name *cur );
EXP_API int x509_get_alg_null(byte_t **p, const byte_t *end,
                       x509_buf *alg );
EXP_API int x509_get_alg(byte_t **p, const byte_t *end,
                  x509_buf *alg, x509_buf *params );
EXP_API int x509_get_rsassa_pss_params(const x509_buf *params,
                                md_type_t *md_alg, md_type_t *mgf_md,
                                int *salt_len );
EXP_API int x509_get_sig(byte_t **p, const byte_t *end, x509_buf *sig);
EXP_API int x509_get_sig_alg(const x509_buf *sig_oid, const x509_buf *sig_params,
                      md_type_t *md_alg, pk_type_t *pk_alg,
                      md_type_t* mgf_md, int *salt_len );
EXP_API int x509_get_time(byte_t **p, const byte_t *end,
                   x509_time *t );
EXP_API int x509_get_serial(byte_t **p, const byte_t *end,
                     x509_buf *serial );
EXP_API int x509_get_ext(byte_t **p, const byte_t *end,
                  x509_buf *ext, int tag );
EXP_API int x509_sig_alg_gets(char *buf, dword_t size, const x509_buf *sig_oid,
                       pk_type_t pk_alg, md_type_t md_alg,
					   md_type_t mgf_md, int salt_len );
EXP_API int x509_key_size_helper(char *buf, dword_t buf_size, const char *name);
EXP_API int x509_string_to_names(asn1_named_data **head, const char *name);
EXP_API int x509_set_extension(asn1_named_data **head, const char *oid, dword_t oid_len,
                        int critical, const byte_t *val,
                        dword_t val_len );
EXP_API int x509_write_extensions(byte_t **p, byte_t *start,
                           asn1_named_data *first );
EXP_API int x509_write_names(byte_t **p, byte_t *start,
                      asn1_named_data *first );
EXP_API int x509_write_sig(byte_t **p, byte_t *start,
                    const char *oid, dword_t oid_len,
                    byte_t *sig, dword_t size );
EXP_API int x509_write_pubkey(byte_t **p, byte_t *start,
	pk_type_t pktype, void *pk_ctx);
EXP_API int x509_write_pubkey_der(pk_type_t pktype, void *pk_ctx, byte_t *buf, dword_t size);
EXP_API int x509_parse_subpubkey(byte_t **p, const byte_t *end, 
	pk_type_t* pk_type, void** pk_ctx);
EXP_API int x509_get_basic_constraints(byte_t **p,const byte_t *end,
	int *ca_istrue,
	int *max_pathlen);
EXP_API int x509_get_key_usage(byte_t **p,
	const byte_t *end,
	unsigned int *key_usage);
EXP_API int x509_get_ext_key_usage(byte_t **p,
	const byte_t *end,
	x509_sequence *ext_key_usage);
EXP_API int x509_get_subject_alt_name(byte_t **p,
	const byte_t *end,
	x509_sequence *subject_alt_name);
EXP_API int x509_get_ns_cert_type(byte_t **p,
	const byte_t *end,
	byte_t *ns_cert_type);
EXP_API int x509_get_version(byte_t **p,
	const byte_t *end,
	int *ver);
EXP_API int x509_get_dates(byte_t **p,
	const byte_t *end,
	x509_time *from,
	x509_time *to);
EXP_API int x509_get_uid(byte_t **p,
	const byte_t *end,
	x509_buf *uid, int n);

#if defined(XDK_SUPPORT_TEST)
EXP_API int x509_self_test(int verbose);
#endif

#ifdef __cplusplus
}
#endif


#endif /* x509.h */

