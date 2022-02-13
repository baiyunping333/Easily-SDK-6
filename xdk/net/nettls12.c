﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@authen ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc ssl document

	@module	netssl.c | implement file

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

#include "netssl.h"

#include "../xdknet.h"
#include "../xdkimp.h"
#include "../xdkoem.h"
#include "../xdkstd.h"

#if defined(XDK_SUPPORT_SOCK)

//DHM serverParams
static char dhm_P[] = "FFFFFFFFFFFFFFFFC90FDAA22168C234C4C6628B80DC1CD1"      \
"29024E088A67CC74020BBEA63B139B22514A08798E3404DD"      \
"EF9519B3CD3A431B302B0A6DF25F14374FE1356D6D51C245"      \
"E485B576625E7EC6F44C42E9A637ED6B0BFF5CB6F406B7ED"      \
"EE386BFB5A899FA5AE9F24117C4B1FE649286651ECE45B3D"      \
"C2007CB8A163BF0598DA48361C55D39A69163FA8FD24CF5F"      \
"83655D23DCA3AD961C62F356208552BB9ED529077096966D"      \
"670C354E4ABC9804F1746C08CA18217C32905E462E36CE3B"      \
"E39E772C180E86039B2783A2EC07A28FB5C55DF06F4C52C9"      \
"DE2BCBF6955817183995497CEA956AE515D2261898FA0510"      \
"15728E5A8AACAA68FFFFFFFFFFFFFFFF";
static char dhm_G[] = "02";

//RSA ServerParams
static char rsa_N[] = "9292758453063D803DD603D5E777D788" \
"8ED1D5BF35786190FA2F23EBC0848AEA" \
"DDA92CA6C3D80B32C4D109BE0F36D6AE" \
"7130B9CED7ACDF54CFC7555AC14EEBAB" \
"93A89813FBF3C4F8066D2D800F7C38A8" \
"1AE31942917403FF4946B0A83D3D3E05" \
"EE57C6F5F5606FB5D4BC6CD34EE0801A" \
"5E94BB77B07507233A0BC7BAC8F90F79";
static char rsa_E[] = "10001";
static char rsa_D[] = "24BF6185468786FDD303083D25E64EFC" \
"66CA472BC44D253102F8B4A9D3BFA750" \
"91386C0077937FE33FA3252D28855837" \
"AE1B484A8A9A45F7EE8C0C634F99E8CD" \
"DF79C5CE07EE72C7F123142198164234" \
"CABB724CF78B8173B9F880FC86322407" \
"AF1FEDFDDE2BEB674CA15F3E81A1521E" \
"071513A1E85B5DFA031F21ECAE91A34D";
static char rsa_P[] = "C36D0EB7FCD285223CFB5AABA5BDA3D8" \
"2C01CAD19EA484A87EA4377637E75500" \
"FCB2005C5C7DD6EC4AC023CDA285D796" \
"C3D9E75E1EFC42488BB4F1D13AC30A57";
static char rsa_Q[] = "C000DF51A7C77AE8D7C7370C1FF55B69" \
"E211C2B9E5DB1ED0BF61D0D9899620F4" \
"910E4168387E3C30AA1E00C339A79508" \
"8452DD96A9A5EA5D9DCA68DA636032AF";

typedef enum {
	ALG_HASH_NONE = 0,
	ALG_HASH_MD5 = 1,
	ALG_HASH_SHA1 = 2,
	ALG_HASH_SHA224 = 3,
	ALG_HASH_SHA256 = 4,
	ALG_HASH_SHA384 = 5,
	ALG_HASH_SHA512 = 6
}HashAlgorithm;

typedef enum {
	ALG_SIGN_NONE = 0,
	ALG_SIGN_RSA = 1,
	ALG_SIGN_DSA = 2,
	ALG_SIGN_ECDSA = 3
}SignatureAlgorithm;

typedef struct _SignHashGroup{
	byte_t alg_hash;
	byte_t alg_sign;
}SignHashGroup;

static SignHashGroup tls12_sign_hash_group[] = {
	{ (byte_t)ALG_HASH_SHA1, (byte_t)ALG_SIGN_RSA },
	{ (byte_t)ALG_HASH_SHA256, (byte_t)ALG_SIGN_RSA },
};

typedef struct _tls12_ciphers_set{
	int cipher;
	int type;
	int bulk;
	int key_size;
	int mac_size;
	int iv_size;
	int iv_impl; //implicit iv
}tls12_ciphers_set;

static tls12_ciphers_set client_ciphers[] = {
	{ SSL_ECDHE_RSA_WITH_AES_128_GCM_SHA256, CIPHER_AEAD, BULK_AES, 16, 0, 12, 4 },
	{ SSL_ECDHE_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16, 0 },
	{ SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 16, 32, 16, 0 },
	{ SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16, 0 },
	{ SSL_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8, 0 },
	{ SSL_DHE_RSA_WITH_AES_256_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 32, 32, 16, 0 },
	{ SSL_DHE_RSA_WITH_AES_128_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 16, 32, 16, 0 },
	{ SSL_DHE_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16, 0 },
	{ SSL_DHE_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16, 0 },
	{ SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8, 0 },
	{ SSL_RSA_WITH_AES_256_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 32, 32, 16, 0 },
	{ SSL_RSA_WITH_AES_128_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 16, 32, 16, 0 },
	{ SSL_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16, 0 },
	{ SSL_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16, 0 },
	{ SSL_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8, 0 },
	{ SSL_RSA_WITH_RC4_128_SHA, CIPHER_STREAM, BULK_RC4, 16, 20, 0, 0 },
	{ SSL_RSA_WITH_RC4_128_MD5, CIPHER_STREAM, BULK_RC4, 16, 16, 0, 0 },
};

static tls12_ciphers_set server_ciphers[] = {
	{ SSL_ECDHE_RSA_WITH_AES_128_GCM_SHA256, CIPHER_AEAD, BULK_AES, 16, 0, 12, 4 },
	{ SSL_ECDHE_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16, 0 },
	{ SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 16, 32, 16, 0 },
	{ SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16, 0 },
	{ SSL_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8, 0 },
	{ SSL_DHE_RSA_WITH_AES_256_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 32, 32, 16, 0 },
	{ SSL_DHE_RSA_WITH_AES_128_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 16, 32, 16, 0 },
	{ SSL_DHE_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16, 0 },
	{ SSL_DHE_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16, 0 },
	{ SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8, 0 },
	{ SSL_RSA_WITH_AES_256_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 32, 32, 16, 0 },
	{ SSL_RSA_WITH_AES_128_CBC_SHA256, CIPHER_BLOCK, BULK_AES, 16, 32, 16, 0 },
	{ SSL_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16, 0 },
	{ SSL_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16, 0 },
	{ SSL_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8, 0 },
	{ SSL_RSA_WITH_RC4_128_SHA, CIPHER_STREAM, BULK_RC4, 16, 20, 0, 0 },
	{ SSL_RSA_WITH_RC4_128_MD5, CIPHER_STREAM, BULK_RC4, 16, 16, 0, 0 },
};

static char label_client_finished[] = "client finished";
static char label_server_finished[] = "server finished";
static char label_master_secret[] = "master secret";
static char label_key_expansion[] = "key expansion";

typedef enum
{
	SSL_HANDSHAKE_ERROR = -1,
	SSL_HELLO_REQUEST = 0,
	SSL_CLIENT_HELLO = 1,
	SSL_SERVER_HELLO = 2,
	SSL_SERVER_HELLO_VERIFY_REQUEST = 3,
	SSL_SERVER_CERTIFICATE = 4,
	SSL_SERVER_KEY_EXCHANGE = 5,
	SSL_CERTIFICATE_REQUEST = 6,
	SSL_SERVER_HELLO_DONE = 7,
	SSL_CLIENT_CERTIFICATE = 8,
	SSL_CLIENT_KEY_EXCHANGE = 9,
	SSL_CERTIFICATE_VERIFY = 10,
	SSL_CLIENT_CHANGE_CIPHER_SPEC = 11,
	SSL_CLIENT_FINISHED = 12,
	SSL_SERVER_CHANGE_CIPHER_SPEC = 13,
	SSL_SERVER_FINISHED = 14,
	SSL_SERVER_EXTENSIONS = 15,
	SSL_HANDSHAKE_OVER = 255
}tls12_handshake_states;

typedef struct _tls12_cipher_context{
	//SecurityParameters
	int endpoint;		//ConnectionEnd: { server, client }
	int alg_prf;	//PRFAlgorithm: enum { tls_prf_sha256 } 
	int cipher_bulk;	//BulkCipherAlgorithm: enum { null, rc4, rc2, des, 3des, des40, idea, aes }
	int cipher_type;	//CipherType: enum { stream, block, aead }
	int cipher;			//the selected cipher
	int key_size;		//the encrypt and decrypt key size
	int key_material_length; //the most material length for key generating
	int exportable;		//IsExportable: { true, false } 
	int alg_mac;		//MACAlgorithm:  enum { null, hmac_md5, hmac_sha1, hmac_sha256, hmac_sha384, hmac_sha512 }
	int mac_size;		//hash length
	int iv_size;		//IV block size
	int iv_implicit;
	int compress_method; //CompressionMethod: { null, (0), (255) }
	int alg_hash;
	int alg_sign;
	int ecp_group;
	int ecp_format;
	byte_t master_secret[SSL_MST_SIZE]; 
	byte_t rnd_srv[SSL_RND_SIZE]; //server_random
	byte_t rnd_cli[SSL_RND_SIZE]; //client_random

	//Generated by SecurityParameters
	byte_t iv_enc[SSL_MAX_IVC]; 
	byte_t iv_dec[SSL_MAX_IVC]; 
	byte_t mac_enc[SSL_MAX_MAC];
	byte_t mac_dec[SSL_MAX_MAC]; 
	dword_t ctx_enc[SSL_CTX_SIZE];
	dword_t ctx_dec[SSL_CTX_SIZE];

	//Tools
	sha256_context sha2;
}tls12_cipher_context;

#define IS_DHE_CIPHER(cipher) ((cipher == SSL_DHE_RSA_WITH_AES_256_CBC_SHA256 || \
								cipher == SSL_DHE_RSA_WITH_AES_256_CBC_SHA || \
								cipher == SSL_DHE_RSA_WITH_AES_128_CBC_SHA256 || \
								cipher == SSL_DHE_RSA_WITH_AES_128_CBC_SHA || \
								cipher == SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA) ? 1 : 0)

#define IS_ECDHE_CIPHER(cipher) ((cipher == SSL_ECDHE_RSA_WITH_AES_128_GCM_SHA256 || \
								cipher == SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA256 || \
								cipher == SSL_ECDHE_RSA_WITH_AES_256_CBC_SHA || \
								cipher == SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA || \
								cipher == SSL_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA)? 1 : 0)

#define IS_AEAD_CIPHER(cipher) ((cipher == SSL_ECDHE_RSA_WITH_AES_128_GCM_SHA256)? 1 : 0)

#define IS_GCM_CIPHER(cipher) ((cipher == SSL_ECDHE_RSA_WITH_AES_128_GCM_SHA256)? 1 : 0)
/***********************************************************************************************************************************/

static void _ssl_set_error(int errcode)
{
	switch (errcode)
	{
	case SSL_ALERT_CLOSE_NOTIFY:
		set_last_error(_T("alert message"), _T("close notity"), -1);
		break;
	case SSL_ALERT_UNEXPECTED_MESSAGE:
		set_last_error(_T("alert message"), _T("unexpected message"), -1);
		break;
	case SSL_ALERT_BAD_RECORD_MAC:
		set_last_error(_T("alert message"), _T("bad record mac"), -1);
		break;
	case SSL_ALERT_DECRYPTION_FAILED:
		set_last_error(_T("alert message"), _T("decryption failed"), -1);
		break;
	case SSL_ALERT_RECORD_OVERFLOW:
		set_last_error(_T("alert message"), _T("record overflow"), -1);
		break;
	case SSL_ALERT_DECOMPRESSION_FAILURE:
		set_last_error(_T("alert message"), _T("decompression failure"), -1);
		break;
	case SSL_ALERT_HANDSHAKE_FAILURE:
		set_last_error(_T("alert message"), _T("handshake failure"), -1);
		break;
	case SSL_ALERT_NO_CERTIFICATE:
		set_last_error(_T("alert message"), _T("no certificate"), -1);
		break;
	case SSL_ALERT_BAD_CERTIFICATE:
		set_last_error(_T("alert message"), _T("bad certificate"), -1);
		break;
	case SSL_ALERT_UNSUPPORTED_CERTIFICATE:
		set_last_error(_T("alert message"), _T("unsupported certificate"), -1);
		break;
	case SSL_ALERT_CERTIFICATE_REVOKED:
		set_last_error(_T("alert message"), _T("certiticate revoked"), -1);
		break;
	case SSL_ALERT_CERTIFICATE_EXPIRED:
		set_last_error(_T("alert message"), _T("certificate expired"), -1);
		break;
	case SSL_ALERT_CERTIFICATE_UNKNOWN:
		set_last_error(_T("alert message"), _T("certificate unknown"), -1);
		break;
	case SSL_ALERT_ILLEGAL_PARAMETER:
		set_last_error(_T("alert message"), _T("illegal parameter"), -1);
		break;
	case SSL_ALERT_UNKNOWN_CA:
		set_last_error(_T("alert message"), _T("unknown ca"), -1);
		break;
	case SSL_ALERT_ACCESS_DENIED:
		set_last_error(_T("alert message"), _T("access denied"), -1);
		break;
	case SSL_ALERT_DECRYPT_ERROR:
		set_last_error(_T("alert message"), _T("decrypt error"), -1);
		break;
	case SSL_ALERT_EXPORT_RESTRICTION:
		set_last_error(_T("alert message"), _T("export restriction"), -1);
		break;
	case SSL_ALERT_PROTOCOL_VERSION:
		set_last_error(_T("alert message"), _T("protocol version"), -1);
		break;
	case SSL_ALERT_INSUFFICIENT_SECURITY:
		set_last_error(_T("alert message"), _T("insufficient security"), -1);
		break;
	case SSL_ALERT_INTERNAL_ERROR:
		set_last_error(_T("alert message"), _T("internal error"), -1);
		break;
	case SSL_ALERT_USER_CANCELED:
		set_last_error(_T("alert message"), _T("user canceled"), -1);
		break;
	case SSL_ALERT_NO_RENEGOTIATION:
		set_last_error(_T("alert message"), _T("no renegotiation"), -1);
		break;
	}
}

