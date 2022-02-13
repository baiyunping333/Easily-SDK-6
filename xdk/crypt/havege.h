/**
* \file havege.h
*
* \brief HAVEGE: HArdware Volatile Entropy Gathering and Expansion
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
#ifndef HAVEGE_H
#define HAVEGE_H

#include "../xdkdef.h"

#define HAVEGE_COLLECT_SIZE 1024

/**
* \brief          HAVEGE state structure
*/
typedef struct havege_state
{
	int PT1, PT2, offset[2];
	int pool[HAVEGE_COLLECT_SIZE];
	int WALK[8192];
}
havege_state;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          HAVEGE initialization
 *
 * \param hs       HAVEGE state to be initialized
 */
EXP_API void havege_init(havege_state *hs);

/**
 * \brief          Clear HAVEGE state
 *
 * \param hs       HAVEGE state to be cleared
 */
EXP_API void havege_free(havege_state *hs);

/**
 * \brief          HAVEGE rand function
 *
 * \param p_rng    A HAVEGE state
 * \param output   Buffer to fill
 * \param len      Length of buffer
 *
 * \return         0
 */
EXP_API int havege_random(void *p_rng, byte_t *output, dword_t len);

EXP_API int havege_rand(void *p_rng);

#ifdef __cplusplus
}
#endif


#endif /* havege.h */


