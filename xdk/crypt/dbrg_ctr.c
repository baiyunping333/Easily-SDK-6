/*
*  CTR_DRBG implementation based on AES-256 (NIST SP 800-90)
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
*  The NIST SP 800-90 DRBGs are described in the following publication.
*
*  http://csrc.nist.gov/publications/nistpubs/800-90/SP800-90revised_March2007.pdf
*/

#include "dbrg_ctr.h"
#include "timing.h"

#include "../xdkimp.h"

/*
* CTR_DRBG context initialization
*/
void ctr_drbg_init(ctr_drbg_context *ctx)
{
	xmem_zero(ctx, sizeof(ctr_drbg_context));
}

void ctr_drbg_free(ctr_drbg_context *ctx)
{
	if (ctx == NULL)
		return;

	aes_free(&ctx->aes_ctx);
	xmem_zero(ctx, sizeof(ctr_drbg_context));
}

void ctr_drbg_set_prediction_resistance(ctr_drbg_context *ctx, int resistance)
{
	ctx->prediction_resistance = resistance;
}

void ctr_drbg_set_entropy_len(ctr_drbg_context *ctx, dword_t len)
{
	ctx->entropy_len = len;
}

void ctr_drbg_set_reseed_interval(ctr_drbg_context *ctx, int interval)
{
	ctx->reseed_interval = interval;
}

static int block_cipher_df(byte_t *output,
	const byte_t *data, dword_t data_len)
{
	byte_t buf[CTR_DRBG_MAX_SEED_INPUT + CTR_DRBG_BLOCKSIZE + 16];
	byte_t tmp[CTR_DRBG_SEEDLEN];
	byte_t key[CTR_DRBG_KEYSIZE];
	byte_t chain[CTR_DRBG_BLOCKSIZE];
	byte_t *p, *iv;
	aes_context aes_ctx;
	int ret = 0;

	int i, j;
	dword_t buf_len, use_len;

	if (data_len > CTR_DRBG_MAX_SEED_INPUT)
	{
		set_last_error(_T("block_cipher_df"), _T("ERR_CTR_DRBG_INPUT_TOO_BIG"), -1);
		return C_ERR;
	}

	xmem_zero(buf, CTR_DRBG_MAX_SEED_INPUT + CTR_DRBG_BLOCKSIZE + 16);
	aes_init(&aes_ctx);

	/*
	* Construct IV (16 bytes) and S in buffer
	* IV = Counter (in 32-bits) padded to 16 with zeroes
	* S = Length input string (in 32-bits) || Length of output (in 32-bits) ||
	*     data || 0x80
	*     (Total is padded to a multiple of 16-bytes with zeroes)
	*/
	p = buf + CTR_DRBG_BLOCKSIZE;
	*p++ = (data_len >> 24) & 0xff;
	*p++ = (data_len >> 16) & 0xff;
	*p++ = (data_len >> 8) & 0xff;
	*p++ = (data_len)& 0xff;
	p += 3;
	*p++ = CTR_DRBG_SEEDLEN;
	xmem_copy(p, data, data_len);
	p[data_len] = 0x80;

	buf_len = CTR_DRBG_BLOCKSIZE + 8 + data_len + 1;

	for (i = 0; i < CTR_DRBG_KEYSIZE; i++)
		key[i] = i;

	if ((ret = aes_setkey_enc(&aes_ctx, key, CTR_DRBG_KEYBITS)) != 0)
	{
		goto exit;
	}

	/*
	* Reduce data to CTR_DRBG_SEEDLEN bytes of data
	*/
	for (j = 0; j < CTR_DRBG_SEEDLEN; j += CTR_DRBG_BLOCKSIZE)
	{
		p = buf;
		xmem_zero(chain, CTR_DRBG_BLOCKSIZE);
		use_len = buf_len;

		while (use_len > 0)
		{
			for (i = 0; i < CTR_DRBG_BLOCKSIZE; i++)
				chain[i] ^= p[i];
			p += CTR_DRBG_BLOCKSIZE;
			use_len -= (use_len >= CTR_DRBG_BLOCKSIZE) ?
			CTR_DRBG_BLOCKSIZE : use_len;

			if ((ret = aes_crypt_ecb(&aes_ctx, AES_ENCRYPT, chain, chain)) != 0)
			{
				goto exit;
			}
		}

		xmem_copy(tmp + j, chain, CTR_DRBG_BLOCKSIZE);

		/*
		* Update IV
		*/
		buf[3]++;
	}

	/*
	* Do final encryption with reduced data
	*/
	if ((ret = aes_setkey_enc(&aes_ctx, tmp, CTR_DRBG_KEYBITS)) != 0)
	{
		goto exit;
	}
	iv = tmp + CTR_DRBG_KEYSIZE;
	p = output;

	for (j = 0; j < CTR_DRBG_SEEDLEN; j += CTR_DRBG_BLOCKSIZE)
	{
		if ((ret = aes_crypt_ecb(&aes_ctx, AES_ENCRYPT, iv, iv)) != 0)
		{
			goto exit;
		}
		xmem_copy(p, iv, CTR_DRBG_BLOCKSIZE);
		p += CTR_DRBG_BLOCKSIZE;
	}
exit:
	aes_free(&aes_ctx);
	/*
	* tidy up the stack
	*/
	xmem_zero(buf, sizeof(buf));
	xmem_zero(tmp, sizeof(tmp));
	xmem_zero(key, sizeof(key));
	xmem_zero(chain, sizeof(chain));
	if (0 != ret)
	{
		/*
		* wipe partial seed from memory
		*/
		xmem_zero(output, CTR_DRBG_SEEDLEN);
	}

	return(ret);
}