static bool_t _ssl_choose_cipher(tls12_cipher_context* pcip, int ciph)
{
	int i, n;
	tls12_ciphers_set* pcs;

	if (pcip->endpoint == SSL_TYPE_CLIENT)
	{
		n = sizeof(client_ciphers) / sizeof(tls12_ciphers_set);
		pcs = client_ciphers;
	}
	else
	{
		n = sizeof(server_ciphers) / sizeof(tls12_ciphers_set);
		pcs = server_ciphers;
	}

	for (i = 0; i < n; i++)
	{
		if (ciph == pcs[i].cipher)
		{
			pcip->cipher = pcs[i].cipher;
			pcip->cipher_type = pcs[i].type;
			pcip->cipher_bulk = pcs[i].bulk;
			pcip->key_size = pcs[i].key_size;
			pcip->mac_size = pcs[i].mac_size;
			pcip->iv_size = pcs[i].iv_size;
			pcip->iv_implicit = pcs[i].iv_impl;

			return 1;
		}
	}

	set_last_error(_T("_ssl_choose_cipher"), _T("unknown cipher"), -1);

	return 0;
}

static void _ssl_derive_keys(tls12_cipher_context* pcip, byte_t* premaster, int prelen)
{
	byte_t rndb[SSL_RND_SIZE * 2] = { 0 };
	byte_t keyblk[SSL_BLK_SIZE] = { 0 };

	byte_t *key_enc, *key_dec;
	int iv_copy;

	//generating master security
	xmem_copy((void*)rndb, (void*)pcip->rnd_cli, SSL_RND_SIZE);
	xmem_copy((void*)(rndb + SSL_RND_SIZE), (void*)pcip->rnd_srv, SSL_RND_SIZE);

	ssl_prf2(premaster, prelen, label_master_secret, rndb, SSL_RND_SIZE * 2, pcip->master_secret, SSL_MST_SIZE);

	// swap the client and server random values.
	xmem_copy((void*)rndb, (void*)pcip->rnd_srv, SSL_RND_SIZE);
	xmem_copy((void*)(rndb + SSL_RND_SIZE), (void*)pcip->rnd_cli, SSL_RND_SIZE);

	xmem_zero(pcip->rnd_cli, sizeof(pcip->rnd_cli));
	xmem_zero(pcip->rnd_srv, sizeof(pcip->rnd_srv));

	// generate key block
	//key_block = 
	//PRF(SecurityParameters.master_secret,
	//"key expansion",
	//SecurityParameters.server_random +
	//SecurityParameters.client_random);
	ssl_prf2(pcip->master_secret, SSL_MST_SIZE, label_key_expansion, rndb, SSL_RND_SIZE * 2, keyblk, SSL_BLK_SIZE);

	if (IS_AEAD_CIPHER(pcip->cipher))
		iv_copy = (pcip->iv_implicit) ? pcip->iv_implicit : pcip->iv_size;
	else
		iv_copy = pcip->iv_size;

	//the key_block is partitioned as follows:
	//client_write_MAC_secret[SecurityParameters.hash_size]
	//server_write_MAC_secret[SecurityParameters.hash_size]
	//client_write_key[SecurityParameters.key_material_length]
	//server_write_key[SecurityParameters.key_material_length]
	//client_write_IV[SecurityParameters.IV_size]
	//server_write_IV[SecurityParameters.IV_size]
	if (pcip->endpoint == SSL_TYPE_CLIENT)
	{
		//client_write_MAC_secret for client encrypting record
		xmem_copy(pcip->mac_enc, keyblk, pcip->mac_size);
		//server_write_MAC_secret for client decrypting record
		xmem_copy(pcip->mac_dec, keyblk + pcip->mac_size, pcip->mac_size);
		//client_write_key for client setup encrypting context
		key_enc = keyblk + pcip->mac_size * 2;
		//server_write_key for client setup decrypting context
		key_dec = keyblk + pcip->mac_size * 2 + pcip->key_size;
		//client_write_IV for client encrypting IV
		xmem_copy(pcip->iv_enc, keyblk + pcip->mac_size * 2 + pcip->key_size * 2, iv_copy);
		//server_write_IV for client decrypting IV
		xmem_copy(pcip->iv_dec, keyblk + pcip->mac_size * 2 + pcip->key_size * 2 + iv_copy, iv_copy);
	}
	else
	{
		//client_write_MAC_secret for server decrypting record
		xmem_copy(pcip->mac_dec, keyblk, pcip->mac_size);
		//server_write_MAC_secret for server encrypting record
		xmem_copy(pcip->mac_enc, keyblk + pcip->mac_size, pcip->mac_size);
		//client_write_key for server decrypting context
		key_dec = keyblk + pcip->mac_size * 2;
		//server_write_key for server encrypting context
		key_enc = keyblk + pcip->mac_size * 2 + pcip->key_size;
		//client_write_IV for server decrypting IV
		xmem_copy(pcip->iv_dec, keyblk + pcip->mac_size * 2 + pcip->key_size * 2, iv_copy);
		//server_write_IV for server encrypting IV
		xmem_copy(pcip->iv_enc, keyblk + pcip->mac_size * 2 + pcip->key_size * 2 + iv_copy, iv_copy);
	}

	//initialize encrypt and decrypt context
	switch (pcip->cipher)
	{
	case SSL_RSA_WITH_RC4_128_MD5:
	case SSL_RSA_WITH_RC4_128_SHA:
		arc4_setup((arc4_context *)pcip->ctx_enc, key_enc, pcip->key_size); //the material size is bytes
		arc4_setup((arc4_context *)pcip->ctx_dec, key_dec, pcip->key_size); //the material size is bytes
		break;
	case SSL_RSA_WITH_3DES_EDE_CBC_SHA:
	case SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
	case SSL_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA:
		des3_set3key_enc((des3_context *)pcip->ctx_enc, key_enc); //the material size is 24 bytes
		des3_set3key_dec((des3_context *)pcip->ctx_dec, key_dec); //the material size is 24 bytes
		break;
	case SSL_RSA_WITH_AES_128_CBC_SHA:
	case SSL_RSA_WITH_AES_256_CBC_SHA:
	case SSL_RSA_WITH_AES_128_CBC_SHA256:
	case SSL_RSA_WITH_AES_256_CBC_SHA256:
	case SSL_DHE_RSA_WITH_AES_128_CBC_SHA:
	case SSL_DHE_RSA_WITH_AES_256_CBC_SHA:
	case SSL_DHE_RSA_WITH_AES_256_CBC_SHA256:
	case SSL_DHE_RSA_WITH_AES_128_CBC_SHA256:
	case SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA256:
	case SSL_ECDHE_RSA_WITH_AES_256_CBC_SHA:
	case SSL_ECDHE_RSA_WITH_AES_128_CBC_SHA:
		aes_setkey_enc((aes_context *)pcip->ctx_enc, key_enc, (pcip->key_size * 8)); //the material size is bits
		aes_setkey_dec((aes_context *)pcip->ctx_dec, key_dec, (pcip->key_size * 8)); //the material size is bits
		break;
	case SSL_ECDHE_RSA_WITH_AES_128_GCM_SHA256:
		gcm_setkey((gcm_context *)pcip->ctx_enc, key_enc, (pcip->key_size * 8));
		gcm_setkey((gcm_context *)pcip->ctx_dec, key_dec, (pcip->key_size * 8));
		break;
	}
}

static int _ssl_encrypt_snd_msg(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int i, padlen;
	byte_t* mac_buf;
	byte_t iv_pre[16] = { 0 };
	int iv_copy;

	//byte_t gcm_add[13] = { 0 };
	//byte_t gcm_iv[12] = { 0 };
	//int addlen;
	int taglen;

	mac_buf = prec->snd_msg + prec->snd_msg_len;

	if (pcip->cipher_type == CIPHER_STREAM || pcip->cipher_type == CIPHER_BLOCK)
	{
		//The MAC is generated as:
		//HMAC_hash(MAC_write_secret, seq_num + TLSCompressed.type + TLSCompressed.version + TLSCompressed.length + TLSCompressed.fragment));

		if (pcip->mac_size == 16)
		{
			md5_hmac(pcip->mac_enc, pcip->mac_size, prec->snd_ctr, prec->snd_msg_len + SSL_CTR_SIZE + SSL_HDR_SIZE, mac_buf);
		}
		else if (pcip->mac_size == 20)
		{
			sha1_hmac(pcip->mac_enc, pcip->mac_size, prec->snd_ctr, prec->snd_msg_len + SSL_CTR_SIZE + SSL_HDR_SIZE, mac_buf);
		}
		else if (pcip->mac_size == 32) //TLS 1.2
		{
			sha256_hmac(pcip->mac_enc, pcip->mac_size, prec->snd_ctr, prec->snd_msg_len + SSL_CTR_SIZE + SSL_HDR_SIZE, mac_buf, 0);
		}
		else
		{
			set_last_error(_T("_ssl_encrypt_snd_msg"), _T("unknown hmac function"), -1);

			return C_ERR;
		}

		prec->snd_msg_len += pcip->mac_size;
	}

	if (pcip->cipher_type == CIPHER_STREAM)
	{
		/*
		stream-ciphered struct {
		opaque content[TLSCompressed.length];
		opaque MAC[SecurityParameters.mac_length];
		} GenericStreamCipher;
		*/
		if (pcip->cipher_bulk == BULK_RC4)
		{
			arc4_crypt((arc4_context *)pcip->ctx_enc, prec->snd_msg_len, prec->snd_msg, prec->snd_msg);
		}
	}
	else if (pcip->cipher_type == CIPHER_BLOCK)
	{
		padlen = pcip->iv_size - (prec->snd_msg_len + 1) % pcip->iv_size;
		if (padlen == pcip->iv_size)
			padlen = 0;

		padlen++;
		for (i = 0; i < padlen; i++)
			prec->snd_msg[prec->snd_msg_len + i] = (byte_t)(padlen - 1);

		prec->snd_msg_len += (padlen);

		/*
		block-ciphered struct {
		opaque IV[CipherSpec.block_length];
		opaque content[TLSCompressed.length];
		opaque MAC[CipherSpec.hash_size];
		uint8 padding[GenericBlockCipher.padding_length];
		uint8 padding_length;
		} GenericBlockCipher;
		*/

		iv_copy = pcip->iv_size;
		xmem_copy((void*)(iv_pre), (void*)(pcip->iv_enc), iv_copy);

		if (pcip->cipher_bulk == BULK_3DES)
		{
			des3_crypt_cbc((des3_context *)pcip->ctx_enc, DES_ENCRYPT, prec->snd_msg_len, pcip->iv_enc, prec->snd_msg, prec->snd_msg);
		}
		else if (pcip->cipher_bulk == BULK_AES)
		{
			aes_crypt_cbc((aes_context *)pcip->ctx_enc, AES_ENCRYPT, prec->snd_msg_len, pcip->iv_enc, prec->snd_msg, prec->snd_msg);
		}

		xmem_move((void*)(prec->snd_msg), prec->snd_msg_len, iv_copy);
		xmem_copy((void*)(prec->snd_msg), (void*)(iv_pre), iv_copy);
		prec->snd_msg_len += iv_copy;
	}
	else if (pcip->cipher_type == CIPHER_AEAD)
	{
		/*
		struct {
		opaque nonce_explicit[SecurityParameters.record_iv_length];
		aead-ciphered struct {
		opaque content[TLSCompressed.length];
		};
		} GenericAEADCipher;
		*/

		if (pcip->iv_implicit)
		{
			xmem_copy((void*)(pcip->iv_enc + pcip->iv_implicit), (void*)prec->snd_ctr, (pcip->iv_size - pcip->iv_implicit));
		}
		iv_copy = (pcip->iv_implicit) ? (pcip->iv_size - pcip->iv_implicit) : pcip->iv_size;
		xmem_copy((void*)(iv_pre), (void*)(pcip->iv_enc + pcip->iv_implicit), iv_copy);

		if (IS_GCM_CIPHER(pcip->cipher))
		{
			/*
			addlen = 0;
			xmem_copy((void*)gcm_add, prec->snd_ctr, SSL_CTR_SIZE);
			addlen += SSL_CTR_SIZE;
			gcm_add[addlen] = prec->snd_msg_type;
			gcm_add[addlen + 1] = (byte_t)(pses->major_ver);
			gcm_add[addlen + 2] = (byte_t)(pses->minor_ver);
			addlen += 3;
			PUT_SWORD_NET(gcm_add, addlen, (unsigned short)prec->snd_msg_len);
			addlen += 2;

			xmem_copy((void*)gcm_iv, (void*)pssl->iv_enc, pssl->iv_size);
			*/

			taglen = 16;
			mac_buf = prec->snd_msg + prec->snd_msg_len;

			//if (C_OK != gcm_crypt_and_tag((gcm_context *)pssl->ctx_enc, AES_ENCRYPT, prec->snd_msg_len, gcm_iv, pssl->iv_size, gcm_add, addlen, prec->snd_msg, prec->snd_msg, taglen, mac_buf))
			if (C_OK != gcm_crypt_and_tag((gcm_context *)pcip->ctx_enc, AES_ENCRYPT, prec->snd_msg_len, pcip->iv_enc, pcip->iv_size, prec->snd_ctr, (SSL_CTR_SIZE + SSL_HDR_SIZE), prec->snd_msg, prec->snd_msg, taglen, mac_buf))
			{
				set_last_error(_T("_ssl_encrypt_snd_msg"), _T("gcm_crypt_and_tag falied"), -1);

				return C_ERR;
			}
			prec->snd_msg_len += taglen;
		}

		xmem_move((void*)(prec->snd_msg), prec->snd_msg_len, iv_copy);
		xmem_copy((void*)(prec->snd_msg), (void*)(iv_pre), iv_copy);
		prec->snd_msg_len += iv_copy;
	}
	else
	{
		set_last_error(_T("_ssl_encrypt_snd_msg"), _T("unknown crypt cipher"), -1);

		return C_ERR;
	}

	//reset message length
	PUT_SWORD_NET(prec->snd_hdr, 3, (unsigned short)prec->snd_msg_len);

	return C_OK;
}

