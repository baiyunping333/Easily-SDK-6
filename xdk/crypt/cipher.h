/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc cipher document

	@module	cipher.h | interface file

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
#ifndef CIPHER_H
#define CIPHER_H

#include "../xdkdef.h"

/**
* \brief     Supported cipher types.
*
* \warning   RC4 and DES are considered weak ciphers and their use
*            constitutes a security risk. Arm recommends considering stronger
*            ciphers instead.
*/
typedef enum {
	CIPHER_ID_NONE = 0,  /**< Placeholder to mark the end of cipher ID lists. */
	CIPHER_ID_NULL,      /**< The identity cipher, treated as a stream cipher. */
	CIPHER_ID_AES,       /**< The AES cipher. */
	CIPHER_ID_DES,       /**< The DES cipher. */
	CIPHER_ID_3DES,      /**< The Triple DES cipher. */
	CIPHER_ID_CAMELLIA,  /**< The Camellia cipher. */
	CIPHER_ID_BLOWFISH,  /**< The Blowfish cipher. */
	CIPHER_ID_ARC4,      /**< The RC4 cipher. */
	CIPHER_ID_ARIA,      /**< The Aria cipher. */
	CIPHER_ID_CHACHA20,  /**< The ChaCha20 cipher. */
} cipher_id_t;

