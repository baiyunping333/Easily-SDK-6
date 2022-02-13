/**
* \file pem.h
*
* \brief Privacy Enhanced Mail (PEM) decoding
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
#ifndef _OEMPEM_H
#define	_OEMPEM_H

#include "../xdkdef.h"
#include "mpi.h"

#define ERR_PEM_NO_HEADER_FOOTER_PRESENT	(C_ERR - 1)

/**
* \brief       PEM context structure
*/
typedef struct pem_context
{
	byte_t *buf;     /*!< buffer for decoded data             */
	dword_t buflen;          /*!< length of the buffer                */
	byte_t *info;    /*!< buffer for extra header information */
}
pem_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief       PEM context setup
 *
 * \param ctx   context to be initialized
 */
EXP_API void pem_init( pem_context *ctx );

/**
 * \brief       Read a buffer for PEM information and store the resulting
 *              data into the specified context buffers.
 *
 * \param ctx       context to use
 * \param header    header string to seek and expect
 * \param footer    footer string to seek and expect
 * \param data      source data to look in (must be nul-terminated)
 * \param pwd       password for decryption (can be NULL)
 * \param pwdlen    length of password
 * \param use_len   destination for total length used (set after header is
 *                  correctly read, so unless you get
 *                  ERR_PEM_BAD_INPUT_DATA or
 *                  ERR_PEM_NO_HEADER_FOOTER_PRESENT, use_len is
 *                  the length to skip)
 *
 * \note            Attempts to check password correctness by verifying if
 *                  the decrypted text starts with an ASN.1 sequence of
 *                  appropriate length
 *
 * \return          0 on success, or a specific PEM error code
 */
EXP_API int pem_read_buffer(pem_context *ctx, const char *header, const char *footer,
                     const byte_t *data,
                     const byte_t *pwd,
                     dword_t pwdlen, dword_t *use_len );

/**
 * \brief       PEM context memory freeing
 *
 * \param ctx   context to be freed
 */
EXP_API void pem_free(pem_context *ctx);


/**
 * \brief           Write a buffer of PEM information from a DER encoded
 *                  buffer.
 *
 * \param header    header string to write
 * \param footer    footer string to write
 * \param der_data  DER data to write
 * \param der_len   length of the DER data
 * \param buf       buffer to write to
 * \param buf_len   length of output buffer
 * \param olen      total length written / required (if buf_len is not enough)
 *
 * \return          0 on success, or a specific PEM or BASE64 error code. On
 *                  ERR_BASE64_BUFFER_TOO_SMALL olen is the required
 *                  size.
 */
EXP_API int pem_write_buffer(const char *header, const char *footer,
                      const byte_t *der_data, dword_t der_len,
                      byte_t *buf, dword_t buf_len, dword_t *olen );

#ifdef __cplusplus
}
#endif

#endif	/*OEMPEM_H */

