﻿/**
* \file md5.h
*
* \brief MD5 message digest algorithm (hash function)
*
* \warning   MD5 is considered a weak message digest and its use constitutes a
*            security risk. We recommend considering stronger message
*            digests instead.
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
#ifndef MD5_H
#define MD5_H

#include "../xdkdef.h"

/**
* \brief          MD5 context structure
*
* \warning        MD5 is considered a weak message digest and its use
*                 constitutes a security risk. We recommend considering
*                 stronger message digests instead.
*
*/
typedef struct md5_context
{
	dword_t total[2];          /*!< number of bytes processed  */
	dword_t state[4];          /*!< intermediate digest state  */
	byte_t buffer[64];   /*!< data block being processed */

	byte_t ipad[64];
	byte_t opad[64];
}
md5_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Initialize MD5 context
 *
 * \param ctx      MD5 context to be initialized
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API void md5_init(md5_context *ctx);

/**
 * \brief          Clear MD5 context
 *
 * \param ctx      MD5 context to be cleared
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API void md5_free(md5_context *ctx);

/**
 * \brief          Clone (the state of) an MD5 context
 *
 * \param dst      The destination context
 * \param src      The context to be cloned
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API void md5_clone(md5_context *dst,
                        const md5_context *src );

/**
 * \brief          MD5 context setup
 *
 * \param ctx      context to be initialized
 *
 * \return         0 if successful
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API int md5_starts(md5_context *ctx);

/**
 * \brief          MD5 process buffer
 *
 * \param ctx      MD5 context
 * \param input    buffer holding the data
 * \param ilen     length of the input data
 *
 * \return         0 if successful
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API int md5_update(md5_context *ctx,
                            const byte_t *input,
                            dword_t ilen );

/**
 * \brief          MD5 final digest
 *
 * \param ctx      MD5 context
 * \param output   MD5 checksum result
 *
 * \return         0 if successful
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API int md5_finish(md5_context *ctx,
                            byte_t output[16] );

/**
 * \brief          MD5 process data block (internal use only)
 *
 * \param ctx      MD5 context
 * \param data     buffer holding one block of data
 *
 * \return         0 if successful
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API int internal_md5_process(md5_context *ctx,
                                  const byte_t data[64] );


/**
 * \brief          Output = MD5( input buffer )
 *
 * \param input    buffer holding the data
 * \param ilen     length of the input data
 * \param output   MD5 checksum result
 *
 * \return         0 if successful
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API int md5(const byte_t *input,
                     dword_t ilen,
                     byte_t output[16] );

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
EXP_API int md5_hmac_starts(md5_context *ctx, const byte_t *key,
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
EXP_API int md5_hmac_update(md5_context *ctx, const byte_t *input,
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
EXP_API int md5_hmac_finish(md5_context *ctx, byte_t output[16]);

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
EXP_API int md5_hmac_reset(md5_context *ctx);

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
EXP_API int md5_hmac(const byte_t *key, dword_t keylen,
                const byte_t *input, dword_t ilen,
                byte_t output[16] );

#if defined(XDK_SUPPORT_TEST)

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 *
 * \warning        MD5 is considered a weak message digest and its use
 *                 constitutes a security risk. We recommend considering
 *                 stronger message digests instead.
 *
 */
EXP_API int md5_self_test( int verbose );

#endif /* SELF_TEST */

#ifdef __cplusplus
}
#endif


#endif	/*OEMMD5_H */