/* CTR_DRBG_Update (SP 800-90A &sect;10.2.1.2)
* ctr_drbg_update_internal(ctx, provided_data)
* implements
* CTR_DRBG_Update(provided_data, Key, V)
* with inputs and outputs
*   ctx->aes_ctx = Key
*   ctx->counter = V
*/
static int ctr_drbg_update_internal(ctr_drbg_context *ctx,
	const byte_t data[CTR_DRBG_SEEDLEN])
{
	byte_t tmp[CTR_DRBG_SEEDLEN];
	byte_t *p = tmp;
	int i, j;
	int ret = 0;

	xmem_zero(tmp, CTR_DRBG_SEEDLEN);

	for (j = 0; j < CTR_DRBG_SEEDLEN; j += CTR_DRBG_BLOCKSIZE)
	{
		/*
		* Increase counter
		*/
		for (i = CTR_DRBG_BLOCKSIZE; i > 0; i--)
			if (++ctx->counter[i - 1] != 0)
				break;

		/*
		* Crypt counter block
		*/
		if ((ret = aes_crypt_ecb(&ctx->aes_ctx, AES_ENCRYPT, ctx->counter, p)) != 0)
			goto exit;

		p += CTR_DRBG_BLOCKSIZE;
	}

	for (i = 0; i < CTR_DRBG_SEEDLEN; i++)
		tmp[i] ^= data[i];

	/*
	* Update key and counter
	*/
	if ((ret = aes_setkey_enc(&ctx->aes_ctx, tmp, CTR_DRBG_KEYBITS)) != 0)
		goto exit;
	xmem_copy(ctx->counter, tmp + CTR_DRBG_KEYSIZE, CTR_DRBG_BLOCKSIZE);

exit:
	xmem_zero(tmp, sizeof(tmp));
	return(ret);
}

/* CTR_DRBG_Instantiate with derivation function (SP 800-90A &sect;10.2.1.3.2)
* ctr_drbg_update(ctx, additional, add_len)
* implements
* CTR_DRBG_Instantiate(entropy_input, nonce, personalization_string,
*                      security_strength) -> initial_working_state
* with inputs
*   ctx->counter = all-bits-0
*   ctx->aes_ctx = context from all-bits-0 key
*   additional[:add_len] = entropy_input || nonce || personalization_string
* and with outputs
*   ctx = initial_working_state
*/
int ctr_drbg_update(ctr_drbg_context *ctx,
	const byte_t *additional,
	dword_t add_len)
{
	byte_t add_input[CTR_DRBG_SEEDLEN];
	int ret;

	if (add_len == 0)
		return(0);
	/* MAX_INPUT would be more logical here, but we have to match
	* block_cipher_df()'s limits since we can't propagate errors */
	if (add_len > CTR_DRBG_MAX_SEED_INPUT)
		add_len = CTR_DRBG_MAX_SEED_INPUT;

	if ((ret = block_cipher_df(add_input, additional, add_len)) != 0)
		goto exit;
	if ((ret = ctr_drbg_update_internal(ctx, add_input)) != 0)
		goto exit;

exit:
	xmem_zero(add_input, sizeof(add_input));
	return(ret);
}

