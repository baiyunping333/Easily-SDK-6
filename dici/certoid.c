/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc cert document

	@module	certoid.c | implement file

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

#include "certoid.h"



/*
* Top level OID tuples
*/
#define OID_ISO_MEMBER_BODIES           "\x2a"          /* {iso(1) member-body(2)} */
#define OID_ISO_IDENTIFIED_ORG          "\x2b"          /* {iso(1) identified-organization(3)} */
#define OID_ISO_CCITT_DS                "\x55"          /* {joint-iso-ccitt(2) ds(5)} */
#define OID_ISO_ITU_COUNTRY             "\x60"          /* {joint-iso-itu-t(2) country(16)} */

/*
* ISO Member bodies OID parts
*/
#define OID_COUNTRY_US                  "\x86\x48"      /* {us(840)} */
#define OID_ORG_RSA_DATA_SECURITY       "\x86\xf7\x0d"  /* {rsadsi(113549)} */
#define OID_RSA_COMPANY                 OID_ISO_MEMBER_BODIES OID_COUNTRY_US \
                                        OID_ORG_RSA_DATA_SECURITY /* {iso(1) member-body(2) us(840) rsadsi(113549)} */
#define OID_ORG_ANSI_X9_62              "\xce\x3d" /* ansi-X9-62(10045) */
#define OID_ANSI_X9_62                  OID_ISO_MEMBER_BODIES OID_COUNTRY_US \
                                        OID_ORG_ANSI_X9_62

/*
* ISO Identified organization OID parts
*/
#define OID_ORG_DOD                     "\x06"          /* {dod(6)} */
#define OID_ORG_OIW                     "\x0e"
#define OID_OIW_SECSIG                  OID_ORG_OIW "\x03"
#define OID_OIW_SECSIG_ALG              OID_OIW_SECSIG "\x02"
#define OID_OIW_SECSIG_SHA1             OID_OIW_SECSIG_ALG "\x1a"
#define OID_ORG_CERTICOM                "\x81\x04"  /* certicom(132) */
#define OID_CERTICOM                    OID_ISO_IDENTIFIED_ORG OID_ORG_CERTICOM
#define OID_ORG_TELETRUST               "\x24" /* teletrust(36) */
#define OID_TELETRUST                   OID_ISO_IDENTIFIED_ORG OID_ORG_TELETRUST

/*
* ISO ITU OID parts
*/
#define OID_ORGANIZATION                "\x01"          /* {organization(1)} */
#define OID_ISO_ITU_US_ORG              OID_ISO_ITU_COUNTRY OID_COUNTRY_US OID_ORGANIZATION /* {joint-iso-itu-t(2) country(16) us(840) organization(1)} */

#define OID_ORG_GOV                     "\x65"          /* {gov(101)} */
#define OID_GOV                         OID_ISO_ITU_US_ORG OID_ORG_GOV /* {joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101)} */

#define OID_ORG_NETSCAPE                "\x86\xF8\x42"  /* {netscape(113730)} */
#define OID_NETSCAPE                    OID_ISO_ITU_US_ORG OID_ORG_NETSCAPE /* Netscape OID {joint-iso-itu-t(2) country(16) us(840) organization(1) netscape(113730)} */

/* ISO arc for standard certificate and CRL extensions */
#define OID_ID_CE                       OID_ISO_CCITT_DS "\x1D" /**< id-ce OBJECT IDENTIFIER  ::=  {joint-iso-ccitt(2) ds(5) 29} */

#define OID_NIST_ALG                    OID_GOV "\x03\x04" /** { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistAlgorithm(4) */

/**
* Private Internet Extensions
* { iso(1) identified-organization(3) dod(6) internet(1)
*                      security(5) mechanisms(5) pkix(7) }
*/
#define OID_PKIX                        OID_ISO_IDENTIFIED_ORG OID_ORG_DOD "\x01\x05\x05\x07"

/*
* Arc for standard naming attributes
*/
#define OID_AT                          OID_ISO_CCITT_DS "\x04" /**< id-at OBJECT IDENTIFIER ::= {joint-iso-ccitt(2) ds(5) 4} */
#define OID_AT_CN                       OID_AT "\x03" /**< id-at-commonName AttributeType:= {id-at 3} */
#define OID_AT_SUR_NAME                 OID_AT "\x04" /**< id-at-surName AttributeType:= {id-at 4} */
#define OID_AT_SERIAL_NUMBER            OID_AT "\x05" /**< id-at-serialNumber AttributeType:= {id-at 5} */
#define OID_AT_COUNTRY                  OID_AT "\x06" /**< id-at-countryName AttributeType:= {id-at 6} */
#define OID_AT_LOCALITY                 OID_AT "\x07" /**< id-at-locality AttributeType:= {id-at 7} */
#define OID_AT_STATE                    OID_AT "\x08" /**< id-at-state AttributeType:= {id-at 8} */
#define OID_AT_ORGANIZATION             OID_AT "\x0A" /**< id-at-organizationName AttributeType:= {id-at 10} */
#define OID_AT_ORG_UNIT                 OID_AT "\x0B" /**< id-at-organizationalUnitName AttributeType:= {id-at 11} */
#define OID_AT_TITLE                    OID_AT "\x0C" /**< id-at-title AttributeType:= {id-at 12} */
#define OID_AT_POSTAL_ADDRESS           OID_AT "\x10" /**< id-at-postalAddress AttributeType:= {id-at 16} */
#define OID_AT_POSTAL_CODE              OID_AT "\x11" /**< id-at-postalCode AttributeType:= {id-at 17} */
#define OID_AT_GIVEN_NAME               OID_AT "\x2A" /**< id-at-givenName AttributeType:= {id-at 42} */
#define OID_AT_INITIALS                 OID_AT "\x2B" /**< id-at-initials AttributeType:= {id-at 43} */
#define OID_AT_GENERATION_QUALIFIER     OID_AT "\x2C" /**< id-at-generationQualifier AttributeType:= {id-at 44} */
#define OID_AT_UNIQUE_IDENTIFIER        OID_AT "\x2D" /**< id-at-uniqueIdentifier AttributType:= {id-at 45} */
#define OID_AT_DN_QUALIFIER             OID_AT "\x2E" /**< id-at-dnQualifier AttributeType:= {id-at 46} */
#define OID_AT_PSEUDONYM                OID_AT "\x41" /**< id-at-pseudonym AttributeType:= {id-at 65} */

#define OID_DOMAIN_COMPONENT            "\x09\x92\x26\x89\x93\xF2\x2C\x64\x01\x19" /** id-domainComponent AttributeType:= {itu-t(0) data(9) pss(2342) ucl(19200300) pilot(100) pilotAttributeType(1) domainComponent(25)} */

