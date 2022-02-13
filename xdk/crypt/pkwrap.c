/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc base64 document

	@module	base64.c | implement file

	@devnote 张文权 2021.01 - 2021.12	v6.0
***********************************************************************/

/**********************************************************************
This program is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
LICENSE.GPL3 for more details.
***********************************************************************/


#include "pkwrap.h"
#include "mdwrap.h"
#include "rsa.h"
#include "ecp.h"
#include "ecdsa.h"


#include "../xdkimp.h"


static dword_t rsa_get_bitlen(const void *ctx)
{
	const rsa_context * rsa = (const rsa_context *)ctx;

	return (8 * rsa_get_len(rsa));
}

const pk_info_t rsa_info = {
	PK_RSA,
	"RSA",
	rsa_get_bitlen,
};

static dword_t eckey_get_bitlen(const void *ctx)
{
	return(((ecp_keypair *)ctx)->grp.pbits);
}

const pk_info_t eckey_info = {
	PK_ECKEY,
	"EC",
	eckey_get_bitlen,
};

const pk_info_t eckeydh_info = {
	PK_ECKEY_DH,
	"EC_DH",
	eckey_get_bitlen,         /* Same underlying key structure */
};

const pk_info_t ecdsa_info = {
	PK_ECDSA,
	"ECDSA",
	eckey_get_bitlen,     /* Compatible key structures */
};


const pk_info_t* pk_info_from_type(pk_type_t pk)
{
	switch (pk)
	{
	case PK_RSA:
		return &rsa_info;
	case PK_ECKEY:
		return &eckey_info;
	case PK_ECKEY_DH:
		return &eckeydh_info;
	case PK_ECDSA:
		return &ecdsa_info;
	default:
		return NULL;
	}
}

/*
* Verify a signature with options
*/
int pk_verify_ext(pk_type_t pktype, void *pk_ctx, md_type_t opt_mgf1_md, int opt_salt_len,
	md_type_t md_alg,
	const byte_t *hash, dword_t hash_len,
	const byte_t *sig, dword_t sig_len)
{
	const pk_info_t* pk_info;
	const md_info_t* md_info;
	int ret;
	ecdsa_context ecdsa;
	dword_t n;

	pk_info = pk_info_from_type(pktype);
	if (pk_info == NULL)
		return C_ERR;

	md_info = md_info_from_type(md_alg);
	//if (md_info == NULL)
		//return C_ERR;

	if (pktype == PK_RSASSA_PSS)
	{
#if SIZE_MAX > UINT_MAX
		if (md_alg == MD_NONE && UINT_MAX < hash_len)
			return(C_ERR);
#endif /* SIZE_MAX > UINT_MAX */

		if (sig_len < rsa_get_len((rsa_context*)pk_ctx))
		{
			set_last_error(_T("pk_verify_ext"), _T("ERR_RSA_VERIFY_FAILED"), -1);
			return C_ERR;
		}

		ret = rsa_rsassa_pss_verify_ext((rsa_context*)(pk_ctx),
			NULL, NULL, RSA_PUBLIC,
			md_alg, hash_len, hash, 
			opt_mgf1_md,
			opt_salt_len,
			sig);
		if (ret != 0)
			return(ret);

		if (sig_len > rsa_get_len((rsa_context*)pk_ctx))
			return(C_ERR);

		return C_OK;
	}
	else if (pktype == PK_RSA)
	{
		n = rsa_get_len((rsa_context*)pk_ctx);
		if (sig_len < n)
			return C_ERR;
		return rsa_pkcs1_verify((rsa_context*)pk_ctx, NULL, NULL, RSA_PUBLIC, md_alg, hash_len, hash, sig);
	}
	else if (pktype == PK_ECDSA)
	{
		return (ecdsa_read_signature((ecdsa_context *)pk_ctx, hash, hash_len, sig, sig_len) == 0) ? C_ERR : C_OK;
	}
	else if (pktype == PK_ECKEY || pktype == PK_ECKEY_DH)
	{
		ecdsa_init(&ecdsa);

		if ((ret = ecdsa_from_keypair(&ecdsa, (ecp_keypair*)pk_ctx)) == 0)
		{
			ret = (ecdsa_read_signature(&ecdsa, hash, hash_len, sig, sig_len) == 0) ? C_ERR : C_OK;
		}

		ecdsa_free(&ecdsa);
		return ret;
	}

	return C_ERR;
}

