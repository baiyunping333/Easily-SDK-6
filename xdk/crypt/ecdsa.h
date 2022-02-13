/**
* \file ecdsa.h
*
* \brief This file contains ECDSA definitions and functions.
*
* The Elliptic Curve Digital Signature Algorithm (ECDSA) is defined in
* <em>Standards for Efficient Cryptography Group (SECG):
* SEC1 Elliptic Curve Cryptography</em>.
* The use of ECDSA for TLS is defined in <em>RFC-4492: Elliptic Curve
* Cryptography (ECC) Cipher Suites for Transport Layer Security (TLS)</em>.
*
*/
/*
*  Copyright (C) 2006-2018, Arm Limited (or its affiliates), All Rights Reserved
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
*  This file is part of Mbed TLS (https://tls.mbed.org)
*/
#ifndef ECDSA_H
#define ECDSA_H


#include "../xdkdef.h"
#include "mdwrap.h"
#include "mpi.h"
#include "ecp.h"


/*
* RFC-4492 page 20:
*
*     Ecdsa-Sig-Value ::= SEQUENCE {
*         r       INTEGER,
*         s       INTEGER
*     }
*
* Size is at most
*    1 (tag) + 1 (len) + 1 (initial 0) + ECP_MAX_BYTES for each of r and s,
*    twice that + 1 (tag) + 2 (len) for the sequence
* (assuming ECP_MAX_BYTES is less than 126 for r and s,
* and less than 124 (total len <= 255) for the sequence)
*/
#if ECP_MAX_BYTES > 124
#error "ECP_MAX_BYTES bigger than expected, please fix ECDSA_MAX_LEN"
#endif
/** The maximal size of an ECDSA signature in Bytes. */
#define ECDSA_MAX_LEN  ( 3 + 2 * ( 3 + ECP_MAX_BYTES ) )

/**
* \brief           The ECDSA context structure.
*
* \warning         Performing multiple operations concurrently on the same
*                  ECDSA context is not supported; objects of this type
*                  should not be shared between multiple threads.
*/
typedef ecp_keypair ecdsa_context;

/* Now we can declare functions that take a pointer to that */
typedef void ecdsa_restart_ctx;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief           This function computes the ECDSA signature of a
 *                  previously-hashed message.
 *
 * \note            The deterministic version implemented in
 *                  ecdsa_sign_det() is usually preferred.
 *
 * \note            If the bitlength of the message hash is larger than the
 *                  bitlength of the group order, then the hash is truncated
 *                  as defined in <em>Standards for Efficient Cryptography Group
 *                  (SECG): SEC1 Elliptic Curve Cryptography</em>, section
 *                  4.1.3, step 5.
 *
 * \see             ecp.h
 *
 * \param grp       The context for the elliptic curve to use.
 *                  This must be initialized and have group parameters
 *                  set, for example through ecp_group_load().
 * \param r         The MPI context in which to store the first part
 *                  the signature. This must be initialized.
 * \param s         The MPI context in which to store the second part
 *                  the signature. This must be initialized.
 * \param d         The private signing key. This must be initialized.
 * \param buf       The content to be signed. This is usually the hash of
 *                  the original data to be signed. This must be a readable
 *                  buffer of length \p blen Bytes. It may be \c NULL if
 *                  \p blen is zero.
 * \param blen      The length of \p buf in Bytes.
 * \param f_rng     The RNG function. This must not be \c NULL.
 * \param p_rng     The RNG context to be passed to \p f_rng. This may be
 *                  \c NULL if \p f_rng doesn't need a context parameter.
 *
 * \return          \c 0 on success.
 * \return          An \c ERR_ECP_XXX
 *                  or \c MPI_XXX error code on failure.
 */
EXP_API int ecdsa_sign(ecp_group *grp, mpi *r, mpi *s,
                const mpi *d, const byte_t *buf, dword_t blen,
                int (*f_rng)(void *, byte_t *, dword_t), void *p_rng );