/*
* OIDs for standard certificate extensions
*/
#define OID_AUTHORITY_KEY_IDENTIFIER    OID_ID_CE "\x23" /**< id-ce-authorityKeyIdentifier OBJECT IDENTIFIER ::=  { id-ce 35 } */
#define OID_SUBJECT_KEY_IDENTIFIER      OID_ID_CE "\x0E" /**< id-ce-subjectKeyIdentifier OBJECT IDENTIFIER ::=  { id-ce 14 } */
#define OID_KEY_USAGE                   OID_ID_CE "\x0F" /**< id-ce-keyUsage OBJECT IDENTIFIER ::=  { id-ce 15 } */
#define OID_CERTIFICATE_POLICIES        OID_ID_CE "\x20" /**< id-ce-certificatePolicies OBJECT IDENTIFIER ::=  { id-ce 32 } */
#define OID_POLICY_MAPPINGS             OID_ID_CE "\x21" /**< id-ce-policyMappings OBJECT IDENTIFIER ::=  { id-ce 33 } */
#define OID_SUBJECT_ALT_NAME            OID_ID_CE "\x11" /**< id-ce-subjectAltName OBJECT IDENTIFIER ::=  { id-ce 17 } */
#define OID_ISSUER_ALT_NAME             OID_ID_CE "\x12" /**< id-ce-issuerAltName OBJECT IDENTIFIER ::=  { id-ce 18 } */
#define OID_SUBJECT_DIRECTORY_ATTRS     OID_ID_CE "\x09" /**< id-ce-subjectDirectoryAttributes OBJECT IDENTIFIER ::=  { id-ce 9 } */
#define OID_BASIC_CONSTRAINTS           OID_ID_CE "\x13" /**< id-ce-basicConstraints OBJECT IDENTIFIER ::=  { id-ce 19 } */
#define OID_NAME_CONSTRAINTS            OID_ID_CE "\x1E" /**< id-ce-nameConstraints OBJECT IDENTIFIER ::=  { id-ce 30 } */
#define OID_POLICY_CONSTRAINTS          OID_ID_CE "\x24" /**< id-ce-policyConstraints OBJECT IDENTIFIER ::=  { id-ce 36 } */
#define OID_EXTENDED_KEY_USAGE          OID_ID_CE "\x25" /**< id-ce-extKeyUsage OBJECT IDENTIFIER ::= { id-ce 37 } */
#define OID_CRL_DISTRIBUTION_POINTS     OID_ID_CE "\x1F" /**< id-ce-cRLDistributionPoints OBJECT IDENTIFIER ::=  { id-ce 31 } */
#define OID_INIHIBIT_ANYPOLICY          OID_ID_CE "\x36" /**< id-ce-inhibitAnyPolicy OBJECT IDENTIFIER ::=  { id-ce 54 } */
#define OID_FRESHEST_CRL                OID_ID_CE "\x2E" /**< id-ce-freshestCRL OBJECT IDENTIFIER ::=  { id-ce 46 } */

/*
* Netscape certificate extensions
*/
#define OID_NS_CERT                 OID_NETSCAPE "\x01"
#define OID_NS_CERT_TYPE            OID_NS_CERT  "\x01"
#define OID_NS_BASE_URL             OID_NS_CERT  "\x02"
#define OID_NS_REVOCATION_URL       OID_NS_CERT  "\x03"
#define OID_NS_CA_REVOCATION_URL    OID_NS_CERT  "\x04"
#define OID_NS_RENEWAL_URL          OID_NS_CERT  "\x07"
#define OID_NS_CA_POLICY_URL        OID_NS_CERT  "\x08"
#define OID_NS_SSL_SERVER_NAME      OID_NS_CERT  "\x0C"
#define OID_NS_COMMENT              OID_NS_CERT  "\x0D"
#define OID_NS_DATA_TYPE            OID_NETSCAPE "\x02"
#define OID_NS_CERT_SEQUENCE        OID_NS_DATA_TYPE "\x05"

/*
* OIDs for CRL extensions
*/
#define OID_PRIVATE_KEY_USAGE_PERIOD    OID_ID_CE "\x10"
#define OID_CRL_NUMBER                  OID_ID_CE "\x14" /**< id-ce-cRLNumber OBJECT IDENTIFIER ::= { id-ce 20 } */

/*
* X.509 v3 Extended key usage OIDs
*/
#define OID_ANY_EXTENDED_KEY_USAGE      OID_EXTENDED_KEY_USAGE "\x00" /**< anyExtendedKeyUsage OBJECT IDENTIFIER ::= { id-ce-extKeyUsage 0 } */

#define OID_KP                          OID_PKIX "\x03" /**< id-kp OBJECT IDENTIFIER ::= { id-pkix 3 } */
#define OID_SERVER_AUTH                 OID_KP "\x01" /**< id-kp-serverAuth OBJECT IDENTIFIER ::= { id-kp 1 } */
#define OID_CLIENT_AUTH                 OID_KP "\x02" /**< id-kp-clientAuth OBJECT IDENTIFIER ::= { id-kp 2 } */
#define OID_CODE_SIGNING                OID_KP "\x03" /**< id-kp-codeSigning OBJECT IDENTIFIER ::= { id-kp 3 } */
#define OID_EMAIL_PROTECTION            OID_KP "\x04" /**< id-kp-emailProtection OBJECT IDENTIFIER ::= { id-kp 4 } */
#define OID_TIME_STAMPING               OID_KP "\x08" /**< id-kp-timeStamping OBJECT IDENTIFIER ::= { id-kp 8 } */
#define OID_OCSP_SIGNING                OID_KP "\x09" /**< id-kp-OCSPSigning OBJECT IDENTIFIER ::= { id-kp 9 } */

/*
* PKCS definition OIDs
*/

#define OID_PKCS                OID_RSA_COMPANY "\x01" /**< pkcs OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) 1 } */
#define OID_PKCS1               OID_PKCS "\x01" /**< pkcs-1 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) 1 } */
#define OID_PKCS5               OID_PKCS "\x05" /**< pkcs-5 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) 5 } */
#define OID_PKCS9               OID_PKCS "\x09" /**< pkcs-9 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) 9 } */
#define OID_PKCS12              OID_PKCS "\x0c" /**< pkcs-12 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) pkcs(1) 12 } */

/*
* PKCS#1 OIDs
*/
#define OID_PKCS1_RSA           OID_PKCS1 "\x01" /**< rsaEncryption OBJECT IDENTIFIER ::= { pkcs-1 1 } */
#define OID_PKCS1_MD2           OID_PKCS1 "\x02" /**< md2WithRSAEncryption ::= { pkcs-1 2 } */
#define OID_PKCS1_MD4           OID_PKCS1 "\x03" /**< md4WithRSAEncryption ::= { pkcs-1 3 } */
#define OID_PKCS1_MD5           OID_PKCS1 "\x04" /**< md5WithRSAEncryption ::= { pkcs-1 4 } */
#define OID_PKCS1_SHA1          OID_PKCS1 "\x05" /**< sha1WithRSAEncryption ::= { pkcs-1 5 } */
#define OID_PKCS1_SHA224        OID_PKCS1 "\x0e" /**< sha224WithRSAEncryption ::= { pkcs-1 14 } */
#define OID_PKCS1_SHA256        OID_PKCS1 "\x0b" /**< sha256WithRSAEncryption ::= { pkcs-1 11 } */
#define OID_PKCS1_SHA384        OID_PKCS1 "\x0c" /**< sha384WithRSAEncryption ::= { pkcs-1 12 } */
#define OID_PKCS1_SHA512        OID_PKCS1 "\x0d" /**< sha512WithRSAEncryption ::= { pkcs-1 13 } */

