/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc ssl document

	@module	ssldef.h | interface file

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

#ifndef _SSLDEF_H
#define _SSLDEF_H

#include "../xdkdef.h"

#if defined(XDK_SUPPORT_SOCK)

#define SSL_MAJOR_VERSION_3             3
#define SSL_MINOR_VERSION_0             0 //SSL v3.0
#define SSL_MINOR_VERSION_1             1 //TLS v1.0
#define SSL_MINOR_VERSION_2             2 //TLS v1.1
#define SSL_MINOR_VERSION_3             3 //TLS v1.2
#define SSL_MINOR_VERSION_4             4 //TLS v1.3

#define DTLS_MAJOR_VERSION_1             254
#define DTLS_MINOR_VERSION_0             255 //DTLS v1.1
#define DTLS_MINOR_VERSION_2             253 //DTLS v1.2

typedef enum{
	SSLv30 = 0x0300,
	TLSv10 = 0x0301,
	TLSv11 = 0x0302,
	TLSv12 = 0x0303,
	TLSv13 = 0x0304,
	DTLSv2 = 0xFEFD,
	DTLSv0 = 0xFEFF
}TLSVER;

#define SSL_MSG_CHANGE_CIPHER_SPEC     20
#define SSL_MSG_ALERT                  21
#define SSL_MSG_HANDSHAKE              22
#define SSL_MSG_APPLICATION_DATA       23

//the alert level
#define SSL_LEVEL_WARNING        1
#define SSL_LEVEL_FATAL          2
//the alert description
#define SSL_ALERT_CLOSE_NOTIFY          0
#define SSL_ALERT_UNEXPECTED_MESSAGE	10
#define SSL_ALERT_BAD_RECORD_MAC		20
#define SSL_ALERT_DECRYPTION_FAILED		21
#define SSL_ALERT_RECORD_OVERFLOW		22
#define SSL_ALERT_DECOMPRESSION_FAILURE		30
#define SSL_ALERT_HANDSHAKE_FAILURE		40
#define SSL_ALERT_NO_CERTIFICATE		41
#define SSL_ALERT_BAD_CERTIFICATE		42
#define SSL_ALERT_UNSUPPORTED_CERTIFICATE	43
#define SSL_ALERT_CERTIFICATE_REVOKED		44
#define SSL_ALERT_CERTIFICATE_EXPIRED		45
#define SSL_ALERT_CERTIFICATE_UNKNOWN		46
#define SSL_ALERT_ILLEGAL_PARAMETER			47
#define SSL_ALERT_UNKNOWN_CA			48
#define SSL_ALERT_ACCESS_DENIED			49
#define SSL_ALERT_DECORD_ERROR			50
#define SSL_ALERT_DECRYPT_ERROR			51
#define SSL_ALERT_EXPORT_RESTRICTION	60
#define SSL_ALERT_PROTOCOL_VERSION		70
#define SSL_ALERT_INSUFFICIENT_SECURITY	71
#define SSL_ALERT_INTERNAL_ERROR		80
#define SSL_ALERT_INAPPROPRIATE_FALLBACK 86
#define SSL_ALERT_USER_CANCELED			90
#define SSL_ALERT_NO_RENEGOTIATION		100
#define SSL_ALERT_MISSING_EXTENSION		109
#define SSL_ALERT_UNSUPPORTED_EXTENSION	110
#define SSL_ALERT_UNRECOGNIZED_NAME		112
#define SSL_ALERT_BAD_CERTIFICATE_STATUS_RESPONSE 113
#define SSL_ALERT_UNKNOWN_PSK_IDENTITY	115
#define SSL_ALERT_CERTIFICATE_REQUIRED	116
#define SSL_ALERT_NO_APPLICATION_PROTOCOL 120
#define SSL_ALERT_UNKNOWN_ERROR			255


