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

#ifndef X509_CSR_H
#define X509_CSR_H


#include "../xdkdef.h"
#include "x509.h"


/**
* Certificate Signing Request (CSR) structure.
*/
typedef struct x509_csr
{
	x509_buf raw;           /**< The raw CSR data (DER). */
	x509_buf cri;           /**< The raw CertificateRequestInfo body (DER). */

	int version;            /**< CSR version (1=v1). */

	x509_buf  subject_raw;  /**< The raw subject data (DER). */
	x509_name subject;      /**< The parsed subject data (named information object). */

	pk_type_t pk_alg;          /**< Container for the public key context. */
	void* pk_ctx;

	x509_buf sig_oid;
	x509_buf sig;
	md_type_t sig_md;       /**< Internal representation of the MD algorithm of the signature algorithm, e.g. MD_SHA256 */
	pk_type_t sig_pk;       /**< Internal representation of the Public Key algorithm of the signature algorithm, e.g. PK_RSA */
	md_type_t sig_opt_mgf1_md;          /**< Signature options to be passed to pk_verify_ext(), e.g. for RSASSA-PSS */
	int sig_opt_sale_len;
}
x509_csr;

/**
* Container for writing a CSR
*/
typedef struct x509write_csr
{
	pk_type_t pk_alg;
	void* pk_ctx;
	asn1_named_data *subject;
	md_type_t md_alg;
	asn1_named_data *extensions;
}
x509write_csr;