#define OID_RSA_SHA_OBS         "\x2B\x0E\x03\x02\x1D"

#define OID_PKCS9_EMAIL         OID_PKCS9 "\x01" /**< emailAddress AttributeType ::= { pkcs-9 1 } */

/* RFC 4055 */
#define OID_RSASSA_PSS          OID_PKCS1 "\x0a" /**< id-RSASSA-PSS ::= { pkcs-1 10 } */
#define OID_MGF1                OID_PKCS1 "\x08" /**< id-mgf1 ::= { pkcs-1 8 } */

/*
* Digest algorithms
*/
#define OID_DIGEST_ALG_MD2              OID_RSA_COMPANY "\x02\x02" /**< id-md2 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 2 } */
#define OID_DIGEST_ALG_MD4              OID_RSA_COMPANY "\x02\x04" /**< id-md4 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 4 } */
#define OID_DIGEST_ALG_MD5              OID_RSA_COMPANY "\x02\x05" /**< id-md5 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 5 } */
#define OID_DIGEST_ALG_SHA1             OID_ISO_IDENTIFIED_ORG OID_OIW_SECSIG_SHA1 /**< id-sha1 OBJECT IDENTIFIER ::= { iso(1) identified-organization(3) oiw(14) secsig(3) algorithms(2) 26 } */
#define OID_DIGEST_ALG_SHA224           OID_NIST_ALG "\x02\x04" /**< id-sha224 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistalgorithm(4) hashalgs(2) 4 } */
#define OID_DIGEST_ALG_SHA256           OID_NIST_ALG "\x02\x01" /**< id-sha256 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistalgorithm(4) hashalgs(2) 1 } */

#define OID_DIGEST_ALG_SHA384           OID_NIST_ALG "\x02\x02" /**< id-sha384 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistalgorithm(4) hashalgs(2) 2 } */

#define OID_DIGEST_ALG_SHA512           OID_NIST_ALG "\x02\x03" /**< id-sha512 OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistalgorithm(4) hashalgs(2) 3 } */

#define OID_HMAC_SHA1                   OID_RSA_COMPANY "\x02\x07" /**< id-hmacWithSHA1 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 7 } */

#define OID_HMAC_SHA224                 OID_RSA_COMPANY "\x02\x08" /**< id-hmacWithSHA224 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 8 } */

#define OID_HMAC_SHA256                 OID_RSA_COMPANY "\x02\x09" /**< id-hmacWithSHA256 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 9 } */

#define OID_HMAC_SHA384                 OID_RSA_COMPANY "\x02\x0A" /**< id-hmacWithSHA384 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 10 } */

#define OID_HMAC_SHA512                 OID_RSA_COMPANY "\x02\x0B" /**< id-hmacWithSHA512 OBJECT IDENTIFIER ::= { iso(1) member-body(2) us(840) rsadsi(113549) digestAlgorithm(2) 11 } */

/*
* Encryption algorithms
*/
#define OID_DES_CBC                     OID_ISO_IDENTIFIED_ORG OID_OIW_SECSIG_ALG "\x07" /**< desCBC OBJECT IDENTIFIER ::= { iso(1) identified-organization(3) oiw(14) secsig(3) algorithms(2) 7 } */
#define OID_DES_EDE3_CBC                OID_RSA_COMPANY "\x03\x07" /**< des-ede3-cbc OBJECT IDENTIFIER ::= { iso(1) member-body(2) -- us(840) rsadsi(113549) encryptionAlgorithm(3) 7 } */
#define OID_AES                         OID_NIST_ALG "\x01" /** aes OBJECT IDENTIFIER ::= { joint-iso-itu-t(2) country(16) us(840) organization(1) gov(101) csor(3) nistAlgorithm(4) 1 } */

/*
* Key Wrapping algorithms
*/
/*
* RFC 5649
*/
#define OID_AES128_KW                   OID_AES "\x05" /** id-aes128-wrap     OBJECT IDENTIFIER ::= { aes 5 } */
#define OID_AES128_KWP                  OID_AES "\x08" /** id-aes128-wrap-pad OBJECT IDENTIFIER ::= { aes 8 } */
#define OID_AES192_KW                   OID_AES "\x19" /** id-aes192-wrap     OBJECT IDENTIFIER ::= { aes 25 } */
#define OID_AES192_KWP                  OID_AES "\x1c" /** id-aes192-wrap-pad OBJECT IDENTIFIER ::= { aes 28 } */
#define OID_AES256_KW                   OID_AES "\x2d" /** id-aes256-wrap     OBJECT IDENTIFIER ::= { aes 45 } */
#define OID_AES256_KWP                  OID_AES "\x30" /** id-aes256-wrap-pad OBJECT IDENTIFIER ::= { aes 48 } */
/*
* PKCS#5 OIDs
*/
#define OID_PKCS5_PBKDF2                OID_PKCS5 "\x0c" /**< id-PBKDF2 OBJECT IDENTIFIER ::= {pkcs-5 12} */
#define OID_PKCS5_PBES2                 OID_PKCS5 "\x0d" /**< id-PBES2 OBJECT IDENTIFIER ::= {pkcs-5 13} */
#define OID_PKCS5_PBMAC1                OID_PKCS5 "\x0e" /**< id-PBMAC1 OBJECT IDENTIFIER ::= {pkcs-5 14} */

/*
* PKCS#5 PBES1 algorithms
*/
#define OID_PKCS5_PBE_MD2_DES_CBC       OID_PKCS5 "\x01" /**< pbeWithMD2AndDES-CBC OBJECT IDENTIFIER ::= {pkcs-5 1} */
#define OID_PKCS5_PBE_MD2_RC2_CBC       OID_PKCS5 "\x04" /**< pbeWithMD2AndRC2-CBC OBJECT IDENTIFIER ::= {pkcs-5 4} */
#define OID_PKCS5_PBE_MD5_DES_CBC       OID_PKCS5 "\x03" /**< pbeWithMD5AndDES-CBC OBJECT IDENTIFIER ::= {pkcs-5 3} */
#define OID_PKCS5_PBE_MD5_RC2_CBC       OID_PKCS5 "\x06" /**< pbeWithMD5AndRC2-CBC OBJECT IDENTIFIER ::= {pkcs-5 6} */
#define OID_PKCS5_PBE_SHA1_DES_CBC      OID_PKCS5 "\x0a" /**< pbeWithSHA1AndDES-CBC OBJECT IDENTIFIER ::= {pkcs-5 10} */
#define OID_PKCS5_PBE_SHA1_RC2_CBC      OID_PKCS5 "\x0b" /**< pbeWithSHA1AndRC2-CBC OBJECT IDENTIFIER ::= {pkcs-5 11} */

