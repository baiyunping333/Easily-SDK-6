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

#ifndef X509_CRT_H
#define X509_CRT_H


#include "../xdkdef.h"
#include "x509.h"
#include "x509_crl.h"
#include "pkwrap.h"
#include "mdwrap.h"
#include "rsa.h"


/**
* Container for an X.509 certificate. The certificate may be chained.
*/
typedef struct x509_crt
{
	x509_buf raw;               /**< The raw certificate data (DER). */
	x509_buf tbs;               /**< The raw certificate body (DER). The part that is To Be Signed. */

	int version;                /**< The X.509 version. (1=v1, 2=v2, 3=v3) */
	x509_buf serial;            /**< Unique id for certificate issued by a specific CA. */
	x509_buf sig_oid;           /**< Signature algorithm, e.g. sha1RSA */

	x509_buf issuer_raw;        /**< The raw issuer data (DER). Used for quick comparison. */
	x509_buf subject_raw;       /**< The raw subject data (DER). Used for quick comparison. */

	x509_name issuer;           /**< The parsed issuer data (named information object). */
	x509_name subject;          /**< The parsed subject data (named information object). */

	x509_time valid_from;       /**< Start time of certificate validity. */
	x509_time valid_to;         /**< End time of certificate validity. */

	pk_type_t pk_alg;              /**< Container for the public key context. */
	union{
		void* pk_ctx;
		rsa_context* rsa;
		ecp_keypair* ecp;
	};

	x509_buf issuer_id;         /**< Optional X.509 v2/v3 issuer unique identifier. */
	x509_buf subject_id;        /**< Optional X.509 v2/v3 subject unique identifier. */
	x509_buf v3_ext;            /**< Optional X.509 v3 extensions.  */
	x509_sequence subject_alt_names;    /**< Optional list of Subject Alternative Names (Only dNSName supported). */

	int ext_types;              /**< Bit string containing detected and parsed extensions */
	int ca_istrue;              /**< Optional Basic Constraint extension value: 1 if this certificate belongs to a CA, 0 otherwise. */
	int max_pathlen;            /**< Optional Basic Constraint extension value: The maximum path length to the root certificate. Path length is 1 higher than RFC 5280 'meaning', so 1+ */

	unsigned int key_usage;     /**< Optional key usage extension value: See the values in x509.h */

	x509_sequence ext_key_usage; /**< Optional list of extended key usage OIDs. */

	byte_t ns_cert_type; /**< Optional Netscape certificate type extension value: See the values in x509.h */

	x509_buf sig;               /**< Signature: hash of the tbs part signed with the private key. */
	md_type_t sig_md;           /**< Internal representation of the MD algorithm of the signature algorithm, e.g. MD_SHA256 */
	pk_type_t sig_pk;           /**< Internal representation of the Public Key algorithm of the signature algorithm, e.g. PK_RSA */
	//void *sig_opts;             /**< Signature options to be passed topk_verify_ext(), e.g. for RSASSA-PSS */
	md_type_t sig_opt_mgf1_md;
	int sig_opt_salt_len;

	struct x509_crt *next;     /**< Next certificate in the CA-chain. */
}x509_crt;

/**
* Build flag from an algorithm/curve identifier (pk, md, ecp)
* Since 0 is always XXX_NONE, ignore it.
*/
#define X509_ID_FLAG( id )   ( 1 << ( (id) - 1 ) )

/**
* Security profile for certificate verification.
*
* All lists are bitfields, built by ORing flags from X509_ID_FLAG().
*/
typedef struct x509_crt_profile
{
	dword_t allowed_mds;       /**< MDs for signatures         */
	dword_t allowed_pks;       /**< PK algs for signatures     */
	dword_t allowed_curves;    /**< Elliptic curves for ECDSA  */
	dword_t rsa_min_bitlen;    /**< Minimum size for RSA keys  */
}
x509_crt_profile;

#define X509_CRT_VERSION_1              0
#define X509_CRT_VERSION_2              1
#define X509_CRT_VERSION_3              2

