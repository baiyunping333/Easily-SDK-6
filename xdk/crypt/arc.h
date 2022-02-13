/**
* \file arc4.h
*
* \brief The ARCFOUR stream cipher
*
* \warning   ARC4 is considered a weak cipher and its use constitutes a
*            security risk. We recommend considering stronger ciphers instead.
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
*
*/
#ifndef ARC4_H
#define ARC4_H

#include "../xdkdef.h"


/**
* \brief     ARC4 context structure
*
* \warning   ARC4 is considered a weak cipher and its use constitutes a
*            security risk. We recommend considering stronger ciphers instead.
*
*/
typedef struct arc4_context
{
	int x;                      /*!< permutation index */
	int y;                      /*!< permutation index */
	byte_t m[256];       /*!< permutation table */
}
arc4_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Initialize ARC4 context
 *
 * \param ctx      ARC4 context to be initialized
 *
 * \warning        ARC4 is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 *
 */
EXP_API void arc4_init( arc4_context *ctx );

/**
 * \brief          Clear ARC4 context
 *
 * \param ctx      ARC4 context to be cleared
 *
 * \warning        ARC4 is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 *
 */
EXP_API void arc4_free(arc4_context *ctx);

/**
 * \brief          ARC4 key schedule
 *
 * \param ctx      ARC4 context to be setup
 * \param key      the secret key
 * \param keylen   length of the key, in bytes
 *
 * \warning        ARC4 is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 *
 */
EXP_API void arc4_setup(arc4_context *ctx, const byte_t *key,
                 unsigned int keylen );

/**
 * \brief          ARC4 cipher function
 *
 * \param ctx      ARC4 context
 * \param length   length of the input data
 * \param input    buffer holding the input data
 * \param output   buffer for the output data
 *
 * \return         0 if successful
 *
 * \warning        ARC4 is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 *
 */
EXP_API int arc4_crypt(arc4_context *ctx, dword_t length, const byte_t *input,
                byte_t *output );

#if defined(XDK_SUPPORT_TEST)

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 *
 * \warning        ARC4 is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 *
 */
EXP_API int arc4_self_test( int verbose );

#endif /* SELF_TEST */

#ifdef __cplusplus
}
#endif

#endif /* arc4.h */