/*
* PKCS#8 OIDs
*/
#define OID_PKCS9_CSR_EXT_REQ           OID_PKCS9 "\x0e" /**< extensionRequest OBJECT IDENTIFIER ::= {pkcs-9 14} */

/*
* PKCS#12 PBE OIDs
*/
#define OID_PKCS12_PBE                      OID_PKCS12 "\x01" /**< pkcs-12PbeIds OBJECT IDENTIFIER ::= {pkcs-12 1} */

#define OID_PKCS12_PBE_SHA1_RC4_128         OID_PKCS12_PBE "\x01" /**< pbeWithSHAAnd128BitRC4 OBJECT IDENTIFIER ::= {pkcs-12PbeIds 1} */
#define OID_PKCS12_PBE_SHA1_RC4_40          OID_PKCS12_PBE "\x02" /**< pbeWithSHAAnd40BitRC4 OBJECT IDENTIFIER ::= {pkcs-12PbeIds 2} */
#define OID_PKCS12_PBE_SHA1_DES3_EDE_CBC    OID_PKCS12_PBE "\x03" /**< pbeWithSHAAnd3-KeyTripleDES-CBC OBJECT IDENTIFIER ::= {pkcs-12PbeIds 3} */
#define OID_PKCS12_PBE_SHA1_DES2_EDE_CBC    OID_PKCS12_PBE "\x04" /**< pbeWithSHAAnd2-KeyTripleDES-CBC OBJECT IDENTIFIER ::= {pkcs-12PbeIds 4} */
#define OID_PKCS12_PBE_SHA1_RC2_128_CBC     OID_PKCS12_PBE "\x05" /**< pbeWithSHAAnd128BitRC2-CBC OBJECT IDENTIFIER ::= {pkcs-12PbeIds 5} */
#define OID_PKCS12_PBE_SHA1_RC2_40_CBC      OID_PKCS12_PBE "\x06" /**< pbeWithSHAAnd40BitRC2-CBC OBJECT IDENTIFIER ::= {pkcs-12PbeIds 6} */

/*
* EC key algorithms from RFC 5480
*/

/* id-ecPublicKey OBJECT IDENTIFIER ::= {
*       iso(1) member-body(2) us(840) ansi-X9-62(10045) keyType(2) 1 } */
#define OID_EC_ALG_UNRESTRICTED         OID_ANSI_X9_62 "\x02\01"

/*   id-ecDH OBJECT IDENTIFIER ::= {
*     iso(1) identified-organization(3) certicom(132)
*     schemes(1) ecdh(12) } */
#define OID_EC_ALG_ECDH                 OID_CERTICOM "\x01\x0c"

/*
* ECParameters namedCurve identifiers, from RFC 5480, RFC 5639, and SEC2
*/

/* secp192r1 OBJECT IDENTIFIER ::= {
*   iso(1) member-body(2) us(840) ansi-X9-62(10045) curves(3) prime(1) 1 } */
#define OID_EC_GRP_SECP192R1        OID_ANSI_X9_62 "\x03\x01\x01"

/* secp224r1 OBJECT IDENTIFIER ::= {
*   iso(1) identified-organization(3) certicom(132) curve(0) 33 } */
#define OID_EC_GRP_SECP224R1        OID_CERTICOM "\x00\x21"

/* secp256r1 OBJECT IDENTIFIER ::= {
*   iso(1) member-body(2) us(840) ansi-X9-62(10045) curves(3) prime(1) 7 } */
#define OID_EC_GRP_SECP256R1        OID_ANSI_X9_62 "\x03\x01\x07"

/* secp384r1 OBJECT IDENTIFIER ::= {
*   iso(1) identified-organization(3) certicom(132) curve(0) 34 } */
#define OID_EC_GRP_SECP384R1        OID_CERTICOM "\x00\x22"

/* secp521r1 OBJECT IDENTIFIER ::= {
*   iso(1) identified-organization(3) certicom(132) curve(0) 35 } */
#define OID_EC_GRP_SECP521R1        OID_CERTICOM "\x00\x23"

/* secp192k1 OBJECT IDENTIFIER ::= {
*   iso(1) identified-organization(3) certicom(132) curve(0) 31 } */
#define OID_EC_GRP_SECP192K1        OID_CERTICOM "\x00\x1f"

/* secp224k1 OBJECT IDENTIFIER ::= {
*   iso(1) identified-organization(3) certicom(132) curve(0) 32 } */
#define OID_EC_GRP_SECP224K1        OID_CERTICOM "\x00\x20"

/* secp256k1 OBJECT IDENTIFIER ::= {
*   iso(1) identified-organization(3) certicom(132) curve(0) 10 } */
#define OID_EC_GRP_SECP256K1        OID_CERTICOM "\x00\x0a"

/* RFC 5639 4.1
* ecStdCurvesAndGeneration OBJECT IDENTIFIER::= {iso(1)
* identified-organization(3) teletrust(36) algorithm(3) signature-
* algorithm(3) ecSign(2) 8}
* ellipticCurve OBJECT IDENTIFIER ::= {ecStdCurvesAndGeneration 1}
* versionOne OBJECT IDENTIFIER ::= {ellipticCurve 1} */
#define OID_EC_BRAINPOOL_V1         OID_TELETRUST "\x03\x03\x02\x08\x01\x01"

/* brainpoolP256r1 OBJECT IDENTIFIER ::= {versionOne 7} */
#define OID_EC_GRP_BP256R1          OID_EC_BRAINPOOL_V1 "\x07"

/* brainpoolP384r1 OBJECT IDENTIFIER ::= {versionOne 11} */
#define OID_EC_GRP_BP384R1          OID_EC_BRAINPOOL_V1 "\x0B"

/* brainpoolP512r1 OBJECT IDENTIFIER ::= {versionOne 13} */
#define OID_EC_GRP_BP512R1          OID_EC_BRAINPOOL_V1 "\x0D"

/*
* SEC1 C.1
*
* prime-field OBJECT IDENTIFIER ::= { id-fieldType 1 }
* id-fieldType OBJECT IDENTIFIER ::= { ansi-X9-62 fieldType(1)}
*/
#define OID_ANSI_X9_62_FIELD_TYPE   OID_ANSI_X9_62 "\x01"
#define OID_ANSI_X9_62_PRIME_FIELD  OID_ANSI_X9_62_FIELD_TYPE "\x01"

/*
* ECDSA signature identifiers, from RFC 5480
*/
#define OID_ANSI_X9_62_SIG          OID_ANSI_X9_62 "\x04" /* signatures(4) */
#define OID_ANSI_X9_62_SIG_SHA2     OID_ANSI_X9_62_SIG "\x03" /* ecdsa-with-SHA2(3) */

/* ecdsa-with-SHA1 OBJECT IDENTIFIER ::= {
*   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4) 1 } */
#define OID_ECDSA_SHA1              OID_ANSI_X9_62_SIG "\x01"

/* ecdsa-with-SHA224 OBJECT IDENTIFIER ::= {
*   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4)
*   ecdsa-with-SHA2(3) 1 } */
#define OID_ECDSA_SHA224            OID_ANSI_X9_62_SIG_SHA2 "\x01"