#ifdef __cplusplus
extern "C" {
#endif


/**
* \brief          Load a Certificate Signing Request (CSR) in DER format
*
* \note           CSR attributes (if any) are currently silently ignored.
*
* \param csr      CSR context to fill
* \param buf      buffer holding the CRL data
* \param buflen   size of the buffer
*
* \return         0 if successful, or a specific X509 error code
*/
EXP_API int x509_csr_parse_der(x509_csr *csr,
	const byte_t *buf, dword_t buflen);

/**
* \brief          Load a Certificate Signing Request (CSR), DER or PEM format
*
* \note           See notes for \c x509_csr_parse_der()
*
* \param csr      CSR context to fill
* \param buf      buffer holding the CRL data
* \param buflen   size of the buffer
*                 (including the terminating null byte for PEM data)
*
* \return         0 if successful, or a specific X509 or PEM error code
*/
EXP_API int x509_csr_parse(x509_csr *csr, const byte_t *buf, dword_t buflen);

/**
* \brief          Returns an informational string about the
*                 CSR.
*
* \param buf      Buffer to write to
* \param size     Maximum size of buffer
* \param prefix   A line prefix
* \param csr      The X509 CSR to represent
*
* \return         The length of the string written (not including the
*                 terminated nul byte), or a negative error code.
*/
EXP_API int x509_csr_info(char *buf, dword_t size, const char *prefix,
	const x509_csr *csr);

/**
* \brief          Initialize a CSR
*
* \param csr      CSR to initialize
*/
EXP_API void x509_csr_init(x509_csr *csr);

/**
* \brief          Unallocate all CSR data
*
* \param csr      CSR to free
*/
EXP_API void x509_csr_free(x509_csr *csr);

/* \} name */
/* \} addtogroup x509_module */

/**
* \brief           Initialize a CSR context
*
* \param ctx       CSR context to initialize
*/
EXP_API void x509write_csr_init(x509write_csr *ctx);

/**
* \brief           Set the subject name for a CSR
*                  Subject names should contain a comma-separated list
*                  of OID types and values:
*                  e.g. "C=UK,O=ARM,CN=mbed TLS Server 1"
*
* \param ctx           CSR context to use
* \param subject_name  subject name to set
*
* \return          0 if subject name was parsed successfully, or
*                  a specific error code
*/
EXP_API int x509write_csr_set_subject_name(x509write_csr *ctx,
	const char *subject_name);

/**
* \brief           Set the key for a CSR (public key will be included,
*                  private key used to sign the CSR when writing it)
*
* \param ctx       CSR context to use
* \param key       Asymetric key to include
*/
EXP_API void x509write_csr_set_key(x509write_csr *ctx, pk_type_t pktype, void *pk_ctx);

/**
* \brief           Set the MD algorithm to use for the signature
*                  (e.g. MD_SHA1)
*
* \param ctx       CSR context to use
* \param md_alg    MD algorithm to use
*/
EXP_API void x509write_csr_set_md_alg(x509write_csr *ctx, md_type_t md_alg);

/**
* \brief           Set the Key Usage Extension flags
*                  (e.g. X509_KU_DIGITAL_SIGNATURE | X509_KU_KEY_CERT_SIGN)
*
* \param ctx       CSR context to use
* \param key_usage key usage flags to set
*
* \return          0 if successful, or ERR_X509_ALLOC_FAILED
*
* \note            The <code>decipherOnly</code> flag from the Key Usage
*                  extension is represented by bit 8 (i.e.
*                  <code>0x8000</code>), which cannot typically be represented
*                  in an byte_t. Therefore, the flag
*                  <code>decipherOnly</code> (i.e.
*                  #X509_KU_DECIPHER_ONLY) cannot be set using this
*                  function.
*/
EXP_API int x509write_csr_set_key_usage(x509write_csr *ctx, byte_t key_usage);

/**
* \brief           Set the Netscape Cert Type flags
*                  (e.g. X509_NS_CERT_TYPE_SSL_CLIENT | X509_NS_CERT_TYPE_EMAIL)
*
* \param ctx           CSR context to use
* \param ns_cert_type  Netscape Cert Type flags to set
*
* \return          0 if successful, or ERR_X509_ALLOC_FAILED
*/
EXP_API int x509write_csr_set_ns_cert_type(x509write_csr *ctx,
	byte_t ns_cert_type);

/**
* \brief           Generic function to add to or replace an extension in the
*                  CSR
*
* \param ctx       CSR context to use
* \param oid       OID of the extension
* \param oid_len   length of the OID
* \param val       value of the extension OCTET STRING
* \param val_len   length of the value data
*
* \return          0 if successful, or a ERR_X509_ALLOC_FAILED
*/
EXP_API int x509write_csr_set_extension(x509write_csr *ctx,
	const char *oid, dword_t oid_len,
	const byte_t *val, dword_t val_len);

/**
* \brief           Free the contents of a CSR context
*
* \param ctx       CSR context to free
*/
EXP_API void x509write_csr_free(x509write_csr *ctx);

/**
* \brief           Write a CSR (Certificate Signing Request) to a
*                  DER structure
*                  Note: data is written at the end of the buffer! Use the
*                        return value to determine where you should start
*                        using the buffer
*
* \param ctx       CSR to write away
* \param buf       buffer to write to
* \param size      size of the buffer
* \param f_rng     RNG function (for signature, see note)
* \param p_rng     RNG parameter
*
* \return          length of data written if successful, or a specific
*                  error code
*
* \note            f_rng may be NULL if RSA is used for signature and the
*                  signature is made offline (otherwise f_rng is desirable
*                  for countermeasures against timing attacks).
*                  ECDSA signatures always require a non-NULL f_rng.
*/
EXP_API int x509write_csr_der(x509write_csr *ctx, byte_t *buf, dword_t size,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng);

/**
* \brief           Write a CSR (Certificate Signing Request) to a
*                  PEM string
*
* \param ctx       CSR to write away
* \param buf       buffer to write to
* \param size      size of the buffer
* \param f_rng     RNG function (for signature, see note)
* \param p_rng     RNG parameter
*
* \return          0 if successful, or a specific error code
*
* \note            f_rng may be NULL if RSA is used for signature and the
*                  signature is made offline (otherwise f_rng is desirable
*                  for countermeasures against timing attacks).
*                  ECDSA signatures always require a non-NULL f_rng.
*/
EXP_API int x509write_csr_pem(x509write_csr *ctx, byte_t *buf, dword_t size,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng);

#ifdef __cplusplus
}
#endif


#endif /* x509.h */