/**
 * \brief           This function computes the ECDSA signature of a
 *                  previously-hashed message, deterministic version.
 *
 *                  For more information, see <em>RFC-6979: Deterministic
 *                  Usage of the Digital Signature Algorithm (DSA) and Elliptic
 *                  Curve Digital Signature Algorithm (ECDSA)</em>.
 *
 * \note            If the bitlength of the message hash is larger than the
 *                  bitlength of the group order, then the hash is truncated as
 *                  defined in <em>Standards for Efficient Cryptography Group
 *                  (SECG): SEC1 Elliptic Curve Cryptography</em>, section
 *                  4.1.3, step 5.
 *
 * \warning         Since the output of the internal RNG is always the same for
 *                  the same key and message, this limits the efficiency of
 *                  blinding and leaks information through side channels. For
 *                  secure behavior use ecdsa_sign_det_ext() instead.
 *
 *                  (Optimally the blinding is a random value that is different
 *                  on every execution. In this case the blinding is still
 *                  random from the attackers perspective, but is the same on
 *                  each execution. This means that this blinding does not
 *                  prevent attackers from recovering secrets by combining
 *                  several measurement traces, but may prevent some attacks
 *                  that exploit relationships between secret data.)
 *
 * \see             ecp.h
 *
 * \param grp       The context for the elliptic curve to use.
 *                  This must be initialized and have group parameters
 *                  set, for example through ecp_group_load().
 * \param r         The MPI context in which to store the first part
 *                  the signature. This must be initialized.
 * \param s         The MPI context in which to store the second part
 *                  the signature. This must be initialized.
 * \param d         The private signing key. This must be initialized
 *                  and setup, for example through ecp_gen_privkey().
 * \param buf       The hashed content to be signed. This must be a readable
 *                  buffer of length \p blen Bytes. It may be \c NULL if
 *                  \p blen is zero.
 * \param blen      The length of \p buf in Bytes.
 * \param md_alg    The hash algorithm used to hash the original data.
 *
 * \return          \c 0 on success.
 * \return          An \c ERR_ECP_XXX or \c MPI_XXX
 *                  error code on failure.
 */
EXP_API int ecdsa_sign_det(ecp_group *grp, mpi *r,
                            mpi *s, const mpi *d,
                            const byte_t *buf, dword_t blen,
                            md_type_t md_alg );
/**
 * \brief           This function computes the ECDSA signature of a
 *                  previously-hashed message, deterministic version.
 *
 *                  For more information, see <em>RFC-6979: Deterministic
 *                  Usage of the Digital Signature Algorithm (DSA) and Elliptic
 *                  Curve Digital Signature Algorithm (ECDSA)</em>.
 *
 * \note            If the bitlength of the message hash is larger than the
 *                  bitlength of the group order, then the hash is truncated as
 *                  defined in <em>Standards for Efficient Cryptography Group
 *                  (SECG): SEC1 Elliptic Curve Cryptography</em>, section
 *                  4.1.3, step 5.
 *
 * \see             ecp.h
 *
 * \param grp           The context for the elliptic curve to use.
 *                      This must be initialized and have group parameters
 *                      set, for example through ecp_group_load().
 * \param r             The MPI context in which to store the first part
 *                      the signature. This must be initialized.
 * \param s             The MPI context in which to store the second part
 *                      the signature. This must be initialized.
 * \param d             The private signing key. This must be initialized
 *                      and setup, for example through ecp_gen_privkey().
 * \param buf           The hashed content to be signed. This must be a readable
 *                      buffer of length \p blen Bytes. It may be \c NULL if
 *                      \p blen is zero.
 * \param blen          The length of \p buf in Bytes.
 * \param md_alg        The hash algorithm used to hash the original data.
 * \param f_rng_blind   The RNG function used for blinding. This must not be
 *                      \c NULL.
 * \param p_rng_blind   The RNG context to be passed to \p f_rng. This may be
 *                      \c NULL if \p f_rng doesn't need a context parameter.
 *
 * \return          \c 0 on success.
 * \return          An \c ERR_ECP_XXX or \c MPI_XXX
 *                  error code on failure.
 */