/* CTR_DRBG_Reseed with derivation function (SP 800-90A &sect;10.2.1.4.2)
* ctr_drbg_reseed(ctx, additional, len)
* implements
* CTR_DRBG_Reseed(working_state, entropy_input, additional_input)
*                -> new_working_state
* with inputs
*   ctx contains working_state
*   additional[:len] = additional_input
* and entropy_input comes from calling ctx->f_entropy
* and with output
*   ctx contains new_working_state
*/
int ctr_drbg_reseed(ctr_drbg_context *ctx,
	const byte_t *additional, dword_t len)
{
	byte_t seed[CTR_DRBG_MAX_SEED_INPUT];
	dword_t seedlen = 0;
	int ret;

	if (ctx->entropy_len > CTR_DRBG_MAX_SEED_INPUT ||
		len > CTR_DRBG_MAX_SEED_INPUT - ctx->entropy_len)
	{
		set_last_error(_T("ctr_drbg_reseed"), _T("ERR_CTR_DRBG_INPUT_TOO_BIG"), -1);
		return C_ERR;
	}

	xmem_zero(seed, CTR_DRBG_MAX_SEED_INPUT);

	/*
	* Gather entropy_len bytes of entropy to seed state
	*/
	if (0 != ctx->f_entropy(ctx->p_entropy, seed,
		ctx->entropy_len))
	{
		set_last_error(_T("ctr_drbg_reseed"), _T("ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED"), -1);
		return C_ERR;
	}

	seedlen += ctx->entropy_len;

	/*
	* Add additional data
	*/
	if (additional && len)
	{
		xmem_copy(seed + seedlen, additional, len);
		seedlen += len;
	}

	/*
	* Reduce to 384 bits
	*/
	if ((ret = block_cipher_df(seed, seed, seedlen)) != 0)
		goto exit;

	/*
	* Update state
	*/
	if ((ret = ctr_drbg_update_internal(ctx, seed)) != 0)
		goto exit;
	ctx->reseed_counter = 1;

exit:
	xmem_zero(seed, sizeof(seed));
	return(ret);
}

/* CTR_DRBG_Instantiate with derivation function (SP 800-90A &sect;10.2.1.3.2)
* ctr_drbg_seed(ctx, f_entropy, p_entropy, custom, len)
* implements
* CTR_DRBG_Instantiate(entropy_input, nonce, personalization_string,
*                      security_strength) -> initial_working_state
* with inputs
*   custom[:len] = nonce || personalization_string
* where entropy_input comes from f_entropy for ctx->entropy_len bytes
* and with outputs
*   ctx = initial_working_state
*/
int ctr_drbg_seed(ctr_drbg_context *ctx,
	int(*f_entropy)(void *, byte_t *, dword_t),
	void *p_entropy,
	const byte_t *custom,
	dword_t len)
{
	int ret;
	byte_t key[CTR_DRBG_KEYSIZE];

	xmem_zero(key, CTR_DRBG_KEYSIZE);

	aes_init(&ctx->aes_ctx);

	ctx->f_entropy = f_entropy;
	ctx->p_entropy = p_entropy;

	if (ctx->entropy_len == 0)
		ctx->entropy_len = CTR_DRBG_ENTROPY_LEN;
	ctx->reseed_interval = CTR_DRBG_RESEED_INTERVAL;

	/*
	* Initialize with an empty key
	*/
	if ((ret = aes_setkey_enc(&ctx->aes_ctx, key, CTR_DRBG_KEYBITS)) != 0)
	{
		return(ret);
	}

	if ((ret = ctr_drbg_reseed(ctx, custom, len)) != 0)
	{
		return(ret);
	}
	return(0);
}

/* Backward compatibility wrapper */
int ctr_drbg_seed_entropy_len(
	ctr_drbg_context *ctx,
	int(*f_entropy)(void *, byte_t *, dword_t), void *p_entropy,
	const byte_t *custom, dword_t len,
	dword_t entropy_len)
{
	ctr_drbg_set_entropy_len(ctx, entropy_len);
	return(ctr_drbg_seed(ctx, f_entropy, p_entropy, custom, len));
}