#define SSL_EXTENSION_SERVERNAME		0
#define SSL_EXTENSION_MAX_FRAGMENT_LENGTH	1
#define SSL_EXTENSION_STATUS_REQUEST	5
#define SSL_EXTENSION_SUPPORTEDGROUPS	10
#define SSL_EXTENSION_ECPOINTFORMATS	11
#define SSL_EXTENSION_SIGNATUREANDHASHALGORITHM	13
#define SSL_EXTENSION_USE_SRTP			14
#define SSL_EXTENSION_HEARTBEAT			15
#define SSL_EXTENSION_APPLICATION_LAYER_PROTOCOL_NEGOTIATION	16
#define SSL_EXTENSION_SIGNED_CERTIFICATE_TIMESTAMP	18
#define SSL_EXTENSION_CLIENT_CERTIFICATE_TYPE	19
#define SSL_EXTENSION_SERVER_CERTIFICATE_TYPE	20
#define SSL_EXTENSION_PADDING			21
#define SSL_EXTENSION_EXTENDEDMASTERSECRET	23
#define SSL_EXTENSION_SESSIONTICKET		35
#define SSL_EXTENSION_PRE_SHARED_KEY	41
#define SSL_EXTENSION_EARLY_DATA		42
#define SSL_EXTENSION_SUPPORTED_VERSION	43
#define SSL_EXTENSION_COOKIE			44
#define SSL_EXTENSION_PSK_KEY_EXCHANGE_MODE	45
#define SSL_EXTENSION_CERTIFICATE_AUTHORITIES	47
#define SSL_EXTENSION_OID_FILTERS		48
#define SSL_EXTENSION_POST_HANDSHAKE_AUTH	49
#define SSL_EXTENSION_SIGNATURE_ALGORITHMS_CERT	50
#define SSL_EXTENSION_KEY_SHARE			51
#define SSL_EXTENSION_RENEGOTIATIONINFO	65281

#define SSL_CERTIFICATE_TYPE_RSA		1


#define SSL_HS_HELLO_REQUEST            0
#define SSL_HS_CLIENT_HELLO             1
#define SSL_HS_SERVER_HELLO             2
#define SSL_HS_HELLO_VERIFY_REQUEST     3
#define SSL_HS_NEW_SESSION_TICKET       4
#define SSL_HS_END_OF_EARLY_DATA	    5
#define SSL_HS_HELLO_RETRY_REQUEST		6
#define SSL_HS_ENCRYPTED_EXTENSIONS     8
#define SSL_HS_CERTIFICATE             11
#define SSL_HS_SERVER_KEY_EXCHANGE     12
#define SSL_HS_CERTIFICATE_REQUEST     13
#define SSL_HS_SERVER_HELLO_DONE       14
#define SSL_HS_CERTIFICATE_VERIFY      15
#define SSL_HS_CLIENT_KEY_EXCHANGE     16
#define SSL_HS_FINISHED                20
#define SSL_HS_KEY_UPDATE		       24
#define SSL_HS_MESSAGE_HASH		       254

/*
Official IANA names
*/
#define SSL_RSA_WITH_NULL_MD5                    0x01   /**< Weak! */
#define SSL_RSA_WITH_NULL_SHA                    0x02   /**< Weak! */

#define SSL_RSA_WITH_RC4_128_MD5                 0x04
#define SSL_RSA_WITH_RC4_128_SHA                 0x05
#define SSL_RSA_WITH_DES_CBC_SHA                 0x09   /**< Weak! Not in TLS 1.2 */

#define SSL_RSA_WITH_3DES_EDE_CBC_SHA            0x0A

#define SSL_DHE_RSA_WITH_DES_CBC_SHA             0x15   /**< Weak! Not in TLS 1.2 */
#define SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA        0x16

#define SSL_PSK_WITH_NULL_SHA                    0x2C   /**< Weak! */
#define SSL_DHE_PSK_WITH_NULL_SHA                0x2D   /**< Weak! */
#define SSL_RSA_PSK_WITH_NULL_SHA                0x2E   /**< Weak! */
#define SSL_RSA_WITH_AES_128_CBC_SHA             0x2F

#define SSL_DHE_RSA_WITH_AES_128_CBC_SHA         0x33
#define SSL_RSA_WITH_AES_256_CBC_SHA             0x35
#define SSL_DHE_RSA_WITH_AES_256_CBC_SHA         0x39

#define SSL_RSA_WITH_NULL_SHA256                 0x3B   /**< Weak! */
#define SSL_RSA_WITH_AES_128_CBC_SHA256          0x3C   /**< TLS 1.2 */
#define SSL_RSA_WITH_AES_256_CBC_SHA256          0x3D   /**< TLS 1.2 */

