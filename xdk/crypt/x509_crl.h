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

#ifndef X509_CRL_H
#define X509_CRL_H


#include "../xdkdef.h"
#include "x509.h"


/**
* Certificate revocation list entry.
* Contains the CA-specific serial numbers and revocation dates.
*/
typedef struct x509_crl_entry
{
	x509_buf raw;

	x509_buf serial;

	x509_time revocation_date;

	x509_buf entry_ext;

	struct x509_crl_entry *next;
}
x509_crl_entry;

/**
* Certificate revocation list structure.
* Every CRL may have multiple entries.
*/
typedef struct x509_crl
{
	x509_buf raw;           /**< The raw certificate data (DER). */
	x509_buf tbs;           /**< The raw certificate body (DER). The part that is To Be Signed. */

	int version;            /**< CRL version (1=v1, 2=v2) */
	x509_buf sig_oid;       /**< CRL signature type identifier */

	x509_buf issuer_raw;    /**< The raw issuer data (DER). */

	x509_name issuer;       /**< The parsed issuer data (named information object). */

	x509_time this_update;
	x509_time next_update;

	x509_crl_entry entry;   /**< The CRL entries containing the certificate revocation times for this CA. */

	x509_buf crl_ext;

	x509_buf sig_oid2;
	x509_buf sig;
	md_type_t sig_md;           /**< Internal representation of the MD algorithm of the signature algorithm, e.g. MD_SHA256 */
	pk_type_t sig_pk;           /**< Internal representation of the Public Key algorithm of the signature algorithm, e.g. PK_RSA */
	md_type_t sig_opt_mgf1_md;          /**< Signature options to be passed to pk_verify_ext(), e.g. for RSASSA-PSS */
	int sig_opt_sale_len;

	struct x509_crl *next;
}
x509_crl;


#ifdef __cplusplus
extern "C" {
#endif


/**
* \brief          Parse a DER-encoded CRL and append it to the chained list
*
* \param chain    points to the start of the chain
* \param buf      buffer holding the CRL data in DER format
* \param buflen   size of the buffer
*                 (including the terminating null byte for PEM data)
*
* \return         0 if successful, or a specific X509 or PEM error code
*/
EXP_API int x509_crl_parse_der(x509_crl *chain,
	const byte_t *buf, dword_t buflen);
/**
* \brief          Parse one or more CRLs and append them to the chained list
*
* \note           Multiple CRLs are accepted only if using PEM format
*
* \param chain    points to the start of the chain
* \param buf      buffer holding the CRL data in PEM or DER format
* \param buflen   size of the buffer
*                 (including the terminating null byte for PEM data)
*
* \return         0 if successful, or a specific X509 or PEM error code
*/
EXP_API int x509_crl_parse(x509_crl *chain, const byte_t *buf, dword_t buflen);

/**
* \brief          Returns an informational string about the CRL.
*
* \param buf      Buffer to write to
* \param size     Maximum size of buffer
* \param prefix   A line prefix
* \param crl      The X509 CRL to represent
*
* \return         The length of the string written (not including the
*                 terminated nul byte), or a negative error code.
*/
EXP_API int x509_crl_info(char *buf, dword_t size, const char *prefix,
	const x509_crl *crl);

/**
* \brief          Initialize a CRL (chain)
*
* \param crl      CRL chain to initialize
*/
EXP_API void x509_crl_init(x509_crl *crl);

/**
* \brief          Unallocate all CRL data
*
* \param crl      CRL chain to free
*/
EXP_API void x509_crl_free(x509_crl *crl);

/* \} name */
/* \} addtogroup x509_module */


#ifdef __cplusplus
}
#endif


#endif /* x509.h */