/**
* \brief     Supported {cipher type, cipher mode} pairs.
*
* \warning   RC4 and DES are considered weak ciphers and their use
*            constitutes a security risk. Arm recommends considering stronger
*            ciphers instead.
*/
typedef enum {
	CIPHER_NONE = 0,             /**< Placeholder to mark the end of cipher-pair lists. */
	CIPHER_NULL,                 /**< The identity stream cipher. */
	CIPHER_AES_128_ECB,          /**< AES cipher with 128-bit ECB mode. */
	CIPHER_AES_192_ECB,          /**< AES cipher with 192-bit ECB mode. */
	CIPHER_AES_256_ECB,          /**< AES cipher with 256-bit ECB mode. */
	CIPHER_AES_128_CBC,          /**< AES cipher with 128-bit CBC mode. */
	CIPHER_AES_192_CBC,          /**< AES cipher with 192-bit CBC mode. */
	CIPHER_AES_256_CBC,          /**< AES cipher with 256-bit CBC mode. */
	CIPHER_AES_128_CFB128,       /**< AES cipher with 128-bit CFB128 mode. */
	CIPHER_AES_192_CFB128,       /**< AES cipher with 192-bit CFB128 mode. */
	CIPHER_AES_256_CFB128,       /**< AES cipher with 256-bit CFB128 mode. */
	CIPHER_AES_128_CTR,          /**< AES cipher with 128-bit CTR mode. */
	CIPHER_AES_192_CTR,          /**< AES cipher with 192-bit CTR mode. */
	CIPHER_AES_256_CTR,          /**< AES cipher with 256-bit CTR mode. */
	CIPHER_AES_128_GCM,          /**< AES cipher with 128-bit GCM mode. */
	CIPHER_AES_192_GCM,          /**< AES cipher with 192-bit GCM mode. */
	CIPHER_AES_256_GCM,          /**< AES cipher with 256-bit GCM mode. */
	CIPHER_CAMELLIA_128_ECB,     /**< Camellia cipher with 128-bit ECB mode. */
	CIPHER_CAMELLIA_192_ECB,     /**< Camellia cipher with 192-bit ECB mode. */
	CIPHER_CAMELLIA_256_ECB,     /**< Camellia cipher with 256-bit ECB mode. */
	CIPHER_CAMELLIA_128_CBC,     /**< Camellia cipher with 128-bit CBC mode. */
	CIPHER_CAMELLIA_192_CBC,     /**< Camellia cipher with 192-bit CBC mode. */
	CIPHER_CAMELLIA_256_CBC,     /**< Camellia cipher with 256-bit CBC mode. */
	CIPHER_CAMELLIA_128_CFB128,  /**< Camellia cipher with 128-bit CFB128 mode. */
	CIPHER_CAMELLIA_192_CFB128,  /**< Camellia cipher with 192-bit CFB128 mode. */
	CIPHER_CAMELLIA_256_CFB128,  /**< Camellia cipher with 256-bit CFB128 mode. */
	CIPHER_CAMELLIA_128_CTR,     /**< Camellia cipher with 128-bit CTR mode. */
	CIPHER_CAMELLIA_192_CTR,     /**< Camellia cipher with 192-bit CTR mode. */
	CIPHER_CAMELLIA_256_CTR,     /**< Camellia cipher with 256-bit CTR mode. */
	CIPHER_CAMELLIA_128_GCM,     /**< Camellia cipher with 128-bit GCM mode. */
	CIPHER_CAMELLIA_192_GCM,     /**< Camellia cipher with 192-bit GCM mode. */
	CIPHER_CAMELLIA_256_GCM,     /**< Camellia cipher with 256-bit GCM mode. */
	CIPHER_DES_ECB,              /**< DES cipher with ECB mode. */
	CIPHER_DES_CBC,              /**< DES cipher with CBC mode. */
	CIPHER_DES_EDE_ECB,          /**< DES cipher with EDE ECB mode. */
	CIPHER_DES_EDE_CBC,          /**< DES cipher with EDE CBC mode. */
	CIPHER_DES_EDE3_ECB,         /**< DES cipher with EDE3 ECB mode. */
	CIPHER_DES_EDE3_CBC,         /**< DES cipher with EDE3 CBC mode. */
	CIPHER_BLOWFISH_ECB,         /**< Blowfish cipher with ECB mode. */
	CIPHER_BLOWFISH_CBC,         /**< Blowfish cipher with CBC mode. */
	CIPHER_BLOWFISH_CFB64,       /**< Blowfish cipher with CFB64 mode. */
	CIPHER_BLOWFISH_CTR,         /**< Blowfish cipher with CTR mode. */
	CIPHER_ARC4_128,             /**< RC4 cipher with 128-bit mode. */
	CIPHER_AES_128_CCM,          /**< AES cipher with 128-bit CCM mode. */
	CIPHER_AES_192_CCM,          /**< AES cipher with 192-bit CCM mode. */
	CIPHER_AES_256_CCM,          /**< AES cipher with 256-bit CCM mode. */
	CIPHER_CAMELLIA_128_CCM,     /**< Camellia cipher with 128-bit CCM mode. */
	CIPHER_CAMELLIA_192_CCM,     /**< Camellia cipher with 192-bit CCM mode. */
	CIPHER_CAMELLIA_256_CCM,     /**< Camellia cipher with 256-bit CCM mode. */
	CIPHER_ARIA_128_ECB,         /**< Aria cipher with 128-bit key and ECB mode. */
	CIPHER_ARIA_192_ECB,         /**< Aria cipher with 192-bit key and ECB mode. */
	CIPHER_ARIA_256_ECB,         /**< Aria cipher with 256-bit key and ECB mode. */
	CIPHER_ARIA_128_CBC,         /**< Aria cipher with 128-bit key and CBC mode. */
	CIPHER_ARIA_192_CBC,         /**< Aria cipher with 192-bit key and CBC mode. */
	CIPHER_ARIA_256_CBC,         /**< Aria cipher with 256-bit key and CBC mode. */
	CIPHER_ARIA_128_CFB128,      /**< Aria cipher with 128-bit key and CFB-128 mode. */
	CIPHER_ARIA_192_CFB128,      /**< Aria cipher with 192-bit key and CFB-128 mode. */
	CIPHER_ARIA_256_CFB128,      /**< Aria cipher with 256-bit key and CFB-128 mode. */
	CIPHER_ARIA_128_CTR,         /**< Aria cipher with 128-bit key and CTR mode. */
	CIPHER_ARIA_192_CTR,         /**< Aria cipher with 192-bit key and CTR mode. */
	CIPHER_ARIA_256_CTR,         /**< Aria cipher with 256-bit key and CTR mode. */
	CIPHER_ARIA_128_GCM,         /**< Aria cipher with 128-bit key and GCM mode. */
	CIPHER_ARIA_192_GCM,         /**< Aria cipher with 192-bit key and GCM mode. */
	CIPHER_ARIA_256_GCM,         /**< Aria cipher with 256-bit key and GCM mode. */
	CIPHER_ARIA_128_CCM,         /**< Aria cipher with 128-bit key and CCM mode. */
	CIPHER_ARIA_192_CCM,         /**< Aria cipher with 192-bit key and CCM mode. */
	CIPHER_ARIA_256_CCM,         /**< Aria cipher with 256-bit key and CCM mode. */
	CIPHER_AES_128_OFB,          /**< AES 128-bit cipher in OFB mode. */
	CIPHER_AES_192_OFB,          /**< AES 192-bit cipher in OFB mode. */
	CIPHER_AES_256_OFB,          /**< AES 256-bit cipher in OFB mode. */
	CIPHER_AES_128_XTS,          /**< AES 128-bit cipher in XTS block mode. */
	CIPHER_AES_256_XTS,          /**< AES 256-bit cipher in XTS block mode. */
	CIPHER_CHACHA20,             /**< ChaCha20 stream cipher. */
	CIPHER_CHACHA20_POLY1305,    /**< ChaCha20-Poly1305 AEAD cipher. */
} cipher_type_t;

/** Supported cipher modes. */
typedef enum {
	MODE_NONE = 0,               /**< None. */
	MODE_ECB,                    /**< The ECB cipher mode. */
	MODE_CBC,                    /**< The CBC cipher mode. */
	MODE_CFB,                    /**< The CFB cipher mode. */
	MODE_OFB,                    /**< The OFB cipher mode. */
	MODE_CTR,                    /**< The CTR cipher mode. */
	MODE_GCM,                    /**< The GCM cipher mode. */
	MODE_STREAM,                 /**< The stream cipher mode. */
	MODE_CCM,                    /**< The CCM cipher mode. */
	MODE_XTS,                    /**< The XTS cipher mode. */
	MODE_CHACHAPOLY,             /**< The ChaCha-Poly cipher mode. */
} cipher_mode_t;

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif


#endif /* md2.h */