#define SSL_RSA_WITH_CAMELLIA_128_CBC_SHA        0x41
#define SSL_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA    0x45

#define SSL_DHE_RSA_WITH_AES_128_CBC_SHA256      0x67   /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_AES_256_CBC_SHA256      0x6B   /**< TLS 1.2 */

#define SSL_RSA_WITH_CAMELLIA_256_CBC_SHA        0x84
#define SSL_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA    0x88

#define SSL_PSK_WITH_RC4_128_SHA                 0x8A
#define SSL_PSK_WITH_3DES_EDE_CBC_SHA            0x8B
#define SSL_PSK_WITH_AES_128_CBC_SHA             0x8C
#define SSL_PSK_WITH_AES_256_CBC_SHA             0x8D

#define SSL_DHE_PSK_WITH_RC4_128_SHA             0x8E
#define SSL_DHE_PSK_WITH_3DES_EDE_CBC_SHA        0x8F
#define SSL_DHE_PSK_WITH_AES_128_CBC_SHA         0x90
#define SSL_DHE_PSK_WITH_AES_256_CBC_SHA         0x91

#define SSL_RSA_PSK_WITH_RC4_128_SHA             0x92
#define SSL_RSA_PSK_WITH_3DES_EDE_CBC_SHA        0x93
#define SSL_RSA_PSK_WITH_AES_128_CBC_SHA         0x94
#define SSL_RSA_PSK_WITH_AES_256_CBC_SHA         0x95

#define SSL_RSA_WITH_AES_128_GCM_SHA256          0x9C   /**< TLS 1.2 */
#define SSL_RSA_WITH_AES_256_GCM_SHA384          0x9D   /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_AES_128_GCM_SHA256      0x9E   /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_AES_256_GCM_SHA384      0x9F   /**< TLS 1.2 */

#define SSL_PSK_WITH_AES_128_GCM_SHA256          0xA8   /**< TLS 1.2 */
#define SSL_PSK_WITH_AES_256_GCM_SHA384          0xA9   /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_AES_128_GCM_SHA256      0xAA   /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_AES_256_GCM_SHA384      0xAB   /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_AES_128_GCM_SHA256      0xAC   /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_AES_256_GCM_SHA384      0xAD   /**< TLS 1.2 */

#define SSL_PSK_WITH_AES_128_CBC_SHA256          0xAE
#define SSL_PSK_WITH_AES_256_CBC_SHA384          0xAF
#define SSL_PSK_WITH_NULL_SHA256                 0xB0   /**< Weak! */
#define SSL_PSK_WITH_NULL_SHA384                 0xB1   /**< Weak! */

#define SSL_DHE_PSK_WITH_AES_128_CBC_SHA256      0xB2
#define SSL_DHE_PSK_WITH_AES_256_CBC_SHA384      0xB3
#define SSL_DHE_PSK_WITH_NULL_SHA256             0xB4   /**< Weak! */
#define SSL_DHE_PSK_WITH_NULL_SHA384             0xB5   /**< Weak! */

#define SSL_RSA_PSK_WITH_AES_128_CBC_SHA256      0xB6
#define SSL_RSA_PSK_WITH_AES_256_CBC_SHA384      0xB7
#define SSL_RSA_PSK_WITH_NULL_SHA256             0xB8   /**< Weak! */
#define SSL_RSA_PSK_WITH_NULL_SHA384             0xB9   /**< Weak! */

#define SSL_RSA_WITH_CAMELLIA_128_CBC_SHA256     0xBA   /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_CAMELLIA_128_CBC_SHA256 0xBE   /**< TLS 1.2 */

#define SSL_RSA_WITH_CAMELLIA_256_CBC_SHA256     0xC0   /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_CAMELLIA_256_CBC_SHA256 0xC4   /**< TLS 1.2 */

#define SSL_ECDH_ECDSA_WITH_NULL_SHA             0xC001 /**< Weak! */
#define SSL_ECDH_ECDSA_WITH_RC4_128_SHA          0xC002 /**< Not in SSL3! */
#define SSL_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA     0xC003 /**< Not in SSL3! */
#define SSL_ECDH_ECDSA_WITH_AES_128_CBC_SHA      0xC004 /**< Not in SSL3! */
#define SSL_ECDH_ECDSA_WITH_AES_256_CBC_SHA      0xC005 /**< Not in SSL3! */