EXP_API int ecdsa_sign_det_ext(ecp_group *grp, mpi *r,
                                mpi *s, const mpi *d,
                                const byte_t *buf, dword_t blen,
                                md_type_t md_alg,
                                int (*f_rng_blind)(void *, byte_t *,
                                                   dword_t),
                                void *p_rng_blind );

/**
 * \brief           This function verifies the ECDSA signature of a
 *                  previously-hashed message.
 *
 * \note            If the bitlength of the message hash is larger than the
 *                  bitlength of the group order, then the hash is truncated as
 *                  defined in <em>Standards for Efficient Cryptography Group
 *                  (SECG): SEC1 Elliptic Curve Cryptography</em>, section
 *                  4.1.4, step 3.
 *
 * \see             ecp.h
 *
 * \param grp       The ECP group to use.
 *                  This must be initialized and have group parameters
 *                  set, for example through ecp_group_load().
 * \param buf       The hashed content that was signed. This must be a readable
 *                  buffer of length \p blen Bytes. It may be \c NULL if
 *                  \p blen is zero.
 * \param blen      The length of \p buf in Bytes.
 * \param Q         The public key to use for verification. This must be
 *                  initialized and setup.
 * \param r         The first integer of the signature.
 *                  This must be initialized.
 * \param s         The second integer of the signature.
 *                  This must be initialized.
 *
 * \return          \c 0 on success.
 * \return          #ERR_ECP_BAD_INPUT_DATA if the signature
 *                  is invalid.
 * \return          An \c ERR_ECP_XXX or \c MPI_XXX
 *                  error code on failure for any other reason.
 */
EXP_API int ecdsa_verify(ecp_group *grp,
                          const byte_t *buf, dword_t blen,
                          const ecp_point *Q, const mpi *r,
                          const mpi *s);

/**
 * \brief           This function computes the ECDSA signature and writes it
 *                  to a buffer, serialized as defined in <em>RFC-4492:
 *                  Elliptic Curve Cryptography (ECC) Cipher Suites for
 *                  Transport Layer Security (TLS)</em>.
 *
 * \warning         It is not thread-safe to use the same context in
 *                  multiple threads.
 *
 * \note            The deterministic version is used if
 *                  #ECDSA_DETERMINISTIC is defined. For more
 *                  information, see <em>RFC-6979: Deterministic Usage
 *                  of the Digital Signature Algorithm (DSA) and Elliptic
 *                  Curve Digital Signature Algorithm (ECDSA)</em>.
 *
 * \note            If the bitlength of the message hash is larger than the
 *                  bitlength of the group order, then the hash is truncated as
 *                  defined in <em>Standards for Efficient Cryptography Group
 *                  (SECG): SEC1 Elliptic Curve Cryptography</em>, section
 *                  4.1.3, step 5.
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDSA context to use. This must be initialized
 *                  and have a group and private key bound to it, for example
 *                  via ecdsa_genkey() or ecdsa_from_keypair().
 * \param md_alg    The message digest that was used to hash the message.
 * \param hash      The message hash to be signed. This must be a readable
 *                  buffer of length \p blen Bytes.
 * \param hlen      The length of the hash \p hash in Bytes.
 * \param sig       The buffer to which to write the signature. This must be a
 *                  writable buffer of length at least twice as large as the
 *                  size of the curve used, plus 9. For example, 73 Bytes if
 *                  a 256-bit curve is used. A buffer length of
 *                  #ECDSA_MAX_LEN is always safe.
 * \param slen      The address at which to store the actual length of
 *                  the signature written. Must not be \c NULL.
 * \param f_rng     The RNG function. This must not be \c NULL if
 *                  #ECDSA_DETERMINISTIC is unset. Otherwise,
 *                  it is unused and may be set to \c NULL.
 * \param p_rng     The RNG context to be passed to \p f_rng. This may be
 *                  \c NULL if \p f_rng is \c NULL or doesn't use a context.
 *
 * \return          \c 0 on success.
 * \return          An \c ERR_ECP_XXX, \c ERR_MPI_XXX or
 *                  \c ERR_ASN1_XXX error code on failure.
 */