/* CTR_DRBG_Generate with derivation function (SP 800-90A &sect;10.2.1.5.2)
* ctr_drbg_random_with_add(ctx, output, output_len, additional, add_len)
* implements
* CTR_DRBG_Reseed(working_state, entropy_input, additional[:add_len])
*                -> working_state_after_reseed
*                if required, then
* CTR_DRBG_Generate(working_state_after_reseed,
*                   requested_number_of_bits, additional_input)
*                -> status, returned_bits, new_working_state
* with inputs
*   ctx contains working_state
*   requested_number_of_bits = 8 * output_len
*   additional[:add_len] = additional_input
* and entropy_input comes from calling ctx->f_entropy
* and with outputs
*   status = SUCCESS (this function does the reseed internally)
*   returned_bits = output[:output_len]
*   ctx contains new_working_state
*/
int ctr_drbg_random_with_add(void *p_rng,
	byte_t *output, dword_t output_len,
	const byte_t *additional, dword_t add_len)
{
	int ret = 0;
	ctr_drbg_context *ctx = (ctr_drbg_context *)p_rng;
	byte_t add_input[CTR_DRBG_SEEDLEN];
	byte_t *p = output;
	byte_t tmp[CTR_DRBG_BLOCKSIZE];
	int i;
	dword_t use_len;

	if (output_len > CTR_DRBG_MAX_REQUEST)
	{
		set_last_error(_T("ctr_drbg_random_with_add"), _T("ERR_CTR_DRBG_REQUEST_TOO_BIG"), -1);
		return C_ERR;
	}

	if (add_len > CTR_DRBG_MAX_INPUT)
	{
		set_last_error(_T("ctr_drbg_random_with_add"), _T("ERR_CTR_DRBG_INPUT_TOO_BIG"), -1);
		return C_ERR;
	}

	xmem_zero(add_input, CTR_DRBG_SEEDLEN);

	if (ctx->reseed_counter > ctx->reseed_interval ||
		ctx->prediction_resistance)
	{
		if ((ret = ctr_drbg_reseed(ctx, additional, add_len)) != 0)
		{
			return(ret);
		}
		add_len = 0;
	}

	if (add_len > 0)
	{
		if ((ret = block_cipher_df(add_input, additional, add_len)) != 0)
			goto exit;
		if ((ret = ctr_drbg_update_internal(ctx, add_input)) != 0)
			goto exit;
	}

	while (output_len > 0)
	{
		/*
		* Increase counter
		*/
		for (i = CTR_DRBG_BLOCKSIZE; i > 0; i--)
			if (++ctx->counter[i - 1] != 0)
				break;

		/*
		* Crypt counter block
		*/
		if ((ret = aes_crypt_ecb(&ctx->aes_ctx, AES_ENCRYPT, ctx->counter, tmp)) != 0)
			goto exit;

		use_len = (output_len > CTR_DRBG_BLOCKSIZE) ? CTR_DRBG_BLOCKSIZE :
			output_len;
		/*
		* Copy random block to destination
		*/
		xmem_copy(p, tmp, use_len);
		p += use_len;
		output_len -= use_len;
	}

	if ((ret = ctr_drbg_update_internal(ctx, add_input)) != 0)
		goto exit;

	ctx->reseed_counter++;

exit:
	xmem_zero(add_input, sizeof(add_input));
	xmem_zero(tmp, sizeof(tmp));
	return(ret);
}

int ctr_drbg_random(void *p_rng, byte_t *output, dword_t output_len)
{
	int ret;
	ctr_drbg_context *ctx = (ctr_drbg_context *)p_rng;

	ret = ctr_drbg_random_with_add(ctx, output, output_len, NULL, 0);

	return(ret);
}


#if defined(XDK_SUPPORT_TEST)

static const byte_t entropy_source_pr[96] =
{ 0xc1, 0x80, 0x81, 0xa6, 0x5d, 0x44, 0x02, 0x16,
0x19, 0xb3, 0xf1, 0x80, 0xb1, 0xc9, 0x20, 0x02,
0x6a, 0x54, 0x6f, 0x0c, 0x70, 0x81, 0x49, 0x8b,
0x6e, 0xa6, 0x62, 0x52, 0x6d, 0x51, 0xb1, 0xcb,
0x58, 0x3b, 0xfa, 0xd5, 0x37, 0x5f, 0xfb, 0xc9,
0xff, 0x46, 0xd2, 0x19, 0xc7, 0x22, 0x3e, 0x95,
0x45, 0x9d, 0x82, 0xe1, 0xe7, 0x22, 0x9f, 0x63,
0x31, 0x69, 0xd2, 0x6b, 0x57, 0x47, 0x4f, 0xa3,
0x37, 0xc9, 0x98, 0x1c, 0x0b, 0xfb, 0x91, 0x31,
0x4d, 0x55, 0xb9, 0xe9, 0x1c, 0x5a, 0x5e, 0xe4,
0x93, 0x92, 0xcf, 0xc5, 0x23, 0x12, 0xd5, 0x56,
0x2c, 0x4a, 0x6e, 0xff, 0xdc, 0x10, 0xd0, 0x68 };

