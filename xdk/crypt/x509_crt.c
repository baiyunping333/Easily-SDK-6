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


#include "x509_crt.h"
#include "pem.h"
#include "oid.h"
#include "ecdsa.h"
#include "sha1.h"
#include "mdwrap.h"

#include "../xdkimp.h"


/*
* Default profile
*/
const x509_crt_profile x509_crt_profile_default =
{
	/* Allow SHA-1 (weak, but still safe in controlled environments) */
	X509_ID_FLAG(MD_SHA1) |
	/* Only SHA-2 hashes */
	X509_ID_FLAG(MD_SHA224) |
	X509_ID_FLAG(MD_SHA256) |
	X509_ID_FLAG(MD_SHA384) |
	X509_ID_FLAG(MD_SHA512),
	0xFFFFFFF, /* Any PK alg    */
	0xFFFFFFF, /* Any curve     */
	2048,
};

/*
* Next-default profile
*/
const x509_crt_profile x509_crt_profile_next =
{
	/* Hashes from SHA-256 and above */
	X509_ID_FLAG(MD_SHA256) |
	X509_ID_FLAG(MD_SHA384) |
	X509_ID_FLAG(MD_SHA512),
	0xFFFFFFF, /* Any PK alg    */
	/* Curves at or above 128-bit security level */
	X509_ID_FLAG(ECP_DP_SECP256R1) |
	X509_ID_FLAG(ECP_DP_SECP384R1) |
	X509_ID_FLAG(ECP_DP_SECP521R1) |
	X509_ID_FLAG(ECP_DP_BP256R1) |
	X509_ID_FLAG(ECP_DP_BP384R1) |
	X509_ID_FLAG(ECP_DP_BP512R1) |
	X509_ID_FLAG(ECP_DP_SECP256K1),
	2048,
};

/*
* NSA Suite B Profile
*/
const x509_crt_profile x509_crt_profile_suiteb =
{
	/* Only SHA-256 and 384 */
	X509_ID_FLAG(MD_SHA256) |
	X509_ID_FLAG(MD_SHA384),
	/* Only ECDSA */
	X509_ID_FLAG(PK_ECDSA) |
	X509_ID_FLAG(PK_ECKEY),
	/* Only NIST P-256 and P-384 */
	X509_ID_FLAG(ECP_DP_SECP256R1) |
	X509_ID_FLAG(ECP_DP_SECP384R1),
	0,
};

/*
* Check md_alg against profile
* Return 0 if md_alg is acceptable for this profile, -1 otherwise
*/
static int x509_profile_check_md_alg(const x509_crt_profile *profile,
	md_type_t md_alg)
{
	if (md_alg == MD_NONE)
		return(-1);

	if ((profile->allowed_mds & X509_ID_FLAG(md_alg)) != 0)
		return(0);

	return(-1);
}

/*
* Check pk_alg against profile
* Return 0 if pk_alg is acceptable for this profile, -1 otherwise
*/
static int x509_profile_check_pk_alg(const x509_crt_profile *profile,
	pk_type_t pk_alg)
{
	if (pk_alg == PK_NONE)
		return(-1);

	if ((profile->allowed_pks & X509_ID_FLAG(pk_alg)) != 0)
		return(0);

	return(-1);
}

/*
* Check key against profile
* Return 0 if pk is acceptable for this profile, -1 otherwise
*/
static int x509_profile_check_key(const x509_crt_profile *profile,
	pk_type_t pk, void* ctx)
{
	const pk_info_t* pk_info;
	
	pk_info = pk_info_from_type(pk);

	if (pk == PK_RSA || pk == PK_RSASSA_PSS)
	{
		if ((*pk_info->get_bitlen)(ctx) >= profile->rsa_min_bitlen)
			return(0);

		return(-1);
	}

	if (pk == PK_ECDSA ||
		pk == PK_ECKEY ||
		pk == PK_ECKEY_DH)
	{
		ecp_group_id gid = ((ecp_keypair*)ctx)->grp.id;

		if (gid == ECP_DP_NONE)
			return(-1);

		if ((profile->allowed_curves & X509_ID_FLAG(gid)) != 0)
			return(0);

		return(-1);
	}

	return(-1);
}

/*
* Like xmem_comp, but case-insensitive and always returns -1 if different
*/
static int x509_memcasecmp(const void *s1, const void *s2, dword_t len)
{
	dword_t i;
	byte_t diff;
	const byte_t *n1 = s1, *n2 = s2;

	for (i = 0; i < len; i++)
	{
		diff = n1[i] ^ n2[i];

		if (diff == 0)
			continue;

		if (diff == 32 &&
			((n1[i] >= 'a' && n1[i] <= 'z') ||
			(n1[i] >= 'A' && n1[i] <= 'Z')))
		{
			continue;
		}

		return(-1);
	}

	return(0);
}

/*
* Return 0 if name matches wildcard, -1 otherwise
*/
static int x509_check_wildcard(const char *cn, const x509_buf *name)
{
	dword_t i;
	dword_t cn_idx = 0, cn_len = strlen(cn);

	/* We can't have a match if there is no wildcard to match */
	if (name->len < 3 || name->p[0] != '*' || name->p[1] != '.')
		return(-1);

	for (i = 0; i < cn_len; ++i)
	{
		if (cn[i] == '.')
		{
			cn_idx = i;
			break;
		}
	}

	if (cn_idx == 0)
		return(-1);

	if (cn_len - cn_idx == name->len - 1 &&
		x509_memcasecmp(name->p + 1, cn + cn_idx, name->len - 1) == 0)
	{
		return(0);
	}

	return(-1);
}

/*
* Compare two X.509 strings, case-insensitive, and allowing for some encoding
* variations (but not all).
*
* Return 0 if equal, -1 otherwise.
*/
static int x509_string_cmp(const x509_buf *a, const x509_buf *b)
{
	if (a->tag == b->tag &&
		a->len == b->len &&
		xmem_comp(a->p, b->p, b->len) == 0)
	{
		return(0);
	}

	if ((a->tag == ASN1_UTF8_STRING || a->tag == ASN1_PRINTABLE_STRING) &&
		(b->tag == ASN1_UTF8_STRING || b->tag == ASN1_PRINTABLE_STRING) &&
		a->len == b->len &&
		x509_memcasecmp(a->p, b->p, b->len) == 0)
	{
		return(0);
	}

	return(-1);
}

/*
* Compare two X.509 Names (aka rdnSequence).
*
* See RFC 5280 section 7.1, though we don't implement the whole algorithm:
* we sometimes return unequal when the full algorithm would return equal,
* but never the other way. (In particular, we don't do Unicode normalisation
* or space folding.)
*
* Return 0 if equal, -1 otherwise.
*/
static int x509_name_cmp(const x509_name *a, const x509_name *b)
{
	/* Avoid recursion, it might not be optimised by the compiler */
	while (a != NULL || b != NULL)
	{
		if (a == NULL || b == NULL)
			return(-1);

		/* type */
		if (a->oid.tag != b->oid.tag ||
			a->oid.len != b->oid.len ||
			xmem_comp(a->oid.p, b->oid.p, b->oid.len) != 0)
		{
			return(-1);
		}

		/* value */
		if (x509_string_cmp(&a->val, &b->val) != 0)
			return(-1);

		/* structure of the list of sets */
		if (a->next_merged != b->next_merged)
			return(-1);

		a = a->next;
		b = b->next;
	}

	/* a == NULL == b */
	return(0);
}

/*
* Reset (init or clear) a verify_chain
*/
static void x509_crt_verify_chain_reset(
	x509_crt_verify_chain *ver_chain)
{
	dword_t i;

	for (i = 0; i < X509_MAX_VERIFY_CHAIN_SIZE; i++)
	{
		ver_chain->items[i].crt = NULL;
		ver_chain->items[i].flags = (dword_t)-1;
	}

	ver_chain->len = 0;
}

