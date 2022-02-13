 /**
 * \file rsa.h
 *
 * \brief This file provides an API for the RSA public-key cryptosystem.
 *
 * The RSA public-key cryptosystem is defined in <em>Public-Key
 * Cryptography Standards (PKCS) #1 v1.5: RSA Encryption</em>
 * and <em>Public-Key Cryptography Standards (PKCS) #1 v2.1:
 * RSA Cryptography Specifications</em>.
 *
 */
/*
 *  Copyright (C) 2006-2018, Arm Limited (or its affiliates), All Rights Reserved
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
 *  This file is part of Mbed TLS (https://tls.mbed.org)
 */
#ifndef RSA_H
#define RSA_H


#include "../xdkdef.h"
#include "mpi.h"
#include "mdwrap.h"
#include "pkwrap.h"

typedef enum {
	RSA_HASH_NONE = 0,    /**< None. */
	RSA_HASH_MD2,       /**< The MD2 message digest. */
	RSA_HASH_MD4,       /**< The MD4 message digest. */
	RSA_HASH_MD5,       /**< The MD5 message digest. */
	RSA_HASH_SHA1,      /**< The SHA-1 message digest. */
	RSA_HASH_SHA224,    /**< The SHA-224 message digest. */
	RSA_HASH_SHA256,    /**< The SHA-256 message digest. */
	RSA_HASH_SHA384,    /**< The SHA-384 message digest. */
	RSA_HASH_SHA512,    /**< The SHA-512 message digest. */
	RSA_HASH_RIPEMD160, /**< The RIPEMD-160 message digest. */
} HASHALG;


/*
 * RSA constants
 */
#define RSA_PUBLIC      0 /**< Request private key operation. */
#define RSA_PRIVATE     1 /**< Request public key operation. */

#define RSA_PKCS_V15    0 /**< Use PKCS#1 v1.5 encoding. */
#define RSA_PKCS_V21    1 /**< Use PKCS#1 v2.1 encoding. */

#define RSA_SIGN        1 /**< Identifier for RSA signature operations. */
#define RSA_CRYPT       2 /**< Identifier for RSA encryption and decryption operations. */

#define RSA_SALT_LEN_ANY    -1

/*
 * The above constants may be used even if the RSA module is compile out,
 * eg for alternative (PKCS#11) RSA implemenations in the PK layers.
 */


/**
* \brief   The RSA context structure.
*
* \note    Direct manipulation of the members of this structure
*          is deprecated. All manipulation should instead be done through
*          the public interface functions.
*/
typedef struct rsa_context
{
	int ver;                    /*!<  Always 0.*/
	dword_t len;                 /*!<  The size of \p N in Bytes. */

	mpi N;              /*!<  The public modulus. */
	mpi E;              /*!<  The public exponent. */

	mpi D;              /*!<  The private exponent. */
	mpi P;              /*!<  The first prime factor. */
	mpi Q;              /*!<  The second prime factor. */

	mpi DP;             /*!<  <code>D % (P - 1)</code>. */
	mpi DQ;             /*!<  <code>D % (Q - 1)</code>. */
	mpi QP;             /*!<  <code>1 / (Q % P)</code>. */

	mpi RN;             /*!<  cached <code>R^2 mod N</code>. */

	mpi RP;             /*!<  cached <code>R^2 mod P</code>. */
	mpi RQ;             /*!<  cached <code>R^2 mod Q</code>. */

	mpi Vi;             /*!<  The cached blinding value. */
	mpi Vf;             /*!<  The cached un-blinding value. */

	int padding;                /*!< Selects padding mode:
								#RSA_PKCS_V15 for 1.5 padding and
								#RSA_PKCS_V21 for OAEP or PSS. */
	int hash_id;                /*!< Hash identifier of md_type_t type,
								as specified in md.h for use in the MGF
								mask generating function used in the
								EME-OAEP and EMSA-PSS encodings. */
}
rsa_context;