#define X509_RFC5280_MAX_SERIAL_LEN 32
#define X509_RFC5280_UTC_TIME_LEN   15


/**
* Container for writing a certificate (CRT)
*/
typedef struct x509write_cert
{
	int version;
	mpi serial;
	pk_type_t subject_pk;
	void *subject_key;
	pk_type_t issuer_pk;
	void *issuer_key;
	asn1_named_data *subject;
	asn1_named_data *issuer;
	md_type_t md_alg;
	char not_before[X509_RFC5280_UTC_TIME_LEN + 1];
	char not_after[X509_RFC5280_UTC_TIME_LEN + 1];
	asn1_named_data *extensions;
}
x509write_cert;

/**
* Item in a verification chain: cert and flags for it
*/
typedef struct {
	x509_crt *crt;
	dword_t flags;
}x509_crt_verify_chain_item;

/**
* Max size of verification chain: end-entity + intermediates + trusted root
*/
#define X509_MAX_VERIFY_CHAIN_SIZE  ( X509_MAX_INTERMEDIATE_CA + 2 )

/**
* Verification chain as built by \ccrt_verify_chain()
*/
typedef struct
{
	x509_crt_verify_chain_item items[X509_MAX_VERIFY_CHAIN_SIZE];
	unsigned len;
}x509_crt_verify_chain;

/* Now we can declare functions that take a pointer to that */
typedef void x509_crt_restart_ctx;