/* ecdsa-with-SHA256 OBJECT IDENTIFIER ::= {
*   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4)
*   ecdsa-with-SHA2(3) 2 } */
#define OID_ECDSA_SHA256            OID_ANSI_X9_62_SIG_SHA2 "\x02"

/* ecdsa-with-SHA384 OBJECT IDENTIFIER ::= {
*   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4)
*   ecdsa-with-SHA2(3) 3 } */
#define OID_ECDSA_SHA384            OID_ANSI_X9_62_SIG_SHA2 "\x03"

/* ecdsa-with-SHA512 OBJECT IDENTIFIER ::= {
*   iso(1) member-body(2) us(840) ansi-X9-62(10045) signatures(4)
*   ecdsa-with-SHA2(3) 4 } */
#define OID_ECDSA_SHA512            OID_ANSI_X9_62_SIG_SHA2 "\x04"

/*******************************************************************************************************************/

#define ADD_LEN(s)      s, OID_SIZE(s)

typedef struct {
	const char *asn1;               /*!< OID ASN.1 representation       */
	dword_t asn1_len;                /*!< length of asn1                 */
	const char *name;               /*!< official name (e.g. from RFC)  */
	const char *description;        /*!< human friendly description     */
} _oid_descriptor_t;

/*
* For X520 attribute types
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	const char          *short_name;
} _oid_x520_attr_t;

static const _oid_x520_attr_t oid_x520_attr_type[] =
{
	{
		{ ADD_LEN(OID_AT_CN), "id-at-commonName", "Common Name" },
		"CN",
	},
	{
		{ ADD_LEN(OID_AT_COUNTRY), "id-at-countryName", "Country" },
		"C",
	},
	{
		{ ADD_LEN(OID_AT_LOCALITY), "id-at-locality", "Locality" },
		"L",
	},
	{
		{ ADD_LEN(OID_AT_STATE), "id-at-state", "State" },
		"ST",
	},
	{
		{ ADD_LEN(OID_AT_ORGANIZATION), "id-at-organizationName", "Organization" },
		"O",
	},
	{
		{ ADD_LEN(OID_AT_ORG_UNIT), "id-at-organizationalUnitName", "Org Unit" },
		"OU",
	},
	{
		{ ADD_LEN(OID_PKCS9_EMAIL), "emailAddress", "E-mail address" },
		"emailAddress",
	},
	{
		{ ADD_LEN(OID_AT_SERIAL_NUMBER), "id-at-serialNumber", "Serial number" },
		"serialNumber",
	},
	{
		{ ADD_LEN(OID_AT_POSTAL_ADDRESS), "id-at-postalAddress", "Postal address" },
		"postalAddress",
	},
	{
		{ ADD_LEN(OID_AT_POSTAL_CODE), "id-at-postalCode", "Postal code" },
		"postalCode",
	},
	{
		{ ADD_LEN(OID_AT_SUR_NAME), "id-at-surName", "Surname" },
		"SN",
	},
	{
		{ ADD_LEN(OID_AT_GIVEN_NAME), "id-at-givenName", "Given name" },
		"GN",
	},
	{
		{ ADD_LEN(OID_AT_INITIALS), "id-at-initials", "Initials" },
		"initials",
	},
	{
		{ ADD_LEN(OID_AT_GENERATION_QUALIFIER), "id-at-generationQualifier", "Generation qualifier" },
		"generationQualifier",
	},
	{
		{ ADD_LEN(OID_AT_TITLE), "id-at-title", "Title" },
		"title",
	},
	{
		{ ADD_LEN(OID_AT_DN_QUALIFIER), "id-at-dnQualifier", "Distinguished Name qualifier" },
		"dnQualifier",
	},
	{
		{ ADD_LEN(OID_AT_PSEUDONYM), "id-at-pseudonym", "Pseudonym" },
		"pseudonym",
	},
	{
		{ ADD_LEN(OID_DOMAIN_COMPONENT), "id-domainComponent", "Domain component" },
		"DC",
	},
	{
		{ ADD_LEN(OID_AT_UNIQUE_IDENTIFIER), "id-at-uniqueIdentifier", "Unique Identifier" },
		"uniqueIdentifier",
	},
	{
		{ NULL, 0, NULL, NULL },
		NULL,
	}
};

bool_t x520_attr_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_x520_attr_type) / sizeof(_oid_x520_attr_t);
	for (i = 0; i < n; i++)
	{
		if (oid_x520_attr_type[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_x520_attr_type[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_x520_attr_type[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_x520_attr_type[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t x520_attr_short_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_x520_attr_type) / sizeof(_oid_x520_attr_t);
	for (i = 0; i < n; i++)
	{
		if (oid_x520_attr_type[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_x520_attr_type[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_x520_attr_type[i].short_name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_x520_attr_type[i].short_name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t x520_attr_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_x520_attr_type) / sizeof(_oid_x520_attr_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_x520_attr_type[i].descriptor.name, alen) == 0)
		{
			*olen = oid_x520_attr_type[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_x520_attr_type[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}

/*
* For X509 extensions
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	int                 ext_type;
} _oid_x509_ext_t;

static const _oid_x509_ext_t oid_x509_ext[] =
{
	{
		{ ADD_LEN(OID_BASIC_CONSTRAINTS), "id-ce-basicConstraints", "Basic Constraints" },
		X509_EXT_BASIC_CONSTRAINTS,
	},
	{
		{ ADD_LEN(OID_KEY_USAGE), "id-ce-keyUsage", "Key Usage" },
		X509_EXT_KEY_USAGE,
	},
	{
		{ ADD_LEN(OID_EXTENDED_KEY_USAGE), "id-ce-extKeyUsage", "Extended Key Usage" },
		X509_EXT_EXTENDED_KEY_USAGE,
	},
	{
		{ ADD_LEN(OID_SUBJECT_ALT_NAME), "id-ce-subjectAltName", "Subject Alt Name" },
		X509_EXT_SUBJECT_ALT_NAME,
	},
	{
		{ ADD_LEN(OID_NS_CERT_TYPE), "id-netscape-certtype", "Netscape Certificate Type" },
		X509_EXT_NS_CERT_TYPE,
	},
	{
		{ NULL, 0, NULL, NULL },
		0,
	},
};

bool_t x509_ext_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_x509_ext) / sizeof(_oid_x509_ext_t);
	for (i = 0; i < n; i++)
	{
		if (oid_x509_ext[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_x509_ext[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_x509_ext[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_x509_ext[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t x509_ext_type_from_oid(const byte_t* oid, dword_t olen, int* type)
{
	int i, n;

	n = sizeof(oid_x509_ext) / sizeof(_oid_x509_ext_t);
	for (i = 0; i < n; i++)
	{
		if (oid_x509_ext[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_x509_ext[i].descriptor.asn1, olen) == 0)
		{
			*type = oid_x509_ext[i].ext_type;
			return 1;
		}
	}

	return 0;
}

bool_t x509_ext_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_x509_ext) / sizeof(_oid_x509_ext_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_x509_ext[i].descriptor.name, alen) == 0)
		{
			*olen = oid_x509_ext[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_x509_ext[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}

static const _oid_descriptor_t oid_ext_key_usage[] =
{
	{ ADD_LEN(OID_SERVER_AUTH), "id-kp-serverAuth", "TLS Web Server Authentication" },
	{ ADD_LEN(OID_CLIENT_AUTH), "id-kp-clientAuth", "TLS Web Client Authentication" },
	{ ADD_LEN(OID_CODE_SIGNING), "id-kp-codeSigning", "Code Signing" },
	{ ADD_LEN(OID_EMAIL_PROTECTION), "id-kp-emailProtection", "E-mail Protection" },
	{ ADD_LEN(OID_TIME_STAMPING), "id-kp-timeStamping", "Time Stamping" },
	{ ADD_LEN(OID_OCSP_SIGNING), "id-kp-OCSPSigning", "OCSP Signing" },
	{ NULL, 0, NULL, NULL },
};

bool_t ext_key_usage_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_ext_key_usage) / sizeof(_oid_descriptor_t);
	for (i = 0; i < n; i++)
	{
		if (oid_ext_key_usage[i].asn1_len == olen && xmem_comp(oid, oid_ext_key_usage[i].asn1, olen) == 0)
		{
			*alen = a_xslen(oid_ext_key_usage[i].name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_ext_key_usage[i].name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t ext_key_usage_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_ext_key_usage) / sizeof(_oid_descriptor_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_ext_key_usage[i].name, alen) == 0)
		{
			*olen = oid_ext_key_usage[i].asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_ext_key_usage[i].asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}

/*
* For SignatureAlgorithmIdentifier
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	md_type_t           md_alg;
	pk_type_t           pk_alg;
} _oid_sig_alg_t;

static const _oid_sig_alg_t oid_sig_alg[] =
{
	{
		{ ADD_LEN(OID_PKCS1_MD2), "md2WithRSAEncryption", "RSA with MD2" },
		MD_MD2, PK_RSA,
	},
	{
		{ ADD_LEN(OID_PKCS1_MD4), "md4WithRSAEncryption", "RSA with MD4" },
		MD_MD4, PK_RSA,
	},
	{
		{ ADD_LEN(OID_PKCS1_MD5), "md5WithRSAEncryption", "RSA with MD5" },
		MD_MD5, PK_RSA,
	},
	{
		{ ADD_LEN(OID_PKCS1_SHA1), "sha-1WithRSAEncryption", "RSA with SHA1" },
		MD_SHA1, PK_RSA,
	},
	{
		{ ADD_LEN(OID_PKCS1_SHA224), "sha224WithRSAEncryption", "RSA with SHA-224" },
		MD_SHA224, PK_RSA,
	},
	{
		{ ADD_LEN(OID_PKCS1_SHA256), "sha256WithRSAEncryption", "RSA with SHA-256" },
		MD_SHA256, PK_RSA,
	},
	{
		{ ADD_LEN(OID_PKCS1_SHA384), "sha384WithRSAEncryption", "RSA with SHA-384" },
		MD_SHA384, PK_RSA,
	},
	{
		{ ADD_LEN(OID_PKCS1_SHA512), "sha512WithRSAEncryption", "RSA with SHA-512" },
		MD_SHA512, PK_RSA,
	},
	{
		{ ADD_LEN(OID_RSA_SHA_OBS), "sha-1WithRSAEncryption", "RSA with SHA1" },
		MD_SHA1, PK_RSA,
	},
	{
		{ ADD_LEN(OID_ECDSA_SHA1), "ecdsa-with-SHA1", "ECDSA with SHA1" },
		MD_SHA1, PK_ECDSA,
	},
	{
		{ ADD_LEN(OID_ECDSA_SHA224), "ecdsa-with-SHA224", "ECDSA with SHA224" },
		MD_SHA224, PK_ECDSA,
	},
	{
		{ ADD_LEN(OID_ECDSA_SHA256), "ecdsa-with-SHA256", "ECDSA with SHA256" },
		MD_SHA256, PK_ECDSA,
	},
	{
		{ ADD_LEN(OID_ECDSA_SHA384), "ecdsa-with-SHA384", "ECDSA with SHA384" },
		MD_SHA384, PK_ECDSA,
	},
	{
		{ ADD_LEN(OID_ECDSA_SHA512), "ecdsa-with-SHA512", "ECDSA with SHA512" },
		MD_SHA512, PK_ECDSA,
	},
	{
		{ ADD_LEN(OID_RSASSA_PSS), "RSASSA-PSS", "RSASSA-PSS" },
		MD_NONE, PK_RSASSA_PSS,
	},
	{
		{ NULL, 0, NULL, NULL },
		MD_NONE, PK_NONE,
	},
};

bool_t sig_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_sig_alg) / sizeof(_oid_sig_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_sig_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_sig_alg[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_sig_alg[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_sig_alg[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t sig_alg_type_from_oid(const byte_t* oid, dword_t olen, md_type_t* md, pk_type_t* pk)
{
	int i, n;

	n = sizeof(oid_sig_alg) / sizeof(_oid_sig_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_sig_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_sig_alg[i].descriptor.asn1, olen) == 0)
		{
			*md = oid_sig_alg[i].md_alg;
			*pk = oid_sig_alg[i].pk_alg;
			return 1;
		}
	}

	return 0;
}

bool_t sig_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_sig_alg) / sizeof(_oid_sig_alg_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_sig_alg[i].descriptor.name, alen) == 0)
		{
			*olen = oid_sig_alg[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_sig_alg[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}

/*
* For PublicKeyInfo (PKCS1, RFC 5480)
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	pk_type_t           pk_alg;
} _oid_pk_alg_t;

static const _oid_pk_alg_t oid_pk_alg[] =
{
	{
		{ ADD_LEN(OID_PKCS1_RSA), "rsaEncryption", "RSA" },
		PK_RSA,
	},
	{
		{ ADD_LEN(OID_EC_ALG_UNRESTRICTED), "id-ecPublicKey", "Generic EC key" },
		PK_ECKEY,
	},
	{
		{ ADD_LEN(OID_EC_ALG_ECDH), "id-ecDH", "EC key for ECDH" },
		PK_ECKEY_DH,
	},
	{
		{ NULL, 0, NULL, NULL },
		PK_NONE,
	},
};

bool_t pk_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_pk_alg) / sizeof(_oid_pk_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_pk_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_pk_alg[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_pk_alg[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_pk_alg[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t pk_alg_type_from_oid(const byte_t* oid, dword_t olen, pk_type_t* pk)
{
	int i, n;

	n = sizeof(oid_pk_alg) / sizeof(_oid_pk_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_pk_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_pk_alg[i].descriptor.asn1, olen) == 0)
		{
			*pk = oid_pk_alg[i].pk_alg;
			return 1;
		}
	}

	return 0;
}

bool_t pk_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_pk_alg) / sizeof(_oid_pk_alg_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_pk_alg[i].descriptor.name, alen) == 0)
		{
			*olen = oid_pk_alg[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_pk_alg[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}


/*
* For namedCurve (RFC 5480)
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	ecp_group_id        grp_id;
} _oid_ecp_grp_t;

static const _oid_ecp_grp_t oid_ecp_grp[] =
{
	{
		{ ADD_LEN(OID_EC_GRP_SECP192R1), "secp192r1", "secp192r1" },
		ECP_DP_SECP192R1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_SECP224R1), "secp224r1", "secp224r1" },
		ECP_DP_SECP224R1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_SECP256R1), "secp256r1", "secp256r1" },
		ECP_DP_SECP256R1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_SECP384R1), "secp384r1", "secp384r1" },
		ECP_DP_SECP384R1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_SECP521R1), "secp521r1", "secp521r1" },
		ECP_DP_SECP521R1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_SECP192K1), "secp192k1", "secp192k1" },
		ECP_DP_SECP192K1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_SECP224K1), "secp224k1", "secp224k1" },
		ECP_DP_SECP224K1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_SECP256K1), "secp256k1", "secp256k1" },
		ECP_DP_SECP256K1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_BP256R1), "brainpoolP256r1", "brainpool256r1" },
		ECP_DP_BP256R1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_BP384R1), "brainpoolP384r1", "brainpool384r1" },
		ECP_DP_BP384R1,
	},
	{
		{ ADD_LEN(OID_EC_GRP_BP512R1), "brainpoolP512r1", "brainpool512r1" },
		ECP_DP_BP512R1,
	},
	{
		{ NULL, 0, NULL, NULL },
		ECP_DP_NONE,
	},
};

bool_t ecp_grp_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_ecp_grp) / sizeof(_oid_ecp_grp_t);
	for (i = 0; i < n; i++)
	{
		if (oid_ecp_grp[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_ecp_grp[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_ecp_grp[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_ecp_grp[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t ecp_grp_type_from_oid(const byte_t* oid, dword_t olen, ecp_group_id* grp_id)
{
	int i, n;

	n = sizeof(oid_ecp_grp) / sizeof(_oid_ecp_grp_t);
	for (i = 0; i < n; i++)
	{
		if (oid_ecp_grp[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_ecp_grp[i].descriptor.asn1, olen) == 0)
		{
			*grp_id = oid_ecp_grp[i].grp_id;
			return 1;
		}
	}

	return 0;
}

bool_t ecp_grp_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_ecp_grp) / sizeof(_oid_ecp_grp_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_ecp_grp[i].descriptor.name, alen) == 0)
		{
			*olen = oid_ecp_grp[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_ecp_grp[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}


/*
* For PKCS#5 PBES2 encryption algorithm
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	cipher_type_t       cipher_alg;
} _oid_cipher_alg_t;

static const _oid_cipher_alg_t oid_cipher_alg[] =
{
	{
		{ ADD_LEN(OID_DES_CBC), "desCBC", "DES-CBC" },
		CIPHER_DES_CBC,
	},
	{
		{ ADD_LEN(OID_DES_EDE3_CBC), "des-ede3-cbc", "DES-EDE3-CBC" },
		CIPHER_DES_EDE3_CBC,
	},
	{
		{ NULL, 0, NULL, NULL },
		CIPHER_NONE,
	},
};

bool_t cipher_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_cipher_alg) / sizeof(_oid_cipher_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_cipher_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_cipher_alg[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_cipher_alg[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_cipher_alg[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t cipher_alg_type_from_oid(const byte_t* oid, dword_t olen, int* cipher)
{
	int i, n;

	n = sizeof(oid_cipher_alg) / sizeof(_oid_cipher_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_cipher_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_cipher_alg[i].descriptor.asn1, olen) == 0)
		{
			*cipher = oid_cipher_alg[i].cipher_alg;
			return 1;
		}
	}

	return 0;
}

bool_t cipher_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_cipher_alg) / sizeof(_oid_cipher_alg_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_cipher_alg[i].descriptor.name, alen) == 0)
		{
			*olen = oid_cipher_alg[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_cipher_alg[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}

/*
* For digestAlgorithm
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	md_type_t           md_alg;
} _oid_md_alg_t;

static const _oid_md_alg_t oid_md_alg[] =
{
	{
		{ ADD_LEN(OID_DIGEST_ALG_MD2), "id-md2", "MD2" },
		MD_MD2,
	},
	{
		{ ADD_LEN(OID_DIGEST_ALG_MD4), "id-md4", "MD4" },
		MD_MD4,
	},
	{
		{ ADD_LEN(OID_DIGEST_ALG_MD5), "id-md5", "MD5" },
		MD_MD5,
	},
	{
		{ ADD_LEN(OID_DIGEST_ALG_SHA1), "id-sha1", "SHA-1" },
		MD_SHA1,
	},
	{
		{ ADD_LEN(OID_DIGEST_ALG_SHA224), "id-sha224", "SHA-224" },
		MD_SHA224,
	},
	{
		{ ADD_LEN(OID_DIGEST_ALG_SHA256), "id-sha256", "SHA-256" },
		MD_SHA256,
	},
	{
		{ ADD_LEN(OID_DIGEST_ALG_SHA384), "id-sha384", "SHA-384" },
		MD_SHA384,
	},
	{
		{ ADD_LEN(OID_DIGEST_ALG_SHA512), "id-sha512", "SHA-512" },
		MD_SHA512,
	},
	{
		{ NULL, 0, NULL, NULL },
		MD_NONE,
	},
};

bool_t md_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_md_alg) / sizeof(_oid_md_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_md_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_md_alg[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_md_alg[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_md_alg[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t md_alg_type_from_oid(const byte_t* oid, dword_t olen, md_type_t* md)
{
	int i, n;

	n = sizeof(oid_md_alg) / sizeof(_oid_md_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_md_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_md_alg[i].descriptor.asn1, olen) == 0)
		{
			*md = oid_md_alg[i].md_alg;
			return 1;
		}
	}

	return 0;
}

bool_t md_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_md_alg) / sizeof(_oid_md_alg_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_md_alg[i].descriptor.name, alen) == 0)
		{
			*olen = oid_md_alg[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_md_alg[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}


/*
* For HMAC digestAlgorithm
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	md_type_t           md_hmac;
} _oid_md_hmac_t;

static const _oid_md_hmac_t oid_md_hmac[] =
{
	{
		{ ADD_LEN(OID_HMAC_SHA1), "hmacSHA1", "HMAC-SHA-1" },
		MD_SHA1,
	},
	{
		{ ADD_LEN(OID_HMAC_SHA224), "hmacSHA224", "HMAC-SHA-224" },
		MD_SHA224,
	},
	{
		{ ADD_LEN(OID_HMAC_SHA256), "hmacSHA256", "HMAC-SHA-256" },
		MD_SHA256,
	},
	{
		{ ADD_LEN(OID_HMAC_SHA384), "hmacSHA384", "HMAC-SHA-384" },
		MD_SHA384,
	},
	{
		{ ADD_LEN(OID_HMAC_SHA512), "hmacSHA512", "HMAC-SHA-512" },
		MD_SHA512,
	},
	{
		{ NULL, 0, NULL, NULL },
		MD_NONE,
	},
};

bool_t md_hmac_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_md_hmac) / sizeof(_oid_md_hmac_t);
	for (i = 0; i < n; i++)
	{
		if (oid_md_hmac[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_md_hmac[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_md_hmac[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_md_hmac[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t md_hmac_type_from_oid(const byte_t* oid, dword_t olen, md_type_t* md)
{
	int i, n;

	n = sizeof(oid_md_hmac) / sizeof(_oid_md_hmac_t);
	for (i = 0; i < n; i++)
	{
		if (oid_md_hmac[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_md_hmac[i].descriptor.asn1, olen) == 0)
		{
			*md = oid_md_hmac[i].md_hmac;
			return 1;
		}
	}

	return 0;
}

bool_t md_hmac_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_md_hmac) / sizeof(_oid_md_hmac_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_md_hmac[i].descriptor.name, alen) == 0)
		{
			*olen = oid_md_hmac[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_md_hmac[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}

/*
* For PKCS#12 PBEs
*/
typedef struct {
	_oid_descriptor_t    descriptor;
	md_type_t           md_alg;
	cipher_type_t       cipher_alg;
} _oid_pkcs12_pbe_alg_t;