/*
* X.509 v3 extensions
*
*/
static int x509_get_crt_ext(byte_t **p,
	const byte_t *end,
	x509_crt *crt)
{
	int ret;
	dword_t len;
	byte_t *end_ext_data, *end_ext_octet;

	if (*p == end)
		return(0);

	if ((ret = x509_get_ext(p, end, &crt->v3_ext, 3)) != 0)
		return(ret);

	end = crt->v3_ext.p + crt->v3_ext.len;
	while (*p < end)
	{
		/*
		* Extension  ::=  SEQUENCE  {
		*      extnID      OBJECT IDENTIFIER,
		*      critical    BOOLEAN DEFAULT FALSE,
		*      extnValue   OCTET STRING  }
		*/
		x509_buf extn_oid = { 0, 0, NULL };
		int is_critical = 0; /* DEFAULT FALSE */
		int ext_type = 0;

		if ((ret = asn1_get_tag(p, end, &len,
			ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
		{
			set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		end_ext_data = *p + len;

		/* Get extension ID */
		if ((ret = asn1_get_tag(p, end_ext_data, &extn_oid.len,
			ASN1_OID)) != 0)
		{
			set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		extn_oid.tag = ASN1_OID;
		extn_oid.p = *p;
		*p += extn_oid.len;

		/* Get optional critical */
		if ((ret = asn1_get_bool(p, end_ext_data, &is_critical)) != 0 &&
			(ret != ERR_ASN1_UNEXPECTED_TAG))
		{
			set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		/* Data should be octet string type */
		if ((ret = asn1_get_tag(p, end_ext_data, &len,
			ASN1_OCTET_STRING)) != 0)
		{
			set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		end_ext_octet = *p + len;

		if (end_ext_octet != end_ext_data)
		{
			set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		/*
		* Detect supported extensions
		*/
		ret = oid_get_x509_ext_type(&extn_oid, &ext_type);

		if (ret != 0)
		{
			/* No parser found, skip extension */
			*p = end_ext_octet;

			if (is_critical)
			{
				set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
				return(C_ERR);
			}
			continue;
		}

		/* Forbid repeated extensions */
		if ((crt->ext_types & ext_type) != 0)
		{
			set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		crt->ext_types |= ext_type;

		switch (ext_type)
		{
		case X509_EXT_BASIC_CONSTRAINTS:
			/* Parse basic constraints */
			if ((ret = x509_get_basic_constraints(p, end_ext_octet,
				&crt->ca_istrue, &crt->max_pathlen)) != 0)
				return(ret);
			break;

		case X509_EXT_KEY_USAGE:
			/* Parse key usage */
			if ((ret = x509_get_key_usage(p, end_ext_octet,
				&crt->key_usage)) != 0)
				return(ret);
			break;

		case X509_EXT_EXTENDED_KEY_USAGE:
			/* Parse extended key usage */
			if ((ret = x509_get_ext_key_usage(p, end_ext_octet,
				&crt->ext_key_usage)) != 0)
				return(ret);
			break;

		case X509_EXT_SUBJECT_ALT_NAME:
			/* Parse subject alt name */
			if ((ret = x509_get_subject_alt_name(p, end_ext_octet,
				&crt->subject_alt_names)) != 0)
				return(ret);
			break;

		case X509_EXT_NS_CERT_TYPE:
			/* Parse netscape certificate type */
			if ((ret = x509_get_ns_cert_type(p, end_ext_octet,
				&crt->ns_cert_type)) != 0)
				return(ret);
			break;

		default:
			set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_FEATURE_UNAVAILABLE"), -1);
			return(C_ERR);
		}
	}

	if (*p != end)
	{
		set_last_error(_T("x509_get_crt_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	return(0);
}


/*
* Parse and fill a single X.509 certificate in DER format
*/
static int x509_crt_parse_der_core(x509_crt *crt, const byte_t *buf,
	dword_t buflen)
{
	int ret;
	dword_t len;
	byte_t *p, *end, *crt_end;
	x509_buf sig_params1, sig_params2, sig_oid2;

	xmem_zero(&sig_params1, sizeof(x509_buf));
	xmem_zero(&sig_params2, sizeof(x509_buf));
	xmem_zero(&sig_oid2, sizeof(x509_buf));

	/*
	* Check for valid input
	*/
	if (crt == NULL || buf == NULL)
	{
		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	// Use the original buffer until we figure out actual length
	p = (byte_t*)buf;
	len = buflen;
	end = p + len;

	/*
	* Certificate  ::=  SEQUENCE  {
	*      tbsCertificate       TBSCertificate,
	*      signatureAlgorithm   AlgorithmIdentifier,
	*      signatureValue       BIT STRING  }
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_crt_free(crt);

		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if (len > (dword_t)(end - p))
	{
		x509_crt_free(crt);

		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}
	crt_end = p + len;

	// Create and populate a new buffer for the raw field
	crt->raw.len = crt_end - buf;
	crt->raw.p = p = xmem_alloc(crt->raw.len);
	if (p == NULL)
	{
		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_ALLOC_FAILED"), -1);
		return(C_ERR);
	}

	xmem_copy(p, buf, crt->raw.len);

	// Direct pointers to the new buffer
	p += crt->raw.len - len;
	end = crt_end = p + len;

	/*
	* TBSCertificate  ::=  SEQUENCE  {
	*/
	crt->tbs.p = p;

	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_crt_free(crt);

		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	end = p + len;
	crt->tbs.len = end - crt->tbs.p;

	/*
	* Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
	*
	* CertificateSerialNumber  ::=  INTEGER
	*
	* signature            AlgorithmIdentifier
	*/
	if ((ret = x509_get_version(&p, end, &crt->version)) != 0 ||
		(ret = x509_get_serial(&p, end, &crt->serial)) != 0 ||
		(ret = x509_get_alg(&p, end, &crt->sig_oid,
		&sig_params1)) != 0)
	{
		x509_crt_free(crt);
		return(ret);
	}

	if (crt->version < 0 || crt->version > 2)
	{
		x509_crt_free(crt);
		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_UNKNOWN_VERSION"), -1);
		return(C_ERR);
	}

	crt->version++;

	if ((ret = x509_get_sig_alg(&crt->sig_oid, &sig_params1,
		&crt->sig_md, &crt->sig_pk,
		&crt->sig_opt_mgf1_md, &crt->sig_opt_salt_len)) != 0)
	{
		x509_crt_free(crt);
		return(ret);
	}

	/*
	* issuer               Name
	*/
	crt->issuer_raw.p = p;

	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_crt_free(crt);

		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((ret = x509_get_name(&p, p + len, &crt->issuer)) != 0)
	{
		x509_crt_free(crt);

		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	crt->issuer_raw.len = p - crt->issuer_raw.p;

	/*
	* Validity ::= SEQUENCE {
	*      notBefore      Time,
	*      notAfter       Time }
	*
	*/
	if ((ret = x509_get_dates(&p, end, &crt->valid_from,
		&crt->valid_to)) != 0)
	{
		x509_crt_free(crt);
		return(ret);
	}

	/*
	* subject              Name
	*/
	crt->subject_raw.p = p;

	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_crt_free(crt);
		return(ret);
	}

	if (len && (ret = x509_get_name(&p, p + len, &crt->subject)) != 0)
	{
		x509_crt_free(crt);
		return(ret);
	}

	crt->subject_raw.len = p - crt->subject_raw.p;

	/*
	* SubjectPublicKeyInfo
	*/
	if ((ret = x509_parse_subpubkey(&p, end, &crt->pk_alg, &crt->pk_ctx)) != 0)
	{
		x509_crt_free(crt);
		return(ret);
	}

	/*
	*  issuerUniqueID  [1]  IMPLICIT UniqueIdentifier OPTIONAL,
	*                       -- If present, version shall be v2 or v3
	*  subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL,
	*                       -- If present, version shall be v2 or v3
	*  extensions      [3]  EXPLICIT Extensions OPTIONAL
	*                       -- If present, version shall be v3
	*/
	if (crt->version == 2 || crt->version == 3)
	{
		ret = x509_get_uid(&p, end, &crt->issuer_id, 1);
		if (ret != 0)
		{
			x509_crt_free(crt);
			return(ret);
		}
	}

	if (crt->version == 2 || crt->version == 3)
	{
		ret = x509_get_uid(&p, end, &crt->subject_id, 2);
		if (ret != 0)
		{
			x509_crt_free(crt);
			return(ret);
		}
	}

	if (crt->version == 3)
	{
		ret = x509_get_crt_ext(&p, end, crt);
		if (ret != 0)
		{
			x509_crt_free(crt);
			return(ret);
		}
	}

	if (p != end)
	{
		x509_crt_free(crt);

		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	end = crt_end;

	/*
	*  }
	*  -- end of TBSCertificate
	*
	*  signatureAlgorithm   AlgorithmIdentifier,
	*  signatureValue       BIT STRING
	*/
	if ((ret = x509_get_alg(&p, end, &sig_oid2, &sig_params2)) != 0)
	{
		x509_crt_free(crt);
		return(ret);
	}

	if (crt->sig_oid.len != sig_oid2.len ||
		xmem_comp(crt->sig_oid.p, sig_oid2.p, crt->sig_oid.len) != 0 ||
		sig_params1.len != sig_params2.len ||
		(sig_params1.len != 0 &&
		xmem_comp(sig_params1.p, sig_params2.p, sig_params1.len) != 0))
	{
		x509_crt_free(crt);

		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_SIG_MISMATCH"), -1);
		return(C_ERR);
	}

	if ((ret = x509_get_sig(&p, end, &crt->sig)) != 0)
	{
		x509_crt_free(crt);
		return(ret);
	}

	if (p != end)
	{
		x509_crt_free(crt);

		set_last_error(_T("x509_crt_parse_der_core"), _T("ERR_X509_SIG_MISMATCH"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* Parse one X.509 certificate in DER format from a buffer and add them to a
* chained list
*/
int x509_crt_parse_der(x509_crt *chain, const byte_t *buf,
	dword_t buflen)
{
	int ret;
	x509_crt *crt = chain, *prev = NULL;

	/*
	* Check for valid input
	*/
	if (crt == NULL || buf == NULL)
	{
		set_last_error(_T("x509_crt_parse_der"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	while (crt->version != 0 && crt->next != NULL)
	{
		prev = crt;
		crt = crt->next;
	}

	/*
	* Add new certificate on the end of the chain if needed.
	*/
	if (crt->version != 0 && crt->next == NULL)
	{
		crt->next = xmem_alloc(sizeof(x509_crt));

		if (crt->next == NULL)
		{
			set_last_error(_T("x509_crt_parse_der"), _T("ERR_X509_ALLOC_FAILED"), -1);
			return(C_ERR);
		}

		prev = crt;
		x509_crt_init(crt->next);
		crt = crt->next;
	}

	if ((ret = x509_crt_parse_der_core(crt, buf, buflen)) != 0)
	{
		if (prev)
			prev->next = NULL;

		if (crt != chain)
			xmem_free(crt);

		return(ret);
	}

	return(0);
}

/*
* Parse one or more PEM certificates from a buffer and add them to the chained
* list
*/
int x509_crt_parse(x509_crt *chain, const byte_t *buf, dword_t buflen)
{
	int success = 0, first_error = 0, total_failed = 0;
	int buf_format = X509_FORMAT_DER;

	/*
	* Check for valid input
	*/
	if (chain == NULL || buf == NULL)
	{
		set_last_error(_T("x509_crt_parse"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	/*
	* Determine buffer content. Buffer contains either one DER certificate or
	* one or more PEM certificates.
	*/
	if (strstr((const char *)buf, "-----BEGIN CERTIFICATE-----") != NULL)
	{
		buf_format = X509_FORMAT_PEM;
	}

	if (buf_format == X509_FORMAT_DER)
		return x509_crt_parse_der(chain, buf, buflen);

	if (buf_format == X509_FORMAT_PEM)
	{
		int ret;
		pem_context pem;

		/* 1 rather than 0 since the terminating NULL byte is counted in */
		while (buflen > 1)
		{
			dword_t use_len;
			pem_init(&pem);

			/* If we get there, we know the string is null-terminated */
			ret = pem_read_buffer(&pem,
				"-----BEGIN CERTIFICATE-----",
				"-----END CERTIFICATE-----",
				buf, NULL, 0, &use_len);

			if (ret == 0)
			{
				/*
				* Was PEM encoded
				*/
				buflen -= use_len;
				buf += use_len;
			}
			else if (ret != ERR_PEM_NO_HEADER_FOOTER_PRESENT)
			{
				pem_free(&pem);

				/*
				* PEM header and footer were found
				*/
				buflen -= use_len;
				buf += use_len;

				if (first_error == 0)
					first_error = ret;

				total_failed++;
				continue;
			}
			else
				break;

			ret = x509_crt_parse_der(chain, pem.buf, pem.buflen);

			pem_free(&pem);

			if (ret != 0)
			{
				/*
				* Quit parsing on a memory error
				*/
				if (first_error == 0)
					first_error = ret;

				total_failed++;
				continue;
			}

			success = 1;
		}
	}

	if (success)
		return(total_failed);
	else if (first_error)
		return(first_error);
	else
	{
		set_last_error(_T("x509_crt_parse"), _T("ERR_X509_CERT_UNKNOWN_FORMAT"), -1);
		return(C_ERR);
	}
}

static int x509_info_subject_alt_name(char **buf, dword_t *size,
	const x509_sequence *subject_alt_name)
{
	dword_t i;
	dword_t n = *size;
	char *p = *buf;
	const x509_sequence *cur = subject_alt_name;
	const char *sep = "";
	dword_t sep_len = 0;

	while (cur != NULL)
	{
		if (cur->buf.len + sep_len >= n)
		{
			*p = '\0';

			set_last_error(_T("x509_info_subject_alt_name"), _T("ERR_X509_BUFFER_TOO_SMALL"), -1);
			return(C_ERR);
		}

		n -= cur->buf.len + sep_len;
		for (i = 0; i < sep_len; i++)
			*p++ = sep[i];
		for (i = 0; i < cur->buf.len; i++)
			*p++ = cur->buf.p[i];

		sep = ", ";
		sep_len = 2;

		cur = cur->next;
	}

	*p = '\0';

	*size = n;
	*buf = p;

	return(0);
}

#define PRINT_ITEM(i)                           \
    {                                           \
        ret = snprintf( p, n, "%s" i, sep );    \
        X509_SAFE_SNPRINTF;                        \
        sep = ", ";                             \
    }

#define CERT_TYPE(type,name)                    \
    if( ns_cert_type & (type) )                 \
        PRINT_ITEM( name );

static int x509_info_cert_type(char **buf, dword_t *size,
	byte_t ns_cert_type)
{
	int ret;
	dword_t n = *size;
	char *p = *buf;
	const char *sep = "";

	CERT_TYPE(X509_NS_CERT_TYPE_SSL_CLIENT, "SSL Client");
	CERT_TYPE(X509_NS_CERT_TYPE_SSL_SERVER, "SSL Server");
	CERT_TYPE(X509_NS_CERT_TYPE_EMAIL, "Email");
	CERT_TYPE(X509_NS_CERT_TYPE_OBJECT_SIGNING, "Object Signing");
	CERT_TYPE(X509_NS_CERT_TYPE_RESERVED, "Reserved");
	CERT_TYPE(X509_NS_CERT_TYPE_SSL_CA, "SSL CA");
	CERT_TYPE(X509_NS_CERT_TYPE_EMAIL_CA, "Email CA");
	CERT_TYPE(X509_NS_CERT_TYPE_OBJECT_SIGNING_CA, "Object Signing CA");

	*size = n;
	*buf = p;

	return(0);
}

#define KEY_USAGE(code,name)    \
    if( key_usage & (code) )    \
        PRINT_ITEM( name );

static int x509_info_key_usage(char **buf, dword_t *size,
	unsigned int key_usage)
{
	int ret;
	dword_t n = *size;
	char *p = *buf;
	const char *sep = "";

	KEY_USAGE(X509_KU_DIGITAL_SIGNATURE, "Digital Signature");
	KEY_USAGE(X509_KU_NON_REPUDIATION, "Non Repudiation");
	KEY_USAGE(X509_KU_KEY_ENCIPHERMENT, "Key Encipherment");
	KEY_USAGE(X509_KU_DATA_ENCIPHERMENT, "Data Encipherment");
	KEY_USAGE(X509_KU_KEY_AGREEMENT, "Key Agreement");
	KEY_USAGE(X509_KU_KEY_CERT_SIGN, "Key Cert Sign");
	KEY_USAGE(X509_KU_CRL_SIGN, "CRL Sign");
	KEY_USAGE(X509_KU_ENCIPHER_ONLY, "Encipher Only");
	KEY_USAGE(X509_KU_DECIPHER_ONLY, "Decipher Only");

	*size = n;
	*buf = p;

	return(0);
}

static int x509_info_ext_key_usage(char **buf, dword_t *size,
	const x509_sequence *extended_key_usage)
{
	int ret;
	const char *desc;
	dword_t n = *size;
	char *p = *buf;
	const x509_sequence *cur = extended_key_usage;
	const char *sep = "";

	while (cur != NULL)
	{
		if (oid_get_extended_key_usage(&cur->buf, &desc) != 0)
			desc = "???";

		ret = snprintf(p, n, "%s%s", sep, desc);
		X509_SAFE_SNPRINTF;

		sep = ", ";

		cur = cur->next;
	}

	*size = n;
	*buf = p;

	return(0);
}

/*
* Return an informational string about the certificate.
*/
#define BEFORE_COLON    18
#define BC              "18"

int x509_crt_info(char *buf, dword_t size, const char *prefix,
	const x509_crt *crt)
{
	int ret;
	dword_t n;
	char *p;
	char key_size_str[BEFORE_COLON];
	const pk_info_t* pk_info;
	int bitlen;

	p = buf;
	n = size;

	if (NULL == crt)
	{
		ret = snprintf(p, n, "\nCertificate is uninitialised!\n");
		X509_SAFE_SNPRINTF;

		return((int)(size - n));
	}

	ret = snprintf(p, n, "%scert. version     : %d\n",
		prefix, crt->version);
	X509_SAFE_SNPRINTF;
	ret = snprintf(p, n, "%sserial number     : ",
		prefix);
	X509_SAFE_SNPRINTF;

	ret = x509_serial_gets(p, n, &crt->serial);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%sissuer name       : ", prefix);
	X509_SAFE_SNPRINTF;
	ret = x509_dn_gets(p, n, &crt->issuer);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%ssubject name      : ", prefix);
	X509_SAFE_SNPRINTF;
	ret = x509_dn_gets(p, n, &crt->subject);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%sissued  on        : " \
		"%04d-%02d-%02d %02d:%02d:%02d", prefix,
		crt->valid_from.year, crt->valid_from.mon,
		crt->valid_from.day, crt->valid_from.hour,
		crt->valid_from.min, crt->valid_from.sec);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%sexpires on        : " \
		"%04d-%02d-%02d %02d:%02d:%02d", prefix,
		crt->valid_to.year, crt->valid_to.mon,
		crt->valid_to.day, crt->valid_to.hour,
		crt->valid_to.min, crt->valid_to.sec);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%ssigned using      : ", prefix);
	X509_SAFE_SNPRINTF;

	ret = x509_sig_alg_gets(p, n, &crt->sig_oid, crt->sig_pk,
		crt->sig_md, crt->sig_opt_mgf1_md, crt->sig_opt_salt_len);
	X509_SAFE_SNPRINTF;

	pk_info = pk_info_from_type(crt->pk_alg);
	/* Key size */
	if ((ret = x509_key_size_helper(key_size_str, BEFORE_COLON, pk_info->name)) != 0)
	{
		return(ret);
	}

	bitlen = (int)(*pk_info->get_bitlen)((void*)crt->pk_ctx);
	ret = snprintf(p, n, "\n%s%-" BC "s: %d bits", prefix, key_size_str, bitlen);
	X509_SAFE_SNPRINTF;

	/*
	* Optional extensions
	*/

	if (crt->ext_types & X509_EXT_BASIC_CONSTRAINTS)
	{
		ret = snprintf(p, n, "\n%sbasic constraints : CA=%s", prefix,
			crt->ca_istrue ? "true" : "false");
		X509_SAFE_SNPRINTF;

		if (crt->max_pathlen > 0)
		{
			ret = snprintf(p, n, ", max_pathlen=%d", crt->max_pathlen - 1);
			X509_SAFE_SNPRINTF;
		}
	}

	if (crt->ext_types & X509_EXT_SUBJECT_ALT_NAME)
	{
		ret = snprintf(p, n, "\n%ssubject alt name  : ", prefix);
		X509_SAFE_SNPRINTF;

		if ((ret = x509_info_subject_alt_name(&p, &n,
			&crt->subject_alt_names)) != 0)
			return(ret);
	}

	if (crt->ext_types & X509_EXT_NS_CERT_TYPE)
	{
		ret = snprintf(p, n, "\n%scert. type        : ", prefix);
		X509_SAFE_SNPRINTF;

		if ((ret = x509_info_cert_type(&p, &n, crt->ns_cert_type)) != 0)
			return(ret);
	}

	if (crt->ext_types & X509_EXT_KEY_USAGE)
	{
		ret = snprintf(p, n, "\n%skey usage         : ", prefix);
		X509_SAFE_SNPRINTF;

		if ((ret = x509_info_key_usage(&p, &n, crt->key_usage)) != 0)
			return(ret);
	}

	if (crt->ext_types & X509_EXT_EXTENDED_KEY_USAGE)
	{
		ret = snprintf(p, n, "\n%sext key usage     : ", prefix);
		X509_SAFE_SNPRINTF;

		if ((ret = x509_info_ext_key_usage(&p, &n,
			&crt->ext_key_usage)) != 0)
			return(ret);
	}

	ret = snprintf(p, n, "\n");
	X509_SAFE_SNPRINTF;

	return((int)(size - n));
}

#undef BEFORE_COLON
#undef BC

struct x509_crt_verify_string {
	int code;
	const char *string;
};

static const struct x509_crt_verify_string x509_crt_verify_strings[] = {
	{ X509_BADCERT_EXPIRED, "The certificate validity has expired" },
	{ X509_BADCERT_REVOKED, "The certificate has been revoked (is on a CRL)" },
	{ X509_BADCERT_CN_MISMATCH, "The certificate Common Name (CN) does not match with the expected CN" },
	{ X509_BADCERT_NOT_TRUSTED, "The certificate is not correctly signed by the trusted CA" },
	{ X509_BADCRL_NOT_TRUSTED, "The CRL is not correctly signed by the trusted CA" },
	{ X509_BADCRL_EXPIRED, "The CRL is expired" },
	{ X509_BADCERT_MISSING, "Certificate was missing" },
	{ X509_BADCERT_SKIP_VERIFY, "Certificate verification was skipped" },
	{ X509_BADCERT_OTHER, "Other reason (can be used by verify callback)" },
	{ X509_BADCERT_FUTURE, "The certificate validity starts in the future" },
	{ X509_BADCRL_FUTURE, "The CRL is from the future" },
	{ X509_BADCERT_KEY_USAGE, "Usage does not match the keyUsage extension" },
	{ X509_BADCERT_EXT_KEY_USAGE, "Usage does not match the extendedKeyUsage extension" },
	{ X509_BADCERT_NS_CERT_TYPE, "Usage does not match the nsCertType extension" },
	{ X509_BADCERT_BAD_MD, "The certificate is signed with an unacceptable hash." },
	{ X509_BADCERT_BAD_PK, "The certificate is signed with an unacceptable PK alg (eg RSA vs ECDSA)." },
	{ X509_BADCERT_BAD_KEY, "The certificate is signed with an unacceptable key (eg bad curve, RSA too short)." },
	{ X509_BADCRL_BAD_MD, "The CRL is signed with an unacceptable hash." },
	{ X509_BADCRL_BAD_PK, "The CRL is signed with an unacceptable PK alg (eg RSA vs ECDSA)." },
	{ X509_BADCRL_BAD_KEY, "The CRL is signed with an unacceptable key (eg bad curve, RSA too short)." },
	{ 0, NULL }
};

int x509_crt_verify_info(char *buf, dword_t size, const char *prefix,
	dword_t flags)
{
	int ret;
	const struct x509_crt_verify_string *cur;
	char *p = buf;
	dword_t n = size;

	for (cur = x509_crt_verify_strings; cur->string != NULL; cur++)
	{
		if ((flags & cur->code) == 0)
			continue;

		ret = snprintf(p, n, "%s%s\n", prefix, cur->string);
		X509_SAFE_SNPRINTF;
		flags ^= cur->code;
	}

	if (flags != 0)
	{
		ret = snprintf(p, n, "%sUnknown reason "
			"(this should not happen)\n", prefix);
		X509_SAFE_SNPRINTF;
	}

	return((int)(size - n));
}

int x509_crt_check_key_usage(const x509_crt *crt,
	unsigned int usage)
{
	unsigned int usage_must, usage_may;
	unsigned int may_mask = X509_KU_ENCIPHER_ONLY
		| X509_KU_DECIPHER_ONLY;

	if ((crt->ext_types & X509_EXT_KEY_USAGE) == 0)
		return(0);

	usage_must = usage & ~may_mask;

	if (((crt->key_usage & ~may_mask) & usage_must) != usage_must)
	{
		set_last_error(_T("x509_crt_check_key_usage"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	usage_may = usage & may_mask;

	if (((crt->key_usage & may_mask) | usage_may) != usage_may)
	{
		set_last_error(_T("x509_crt_check_key_usage"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	return(0);
}

int x509_crt_check_extended_key_usage(const x509_crt *crt,
	const char *usage_oid,
	dword_t usage_len)
{
	const x509_sequence *cur;

	/* Extension is not mandatory, absent means no restriction */
	if ((crt->ext_types & X509_EXT_EXTENDED_KEY_USAGE) == 0)
		return(0);

	/*
	* Look for the requested usage (or wildcard ANY) in our list
	*/
	for (cur = &crt->ext_key_usage; cur != NULL; cur = cur->next)
	{
		const x509_buf *cur_oid = &cur->buf;

		if (cur_oid->len == usage_len &&
			xmem_comp(cur_oid->p, usage_oid, usage_len) == 0)
		{
			return(0);
		}

		if (OID_CMP(OID_ANY_EXTENDED_KEY_USAGE, cur_oid) == 0)
			return(0);
	}

	set_last_error(_T("x509_crt_check_extended_key_usage"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
	return(C_ERR);
}

/*
* Return 1 if the certificate is revoked, or 0 otherwise.
*/
int x509_crt_is_revoked(const x509_crt *crt, const x509_crl *crl)
{
	const x509_crl_entry *cur = &crl->entry;

	while (cur != NULL && cur->serial.len != 0)
	{
		if (crt->serial.len == cur->serial.len &&
			xmem_comp(crt->serial.p, cur->serial.p, crt->serial.len) == 0)
		{
			if (x509_time_is_past(&cur->revocation_date))
				return(1);
		}

		cur = cur->next;
	}

	return(0);
}

/*
* Check that the given certificate is not revoked according to the CRL.
* Skip validation if no CRL for the given CA is present.
*/
static int x509_crt_verifycrl(x509_crt *crt, x509_crt *ca,
	x509_crl *crl_list,
	const x509_crt_profile *profile)
{
	int flags = 0;
	byte_t hash[MD_MAX_SIZE];
	const md_info_t *md_info;

	if (ca == NULL)
		return(flags);

	while (crl_list != NULL)
	{
		if (crl_list->version == 0 ||
			x509_name_cmp(&crl_list->issuer, &ca->subject) != 0)
		{
			crl_list = crl_list->next;
			continue;
		}

		/*
		* Check if the CA is configured to sign CRLs
		*/
		if (x509_crt_check_key_usage(ca,
			X509_KU_CRL_SIGN) != 0)
		{
			flags |= X509_BADCRL_NOT_TRUSTED;
			break;
		}

		/*
		* Check if CRL is correctly signed by the trusted CA
		*/
		if (x509_profile_check_md_alg(profile, crl_list->sig_md) != 0)
			flags |= X509_BADCRL_BAD_MD;

		if (x509_profile_check_pk_alg(profile, crl_list->sig_pk) != 0)
			flags |= X509_BADCRL_BAD_PK;

		md_info = md_info_from_type(crl_list->sig_md);
		if (md(md_info, crl_list->tbs.p, crl_list->tbs.len, hash) != 0)
		{
			/* Note: this can't happen except after an internal error */
			flags |= X509_BADCRL_NOT_TRUSTED;
			break;
		}

		if (x509_profile_check_key(profile, ca->sig_pk, (void*)ca->pk_ctx) != 0)
			flags |= X509_BADCERT_BAD_KEY;

		if (pk_verify_ext(crl_list->sig_pk, (void*)&ca->pk_ctx, 
			crl_list->sig_opt_mgf1_md, crl_list->sig_opt_sale_len,
			crl_list->sig_md, hash, md_info->size,
			crl_list->sig.p, crl_list->sig.len) != 0)
		{
			flags |= X509_BADCRL_NOT_TRUSTED;
			break;
		}

		/*
		* Check for validity of CRL (Do not drop out)
		*/
		if (x509_time_is_past(&crl_list->next_update))
			flags |= X509_BADCRL_EXPIRED;

		if (x509_time_is_future(&crl_list->this_update))
			flags |= X509_BADCRL_FUTURE;

		/*
		* Check if certificate is revoked
		*/
		if (x509_crt_is_revoked(crt, crl_list))
		{
			flags |= X509_BADCERT_REVOKED;
			break;
		}

		crl_list = crl_list->next;
	}

	return(flags);
}

/*
* Check the signature of a certificate by its parent
*/
static int x509_crt_check_signature(const x509_crt *child,
	x509_crt *parent,
	x509_crt_restart_ctx *rs_ctx)
{
	const md_info_t *md_info;
	byte_t hash[MD_MAX_SIZE];

	md_info = md_info_from_type(child->sig_md);
	if (md(md_info, child->tbs.p, child->tbs.len, hash) != 0)
	{
		/* Note: this can't happen except after an internal error */
		return(-1);
	}

	return(pk_verify_ext(child->sig_pk, (void*)&parent->pk_ctx, 
		child->sig_opt_mgf1_md, child->sig_opt_salt_len,
		child->sig_md, hash, md_info->size,
		child->sig.p, child->sig.len));
}

/*
* Check if 'parent' is a suitable parent (signing CA) for 'child'.
* Return 0 if yes, -1 if not.
*
* top means parent is a locally-trusted certificate
*/
static int x509_crt_check_parent(const x509_crt *child,
	const x509_crt *parent,
	int top)
{
	int need_ca_bit;

	/* Parent must be the issuer */
	if (x509_name_cmp(&child->issuer, &parent->subject) != 0)
		return(-1);

	/* Parent must have the basicConstraints CA bit set as a general rule */
	need_ca_bit = 1;

	/* Exception: v1/v2 certificates that are locally trusted. */
	if (top && parent->version < 3)
		need_ca_bit = 0;

	if (need_ca_bit && !parent->ca_istrue)
		return(-1);

	if (need_ca_bit &&
		x509_crt_check_key_usage(parent, X509_KU_KEY_CERT_SIGN) != 0)
	{
		return(-1);
	}

	return(0);
}

/*
* Find a suitable parent for child in candidates, or return NULL.
*
* Here suitable is defined as:
*  1. subject name matches child's issuer
*  2. if necessary, the CA bit is set and key usage allows signing certs
*  3. for trusted roots, the signature is correct
*     (for intermediates, the signature is checked and the result reported)
*  4. pathlen constraints are satisfied
*
* If there's a suitable candidate which is also time-valid, return the first
* such. Otherwise, return the first suitable candidate (or NULL if there is
* none).
*
* The rationale for this rule is that someone could have a list of trusted
* roots with two versions on the same root with different validity periods.
* (At least one user reported having such a list and wanted it to just work.)
* The reason we don't just require time-validity is that generally there is
* only one version, and if it's expired we want the flags to state that
* rather than NOT_TRUSTED, as would be the case if we required it here.
*
* The rationale for rule 3 (signature for trusted roots) is that users might
* have two versions of the same CA with different keys in their list, and the
* way we select the correct one is by checking the signature (as we don't
* rely on key identifier extensions). (This is one way users might choose to
* handle key rollover, another relies on self-issued certs, see [SIRO].)
*
* Arguments:
*  - [in] child: certificate for which we're looking for a parent
*  - [in] candidates: chained list of potential parents
*  - [out] r_parent: parent found (or NULL)
*  - [out] r_signature_is_good: 1 if child signature by parent is valid, or 0
*  - [in] top: 1 if candidates consists of trusted roots, ie we're at the top
*         of the chain, 0 otherwise
*  - [in] path_cnt: number of intermediates seen so far
*  - [in] self_cnt: number of self-signed intermediates seen so far
*         (will never be greater than path_cnt)
*  - [in-out] rs_ctx: context for restarting operations
*
* Return value:
*  - 0 on success
*  - ERR_ECP_IN_PROGRESS otherwise
*/
static int x509_crt_find_parent_in(
	x509_crt *child,
	x509_crt *candidates,
	x509_crt **r_parent,
	int *r_signature_is_good,
	int top,
	unsigned path_cnt,
	unsigned self_cnt,
	x509_crt_restart_ctx *rs_ctx)
{
	int ret;
	x509_crt *parent, *fallback_parent;
	int signature_is_good, fallback_signature_is_good;

	fallback_parent = NULL;
	fallback_signature_is_good = 0;

	for (parent = candidates; parent != NULL; parent = parent->next)
	{
		/* basic parenting skills (name, CA bit, key usage) */
		if (x509_crt_check_parent(child, parent, top) != 0)
			continue;

		/* +1 because stored max_pathlen is 1 higher that the actual value */
		if (parent->max_pathlen > 0 &&
			(dword_t)parent->max_pathlen < 1 + path_cnt - self_cnt)
		{
			continue;
		}

		ret = x509_crt_check_signature(child, parent, rs_ctx);

		signature_is_good = ret == 0;
		if (top && !signature_is_good)
			continue;

		/* optional time check */
		if (x509_time_is_past(&parent->valid_to) ||
			x509_time_is_future(&parent->valid_from))
		{
			if (fallback_parent == NULL)
			{
				fallback_parent = parent;
				fallback_signature_is_good = signature_is_good;
			}

			continue;
		}

		*r_parent = parent;
		*r_signature_is_good = signature_is_good;

		break;
	}

	if (parent == NULL)
	{
		*r_parent = fallback_parent;
		*r_signature_is_good = fallback_signature_is_good;
	}

	return(0);
}

/*
* Find a parent in trusted CAs or the provided chain, or return NULL.
*
* Searches in trusted CAs first, and return the first suitable parent found
* (see find_parent_in() for definition of suitable).
*
* Arguments:
*  - [in] child: certificate for which we're looking for a parent, followed
*         by a chain of possible intermediates
*  - [in] trust_ca: list of locally trusted certificates
*  - [out] parent: parent found (or NULL)
*  - [out] parent_is_trusted: 1 if returned `parent` is trusted, or 0
*  - [out] signature_is_good: 1 if child signature by parent is valid, or 0
*  - [in] path_cnt: number of links in the chain so far (EE -> ... -> child)
*  - [in] self_cnt: number of self-signed certs in the chain so far
*         (will always be no greater than path_cnt)
*  - [in-out] rs_ctx: context for restarting operations
*
* Return value:
*  - 0 on success
*  - ERR_ECP_IN_PROGRESS otherwise
*/
static int x509_crt_find_parent(
	x509_crt *child,
	x509_crt *trust_ca,
	x509_crt **parent,
	int *parent_is_trusted,
	int *signature_is_good,
	unsigned path_cnt,
	unsigned self_cnt,
	x509_crt_restart_ctx *rs_ctx)
{
	int ret;
	x509_crt *search_list;

	*parent_is_trusted = 1;

	while (1) {
		search_list = *parent_is_trusted ? trust_ca : child->next;

		ret = x509_crt_find_parent_in(child, search_list,
			parent, signature_is_good,
			*parent_is_trusted,
			path_cnt, self_cnt, rs_ctx);

		/* stop here if found or already in second iteration */
		if (*parent != NULL || *parent_is_trusted == 0)
			break;

		/* prepare second iteration */
		*parent_is_trusted = 0;
	}

	/* extra precaution against mistakes in the caller */
	if (*parent == NULL)
	{
		*parent_is_trusted = 0;
		*signature_is_good = 0;
	}

	return(0);
}

/*
* Check if an end-entity certificate is locally trusted
*
* Currently we require such certificates to be self-signed (actually only
* check for self-issued as self-signatures are not checked)
*/
static int x509_crt_check_ee_locally_trusted(
	x509_crt *crt,
	x509_crt *trust_ca)
{
	x509_crt *cur;

	/* must be self-issued */
	if (x509_name_cmp(&crt->issuer, &crt->subject) != 0)
		return(-1);

	/* look for an exact match with trusted cert */
	for (cur = trust_ca; cur != NULL; cur = cur->next)
	{
		if (crt->raw.len == cur->raw.len &&
			xmem_comp(crt->raw.p, cur->raw.p, crt->raw.len) == 0)
		{
			return(0);
		}
	}

	/* too bad */
	return(-1);
}

/*
* Build and verify a certificate chain
*
* Given a peer-provided list of certificates EE, C1, ..., Cn and
* a list of trusted certs R1, ... Rp, try to build and verify a chain
*      EE, Ci1, ... Ciq [, Rj]
* such that every cert in the chain is a child of the next one,
* jumping to a trusted root as early as possible.
*
* Verify that chain and return it with flags for all issues found.
*
* Special cases:
* - EE == Rj -> return a one-element list containing it
* - EE, Ci1, ..., Ciq cannot be continued with a trusted root
*   -> return that chain with NOT_TRUSTED set on Ciq
*
* Tests for (aspects of) this function should include at least:
* - trusted EE
* - EE -> trusted root
* - EE -> intermediate CA -> trusted root
* - if relevant: EE untrusted
* - if relevant: EE -> intermediate, untrusted
* with the aspect under test checked at each relevant level (EE, int, root).
* For some aspects longer chains are required, but usually length 2 is
* enough (but length 1 is not in general).
*
* Arguments:
*  - [in] crt: the cert list EE, C1, ..., Cn
*  - [in] trust_ca: the trusted list R1, ..., Rp
*  - [in] ca_crl, profile: as in verify_with_profile()
*  - [out] ver_chain: the built and verified chain
*      Only valid when return value is 0, may contain garbage otherwise!
*      Restart note: need not be the same when calling again to resume.
*  - [in-out] rs_ctx: context for restarting operations
*
* Return value:
*  - non-zero if the chain could not be fully built and examined
*  - 0 is the chain was successfully built and examined,
*      even if it was found to be invalid
*/
static int x509_crt_verify_chain_ret(
	x509_crt *crt,
	x509_crt *trust_ca,
	x509_crl *ca_crl,
	const x509_crt_profile *profile,
	x509_crt_verify_chain *ver_chain,
	x509_crt_restart_ctx *rs_ctx)
{
	/* Don't initialize any of those variables here, so that the compiler can
	* catch potential issues with jumping ahead when restarting */
	int ret;
	dword_t *flags;
	x509_crt_verify_chain_item *cur;
	x509_crt *child;
	x509_crt *parent;
	int parent_is_trusted;
	int child_is_trusted;
	int signature_is_good;
	unsigned self_cnt;

	child = crt;
	self_cnt = 0;
	parent_is_trusted = 0;
	child_is_trusted = 0;

	while (1) {
		/* Add certificate to the verification chain */
		cur = &ver_chain->items[ver_chain->len];
		cur->crt = child;
		cur->flags = 0;
		ver_chain->len++;
		flags = &cur->flags;

		/* Check time-validity (all certificates) */
		if (x509_time_is_past(&child->valid_to))
			*flags |= X509_BADCERT_EXPIRED;

		if (x509_time_is_future(&child->valid_from))
			*flags |= X509_BADCERT_FUTURE;

		/* Stop here for trusted roots (but not for trusted EE certs) */
		if (child_is_trusted)
			return(0);

		/* Check signature algorithm: MD & PK algs */
		if (x509_profile_check_md_alg(profile, child->sig_md) != 0)
			*flags |= X509_BADCERT_BAD_MD;

		if (x509_profile_check_pk_alg(profile, child->sig_pk) != 0)
			*flags |= X509_BADCERT_BAD_PK;

		/* Special case: EE certs that are locally trusted */
		if (ver_chain->len == 1 &&
			x509_crt_check_ee_locally_trusted(child, trust_ca) == 0)
		{
			return(0);
		}

		/* Look for a parent in trusted CAs or up the chain */
		ret = x509_crt_find_parent(child, trust_ca, &parent,
			&parent_is_trusted, &signature_is_good,
			ver_chain->len - 1, self_cnt, rs_ctx);

		/* No parent? We're done here */
		if (parent == NULL)
		{
			*flags |= X509_BADCERT_NOT_TRUSTED;
			return(0);
		}

		/* Count intermediate self-issued (not necessarily self-signed) certs.
		* These can occur with some strategies for key rollover, see [SIRO],
		* and should be excluded from max_pathlen checks. */
		if (ver_chain->len != 1 &&
			x509_name_cmp(&child->issuer, &child->subject) == 0)
		{
			self_cnt++;
		}

		/* path_cnt is 0 for the first intermediate CA,
		* and if parent is trusted it's not an intermediate CA */
		if (!parent_is_trusted &&
			ver_chain->len > X509_MAX_INTERMEDIATE_CA)
		{
			/* return immediately to avoid overflow the chain array */
			set_last_error(_T("x509_crt_verify_chain_ret"), _T("ERR_X509_FATAL_ERROR"), -1);
			return(C_ERR);
		}

		/* signature was checked while searching parent */
		if (!signature_is_good)
			*flags |= X509_BADCERT_NOT_TRUSTED;

		/* check size of signing key */
		if (x509_profile_check_key(profile, parent->pk_alg, (void*)&parent->pk_ctx) != 0)
			*flags |= X509_BADCERT_BAD_KEY;

		/* Check trusted CA's CRL for the given crt */
		*flags |= x509_crt_verifycrl(child, parent, ca_crl, profile);

		/* prepare for next iteration */
		child = parent;
		parent = NULL;
		child_is_trusted = parent_is_trusted;
		signature_is_good = 0;
	}
}

/*
* Check for CN match
*/
static int x509_crt_check_cn(const x509_buf *name,
	const char *cn, dword_t cn_len)
{
	/* try exact match */
	if (name->len == cn_len &&
		x509_memcasecmp(cn, name->p, cn_len) == 0)
	{
		return(0);
	}

	/* try wildcard match */
	if (x509_check_wildcard(cn, name) == 0)
	{
		return(0);
	}

	return(-1);
}

/*
* Verify the requested CN - only call this if cn is not NULL!
*/
static void x509_crt_verify_name(const x509_crt *crt,
	const char *cn,
	dword_t *flags)
{
	const x509_name *name;
	const x509_sequence *cur;
	dword_t cn_len = strlen(cn);

	if (crt->ext_types & X509_EXT_SUBJECT_ALT_NAME)
	{
		for (cur = &crt->subject_alt_names; cur != NULL; cur = cur->next)
		{
			if (x509_crt_check_cn(&cur->buf, cn, cn_len) == 0)
				break;
		}

		if (cur == NULL)
			*flags |= X509_BADCERT_CN_MISMATCH;
	}
	else
	{
		for (name = &crt->subject; name != NULL; name = name->next)
		{
			if (OID_CMP(OID_AT_CN, &name->oid) == 0 &&
				x509_crt_check_cn(&name->val, cn, cn_len) == 0)
			{
				break;
			}
		}

		if (name == NULL)
			*flags |= X509_BADCERT_CN_MISMATCH;
	}
}

/*
* Merge the flags for all certs in the chain, after calling callback
*/
static int x509_crt_merge_flags_with_cb(
	dword_t *flags,
	const x509_crt_verify_chain *ver_chain,
	int(*f_vrfy)(void *, x509_crt *, int, dword_t *),
	void *p_vrfy)
{
	int ret;
	unsigned i;
	dword_t cur_flags;
	const x509_crt_verify_chain_item *cur;

	for (i = ver_chain->len; i != 0; --i)
	{
		cur = &ver_chain->items[i - 1];
		cur_flags = cur->flags;

		if (NULL != f_vrfy)
			if ((ret = f_vrfy(p_vrfy, cur->crt, (int)i - 1, &cur_flags)) != 0)
				return(ret);

		*flags |= cur_flags;
	}

	return(0);
}

/*
* Verify the certificate validity (default profile, not restartable)
*/
int x509_crt_verify(x509_crt *crt,
	x509_crt *trust_ca,
	x509_crl *ca_crl,
	const char *cn, dword_t *flags,
	int(*f_vrfy)(void *, x509_crt *, int, dword_t *),
	void *p_vrfy)
{
	return(x509_crt_verify_restartable(crt, trust_ca, ca_crl,
		&x509_crt_profile_default, cn, flags,
		f_vrfy, p_vrfy, NULL));
}

/*
* Verify the certificate validity (user-chosen profile, not restartable)
*/
int x509_crt_verify_with_profile(x509_crt *crt,
	x509_crt *trust_ca,
	x509_crl *ca_crl,
	const x509_crt_profile *profile,
	const char *cn, dword_t *flags,
	int(*f_vrfy)(void *, x509_crt *, int, dword_t *),
	void *p_vrfy)
{
	return(x509_crt_verify_restartable(crt, trust_ca, ca_crl,
		profile, cn, flags, f_vrfy, p_vrfy, NULL));
}

/*
* Verify the certificate validity, with profile, restartable version
*
* This function:
*  - checks the requested CN (if any)
*  - checks the type and size of the EE cert's key,
*    as that isn't done as part of chain building/verification currently
*  - builds and verifies the chain
*  - then calls the callback and merges the flags
*/
int x509_crt_verify_restartable(x509_crt *crt,
	x509_crt *trust_ca,
	x509_crl *ca_crl,
	const x509_crt_profile *profile,
	const char *cn, dword_t *flags,
	int(*f_vrfy)(void *, x509_crt *, int, dword_t *),
	void *p_vrfy,
	x509_crt_restart_ctx *rs_ctx)
{
	int ret;
	x509_crt_verify_chain ver_chain;
	dword_t ee_flags;

	*flags = 0;
	ee_flags = 0;
	x509_crt_verify_chain_reset(&ver_chain);

	if (profile == NULL)
	{
		set_last_error(_T("x509_crt_verify_restartable"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		ret = C_ERR;
		goto exit;
	}

	/* check name if requested */
	if (cn != NULL)
		x509_crt_verify_name(crt, cn, &ee_flags);

	/* Check the type and size of the key */

	if (x509_profile_check_pk_alg(profile, crt->pk_alg) != 0)
		ee_flags |= X509_BADCERT_BAD_PK;

	if (x509_profile_check_key(profile, crt->pk_alg, (void*)&crt->pk_ctx) != 0)
		ee_flags |= X509_BADCERT_BAD_KEY;

	/* Check the chain */
	ret = x509_crt_verify_chain_ret(crt, trust_ca, ca_crl, profile,
		&ver_chain, rs_ctx);

	if (ret != 0)
		goto exit;

	/* Merge end-entity flags */
	ver_chain.items[0].flags |= ee_flags;

	/* Build final flags, calling callback on the way if any */
	ret = x509_crt_merge_flags_with_cb(flags, &ver_chain, f_vrfy, p_vrfy);

exit:

	if (ret != 0)
	{
		*flags = (dword_t)-1;
		return(ret);
	}

	if (*flags != 0)
		return(C_ERR);

	return(0);
}

/*
* Initialize a certificate chain
*/
void x509_crt_init(x509_crt *crt)
{
	xmem_zero(crt, sizeof(x509_crt));
}

/*
* Unallocate all certificate data
*/
void x509_crt_free(x509_crt *crt)
{
	x509_crt *cert_cur = crt;
	x509_crt *cert_prv;
	x509_name *name_cur;
	x509_name *name_prv;
	x509_sequence *seq_cur;
	x509_sequence *seq_prv;

	if (crt == NULL)
		return;

	do
	{
		if (crt->pk_alg == PK_RSA)
		{
			rsa_free((rsa_context*)crt->pk_ctx);
			xmem_free(crt->pk_ctx);
			crt->pk_ctx = NULL;
		}
		else if (crt->pk_alg == PK_ECKEY_DH || crt->pk_alg == PK_ECKEY)
		{
			ecp_keypair_free((ecp_keypair*)crt->pk_ctx);
			xmem_free(crt->pk_ctx);
			crt->pk_ctx = NULL;
		}

		name_cur = cert_cur->issuer.next;
		while (name_cur != NULL)
		{
			name_prv = name_cur;
			name_cur = name_cur->next;
			xmem_zero(name_prv, sizeof(x509_name));
			xmem_free(name_prv);
		}

		name_cur = cert_cur->subject.next;
		while (name_cur != NULL)
		{
			name_prv = name_cur;
			name_cur = name_cur->next;
			xmem_zero(name_prv, sizeof(x509_name));
			xmem_free(name_prv);
		}

		seq_cur = cert_cur->ext_key_usage.next;
		while (seq_cur != NULL)
		{
			seq_prv = seq_cur;
			seq_cur = seq_cur->next;
			xmem_zero(seq_prv, sizeof(x509_sequence));
			xmem_free(seq_prv);
		}

		seq_cur = cert_cur->subject_alt_names.next;
		while (seq_cur != NULL)
		{
			seq_prv = seq_cur;
			seq_cur = seq_cur->next;
			xmem_zero(seq_prv, sizeof(x509_sequence));
			xmem_free(seq_prv);
		}

		if (cert_cur->raw.p != NULL)
		{
			xmem_zero(cert_cur->raw.p, cert_cur->raw.len);
			xmem_free(cert_cur->raw.p);
		}

		cert_cur = cert_cur->next;
	} while (cert_cur != NULL);

	cert_cur = crt;
	do
	{
		cert_prv = cert_cur;
		cert_cur = cert_cur->next;

		xmem_zero(cert_prv, sizeof(x509_crt));
		if (cert_prv != crt)
			xmem_free(cert_prv);
	} while (cert_cur != NULL);
}

/*
* For the currently used signature algorithms the buffer to store any signature
* must be at least of size MAX(ECDSA_MAX_LEN, MPI_MAX_SIZE)
*/
#if ECDSA_MAX_LEN > MPI_MAX_SIZE
#define SIGNATURE_MAX_SIZE ECDSA_MAX_LEN
#else
#define SIGNATURE_MAX_SIZE MPI_MAX_SIZE
#endif

void x509write_crt_init(x509write_cert *ctx)
{
	xmem_zero(ctx, sizeof(x509write_cert));

	mpi_init(&ctx->serial);
	ctx->version = X509_CRT_VERSION_3;
}

void x509write_crt_free(x509write_cert *ctx)
{
	mpi_free(&ctx->serial);

	asn1_free_named_data_list(&ctx->subject);
	asn1_free_named_data_list(&ctx->issuer);
	asn1_free_named_data_list(&ctx->extensions);

	xmem_zero(ctx, sizeof(x509write_cert));
}

void x509write_crt_set_version(x509write_cert *ctx, int version)
{
	ctx->version = version;
}

void x509write_crt_set_md_alg(x509write_cert *ctx, md_type_t md_alg)
{
	ctx->md_alg = md_alg;
}

void x509write_crt_set_subject_key(x509write_cert *ctx, pk_type_t pktype, void *pk_ctx)
{
	ctx->subject_pk = pktype;
	ctx->subject_key = pk_ctx;
}

void x509write_crt_set_issuer_key(x509write_cert *ctx, pk_type_t pktype, void *pk_ctx)
{
	ctx->issuer_pk = pktype;
	ctx->issuer_key = pk_ctx;
}

int x509write_crt_set_subject_name(x509write_cert *ctx,
	const char *subject_name)
{
	return x509_string_to_names(&ctx->subject, subject_name);
}

int x509write_crt_set_issuer_name(x509write_cert *ctx,
	const char *issuer_name)
{
	return x509_string_to_names(&ctx->issuer, issuer_name);
}

int x509write_crt_set_serial(x509write_cert *ctx, const mpi *serial)
{
	int ret;

	if ((ret = mpi_copy(&ctx->serial, serial)) != 0)
		return(ret);

	return(0);
}

int x509write_crt_set_validity(x509write_cert *ctx, const char *not_before,
	const char *not_after)
{
	if (strlen(not_before) != X509_RFC5280_UTC_TIME_LEN - 1 ||
		strlen(not_after) != X509_RFC5280_UTC_TIME_LEN - 1)
	{
		set_last_error(_T("x509write_crt_set_validity"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}
	strncpy(ctx->not_before, not_before, X509_RFC5280_UTC_TIME_LEN);
	strncpy(ctx->not_after, not_after, X509_RFC5280_UTC_TIME_LEN);
	ctx->not_before[X509_RFC5280_UTC_TIME_LEN - 1] = 'Z';
	ctx->not_after[X509_RFC5280_UTC_TIME_LEN - 1] = 'Z';

	return(0);
}

int x509write_crt_set_extension(x509write_cert *ctx,
	const char *oid, dword_t oid_len,
	int critical,
	const byte_t *val, dword_t val_len)
{
	return x509_set_extension(&ctx->extensions, oid, oid_len,
		critical, val, val_len);
}

int x509write_crt_set_basic_constraints(x509write_cert *ctx,
	int is_ca, int max_pathlen)
{
	int ret;
	byte_t buf[9];
	byte_t *c = buf + sizeof(buf);
	dword_t len = 0;

	xmem_zero(buf, sizeof(buf));

	if (is_ca && max_pathlen > 127)
	{
		set_last_error(_T("x509write_crt_set_basic_constraints"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	if (is_ca)
	{
		if (max_pathlen >= 0)
		{
			ASN1_CHK_ADD(len, asn1_write_int(&c, buf, max_pathlen));
		}
		ASN1_CHK_ADD(len, asn1_write_bool(&c, buf, 1));
	}

	ASN1_CHK_ADD(len, asn1_write_len(&c, buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, buf, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	return x509write_crt_set_extension(ctx, OID_BASIC_CONSTRAINTS,
		OID_SIZE(OID_BASIC_CONSTRAINTS),
		0, buf + sizeof(buf) - len, len);
}

int x509write_crt_set_subject_key_identifier(x509write_cert *ctx)
{
	int ret;
	byte_t buf[MPI_MAX_SIZE * 2 + 20]; /* tag, length + 2xMPI */
	byte_t *c = buf + sizeof(buf);
	dword_t len = 0;

	xmem_zero(buf, sizeof(buf));
	ASN1_CHK_ADD(len, x509_write_pubkey(&c, buf, ctx->subject_pk, ctx->subject_key));

	ret = sha1(buf + sizeof(buf) - len, len,
		buf + sizeof(buf) - 20);
	if (ret != 0)
		return(ret);
	c = buf + sizeof(buf) - 20;
	len = 20;

	ASN1_CHK_ADD(len, asn1_write_len(&c, buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, buf, ASN1_OCTET_STRING));

	return x509write_crt_set_extension(ctx, OID_SUBJECT_KEY_IDENTIFIER,
		OID_SIZE(OID_SUBJECT_KEY_IDENTIFIER),
		0, buf + sizeof(buf) - len, len);
}

int x509write_crt_set_authority_key_identifier(x509write_cert *ctx)
{
	int ret;
	byte_t buf[MPI_MAX_SIZE * 2 + 20]; /* tag, length + 2xMPI */
	byte_t *c = buf + sizeof(buf);
	dword_t len = 0;

	xmem_zero(buf, sizeof(buf));
	ASN1_CHK_ADD(len, x509_write_pubkey(&c, buf, ctx->issuer_pk, ctx->issuer_key));

	ret = sha1(buf + sizeof(buf) - len, len,
		buf + sizeof(buf) - 20);
	if (ret != 0)
		return(ret);
	c = buf + sizeof(buf) - 20;
	len = 20;

	ASN1_CHK_ADD(len, asn1_write_len(&c, buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, buf, ASN1_CONTEXT_SPECIFIC | 0));

	ASN1_CHK_ADD(len, asn1_write_len(&c, buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, buf, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	return x509write_crt_set_extension(ctx, OID_AUTHORITY_KEY_IDENTIFIER,
		OID_SIZE(OID_AUTHORITY_KEY_IDENTIFIER),
		0, buf + sizeof(buf) - len, len);
}

static dword_t crt_get_unused_bits_for_named_bitstring(byte_t bitstring,
	dword_t bit_offset)
{
	dword_t unused_bits;

	/* Count the unused bits removing trailing 0s */
	for (unused_bits = bit_offset; unused_bits < 8; unused_bits++)
		if (((bitstring >> unused_bits) & 0x1) != 0)
			break;

	return(unused_bits);
}

int x509write_crt_set_key_usage(x509write_cert *ctx,
	unsigned int key_usage)
{
	byte_t buf[4], ku;
	byte_t *c;
	int ret;
	dword_t unused_bits;
	const unsigned int allowed_bits = X509_KU_DIGITAL_SIGNATURE |
		X509_KU_NON_REPUDIATION |
		X509_KU_KEY_ENCIPHERMENT |
		X509_KU_DATA_ENCIPHERMENT |
		X509_KU_KEY_AGREEMENT |
		X509_KU_KEY_CERT_SIGN |
		X509_KU_CRL_SIGN;

	/* Check that nothing other than the allowed flags is set */
	if ((key_usage & ~allowed_bits) != 0)
	{
		set_last_error(_T("x509write_crt_set_key_usage"), _T("ERR_X509_FEATURE_UNAVAILABLE"), -1);
		return(C_ERR);
	}

	c = buf + 4;
	ku = (byte_t)key_usage;
	unused_bits = crt_get_unused_bits_for_named_bitstring(ku, 1);
	ret = asn1_write_bitstring(&c, buf, &ku, 8 - unused_bits);

	if (ret < 0)
		return(ret);
	else if (ret < 3 || ret > 4)
	{
		set_last_error(_T("x509write_crt_set_key_usage"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	ret = x509write_crt_set_extension(ctx, OID_KEY_USAGE,
		OID_SIZE(OID_KEY_USAGE),
		1, c, (dword_t)ret);
	if (ret != 0)
		return(ret);

	return(0);
}

int x509write_crt_set_ns_cert_type(x509write_cert *ctx,
	byte_t ns_cert_type)
{
	byte_t buf[4];
	byte_t *c;
	dword_t unused_bits;
	int ret;

	c = buf + 4;

	unused_bits = crt_get_unused_bits_for_named_bitstring(ns_cert_type, 0);
	ret = asn1_write_bitstring(&c,
		buf,
		&ns_cert_type,
		8 - unused_bits);
	if (ret < 3 || ret > 4)
		return(ret);

	ret = x509write_crt_set_extension(ctx, OID_NS_CERT_TYPE,
		OID_SIZE(OID_NS_CERT_TYPE),
		0, c, (dword_t)ret);
	if (ret != 0)
		return(ret);

	return(0);
}

static int x509_write_time(byte_t **p, byte_t *start,
	const char *t, dword_t size)
{
	int ret;
	dword_t len = 0;

	/*
	* write ASN1_UTC_TIME if year < 2050 (2 bytes shorter)
	*/
	if (t[0] == '2' && t[1] == '0' && t[2] < '5')
	{
		ASN1_CHK_ADD(len, asn1_write_raw_buffer(p, start,
			(const byte_t *)t + 2,
			size - 2));
		ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
		ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_UTC_TIME));
	}
	else
	{
		ASN1_CHK_ADD(len, asn1_write_raw_buffer(p, start,
			(const byte_t *)t,
			size));
		ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
		ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_GENERALIZED_TIME));
	}

	return((int)len);
}

int x509write_crt_der(x509write_cert *ctx, byte_t *buf, dword_t size,
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
	dword_t sub_len = 0, pub_len = 0, sig_and_oid_len = 0, sig_len;
	dword_t len = 0;
	pk_type_t pk_alg;

	/*
	* Prepare data to be signed in tmp_buf
	*/
	c = tmp_buf + sizeof(tmp_buf);

	/* Signature algorithm needed in TBS, and later for actual signature */

	/* There's no direct way of extracting a signature algorithm
	* (represented as an element of pk_type) from a PK instance. */
	if (ctx->issuer_pk == PK_RSA)
		pk_alg = PK_RSA;
	else if (ctx->issuer_pk == PK_ECDSA)
		pk_alg = PK_ECDSA;
	else
	{
		set_last_error(_T("x509write_crt_der"), _T("ERR_X509_INVALID_ALG"), -1);
		return(C_ERR);
	}

	if ((ret = oid_get_oid_by_sig_alg(pk_alg, ctx->md_alg, &sig_oid, &sig_oid_len)) != 0)
	{
		return(ret);
	}

	/*
	*  Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
	*/

	/* Only for v3 */
	if (ctx->version == X509_CRT_VERSION_3)
	{
		ASN1_CHK_ADD(len, x509_write_extensions(&c, tmp_buf, ctx->extensions));
		ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, len));
		ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONSTRUCTED |
			ASN1_SEQUENCE));
		ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, len));
		ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONTEXT_SPECIFIC |
			ASN1_CONSTRUCTED | 3));
	}

	/*
	*  SubjectPublicKeyInfo
	*/
	ASN1_CHK_ADD(pub_len, x509_write_pubkey_der(ctx->subject_pk, ctx->subject_key, tmp_buf, c - tmp_buf));
	c -= pub_len;
	len += pub_len;

	/*
	*  Subject  ::=  Name
	*/
	ASN1_CHK_ADD(len, x509_write_names(&c, tmp_buf, ctx->subject));

	/*
	*  Validity ::= SEQUENCE {
	*       notBefore      Time,
	*       notAfter       Time }
	*/
	sub_len = 0;

	ASN1_CHK_ADD(sub_len, x509_write_time(&c, tmp_buf, ctx->not_after,
		X509_RFC5280_UTC_TIME_LEN));

	ASN1_CHK_ADD(sub_len, x509_write_time(&c, tmp_buf, ctx->not_before,
		X509_RFC5280_UTC_TIME_LEN));

	len += sub_len;
	ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, sub_len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	/*
	*  Issuer  ::=  Name
	*/
	ASN1_CHK_ADD(len, x509_write_names(&c, tmp_buf, ctx->issuer));

	/*
	*  Signature   ::=  AlgorithmIdentifier
	*/
	ASN1_CHK_ADD(len, asn1_write_algorithm_identifier(&c, tmp_buf,
		sig_oid, strlen(sig_oid), 0));

	/*
	*  Serial   ::=  INTEGER
	*/
	ASN1_CHK_ADD(len, asn1_write_mpi(&c, tmp_buf, &ctx->serial));

	/*
	*  Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
	*/

	/* Can be omitted for v1 */
	if (ctx->version != X509_CRT_VERSION_1)
	{
		sub_len = 0;
		ASN1_CHK_ADD(sub_len, asn1_write_int(&c, tmp_buf, ctx->version));
		len += sub_len;
		ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, sub_len));
		ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONTEXT_SPECIFIC |
			ASN1_CONSTRUCTED | 0));
	}

	ASN1_CHK_ADD(len, asn1_write_len(&c, tmp_buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, tmp_buf, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	/*
	* Make signature
	*/
	if ((ret = md(md_info_from_type(ctx->md_alg), c,
		len, hash)) != 0)
	{
		return(ret);
	}

	if ((ret = pk_sign(ctx->issuer_pk, (void*)ctx->issuer_key, ctx->md_alg, hash, 0, sig, &sig_len, f_rng, p_rng)) != 0)
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
		set_last_error(_T("x509write_crt_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
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

#define PEM_BEGIN_CRT           "-----BEGIN CERTIFICATE-----\n"
#define PEM_END_CRT             "-----END CERTIFICATE-----\n"

int x509write_crt_pem(x509write_cert *crt, byte_t *buf, dword_t size,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	byte_t output_buf[4096];
	dword_t olen = 0;

	if ((ret = x509write_crt_der(crt, output_buf, sizeof(output_buf),
		f_rng, p_rng)) < 0)
	{
		return(ret);
	}

	if ((ret = pem_write_buffer(PEM_BEGIN_CRT, PEM_END_CRT,
		output_buf + sizeof(output_buf) - ret,
		ret, buf, size, &olen)) != 0)
	{
		return(ret);
	}

	return(0);
}

