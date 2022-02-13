/**
* \file des.h
*
* \brief DES block cipher
*
* \warning   DES is considered a weak cipher and its use constitutes a
*            security risk. We recommend considering stronger ciphers
*            instead.
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
#ifndef DES_H
#define DES_H

#include "../xdkdef.h"


#define DES_ENCRYPT     1
#define DES_DECRYPT     0


#define DES_KEY_SIZE    8

/**
* \brief          DES context structure
*
* \warning        DES is considered a weak cipher and its use constitutes a
*                 security risk. We recommend considering stronger ciphers
*                 instead.
*/
typedef struct des_context
{
	dword_t sk[32];            /*!<  DES subkeys       */
}
des_context;

/**
* \brief          Triple-DES context structure
*/
typedef struct des3_context
{
	dword_t sk[96];            /*!<  3DES subkeys      */
}
des3_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          Initialize DES context
 *
 * \param ctx      DES context to be initialized
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API void des_init(des_context *ctx);

/**
 * \brief          Clear DES context
 *
 * \param ctx      DES context to be cleared
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API void des_free(des_context *ctx);

/**
 * \brief          Initialize Triple-DES context
 *
 * \param ctx      DES3 context to be initialized
 */
EXP_API void des3_init(des3_context *ctx);

/**
 * \brief          Clear Triple-DES context
 *
 * \param ctx      DES3 context to be cleared
 */
EXP_API void des3_free(des3_context *ctx);

/**
 * \brief          Set key parity on the given key to odd.
 *
 *                 DES keys are 56 bits long, but each byte is padded with
 *                 a parity bit to allow verification.
 *
 * \param key      8-byte secret key
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API void des_key_set_parity(byte_t key[DES_KEY_SIZE]);

/**
 * \brief          Check that key parity on the given key is odd.
 *
 *                 DES keys are 56 bits long, but each byte is padded with
 *                 a parity bit to allow verification.
 *
 * \param key      8-byte secret key
 *
 * \return         0 is parity was ok, 1 if parity was not correct.
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API int des_key_check_key_parity(const byte_t key[DES_KEY_SIZE]);

/**
 * \brief          Check that key is not a weak or semi-weak DES key
 *
 * \param key      8-byte secret key
 *
 * \return         0 if no weak key was found, 1 if a weak key was identified.
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API int des_key_check_weak(const byte_t key[DES_KEY_SIZE]);

/**
 * \brief          DES key schedule (56-bit, encryption)
 *
 * \param ctx      DES context to be initialized
 * \param key      8-byte secret key
 *
 * \return         0
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API int des_setkey_enc(des_context *ctx, const byte_t key[DES_KEY_SIZE]);

/**
 * \brief          DES key schedule (56-bit, decryption)
 *
 * \param ctx      DES context to be initialized
 * \param key      8-byte secret key
 *
 * \return         0
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API int des_setkey_dec(des_context *ctx, const byte_t key[DES_KEY_SIZE]);

/**
 * \brief          Triple-DES key schedule (112-bit, encryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      16-byte secret key
 *
 * \return         0
 */
EXP_API int des3_set2key_enc(des3_context *ctx,
                      const byte_t key[DES_KEY_SIZE * 2] );

/**
 * \brief          Triple-DES key schedule (112-bit, decryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      16-byte secret key
 *
 * \return         0
 */
EXP_API int des3_set2key_dec(des3_context *ctx,
                      const byte_t key[DES_KEY_SIZE * 2] );

/**
 * \brief          Triple-DES key schedule (168-bit, encryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      24-byte secret key
 *
 * \return         0
 */
EXP_API int des3_set3key_enc(des3_context *ctx,
                      const byte_t key[DES_KEY_SIZE * 3] );

/**
 * \brief          Triple-DES key schedule (168-bit, decryption)
 *
 * \param ctx      3DES context to be initialized
 * \param key      24-byte secret key
 *
 * \return         0
 */
EXP_API int des3_set3key_dec(des3_context *ctx,
                      const byte_t key[DES_KEY_SIZE * 3] );

/**
 * \brief          DES-ECB block encryption/decryption
 *
 * \param ctx      DES context
 * \param input    64-bit input block
 * \param output   64-bit output block
 *
 * \return         0 if successful
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API int des_crypt_ecb(des_context *ctx,
                    const byte_t input[8],
                    byte_t output[8] );

/**
 * \brief          DES-CBC buffer encryption/decryption
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      DES context
 * \param mode     DES_ENCRYPT or DES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API int des_crypt_cbc(des_context *ctx,
                    int mode,
                    dword_t length,
                    byte_t iv[8],
                    const byte_t *input,
                    byte_t *output );

/**
 * \brief          3DES-ECB block encryption/decryption
 *
 * \param ctx      3DES context
 * \param input    64-bit input block
 * \param output   64-bit output block
 *
 * \return         0 if successful
 */
EXP_API int des3_crypt_ecb(des3_context *ctx,
                     const byte_t input[8],
                     byte_t output[8] );

/**
 * \brief          3DES-CBC buffer encryption/decryption
 *
 * \note           Upon exit, the content of the IV is updated so that you can
 *                 call the function same function again on the following
 *                 block(s) of data and get the same result as if it was
 *                 encrypted in one call. This allows a "streaming" usage.
 *                 If on the other hand you need to retain the contents of the
 *                 IV, you should either save it manually or use the cipher
 *                 module instead.
 *
 * \param ctx      3DES context
 * \param mode     DES_ENCRYPT or DES_DECRYPT
 * \param length   length of the input data
 * \param iv       initialization vector (updated after use)
 * \param input    buffer holding the input data
 * \param output   buffer holding the output data
 *
 * \return         0 if successful, or ERR_DES_INVALID_INPUT_LENGTH
 */
EXP_API int des3_crypt_cbc(des3_context *ctx,
                     int mode,
                     dword_t length,
                     byte_t iv[8],
                     const byte_t *input,
                     byte_t *output );

/**
 * \brief          Internal function for key expansion.
 *                 (Only exposed to allow overriding it,
 *                 see DES_SETKEY_ALT)
 *
 * \param SK       Round keys
 * \param key      Base key
 *
 * \warning        DES is considered a weak cipher and its use constitutes a
 *                 security risk. We recommend considering stronger ciphers
 *                 instead.
 */
EXP_API void des_setkey(dword_t SK[32],
                         const byte_t key[DES_KEY_SIZE] );

#if defined(XDK_SUPPORT_TEST)

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
EXP_API int des_self_test( int verbose );

#endif /* SELF_TEST */


#ifdef __cplusplus
}
#endif

#endif /* des.h */


