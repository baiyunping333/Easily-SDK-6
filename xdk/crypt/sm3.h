/**
* \file sm3.h
*
* \brief This file contains SM3 definitions and functions.
*
* The Secure Hash Algorithm 1 (SM3) cryptographic hash function is defined in
* <em>FIPS 180-4: Secure Hash Standard (SHS)</em>.
*
* \warning   SM3 is considered a weak message digest and its use constitutes
*            a security risk. We recommend considering stronger message
*            digests instead.
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
#ifndef SM3_H
#define SM3_H

#include "../xdkdef.h"

/**
* \brief          The SM3 context structure.
*
* \warning        SM3 is considered a weak message digest and its use
*                 constitutes a security risk. We recommend considering
*                 stronger message digests instead.
*
*/
typedef struct sm3_context
{
	dword_t total[2];          /*!< The number of Bytes processed.  */
	dword_t state[8];          /*!< The intermediate digest state.  */
	dword_t used;			/*!<the used bytes>*/
	byte_t buffer[64];   /*!< The data block being processed. */

	byte_t ipad[64];
	byte_t opad[64];
}
sm3_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          This function initializes a SM3 context.
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \param ctx      The SM3 context to initialize.
 *                 This must not be \c NULL.
 *
 */
EXP_API void sm3_init(sm3_context *ctx);

/**
 * \brief          This function clears a SM3 context.
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \param ctx      The SM3 context to clear. This may be \c NULL,
 *                 in which case this function does nothing. If it is
 *                 not \c NULL, it must point to an initialized
 *                 SM3 context.
 *
 */
EXP_API void sm3_free(sm3_context *ctx);

/**
 * \brief          This function clones the state of a SM3 context.
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \param dst      The SM3 context to clone to. This must be initialized.
 * \param src      The SM3 context to clone from. This must be initialized.
 *
 */
EXP_API void sm3_clone(sm3_context *dst,
                         const sm3_context *src );

/**
 * \brief          This function starts a SM3 checksum calculation.
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \param ctx      The SM3 context to initialize. This must be initialized.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 *
 */
EXP_API int sm3_starts(sm3_context *ctx);

/**
 * \brief          This function feeds an input buffer into an ongoing SM3
 *                 checksum calculation.
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \param ctx      The SM3 context. This must be initialized
 *                 and have a hash operation started.
 * \param input    The buffer holding the input data.
 *                 This must be a readable buffer of length \p ilen Bytes.
 * \param ilen     The length of the input data \p input in Bytes.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 */
EXP_API int sm3_update(sm3_context *ctx,
                             const byte_t *input,
                             dword_t ilen );

/**
 * \brief          This function finishes the SM3 operation, and writes
 *                 the result to the output buffer.
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \param ctx      The SM3 context to use. This must be initialized and
 *                 have a hash operation started.
 * \param output   The SM3 checksum result. This must be a writable
 *                 buffer of length \c 32 Bytes.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 */
EXP_API int sm3_finish(sm3_context *ctx,
                             byte_t output[32] );

/**
 * \brief          SM3 process data block (internal use only).
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \param ctx      The SM3 context to use. This must be initialized.
 * \param data     The data block being processed. This must be a
 *                 readable buffer of length \c 64 Bytes.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 *
 */
EXP_API int internal_sm3_process(sm3_context *ctx,
                                   const byte_t data[64] );

/**
 * \brief          This function calculates the SM3 checksum of a buffer.
 *
 *                 The function allocates the context, performs the
 *                 calculation, and frees the context.
 *
 *                 The SM3 result is calculated as
 *                 output = SM3(input buffer).
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \param input    The buffer holding the input data.
 *                 This must be a readable buffer of length \p ilen Bytes.
 * \param ilen     The length of the input data \p input in Bytes.
 * \param output   The SM3 checksum result.
 *                 This must be a writable buffer of length \c 32 Bytes.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 *
 */