static const _oid_pkcs12_pbe_alg_t oid_pkcs12_pbe_alg[] =
{
	{
		{ ADD_LEN(OID_PKCS12_PBE_SHA1_DES3_EDE_CBC), "pbeWithSHAAnd3-KeyTripleDES-CBC", "PBE with SHA1 and 3-Key 3DES" },
		MD_SHA1, CIPHER_DES_EDE3_CBC,
	},
	{
		{ ADD_LEN(OID_PKCS12_PBE_SHA1_DES2_EDE_CBC), "pbeWithSHAAnd2-KeyTripleDES-CBC", "PBE with SHA1 and 2-Key 3DES" },
		MD_SHA1, CIPHER_DES_EDE_CBC,
	},
	{
		{ NULL, 0, NULL, NULL },
		MD_NONE, CIPHER_NONE,
	},
};

bool_t pkcs12_pbe_alg_name_from_oid(const byte_t* oid, dword_t olen, byte_t* attr, dword_t* alen)
{
	int i, n;

	n = sizeof(oid_pkcs12_pbe_alg) / sizeof(_oid_pkcs12_pbe_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_pkcs12_pbe_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_pkcs12_pbe_alg[i].descriptor.asn1, olen) == 0)
		{
			*alen = a_xslen(oid_pkcs12_pbe_alg[i].descriptor.name);
			if (attr)
			{
				xmem_copy((void*)attr, (void*)(oid_pkcs12_pbe_alg[i].descriptor.name), *alen);
			}
			return 1;
		}
	}

	return 0;
}