#define SSL_ECDHE_ECDSA_WITH_NULL_SHA            0xC006 /**< Weak! */
#define SSL_ECDHE_ECDSA_WITH_RC4_128_SHA         0xC007 /**< Not in SSL3! */
#define SSL_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA    0xC008 /**< Not in SSL3! */
#define SSL_ECDHE_ECDSA_WITH_AES_128_CBC_SHA     0xC009 /**< Not in SSL3! */
#define SSL_ECDHE_ECDSA_WITH_AES_256_CBC_SHA     0xC00A /**< Not in SSL3! */

#define SSL_ECDH_RSA_WITH_NULL_SHA               0xC00B /**< Weak! */
#define SSL_ECDH_RSA_WITH_RC4_128_SHA            0xC00C /**< Not in SSL3! */
#define SSL_ECDH_RSA_WITH_3DES_EDE_CBC_SHA       0xC00D /**< Not in SSL3! */
#define SSL_ECDH_RSA_WITH_AES_128_CBC_SHA        0xC00E /**< Not in SSL3! */
#define SSL_ECDH_RSA_WITH_AES_256_CBC_SHA        0xC00F /**< Not in SSL3! */

#define SSL_ECDHE_RSA_WITH_NULL_SHA              0xC010 /**< Weak! */
#define SSL_ECDHE_RSA_WITH_RC4_128_SHA           0xC011 /**< Not in SSL3! */
#define SSL_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA      0xC012 /**< Not in SSL3! */
#define SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA       0xC013 /**< Not in SSL3! */
#define SSL_ECDHE_RSA_WITH_AES_256_CBC_SHA       0xC014 /**< Not in SSL3! */

#define SSL_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256  0xC023 /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384  0xC024 /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_AES_128_CBC_SHA256   0xC025 /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_AES_256_CBC_SHA384   0xC026 /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA256    0xC027 /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_AES_256_CBC_SHA384    0xC028 /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_AES_128_CBC_SHA256     0xC029 /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_AES_256_CBC_SHA384     0xC02A /**< TLS 1.2 */

#define SSL_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256  0xC02B /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_AES_256_GCM_SHA384  0xC02C /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_AES_128_GCM_SHA256   0xC02D /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_AES_256_GCM_SHA384   0xC02E /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_AES_128_GCM_SHA256    0xC02F /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_AES_256_GCM_SHA384    0xC030 /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_AES_128_GCM_SHA256     0xC031 /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_AES_256_GCM_SHA384     0xC032 /**< TLS 1.2 */

#define SSL_ECDHE_PSK_WITH_RC4_128_SHA           0xC033 /**< Not in SSL3! */
#define SSL_ECDHE_PSK_WITH_3DES_EDE_CBC_SHA      0xC034 /**< Not in SSL3! */
#define SSL_ECDHE_PSK_WITH_AES_128_CBC_SHA       0xC035 /**< Not in SSL3! */
#define SSL_ECDHE_PSK_WITH_AES_256_CBC_SHA       0xC036 /**< Not in SSL3! */
#define SSL_ECDHE_PSK_WITH_AES_128_CBC_SHA256    0xC037 /**< Not in SSL3! */
#define SSL_ECDHE_PSK_WITH_AES_256_CBC_SHA384    0xC038 /**< Not in SSL3! */
#define SSL_ECDHE_PSK_WITH_NULL_SHA              0xC039 /**< Weak! No SSL3! */
#define SSL_ECDHE_PSK_WITH_NULL_SHA256           0xC03A /**< Weak! No SSL3! */
#define SSL_ECDHE_PSK_WITH_NULL_SHA384           0xC03B /**< Weak! No SSL3! */