static const byte_t entropy_source_nopr[64] =
{ 0x5a, 0x19, 0x4d, 0x5e, 0x2b, 0x31, 0x58, 0x14,
0x54, 0xde, 0xf6, 0x75, 0xfb, 0x79, 0x58, 0xfe,
0xc7, 0xdb, 0x87, 0x3e, 0x56, 0x89, 0xfc, 0x9d,
0x03, 0x21, 0x7c, 0x68, 0xd8, 0x03, 0x38, 0x20,
0xf9, 0xe6, 0x5e, 0x04, 0xd8, 0x56, 0xf3, 0xa9,
0xc4, 0x4a, 0x4c, 0xbd, 0xc1, 0xd0, 0x08, 0x46,
0xf5, 0x98, 0x3d, 0x77, 0x1c, 0x1b, 0x13, 0x7e,
0x4e, 0x0f, 0x9d, 0x8e, 0xf4, 0x09, 0xf9, 0x2e };

static const byte_t nonce_pers_pr[16] =
{ 0xd2, 0x54, 0xfc, 0xff, 0x02, 0x1e, 0x69, 0xd2,
0x29, 0xc9, 0xcf, 0xad, 0x85, 0xfa, 0x48, 0x6c };

static const byte_t nonce_pers_nopr[16] =
{ 0x1b, 0x54, 0xb8, 0xff, 0x06, 0x42, 0xbf, 0xf5,
0x21, 0xf1, 0x5c, 0x1c, 0x0b, 0x66, 0x5f, 0x3f };

static const byte_t result_pr[16] =
{ 0x34, 0x01, 0x16, 0x56, 0xb4, 0x29, 0x00, 0x8f,
0x35, 0x63, 0xec, 0xb5, 0xf2, 0x59, 0x07, 0x23 };

static const byte_t result_nopr[16] =
{ 0xa0, 0x54, 0x30, 0x3d, 0x8a, 0x7e, 0xa9, 0x88,
0x9d, 0x90, 0x3e, 0x07, 0x7c, 0x6f, 0x21, 0x8f };

static dword_t test_offset;
static int ctr_drbg_self_test_entropy(void *data, byte_t *buf,
	dword_t len)
{
	const byte_t *p = data;
	xmem_copy(buf, p + test_offset, len);
	test_offset += len;
	return(0);
}

#define CHK( c )    if( (c) != 0 )                          \
                    {                                       \
                        if( verbose != 0 )                  \
                            printf( "failed\n" );  \
                        return( 1 );                        \
                    }

/*
* Checkup routine
*/
int ctr_drbg_self_test(int verbose)
{
	ctr_drbg_context ctx;
	byte_t buf[16];

	ctr_drbg_init(&ctx);

	/*
	* Based on a NIST CTR_DRBG test vector (PR = True)
	*/
	if (verbose != 0)
		printf("  CTR_DRBG (PR = TRUE) : ");

	test_offset = 0;
	ctr_drbg_set_entropy_len(&ctx, 32);
	CHK(ctr_drbg_seed(&ctx,
		ctr_drbg_self_test_entropy,
		(void *)entropy_source_pr,
		nonce_pers_pr, 16));
	ctr_drbg_set_prediction_resistance(&ctx, CTR_DRBG_PR_ON);
	CHK(ctr_drbg_random(&ctx, buf, CTR_DRBG_BLOCKSIZE));
	CHK(ctr_drbg_random(&ctx, buf, CTR_DRBG_BLOCKSIZE));
	CHK(xmem_comp(buf, result_pr, CTR_DRBG_BLOCKSIZE));

	ctr_drbg_free(&ctx);

	if (verbose != 0)
		printf("passed\n");

	/*
	* Based on a NIST CTR_DRBG test vector (PR = FALSE)
	*/
	if (verbose != 0)
		printf("  CTR_DRBG (PR = FALSE): ");

	ctr_drbg_init(&ctx);

	test_offset = 0;
	ctr_drbg_set_entropy_len(&ctx, 32);
	CHK(ctr_drbg_seed(&ctx,
		ctr_drbg_self_test_entropy,
		(void *)entropy_source_nopr,
		nonce_pers_nopr, 16));
	CHK(ctr_drbg_random(&ctx, buf, 16));
	CHK(ctr_drbg_reseed(&ctx, NULL, 0));
	CHK(ctr_drbg_random(&ctx, buf, 16));
	CHK(xmem_comp(buf, result_nopr, 16));

	ctr_drbg_free(&ctx);

	if (verbose != 0)
		printf("passed\n");

	if (verbose != 0)
		printf("\n");

	return(0);
}
#endif /* SELF_TEST */