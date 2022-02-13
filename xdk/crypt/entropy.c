/*
*  Entropy accumulator implementation
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
#include "havege.h"

#include "../xdkimp.h"

#define ENTROPY_MAX_LOOP    256     /**< Maximum amount to loop before error */

void entropy_init(entropy_context *ctx)
{
	ctx->source_count = 0;
	memset(ctx->source, 0, sizeof(ctx->source));

	ctx->accumulator_started = 0;
#if defined(_OS_64)
	sha512_init(&ctx->accumulator);
#else
	sha256_init(&ctx->accumulator);
#endif

	havege_init(&ctx->havege_data);

	/* Reminder: Update ENTROPY_HAVE_STRONG in the test files
	*           when adding more strong entropy sources here. */

	entropy_add_source(ctx, null_entropy_poll, NULL,
		1, ENTROPY_SOURCE_STRONG);

	entropy_add_source(ctx, platform_entropy_poll, NULL,
		ENTROPY_MIN_PLATFORM,
		ENTROPY_SOURCE_STRONG);

	entropy_add_source(ctx, hardclock_poll, NULL,
		ENTROPY_MIN_HARDCLOCK,
		ENTROPY_SOURCE_WEAK);

	entropy_add_source(ctx, havege_poll, &ctx->havege_data,
		ENTROPY_MIN_HAVEGE,
		ENTROPY_SOURCE_STRONG);
}

void entropy_free(entropy_context *ctx)
{
	havege_free(&ctx->havege_data);

#if defined(_OS_64)
	sha512_free(&ctx->accumulator);
#else
	sha256_free(&ctx->accumulator);
#endif

	ctx->source_count = 0;
	xmem_zero(ctx->source, sizeof(ctx->source));
	ctx->accumulator_started = 0;
}

int entropy_add_source(entropy_context *ctx,
	entropy_f_source_ptr f_source, void *p_source,
	dword_t threshold, int strong)
{
	int idx, ret = 0;

	idx = ctx->source_count;
	if (idx >= ENTROPY_MAX_SOURCES)
	{
		ret = ERR_ENTROPY_MAX_SOURCES;
		goto exit;
	}

	ctx->source[idx].f_source = f_source;
	ctx->source[idx].p_source = p_source;
	ctx->source[idx].threshold = threshold;
	ctx->source[idx].strong = strong;

	ctx->source_count++;

exit:

	return(ret);
}

/*
* Entropy accumulator update
*/
static int entropy_update(entropy_context *ctx, unsigned char source_id,
	const unsigned char *data, dword_t len)
{
	unsigned char header[2];
	unsigned char tmp[ENTROPY_BLOCK_SIZE];
	dword_t use_len = len;
	const unsigned char *p = data;
	int ret = 0;

	if (use_len > ENTROPY_BLOCK_SIZE)
	{
#if defined(_OS_64)
		if ((ret = sha512(data, len, tmp, 0)) != 0)
			goto cleanup;
#else
		if ((ret = sha256(data, len, tmp, 0)) != 0)
			goto cleanup;
#endif
		p = tmp;
		use_len = ENTROPY_BLOCK_SIZE;
	}

	header[0] = source_id;
	header[1] = use_len & 0xFF;

	/*
	* Start the accumulator if this has not already happened. Note that
	* it is sufficient to start the accumulator here only because all calls to
	* gather entropy eventually execute this code.
	*/
#if defined(_OS_64)
	if (ctx->accumulator_started == 0 &&
		(ret = sha512_starts(&ctx->accumulator, 0)) != 0)
		goto cleanup;
	else
		ctx->accumulator_started = 1;
	if ((ret = sha512_update(&ctx->accumulator, header, 2)) != 0)
		goto cleanup;
	ret = sha512_update(&ctx->accumulator, p, use_len);
#else
	if (ctx->accumulator_started == 0 &&
		(ret = sha256_starts(&ctx->accumulator, 0)) != 0)
		goto cleanup;
	else
		ctx->accumulator_started = 1;
	if ((ret = sha256_update(&ctx->accumulator, header, 2)) != 0)
		goto cleanup;
	ret = sha256_update(&ctx->accumulator, p, use_len);
#endif

cleanup:
	xmem_zero(tmp, sizeof(tmp));

	return(ret);
}

int entropy_update_manual(entropy_context *ctx,
	const unsigned char *data, dword_t len)
{
	int ret;

	ret = entropy_update(ctx, ENTROPY_SOURCE_MANUAL, data, len);

	return(ret);
}

