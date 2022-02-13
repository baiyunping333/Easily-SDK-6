/*
*  The RSA public-key cryptosystem
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
*  The following sources were referenced in the design of this implementation
*  of the RSA algorithm:
*
*  [1] A method for obtaining digital signatures and public-key cryptosystems
*      R Rivest, A Shamir, and L Adleman
*      http://people.csail.mit.edu/rivest/pubs.html#RSA78
*
*  [2] Handbook of Applied Cryptography - 1997, Chapter 8
*      Menezes, van Oorschot and Vanstone
*
*  [3] Malware Guard Extension: Using SGX to Conceal Cache Attacks
*      Michael Schwarz, Samuel Weiser, Daniel Gruss, Clémentine Maurice and
*      Stefan Mangard
*      https://arxiv.org/abs/1702.08719v2
*
*/

#include "rsa.h"

#include "mdwrap.h"
#include "asn1.h"
#include "oid.h"
#include "pem.h"

#include "../xdkimp.h"

/*
* Compute RSA prime factors from public and private exponents
*
* Summary of algorithm :
*Setting F : = lcm(P - 1, Q - 1), the idea is as follows :
*
* (a)For any 1 <= X < N with gcd(X, N) = 1, we have X^F = 1 modulo N, so X ^ (F / 2)
*     is a square root of 1 in Z / NZ.Since Z / NZ ~= Z / PZ x Z / QZ by CRT and the
*     square roots of 1 in Z / PZ and Z / QZ are + 1 and - 1, this leaves the four
*     possibilities X ^ (F / 2) = (+-1, +-1).If it happens that X ^ (F / 2) = (-1, +1)
*     or(+1, -1), then gcd(X ^ (F / 2) + 1, N) will be equal to one of the prime
*     factors of N.
*
* (b)If we don't know F/2 but (F/2) * K for some odd (!) K, then the same
*     construction still applies since(-) ^ K is the identity on the set of
*     roots of 1 in Z / NZ.
*
* The public and private key primitives(-) ^ E and(-) ^ D are mutually inverse
* bijections on Z / NZ if and only if (-) ^ (DE)is the identity on Z / NZ, i.e.
* if and only if DE - 1 is a multiple of F, say DE - 1 = F * L.
* Splitting L = 2 ^ t * K with K odd, we have
*
*   DE - 1 = FL = (F / 2) * (2 ^ (t + 1)) * K,
*
* so(F / 2) * K is among the numbers
*
*   (DE - 1) >> 1, (DE - 1) >> 2, ..., (DE - 1) >> ord
*
* where ord is the order of 2 in(DE - 1).
* We can therefore iterate through these numbers apply the construction
* of(a) and(b) above to attempt to factor N.
*
*/

