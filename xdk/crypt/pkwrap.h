/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc pk document

	@module	pk.h | interface file

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
#ifndef PKWRAP_H
#define PKWRAP_H

#include "../xdkdef.h"
#include "mdwrap.h"

/**
* \brief          Public key types
*/
typedef enum {
	PK_NONE = 0,
	PK_RSA,
	PK_ECKEY,
	PK_ECKEY_DH,
	PK_ECDSA,
	PK_RSA_ALT,
	PK_RSASSA_PSS,
} pk_type_t;


typedef struct _pk_info_t
{
    /** Public key type */
    pk_type_t type;

    /** Type name */
    const char *name;

    /** Get key size in bits */
	dword_t(*get_bitlen)(const void *);
}pk_info_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const pk_info_t rsa_info;

extern const pk_info_t eckey_info;
extern const pk_info_t eckeydh_info;

extern const pk_info_t ecdsa_info;

/**
 * \brief           Return information associated with the given PK type
 *
 * \param pk_type_t   PK type to search for.
 *
 * \return          The PK info associated with the type or NULL if not found.
 */
EXP_API const pk_info_t* pk_info_from_type(pk_type_t pk);

/**
 * \brief           Verify signature (including padding if relevant).
 *
 * \param ctx       The PK context to use. It must have been set up.
 * \param md_alg    Hash algorithm used (see notes)
 * \param hash      Hash of the message to sign
 * \param hash_len  Hash length or 0 (see notes)
 * \param sig       Signature to verify
 * \param sig_len   Signature length
 *
 * \return          0 on success (signature is valid),
 *                  #ERR_PK_SIG_LEN_MISMATCH if there is a valid
 *                  signature in sig but its length is less than \p siglen,
 *                  or a specific error code.
 *
 * \note            For RSA keys, the default padding type is PKCS#1 v1.5.
 *                  Use \c pk_verify_ext( PK_RSASSA_PSS, ... )
 *                  to verify RSASSA_PSS signatures.
 *
 * \note            If hash_len is 0, then the length associated with md_alg
 *                  is used instead, or an error returned if it is invalid.
 *
 * \note            md_alg may be MD_NONE, only if hash_len != 0
 */
EXP_API int pk_verify( pk_type_t pktype, void *pk_ctx, md_type_t md_alg,
               const byte_t *hash, dword_t hash_len,
               const byte_t *sig, dword_t sig_len );

/**
 * \brief           Verify signature, with options.
 *                  (Includes verification of the padding depending on type.)
 *
 * \param type      Signature type (inc. possible padding type) to verify
 * \param options   Pointer to type-specific options, or NULL
 * \param ctx       The PK context to use. It must have been set up.
 * \param md_alg    Hash algorithm used (see notes)
 * \param hash      Hash of the message to sign
 * \param hash_len  Hash length or 0 (see notes)
 * \param sig       Signature to verify
 * \param sig_len   Signature length
 *
 * \return          0 on success (signature is valid),
 *                  #ERR_PK_TYPE_MISMATCH if the PK context can't be
 *                  used for this type of signatures,
 *                  #ERR_PK_SIG_LEN_MISMATCH if there is a valid
 *                  signature in sig but its length is less than \p siglen,
 *                  or a specific error code.
 *
 * \note            If hash_len is 0, then the length associated with md_alg
 *                  is used instead, or an error returned if it is invalid.
 *
 * \note            md_alg may be MD_NONE, only if hash_len != 0
 *
 * \note            If type is PK_RSASSA_PSS, then options must point
 *                  to a pk_rsassa_pss_options structure,
 *                  otherwise it must be NULL.
 */
int pk_verify_ext(pk_type_t pktype, void *pk_ctx, md_type_t opt_mgf1_md, int opt_salt_len,
                   md_type_t md_alg,
                   const byte_t *hash, dword_t hash_len,
                   const byte_t *sig, dword_t sig_len );

/**
 * \brief           Make signature, including padding if relevant.
 *
 * \param ctx       The PK context to use. It must have been set up
 *                  with a private key.
 * \param md_alg    Hash algorithm used (see notes)
 * \param hash      Hash of the message to sign
 * \param hash_len  Hash length or 0 (see notes)
 * \param sig       Place to write the signature
 * \param sig_len   Number of bytes written
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \return          0 on success, or a specific error code.
 *
 * \note            For RSA keys, the default padding type is PKCS#1 v1.5.
 *                  There is no interface in the PK module to make RSASSA-PSS
 *                  signatures yet.
 *
 * \note            If hash_len is 0, then the length associated with md_alg
 *                  is used instead, or an error returned if it is invalid.
 *
 * \note            For RSA, md_alg may be MD_NONE if hash_len != 0.
 *                  For ECDSA, md_alg may never be MD_NONE.
 *
 * \note            In order to ensure enough space for the signature, the
 *                  \p sig buffer size must be of at least
 *                  `max(ECDSA_MAX_LEN, MPI_MAX_SIZE)` bytes.
 */
int pk_sign( pk_type_t pktype, void *pk_ctx, md_type_t md_alg,
             const byte_t *hash, dword_t hash_len,
             byte_t *sig, dword_t *sig_len,
             int (*f_rng)(void *, byte_t *, dword_t), void *p_rng );

/**
 * \brief           Decrypt message (including padding if relevant).
 *
 * \param ctx       The PK context to use. It must have been set up
 *                  with a private key.
 * \param input     Input to decrypt
 * \param ilen      Input size
 * \param output    Decrypted output
 * \param olen      Decrypted message length
 * \param osize     Size of the output buffer
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \note            For RSA keys, the default padding type is PKCS#1 v1.5.
 *
 * \return          0 on success, or a specific error code.
 */
int pk_decrypt( pk_type_t pktype, void *pk_ctx,
                const byte_t *input, dword_t ilen,
                byte_t *output, dword_t *olen, dword_t osize,
                int (*f_rng)(void *, byte_t *, dword_t), void *p_rng );

/**
 * \brief           Encrypt message (including padding if relevant).
 *
 * \param ctx       The PK context to use. It must have been set up.
 * \param input     Message to encrypt
 * \param ilen      Message size
 * \param output    Encrypted output
 * \param olen      Encrypted output length
 * \param osize     Size of the output buffer
 * \param f_rng     RNG function
 * \param p_rng     RNG parameter
 *
 * \note            For RSA keys, the default padding type is PKCS#1 v1.5.
 *
 * \return          0 on success, or a specific error code.
 */
int pk_encrypt( pk_type_t pktype, void *pk_ctx,
                const byte_t *input, dword_t ilen,
                byte_t *output, dword_t *olen, dword_t osize,
                int (*f_rng)(void *, byte_t *, dword_t), void *p_rng );


/**
 * \brief           Check if a public-private pair of keys matches.
 *
 * \param pub       Context holding a public key.
 * \param prv       Context holding a private (and public) key.
 *
 * \return          0 on success or ERR_PK_BAD_INPUT_DATA
 */
int pk_check_pair(pk_type_t pktype, void *pub_ctx, void *prv_ctx);

#ifdef __cplusplus
}
#endif


#endif /* md2.h */