bool_t pkcs12_pbe_alg_type_from_oid(const byte_t* oid, dword_t olen, int* md, int* cipher)
{
	int i, n;

	n = sizeof(oid_pkcs12_pbe_alg) / sizeof(_oid_pkcs12_pbe_alg_t);
	for (i = 0; i < n; i++)
	{
		if (oid_pkcs12_pbe_alg[i].descriptor.asn1_len == olen && xmem_comp(oid, oid_pkcs12_pbe_alg[i].descriptor.asn1, olen) == 0)
		{
			*md = oid_pkcs12_pbe_alg[i].md_alg;
			*cipher = oid_pkcs12_pbe_alg[i].cipher_alg;
			return 1;
		}
	}

	return 0;
}

bool_t pkcs12_pbe_alg_oid_from_name(const byte_t* attr, dword_t alen, byte_t* oid, dword_t* olen)
{
	int i, n;

	n = sizeof(oid_pkcs12_pbe_alg) / sizeof(_oid_pkcs12_pbe_alg_t);
	for (i = 0; i < n; i++)
	{
		if (xmem_comp(attr, oid_pkcs12_pbe_alg[i].descriptor.name, alen) == 0)
		{
			*olen = oid_pkcs12_pbe_alg[i].descriptor.asn1_len;
			if (oid)
			{
				xmem_copy((void*)oid, (void*)(oid_pkcs12_pbe_alg[i].descriptor.asn1), *olen);
			}
			return 1;
		}
	}

	return 0;
}
/*
前两部分如果定义为x.y, 那么它们将合成一个字40*x + y, 其余部分单独作为一个字节进行编码.
每个字首先被分割为最少数量的没有头零数字的7位数字.这些数字以big - endian格式进行组织, 
并且一个接一个地组合成字节.除了编码的最后一个字节外, 其他所有字节的最高位(位8)都为1.
举例: 30331 = 1 * 128 ^ 2 + 108 * 128 + 123   分割成7位数字(0x80)后为{ 1, 108, 123 }
设置最高位后变成{ 129, 236, 123 }.如果该字只有一个7位数字, 那么最高为0.
*/
int oid_to_string(const byte_t* oid, dword_t len, tchar_t* buf, int max)
{
	int n, total = 0;
	dword_t i, value = 0;

	/* First byte contains first two dots */
	if (len > 0)
	{
		n = xsprintf((tchar_t*)((buf)? (buf + total) : NULL), _T("%d.%d"), oid[0] / 40, oid[0] % 40);
		total += n;
	}

	for (i = 1; i < len; i++)
	{
		/* Prevent overflow in value. */
		if (((value << 7) >> 7) != value)
			return(0);

		value *= 128;
		value += (oid[i] & 0x7F);

		if (!(oid[i] & 0x80))
		{
			/* Last byte */
			n = xsprintf(((buf) ? (buf + total) : NULL), _T(".%d"), value);
			total += n;
			value = 0;
		}
	}

	return total;
}

