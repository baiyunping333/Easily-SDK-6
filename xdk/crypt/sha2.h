﻿/**
* \file sha256.h
*
* \brief This file contains SHA-224 and SHA-256 definitions and functions.
*
* The Secure Hash Algorithms 224 and 256 (SHA-224 and SHA-256) cryptographic
* hash functions are defined in <em>FIPS 180-4: Secure Hash Standard (SHS)</em>.
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
#ifndef SHA2_H
#define SHA2_H

#include "../xdkdef.h"

/**
* \brief          The SHA-256 context structure.
*
*                 The structure is used both for SHA-256 and for SHA-224
*                 checksum calculations. The choice between these two is
*                 made in the call to sha256_starts().
*/
typedef struct sha256_context
{
	dword_t total[2];          /*!< The number of Bytes processed.  */
	dword_t state[8];          /*!< The intermediate digest state.  */
	byte_t buffer[64];   /*!< The data block being processed. */
	int is224;                  /*!< Determines which function to use:
								0: Use SHA-256, or 1: Use SHA-224. */

	byte_t ipad[64];
	byte_t opad[64];
}
sha256_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          This function initializes a SHA-256 context.
 *
 * \param ctx      The SHA-256 context to initialize. This must not be \c NULL.
 */
EXP_API void sha256_init(sha256_context *ctx);

/**
 * \brief          This function clears a SHA-256 context.
 *
 * \param ctx      The SHA-256 context to clear. This may be \c NULL, in which
 *                 case this function returns immediately. If it is not \c NULL,
 *                 it must point to an initialized SHA-256 context.
 */
EXP_API void sha256_free(sha256_context *ctx);

/**
 * \brief          This function clones the state of a SHA-256 context.
 *
 * \param dst      The destination context. This must be initialized.
 * \param src      The context to clone. This must be initialized.
 */
EXP_API void sha256_clone(sha256_context *dst,
                           const sha256_context *src );

/**
 * \brief          This function starts a SHA-224 or SHA-256 checksum
 *                 calculation.
 *
 * \param ctx      The context to use. This must be initialized.
 * \param is224    This determines which function to use. This must be
 *                 either \c 0 for SHA-256, or \c 1 for SHA-224.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 */
EXP_API int sha256_starts(sha256_context *ctx, int is224);

/**
 * \brief          This function feeds an input buffer into an ongoing
 *                 SHA-256 checksum calculation.
 *
 * \param ctx      The SHA-256 context. This must be initialized
 *                 and have a hash operation started.
 * \param input    The buffer holding the data. This must be a readable
 *                 buffer of length \p ilen Bytes.
 * \param ilen     The length of the input data in Bytes.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 */
EXP_API int sha256_update(sha256_context *ctx,
                               const byte_t *input,
                               dword_t ilen );

/**
 * \brief          This function finishes the SHA-256 operation, and writes
 *                 the result to the output buffer.
 *
 * \param ctx      The SHA-256 context. This must be initialized
 *                 and have a hash operation started.
 * \param output   The SHA-224 or SHA-256 checksum result.
 *                 This must be a writable buffer of length \c 32 Bytes.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 */
EXP_API int sha256_finish(sha256_context *ctx,
                               byte_t output[32] );

/**
 * \brief          This function processes a single data block within
 *                 the ongoing SHA-256 computation. This function is for
 *                 internal use only.
 *
 * \param ctx      The SHA-256 context. This must be initialized.
 * \param data     The buffer holding one block of data. This must
 *                 be a readable buffer of length \c 64 Bytes.
 *
 * \return         \c 0 on success.
 * \return         A negative error code on failure.
 */
EXP_API int internal_sha256_process(sha256_context *ctx,
                                     const byte_t data[64] );

/**
 * \brief          This function calculates the SHA-224 or SHA-256
 *                 checksum of a buffer.
 *
 *                 The function allocates the context, performs the
 *                 calculation, and frees the context.
 *
 *                 The SHA-256 result is calculated as
 *                 output = SHA-256(input buffer).
 *
 * \param input    The buffer holding the data. This must be a readable
 *                 buffer of length \p ilen Bytes.
 * \param ilen     The length of the input data in Bytes.
 * \param output   The SHA-224 or SHA-256 checksum result. This must
 *                 be a writable buffer of length \c 32 Bytes.
 * \param is224    Determines which function to use. This must be
 *                 either \c 0 for SHA-256, or \c 1 for SHA-224.
 */
EXP_API int sha256(const byte_t *input,
                        dword_t ilen,
                        byte_t output[32],
                        int is224 );


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
EXP_API int sha256_hmac_starts(sha256_context *ctx, const byte_t *key,
	dword_t keylen, int is224);

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
EXP_API int sha256_hmac_update(sha256_context *ctx, const byte_t *input,
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
EXP_API int sha256_hmac_finish(sha256_context *ctx, byte_t output[32]);

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
EXP_API int sha256_hmac_reset(sha256_context *ctx);

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
EXP_API int sha256_hmac(const byte_t *key, dword_t keylen,
                const byte_t *input, dword_t ilen,
				byte_t output[32], int is224);

#if defined(XDK_SUPPORT_TEST)

/**
 * \brief          The SHA-224 and SHA-256 checkup routine.
 *
 * \return         \c 0 on success.
 * \return         \c 1 on failure.
 */
EXP_API int sha256_self_test( int verbose );

#endif /* SELF_TEST */


#ifdef __cplusplus
}
#endif

#endif /* sha2.h */

