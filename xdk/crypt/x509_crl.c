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


#include "x509_crl.h"
#include "pem.h"
#include "oid.h"
#include "ecdsa.h"

#include "../xdkimp.h"

/*
*  Version  ::=  INTEGER  {  v1(0), v2(1)  }
*/
static int x509_crl_get_version(byte_t **p,
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

		set_last_error(_T("x509_crl_get_version"), _T("ERR_X509_INVALID_VERSION"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* X.509 CRL v2 extensions
*
* We currently don't parse any extension's content, but we do check that the
* list of extensions is well-formed and abort on critical extensions (that
* are unsupported as we don't support any extension so far)
*/
static int x509_get_crl_ext(byte_t **p,
	const byte_t *end,
	x509_buf *ext)
{
	int ret;

	if (*p == end)
		return(0);

	/*
	* crlExtensions           [0]  EXPLICIT Extensions OPTIONAL
	*                              -- if present, version MUST be v2
	*/
	if ((ret = x509_get_ext(p, end, ext, 0)) != 0)
		return(ret);

	end = ext->p + ext->len;

	while (*p < end)
	{
		/*
		* Extension  ::=  SEQUENCE  {
		*      extnID      OBJECT IDENTIFIER,
		*      critical    BOOLEAN DEFAULT FALSE,
		*      extnValue   OCTET STRING  }
		*/
		int is_critical = 0;
		const byte_t *end_ext_data;
		dword_t len;

		/* Get enclosing sequence tag */
		if ((ret = asn1_get_tag(p, end, &len,
			ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
		{
			set_last_error(_T("x509_get_crl_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		end_ext_data = *p + len;

		/* Get OID (currently ignored) */
		if ((ret = asn1_get_tag(p, end_ext_data, &len,
			ASN1_OID)) != 0)
		{
			set_last_error(_T("x509_get_crl_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		*p += len;

		/* Get optional critical */
		if ((ret = asn1_get_bool(p, end_ext_data,
			&is_critical)) != 0 &&
			(ret != ERR_ASN1_UNEXPECTED_TAG))
		{
			set_last_error(_T("x509_get_crl_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		/* Data should be octet string type */
		if ((ret = asn1_get_tag(p, end_ext_data, &len,
			ASN1_OCTET_STRING)) != 0)
		{
			set_last_error(_T("x509_get_crl_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		/* Ignore data so far and just check its length */
		*p += len;
		if (*p != end_ext_data)
		{
			set_last_error(_T("x509_get_crl_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		/* Abort on (unsupported) critical extensions */
		if (is_critical)
		{
			set_last_error(_T("x509_get_crl_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}
	}

	if (*p != end)
	{
		set_last_error(_T("x509_get_crl_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* X.509 CRL v2 entry extensions (no extensions parsed yet.)
*/
static int x509_get_crl_entry_ext(byte_t **p,
	const byte_t *end,
	x509_buf *ext)
{
	int ret;
	dword_t len = 0;

	/* OPTIONAL */
	if (end <= *p)
		return(0);

	ext->tag = **p;
	ext->p = *p;

	/*
	* Get CRL-entry extension sequence header
	* crlEntryExtensions      Extensions OPTIONAL  -- if present, MUST be v2
	*/
	if ((ret = asn1_get_tag(p, end, &ext->len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		if (ret == ERR_ASN1_UNEXPECTED_TAG)
		{
			ext->p = NULL;
			return(0);
		}

		set_last_error(_T("x509_get_crl_entry_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	end = *p + ext->len;

	if (end != *p + ext->len)
	{
		set_last_error(_T("x509_get_crl_entry_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	while (*p < end)
	{
		if ((ret = asn1_get_tag(p, end, &len,
			ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
		{
			set_last_error(_T("x509_get_crl_entry_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		*p += len;
	}

	if (*p != end)
	{
		set_last_error(_T("x509_get_crl_entry_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* X.509 CRL Entries
*/
static int x509_get_entries(byte_t **p,
	const byte_t *end,
	x509_crl_entry *entry)
{
	int ret;
	dword_t entry_len;
	x509_crl_entry *cur_entry = entry;

	if (*p == end)
		return(0);

	if ((ret = asn1_get_tag(p, end, &entry_len,
		ASN1_SEQUENCE | ASN1_CONSTRUCTED)) != 0)
	{
		if (ret == ERR_ASN1_UNEXPECTED_TAG)
			return(0);

		return(ret);
	}

	end = *p + entry_len;

	while (*p < end)
	{
		dword_t len2;
		const byte_t *end2;

		if ((ret = asn1_get_tag(p, end, &len2,
			ASN1_SEQUENCE | ASN1_CONSTRUCTED)) != 0)
		{
			return(ret);
		}

		cur_entry->raw.tag = **p;
		cur_entry->raw.p = *p;
		cur_entry->raw.len = len2;
		end2 = *p + len2;

		if ((ret = x509_get_serial(p, end2, &cur_entry->serial)) != 0)
			return(ret);

		if ((ret = x509_get_time(p, end2,
			&cur_entry->revocation_date)) != 0)
			return(ret);

		if ((ret = x509_get_crl_entry_ext(p, end2,
			&cur_entry->entry_ext)) != 0)
			return(ret);

		if (*p < end)
		{
			cur_entry->next = xmem_alloc(sizeof(x509_crl_entry));

			if (cur_entry->next == NULL)
			{
				set_last_error(_T("x509_get_entries"), _T("ERR_X509_ALLOC_FAILED"), -1);
				return(C_ERR);
			}

			cur_entry = cur_entry->next;
		}
	}

	return(0);
}

/*
* Parse one  CRLs in DER format and append it to the chained list
*/
int x509_crl_parse_der(x509_crl *chain,
	const byte_t *buf, dword_t buflen)
{
	int ret;
	dword_t len;
	byte_t *p = NULL, *end = NULL;
	x509_buf sig_params1, sig_params2, sig_oid2;
	x509_crl *crl = chain;

	/*
	* Check for valid input
	*/
	if (crl == NULL || buf == NULL)
	{
		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	xmem_zero(&sig_params1, sizeof(x509_buf));
	xmem_zero(&sig_params2, sizeof(x509_buf));
	xmem_zero(&sig_oid2, sizeof(x509_buf));

	/*
	* Add new CRL on the end of the chain if needed.
	*/
	while (crl->version != 0 && crl->next != NULL)
		crl = crl->next;

	if (crl->version != 0 && crl->next == NULL)
	{
		crl->next = xmem_alloc(sizeof(x509_crl));

		if (crl->next == NULL)
		{
			x509_crl_free(crl);

			set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_ALLOC_FAILED"), -1);
			return(C_ERR);
		}

		x509_crl_init(crl->next);
		crl = crl->next;
	}

	/*
	* Copy raw DER-encoded CRL
	*/
	if (buflen == 0)
	{
		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	p = xmem_alloc(buflen);
	if (p == NULL)
	{
		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_ALLOC_FAILED"), -1);
		return(C_ERR);
	}

	xmem_copy(p, buf, buflen);

	crl->raw.p = p;
	crl->raw.len = buflen;

	end = p + buflen;

	/*
	* CertificateList  ::=  SEQUENCE  {
	*      tbsCertList          TBSCertList,
	*      signatureAlgorithm   AlgorithmIdentifier,
	*      signatureValue       BIT STRING  }
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if (len != (dword_t)(end - p))
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	/*
	* TBSCertList  ::=  SEQUENCE  {
	*/
	crl->tbs.p = p;

	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	end = p + len;
	crl->tbs.len = end - crl->tbs.p;

	/*
	* Version  ::=  INTEGER  OPTIONAL {  v1(0), v2(1)  }
	*               -- if present, MUST be v2
	*
	* signature            AlgorithmIdentifier
	*/
	if ((ret = x509_crl_get_version(&p, end, &crl->version)) != 0 ||
		(ret = x509_get_alg(&p, end, &crl->sig_oid, &sig_params1)) != 0)
	{
		x509_crl_free(crl);
		return(ret);
	}

	if (crl->version < 0 || crl->version > 1)
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_UNKNOWN_VERSION"), -1);
		return(C_ERR);
	}

	crl->version++;

	if ((ret = x509_get_sig_alg(&crl->sig_oid, &sig_params1,
		&crl->sig_md, &crl->sig_pk,
		&crl->sig_opt_mgf1_md, &crl->sig_opt_sale_len)) != 0)
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_UNKNOWN_SIG_ALG"), -1);
		return(C_ERR);
	}

	/*
	* issuer               Name
	*/
	crl->issuer_raw.p = p;

	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((ret = x509_get_name(&p, p + len, &crl->issuer)) != 0)
	{
		x509_crl_free(crl);
		return(ret);
	}

	crl->issuer_raw.len = p - crl->issuer_raw.p;

	/*
	* thisUpdate          Time
	* nextUpdate          Time OPTIONAL
	*/
	if ((ret = x509_get_time(&p, end, &crl->this_update)) != 0)
	{
		x509_crl_free(crl);
		return(ret);
	}

	if ((ret = x509_get_time(&p, end, &crl->next_update)) != 0)
	{
		x509_crl_free(crl);
		return(ret);
	}

	/*
	* revokedCertificates    SEQUENCE OF SEQUENCE   {
	*      userCertificate        CertificateSerialNumber,
	*      revocationDate         Time,
	*      crlEntryExtensions     Extensions OPTIONAL
	*                                   -- if present, MUST be v2
	*                        } OPTIONAL
	*/
	if ((ret = x509_get_entries(&p, end, &crl->entry)) != 0)
	{
		x509_crl_free(crl);
		return(ret);
	}

	/*
	* crlExtensions          EXPLICIT Extensions OPTIONAL
	*                              -- if present, MUST be v2
	*/
	if (crl->version == 2)
	{
		ret = x509_get_crl_ext(&p, end, &crl->crl_ext);

		if (ret != 0)
		{
			x509_crl_free(crl);
			return(ret);
		}
	}

	if (p != end)
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	end = crl->raw.p + crl->raw.len;

	/*
	*  signatureAlgorithm   AlgorithmIdentifier,
	*  signatureValue       BIT STRING
	*/
	if ((ret = x509_get_alg(&p, end, &sig_oid2, &sig_params2)) != 0)
	{
		x509_crl_free(crl);
		return(ret);
	}

	if (crl->sig_oid.len != sig_oid2.len ||
		xmem_comp(crl->sig_oid.p, sig_oid2.p, crl->sig_oid.len) != 0 ||
		sig_params1.len != sig_params2.len ||
		(sig_params1.len != 0 &&
		xmem_comp(sig_params1.p, sig_params2.p, sig_params1.len) != 0))
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse_der"), _T("ERR_X509_SIG_MISMATCH"), -1);
		return(C_ERR);
	}

	if ((ret = x509_get_sig(&p, end, &crl->sig)) != 0)
	{
		x509_crl_free(crl);
		return(ret);
	}

	if (p != end)
	{
		x509_crl_free(crl);

		set_last_error(_T("x509_crl_parse"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* Parse one or more CRLs and add them to the chained list
*/
int x509_crl_parse(x509_crl *chain, const byte_t *buf, dword_t buflen)
{
	int ret;
	dword_t use_len;
	pem_context pem;
	int is_pem = 0;

	if (chain == NULL || buf == NULL)
	{
		set_last_error(_T("x509_crl_parse"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return(C_ERR);
	}

	do
	{
		pem_init(&pem);

		// Avoid calling pem_read_buffer() on non-null-terminated
		// string
		if (buflen == 0 || buf[buflen - 1] != '\0')
			ret = ERR_PEM_NO_HEADER_FOOTER_PRESENT;
		else
			ret = pem_read_buffer(&pem,
			"-----BEGIN X509 CRL-----",
			"-----END X509 CRL-----",
			buf, NULL, 0, &use_len);

		if (ret == 0)
		{
			/*
			* Was PEM encoded
			*/
			is_pem = 1;

			buflen -= use_len;
			buf += use_len;

			if ((ret = x509_crl_parse_der(chain,
				pem.buf, pem.buflen)) != 0)
			{
				pem_free(&pem);
				return(ret);
			}
		}
		else if (is_pem)
		{
			pem_free(&pem);
			return(ret);
		}

		pem_free(&pem);
	}
	/* In the PEM case, buflen is 1 at the end, for the terminated NULL byte.
	* And a valid CRL cannot be less than 1 byte anyway. */
	while (is_pem && buflen > 1);

	if (is_pem)
		return(0);
	else
		return(x509_crl_parse_der(chain, buf, buflen));
}

/*
* Return an informational string about the certificate.
*/

#define BEFORE_COLON    14
#define BC              "14"

/*
* Return an informational string about the CRL.
*/
int x509_crl_info(char *buf, dword_t size, const char *prefix,
	const x509_crl *crl)
{
	int ret;
	dword_t n;
	char *p;
	const x509_crl_entry *entry;

	p = buf;
	n = size;

	ret = snprintf(p, n, "%sCRL version   : %d",
		prefix, crl->version);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%sissuer name   : ", prefix);
	X509_SAFE_SNPRINTF;
	ret = x509_dn_gets(p, n, &crl->issuer);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%sthis update   : " \
		"%04d-%02d-%02d %02d:%02d:%02d", prefix,
		crl->this_update.year, crl->this_update.mon,
		crl->this_update.day, crl->this_update.hour,
		crl->this_update.min, crl->this_update.sec);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n%snext update   : " \
		"%04d-%02d-%02d %02d:%02d:%02d", prefix,
		crl->next_update.year, crl->next_update.mon,
		crl->next_update.day, crl->next_update.hour,
		crl->next_update.min, crl->next_update.sec);
	X509_SAFE_SNPRINTF;

	entry = &crl->entry;

	ret = snprintf(p, n, "\n%sRevoked certificates:",
		prefix);
	X509_SAFE_SNPRINTF;

	while (entry != NULL && entry->raw.len != 0)
	{
		ret = snprintf(p, n, "\n%sserial number: ",
			prefix);
		X509_SAFE_SNPRINTF;

		ret = x509_serial_gets(p, n, &entry->serial);
		X509_SAFE_SNPRINTF;

		ret = snprintf(p, n, " revocation date: " \
			"%04d-%02d-%02d %02d:%02d:%02d",
			entry->revocation_date.year, entry->revocation_date.mon,
			entry->revocation_date.day, entry->revocation_date.hour,
			entry->revocation_date.min, entry->revocation_date.sec);
		X509_SAFE_SNPRINTF;

		entry = entry->next;
	}

	ret = snprintf(p, n, "\n%ssigned using  : ", prefix);
	X509_SAFE_SNPRINTF;

	ret = x509_sig_alg_gets(p, n, &crl->sig_oid, crl->sig_pk, crl->sig_md, crl->sig_opt_mgf1_md, crl->sig_opt_sale_len);
	X509_SAFE_SNPRINTF;

	ret = snprintf(p, n, "\n");
	X509_SAFE_SNPRINTF;

	return((int)(size - n));
}

#undef BEFORE_COLON
#undef BC

/*
* Initialize a CRL chain
*/
void x509_crl_init(x509_crl *crl)
{
	xmem_zero(crl, sizeof(x509_crl));
}

/*
* Unallocate all CRL data
*/
void x509_crl_free(x509_crl *crl)
{
	x509_crl *crl_cur = crl;
	x509_crl *crl_prv;
	x509_name *name_cur;
	x509_name *name_prv;
	x509_crl_entry *entry_cur;
	x509_crl_entry *entry_prv;

	if (crl == NULL)
		return;

	do
	{
		name_cur = crl_cur->issuer.next;
		while (name_cur != NULL)
		{
			name_prv = name_cur;
			name_cur = name_cur->next;
			xmem_zero(name_prv, sizeof(x509_name));
			xmem_free(name_prv);
		}

		entry_cur = crl_cur->entry.next;
		while (entry_cur != NULL)
		{
			entry_prv = entry_cur;
			entry_cur = entry_cur->next;
			xmem_zero(entry_prv, sizeof(x509_crl_entry));
			xmem_free(entry_prv);
		}

		if (crl_cur->raw.p != NULL)
		{
			xmem_zero(crl_cur->raw.p, crl_cur->raw.len);
			xmem_free(crl_cur->raw.p);
		}

		crl_cur = crl_cur->next;
	} while (crl_cur != NULL);

	crl_cur = crl;
	do
	{
		crl_prv = crl_cur;
		crl_cur = crl_cur->next;

		xmem_zero(crl_prv, sizeof(x509_crl));
		if (crl_prv != crl)
			xmem_free(crl_prv);
	} while (crl_cur != NULL);
}