#ifdef __cplusplus
extern "C" {
#endif


/**
 * Default security profile. Should provide a good balance between security
 * and compatibility with current deployments.
 */
extern const x509_crt_profile x509_crt_profile_default;

/**
 * Expected next default profile. Recommended for new deployments.
 * Currently targets a 128-bit security level, except for RSA-2048.
 */
extern const x509_crt_profile x509_crt_profile_next;

/**
 * NSA Suite B profile.
 */
extern const x509_crt_profile x509_crt_profile_suiteb;

/**
 * \brief          Parse a single DER formatted certificate and add it
 *                 to the chained list.
 *
 * \param chain    points to the start of the chain
 * \param buf      buffer holding the certificate DER data
 * \param buflen   size of the buffer
 *
 * \return         0 if successful, or a specific X509 or PEM error code
 */
EXP_API int x509_crt_parse_der(x509_crt *chain, const byte_t *buf,
                        dword_t buflen );

/**
 * \brief          Parse one DER-encoded or one or more concatenated PEM-encoded
 *                 certificates and add them to the chained list.
 *
 *                 For CRTs in PEM encoding, the function parses permissively:
 *                 if at least one certificate can be parsed, the function
 *                 returns the number of certificates for which parsing failed
 *                 (hence \c 0 if all certificates were parsed successfully).
 *                 If no certificate could be parsed, the function returns
 *                 the first (negative) error encountered during parsing.
 *
 *                 PEM encoded certificates may be interleaved by other data
 *                 such as human readable descriptions of their content, as
 *                 long as the certificates are enclosed in the PEM specific
 *                 '-----{BEGIN/END} CERTIFICATE-----' delimiters.
 *
 * \param chain    The chain to which to add the parsed certificates.
 * \param buf      The buffer holding the certificate data in PEM or DER format.
 *                 For certificates in PEM encoding, this may be a concatenation
 *                 of multiple certificates; for DER encoding, the buffer must
 *                 comprise exactly one certificate.
 * \param buflen   The size of \p buf, including the terminating \c NULL byte
 *                 in case of PEM encoded data.
 *
 * \return         \c 0 if all certificates were parsed successfully.
 * \return         The (positive) number of certificates that couldn't
 *                 be parsed if parsing was partly successful (see above).
 * \return         A negative X509 or PEM error code otherwise.
 *
 */
EXP_API int x509_crt_parse(x509_crt *chain, const byte_t *buf, dword_t buflen);

/**
 * \brief          Returns an informational string about the
 *                 certificate.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param prefix   A line prefix
 * \param crt      The X509 certificate to represent
 *
 * \return         The length of the string written (not including the
 *                 terminated nul byte), or a negative error code.
 */
EXP_API int x509_crt_info(char *buf, dword_t size, const char *prefix,
                   const x509_crt *crt );

/**
 * \brief          Returns an informational string about the
 *                 verification status of a certificate.
 *
 * \param buf      Buffer to write to
 * \param size     Maximum size of buffer
 * \param prefix   A line prefix
 * \param flags    Verification flags created byx509_crt_verify()
 *
 * \return         The length of the string written (not including the
 *                 terminated nul byte), or a negative error code.
 */
EXP_API int x509_crt_verify_info(char *buf, dword_t size, const char *prefix,
                          dword_t flags );

/**
 * \brief          Verify the certificate signature
 *
 *                 The verify callback is a user-supplied callback that
 *                 can clear / modify / add flags for a certificate. If set,
 *                 the verification callback is called for each
 *                 certificate in the chain (from the trust-ca down to the
 *                 presented crt). The parameters for the callback are:
 *                 (void *parameter,x509_crt *crt, int certificate_depth,
 *                 int *flags). With the flags representing current flags for
 *                 that specific certificate and the certificate depth from
 *                 the bottom (Peer cert depth = 0).
 *
 *                 All flags left after returning from the callback
 *                 are also returned to the application. The function should
 *                 return 0 for anything (including invalid certificates)
 *                 other than fatal error, as a non-zero return code
 *                 immediately aborts the verification process. For fatal
 *                 errors, a specific error code should be used (different
 *                 from ERR_X509_CERT_VERIFY_FAILED which should not
 *                 be returned at this point), or ERR_X509_FATAL_ERROR
 *                 can be used if no better code is available.
 *
 * \note           In case verification failed, the results can be displayed
 *                 using \cx509_crt_verify_info()
 *
 * \note           Same as \cx509_crt_verify_with_profile() with the
 *                 default security profile.
 *
 * \note           It is your responsibility to provide up-to-date CRLs for
 *                 all trusted CAs. If no CRL is provided for the CA that was
 *                 used to sign the certificate, CRL verification is skipped
 *                 silently, that is *without* setting any flag.
 *
 * \note           The \c trust_ca list can contain two types of certificates:
 *                 (1) those of trusted root CAs, so that certificates
 *                 chaining up to those CAs will be trusted, and (2)
 *                 self-signed end-entity certificates to be trusted (for
 *                 specific peers you know) - in that case, the self-signed
 *                 certificate doesn't need to have the CA bit set.
 *
 * \param crt      a certificate (chain) to be verified
 * \param trust_ca the list of trusted CAs (see note above)
 * \param ca_crl   the list of CRLs for trusted CAs (see note above)
 * \param cn       expected Common Name (can be set to
 *                 NULL if the CN must not be verified)
 * \param flags    result of the verification
 * \param f_vrfy   verification function
 * \param p_vrfy   verification parameter
 *
 * \return         0 (and flags set to 0) if the chain was verified and valid,
 *                 ERR_X509_CERT_VERIFY_FAILED if the chain was verified
 *                 but found to be invalid, in which case *flags will have one
 *                 or more X509_BADCERT_XXX or X509_BADCRL_XXX
 *                 flags set, or another error (and flags set to 0xffffffff)
 *                 in case of a fatal error encountered during the
 *                 verification process.
 */
EXP_API int x509_crt_verify(x509_crt *crt,
                    x509_crt *trust_ca,
                    x509_crl *ca_crl,
                     const char *cn, dword_t *flags,
                     int (*f_vrfy)(void *,x509_crt *, int, dword_t *),
                     void *p_vrfy );

/**
 * \brief          Verify the certificate signature according to profile
 *
 * \note           Same as \cx509_crt_verify(), but with explicit
 *                 security profile.
 *
 * \note           The restrictions on keys (RSA minimum size, allowed curves
 *                 for ECDSA) apply to all certificates: trusted root,
 *                 intermediate CAs if any, and end entity certificate.
 *
 * \param crt      a certificate (chain) to be verified
 * \param trust_ca the list of trusted CAs
 * \param ca_crl   the list of CRLs for trusted CAs
 * \param profile  security profile for verification
 * \param cn       expected Common Name (can be set to
 *                 NULL if the CN must not be verified)
 * \param flags    result of the verification
 * \param f_vrfy   verification function
 * \param p_vrfy   verification parameter
 *
 * \return         0 if successful or ERR_X509_CERT_VERIFY_FAILED
 *                 in which case *flags will have one or more
 *                 X509_BADCERT_XXX or X509_BADCRL_XXX flags
 *                 set,
 *                 or another error in case of a fatal error encountered
 *                 during the verification process.
 */
EXP_API int x509_crt_verify_with_profile(x509_crt *crt,
                    x509_crt *trust_ca,
                    x509_crl *ca_crl,
                     const x509_crt_profile *profile,
                     const char *cn, dword_t *flags,
                     int (*f_vrfy)(void *,x509_crt *, int, dword_t *),
                     void *p_vrfy );

/**
 * \brief          Restartable version of \ccrt_verify_with_profile()
 *
 * \note           Performs the same job as \ccrt_verify_with_profile()
 *                 but can return early and restart according to the limit
 *                 set with \cecp_set_max_ops() to reduce blocking.
 *
 * \param crt      a certificate (chain) to be verified
 * \param trust_ca the list of trusted CAs
 * \param ca_crl   the list of CRLs for trusted CAs
 * \param profile  security profile for verification
 * \param cn       expected Common Name (can be set to
 *                 NULL if the CN must not be verified)
 * \param flags    result of the verification
 * \param f_vrfy   verification function
 * \param p_vrfy   verification parameter
 * \param rs_ctx   restart context (NULL to disable restart)
 *
 * \return         See \ccrt_verify_with_profile(), or
 * \return         #ERR_ECP_IN_PROGRESS if maximum number of
 *                 operations was reached: see \cecp_set_max_ops().
 */
EXP_API int x509_crt_verify_restartable(x509_crt *crt,
                    x509_crt *trust_ca,
                    x509_crl *ca_crl,
                     const x509_crt_profile *profile,
                     const char *cn, dword_t *flags,
                     int (*f_vrfy)(void *,x509_crt *, int, dword_t *),
                     void *p_vrfy,
                    x509_crt_restart_ctx *rs_ctx );

/**
 * \brief          Check usage of certificate against keyUsage extension.
 *
 * \param crt      Leaf certificate used.
 * \param usage    Intended usage(s) (eg X509_KU_KEY_ENCIPHERMENT
 *                 before using the certificate to perform an RSA key
 *                 exchange).
 *
 * \note           Except for decipherOnly and encipherOnly, a bit set in the
 *                 usage argument means this bit MUST be set in the
 *                 certificate. For decipherOnly and encipherOnly, it means
 *                 that bit MAY be set.
 *
 * \return         0 is these uses of the certificate are allowed,
 *                 ERR_X509_BAD_INPUT_DATA if the keyUsage extension
 *                 is present but does not match the usage argument.
 *
 * \note           You should only call this function on leaf certificates, on
 *                 (intermediate) CAs the keyUsage extension is automatically
 *                 checked by \cx509_crt_verify().
 */
EXP_API int x509_crt_check_key_usage(const x509_crt *crt,
                                      unsigned int usage );

/**
 * \brief           Check usage of certificate against extendedKeyUsage.
 *
 * \param crt       Leaf certificate used.
 * \param usage_oid Intended usage (eg OID_SERVER_AUTH or
 *                  OID_CLIENT_AUTH).
 * \param usage_len Length of usage_oid (eg given by OID_SIZE()).
 *
 * \return          0 if this use of the certificate is allowed,
 *                  ERR_X509_BAD_INPUT_DATA if not.
 *
 * \note            Usually only makes sense on leaf certificates.
 */
EXP_API int x509_crt_check_extended_key_usage(const x509_crt *crt,
                                               const char *usage_oid,
                                               dword_t usage_len );

/**
 * \brief          Verify the certificate revocation status
 *
 * \param crt      a certificate to be verified
 * \param crl      the CRL to verify against
 *
 * \return         1 if the certificate is revoked, 0 otherwise
 *
 */
EXP_API int x509_crt_is_revoked(const x509_crt *crt, const x509_crl *crl);

/**
 * \brief          Initialize a certificate (chain)
 *
 * \param crt      Certificate chain to initialize
 */
EXP_API void x509_crt_init(x509_crt *crt);

/**
 * \brief          Unallocate all certificate data
 *
 * \param crt      Certificate chain to free
 */
EXP_API void x509_crt_free(x509_crt *crt);


/* \} name */
/* \} addtogroup x509_module */

/**
 * \brief           Initialize a CRT writing context
 *
 * \param ctx       CRT context to initialize
 */
EXP_API void x509write_crt_init(x509write_cert *ctx);

/**
 * \brief           Set the verion for a Certificate
 *                  Default: X509_CRT_VERSION_3
 *
 * \param ctx       CRT context to use
 * \param version   version to set (X509_CRT_VERSION_1, X509_CRT_VERSION_2 or
 *                                  X509_CRT_VERSION_3)
 */
EXP_API void x509write_crt_set_version(x509write_cert *ctx, int version);

/**
 * \brief           Set the serial number for a Certificate.
 *
 * \param ctx       CRT context to use
 * \param serial    serial number to set
 *
 * \return          0 if successful
 */
EXP_API int x509write_crt_set_serial(x509write_cert *ctx, const mpi *serial);

/**
 * \brief           Set the validity period for a Certificate
 *                  Timestamps should be in string format for UTC timezone
 *                  i.e. "YYYYMMDDhhmmss"
 *                  e.g. "20131231235959" for December 31st 2013
 *                       at 23:59:59
 *
 * \param ctx       CRT context to use
 * \param not_before    not_before timestamp
 * \param not_after     not_after timestamp
 *
 * \return          0 if timestamp was parsed successfully, or
 *                  a specific error code
 */
EXP_API int x509write_crt_set_validity(x509write_cert *ctx, const char *not_before,
                                const char *not_after );

/**
 * \brief           Set the issuer name for a Certificate
 *                  Issuer names should contain a comma-separated list
 *                  of OID types and values:
 *                  e.g. "C=UK,O=ARM,CN=mbed TLS CA"
 *
 * \param ctx           CRT context to use
 * \param issuer_name   issuer name to set
 *
 * \return          0 if issuer name was parsed successfully, or
 *                  a specific error code
 */
EXP_API int x509write_crt_set_issuer_name(x509write_cert *ctx,
                                   const char *issuer_name );

/**
 * \brief           Set the subject name for a Certificate
 *                  Subject names should contain a comma-separated list
 *                  of OID types and values:
 *                  e.g. "C=UK,O=ARM,CN=mbed TLS Server 1"
 *
 * \param ctx           CRT context to use
 * \param subject_name  subject name to set
 *
 * \return          0 if subject name was parsed successfully, or
 *                  a specific error code
 */
EXP_API int x509write_crt_set_subject_name(x509write_cert *ctx,
                                    const char *subject_name );

/**
 * \brief           Set the subject public key for the certificate
 *
 * \param ctx       CRT context to use
 * \param key       public key to include
 */
EXP_API void x509write_crt_set_subject_key(x509write_cert *ctx, pk_type_t pktype, void *pk_ctx);

/**
 * \brief           Set the issuer key used for signing the certificate
 *
 * \param ctx       CRT context to use
 * \param key       private key to sign with
 */
EXP_API void x509write_crt_set_issuer_key(x509write_cert *ctx, pk_type_t pktype, void *pk_ctx);

/**
 * \brief           Set the MD algorithm to use for the signature
 *                  (e.g. MD_SHA1)
 *
 * \param ctx       CRT context to use
 * \param md_alg    MD algorithm to use
 */
EXP_API void x509write_crt_set_md_alg(x509write_cert *ctx, md_type_t md_alg);

/**
 * \brief           Generic function to add to or replace an extension in the
 *                  CRT
 *
 * \param ctx       CRT context to use
 * \param oid       OID of the extension
 * \param oid_len   length of the OID
 * \param critical  if the extension is critical (per the RFC's definition)
 * \param val       value of the extension OCTET STRING
 * \param val_len   length of the value data
 *
 * \return          0 if successful, or a ERR_X509_ALLOC_FAILED
 */
EXP_API int x509write_crt_set_extension(x509write_cert *ctx,
                                 const char *oid, dword_t oid_len,
                                 int critical,
                                 const byte_t *val, dword_t val_len );

/**
 * \brief           Set the basicConstraints extension for a CRT
 *
 * \param ctx       CRT context to use
 * \param is_ca     is this a CA certificate
 * \param max_pathlen   maximum length of certificate chains below this
 *                      certificate (only for CA certificates, -1 is
 *                      inlimited)
 *
 * \return          0 if successful, or a ERR_X509_ALLOC_FAILED
 */
EXP_API int x509write_crt_set_basic_constraints(x509write_cert *ctx,
                                         int is_ca, int max_pathlen );

/**
 * \brief           Set the subjectKeyIdentifier extension for a CRT
 *                  Requires thatx509write_crt_set_subject_key() has been
 *                  called before
 *
 * \param ctx       CRT context to use
 *
 * \return          0 if successful, or a ERR_X509_ALLOC_FAILED
 */
EXP_API int x509write_crt_set_subject_key_identifier(x509write_cert *ctx);

/**
 * \brief           Set the authorityKeyIdentifier extension for a CRT
 *                  Requires thatx509write_crt_set_issuer_key() has been
 *                  called before
 *
 * \param ctx       CRT context to use
 *
 * \return          0 if successful, or a ERR_X509_ALLOC_FAILED
 */
EXP_API int x509write_crt_set_authority_key_identifier(x509write_cert *ctx);

/**
 * \brief           Set the Key Usage Extension flags
 *                  (e.g. X509_KU_DIGITAL_SIGNATURE | X509_KU_KEY_CERT_SIGN)
 *
 * \param ctx       CRT context to use
 * \param key_usage key usage flags to set
 *
 * \return          0 if successful, or ERR_X509_ALLOC_FAILED
 */
EXP_API int x509write_crt_set_key_usage(x509write_cert *ctx,
                                         unsigned int key_usage );

/**
 * \brief           Set the Netscape Cert Type flags
 *                  (e.g. X509_NS_CERT_TYPE_SSL_CLIENT | X509_NS_CERT_TYPE_EMAIL)
 *
 * \param ctx           CRT context to use
 * \param ns_cert_type  Netscape Cert Type flags to set
 *
 * \return          0 if successful, or ERR_X509_ALLOC_FAILED
 */
EXP_API int x509write_crt_set_ns_cert_type(x509write_cert *ctx,
                                    byte_t ns_cert_type );

/**
 * \brief           Free the contents of a CRT write context
 *
 * \param ctx       CRT context to free
 */
EXP_API void x509write_crt_free(x509write_cert *ctx);

/**
 * \brief           Write a built up certificate to a X509 DER structure
 *                  Note: data is written at the end of the buffer! Use the
 *                        return value to determine where you should start
 *                        using the buffer
 *
 * \param ctx       certificate to write away
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
EXP_API int x509write_crt_der(x509write_cert *ctx, byte_t *buf, dword_t size,
                       int (*f_rng)(void *, byte_t *, dword_t),
                       void *p_rng );

/**
 * \brief           Write a built up certificate to a X509 PEM string
 *
 * \param ctx       certificate to write away
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
EXP_API int x509write_crt_pem(x509write_cert *ctx, byte_t *buf, dword_t size,
                       int (*f_rng)(void *, byte_t *, dword_t),
                       void *p_rng );


#ifdef __cplusplus
}
#endif


#endif /* x509.h */