/*
* Run through the different sources to add entropy to our accumulator
*/
static int entropy_gather_internal(entropy_context *ctx)
{
	int ret, i, have_one_strong = 0;
	unsigned char buf[ENTROPY_MAX_GATHER];
	dword_t olen;

	if (ctx->source_count == 0)
		return(ERR_ENTROPY_NO_SOURCES_DEFINED);

	/*
	* Run through our entropy sources
	*/
	for (i = 0; i < ctx->source_count; i++)
	{
		if (ctx->source[i].strong == ENTROPY_SOURCE_STRONG)
			have_one_strong = 1;

		olen = 0;
		if ((ret = ctx->source[i].f_source(ctx->source[i].p_source,
			buf, ENTROPY_MAX_GATHER, &olen)) != 0)
		{
			goto cleanup;
		}

		/*
		* Add if we actually gathered something
		*/
		if (olen > 0)
		{
			if ((ret = entropy_update(ctx, (unsigned char)i,
				buf, olen)) != 0)
				return(ret);
			ctx->source[i].size += olen;
		}
	}

	if (have_one_strong == 0)
		ret = ERR_ENTROPY_NO_STRONG_SOURCE;

cleanup:
	xmem_zero(buf, sizeof(buf));

	return(ret);
}

/*
* Thread-safe wrapper for entropy_gather_internal()
*/
int entropy_gather(entropy_context *ctx)
{
	int ret;

	ret = entropy_gather_internal(ctx);

	return(ret);
}

int entropy_func(void *data, unsigned char *output, dword_t len)
{
	int ret, count = 0, i, done;
	entropy_context *ctx = (entropy_context *)data;
	unsigned char buf[ENTROPY_BLOCK_SIZE];

	if (len > ENTROPY_BLOCK_SIZE)
		return(ERR_ENTROPY_SOURCE_FAILED);

	/*
	* Always gather extra entropy before a call
	*/
	do
	{
		if (count++ > ENTROPY_MAX_LOOP)
		{
			ret = ERR_ENTROPY_SOURCE_FAILED;
			goto exit;
		}

		if ((ret = entropy_gather_internal(ctx)) != 0)
			goto exit;

		done = 1;
		for (i = 0; i < ctx->source_count; i++)
			if (ctx->source[i].size < ctx->source[i].threshold)
				done = 0;
	} while (!done);

	memset(buf, 0, ENTROPY_BLOCK_SIZE);

#if defined(_OS_64)
	/*
	* Note that at this stage it is assumed that the accumulator was started
	* in a previous call to entropy_update(). If this is not guaranteed, the
	* code below will fail.
	*/
	if ((ret = sha512_finish(&ctx->accumulator, buf)) != 0)
		goto exit;

	/*
	* Reset accumulator and counters and recycle existing entropy
	*/
	sha512_free(&ctx->accumulator);
	sha512_init(&ctx->accumulator);
	if ((ret = sha512_starts(&ctx->accumulator, 0)) != 0)
		goto exit;
	if ((ret = sha512_update(&ctx->accumulator, buf,
		ENTROPY_BLOCK_SIZE)) != 0)
		goto exit;

	/*
	* Perform second SHA-512 on entropy
	*/
	if ((ret = sha512(buf, ENTROPY_BLOCK_SIZE,
		buf, 0)) != 0)
		goto exit;
#else /* ENTROPY_SHA512_ACCUMULATOR */
	if ((ret = sha256_finish(&ctx->accumulator, buf)) != 0)
		goto exit;

	/*
	* Reset accumulator and counters and recycle existing entropy
	*/
	sha256_free(&ctx->accumulator);
	sha256_init(&ctx->accumulator);
	if ((ret = sha256_starts(&ctx->accumulator, 0)) != 0)
		goto exit;
	if ((ret = sha256_update(&ctx->accumulator, buf,
		ENTROPY_BLOCK_SIZE)) != 0)
		goto exit;

	/*
	* Perform second SHA-256 on entropy
	*/
	if ((ret = sha256(buf, ENTROPY_BLOCK_SIZE,
		buf, 0)) != 0)
		goto exit;
#endif /* ENTROPY_SHA512_ACCUMULATOR */

	for (i = 0; i < ctx->source_count; i++)
		ctx->source[i].size = 0;

	xmem_copy(output, buf, len);

	ret = 0;

exit:
	xmem_zero(buf, sizeof(buf));

	return(ret);
}


#if defined(XDK_SUPPORT_TEST)
/*
* Dummy source function
*/
static int entropy_dummy_source(void *data, unsigned char *output,
	dword_t len, dword_t *olen)
{
	((void)data);

	memset(output, 0x2a, len);
	*olen = len;

	return(0);
}

#if defined(ENTROPY_HARDWARE_ALT)