#define SSL_RSA_WITH_ARIA_128_CBC_SHA256         0xC03C /**< TLS 1.2 */
#define SSL_RSA_WITH_ARIA_256_CBC_SHA384         0xC03D /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_ARIA_128_CBC_SHA256     0xC044 /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_ARIA_256_CBC_SHA384     0xC045 /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_ARIA_128_CBC_SHA256 0xC048 /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_ARIA_256_CBC_SHA384 0xC049 /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_ARIA_128_CBC_SHA256  0xC04A /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_ARIA_256_CBC_SHA384  0xC04B /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_ARIA_128_CBC_SHA256   0xC04C /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_ARIA_256_CBC_SHA384   0xC04D /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_ARIA_128_CBC_SHA256    0xC04E /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_ARIA_256_CBC_SHA384    0xC04F /**< TLS 1.2 */
#define SSL_RSA_WITH_ARIA_128_GCM_SHA256         0xC050 /**< TLS 1.2 */
#define SSL_RSA_WITH_ARIA_256_GCM_SHA384         0xC051 /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_ARIA_128_GCM_SHA256     0xC052 /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_ARIA_256_GCM_SHA384     0xC053 /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_ARIA_128_GCM_SHA256 0xC05C /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_ARIA_256_GCM_SHA384 0xC05D /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_ARIA_128_GCM_SHA256  0xC05E /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_ARIA_256_GCM_SHA384  0xC05F /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_ARIA_128_GCM_SHA256   0xC060 /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_ARIA_256_GCM_SHA384   0xC061 /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_ARIA_128_GCM_SHA256    0xC062 /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_ARIA_256_GCM_SHA384    0xC063 /**< TLS 1.2 */
#define SSL_PSK_WITH_ARIA_128_CBC_SHA256         0xC064 /**< TLS 1.2 */
#define SSL_PSK_WITH_ARIA_256_CBC_SHA384         0xC065 /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_ARIA_128_CBC_SHA256     0xC066 /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_ARIA_256_CBC_SHA384     0xC067 /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_ARIA_128_CBC_SHA256     0xC068 /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_ARIA_256_CBC_SHA384     0xC069 /**< TLS 1.2 */
#define SSL_PSK_WITH_ARIA_128_GCM_SHA256         0xC06A /**< TLS 1.2 */
#define SSL_PSK_WITH_ARIA_256_GCM_SHA384         0xC06B /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_ARIA_128_GCM_SHA256     0xC06C /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_ARIA_256_GCM_SHA384     0xC06D /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_ARIA_128_GCM_SHA256     0xC06E /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_ARIA_256_GCM_SHA384     0xC06F /**< TLS 1.2 */
#define SSL_ECDHE_PSK_WITH_ARIA_128_CBC_SHA256   0xC070 /**< TLS 1.2 */
#define SSL_ECDHE_PSK_WITH_ARIA_256_CBC_SHA384   0xC071 /**< TLS 1.2 */

#define SSL_ECDHE_ECDSA_WITH_CAMELLIA_128_CBC_SHA256 0xC072 /**< Not in SSL3! */
#define SSL_ECDHE_ECDSA_WITH_CAMELLIA_256_CBC_SHA384 0xC073 /**< Not in SSL3! */
#define SSL_ECDH_ECDSA_WITH_CAMELLIA_128_CBC_SHA256  0xC074 /**< Not in SSL3! */
#define SSL_ECDH_ECDSA_WITH_CAMELLIA_256_CBC_SHA384  0xC075 /**< Not in SSL3! */
#define SSL_ECDHE_RSA_WITH_CAMELLIA_128_CBC_SHA256   0xC076 /**< Not in SSL3! */
#define SSL_ECDHE_RSA_WITH_CAMELLIA_256_CBC_SHA384   0xC077 /**< Not in SSL3! */
#define SSL_ECDH_RSA_WITH_CAMELLIA_128_CBC_SHA256    0xC078 /**< Not in SSL3! */
#define SSL_ECDH_RSA_WITH_CAMELLIA_256_CBC_SHA384    0xC079 /**< Not in SSL3! */

#define SSL_RSA_WITH_CAMELLIA_128_GCM_SHA256         0xC07A /**< TLS 1.2 */
#define SSL_RSA_WITH_CAMELLIA_256_GCM_SHA384         0xC07B /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_CAMELLIA_128_GCM_SHA256     0xC07C /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_CAMELLIA_256_GCM_SHA384     0xC07D /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_CAMELLIA_128_GCM_SHA256 0xC086 /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_CAMELLIA_256_GCM_SHA384 0xC087 /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_CAMELLIA_128_GCM_SHA256  0xC088 /**< TLS 1.2 */
#define SSL_ECDH_ECDSA_WITH_CAMELLIA_256_GCM_SHA384  0xC089 /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_CAMELLIA_128_GCM_SHA256   0xC08A /**< TLS 1.2 */
#define SSL_ECDHE_RSA_WITH_CAMELLIA_256_GCM_SHA384   0xC08B /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_CAMELLIA_128_GCM_SHA256    0xC08C /**< TLS 1.2 */
#define SSL_ECDH_RSA_WITH_CAMELLIA_256_GCM_SHA384    0xC08D /**< TLS 1.2 */