/*
* Verify a signature with options
*/
int pk_verify(pk_type_t pktype, void *pk_ctx,
	md_type_t md_alg,
	const byte_t *hash, dword_t hash_len,
	const byte_t *sig, dword_t sig_len)
{

	return(pk_verify_ext(pktype, pk_ctx, 0, 0, md_alg, hash, hash_len, sig, sig_len));
}

/*
* Make a signature
*/
int pk_sign(pk_type_t pktype, void *pk_ctx, md_type_t md_alg,
	const byte_t *hash, dword_t hash_len,
	byte_t *sig, dword_t *sig_len,
	int(*f_rng)(void *, byte_t *, dword_t), void *p_rng)
{
	int ret;
	ecdsa_context ecdsa;

	switch (pktype)
	{
	case PK_RSA:
		*sig_len = rsa_get_len((rsa_context*)pk_ctx);
		return(rsa_pkcs1_sign((rsa_context*)pk_ctx, f_rng, p_rng, RSA_PRIVATE, md_alg, hash_len, hash, sig));
	case PK_ECKEY:
		ecdsa_init(&ecdsa);
		if ((ret = ecdsa_from_keypair(&ecdsa, (ecp_keypair*)pk_ctx)) == 0)
		{
			ret = ecdsa_write_signature(&ecdsa, md_alg, hash, hash_len, sig, sig_len, f_rng, p_rng);
		}
		ecdsa_free(&ecdsa);
		return(ret);
	case PK_ECDSA:
		return(ecdsa_write_signature((ecdsa_context *)pk_ctx, md_alg, hash, hash_len, sig, sig_len, f_rng, p_rng));
	default:
		return C_ERR;
	}
}

/*
* Decrypt message
*/
int pk_decrypt(pk_type_t pktype, void *pk_ctx,
	const byte_t *input, dword_t ilen,
	byte_t *output, dword_t *olen, dword_t osize,
	int(*f_rng)(void *, byte_t *, dword_t), void *p_rng)
{
	switch (pktype)
	{
	case PK_RSA:
		if (ilen != rsa_get_len((rsa_context*)pk_ctx))
			return C_ERR;

		return(rsa_pkcs1_decrypt((rsa_context*)pk_ctx, f_rng, p_rng, RSA_PRIVATE, olen, input, output, osize));
	default:
		return C_ERR;
	}
}

/*
* Encrypt message
*/
int pk_encrypt(pk_type_t pktype, void *pk_ctx,
	const byte_t *input, dword_t ilen,
	byte_t *output, dword_t *olen, dword_t osize,
	int(*f_rng)(void *, byte_t *, dword_t), void *p_rng)
{
	switch (pktype)
	{
	case PK_RSA:
		*olen = rsa_get_len((rsa_context*)pk_ctx);
		return(rsa_pkcs1_encrypt((rsa_context*)pk_ctx, f_rng, p_rng, RSA_PUBLIC, ilen, input, output));
	default:
		return 0;
	}
}

/*
* Check public-private key pair
*/
int pk_check_pair(pk_type_t pktype, void *pub_ctx, void *prv_ctx)
{
	switch (pktype)
	{
	case PK_RSA:
		return(rsa_check_pub_priv((rsa_context *)pub_ctx, (rsa_context *)prv_ctx));
	case PK_ECKEY:
	case PK_ECKEY_DH:
		return(ecp_check_pub_priv((ecp_keypair *)pub_ctx, (ecp_keypair *)prv_ctx));
	case PK_ECDSA:
		return(ecp_check_pub_priv((ecp_keypair *)pub_ctx, (ecp_keypair *)prv_ctx));
	default:
		return C_ERR;
	}
}