static int _ssl_decrypt_rcv_msg(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int i, n, padlen = 0;
	byte_t* mac_buf;
	byte_t mac_tmp[32];
	int iv_copy;

	//byte_t gcm_add[13] = { 0 };
	//byte_t gcm_iv[12] = { 0 };
	//int addlen;
	int taglen = 16;

	if (pcip->cipher_type == CIPHER_AEAD)
		iv_copy = (pcip->iv_implicit) ? (pcip->iv_size - pcip->iv_implicit) : pcip->iv_size;
	else
		iv_copy = pcip->iv_size;

	if (pcip->cipher_type == CIPHER_AEAD)
		n = taglen + iv_copy;
	else
		n = pcip->mac_size + iv_copy;

	if (prec->rcv_msg_len < n)
	{
		set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("message length to small"), -1);

		return C_ERR;
	}

	if (pcip->cipher_type == CIPHER_STREAM)
	{
		/*
		stream-ciphered struct {
		opaque content[TLSCompressed.length];
		opaque MAC[SecurityParameters.mac_length];
		} GenericStreamCipher;
		*/
		if (pcip->cipher_bulk == BULK_RC4)
		{
			arc4_crypt((arc4_context *)pcip->ctx_dec, prec->rcv_msg_len, prec->rcv_msg, prec->rcv_msg);
		}
	}
	else if (pcip->cipher_type == CIPHER_BLOCK)
	{
		/*
		block-ciphered struct {
		opaque IV[CipherSpec.block_length];
		opaque content[TLSCompressed.length];
		opaque MAC[CipherSpec.hash_size];
		uint8 padding[GenericBlockCipher.padding_length];
		uint8 padding_length;
		} GenericBlockCipher;
		*/
		xmem_copy((void*)(pcip->iv_dec), (void*)prec->rcv_msg, pcip->iv_size);
		xmem_move((void*)(prec->rcv_msg + pcip->iv_size), prec->rcv_msg_len - pcip->iv_size, 0 - pcip->iv_size);
		prec->rcv_msg_len -= pcip->iv_size;

		if (prec->rcv_msg_len % pcip->iv_size != 0)
		{
			set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("message length not multiple of IV size"), -1);

			return C_ERR;
		}

		if (pcip->cipher_bulk == BULK_3DES)
		{
			des3_crypt_cbc((des3_context *)pcip->ctx_dec, DES_DECRYPT, prec->rcv_msg_len, pcip->iv_dec, prec->rcv_msg, prec->rcv_msg);
		}
		else if (pcip->cipher_bulk == BULK_AES)
		{
			aes_crypt_cbc((aes_context *)pcip->ctx_dec, AES_DECRYPT, prec->rcv_msg_len, pcip->iv_dec, prec->rcv_msg, prec->rcv_msg);
		}

		padlen = prec->rcv_msg[prec->rcv_msg_len - 1] + 1;
		for (i = 1; i <= padlen; i++)
		{
			if (prec->rcv_msg[prec->rcv_msg_len - i] != (padlen - 1))
			{
				padlen = 0;
			}
		}

		if (pcip->iv_size != 0 && padlen == 0)
		{
			set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("invalid message pading length"), -1);

			return C_ERR;
		}

		prec->rcv_msg_len -= padlen;
	}
	else if (pcip->cipher_type == CIPHER_AEAD)
	{
		/*
		struct {
		opaque nonce_explicit[SecurityParameters.record_iv_length];
		aead-ciphered struct {
		opaque content[TLSCompressed.length];
		};
		} GenericAEADCipher;
		*/

		xmem_copy((void*)(pcip->iv_dec + pcip->iv_implicit), (void*)prec->rcv_msg, iv_copy);
		xmem_move((void*)(prec->rcv_msg + iv_copy), prec->rcv_msg_len - iv_copy, 0 - iv_copy);
		prec->rcv_msg_len -= iv_copy;

		if (IS_GCM_CIPHER(pcip->cipher))
		{
			taglen = 16;
			prec->rcv_msg_len -= taglen;
			mac_buf = prec->rcv_msg + prec->rcv_msg_len;

			//reset message length
			PUT_SWORD_NET(prec->rcv_hdr, 3, (unsigned short)prec->rcv_msg_len);

			/*
			addlen = 0;
			xmem_copy((void*)gcm_add, prec->rcv_ctr, SSL_CTR_SIZE);
			addlen += SSL_CTR_SIZE;
			gcm_add[addlen] = prec->rcv_msg_type;
			gcm_add[addlen + 1] = (byte_t)(pses->major_ver);
			gcm_add[addlen + 2] = (byte_t)(pses->minor_ver);
			addlen += 3;
			PUT_SWORD_NET(gcm_add, addlen, (unsigned short)(prec->rcv_msg_len));
			addlen += 2;

			xmem_copy((void*)gcm_iv, (void*)pssl->iv_dec, pssl->iv_size);
			*/

			//if (C_OK != gcm_crypt_and_tag((gcm_context *)pcip->ctx_dec, AES_DECRYPT, prec->rcv_msg_len, gcm_iv, pcip->iv_size, gcm_add, addlen, prec->rcv_msg, prec->rcv_msg, taglen, mac_tmp))
			if (C_OK != gcm_crypt_and_tag((gcm_context *)pcip->ctx_dec, AES_DECRYPT, prec->rcv_msg_len, pcip->iv_dec, pcip->iv_size, prec->rcv_ctr, (SSL_CTR_SIZE + SSL_HDR_SIZE), prec->rcv_msg, prec->rcv_msg, taglen, mac_tmp))
			{
				set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("gcm_crypt_and_tag falied"), -1);

				return C_ERR;
			}

			if (xmem_comp((void*)mac_tmp, (void*)mac_buf, taglen) != 0)
			{
				set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("gcm tag checked falied"), -1);

				return C_ERR;
			}
		}
	}
	else
	{
		set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("unknown crypt cipher"), -1);

		return C_ERR;
	}

	if (pcip->cipher_type == CIPHER_STREAM || pcip->cipher_type == CIPHER_BLOCK)
	{
		prec->rcv_msg_len -= pcip->mac_size;
		mac_buf = prec->rcv_msg + prec->rcv_msg_len;

		//reset message length
		PUT_SWORD_NET(prec->rcv_hdr, 3, (unsigned short)prec->rcv_msg_len);

		if (pcip->mac_size == 16)
			md5_hmac(pcip->mac_dec, pcip->mac_size, prec->rcv_ctr, (prec->rcv_msg_len + SSL_CTR_SIZE + SSL_HDR_SIZE), mac_tmp);
		else if (pcip->mac_size == 20)
			sha1_hmac(pcip->mac_dec, pcip->mac_size, prec->rcv_ctr, (prec->rcv_msg_len + SSL_CTR_SIZE + SSL_HDR_SIZE), mac_tmp);
		else if (pcip->mac_size == 32)
			sha256_hmac(pcip->mac_dec, pcip->mac_size, prec->rcv_ctr, (prec->rcv_msg_len + SSL_CTR_SIZE + SSL_HDR_SIZE), mac_tmp, 0);

		if (xmem_comp((void*)mac_tmp, (void*)mac_buf, pcip->mac_size) != 0)
		{
			set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("message signature hash not matched"), -1);

			return C_ERR;
		}
	}

	return C_OK;
}

static int _ssl_write_snd_msg(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	dword_t dw;
	int i, haslen;
	byte_t* token;
	int total;

	/*
	struct {
       ContentType type;
       ProtocolVersion version;
       uint16 length;
       select (SecurityParameters.cipher_type) {
           case stream: GenericStreamCipher;
           case block:  GenericBlockCipher;
           case aead:   GenericAEADCipher;
       } fragment;
   } TLSCiphertext;
	*/
	PUT_BYTE(prec->snd_hdr, 0, (byte_t)(prec->snd_msg_type));
	PUT_BYTE(prec->snd_hdr, 1, (byte_t)(pses->major_ver));
	PUT_BYTE(prec->snd_hdr, 2, (byte_t)(pses->minor_ver));
	PUT_SWORD_NET(prec->snd_hdr, 3, (unsigned short)(prec->snd_msg_len));

	//will hash all handshake message sended
	if (prec->snd_msg_type == SSL_MSG_HANDSHAKE)
	{
		token = prec->snd_msg;
		total = prec->snd_msg_len;
		while (total)
		{
			haslen = GET_THREEBYTE_NET(token, 1);

			sha256_update(&pcip->sha2, token, SSL_HSH_SIZE + haslen);

			total -= (SSL_HSH_SIZE + haslen);
			token += (SSL_HSH_SIZE + haslen);
		}
	}

	if (prec->crypted)
	{
		if (C_OK != _ssl_encrypt_snd_msg(pssl))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_UNKNOWN_ERROR);
			set_last_error(_T("_ssl_write_snd_msg"), _T("encrypt message block failed"), -1);
			return C_ERR;
		}

		//incre send message control bits
		for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
		{
			if (++prec->snd_ctr[i] != 0)
				break;
		}
	}

	dw = SSL_HDR_SIZE + prec->snd_msg_len;

	if (!(*pssl->pif->pf_write)(pssl->pif->fd, prec->snd_hdr, &dw))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNKNOWN_ERROR);
		set_last_error(_T("_ssl_write_snd_msg"), _T("write message block failed"), -1);
		return C_ERR;
	}

	prec->snd_msg_pop = 0;

	return C_OK;
}

