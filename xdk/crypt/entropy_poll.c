/*
*  Platform-specific and custom entropy polling functions
*
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

#include "entropy.h"
#include "entropy_poll.h"
#include "timing.h"
#include "havege.h"

#include "../xdkimp.h"


int platform_entropy_poll(void *data,
	unsigned char *output, dword_t len, dword_t *olen)
{
	if (system_random(output, len))
	{
		*olen = len;
		return C_OK;
	}
	else
	{
		*olen = 0;
		return C_ERR;
	}
}

int null_entropy_poll(void *data,
	unsigned char *output, dword_t len, dword_t *olen)
{
	((void)data);
	((void)output);
	*olen = 0;

	if (len < sizeof(unsigned char))
		return(0);

	*olen = sizeof(unsigned char);

	return(0);
}

int hardclock_poll(void *data,
	unsigned char *output, dword_t len, dword_t *olen)
{
	unsigned long timer = timing_hardclock();
	((void)data);
	*olen = 0;

	if (len < sizeof(unsigned long))
		return(0);

	memcpy(output, &timer, sizeof(unsigned long));
	*olen = sizeof(unsigned long);

	return(0);
}

int havege_poll(void *data,
	unsigned char *output, dword_t len, dword_t *olen)
{
	havege_state *hs = (havege_state *)data;
	*olen = 0;

	if (havege_random(hs, output, len) != 0)
		return(C_ERR);

	*olen = len;

	return(0);
}