#define SSL_PSK_WITH_CAMELLIA_128_GCM_SHA256       0xC08E /**< TLS 1.2 */
#define SSL_PSK_WITH_CAMELLIA_256_GCM_SHA384       0xC08F /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_CAMELLIA_128_GCM_SHA256   0xC090 /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_CAMELLIA_256_GCM_SHA384   0xC091 /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_CAMELLIA_128_GCM_SHA256   0xC092 /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_CAMELLIA_256_GCM_SHA384   0xC093 /**< TLS 1.2 */

#define SSL_PSK_WITH_CAMELLIA_128_CBC_SHA256       0xC094
#define SSL_PSK_WITH_CAMELLIA_256_CBC_SHA384       0xC095
#define SSL_DHE_PSK_WITH_CAMELLIA_128_CBC_SHA256   0xC096
#define SSL_DHE_PSK_WITH_CAMELLIA_256_CBC_SHA384   0xC097
#define SSL_RSA_PSK_WITH_CAMELLIA_128_CBC_SHA256   0xC098
#define SSL_RSA_PSK_WITH_CAMELLIA_256_CBC_SHA384   0xC099
#define SSL_ECDHE_PSK_WITH_CAMELLIA_128_CBC_SHA256 0xC09A /**< Not in SSL3! */
#define SSL_ECDHE_PSK_WITH_CAMELLIA_256_CBC_SHA384 0xC09B /**< Not in SSL3! */

#define SSL_RSA_WITH_AES_128_CCM                0xC09C  /**< TLS 1.2 */
#define SSL_RSA_WITH_AES_256_CCM                0xC09D  /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_AES_128_CCM            0xC09E  /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_AES_256_CCM            0xC09F  /**< TLS 1.2 */
#define SSL_RSA_WITH_AES_128_CCM_8              0xC0A0  /**< TLS 1.2 */
#define SSL_RSA_WITH_AES_256_CCM_8              0xC0A1  /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_AES_128_CCM_8          0xC0A2  /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_AES_256_CCM_8          0xC0A3  /**< TLS 1.2 */
#define SSL_PSK_WITH_AES_128_CCM                0xC0A4  /**< TLS 1.2 */
#define SSL_PSK_WITH_AES_256_CCM                0xC0A5  /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_AES_128_CCM            0xC0A6  /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_AES_256_CCM            0xC0A7  /**< TLS 1.2 */
#define SSL_PSK_WITH_AES_128_CCM_8              0xC0A8  /**< TLS 1.2 */
#define SSL_PSK_WITH_AES_256_CCM_8              0xC0A9  /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_AES_128_CCM_8          0xC0AA  /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_AES_256_CCM_8          0xC0AB  /**< TLS 1.2 */
/* The last two are named with PSK_DHE in the RFC, which looks like a typo */

#define SSL_ECDHE_ECDSA_WITH_AES_128_CCM        0xC0AC  /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_AES_256_CCM        0xC0AD  /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_AES_128_CCM_8      0xC0AE  /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_AES_256_CCM_8      0xC0AF  /**< TLS 1.2 */

#define SSL_ECJPAKE_WITH_AES_128_CCM_8          0xC0FF  /**< experimental */

/* RFC 7905 */
#define SSL_ECDHE_RSA_WITH_CHACHA20_POLY1305_SHA256   0xCCA8 /**< TLS 1.2 */
#define SSL_ECDHE_ECDSA_WITH_CHACHA20_POLY1305_SHA256 0xCCA9 /**< TLS 1.2 */
#define SSL_DHE_RSA_WITH_CHACHA20_POLY1305_SHA256     0xCCAA /**< TLS 1.2 */
#define SSL_PSK_WITH_CHACHA20_POLY1305_SHA256         0xCCAB /**< TLS 1.2 */
#define SSL_ECDHE_PSK_WITH_CHACHA20_POLY1305_SHA256   0xCCAC /**< TLS 1.2 */
#define SSL_DHE_PSK_WITH_CHACHA20_POLY1305_SHA256     0xCCAD /**< TLS 1.2 */
#define SSL_RSA_PSK_WITH_CHACHA20_POLY1305_SHA256     0xCCAE /**< TLS 1.2 */