static int _ssl_read_rcv_msg(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	dword_t dw;
	int i, haslen;
	byte_t* token;
	int total;

	/*
	struct {
       ContentType type;
       ProtocolVersion version;
       uint16 length;
       select (SecurityParameters.cipher_type) {
           case stream: GenericStreamCipher;
           case block:  GenericBlockCipher;
           case aead:   GenericAEADCipher;
       } fragment;
   } TLSCiphertext;
	*/
	//if the head already readed at handshake begain
	if (pses->major_ver)
	{
		dw = SSL_HDR_SIZE;
		if (!(pssl->pif->pf_read)(pssl->pif->fd, prec->rcv_hdr, &dw))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_UNKNOWN_ERROR);
			set_last_error(_T("_ssl_read_rcv_msg"), _T("read message head failed"), -1);
			return C_ERR;
		}

		if (!dw)
		{
			xmem_zero((void*)prec->rcv_hdr, SSL_HDR_SIZE);
			return C_OK;
		}

		if (prec->rcv_hdr[1] != SSL_MAJOR_VERSION_3)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
			set_last_error(_T("_ssl_read_rcv_msg"), _T("major version mismatch"), -1);
			return C_ERR;
		}

		if (prec->rcv_hdr[2] > SSL_MINOR_VERSION_3)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
			set_last_error(_T("_ssl_read_rcv_msg"), _T("minor version mismatch"), -1);
			return C_ERR;
		}

		prec->rcv_msg_type = GET_BYTE(prec->rcv_hdr, 0);
		prec->rcv_msg_len = GET_SWORD_NET(prec->rcv_hdr, 3);

		if (prec->rcv_msg_len < 1 || prec->rcv_msg_len > SSL_MAX_SIZE)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
			set_last_error(_T("_ssl_read_rcv_msg"), _T("invalid message block length"), -1);
			return C_ERR;
		}

		dw = prec->rcv_msg_len;
		if (!(*pssl->pif->pf_read)(pssl->pif->fd, prec->rcv_msg, &dw))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_UNKNOWN_ERROR);
			set_last_error(_T("_ssl_read_rcv_msg"), _T("read message block failed"), -1);
			return C_ERR;
		}
	}

	if (prec->crypted)
	{
		if (C_OK != _ssl_decrypt_rcv_msg(pssl))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_DECRYPT_ERROR);
			set_last_error(_T("_ssl_read_rcv_msg"), _T("decrypt message block failed"), -1);
			return C_ERR;
		}

		//incre recv message control bits
		for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
		{
			if (++prec->rcv_ctr[i] != 0)
				break;
		}
	}

	//will hash all handshake message received
	if (prec->rcv_msg_type == SSL_MSG_HANDSHAKE)
	{
		total = prec->rcv_msg_len;
		token = prec->rcv_msg;
		while (total)
		{
			haslen = GET_THREEBYTE_NET(token, 1);

			sha256_update(&pcip->sha2, token, SSL_HSH_SIZE + haslen);

			total -= (SSL_HSH_SIZE + haslen);
			token += (SSL_HSH_SIZE + haslen);
		}
	}
	else if (prec->rcv_msg_type == SSL_MSG_ALERT)
	{
		if (prec->rcv_msg[0] == SSL_LEVEL_FATAL)
		{
			_ssl_set_error(prec->rcv_msg[1]);
			return C_ERR;
		}

		if (prec->rcv_msg[0] == SSL_LEVEL_WARNING && prec->rcv_msg[1] == SSL_ALERT_CLOSE_NOTIFY)
		{
			pses->handshake_over = -1;
			prec->rcv_msg_len = 0;
		}
	}

	prec->rcv_msg_pop = 0;

	return C_OK;
}

static int _ssl_write_alert(ssl_context* pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	prec->snd_msg_type = SSL_MSG_ALERT;
	prec->snd_msg_len = 2;
	prec->snd_msg[0] = SSL_LEVEL_FATAL;
	prec->snd_msg[1] = pses->alert_code;

	return _ssl_write_snd_msg(pssl);
}


static void _ssl_free_cipher(ssl_session_context* pses)
{
	tls12_cipher_context* pcip = (pses) ? (tls12_cipher_context*)pses->cipher_context : NULL;

	if (pcip)
	{
		xmem_free(pcip);
		pses->cipher_context = NULL;
	}
}

static void _ssl_alloc_cipher(ssl_context* pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip;

	pcip = (tls12_cipher_context*)xmem_alloc(sizeof(tls12_cipher_context));
	//initialize tools
	sha256_starts(&pcip->sha2, 0);

	pcip->endpoint = pssl->type;

	pses->cipher_context = (ssl_cipher_context_ptr)pcip;

	pssl->ssl_send = _ssl_write_snd_msg;
	pssl->ssl_recv = _ssl_read_rcv_msg;

	pses->free_cipher_context = _ssl_free_cipher;

	if (pssl->type == SSL_TYPE_SERVER)
	{
		pssl->srv_major_ver = SSL_MAJOR_VERSION_3;
		pssl->srv_minor_ver = SSL_MINOR_VERSION_3;
	}
}

/***************************************client routing************************************************************/