static int entropy_source_self_test_gather(unsigned char *buf, dword_t buf_len)
{
	int ret = 0;
	dword_t entropy_len = 0;
	dword_t olen = 0;
	dword_t attempts = buf_len;

	while (attempts > 0 && entropy_len < buf_len)
	{
		if ((ret = hardware_poll(NULL, buf + entropy_len,
			buf_len - entropy_len, &olen)) != 0)
			return(ret);

		entropy_len += olen;
		attempts--;
	}

	if (entropy_len < buf_len)
	{
		ret = 1;
	}

	return(ret);
}


static int entropy_source_self_test_check_bits(const unsigned char *buf,
	dword_t buf_len)
{
	unsigned char set = 0xFF;
	unsigned char unset = 0x00;
	dword_t i;

	for (i = 0; i < buf_len; i++)
	{
		set &= buf[i];
		unset |= buf[i];
	}

	return(set == 0xFF || unset == 0x00);
}

/*
* A test to ensure hat the entropy sources are functioning correctly
* and there is no obvious failure. The test performs the following checks:
*  - The entropy source is not providing only 0s (all bits unset) or 1s (all
*    bits set).
*  - The entropy source is not providing values in a pattern. Because the
*    hardware could be providing data in an arbitrary length, this check polls
*    the hardware entropy source twice and compares the result to ensure they
*    are not equal.
*  - The error code returned by the entropy source is not an error.
*/
int entropy_source_self_test(int verbose)
{
	int ret = 0;
	unsigned char buf0[2 * sizeof(unsigned long long int)];
	unsigned char buf1[2 * sizeof(unsigned long long int)];

	if (verbose != 0)
		printf("  ENTROPY_BIAS test: ");

	memset(buf0, 0x00, sizeof(buf0));
	memset(buf1, 0x00, sizeof(buf1));

	if ((ret = entropy_source_self_test_gather(buf0, sizeof(buf0))) != 0)
		goto cleanup;
	if ((ret = entropy_source_self_test_gather(buf1, sizeof(buf1))) != 0)
		goto cleanup;

	/* Make sure that the returned values are not all 0 or 1 */
	if ((ret = entropy_source_self_test_check_bits(buf0, sizeof(buf0))) != 0)
		goto cleanup;
	if ((ret = entropy_source_self_test_check_bits(buf1, sizeof(buf1))) != 0)
		goto cleanup;

	/* Make sure that the entropy source is not returning values in a
	* pattern */
	ret = memcmp(buf0, buf1, sizeof(buf0)) == 0;

cleanup:
	if (verbose != 0)
	{
		if (ret != 0)
			printf("failed\n");
		else
			printf("passed\n");

		printf("\n");
	}

	return(ret != 0);
}

#endif /* ENTROPY_HARDWARE_ALT */

/*
* The actual entropy quality is hard to test, but we can at least
* test that the functions don't cause errors and write the correct
* amount of data to buffers.
*/
int entropy_self_test(int verbose)
{
	int ret = 1;

	entropy_context ctx;
	unsigned char buf[ENTROPY_BLOCK_SIZE] = { 0 };
	unsigned char acc[ENTROPY_BLOCK_SIZE] = { 0 };
	dword_t i, j;

	if (verbose != 0)
		printf("  ENTROPY test: ");

	entropy_init(&ctx);

	/* First do a gather to make sure we have default sources */
	if ((ret = entropy_gather(&ctx)) != 0)
		goto cleanup;

	ret = entropy_add_source(&ctx, entropy_dummy_source, NULL, 16,
		ENTROPY_SOURCE_WEAK);
	if (ret != 0)
		goto cleanup;

	if ((ret = entropy_update_manual(&ctx, buf, sizeof(buf))) != 0)
		goto cleanup;

	/*
	* To test that entropy_func writes correct number of bytes:
	* - use the whole buffer and rely on ASan to detect overruns
	* - collect entropy 8 times and OR the result in an accumulator:
	*   any byte should then be 0 with probably 2^(-64), so requiring
	*   each of the 32 or 64 bytes to be non-zero has a false failure rate
	*   of at most 2^(-58) which is acceptable.
	*/
	for (i = 0; i < 8; i++)
	{
		if ((ret = entropy_func(&ctx, buf, sizeof(buf))) != 0)
			goto cleanup;

		for (j = 0; j < sizeof(buf); j++)
			acc[j] |= buf[j];
	}

	for (j = 0; j < sizeof(buf); j++)
	{
		if (acc[j] == 0)
		{
			ret = 1;
			goto cleanup;
		}
	}

cleanup:
	entropy_free(&ctx);

	if (verbose != 0)
	{
		if (ret != 0)
			printf("failed\n");
		else
			printf("passed\n");

		printf("\n");
	}

	return(ret != 0);
}
#endif /* SELF_TEST */