dword_t oid_from_string(byte_t* oid, dword_t max, const tchar_t* str, int len)
{
	int i, n = 0;
	dword_t total = 0;
	const tchar_t* pre;
	unsigned int v1, v2, v;

	if (!str || !len)
		return 0;

	n = 0;
	pre = str;
	while (*str != _T('.') && n < len)
	{
		str++;
		n++;
	}
	if (n == len)
		return 0;

	v1 = xsntol(pre, n);
	len -= n;

	if (*str == _T('.'))
	{
		str++;
		len--;
	}

	n = 0;
	pre = str;
	while (*str != _T('.') && n < len)
	{
		str++;
		n++;
	}
	v2 = xsntol(pre, n);
	len -= n;

	oid[total] = v1 * 40 + v2;
	total++;

	if (*str == _T('.'))
	{
		str++;
		len--;
	}

	while (len)
	{
		n = 0;
		pre = str;
		while (*str != _T('.') && n < len)
		{
			str++;
			n++;
		}

		v = xsntol(pre, n);
		len -= n;

		if (*str == _T('.'))
		{
			str++;
			len--;
		}

		n = v;
		i = 0;
		while (n)
		{
			i++;
			n >>= 7;
		}

		n = 0;
		while (v)
		{
			i--;
			oid[total + i] = v % 128;
			if (n)
			{
				oid[total + i] |= 0x80;
			}
			n++;
			v /= 128;
		}
		total += n;
	}

	return total;
}