static tls12_handshake_states _ssl_write_client_hello(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int i, n, pos, extlen, len;
	int msglen;
	dword_t t;

	TRY_CATCH;

	//gen client random bytes
	t = get_times();
	PUT_DWORD_NET(pcip->rnd_cli, 0, t);

	/*for (i = 4; i < SSL_RND_SIZE; i++)
	{
		pcip->rnd_cli[i] = (byte_t)havege_rand(&pcip->rng);
	}*/
	(*pssl->f_rng)(pssl->r_rng, (pcip->rnd_cli + 4), (SSL_RND_SIZE - 4));

	msglen = SSL_HSH_SIZE;

	/*
	struct {
          ProtocolVersion client_version;
          Random random;
          SessionID session_id;
          CipherSuite cipher_suites<2..2^16-2>;
          CompressionMethod compression_methods<1..2^8-1>;
          select (extensions_present) {
              case false:
                  struct {};
              case true:
                  Extension extensions<0..2^16-1>;
          };
      } ClientHello;
	*/

	/*
	ProtocolVersion
	*/
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pssl->cli_major_ver));
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pssl->cli_minor_ver));
	
	/*
	Random
	*/
	xmem_copy(prec->snd_msg + msglen, pcip->rnd_cli, SSL_RND_SIZE);
	msglen += SSL_RND_SIZE;

	/*
	SessionID
	*/
	n = pses->session_size;
	if (n < 16 || n > 32 || pses->session_resumed == 0)
		n = 0;

	//sension id length
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)n);
	//sension id
	xmem_copy(prec->snd_msg + msglen, pses->session_id, n);
	msglen += n;

	/*
	CipherSuite
	*/
	n = sizeof(client_ciphers) / sizeof(tls12_ciphers_set);

	//cipher list length
	PUT_SWORD_NET(prec->snd_msg, msglen, n * 2);
	msglen += 2;

	for (i = 0; i < n; i++)
	{
		//cipher
		PUT_SWORD_NET(prec->snd_msg, msglen, client_ciphers[i].cipher);
		msglen += 2;
	}

	/*
	CompressionMethod
	*/
	//compression methods length
	PUT_BYTE(prec->snd_msg, msglen++, 1);
	//compression method (null)
	PUT_BYTE(prec->snd_msg, msglen++, 0);

	//position for reseting extension length
	pos = msglen;
	extlen = 0;

	// preset Extensions length to zero
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(0));
	msglen += 2;

	if (!a_is_null(psec->host_cn))
	{
		len = a_xslen(psec->host_cn);

		// Extension type: Server name(0)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SERVERNAME);
		msglen += 2;
		extlen += 2;

		// Server name extension length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(len + 5));
		msglen += 2;
		extlen += 2;

		// Server name list count length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(len + 3));
		msglen += 2;
		extlen += 2;

		// Host name type
		PUT_BYTE(prec->snd_msg, msglen, 0);
		msglen++;
		extlen++;

		// Host name length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(len));
		msglen += 2;
		extlen += 2;

		// Host name
		xmem_copy(prec->snd_msg + msglen, psec->host_cn, len);
		msglen += len;
		extlen += len;
	}

	if (pses->minor_ver == SSL_MINOR_VERSION_3) //TLS 1.2
	{
		/*struct {
		NamedCurve elliptic_curve_list<1..2 ^ 16 - 1>
		} EllipticCurveList;
		*/
		// Extension type: supported_groups (10)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SUPPORTEDGROUPS);
		msglen += 2;
		extlen += 2;

		// supported_groups extension length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(6));
		msglen += 2;
		extlen += 2;

		// Elliptic curve list length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(4));
		msglen += 2;
		extlen += 2;

		// Supported Group: secp256r1 (0x0017)
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(23));
		msglen += 2;
		extlen += 2;

		// Supported Group: secp384r1 (0x0018)
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(24));
		msglen += 2;
		extlen += 2;

		/*struct {
		ECPointFormat ec_point_format_list<1..2 ^ 8 - 1>
		} ECPointFormatList;
		*/
		// Extension type: ec_point_formats (11)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_ECPOINTFORMATS);
		msglen += 2;
		extlen += 2;

		// ec_point_format extension length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(2));
		msglen += 2;
		extlen += 2;

		//EC point formats Length: 1
		PUT_BYTE(prec->snd_msg, msglen, (1));
		msglen++;
		extlen++;

		//EC point format: uncompressed (0)
		PUT_BYTE(prec->snd_msg, msglen, (0));
		msglen++;
		extlen++;

		/*
		struct {
		ExtensionType extension_type;
		opaque extension_data<0..2^16-1>;
		} Extension;
		struct {
		HashAlgorithm hash;
		SignatureAlgorithm signature;
		} SignatureAndHashAlgorithm;
		*/

		// Extension type: Signature and Hash algorithm(13)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SIGNATUREANDHASHALGORITHM);
		msglen += 2;
		extlen += 2;

		n = sizeof(tls12_sign_hash_group) / sizeof(SignHashGroup) * 2 + 2;
		// Signature and Hash extension length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(n));
		msglen += 2;
		extlen += 2;

		n = sizeof(tls12_sign_hash_group) / sizeof(SignHashGroup) * 2;
		// Signature and Hash algorithm list count length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(n));
		msglen += 2;
		extlen += 2;

		n = sizeof(tls12_sign_hash_group) / sizeof(SignHashGroup);
		for (i = 0; i < n; i++)
		{
			//HashAlgorithm
			PUT_BYTE(prec->snd_msg, msglen, tls12_sign_hash_group[i].alg_hash);
			msglen++;
			extlen++;

			//SignatureAlgorithm
			PUT_BYTE(prec->snd_msg, msglen, tls12_sign_hash_group[i].alg_sign);
			msglen++;
			extlen++;
		}
	}

	if (!extlen && pssl->cli_minor_ver < SSL_MINOR_VERSION_3)
	{
		//remove the extension length if empty
		msglen -= 2;
	}
	else
	{
		// reset extension length
		PUT_SWORD_NET(prec->snd_msg, pos, (unsigned short)(extlen));
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ClientHello;
	} Handshake;
	*/

	//handshake type
	PUT_BYTE(prec->snd_msg, 0, (byte_t)SSL_HS_CLIENT_HELLO);
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_client_hello"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_SERVER_HELLO;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_server_hello(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	dword_t t;
	int ciph;
	int msglen, haslen, seslen, extlen;

	TRY_CATCH;

	if (C_OK != _ssl_read_rcv_msg(pssl))
	{
		raise_user_error(_T("_ssl_parse_server_hello"), _T("_ssl_read_rcv_msg"));
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ServerHello;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_SERVER_HELLO)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_hello"), _T("invalid message type"));
	}

	if (prec->rcv_msg[4] != SSL_MAJOR_VERSION_3 || prec->rcv_msg[5] > SSL_MINOR_VERSION_3)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_hello"), _T("invalid message version"));
	}

	if (prec->rcv_msg_len < 38)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_hello"), _T("invalid message length"));
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	struct {
          ProtocolVersion server_version;
          Random random;
          SessionID session_id;
          CipherSuite cipher_suite;
          CompressionMethod compression_method;
          select (extensions_present) {
              case false:
                  struct {};
              case true:
                  Extension extensions<0..2^16-1>;
          };
      } ServerHello;
	*/

	msglen = SSL_HSH_SIZE;

	/*
	ProtocolVersion
	*/
	pssl->srv_major_ver = GET_BYTE(prec->rcv_msg, msglen);
	pssl->srv_minor_ver = GET_BYTE(prec->rcv_msg, msglen + 1);
	msglen += 2;

	if (pssl->cli_major_ver != pssl->srv_major_ver)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_hello"), _T("invalid message version"));
	}

	pses->major_ver = pssl->cli_major_ver;
	pses->minor_ver = (pssl->cli_minor_ver < pssl->srv_minor_ver) ? pssl->cli_minor_ver : pssl->srv_minor_ver;

	/*
	Random
	*/
	t = GET_DWORD_NET(prec->rcv_msg, msglen);
	xmem_copy(pcip->rnd_srv, prec->rcv_msg + msglen, SSL_RND_SIZE);
	msglen += SSL_RND_SIZE;

	/*
	SessionID
	*/
	seslen = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	/*
	CipherSuite
	*/
	ciph = GET_SWORD_NET(prec->rcv_msg, msglen + seslen);

	if (pses->session_resumed == 0 || pcip->cipher != ciph || a_xslen(pses->session_id) != seslen || xmem_comp(pses->session_id, prec->rcv_msg + msglen, seslen) != 0)
	{
		pses->session_resumed = 0;
		xmem_copy(pses->session_id, prec->rcv_msg + msglen, seslen);
		pses->session_size = seslen;

		if (!_ssl_choose_cipher(pcip, ciph))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_server_hello"), _T("invalid cipher type"));
		}
	}

	//session id copyed
	msglen += seslen;
	//cipher choosed
	msglen += 2;

	/*
	CompressionMethod
	*/
	//skip compression alg
	msglen++;

	//has no extension
	if (msglen == haslen + SSL_HSH_SIZE)
	{
		CLN_CATCH;
		return SSL_SERVER_CERTIFICATE;
	}
	
	//extension length
	extlen = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	//skip extension
	msglen += extlen;

	if (haslen + SSL_HSH_SIZE != msglen)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
		raise_user_error(_T("_ssl_parse_server_hello"), _T("invalid message length"));
	}

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	END_CATCH;

	return SSL_SERVER_CERTIFICATE;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_server_certificate(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int  ret, n;
	int msglen, haslen, crtlen;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_server_certificate"), _T("_ssl_read_rcv_msg"));
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	Certificate;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CERTIFICATE)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_certificate"), _T("invalid message type"));
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	opaque ASN.1Cert<1..2^24-1>;

	struct {
	ASN.1Cert certificate_list<0..2^24-1>;
	} Certificate;
	*/
	msglen = SSL_HSH_SIZE;

	crtlen = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	if (haslen != 3 + crtlen)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
		raise_user_error(_T("_ssl_parse_server_certificate"), _T("invalid message length"));
	}

	psec->peer_crt = (x509_crt*)xmem_alloc(sizeof(x509_crt));

	while (crtlen)
	{
		//per cert length
		n = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
		msglen += 3;
		crtlen -= 3;

		if (n < 128 || n > crtlen)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_server_certificate"), _T("invalid certificate length"));
		}

		if (C_OK != x509_crt_parse(psec->peer_crt, prec->rcv_msg + msglen, n))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_server_certificate"), _T("x509_crt_parse"));
		}

		msglen += n;
		crtlen -= n;
	}

	if (psec->verify_mode != SSL_VERIFY_NONE)
	{
		if (psec->chain_ca == NULL)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_UNKNOWN_CA);
			raise_user_error(_T("_ssl_parse_server_certificate"), _T("invalid CA"));
		}

		if (psec->verify_mode == SSL_VERIFY_REQUIRED)
		{
			if (C_OK != x509_crt_verify(psec->peer_crt, psec->chain_ca, NULL, psec->peer_cn, &ret, NULL, NULL))
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
				raise_user_error(_T("_ssl_parse_server_certificate"), _T("x509_crt_verify"));
			}
		}
	}

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	END_CATCH;

	return (IS_DHE_CIPHER(pcip->cipher) || IS_ECDHE_CIPHER(pcip->cipher)) ? SSL_SERVER_KEY_EXCHANGE : SSL_CERTIFICATE_REQUEST;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static int _ssl_parse_server_key_exchange(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int n, haslen, msglen;
	byte_t *p, *end;
	byte_t hash[36];
	sha1_context sha1;
	sha256_context sha2;
	int alg_hash, alg_sign;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_SERVER_KEY_EXCHANGE)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("_ssl_read_rcv_msg"));
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ServerKeyExchange;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_SERVER_KEY_EXCHANGE)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("invalid message type"));
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	struct {
          select (KeyExchangeAlgorithm) {
              case dh_anon:
                  ServerDHParams params;
              case dhe_dss:
              case dhe_rsa:
                  ServerDHParams params;
                  digitally-signed struct {
                      opaque client_random[32];
                      opaque server_random[32];
                      ServerDHParams params;
                  } signed_params;
              case rsa:
              case dh_dss:
              case dh_rsa:
                  struct {} ;
		};
	  } ServerKeyExchange;
	*/
	msglen = SSL_HSH_SIZE;

	p = prec->rcv_msg + SSL_HSH_SIZE;
	end = p + haslen;

	if (IS_DHE_CIPHER(pcip->cipher))
	{
		/*
		struct {
		opaque dh_p<1..2^16-1>;
		opaque dh_g<1..2^16-1>;
		opaque dh_Ys<1..2^16-1>;
		} ServerDHParams;
		*/
		if (!psec->dhm_ctx)
		{
			psec->dhm_ctx = (dhm_context*)xmem_alloc(sizeof(dhm_context));
			dhm_init(psec->dhm_ctx);
		}

		if (C_OK != dhm_read_params(psec->dhm_ctx, &p, end))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("dhm_read_params"));
		}

		if (psec->dhm_ctx->len < 64 || psec->dhm_ctx->len > 256)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("invalid dh conext"));
		}
	}
	else if (IS_ECDHE_CIPHER(pcip->cipher))
	{
		if (!psec->ecdh_ctx)
		{
			psec->ecdh_ctx = (ecdh_context*)xmem_alloc(sizeof(ecdh_context));

			ecdh_init(psec->ecdh_ctx);

			if (C_OK != ecdh_read_params(psec->ecdh_ctx, (const unsigned char **)&p, end, pssl->f_rng, pssl->r_rng))
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
				raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("ecdh_read_params"));
			}
		}
	}
	else
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("invalid cipher type"));
	}

	/*
	select (SignatureAlgorithm)
	{   
	case anonymous: struct { };
	case rsa:
	digitally-signed struct {
	opaque md5_hash[16];
	opaque sha_hash[20];
	};
	case dsa:
	digitally-signed struct {
	opaque sha_hash[20];
	};
	} Signature;
	*/

	alg_hash = *p;
	p++;
	msglen++;

	alg_sign = *p;
	p++;
	msglen++;

	n = GET_SWORD_NET(p, 0);
	p += 2;
	msglen += 2;

	if ((int)(end - p) != psec->peer_crt->rsa->len)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("invalid message length"));
	}

	n = haslen - (end - p) - 2 - 2;

	if (alg_hash == RSA_HASH_SHA256)
	{
		sha256_starts(&sha2, 0);
		sha256_update(&sha2, pcip->rnd_cli, SSL_RND_SIZE);
		sha256_update(&sha2, pcip->rnd_srv, SSL_RND_SIZE);
		sha256_update(&sha2, prec->rcv_msg + SSL_HSH_SIZE, n);
		sha256_finish(&sha2, hash);

		if (C_OK != rsa_pkcs1_verify(psec->peer_crt->rsa, pssl->f_rng, pssl->r_rng, RSA_PUBLIC, RSA_HASH_SHA256, 32, hash, p))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("rsa_pkcs1_verify"));
		}
	}
	else
	{
		sha1_starts(&sha1);
		sha1_update(&sha1, pcip->rnd_cli, SSL_RND_SIZE);
		sha1_update(&sha1, pcip->rnd_srv, SSL_RND_SIZE);
		sha1_update(&sha1, prec->rcv_msg + SSL_HSH_SIZE, n);
		sha1_finish(&sha1, hash);

		if (C_OK != rsa_pkcs1_verify(psec->peer_crt->rsa, pssl->f_rng, pssl->r_rng, RSA_PUBLIC, RSA_HASH_SHA1, 20, hash, p))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_server_key_exchange"), _T("rsa_pkcs1_verify"));
		}
	}

	msglen += (n + psec->peer_crt->rsa->len);

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	END_CATCH;

	return SSL_CERTIFICATE_REQUEST;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static int _ssl_parse_server_certificate_request(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int haslen, msglen, alg_hash, alg_sign;
	int n, crttype, dsnlen;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE_REQUEST && prec->rcv_msg[0] != SSL_HS_SERVER_HELLO_DONE)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_server_certificate_request"), _T("_ssl_read_rcv_msg"));
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	CertificateRequest;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_certificate_request"), _T("invalid message type"));
	}

	pses->authen_client = (prec->rcv_msg[0] == SSL_HS_CERTIFICATE_REQUEST) ? 1 : 0;
	if (pses->authen_client == 0)
	{
		CLN_CATCH;
		return SSL_SERVER_HELLO_DONE;
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	struct {
          ClientCertificateType certificate_types<1..2^8-1>;
          SignatureAndHashAlgorithm
            supported_signature_algorithms<2^16-1>;
          DistinguishedName certificate_authorities<0..2^16-1>;
      } CertificateRequest;
	*/

	msglen = SSL_HSH_SIZE;

	//certificate_types count
	n = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	//certificate_types
	while (n--)
	{
		crttype = GET_BYTE(prec->rcv_msg, msglen);
		msglen++;
	}

	//SignatureAndHashAlgorithm length
	n = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	while (n)
	{
		alg_hash = GET_BYTE(prec->rcv_msg, msglen);
		msglen++;

		alg_sign = GET_BYTE(prec->rcv_msg, msglen);
		msglen++;

		n -= 2;
	}

	//all DistinguishedName length
	dsnlen = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	while (dsnlen)
	{
		n = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;
		dsnlen -= 2;

		if (n > dsnlen)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_server_certificate_request"), _T("invalid DistinguishedName"));
		}

		msglen += n;
		dsnlen -= n;
	}

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	END_CATCH;

	return SSL_SERVER_HELLO_DONE;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static int _ssl_parse_server_hello_done(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_SERVER_HELLO_DONE)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_server_hello_done"), _T("_ssl_read_rcv_msg"));
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ServerHelloDone;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_SERVER_HELLO_DONE)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_hello_done"), _T("invalid message type"));
	}

	/*
	struct { } ServerHelloDone;
	*/

	END_CATCH;

	return (pses->authen_client) ? SSL_CLIENT_CERTIFICATE : SSL_CLIENT_KEY_EXCHANGE;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_client_certificate(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, lenpos, crtlen, msglen = SSL_HSH_SIZE;
	x509_crt *crt;

	if (psec->host_crt == NULL)
	{
		pses->alert_code = SSL_ALERT_NO_CERTIFICATE;

		return (C_OK == _ssl_write_alert(pssl)) ? SSL_CLIENT_KEY_EXCHANGE : SSL_HANDSHAKE_ERROR;
	}

	TRY_CATCH;

	/*
	opaque ASN.1Cert<1..2^24-1>;

	struct {
	ASN.1Cert certificate_list<0..2^24-1>;
	} Certificate;
	*/

	//preset certs length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	lenpos = msglen;
	msglen += 3;

	crtlen = 0;
	crt = psec->host_crt;
	while (crt != NULL && crt->version != 0)
	{
		n = crt->raw.len;
		if (msglen + 3 + n > SSL_PKG_SIZE)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_write_client_certificate"), _T("invalid certifacate length"));
		}

		//peer cert length
		PUT_THREEBYTE_NET(prec->snd_msg, msglen, n);
		msglen += 3;
		crtlen += 3;

		//peer cert data
		xmem_copy(prec->snd_msg + msglen, crt->raw.p, n);
		msglen += n;
		crtlen += n;

		crt = crt->next;
	}

	//reset certs length
	PUT_THREEBYTE_NET(prec->snd_msg, lenpos, crtlen);
	
	/*
	struct {
		HandshakeType msg_type; 1 byte
		uint24 length; 3 bytes
		Certificate;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_CERTIFICATE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_client_certificate"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_CLIENT_KEY_EXCHANGE;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static int _ssl_write_client_key_exchange(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int pos, n, msglen;
	byte_t premaster[SSL_BLK_SIZE] = {0};
	int prelen = SSL_MST_SIZE;
	dword_t m;

	TRY_CATCH;

	/*
	struct {
		select (KeyExchangeAlgorithm) {
		case rsa:
			EncryptedPreMasterSecret;
		case dhe_dss:
		case dhe_rsa:
		case dh_dss:
		case dh_rsa:
		case dh_anon:
			ClientDiffieHellmanPublic;
		} exchange_keys;
	} ClientKeyExchange;
	*/

	msglen = SSL_HSH_SIZE;

	if (IS_DHE_CIPHER(pcip->cipher))
	{
		/*
		 struct {
           select (PublicValueEncoding) {
               case implicit: struct { };
               case explicit: opaque dh_Yc<1..2^16-1>;
           } dh_public;
       } ClientDiffieHellmanPublic;
		*/

		n = psec->dhm_ctx->len;

		pos = msglen;
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)n);
		msglen += 2;

		m = mpi_size(&(psec->dhm_ctx->P)); //256
		n = psec->dhm_ctx->len;
		if (C_OK != dhm_make_public(psec->dhm_ctx, (int)m, prec->snd_msg + msglen, n, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_client_key_exchange"), _T("dhm_make_public"));
		}
		msglen += n;

		//if n changed, reset key len
		PUT_SWORD_NET(prec->snd_msg, pos, (unsigned short)n);

		m = psec->dhm_ctx->len;
		if (C_OK != dhm_calc_secret(psec->dhm_ctx, premaster, m, &m, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_client_key_exchange"), _T("dhm_calc_secret"));
		}
		prelen = (int)m;
	}
	else if (IS_ECDHE_CIPHER(pcip->cipher))
	{
		n = 0;
		if (C_OK != ecdh_make_public(psec->ecdh_ctx, &n, prec->snd_msg + msglen, 4096, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_client_key_exchange"), _T("ecdh_make_public"));
		}
		msglen += n;

		prelen = 0;
		if (C_OK != ecdh_calc_secret(psec->ecdh_ctx, &prelen, premaster, 256, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_client_key_exchange"), _T("ecdh_calc_secret"));
		}
	}
	else
	{
		/*
		struct {
			ProtocolVersion client_version;
			opaque random[46];
		} PreMasterSecret;

		public-key-encrypted PreMasterSecret pre_master_secret;
		*/

		premaster[0] = (byte_t)pses->major_ver;
		premaster[1] = (byte_t)pses->minor_ver;
		prelen = SSL_MST_SIZE;

		//for (n = 2; n < prelen; n++)
		//	premaster[n] = (byte_t)havege_rand(&pcip->rng);
		(*pssl->f_rng)(pssl->r_rng, (premaster + 2), (prelen - 2));

		n = psec->peer_crt->rsa->len;

		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)n);
		msglen += 2;

		if (C_OK != rsa_pkcs1_encrypt(psec->peer_crt->rsa, pssl->f_rng, pssl->r_rng, RSA_PUBLIC, prelen, premaster, prec->snd_msg + msglen))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_client_key_exchange"), _T("rsa_pkcs1_encrypt"));
		}
		msglen += n;
	}

	_ssl_derive_keys(pcip, premaster, prelen);

	/*
	struct {
		HandshakeType msg_type; 1 byte
		uint24 length; 3 bytes
		ClientKeyExchange;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_CLIENT_KEY_EXCHANGE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_client_key_exchange"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return (pses->authen_client) ? SSL_CERTIFICATE_VERIFY : SSL_CLIENT_CHANGE_CIPHER_SPEC;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static int _ssl_write_client_certificate_verify(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen;
	byte_t hash[36];
	sha256_context sha2;

	TRY_CATCH;

	xmem_copy(&sha2, &pcip->sha2, sizeof(sha256_context));

	sha256_finish(&sha2, hash);

	/*
	struct {
	Signature signature;
	} CertificateVerify;
	*/
	msglen = SSL_HSH_SIZE;

	PUT_BYTE(prec->snd_msg, msglen, ALG_HASH_SHA256);
	msglen++;

	PUT_BYTE(prec->snd_msg, msglen, ALG_SIGN_RSA);
	msglen++;

	if (psec->rsa_ctx == NULL)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_NO_CERTIFICATE);
		raise_user_error(_T("_ssl_write_client_certificate_verify"), _T("invalid rsa context"));
	}

	n = psec->rsa_ctx->len;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)n);
	msglen += 2;

	if (C_OK != rsa_pkcs1_sign(psec->rsa_ctx, pssl->f_rng, pssl->r_rng, RSA_PRIVATE, RSA_HASH_NONE, 32, hash, prec->snd_msg + msglen))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_client_certificate_verify"), _T("rsa_pkcs1_sign"));
	}

	msglen += n;

	/*
	struct {
		HandshakeType msg_type; 1 byte
		uint24 length; 3 bytes
		CertificateVerify;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_CERTIFICATE_VERIFY;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_client_certificate_verify"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_CLIENT_CHANGE_CIPHER_SPEC;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_client_change_cipher_spec(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int i;

	TRY_CATCH;

	/*
	struct {
		enum { change_cipher_spec(1), (255) } type;
	} ChangeCipherSpec;
	*/

	prec->snd_msg_type = SSL_MSG_CHANGE_CIPHER_SPEC;
	prec->snd_msg_len = 1;
	prec->snd_msg[0] = 1;

	//clear send message control bits
	for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
	{
		prec->snd_ctr[i] = 0;
	}

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_client_change_cipher_spec"), _T("_ssl_write_snd_msg"));
	}

	//after send change cipher all record must be crypted sending
	prec->crypted = 1;

	END_CATCH;

	return SSL_CLIENT_FINISHED;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_client_finished(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int msglen;
	sha256_context sha2;
	byte_t padbuf[48] = {0};
	byte_t* mac_buf;

	TRY_CATCH;

	xmem_copy(&sha2, &pcip->sha2, sizeof(sha256_context));

	/*
	struct {
	opaque verify_data[12];
	} Finished;
	*/

	msglen = SSL_HSH_SIZE;
	mac_buf = prec->snd_msg + msglen;

	sha256_finish(&sha2, padbuf); //32 bytes

	ssl_prf2(pcip->master_secret, SSL_MST_SIZE, label_client_finished, padbuf, 32, mac_buf, 12);

	msglen += 12;

	/*
	struct {
		HandshakeType msg_type; 1 byte
		uint24 length; 3 bytes
		Finished;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_FINISHED;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_client_finished"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_SERVER_CHANGE_CIPHER_SPEC;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_server_change_cipher_spec(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int i;

	TRY_CATCH;

	/*
	struct {
	enum { change_cipher_spec(1), (255) } type;
	} ChangeCipherSpec;
	*/

	if (C_OK != _ssl_read_rcv_msg(pssl))
	{
		raise_user_error(_T("_ssl_parse_server_change_cipher_spec"), _T("_ssl_read_rcv_msg"));
	}

	if (prec->rcv_msg_type != SSL_MSG_CHANGE_CIPHER_SPEC)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_change_cipher_spec"), _T("invalid message type"));
	}

	if (prec->rcv_msg_len != 1 || prec->rcv_msg[0] != 1)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_change_cipher_spec"), _T("invalid message length"));
	}

	//clear recv message control bits
	for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
	{
		prec->rcv_ctr[i] = 0;
	}

	//after recv change cipher all record must be crypted recving
	prec->crypted = 1;

	END_CATCH;

	return SSL_SERVER_FINISHED;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_server_finished(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int hash_len;
	sha256_context sha2;
	byte_t padbuf[48] = { 0 };
	byte_t mac_buf[36] = { 0 };

	TRY_CATCH;

	xmem_copy(&sha2, &pcip->sha2, sizeof(sha256_context));

	sha256_finish(&sha2, padbuf);

	ssl_prf2(pcip->master_secret, SSL_MST_SIZE, label_server_finished, padbuf, 32, mac_buf, 12);
	hash_len = 12;

	if (C_OK != _ssl_read_rcv_msg(pssl))
	{
		raise_user_error(_T("_ssl_parse_server_finished"), _T("_ssl_read_rcv_msg"));
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	Finished;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_FINISHED)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_finished"), _T("invalid message type"));
	}
	/*
	struct {
	opaque verify_data[12];
	} Finished;
	*/
	if (xmem_comp(prec->rcv_msg + SSL_HSH_SIZE, mac_buf, hash_len) != 0)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_RECORD_MAC);
		raise_user_error(_T("_ssl_parse_server_finished"), _T("invalid message mac"));
	}

	END_CATCH;

	return SSL_HANDSHAKE_OVER;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