EXP_API int ecdsa_write_signature(ecdsa_context *ctx,
                                   md_type_t md_alg,
                           const byte_t *hash, dword_t hlen,
                           byte_t *sig, dword_t *slen,
                           int (*f_rng)(void *, byte_t *, dword_t),
                           void *p_rng );

/**
 * \brief           This function computes the ECDSA signature and writes it
 *                  to a buffer, in a restartable way.
 *
 * \see             \c ecdsa_write_signature()
 *
 * \note            This function is like \c ecdsa_write_signature()
 *                  but it can return early and restart according to the limit
 *                  set with \c ecp_set_max_ops() to reduce blocking.
 *
 * \param ctx       The ECDSA context to use. This must be initialized
 *                  and have a group and private key bound to it, for example
 *                  via ecdsa_genkey() or ecdsa_from_keypair().
 * \param md_alg    The message digest that was used to hash the message.
 * \param hash      The message hash to be signed. This must be a readable
 *                  buffer of length \p blen Bytes.
 * \param hlen      The length of the hash \p hash in Bytes.
 * \param sig       The buffer to which to write the signature. This must be a
 *                  writable buffer of length at least twice as large as the
 *                  size of the curve used, plus 9. For example, 73 Bytes if
 *                  a 256-bit curve is used. A buffer length of
 *                  #ECDSA_MAX_LEN is always safe.
 * \param slen      The address at which to store the actual length of
 *                  the signature written. Must not be \c NULL.
 * \param f_rng     The RNG function. This must not be \c NULL if
 *                  #ECDSA_DETERMINISTIC is unset. Otherwise,
 *                  it is unused and may be set to \c NULL.
 * \param p_rng     The RNG context to be passed to \p f_rng. This may be
 *                  \c NULL if \p f_rng is \c NULL or doesn't use a context.
 * \param rs_ctx    The restart context to use. This may be \c NULL to disable
 *                  restarting. If it is not \c NULL, it must point to an
 *                  initialized restart context.
 *
 * \return          \c 0 on success.
 * \return          #ERR_ECP_IN_PROGRESS if maximum number of
 *                  operations was reached: see \c ecp_set_max_ops().
 * \return          Another \c ERR_ECP_XXX, \c ERR_MPI_XXX or
 *                  \c ERR_ASN1_XXX error code on failure.
 */
EXP_API int ecdsa_write_signature_restartable(ecdsa_context *ctx,
                           md_type_t md_alg,
                           const byte_t *hash, dword_t hlen,
                           byte_t *sig, dword_t *slen,
                           int (*f_rng)(void *, byte_t *, dword_t),
                           void *p_rng,
                           ecdsa_restart_ctx *rs_ctx );

/**
 * \brief           This function reads and verifies an ECDSA signature.
 *
 * \note            If the bitlength of the message hash is larger than the
 *                  bitlength of the group order, then the hash is truncated as
 *                  defined in <em>Standards for Efficient Cryptography Group
 *                  (SECG): SEC1 Elliptic Curve Cryptography</em>, section
 *                  4.1.4, step 3.
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDSA context to use. This must be initialized
 *                  and have a group and public key bound to it.
 * \param hash      The message hash that was signed. This must be a readable
 *                  buffer of length \p size Bytes.
 * \param hlen      The size of the hash \p hash.
 * \param sig       The signature to read and verify. This must be a readable
 *                  buffer of length \p slen Bytes.
 * \param slen      The size of \p sig in Bytes.
 *
 * \return          \c 0 on success.
 * \return          #ERR_ECP_BAD_INPUT_DATA if signature is invalid.
 * \return          #ERR_ECP_SIG_LEN_MISMATCH if there is a valid
 *                  signature in \p sig, but its length is less than \p siglen.
 * \return          An \c ERR_ECP_XXX or \c ERR_MPI_XXX
 *                  error code on failure for any other reason.
 */