/* RFC 8446 */
#define TLS_AES_128_GCM_SHA256      0x1301  /**< TLS 1.3 */
#define TLS_AES_256_GCM_SHA384      0x1302  /**< TLS 1.3 */
#define TLS_CHACHA20_POLY1305_SHA256      0x1303  /**< TLS 1.3 */
#define TLS_AES_128_CCM_SHA256      0x1304  /**< TLS 1.3 */
#define TLS_AES_128_CCM_8_SHA256     0x1305  /**< TLS 1.3 */


/* Elliptic Curve Groups (ECDHE) */
#define TLS_EC_GROUP_SECP256R1		0x0017
#define TLS_EC_GROUP_SECP384R1		0x0018
#define TLS_EC_GROUP_SECP512R1		0x0019
#define TLS_EC_GROUP_X25519			0x001D
#define TLS_EC_GROUP_X448			0x001E

/* Finite Field Groups (DHE) */
#define TLS_FF_GROUP_FFDHE2048		0x0100
#define TLS_FF_GROUP_FFDHE3072		0x0101
#define TLS_FF_GROUP_FFDHE4096		0x0102
#define TLS_FF_GROUP_FFDHE6144		0x0103
#define TLS_FF_GROUP_FFDHE8192		0x0104

typedef enum {
	CIPHER_STREAM = 0,
	CIPHER_BLOCK = 1,
	CIPHER_AEAD = 2
}CipherType;

typedef enum {
	BULK_NULL = 0,
	BULK_RC4 = 1,
	BULK_RC2 = 2,
	BULK_DES = 3,
	BULK_3DES = 4,
	BULK_DES40 = 5,
	BULK_IDEA = 6,
	BULK_AES = 7
}CipherBulk;

typedef enum { 
	PSK_KE = 0,
	PSK_DHE_KE = 1,
} PskKeyExchangeMode;

typedef enum {
	X509 = 0,
	RawPublicKey = 2,
} CertificateType;

/*TLS1.3 Signature Scheme*/

/* RSASSA-PKCS1-v1_5 algorithms */
#define TLS_ALG_RSA_PKCS1_SHA256		0x0401
#define TLS_ALG_RSA_PKCS1_SHA384		0x0501
#define TLS_ALG_RSA_PKCS1_SHA512		0x0601
/* ECDSA algorithms */
#define TLS_ALG_ECDSA_SECP256R1_SHA256		0x0403
#define TLS_ALG_ECDSA_SECP384R1_SHA384		0x0503
#define TLS_ALG_ECDSA_SECP512R1_SHA512		0x0603
/* RSASSA-PSS algorithms with public key OID rsaEncryption */
#define TLS_ALG_RSA_PSS_RSAE_SHA256		0x0804
#define TLS_ALG_RSA_PSS_RSAE_SHA384		0x0805
#define TLS_ALG_RSA_PSS_RSAE_SHA512		0x0806
/* EdDSA algorithms */
#define TLS_ALG_ED25519		0x0807
#define TLS_ALG_ED448		0x0808
/* RSASSA-PSS algorithms with public key OID RSASSA-PSS */
#define TLS_ALG_RSA_PSS_SHA256		0x0809
#define TLS_ALG_RSA_PSS_SHA384		0x080A
#define TLS_ALG_RSA_PSS_SHA512		0x080B
/* Legacy algorithms */
#define TLS_ALG_RSA_PKCS1_SHA1		0x0201
#define TLS_ALG_ECDSA_SHA1			0x0203


#define SSL_VERIFY_NONE             0
#define SSL_VERIFY_OPTIONAL         1
#define SSL_VERIFY_REQUIRED         2

#define SSL_RSA_PUBLIC				0
#define SSL_RSA_PRIVATE				1



#endif /*XDK_SUPPORT_SOCK*/

#endif /*SSLDEF_H*/