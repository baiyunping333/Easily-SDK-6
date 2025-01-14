﻿/**
*  \brief HAVEGE: HArdware Volatile Entropy Gathering and Expansion
*
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
/*
*  The HAVEGE RNG was designed by Andre Seznec in 2002.
*
*  http://www.irisa.fr/caps/projects/hipsor/publi.php
*
*  Contact: seznec(at)irisa_dot_fr - orocheco(at)irisa_dot_fr
*/

#include "havege.h"
#include "timing.h"

#include "../xdkimp.h"

/* If int isn't capable of storing 2^32 distinct values, the code of this
* module may cause a processor trap or a miscalculation. If int is more
* than 32 bits, the code may not calculate the intended values. 
#if INT_MIN + 1 != -0x7fffffff
#error "The HAVEGE module requires int to be exactly 32 bits, with INT_MIN = -2^31."
#endif
#if UINT_MAX != 0xffffffff
#error "The HAVEGE module requires unsigned to be exactly 32 bits."
#endif */

/* ------------------------------------------------------------------------
* On average, one iteration accesses two 8-word blocks in the havege WALK
* table, and generates 16 words in the RES array.
*
* The data read in the WALK table is updated and permuted after each use.
* The result of the hardware clock counter read is used  for this update.
*
* 25 conditional tests are present.  The conditional tests are grouped in
* two nested  groups of 12 conditional tests and 1 test that controls the
* permutation; on average, there should be 6 tests executed and 3 of them
* should be mispredicted.
* ------------------------------------------------------------------------
*/

#define SWAP(X,Y) { unsigned *T = (X); (X) = (Y); (Y) = T; }

#define TST1_ENTER if( PTEST & 1 ) { PTEST ^= 3; PTEST >>= 1;
#define TST2_ENTER if( PTEST & 1 ) { PTEST ^= 3; PTEST >>= 1;

#define TST1_LEAVE U1++; }
#define TST2_LEAVE U2++; }

#define ONE_ITERATION                                   \
                                                        \
    PTEST = PT1 >> 20;                                  \
                                                        \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
                                                        \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
                                                        \
    PTX = (PT1 >> 18) & 7;                              \
    PT1 &= 0x1FFF;                                      \
    PT2 &= 0x1FFF;                                      \
    CLK = (unsigned) timing_hardclock();        \
                                                        \
    i = 0;                                              \
    A = &WALK[PT1    ]; RES[i++] ^= *A;                 \
    B = &WALK[PT2    ]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 1]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 4]; RES[i++] ^= *D;                 \
                                                        \
    TIN = (*A >> (1)) ^ (*A << (31)) ^ CLK;              \
    *A = (*B >> (2)) ^ (*B << (30)) ^ CLK;              \
    *B = TIN ^ U1;                                       \
    *C = (*C >> (3)) ^ (*C << (29)) ^ CLK;              \
    *D = (*D >> (4)) ^ (*D << (28)) ^ CLK;              \
                                                        \
    A = &WALK[PT1 ^ 2]; RES[i++] ^= *A;                 \
    B = &WALK[PT2 ^ 2]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 3]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 6]; RES[i++] ^= *D;                 \
                                                        \
    if( PTEST & 1 ) SWAP( A, C );                       \
                                                        \
    TIN = (*A >> (5)) ^ (*A << (27)) ^ CLK;              \
    *A = (*B >> (6)) ^ (*B << (26)) ^ CLK;              \
    *B = TIN; CLK = (unsigned) timing_hardclock(); \
    *C = (*C >> (7)) ^ (*C << (25)) ^ CLK;              \
    *D = (*D >> (8)) ^ (*D << (24)) ^ CLK;              \
                                                        \
    A = &WALK[PT1 ^ 4];                                 \
    B = &WALK[PT2 ^ 1];                                 \
                                                        \
    PTEST = PT2 >> 1;                                   \
                                                        \
    PT2 = (RES[(i - 8) ^ PTY] ^ WALK[PT2 ^ PTY ^ 7]);   \
    PT2 = ((PT2 & 0x1FFF) & (~8)) ^ ((PT1 ^ 8) & 0x8);  \
    PTY = (PT2 >> 10) & 7;                              \
                                                        \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
                                                        \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
                                                        \
    C = &WALK[PT1 ^ 5];                                 \
    D = &WALK[PT2 ^ 5];                                 \
                                                        \
    RES[i++] ^= *A;                                     \
    RES[i++] ^= *B;                                     \
    RES[i++] ^= *C;                                     \
    RES[i++] ^= *D;                                     \
                                                        \
    TIN = (*A >> ( 9)) ^ (*A << (23)) ^ CLK;             \
    *A = (*B >> (10)) ^ (*B << (22)) ^ CLK;             \
    *B = TIN ^ U2;                                       \
    *C = (*C >> (11)) ^ (*C << (21)) ^ CLK;             \
    *D = (*D >> (12)) ^ (*D << (20)) ^ CLK;             \
                                                        \
    A = &WALK[PT1 ^ 6]; RES[i++] ^= *A;                 \
    B = &WALK[PT2 ^ 3]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 7]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 7]; RES[i++] ^= *D;                 \
                                                        \
    TIN = (*A >> (13)) ^ (*A << (19)) ^ CLK;             \
    *A = (*B >> (14)) ^ (*B << (18)) ^ CLK;             \
    *B = TIN;                                            \
    *C = (*C >> (15)) ^ (*C << (17)) ^ CLK;             \
    *D = (*D >> (16)) ^ (*D << (16)) ^ CLK;             \
                                                        \
    PT1 = ( RES[( i - 8 ) ^ PTX] ^                      \
            WALK[PT1 ^ PTX ^ 7] ) & (~1);               \
    PT1 ^= (PT2 ^ 0x10) & 0x10;                         \
                                                        \
    for( n++, i = 0; i < 16; i++ )                      \
        POOL[n % HAVEGE_COLLECT_SIZE] ^= RES[i];

