/*
*  X.509 common functions for parsing and verification
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
*  The ITU-T X.509 standard defines a certificate format for PKI.
*
*  http://www.ietf.org/rfc/rfc5280.txt (Certificates and CRLs)
*  http://www.ietf.org/rfc/rfc3279.txt (Alg IDs for CRLs)
*  http://www.ietf.org/rfc/rfc2986.txt (CSRs, aka PKCS#10)
*
*  http://www.itu.int/ITU-T/studygroups/com17/languages/X.680-0207.pdf
*  http://www.itu.int/ITU-T/studygroups/com17/languages/X.690-0207.pdf
*/


#include "x509_csr.h"
#include "pem.h"
#include "oid.h"
#include "ecdsa.h"
#include "mdwrap.h"

#include "../xdkimp.h"


#if ECDSA_MAX_LEN > MPI_MAX_SIZE
#define SIGNATURE_MAX_SIZE ECDSA_MAX_LEN
#else
#define SIGNATURE_MAX_SIZE MPI_MAX_SIZE
#endif

/*
*  Version  ::=  INTEGER  {  v1(0)  }
*/
static int x509_csr_get_version(byte_t **p,
	const byte_t *end,
	int *ver)
{
	int ret;

	if ((ret = asn1_get_int(p, end, ver)) != 0)
	{
		if (ret == ERR_ASN1_UNEXPECTED_TAG)
		{
			*ver = 0;
			return(0);
		}

		set_last_error(_T("x509_crl_parse"), _T("ERR_X509_INVALID_VERSION"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* Parse a CSR in DER format
*/
int x509_csr_parse_der(x509_csr *csr,
	const byte_t *buf, dword_t buflen)
{
	int ret;
	dword_t len;
	byte_t *p, *end;
	x509_buf sig_params;

	xmem_zero(&sig_params, sizeof(x509_buf));

	/*
	* Check for valid input
	*/
	if (csr == NULL || buf == NULL || buflen == 0)
	{
		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	x509_csr_init(csr);

	/*
	* first copy the raw DER data
	*/
	p = xmem_alloc(len = buflen);

	if (p == NULL)
	{
		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_ALLOC_FAILED"), -1);
		return(C_ERR);
	}

	xmem_copy(p, buf, buflen);

	csr->raw.p = p;
	csr->raw.len = len;
	end = p + len;

	/*
	*  CertificationRequest ::= SEQUENCE {
	*       certificationRequestInfo CertificationRequestInfo,
	*       signatureAlgorithm AlgorithmIdentifier,
	*       signature          BIT STRING
	*  }
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_csr_free(csr);

		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if (len != (dword_t)(end - p))
	{
		x509_csr_free(csr);

		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	/*
	*  CertificationRequestInfo ::= SEQUENCE {
	*/
	csr->cri.p = p;

	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_csr_free(csr);

		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	end = p + len;
	csr->cri.len = end - csr->cri.p;

	/*
	*  Version  ::=  INTEGER {  v1(0) }
	*/
	if ((ret = x509_csr_get_version(&p, end, &csr->version)) != 0)
	{
		x509_csr_free(csr);
		return(ret);
	}

	if (csr->version != 0)
	{
		x509_csr_free(csr);

		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_UNKNOWN_VERSION"), -1);
		return(C_ERR);
	}

	csr->version++;

	/*
	*  subject               Name
	*/
	csr->subject_raw.p = p;

	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_csr_free(csr);

		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((ret = x509_get_name(&p, p + len, &csr->subject)) != 0)
	{
		x509_csr_free(csr);
		return(ret);
	}

	csr->subject_raw.len = p - csr->subject_raw.p;

	/*
	*  subjectPKInfo SubjectPublicKeyInfo
	*/
	if ((ret = x509_parse_subpubkey(&p, end, &csr->pk_alg, &csr->pk_ctx)) != 0)
	{
		x509_csr_free(csr);
		return(ret);
	}

	/*
	*  attributes    [0] Attributes
	*
	*  The list of possible attributes is open-ended, though RFC 2985
	*  (PKCS#9) defines a few in section 5.4. We currently don't support any,
	*  so we just ignore them. This is a safe thing to do as the worst thing
	*  that could happen is that we issue a certificate that does not match
	*  the requester's expectations - this cannot cause a violation of our
	*  signature policies.
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_CONTEXT_SPECIFIC)) != 0)
	{
		x509_csr_free(csr);

		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	p += len;

	end = csr->raw.p + csr->raw.len;

	/*
	*  signatureAlgorithm   AlgorithmIdentifier,
	*  signature            BIT STRING
	*/
	if ((ret = x509_get_alg(&p, end, &csr->sig_oid, &sig_params)) != 0)
	{
		x509_csr_free(csr);
		return(ret);
	}

	if ((ret = x509_get_sig_alg(&csr->sig_oid, &sig_params,
		&csr->sig_md, &csr->sig_pk,
		&csr->sig_opt_mgf1_md, &csr->sig_opt_sale_len)) != 0)
	{
		x509_csr_free(csr);

		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_UNKNOWN_SIG_ALG"), -1);
		return(C_ERR);
	}

	if ((ret = x509_get_sig(&p, end, &csr->sig)) != 0)
	{
		x509_csr_free(csr);
		return(ret);
	}

	if (p != end)
	{
		x509_csr_free(csr);

		set_last_error(_T("x509_csr_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* Parse a CSR, allowing for PEM or raw DER encoding
*/
int x509_csr_parse(x509_csr *csr, const byte_t *buf, dword_t buflen)
{
	int ret;
	dword_t use_len;
	pem_context pem;

	/*
	* Check for valid input
	*/
	if (csr == NULL || buf == NULL || buflen == 0)
	{
		set_last_error(_T("x509_csr_parse"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	/* Avoid calling pem_read_buffer() on non-null-terminated string */
	if (buf[buflen - 1] == '\0')
	{
		pem_init(&pem);
		ret = pem_read_buffer(&pem,
			"-----BEGIN CERTIFICATE REQUEST-----",
			"-----END CERTIFICATE REQUEST-----",
			buf, NULL, 0, &use_len);
		if (ret == ERR_PEM_NO_HEADER_FOOTER_PRESENT)
		{
			ret = pem_read_buffer(&pem,
				"-----BEGIN NEW CERTIFICATE REQUEST-----",
				"-----END NEW CERTIFICATE REQUEST-----",
				buf, NULL, 0, &use_len);
		}

		if (ret == 0)
		{
			/*
			* Was PEM encoded, parse the result
			*/
			ret = x509_csr_parse_der(csr, pem.buf, pem.buflen);
		}

		pem_free(&pem);
		if (ret != ERR_PEM_NO_HEADER_FOOTER_PRESENT)
			return(ret);
	}

	return(x509_csr_parse_der(csr, buf, buflen));
}

#define BEFORE_COLON    14
#define BC              "14"
/*
* Return an informational string about the CSR.
*/
int x509_csr_info(char *buf, dword_t size, const char *prefix,
	const x509_csr *csr)
{
	int ret;
	dword_t n;
	char *p;
	char key_size_str[BEFORE_COLON];
	const pk_info_t* pk_info;

	p = buf;
	n = size;

	ret = snprintf(p, n, "%sCSR version   : %d",
		prefix, csr->version);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%ssubject name  : ", prefix);
	X509_SAFE_SNPRINTF;
	ret = x509_dn_gets(p, n, &csr->subject);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%ssigned using  : ", prefix);
	X509_SAFE_SNPRINTF;

	ret = x509_sig_alg_gets(p, n, &csr->sig_oid, csr->sig_pk, csr->sig_md,
		csr->sig_opt_mgf1_md, csr->sig_opt_sale_len);
	X509_SAFE_SNPRINTF;

	pk_info = pk_info_from_type(csr->pk_alg);

	if ((ret = x509_key_size_helper(key_size_str, BEFORE_COLON,
		pk_info->name)) != 0)
	{
		return(ret);
	}

	ret = snprintf(p, n, "\n%s%-" BC "s: %d bits\n", prefix, key_size_str,
		(int)(*pk_info->get_bitlen)(csr->pk_ctx));
	X509_SAFE_SNPRINTF;

	return((int)(size - n));
}

#undef BEFORE_COLON
#undef BC

/*
* Initialize a CSR
*/
void x509_csr_init(x509_csr *csr)
{
	xmem_zero(csr, sizeof(x509_csr));
}

/*
* Unallocate all CSR data
*/
void x509_csr_free(x509_csr *csr)
{
	x509_name *name_cur;
	x509_name *name_prv;

	if (csr == NULL)
		return;

	if (csr->pk_alg == PK_RSA)
	{
		rsa_free((rsa_context*)csr->pk_ctx);
		xmem_free(csr->pk_ctx);
		csr->pk_ctx = NULL;
	}
	else if (csr->pk_alg == PK_ECKEY_DH || csr->pk_alg == PK_ECKEY)
	{
		ecp_keypair_free((ecp_keypair*)csr->pk_ctx);
		xmem_free(csr->pk_ctx);
		csr->pk_ctx = NULL;
	}

	name_cur = csr->subject.next;
	while (name_cur != NULL)
	{
		name_prv = name_cur;
		name_cur = name_cur->next;
		xmem_zero(name_prv, sizeof(x509_name));
		xmem_free(name_prv);
	}

	if (csr->raw.p != NULL)
	{
		xmem_zero(csr->raw.p, csr->raw.len);
		xmem_free(csr->raw.p);
	}

	xmem_zero(csr, sizeof(x509_csr));
}

void x509write_csr_init(x509write_csr *ctx)
{
	xmem_zero(ctx, sizeof(x509write_csr));
}

void x509write_csr_free(x509write_csr *ctx)
{
	asn1_free_named_data_list(&ctx->subject);
	asn1_free_named_data_list(&ctx->extensions);

	xmem_zero(ctx, sizeof(x509write_csr));
}

void x509write_csr_set_md_alg(x509write_csr *ctx, md_type_t md_alg)
{
	ctx->md_alg = md_alg;
}

void x509write_csr_set_key(x509write_csr *ctx, pk_type_t pktype, void *pk_ctx)
{
	ctx->pk_alg = pktype;
	ctx->pk_ctx = pk_ctx;
}

int x509write_csr_set_subject_name(x509write_csr *ctx,
	const char *subject_name)
{
	return x509_string_to_names(&ctx->subject, subject_name);
}

int x509write_csr_set_extension(x509write_csr *ctx,
	const char *oid, dword_t oid_len,
	const byte_t *val, dword_t val_len)
{
	return x509_set_extension(&ctx->extensions, oid, oid_len,
		0, val, val_len);
}

static dword_t csr_get_unused_bits_for_named_bitstring(byte_t bitstring,
	dword_t bit_offset)
{
	dword_t unused_bits;

	/* Count the unused bits removing trailing 0s */
	for (unused_bits = bit_offset; unused_bits < 8; unused_bits++)
		if (((bitstring >> unused_bits) & 0x1) != 0)
			break;

	return(unused_bits);
}

int x509write_csr_set_key_usage(x509write_csr *ctx, byte_t key_usage)
{
	byte_t buf[4];
	byte_t *c;
	dword_t unused_bits;
	int ret;

	c = buf + 4;

	unused_bits = csr_get_unused_bits_for_named_bitstring(key_usage, 0);
	ret = asn1_write_bitstring(&c, buf, &key_usage, 8 - unused_bits);

	if (ret < 0)
		return(ret);
	else if (ret < 3 || ret > 4)
	{
		set_last_error(_T("x509write_csr_set_key_usage"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	ret = x509write_csr_set_extension(ctx, OID_KEY_USAGE,
		OID_SIZE(OID_KEY_USAGE),
		c, (dword_t)ret);
	if (ret != 0)
		return(ret);

	return(0);
}

int x509write_csr_set_ns_cert_type(x509write_csr *ctx,
	byte_t ns_cert_type)
{
	byte_t buf[4];
	byte_t *c;
	dword_t unused_bits;
	int ret;

	c = buf + 4;

	unused_bits = csr_get_unused_bits_for_named_bitstring(ns_cert_type, 0);
	ret = asn1_write_bitstring(&c,
		buf,
		&ns_cert_type,
		8 - unused_bits);

	if (ret < 0)
		return(ret);
	else if (ret < 3 || ret > 4)
		return(ret);

	ret = x509write_csr_set_extension(ctx, OID_NS_CERT_TYPE,
		OID_SIZE(OID_NS_CERT_TYPE),
		c, (dword_t)ret);
	if (ret != 0)
		return(ret);

	return(0);
}

int x509write_csr_der(x509write_csr *ctx, byte_t *buf, dword_t size,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	const char *sig_oid;
	dword_t sig_oid_len = 0;
	byte_t *c, *c2;
	byte_t hash[64];
	byte_t sig[SIGNATURE_MAX_SIZE];
	byte_t tmp_buf[2048];
	dword_t pub_len = 0, sig_and_oid_len = 0, sig_len;
	dword_t len = 0;
	pk_type_t pk_alg;

	/*
	* Prepare data to be signed in tmp_buf
	*/
	c = tmp_buf + sizeof(tmp_buf);

	ASN1_CHK_ADD(len, x509_write_extensions(&c, tmp_buf, ctx->extensions));

	if (len)
	{
		ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, len));
		ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONSTRUCTED |
			ASN1_SEQUENCE));

		ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, len));
		ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONSTRUCTED |
			ASN1_SET));

		ASN1_CHK_ADD(len, asn1_write_oid(&c, tmp_buf, OID_PKCS9_CSR_EXT_REQ,
			OID_SIZE(OID_PKCS9_CSR_EXT_REQ)));

		ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, len));
		ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONSTRUCTED |
			ASN1_SEQUENCE));
	}

	ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONSTRUCTED |
		ASN1_CONTEXT_SPECIFIC));

	ASN1_CHK_ADD(pub_len, x509_write_pubkey_der(ctx->pk_alg, ctx->pk_ctx, tmp_buf, c - tmp_buf));
	c -= pub_len;
	len += pub_len;

	/*
	*  Subject  ::=  Name
	*/
	ASN1_CHK_ADD(len, x509_write_names(&c, tmp_buf, ctx->subject));

	/*
	*  Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
	*/
	ASN1_CHK_ADD(len, asn1_write_int(&c, tmp_buf, 0));

	ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	/*
	* Prepare signature
	*/
	ret = md(md_info_from_type(ctx->md_alg), c, len, hash);
	if (ret != 0)
		return(ret);

	if ((ret = pk_sign(ctx->pk_alg, ctx->pk_ctx, ctx->md_alg, hash, 0, sig, &sig_len, f_rng, p_rng)) != 0)
	{
		return(ret);
	}

	if (ctx->pk_alg ==  PK_RSA)
		pk_alg = PK_RSA;
	else if (ctx->pk_alg == PK_ECDSA)
		pk_alg = PK_ECDSA;
	else
	{
		set_last_error(_T("x509write_csr_der"), _T("ERR_X509_INVALID_ALG"), -1);
		return(C_ERR);
	}

	if ((ret = oid_get_oid_by_sig_alg(pk_alg, ctx->md_alg, &sig_oid, &sig_oid_len)) != 0)
	{
		return(ret);
	}

	/*
	* Write data to output buffer
	*/
	c2 = buf + size;
	ASN1_CHK_ADD(sig_and_oid_len, x509_write_sig(&c2, buf,
		sig_oid, sig_oid_len, sig, sig_len));

	if (len > (dword_t)(c2 - buf))
	{
		set_last_error(_T("x509write_csr_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	c2 -= len;
	xmem_copy(c2, c, len);

	len += sig_and_oid_len;
	ASN1_CHK_ADD(len, asn1_write_len(&c2, buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c2, buf, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	return((int)len);
}

#define PEM_BEGIN_CSR           "-----BEGIN CERTIFICATE REQUEST-----\n"
#define PEM_END_CSR             "-----END CERTIFICATE REQUEST-----\n"

int x509write_csr_pem(x509write_csr *ctx, byte_t *buf, dword_t size,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	byte_t output_buf[4096];
	dword_t olen = 0;

	if ((ret = x509write_csr_der(ctx, output_buf, sizeof(output_buf),
		f_rng, p_rng)) < 0)
	{
		return(ret);
	}

	if ((ret = pem_write_buffer(PEM_BEGIN_CSR, PEM_END_CSR,
		output_buf + sizeof(output_buf) - ret,
		ret, buf, size, &olen)) != 0)
	{
		return(ret);
	}

	return(0);
}