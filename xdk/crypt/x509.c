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


#include "x509.h"
#include "oid.h"
#include "ecdsa.h"
#include "pem.h"
#include "asn1.h"

#include "../xdkimp.h"



#define CHECK(code) if( ( ret = ( code ) ) != 0 ){ return( ret ); }
#define CHECK_RANGE(min, max, val)                      \
    do                                                  \
	    {                                                   \
        if( ( val ) < ( min ) || ( val ) > ( max ) )    \
		        {                                               \
            return( ret );                              \
		        }                                               \
	    } while( 0 )

/*
*  CertificateSerialNumber  :: = INTEGER
*/
int x509_get_serial(byte_t **p, const byte_t *end,
	x509_buf *serial)
{
	int ret;

	if ((end - *p) < 1)
	{
		set_last_error(_T("x509_get_serial"), _T("ERR_X509_INVALID_SERIAL"), -1);
		return C_ERR;
	}

	if (**p != (ASN1_CONTEXT_SPECIFIC | ASN1_PRIMITIVE | 2) &&
		**p != ASN1_INTEGER)
	{
		set_last_error(_T("x509_get_serial"), _T("ERR_X509_INVALID_SERIAL"), -1);
		return C_ERR;
	}

	serial->tag = *(*p)++;

	if ((ret = asn1_get_len(p, end, &serial->len)) != 0)
	{
		set_last_error(_T("x509_get_serial"), _T("ERR_X509_INVALID_SERIAL"), -1);
		return C_ERR;
	}

	serial->p = *p;
	*p += serial->len;

	return(0);
}

