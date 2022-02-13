/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc cert document

	@module	x509.c | implement file

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

#include "certctx.h"
#include "certoid.h"

#define X509_RFC5280_MAX_SERIAL_LEN 32
#define X509_RFC5280_UTC_TIME_LEN   15

static int cert_read_mpi(const byte_t* buf, dword_t size, mpi *X)
{
	int n, total = 0;
	byte_t tag;
	dword_t len;

	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_read_mpi"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_INTEGER))
	{
		set_last_error(_T("cert_read_mpi"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	if(mpi_read_binary(X, (buf + total), len) != 0)
	{
		set_last_error(_T("cert_read_mpi"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return C_ERR;
	}
	total += len;

	return total;
}

/*
* Parse a SpecifiedECDomain (SEC 1 C.2) and (mostly) fill the group with it.
* WARNING: the resulting group should only be used with
* pk_group_id_from_specified(), since its base point may not be set correctly
* if it was encoded compressed.
* We only support prime-field as field type, and ignore hash and cofactor.
*/
static int cert_group_from_specified(const byte_t *buf, dword_t size, ecp_group *grp)
{
	int n, total = 0;
	dword_t len;
	byte_t tag;
	int ver;
	byte_t *oid, *oct, *bit;
	dword_t oid_len, oct_len, bit_len, unu_len;

	/* SpecifiedECDomain :: = SEQUENCE{
	*	version SpecifiedECDomainVersion(ecdpVer1 | ecdpVer2 | ecdpVer3, ...),
	*	fieldID FieldID{ { FieldTypes } },
	*	curve Curve,
	*	base ECPoint,
	*	order INTEGER,
	*	cofactor INTEGER OPTIONAL,
	*	hash HashAlgorithm OPTIONAL,
	* }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;
	size = total + len;

	/* SpecifiedECDomainVersion ::= INTEGER { 1, 2, 3 } */
	n = der_read_integer((buf + total), &ver);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;
	
	if (ver < 1 || ver > 3)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_FORMAT"), -1);
		return C_ERR;
	}

	/*
	* FieldID { FIELD-ID:IOSet } ::= SEQUENCE { -- Finite field
	*       fieldType FIELD-ID.&id({IOSet}),
	*       parameters FIELD-ID.&Type({IOSet}{@fieldType})
	* }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	/*
	* FIELD-ID ::= TYPE-IDENTIFIER
	* FieldTypes FIELD-ID ::= {
	*       { Prime-p IDENTIFIED BY prime-field } |
	*       { Characteristic-two IDENTIFIED BY characteristic-two-field }
	* }
	* prime-field OBJECT IDENTIFIER ::= { id-fieldType 1 }
	*/
	n = der_read_oid((buf + total), &oid, &oid_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (xmem_comp(oid, OID_ANSI_X9_62_PRIME_FIELD, oid_len) != 0)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_FORMAT"), -1);
		return C_ERR;
	}

	/* Prime-p ::= INTEGER -- Field of size p. */
	n = cert_read_mpi((buf + total), (size - total), &grp->P);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	grp->pbits = mpi_bitlen(&grp->P);

	/*
	* Curve ::= SEQUENCE {
	*       a FieldElement,
	*       b FieldElement,
	*       seed BIT STRING OPTIONAL
	*       -- Shall be present if used in SpecifiedECDomain
	*       -- with version equal to ecdpVer2 or ecdpVer3
	* }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	/*
	* FieldElement ::= OCTET STRING
	* containing an integer in the case of a prime field
	*/
	n = der_read_octet_string((buf + total), &oct, &oct_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (mpi_read_binary(&grp->A, oct, oct_len) != 0)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_FORMAT"), -1);
		return C_ERR;
	}

	n = der_read_octet_string((buf + total), &oct, &oct_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_FORMAT"), -1);
		return C_ERR;
	}
	total += n;

	if (mpi_read_binary(&grp->B, oct, oct_len) != 0)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_FORMAT"), -1);
		return C_ERR;
	}

	n = der_read_bit_string((buf + total), &bit, &bit_len, &unu_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_FORMAT"), -1);
		return C_ERR;
	}
	total += n;

	/*
	* ECPoint ::= OCTET STRING
	*/
	n = der_read_octet_string((buf + total), &oct, &oct_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (ecp_point_read_binary(grp, &grp->G, oct, oct_len) != 0)
	{
		/*
		* If we can't read the point because it's compressed, cheat by
		* reading only the X coordinate and the parity bit of Y.
		*/
		if ((oct[0] != 0x02 && oct[0] != 0x03) ||
			len != mpi_size(&grp->P) + 1 ||
			mpi_read_binary(&grp->G.X, oct + 1, oct_len - 1) != 0 ||
			mpi_lset(&grp->G.Y, oct[0] - 2) != 0 ||
			mpi_lset(&grp->G.Z, 1) != 0)
		{
			set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_FORMAT"), -1);
			return C_ERR;
		}
	}

	/*
	* order INTEGER
	*/
	n = cert_read_mpi((buf + total), (size - total), &grp->N);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_group_from_specified"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	grp->nbits = mpi_bitlen(&grp->N);

	/*
	* Allow optional elements by purposefully not enforcing p == end here.
	*/

	return total;
}

/*
*  RSAPublicKey ::= SEQUENCE {
*      modulus           INTEGER,  -- n
*      publicExponent    INTEGER   -- e
*  }
*/
static int cert_read_rsa_pubkey(const byte_t* buf, dword_t size, rsa_context *rsa)
{
	int n, total = 0;
	dword_t len;
	byte_t tag;

	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_read_rsa_pubkey"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_read_rsa_pubkey"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	/* Import N */
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_read_rsa_pubkey"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_INTEGER))
	{
		set_last_error(_T("cert_read_rsa_pubkey"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	if (rsa_import_raw(rsa, (buf + total), len, NULL, 0, NULL, 0, NULL, 0, NULL, 0) != 0)
	{
		set_last_error(_T("cert_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return C_ERR;
	}

	total += len;

	/* Import E */
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_read_rsa_pubkey"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_INTEGER))
	{
		set_last_error(_T("cert_read_rsa_pubkey"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	if (rsa_import_raw(rsa, NULL, 0, NULL, 0, NULL, 0, NULL, 0, (buf + total), len) != 0)
	{
		set_last_error(_T("cert_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return(C_ERR);
	}

	total += len;

	if (rsa_complete(rsa) != 0 || rsa_check_pubkey(rsa) != 0)
	{
		set_last_error(_T("cert_get_rsapubkey"), _T("ERR_X509_INVALID_FORMAT"), -1);
		return C_ERR;
	}

	return total;
}

/*
* EC public key is an EC point
*
* The caller is responsible for clearing the structure upon failure if
* desired. Take care to pass along the possible ECP_FEATURE_UNAVAILABLE
* return code of ecp_point_read_binary() and leave p in a usable state.
*/
static int cert_read_ec_pubkey(const byte_t* buf, dword_t size, ecp_keypair *key)
{
	int ret;

	if ((ret = ecp_point_read_binary(&key->grp, &key->Q, buf, size)) == 0)
	{
		ret = ecp_check_pubkey(&key->grp, &key->Q);
	}

	return (!ret)? size : C_ERR;
}

/*
* RSASSA-PSS-params  ::=  SEQUENCE  {
*    hashAlgorithm     [0] HashAlgorithm DEFAULT sha1Identifier,
*    maskGenAlgorithm  [1] MaskGenAlgorithm DEFAULT mgf1SHA1Identifier,
*    saltLength        [2] INTEGER DEFAULT 20,
*    trailerField      [3] INTEGER DEFAULT 1  }
*    -- Note that the tags in this Sequence are explicit.
*
* RFC 4055 (which defines use of RSASSA-PSS in PKIX) states that the value
* of trailerField MUST be 1, and PKCS#1 v2.2 doesn't even define any other
* option. Enfore this at parsing time.
*/
static int cert_read_rsassa_pss_params(const byte_t* buf, dword_t size, md_type_t *hash_md, md_type_t *mgf_md, int* salt_len)
{
	int n, total = 0;
	dword_t len;
	byte_t tag;
	byte_t *oid;
	dword_t oid_len;
	int trailer_field;

	*hash_md = MD_SHA1;
	*mgf_md = MD_SHA1;
	*salt_len = 20;

	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE | 0))
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	/*
	* HashAlgorithm
	*/
	n = der_read_oid((buf + total), &oid, &oid_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if(!md_alg_type_from_oid(oid, oid_len, hash_md))
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_SIG_ALG"), -1);
		return C_ERR;
	}

	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE | 1))
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	/*
	* maskGenAlgorithm
	*/
	n = der_read_oid((buf + total), &oid, &oid_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (!md_alg_type_from_oid(oid, oid_len, mgf_md))
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_SIG_ALG"), -1);
		return C_ERR;
	}

	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE | 2))
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	/*
	* saltLength
	*/
	n = der_read_integer((buf + total), salt_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE | 3))
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	/*
	* trailerField
	*/
	n = der_read_integer((buf + total), &trailer_field);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (trailer_field != 1)
	{
		set_last_error(_T("cert_read_rsassa_pss_params"), _T("ERR_X509_INVALID_PARAM"), -1);
		return C_ERR;
	}

	return total;
}

/*
* Parse and fill a single X.509 certificate in DER format
*/
int cert_read_der(cert_reader *cr, void* pa, const byte_t *buf, dword_t size)
{
	byte_t tag;
	dword_t set_size, seq_size, ext_size, len, total = 0;
	int n;

	int ver;
	byte_t *oid, *bit, *oct;
	dword_t oid_len, bit_len, unu_len, oct_len;
	md_type_t md_alg;
	pk_type_t pk_alg;
	md_type_t hash_md, mgf_md;
	int salt_len;
	xdate_t dt_before, dt_after;
	int ext_type;
	bool_t criti;
	bool_t is_cacert;
	int max_pathlen;

	ecp_group_id grp_id;
	ecp_group grp;
	rsa_context rsa;
	ecp_keypair ecp;

	XDK_ASSERT(cr != NULL);
	
	if (buf == NULL || !size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* Certificate  ::=  SEQUENCE  {
	*      tbsCertificate       TBSCertificate,
	*      signatureAlgorithm   AlgorithmIdentifier,
	*      signatureValue       BIT STRING  }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;
	size = total + len;

	if (cr->read_cert_begin && !(*cr->read_cert_begin)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	* TBSCertificate  ::=  SEQUENCE  {
	* Version  [0]  EXPLICIT Version DEFAULT v1
	* serialNumber	CertificateSerialNumber
	* signature            AlgorithmIdentifier
	* issuer               Name
	* validity             Validity,
	* subject              Name,
	* subjectPublicKeyInfo SubjectPublicKeyInfo,
	* issuerUniqueID  [1]  IMPLICIT UniqueIdentifier OPTIONAL, -- If present, version MUST be v2 or v3
	* subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL, -- If present, version MUST be v2 or v3
	* extensions      [3]  EXPLICIT Extensions OPTIONAL -- If present, version MUST be v3
	}
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	if (cr->read_tbs_begin && !(*cr->read_tbs_begin)(pa, (buf + total - n), (len + n)))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	* Version [0]
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 0))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	/*
	* Version ::=  INTEGER  {  v1(0), v2(1), v3(2)  }
	*/
	n = der_read_integer((buf + total), &ver);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (cr->read_tbs_version && !(*cr->read_tbs_version)(pa, ver))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	* CertificateSerialNumber ::=  INTEGER
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONTEXT_SPECIFIC | ASN1_PRIMITIVE | 2) && tag != ASN1_INTEGER)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_TAG"), -1);
		return total;
	}
	total += n;

	if (cr->read_tbs_serial_number && !(*cr->read_tbs_serial_number)(pa, (buf + total), len))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}
	total += len;

	/*
	* signature ::=  SEQUENCE  {
    *    algorithm               OBJECT IDENTIFIER,
    *    parameters              ANY DEFINED BY algorithm OPTIONAL  
	* }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;
	seq_size = len;

	if (cr->read_tbs_signature_algorithm_begin && !(*cr->read_tbs_signature_algorithm_begin)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	n = der_read_oid((buf + total), &oid, &oid_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;
	seq_size -= n;

	if (!sig_alg_type_from_oid(oid, oid_len, &md_alg, &pk_alg))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_SIG_ALG"), -1);
		return C_ERR;
	}

	if (cr->read_tbs_signature_algorithm_identifier && !(*cr->read_tbs_signature_algorithm_identifier)(pa, oid, oid_len))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	* RSASSA-PSS-params
	*/
	if (pk_alg == PK_RSASSA_PSS)
	{
		n = cert_read_rsassa_pss_params((buf + total), seq_size, &hash_md, &mgf_md, &salt_len);
		if (n < 0 || total + n > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}

		if (cr->read_tbs_signature_algorithm_parameters && !(*cr->read_tbs_signature_algorithm_parameters)(pa, (buf + total), n))
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
			return C_OK;
		}

		total += n;
	}
	else
	{
		n = der_read_null((buf + total));
		if (n < 0 || total + n > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}
		total += n;
	}

	if (cr->read_tbs_signature_algorithm_end && !(*cr->read_tbs_signature_algorithm_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	if (ver < 0 || ver > 2)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_UNKNOWN_VERSION"), -1);
		return(C_ERR);
	}
	ver++;

	/*
	* issuer ::= CHOICE { -- only one possibility for now --
    *	rdnSequence  RDNSequence 
	* }
	
	* RDNSequence ::= SEQUENCE OF RelativeDistinguishedName

	* RelativeDistinguishedName ::= SET SIZE (1..MAX) OF AttributeTypeAndValue

	* AttributeTypeAndValue ::= SEQUENCE {
	* type     AttributeType,
	* value    AttributeValue 
	* }

	* AttributeType ::= OBJECT IDENTIFIER
	* AttributeValue ::= ANY -- DEFINED BY AttributeType

	* DirectoryString ::= CHOICE {
	* teletexString           TeletexString (SIZE (1..MAX)),
	* printableString         PrintableString (SIZE (1..MAX)),
	* universalString         UniversalString (SIZE (1..MAX)),
	* utf8String              UTF8String (SIZE (1..MAX)),
	* bmpString               BMPString (SIZE (1..MAX)) 
	* }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	if (cr->read_tbs_issuer_begin && !(*cr->read_tbs_issuer_begin)(pa, (buf + total - n), (len + n)))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	seq_size = len;
	while (seq_size)
	{
		n = der_read_tag((buf + total), &tag, &len);
		if (n < 0 || total + n + len > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}
		if (tag != (ASN1_CONSTRUCTED | ASN1_SET))
		{
			break;
		}
		total += n;
		seq_size -= n;

		if (cr->read_tbs_issuer_name_begin && !(*cr->read_tbs_issuer_name_begin)(pa))
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
			return C_OK;
		}

		set_size = len;
		while (set_size)
		{
			n = der_read_tag((buf + total), &tag, &len);
			if (n < 0 || total + n + len > size)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
				return C_ERR;
			}
			if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
			{
				break;
			}
			total += n;
			seq_size -= n;
			set_size -= n;

			if (cr->read_tbs_issuer_attribute_begin && !(*cr->read_tbs_issuer_attribute_begin)(pa))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}

			n = der_read_oid((buf + total), &oid, &oid_len);
			if (n < 0 || total + n > size)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
				return C_ERR;
			}
			total += n;
			seq_size -= n;
			set_size -= n;

			if (cr->read_tbs_issuer_attribute_type && !(*cr->read_tbs_issuer_attribute_type)(pa, oid, oid_len))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}

			n = der_read_tag((buf + total), &tag, &len);
			if (n < 0 || total + n + len > size)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
				return C_ERR;
			}
			total += n;
			seq_size -= n;
			set_size -= n;

			if (tag != ASN1_BMP_STRING && tag != ASN1_UTF8_STRING      &&
				tag != ASN1_T61_STRING && tag != ASN1_PRINTABLE_STRING &&
				tag != ASN1_IA5_STRING && tag != ASN1_UNIVERSAL_STRING &&
				tag != ASN1_BIT_STRING)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
				return C_ERR;
			}

			if (cr->read_tbs_issuer_attribute_value && !(cr->read_tbs_issuer_attribute_value)(pa, (buf + total), len))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}

			total += len;
			seq_size -= len;
			set_size -= len;

			if (cr->read_tbs_issuer_attribute_end && !(*cr->read_tbs_issuer_attribute_end)(pa))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}
		}

		if (cr->read_tbs_issuer_name_end && !(*cr->read_tbs_issuer_name_end)(pa))
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
			return C_OK;
		}
	}

	if (cr->read_tbs_issuer_end && !(*cr->read_tbs_issuer_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	* Validity ::= SEQUENCE {
    *    notBefore      Time,
    *    notAfter       Time 
	* }

	* Time ::= CHOICE {
		utcTime        UTCTime,
		generalTime    GeneralizedTime 
	* }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	if (cr->read_tbs_validity_begin && !(*cr->read_tbs_validity_begin)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	n = der_read_time((buf + total), &dt_before);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (cr->read_tbs_validity_notbefore && !(*cr->read_tbs_validity_notbefore)(pa, &dt_before))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	n = der_read_time((buf + total), &dt_after);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (cr->read_tbs_validity_notafter && !(*cr->read_tbs_validity_notafter)(pa, &dt_after))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	if (cr->read_tbs_validity_end && !(*cr->read_tbs_validity_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	if (total == size)
	{
		if (cr->read_tbs_end) (*cr->read_tbs_end)(pa);

		if (cr->read_cert_end) (*cr->read_cert_end)(pa);

		return total;
	}

	/*
	* subject	Name
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	if (cr->read_tbs_subject_begin && !(*cr->read_tbs_subject_begin)(pa, (buf + total - n), (len + n)))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	seq_size = len;
	while (seq_size)
	{
		n = der_read_tag((buf + total), &tag, &len);
		if (n < 0 || total + n + len > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}
		if (tag != (ASN1_CONSTRUCTED | ASN1_SET))
		{
			break;
		}
		total += n;
		seq_size -= n;

		if (cr->read_tbs_subject_name_begin && !(*cr->read_tbs_subject_name_begin)(pa))
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
			return C_OK;
		}

		set_size = len;
		while (set_size)
		{
			n = der_read_tag((buf + total), &tag, &len);
			if (n < 0 || total + n + len > size)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
				return C_ERR;
			}
			if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
			{
				break;
			}
			total += n;
			seq_size -= n;
			set_size -= n;

			n = der_read_oid((buf + total), &oid, &oid_len);
			if (n < 0 || total + n > size)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
				return C_ERR;
			}
			total += n;
			seq_size -= n;
			set_size -= n;

			if (cr->read_tbs_subject_attribute_type && !(*cr->read_tbs_subject_attribute_type)(pa, oid, oid_len))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}

			n = der_read_tag((buf + total), &tag, &len);
			if (n < 0 || total + n + len > size)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
				return C_ERR;
			}
			total += n;
			seq_size -= n;
			set_size -= n;

			if (tag != ASN1_BMP_STRING && tag != ASN1_UTF8_STRING      &&
				tag != ASN1_T61_STRING && tag != ASN1_PRINTABLE_STRING &&
				tag != ASN1_IA5_STRING && tag != ASN1_UNIVERSAL_STRING &&
				tag != ASN1_BIT_STRING)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
				return C_ERR;
			}

			if (cr->read_tbs_subject_attribute_value && !(cr->read_tbs_subject_attribute_value)(pa, (buf + total), len))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}

			total += len;
			seq_size -= len;
			set_size -= len;
		}

		if (cr->read_tbs_subject_name_end && !(*cr->read_tbs_subject_name_end)(pa))
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
			return C_OK;
		}
	}

	if (cr->read_tbs_subject_end && !(*cr->read_tbs_subject_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	if (total == size)
	{
		if (cr->read_tbs_end) (*cr->read_tbs_end)(pa);

		if (cr->read_cert_end) (*cr->read_cert_end)(pa);

		return total;
	}

	/*
	* SubjectPublicKeyInfo ::=  SEQUENCE  {
    *    algorithm            AlgorithmIdentifier,
    *    subjectPublicKey     BIT STRING  
	* }

	* AlgorithmIdentifier ::=  SEQUENCE  {
    *    algorithm            OBJECT IDENTIFIER,
    *    parameters           ANY DEFINED BY algorithm OPTIONAL  
	* }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	if (cr->read_tbs_subject_publickey_info_begin && !(*cr->read_tbs_subject_publickey_info_begin)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	* algorithm
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	if (cr->read_tbs_subject_publickey_algorithm_begin && !(*cr->read_tbs_subject_publickey_algorithm_begin)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	n = der_read_oid((buf + total), &oid, &oid_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (cr->read_tbs_subject_publickey_algorithm_identifier && !(*cr->read_tbs_subject_publickey_algorithm_identifier)(pa, oid, oid_len))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	if (!pk_alg_type_from_oid(oid, oid_len, &pk_alg))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_SIG_ALG"), -1);
		return C_ERR;
	}

	if (pk_alg == PK_ECKEY || pk_alg == PK_ECKEY_DH)
	{
		/*
		* ECParameters ::= CHOICE {
		*   namedCurve         OBJECT IDENTIFIER
		*   specifiedCurve     SpecifiedECDomain -- = SEQUENCE { ... }  -- implicitCurve   NULL
		*/
		oid = NULL;
		oid_len = 0;
		n = der_read_oid((buf + total), &oid, &oid_len);
		if (n < 0 || total + n > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}
		total += n;

		if (cr->read_tbs_subject_publickey_algorithm_parameters && !(*cr->read_tbs_subject_publickey_algorithm_parameters)(pa, oid, oid_len))
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
			return C_OK;
		}

		grp_id = 0;
		if (!ecp_grp_type_from_oid(oid, oid_len, &grp_id))
		{
			ecp_group_init(&grp);

			n = cert_group_from_specified((buf + total), (size - total), &grp);
			if (n < 0 || total + n > size)
			{
				ecp_group_free(&grp);

				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
				return C_ERR;
			}
			total += n;

			ecp_group_id_from_group(&grp, &grp_id);
			ecp_group_free(&grp);
		}

		if (!grp_id)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_FORMAT"), -1);
			return C_ERR;
		}
	}
	else
	{
		n = der_read_null((buf + total));
		if (n < 0 || total + n > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}
		total += n;
	}

	if (cr->read_tbs_subject_publickey_algorithm_end && !(*cr->read_tbs_subject_publickey_algorithm_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	* subjectPublicKey
	*/
	n = der_read_bit_string((buf + total), &bit, &bit_len, &unu_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (cr->read_tbs_subject_publickey && !(*cr->read_tbs_subject_publickey)(pa, bit, bit_len))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	if (pk_alg == PK_RSA)
	{
		rsa_init((rsa_context*)(&rsa), 0, 0);

		if(cert_read_rsa_pubkey(bit, bit_len, &rsa) < 0)
		{
			rsa_free(&rsa);

			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}

		rsa_free(&rsa);
	}
	else if (pk_alg == PK_ECKEY_DH || pk_alg == PK_ECKEY)
	{
		ecp_keypair_init((ecp_keypair*)(&ecp));

		if (ecp_group_load(&(ecp.grp), grp_id) != 0)
		{
			ecp_keypair_free(&ecp);

			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}

		if(cert_read_ec_pubkey(bit, bit_len, &ecp) < 0)
		{
			ecp_keypair_free(&ecp);

			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}

		ecp_keypair_free(&ecp);
	}

	if (cr->read_tbs_subject_publickey_info_end && !(*cr->read_tbs_subject_publickey_info_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	*  issuerUniqueID  [1]  IMPLICIT UniqueIdentifier OPTIONAL,
	*                       -- If present, version shall be v2 or v3
	*  subjectUniqueID [2]  IMPLICIT UniqueIdentifier OPTIONAL,
	*                       -- If present, version shall be v2 or v3
	*  extensions      [3]  EXPLICIT Extensions OPTIONAL
	*                       -- If present, version shall be v3
	*/
	while (total < size && (ver == 2 || ver == 3))
	{
		n = der_read_tag((buf + total), &tag, &len);
		if (n < 0 || total + n + len > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}
		total += n;
		/*
		* issuerUniqueID [1]
		*/
		if (tag == (ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 1))
		{
			if (cr->read_tbs_issuer_uuid && !(cr->read_tbs_issuer_uuid)(pa, (buf + total), len))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}
			total += len;
		}
		/*
		* subjectUniqueID [2]
		*/
		else if (tag == (ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 2))
		{
			if (cr->read_tbs_subject_uuid && !(cr->read_tbs_subject_uuid)(pa, (buf + total), len))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}
			total += len;
		}
		/*
		* extensions [3]
		*/
		else if (tag == (ASN1_CONTEXT_SPECIFIC | ASN1_CONSTRUCTED | 3))
		{
			/* Extension structure use EXPLICIT tagging. That is, the actual
			* `Extensions` structure is wrapped by a tag-length pair using
			* the respective context-specific tag.
			*
			* Extensions  ::=  SEQUENCE SIZE (1..MAX) OF Extension
			*
			* Extension  ::=  SEQUENCE  {
			*      extnID      OBJECT IDENTIFIER,
			*      critical    BOOLEAN DEFAULT FALSE OPTIONAL,
			*      extnValue   OCTET STRING  }
			*/

			/*
			* Extensions
			*/
			n = der_read_tag((buf + total), &tag, &len);
			if (n < 0 || total + n + len > size)
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
				return C_ERR;
			}
			if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
			{
				break;
			}
			total += n;

			if (cr->read_tbs_extensions_begin && !(cr->read_tbs_extensions_begin)(pa))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}

			is_cacert = 0;
			max_pathlen = 0;

			ext_size = len;
			while (ext_size)
			{
				/*
				* Extension
				*/
				n = der_read_tag((buf + total), &tag, &len);
				if (n < 0 || total + n + len > size)
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
					return C_ERR;
				}
				if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
				{
					break;
				}
				total += n;
				ext_size -= n;

				if (cr->read_tbs_extensions_extn_begin && !(cr->read_tbs_extensions_extn_begin)(pa))
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
					return C_OK;
				}

				n = der_read_oid((buf + total), &oid, &oid_len);
				if (n < 0 || total + n > size)
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
					return C_ERR;
				}
				total += n;
				ext_size -= n;

				if (cr->read_tbs_extensions_extn_id && !(cr->read_tbs_extensions_extn_id)(pa, oid, oid_len))
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
					return C_OK;
				}

				criti = 0;
				n = der_read_bool((buf + total), &criti);
				if (n < 0 || total + n > size)
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
					return C_ERR;
				}
				total += n;
				ext_size -= n;

				if (cr->read_tbs_extensions_critical && !(cr->read_tbs_extensions_critical)(pa, criti))
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
					return C_OK;
				}

				oct = NULL;
				oct_len = 0;
				n = der_read_octet_string((buf + total), &oct, &oct_len);
				if (n < 0 || total + n > size)
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
					return C_ERR;
				}
				total += n;
				ext_size -= n;

				if (cr->read_tbs_extensions_extn_value && !(cr->read_tbs_extensions_extn_value)(pa, oct, oct_len))
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
					return C_OK;
				}

				ext_type = 0;
				if (!x509_ext_type_from_oid(oid, oid_len, &ext_type) && criti)
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_SIG_ALG"), -1);
					return C_ERR;
				}

				switch (ext_type)
				{
				case X509_EXT_BASIC_CONSTRAINTS:
					/*
					* BasicConstraints ::= SEQUENCE {
					*      cA                      BOOLEAN DEFAULT FALSE,
					*      pathLenConstraint       INTEGER (0..MAX) OPTIONAL }
					*/
					n = der_read_tag(oct, &tag, &len);
					if (n < 0 || n + len > oct_len)
					{
						set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
						return C_ERR;
					}
					if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
					{
						break;
					}
					oct_len -= n;

					// CA
					n = der_read_bool(oct + n, &is_cacert);
					if (n < 0 || n > oct_len)
					{
						set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_DATA"), -1);
						return C_ERR;
					}
					oct_len -= n;
					if (oct_len)
					{
						//pathLenConstraint
						n = der_read_integer(oct + n, &max_pathlen);
						if (n < 0 || n > oct_len)
						{
							set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_DATA"), -1);
							return C_ERR;
						}
						oct_len -= n;
					}

					break;
				case X509_EXT_KEY_USAGE:
					break;
				case X509_EXT_EXTENDED_KEY_USAGE:
					break;
				case X509_EXT_SUBJECT_ALT_NAME:
					break;
				case X509_EXT_NS_CERT_TYPE:
					break;
				default:
					break;
				}

				if (cr->read_tbs_extensions_extn_end && !(cr->read_tbs_extensions_extn_end)(pa))
				{
					set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
					return C_OK;
				}
			}

			if (cr->read_tbs_extensions_end && !(cr->read_tbs_extensions_end)(pa))
			{
				set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
				return C_OK;
			}

			break;
		}
	}

	if (cr->read_tbs_end && !(*cr->read_tbs_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}
	
	/*
	*  }
	*  -- end of TBSCertificate
	*
	*  signatureAlgorithm   AlgorithmIdentifier,
	*  signatureValue       BIT STRING
	*/

	/*
	* signatureAlgorithm ::=  SEQUENCE  {
	*    algorithm               OBJECT IDENTIFIER,
	*    parameters              ANY DEFINED BY algorithm OPTIONAL
	* }
	*/
	n = der_read_tag((buf + total), &tag, &len);
	if (n < 0 || total + n + len > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
		return total;
	}
	total += n;

	if (cr->read_cert_signature_algorithm_begin && !(*cr->read_cert_signature_algorithm_begin)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	n = der_read_oid((buf + total), &oid, &oid_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (cr->read_cert_signature_algorithm_identifier && !(*cr->read_cert_signature_algorithm_identifier)(pa, oid, oid_len))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	if (!sig_alg_type_from_oid(oid, oid_len, &md_alg, &pk_alg))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_SIG_ALG"), -1);
		return C_ERR;
	}

	/*
	* RSASSA-PSS-params 
	*/
	if (pk_alg == PK_RSASSA_PSS)
	{
		n = der_read_tag((buf + total), &tag, &len);
		if (n < 0 || total + n + len > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}
		if (tag != (ASN1_CONSTRUCTED | ASN1_SEQUENCE | 0))
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_ALG_TAG"), -1);
			return total;
		}

		if (cr->read_cert_signature_algorithm_parameters && !(*cr->read_cert_signature_algorithm_parameters)(pa, (buf + total), (n + len)))
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
			return C_OK;
		}

		total += (n + len);
	}
	else
	{
		n = der_read_null((buf + total));
		if (n < 0 || total + n > size)
		{
			set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
			return C_ERR;
		}
		total += n;
	}

	if (cr->read_cert_signature_algorithm_end && !(*cr->read_cert_signature_algorithm_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	/*
	* signatureValue       BIT STRING
	*/
	n = der_read_bit_string((buf + total), &bit, &bit_len, &unu_len);
	if (n < 0 || total + n > size)
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_INVALID_BUFFER_LENGTH"), -1);
		return C_ERR;
	}
	total += n;

	if (cr->read_cert_signature && !(*cr->read_cert_signature)(pa, bit, bit_len))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	if (cr->read_cert_end && !(*cr->read_cert_end)(pa))
	{
		set_last_error(_T("cert_crt_parse_der"), _T("ERR_X509_READER_BREAK"), -1);
		return C_OK;
	}

	return C_OK;
}