/*
* Entropy gathering function
*/
static void havege_fill(havege_state *hs)
{
	unsigned i, n = 0;
	unsigned  U1, U2, *A, *B, *C, *D;
	unsigned PT1, PT2, *WALK, *POOL, RES[16];
	unsigned PTX, PTY, CLK, PTEST, TIN;

	WALK = (unsigned *)hs->WALK;
	POOL = (unsigned *)hs->pool;
	PT1 = hs->PT1;
	PT2 = hs->PT2;

	PTX = U1 = 0;
	PTY = U2 = 0;

	(void)PTX;

	xmem_zero(RES, sizeof(RES));

	while (n < HAVEGE_COLLECT_SIZE * 4)
	{
		ONE_ITERATION
		ONE_ITERATION
		ONE_ITERATION
		ONE_ITERATION
	}

	hs->PT1 = PT1;
	hs->PT2 = PT2;

	hs->offset[0] = 0;
	hs->offset[1] = HAVEGE_COLLECT_SIZE / 2;
}

/*
* HAVEGE initialization
*/
void havege_init(havege_state *hs)
{
	xmem_zero(hs, sizeof(havege_state));

	havege_fill(hs);
}

void havege_free(havege_state *hs)
{
	if (hs == NULL)
		return;

	xmem_zero(hs, sizeof(havege_state));
}

/*
* HAVEGE rand function
*/
int havege_rand(void *p_rng)
{
	int ret;
	havege_state *hs = (havege_state *)p_rng;

	if (hs->offset[1] >= HAVEGE_COLLECT_SIZE)
		havege_fill(hs);

	ret = hs->pool[hs->offset[0]++];
	ret ^= hs->pool[hs->offset[1]++];

	return(ret);
}

/*
* HAVEGE rand function
*/
int havege_random(void *p_rng, byte_t *buf, dword_t len)
{
	int val;
	dword_t use_len;
	havege_state *hs = (havege_state *)p_rng;
	byte_t *p = buf;

	while (len > 0)
	{
		use_len = len;
		if (use_len > sizeof(int))
			use_len = sizeof(int);

		if (hs->offset[1] >= HAVEGE_COLLECT_SIZE)
			havege_fill(hs);

		val = hs->pool[hs->offset[0]++];
		val ^= hs->pool[hs->offset[1]++];

		xmem_copy(p, &val, use_len);

		len -= use_len;
		p += use_len;
	}

	return(0);
}