EXP_API int sm3(const byte_t *input,
                      dword_t ilen,
                      byte_t output[32] );


/**
 * \brief           This function sets the HMAC key and prepares to
 *                  authenticate a new message.
 *
 *                  Call this function after md_setup(), to use
 *                  the MD context for an HMAC calculation, then call
 *                  md_hmac_update() to provide the input data, and
 *                  md_hmac_finish() to get the HMAC value.
 *
 * \param ctx       The message digest context containing an embedded HMAC
 *                  context.
 * \param key       The HMAC secret key.
 * \param keylen    The length of the HMAC key in Bytes.
 *
 * \return          \c 0 on success.
 * \return          #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                  failure.
 */
EXP_API int sm3_hmac_starts(sm3_context *ctx, const byte_t *key,
                    dword_t keylen );

/**
 * \brief           This function feeds an input buffer into an ongoing HMAC
 *                  computation.
 *
 *                  Call md_hmac_starts() or md_hmac_reset()
 *                  before calling this function.
 *                  You may call this function multiple times to pass the
 *                  input piecewise.
 *                  Afterwards, call md_hmac_finish().
 *
 * \param ctx       The message digest context containing an embedded HMAC
 *                  context.
 * \param input     The buffer holding the input data.
 * \param ilen      The length of the input data.
 *
 * \return          \c 0 on success.
 * \return          #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                  failure.
 */
EXP_API int sm3_hmac_update(sm3_context *ctx, const byte_t *input,
                    dword_t ilen );

/**
 * \brief           This function finishes the HMAC operation, and writes
 *                  the result to the output buffer.
 *
 *                  Call this function after md_hmac_starts() and
 *                  md_hmac_update() to get the HMAC value. Afterwards
 *                  you may either call md_free() to clear the context,
 *                  or call md_hmac_reset() to reuse the context with
 *                  the same HMAC key.
 *
 * \param ctx       The message digest context containing an embedded HMAC
 *                  context.
 * \param output    The generic HMAC checksum result.
 *
 * \return          \c 0 on success.
 * \return          #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                  failure.
 */
EXP_API int sm3_hmac_finish(sm3_context *ctx, byte_t output[32]);

/**
 * \brief           This function prepares to authenticate a new message with
 *                  the same key as the previous HMAC operation.
 *
 *                  You may call this function after md_hmac_finish().
 *                  Afterwards call md_hmac_update() to pass the new
 *                  input.
 *
 * \param ctx       The message digest context containing an embedded HMAC
 *                  context.
 *
 * \return          \c 0 on success.
 * \return          #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                  failure.
 */
EXP_API int sm3_hmac_reset(sm3_context *ctx);

/**
 * \brief          This function calculates the full generic HMAC
 *                 on the input buffer with the provided key.
 *
 *                 The function allocates the context, performs the
 *                 calculation, and frees the context.
 *
 *                 The HMAC result is calculated as
 *                 output = generic HMAC(hmac key, input buffer).
 *
 * \param md_info  The information structure of the message-digest algorithm
 *                 to use.
 * \param key      The HMAC secret key.
 * \param keylen   The length of the HMAC secret key in Bytes.
 * \param input    The buffer holding the input data.
 * \param ilen     The length of the input data.
 * \param output   The generic HMAC result.
 *
 * \return         \c 0 on success.
 * \return         #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                 failure.
 */
EXP_API int sm3_hmac(const byte_t *key, dword_t keylen,
                const byte_t *input, dword_t ilen,
                byte_t output[32] );

#if defined(XDK_SUPPORT_TEST)

/**
 * \brief          The SM3 checkup routine.
 *
 * \warning        SM3 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 * \return         \c 0 on success.
 * \return         \c 1 on failure.
 *
 */
EXP_API int sm3_self_test( int verbose );

#endif /* SELF_TEST */

#ifdef __cplusplus
}
#endif


#endif /* sm3.h */