EXP_API int ecdsa_read_signature(ecdsa_context *ctx,
                          const byte_t *hash, dword_t hlen,
                          const byte_t *sig, dword_t slen );

/**
 * \brief           This function reads and verifies an ECDSA signature,
 *                  in a restartable way.
 *
 * \see             \c ecdsa_read_signature()
 *
 * \note            This function is like \c ecdsa_read_signature()
 *                  but it can return early and restart according to the limit
 *                  set with \c ecp_set_max_ops() to reduce blocking.
 *
 * \param ctx       The ECDSA context to use. This must be initialized
 *                  and have a group and public key bound to it.
 * \param hash      The message hash that was signed. This must be a readable
 *                  buffer of length \p size Bytes.
 * \param hlen      The size of the hash \p hash.
 * \param sig       The signature to read and verify. This must be a readable
 *                  buffer of length \p slen Bytes.
 * \param slen      The size of \p sig in Bytes.
 * \param rs_ctx    The restart context to use. This may be \c NULL to disable
 *                  restarting. If it is not \c NULL, it must point to an
 *                  initialized restart context.
 *
 * \return          \c 0 on success.
 * \return          #ERR_ECP_BAD_INPUT_DATA if signature is invalid.
 * \return          #ERR_ECP_SIG_LEN_MISMATCH if there is a valid
 *                  signature in \p sig, but its length is less than \p siglen.
 * \return          #ERR_ECP_IN_PROGRESS if maximum number of
 *                  operations was reached: see \c ecp_set_max_ops().
 * \return          Another \c ERR_ECP_XXX or \c ERR_MPI_XXX
 *                  error code on failure for any other reason.
 */
EXP_API int ecdsa_read_signature_restartable(ecdsa_context *ctx,
                          const byte_t *hash, dword_t hlen,
                          const byte_t *sig, dword_t slen,
                          ecdsa_restart_ctx *rs_ctx );

/**
 * \brief          This function generates an ECDSA keypair on the given curve.
 *
 * \see            ecp.h
 *
 * \param ctx      The ECDSA context to store the keypair in.
 *                 This must be initialized.
 * \param gid      The elliptic curve to use. One of the various
 *                 \c ECP_DP_XXX macros depending on configuration.
 * \param f_rng    The RNG function to use. This must not be \c NULL.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be
 *                 \c NULL if \p f_rng doesn't need a context argument.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_ECP_XXX code on failure.
 */
EXP_API int ecdsa_genkey(ecdsa_context *ctx, ecp_group_id gid,
                  int (*f_rng)(void *, byte_t *, dword_t), void *p_rng );

/**
 * \brief           This function sets up an ECDSA context from an EC key pair.
 *
 * \see             ecp.h
 *
 * \param ctx       The ECDSA context to setup. This must be initialized.
 * \param key       The EC key to use. This must be initialized and hold
 *                  a private-public key pair or a public key. In the former
 *                  case, the ECDSA context may be used for signature creation
 *                  and verification after this call. In the latter case, it
 *                  may be used for signature verification.
 *
 * \return          \c 0 on success.
 * \return          An \c ERR_ECP_XXX code on failure.
 */
EXP_API int ecdsa_from_keypair(ecdsa_context *ctx,
                                const ecp_keypair *key );

/**
 * \brief           This function initializes an ECDSA context.
 *
 * \param ctx       The ECDSA context to initialize.
 *                  This must not be \c NULL.
 */
EXP_API void ecdsa_init(ecdsa_context *ctx);

/**
 * \brief           This function frees an ECDSA context.
 *
 * \param ctx       The ECDSA context to free. This may be \c NULL,
 *                  in which case this function does nothing. If it
 *                  is not \c NULL, it must be initialized.
 */
EXP_API void ecdsa_free(ecdsa_context *ctx);


#ifdef __cplusplus
}
#endif


#endif /* rsa.h */