bool_t tls12_handshake_client(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_handshake_states state = SSL_HELLO_REQUEST;

	while (state != SSL_HANDSHAKE_OVER)
	{
		switch (state)
		{
		case SSL_HELLO_REQUEST:
			_ssl_alloc_cipher(pssl);

			state = SSL_CLIENT_HELLO;
			break;

			/*
			*  ==>   ClientHello
			*/
		case SSL_CLIENT_HELLO:
			state = _ssl_write_client_hello(pssl);
			break;

			/*
			*  <==   ServerHello
			*        Certificate
			*      ( ServerKeyExchange  )
			*      ( CertificateRequest )
			*        ServerHelloDone
			*/
		case SSL_SERVER_HELLO:
			state = _ssl_parse_server_hello(pssl);
			break;

		case SSL_SERVER_CERTIFICATE:
			state = _ssl_parse_server_certificate(pssl);
			break;

		case SSL_SERVER_KEY_EXCHANGE:
			state = _ssl_parse_server_key_exchange(pssl);
			break;

		case SSL_CERTIFICATE_REQUEST:
			state = _ssl_parse_server_certificate_request(pssl);
			break;

		case SSL_SERVER_HELLO_DONE:
			state = _ssl_parse_server_hello_done(pssl);
			break;

			/*
			*  ==> ( Certificate/Alert  )
			*        ClientKeyExchange
			*      ( CertificateVerify  )
			*        ChangeCipherSpec
			*        Finished
			*/
		case SSL_CLIENT_CERTIFICATE:
			state = _ssl_write_client_certificate(pssl);
			break;

		case SSL_CLIENT_KEY_EXCHANGE:
			state = _ssl_write_client_key_exchange(pssl);
			break;

		case SSL_CERTIFICATE_VERIFY:
			state = _ssl_write_client_certificate_verify(pssl);
			break;

		case SSL_CLIENT_CHANGE_CIPHER_SPEC:
			state = _ssl_write_client_change_cipher_spec(pssl);
			break;

		case SSL_CLIENT_FINISHED:
			state = _ssl_write_client_finished(pssl);
			break;

			/*
			*  <==   ChangeCipherSpec
			*        Finished
			*/
		case SSL_SERVER_CHANGE_CIPHER_SPEC:
			state = _ssl_parse_server_change_cipher_spec(pssl);
			break;

		case SSL_SERVER_FINISHED:
			state = _ssl_parse_server_finished(pssl);
			break;
		}
		
		if (state == SSL_HANDSHAKE_ERROR)
		{
			_ssl_write_alert(pssl);
			break;
		}
	}

	pses->handshake_over = (state == SSL_HANDSHAKE_OVER) ? 1 : -1;

	return (pses->handshake_over == 1)? 1 : 0;
}

/***************************************server routing************************************************************/

