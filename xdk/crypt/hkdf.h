/**
* \file hkdf.h
*
* \brief   This file contains the HKDF interface.
*
*          The HMAC-based Extract-and-Expand Key Derivation Function (HKDF) is
*          specified by RFC 5869.
*/
/*
*  Copyright (C) 2016-2019, ARM Limited, All Rights Reserved
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
#ifndef HKDF_H
#define HKDF_H

#include "mdwrap.h"

/**
*  \name HKDF Error codes
*  \{
*/
#define ERR_HKDF_BAD_INPUT_DATA  -0x5F80  /**< Bad input parameters to function. */
/* \} name */

#ifdef __cplusplus
extern "C" {
#endif

	/**
	*  \brief  This is the HMAC-based Extract-and-Expand Key Derivation Function
	*          (HKDF).
	*
	*  \param  md        A hash function; md.size denotes the length of the hash
	*                    function output in bytes.
	*  \param  salt      An optional salt value (a non-secret random value);
	*                    if the salt is not provided, a string of all zeros of
	*                    md.size length is used as the salt.
	*  \param  salt_len  The length in bytes of the optional \p salt.
	*  \param  ikm       The input keying material.
	*  \param  ikm_len   The length in bytes of \p ikm.
	*  \param  info      An optional context and application specific information
	*                    string. This can be a zero-length string.
	*  \param  info_len  The length of \p info in bytes.
	*  \param  okm       The output keying material of \p okm_len bytes.
	*  \param  okm_len   The length of the output keying material in bytes. This
	*                    must be less than or equal to 255 * md.size bytes.
	*
	*  \return 0 on success.
	*  \return #ERR_HKDF_BAD_INPUT_DATA when the parameters are invalid.
	*  \return An ERR_MD_* error for errors returned from the underlying
	*          MD layer.
	*/
	EXP_API int hkdf(const md_info_t *md, const unsigned char *salt,
		dword_t salt_len, const unsigned char *ikm, dword_t ikm_len,
		const unsigned char *info, dword_t info_len,
		unsigned char *okm, dword_t okm_len);

	/**
	*  \brief  Take the input keying material \p ikm and extract from it a
	*          fixed-length pseudorandom key \p prk.
	*
	*  \warning    This function should only be used if the security of it has been
	*              studied and established in that particular context (eg. TLS 1.3
	*              key schedule). For standard HKDF security guarantees use
	*              \c hkdf instead.
	*
	*  \param       md        A hash function; md.size denotes the length of the
	*                         hash function output in bytes.
	*  \param       salt      An optional salt value (a non-secret random value);
	*                         if the salt is not provided, a string of all zeros
	*                         of md.size length is used as the salt.
	*  \param       salt_len  The length in bytes of the optional \p salt.
	*  \param       ikm       The input keying material.
	*  \param       ikm_len   The length in bytes of \p ikm.
	*  \param[out]  prk       A pseudorandom key of at least md.size bytes.
	*
	*  \return 0 on success.
	*  \return #ERR_HKDF_BAD_INPUT_DATA when the parameters are invalid.
	*  \return An ERR_MD_* error for errors returned from the underlying
	*          MD layer.
	*/
	EXP_API int hkdf_extract(const md_info_t *md,
		const unsigned char *salt, dword_t salt_len,
		const unsigned char *ikm, dword_t ikm_len,
		unsigned char *prk);

	/**
	*  \brief  Expand the supplied \p prk into several additional pseudorandom
	*          keys, which is the output of the HKDF.
	*
	*  \warning    This function should only be used if the security of it has been
	*              studied and established in that particular context (eg. TLS 1.3
	*              key schedule). For standard HKDF security guarantees use
	*              \c hkdf instead.
	*
	*  \param  md        A hash function; md.size denotes the length of the hash
	*                    function output in bytes.
	*  \param  prk       A pseudorandom key of at least md.size bytes. \p prk is
	*                    usually the output from the HKDF extract step.
	*  \param  prk_len   The length in bytes of \p prk.
	*  \param  info      An optional context and application specific information
	*                    string. This can be a zero-length string.
	*  \param  info_len  The length of \p info in bytes.
	*  \param  okm       The output keying material of \p okm_len bytes.
	*  \param  okm_len   The length of the output keying material in bytes. This
	*                    must be less than or equal to 255 * md.size bytes.
	*
	*  \return 0 on success.
	*  \return #ERR_HKDF_BAD_INPUT_DATA when the parameters are invalid.
	*  \return An ERR_MD_* error for errors returned from the underlying
	*          MD layer.
	*/
	EXP_API int hkdf_expand(const md_info_t *md, const unsigned char *prk,
		dword_t prk_len, const unsigned char *info,
		dword_t info_len, unsigned char *okm, dword_t okm_len);

#if defined(XDK_SUPPORT_TEST)
	EXP_API void test_hkdf(int verbos);
#endif

#ifdef __cplusplus
}
#endif

#endif /* hkdf.h */
