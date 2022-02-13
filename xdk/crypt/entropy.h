/**
* \file entropy.h
*
* \brief Entropy accumulator implementation
*/
/*
*  Copyright (C) 2006-2016, ARM Limited, All Rights Reserved
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
#ifndef ENTROPY_H
#define ENTROPY_H

#include "../xdkdef.h"

#include <stddef.h>

#include "sha4.h"
#include "sha2.h"
#include "havege.h"

#define ERR_ENTROPY_SOURCE_FAILED                 -0x003C  /**< Critical entropy source failure. */
#define ERR_ENTROPY_MAX_SOURCES                   -0x003E  /**< No more sources can be added. */
#define ERR_ENTROPY_NO_SOURCES_DEFINED            -0x0040  /**< No sources have been added to poll. */
#define ERR_ENTROPY_NO_STRONG_SOURCE              -0x003D  /**< No strong sources have been added to poll. */
#define ERR_ENTROPY_FILE_IO_ERROR                 -0x003F  /**< Read/write error in file. */

/**
* \name SECTION: Module settings
*
* The configuration options you can set for this module are in this section.
* Either change them in config.h or define them on the compiler command line.
* \{
*/

#define ENTROPY_MAX_SOURCES     20      /**< Maximum number of sources supported */
#define ENTROPY_MAX_GATHER      128     /**< Maximum amount requested from entropy sources */

/* \} name SECTION: Module settings */

#if defined(_OS_64)
#define ENTROPY_BLOCK_SIZE      64      /**< Block size of entropy accumulator (SHA-512) */
#else
#define ENTROPY_BLOCK_SIZE      32      /**< Block size of entropy accumulator (SHA-256) */
#endif

#define ENTROPY_MAX_SEED_SIZE   1024    /**< Maximum size of seed we read from seed file */
#define ENTROPY_SOURCE_MANUAL   ENTROPY_MAX_SOURCES

#define ENTROPY_SOURCE_STRONG   1       /**< Entropy source is strong   */
#define ENTROPY_SOURCE_WEAK     0       /**< Entropy source is weak     */

#ifdef __cplusplus
extern "C" {
#endif

	/**
	* \brief           Entropy poll callback pointer
	*
	* \param data      Callback-specific data pointer
	* \param output    Data to fill
	* \param len       Maximum size to provide
	* \param olen      The actual amount of bytes put into the buffer (Can be 0)
	*
	* \return          0 if no critical failures occurred,
	*                  ERR_ENTROPY_SOURCE_FAILED otherwise
	*/
	typedef int(*entropy_f_source_ptr)(void *data, unsigned char *output, dword_t len,
		dword_t *olen);

	/**
	* \brief           Entropy source state
	*/
	typedef struct entropy_source_state
	{
		entropy_f_source_ptr    f_source;   /**< The entropy source callback */
		void *          p_source;   /**< The callback data pointer */
		dword_t          size;       /**< Amount received in bytes */
		dword_t          threshold;  /**< Minimum bytes required before release */
		int             strong;     /**< Is the source strong? */
	}
	entropy_source_state;

	/**
	* \brief           Entropy context structure
	*/
	typedef struct entropy_context
	{
		int accumulator_started;
#if defined(_OS_64)
		sha512_context  accumulator;
#else
		sha256_context  accumulator;
#endif
		int             source_count;
		entropy_source_state    source[ENTROPY_MAX_SOURCES];

		havege_state    havege_data;
	}
	entropy_context;

	/**
	* \brief           Initialize the context
	*
	* \param ctx       Entropy context to initialize
	*/
	EXP_API void entropy_init(entropy_context *ctx);

	/**
	* \brief           Free the data in the context
	*
	* \param ctx       Entropy context to free
	*/
	EXP_API void entropy_free(entropy_context *ctx);

	/**
	* \brief           Adds an entropy source to poll
	*                  (Thread-safe if THREADING_C is enabled)
	*
	* \param ctx       Entropy context
	* \param f_source  Entropy function
	* \param p_source  Function data
	* \param threshold Minimum required from source before entropy is released
	*                  ( with entropy_func() ) (in bytes)
	* \param strong    ENTROPY_SOURCE_STRONG or
	*                  ENTROPY_SOURCE_WEAK.
	*                  At least one strong source needs to be added.
	*                  Weaker sources (such as the cycle counter) can be used as
	*                  a complement.
	*
	* \return          0 if successful or ERR_ENTROPY_MAX_SOURCES
	*/
	EXP_API int entropy_add_source(entropy_context *ctx,
		entropy_f_source_ptr f_source, void *p_source,
		dword_t threshold, int strong);

	/**
	* \brief           Trigger an extra gather poll for the accumulator
	*                  (Thread-safe if THREADING_C is enabled)
	*
	* \param ctx       Entropy context
	*
	* \return          0 if successful, or ERR_ENTROPY_SOURCE_FAILED
	*/
	EXP_API int entropy_gather(entropy_context *ctx);

	/**
	* \brief           Retrieve entropy from the accumulator
	*                  (Maximum length: ENTROPY_BLOCK_SIZE)
	*                  (Thread-safe if THREADING_C is enabled)
	*
	* \param data      Entropy context
	* \param output    Buffer to fill
	* \param len       Number of bytes desired, must be at most ENTROPY_BLOCK_SIZE
	*
	* \return          0 if successful, or ERR_ENTROPY_SOURCE_FAILED
	*/
	EXP_API int entropy_func(void *data, unsigned char *output, dword_t len);

	/**
	* \brief           Add data to the accumulator manually
	*                  (Thread-safe if THREADING_C is enabled)
	*
	* \param ctx       Entropy context
	* \param data      Data to add
	* \param len       Length of data
	*
	* \return          0 if successful
	*/
	EXP_API int entropy_update_manual(entropy_context *ctx,
		const unsigned char *data, dword_t len);

#if defined(XDK_SUPPORT_TEST)
	/**
	* \brief          Checkup routine
	*
	*                 This module self-test also calls the entropy self-test,
	*                 entropy_source_self_test();
	*
	* \return         0 if successful, or 1 if a test failed
	*/
	EXP_API int entropy_self_test(int verbose);
#endif /* SELF_TEST */

#ifdef __cplusplus
}
#endif

#endif /* entropy.h */