static tls12_handshake_states _ssl_parse_client_hello(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int msglen, haslen, seslen, ciphlen, complen, comped, extlen, lstlen, grplen;
	int i, j, n;
	int ciph, type;
	byte_t* ciph_buf;

	TRY_CATCH;

	if (C_OK != _ssl_read_rcv_msg(pssl))
	{
		raise_user_error(_T("_ssl_parse_client_hello"), _T("_ssl_read_rcv_msg"));
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ClientHello;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CLIENT_HELLO)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message type"));
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	struct {
          ProtocolVersion client_version;
          Random random;
          SessionID session_id;
          CipherSuite cipher_suites<2..2^16-2>;
          CompressionMethod compression_methods<1..2^8-1>;
          select (extensions_present) {
              case false:
                  struct {};
              case true:
                  Extension extensions<0..2^16-1>;
          };
      } ClientHello;
	*/

	msglen = SSL_HSH_SIZE;

	/*
	ProtocolVersion
	*/
	pssl->cli_major_ver = prec->rcv_msg[msglen];
	pssl->cli_minor_ver = prec->rcv_msg[msglen + 1];
	msglen += 2;

	if (pssl->srv_major_ver != pssl->cli_major_ver)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message version"));
	}

	if (pssl->cli_minor_ver < SSL_MINOR_VERSION_1)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message version"));
	}

	pses->major_ver = pssl->cli_major_ver;
	pses->minor_ver = (pssl->srv_minor_ver < pssl->cli_minor_ver) ? pssl->srv_minor_ver : pssl->cli_minor_ver;

	/*
	Random
	*/
	xmem_copy(pcip->rnd_cli, prec->rcv_msg + msglen, SSL_RND_SIZE);
	msglen += SSL_RND_SIZE;

	/*
	SessionID
	*/
	seslen = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	if (seslen < 0 || seslen > 32)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid session id"));
	}

	xmem_copy(pses->session_id, prec->rcv_msg + msglen, seslen);
	msglen += seslen;

	/*
	CipherSuite
	*/
	ciphlen = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (ciphlen < 2 || ciphlen > 256 || (ciphlen % 2) != 0)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid cipher type"));
	}

	ciph_buf = prec->rcv_msg + msglen;
	ciph = 0;
	n = sizeof(server_ciphers) / sizeof(tls12_ciphers_set);
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < ciphlen; j += 2)
		{
			ciph = GET_SWORD_NET(ciph_buf, j);
			if (ciph == server_ciphers[i].cipher)
				break;
		}

		if (j < ciphlen)
			break;
	}
	msglen += ciphlen;

	if (!_ssl_choose_cipher(pcip, ciph))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid cipher type"));
	}

	/*
	CompressionMethod
	*/
	complen = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	comped = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	if (complen < 1 || complen > 16)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid compress method"));
	}

	//has no extension
	if (msglen == haslen + SSL_HSH_SIZE)
	{
		CLN_CATCH;
		return SSL_SERVER_HELLO;
	}

	//extension length
	extlen = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	while (extlen)
	{
		//extension type
		type = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;
		extlen -= 2;

		//extension list length
		lstlen = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;
		extlen -= 2;

		switch (type)
		{
		case SSL_EXTENSION_SERVERNAME:
			//skip server name
			msglen += lstlen;
			extlen -= lstlen;
			break;
		case SSL_EXTENSION_SUPPORTEDGROUPS:
			//Elliptic curve list length
			grplen = GET_SWORD_NET(prec->rcv_msg, msglen);
			msglen += 2;
			extlen -= 2;
			lstlen -= 2;

			while (grplen)
			{
				if (!pcip->ecp_group)
					pcip->ecp_group = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;
				grplen -= 2;
			}
			msglen += lstlen;
			extlen -= lstlen;
			break;
		case SSL_EXTENSION_ECPOINTFORMATS:
			//Elliptic curve list length
			grplen = GET_BYTE(prec->rcv_msg, msglen);
			msglen++;
			extlen--;
			lstlen--;

			if (!pcip->ecp_format)
				pcip->ecp_format = GET_BYTE(prec->rcv_msg, msglen);
			msglen += lstlen;
			extlen -= lstlen;
			break;
		case SSL_EXTENSION_SIGNATUREANDHASHALGORITHM:
			//Algorithm list count length
			grplen = GET_SWORD_NET(prec->rcv_msg, msglen);
			msglen += 2;
			extlen -= 2;
			lstlen -= 2;

			while (grplen)
			{
				if (!pcip->alg_hash)
					pcip->alg_hash = GET_BYTE(prec->rcv_msg, msglen);
				msglen++;
				extlen--;
				lstlen--;
				grplen--;

				if (!pcip->alg_sign)
					pcip->alg_sign = GET_BYTE(prec->rcv_msg, msglen);
				msglen++;
				extlen--;
				lstlen--;
				grplen--;
			}
			break;
		default:
			//skip 
			msglen += lstlen;
			extlen -= lstlen;
			break;
		}

		if (extlen < 0)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
			raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message length"));
		}
	}

	END_CATCH;

	return SSL_SERVER_HELLO;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_server_hello(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int msglen, extlen;
	dword_t t;

	TRY_CATCH;

	//gen server random bits
	t = get_times();
	PUT_DWORD_NET(pcip->rnd_srv, 0, t);

	/*for (i = 4; i < SSL_RND_SIZE; i++)
	{
		pcip->rnd_srv[i] = (byte_t)havege_rand(&pcip->rng);
	}*/
	(*pssl->f_rng)(pssl->r_rng, (pcip->rnd_srv + 4), (SSL_RND_SIZE - 4));

	//gen server session id
	pses->session_size = 32;
	/*for (i = 0; i < pses->session_size; i++)
	{
		pses->session_id[i] = (byte_t)havege_rand(&pcip->rng);
	}*/
	(*pssl->f_rng)(pssl->r_rng, pses->session_id, pses->session_size);

	/*
	 struct {
          ProtocolVersion server_version;
          Random random;
          SessionID session_id;
          CipherSuite cipher_suite;
          CompressionMethod compression_method;
          select (extensions_present) {
              case false:
                  struct {};
              case true:
                  Extension extensions<0..2^16-1>;
          };
      } ServerHello;
	*/

	msglen = SSL_HSH_SIZE;

	/*
	ProtocolVersion
	*/
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pses->major_ver));
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pses->minor_ver));

	/*
	Random
	*/
	xmem_copy(prec->snd_msg + msglen, pcip->rnd_srv, SSL_RND_SIZE);
	msglen += SSL_RND_SIZE;

	/*
	SessionID
	*/
	PUT_BYTE(prec->snd_msg, msglen, (byte_t)(pses->session_size));
	msglen++;
	xmem_copy(prec->snd_msg + msglen, pses->session_id, pses->session_size);
	msglen += pses->session_size;

	/*
	CipherSuite
	*/
	PUT_SWORD_NET(prec->snd_msg, msglen, pcip->cipher);
	msglen += 2;

	/*
	CompressionMethod
	*/
	PUT_BYTE(prec->snd_msg, msglen, 0);
	msglen++;

	// preset Extensions length to zero
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(0));
	msglen += 2;
	extlen = 0;

	/*if (pssl->ecp_group)
	{
	//struct {
	//NamedCurve elliptic_curve_list<1..2 ^ 16 - 1>
	//} EllipticCurveList;

	// Extension type: supported_groups (10)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SUPPORTEDGROUPS);
	msglen += 2;
	extlen += 2;

	// supported_groups length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(4));
	msglen += 2;
	extlen += 2;

	// list length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(2));
	msglen += 2;
	extlen += 2;

	// Supported Group: secp256r1 (0x0017)
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(pssl->ecp_group));
	msglen += 2;
	extlen += 2;

	}*/

	if (extlen)
	{
		//reset Extensions length
		PUT_SWORD_NET(prec->snd_msg, (msglen - extlen - 2), (unsigned short)(extlen));
	}
	else
	{
		//remove Extensions length
		msglen -= 2;
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ServerHello;
	} Handshake;
	*/
	//handshake type
	PUT_BYTE(prec->snd_msg, 0, (byte_t)SSL_HS_SERVER_HELLO);
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_hello"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_SERVER_CERTIFICATE;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_server_certificate(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, lenpos, crtlen, msglen;
	x509_crt *crt;

	TRY_CATCH;

	if (psec->host_crt == NULL)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_NO_CERTIFICATE);
		raise_user_error(_T("_ssl_write_server_certificate"), _T("invalid certificate"));
	}
	/*
	opaque ASN.1Cert<1..2^24-1>;

	struct {
	ASN.1Cert certificate_list<0..2^24-1>;
	} Certificate;
	*/

	msglen = SSL_HSH_SIZE;

	//preset certs length to zero
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	lenpos = msglen;
	msglen += 3;

	crtlen = 0;
	crt = psec->host_crt;
	while (crt != NULL && crt->version != 0)
	{
		n = crt->raw.len;
		if (msglen + 3 + n > SSL_PKG_SIZE)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_write_server_certificate"), _T("invalid certificate"));
		}

		//peer cert length
		PUT_THREEBYTE_NET(prec->snd_msg, msglen, n);
		msglen += 3;
		crtlen += 3;

		//peer cert data
		xmem_copy(prec->snd_msg + msglen, crt->raw.p, n);
		msglen += n;
		crtlen += n;

		crt = crt->next;
	}

	//reset all cert length
	PUT_THREEBYTE_NET(prec->snd_msg, lenpos, crtlen);

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	Certificate;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_CERTIFICATE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_certificate"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	if (IS_DHE_CIPHER(pcip->cipher) || IS_ECDHE_CIPHER(pcip->cipher))
		return SSL_SERVER_KEY_EXCHANGE;
	else
		return (psec->verify_mode == SSL_VERIFY_NONE) ? SSL_SERVER_HELLO_DONE : SSL_CERTIFICATE_REQUEST;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_server_key_exchange(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen;
	dword_t m;
	byte_t hash[64] = { 0 };
	//sha1_context sha1;
	sha256_context sha2;
	const ecp_curve_info* pec;

	TRY_CATCH;

	/*
	struct {
		select (KeyExchangeAlgorithm) {
		case dh_anon:
			ServerDHParams params;
		case dhe_dss:
		case dhe_rsa:
			ServerDHParams params;
			digitally-signed struct {
				opaque client_random[32];
				opaque server_random[32];
				ServerDHParams params;
			} signed_params;
		case rsa:
		case dh_dss:
		case dh_rsa:
		struct {} ;
		};
	  } ServerKeyExchange;
	*/

	msglen = SSL_HSH_SIZE;

	if (IS_DHE_CIPHER(pcip->cipher))
	{
		/*
		struct {
		opaque dh_p<1..2^16-1>;
		opaque dh_g<1..2^16-1>;
		opaque dh_Ys<1..2^16-1>;
		} ServerDHParams;
		*/
		if (!psec->dhm_ctx)
		{
			psec->dhm_ctx = (dhm_context*)xmem_alloc(sizeof(dhm_context));
			mpi_read_string(&(psec->dhm_ctx->P), 16, dhm_P, -1);
			mpi_read_string(&(psec->dhm_ctx->G), 16, dhm_G, -1);
		}

		// Ephemeral DH parameters:
		m = mpi_size(&(psec->dhm_ctx->P)); //256
		n = 0;
		if (C_OK != dhm_make_params(psec->dhm_ctx, (int)m, prec->snd_msg + msglen, &n, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_server_key_exchange"), _T("dhm_make_params"));
		}

		msglen += n;
	}
	else if (IS_ECDHE_CIPHER(pcip->cipher))
	{
		if (!psec->ecdh_ctx)
		{
			psec->ecdh_ctx = (ecdh_context*)xmem_alloc(sizeof(ecdh_context));
			pec = ecp_curve_info_from_tls_id(TLS_EC_GROUP_SECP256R1);
			if (!pec)
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
				raise_user_error(_T("_ssl_write_server_key_exchange"), _T("ecp_curve_info_from_tls_id"));
			}
			ecdh_setup(psec->ecdh_ctx, pec->grp_id);
		}

		//
		n = 0;
		if (C_OK != ecdh_make_params(psec->ecdh_ctx, &n, prec->snd_msg + msglen, 4096, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_server_key_exchange"), _T("ecdh_make_params"));
		}
		msglen += n;
	}

	/*
	struct {
		SignatureAndHashAlgorithm algorithm;
		opaque signature<0..2^16-1>;
	} DigitallySigned;
	*/

	sha256_starts(&sha2, 0);
	sha256_update(&sha2, pcip->rnd_cli, SSL_RND_SIZE);
	sha256_update(&sha2, pcip->rnd_srv, SSL_RND_SIZE);
	sha256_update(&sha2, prec->snd_msg + SSL_HSH_SIZE, n);
	sha256_finish(&sha2, hash);

	PUT_BYTE(prec->snd_msg, msglen, ALG_HASH_SHA256);
	msglen++;

	/*sha1_starts(&sha1);
	sha1_update(&sha1, pcip->rnd_cli, SSL_RND_SIZE);
	sha1_update(&sha1, pcip->rnd_srv, SSL_RND_SIZE);
	sha1_update(&sha1, prec->snd_msg + SSL_HSH_SIZE, n);
	sha1_finish(&sha1, hash);

	PUT_BYTE(prec->snd_msg, msglen, ALG_HASH_SHA1);
	msglen++;*/

	PUT_BYTE(prec->snd_msg, msglen, ALG_SIGN_RSA);
	msglen++;

	PUT_SWORD_NET(prec->snd_msg, msglen, psec->rsa_ctx->len);
	msglen += 2;

	//if (C_OK != rsa_pkcs1_sign(psec->rsa_ctx, RSA_PRIVATE, RSA_RAW, 32, hash, prec->snd_msg + msglen))
	if (C_OK != rsa_pkcs1_sign(psec->rsa_ctx, pssl->f_rng, pssl->r_rng, RSA_PRIVATE, RSA_HASH_SHA256, 32, hash, prec->snd_msg + msglen))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_server_key_exchange"), _T("rsa_pkcs1_sign"));
	}
	msglen += (psec->rsa_ctx->len);

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	Signature;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_SERVER_KEY_EXCHANGE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_key_exchange"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return (psec->verify_mode == SSL_VERIFY_NONE) ? SSL_SERVER_HELLO_DONE : SSL_CERTIFICATE_REQUEST;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_server_certificate_request(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, lenpos, crtlen, msglen;
	x509_crt *crt;

	TRY_CATCH;

	/*
	struct {
		ClientCertificateType certificate_types<1..2^8-1>; enum {rsa_sign(1), dss_sign(2), rsa_fixed_dh(3), dss_fixed_dh(4), rsa_ephemeral_dh_RESERVED(5), dss_ephemeral_dh_RESERVED(6), fortezza_dms_RESERVED(20), (255)}
		SignatureAndHashAlgorithm; supported_signature_algorithms<2^16-1>;
		DistinguishedName certificate_authorities<0..2^16-1>;
	} CertificateRequest;
	*/

	msglen = SSL_HSH_SIZE;

	//cert type count
	PUT_BYTE(prec->snd_msg, msglen, 1);
	msglen++;

	//cert type
	PUT_BYTE(prec->snd_msg, msglen, SSL_CERTIFICATE_TYPE_RSA);
	msglen++;

	/*
	SignatureAndHashAlgorithm
	*/
	// SignatureAndHashAlgorithm length
	PUT_SWORD_NET(prec->snd_msg, msglen, 2);
	msglen += 2;

	//HashAlgorithm
	PUT_BYTE(prec->snd_msg, msglen, ALG_HASH_SHA256);
	msglen++;

	//SignatureAlgorithm
	PUT_BYTE(prec->snd_msg, msglen, ALG_SIGN_RSA);
	msglen++;
	
	//preset all cert length
	PUT_SWORD_NET(prec->snd_msg, msglen, 0);
	lenpos = msglen;
	msglen += 2;

	crtlen = 0;
	crt = psec->chain_ca;
	while (crt != NULL && crt->next != NULL)
	{
		n = crt->subject_raw.len;
		PUT_SWORD_NET(prec->snd_msg, msglen, n);
		msglen += 2;
		crtlen += 2;

		xmem_copy(prec->snd_msg + msglen, crt->subject_raw.p, n);
		msglen += n;
		crtlen += n;
		
		crt = crt->next;
	}

	//reset all cert length
	PUT_SWORD_NET(prec->snd_msg, lenpos, crtlen);

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	CertificateRequest;
	} Handshake;
	*/
	prec->snd_msg[0] = SSL_HS_CERTIFICATE_REQUEST;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_certificate_request"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_SERVER_HELLO_DONE;
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_server_hello_done(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	TRY_CATCH;

	/*
	struct { } ServerHelloDone;
	*/

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ServerHelloDone;
	} Handshake;
	*/
	prec->snd_msg[0] = SSL_HS_SERVER_HELLO_DONE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	prec->snd_msg_len = SSL_HSH_SIZE;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_hello_done"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return (psec->verify_mode == SSL_VERIFY_NONE) ? SSL_CLIENT_KEY_EXCHANGE : SSL_CLIENT_CERTIFICATE;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_client_certificate(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int msglen, haslen, crtlen;
	int n, ret;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE && (prec->rcv_msg[0] != SSL_LEVEL_WARNING || prec->snd_msg[1] != SSL_ALERT_NO_CERTIFICATE))
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("_ssl_read_rcv_msg"));
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	Certificate;
	} Handshake;
	*/
	if (prec->rcv_msg_type == SSL_MSG_ALERT && prec->rcv_msg[0] == SSL_LEVEL_WARNING && prec->snd_msg[1] == SSL_ALERT_NO_CERTIFICATE)
	{
		if (psec->verify_mode == SSL_VERIFY_REQUIRED)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid certificate"));
		}
		else
		{
			pses->authen_client = 0;
			CLN_CATCH;
			return SSL_CLIENT_KEY_EXCHANGE;
		}
	}

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CERTIFICATE)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid message type"));
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	opaque ASN.1Cert<1..2^24-1>;

	struct {
	ASN.1Cert certificate_list<0..2^24-1>;
	} Certificate;
	*/
	msglen = SSL_HSH_SIZE;

	crtlen = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	if (haslen != 3 + crtlen)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid message length"));
	}
	//empty certificate
	if (!crtlen)
	{
		if (psec->verify_mode == SSL_VERIFY_REQUIRED)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid certificate"));
		}
		else
		{
			pses->authen_client = 0;
			CLN_CATCH;
			return SSL_CLIENT_KEY_EXCHANGE;
		}
	}

	psec->peer_crt = (x509_crt*)xmem_alloc(sizeof(x509_crt));

	while (crtlen)
	{
		n = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
		msglen += 3;
		crtlen -= 3;

		if (n < 128 || n > crtlen)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid certificate"));
		}

		if (C_OK != x509_crt_parse(psec->peer_crt, prec->rcv_msg + msglen, n))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("x509_crt_parse"));
		}

		msglen += n;
		crtlen -= n;
	}

	if (psec->verify_mode != SSL_VERIFY_NONE)
	{
		if (psec->chain_ca == NULL)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid CA"));
		}

		if (C_OK != x509_crt_verify(psec->peer_crt, psec->chain_ca, NULL, psec->peer_cn, &ret, NULL, NULL))
		{
			if (psec->verify_mode == SSL_VERIFY_REQUIRED)
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
				raise_user_error(_T("_ssl_parse_client_certificate"), _T("x509_crt_verify"));
			}
		}
	}

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	pses->authen_client = 1;

	END_CATCH;

	return SSL_CLIENT_KEY_EXCHANGE;
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_client_key_exchange(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int haslen, n, msglen;
	byte_t premaster[SSL_BLK_SIZE] = { 0 };
	int prelen = SSL_MST_SIZE;
	dword_t m;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_CLIENT_KEY_EXCHANGE)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("_ssl_read_rcv_msg"));
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ClientKeyExchange;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CLIENT_KEY_EXCHANGE)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("invalid message type"));
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	struct {
		select (KeyExchangeAlgorithm) {
		case rsa:
			EncryptedPreMasterSecret;
		case dhe_dss:
		case dhe_rsa:
		case dh_dss:
		case dh_rsa:
		case dh_anon:
			ClientDiffieHellmanPublic;
		} exchange_keys;
	} ClientKeyExchange;
	*/
	msglen = SSL_HSH_SIZE;

	if (IS_DHE_CIPHER(pcip->cipher))
	{
		//Receive G^Y mod P, premaster = (G^Y)^X mod P
		n = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;

		//if (n < 1 || n > psec->dhm_ctx->len || n + 2 != haslen) //key size maybe changed
		if (n < 1 || n + 2 != haslen)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("invalid message length"));
		}

		if (C_OK != dhm_read_public(psec->dhm_ctx, prec->rcv_msg + msglen, n))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("dhm_read_public"));
		}

		prelen = psec->dhm_ctx->len;
		m = 0;
		if (C_OK != dhm_calc_secret(psec->dhm_ctx, premaster, prelen, &m, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("dhm_calc_secret"));
		}
		prelen = (int)m;

		msglen += n;
	}
	else if (IS_ECDHE_CIPHER(pcip->cipher))
	{
		/*n = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;

		//if (n < 1 || n > psec->dhm_ctx->len || n + 2 != haslen) //key size maybe changed
		if (n < 1 || n + 2 != haslen)
		{
		set_last_error(_T("_ssl_parse_client_key_exchange"), _T("invalid client key exchange length"), -1);
		return SSL_HANDSHAKE_ERROR;
		}*/

		if (C_OK != ecdh_read_public(psec->ecdh_ctx, prec->rcv_msg + msglen, haslen, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("ecdh_read_public"));
		}

		prelen = 0;
		if (C_OK != ecdh_calc_secret(psec->ecdh_ctx, &prelen, premaster, 256, pssl->f_rng, pssl->r_rng))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("ecdh_calc_secret"));
		}

		msglen += haslen;
	}
	else
	{
		/*
		struct {
		ProtocolVersion client_version;
		opaque random[46];
		} PreMasterSecret;

		struct {
		public-key-encrypted PreMasterSecret pre_master_secret;
		} EncryptedPreMasterSecret;
		*/
		// Decrypt the premaster using own private RSA key
		prelen = SSL_MST_SIZE;

		n = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;

		if (n != psec->rsa_ctx->len)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("invalid message length"));
		}

		if (C_OK != rsa_pkcs1_decrypt(psec->rsa_ctx, pssl->f_rng, pssl->r_rng, RSA_PRIVATE, &prelen, prec->rcv_msg + msglen, premaster, prelen))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_key_exchange"), _T("rsa_pkcs1_decrypt"));
		}

		if (prelen != SSL_MST_SIZE)// || premaster[0] != pses->major_ver || premaster[1] != pses->minor_ver)
		{
			/*
			* Protection against Bleichenbacher's attack:
			* invalid PKCS#1 v1.5 padding must not cause
			* the connection to end immediately; instead,
			* send a bad_record_mac later in the handshake.
			*/
			//for (i = 0; i < prelen; i++)
			//	premaster[i] = (byte_t)havege_rand(&pcip->rng);
			(*pssl->f_rng)(pssl->r_rng, premaster, prelen);
		}

		msglen += n;
	}

	_ssl_derive_keys(pcip, premaster, prelen);

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	END_CATCH;

	return (pses->authen_client) ? SSL_CERTIFICATE_VERIFY : SSL_CLIENT_CHANGE_CIPHER_SPEC;
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_client_certificate_verify(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int msglen, haslen, n;
	sha256_context sha2;
	byte_t hash[36];
	int alg_hash, alg_sign;

	TRY_CATCH;

	xmem_copy(&sha2, &pcip->sha2, sizeof(sha256_context));

	sha256_finish(&sha2, hash);

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE_VERIFY)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("_ssl_read_rcv_msg"));
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	CertificateVerify;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CERTIFICATE_VERIFY)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("invalid message type"));
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	struct {
           digitally-signed struct {
               opaque handshake_messages[handshake_messages_length];
           }
      } CertificateVerify;
	*/
	msglen = SSL_HSH_SIZE;

	alg_hash = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	alg_sign = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	n = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (n != psec->peer_crt->rsa->len)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("invalid message length"));
	}

	if (C_OK != rsa_pkcs1_verify(psec->peer_crt->rsa, pssl->f_rng, pssl->r_rng, RSA_PUBLIC, RSA_HASH_NONE, 32, hash, prec->rcv_msg + msglen))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("rsa_pkcs1_verify"));
	}

	msglen += n;

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}
	
	END_CATCH;

	return SSL_CLIENT_CHANGE_CIPHER_SPEC;
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_client_change_cipher_spec(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int i;

	TRY_CATCH;

	/*
	struct {
		enum { change_cipher_spec(1), (255) } type;
	} ChangeCipherSpec;
	*/

	if (C_OK != _ssl_read_rcv_msg(pssl))
	{
		raise_user_error(_T("_ssl_parse_client_change_cipher_spec"), _T("_ssl_read_rcv_msg"));
	}

	if (prec->rcv_msg_type != SSL_MSG_CHANGE_CIPHER_SPEC)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_change_cipher_spec"), _T("invalid message type"));
	}

	if (prec->rcv_msg_len != 1 || prec->rcv_msg[0] != 1)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_change_cipher_spec"), _T("invalid message length"));
	}

	//clear recv message control bits
	for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
	{
		prec->rcv_ctr[i] = 0;
	}

	//after read change cipher all record must be crypted recving
	prec->crypted = 1;

	END_CATCH;

	return SSL_CLIENT_FINISHED;
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_parse_client_finished(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int hash_len;
	sha256_context sha2;
	byte_t padbuf[48] = { 0 };
	byte_t mac_buf[36] = { 0 };

	TRY_CATCH;

	xmem_copy(&sha2, &pcip->sha2, sizeof(sha256_context));

	/*
	PRF(master_secret, finished_label, Hash(handshake_messages)) [0..verify_data_length-1];
	*/
	sha256_finish(&sha2, padbuf);
	ssl_prf2(pcip->master_secret, SSL_MST_SIZE, label_client_finished, padbuf, 32, mac_buf, 12);

	hash_len = 12;

	if (C_OK != _ssl_read_rcv_msg(pssl))
	{
		raise_user_error(_T("_ssl_parse_client_finished"), _T("_ssl_read_rcv_msg"));
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	Finished;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_FINISHED)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_finished"), _T("invalid message type"));
	}

	/*
	struct {
	opaque verify_data[12];
	} Finished;
	*/
	if (xmem_comp(prec->rcv_msg + SSL_HSH_SIZE, mac_buf, hash_len) != 0)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_RECORD_MAC);
		raise_user_error(_T("_ssl_parse_client_finished"), _T("invalid message MAC"));
	}

	END_CATCH;

	return SSL_SERVER_CHANGE_CIPHER_SPEC;
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_server_change_cipher_spec(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int i;

	TRY_CATCH;

	/*
	struct {
		enum { change_cipher_spec(1), (255) } type;
	} ChangeCipherSpec;
	*/

	prec->snd_msg_type = SSL_MSG_CHANGE_CIPHER_SPEC;
	prec->snd_msg_len = 1;
	prec->snd_msg[0] = 1;

	//clear send message control bits
	for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
	{
		prec->snd_ctr[i] = 0;
	}

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_change_cipher_spec"), _T("_ssl_write_snd_msg"));
	}

	//after write change cipher all record must be crypted sending
	prec->crypted = 1;

	END_CATCH;

	return SSL_SERVER_FINISHED;
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls12_handshake_states _ssl_write_server_finished(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_cipher_context* pcip = (tls12_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int msglen;
	sha256_context sha2;
	byte_t padbuf[48] = { 0 };
	byte_t* mac_buf;

	TRY_CATCH;

	/*
	struct {
	opaque verify_data[12];
	} Finished;
	*/
	xmem_copy(&sha2, &pcip->sha2, sizeof(sha256_context));

	msglen = SSL_HSH_SIZE;
	mac_buf = prec->snd_msg + msglen;

	/*
	PRF(master_secret, finished_label, Hash(handshake_messages)) [0..verify_data_length-1];
	*/
	sha256_finish(&sha2, padbuf);
	ssl_prf2(pcip->master_secret, SSL_MST_SIZE, label_server_finished, padbuf, 32, mac_buf, 12);

	msglen += 12;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	Finished;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_FINISHED;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_finished"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_HANDSHAKE_OVER;
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

bool_t tls12_handshake_server(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls12_handshake_states state = SSL_HELLO_REQUEST;

	while (state != SSL_HANDSHAKE_OVER)
	{
		switch (state)
		{
		case SSL_HELLO_REQUEST:
			_ssl_alloc_cipher(pssl);

			state = SSL_CLIENT_HELLO;
			break;

			/*
			*  <==   ClientHello
			*/
		case SSL_CLIENT_HELLO:
			state = _ssl_parse_client_hello(pssl);
			break;

			/*
			*  ==>   ServerHello
			*        Certificate
			*      ( ServerKeyExchange  )
			*      ( CertificateRequest )
			*        ServerHelloDone
			*/
		case SSL_SERVER_HELLO:
			state = _ssl_write_server_hello(pssl);
			break;

		case SSL_SERVER_CERTIFICATE:
			state = _ssl_write_server_certificate(pssl);
			break;

		case SSL_SERVER_KEY_EXCHANGE:
			state = _ssl_write_server_key_exchange(pssl);
			break;

		case SSL_CERTIFICATE_REQUEST:
			state = _ssl_write_server_certificate_request(pssl);
			break;

		case SSL_SERVER_HELLO_DONE:
			state = _ssl_write_server_hello_done(pssl);
			break;

			/*
			*  <== ( Certificate/Alert  )
			*        ClientKeyExchange
			*      ( CertificateVerify  )
			*        ChangeCipherSpec
			*        Finished
			*/
		case SSL_CLIENT_CERTIFICATE:
			state = _ssl_parse_client_certificate(pssl);
			break;

		case SSL_CLIENT_KEY_EXCHANGE:
			state = _ssl_parse_client_key_exchange(pssl);
			break;

		case SSL_CERTIFICATE_VERIFY:
			state = _ssl_parse_client_certificate_verify(pssl);
			break;

		case SSL_CLIENT_CHANGE_CIPHER_SPEC:
			state = _ssl_parse_client_change_cipher_spec(pssl);
			break;

		case SSL_CLIENT_FINISHED:
			state = _ssl_parse_client_finished(pssl);
			break;

			/*
			*  ==>   ChangeCipherSpec
			*        Finished
			*/
		case SSL_SERVER_CHANGE_CIPHER_SPEC:
			state = _ssl_write_server_change_cipher_spec(pssl);
			break;

		case SSL_SERVER_FINISHED:
			state = _ssl_write_server_finished(pssl);
			break;
		}

		if (state == SSL_HANDSHAKE_ERROR)
		{
			_ssl_write_alert(pssl);
			break;
		}
	}
		
	pses->handshake_over = (state == SSL_HANDSHAKE_OVER) ? 1 : -1;

	return (pses->handshake_over == 1)? 1 : 0;
}



#endif //XDK_SUPPORT_SOCK
