/**
* \file entropy_poll.h
*
* \brief Platform-specific and custom entropy polling functions
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
#ifndef ENTROPY_POLL_H
#define ENTROPY_POLL_H

#include "../xdkdef.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

	/*
	* Default thresholds for built-in sources, in bytes
	*/
#define ENTROPY_MIN_PLATFORM     32     /**< Minimum for platform source    */
#define ENTROPY_MIN_HAVEGE       32     /**< Minimum for HAVEGE             */
#define ENTROPY_MIN_HARDCLOCK     4     /**< Minimum for timing_hardclock()        */
#if !defined(ENTROPY_MIN_HARDWARE)
#define ENTROPY_MIN_HARDWARE     32     /**< Minimum for the hardware source */
#endif

	/**
	* \brief           Entropy poll callback that provides 0 entropy.
	*/
	EXP_API int null_entropy_poll(void *data,
		unsigned char *output, dword_t len, dword_t *olen);

	/**
	* \brief           Platform-specific entropy poll callback
	*/
	EXP_API int platform_entropy_poll(void *data,
		unsigned char *output, dword_t len, dword_t *olen);

	/**
	* \brief           HAVEGE based entropy poll callback
	*
	* Requires an HAVEGE state as its data pointer.
	*/
	EXP_API int havege_poll(void *data,
		unsigned char *output, dword_t len, dword_t *olen);

	/**
	* \brief           timing_hardclock-based entropy poll callback
	*/
	EXP_API int hardclock_poll(void *data,
		unsigned char *output, dword_t len, dword_t *olen);


#ifdef __cplusplus
}
#endif

#endif /* entropy_poll.h */
