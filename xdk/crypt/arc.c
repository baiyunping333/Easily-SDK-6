﻿/*
*  An implementation of the ARCFOUR algorithm
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
*  The ARCFOUR algorithm was publicly disclosed on 94/09.
*
*  http://groups.google.com/group/sci.crypt/msg/10a300c9d21afca0
*/

#include "arc.h"

#include "../xdkimp.h"

void arc4_init(arc4_context *ctx)
{
	xmem_zero(ctx, sizeof(arc4_context));
}

void arc4_free(arc4_context *ctx)
{
	if (ctx == NULL)
		return;

	xmem_zero(ctx, sizeof(arc4_context));
}

/*
* ARC4 key schedule
*/
void arc4_setup(arc4_context *ctx, const byte_t *key,
	unsigned int keylen)
{
	int i, j, a;
	unsigned int k;
	byte_t *m;

	ctx->x = 0;
	ctx->y = 0;
	m = ctx->m;

	for (i = 0; i < 256; i++)
		m[i] = (byte_t)i;

	j = k = 0;

	for (i = 0; i < 256; i++, k++)
	{
		if (k >= keylen) k = 0;

		a = m[i];
		j = (j + a + key[k]) & 0xFF;
		m[i] = m[j];
		m[j] = (byte_t)a;
	}
}

/*
* ARC4 cipher function
*/
int arc4_crypt(arc4_context *ctx, dword_t length, const byte_t *input,
	byte_t *output)
{
	int x, y, a, b;
	dword_t i;
	byte_t *m;

	x = ctx->x;
	y = ctx->y;
	m = ctx->m;

	for (i = 0; i < length; i++)
	{
		x = (x + 1) & 0xFF; a = m[x];
		y = (y + a) & 0xFF; b = m[y];

		m[x] = (byte_t)b;
		m[y] = (byte_t)a;

		output[i] = (byte_t)
			(input[i] ^ m[(byte_t)(a + b)]);
	}

	ctx->x = x;
	ctx->y = y;

	return(0);
}


#if defined(XDK_SUPPORT_TEST)
/*
* ARC4 tests vectors as posted by Eric Rescorla in sep. 1994:
*
* http://groups.google.com/group/comp.security.misc/msg/10a300c9d21afca0
*/
static const byte_t arc4_test_key[3][8] =
{
	{ 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF },
	{ 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

static const byte_t arc4_test_pt[3][8] =
{
	{ 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
};

static const byte_t arc4_test_ct[3][8] =
{
	{ 0x75, 0xB7, 0x87, 0x80, 0x99, 0xE0, 0xC5, 0x96 },
	{ 0x74, 0x94, 0xC2, 0xE7, 0x10, 0x4B, 0x08, 0x79 },
	{ 0xDE, 0x18, 0x89, 0x41, 0xA3, 0x37, 0x5D, 0x3A }
};

/*
* Checkup routine
*/
int arc4_self_test(int verbose)
{
	int i, ret = 0;
	byte_t ibuf[8];
	byte_t obuf[8];
	arc4_context ctx;

	arc4_init(&ctx);

	for (i = 0; i < 3; i++)
	{
		if (verbose != 0)
			printf("  ARC4 test #%d: ", i + 1);

		memcpy(ibuf, arc4_test_pt[i], 8);

		arc4_setup(&ctx, arc4_test_key[i], 8);
		arc4_crypt(&ctx, 8, ibuf, obuf);

		if (memcmp(obuf, arc4_test_ct[i], 8) != 0)
		{
			if (verbose != 0)
				printf("failed\n");

			ret = 1;
			goto exit;
		}

		if (verbose != 0)
			printf("passed\n");
	}

	if (verbose != 0)
		printf("\n");

exit:
	arc4_free(&ctx);

	return(ret);
}

#endif /* SELF_TEST */