/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc md document

	@module	md.h | interface file

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
#ifndef MDWRAP_H
#define MDWRAP_H

#include "../xdkdef.h"

#define MD_MAX_SIZE         64 


typedef enum {
	MD_NONE = 0,    /**< None. */
	MD_MD2,       /**< The MD2 message digest. */
	MD_MD4,       /**< The MD4 message digest. */
	MD_MD5,       /**< The MD5 message digest. */
	MD_SHA1,      /**< The SHA-1 message digest. */
	MD_SHA224,    /**< The SHA-224 message digest. */
	MD_SHA256,    /**< The SHA-256 message digest. */
	MD_SHA384,    /**< The SHA-384 message digest. */
	MD_SHA512,    /**< The SHA-512 message digest. */
	MD_RIPEMD160, /**< The RIPEMD-160 message digest. */
} md_type_t;


/**
* Message digest information.
* Allows message digest functions to be called in a generic way.
*/
typedef struct _md_info_t
{
	/** Digest identifier */
	md_type_t type;

	/** Name of the message digest */
	const char * name;

	/** Output length of the digest function in bytes */
	int size;

	/** Block length of the digest function in bytes */
	int block_size;
}md_info_t;

#ifdef __cplusplus
extern "C" {
#endif

extern const md_info_t md2_info;

extern const md_info_t md4_info;

extern const md_info_t md5_info;

extern const md_info_t ripemd160_info;

extern const md_info_t sha1_info;

extern const md_info_t sha224_info;
extern const md_info_t sha256_info;

extern const md_info_t sha384_info;
extern const md_info_t sha512_info;

/**
* \brief           This function returns the message-digest information
*                  associated with the given digest type.
*
* \param md_type_t   The type of digest to search for.
*
* \return          The message-digest information associated with \p md_type.
* \return          NULL if the associated message-digest information is not found.
*/
EXP_API const md_info_t* md_info_from_type(md_type_t md);

/**
 * \brief           This function starts a message-digest computation.
 *
 *                  You must call this function after setting up the context
 *                  with md_setup(), and before passing data with
 *                  md_update().
 *
 * \param ctx       The generic message-digest context.
 *
 * \return          \c 0 on success.
 * \return          #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                  failure.
 */
EXP_API int md_starts(const md_info_t *md_info, void *ctx);

/**
 * \brief           This function feeds an input buffer into an ongoing
 *                  message-digest computation.
 *
 *                  You must call md_starts() before calling this
 *                  function. You may call this function multiple times.
 *                  Afterwards, call md_finish().
 *
 * \param ctx       The generic message-digest context.
 * \param input     The buffer holding the input data.
 * \param ilen      The length of the input data.
 *
 * \return          \c 0 on success.
 * \return          #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                  failure.
 */
EXP_API int md_update(const md_info_t *md_info, void *ctx, const byte_t *input, dword_t ilen);

/**
 * \brief           This function finishes the digest operation,
 *                  and writes the result to the output buffer.
 *
 *                  Call this function after a call to md_starts(),
 *                  followed by any number of calls to md_update().
 *                  Afterwards, you may either clear the context with
 *                  md_free(), or call md_starts() to reuse
 *                  the context for another digest operation with the same
 *                  algorithm.
 *
 * \param ctx       The generic message-digest context.
 * \param output    The buffer for the generic message-digest checksum result.
 *
 * \return          \c 0 on success.
 * \return          #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                  failure.
 */
EXP_API int md_finish(const md_info_t *md_info, void *ctx, byte_t *output);

/**
 * \brief          This function calculates the message-digest of a buffer,
 *                 with respect to a configurable message-digest algorithm
 *                 in a single call.
 *
 *                 The result is calculated as
 *                 Output = message_digest(input buffer).
 *
 * \param md_info  The information structure of the message-digest algorithm
 *                 to use.
 * \param input    The buffer holding the data.
 * \param ilen     The length of the input data.
 * \param output   The generic message-digest checksum result.
 *
 * \return         \c 0 on success.
 * \return         #ERR_MD_BAD_INPUT_DATA on parameter-verification
 *                 failure.
 */
EXP_API int md(const md_info_t *md_info, const byte_t *input, dword_t ilen,
        byte_t *output );


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
EXP_API int md_hmac(const md_info_t *md_info, const byte_t *key, dword_t keylen,
                const byte_t *input, dword_t ilen,
                byte_t *output );

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
EXP_API int md_hmac_starts(const md_info_t *md_info, void *ctx, const byte_t *key,
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
EXP_API int md_hmac_update(const md_info_t *md_info, void *ctx, const byte_t *input,
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
EXP_API int md_hmac_finish(const md_info_t *md_info, void *ctx, byte_t *output);

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
EXP_API int md_hmac_reset(const md_info_t *md_info, void *ctx);

EXP_API void* md_alloc(const md_info_t *md_info);

EXP_API void md_free(const md_info_t *md_info, void* ctx);

#ifdef __cplusplus
}
#endif


#endif /* md2.h */