/* Get an algorithm identifier without parameters (eg for signatures)
*
*  AlgorithmIdentifier  ::=  SEQUENCE  {
*       algorithm               OBJECT IDENTIFIER,
*       parameters              ANY DEFINED BY algorithm OPTIONAL  }
*/
int x509_get_alg_null(byte_t **p, const byte_t *end,
	x509_buf *alg)
{
	int ret;

	if ((ret = asn1_get_alg_null(p, end, alg)) != 0)
	{
		set_last_error(_T("x509_get_serial"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Parse an algorithm identifier with (optional) parameters
*/
int x509_get_alg(byte_t **p, const byte_t *end,
	x509_buf *alg, x509_buf *params)
{
	int ret;

	if ((ret = asn1_get_alg(p, end, alg, params)) != 0)
	{
		set_last_error(_T("x509_get_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* HashAlgorithm ::= AlgorithmIdentifier
*
* AlgorithmIdentifier  ::=  SEQUENCE  {
*      algorithm               OBJECT IDENTIFIER,
*      parameters              ANY DEFINED BY algorithm OPTIONAL  }
*
* For HashAlgorithm, parameters MUST be NULL or absent.
*/
static int x509_get_hash_alg(const x509_buf *alg, md_type_t *md_alg)
{
	int ret;
	byte_t *p;
	const byte_t *end;
	x509_buf md_oid;
	dword_t len;

	/* Make sure we got a SEQUENCE and setup bounds */
	if (alg->tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("x509_get_hash_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	p = (byte_t *)alg->p;
	end = p + alg->len;

	if (p >= end)
	{
		set_last_error(_T("x509_get_hash_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	/* Parse md_oid */
	md_oid.tag = *p;

	if ((ret = asn1_get_tag(&p, end, &md_oid.len, ASN1_OID)) != 0)
	{
		set_last_error(_T("x509_get_hash_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	md_oid.p = p;
	p += md_oid.len;

	/* Get md_alg from md_oid */
	if ((ret = oid_get_md_alg(&md_oid, md_alg)) != 0)
	{
		set_last_error(_T("x509_get_hash_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	/* Make sure params is absent of NULL */
	if (p == end)
		return(0);

	if ((ret = asn1_get_tag(&p, end, &len, ASN1_NULL)) != 0 || len != 0)
	{
		set_last_error(_T("x509_get_hash_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	if (p != end)
	{
		set_last_error(_T("x509_get_hash_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	return(0);
}

/*
*    RSASSA-PSS-params  ::=  SEQUENCE  {
*       hashAlgorithm     [0] HashAlgorithm DEFAULT sha1Identifier,
*       maskGenAlgorithm  [1] MaskGenAlgorithm DEFAULT mgf1SHA1Identifier,
*       saltLength        [2] INTEGER DEFAULT 20,
*       trailerField      [3] INTEGER DEFAULT 1  }
*    -- Note that the tags in this Sequence are explicit.
*
* RFC 4055 (which defines use of RSASSA-PSS in PKIX) states that the value
* of trailerField MUST be 1, and PKCS#1 v2.2 doesn't even define any other
* option. Enfore this at parsing time.
*/
int x509_get_rsassa_pss_params(const x509_buf *params,
	md_type_t *md_alg, md_type_t *mgf_md,
	int *salt_len)
{
	int ret;
	byte_t *p;
	const byte_t *end, *end2;
	dword_t len;
	x509_buf alg_id, alg_params;

	/* First set everything to defaults */
	*md_alg = MD_SHA1;
	*mgf_md = MD_SHA1;
	*salt_len = 20;

	/* Make sure params is a SEQUENCE and setup bounds */
	if (params->tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	p = (byte_t *)params->p;
	end = p + params->len;

	if (p == end)
		return(0);

	/*
	* HashAlgorithm
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0)) == 0)
	{
		end2 = p + len;

		/* HashAlgorithm ::= AlgorithmIdentifier (without parameters) */
		if ((ret = x509_get_alg_null(&p, end2, &alg_id)) != 0)
			return(ret);

		if ((ret = oid_get_md_alg(&alg_id, md_alg)) != 0)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}

		if (p != end2)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}
	}
	else if (ret != ERR_ASN1_UNEXPECTED_TAG)
	{
		set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	if (p == end)
		return(0);

	/*
	* MaskGenAlgorithm
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 1)) == 0)
	{
		end2 = p + len;

		/* MaskGenAlgorithm ::= AlgorithmIdentifier (params = HashAlgorithm) */
		if ((ret = x509_get_alg(&p, end2, &alg_id, &alg_params)) != 0)
			return(ret);

		/* Only MFG1 is recognised for now */
		if (OID_CMP(OID_MGF1, &alg_id) != 0)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_FEATURE_UNAVAILABLE"), -1);
			return C_ERR;
		}

		/* Parse HashAlgorithm */
		if ((ret = x509_get_hash_alg(&alg_params, mgf_md)) != 0)
			return(ret);

		if (p != end2)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}
	}
	else if (ret != ERR_ASN1_UNEXPECTED_TAG)
	{
		set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	if (p == end)
		return(0);

	/*
	* salt_len
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 2)) == 0)
	{
		end2 = p + len;

		if ((ret = asn1_get_int(&p, end2, salt_len)) != 0)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}

		if (p != end2)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}
	}
	else if (ret != ERR_ASN1_UNEXPECTED_TAG)
	{
		set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	if (p == end)
		return(0);

	/*
	* trailer_field (if present, must be 1)
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 3)) == 0)
	{
		int trailer_field;

		end2 = p + len;

		if ((ret = asn1_get_int(&p, end2, &trailer_field)) != 0)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}

		if (p != end2)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}

		if (trailer_field != 1)
		{
			set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}
	}
	else if (ret != ERR_ASN1_UNEXPECTED_TAG)
	{
		set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	if (p != end)
	{
		set_last_error(_T("x509_get_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG"), -1);
		return C_ERR;
	}

	return(0);
}

/*
*  AttributeTypeAndValue ::= SEQUENCE {
*    type     AttributeType,
*    value    AttributeValue }
*
*  AttributeType ::= OBJECT IDENTIFIER
*
*  AttributeValue ::= ANY DEFINED BY AttributeType
*/
static int x509_get_attr_type_value(byte_t **p,
	const byte_t *end,
	x509_name *cur)
{
	int ret;
	dword_t len;
	x509_buf *oid;
	x509_buf *val;

	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		set_last_error(_T("x509_get_attr_type_value"), _T("ERR_X509_INVALID_NAME"), -1);
		return C_ERR;
	}

	end = *p + len;

	if ((end - *p) < 1)
	{
		set_last_error(_T("x509_get_attr_type_value"), _T("ERR_X509_INVALID_NAME"), -1);
		return C_ERR;
	}

	oid = &cur->oid;
	oid->tag = **p;

	if ((ret = asn1_get_tag(p, end, &oid->len, ASN1_OID)) != 0)
	{
		set_last_error(_T("x509_get_attr_type_value"), _T("ERR_X509_INVALID_NAME"), -1);
		return C_ERR;
	}

	oid->p = *p;
	*p += oid->len;

	if ((end - *p) < 1)
	{
		set_last_error(_T("x509_get_attr_type_value"), _T("ERR_X509_INVALID_NAME"), -1);
		return C_ERR;
	}

	if (**p != ASN1_BMP_STRING && **p != ASN1_UTF8_STRING      &&
		**p != ASN1_T61_STRING && **p != ASN1_PRINTABLE_STRING &&
		**p != ASN1_IA5_STRING && **p != ASN1_UNIVERSAL_STRING &&
		**p != ASN1_BIT_STRING)
	{
		set_last_error(_T("x509_get_attr_type_value"), _T("ERR_X509_INVALID_NAME"), -1);
		return C_ERR;
	}

	val = &cur->val;
	val->tag = *(*p)++;

	if ((ret = asn1_get_len(p, end, &val->len)) != 0)
	{
		set_last_error(_T("x509_get_attr_type_value"), _T("ERR_X509_INVALID_NAME"), -1);
		return C_ERR;
	}

	val->p = *p;
	*p += val->len;

	if (*p != end)
	{
		set_last_error(_T("x509_get_attr_type_value"), _T("ERR_X509_INVALID_NAME"), -1);
		return C_ERR;
	}

	cur->next = NULL;

	return(0);
}

/*
*  Name ::= CHOICE { -- only one possibility for now --
*       rdnSequence  RDNSequence }
*
*  RDNSequence ::= SEQUENCE OF RelativeDistinguishedName
*
*  RelativeDistinguishedName ::=
*    SET OF AttributeTypeAndValue
*
*  AttributeTypeAndValue ::= SEQUENCE {
*    type     AttributeType,
*    value    AttributeValue }
*
*  AttributeType ::= OBJECT IDENTIFIER
*
*  AttributeValue ::= ANY DEFINED BY AttributeType
*
* The data structure is optimized for the common case where each RDN has only
* one element, which is represented as a list of AttributeTypeAndValue.
* For the general case we still use a flat list, but we mark elements of the
* same set so that they are "merged" together in the functions that consume
* this list, eg x509_dn_gets().
*/
int x509_get_name(byte_t **p, const byte_t *end,
	x509_name *cur)
{
	int ret;
	dword_t set_len;
	const byte_t *end_set;

	/* don't use recursion, we'd risk stack overflow if not optimized */
	while (1)
	{
		/*
		* parse SET
		*/
		if ((ret = asn1_get_tag(p, end, &set_len,
			ASN1_CONSTRUCTED | ASN1_SET)) != 0)
		{
			set_last_error(_T("x509_get_name"), _T("ERR_X509_INVALID_NAME"), -1);
			return C_ERR;
		}

		end_set = *p + set_len;

		while (1)
		{
			if ((ret = x509_get_attr_type_value(p, end_set, cur)) != 0)
				return(ret);

			if (*p == end_set)
				break;

			/* Mark this item as being no the only one in a set */
			cur->next_merged = 1;

			cur->next = xmem_alloc(sizeof(x509_name));

			if (cur->next == NULL)
			{
				set_last_error(_T("x509_get_name"), _T("ERR_X509_ALLOC_FAILED"), -1);
				return C_ERR;
			}

			cur = cur->next;
		}

		/*
		* continue until end of SEQUENCE is reached
		*/
		if (*p == end)
			return(0);

		cur->next = xmem_alloc(sizeof(x509_name));

		if (cur->next == NULL)
		{
			set_last_error(_T("x509_get_name"), _T("ERR_X509_ALLOC_FAILED"), -1);
			return C_ERR;
		}

		cur = cur->next;
	}
}

static int x509_parse_int(byte_t **p, dword_t n, int *res)
{
	*res = 0;

	for (; n > 0; --n)
	{
		if ((**p < '0') || (**p > '9'))
		{
			set_last_error(_T("x509_parse_int"), _T("ERR_X509_INVALID_DATE"), -1);
			return C_ERR;
		}

		*res *= 10;
		*res += (*(*p)++ - '0');
	}

	return(0);
}

static int x509_date_is_valid(const x509_time *t)
{
	int ret = C_ERR;
	int month_len;

	CHECK_RANGE(0, 9999, t->year);
	CHECK_RANGE(0, 23, t->hour);
	CHECK_RANGE(0, 59, t->min);
	CHECK_RANGE(0, 59, t->sec);

	switch (t->mon)
	{
	case 1: case 3: case 5: case 7: case 8: case 10: case 12:
		month_len = 31;
		break;
	case 4: case 6: case 9: case 11:
		month_len = 30;
		break;
	case 2:
		if ((!(t->year % 4) && t->year % 100) ||
			!(t->year % 400))
			month_len = 29;
		else
			month_len = 28;
		break;
	default:
		return(ret);
	}
	CHECK_RANGE(1, month_len, t->day);

	return(0);
}

/*
* Parse an ASN1_UTC_TIME (yearlen=2) or ASN1_GENERALIZED_TIME (yearlen=4)
* field.
*/
static int x509_parse_time(byte_t **p, dword_t len, dword_t yearlen,
	x509_time *tm)
{
	int ret;

	/*
	* Minimum length is 10 or 12 depending on yearlen
	*/
	if (len < yearlen + 8)
	{
		set_last_error(_T("x509_parse_time"), _T("ERR_X509_INVALID_DATE"), -1);
		return C_ERR;
	}

	len -= yearlen + 8;

	/*
	* Parse year, month, day, hour, minute
	*/
	CHECK(x509_parse_int(p, yearlen, &tm->year));
	if (2 == yearlen)
	{
		if (tm->year < 50)
			tm->year += 100;

		tm->year += 1900;
	}

	CHECK(x509_parse_int(p, 2, &tm->mon));
	CHECK(x509_parse_int(p, 2, &tm->day));
	CHECK(x509_parse_int(p, 2, &tm->hour));
	CHECK(x509_parse_int(p, 2, &tm->min));

	/*
	* Parse seconds if present
	*/
	if (len >= 2)
	{
		CHECK(x509_parse_int(p, 2, &tm->sec));
		len -= 2;
	}
	else
	{
		set_last_error(_T("x509_parse_time"), _T("ERR_X509_INVALID_DATE"), -1);
		return(C_ERR);
	}

	/*
	* Parse trailing 'Z' if present
	*/
	if (1 == len && 'Z' == **p)
	{
		(*p)++;
		len--;
	}

	/*
	* We should have parsed all characters at this point
	*/
	if (0 != len)
	{
		set_last_error(_T("x509_parse_time"), _T("ERR_X509_INVALID_DATE"), -1);
		return C_ERR;
	}

	CHECK(x509_date_is_valid(tm));

	return (0);
}

/*
*  Time ::= CHOICE {
*       utcTime        UTCTime,
*       generalTime    GeneralizedTime }
*/
int x509_get_time(byte_t **p, const byte_t *end,
	x509_time *tm)
{
	int ret;
	dword_t len, year_len;
	byte_t tag;

	if ((end - *p) < 1)
	{
		set_last_error(_T("x509_parse_time"), _T("ERR_X509_INVALID_DATE"), -1);
		return C_ERR;
	}

	tag = **p;

	if (tag == ASN1_UTC_TIME)
		year_len = 2;
	else if (tag == ASN1_GENERALIZED_TIME)
		year_len = 4;
	else
	{
		set_last_error(_T("x509_parse_time"), _T("ERR_X509_INVALID_DATE"), -1);
		return C_ERR;
	}

	(*p)++;
	ret = asn1_get_len(p, end, &len);

	if (ret != 0)
	{
		set_last_error(_T("x509_parse_time"), _T("ERR_X509_INVALID_DATE"), -1);
		return C_ERR;
	}

	return x509_parse_time(p, len, year_len, tm);
}

int x509_get_sig(byte_t **p, const byte_t *end, x509_buf *sig)
{
	int ret;
	dword_t len;
	int tag_type;

	if ((end - *p) < 1)
	{
		set_last_error(_T("x509_get_sig"), _T("ERR_X509_INVALID_SIGNATURE"), -1);
		return C_ERR;
	}

	tag_type = **p;

	if ((ret = asn1_get_bitstring_null(p, end, &len)) != 0)
	{
		set_last_error(_T("x509_get_sig"), _T("ERR_X509_INVALID_SIGNATURE"), -1);
		return C_ERR;
	}

	sig->tag = tag_type;
	sig->len = len;
	sig->p = *p;

	*p += len;

	return(0);
}

/*
* Get signature algorithm from alg OID and optional parameters
*/
int x509_get_sig_alg(const x509_buf *sig_oid, const x509_buf *sig_params,
	md_type_t *md_alg, pk_type_t *pk_alg,
	md_type_t* mgf_md, int *salt_len)
{
	int ret;

	if ((ret = oid_get_sig_alg(sig_oid, md_alg, pk_alg)) != 0)
	{
		set_last_error(_T("x509_get_sig_alg"), _T("ERR_X509_UNKNOWN_SIG_ALG"), -1);
		return C_ERR;
	}

	if (*pk_alg == PK_RSASSA_PSS)
	{
		ret = x509_get_rsassa_pss_params(sig_params,
			md_alg,
			mgf_md,
			salt_len);
		if (ret != 0)
		{
			return(ret);
		}
	}
	else
	{
		/* Make sure parameters are absent or NULL */
		if ((sig_params->tag != ASN1_NULL && sig_params->tag != 0) ||
			sig_params->len != 0)
		{
			set_last_error(_T("x509_get_sig_alg"), _T("ERR_X509_INVALID_ALG"), -1);
			return C_ERR;
		}
	}

	return(0);
}

/*
* X.509 Extensions (No parsing of extensions, pointer should
* be either manually updated or extensions should be parsed!)
*/
int x509_get_ext(byte_t **p, const byte_t *end,
	x509_buf *ext, int tag)
{
	int ret;
	dword_t len;

	/* Extension structure use EXPLICIT tagging. That is, the actual
	* `Extensions` structure is wrapped by a tag-length pair using
	* the respective context-specific tag. */
	ret = asn1_get_tag(p, end, &ext->len,
		ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | tag);
	if (ret != 0)
	{
		set_last_error(_T("x509_get_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return C_ERR;
	}

	ext->tag = ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | tag;
	ext->p = *p;
	end = *p + ext->len;

	/*
	* Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
	*/
	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		set_last_error(_T("x509_get_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return C_ERR;
	}

	if (end != *p + len)
	{
		set_last_error(_T("x509_get_ext"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Store the name in printable form into buf; no more
* than size characters will be written
*/
int x509_dn_gets(char *buf, dword_t size, const x509_name *dn)
{
	int ret;
	dword_t i, n;
	byte_t c, merge = 0;
	const x509_name *name;
	const char *short_name = NULL;
	char s[X509_MAX_DN_NAME_SIZE], *p;

	xmem_zero(s, sizeof(s));

	name = dn;
	p = buf;
	n = size;

	while (name != NULL)
	{
		if (!name->oid.p)
		{
			name = name->next;
			continue;
		}

		if (name != dn)
		{
			ret = snprintf(p, n, merge ? " + " : ", ");
			X509_SAFE_SNPRINTF;
		}

		ret = oid_get_attr_short_name(&name->oid, &short_name);

		if (ret == 0)
			ret = snprintf(p, n, "%s=", short_name);
		else
			ret = snprintf(p, n, "\?\?=");
		X509_SAFE_SNPRINTF;

		for (i = 0; i < name->val.len; i++)
		{
			if (i >= sizeof(s) - 1)
				break;

			c = name->val.p[i];
			if (c < 32 || c == 127 || (c > 128 && c < 160))
				s[i] = '?';
			else s[i] = c;
		}
		s[i] = '\0';
		ret = snprintf(p, n, "%s", s);
		X509_SAFE_SNPRINTF;

		merge = name->next_merged;
		name = name->next;
	}

	return((int)(size - n));
}

/*
* Store the serial in printable form into buf; no more
* than size characters will be written
*/
int x509_serial_gets(char *buf, dword_t size, const x509_buf *serial)
{
	int ret;
	dword_t i, n, nr;
	char *p;

	p = buf;
	n = size;

	nr = (serial->len <= 32)
		? serial->len : 28;

	for (i = 0; i < nr; i++)
	{
		if (i == 0 && nr > 1 && serial->p[i] == 0x0)
			continue;

		ret = snprintf(p, n, "%02X%s",
			serial->p[i], (i < nr - 1) ? ":" : "");
		X509_SAFE_SNPRINTF;
	}

	if (nr != serial->len)
	{
		ret = snprintf(p, n, "....");
		X509_SAFE_SNPRINTF;
	}

	return((int)(size - n));
}

/*
* Helper for writing signature algorithms
*/
int x509_sig_alg_gets(char *buf, dword_t size, const x509_buf *sig_oid,
	pk_type_t pk_alg, md_type_t md_alg,
	md_type_t opt_mgf_md, int opt_salt_len)
{
	int ret;
	char *p = buf;
	dword_t n = size;
	const char *desc = NULL;

	ret = oid_get_sig_alg_desc(sig_oid, &desc);
	if (ret != 0)
		ret = snprintf(p, n, "???");
	else
		ret = snprintf(p, n, "%s", desc);
	X509_SAFE_SNPRINTF;

	if (pk_alg == PK_RSASSA_PSS)
	{
		const md_info_t *md_info, *mgf_md_info;

		md_info = md_info_from_type(md_alg);
		mgf_md_info = md_info_from_type(opt_mgf_md);

		ret = snprintf(p, n, " (%s, MGF1-%s, 0x%02X)",
			md_info ? md_info->name : "???",
			mgf_md_info ? mgf_md_info->name : "???",
			opt_salt_len);
		X509_SAFE_SNPRINTF;
	}
	else
	{
		((void)pk_alg);
		((void)md_alg);
	}

	return((int)(size - n));
}

/*
* Helper for writing "RSA key size", "EC key size", etc
*/
int x509_key_size_helper(char *buf, dword_t buf_size, const char *name)
{
	char *p = buf;
	dword_t n = buf_size;
	int ret;

	ret = snprintf(p, n, "%s key size", name);
	X509_SAFE_SNPRINTF;

	return(0);
}

/*
* Set the time structure to the current time.
* Return 0 on success, non-zero on failure.
*/
static int x509_get_current_time(x509_time *now)
{
	struct tm *lt, tm_buf;
	time_t tt;
	int ret = 0;

	tt = time(NULL);

#if defined(_WIN32) && !defined(EFIX64) && !defined(EFI32)
	lt = (gmtime_s(&tm_buf, &tt) == 0)? &tm_buf : NULL;
#else
	lt = gmtime_r(&tt, &tm_buf);
#endif

	if (lt == NULL)
		ret = -1;
	else
	{
		now->year = lt->tm_year + 1900;
		now->mon = lt->tm_mon + 1;
		now->day = lt->tm_mday;
		now->hour = lt->tm_hour;
		now->min = lt->tm_min;
		now->sec = lt->tm_sec;
	}

	return(ret);
}

/*
* Return 0 if before <= after, 1 otherwise
*/
static int x509_check_time(const x509_time *before, const x509_time *after)
{
	if (before->year  > after->year)
		return(1);

	if (before->year == after->year &&
		before->mon   > after->mon)
		return(1);

	if (before->year == after->year &&
		before->mon == after->mon  &&
		before->day   > after->day)
		return(1);

	if (before->year == after->year &&
		before->mon == after->mon  &&
		before->day == after->day  &&
		before->hour  > after->hour)
		return(1);

	if (before->year == after->year &&
		before->mon == after->mon  &&
		before->day == after->day  &&
		before->hour == after->hour &&
		before->min   > after->min)
		return(1);

	if (before->year == after->year &&
		before->mon == after->mon  &&
		before->day == after->day  &&
		before->hour == after->hour &&
		before->min == after->min  &&
		before->sec   > after->sec)
		return(1);

	return(0);
}

int x509_time_is_past(const x509_time *to)
{
	x509_time now;

	if (x509_get_current_time(&now) != 0)
		return(1);

	return(x509_check_time(&now, to));
}

int x509_time_is_future(const x509_time *from)
{
	x509_time now;

	if (x509_get_current_time(&now) != 0)
		return(1);

	return(x509_check_time(from, &now));
}

/* Structure linking OIDs for X.509 DN AttributeTypes to their
* string representations and default string encodings used by Mbed TLS. */
typedef struct {
	const char *name; /* String representation of AttributeType, e.g.
					  * "CN" or "emailAddress". */
	dword_t name_len;  /* Length of 'name', without trailing 0 byte. */
	const char *oid;  /* String representation of OID of AttributeType,
					  * as per RFC 5280, Appendix A.1. */
	int default_tag;  /* The default character encoding used for the
					  * given attribute type, e.g.
					  * ASN1_UTF8_STRING for UTF-8. */
} x509_attr_descriptor_t;

#define ADD_STRLEN( s )     s, sizeof( s ) - 1

/* X.509 DN attributes from RFC 5280, Appendix A.1. */
static const x509_attr_descriptor_t x509_attrs[] =
{
	{ ADD_STRLEN( "CN" ),
	OID_AT_CN, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "commonName" ),
	OID_AT_CN, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "C" ),
	OID_AT_COUNTRY, ASN1_PRINTABLE_STRING },
	{ ADD_STRLEN( "countryName" ),
	OID_AT_COUNTRY, ASN1_PRINTABLE_STRING },
	{ ADD_STRLEN( "O" ),
	OID_AT_ORGANIZATION, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "organizationName" ),
	OID_AT_ORGANIZATION, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "L" ),
	OID_AT_LOCALITY, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "locality" ),
	OID_AT_LOCALITY, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "R" ),
	OID_PKCS9_EMAIL, ASN1_IA5_STRING },
	{ ADD_STRLEN( "OU" ),
	OID_AT_ORG_UNIT, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "organizationalUnitName" ),
	OID_AT_ORG_UNIT, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "ST" ),
	OID_AT_STATE, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "stateOrProvinceName" ),
	OID_AT_STATE, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "emailAddress" ),
	OID_PKCS9_EMAIL, ASN1_IA5_STRING },
	{ ADD_STRLEN( "serialNumber" ),
	OID_AT_SERIAL_NUMBER, ASN1_PRINTABLE_STRING },
	{ ADD_STRLEN( "postalAddress" ),
	OID_AT_POSTAL_ADDRESS, ASN1_PRINTABLE_STRING },
	{ ADD_STRLEN( "postalCode" ),
	OID_AT_POSTAL_CODE, ASN1_PRINTABLE_STRING },
	{ ADD_STRLEN( "dnQualifier" ),
	OID_AT_DN_QUALIFIER, ASN1_PRINTABLE_STRING },
	{ ADD_STRLEN( "title" ),
	OID_AT_TITLE, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "surName" ),
	OID_AT_SUR_NAME, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "SN" ),
	OID_AT_SUR_NAME, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "givenName" ),
	OID_AT_GIVEN_NAME, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "GN" ),
	OID_AT_GIVEN_NAME, ASN1_UTF8_STRING },
	{ ADD_STRLEN( "initials" ),
	OID_AT_INITIALS, ASN1_UTF8_STRING },
	{ ADD_STRLEN("pseudonym"),
	OID_AT_PSEUDONYM, ASN1_UTF8_STRING },
	{ ADD_STRLEN("generationQualifier"),
	OID_AT_GENERATION_QUALIFIER, ASN1_UTF8_STRING },
	{ ADD_STRLEN("domainComponent"),
	OID_DOMAIN_COMPONENT, ASN1_IA5_STRING },
	{ ADD_STRLEN("DC"),
	OID_DOMAIN_COMPONENT, ASN1_IA5_STRING },
	{ NULL, 0, NULL, ASN1_NULL }
};

static const x509_attr_descriptor_t *x509_attr_descr_from_name(const char *name, dword_t name_len)
{
	const x509_attr_descriptor_t *cur;

	for (cur = x509_attrs; cur->name != NULL; cur++)
		if (cur->name_len == name_len &&
			strncmp(cur->name, name, name_len) == 0)
			break;

	if (cur->name == NULL)
		return(NULL);

	return(cur);
}

int x509_string_to_names(asn1_named_data **head, const char *name)
{
	int ret = 0;
	const char *s = name, *c = s;
	const char *end = s + strlen(s);
	const char *oid = NULL;
	const x509_attr_descriptor_t* attr_descr = NULL;
	int in_tag = 1;
	char data[X509_MAX_DN_NAME_SIZE];
	char *d = data;

	/* Clear existing chain if present */
	asn1_free_named_data_list(head);

	while (c <= end)
	{
		if (in_tag && *c == '=')
		{
			if ((attr_descr = x509_attr_descr_from_name(s, c - s)) == NULL)
			{
				set_last_error(_T("x509_string_to_names"), _T("ERR_X509_UNKNOWN_OID"), -1);
				ret = C_ERR;
				goto exit;
			}

			oid = attr_descr->oid;
			s = c + 1;
			in_tag = 0;
			d = data;
		}

		if (!in_tag && *c == '\\' && c != end)
		{
			c++;

			/* Check for valid escaped characters */
			if (c == end || *c != ',')
			{
				set_last_error(_T("x509_string_to_names"), _T("ERR_X509_INVALID_NAME"), -1);
				ret = C_ERR;
				goto exit;
			}
		}
		else if (!in_tag && (*c == ',' || c == end))
		{
			asn1_named_data* cur =
				asn1_store_named_data(head, oid, strlen(oid),
				(byte_t *)data,
				d - data);

			if (cur == NULL)
			{
				set_last_error(_T("x509_string_to_names"), _T("ERR_X509_ALLOC_FAILED"), -1);
				return(C_ERR);
			}

			// set tagType
			cur->val.tag = attr_descr->default_tag;

			while (c < end && *(c + 1) == ' ')
				c++;

			s = c + 1;
			in_tag = 1;
		}

		if (!in_tag && s != c + 1)
		{
			*(d++) = *c;

			if (d - data == X509_MAX_DN_NAME_SIZE)
			{
				set_last_error(_T("x509_string_to_names"), _T("ERR_X509_INVALID_NAME"), -1);
				ret = C_ERR;
				goto exit;
			}
		}

		c++;
	}

exit:

	return(ret);
}

/* The first byte of the value in the asn1_named_data structure is reserved
* to store the critical boolean for us
*/
int x509_set_extension(asn1_named_data **head, const char *oid, dword_t oid_len,
	int critical, const byte_t *val, dword_t val_len)
{
	asn1_named_data *cur;

	if ((cur = asn1_store_named_data(head, oid, oid_len,
		NULL, val_len + 1)) == NULL)
	{
		set_last_error(_T("x509_set_extension"), _T("ERR_X509_ALLOC_FAILED"), -1);
		return(C_ERR);
	}

	cur->val.p[0] = critical;
	xmem_copy(cur->val.p + 1, val, val_len);

	return(0);
}

/*
*  RelativeDistinguishedName ::=
*    SET OF AttributeTypeAndValue
*
*  AttributeTypeAndValue ::= SEQUENCE {
*    type     AttributeType,
*    value    AttributeValue }
*
*  AttributeType ::= OBJECT IDENTIFIER
*
*  AttributeValue ::= ANY DEFINED BY AttributeType
*/
static int x509_write_name(byte_t **p, byte_t *start, asn1_named_data* cur_name)
{
	int ret;
	dword_t len = 0;
	const char *oid = (const char*)cur_name->oid.p;
	dword_t oid_len = cur_name->oid.len;
	const byte_t *name = cur_name->val.p;
	dword_t name_len = cur_name->val.len;

	// Write correct string tag and value
	ASN1_CHK_ADD(len, asn1_write_tagged_string(p, start,
		cur_name->val.tag,
		(const char *)name,
		name_len));
	// Write OID
	//
	ASN1_CHK_ADD(len, asn1_write_oid(p, start, oid,
		oid_len));

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start,
		ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start,
		ASN1_CONSTRUCTED |
		ASN1_SET));

	return((int)len);
}

int x509_write_names(byte_t **p, byte_t *start,
	asn1_named_data *first)
{
	int ret;
	dword_t len = 0;
	asn1_named_data *cur = first;

	while (cur != NULL)
	{
		ASN1_CHK_ADD(len, x509_write_name(p, start, cur));
		cur = cur->next;
	}

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	return((int)len);
}

int x509_write_sig(byte_t **p, byte_t *start,
	const char *oid, dword_t oid_len,
	byte_t *sig, dword_t size)
{
	int ret;
	dword_t len = 0;

	len = size;
	(*p) -= len;
	xmem_copy(*p, sig, len);

	*--(*p) = 0;
	len += 1;

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_BIT_STRING));

	// Write OID
	//
	ASN1_CHK_ADD(len, asn1_write_algorithm_identifier(p, start, oid,
		oid_len, 0));

	return((int)len);
}

static int x509_write_extension(byte_t **p, byte_t *start,
	asn1_named_data *ext)
{
	int ret;
	dword_t len = 0;

	ASN1_CHK_ADD(len, asn1_write_raw_buffer(p, start, ext->val.p + 1,
		ext->val.len - 1));
	ASN1_CHK_ADD(len, asn1_write_len(p, start, ext->val.len - 1));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_OCTET_STRING));

	if (ext->val.p[0] != 0)
	{
		ASN1_CHK_ADD(len, asn1_write_bool(p, start, 1));
	}

	ASN1_CHK_ADD(len, asn1_write_raw_buffer(p, start, ext->oid.p,
		ext->oid.len));
	ASN1_CHK_ADD(len, asn1_write_len(p, start, ext->oid.len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_OID));

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	return((int)len);
}

/*
* Extension  ::=  SEQUENCE  {
*     extnID      OBJECT IDENTIFIER,
*     critical    BOOLEAN DEFAULT FALSE,
*     extnValue   OCTET STRING
*                 -- contains the DER encoding of an ASN.1 value
*                 -- corresponding to the extension type identified
*                 -- by extnID
*     }
*/
int x509_write_extensions(byte_t **p, byte_t *start,
	asn1_named_data *first)
{
	int ret;
	dword_t len = 0;
	asn1_named_data *cur_ext = first;

	while (cur_ext != NULL)
	{
		ASN1_CHK_ADD(len, x509_write_extension(p, start, cur_ext));
		cur_ext = cur_ext->next;
	}

	return((int)len);
}

/*
*  Version  ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
*/
int x509_get_version(byte_t **p,
	const byte_t *end,
	int *ver)
{
	int ret;
	dword_t len;

	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0)) != 0)
	{
		if (ret == ERR_ASN1_UNEXPECTED_TAG)
		{
			*ver = 0;
			return(0);
		}

		set_last_error(_T("x509_get_version"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	end = *p + len;

	if ((ret = asn1_get_int(p, end, ver)) != 0)
	{
		set_last_error(_T("x509_get_version"), _T("ERR_X509_INVALID_VERSION"), -1);
		return(C_ERR);
	}

	if (*p != end)
	{
		set_last_error(_T("x509_get_version"), _T("ERR_X509_INVALID_VERSION"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
*  Validity ::= SEQUENCE {
*       notBefore      Time,
*       notAfter       Time }
*/
int x509_get_dates(byte_t **p,
	const byte_t *end,
	x509_time *from,
	x509_time *to)
{
	int ret;
	dword_t len;

	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		set_last_error(_T("x509_get_dates"), _T("ERR_X509_INVALID_DATE"), -1);
		return(C_ERR);
	}

	end = *p + len;

	if ((ret = x509_get_time(p, end, from)) != 0)
		return(ret);

	if ((ret = x509_get_time(p, end, to)) != 0)
		return(ret);

	if (*p != end)
	{
		set_last_error(_T("x509_get_dates"), _T("ERR_X509_INVALID_DATE"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* X.509 v2/v3 unique identifier (not parsed)
*/
int x509_get_uid(byte_t **p,
	const byte_t *end,
	x509_buf *uid, int n)
{
	int ret;

	if (*p == end)
		return(0);

	uid->tag = **p;

	if ((ret = asn1_get_tag(p, end, &uid->len,
		ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | n)) != 0)
	{
		if (ret == ERR_ASN1_UNEXPECTED_TAG)
			return(0);

		set_last_error(_T("x509_get_uid"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	uid->p = *p;
	*p += uid->len;

	return(0);
}

int x509_get_basic_constraints(byte_t **p,
	const byte_t *end,
	int *ca_istrue,
	int *max_pathlen)
{
	int ret;
	dword_t len;

	/*
	* BasicConstraints ::= SEQUENCE {
	*      cA                      BOOLEAN DEFAULT FALSE,
	*      pathLenConstraint       INTEGER (0..MAX) OPTIONAL }
	*/
	*ca_istrue = 0; /* DEFAULT FALSE */
	*max_pathlen = 0; /* endless */

	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		set_last_error(_T("x509_get_basic_constraints"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	if (*p == end)
		return(0);

	if ((ret = asn1_get_bool(p, end, ca_istrue)) != 0)
	{
		if (ret == ERR_ASN1_UNEXPECTED_TAG)
			ret = asn1_get_int(p, end, ca_istrue);

		if (ret != 0)
		{
			set_last_error(_T("x509_get_basic_constraints"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		if (*ca_istrue != 0)
			*ca_istrue = 1;
	}

	if (*p == end)
		return(0);

	if ((ret = asn1_get_int(p, end, max_pathlen)) != 0)
	{
		set_last_error(_T("x509_get_basic_constraints"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	if (*p != end)
	{
		set_last_error(_T("x509_get_basic_constraints"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	(*max_pathlen)++;

	return(0);
}

int x509_get_ns_cert_type(byte_t **p,
	const byte_t *end,
	byte_t *ns_cert_type)
{
	int ret;
	x509_bitstring bs = { 0, 0, NULL };

	if ((ret = asn1_get_bitstring(p, end, &bs)) != 0)
	{
		set_last_error(_T("x509_get_ns_cert_type"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	if (bs.len != 1)
	{
		set_last_error(_T("x509_get_ns_cert_type"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	/* Get actual bitstring */
	*ns_cert_type = *bs.p;
	return(0);
}

int x509_get_key_usage(byte_t **p,
	const byte_t *end,
	unsigned int *key_usage)
{
	int ret;
	dword_t i;
	x509_bitstring bs = { 0, 0, NULL };

	if ((ret = asn1_get_bitstring(p, end, &bs)) != 0)
	{
		set_last_error(_T("x509_get_key_usage"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	if (bs.len < 1)
	{
		set_last_error(_T("x509_get_key_usage"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	/* Get actual bitstring */
	*key_usage = 0;
	for (i = 0; i < bs.len && i < sizeof(unsigned int); i++)
	{
		*key_usage |= (unsigned int)bs.p[i] << (8 * i);
	}

	return(0);
}

/*
* ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId
*
* KeyPurposeId ::= OBJECT IDENTIFIER
*/
int x509_get_ext_key_usage(byte_t **p,
	const byte_t *end,
	x509_sequence *ext_key_usage)
{
	int ret;

	if ((ret = asn1_get_sequence_of(p, end, ext_key_usage, ASN1_OID)) != 0)
	{
		set_last_error(_T("x509_get_ext_key_usage"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	/* Sequence length must be >= 1 */
	if (ext_key_usage->buf.p == NULL)
	{
		set_last_error(_T("x509_get_ext_key_usage"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* SubjectAltName ::= GeneralNames
*
* GeneralNames ::= SEQUENCE SIZE (1..MAX) OF GeneralName
*
* GeneralName ::= CHOICE {
*      otherName                       [0]     OtherName,
*      rfc822Name                      [1]     IA5String,
*      dNSName                         [2]     IA5String,
*      x400Address                     [3]     ORAddress,
*      directoryName                   [4]     Name,
*      ediPartyName                    [5]     EDIPartyName,
*      uniformResourceIdentifier       [6]     IA5String,
*      iPAddress                       [7]     OCTET STRING,
*      registeredID                    [8]     OBJECT IDENTIFIER }
*
* OtherName ::= SEQUENCE {
*      type-id    OBJECT IDENTIFIER,
*      value      [0] EXPLICIT ANY DEFINED BY type-id }
*
* EDIPartyName ::= SEQUENCE {
*      nameAssigner            [0]     DirectoryString OPTIONAL,
*      partyName               [1]     DirectoryString }
*
* NOTE: we only parse and use dNSName at this point.
*/
int x509_get_subject_alt_name(byte_t **p,
	const byte_t *end,
	x509_sequence *subject_alt_name)
{
	int ret;
	dword_t len, tag_len;
	asn1_buf *buf;
	byte_t tag;
	asn1_sequence *cur = subject_alt_name;

	/* Get main sequence tag */
	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		set_last_error(_T("x509_get_subject_alt_name"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	if (*p + len != end)
	{
		set_last_error(_T("x509_get_subject_alt_name"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	while (*p < end)
	{
		if ((end - *p) < 1)
		{
			set_last_error(_T("x509_get_subject_alt_name"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		tag = **p;
		(*p)++;
		if ((ret = asn1_get_len(p, end, &tag_len)) != 0)
		{
			set_last_error(_T("x509_get_subject_alt_name"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		if ((tag & ASN1_TAG_CLASS_MASK) !=
			ASN1_CONTEXT_SPECIFIC)
		{
			set_last_error(_T("x509_get_subject_alt_name"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
			return(C_ERR);
		}

		/* Skip everything but DNS name */
		if (tag != (ASN1_CONTEXT_SPECIFIC | 2))
		{
			*p += tag_len;
			continue;
		}

		/* Allocate and assign next pointer */
		if (cur->buf.p != NULL)
		{
			if (cur->next != NULL)
			{
				set_last_error(_T("x509_get_subject_alt_name"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
				return(C_ERR);
			}

			cur->next = xmem_alloc(sizeof(asn1_sequence));

			if (cur->next == NULL)
			{
				set_last_error(_T("x509_get_subject_alt_name"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
				return(C_ERR);
			}

			cur = cur->next;
		}

		buf = &(cur->buf);
		buf->tag = tag;
		buf->p = *p;
		buf->len = tag_len;
		*p += buf->len;
	}

	/* Set final sequence entry's next pointer to NULL */
	cur->next = NULL;

	if (*p != end)
	{
		set_last_error(_T("x509_get_subject_alt_name"), _T("ERR_X509_INVALID_EXTENSIONS"), -1);
		return(C_ERR);
	}

	return(0);
}


/* Get a PK algorithm identifier
*
*  AlgorithmIdentifier  ::=  SEQUENCE  {
*       algorithm               OBJECT IDENTIFIER,
*       parameters              ANY DEFINED BY algorithm OPTIONAL  }
*/
static int x509_get_pk_alg(byte_t **p,
	const byte_t *end,
	pk_type_t *pk_alg, asn1_buf *params)
{
	int ret;
	asn1_buf alg_oid;

	xmem_zero(params, sizeof(asn1_buf));

	if ((ret = asn1_get_alg(p, end, &alg_oid, params)) != 0)
	{
		set_last_error(_T("x509_get_pk_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return(C_ERR);
	}

	if (oid_get_pk_alg(&alg_oid, pk_alg) != 0)
	{
		set_last_error(_T("x509_get_pk_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return(C_ERR);
	}

	/*
	* No parameters with RSA (only for EC)
	*/
	if (*pk_alg == PK_RSA &&
		((params->tag != ASN1_NULL && params->tag != 0) ||
		params->len != 0))
	{
		set_last_error(_T("x509_get_pk_alg"), _T("ERR_X509_INVALID_ALG"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
*  RSAPublicKey ::= SEQUENCE {
*      modulus           INTEGER,  -- n
*      publicExponent    INTEGER   -- e
*  }
*/
static int x509_get_rsapubkey(byte_t **p,
	const byte_t *end,
	rsa_context *rsa)
{
	int ret;
	dword_t len;

	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		set_last_error(_T("x509_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if (*p + len != end)
	{
		set_last_error(_T("x509_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	/* Import N */
	if ((ret = asn1_get_tag(p, end, &len, ASN1_INTEGER)) != 0)
	{
		set_last_error(_T("x509_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((ret = rsa_import_raw(rsa, *p, len, NULL, 0, NULL, 0,
		NULL, 0, NULL, 0)) != 0)
	{
		set_last_error(_T("x509_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	*p += len;

	/* Import E */
	if ((ret = asn1_get_tag(p, end, &len, ASN1_INTEGER)) != 0)
	{
		set_last_error(_T("x509_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((ret = rsa_import_raw(rsa, NULL, 0, NULL, 0, NULL, 0,
		NULL, 0, *p, len)) != 0)
	{
		set_last_error(_T("x509_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	*p += len;

	if (rsa_complete(rsa) != 0 ||
		rsa_check_pubkey(rsa) != 0)
	{
		set_last_error(_T("x509_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if (*p != end)
	{
		set_last_error(_T("x509_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	return(0);
}

/*
* Parse a SpecifiedECDomain (SEC 1 C.2) and (mostly) fill the group with it.
* WARNING: the resulting group should only be used with
* pk_group_id_from_specified(), since its base point may not be set correctly
* if it was encoded compressed.
*
*  SpecifiedECDomain ::= SEQUENCE {
*      version SpecifiedECDomainVersion(ecdpVer1 | ecdpVer2 | ecdpVer3, ...),
*      fieldID FieldID {{FieldTypes}},
*      curve Curve,
*      base ECPoint,
*      order INTEGER,
*      cofactor INTEGER OPTIONAL,
*      hash HashAlgorithm OPTIONAL,
*      ...
*  }
*
* We only support prime-field as field type, and ignore hash and cofactor.
*/
static int x509_group_from_specified(const asn1_buf *params, ecp_group *grp)
{
	int ret;
	byte_t *p = params->p;
	const byte_t * const end = params->p + params->len;
	const byte_t *end_field, *end_curve;
	dword_t len;
	int ver;

	/* SpecifiedECDomainVersion ::= INTEGER { 1, 2, 3 } */
	if ((ret = asn1_get_int(&p, end, &ver)) != 0)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if (ver < 1 || ver > 3)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	/*
	* FieldID { FIELD-ID:IOSet } ::= SEQUENCE { -- Finite field
	*       fieldType FIELD-ID.&id({IOSet}),
	*       parameters FIELD-ID.&Type({IOSet}{@fieldType})
	* }
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
		return(ret);

	end_field = p + len;

	/*
	* FIELD-ID ::= TYPE-IDENTIFIER
	* FieldTypes FIELD-ID ::= {
	*       { Prime-p IDENTIFIED BY prime-field } |
	*       { Characteristic-two IDENTIFIED BY characteristic-two-field }
	* }
	* prime-field OBJECT IDENTIFIER ::= { id-fieldType 1 }
	*/
	if ((ret = asn1_get_tag(&p, end_field, &len, ASN1_OID)) != 0)
		return(ret);

	if (len != OID_SIZE(OID_ANSI_X9_62_PRIME_FIELD) ||
		xmem_comp(p, OID_ANSI_X9_62_PRIME_FIELD, len) != 0)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	p += len;

	/* Prime-p ::= INTEGER -- Field of size p. */
	if ((ret = asn1_get_mpi(&p, end_field, &grp->P)) != 0)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	grp->pbits = mpi_bitlen(&grp->P);

	if (p != end_field)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	/*
	* Curve ::= SEQUENCE {
	*       a FieldElement,
	*       b FieldElement,
	*       seed BIT STRING OPTIONAL
	*       -- Shall be present if used in SpecifiedECDomain
	*       -- with version equal to ecdpVer2 or ecdpVer3
	* }
	*/
	if ((ret = asn1_get_tag(&p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
		return(ret);

	end_curve = p + len;

	/*
	* FieldElement ::= OCTET STRING
	* containing an integer in the case of a prime field
	*/
	if ((ret = asn1_get_tag(&p, end_curve, &len, ASN1_OCTET_STRING)) != 0 ||
		(ret = mpi_read_binary(&grp->A, p, len)) != 0)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	p += len;

	if ((ret = asn1_get_tag(&p, end_curve, &len, ASN1_OCTET_STRING)) != 0 ||
		(ret = mpi_read_binary(&grp->B, p, len)) != 0)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	p += len;

	/* Ignore seed BIT STRING OPTIONAL */
	if ((ret = asn1_get_tag(&p, end_curve, &len, ASN1_BIT_STRING)) == 0)
		p += len;

	if (p != end_curve)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	/*
	* ECPoint ::= OCTET STRING
	*/
	if ((ret = asn1_get_tag(&p, end, &len, ASN1_OCTET_STRING)) != 0)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((ret = ecp_point_read_binary(grp, &grp->G,
		(const byte_t *)p, len)) != 0)
	{
		/*
		* If we can't read the point because it's compressed, cheat by
		* reading only the X coordinate and the parity bit of Y.
		*/
		if (ret != ERR_ECP_FEATURE_UNAVAILABLE ||
			(p[0] != 0x02 && p[0] != 0x03) ||
			len != mpi_size(&grp->P) + 1 ||
			mpi_read_binary(&grp->G.X, p + 1, len - 1) != 0 ||
			mpi_lset(&grp->G.Y, p[0] - 2) != 0 ||
			mpi_lset(&grp->G.Z, 1) != 0)
		{
			set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
			return(C_ERR);
		}
	}

	p += len;

	/*
	* order INTEGER
	*/
	if ((ret = asn1_get_mpi(&p, end, &grp->N)) != 0)
	{
		set_last_error(_T("x509_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	grp->nbits = mpi_bitlen(&grp->N);

	/*
	* Allow optional elements by purposefully not enforcing p == end here.
	*/

	return(0);
}

/*
* Find the group id associated with an (almost filled) group as generated by
* pk_group_from_specified(), or return an error if unknown.
*/
static int group_id_from_group(const ecp_group *grp, ecp_group_id *grp_id)
{
	int ret = 0;
	ecp_group ref;
	const ecp_group_id *id;

	ecp_group_init(&ref);

	for (id = ecp_grp_id_list(); *id != ECP_DP_NONE; id++)
	{
		/* Load the group associated to that id */
		ecp_group_free(&ref);
		MPI_CHK(ecp_group_load(&ref, *id));

		/* Compare to the group we were given, starting with easy tests */
		if (grp->pbits == ref.pbits && grp->nbits == ref.nbits &&
			mpi_cmp_mpi(&grp->P, &ref.P) == 0 &&
			mpi_cmp_mpi(&grp->A, &ref.A) == 0 &&
			mpi_cmp_mpi(&grp->B, &ref.B) == 0 &&
			mpi_cmp_mpi(&grp->N, &ref.N) == 0 &&
			mpi_cmp_mpi(&grp->G.X, &ref.G.X) == 0 &&
			mpi_cmp_mpi(&grp->G.Z, &ref.G.Z) == 0 &&
			/* For Y we may only know the parity bit, so compare only that */
			mpi_get_bit(&grp->G.Y, 0) == mpi_get_bit(&ref.G.Y, 0))
		{
			break;
		}

	}

cleanup:
	ecp_group_free(&ref);

	*grp_id = *id;

	if (ret == 0 && *id == ECP_DP_NONE)
		ret = ERR_ECP_FEATURE_UNAVAILABLE;

	return(ret);
}

/*
* Parse a SpecifiedECDomain (SEC 1 C.2) and find the associated group ID
*/
static int group_id_from_specified(const asn1_buf *params,
	ecp_group_id *grp_id)
{
	int ret;
	ecp_group grp;

	ecp_group_init(&grp);

	if ((ret = x509_group_from_specified(params, &grp)) != 0)
		goto cleanup;

	ret = group_id_from_group(&grp, grp_id);

cleanup:
	ecp_group_free(&grp);

	return(ret);
}

/*
* Use EC parameters to initialise an EC group
*
* ECParameters ::= CHOICE {
*   namedCurve         OBJECT IDENTIFIER
*   specifiedCurve     SpecifiedECDomain -- = SEQUENCE { ... }
*   -- implicitCurve   NULL
*/
static int x509_use_ecparams(const asn1_buf *params, ecp_group *grp)
{
	int ret;
	ecp_group_id grp_id;

	if (params->tag == ASN1_OID)
	{
		if (oid_get_ec_grp(params, &grp_id) != 0)
		{
			set_last_error(_T("x509_use_ecparams"), _T("ERR_X509_UNKNOWN_OID"), -1);
			return(C_ERR);
		}
	}
	else
	{
		if ((ret = group_id_from_specified(params, &grp_id)) != 0)
			return(ret);
	}

	/*
	* grp may already be initilialized; if so, make sure IDs match
	*/
	if (grp->id != ECP_DP_NONE && grp->id != grp_id)
	{
		set_last_error(_T("x509_use_ecparams"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((ret = ecp_group_load(grp, grp_id)) != 0)
		return(ret);

	return(0);
}

/*
* EC public key is an EC point
*
* The caller is responsible for clearing the structure upon failure if
* desired. Take care to pass along the possible ECP_FEATURE_UNAVAILABLE
* return code of ecp_point_read_binary() and leave p in a usable state.
*/
static int x509_get_ecpubkey(byte_t **p, const byte_t *end,
	ecp_keypair *key)
{
	int ret;

	if ((ret = ecp_point_read_binary(&key->grp, &key->Q,
		(const byte_t *)*p, end - *p)) == 0)
	{
		ret = ecp_check_pubkey(&key->grp, &key->Q);
	}

	/*
	* We know ecp_point_read_binary consumed all bytes or failed
	*/
	*p = (byte_t *)end;

	return(ret);
}

/*
*  SubjectPublicKeyInfo  ::=  SEQUENCE  {
*       algorithm            AlgorithmIdentifier,
*       subjectPublicKey     BIT STRING }
*/
int x509_parse_subpubkey(byte_t **p, const byte_t *end, pk_type_t* pkalg, void** pk_ctx)
{
	int ret;
	dword_t len;
	asn1_buf alg_params;
	pk_type_t pk_alg = PK_NONE;
	const pk_info_t *pk_info;

	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
	{
		set_last_error(_T("x509_parse_subpubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	end = *p + len;

	if ((ret = x509_get_pk_alg(p, end, &pk_alg, &alg_params)) != 0)
		return(ret);

	if ((ret = asn1_get_bitstring_null(p, end, &len)) != 0)
	{
		set_last_error(_T("x509_parse_subpubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if (*p + len != end)
	{
		set_last_error(_T("x509_parse_subpubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((pk_info = pk_info_from_type(pk_alg)) == NULL)
	{
		set_last_error(_T("x509_parse_subpubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if (pk_alg == PK_RSA)
	{
		*pkalg = pk_alg;
		*pk_ctx = xmem_alloc(sizeof(rsa_context));
		rsa_init((rsa_context*)(*pk_ctx), 0, 0);

		ret = x509_get_rsapubkey(p, end, (rsa_context*)(*pk_ctx));
	}
	else if (pk_alg == PK_ECKEY_DH || pk_alg == PK_ECKEY)
	{
		*pkalg = pk_alg;
		*pk_ctx = xmem_alloc(sizeof(ecp_keypair));
		ecp_keypair_init((ecp_keypair*)(*pk_ctx));

		ret = x509_use_ecparams(&alg_params, &(((ecp_keypair*)*pk_ctx)->grp));
		if (ret == 0)
			ret = x509_get_ecpubkey(p, end, (ecp_keypair*)(*pk_ctx));
	}
	else
		ret = C_ERR;

	if (ret == 0 && *p != end)
		ret = C_ERR;

	if (ret != 0)
	{
		if (pk_alg == PK_RSA)
		{
			rsa_free((rsa_context*)(*pk_ctx));
			xmem_free(*pk_ctx);
			*pk_ctx = NULL;
		}
		else if (pk_alg == PK_ECKEY_DH || pk_alg == PK_ECKEY)
		{
			ecp_keypair_free((ecp_keypair*)(*pk_ctx));
			xmem_free(*pk_ctx);
			*pk_ctx = NULL;
		}
	}

	return(ret);
}

/*
*  RSAPublicKey ::= SEQUENCE {
*      modulus           INTEGER,  -- n
*      publicExponent    INTEGER   -- e
*  }
*/
static int x509_write_rsa_pubkey(byte_t **p, byte_t *start,
	rsa_context *rsa)
{
	int ret;
	dword_t len = 0;
	mpi T;

	mpi_init(&T);

	/* Export E */
	if ((ret = rsa_export(rsa, NULL, NULL, NULL, NULL, &T)) != 0 ||
		(ret = asn1_write_mpi(p, start, &T)) < 0)
		goto end_of_export;
	len += ret;

	/* Export N */
	if ((ret = rsa_export(rsa, &T, NULL, NULL, NULL, NULL)) != 0 ||
		(ret = asn1_write_mpi(p, start, &T)) < 0)
		goto end_of_export;
	len += ret;

end_of_export:

	mpi_free(&T);
	if (ret < 0)
		return(ret);

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	return((int)len);
}

/*
* EC public key is an EC point
*/
static int x509_write_ec_pubkey(byte_t **p, byte_t *start,
	ecp_keypair *ec)
{
	int ret;
	dword_t len = 0;
	byte_t buf[ECP_MAX_PT_LEN];

	if ((ret = ecp_point_write_binary(&ec->grp, &ec->Q,
		ECP_PF_UNCOMPRESSED,
		&len, buf, sizeof(buf))) != 0)
	{
		return(ret);
	}

	if (*p < start || (dword_t)(*p - start) < len)
	{
		set_last_error(_T("x509_write_ec_pubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	*p -= len;
	xmem_copy(*p, buf, len);

	return((int)len);
}

/*
* ECParameters ::= CHOICE {
*   namedCurve         OBJECT IDENTIFIER
* }
*/
static int x509_write_ec_param(byte_t **p, byte_t *start,
	ecp_keypair *ec)
{
	int ret;
	dword_t len = 0;
	const char *oid;
	dword_t oid_len;

	if ((ret = oid_get_oid_by_ec_grp(ec->grp.id, &oid, &oid_len)) != 0)
		return(ret);

	ASN1_CHK_ADD(len, asn1_write_oid(p, start, oid, oid_len));

	return((int)len);
}

/*
* privateKey  OCTET STRING -- always of length ceil(log2(n)/8)
*/
static int x509_write_ec_private(byte_t **p, byte_t *start,
	ecp_keypair *ec)
{
	int ret;
	dword_t byte_length = (ec->grp.pbits + 7) / 8;
	byte_t tmp[ECP_MAX_BYTES];

	ret = mpi_write_binary(&ec->d, tmp, byte_length);
	if (ret != 0)
		goto exit;
	ret = asn1_write_octet_string(p, start, tmp, byte_length);

exit:
	xmem_zero(tmp, byte_length);
	return(ret);
}

int x509_write_pubkey(byte_t **p, byte_t *start,
	pk_type_t pktype, void *pk_ctx)
{
	int ret;
	dword_t len = 0;

	if (pktype == PK_RSA)
		ASN1_CHK_ADD(len, x509_write_rsa_pubkey(p, start, (rsa_context*)pk_ctx));
	else if (pktype == PK_ECKEY)
		ASN1_CHK_ADD(len, x509_write_ec_pubkey(p, start, (ecp_keypair*)pk_ctx));
	else
	{
		set_last_error(_T("x509_write_pubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	return((int)len);
}

int x509_write_pubkey_der(pk_type_t pktype, void *pk_ctx, byte_t *buf, dword_t size)
{
	int ret;
	byte_t *c;
	dword_t len = 0, par_len = 0, oid_len;
	const char *oid;

	if (size == 0)
	{
		set_last_error(_T("x509_write_pubkey_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	c = buf + size;

	ASN1_CHK_ADD(len, x509_write_pubkey(&c, buf, pktype, pk_ctx));

	if (c - buf < 1)
	{
		set_last_error(_T("x509_write_pubkey_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	/*
	*  SubjectPublicKeyInfo  ::=  SEQUENCE  {
	*       algorithm            AlgorithmIdentifier,
	*       subjectPublicKey     BIT STRING }
	*/
	*--c = 0;
	len += 1;

	ASN1_CHK_ADD(len, asn1_write_len(&c, buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, buf, ASN1_BIT_STRING));

	if ((ret = oid_get_oid_by_pk_alg(pktype,
		&oid, &oid_len)) != 0)
	{
		return(ret);
	}

	if (pktype == PK_ECKEY)
	{
		ASN1_CHK_ADD(par_len, x509_write_ec_param(&c, buf, (ecp_keypair*)pk_ctx));
	}

	ASN1_CHK_ADD(len, asn1_write_algorithm_identifier(&c, buf, oid, oid_len,
		par_len));

	ASN1_CHK_ADD(len, asn1_write_len(&c, buf, len));
	ASN1_CHK_ADD(len, asn1_write_tag(&c, buf, ASN1_CONSTRUCTED |
		ASN1_SEQUENCE));

	return((int)len);
}

int x509_write_key_der(pk_type_t pktype, void *pk_ctx, byte_t *buf, dword_t size)
{
	int ret;
	byte_t *c;
	dword_t len = 0;

	if (size == 0)
	{
		set_last_error(_T("x509_write_key_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	c = buf + size;

	if (pktype == PK_RSA)
	{
		mpi T; /* Temporary holding the exported parameters */
		rsa_context *rsa = (rsa_context*)pk_ctx;

		/*
		* Export the parameters one after another to avoid simultaneous copies.
		*/

		mpi_init(&T);

		/* Export QP */
		if ((ret = rsa_export_crt(rsa, NULL, NULL, &T)) != 0 ||
			(ret = asn1_write_mpi(&c, buf, &T)) < 0)
			goto end_of_export;
		len += ret;

		/* Export DQ */
		if ((ret = rsa_export_crt(rsa, NULL, &T, NULL)) != 0 ||
			(ret = asn1_write_mpi(&c, buf, &T)) < 0)
			goto end_of_export;
		len += ret;

		/* Export DP */
		if ((ret = rsa_export_crt(rsa, &T, NULL, NULL)) != 0 ||
			(ret = asn1_write_mpi(&c, buf, &T)) < 0)
			goto end_of_export;
		len += ret;

		/* Export Q */
		if ((ret = rsa_export(rsa, NULL, NULL,
			&T, NULL, NULL)) != 0 ||
			(ret = asn1_write_mpi(&c, buf, &T)) < 0)
			goto end_of_export;
		len += ret;

		/* Export P */
		if ((ret = rsa_export(rsa, NULL, &T,
			NULL, NULL, NULL)) != 0 ||
			(ret = asn1_write_mpi(&c, buf, &T)) < 0)
			goto end_of_export;
		len += ret;

		/* Export D */
		if ((ret = rsa_export(rsa, NULL, NULL,
			NULL, &T, NULL)) != 0 ||
			(ret = asn1_write_mpi(&c, buf, &T)) < 0)
			goto end_of_export;
		len += ret;

		/* Export E */
		if ((ret = rsa_export(rsa, NULL, NULL,
			NULL, NULL, &T)) != 0 ||
			(ret = asn1_write_mpi(&c, buf, &T)) < 0)
			goto end_of_export;
		len += ret;

		/* Export N */
		if ((ret = rsa_export(rsa, &T, NULL,
			NULL, NULL, NULL)) != 0 ||
			(ret = asn1_write_mpi(&c, buf, &T)) < 0)
			goto end_of_export;
		len += ret;

	end_of_export:

		mpi_free(&T);
		if (ret < 0)
			return(ret);

		ASN1_CHK_ADD(len, asn1_write_int(&c, buf, 0));
		ASN1_CHK_ADD(len, asn1_write_len(&c, buf, len));
		ASN1_CHK_ADD(len, asn1_write_tag(&c,
			buf, ASN1_CONSTRUCTED |
			ASN1_SEQUENCE));
	}
	else if (pktype == PK_ECKEY)
	{
		ecp_keypair *ec = (ecp_keypair*)pk_ctx;
		dword_t pub_len = 0, par_len = 0;

		/*
		* RFC 5915, or SEC1 Appendix C.4
		*
		* ECPrivateKey ::= SEQUENCE {
		*      version        INTEGER { ecPrivkeyVer1(1) } (ecPrivkeyVer1),
		*      privateKey     OCTET STRING,
		*      parameters [0] ECParameters {{ NamedCurve }} OPTIONAL,
		*      publicKey  [1] BIT STRING OPTIONAL
		*    }
		*/

		/* publicKey */
		ASN1_CHK_ADD(pub_len, x509_write_ec_pubkey(&c, buf, ec));

		if (c - buf < 1)
		{
			set_last_error(_T("x509_write_key_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
			return(C_ERR);
		}

		*--c = 0;
		pub_len += 1;

		ASN1_CHK_ADD(pub_len, asn1_write_len(&c, buf, pub_len));
		ASN1_CHK_ADD(pub_len, asn1_write_tag(&c, buf, ASN1_BIT_STRING));

		ASN1_CHK_ADD(pub_len, asn1_write_len(&c, buf, pub_len));
		ASN1_CHK_ADD(pub_len, asn1_write_tag(&c, buf,
			ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 1));
		len += pub_len;

		/* parameters */
		ASN1_CHK_ADD(par_len, x509_write_ec_param(&c, buf, ec));

		ASN1_CHK_ADD(par_len, asn1_write_len(&c, buf, par_len));
		ASN1_CHK_ADD(par_len, asn1_write_tag(&c, buf,
			ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0));
		len += par_len;

		/* privateKey */
		ASN1_CHK_ADD(len, x509_write_ec_private(&c, buf, ec));

		/* version */
		ASN1_CHK_ADD(len, asn1_write_int(&c, buf, 1));

		ASN1_CHK_ADD(len, asn1_write_len(&c, buf, len));
		ASN1_CHK_ADD(len, asn1_write_tag(&c, buf, ASN1_CONSTRUCTED |
			ASN1_SEQUENCE));
	}
	else
	{
		set_last_error(_T("x509_write_key_der"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	return((int)len);
}

#define PEM_BEGIN_PUBLIC_KEY    "-----BEGIN PUBLIC KEY-----\n"
#define PEM_END_PUBLIC_KEY      "-----END PUBLIC KEY-----\n"

#define PEM_BEGIN_PRIVATE_KEY_RSA   "-----BEGIN RSA PRIVATE KEY-----\n"
#define PEM_END_PRIVATE_KEY_RSA     "-----END RSA PRIVATE KEY-----\n"
#define PEM_BEGIN_PRIVATE_KEY_EC    "-----BEGIN EC PRIVATE KEY-----\n"
#define PEM_END_PRIVATE_KEY_EC      "-----END EC PRIVATE KEY-----\n"

/*
* Max sizes of key per types. Shown as tag + len (+ content).
*/

/*
* RSA public keys:
*  SubjectPublicKeyInfo  ::=  SEQUENCE  {          1 + 3
*       algorithm            AlgorithmIdentifier,  1 + 1 (sequence)
*                                                + 1 + 1 + 9 (rsa oid)
*                                                + 1 + 1 (params null)
*       subjectPublicKey     BIT STRING }          1 + 3 + (1 + below)
*  RSAPublicKey ::= SEQUENCE {                     1 + 3
*      modulus           INTEGER,  -- n            1 + 3 + MPI_MAX + 1
*      publicExponent    INTEGER   -- e            1 + 3 + MPI_MAX + 1
*  }
*/
#define RSA_PUB_DER_MAX_BYTES   38 + 2 * MPI_MAX_SIZE

/*
* RSA private keys:
*  RSAPrivateKey ::= SEQUENCE {                    1 + 3
*      version           Version,                  1 + 1 + 1
*      modulus           INTEGER,                  1 + 3 + MPI_MAX + 1
*      publicExponent    INTEGER,                  1 + 3 + MPI_MAX + 1
*      privateExponent   INTEGER,                  1 + 3 + MPI_MAX + 1
*      prime1            INTEGER,                  1 + 3 + MPI_MAX / 2 + 1
*      prime2            INTEGER,                  1 + 3 + MPI_MAX / 2 + 1
*      exponent1         INTEGER,                  1 + 3 + MPI_MAX / 2 + 1
*      exponent2         INTEGER,                  1 + 3 + MPI_MAX / 2 + 1
*      coefficient       INTEGER,                  1 + 3 + MPI_MAX / 2 + 1
*      otherPrimeInfos   OtherPrimeInfos OPTIONAL  0 (not supported)
*  }
*/
#define MPI_MAX_SIZE_2          MPI_MAX_SIZE / 2 + \
                                MPI_MAX_SIZE % 2
#define RSA_PRV_DER_MAX_BYTES   47 + 3 * MPI_MAX_SIZE \
                                   + 5 * MPI_MAX_SIZE_2

/*
* EC public keys:
*  SubjectPublicKeyInfo  ::=  SEQUENCE  {      1 + 2
*    algorithm         AlgorithmIdentifier,    1 + 1 (sequence)
*                                            + 1 + 1 + 7 (ec oid)
*                                            + 1 + 1 + 9 (namedCurve oid)
*    subjectPublicKey  BIT STRING              1 + 2 + 1               [1]
*                                            + 1 (point format)        [1]
*                                            + 2 * ECP_MAX (coords)    [1]
*  }
*/
#define ECP_PUB_DER_MAX_BYTES   30 + 2 * ECP_MAX_BYTES

/*
* EC private keys:
* ECPrivateKey ::= SEQUENCE {                  1 + 2
*      version        INTEGER ,                1 + 1 + 1
*      privateKey     OCTET STRING,            1 + 1 + ECP_MAX
*      parameters [0] ECParameters OPTIONAL,   1 + 1 + (1 + 1 + 9)
*      publicKey  [1] BIT STRING OPTIONAL      1 + 2 + [1] above
*    }
*/
#define ECP_PRV_DER_MAX_BYTES   29 + 3 * ECP_MAX_BYTES

#define PUB_DER_MAX_BYTES   ((RSA_PUB_DER_MAX_BYTES > ECP_PUB_DER_MAX_BYTES) ? RSA_PUB_DER_MAX_BYTES : ECP_PUB_DER_MAX_BYTES)
#define PRV_DER_MAX_BYTES   ((RSA_PRV_DER_MAX_BYTES > ECP_PRV_DER_MAX_BYTES) ? RSA_PRV_DER_MAX_BYTES : ECP_PRV_DER_MAX_BYTES)

int x509_write_pubkey_pem(pk_type_t pktype, void *pk_ctx, byte_t *buf, dword_t size)
{
	int ret;
	byte_t output_buf[PUB_DER_MAX_BYTES];
	dword_t olen = 0;

	if ((ret = x509_write_pubkey_der(pktype, pk_ctx, output_buf,
		sizeof(output_buf))) < 0)
	{
		return(ret);
	}

	if ((ret = pem_write_buffer(PEM_BEGIN_PUBLIC_KEY, PEM_END_PUBLIC_KEY,
		output_buf + sizeof(output_buf) - ret,
		ret, buf, size, &olen)) != 0)
	{
		return(ret);
	}

	return(0);
}

int x509_write_key_pem(pk_type_t pktype, void *pk_ctx, byte_t *buf, dword_t size)
{
	int ret;
	byte_t output_buf[PRV_DER_MAX_BYTES];
	const char *begin, *end;
	dword_t olen = 0;

	if ((ret = x509_write_key_der(pktype, pk_ctx, output_buf, sizeof(output_buf))) < 0)
		return(ret);

	if (pktype == PK_RSA)
	{
		begin = PEM_BEGIN_PRIVATE_KEY_RSA;
		end = PEM_END_PRIVATE_KEY_RSA;
	}
	else if (pktype == PK_ECKEY)
	{
		begin = PEM_BEGIN_PRIVATE_KEY_EC;
		end = PEM_END_PRIVATE_KEY_EC;
	}
	else
	{
		set_last_error(_T("x509_write_key_pem"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	if ((ret = pem_write_buffer(begin, end,
		output_buf + sizeof(output_buf) - ret,
		ret, buf, size, &olen)) != 0)
	{
		return(ret);
	}

	return(0);
}

