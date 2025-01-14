﻿/**
* \file ecdh.h
*
* \brief This file contains ECDH definitions and functions.
*
* The Elliptic Curve Diffie-Hellman (ECDH) protocol is an anonymous
* key agreement protocol allowing two parties to establish a shared
* secret over an insecure channel. Each party must have an
* elliptic-curve public–private key pair.
*
* For more information, see <em>NIST SP 800-56A Rev. 2: Recommendation for
* Pair-Wise Key Establishment Schemes Using Discrete Logarithm
* Cryptography</em>.
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

#ifndef ECDH_H
#define ECDH_H

#include "../xdkdef.h"
#include "mpi.h"
#include "ecp.h"

/**
* Defines the source of the imported EC key.
*/
typedef enum
{
	ECDH_OURS,   /**< Our key. */
	ECDH_THEIRS, /**< The key of the peer. */
} ecdh_side;


/**
*
* \warning         Performing multiple operations concurrently on the same
*                  ECDSA context is not supported; objects of this type
*                  should not be shared between multiple threads.
* \brief           The ECDH context structure.
*/
typedef struct ecdh_context
{
	ecp_group grp;   /*!< The elliptic curve used. */
	mpi d;           /*!< The private key. */
	ecp_point Q;     /*!< The public key. */
	ecp_point Qp;    /*!< The value of the public key of the peer. */
	mpi z;           /*!< The shared secret. */
	int point_format;        /*!< The format of point export in TLS messages. */
	ecp_point Vi;    /*!< The blinding value. */
	ecp_point Vf;    /*!< The unblinding value. */
	mpi _d;          /*!< The previous \p d. */
}ecdh_context;

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* \brief           This function generates an ECDH keypair on an elliptic
	*                  curve.
	*
	*                  This function performs the first of two core computations
	*                  implemented during the ECDH key exchange. The second core
	*                  computation is performed by mbedtls_ecdh_compute_shared().
	*
	* \see             ecp.h
	*
	* \param grp       The ECP group to use. This must be initialized and have
	*                  domain parameters loaded, for example through
	*                  mbedtls_ecp_load() or mbedtls_ecp_tls_read_group().
	* \param d         The destination MPI (private key).
	*                  This must be initialized.
	* \param Q         The destination point (public key).
	*                  This must be initialized.
	* \param f_rng     The RNG function to use. This must not be \c NULL.
	* \param p_rng     The RNG context to be passed to \p f_rng. This may be
	*                  \c NULL in case \p f_rng doesn't need a context argument.
	*
	* \return          \c 0 on success.
	* \return          Another \c MBEDTLS_ERR_ECP_XXX or
	*                  \c MBEDTLS_MPI_XXX error code on failure.
	*/
	EXP_API int ecdh_gen_public(ecp_group *grp, mpi *d, ecp_point *Q,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	/**
	* \brief           This function computes the shared secret.
	*
	*                  This function performs the second of two core computations
	*                  implemented during the ECDH key exchange. The first core
	*                  computation is performed by mbedtls_ecdh_gen_public().
	*
	* \see             ecp.h
	*
	* \note            If \p f_rng is not NULL, it is used to implement
	*                  countermeasures against side-channel attacks.
	*                  For more information, see mbedtls_ecp_mul().
	*
	* \param grp       The ECP group to use. This must be initialized and have
	*                  domain parameters loaded, for example through
	*                  mbedtls_ecp_load() or mbedtls_ecp_tls_read_group().
	* \param z         The destination MPI (shared secret).
	*                  This must be initialized.
	* \param Q         The public key from another party.
	*                  This must be initialized.
	* \param d         Our secret exponent (private key).
	*                  This must be initialized.
	* \param f_rng     The RNG function. This may be \c NULL if randomization
	*                  of intermediate results during the ECP computations is
	*                  not needed (discouraged). See the documentation of
	*                  mbedtls_ecp_mul() for more.
	* \param p_rng     The RNG context to be passed to \p f_rng. This may be
	*                  \c NULL if \p f_rng is \c NULL or doesn't need a
	*                  context argument.
	*
	* \return          \c 0 on success.
	* \return          Another \c MBEDTLS_ERR_ECP_XXX or
	*                  \c MBEDTLS_MPI_XXX error code on failure.
	*/
	EXP_API int ecdh_compute_shared(ecp_group *grp, mpi *z,
		const ecp_point *Q, const mpi *d,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	/**
	* \brief           This function initializes an ECDH context.
	*
	* \param ctx       The ECDH context to initialize. This must not be \c NULL.
	*/
	EXP_API void ecdh_init(ecdh_context *ctx);

	/**
	* \brief           This function sets up the ECDH context with the information
	*                  given.
	*
	*                  This function should be called after mbedtls_ecdh_init() but
	*                  before mbedtls_ecdh_make_params(). There is no need to call
	*                  this function before mbedtls_ecdh_read_params().
	*
	*                  This is the first function used by a TLS server for ECDHE
	*                  ciphersuites.
	*
	* \param ctx       The ECDH context to set up. This must be initialized.
	* \param grp_id    The group id of the group to set up the context for.
	*
	* \return          \c 0 on success.
	*/
	EXP_API int ecdh_setup(ecdh_context *ctx, ecp_group_id grp_id);

	/**
	* \brief           This function frees a context.
	*
	* \param ctx       The context to free. This may be \c NULL, in which
	*                  case this function does nothing. If it is not \c NULL,
	*                  it must point to an initialized ECDH context.
	*/
	EXP_API void ecdh_free(ecdh_context *ctx);

	/**
	* \brief           This function generates an EC key pair and exports its
	*                  in the format used in a TLS ServerKeyExchange handshake
	*                  message.
	*
	*                  This is the second function used by a TLS server for ECDHE
	*                  ciphersuites. (It is called after mbedtls_ecdh_setup().)
	*
	* \see             ecp.h
	*
	* \param ctx       The ECDH context to use. This must be initialized
	*                  and bound to a group, for example via mbedtls_ecdh_setup().
	* \param olen      The address at which to store the number of Bytes written.
	* \param buf       The destination buffer. This must be a writable buffer of
	*                  length \p blen Bytes.
	* \param blen      The length of the destination buffer \p buf in Bytes.
	* \param f_rng     The RNG function to use. This must not be \c NULL.
	* \param p_rng     The RNG context to be passed to \p f_rng. This may be
	*                  \c NULL in case \p f_rng doesn't need a context argument.
	*
	* \return          \c 0 on success.
	* \return          #MBEDTLS_ERR_ECP_IN_PROGRESS if maximum number of
	*                  operations was reached: see \c mbedtls_ecp_set_max_ops().
	* \return          Another \c MBEDTLS_ERR_ECP_XXX error code on failure.
	*/
	EXP_API int ecdh_make_params(ecdh_context *ctx, dword_t *olen,
		byte_t *buf, dword_t blen,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	/**
	* \brief           This function parses the ECDHE parameters in a
	*                  TLS ServerKeyExchange handshake message.
	*
	* \note            In a TLS handshake, this is the how the client
	*                  sets up its ECDHE context from the server's public
	*                  ECDHE key material.
	*
	* \see             ecp.h
	*
	* \param ctx       The ECDHE context to use. This must be initialized.
	* \param buf       On input, \c *buf must be the start of the input buffer.
	*                  On output, \c *buf is updated to point to the end of the
	*                  data that has been read. On success, this is the first byte
	*                  past the end of the ServerKeyExchange parameters.
	*                  On error, this is the point at which an error has been
	*                  detected, which is usually not useful except to debug
	*                  failures.
	* \param end       The end of the input buffer.
	*
	* \return          \c 0 on success.
	* \return          An \c MBEDTLS_ERR_ECP_XXX error code on failure.
	*
	*/
	EXP_API int ecdh_read_params(ecdh_context *ctx,
		const byte_t **buf,
		const byte_t *end,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	/**
	* \brief           This function sets up an ECDH context from an EC key.
	*
	*                  It is used by clients and servers in place of the
	*                  ServerKeyEchange for static ECDH, and imports ECDH
	*                  parameters from the EC key information of a certificate.
	*
	* \see             ecp.h
	*
	* \param ctx       The ECDH context to set up. This must be initialized.
	* \param key       The EC key to use. This must be initialized.
	* \param side      Defines the source of the key. Possible values are:
	*                  - #MBEDTLS_ECDH_OURS: The key is ours.
	*                  - #MBEDTLS_ECDH_THEIRS: The key is that of the peer.
	*
	* \return          \c 0 on success.
	* \return          Another \c MBEDTLS_ERR_ECP_XXX error code on failure.
	*
	*/
	EXP_API int ecdh_get_params(ecdh_context *ctx,
		const ecp_keypair *key,
		ecdh_side side);

	/**
	* \brief           This function generates a public key and exports it
	*                  as a TLS ClientKeyExchange payload.
	*
	*                  This is the second function used by a TLS client for ECDH(E)
	*                  ciphersuites.
	*
	* \see             ecp.h
	*
	* \param ctx       The ECDH context to use. This must be initialized
	*                  and bound to a group, the latter usually by
	*                  mbedtls_ecdh_read_params().
	* \param olen      The address at which to store the number of Bytes written.
	*                  This must not be \c NULL.
	* \param buf       The destination buffer. This must be a writable buffer
	*                  of length \p blen Bytes.
	* \param blen      The size of the destination buffer \p buf in Bytes.
	* \param f_rng     The RNG function to use. This must not be \c NULL.
	* \param p_rng     The RNG context to be passed to \p f_rng. This may be
	*                  \c NULL in case \p f_rng doesn't need a context argument.
	*
	* \return          \c 0 on success.
	* \return          #MBEDTLS_ERR_ECP_IN_PROGRESS if maximum number of
	*                  operations was reached: see \c mbedtls_ecp_set_max_ops().
	* \return          Another \c MBEDTLS_ERR_ECP_XXX error code on failure.
	*/
	EXP_API int ecdh_make_public(ecdh_context *ctx, dword_t *olen,
		byte_t *buf, dword_t blen,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	/**
	* \brief       This function parses and processes the ECDHE payload of a
	*              TLS ClientKeyExchange message.
	*
	*              This is the third function used by a TLS server for ECDH(E)
	*              ciphersuites. (It is called after mbedtls_ecdh_setup() and
	*              mbedtls_ecdh_make_params().)
	*
	* \see         ecp.h
	*
	* \param ctx   The ECDH context to use. This must be initialized
	*              and bound to a group, for example via mbedtls_ecdh_setup().
	* \param buf   The pointer to the ClientKeyExchange payload. This must
	*              be a readable buffer of length \p blen Bytes.
	* \param blen  The length of the input buffer \p buf in Bytes.
	*
	* \return      \c 0 on success.
	* \return      An \c MBEDTLS_ERR_ECP_XXX error code on failure.
	*/
	EXP_API int ecdh_read_public(ecdh_context *ctx, 
		const byte_t *buf, dword_t blen,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	/**
	* \brief           This function derives and exports the shared secret.
	*
	*                  This is the last function used by both TLS client
	*                  and servers.
	*
	* \note            If \p f_rng is not NULL, it is used to implement
	*                  countermeasures against side-channel attacks.
	*                  For more information, see mbedtls_ecp_mul().
	*
	* \see             ecp.h

	* \param ctx       The ECDH context to use. This must be initialized
	*                  and have its own private key generated and the peer's
	*                  public key imported.
	* \param olen      The address at which to store the total number of
	*                  Bytes written on success. This must not be \c NULL.
	* \param buf       The buffer to write the generated shared key to. This
	*                  must be a writable buffer of size \p blen Bytes.
	* \param blen      The length of the destination buffer \p buf in Bytes.
	* \param f_rng     The RNG function, for blinding purposes. This may
	*                  b \c NULL if blinding isn't needed.
	* \param p_rng     The RNG context. This may be \c NULL if \p f_rng
	*                  doesn't need a context argument.
	*
	* \return          \c 0 on success.
	* \return          #MBEDTLS_ERR_ECP_IN_PROGRESS if maximum number of
	*                  operations was reached: see \c mbedtls_ecp_set_max_ops().
	* \return          Another \c MBEDTLS_ERR_ECP_XXX error code on failure.
	*/
	EXP_API int ecdh_calc_secret(ecdh_context *ctx, dword_t *olen,
		byte_t *buf, dword_t blen,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	EXP_API int ecdh_make_params_tls13(ecdh_context *ctx,
		ecp_group_id grp_id,
		dword_t *olen, byte_t *buf, dword_t blen,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	EXP_API int ecdh_read_params_tls13(ecdh_context *ctx,
		ecp_group_id grp_id,
		const byte_t *buf, dword_t blen,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	EXP_API int ecdh_make_public_tls13(ecdh_context *ctx,
		dword_t *olen, byte_t *buf, dword_t blen,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);

	EXP_API int ecdh_read_public_tls13(ecdh_context *ctx,
		const byte_t *buf, dword_t blen,
		int(*f_rng)(void *, byte_t *, dword_t),
		void *p_rng);


#if defined(XDK_SUPPORT_TEST)
	/**
	* \brief          The ecdh checkup routine.
	*
	* \return         \c 0 on success.
	* \return         \c 1 on failure.
	*/
	EXP_API int ecdh_x25519_test(int verbose);

	EXP_API int ecdh_test(int verbose);
#endif

#ifdef __cplusplus
}
#endif


#endif