#ifdef __cplusplus
extern "C" {
#endif

/**
* \brief          Compute RSA prime moduli P, Q from public modulus N = PQ
*                 and a pair of private and public key.
*
* \note           This is a 'static' helper function not operating on
*                 an RSA context.Alternative implementations need not
*                 overwrite it.
*
* \param N        RSA modulus N = PQ, with P, Q to be found
* \param E        RSA public exponent
* \param D        RSA private exponent
* \param P        Pointer to MPI holding first prime factor of N on success
* \param Q        Pointer to MPI holding second prime factor of N on success
*
* \return
*-0 if successful.In this case, P and Q constitute a
*                   factorization of N.
*                 -A non - zero error code otherwise.
*
* \note           It is neither checked that P, Q are prime nor that
*                 D, E are modular inverses wrt.P - 1 and Q - 1. For that,
*use the helper function \c rsa_validate_params.
*
*/
EXP_API int rsa_deduce_primes(mpi const *N, mpi const *E,
mpi const *D,
mpi *P, mpi *Q);

/**
* \brief          Compute RSA private exponent from
*                 prime moduli and public key.
*
* \note           This is a 'static' helper function not operating on
*                 an RSA context. Alternative implementations need not
*                 overwrite it.
*
* \param P        First prime factor of RSA modulus
* \param Q        Second prime factor of RSA modulus
* \param E        RSA public exponent
* \param D        Pointer to MPI holding the private exponent on success.
*
* \return
*                 - 0 if successful. In this case, D is set to a simultaneous
*                   modular inverse of E modulo both P-1 and Q-1.
*                 - A non-zero error code otherwise.
*
* \note           This function does not check whether P and Q are primes.
*
*/
EXP_API int rsa_deduce_private_exponent(mpi const *P,
	mpi const *Q,
	mpi const *E,
	mpi *D);


/**
* \brief          Generate RSA-CRT parameters
*
* \note           This is a 'static' helper function not operating on
*                 an RSA context. Alternative implementations need not
*                 overwrite it.
*
* \param P        First prime factor of N
* \param Q        Second prime factor of N
* \param D        RSA private exponent
* \param DP       Output variable for D modulo P-1
* \param DQ       Output variable for D modulo Q-1
* \param QP       Output variable for the modular inverse of Q modulo P.
*
* \return         0 on success, non-zero error code otherwise.
*
* \note           This function does not check whether P, Q are
*                 prime and whether D is a valid private exponent.
*
*/
EXP_API int rsa_deduce_crt(const mpi *P, const mpi *Q,
	const mpi *D, mpi *DP,
	mpi *DQ, mpi *QP);


/**
* \brief          Check validity of core RSA parameters
*
* \note           This is a 'static' helper function not operating on
*                 an RSA context. Alternative implementations need not
*                 overwrite it.
*
* \param N        RSA modulus N = PQ
* \param P        First prime factor of N
* \param Q        Second prime factor of N
* \param D        RSA private exponent
* \param E        RSA public exponent
* \param f_rng    PRNG to be used for primality check, or NULL
* \param p_rng    PRNG context for f_rng, or NULL
*
* \return
*                 - 0 if the following conditions are satisfied
*                   if all relevant parameters are provided:
*                    - P prime if f_rng != NULL (%)
*                    - Q prime if f_rng != NULL (%)
*                    - 1 < N = P * Q
*                    - 1 < D, E < N
*                    - D and E are modular inverses modulo P-1 and Q-1
*                   (%) This is only done if MBEDTLS_GENPRIME is defined.
*                 - A non-zero error code otherwise.
*
* \note           The function can be used with a restricted set of arguments
*                 to perform specific checks only. E.g., calling it with
*                 (-,P,-,-,-) and a PRNG amounts to a primality check for P.
*/
EXP_API int rsa_validate_params(const mpi *N, const mpi *P,
	const mpi *Q, const mpi *D,
	const mpi *E,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng);

/**
* \brief          Check validity of RSA CRT parameters
*
* \note           This is a 'static' helper function not operating on
*                 an RSA context. Alternative implementations need not
*                 overwrite it.
*
* \param P        First prime factor of RSA modulus
* \param Q        Second prime factor of RSA modulus
* \param D        RSA private exponent
* \param DP       MPI to check for D modulo P-1
* \param DQ       MPI to check for D modulo P-1
* \param QP       MPI to check for the modular inverse of Q modulo P.
*
* \return
*                 - 0 if the following conditions are satisfied:
*                    - D = DP mod P-1 if P, D, DP != NULL
*                    - Q = DQ mod P-1 if P, D, DQ != NULL
*                    - QP = Q^-1 mod P if P, Q, QP != NULL
*                 - \c MBEDTLS_ERR_RSA_KEY_CHECK_FAILED if check failed,
*                   potentially including \c MBEDTLS_ERR_MPI_XXX if some
*                   MPI calculations failed.
*                 - \c MBEDTLS_ERR_RSA_BAD_INPUT_DATA if insufficient
*                   data was provided to check DP, DQ or QP.
*
* \note           The function can be used with a restricted set of arguments
*                 to perform specific checks only. E.g., calling it with the
*                 parameters (P, -, D, DP, -, -) will check DP = D mod P-1.
*/
EXP_API int rsa_validate_crt(const mpi *P, const mpi *Q,
	const mpi *D, const mpi *DP,
	const mpi *DQ, const mpi *QP);


/**
 * \brief          This function initializes an RSA context.
 *
 * \note           Set padding to #RSA_PKCS_V21 for the RSAES-OAEP
 *                 encryption scheme and the RSASSA-PSS signature scheme.
 *
 * \note           The \p hash_id parameter is ignored when using
 *                 #RSA_PKCS_V15 padding.
 *
 * \note           The choice of padding mode is strictly enforced for private key
 *                 operations, since there might be security concerns in
 *                 mixing padding modes. For public key operations it is
 *                 a default value, which can be overridden by calling specific
 *                 \c rsa_rsaes_xxx or \c rsa_rsassa_xxx functions.
 *
 * \note           The hash selected in \p hash_id is always used for OEAP
 *                 encryption. For PSS signatures, it is always used for
 *                 making signatures, but can be overridden for verifying them.
 *                 If set to #MD_NONE, it is always overridden.
 *
 * \param ctx      The RSA context to initialize. This must not be \c NULL.
 * \param padding  The padding mode to use. This must be either
 *                 #RSA_PKCS_V15 or #RSA_PKCS_V21.
 * \param hash_id  The hash identifier of ::md_type_t type, if
 *                 \p padding is #RSA_PKCS_V21. It is unused
 *                 otherwise.
 */
EXP_API void rsa_init(rsa_context *ctx,
                       int padding,
                       int hash_id );

/**
 * \brief          This function imports a set of core parameters into an
 *                 RSA context.
 *
 * \note           This function can be called multiple times for successive
 *                 imports, if the parameters are not simultaneously present.
 *
 *                 Any sequence of calls to this function should be followed
 *                 by a call to rsa_complete(), which checks and
 *                 completes the provided information to a ready-for-use
 *                 public or private RSA key.
 *
 * \note           See rsa_complete() for more information on which
 *                 parameters are necessary to set up a private or public
 *                 RSA key.
 *
 * \note           The imported parameters are copied and need not be preserved
 *                 for the lifetime of the RSA context being set up.
 *
 * \param ctx      The initialized RSA context to store the parameters in.
 * \param N        The RSA modulus. This may be \c NULL.
 * \param P        The first prime factor of \p N. This may be \c NULL.
 * \param Q        The second prime factor of \p N. This may be \c NULL.
 * \param D        The private exponent. This may be \c NULL.
 * \param E        The public exponent. This may be \c NULL.
 *
 * \return         \c 0 on success.
 * \return         A non-zero error code on failure.
 */
EXP_API int rsa_import(rsa_context *ctx,
                        const mpi *N,
                        const mpi *P, const mpi *Q,
                        const mpi *D, const mpi *E );

/**
 * \brief          This function imports core RSA parameters, in raw big-endian
 *                 binary format, into an RSA context.
 *
 * \note           This function can be called multiple times for successive
 *                 imports, if the parameters are not simultaneously present.
 *
 *                 Any sequence of calls to this function should be followed
 *                 by a call to rsa_complete(), which checks and
 *                 completes the provided information to a ready-for-use
 *                 public or private RSA key.
 *
 * \note           See rsa_complete() for more information on which
 *                 parameters are necessary to set up a private or public
 *                 RSA key.
 *
 * \note           The imported parameters are copied and need not be preserved
 *                 for the lifetime of the RSA context being set up.
 *
 * \param ctx      The initialized RSA context to store the parameters in.
 * \param N        The RSA modulus. This may be \c NULL.
 * \param N_len    The Byte length of \p N; it is ignored if \p N == NULL.
 * \param P        The first prime factor of \p N. This may be \c NULL.
 * \param P_len    The Byte length of \p P; it ns ignored if \p P == NULL.
 * \param Q        The second prime factor of \p N. This may be \c NULL.
 * \param Q_len    The Byte length of \p Q; it is ignored if \p Q == NULL.
 * \param D        The private exponent. This may be \c NULL.
 * \param D_len    The Byte length of \p D; it is ignored if \p D == NULL.
 * \param E        The public exponent. This may be \c NULL.
 * \param E_len    The Byte length of \p E; it is ignored if \p E == NULL.
 *
 * \return         \c 0 on success.
 * \return         A non-zero error code on failure.
 */
EXP_API int rsa_import_raw(rsa_context *ctx,
                            byte_t const *N, dword_t N_len,
                            byte_t const *P, dword_t P_len,
                            byte_t const *Q, dword_t Q_len,
                            byte_t const *D, dword_t D_len,
                            byte_t const *E, dword_t E_len );

/**
 * \brief          This function completes an RSA context from
 *                 a set of imported core parameters.
 *
 *                 To setup an RSA public key, precisely \p N and \p E
 *                 must have been imported.
 *
 *                 To setup an RSA private key, sufficient information must
 *                 be present for the other parameters to be derivable.
 *
 *                 The default implementation supports the following:
 *                 <ul><li>Derive \p P, \p Q from \p N, \p D, \p E.</li>
 *                 <li>Derive \p N, \p D from \p P, \p Q, \p E.</li></ul>
 *                 Alternative implementations need not support these.
 *
 *                 If this function runs successfully, it guarantees that
 *                 the RSA context can be used for RSA operations without
 *                 the risk of failure or crash.
 *
 * \warning        This function need not perform consistency checks
 *                 for the imported parameters. In particular, parameters that
 *                 are not needed by the implementation might be silently
 *                 discarded and left unchecked. To check the consistency
 *                 of the key material, see rsa_check_privkey().
 *
 * \param ctx      The initialized RSA context holding imported parameters.
 *
 * \return         \c 0 on success.
 * \return         #ERR_RSA_BAD_INPUT_DATA if the attempted derivations
 *                 failed.
 *
 */
EXP_API int rsa_complete(rsa_context *ctx);

/**
 * \brief          This function exports the core parameters of an RSA key.
 *
 *                 If this function runs successfully, the non-NULL buffers
 *                 pointed to by \p N, \p P, \p Q, \p D, and \p E are fully
 *                 written, with additional unused space filled leading by
 *                 zero Bytes.
 *
 *                 Possible reasons for returning
 *                 #ERR_PLATFORM_FEATURE_UNSUPPORTED:<ul>
 *                 <li>An alternative RSA implementation is in use, which
 *                 stores the key externally, and either cannot or should
 *                 not export it into RAM.</li>
 *                 <li>A SW or HW implementation might not support a certain
 *                 deduction. For example, \p P, \p Q from \p N, \p D,
 *                 and \p E if the former are not part of the
 *                 implementation.</li></ul>
 *
 *                 If the function fails due to an unsupported operation,
 *                 the RSA context stays intact and remains usable.
 *
 * \param ctx      The initialized RSA context.
 * \param N        The MPI to hold the RSA modulus.
 *                 This may be \c NULL if this field need not be exported.
 * \param P        The MPI to hold the first prime factor of \p N.
 *                 This may be \c NULL if this field need not be exported.
 * \param Q        The MPI to hold the second prime factor of \p N.
 *                 This may be \c NULL if this field need not be exported.
 * \param D        The MPI to hold the private exponent.
 *                 This may be \c NULL if this field need not be exported.
 * \param E        The MPI to hold the public exponent.
 *                 This may be \c NULL if this field need not be exported.
 *
 * \return         \c 0 on success.
 * \return         #ERR_PLATFORM_FEATURE_UNSUPPORTED if exporting the
 *                 requested parameters cannot be done due to missing
 *                 functionality or because of security policies.
 * \return         A non-zero return code on any other failure.
 *
 */
EXP_API int rsa_export(const rsa_context *ctx,
                        mpi *N, mpi *P, mpi *Q,
                        mpi *D, mpi *E );

/**
 * \brief          This function exports core parameters of an RSA key
 *                 in raw big-endian binary format.
 *
 *                 If this function runs successfully, the non-NULL buffers
 *                 pointed to by \p N, \p P, \p Q, \p D, and \p E are fully
 *                 written, with additional unused space filled leading by
 *                 zero Bytes.
 *
 *                 Possible reasons for returning
 *                 #ERR_PLATFORM_FEATURE_UNSUPPORTED:<ul>
 *                 <li>An alternative RSA implementation is in use, which
 *                 stores the key externally, and either cannot or should
 *                 not export it into RAM.</li>
 *                 <li>A SW or HW implementation might not support a certain
 *                 deduction. For example, \p P, \p Q from \p N, \p D,
 *                 and \p E if the former are not part of the
 *                 implementation.</li></ul>
 *                 If the function fails due to an unsupported operation,
 *                 the RSA context stays intact and remains usable.
 *
 * \note           The length parameters are ignored if the corresponding
 *                 buffer pointers are NULL.
 *
 * \param ctx      The initialized RSA context.
 * \param N        The Byte array to store the RSA modulus,
 *                 or \c NULL if this field need not be exported.
 * \param N_len    The size of the buffer for the modulus.
 * \param P        The Byte array to hold the first prime factor of \p N,
 *                 or \c NULL if this field need not be exported.
 * \param P_len    The size of the buffer for the first prime factor.
 * \param Q        The Byte array to hold the second prime factor of \p N,
 *                 or \c NULL if this field need not be exported.
 * \param Q_len    The size of the buffer for the second prime factor.
 * \param D        The Byte array to hold the private exponent,
 *                 or \c NULL if this field need not be exported.
 * \param D_len    The size of the buffer for the private exponent.
 * \param E        The Byte array to hold the public exponent,
 *                 or \c NULL if this field need not be exported.
 * \param E_len    The size of the buffer for the public exponent.
 *
 * \return         \c 0 on success.
 * \return         #ERR_PLATFORM_FEATURE_UNSUPPORTED if exporting the
 *                 requested parameters cannot be done due to missing
 *                 functionality or because of security policies.
 * \return         A non-zero return code on any other failure.
 */
EXP_API int rsa_export_raw(const rsa_context *ctx,
                            byte_t *N, dword_t N_len,
                            byte_t *P, dword_t P_len,
                            byte_t *Q, dword_t Q_len,
                            byte_t *D, dword_t D_len,
                            byte_t *E, dword_t E_len );

/**
 * \brief          This function exports CRT parameters of a private RSA key.
 *
 * \note           Alternative RSA implementations not using CRT-parameters
 *                 internally can implement this function based on
 *                 rsa_deduce_opt().
 *
 * \param ctx      The initialized RSA context.
 * \param DP       The MPI to hold \c D modulo `P-1`,
 *                 or \c NULL if it need not be exported.
 * \param DQ       The MPI to hold \c D modulo `Q-1`,
 *                 or \c NULL if it need not be exported.
 * \param QP       The MPI to hold modular inverse of \c Q modulo \c P,
 *                 or \c NULL if it need not be exported.
 *
 * \return         \c 0 on success.
 * \return         A non-zero error code on failure.
 *
 */
EXP_API int rsa_export_crt(const rsa_context *ctx,
                            mpi *DP, mpi *DQ, mpi *QP );

/**
 * \brief          This function sets padding for an already initialized RSA
 *                 context. See rsa_init() for details.
 *
 * \param ctx      The initialized RSA context to be configured.
 * \param padding  The padding mode to use. This must be either
 *                 #RSA_PKCS_V15 or #RSA_PKCS_V21.
 * \param hash_id  The #RSA_PKCS_V21 hash identifier.
 */
EXP_API void rsa_set_padding(rsa_context *ctx, int padding,
                              int hash_id );

/**
 * \brief          This function retrieves the length of RSA modulus in Bytes.
 *
 * \param ctx      The initialized RSA context.
 *
 * \return         The length of the RSA modulus in Bytes.
 *
 */
EXP_API dword_t rsa_get_len(const rsa_context *ctx);

/**
 * \brief          This function generates an RSA keypair.
 *
 * \note           rsa_init() must be called before this function,
 *                 to set up the RSA context.
 *
 * \param ctx      The initialized RSA context used to hold the key.
 * \param f_rng    The RNG function to be used for key generation.
 *                 This must not be \c NULL.
 * \param p_rng    The RNG context to be passed to \p f_rng.
 *                 This may be \c NULL if \p f_rng doesn't need a context.
 * \param nbits    The size of the public key in bits.
 * \param exponent The public exponent to use. For example, \c 65537.
 *                 This must be odd and greater than \c 1.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_gen_key(rsa_context *ctx,
                         int (*f_rng)(void *, byte_t *, dword_t),
                         void *p_rng,
                         dword_t nbits, int exponent );

/**
 * \brief          This function checks if a context contains at least an RSA
 *                 public key.
 *
 *                 If the function runs successfully, it is guaranteed that
 *                 enough information is present to perform an RSA public key
 *                 operation using rsa_public().
 *
 * \param ctx      The initialized RSA context to check.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 *
 */
EXP_API int rsa_check_pubkey(const rsa_context *ctx);

/**
 * \brief      This function checks if a context contains an RSA private key
 *             and perform basic consistency checks.
 *
 * \note       The consistency checks performed by this function not only
 *             ensure that rsa_private() can be called successfully
 *             on the given context, but that the various parameters are
 *             mutually consistent with high probability, in the sense that
 *             rsa_public() and rsa_private() are inverses.
 *
 * \warning    This function should catch accidental misconfigurations
 *             like swapping of parameters, but it cannot establish full
 *             trust in neither the quality nor the consistency of the key
 *             material that was used to setup the given RSA context:
 *             <ul><li>Consistency: Imported parameters that are irrelevant
 *             for the implementation might be silently dropped. If dropped,
 *             the current function does not have access to them,
 *             and therefore cannot check them. See rsa_complete().
 *             If you want to check the consistency of the entire
 *             content of an PKCS1-encoded RSA private key, for example, you
 *             should use rsa_validate_params() before setting
 *             up the RSA context.
 *             Additionally, if the implementation performs empirical checks,
 *             these checks substantiate but do not guarantee consistency.</li>
 *             <li>Quality: This function is not expected to perform
 *             extended quality assessments like checking that the prime
 *             factors are safe. Additionally, it is the responsibility of the
 *             user to ensure the trustworthiness of the source of his RSA
 *             parameters, which goes beyond what is effectively checkable
 *             by the library.</li></ul>
 *
 * \param ctx  The initialized RSA context to check.
 *
 * \return     \c 0 on success.
 * \return     An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_check_privkey(const rsa_context *ctx);

/**
 * \brief          This function checks a public-private RSA key pair.
 *
 *                 It checks each of the contexts, and makes sure they match.
 *
 * \param pub      The initialized RSA context holding the public key.
 * \param prv      The initialized RSA context holding the private key.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_check_pub_priv(const rsa_context *pub,
                                const rsa_context *prv );

/**
 * \brief          This function performs an RSA public key operation.
 *
 * \param ctx      The initialized RSA context to use.
 * \param input    The input buffer. This must be a readable buffer
 *                 of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 * \param output   The output buffer. This must be a writable buffer
 *                 of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 *
 * \note           This function does not handle message padding.
 *
 * \note           Make sure to set \p input[0] = 0 or ensure that
 *                 input is smaller than \p N.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_public(rsa_context *ctx,
                const byte_t *input,
                byte_t *output );

/**
 * \brief          This function performs an RSA private key operation.
 *
 * \note           Blinding is used if and only if a PRNG is provided.
 *
 * \note           If blinding is used, both the base of exponentation
 *                 and the exponent are blinded, providing protection
 *                 against some side-channel attacks.
 *
 * \warning        It is deprecated and a security risk to not provide
 *                 a PRNG here and thereby prevent the use of blinding.
 *                 Future versions of the library may enforce the presence
 *                 of a PRNG.
 *
 * \param ctx      The initialized RSA context to use.
 * \param f_rng    The RNG function, used for blinding. It is discouraged
 *                 and deprecated to pass \c NULL here, in which case
 *                 blinding will be omitted.
 * \param p_rng    The RNG context to pass to \p f_rng. This may be \c NULL
 *                 if \p f_rng is \c NULL or if \p f_rng doesn't need a context.
 * \param input    The input buffer. This must be a readable buffer
 *                 of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 * \param output   The output buffer. This must be a writable buffer
 *                 of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 *
 */
EXP_API int rsa_private(rsa_context *ctx,
                 int (*f_rng)(void *, byte_t *, dword_t),
                 void *p_rng,
                 const byte_t *input,
                 byte_t *output );

/**
 * \brief          This function adds the message padding, then performs an RSA
 *                 operation.
 *
 *                 It is the generic wrapper for performing a PKCS#1 encryption
 *                 operation using the \p mode from the context.
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PRIVATE mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 implicitly set to #RSA_PUBLIC.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PRIVATE and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA context to use.
 * \param f_rng    The RNG to use. It is mandatory for PKCS#1 v2.1 padding
 *                 encoding, and for PKCS#1 v1.5 padding encoding when used
 *                 with \p mode set to #RSA_PUBLIC. For PKCS#1 v1.5
 *                 padding encoding and \p mode set to #RSA_PRIVATE,
 *                 it is used for blinding and should be provided in this
 *                 case; see rsa_private() for more.
 * \param p_rng    The RNG context to be passed to \p f_rng. May be
 *                 \c NULL if \p f_rng is \c NULL or if \p f_rng doesn't
 *                 need a context argument.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PUBLIC or #RSA_PRIVATE (deprecated).
 * \param ilen     The length of the plaintext in Bytes.
 * \param input    The input data to encrypt. This must be a readable
 *                 buffer of size \p ilen Bytes. This must not be \c NULL.
 * \param output   The output buffer. This must be a writable buffer
 *                 of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_pkcs1_encrypt(rsa_context *ctx,
                       int (*f_rng)(void *, byte_t *, dword_t),
                       void *p_rng,
                       int mode, dword_t ilen,
                       const byte_t *input,
                       byte_t *output );

/**
 * \brief          This function performs a PKCS#1 v1.5 encryption operation
 *                 (RSAES-PKCS1-v1_5-ENCRYPT).
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PRIVATE mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 implicitly set to #RSA_PUBLIC.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PRIVATE and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA context to use.
 * \param f_rng    The RNG function to use. It is needed for padding generation
 *                 if \p mode is #RSA_PUBLIC. If \p mode is
 *                 #RSA_PRIVATE (discouraged), it is used for
 *                 blinding and should be provided; see rsa_private().
 * \param p_rng    The RNG context to be passed to \p f_rng. This may
 *                 be \c NULL if \p f_rng is \c NULL or if \p f_rng
 *                 doesn't need a context argument.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PUBLIC or #RSA_PRIVATE (deprecated).
 * \param ilen     The length of the plaintext in Bytes.
 * \param input    The input data to encrypt. This must be a readable
 *                 buffer of size \p ilen Bytes. This must not be \c NULL.
 * \param output   The output buffer. This must be a writable buffer
 *                 of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_rsaes_pkcs1_v15_encrypt(rsa_context *ctx,
                                 int (*f_rng)(void *, byte_t *, dword_t),
                                 void *p_rng,
                                 int mode, dword_t ilen,
                                 const byte_t *input,
                                 byte_t *output );

/**
 * \brief            This function performs a PKCS#1 v2.1 OAEP encryption
 *                   operation (RSAES-OAEP-ENCRYPT).
 *
 * \note             The output buffer must be as large as the size
 *                   of ctx->N. For example, 128 Bytes if RSA-1024 is used.
 *
 * \deprecated       It is deprecated and discouraged to call this function
 *                   in #RSA_PRIVATE mode. Future versions of the library
 *                   are likely to remove the \p mode argument and have it
 *                   implicitly set to #RSA_PUBLIC.
 *
 * \note             Alternative implementations of RSA need not support
 *                   mode being set to #RSA_PRIVATE and might instead
 *                   return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx        The initnialized RSA context to use.
 * \param f_rng      The RNG function to use. This is needed for padding
 *                   generation and must be provided.
 * \param p_rng      The RNG context to be passed to \p f_rng. This may
 *                   be \c NULL if \p f_rng doesn't need a context argument.
 * \param mode       The mode of operation. This must be either
 *                   #RSA_PUBLIC or #RSA_PRIVATE (deprecated).
 * \param label      The buffer holding the custom label to use.
 *                   This must be a readable buffer of length \p label_len
 *                   Bytes. It may be \c NULL if \p label_len is \c 0.
 * \param label_len  The length of the label in Bytes.
 * \param ilen       The length of the plaintext buffer \p input in Bytes.
 * \param input      The input data to encrypt. This must be a readable
 *                   buffer of size \p ilen Bytes. This must not be \c NULL.
 * \param output     The output buffer. This must be a writable buffer
 *                   of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                   for an 2048-bit RSA modulus.
 *
 * \return           \c 0 on success.
 * \return           An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_rsaes_oaep_encrypt(rsa_context *ctx,
                            int (*f_rng)(void *, byte_t *, dword_t),
                            void *p_rng,
                            int mode,
                            const byte_t *label, dword_t label_len,
                            dword_t ilen,
                            const byte_t *input,
                            byte_t *output );

/**
 * \brief          This function performs an RSA operation, then removes the
 *                 message padding.
 *
 *                 It is the generic wrapper for performing a PKCS#1 decryption
 *                 operation using the \p mode from the context.
 *
 * \note           The output buffer length \c output_max_len should be
 *                 as large as the size \p ctx->len of \p ctx->N (for example,
 *                 128 Bytes if RSA-1024 is used) to be able to hold an
 *                 arbitrary decrypted message. If it is not large enough to
 *                 hold the decryption of the particular ciphertext provided,
 *                 the function returns \c ERR_RSA_OUTPUT_TOO_LARGE.
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PUBLIC mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 implicitly set to #RSA_PRIVATE.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PUBLIC and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA context to use.
 * \param f_rng    The RNG function. If \p mode is #RSA_PRIVATE,
 *                 this is used for blinding and should be provided; see
 *                 rsa_private() for more. If \p mode is
 *                 #RSA_PUBLIC, it is ignored.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be
 *                 \c NULL if \p f_rng is \c NULL or doesn't need a context.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PRIVATE or #RSA_PUBLIC (deprecated).
 * \param olen     The address at which to store the length of
 *                 the plaintext. This must not be \c NULL.
 * \param input    The ciphertext buffer. This must be a readable buffer
 *                 of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 * \param output   The buffer used to hold the plaintext. This must
 *                 be a writable buffer of length \p output_max_len Bytes.
 * \param output_max_len The length in Bytes of the output buffer \p output.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_pkcs1_decrypt(rsa_context *ctx,
                       int (*f_rng)(void *, byte_t *, dword_t),
                       void *p_rng,
                       int mode, dword_t *olen,
                       const byte_t *input,
                       byte_t *output,
                       dword_t output_max_len );

/**
 * \brief          This function performs a PKCS#1 v1.5 decryption
 *                 operation (RSAES-PKCS1-v1_5-DECRYPT).
 *
 * \note           The output buffer length \c output_max_len should be
 *                 as large as the size \p ctx->len of \p ctx->N, for example,
 *                 128 Bytes if RSA-1024 is used, to be able to hold an
 *                 arbitrary decrypted message. If it is not large enough to
 *                 hold the decryption of the particular ciphertext provided,
 *                 the function returns #ERR_RSA_OUTPUT_TOO_LARGE.
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PUBLIC mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 implicitly set to #RSA_PRIVATE.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PUBLIC and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA context to use.
 * \param f_rng    The RNG function. If \p mode is #RSA_PRIVATE,
 *                 this is used for blinding and should be provided; see
 *                 rsa_private() for more. If \p mode is
 *                 #RSA_PUBLIC, it is ignored.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be
 *                 \c NULL if \p f_rng is \c NULL or doesn't need a context.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PRIVATE or #RSA_PUBLIC (deprecated).
 * \param olen     The address at which to store the length of
 *                 the plaintext. This must not be \c NULL.
 * \param input    The ciphertext buffer. This must be a readable buffer
 *                 of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 * \param output   The buffer used to hold the plaintext. This must
 *                 be a writable buffer of length \p output_max_len Bytes.
 * \param output_max_len The length in Bytes of the output buffer \p output.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 *
 */
EXP_API int rsa_rsaes_pkcs1_v15_decrypt(rsa_context *ctx,
                                 int (*f_rng)(void *, byte_t *, dword_t),
                                 void *p_rng,
                                 int mode, dword_t *olen,
                                 const byte_t *input,
                                 byte_t *output,
                                 dword_t output_max_len );

/**
 * \brief            This function performs a PKCS#1 v2.1 OAEP decryption
 *                   operation (RSAES-OAEP-DECRYPT).
 *
 * \note             The output buffer length \c output_max_len should be
 *                   as large as the size \p ctx->len of \p ctx->N, for
 *                   example, 128 Bytes if RSA-1024 is used, to be able to
 *                   hold an arbitrary decrypted message. If it is not
 *                   large enough to hold the decryption of the particular
 *                   ciphertext provided, the function returns
 *                   #ERR_RSA_OUTPUT_TOO_LARGE.
 *
 * \deprecated       It is deprecated and discouraged to call this function
 *                   in #RSA_PUBLIC mode. Future versions of the library
 *                   are likely to remove the \p mode argument and have it
 *                   implicitly set to #RSA_PRIVATE.
 *
 * \note             Alternative implementations of RSA need not support
 *                   mode being set to #RSA_PUBLIC and might instead
 *                   return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx        The initialized RSA context to use.
 * \param f_rng      The RNG function. If \p mode is #RSA_PRIVATE,
 *                   this is used for blinding and should be provided; see
 *                   rsa_private() for more. If \p mode is
 *                   #RSA_PUBLIC, it is ignored.
 * \param p_rng      The RNG context to be passed to \p f_rng. This may be
 *                   \c NULL if \p f_rng is \c NULL or doesn't need a context.
 * \param mode       The mode of operation. This must be either
 *                   #RSA_PRIVATE or #RSA_PUBLIC (deprecated).
 * \param label      The buffer holding the custom label to use.
 *                   This must be a readable buffer of length \p label_len
 *                   Bytes. It may be \c NULL if \p label_len is \c 0.
 * \param label_len  The length of the label in Bytes.
 * \param olen       The address at which to store the length of
 *                   the plaintext. This must not be \c NULL.
 * \param input      The ciphertext buffer. This must be a readable buffer
 *                   of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                   for an 2048-bit RSA modulus.
 * \param output     The buffer used to hold the plaintext. This must
 *                   be a writable buffer of length \p output_max_len Bytes.
 * \param output_max_len The length in Bytes of the output buffer \p output.
 *
 * \return         \c 0 on success.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_rsaes_oaep_decrypt(rsa_context *ctx,
                            int (*f_rng)(void *, byte_t *, dword_t),
                            void *p_rng,
                            int mode,
                            const byte_t *label, dword_t label_len,
                            dword_t *olen,
                            const byte_t *input,
                            byte_t *output,
                            dword_t output_max_len );

/**
 * \brief          This function performs a private RSA operation to sign
 *                 a message digest using PKCS#1.
 *
 *                 It is the generic wrapper for performing a PKCS#1
 *                 signature using the \p mode from the context.
 *
 * \note           The \p sig buffer must be as large as the size
 *                 of \p ctx->N. For example, 128 Bytes if RSA-1024 is used.
 *
 * \note           For PKCS#1 v2.1 encoding, see comments on
 *                 rsa_rsassa_pss_sign() for details on
 *                 \p md_alg and \p hash_id.
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PUBLIC mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 implicitly set to #RSA_PRIVATE.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PUBLIC and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA context to use.
 * \param f_rng    The RNG function to use. If the padding mode is PKCS#1 v2.1,
 *                 this must be provided. If the padding mode is PKCS#1 v1.5 and
 *                 \p mode is #RSA_PRIVATE, it is used for blinding
 *                 and should be provided; see rsa_private() for more
 *                 more. It is ignored otherwise.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be \c NULL
 *                 if \p f_rng is \c NULL or doesn't need a context argument.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PRIVATE or #RSA_PUBLIC (deprecated).
 * \param md_alg   The message-digest algorithm used to hash the original data.
 *                 Use #MD_NONE for signing raw data.
 * \param hashlen  The length of the message digest.
 *                 Ths is only used if \p md_alg is #MD_NONE.
 * \param hash     The buffer holding the message digest or raw data.
 *                 If \p md_alg is #MD_NONE, this must be a readable
 *                 buffer of length \p hashlen Bytes. If \p md_alg is not
 *                 #MD_NONE, it must be a readable buffer of length
 *                 the size of the hash corresponding to \p md_alg.
 * \param sig      The buffer to hold the signature. This must be a writable
 *                 buffer of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus. A buffer length of
 *                 #MPI_MAX_SIZE is always safe.
 *
 * \return         \c 0 if the signing operation was successful.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_pkcs1_sign(rsa_context *ctx,
                    int (*f_rng)(void *, byte_t *, dword_t),
                    void *p_rng,
                    int mode,
                    int hashalg,
                    dword_t hashlen,
                    const byte_t *hash,
                    byte_t *sig );

/**
 * \brief          This function performs a PKCS#1 v1.5 signature
 *                 operation (RSASSA-PKCS1-v1_5-SIGN).
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PUBLIC mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 implicitly set to #RSA_PRIVATE.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PUBLIC and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA context to use.
 * \param f_rng    The RNG function. If \p mode is #RSA_PRIVATE,
 *                 this is used for blinding and should be provided; see
 *                 rsa_private() for more. If \p mode is
 *                 #RSA_PUBLIC, it is ignored.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be \c NULL
 *                 if \p f_rng is \c NULL or doesn't need a context argument.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PRIVATE or #RSA_PUBLIC (deprecated).
 * \param md_alg   The message-digest algorithm used to hash the original data.
 *                 Use #MD_NONE for signing raw data.
 * \param hashlen  The length of the message digest.
 *                 Ths is only used if \p md_alg is #MD_NONE.
 * \param hash     The buffer holding the message digest or raw data.
 *                 If \p md_alg is #MD_NONE, this must be a readable
 *                 buffer of length \p hashlen Bytes. If \p md_alg is not
 *                 #MD_NONE, it must be a readable buffer of length
 *                 the size of the hash corresponding to \p md_alg.
 * \param sig      The buffer to hold the signature. This must be a writable
 *                 buffer of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus. A buffer length of
 *                 #MPI_MAX_SIZE is always safe.
 *
 * \return         \c 0 if the signing operation was successful.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_rsassa_pkcs1_v15_sign(rsa_context *ctx,
                               int (*f_rng)(void *, byte_t *, dword_t),
                               void *p_rng,
                               int mode,
                               int hashalg,
                               dword_t hashlen,
                               const byte_t *hash,
                               byte_t *sig );

/**
 * \brief          This function performs a PKCS#1 v2.1 PSS signature
 *                 operation (RSASSA-PSS-SIGN).
 *
 * \note           The \p hash_id in the RSA context is the one used for the
 *                 encoding. \p md_alg in the function call is the type of hash
 *                 that is encoded. According to <em>RFC-3447: Public-Key
 *                 Cryptography Standards (PKCS) #1 v2.1: RSA Cryptography
 *                 Specifications</em> it is advised to keep both hashes the
 *                 same.
 *
 * \note           This function always uses the maximum possible salt size,
 *                 up to the length of the payload hash. This choice of salt
 *                 size complies with FIPS 186-4 §5.5 (e) and RFC 8017 (PKCS#1
 *                 v2.2) §9.1.1 step 3. Furthermore this function enforces a
 *                 minimum salt size which is the hash size minus 2 bytes. If
 *                 this minimum size is too large given the key size (the salt
 *                 size, plus the hash size, plus 2 bytes must be no more than
 *                 the key size in bytes), this function returns
 *                 #ERR_RSA_BAD_INPUT_DATA.
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PUBLIC mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 implicitly set to #RSA_PRIVATE.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PUBLIC and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA context to use.
 * \param f_rng    The RNG function. It must not be \c NULL.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be \c NULL
 *                 if \p f_rng doesn't need a context argument.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PRIVATE or #RSA_PUBLIC (deprecated).
 * \param md_alg   The message-digest algorithm used to hash the original data.
 *                 Use #MD_NONE for signing raw data.
 * \param hashlen  The length of the message digest.
 *                 Ths is only used if \p md_alg is #MD_NONE.
 * \param hash     The buffer holding the message digest or raw data.
 *                 If \p md_alg is #MD_NONE, this must be a readable
 *                 buffer of length \p hashlen Bytes. If \p md_alg is not
 *                 #MD_NONE, it must be a readable buffer of length
 *                 the size of the hash corresponding to \p md_alg.
 * \param sig      The buffer to hold the signature. This must be a writable
 *                 buffer of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus. A buffer length of
 *                 #MPI_MAX_SIZE is always safe.
 *
 * \return         \c 0 if the signing operation was successful.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_rsassa_pss_sign(rsa_context *ctx,
                         int (*f_rng)(void *, byte_t *, dword_t),
                         void *p_rng,
                         int mode,
                         int hashalg,
                         dword_t hashlen,
                         const byte_t *hash,
                         byte_t *sig );

/**
 * \brief          This function performs a public RSA operation and checks
 *                 the message digest.
 *
 *                 This is the generic wrapper for performing a PKCS#1
 *                 verification using the mode from the context.
 *
 * \note           For PKCS#1 v2.1 encoding, see comments on
 *                 rsa_rsassa_pss_verify() about \p md_alg and
 *                 \p hash_id.
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PRIVATE mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 set to #RSA_PUBLIC.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PRIVATE and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA public key context to use.
 * \param f_rng    The RNG function to use. If \p mode is #RSA_PRIVATE,
 *                 this is used for blinding and should be provided; see
 *                 rsa_private() for more. Otherwise, it is ignored.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be
 *                 \c NULL if \p f_rng is \c NULL or doesn't need a context.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PUBLIC or #RSA_PRIVATE (deprecated).
 * \param md_alg   The message-digest algorithm used to hash the original data.
 *                 Use #MD_NONE for signing raw data.
 * \param hashlen  The length of the message digest.
 *                 This is only used if \p md_alg is #MD_NONE.
 * \param hash     The buffer holding the message digest or raw data.
 *                 If \p md_alg is #MD_NONE, this must be a readable
 *                 buffer of length \p hashlen Bytes. If \p md_alg is not
 *                 #MD_NONE, it must be a readable buffer of length
 *                 the size of the hash corresponding to \p md_alg.
 * \param sig      The buffer holding the signature. This must be a readable
 *                 buffer of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 *
 * \return         \c 0 if the verify operation was successful.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_pkcs1_verify(rsa_context *ctx,
                      int (*f_rng)(void *, byte_t *, dword_t),
                      void *p_rng,
                      int mode,
                      int hashalg,
                      dword_t hashlen,
                      const byte_t *hash,
                      const byte_t *sig );

/**
 * \brief          This function performs a PKCS#1 v1.5 verification
 *                 operation (RSASSA-PKCS1-v1_5-VERIFY).
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PRIVATE mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 set to #RSA_PUBLIC.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PRIVATE and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA public key context to use.
 * \param f_rng    The RNG function to use. If \p mode is #RSA_PRIVATE,
 *                 this is used for blinding and should be provided; see
 *                 rsa_private() for more. Otherwise, it is ignored.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be
 *                 \c NULL if \p f_rng is \c NULL or doesn't need a context.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PUBLIC or #RSA_PRIVATE (deprecated).
 * \param md_alg   The message-digest algorithm used to hash the original data.
 *                 Use #MD_NONE for signing raw data.
 * \param hashlen  The length of the message digest.
 *                 This is only used if \p md_alg is #MD_NONE.
 * \param hash     The buffer holding the message digest or raw data.
 *                 If \p md_alg is #MD_NONE, this must be a readable
 *                 buffer of length \p hashlen Bytes. If \p md_alg is not
 *                 #MD_NONE, it must be a readable buffer of length
 *                 the size of the hash corresponding to \p md_alg.
 * \param sig      The buffer holding the signature. This must be a readable
 *                 buffer of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 *
 * \return         \c 0 if the verify operation was successful.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_rsassa_pkcs1_v15_verify(rsa_context *ctx,
                                 int (*f_rng)(void *, byte_t *, dword_t),
                                 void *p_rng,
                                 int mode,
                                 int hashalg,
                                 dword_t hashlen,
                                 const byte_t *hash,
                                 const byte_t *sig );

/**
 * \brief          This function performs a PKCS#1 v2.1 PSS verification
 *                 operation (RSASSA-PSS-VERIFY).
 *
 *                 The hash function for the MGF mask generating function
 *                 is that specified in the RSA context.
 *
 * \note           The \p hash_id in the RSA context is the one used for the
 *                 verification. \p md_alg in the function call is the type of
 *                 hash that is verified. According to <em>RFC-3447: Public-Key
 *                 Cryptography Standards (PKCS) #1 v2.1: RSA Cryptography
 *                 Specifications</em> it is advised to keep both hashes the
 *                 same. If \p hash_id in the RSA context is unset,
 *                 the \p md_alg from the function call is used.
 *
 * \deprecated     It is deprecated and discouraged to call this function
 *                 in #RSA_PRIVATE mode. Future versions of the library
 *                 are likely to remove the \p mode argument and have it
 *                 implicitly set to #RSA_PUBLIC.
 *
 * \note           Alternative implementations of RSA need not support
 *                 mode being set to #RSA_PRIVATE and might instead
 *                 return #ERR_PLATFORM_FEATURE_UNSUPPORTED.
 *
 * \param ctx      The initialized RSA public key context to use.
 * \param f_rng    The RNG function to use. If \p mode is #RSA_PRIVATE,
 *                 this is used for blinding and should be provided; see
 *                 rsa_private() for more. Otherwise, it is ignored.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be
 *                 \c NULL if \p f_rng is \c NULL or doesn't need a context.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PUBLIC or #RSA_PRIVATE (deprecated).
 * \param md_alg   The message-digest algorithm used to hash the original data.
 *                 Use #MD_NONE for signing raw data.
 * \param hashlen  The length of the message digest.
 *                 This is only used if \p md_alg is #MD_NONE.
 * \param hash     The buffer holding the message digest or raw data.
 *                 If \p md_alg is #MD_NONE, this must be a readable
 *                 buffer of length \p hashlen Bytes. If \p md_alg is not
 *                 #MD_NONE, it must be a readable buffer of length
 *                 the size of the hash corresponding to \p md_alg.
 * \param sig      The buffer holding the signature. This must be a readable
 *                 buffer of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 *
 * \return         \c 0 if the verify operation was successful.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_rsassa_pss_verify(rsa_context *ctx,
                           int (*f_rng)(void *, byte_t *, dword_t),
                           void *p_rng,
                           int mode,
                           int hashalg,
                           dword_t hashlen,
                           const byte_t *hash,
                           const byte_t *sig );

/**
 * \brief          This function performs a PKCS#1 v2.1 PSS verification
 *                 operation (RSASSA-PSS-VERIFY).
 *
 *                 The hash function for the MGF mask generating function
 *                 is that specified in \p mgf1_hash_id.
 *
 * \note           The \p sig buffer must be as large as the size
 *                 of \p ctx->N. For example, 128 Bytes if RSA-1024 is used.
 *
 * \note           The \p hash_id in the RSA context is ignored.
 *
 * \param ctx      The initialized RSA public key context to use.
 * \param f_rng    The RNG function to use. If \p mode is #RSA_PRIVATE,
 *                 this is used for blinding and should be provided; see
 *                 rsa_private() for more. Otherwise, it is ignored.
 * \param p_rng    The RNG context to be passed to \p f_rng. This may be
 *                 \c NULL if \p f_rng is \c NULL or doesn't need a context.
 * \param mode     The mode of operation. This must be either
 *                 #RSA_PUBLIC or #RSA_PRIVATE.
 * \param md_alg   The message-digest algorithm used to hash the original data.
 *                 Use #MD_NONE for signing raw data.
 * \param hashlen  The length of the message digest.
 *                 This is only used if \p md_alg is #MD_NONE.
 * \param hash     The buffer holding the message digest or raw data.
 *                 If \p md_alg is #MD_NONE, this must be a readable
 *                 buffer of length \p hashlen Bytes. If \p md_alg is not
 *                 #MD_NONE, it must be a readable buffer of length
 *                 the size of the hash corresponding to \p md_alg.
 * \param mgf1_hash_id      The message digest used for mask generation.
 * \param expected_salt_len The length of the salt used in padding. Use
 *                          #RSA_SALT_LEN_ANY to accept any salt length.
 * \param sig      The buffer holding the signature. This must be a readable
 *                 buffer of length \c ctx->len Bytes. For example, \c 256 Bytes
 *                 for an 2048-bit RSA modulus.
 *
 * \return         \c 0 if the verify operation was successful.
 * \return         An \c ERR_RSA_XXX error code on failure.
 */
EXP_API int rsa_rsassa_pss_verify_ext(rsa_context *ctx,
                               int (*f_rng)(void *, byte_t *, dword_t),
                               void *p_rng,
                               int mode,
                               int hashalg,
                               dword_t hashlen,
                               const byte_t *hash,
                               md_type_t mgf1_hash_id,
                               int expected_salt_len,
                               const byte_t *sig );

/**
 * \brief          This function copies the components of an RSA context.
 *
 * \param dst      The destination context. This must be initialized.
 * \param src      The source context. This must be initialized.
 *
 * \return         \c 0 on success.
 * \return         #ERR_MPI_ALLOC_FAILED on memory allocation failure.
 */
EXP_API int rsa_copy(rsa_context *dst, const rsa_context *src);

/**
 * \brief          This function frees the components of an RSA key.
 *
 * \param ctx      The RSA context to free. May be \c NULL, in which case
 *                 this function is a no-op. If it is not \c NULL, it must
 *                 point to an initialized RSA context.
 */
EXP_API void rsa_free(rsa_context *ctx);

EXP_API int rsa_export_pubkey(rsa_context *ctx, unsigned char *data, int* olen, int ne);

EXP_API int rsa_pubkey_size(rsa_context* ctx);

EXP_API int rsa_import_pubkey(rsa_context *ctx, unsigned char **p, unsigned char* end, int ne);

EXP_API int rsa_parse_key(rsa_context *rsa, unsigned char *buf, int buflen, unsigned char *pwd, int pwdlen);

#if defined(XDK_SUPPORT_TEST)

/**
 * \brief          The RSA checkup routine.
 *
 * \return         \c 0 on success.
 * \return         \c 1 on failure.
 */
EXP_API int rsa_self_test( int verbose );

EXP_API int rsa_test_parse(int verbose);

#endif /* SELF_TEST */

#ifdef __cplusplus
}
#endif

#endif /* rsa.h */