int rsa_deduce_primes(mpi const *N,
	mpi const *E, mpi const *D,
	mpi *P, mpi *Q)
{
	int ret = 0;

	sword_t attempt;  /* Number of current attempt  */
	sword_t iter;     /* Number of squares computed in the current attempt */

	sword_t order;    /* Order of 2 in DE - 1 */

	mpi T;  /* Holds largest odd divisor of DE - 1     */
	mpi K;  /* Temporary holding the current candidate */

	const byte_t primes[] = { 2,
		3, 5, 7, 11, 13, 17, 19, 23,
		29, 31, 37, 41, 43, 47, 53, 59,
		61, 67, 71, 73, 79, 83, 89, 97,
		101, 103, 107, 109, 113, 127, 131, 137,
		139, 149, 151, 157, 163, 167, 173, 179,
		181, 191, 193, 197, 199, 211, 223, 227,
		229, 233, 239, 241, 251
	};

	const dword_t num_primes = sizeof(primes) / sizeof(*primes);

	if (P == NULL || Q == NULL || P->p != NULL || Q->p != NULL)
	{
		set_last_error(_T("rsa_deduce_primes"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (mpi_cmp_int(N, 0) <= 0 ||
		mpi_cmp_int(D, 1) <= 0 ||
		mpi_cmp_mpi(D, N) >= 0 ||
		mpi_cmp_int(E, 1) <= 0 ||
		mpi_cmp_mpi(E, N) >= 0)
	{
		set_last_error(_T("rsa_deduce_primes"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* Initializations and temporary changes
	*/

	mpi_init(&K);
	mpi_init(&T);

	/* T := DE - 1 */
	MPI_CHK(mpi_mul_mpi(&T, D, E));
	MPI_CHK(mpi_sub_int(&T, &T, 1));

	if ((order = (sword_t)mpi_lsb(&T)) == 0)
	{
		set_last_error(_T("rsa_deduce_primes"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	/* After this operation, T holds the largest odd divisor of DE - 1. */
	MPI_CHK(mpi_shift_r(&T, order));

	/*
	* Actual work
	*/

	/* Skip trying 2 if N == 1 mod 8 */
	attempt = 0;
	if (N->p[0] % 8 == 1)
		attempt = 1;

	for (; attempt < num_primes; ++attempt)
	{
		mpi_lset(&K, primes[attempt]);

		/* Check if gcd(K,N) = 1 */
		MPI_CHK(mpi_gcd(P, &K, N));
		if (mpi_cmp_int(P, 1) != 0)
			continue;

		/* Go through K^T + 1, K^(2T) + 1, K^(4T) + 1, ...
		* and check whether they have nontrivial GCD with N. */
		MPI_CHK(mpi_exp_mod(&K, &K, &T, N,
			Q /* temporarily use Q for storing Montgomery
			  * multiplication helper values */));

		for (iter = 1; iter <= order; ++iter)
		{
			/* If we reach 1 prematurely, there's no point
			* in continuing to square K */
			if (mpi_cmp_int(&K, 1) == 0)
				break;

			MPI_CHK(mpi_add_int(&K, &K, 1));
			MPI_CHK(mpi_gcd(P, &K, N));

			if (mpi_cmp_int(P, 1) == 1 &&
				mpi_cmp_mpi(P, N) == -1)
			{
				/*
				* Have found a nontrivial divisor P of N.
				* Set Q := N / P.
				*/

				MPI_CHK(mpi_div_mpi(Q, NULL, N, P));
				goto cleanup;
			}

			MPI_CHK(mpi_sub_int(&K, &K, 1));
			MPI_CHK(mpi_mul_mpi(&K, &K, &K));
			MPI_CHK(mpi_mod_mpi(&K, &K, N));
		}

		/*
		* If we get here, then either we prematurely aborted the loop because
		* we reached 1, or K holds primes[attempt]^(DE - 1) mod N, which must
		* be 1 if D,E,N were consistent.
		* Check if that's the case and abort if not, to avoid very long,
		* yet eventually failing, computations if N,D,E were not sane.
		*/
		if (mpi_cmp_int(&K, 1) != 0)
		{
			break;
		}
	}

	ret = C_ERR;

cleanup:

	mpi_free(&K);
	mpi_free(&T);
	return(ret);
}

/*
* Given P, Q and the public exponent E, deduce D.
* This is essentially a modular inversion.
*/
int rsa_deduce_private_exponent(mpi const *P,
	mpi const *Q,
	mpi const *E,
	mpi *D)
{
	int ret = 0;
	mpi K, L;

	if (D == NULL || mpi_cmp_int(D, 0) != 0)
	{
		set_last_error(_T("rsa_deduce_private_exponent"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (mpi_cmp_int(P, 1) <= 0 ||
		mpi_cmp_int(Q, 1) <= 0 ||
		mpi_cmp_int(E, 0) == 0)
	{
		set_last_error(_T("rsa_deduce_private_exponent"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	mpi_init(&K);
	mpi_init(&L);

	/* Temporarily put K := P-1 and L := Q-1 */
	MPI_CHK(mpi_sub_int(&K, P, 1));
	MPI_CHK(mpi_sub_int(&L, Q, 1));

	/* Temporarily put D := gcd(P-1, Q-1) */
	MPI_CHK(mpi_gcd(D, &K, &L));

	/* K := LCM(P-1, Q-1) */
	MPI_CHK(mpi_mul_mpi(&K, &K, &L));
	MPI_CHK(mpi_div_mpi(&K, NULL, &K, D));

	/* Compute modular inverse of E in LCM(P-1, Q-1) */
	MPI_CHK(mpi_inv_mod(D, E, &K));

cleanup:

	mpi_free(&K);
	mpi_free(&L);

	return(ret);
}

/*
* Check that RSA CRT parameters are in accordance with core parameters.
*/
int rsa_validate_crt(const mpi *P, const mpi *Q,
	const mpi *D, const mpi *DP,
	const mpi *DQ, const mpi *QP)
{
	int ret = 0;

	mpi K, L;
	mpi_init(&K);
	mpi_init(&L);

	/* Check that DP - D == 0 mod P - 1 */
	if (DP != NULL)
	{
		if (P == NULL)
		{
			set_last_error(_T("rsa_validate_crt"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
			ret = C_ERR;
			goto cleanup;
		}

		MPI_CHK(mpi_sub_int(&K, P, 1));
		MPI_CHK(mpi_sub_mpi(&L, DP, D));
		MPI_CHK(mpi_mod_mpi(&L, &L, &K));

		if (mpi_cmp_int(&L, 0) != 0)
		{
			set_last_error(_T("rsa_validate_crt"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}
	}

	/* Check that DQ - D == 0 mod Q - 1 */
	if (DQ != NULL)
	{
		if (Q == NULL)
		{
			set_last_error(_T("rsa_validate_crt"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
			ret = C_ERR;
			goto cleanup;
		}

		MPI_CHK(mpi_sub_int(&K, Q, 1));
		MPI_CHK(mpi_sub_mpi(&L, DQ, D));
		MPI_CHK(mpi_mod_mpi(&L, &L, &K));

		if (mpi_cmp_int(&L, 0) != 0)
		{
			set_last_error(_T("rsa_validate_crt"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}
	}

	/* Check that QP * Q - 1 == 0 mod P */
	if (QP != NULL)
	{
		if (P == NULL || Q == NULL)
		{
			set_last_error(_T("rsa_validate_crt"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
			ret = C_ERR;
			goto cleanup;
		}

		MPI_CHK(mpi_mul_mpi(&K, QP, Q));
		MPI_CHK(mpi_sub_int(&K, &K, 1));
		MPI_CHK(mpi_mod_mpi(&K, &K, P));
		if (mpi_cmp_int(&K, 0) != 0)
		{
			set_last_error(_T("rsa_validate_crt"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}
	}

cleanup:

	mpi_free(&K);
	mpi_free(&L);

	return(ret);
}

/*
* Check that core RSA parameters are sane.
*/
int rsa_validate_params(const mpi *N, const mpi *P,
	const mpi *Q, const mpi *D,
	const mpi *E,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret = 0;
	mpi K, L;

	mpi_init(&K);
	mpi_init(&L);

	/*
	* Step 1: If PRNG provided, check that P and Q are prime
	*/

#if defined(GENPRIME)
	/*
	* When generating keys, the strongest security we support aims for an error
	* rate of at most 2^-100 and we are aiming for the same certainty here as
	* well.
	*/
	if (f_rng != NULL && P != NULL &&
		(ret = mpi_is_prime(P, 50, f_rng, p_rng)) != 0)
	{
		set_last_error(_T("rsa_validate_params"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	if (f_rng != NULL && Q != NULL &&
		(ret = mpi_is_prime(Q, 50, f_rng, p_rng)) != 0)
	{
		set_last_error(_T("rsa_validate_params"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}
#else
	((void)f_rng);
	((void)p_rng);
#endif /* GENPRIME */

	/*
	* Step 2: Check that 1 < N = P * Q
	*/

	if (P != NULL && Q != NULL && N != NULL)
	{
		MPI_CHK(mpi_mul_mpi(&K, P, Q));
		if (mpi_cmp_int(N, 1) <= 0 ||
			mpi_cmp_mpi(&K, N) != 0)
		{
			set_last_error(_T("rsa_validate_params"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}
	}

	/*
	* Step 3: Check and 1 < D, E < N if present.
	*/

	if (N != NULL && D != NULL && E != NULL)
	{
		if (mpi_cmp_int(D, 1) <= 0 ||
			mpi_cmp_int(E, 1) <= 0 ||
			mpi_cmp_mpi(D, N) >= 0 ||
			mpi_cmp_mpi(E, N) >= 0)
		{
			set_last_error(_T("rsa_validate_params"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}
	}

	/*
	* Step 4: Check that D, E are inverse modulo P-1 and Q-1
	*/

	if (P != NULL && Q != NULL && D != NULL && E != NULL)
	{
		if (mpi_cmp_int(P, 1) <= 0 ||
			mpi_cmp_int(Q, 1) <= 0)
		{
			set_last_error(_T("rsa_validate_params"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}

		/* Compute DE-1 mod P-1 */
		MPI_CHK(mpi_mul_mpi(&K, D, E));
		MPI_CHK(mpi_sub_int(&K, &K, 1));
		MPI_CHK(mpi_sub_int(&L, P, 1));
		MPI_CHK(mpi_mod_mpi(&K, &K, &L));
		if (mpi_cmp_int(&K, 0) != 0)
		{
			set_last_error(_T("rsa_validate_params"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}

		/* Compute DE-1 mod Q-1 */
		MPI_CHK(mpi_mul_mpi(&K, D, E));
		MPI_CHK(mpi_sub_int(&K, &K, 1));
		MPI_CHK(mpi_sub_int(&L, Q, 1));
		MPI_CHK(mpi_mod_mpi(&K, &K, &L));
		if (mpi_cmp_int(&K, 0) != 0)
		{
			set_last_error(_T("rsa_validate_params"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
			ret = C_ERR;
			goto cleanup;
		}
	}

cleanup:

	mpi_free(&K);
	mpi_free(&L);

	return(ret);
}

int rsa_deduce_crt(const mpi *P, const mpi *Q,
	const mpi *D, mpi *DP,
	mpi *DQ, mpi *QP)
{
	int ret = 0;
	mpi K;
	mpi_init(&K);

	/* DP = D mod P-1 */
	if (DP != NULL)
	{
		MPI_CHK(mpi_sub_int(&K, P, 1));
		MPI_CHK(mpi_mod_mpi(DP, D, &K));
	}

	/* DQ = D mod Q-1 */
	if (DQ != NULL)
	{
		MPI_CHK(mpi_sub_int(&K, Q, 1));
		MPI_CHK(mpi_mod_mpi(DQ, D, &K));
	}

	/* QP = Q^{-1} mod P */
	if (QP != NULL)
	{
		MPI_CHK(mpi_inv_mod(QP, Q, P));
	}

cleanup:
	mpi_free(&K);

	return(ret);
}

int rsa_import(rsa_context *ctx,
	const mpi *N,
	const mpi *P, const mpi *Q,
	const mpi *D, const mpi *E)
{
	int ret;
	XDK_ASSERT(ctx != NULL);

	if ((N != NULL && (ret = mpi_copy(&ctx->N, N)) != 0) ||
		(P != NULL && (ret = mpi_copy(&ctx->P, P)) != 0) ||
		(Q != NULL && (ret = mpi_copy(&ctx->Q, Q)) != 0) ||
		(D != NULL && (ret = mpi_copy(&ctx->D, D)) != 0) ||
		(E != NULL && (ret = mpi_copy(&ctx->E, E)) != 0))
	{
		set_last_error(_T("rsa_import"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (N != NULL)
		ctx->len = mpi_size(&ctx->N);

	return(0);
}

int rsa_import_raw(rsa_context *ctx,
	byte_t const *N, dword_t N_len,
	byte_t const *P, dword_t P_len,
	byte_t const *Q, dword_t Q_len,
	byte_t const *D, dword_t D_len,
	byte_t const *E, dword_t E_len)
{
	int ret = 0;
	XDK_ASSERT(ctx != NULL);

	if (N != NULL)
	{
		MPI_CHK(mpi_read_binary(&ctx->N, N, N_len));
		ctx->len = mpi_size(&ctx->N);
	}

	if (P != NULL)
		MPI_CHK(mpi_read_binary(&ctx->P, P, P_len));

	if (Q != NULL)
		MPI_CHK(mpi_read_binary(&ctx->Q, Q, Q_len));

	if (D != NULL)
		MPI_CHK(mpi_read_binary(&ctx->D, D, D_len));

	if (E != NULL)
		MPI_CHK(mpi_read_binary(&ctx->E, E, E_len));

cleanup:

	return(0);
}

/*
* Checks whether the context fields are set in such a way
* that the RSA primitives will be able to execute without error.
* It does *not* make guarantees for consistency of the parameters.
*/
static int rsa_check_context(rsa_context const *ctx, int is_priv,
	int blinding_needed)
{
	if (ctx->len != mpi_size(&ctx->N) ||
		ctx->len > MPI_MAX_SIZE)
	{
		set_last_error(_T("rsa_check_context"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* 1. Modular exponentiation needs positive, odd moduli.
	*/

	/* Modular exponentiation wrt. N is always used for
	* RSA public key operations. */
	if (mpi_cmp_int(&ctx->N, 0) <= 0 ||
		mpi_get_bit(&ctx->N, 0) == 0)
	{
		set_last_error(_T("rsa_check_context"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/* Modular exponentiation for P and Q is only
	* used for private key operations and if CRT
	* is used. */
	if (is_priv &&
		(mpi_cmp_int(&ctx->P, 0) <= 0 ||
		mpi_get_bit(&ctx->P, 0) == 0 ||
		mpi_cmp_int(&ctx->Q, 0) <= 0 ||
		mpi_get_bit(&ctx->Q, 0) == 0))
	{
		set_last_error(_T("rsa_check_context"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* 2. Exponents must be positive
	*/

	/* Always need E for public key operations */
	if (mpi_cmp_int(&ctx->E, 0) <= 0)
	{
		set_last_error(_T("rsa_check_context"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (is_priv &&
		(mpi_cmp_int(&ctx->DP, 0) <= 0 ||
		mpi_cmp_int(&ctx->DQ, 0) <= 0))
	{
		set_last_error(_T("rsa_check_context"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/* It wouldn't lead to an error if it wasn't satisfied,
	* but check for QP >= 1 nonetheless. */
	if (is_priv &&
		mpi_cmp_int(&ctx->QP, 0) <= 0)
	{
		set_last_error(_T("rsa_check_context"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	return(0);
}

int rsa_complete(rsa_context *ctx)
{
	int ret = 0;
	int have_N, have_P, have_Q, have_D, have_E;
	int have_DP, have_DQ, have_QP;
	int n_missing, pq_missing, d_missing, is_pub, is_priv;

	XDK_ASSERT(ctx != NULL);

	have_N = (mpi_cmp_int(&ctx->N, 0) != 0);
	have_P = (mpi_cmp_int(&ctx->P, 0) != 0);
	have_Q = (mpi_cmp_int(&ctx->Q, 0) != 0);
	have_D = (mpi_cmp_int(&ctx->D, 0) != 0);
	have_E = (mpi_cmp_int(&ctx->E, 0) != 0);

	have_DP = (mpi_cmp_int(&ctx->DP, 0) != 0);
	have_DQ = (mpi_cmp_int(&ctx->DQ, 0) != 0);
	have_QP = (mpi_cmp_int(&ctx->QP, 0) != 0);

	/*
	* Check whether provided parameters are enough
	* to deduce all others. The following incomplete
	* parameter sets for private keys are supported:
	*
	* (1) P, Q missing.
	* (2) D and potentially N missing.
	*
	*/

	n_missing = have_P &&  have_Q &&  have_D && have_E;
	pq_missing = have_N && !have_P && !have_Q &&  have_D && have_E;
	d_missing = have_P &&  have_Q && !have_D && have_E;
	is_pub = have_N && !have_P && !have_Q && !have_D && have_E;

	/* These three alternatives are mutually exclusive */
	is_priv = n_missing || pq_missing || d_missing;

	if (!is_priv && !is_pub)
	{
		set_last_error(_T("rsa_complete"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* Step 1: Deduce N if P, Q are provided.
	*/

	if (!have_N && have_P && have_Q)
	{
		if ((ret = mpi_mul_mpi(&ctx->N, &ctx->P,
			&ctx->Q)) != 0)
		{
			set_last_error(_T("rsa_complete"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		ctx->len = mpi_size(&ctx->N);
	}

	/*
	* Step 2: Deduce and verify all remaining core parameters.
	*/

	if (pq_missing)
	{
		ret = rsa_deduce_primes(&ctx->N, &ctx->E, &ctx->D,
			&ctx->P, &ctx->Q);
		if (ret != 0)
		{
			set_last_error(_T("rsa_complete"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}
	}
	else if (d_missing)
	{
		if ((ret = rsa_deduce_private_exponent(&ctx->P,
			&ctx->Q,
			&ctx->E,
			&ctx->D)) != 0)
		{
			set_last_error(_T("rsa_complete"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}
	}

	/*
	* Step 3: Deduce all additional parameters specific
	*         to our current RSA implementation.
	*/

	if (is_priv && !(have_DP && have_DQ && have_QP))
	{
		ret = rsa_deduce_crt(&ctx->P, &ctx->Q, &ctx->D,
			&ctx->DP, &ctx->DQ, &ctx->QP);
		if (ret != 0)
		{
			set_last_error(_T("rsa_complete"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}
	}

	/*
	* Step 3: Basic sanity checks
	*/

	return(rsa_check_context(ctx, is_priv, 1));
}

int rsa_export_raw(const rsa_context *ctx,
	byte_t *N, dword_t N_len,
	byte_t *P, dword_t P_len,
	byte_t *Q, dword_t Q_len,
	byte_t *D, dword_t D_len,
	byte_t *E, dword_t E_len)
{
	int ret = 0;
	int is_priv;
	XDK_ASSERT(ctx != NULL);

	/* Check if key is private or public */
	is_priv =
		mpi_cmp_int(&ctx->N, 0) != 0 &&
		mpi_cmp_int(&ctx->P, 0) != 0 &&
		mpi_cmp_int(&ctx->Q, 0) != 0 &&
		mpi_cmp_int(&ctx->D, 0) != 0 &&
		mpi_cmp_int(&ctx->E, 0) != 0;

	if (!is_priv)
	{
		/* If we're trying to export private parameters for a public key,
		* something must be wrong. */
		if (P != NULL || Q != NULL || D != NULL)
		{
			set_last_error(_T("rsa_export_raw"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

	}

	if (N != NULL)
		MPI_CHK(mpi_write_binary(&ctx->N, N, N_len));

	if (P != NULL)
		MPI_CHK(mpi_write_binary(&ctx->P, P, P_len));

	if (Q != NULL)
		MPI_CHK(mpi_write_binary(&ctx->Q, Q, Q_len));

	if (D != NULL)
		MPI_CHK(mpi_write_binary(&ctx->D, D, D_len));

	if (E != NULL)
		MPI_CHK(mpi_write_binary(&ctx->E, E, E_len));

cleanup:

	return(ret);
}

int rsa_export(const rsa_context *ctx,
	mpi *N, mpi *P, mpi *Q,
	mpi *D, mpi *E)
{
	int ret;
	int is_priv;
	XDK_ASSERT(ctx != NULL);

	/* Check if key is private or public */
	is_priv =
		mpi_cmp_int(&ctx->N, 0) != 0 &&
		mpi_cmp_int(&ctx->P, 0) != 0 &&
		mpi_cmp_int(&ctx->Q, 0) != 0 &&
		mpi_cmp_int(&ctx->D, 0) != 0 &&
		mpi_cmp_int(&ctx->E, 0) != 0;

	if (!is_priv)
	{
		/* If we're trying to export private parameters for a public key,
		* something must be wrong. */
		if (P != NULL || Q != NULL || D != NULL)
		{
			set_last_error(_T("rsa_export"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}
	}

	/* Export all requested core parameters. */

	if ((N != NULL && (ret = mpi_copy(N, &ctx->N)) != 0) ||
		(P != NULL && (ret = mpi_copy(P, &ctx->P)) != 0) ||
		(Q != NULL && (ret = mpi_copy(Q, &ctx->Q)) != 0) ||
		(D != NULL && (ret = mpi_copy(D, &ctx->D)) != 0) ||
		(E != NULL && (ret = mpi_copy(E, &ctx->E)) != 0))
	{
		return(ret);
	}

	return(0);
}

/*
* Export CRT parameters
* This must also be implemented if CRT is not used, for being able to
* write DER encoded RSA keys. The helper function rsa_deduce_crt
* can be used in this case.
*/
int rsa_export_crt(const rsa_context *ctx,
	mpi *DP, mpi *DQ, mpi *QP)
{
	int ret;
	int is_priv;
	XDK_ASSERT(ctx != NULL);

	/* Check if key is private or public */
	is_priv =
		mpi_cmp_int(&ctx->N, 0) != 0 &&
		mpi_cmp_int(&ctx->P, 0) != 0 &&
		mpi_cmp_int(&ctx->Q, 0) != 0 &&
		mpi_cmp_int(&ctx->D, 0) != 0 &&
		mpi_cmp_int(&ctx->E, 0) != 0;

	if (!is_priv)
	{
		set_last_error(_T("rsa_export_crt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/* Export all requested blinding parameters. */
	if ((DP != NULL && (ret = mpi_copy(DP, &ctx->DP)) != 0) ||
		(DQ != NULL && (ret = mpi_copy(DQ, &ctx->DQ)) != 0) ||
		(QP != NULL && (ret = mpi_copy(QP, &ctx->QP)) != 0))
	{
		set_last_error(_T("rsa_export_crt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Initialize an RSA context
*/
void rsa_init(rsa_context *ctx,
	int padding,
	int hash_id)
{
	XDK_ASSERT(ctx != NULL);

	XDK_ASSERT(padding == RSA_PKCS_V15 || padding == RSA_PKCS_V21);

	xmem_zero(ctx, sizeof(rsa_context));

	rsa_set_padding(ctx, padding, hash_id);
}

/*
* Set padding for an existing RSA context
*/
void rsa_set_padding(rsa_context *ctx, int padding,
	int hash_id)
{
	XDK_ASSERT(ctx != NULL);

	XDK_ASSERT(padding == RSA_PKCS_V15 || padding == RSA_PKCS_V21);

	ctx->padding = padding;
	ctx->hash_id = hash_id;
}

/*
* Get length in bytes of RSA modulus
*/

dword_t rsa_get_len(const rsa_context *ctx)
{
	return(ctx->len);
}


#if defined(GENPRIME)

/*
* Generate an RSA keypair
*
* This generation method follows the RSA key pair generation procedure of
* FIPS 186-4 if 2^16 < exponent < 2^256 and nbits = 2048 or nbits = 3072.
*/
int rsa_gen_key(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	dword_t nbits, int exponent)
{
	int ret;
	mpi H, G, L;
	int prime_quality = 0;
	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(f_rng != NULL);

	if (nbits < 128 || exponent < 3 || nbits % 2 != 0)
	{
		set_last_error(_T("rsa_gen_key"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* If the modulus is 1024 bit long or shorter, then the security strength of
	* the RSA algorithm is less than or equal to 80 bits and therefore an error
	* rate of 2^-80 is sufficient.
	*/
	if (nbits > 1024)
		prime_quality = MPI_GEN_PRIME_FLAG_LOW_ERR;

	mpi_init(&H);
	mpi_init(&G);
	mpi_init(&L);

	/*
	* find primes P and Q with Q < P so that:
	* 1.  |P-Q| > 2^( nbits / 2 - 100 )
	* 2.  GCD( E, (P-1)*(Q-1) ) == 1
	* 3.  E^-1 mod LCM(P-1, Q-1) > 2^( nbits / 2 )
	*/
	MPI_CHK(mpi_lset(&ctx->E, exponent));

	do
	{
		MPI_CHK(mpi_gen_prime(&ctx->P, nbits >> 1,
			prime_quality, f_rng, p_rng));

		MPI_CHK(mpi_gen_prime(&ctx->Q, nbits >> 1,
			prime_quality, f_rng, p_rng));

		/* make sure the difference between p and q is not too small (FIPS 186-4 §B.3.3 step 5.4) */
		MPI_CHK(mpi_sub_mpi(&H, &ctx->P, &ctx->Q));
		if (mpi_bitlen(&H) <= ((nbits >= 200) ? ((nbits >> 1) - 99) : 0))
			continue;

		/* not required by any standards, but some users rely on the fact that P > Q */
		if (H.s < 0)
			mpi_swap(&ctx->P, &ctx->Q);

		/* Temporarily replace P,Q by P-1, Q-1 */
		MPI_CHK(mpi_sub_int(&ctx->P, &ctx->P, 1));
		MPI_CHK(mpi_sub_int(&ctx->Q, &ctx->Q, 1));
		MPI_CHK(mpi_mul_mpi(&H, &ctx->P, &ctx->Q));

		/* check GCD( E, (P-1)*(Q-1) ) == 1 (FIPS 186-4 §B.3.1 criterion 2(a)) */
		MPI_CHK(mpi_gcd(&G, &ctx->E, &H));
		if (mpi_cmp_int(&G, 1) != 0)
			continue;

		/* compute smallest possible D = E^-1 mod LCM(P-1, Q-1) (FIPS 186-4 §B.3.1 criterion 3(b)) */
		MPI_CHK(mpi_gcd(&G, &ctx->P, &ctx->Q));
		MPI_CHK(mpi_div_mpi(&L, NULL, &H, &G));
		MPI_CHK(mpi_inv_mod(&ctx->D, &ctx->E, &L));

		if (mpi_bitlen(&ctx->D) <= ((nbits + 1) / 2)) // (FIPS 186-4 §B.3.1 criterion 3(a))
			continue;

		break;
	} while (1);

	/* Restore P,Q */
	MPI_CHK(mpi_add_int(&ctx->P, &ctx->P, 1));
	MPI_CHK(mpi_add_int(&ctx->Q, &ctx->Q, 1));

	MPI_CHK(mpi_mul_mpi(&ctx->N, &ctx->P, &ctx->Q));

	ctx->len = mpi_size(&ctx->N);

	/*
	* DP = D mod (P - 1)
	* DQ = D mod (Q - 1)
	* QP = Q^-1 mod P
	*/
	MPI_CHK(rsa_deduce_crt(&ctx->P, &ctx->Q, &ctx->D,
		&ctx->DP, &ctx->DQ, &ctx->QP));

	/* Double-check */
	MPI_CHK(rsa_check_privkey(ctx));

cleanup:

	mpi_free(&H);
	mpi_free(&G);
	mpi_free(&L);

	if (ret != 0)
	{
		rsa_free(ctx);

		set_last_error(_T("rsa_gen_key"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	return(0);
}

#endif /* GENPRIME */

/*
* Check a public RSA key
*/
int rsa_check_pubkey(const rsa_context *ctx)
{
	XDK_ASSERT(ctx != NULL);

	if (rsa_check_context(ctx, 0 /* public */, 0 /* no blinding */) != 0)
	{
		set_last_error(_T("rsa_check_pubkey"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}

	if (mpi_bitlen(&ctx->N) < 128)
	{
		set_last_error(_T("rsa_check_pubkey"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}

	if (mpi_get_bit(&ctx->E, 0) == 0 ||
		mpi_bitlen(&ctx->E)     < 2 ||
		mpi_cmp_mpi(&ctx->E, &ctx->N) >= 0)
	{
		set_last_error(_T("rsa_check_pubkey"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Check for the consistency of all fields in an RSA private key context
*/
int rsa_check_privkey(const rsa_context *ctx)
{
	XDK_ASSERT(ctx != NULL);

	if (rsa_check_pubkey(ctx) != 0 ||
		rsa_check_context(ctx, 1 /* private */, 1 /* blinding */) != 0)
	{
		set_last_error(_T("rsa_check_privkey"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}

	if (rsa_validate_params(&ctx->N, &ctx->P, &ctx->Q,
		&ctx->D, &ctx->E, NULL, NULL) != 0)
	{
		set_last_error(_T("rsa_check_privkey"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}
	else if (rsa_validate_crt(&ctx->P, &ctx->Q, &ctx->D,
		&ctx->DP, &ctx->DQ, &ctx->QP) != 0)
	{
		set_last_error(_T("rsa_check_privkey"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Check if contexts holding a public and private key match
*/
int rsa_check_pub_priv(const rsa_context *pub,
	const rsa_context *prv)
{
	XDK_ASSERT(pub != NULL);
	XDK_ASSERT(prv != NULL);

	if (rsa_check_pubkey(pub) != 0 ||
		rsa_check_privkey(prv) != 0)
	{
		set_last_error(_T("rsa_check_pub_priv"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}

	if (mpi_cmp_mpi(&pub->N, &prv->N) != 0 ||
		mpi_cmp_mpi(&pub->E, &prv->E) != 0)
	{
		set_last_error(_T("rsa_check_pub_priv"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Do an RSA public key operation
*/
int rsa_public(rsa_context *ctx,
	const byte_t *input,
	byte_t *output)
{
	int ret;
	dword_t olen;
	mpi T;
	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(input != NULL);
	XDK_ASSERT(output != NULL);

	if (rsa_check_context(ctx, 0 /* public */, 0 /* no blinding */))
	{
		set_last_error(_T("rsa_public"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		return C_ERR;
	}

	mpi_init(&T);

	MPI_CHK(mpi_read_binary(&T, input, ctx->len));

	if (mpi_cmp_mpi(&T, &ctx->N) >= 0)
	{
		set_last_error(_T("rsa_public"), _T("ERR_RSA_KEY_CHECK_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	olen = ctx->len;
	MPI_CHK(mpi_exp_mod(&T, &T, &ctx->E, &ctx->N, &ctx->RN));
	MPI_CHK(mpi_write_binary(&T, output, olen));

cleanup:

	mpi_free(&T);

	if (ret != 0)
	{
		set_last_error(_T("rsa_public"), _T("ERR_RSA_PUBLIC_FAILED"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Generate or update blinding values, see section 10 of:
*  KOCHER, Paul C. Timing attacks on implementations of Diffie-Hellman, RSA,
*  DSS, and other systems. In : Advances in Cryptology-CRYPTO'96. Springer
*  Berlin Heidelberg, 1996. p. 104-113.
*/
static int rsa_prepare_blinding(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t), void *p_rng)
{
	int ret, count = 0;

	if (ctx->Vf.p != NULL)
	{
		/* We already have blinding values, just update them by squaring */
		MPI_CHK(mpi_mul_mpi(&ctx->Vi, &ctx->Vi, &ctx->Vi));
		MPI_CHK(mpi_mod_mpi(&ctx->Vi, &ctx->Vi, &ctx->N));
		MPI_CHK(mpi_mul_mpi(&ctx->Vf, &ctx->Vf, &ctx->Vf));
		MPI_CHK(mpi_mod_mpi(&ctx->Vf, &ctx->Vf, &ctx->N));

		goto cleanup;
	}

	/* Unblinding value: Vf = random number, invertible mod N */
	do {
		if (count++ > 10)
		{
			set_last_error(_T("rsa_prepare_blinding"), _T("ERR_RSA_RNG_FAILED"), -1);
			return C_ERR;
		}

		MPI_CHK(mpi_fill_random(&ctx->Vf, ctx->len - 1, f_rng, p_rng));
		MPI_CHK(mpi_gcd(&ctx->Vi, &ctx->Vf, &ctx->N));
	} while (mpi_cmp_int(&ctx->Vi, 1) != 0);

	/* Blinding value: Vi =  Vf^(-e) mod N */
	MPI_CHK(mpi_inv_mod(&ctx->Vi, &ctx->Vf, &ctx->N));
	MPI_CHK(mpi_exp_mod(&ctx->Vi, &ctx->Vi, &ctx->E, &ctx->N, &ctx->RN));


cleanup:
	return(ret);
}

/*
* Exponent blinding supposed to prevent side-channel attacks using multiple
* traces of measurements to recover the RSA key. The more collisions are there,
* the more bits of the key can be recovered. See [3].
*
* Collecting n collisions with m bit long blinding value requires 2^(m-m/n)
* observations on avarage.
*
* For example with 28 byte blinding to achieve 2 collisions the adversary has
* to make 2^112 observations on avarage.
*
* (With the currently (as of 2017 April) known best algorithms breaking 2048
* bit RSA requires approximately as much time as trying out 2^112 random keys.
* Thus in this sense with 28 byte blinding the security is not reduced by
* side-channel attacks like the one in [3])
*
* This countermeasure does not help if the key recovery is possible with a
* single trace.
*/
#define RSA_EXPONENT_BLINDING 28

/*
* Do an RSA private key operation
*/
int rsa_private(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	const byte_t *input,
	byte_t *output)
{
	int ret;
	dword_t olen;

	/* Temporary holding the result */
	mpi T;

	/* Temporaries holding P-1, Q-1 and the
	* exponent blinding factor, respectively. */
	mpi P1, Q1, R;

	/* Temporaries holding the results mod p resp. mod q. */
	mpi TP, TQ;

	/* Temporaries holding the blinded exponents for
	* the mod p resp. mod q computation (if used). */
	mpi DP_blind, DQ_blind;

	/* Pointers to actual exponents to be used - either the unblinded
	* or the blinded ones, depending on the presence of a PRNG. */
	mpi *DP = &ctx->DP;
	mpi *DQ = &ctx->DQ;

	/* Temporaries holding the initial input and the double
	* checked result; should be the same in the end. */
	mpi I, C;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(input != NULL);
	XDK_ASSERT(output != NULL);

	if (rsa_check_context(ctx, 1             /* private key checks */,
		f_rng != NULL /* blinding y/n       */) != 0)
	{
		set_last_error(_T("rsa_private"), _T("ERR_RSA_RNG_FAILED"), -1);
		return C_ERR;
	}

	/* MPI Initialization */
	mpi_init(&T);

	mpi_init(&P1);
	mpi_init(&Q1);
	mpi_init(&R);

	if (f_rng != NULL)
	{
		mpi_init(&DP_blind);
		mpi_init(&DQ_blind);
	}

	mpi_init(&TP); mpi_init(&TQ);

	mpi_init(&I);
	mpi_init(&C);

	/* End of MPI initialization */

	MPI_CHK(mpi_read_binary(&T, input, ctx->len));
	if (mpi_cmp_mpi(&T, &ctx->N) >= 0)
	{
		set_last_error(_T("rsa_private"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	MPI_CHK(mpi_copy(&I, &T));

	if (f_rng != NULL)
	{
		/*
		* Blinding
		* T = T * Vi mod N
		*/
		MPI_CHK(rsa_prepare_blinding(ctx, f_rng, p_rng));
		MPI_CHK(mpi_mul_mpi(&T, &T, &ctx->Vi));
		MPI_CHK(mpi_mod_mpi(&T, &T, &ctx->N));

		/*
		* Exponent blinding
		*/
		MPI_CHK(mpi_sub_int(&P1, &ctx->P, 1));
		MPI_CHK(mpi_sub_int(&Q1, &ctx->Q, 1));
		/*
		* DP_blind = ( P - 1 ) * R + DP
		*/
		MPI_CHK(mpi_fill_random(&R, RSA_EXPONENT_BLINDING,
			f_rng, p_rng));
		MPI_CHK(mpi_mul_mpi(&DP_blind, &P1, &R));
		MPI_CHK(mpi_add_mpi(&DP_blind, &DP_blind,
			&ctx->DP));

		DP = &DP_blind;

		/*
		* DQ_blind = ( Q - 1 ) * R + DQ
		*/
		MPI_CHK(mpi_fill_random(&R, RSA_EXPONENT_BLINDING,
			f_rng, p_rng));
		MPI_CHK(mpi_mul_mpi(&DQ_blind, &Q1, &R));
		MPI_CHK(mpi_add_mpi(&DQ_blind, &DQ_blind,
			&ctx->DQ));

		DQ = &DQ_blind;
	}

	/*
	* Faster decryption using the CRT
	*
	* TP = input ^ dP mod P
	* TQ = input ^ dQ mod Q
	*/

	MPI_CHK(mpi_exp_mod(&TP, &T, DP, &ctx->P, &ctx->RP));
	MPI_CHK(mpi_exp_mod(&TQ, &T, DQ, &ctx->Q, &ctx->RQ));

	/*
	* T = (TP - TQ) * (Q^-1 mod P) mod P
	*/
	MPI_CHK(mpi_sub_mpi(&T, &TP, &TQ));
	MPI_CHK(mpi_mul_mpi(&TP, &T, &ctx->QP));
	MPI_CHK(mpi_mod_mpi(&T, &TP, &ctx->P));

	/*
	* T = TQ + T * Q
	*/
	MPI_CHK(mpi_mul_mpi(&TP, &T, &ctx->Q));
	MPI_CHK(mpi_add_mpi(&T, &TQ, &TP));

	if (f_rng != NULL)
	{
		/*
		* Unblind
		* T = T * Vf mod N
		*/
		MPI_CHK(mpi_mul_mpi(&T, &T, &ctx->Vf));
		MPI_CHK(mpi_mod_mpi(&T, &T, &ctx->N));
	}

	/* Verify the result to prevent glitching attacks. */
	MPI_CHK(mpi_exp_mod(&C, &T, &ctx->E,
		&ctx->N, &ctx->RN));
	if (mpi_cmp_mpi(&C, &I) != 0)
	{
		set_last_error(_T("rsa_private"), _T("ERR_RSA_VERIFY_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	olen = ctx->len;
	MPI_CHK(mpi_write_binary(&T, output, olen));

cleanup:

	mpi_free(&P1);
	mpi_free(&Q1);
	mpi_free(&R);

	if (f_rng != NULL)
	{
		mpi_free(&DP_blind);
		mpi_free(&DQ_blind);
	}

	mpi_free(&T);

	mpi_free(&TP); mpi_free(&TQ);

	mpi_free(&C);
	mpi_free(&I);

	if (ret != 0)
	{
		set_last_error(_T("rsa_private"), _T("ERR_RSA_PRIVATE_FAILED"), -1);
		return C_ERR;
	}

	return(0);
}

/**
* Generate and apply the MGF1 operation (from PKCS#1 v2.1) to a buffer.
*
* \param dst       buffer to mask
* \param dlen      length of destination buffer
* \param src       source of the mask generation
* \param slen      length of the source buffer
* \param md_ctx    message digest context to use
*/
static int mgf_mask(byte_t *dst, dword_t dlen, byte_t *src,
	dword_t slen, md_type_t md_type, void *md_ctx)
{
	byte_t mask[MD_MAX_SIZE];
	byte_t counter[4];
	byte_t *p;
	dword_t hlen;
	dword_t i, use_len;
	int ret = 0;
	const md_info_t* md_info;

	xmem_zero(mask, MD_MAX_SIZE);
	xmem_zero(counter, 4);

	md_info = md_info_from_type(md_type);
	if (!md_info)
	{
		set_last_error(_T("mgf_mask"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	hlen = md_info->size;

	/* Generate and apply dbMask */
	p = dst;

	while (dlen > 0)
	{
		use_len = hlen;
		if (dlen < hlen)
			use_len = dlen;

		if ((ret = md_starts(md_info, md_ctx)) != 0)
			goto exit;
		if ((ret = md_update(md_info, md_ctx, src, slen)) != 0)
			goto exit;
		if ((ret = md_update(md_info, md_ctx, counter, 4)) != 0)
			goto exit;
		if ((ret = md_finish(md_info, md_ctx, mask)) != 0)
			goto exit;

		for (i = 0; i < use_len; ++i)
			*p++ ^= mask[i];

		counter[3]++;

		dlen -= use_len;
	}

exit:
	xmem_zero(mask, sizeof(mask));

	return(ret);
}

/*
* Implementation of the PKCS#1 v2.1 RSAES-OAEP-ENCRYPT function
*/
int rsa_rsaes_oaep_encrypt(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	const byte_t *label, dword_t label_len,
	dword_t ilen,
	const byte_t *input,
	byte_t *output)
{
	dword_t olen;
	int ret;
	byte_t *p = output;
	dword_t hlen;
	const md_info_t *md_info;
	void* md_ctx = NULL;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (output == NULL || input == NULL || label == NULL)
	{
		set_last_error(_T("mgf_mask"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (mode == RSA_PRIVATE && ctx->padding != RSA_PKCS_V21)
	{
		set_last_error(_T("mgf_mask"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (f_rng == NULL)
	{
		set_last_error(_T("mgf_mask"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	md_info = md_info_from_type((md_type_t)ctx->hash_id);
	if (md_info == NULL)
	{
		set_last_error(_T("mgf_mask"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	olen = ctx->len;
	hlen = md_info->size;

	/* first comparison checks for overflow */
	if (ilen + 2 * hlen + 2 < ilen || olen < ilen + 2 * hlen + 2)
	{
		set_last_error(_T("mgf_mask"), _T("ERR_RSA_RNG_FAILED"), -1);
		return C_ERR;
	}

	xmem_zero(output, olen);

	*p++ = 0;

	/* Generate a random octet string seed */
	if ((ret = f_rng(p_rng, p, hlen)) != 0)
	{
		set_last_error(_T("mgf_mask"), _T("ERR_RSA_RNG_FAILED"), -1);
		return C_ERR;
	}

	p += hlen;

	/* Construct DB */
	if ((ret = md(md_info, label, label_len, p)) != 0)
		return(ret);
	p += hlen;
	p += olen - 2 * hlen - 2 - ilen;
	*p++ = 1;
	xmem_copy(p, input, ilen);

	md_ctx = md_alloc(md_info);
	if (!md_ctx)
		goto exit;

	/* maskedDB: Apply dbMask to DB */
	if ((ret = mgf_mask(output + hlen + 1, olen - hlen - 1, output + 1, hlen, md_info->type, md_ctx)) != 0)
		goto exit;

	/* maskedSeed: Apply seedMask to seed */
	if ((ret = mgf_mask(output + 1, hlen, output + hlen + 1, olen - hlen - 1, md_info->type, md_ctx)) != 0)
		goto exit;

exit:
	if (md_ctx)
		md_free(md_info, md_ctx);

	if (ret != 0)
		return(ret);

	return((mode == RSA_PUBLIC)
		? rsa_public(ctx, output, output)
		: rsa_private(ctx, f_rng, p_rng, output, output));
}

/*
* Implementation of the PKCS#1 v2.1 RSAES-PKCS1-V1_5-ENCRYPT function
*/
int rsa_rsaes_pkcs1_v15_encrypt(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode, dword_t ilen,
	const byte_t *input,
	byte_t *output)
{
	dword_t nb_pad, olen;
	int ret;
	byte_t *p = output;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (output == NULL || input == NULL)
	{
		set_last_error(_T("rsa_rsaes_pkcs1_v15_encrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (mode == RSA_PRIVATE && ctx->padding != RSA_PKCS_V15)
	{
		set_last_error(_T("rsa_rsaes_pkcs1_v15_encrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	olen = ctx->len;

	/* first comparison checks for overflow */
	if (ilen + 11 < ilen || olen < ilen + 11)
	{
		set_last_error(_T("rsa_rsaes_pkcs1_v15_encrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	nb_pad = olen - 3 - ilen;

	*p++ = 0;
	if (mode == RSA_PUBLIC)
	{
		if (f_rng == NULL)
		{
			set_last_error(_T("rsa_rsaes_pkcs1_v15_encrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		*p++ = RSA_CRYPT;

		while (nb_pad-- > 0)
		{
			int rng_dl = 100;

			do {
				ret = f_rng(p_rng, p, 1);
			} while (*p == 0 && --rng_dl && ret == 0);

			/* Check if RNG failed to generate data */
			if (rng_dl == 0 || ret != 0)
			{
				set_last_error(_T("rsa_rsaes_pkcs1_v15_encrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
				return C_ERR;
			}

			p++;
		}
	}
	else
	{
		*p++ = RSA_SIGN;

		while (nb_pad-- > 0)
			*p++ = 0xFF;
	}

	*p++ = 0;
	xmem_copy(p, input, ilen);

	return((mode == RSA_PUBLIC)
		? rsa_public(ctx, output, output)
		: rsa_private(ctx, f_rng, p_rng, output, output));
}

/*
* Add the message padding, then do an RSA operation
*/
int rsa_pkcs1_encrypt(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode, dword_t ilen,
	const byte_t *input,
	byte_t *output)
{
	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (output == NULL || input == NULL)
	{
		set_last_error(_T("rsa_pkcs1_encrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	switch (ctx->padding)
	{
	case RSA_PKCS_V15:
		return rsa_rsaes_pkcs1_v15_encrypt(ctx, f_rng, p_rng, mode, ilen,
			input, output);

	case RSA_PKCS_V21:
		return rsa_rsaes_oaep_encrypt(ctx, f_rng, p_rng, mode, NULL, 0,
			ilen, input, output);

	default:
		set_last_error(_T("rsa_pkcs1_encrypt"), _T("ERR_RSA_INVALID_PADDING"), -1);
		return C_ERR;
	}
}

/*
* Implementation of the PKCS#1 v2.1 RSAES-OAEP-DECRYPT function
*/
int rsa_rsaes_oaep_decrypt(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	const byte_t *label, dword_t label_len,
	dword_t *olen,
	const byte_t *input,
	byte_t *output,
	dword_t output_max_len)
{
	int ret;
	dword_t ilen, i, pad_len;
	byte_t *p, bad, pad_done;
	byte_t buf[MPI_MAX_SIZE];
	byte_t lhash[MD_MAX_SIZE];
	dword_t hlen;
	const md_info_t *md_info;
	void *md_ctx = NULL;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (output == NULL || input == NULL)
	{
		set_last_error(_T("rsa_rsaes_oaep_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* Parameters sanity checks
	*/
	if (mode == RSA_PRIVATE && ctx->padding != RSA_PKCS_V21)
	{
		set_last_error(_T("rsa_rsaes_oaep_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	ilen = ctx->len;

	if (ilen < 16 || ilen > sizeof(buf))
	{
		set_last_error(_T("rsa_rsaes_oaep_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	md_info = md_info_from_type((md_type_t)ctx->hash_id);
	if (md_info == NULL)
	{
		set_last_error(_T("rsa_rsaes_oaep_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	hlen = md_info->size;

	// checking for integer underflow
	if (2 * hlen + 2 > ilen)
	{
		set_last_error(_T("rsa_rsaes_oaep_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* RSA operation
	*/
	ret = (mode == RSA_PUBLIC)
		? rsa_public(ctx, input, buf)
		: rsa_private(ctx, f_rng, p_rng, input, buf);

	if (ret != 0)
		goto cleanup;

	/*
	* Unmask data and generate lHash
	*/
	md_ctx = md_alloc(md_info);
	if (!md_ctx)
		goto cleanup;

	/* seed: Apply seedMask to maskedSeed */
	if ((ret = mgf_mask(buf + 1, hlen, buf + hlen + 1, ilen - hlen - 1, md_info->type, md_ctx)) != 0 ||
		/* DB: Apply dbMask to maskedDB */
		(ret = mgf_mask(buf + hlen + 1, ilen - hlen - 1, buf + 1, hlen, md_info->type, md_ctx)) != 0)
	{
		md_free(md_info, md_ctx);
		goto cleanup;
	}

	md_free(md_info, md_ctx);

	/* Generate lHash */
	if ((ret = md(md_info, label, label_len, lhash)) != 0)
		goto cleanup;

	/*
	* Check contents, in "constant-time"
	*/
	p = buf;
	bad = 0;

	bad |= *p++; /* First byte must be 0 */

	p += hlen; /* Skip seed */

	/* Check lHash */
	for (i = 0; i < hlen; i++)
		bad |= lhash[i] ^ *p++;

	/* Get zero-padding len, but always read till end of buffer
	* (minus one, for the 01 byte) */
	pad_len = 0;
	pad_done = 0;
	for (i = 0; i < ilen - 2 * hlen - 2; i++)
	{
		pad_done |= p[i];
		pad_len += ((pad_done | (byte_t)-pad_done) >> 7) ^ 1;
	}

	p += pad_len;
	bad |= *p++ ^ 0x01;

	/*
	* The only information "leaked" is whether the padding was correct or not
	* (eg, no data is copied if it was not correct). This meets the
	* recommendations in PKCS#1 v2.2: an opponent cannot distinguish between
	* the different error conditions.
	*/
	if (bad != 0)
	{
		set_last_error(_T("rsa_rsaes_oaep_decrypt"), _T("ERR_RSA_INVALID_PADDING"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	if (ilen - (p - buf) > output_max_len)
	{
		set_last_error(_T("rsa_rsaes_oaep_decrypt"), _T("ERR_RSA_OUTPUT_TOO_LARGE"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	*olen = ilen - (p - buf);
	xmem_copy(output, p, *olen);
	ret = 0;

cleanup:
	xmem_zero(buf, sizeof(buf));
	xmem_zero(lhash, sizeof(lhash));

	return(ret);
}

/** Turn zero-or-nonzero into zero-or-all-bits-one, without branches.
*
* \param value     The value to analyze.
* \return          Zero if \p value is zero, otherwise all-bits-one.
*/
static unsigned all_or_nothing_int(unsigned value)
{
	/* MSVC has a warning about unary minus on unsigned, but this is
	* well-defined and precisely what we want to do here */
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 4146 )
#endif
	return(-((value | -value) >> (sizeof(value) * 8 - 1)));
#if defined(_MSC_VER)
#pragma warning( pop )
#endif
}

/** Check whether a size is out of bounds, without branches.
*
* This is equivalent to `size > max`, but is likely to be compiled to
* to code using bitwise operation rather than a branch.
*
* \param size      Size to check.
* \param max       Maximum desired value for \p size.
* \return          \c 0 if `size <= max`.
* \return          \c 1 if `size > max`.
*/
static unsigned size_greater_than(dword_t size, dword_t max)
{
	/* Return the sign bit (1 for negative) of (max - size). */
	return((max - size) >> (sizeof(dword_t) * 8 - 1));
}

/** Choose between two integer values, without branches.
*
* This is equivalent to `cond ? if1 : if0`, but is likely to be compiled
* to code using bitwise operation rather than a branch.
*
* \param cond      Condition to test.
* \param if1       Value to use if \p cond is nonzero.
* \param if0       Value to use if \p cond is zero.
* \return          \c if1 if \p cond is nonzero, otherwise \c if0.
*/
static unsigned if_int(unsigned cond, unsigned if1, unsigned if0)
{
	unsigned mask = all_or_nothing_int(cond);
	return((mask & if1) | (~mask & if0));
}

/** Shift some data towards the left inside a buffer without leaking
* the length of the data through side channels.
*
* `mem_move_to_left(start, total, offset)` is functionally equivalent to
* ```
* memmove(start, start + offset, total - offset);
* xmem_zero(start + offset, 0, total - offset);
* ```
* but it strives to use a memory access pattern (and thus total timing)
* that does not depend on \p offset. This timing independence comes at
* the expense of performance.
*
* \param start     Pointer to the start of the buffer.
* \param total     Total size of the buffer.
* \param offset    Offset from which to copy \p total - \p offset bytes.
*/
static void mem_move_to_left(void *start,
	dword_t total,
	dword_t offset)
{
	volatile byte_t *buf = start;
	dword_t i, n;
	if (total == 0)
		return;
	for (i = 0; i < total; i++)
	{
		unsigned no_op = size_greater_than(total - offset, i);
		/* The first `total - offset` passes are a no-op. The last
		* `offset` passes shift the data one byte to the left and
		* zero out the last byte. */
		for (n = 0; n < total - 1; n++)
		{
			byte_t current = buf[n];
			byte_t next = buf[n + 1];
			buf[n] = if_int(no_op, current, next);
		}
		buf[total - 1] = if_int(no_op, buf[total - 1], 0);
	}
}

/*
* Implementation of the PKCS#1 v2.1 RSAES-PKCS1-V1_5-DECRYPT function
*/
int rsa_rsaes_pkcs1_v15_decrypt(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode, dword_t *olen,
	const byte_t *input,
	byte_t *output,
	dword_t output_max_len)
{
	int ret;
	dword_t ilen, i, plaintext_max_size;
	byte_t buf[MPI_MAX_SIZE];
	/* The following variables take sensitive values: their value must
	* not leak into the observable behavior of the function other than
	* the designated outputs (output, olen, return value). Otherwise
	* this would open the execution of the function to
	* side-channel-based variants of the Bleichenbacher padding oracle
	* attack. Potential side channels include overall timing, memory
	* access patterns (especially visible to an adversary who has access
	* to a shared memory cache), and branches (especially visible to
	* an adversary who has access to a shared code cache or to a shared
	* branch predictor). */
	dword_t pad_count = 0;
	unsigned bad = 0;
	byte_t pad_done = 0;
	dword_t plaintext_size = 0;
	unsigned output_too_large;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (output == NULL || input == NULL || olen == NULL)
	{
		set_last_error(_T("rsa_rsaes_pkcs1_v15_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	ilen = ctx->len;
	plaintext_max_size = (output_max_len > ilen - 11 ?
		ilen - 11 :
		output_max_len);

	if (mode == RSA_PRIVATE && ctx->padding != RSA_PKCS_V15)
	{
		set_last_error(_T("rsa_rsaes_pkcs1_v15_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (ilen < 16 || ilen > sizeof(buf))
	{
		set_last_error(_T("rsa_rsaes_pkcs1_v15_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	ret = (mode == RSA_PUBLIC)
		? rsa_public(ctx, input, buf)
		: rsa_private(ctx, f_rng, p_rng, input, buf);

	if (ret != 0)
		goto cleanup;

	/* Check and get padding length in constant time and constant
	* memory trace. The first byte must be 0. */
	bad |= buf[0];

	if (mode == RSA_PRIVATE)
	{
		/* Decode EME-PKCS1-v1_5 padding: 0x00 || 0x02 || PS || 0x00
		* where PS must be at least 8 nonzero bytes. */
		bad |= buf[1] ^ RSA_CRYPT;

		/* Read the whole buffer. Set pad_done to nonzero if we find
		* the 0x00 byte and remember the padding length in pad_count. */
		for (i = 2; i < ilen; i++)
		{
			pad_done |= ((buf[i] | (byte_t)-buf[i]) >> 7) ^ 1;
			pad_count += ((pad_done | (byte_t)-pad_done) >> 7) ^ 1;
		}
	}
	else
	{
		/* Decode EMSA-PKCS1-v1_5 padding: 0x00 || 0x01 || PS || 0x00
		* where PS must be at least 8 bytes with the value 0xFF. */
		bad |= buf[1] ^ RSA_SIGN;

		/* Read the whole buffer. Set pad_done to nonzero if we find
		* the 0x00 byte and remember the padding length in pad_count.
		* If there's a non-0xff byte in the padding, the padding is bad. */
		for (i = 2; i < ilen; i++)
		{
			pad_done |= if_int(buf[i], 0, 1);
			pad_count += if_int(pad_done, 0, 1);
			bad |= if_int(pad_done, 0, buf[i] ^ 0xFF);
		}
	}

	/* If pad_done is still zero, there's no data, only unfinished padding. */
	bad |= if_int(pad_done, 0, 1);

	/* There must be at least 8 bytes of padding. */
	bad |= size_greater_than(8, pad_count);

	/* If the padding is valid, set plaintext_size to the number of
	* remaining bytes after stripping the padding. If the padding
	* is invalid, avoid leaking this fact through the size of the
	* output: use the maximum message size that fits in the output
	* buffer. Do it without branches to avoid leaking the padding
	* validity through timing. RSA keys are small enough that all the
	* dword_t values involved fit in dword_t. */
	plaintext_size = if_int(bad,
		(unsigned)plaintext_max_size,
		(unsigned)(ilen - pad_count - 3));

	/* Set output_too_large to 0 if the plaintext fits in the output
	* buffer and to 1 otherwise. */
	output_too_large = size_greater_than(plaintext_size,
		plaintext_max_size);

	/* Set ret without branches to avoid timing attacks. Return:
	* - INVALID_PADDING if the padding is bad (bad != 0).
	* - OUTPUT_TOO_LARGE if the padding is good but the decrypted
	*   plaintext does not fit in the output buffer.
	* - 0 if the padding is correct. */
	//ret = -(int)if_int(bad, -ERR_RSA_INVALID_PADDING,
	//	if_int(output_too_large, -ERR_RSA_OUTPUT_TOO_LARGE,
	//	0));

	/* If the padding is bad or the plaintext is too large, zero the
	* data that we're about to copy to the output buffer.
	* We need to copy the same amount of data
	* from the same buffer whether the padding is good or not to
	* avoid leaking the padding validity through overall timing or
	* through memory or cache access patterns. */
	bad = all_or_nothing_int(bad | output_too_large);
	for (i = 11; i < ilen; i++)
		buf[i] &= ~bad;

	/* If the plaintext is too large, truncate it to the buffer size.
	* Copy anyway to avoid revealing the length through timing, because
	* revealing the length is as bad as revealing the padding validity
	* for a Bleichenbacher attack. */
	plaintext_size = if_int(output_too_large,
		(unsigned)plaintext_max_size,
		(unsigned)plaintext_size);

	/* Move the plaintext to the leftmost position where it can start in
	* the working buffer, i.e. make it start plaintext_max_size from
	* the end of the buffer. Do this with a memory access trace that
	* does not depend on the plaintext size. After this move, the
	* starting location of the plaintext is no longer sensitive
	* information. */
	mem_move_to_left(buf + ilen - plaintext_max_size,
		plaintext_max_size,
		plaintext_max_size - plaintext_size);

	/* Finally copy the decrypted plaintext plus trailing zeros
	* into the output buffer. */
	xmem_copy(output, buf + ilen - plaintext_max_size, plaintext_max_size);

	/* Report the amount of data we copied to the output buffer. In case
	* of errors (bad padding or output too large), the value of *olen
	* when this function returns is not specified. Making it equivalent
	* to the good case limits the risks of leaking the padding validity. */
	*olen = plaintext_size;

cleanup:
	xmem_zero(buf, sizeof(buf));

	return(ret);
}

/*
* Do an RSA operation, then remove the message padding
*/
int rsa_pkcs1_decrypt(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode, dword_t *olen,
	const byte_t *input,
	byte_t *output,
	dword_t output_max_len)
{
	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (output == NULL || input == NULL || olen == NULL)
	{
		set_last_error(_T("rsa_pkcs1_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	switch (ctx->padding)
	{
	case RSA_PKCS_V15:
		return rsa_rsaes_pkcs1_v15_decrypt(ctx, f_rng, p_rng, mode, olen,
			input, output, output_max_len);

	case RSA_PKCS_V21:
		return rsa_rsaes_oaep_decrypt(ctx, f_rng, p_rng, mode, NULL, 0,
			olen, input, output,
			output_max_len);

	default:
		set_last_error(_T("rsa_pkcs1_decrypt"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}
}

/*
* Implementation of the PKCS#1 v2.1 RSASSA-PSS-SIGN function
*/
int rsa_rsassa_pss_sign(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	int hashalg,
	dword_t hashlen,
	const byte_t *hash,
	byte_t *sig)
{
	dword_t olen;
	byte_t *p = sig;
	byte_t salt[MD_MAX_SIZE];
	dword_t slen, min_slen, hlen, offset = 0;
	int ret;
	dword_t msb;
	const md_info_t *md_info;
	void* md_ctx = NULL;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (sig == NULL)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (mode == RSA_PRIVATE && ctx->padding != RSA_PKCS_V21)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	olen = ctx->len;

	if (hashalg != MD_NONE)
	{
		/* Gather length of hash to sign */
		md_info = md_info_from_type(hashalg);
		if (md_info == NULL)
		{
			set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		hashlen = md_info->size;
	}

	md_info = md_info_from_type((md_type_t)ctx->hash_id);
	if (md_info == NULL)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	hlen = md_info->size;

	/* Calculate the largest possible salt length. Normally this is the hash
	* length, which is the maximum length the salt can have. If there is not
	* enough room, use the maximum salt length that fits. The constraint is
	* that the hash length plus the salt length plus 2 bytes must be at most
	* the key length. This complies with FIPS 186-4 §5.5 (e) and RFC 8017
	* (PKCS#1 v2.2) §9.1.1 step 3. */
	min_slen = hlen - 2;
	if (olen < hlen + min_slen + 2)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}
	else if (olen >= hlen + hlen + 2)
		slen = hlen;
	else
		slen = olen - hlen - 2;

	xmem_zero(sig, olen);

	/* Generate salt of length slen */
	if ((ret = f_rng(p_rng, salt, slen)) != 0)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_RNG_FAILED"), -1);
		return C_ERR;
	}

	/* Note: EMSA-PSS encoding is over the length of N - 1 bits */
	msb = mpi_bitlen(&ctx->N) - 1;
	p += olen - hlen - slen - 2;
	*p++ = 0x01;
	xmem_copy(p, salt, slen);
	p += slen;

	md_ctx = md_alloc(md_info);
	if (!md_ctx)
		goto exit;

	/* Generate H = Hash( M' ) */
	if ((ret = md_starts(md_info, md_ctx)) != 0)
		goto exit;
	if ((ret = md_update(md_info, md_ctx, p, 8)) != 0)
		goto exit;
	if ((ret = md_update(md_info, md_ctx, hash, hashlen)) != 0)
		goto exit;
	if ((ret = md_update(md_info, md_ctx, salt, slen)) != 0)
		goto exit;
	if ((ret = md_finish(md_info, md_ctx, p)) != 0)
		goto exit;

	/* Compensate for boundary condition when applying mask */
	if (msb % 8 == 0)
		offset = 1;

	/* maskedDB: Apply dbMask to DB */
	if ((ret = mgf_mask(sig + offset, olen - hlen - 1 - offset, p, hlen, md_info->type, md_ctx)) != 0)
		goto exit;

	msb = mpi_bitlen(&ctx->N) - 1;
	sig[0] &= 0xFF >> (olen * 8 - msb);

	p += hlen;
	*p++ = 0xBC;

	xmem_zero(salt, sizeof(salt));

exit:
	if (md_ctx)
		md_free(md_info, md_ctx);

	if (ret != 0)
		return(ret);

	return((mode == RSA_PUBLIC)
		? rsa_public(ctx, sig, sig)
		: rsa_private(ctx, f_rng, p_rng, sig, sig));
}

/*
* Implementation of the PKCS#1 v2.1 RSASSA-PKCS1-V1_5-SIGN function
*/

/* Construct a PKCS v1.5 encoding of a hashed message
*
* This is used both for signature generation and verification.
*
* Parameters:
* - md_alg:  Identifies the hash algorithm used to generate the given hash;
*            MD_NONE if raw data is signed.
* - hashlen: Length of hash in case hashlen is MD_NONE.
* - hash:    Buffer containing the hashed message or the raw data.
* - dst_len: Length of the encoded message.
* - dst:     Buffer to hold the encoded message.
*
* Assumptions:
* - hash has size hashlen if md_alg == MD_NONE.
* - hash has size corresponding to md_alg if md_alg != MD_NONE.
* - dst points to a buffer of size at least dst_len.
*
*/
static int rsa_rsassa_pkcs1_v15_encode(int hashalg,
	dword_t hashlen,
	const byte_t *hash,
	dword_t dst_len,
	byte_t *dst)
{
	dword_t oid_size = 0;
	dword_t nb_pad = dst_len;
	byte_t *p = dst;
	const char *oid = NULL;
	const md_info_t *md_info;

	/* Are we signing hashed or raw data? */
	if (hashalg != MD_NONE)
	{
		md_info = md_info_from_type(hashalg);
		if (md_info == NULL)
		{
			set_last_error(_T("rsa_rsassa_pkcs1_v15_encode"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		if (oid_get_oid_by_md(hashalg, &oid, &oid_size) != 0)
		{
			set_last_error(_T("rsa_rsassa_pkcs1_v15_encode"), _T("MBEDTLS_ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		hashlen = md_info->size;

		/* Double-check that 8 + hashlen + oid_size can be used as a
		* 1-byte ASN.1 length encoding and that there's no overflow. */
		if (8 + hashlen + oid_size >= 0x80 ||
			10 + hashlen            <  hashlen ||
			10 + hashlen + oid_size <  10 + hashlen)
		{
			set_last_error(_T("rsa_rsassa_pkcs1_v15_encode"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		/*
		* Static bounds check:
		* - Need 10 bytes for five tag-length pairs.
		*   (Insist on 1-byte length encodings to protect against variants of
		*    Bleichenbacher's forgery attack against lax PKCS#1v1.5 verification)
		* - Need hashlen bytes for hash
		* - Need oid_size bytes for hash alg OID.
		*/
		if (nb_pad < 10 + hashlen + oid_size)
		{
			set_last_error(_T("rsa_rsassa_pkcs1_v15_encode"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		nb_pad -= 10 + hashlen + oid_size;
	}
	else
	{
		if (nb_pad < hashlen)
		{
			set_last_error(_T("rsa_rsassa_pkcs1_v15_encode"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		nb_pad -= hashlen;
	}

	/* Need space for signature header and padding delimiter (3 bytes),
	* and 8 bytes for the minimal padding */
	if (nb_pad < 3 + 8)
	{
		set_last_error(_T("rsa_rsassa_pkcs1_v15_encode"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	nb_pad -= 3;

	/* Now nb_pad is the amount of memory to be filled
	* with padding, and at least 8 bytes long. */

	/* Write signature header and padding */
	*p++ = 0;
	*p++ = RSA_SIGN;
	xmem_set(p, 0xFF, nb_pad);
	p += nb_pad;
	*p++ = 0;

	/* Are we signing raw data? */
	if (hashalg == MD_NONE)
	{
		xmem_copy(p, hash, hashlen);
		return(0);
	}

	/* Signing hashed data, add corresponding ASN.1 structure
	*
	* DigestInfo ::= SEQUENCE {
	*   digestAlgorithm DigestAlgorithmIdentifier,
	*   digest Digest }
	* DigestAlgorithmIdentifier ::= AlgorithmIdentifier
	* Digest ::= OCTET STRING
	*
	* Schematic:
	* TAG-SEQ + LEN [ TAG-SEQ + LEN [ TAG-OID  + LEN [ OID  ]
	*                                 TAG-NULL + LEN [ NULL ] ]
	*                 TAG-OCTET + LEN [ HASH ] ]
	*/
	*p++ = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	*p++ = (byte_t)(0x08 + oid_size + hashlen);
	*p++ = ASN1_SEQUENCE | ASN1_CONSTRUCTED;
	*p++ = (byte_t)(0x04 + oid_size);
	*p++ = ASN1_OID;
	*p++ = (byte_t)oid_size;
	xmem_copy(p, oid, oid_size);
	p += oid_size;
	*p++ = ASN1_NULL;
	*p++ = 0x00;
	*p++ = ASN1_OCTET_STRING;
	*p++ = (byte_t)hashlen;
	xmem_copy(p, hash, hashlen);
	p += hashlen;

	/* Just a sanity-check, should be automatic
	* after the initial bounds check. */
	if (p != dst + dst_len)
	{
		xmem_zero(dst, dst_len);

		set_last_error(_T("rsa_rsassa_pkcs1_v15_encode"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Do an RSA operation to sign the message digest
*/
int rsa_rsassa_pkcs1_v15_sign(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	int hashalg,
	dword_t hashlen,
	const byte_t *hash,
	byte_t *sig)
{
	int ret;
	byte_t *sig_try = NULL, *verif = NULL;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (sig == NULL)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (mode == RSA_PRIVATE && ctx->padding != RSA_PKCS_V15)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* Prepare PKCS1-v1.5 encoding (padding and hash identifier)
	*/

	if ((ret = rsa_rsassa_pkcs1_v15_encode(hashalg, hashlen, hash,
		ctx->len, sig)) != 0)
		return(ret);

	/*
	* Call respective RSA primitive
	*/

	if (mode == RSA_PUBLIC)
	{
		/* Skip verification on a public key operation */
		return(rsa_public(ctx, sig, sig));
	}

	/* Private key operation
	*
	* In order to prevent Lenstra's attack, make the signature in a
	* temporary buffer and check it before returning it.
	*/

	sig_try = xmem_alloc(ctx->len);
	if (sig_try == NULL)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	verif = xmem_alloc(ctx->len);
	if (verif == NULL)
	{
		xmem_free(sig_try);
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_MPI_ALLOC_FAILED"), -1);
		return C_ERR;
	}

	MPI_CHK(rsa_private(ctx, f_rng, p_rng, sig, sig_try));
	MPI_CHK(rsa_public(ctx, sig_try, verif));

	if (xmem_comp(verif, sig, ctx->len) != 0)
	{
		set_last_error(_T("rsa_rsassa_pss_sign"), _T("ERR_RSA_PRIVATE_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	xmem_copy(sig, sig_try, ctx->len);

cleanup:
	xmem_free(sig_try);
	xmem_free(verif);

	return(ret);
}

/*
* Do an RSA operation to sign the message digest
*/
int rsa_pkcs1_sign(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	int hashalg,
	dword_t hashlen,
	const byte_t *hash,
	byte_t *sig)
{
	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (sig == NULL)
	{
		set_last_error(_T("rsa_pkcs1_sign"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	switch (ctx->padding)
	{
	case RSA_PKCS_V15:
		return rsa_rsassa_pkcs1_v15_sign(ctx, f_rng, p_rng, mode, hashalg,
			hashlen, hash, sig);

	case RSA_PKCS_V21:
		return rsa_rsassa_pss_sign(ctx, f_rng, p_rng, mode, hashalg,
			hashlen, hash, sig);

	default:
		set_last_error(_T("rsa_pkcs1_sign"), _T("ERR_RSA_INVALID_PADDING"), -1);
		return C_ERR;
	}
}

/*
* Implementation of the PKCS#1 v2.1 RSASSA-PSS-VERIFY function
*/
int rsa_rsassa_pss_verify_ext(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	int hashalg,
	dword_t hashlen,
	const byte_t *hash,
	md_type_t mgf1_hash_id,
	int expected_salt_len,
	const byte_t *sig)
{
	int ret;
	dword_t siglen;
	byte_t *p;
	byte_t *hash_start;
	byte_t result[MD_MAX_SIZE];
	byte_t zeros[8];
	dword_t hlen;
	dword_t observed_salt_len, msb;
	const md_info_t *md_info;
	void *md_ctx = NULL;
	byte_t buf[MPI_MAX_SIZE];

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (sig == NULL)
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (mode == RSA_PRIVATE && ctx->padding != RSA_PKCS_V21)
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	siglen = ctx->len;

	if (siglen < 16 || siglen > sizeof(buf))
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	ret = (mode == RSA_PUBLIC)
		? rsa_public(ctx, sig, buf)
		: rsa_private(ctx, f_rng, p_rng, sig, buf);

	if (ret != 0)
		return(ret);

	p = buf;

	if (buf[siglen - 1] != 0xBC)
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_INVALID_PADDING"), -1);
		return C_ERR;
	}

	if (hashalg != MD_NONE)
	{
		/* Gather length of hash to sign */
		md_info = md_info_from_type(hashalg);
		if (md_info == NULL)
		{
			set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		hashlen = md_info->size;
	}

	md_info = md_info_from_type(mgf1_hash_id);
	if (md_info == NULL)
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	hlen = md_info->size;

	xmem_zero(zeros, 8);

	/*
	* Note: EMSA-PSS verification is over the length of N - 1 bits
	*/
	msb = mpi_bitlen(&ctx->N) - 1;

	if (buf[0] >> (8 - siglen * 8 + msb))
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/* Compensate for boundary condition when applying mask */
	if (msb % 8 == 0)
	{
		p++;
		siglen -= 1;
	}

	if (siglen < hlen + 2)
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	hash_start = p + siglen - hlen - 1;

	md_ctx = md_alloc(md_info);
	if (!md_ctx)
		goto exit;

	ret = mgf_mask(p, siglen - hlen - 1, hash_start, hlen, md_info->type, md_ctx);
	if (ret != 0)
		goto exit;

	buf[0] &= 0xFF >> (siglen * 8 - msb);

	while (p < hash_start - 1 && *p == 0)
		p++;

	if (*p++ != 0x01)
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_INVALID_PADDING"), -1);
		ret = C_ERR;
		goto exit;
	}

	observed_salt_len = hash_start - p;

	if (expected_salt_len != RSA_SALT_LEN_ANY &&
		observed_salt_len != (dword_t)expected_salt_len)
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_INVALID_PADDING"), -1);
		ret = C_ERR;
		goto exit;
	}

	/*
	* Generate H = Hash( M' )
	*/
	ret = md_starts(md_info, md_ctx);
	if (ret != 0)
		goto exit;
	ret = md_update(md_info, md_ctx, zeros, 8);
	if (ret != 0)
		goto exit;
	ret = md_update(md_info, md_ctx, hash, hashlen);
	if (ret != 0)
		goto exit;
	ret = md_update(md_info, md_ctx, p, observed_salt_len);
	if (ret != 0)
		goto exit;
	ret = md_finish(md_info, md_ctx, result);
	if (ret != 0)
		goto exit;

	if (xmem_comp(hash_start, result, hlen) != 0)
	{
		set_last_error(_T("rsa_rsassa_pss_verify_ext"), _T("ERR_RSA_VERIFY_FAILED"), -1);
		ret = C_ERR;
		goto exit;
	}

exit:
	if (md_ctx)
		md_free(md_info, md_ctx);

	return(ret);
}

/*
* Simplified PKCS#1 v2.1 RSASSA-PSS-VERIFY function
*/
int rsa_rsassa_pss_verify(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	int hashalg,
	dword_t hashlen,
	const byte_t *hash,
	const byte_t *sig)
{
	md_type_t mgf1_hash_id;
	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (sig == NULL)
	{
		set_last_error(_T("rsa_rsassa_pss_verify"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	mgf1_hash_id = (ctx->hash_id != MD_NONE)
		? (md_type_t)ctx->hash_id
		: hashalg;

	return(rsa_rsassa_pss_verify_ext(ctx, f_rng, p_rng, mode,
		hashalg, hashlen, hash,
		mgf1_hash_id, RSA_SALT_LEN_ANY,
		sig));

}

/*
* Implementation of the PKCS#1 v2.1 RSASSA-PKCS1-v1_5-VERIFY function
*/
int rsa_rsassa_pkcs1_v15_verify(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	int hashalg,
	dword_t hashlen,
	const byte_t *hash,
	const byte_t *sig)
{
	int ret = 0;
	dword_t sig_len;
	byte_t *encoded = NULL, *encoded_expected = NULL;

	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (sig == NULL)
	{
		set_last_error(_T("rsa_rsassa_pkcs1_v15_verify"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	sig_len = ctx->len;

	if (mode == RSA_PRIVATE && ctx->padding != RSA_PKCS_V15)
	{
		set_last_error(_T("rsa_rsassa_pkcs1_v15_verify"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* Prepare expected PKCS1 v1.5 encoding of hash.
	*/

	if ((encoded = xmem_alloc(sig_len)) == NULL ||
		(encoded_expected = xmem_alloc(sig_len)) == NULL)
	{
		set_last_error(_T("rsa_rsassa_pkcs1_v15_verify"), _T("ERR_MPI_ALLOC_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

	if ((ret = rsa_rsassa_pkcs1_v15_encode(hashalg, hashlen, hash, sig_len,
		encoded_expected)) != 0)
		goto cleanup;

	/*
	* Apply RSA primitive to get what should be PKCS1 encoded hash.
	*/

	ret = (mode == RSA_PUBLIC)
		? rsa_public(ctx, sig, encoded)
		: rsa_private(ctx, f_rng, p_rng, sig, encoded);
	if (ret != 0)
		goto cleanup;

	/*
	* Compare
	*/

	if ((ret = xmem_comp(encoded, encoded_expected,
		sig_len)) != 0)
	{
		set_last_error(_T("rsa_rsassa_pkcs1_v15_verify"), _T("ERR_RSA_VERIFY_FAILED"), -1);
		ret = C_ERR;
		goto cleanup;
	}

cleanup:

	if (encoded != NULL)
	{
		xmem_zero(encoded, sig_len);
		xmem_free(encoded);
	}

	if (encoded_expected != NULL)
	{
		xmem_zero(encoded_expected, sig_len);
		xmem_free(encoded_expected);
	}

	return(ret);
}

/*
* Do an RSA operation and check the message digest
*/
int rsa_pkcs1_verify(rsa_context *ctx,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng,
	int mode,
	int hashalg,
	dword_t hashlen,
	const byte_t *hash,
	const byte_t *sig)
{
	XDK_ASSERT(ctx != NULL);
	XDK_ASSERT(mode == RSA_PRIVATE || mode == RSA_PUBLIC);

	if (sig == NULL)
	{
		set_last_error(_T("rsa_rsassa_pkcs1_v15_verify"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	switch (ctx->padding)
	{
	case RSA_PKCS_V15:
		return rsa_rsassa_pkcs1_v15_verify(ctx, f_rng, p_rng, mode, hashalg,
			hashlen, hash, sig);

	case RSA_PKCS_V21:
		return rsa_rsassa_pss_verify(ctx, f_rng, p_rng, mode, hashalg,
			hashlen, hash, sig);

	default:
		set_last_error(_T("rsa_rsassa_pkcs1_v15_verify"), _T("ERR_RSA_INVALID_PADDING"), -1);
		return C_ERR;
	}
}

/*
* Copy the components of an RSA key
*/
int rsa_copy(rsa_context *dst, const rsa_context *src)
{
	int ret;
	XDK_ASSERT(dst != NULL);
	XDK_ASSERT(src != NULL);

	dst->ver = src->ver;
	dst->len = src->len;

	MPI_CHK(mpi_copy(&dst->N, &src->N));
	MPI_CHK(mpi_copy(&dst->E, &src->E));

	MPI_CHK(mpi_copy(&dst->D, &src->D));
	MPI_CHK(mpi_copy(&dst->P, &src->P));
	MPI_CHK(mpi_copy(&dst->Q, &src->Q));

	MPI_CHK(mpi_copy(&dst->DP, &src->DP));
	MPI_CHK(mpi_copy(&dst->DQ, &src->DQ));
	MPI_CHK(mpi_copy(&dst->QP, &src->QP));
	MPI_CHK(mpi_copy(&dst->RP, &src->RP));
	MPI_CHK(mpi_copy(&dst->RQ, &src->RQ));

	MPI_CHK(mpi_copy(&dst->RN, &src->RN));

	MPI_CHK(mpi_copy(&dst->Vi, &src->Vi));
	MPI_CHK(mpi_copy(&dst->Vf, &src->Vf));

	dst->padding = src->padding;
	dst->hash_id = src->hash_id;

cleanup:
	if (ret != 0)
		rsa_free(dst);

	return(ret);
}

/*
* Free the components of an RSA key
*/
void rsa_free(rsa_context *ctx)
{
	if (ctx == NULL)
		return;

	mpi_free(&ctx->Vi);
	mpi_free(&ctx->Vf);
	mpi_free(&ctx->RN);
	mpi_free(&ctx->D);
	mpi_free(&ctx->Q);
	mpi_free(&ctx->P);
	mpi_free(&ctx->E);
	mpi_free(&ctx->N);

	mpi_free(&ctx->RQ);
	mpi_free(&ctx->RP);
	mpi_free(&ctx->QP);
	mpi_free(&ctx->DQ);
	mpi_free(&ctx->DP);
}

int rsa_import_pubkey(rsa_context *ctx, unsigned char **p, unsigned char* end, int ne)
{
	int i, bys;

	if ((end - *p) < 4)
	{
		set_last_error(_T("rsa_import_pubkey"), _T("ERR_RSA_BAD_INPUT_DATA"), -1);
		return (C_ERR);
	}

	/*
	* ne=0 means E then N (the keys sent by the server).
	* ne=1 means N then E (the keys stored in a keyfile).
	*/

	if (!ne) {
		bys = 0;
		for (i = 0; i < 4; i++)
		{
			bys = (bys << 8) + *p[0];
			*p++;
		}

		if (0 != mpi_read_binary(&(ctx->E), *p, bys))
		{
			set_last_error(_T("rsa_import_pubkey"), _T("ERR_RSA_KEY_GEN_FAILED"), -1);
			return (C_ERR);
		}

		*p += bys;
	}

	bys = 0;
	for (i = 0; i < 4; i++)
	{
		bys = (bys << 8) + *p[0];
		*p++;
	}

	if (0 != mpi_read_binary(&(ctx->N), *p, bys))
	{
		set_last_error(_T("rsa_import_pubkey"), _T("ERR_RSA_KEY_GEN_FAILED"), -1);
		return (C_ERR);
	}

	ctx->len = (mpi_msb(&ctx->N) + 7) >> 3;

	*p += bys;

	if (ne) {
		bys = 0;
		for (i = 0; i < 4; i++)
		{
			bys = (bys << 8) + *p[0];
			*p++;
		}

		if (0 != mpi_read_binary(&(ctx->E), *p, bys))
		{
			set_last_error(_T("rsa_import_pubkey"), _T("ERR_RSA_KEY_GEN_FAILED"), -1);
			return (C_ERR);
		}

		*p += bys;
	}

	return (0);
}

int rsa_pubkey_size(rsa_context* ctx)
{
	return 4 + mpi_size(&ctx->N) + 4 + mpi_size(&ctx->E);
}

int rsa_export_pubkey(rsa_context *ctx, unsigned char *data, int* olen, int ne)
{
	unsigned char* p = data;
	int bys;

	if (!ne)
	{
		bys = mpi_size(&ctx->E);
		*p++ = (unsigned char)((bys >> 24) & 0xFF);
		*p++ = (unsigned char)((bys >> 16) & 0xFF);
		*p++ = (unsigned char)((bys >> 8) & 0xFF);
		*p++ = (unsigned char)((bys)& 0xFF);

		mpi_write_binary(&ctx->E, p, bys);
		p += bys;
	}

	bys = mpi_size(&ctx->N);
	*p++ = (unsigned char)((bys >> 24) & 0xFF);
	*p++ = (unsigned char)((bys >> 16) & 0xFF);
	*p++ = (unsigned char)((bys >> 8) & 0xFF);
	*p++ = (unsigned char)((bys)& 0xFF);

	mpi_write_binary(&ctx->N, p, bys);
	p += bys;

	if (ne)
	{
		bys = mpi_size(&ctx->E);
		*p++ = (unsigned char)((bys >> 24) & 0xFF);
		*p++ = (unsigned char)((bys >> 16) & 0xFF);
		*p++ = (unsigned char)((bys >> 8) & 0xFF);
		*p++ = (unsigned char)((bys)& 0xFF);

		mpi_write_binary(&ctx->E, p, bys);
		p += bys;
	}

	*olen = (p - data);

	return (0);
}

int rsa_parse_der(rsa_context *rsa, unsigned char *buf, int buflen)
{
	int ret, len;
	unsigned char *p, *end;

	xmem_zero(rsa, sizeof(rsa_context));

	p = buf;
	end = buf + buflen;

	/*
	*  RSAPrivateKey ::= SEQUENCE {
	*      version           Version,
	*      modulus           INTEGER,  -- n
	*      publicExponent    INTEGER,  -- e
	*      privateExponent   INTEGER,  -- d
	*      prime1            INTEGER,  -- p
	*      prime2            INTEGER,  -- q
	*      exponent1         INTEGER,  -- d mod (p-1)
	*      exponent2         INTEGER,  -- d mod (q-1)
	*      coefficient       INTEGER,  -- (inverse of q) mod p
	*      otherPrimeInfos   OtherPrimeInfos OPTIONAL
	*  }
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		rsa_free(rsa);

		set_last_error(_T("rsa_parse_der"), _T("asn1_get_tag"), -1);
		return(C_ERR);
	}

	end = p + len;

	if ((ret = asn1_get_int(&p, end, &rsa->ver)) != 0)
	{
		rsa_free(rsa);

		set_last_error(_T("rsa_parse_der"), _T("asn1_get_tag"), -1);
		return(C_ERR);
	}

	if (rsa->ver != 0)
	{
		rsa_free(rsa);

		set_last_error(_T("rsa_parse_der"), _T("ERR_RSA_KEY_INVALID_VERSION"), -1);
		return(C_ERR);
	}

	if ((ret = asn1_get_mpi(&p, end, &rsa->N)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &rsa->E)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &rsa->D)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &rsa->P)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &rsa->Q)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &rsa->DP)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &rsa->DQ)) != 0 ||
		(ret = asn1_get_mpi(&p, end, &rsa->QP)) != 0)
	{
		rsa_free(rsa);

		set_last_error(_T("rsa_parse_der"), _T("asn1_get_mpi"), -1);
		return(C_ERR);
	}

	rsa->len = mpi_size(&rsa->N);

	if (p != end)
	{
		rsa_free(rsa);

		set_last_error(_T("rsa_parse_der"), _T("ERR_RSA_KEY_INVALID_BUFFER"), -1);
		return(C_ERR);
	}

	if ((ret = rsa_check_privkey(rsa)) != 0)
	{
		rsa_free(rsa);

		set_last_error(_T("rsa_parse_der"), _T("ERR_RSA_KEY_INVALID_BUFFER"), -1);
		return(C_ERR);
	}

	return(C_OK);
}

#define PEM_BEGIN_RSA           "-----BEGIN RSA PRIVATE KEY-----"
#define PEM_END_RSA             "-----END RSA PRIVATE KEY-----"

/*
* Parse a private RSA key
*/
int rsa_parse_key(rsa_context *rsa, unsigned char *buf, int buflen,
	unsigned char *pwd, int pwdlen)
{
	bool_t b_pem = 0;
	pem_context pem;
	dword_t use_len;

	if (strstr((const char *)buf, PEM_BEGIN_RSA) != NULL)
	{
		b_pem = 1;
	}

	if (b_pem)
	{
		pem_init(&pem);

		/* If we get there, we know the string is null-terminated */
		if (pem_read_buffer(&pem, PEM_BEGIN_RSA, PEM_END_RSA, buf, pwd, pwdlen, &use_len) != 0)
		{
			pem_free(&pem);

			set_last_error(_T("rsa_parse"), _T("pem_read_buffer"), -1);
			return(C_ERR);
		}

		if (rsa_parse_der(rsa, pem.buf, pem.buflen) != 0)
		{
			pem_free(&pem);

			set_last_error(_T("rsa_parse"), _T("pem_read_buffer"), -1);
			return(C_ERR);
		}

		pem_free(&pem);
	}
	else
	{
		if (rsa_parse_der(rsa, buf, buflen) != 0)
		{
			pem_free(&pem);

			set_last_error(_T("rsa_parse"), _T("pem_read_buffer"), -1);
			return(C_ERR);
		}
	}

	return(C_OK);
}


