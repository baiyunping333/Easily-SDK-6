/***********************************************************************
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


typedef struct _tls13_signature_set{
	sword_t sig_alg;
	pk_type_t pk_alg;
	int tls_grp;
	md_type_t md_alg;
}tls13_signature_set;

static tls13_signature_set signature_set[] = {
	{ TLS_ALG_RSA_PKCS1_SHA256, PK_RSA, 0, MD_SHA256 },
	{ TLS_ALG_RSA_PKCS1_SHA384, PK_RSA, 0, MD_SHA384 },
	{ TLS_ALG_RSA_PKCS1_SHA512, PK_RSA, 0, MD_SHA512 },

	{ TLS_ALG_ECDSA_SECP256R1_SHA256, PK_ECDSA, TLS_EC_GROUP_SECP256R1, MD_SHA256 },
	{ TLS_ALG_ECDSA_SECP384R1_SHA384, PK_ECDSA, TLS_EC_GROUP_SECP384R1, MD_SHA384 },
	{ TLS_ALG_ECDSA_SECP512R1_SHA512, PK_ECDSA, TLS_EC_GROUP_SECP512R1, MD_SHA512 },

	{ TLS_ALG_RSA_PSS_RSAE_SHA256, PK_RSASSA_PSS, 0, MD_SHA256 },
	{ TLS_ALG_RSA_PSS_RSAE_SHA384, PK_RSASSA_PSS, 0, MD_SHA384 },
	{ TLS_ALG_RSA_PSS_RSAE_SHA512, PK_RSASSA_PSS, 0, MD_SHA512 },

	{ TLS_ALG_RSA_PSS_SHA256, PK_RSASSA_PSS, 0, MD_SHA256 },
	{ TLS_ALG_RSA_PSS_SHA384, PK_RSASSA_PSS, 0, MD_SHA384 },
	{ TLS_ALG_RSA_PSS_SHA512, PK_RSASSA_PSS, 0, MD_SHA512 },

	{ TLS_ALG_RSA_PKCS1_SHA1, PK_RSA, 0, MD_SHA1 },
	{ TLS_ALG_ECDSA_SHA1, PK_ECDSA, 0, MD_SHA1 },
};

static sword_t named_group[] = {
	TLS_EC_GROUP_SECP256R1,
	TLS_EC_GROUP_SECP384R1,
	TLS_EC_GROUP_SECP512R1,
	//TLS_EC_GROUP_X25519,
	//TLS_EC_GROUP_X448,
};

typedef struct _tls13_ciphers_set{
	int cip_id;	//cipher type
	md_type_t md_type;	//MD type: enum{ MD_SHA256, MD_SHA384 }
	int key_size; //the encrypt and decrypt key size
	int iv_size; //iv block size
}tls13_ciphers_set;

static tls13_ciphers_set client_ciphers[] = {
	{ TLS_AES_128_GCM_SHA256, MD_SHA256, 16, 12 },
	{ TLS_AES_256_GCM_SHA384, MD_SHA384, 32, 12 },
	{ TLS_CHACHA20_POLY1305_SHA256, MD_SHA256, 32, 12 },
};

static tls13_ciphers_set server_ciphers[] = {
	{ TLS_CHACHA20_POLY1305_SHA256, MD_SHA256, 32, 12 },
	{ TLS_AES_128_GCM_SHA256, MD_SHA256, 16, 12 },
	{ TLS_AES_256_GCM_SHA384, MD_SHA384, 32, 12 },
};

static const unsigned char label_client_early_traffic[] = "c e traffic";
static const unsigned char label_client_handshake_traffic[] = "c hs traffic";
static const unsigned char label_client_application_traffic[] = "c ap traffic";
static const unsigned char label_server_handshake_traffic[] = "s hs traffic";
static const unsigned char label_server_application_traffic[] = "s ap traffic";
static const unsigned char label_exporter_master_secret[] = "exp master";
static const unsigned char label_resumption_master_secret[] = "res master";
static const unsigned char label_early_exporter_master_secret[] = "e exp master";

static const byte_t tls13_hello_retry_random[32] = { 0xCF, 0x21, 0xAD, 0x74, 0xE5, 0x9A, 0x61, 0x11, 0xBE, 0x1D, 0x8C, 0x02, 0x1E, 0x65, 0xB8, 0x91,
0xC2, 0xA2, 0x11, 0x16, 0x7A, 0xBB, 0x8C, 0x5E, 0x07, 0x9E, 0x09, 0xE2, 0xC8, 0xA8, 0x33, 0x9C };
static const byte_t tls12_downgrade_radom[8] = { 0x44, 0x4F, 0x57, 0x4E, 0x47, 0x52, 0x44, 0x01 };
static const byte_t tls11_downgrade_radom[8] = { 0x44, 0x4F, 0x57, 0x4E, 0x47, 0x52, 0x44, 0x00 };

typedef enum
{
	SSL_HANDSHAKE_ERROR = -1,

	SSL_HELLO_RETRY_REQUEST = 0,
	SSL_SERVER_HELLO = 2,
	SSL_SERVER_CHANGE_CIPHER_SPEC = 4,
	SSL_SERVER_EXTENSIONS = 6,
	SSL_CERTIFICATE_REQUEST = 8,
	SSL_SERVER_CERTIFICATE = 10,
	SSL_SERVER_CERTIFICATE_VERIFY = 12,
	SSL_SERVER_FINISHED = 14,
	SSL_SERVER_SESSION_TICKET = 16,

	SSL_CLIENT_HELLO = 1,
	SSL_CLIENT_CHANGE_CIPHER_SPEC = 3,
	SSL_CLIENT_CERTIFICATE = 5,
	SSL_CLIENT_CERTIFICATE_VERIFY = 7,
	SSL_CLIENT_FINISHED = 9,

	SSL_HANDSHAKE_OVER = 255
}tls13_handshake_states;

typedef struct _tls13_hs_buffer* tls13_hs_buffer_ptr;

typedef struct _tls13_hs_buffer{
	int hs_type;
	int hs_length;
	byte_t* hs_message;

	tls13_hs_buffer_ptr hs_next;
}tls13_hs_buffer;

typedef struct _tls13_cipher_context{
	//SecurityParameters
	int endpoint;		//ConnectionEnd: { server, client }
		
	tls13_ciphers_set cipher; //the selected cipher

	int exportable;		//IsExportable: { true, false } 
	int compress_method; //CompressionMethod: { null, (0), (255) }
	int psk_mode;

	tls13_signature_set signature; //the selected signature alg

	int tls_ecp_group; //the selected ecp group tls id
	int tls_ecp_format; //the ecp group format: {}

	int ext_major_ver; //the extension major version
	int ext_minor_ver; //the extension minor version

	byte_t rnd_srv[SSL_RND_SIZE]; //server_random
	byte_t rnd_cli[SSL_RND_SIZE]; //client_random

	int psk_length;
	byte_t psk[MD_MAX_SIZE];
	int dhe_length;
	byte_t dhe[SSL_BLK_SIZE];
	int early_secret_length;
	byte_t early_secret[MD_MAX_SIZE]; 
	int handshake_secret_length;
	byte_t handshake_secret[MD_MAX_SIZE];
	int master_secret_length;
	byte_t master_secret[MD_MAX_SIZE];

	//Generated by SecurityParameters
	byte_t iv_enc[SSL_MAX_IVC]; 
	byte_t iv_dec[SSL_MAX_IVC];  
	dword_t ctx_enc[SSL_CTX_SIZE];
	dword_t ctx_dec[SSL_CTX_SIZE];

	int hello_retry;
	//cache the handshake message
	tls13_hs_buffer* hs_buffer;
	
	//Tools
}tls13_cipher_context;


#define IS_GCM_CIPHER(cipher) ((cipher == TLS_AES_128_GCM_SHA256 || \
								cipher == TLS_AES_256_GCM_SHA384)? 1 : 0)

#define IS_CHACHAPOLY_CIPHER(cipher) ((cipher == TLS_CHACHA20_POLY1305_SHA256)? 1 : 0)

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
	case SSL_ALERT_INAPPROPRIATE_FALLBACK:
		set_last_error(_T("alert message"), _T("inappropriate fallback"), -1);
		break;
	case SSL_ALERT_USER_CANCELED:
		set_last_error(_T("alert message"), _T("user canceled"), -1);
		break;
	case SSL_ALERT_NO_RENEGOTIATION:
		set_last_error(_T("alert message"), _T("no renegotiation"), -1);
		break;
	case SSL_ALERT_MISSING_EXTENSION:
		set_last_error(_T("alert message"), _T("missing extension"), -1);
		break;
	case SSL_ALERT_UNSUPPORTED_EXTENSION:
		set_last_error(_T("alert message"), _T("unsupported extension"), -1);
		break;
	case SSL_ALERT_UNRECOGNIZED_NAME:
		set_last_error(_T("alert message"), _T("unrecognized name"), -1);
		break;
	case SSL_ALERT_BAD_CERTIFICATE_STATUS_RESPONSE:
		set_last_error(_T("alert message"), _T("bad certificate status response"), -1);
		break;
	case SSL_ALERT_UNKNOWN_PSK_IDENTITY:
		set_last_error(_T("alert message"), _T("unknown psk identity"), -1);
		break;
	case SSL_ALERT_CERTIFICATE_REQUIRED:
		set_last_error(_T("alert message"), _T("certificate required"), -1);
		break;
	case SSL_ALERT_NO_APPLICATION_PROTOCOL:
		set_last_error(_T("alert message"), _T("no application protocol"), -1);
		break;
	}
}

static void _ssl_save_handshake(tls13_cipher_context* pcip, int type, const byte_t* hs, int len)
{
	tls13_hs_buffer *pb, *pre = pcip->hs_buffer;

	while (pre)
	{
		if (!pre->hs_next)
			break;

		pre = pre->hs_next;
	}

	pb = (tls13_hs_buffer*)xmem_alloc(sizeof(tls13_hs_buffer));
	pb->hs_type = type;
	pb->hs_message = (byte_t*)xmem_clone(hs, len);
	pb->hs_length = len;

	if (!pre)
		pcip->hs_buffer = pb;
	else
		pre->hs_next = pb;
}

static void _ssl_free_handshake(tls13_cipher_context* pcip)
{
	tls13_hs_buffer *pb;

	while (pcip->hs_buffer)
	{
		pb = pcip->hs_buffer;
		pcip->hs_buffer = pb->hs_next;

		xmem_free(pb->hs_message);
		xmem_free(pb);
	}
}

static bool_t _ssl_choose_cipher(tls13_cipher_context* pcip, int ciph)
{
	int i, n;
	tls13_ciphers_set* pcs;

	if (pcip->endpoint == SSL_TYPE_CLIENT)
	{
		n = sizeof(client_ciphers) / sizeof(tls13_ciphers_set);
		pcs = client_ciphers;
	}
	else
	{
		n = sizeof(server_ciphers) / sizeof(tls13_ciphers_set);
		pcs = server_ciphers;
	}

	for (i = 0; i < n; i++)
	{
		if (ciph == pcs[i].cip_id)
		{
			pcip->cipher.cip_id = pcs[i].cip_id;
			pcip->cipher.md_type = pcs[i].md_type;
			pcip->cipher.key_size = pcs[i].key_size;
			pcip->cipher.iv_size = pcs[i].iv_size;

			return 1;
		}
	}

	set_last_error(_T("_ssl_choose_cipher"), _T("unknown cipher"), -1);

	return 0;
}

static bool_t _ssl_select_signature(int sig_alg, int* sig_pk, int* sig_md)
{
	int i, n;
	
	n = sizeof(signature_set) / sizeof(tls13_signature_set);
	
	for (i = 0; i < n; i++)
	{
		if (sig_alg == signature_set[i].sig_alg)
		{
			*sig_pk = signature_set[i].pk_alg;
			*sig_md = signature_set[i].md_alg;

			return 1;
		}
	}

	return 0;
}

static void _ssl_gen_message_hash(tls13_cipher_context* pcip, byte_t* hash, int* plen)
{
	tls13_hs_buffer *pb = pcip->hs_buffer;
	const md_info_t* md_info;
	void* md_ctx;

	/*
	* Transcript-Hash(ClientHello1) = Hash(message_hash ||  -- Handshake type
	* 00 00 Hash.length ||  -- Handshake message length (bytes)
	* Hash(ClientHello1)  -- Hash of ClientHello1
	* )
	*/

	//locate the first ClientHello message
	while (pb)
	{
		if (pb->hs_type == SSL_HS_CLIENT_HELLO)
			break;

		pb = pb->hs_next;
	}

	if (!pb)
	{
		*plen = 0;
		return;
	}

	md_info = md_info_from_type(pcip->cipher.md_type);
	if (!md_info)
	{
		*plen = 0;
		return;
	}

	//generate ClientHello1 hash
	md_ctx = md_alloc(md_info);
	md_starts(md_info, md_ctx);
	md_update(md_info, md_ctx, pb->hs_message, pb->hs_length);
	md_finish(md_info, md_ctx, hash);
	*plen = md_info->size;
	md_free(md_info, md_ctx);

	//replace ClientHello1 with MessageHash
	pb->hs_type = SSL_HS_MESSAGE_HASH;
	pb->hs_length = 4 + *plen;
	pb->hs_message = (byte_t*)xmem_realloc(pb->hs_message, pb->hs_length);
	PUT_BYTE(pb->hs_message, 0, SSL_HS_MESSAGE_HASH);
	PUT_THREEBYTE_NET(pb->hs_message, 1, md_info->size);
	xmem_copy((void*)(pb->hs_message + 4), (void*)hash, *plen);
}

static void _ssl_gen_handshake_hash(tls13_cipher_context* pcip, byte_t* hash, int* plen)
{
	tls13_hs_buffer *pb = pcip->hs_buffer;
	const md_info_t* md_info;
	void* md_ctx;

	if (!pb)
	{
		*plen = 0;
		return;
	}

	md_info = md_info_from_type(pcip->cipher.md_type);
	if (!md_info)
	{
		*plen = 0;
		return;
	}
	md_ctx = md_alloc(md_info);
	md_starts(md_info, md_ctx);
	while (pb)
	{
		md_update(md_info, md_ctx, pb->hs_message, pb->hs_length);

		pb = pb->hs_next;
	}
	md_finish(md_info, md_ctx, hash);
	*plen = md_info->size;

	md_free(md_info, md_ctx);
}

static void _ssl_gen_hello_hash(tls13_cipher_context* pcip, byte_t* hash, int* plen)
{
	tls13_hs_buffer *pb = pcip->hs_buffer;
	const md_info_t* md_info;
	void* md_ctx;
	int tag = 0;

	if (!pb)
	{
		*plen = 0;
		return;
	}

	md_info = md_info_from_type(pcip->cipher.md_type);
	if (!md_info)
	{
		*plen = 0;
		return;
	}
	md_ctx = md_alloc(md_info);
	md_starts(md_info, md_ctx);
	while (pb)
	{
		if (!tag && pb->hs_type == SSL_HS_CLIENT_HELLO)
			tag = 1;

		if (tag)
		{
			md_update(md_info, md_ctx, pb->hs_message, pb->hs_length);
		}

		if (tag && pb->hs_type == SSL_HS_SERVER_HELLO)
			tag = 0;

		pb = pb->hs_next;
	}
	md_finish(md_info, md_ctx, hash);
	*plen = md_info->size;

	md_free(md_info, md_ctx);
}

static void _ssl_gen_finished_hash(tls13_cipher_context* pcip, byte_t* hash, int* plen)
{
	tls13_hs_buffer *pb = pcip->hs_buffer;
	const md_info_t* md_info;
	void* md_ctx;
	int tag = 0;

	if (!pb)
	{
		*plen = 0;
		return;
	}

	md_info = md_info_from_type(pcip->cipher.md_type);
	if (!md_info)
	{
		*plen = 0;
		return;
	}
	md_ctx = md_alloc(md_info);
	md_starts(md_info, md_ctx);
	while (pb)
	{
		if (!tag && pb->hs_type == SSL_HS_CLIENT_HELLO)
			tag = 1;

		if (tag)
		{
			md_update(md_info, md_ctx, pb->hs_message, pb->hs_length);
		}

		if (tag && pb->hs_type == SSL_HS_FINISHED)
			tag = 0;

		pb = pb->hs_next;
	}
	md_finish(md_info, md_ctx, hash);
	*plen = md_info->size;

	md_free(md_info, md_ctx);
}

static void _ssl_extract_early_secret(tls13_cipher_context* pcip)
{
	const md_info_t* md_info;

	md_info = md_info_from_type(pcip->cipher.md_type);
	//PSK ->  HKDF-Extract = Early Secret
	ssl_extract(pcip->cipher.md_type, NULL, 0, NULL, 0, pcip->early_secret, &pcip->early_secret_length);
}

static void _ssl_extract_handshake_secret(tls13_cipher_context* pcip)
{
	byte_t salt[MD_MAX_SIZE] = { 0 };
	int salt_len;

	const md_info_t* md_info;

	md_info = md_info_from_type(pcip->cipher.md_type);

	//Derive-Secret(., "derived", "")
	salt_len = md_info->size;
	ssl_expand(pcip->cipher.md_type, pcip->early_secret, pcip->early_secret_length, "derived", "", 0, salt, salt_len);

	//(EC)DHE -> HKDF-Extract = Handshake Secret
	ssl_extract(pcip->cipher.md_type, pcip->dhe, pcip->dhe_length, salt, salt_len, pcip->handshake_secret, &pcip->handshake_secret_length);
}

static void _ssl_derive_handshake_key(tls13_cipher_context* pcip)
{
	byte_t hash[MD_MAX_SIZE] = { 0 };
	int hlen;

	byte_t okm_cli[MD_MAX_SIZE] = { 0 };
	byte_t okm_srv[MD_MAX_SIZE] = { 0 };
	int okm_len;

	byte_t blk[SSL_BLK_SIZE] = { 0 };

	byte_t *key_enc, *key_dec;
	
	//transcript-hash [ClientHello...ServerHello]
	_ssl_gen_hello_hash(pcip, hash, &hlen);

	//Derive-Secret(., "c hs traffic", ClientHello...ServerHello) = client_handshake_traffic_secret
	//Derive-Secret(., "s hs traffic", ClientHello...ServerHello) = server_handshake_traffic_secret
	okm_len = hlen;
	ssl_expand(pcip->cipher.md_type, pcip->handshake_secret, pcip->handshake_secret_length, label_client_handshake_traffic, hash, hlen, okm_cli, okm_len);
	ssl_expand(pcip->cipher.md_type, pcip->handshake_secret, pcip->handshake_secret_length, label_server_handshake_traffic, hash, hlen, okm_srv, okm_len);

	//[sender]_write_key = HKDF-Expand-Label(Secret, "key", "", key_length)
	//[sender]_write_iv  = HKDF-Expand-Label(Secret, "iv", "", iv_length)
	ssl_expand(pcip->cipher.md_type, okm_cli, okm_len, "key", "", 0, blk, pcip->cipher.key_size);
	ssl_expand(pcip->cipher.md_type, okm_srv, okm_len, "key", "", 0, (blk + pcip->cipher.key_size), pcip->cipher.key_size);
	ssl_expand(pcip->cipher.md_type, okm_cli, okm_len, "iv", "", 0, (blk + pcip->cipher.key_size * 2), pcip->cipher.iv_size);
	ssl_expand(pcip->cipher.md_type, okm_srv, okm_len, "iv", "", 0, (blk + pcip->cipher.key_size * 2 + pcip->cipher.iv_size), pcip->cipher.iv_size);

	if (pcip->endpoint == SSL_TYPE_CLIENT)
	{
		//client_write_key for client setup encrypting context
		key_enc = blk;
		//server_write_key for client setup decrypting context
		key_dec = (blk + pcip->cipher.key_size);
		//client_write_IV for client encrypting IV
		xmem_copy(pcip->iv_enc, (blk + pcip->cipher.key_size * 2), pcip->cipher.iv_size);
		//server_write_IV for client decrypting IV
		xmem_copy(pcip->iv_dec, (blk + pcip->cipher.key_size * 2 + pcip->cipher.iv_size), pcip->cipher.iv_size);
	}
	else
	{
		//client_write_key for server decrypting context
		key_dec = blk;
		//server_write_key for server encrypting context
		key_enc = (blk + pcip->cipher.key_size);
		//client_write_IV for server decrypting IV
		xmem_copy(pcip->iv_dec, (blk + pcip->cipher.key_size * 2), pcip->cipher.iv_size);
		//server_write_IV for server encrypting IV
		xmem_copy(pcip->iv_enc, (blk + pcip->cipher.key_size * 2 + pcip->cipher.iv_size), pcip->cipher.iv_size);
	}

	//initialize encrypt and decrypt context
	switch (pcip->cipher.cip_id)
	{
	case TLS_AES_128_GCM_SHA256:
	case TLS_AES_256_GCM_SHA384:
		gcm_setkey((gcm_context *)pcip->ctx_enc, key_enc, (pcip->cipher.key_size * 8));
		gcm_setkey((gcm_context *)pcip->ctx_dec, key_dec, (pcip->cipher.key_size * 8));
		break;
	case TLS_CHACHA20_POLY1305_SHA256:
		chachapoly_init((chachapoly_context *)pcip->ctx_enc);
		chachapoly_init((chachapoly_context *)pcip->ctx_dec);
		chachapoly_setkey((chachapoly_context *)pcip->ctx_enc, key_enc);
		chachapoly_setkey((chachapoly_context *)pcip->ctx_dec, key_dec);
		break;
	}
}

static void _ssl_extract_master_secret(tls13_cipher_context* pcip)
{
	byte_t salt[MD_MAX_SIZE] = { 0 };
	int salt_len;

	const md_info_t* md_info;

	md_info = md_info_from_type(pcip->cipher.md_type);

	//Derive-Secret(., "derived", "")
	salt_len = md_info->size;
	ssl_expand(pcip->cipher.md_type, pcip->handshake_secret, pcip->handshake_secret_length, "derived", "", 0, salt, salt_len);

	//(EC)DHE -> HKDF-Extract = Handshake Secret
	ssl_extract(pcip->cipher.md_type, NULL, 0, salt, salt_len, pcip->master_secret, &pcip->master_secret_length);
}

static void _ssl_derive_application_key(tls13_cipher_context* pcip)
{
	byte_t hash[MD_MAX_SIZE] = { 0 };
	int hlen;

	byte_t okm_cli[MD_MAX_SIZE] = { 0 };
	byte_t okm_srv[MD_MAX_SIZE] = { 0 };
	int okm_len;

	byte_t blk[SSL_BLK_SIZE] = { 0 };

	byte_t *key_enc, *key_dec;

	//transcript-hash [ClientHello...ServerFinished]
	_ssl_gen_finished_hash(pcip, hash, &hlen);

	//Derive-Secret(., "c ap traffic", ClientHello...ServerFinished) = client_application_traffic_secret
	//Derive-Secret(., "s ap traffic", ClientHello...ServerFinished) = server_application_traffic_secret
	okm_len = hlen;
	ssl_expand(pcip->cipher.md_type, pcip->master_secret, pcip->master_secret_length, label_client_application_traffic, hash, hlen, okm_cli, okm_len);
	ssl_expand(pcip->cipher.md_type, pcip->master_secret, pcip->master_secret_length, label_server_application_traffic, hash, hlen, okm_srv, okm_len);

	//[sender]_write_key = HKDF-Expand-Label(Secret, "key", "", key_length)
	//[sender]_write_iv  = HKDF-Expand-Label(Secret, "iv", "", iv_length)
	ssl_expand(pcip->cipher.md_type, okm_cli, okm_len, "key", "", 0, blk, pcip->cipher.key_size);
	ssl_expand(pcip->cipher.md_type, okm_srv, okm_len, "key", "", 0, (blk + pcip->cipher.key_size), pcip->cipher.key_size);
	ssl_expand(pcip->cipher.md_type, okm_cli, okm_len, "iv", "", 0, (blk + pcip->cipher.key_size * 2), pcip->cipher.iv_size);
	ssl_expand(pcip->cipher.md_type, okm_srv, okm_len, "iv", "", 0, (blk + pcip->cipher.key_size * 2 + pcip->cipher.iv_size), pcip->cipher.iv_size);

	if (pcip->endpoint == SSL_TYPE_CLIENT)
	{
		//client_write_key for client setup encrypting context
		key_enc = blk;
		//server_write_key for client setup decrypting context
		key_dec = (blk + pcip->cipher.key_size);
		//client_write_IV for client encrypting IV
		xmem_copy(pcip->iv_enc, (blk + pcip->cipher.key_size * 2), pcip->cipher.iv_size);
		//server_write_IV for client decrypting IV
		xmem_copy(pcip->iv_dec, (blk + pcip->cipher.key_size * 2 + pcip->cipher.iv_size), pcip->cipher.iv_size);
	}
	else
	{
		//client_write_key for server decrypting context
		key_dec = blk;
		//server_write_key for server encrypting context
		key_enc = (blk + pcip->cipher.key_size);
		//client_write_IV for server decrypting IV
		xmem_copy(pcip->iv_dec, (blk + pcip->cipher.key_size * 2), pcip->cipher.iv_size);
		//server_write_IV for server encrypting IV
		xmem_copy(pcip->iv_enc, (blk + pcip->cipher.key_size * 2 + pcip->cipher.iv_size), pcip->cipher.iv_size);
	}

	//initialize encrypt and decrypt context
	switch (pcip->cipher.cip_id)
	{
	case TLS_AES_128_GCM_SHA256:
	case TLS_AES_256_GCM_SHA384:
		gcm_setkey((gcm_context *)pcip->ctx_enc, key_enc, (pcip->cipher.key_size * 8));
		gcm_setkey((gcm_context *)pcip->ctx_dec, key_dec, (pcip->cipher.key_size * 8));
		break;
	case TLS_CHACHA20_POLY1305_SHA256:
		chachapoly_init((chachapoly_context *)pcip->ctx_enc);
		chachapoly_init((chachapoly_context *)pcip->ctx_dec);
		chachapoly_setkey((chachapoly_context *)pcip->ctx_enc, key_enc);
		chachapoly_setkey((chachapoly_context *)pcip->ctx_dec, key_dec);
		break;
	}
}

static void _ssl_reset_sequence_number(ssl_session_context* pses)
{
	ssl_record_context* pr = (ssl_record_context*)pses->rcv_record;
	ssl_record_context* pw = (ssl_record_context*)pses->snd_record;
	int i;
	//clear send and recv message control bits
	for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
	{
		pr->rcv_ctr[i] = 0;
		pw->snd_ctr[i] = 0;
	}

	pr->crypted = 1;
	pw->crypted = 1;
}

static int _ssl_encrypt_snd_msg(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	byte_t* mac_buf;

	byte_t addbuf[SSL_HDR_SIZE] = { 0 };
	byte_t nonce[SSL_MAX_IVC] = { 0 };
	int i, addlen, taglen = 16;;

	if (IS_GCM_CIPHER(pcip->cipher.cip_id))
	{
		//Per-Record Nonce
		//The 64-bit record sequence number is encoded in network byte order and padded to the left with zeros to iv_length.
		xmem_copy((void*)(nonce + pcip->cipher.iv_size - SSL_CTR_SIZE), (void*)prec->snd_ctr, SSL_CTR_SIZE);
		//The padded sequence number is XOR with either the static client_write_iv or server_write_iv(depending on the role).
		for (i = 0; i < pcip->cipher.iv_size; i++)
		{
			nonce[i] ^= pcip->iv_enc[i];
		}

		//additional_data = TLSCiphertext.opaque_type || TLSCiphertext.legacy_record_version || TLSCiphertext.length
		addlen = 0;
		addbuf[addlen] = SSL_MSG_APPLICATION_DATA;
		addbuf[addlen + 1] = SSL_MAJOR_VERSION_3;
		addbuf[addlen + 2] = SSL_MINOR_VERSION_3;
		addlen += 3;
		PUT_SWORD_NET(addbuf, addlen, (unsigned short)(prec->snd_msg_len + taglen));
		addlen += 2;
		
		mac_buf = prec->snd_msg + prec->snd_msg_len;

		if (C_OK != gcm_crypt_and_tag((gcm_context *)pcip->ctx_enc, AES_ENCRYPT, prec->snd_msg_len, nonce, pcip->cipher.iv_size, addbuf, addlen, prec->snd_msg, prec->snd_msg, taglen, mac_buf))
		{
			set_last_error(_T("_ssl_encrypt_snd_msg"), _T("gcm_crypt_and_tag falied"), -1);

			return C_ERR;
		}
		prec->snd_msg_len += taglen;
	}
	else if (IS_CHACHAPOLY_CIPHER(pcip->cipher.cip_id))
	{
		//Per-Record Nonce
		//The 64-bit record sequence number is encoded in network byte order and padded to the left with zeros to iv_length.
		xmem_copy((void*)(nonce + pcip->cipher.iv_size - SSL_CTR_SIZE), (void*)prec->snd_ctr, SSL_CTR_SIZE);
		//The padded sequence number is XOR with either the static client_write_iv or server_write_iv(depending on the role).
		for (i = 0; i < pcip->cipher.iv_size; i++)
		{
			nonce[i] ^= pcip->iv_enc[i];
		}

		//additional_data = TLSCiphertext.opaque_type || TLSCiphertext.legacy_record_version || TLSCiphertext.length
		addlen = 0;
		addbuf[addlen] = SSL_MSG_APPLICATION_DATA;
		addbuf[addlen + 1] = SSL_MAJOR_VERSION_3;
		addbuf[addlen + 2] = SSL_MINOR_VERSION_3;
		addlen += 3;
		PUT_SWORD_NET(addbuf, addlen, (unsigned short)(prec->snd_msg_len + taglen));
		addlen += 2;

		mac_buf = prec->snd_msg + prec->snd_msg_len;

		if (C_OK != chachapoly_crypt_and_tag((chachapoly_context *)pcip->ctx_enc, CHACHAPOLY_ENCRYPT, prec->snd_msg_len, nonce, addbuf, addlen, prec->snd_msg, prec->snd_msg, mac_buf))
		{
			set_last_error(_T("_ssl_encrypt_snd_msg"), _T("chachapoly_crypt_and_tag"), -1);

			return C_ERR;
		}
		prec->snd_msg_len += taglen;
	}

	/*
	struct {
	ContentType opaque_type = application_data(23)
	ProtocolVersion legacy_record_version = 0x0303;
	uint16 length;
	opaque encrypted_record[TLSCiphertext.length];
	} TLSCiphertext;
	*/
	PUT_BYTE(prec->snd_hdr, 0, SSL_MSG_APPLICATION_DATA);
	PUT_BYTE(prec->snd_hdr, 1, SSL_MAJOR_VERSION_3);
	PUT_BYTE(prec->snd_hdr, 2, SSL_MINOR_VERSION_3);
	//reset message length
	PUT_SWORD_NET(prec->snd_hdr, 3, (unsigned short)prec->snd_msg_len);

	return C_OK;
}

static int _ssl_decrypt_rcv_msg(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	byte_t* mac_buf;
	byte_t mac_tmp[32];

	byte_t addbuf[SSL_HDR_SIZE] = { 0 };
	byte_t nonce[SSL_MAX_IVC] = { 0 };
	int i, addlen, taglen = 16;

	/*
	struct {
	ContentType opaque_type = application_data(23)
	ProtocolVersion legacy_record_version = 0x0303;
	uint16 length;
	opaque encrypted_record[TLSCiphertext.length];
	} TLSCiphertext;
	*/

	if (prec->rcv_msg_len < taglen)
	{
		set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("message length to small"), -1);

		return C_ERR;
	}

	if (IS_GCM_CIPHER(pcip->cipher.cip_id))
	{
		//Per-Record Nonce
		//The 64-bit record sequence number is encoded in network byte order and padded to the left with zeros to iv_length.
		xmem_copy((void*)(nonce + pcip->cipher.iv_size - SSL_CTR_SIZE), (void*)prec->rcv_ctr, SSL_CTR_SIZE);
		//The padded sequence number is XORed with either the static client_write_iv or server_write_iv(depending on the role).
		for (i = 0; i < pcip->cipher.iv_size; i++)
		{
			nonce[i] ^= pcip->iv_dec[i];
		}

		taglen = 16;
		prec->rcv_msg_len -= taglen;
		mac_buf = prec->rcv_msg + prec->rcv_msg_len;

		//reset message length
		PUT_SWORD_NET(prec->rcv_hdr, 3, (unsigned short)prec->rcv_msg_len);

		//additional_data = TLSCiphertext.opaque_type || TLSCiphertext.legacy_record_version || TLSCiphertext.length
		addlen = 0;
		addbuf[addlen] = SSL_MSG_APPLICATION_DATA;
		addbuf[addlen + 1] = SSL_MAJOR_VERSION_3;
		addbuf[addlen + 2] = SSL_MINOR_VERSION_3;
		addlen += 3;
		PUT_SWORD_NET(addbuf, addlen, (unsigned short)(prec->rcv_msg_len + taglen));
		addlen += 2;

		if (C_OK != gcm_crypt_and_tag((gcm_context *)pcip->ctx_dec, AES_DECRYPT, prec->rcv_msg_len, nonce, pcip->cipher.iv_size, addbuf, addlen, prec->rcv_msg, prec->rcv_msg, taglen, mac_tmp))
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
	else if (IS_CHACHAPOLY_CIPHER(pcip->cipher.cip_id))
	{
		//Per-Record Nonce
		//The 64-bit record sequence number is encoded in network byte order and padded to the left with zeros to iv_length.
		xmem_copy((void*)(nonce + pcip->cipher.iv_size - SSL_CTR_SIZE), (void*)prec->rcv_ctr, SSL_CTR_SIZE);
		//The padded sequence number is XORed with either the static client_write_iv or server_write_iv(depending on the role).
		for (i = 0; i < pcip->cipher.iv_size; i++)
		{
			nonce[i] ^= pcip->iv_dec[i];
		}

		taglen = 16;
		prec->rcv_msg_len -= taglen;
		mac_buf = prec->rcv_msg + prec->rcv_msg_len;

		//reset message length
		PUT_SWORD_NET(prec->rcv_hdr, 3, (unsigned short)prec->rcv_msg_len);

		//additional_data = TLSCiphertext.opaque_type || TLSCiphertext.legacy_record_version || TLSCiphertext.length
		addlen = 0;
		addbuf[addlen] = SSL_MSG_APPLICATION_DATA;
		addbuf[addlen + 1] = SSL_MAJOR_VERSION_3;
		addbuf[addlen + 2] = SSL_MINOR_VERSION_3;
		addlen += 3;
		PUT_SWORD_NET(addbuf, addlen, (unsigned short)(prec->rcv_msg_len + taglen));
		addlen += 2;

		if (C_OK != chachapoly_crypt_and_tag((chachapoly_context *)pcip->ctx_dec, CHACHAPOLY_DECRYPT, prec->rcv_msg_len, nonce, addbuf, addlen, prec->rcv_msg, prec->rcv_msg, mac_tmp))
		{
			set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("chachapoly_crypt_and_tag"), -1);

			return C_ERR;
		}

		if (xmem_comp((void*)mac_tmp, (void*)mac_buf, taglen) != 0)
		{
			set_last_error(_T("_ssl_decrypt_rcv_msg"), _T("gcm tag checked falied"), -1);

			return C_ERR;
		}
	}

	return C_OK;
}

static int _ssl_write_snd_msg(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	dword_t dw;
	int i, haslen, hastype;
	byte_t* token;
	int total;

	/*
	struct {
	ContentType type;
	ProtocolVersion legacy_record_version;
	uint16 length;
	opaque fragment[TLSPlaintext.length];
	} TLSPlaintext;
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

		while (total > 0)
		{
			hastype = GET_BYTE(token, 0);
			haslen = GET_THREEBYTE_NET(token, 1);

			_ssl_save_handshake(pcip, hastype, token, SSL_HSH_SIZE + haslen);

			total -= (SSL_HSH_SIZE + haslen);
			token += (SSL_HSH_SIZE + haslen);
		}
	}

	if (prec->crypted)
	{
		/*
		struct {
		opaque content[TLSPlaintext.length];
		ContentType type;
		uint8 zeros[length_of_padding];
		} TLSInnerPlaintext;
		*/
		PUT_BYTE(prec->snd_msg, prec->snd_msg_len, prec->snd_msg_type);
		prec->snd_msg_len++;
		PUT_BYTE(prec->snd_msg, prec->snd_msg_len, 0x00);
		prec->snd_msg_len++;
		//reset message size
		PUT_SWORD_NET(prec->snd_hdr, 3, (unsigned short)(prec->snd_msg_len));

		if (C_OK != _ssl_encrypt_snd_msg(pssl))
		{
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
		set_last_error(_T("_ssl_write_snd_msg"), _T("write message block failed"), -1);
		return C_ERR;
	}

	prec->snd_msg_pop = 0;

	return C_OK;
}

static int _ssl_read_rcv_msg(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	dword_t dw;
	int i, haslen, hastype;
	byte_t* token;
	int total;

restart:

	//if the head already readed at handshake begain
	if (pses->major_ver)
	{
		dw = SSL_HDR_SIZE;
		if (!(pssl->pif->pf_read)(pssl->pif->fd, prec->rcv_hdr, &dw))
		{
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
			set_last_error(_T("_ssl_read_rcv_msg"), _T("major version mismatch"), -1);
			return C_ERR;
		}

		if (prec->rcv_hdr[2] > SSL_MINOR_VERSION_4)
		{
			set_last_error(_T("_ssl_read_rcv_msg"), _T("minor version mismatch"), -1);
			return C_ERR;
		}

		prec->rcv_msg_type = GET_BYTE(prec->rcv_hdr, 0);
		prec->rcv_msg_len = GET_SWORD_NET(prec->rcv_hdr, 3);

		if (prec->rcv_msg_len < 1 || prec->rcv_msg_len > SSL_MAX_SIZE)
		{
			set_last_error(_T("_ssl_read_rcv_msg"), _T("invalid message block length"), -1);
			return C_ERR;
		}

		dw = prec->rcv_msg_len;
		if (!(*pssl->pif->pf_read)(pssl->pif->fd, prec->rcv_msg, &dw))
		{
			set_last_error(_T("_ssl_read_rcv_msg"), _T("read message block failed"), -1);
			return C_ERR;
		}
	}

	if (prec->crypted)
	{
		if (C_OK != _ssl_decrypt_rcv_msg(pssl))
		{
			set_last_error(_T("_ssl_read_rcv_msg"), _T("decrypt message block failed"), -1);
			return C_ERR;
		}

		/*
		* struct {
		* opaque content[TLSPlaintext.length];
		* ContentType type;
		* uint8 zeros[length_of_padding];
		* } TLSInnerPlaintext;
		*/
		while (GET_BYTE(prec->rcv_msg, (prec->rcv_msg_len-1)) == 0x00)
		{
			prec->rcv_msg_len--;
		}
		prec->rcv_msg_type = GET_BYTE(prec->rcv_msg, (prec->rcv_msg_len-1));
		prec->rcv_msg_len--;

		//incre recv message control bits
		for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
		{
			if (++prec->rcv_ctr[i] != 0)
				break;
		}
	}

	/*
	* struct {
	* ContentType type;
	* ProtocolVersion legacy_record_version;
	* uint16 length;
	* opaque fragment[TLSPlaintext.length];
	* } TLSPlaintext;
	*/

	//will hash all handshake message received
	if (prec->rcv_msg_type == SSL_MSG_HANDSHAKE)
	{
		total = prec->rcv_msg_len;
		token = prec->rcv_msg;
		while (total > 0)
		{
			hastype = GET_BYTE(token, 0);
			haslen = GET_THREEBYTE_NET(token, 1);

			_ssl_save_handshake(pcip, hastype, token, SSL_HSH_SIZE + haslen);

			total -= (SSL_HSH_SIZE + haslen);
			token += (SSL_HSH_SIZE + haslen);
		}
	}
	else if (prec->rcv_msg_type == SSL_MSG_ALERT)
	{
		if (prec->rcv_msg[0] == SSL_LEVEL_FATAL)
		{
			set_last_error(_T("_ssl_read_rcv_msg"), _T("fatal alert message"), -1);
			return C_ERR;
		}

		if (prec->rcv_msg[0] == SSL_LEVEL_WARNING && prec->rcv_msg[1] == SSL_ALERT_CLOSE_NOTIFY)
		{
			pses->handshake_over = -1;
			prec->rcv_msg_len = 0;
		}
	}
	else if (prec->rcv_msg_type == SSL_MSG_CHANGE_CIPHER_SPEC)
	{
		//clear send message control bits
		for (i = SSL_CTR_SIZE - 1; i >= 0; i--)
		{
			prec->rcv_ctr[i] = 0;
		}

		goto restart;
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
	tls13_cipher_context* pcip = (pses) ? (tls13_cipher_context*)pses->cipher_context : NULL;

	if (pcip)
	{
		_ssl_free_handshake(pcip);

		xmem_free(pcip);
		pses->cipher_context = NULL;
	}
}

static tls13_cipher_context* _ssl_alloc_cipher(ssl_context* pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip;

	pcip = (tls13_cipher_context*)xmem_alloc(sizeof(tls13_cipher_context));

	pcip->endpoint = pssl->type;

	pses->cipher_context = (ssl_cipher_context_ptr)pcip;

	pssl->ssl_send = _ssl_write_snd_msg;
	pssl->ssl_recv = _ssl_read_rcv_msg;

	pses->free_cipher_context = _ssl_free_cipher;

	if (pssl->type == SSL_TYPE_SERVER)
	{
		pssl->srv_major_ver = SSL_MAJOR_VERSION_3;
		pssl->srv_minor_ver = SSL_MINOR_VERSION_4;
	}

	return pcip;
}
/***************************************client routing************************************************************/

static tls13_handshake_states _ssl_write_client_hello(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int i, n, msglen, extlen, lstlen, grplen;
	int tls_id;
	dword_t t;
	const ecp_curve_info* curve;
	x509_crt* pcrt;

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
        ProtocolVersion legacy_version = 0x0303; 
		Random random;
		opaque legacy_session_id<0..32>;
		CipherSuite cipher_suites<2..2 ^ 16 - 2>;
		opaque legacy_compression_methods<1..2 ^ 8 - 1>;
		Extension extensions<8..2 ^ 16 - 1>;
	} ClientHello;
	*/

	/*
	ProtocolVersion
	*/
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pses->major_ver));
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pses->minor_ver));
	
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
	n = sizeof(client_ciphers) / sizeof(tls13_ciphers_set);

	//cipher list length
	PUT_SWORD_NET(prec->snd_msg, msglen, n * 2);
	msglen += 2;

	for (i = 0; i < n; i++)
	{
		//cipher
		PUT_SWORD_NET(prec->snd_msg, msglen, client_ciphers[i].cip_id);
		msglen += 2;
	}

	/*
	CompressionMethod
	*/
	//compression methods length
	PUT_BYTE(prec->snd_msg, msglen++, 1);
	//compression method (null)
	PUT_BYTE(prec->snd_msg, msglen++, 0);

	/*
	struct {
	ExtensionType extension_type;
	opaque extension_data<0..2^16-1>;
	} Extension;
	*/

	// preset Extensions length to zero
	extlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(extlen));
	msglen += 2;

	// Extension type: supported_version (43)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SUPPORTED_VERSION);
	msglen += 2;
	extlen += 2;

	lstlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, lstlen);
	msglen += 2;
	extlen += 2;

	/*
	* struct {
	*	select (Handshake.msg_type) {
	*	case client_hello:
	*	ProtocolVersion versions<2..254>;
	*	case server_hello:
	*	ProtocolVersion selected_version;
	*	};
	* } SupportedVersions;
	*/
	PUT_BYTE(prec->snd_msg, msglen, 2);
	msglen += 1;
	extlen += 1;
	lstlen += 1;

	PUT_BYTE(prec->snd_msg, msglen, SSL_MAJOR_VERSION_3);
	msglen += 1;
	extlen += 1;
	lstlen += 1;

	PUT_BYTE(prec->snd_msg, msglen, SSL_MINOR_VERSION_4);
	msglen += 1;
	extlen += 1;
	lstlen += 1;

	//reset length
	PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);

	//init signature schema
	pcip->signature.sig_alg = signature_set[0].sig_alg;
	pcip->signature.pk_alg = signature_set[0].pk_alg;
	pcip->signature.md_alg = signature_set[0].md_alg;
	pcip->signature.tls_grp = signature_set[0].tls_grp;

	// Extension type: Signature and Hash algorithm(13)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SIGNATUREANDHASHALGORITHM);
	msglen += 2;
	extlen += 2;

	// Signature and Hash extension length
	lstlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(lstlen));
	msglen += 2;
	extlen += 2;
	/*
	* struct {
	* HashAlgorithm hash;
	* SignatureAlgorithm signature;
	* } SignatureAndHashAlgorithm;
	*/
	n = sizeof(signature_set) / sizeof(sword_t) * 2;
	// Signature and Hash algorithm list count length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(n));
	msglen += 2;
	extlen += 2;
	lstlen += 2;

	n = sizeof(signature_set) / sizeof(sword_t);
	for (i = 0; i < n; i++)
	{
		//HashAlgorithm And SignatureAlgorithm
		PUT_SWORD_NET(prec->snd_msg, msglen, signature_set[i].sig_alg);
		msglen += 2;
		extlen += 2;
		lstlen += 2;
	}

	//reset length
	PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);

	/*struct {
	* NamedCurve elliptic_curve_list<1..2 ^ 16 - 1>
	} EllipticCurveList;
	*/
	// Extension type: supported_groups (10)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SUPPORTEDGROUPS);
	msglen += 2;
	extlen += 2;

	lstlen = 0;
	// supported_groups extension length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)lstlen);
	msglen += 2;
	extlen += 2;

	n = sizeof(named_group) / sizeof(sword_t) * 2;
	// Elliptic curve list length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(n));
	msglen += 2;
	extlen += 2;
	lstlen += 2;

	n = sizeof(named_group) / sizeof(sword_t);
	for (i = 0; i < n; i++)
	{
		// Supported Group
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(named_group[i]));
		msglen += 2;
		extlen += 2;
		lstlen += 2;
	}

	//reset length
	PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);

	// Extension type: ec_point_formats (11)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_ECPOINTFORMATS);
	msglen += 2;
	extlen += 2;

	// ec_point_format extension length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(2));
	msglen += 2;
	extlen += 2;

	/*struct {
	* ECPointFormat ec_point_format_list<1..2 ^ 8 - 1>
	} ECPointFormatList;
	*/

	//EC point formats Length: 1
	PUT_BYTE(prec->snd_msg, msglen, (1));
	msglen++;
	extlen++;

	//EC point format: uncompressed (0)
	PUT_BYTE(prec->snd_msg, msglen, (0));
	msglen++;
	extlen++;

	// Extension type: psk mode (45)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_PSK_KEY_EXCHANGE_MODE);
	msglen += 2;
	extlen += 2;

	// psk mode extension length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(2));
	msglen += 2;
	extlen += 2;

	/*
	* struct {
	* PskKeyExchangeMode ke_modes<1..255>;
	* } PskKeyExchangeModes;
	*/

	//psk mode Length: 1
	PUT_BYTE(prec->snd_msg, msglen, (1));
	msglen++;
	extlen++;

	//psk mode (1)
	PUT_BYTE(prec->snd_msg, msglen, PSK_DHE_KE);
	msglen++;
	extlen++;

	if (psec->chain_ca)
	{
		// Extension type: Certificate Authorities (47)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_CERTIFICATE_AUTHORITIES);
		msglen += 2;
		extlen += 2;

		lstlen = 0;
		// Certificate Authorities extension length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)lstlen);
		msglen += 2;
		extlen += 2;

		/*
		* struct {
		* DistinguishedName authorities<3..2^16-1>;
		* } CertificateAuthoritiesExtension;
		*/

		//DistinguishedName group length
		grplen = 0;
		PUT_SWORD_NET(prec->snd_msg, msglen, grplen);
		msglen += 2;
		extlen += 2;
		lstlen += 2;

		pcrt = psec->chain_ca;
		while (pcrt != NULL && pcrt->next != NULL)
		{
			/*
			opaque DistinguishedName<1..2^16-1>;
			*/
			n = pcrt->subject_raw.len;
			PUT_SWORD_NET(prec->snd_msg, msglen, n);
			msglen += 2;
			extlen += 2;
			lstlen += 2;
			grplen += 2;

			xmem_copy(prec->snd_msg + msglen, pcrt->subject_raw.p, n);

			msglen += n;
			extlen += n;
			lstlen += n;
			grplen += n;

			pcrt = pcrt->next;
		}

		//reset DistinguishedName group length
		PUT_SWORD_NET(prec->snd_msg, (msglen - grplen - 2), grplen);
		//reset Certificate Authorities extension length
		PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);
	}

	// Extension type: key share (51)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_KEY_SHARE);
	msglen += 2;
	extlen += 2;

	lstlen = 0;
	// key share extension length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)lstlen);
	msglen += 2;
	extlen += 2;

	/*
	* struct {
	* KeyShareEntry client_shares<0..2^16-1>;
	* } KeyShareClientHello;
	*/

	// KeyShareEntry group length
	grplen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)grplen);
	msglen += 2;
	extlen += 2;
	lstlen += 2;

	/*
	* struct {
	* NamedGroup group;
	* opaque key_exchange<1..2^16-1>;
	} KeyShareEntry;
	*/

	tls_id = (pcip->tls_ecp_group) ? pcip->tls_ecp_group : TLS_EC_GROUP_X25519;

	//named group tls id
	PUT_SWORD_NET(prec->snd_msg, msglen, tls_id);
	msglen += 2;
	extlen += 2;
	lstlen += 2;
	grplen += 2;

	//opaque key_exchange length
	n = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, n);
	msglen += 2;
	extlen += 2;
	lstlen += 2;
	grplen += 2;

	curve = ecp_curve_info_from_tls_id(tls_id);
	if (curve == NULL)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_client_hello"), _T("ecp_curve_info_from_tls_id"));
	}
	if (!psec->ecdh_ctx)
	{
		psec->ecdh_ctx = (ecdh_context*)xmem_alloc(sizeof(ecdh_context));
		ecdh_init(psec->ecdh_ctx);
	}
	n = 1024;
	if (ecdh_make_params_tls13(psec->ecdh_ctx, curve->grp_id, &n, (prec->snd_msg + msglen), n, pssl->f_rng, pssl->r_rng) != 0)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_client_hello"), _T("ecdh_make_params_tls13"));
	}

	msglen += n;
	extlen += n;
	lstlen += n;
	grplen += n;

	//reset length
	PUT_SWORD_NET(prec->snd_msg, (msglen - n - 2), n);
	PUT_SWORD_NET(prec->snd_msg, (msglen - grplen - 2), grplen);
	PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);

	//extension end
	//reset Extensions length
	PUT_SWORD_NET(prec->snd_msg, (msglen - extlen - 2), (unsigned short)(extlen));

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

static tls13_handshake_states _ssl_parse_server_hello(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	dword_t t;
	int ciph, type;
	int msglen, haslen, seslen, extlen, lstlen;
	int n;
	const ecp_curve_info* curve;
	byte_t message_hash[MD_MAX_SIZE] = { 0 };
	int hlen = 0;

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

	if (prec->rcv_msg[4] != SSL_MAJOR_VERSION_3 || prec->rcv_msg[5] > SSL_MINOR_VERSION_4)
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
	* struct {
    *   ProtocolVersion legacy_version = 0x0303; 
	*	Random random;
	*	opaque legacy_session_id_echo<0..32>;
	*	CipherSuite cipher_suite;
	*	uint8 legacy_compression_method = 0;
	*	Extension extensions<6..2 ^ 16 - 1>;
	* } ServerHello;
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
	if (xmem_comp((void*)(prec->rcv_msg + msglen), (void*)tls13_hello_retry_random, SSL_RND_SIZE) != 0)
	{
		t = GET_DWORD_NET(prec->rcv_msg, msglen);
		xmem_copy(pcip->rnd_srv, prec->rcv_msg + msglen, SSL_RND_SIZE);

		pcip->hello_retry = 0;
	}
	else
	{
		ecdh_free(psec->ecdh_ctx);
		ecdh_init(psec->ecdh_ctx);

		pcip->hello_retry = 1;
	}
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

	if (pses->session_resumed == 0 || pcip->cipher.cip_id != ciph || a_xslen(pses->session_id) != seslen || xmem_comp(pses->session_id, prec->rcv_msg + msglen, seslen) != 0)
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
		return SSL_SERVER_EXTENSIONS;
	}

	if (pcip->hello_retry)
	{
		_ssl_gen_message_hash(pcip, message_hash, &hlen);
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

		if (!lstlen)
		{
			continue;
		}

		switch (type)
		{
		case SSL_EXTENSION_SUPPORTED_VERSION:
			/*
			* struct {
			*	select (Handshake.msg_type) {
			*	case client_hello:
			*	ProtocolVersion versions<2..254>;
			*	case server_hello:
			*	ProtocolVersion selected_version;
			*	};
			* } SupportedVersions;
			*/
			pcip->ext_major_ver = prec->rcv_msg[msglen];
			pcip->ext_minor_ver = prec->rcv_msg[msglen + 1];

			msglen += 2;
			extlen -= 2;
			lstlen -= 2;
			break;
		case SSL_EXTENSION_COOKIE:
			/*
			struct {
			opaque cookie<1..2^16-1>;
			} Cookie;
			*/
			n = GET_SWORD_NET(prec->rcv_msg, msglen);
			msglen += 2;
			extlen -= 2;
			lstlen -= 2;

			pses->cookie_size = n;
			xmem_copy((void*)(pses->session_cookie), (void*)(prec->rcv_msg + msglen), n);
			
			if (pcip->hello_retry && xmem_comp((void*)pses->session_cookie, (void*)message_hash, hlen) != 0)
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
				raise_user_error(_T("_ssl_parse_server_hello"), _T("invalid session cookie"));
			}

			msglen += n;
			extlen -= n;
			lstlen -= n;
			break;
		case SSL_EXTENSION_KEY_SHARE:
			if (pcip->hello_retry)
			{
				/*
				* struct {
				* NamedGroup selected_group;
				* } KeyShareHelloRetryRequest;
				*/
				pcip->tls_ecp_group = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;
			}
			else
			{
				/*
				* struct {
				*  KeyShareEntry server_share;
				* } KeyShareServerHello;
				*/

				/*
				* struct {
				* NamedGroup group;
				* opaque key_exchange<1..2^16-1>;
				} KeyShareEntry;
				*/
				pcip->tls_ecp_group = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;

				n = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;

				curve = ecp_curve_info_from_tls_id(pcip->tls_ecp_group);
				if (!curve)
				{
					_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
					raise_user_error(_T("_ssl_parse_server_hello"), _T("ecp_curve_info_from_tls_id"));
				}
				if (ecdh_read_public_tls13(psec->ecdh_ctx, (prec->rcv_msg + msglen), n, pssl->f_rng, pssl->r_rng) != 0)
				{
					_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
					raise_user_error(_T("_ssl_parse_server_hello"), _T("ecdh_read_public_tls13"));
				}
				if (ecdh_calc_secret(psec->ecdh_ctx, &pcip->dhe_length, pcip->dhe, 256, pssl->f_rng, pssl->r_rng) != 0)
				{
					_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
					raise_user_error(_T("_ssl_parse_server_hello"), _T("ecdh_calc_secret"));
				}

				msglen += n;
				extlen -= n;
				lstlen -= n;
			}
			break;
		default:
			//skip 
			break;
		}

		msglen += lstlen;
		extlen -= lstlen;

		if (extlen < 0)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
			raise_user_error(_T("_ssl_parse_server_hello"), _T("invalid message length"));
		}
	}

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

	return (pcip->hello_retry)? SSL_CLIENT_HELLO : SSL_SERVER_EXTENSIONS;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_parse_server_encrypted_extensions(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int type;
	int msglen, haslen, extlen, lstlen;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_ENCRYPTED_EXTENSIONS)
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
	EncryptedExtensions;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_ENCRYPTED_EXTENSIONS)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_server_encrypted_extensions"), _T("invalid message type"));
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	msglen = SSL_HSH_SIZE;

	/*
	* struct {
	* Extension extensions<0..2^16-1>;
	* } EncryptedExtensions;
	*/

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

		msglen += lstlen;
		extlen -= lstlen;

		if (extlen < 0)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
			raise_user_error(_T("_ssl_parse_server_encrypted_extensions"), _T("invalid message length"));
		}
	}

	if (haslen + SSL_HSH_SIZE != msglen)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
		raise_user_error(_T("_ssl_parse_server_encrypted_extensions"), _T("invalid message length"));
	}

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
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int haslen, msglen, extlen, lstlen;
	int n, type;
	int sig_alg;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE_REQUEST && prec->rcv_msg[0] != SSL_HS_CERTIFICATE && prec->rcv_msg[0] != SSL_HS_FINISHED)
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
		return SSL_SERVER_CERTIFICATE;
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	* struct {
    * opaque certificate_request_context<0..2^8-1>;
    * Extension extensions<2..2^16-1>;
    * } CertificateRequest;
	*/

	msglen = SSL_HSH_SIZE;

	//certificate_request_context length
	n = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	//certificate_request_context
	msglen += n;

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

		if (!lstlen)
		{
			continue;
		}

		switch (type)
		{
		case SSL_EXTENSION_SIGNATUREANDHASHALGORITHM:
			sig_alg = GET_SWORD_NET(prec->rcv_msg, msglen);

			msglen += 2;
			extlen -= 2;
			lstlen -= 2;
			break;
		default:
			//skip 
			break;
		}

		msglen += lstlen;
		extlen -= lstlen;

		if (extlen < 0)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
			raise_user_error(_T("_ssl_parse_server_certificate_request"), _T("invalid message length"));
		}
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

static tls13_handshake_states _ssl_parse_server_certificate(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int ret, n;
	int msglen, haslen, crtlen, extlen, lstlen;
	int type, cert;

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
	* struct {
    * opaque certificate_request_context<0..2^8-1>;
    * CertificateEntry certificate_list<0..2^24-1>;
    * } Certificate;
	*/
	msglen = SSL_HSH_SIZE;

	/*
	* certificate_request_context
	*/
	//certificate_request_context length
	n = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;
	//certificate_request_context
	msglen += n;

	/*
	* certificate_list
	*/
	crtlen = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	psec->peer_crt = (x509_crt*)xmem_alloc(sizeof(x509_crt));

	while (crtlen)
	{
		/*
		* struct {
		* select (certificate_type) {
		* case RawPublicKey:  --From RFC 7250 ASN.1_subjectPublicKeyInfo 
		*	opaque ASN1_subjectPublicKeyInfo<1..2 ^ 24 - 1>;
        * case X509:
		*	opaque cert_data<1..2 ^ 24 - 1>;
		* };
		* Extension extensions<0..2 ^ 16 - 1>;
		* } CertificateEntry;
		*/

		/*
		* cert_data
		*/
		//per cert length
		n = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
		msglen += 3;
		crtlen -= 3;

		if (n < 128 || n > crtlen)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_server_certificate"), _T("invalid message length"));
		}

		if (C_OK != x509_crt_parse(psec->peer_crt, prec->rcv_msg + msglen, n))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_server_certificate"), _T("x509_crt_parse"));
		}

		msglen += n;
		crtlen -= n;

		/*
		* extensions
		*/
		//extension length
		extlen = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;
		crtlen -= 2;

		crtlen -= extlen;

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

			if (!lstlen)
			{
				continue;
			}

			switch (type)
			{
			case SSL_EXTENSION_SERVER_CERTIFICATE_TYPE:
				cert = GET_BYTE(prec->rcv_msg, msglen);

				msglen += 1;
				extlen -= 1;
				lstlen -= 1;
				break;
			default:
				//skip 
				break;
			}

			msglen += lstlen;
			extlen -= lstlen;

			if (extlen < 0)
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
				raise_user_error(_T("_ssl_parse_server_certificate"), _T("invalid message length"));
			}
		}
	}

	if (psec->verify_mode != SSL_VERIFY_NONE)
	{
		if (psec->chain_ca == NULL)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_NO_CERTIFICATE);
			raise_user_error(_T("_ssl_parse_server_certificate"), _T("invalid ca"));
		}

		if (psec->verify_mode == SSL_VERIFY_REQUIRED)
		{
			if (C_OK != x509_crt_verify(psec->peer_crt, psec->chain_ca, NULL, psec->peer_cn, &ret, NULL, NULL))
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
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

	return SSL_SERVER_CERTIFICATE_VERIFY;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_parse_server_certificate_verify(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int msglen, haslen, n;
	byte_t hash[MD_MAX_SIZE];
	int hlen;
	int sig_alg, sig_md, sig_pk;
	const md_info_t* md_info;
	void* md_ctx;
	
	byte_t blank_context_prefic[64] = { 0x20 };
	char server_context_string[] = "TLS 1.3, server CertificateVerify";
	byte_t zero[1] = { 0 };

	TRY_CATCH;

	//certificate verify hash = Transcript-Hash(Handshake Context, Certificate))
	_ssl_gen_handshake_hash(pcip, hash, &hlen);

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE_VERIFY)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_server_certificate_verify"), _T("_ssl_read_rcv_msg"));
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
		raise_user_error(_T("_ssl_parse_server_certificate_verify"), _T("invalid message type"));
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	* struct {
    * SignatureScheme algorithm;
    * opaque signature<0..2^16-1>;
    * } CertificateVerify;
	*/
	msglen = SSL_HSH_SIZE;

	/*
	* algorithm
	*/
	sig_alg = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (!_ssl_select_signature(sig_alg, &sig_pk, &sig_md))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_server_certificate_verify"), _T("invalid algorithm"));
	}

	md_info = md_info_from_type(sig_md);
	if (!md_info)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_server_certificate_verify"), _T("md_info_from_type"));
	}
	md_ctx = md_alloc(md_info);
	if (!md_ctx)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_server_certificate_verify"), _T("invalid md context"));
	}

	md_starts(md_info, md_ctx);
	md_update(md_info, md_ctx, blank_context_prefic, 64);
	md_update(md_info, md_ctx, (byte_t*)server_context_string, a_xslen(server_context_string));
	md_update(md_info, md_ctx, zero, 1);
	md_update(md_info, md_ctx, hash, hlen);
	xmem_zero((void*)hash, md_info->size);
	md_finish(md_info, md_ctx, hash);
	md_free(md_info, md_ctx);

	if (psec->peer_crt == NULL)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_NO_CERTIFICATE);
		raise_user_error(_T("_ssl_parse_server_certificate_verify"), _T("invalid peer certificate"));
	}

	/*
	* signature
	*/
	n = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (n != psec->peer_crt->rsa->len)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_server_certificate_verify"), _T("invalid certificate length"));
	}

	if (C_OK != rsa_pkcs1_verify(psec->peer_crt->rsa, pssl->f_rng, pssl->r_rng, RSA_PUBLIC, md_info->type, md_info->size, hash, prec->rcv_msg + msglen))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_server_certificate_verify"), _T("rsa_pkcs1_verify"));
	}

	msglen += n;

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	END_CATCH;

	return SSL_SERVER_FINISHED;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_parse_server_finished(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int mac_len;
	byte_t mac_buf[MD_MAX_SIZE];

	byte_t hash[MD_MAX_SIZE] = { 0 };
	int hlen;
	const md_info_t* md_info;

	byte_t okm[MD_MAX_SIZE] = { 0 };
	int okm_len = 0;

	TRY_CATCH;

	//finished hash = Transcript-Hash(Handshake Context, Certificate*, CertificateVerify*))
	_ssl_gen_handshake_hash(pcip, hash, &hlen);

	if (prec->rcv_msg[0] != SSL_HS_FINISHED)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_server_finished"), _T("_ssl_read_rcv_msg"));
		}
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

	//finished key
	ssl_expand(pcip->cipher.md_type, pcip->handshake_secret, pcip->handshake_secret_length, "finished", "", 0, okm, okm_len);

	md_info = md_info_from_type(pcip->cipher.md_type);
	if (!md_info)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_server_finished"), _T("invalid cipher type"));
	}
	//finished mac
	md_hmac(md_info, okm, okm_len, hash, hlen, mac_buf);
	mac_len = md_info->size;

	/*
	struct {
	opaque verify_data[Hash.length];
	} Finished;
	*/

	if (xmem_comp(prec->rcv_msg + SSL_HSH_SIZE, mac_buf, mac_len) != 0)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_BAD_RECORD_MAC);
		raise_user_error(_T("_ssl_parse_server_finished"), _T("invalid message mac"));
	}

	END_CATCH;

	return (pses->authen_client) ? SSL_CLIENT_CERTIFICATE : SSL_CLIENT_FINISHED;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_write_client_certificate(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen, crtlen, extlen;
	x509_crt *crt;

	TRY_CATCH;

	/*
	* struct {
	* opaque certificate_request_context<0..2 ^ 8 - 1>;
	* CertificateEntry certificate_list<0..2 ^ 24 - 1>;
	* } Certificate;
	*/

	msglen = SSL_HSH_SIZE;

	/*
	* certificate_request_context (empty)
	*/
	PUT_BYTE(prec->rcv_msg, msglen, 0);
	msglen++;

	/*
	* certificate_list
	*/
	//preset certs length to zero
	crtlen = 0;
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, crtlen);
	msglen += 3;

	crt = psec->host_crt;
	while (crt != NULL && crt->version != 0)
	{
		/*
		* struct {
		* select (certificate_type) {
		* case RawPublicKey: --From RFC 7250 ASN.1_subjectPublicKeyInfo
		* opaque ASN1_subjectPublicKeyInfo<1..2 ^ 24 - 1>;
		* case X509:
		* opaque cert_data<1..2 ^ 24 - 1>;
		* };
		* Extension extensions<0..2 ^ 16 - 1>;
		* } CertificateEntry;
		*/
		n = crt->raw.len;
		if (msglen + 3 + n > SSL_PKG_SIZE)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_RECORD_OVERFLOW);
			raise_user_error(_T("_ssl_write_client_certificate"), _T("invalid message length"));
		}

		//peer cert length
		PUT_THREEBYTE_NET(prec->snd_msg, msglen, n);
		msglen += 3;
		crtlen += 3;

		//peer cert data
		xmem_copy(prec->snd_msg + msglen, crt->raw.p, n);
		msglen += n;
		crtlen += n;

		//zero extensions
		extlen = 0;
		PUT_DWORD_NET(prec->snd_msg, msglen, extlen);
		msglen += 2;
		crtlen += 2;

		msglen += extlen;
		crtlen += extlen;

		crt = crt->next;
	}

	//reset certificate_list length
	PUT_THREEBYTE_NET(prec->snd_msg, (msglen - crtlen - 3), crtlen);
	
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

	return SSL_CLIENT_CERTIFICATE_VERIFY;
	
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static int _ssl_write_client_certificate_verify(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen;
	byte_t hash[MD_MAX_SIZE];
	int hlen;
	int sig_alg, sig_md, sig_pk;
	const md_info_t* md_info;
	void* md_ctx;

	byte_t blank_context_prefic[64] = { 0x20 };
	char client_context_string[] = "TLS 1.3, client CertificateVerify";
	byte_t zero[1] = { 0 };

	TRY_CATCH;

	//certificate verify hash = Transcript-Hash(Handshake Context, Certificate))
	_ssl_gen_handshake_hash(pcip, hash, &hlen);

	/*
	struct {
	SignatureScheme algorithm;
	opaque signature<0..2^16-1>;
	} CertificateVerify;
	*/
	msglen = SSL_HSH_SIZE;

	sig_alg = TLS_ALG_RSA_PKCS1_SHA256;

	//algorithm
	PUT_SWORD_NET(prec->snd_msg, msglen, sig_alg);
	msglen += 2;

	if (!_ssl_select_signature(sig_alg, &sig_pk, &sig_md))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_client_certificate_verify"), _T("invalid message algorithm"));
	}

	md_info = md_info_from_type(sig_md);
	if (!md_info)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_client_certificate_verify"), _T("invalid message algorithm"));
	}
	md_ctx = md_alloc(md_info);
	if (!md_ctx)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_client_certificate_verify"), _T("invalid signature context"));
	}

	md_starts(md_info, md_ctx);
	md_update(md_info, md_ctx, blank_context_prefic, 64);
	md_update(md_info, md_ctx, (byte_t*)client_context_string, a_xslen(client_context_string));
	md_update(md_info, md_ctx, zero, 1);
	md_update(md_info, md_ctx, hash, hlen);
	xmem_zero((void*)hash, md_info->size);
	md_finish(md_info, md_ctx, hash);
	md_free(md_info, md_ctx);

	if (psec->rsa_ctx == NULL)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_client_certificate_verify"), _T("invalid signature context"));
	}

	//signature length
	n = psec->rsa_ctx->len;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)n);
	msglen += 2;

	//signature
	if (C_OK != rsa_pkcs1_sign(psec->rsa_ctx, pssl->f_rng, pssl->r_rng, RSA_PRIVATE, md_info->type, md_info->size, hash, prec->snd_msg + msglen))
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

	return SSL_CLIENT_FINISHED;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_write_client_finished(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int msglen;
	byte_t* mac_buf;

	byte_t hash[MD_MAX_SIZE] = { 0 };
	int hlen;
	const md_info_t* md_info;

	byte_t okm[MD_MAX_SIZE] = { 0 };
	int okm_len = 0;

	TRY_CATCH;

	//finished hash = Transcript-Hash(Handshake Context, Certificate*, CertificateVerify*))
	_ssl_gen_handshake_hash(pcip, hash, &hlen);

	//finished key
	ssl_expand(pcip->cipher.md_type, pcip->handshake_secret, pcip->handshake_secret_length, "finished", "", 0, okm, okm_len);

	//finished mac
	md_info = md_info_from_type(pcip->cipher.md_type);
	if (!md_info)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_client_finished"), _T("invalid message algorithm"));
	}

	/*
	struct {
          opaque verify_data[Hash.length];
    *　} Finished;
	*/

	msglen = SSL_HSH_SIZE;
	mac_buf = prec->snd_msg + msglen;

	md_hmac(md_info, okm, okm_len, hash, hlen, mac_buf);
	msglen += md_info->size;

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
		raise_user_error(_T("_ssl_write_client_certificate_verify"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_HANDSHAKE_OVER;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

bool_t tls13_handshake_client(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip;

	tls13_handshake_states state = SSL_CLIENT_HELLO;

	pcip = _ssl_alloc_cipher(pssl);

	while (state != SSL_HANDSHAKE_OVER)
	{
		switch (state)
		{
			/*
			*  ==>   ClientHello
			*/
		case SSL_CLIENT_HELLO:
			state = _ssl_write_client_hello(pssl);
			break;

			/*
			*  <==   ServerHello (ServerHelloRetryRequest)
			*      ( ChangeCipherSpec )
			*      EncryptedExtensions
			*      ( CertificateRequest )
			*      Certificate
			*      CertificateVerify
			*      Finished
			*/
		case SSL_SERVER_HELLO:
			state = _ssl_parse_server_hello(pssl);

			if (!pcip->hello_retry)
			{
				_ssl_extract_early_secret(pcip);
			}
			break;
		case SSL_SERVER_EXTENSIONS:
			_ssl_extract_handshake_secret(pcip);
			_ssl_derive_handshake_key(pcip);
			_ssl_reset_sequence_number(pses);

			state = _ssl_parse_server_encrypted_extensions(pssl);
			break;
		case SSL_CERTIFICATE_REQUEST:
			state = _ssl_parse_server_certificate_request(pssl);
			break;
		case SSL_SERVER_CERTIFICATE:
			state = _ssl_parse_server_certificate(pssl);
			break;
		case SSL_SERVER_CERTIFICATE_VERIFY:
			state = _ssl_parse_server_certificate_verify(pssl);
			break;
		case SSL_SERVER_FINISHED:
			state = _ssl_parse_server_finished(pssl);
			break;

			/*
			*  ==> ( Certificate/Alert  )
			*      ( CertificateVerify  )
			*        ChangeCipherSpec
			*        Finished
			*/
		case SSL_CLIENT_CERTIFICATE:
			state = _ssl_write_client_certificate(pssl);
			break;

		case SSL_CLIENT_CERTIFICATE_VERIFY:
			state = _ssl_write_client_certificate_verify(pssl);
			break;
		case SSL_CLIENT_FINISHED:
			state = _ssl_write_client_finished(pssl);

			_ssl_extract_master_secret(pcip);
			_ssl_derive_application_key(pcip);
			_ssl_reset_sequence_number(pses);
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

static tls13_handshake_states _ssl_parse_client_hello(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int ret, msglen, haslen, seslen, ciphlen, complen, comped, extlen, lstlen, grplen;
	int i, j, n;
	int ciph, type, group, sig_alg, tls_id;
	byte_t* ciph_buf;
	const ecp_curve_info* curve;

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
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message type"));
	}

	if (pssl->cli_minor_ver < SSL_MINOR_VERSION_1)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message type"));
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
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message length"));
	}

	pses->session_size = seslen;
	xmem_copy(pses->session_id, prec->rcv_msg + msglen, pses->session_size);
	msglen += seslen;

	/*
	CipherSuite
	*/
	ciphlen = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (ciphlen < 2 || ciphlen > 256 || (ciphlen % 2) != 0)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message length"));
	}

	ciph_buf = prec->rcv_msg + msglen;
	ciph = 0;
	n = sizeof(server_ciphers) / sizeof(tls13_ciphers_set);
	for (i = 0; i < n; i++)
	{
		for (j = 0; j < ciphlen; j += 2)
		{
			ciph = GET_SWORD_NET(ciph_buf, j);
			if (ciph == server_ciphers[i].cip_id)
				break;
		}

		if (j < ciphlen)
			break;
	}
	msglen += ciphlen;

	if (!_ssl_choose_cipher(pcip, ciph))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message cipher"));
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
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid message compress"));
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

		if (!lstlen)
		{
			continue;
		}

		switch (type)
		{
		case SSL_EXTENSION_SERVERNAME:
			//skip server name
			break;
		case SSL_EXTENSION_SUPPORTEDGROUPS:
			pcip->tls_ecp_group = 0;
			/*
			* struct {
			* NamedGroup named_group_list<2..2^16-1>;
			* } NamedGroupList;
			*/
			grplen = GET_SWORD_NET(prec->rcv_msg, msglen);
			msglen += 2;
			extlen -= 2;
			lstlen -= 2;

			while (grplen > 0)
			{
				group = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;
				grplen -= 2;

				if (!pcip->tls_ecp_group)
				{
					n = sizeof(named_group) / sizeof(sword_t);
					for (i = 0; i < n; i++)
					{
						if (group == named_group[i])
						{
							pcip->tls_ecp_group = group;
							break;
						}
					}
				}
			}
			break;
		case SSL_EXTENSION_ECPOINTFORMATS:
			//Elliptic curve list length
			grplen = GET_BYTE(prec->rcv_msg, msglen);
			msglen++;
			extlen--;
			lstlen--;

			pcip->tls_ecp_format = GET_BYTE(prec->rcv_msg, msglen);

			msglen += grplen;
			extlen -= grplen;
			lstlen -= grplen;
			break;

		case SSL_EXTENSION_SIGNATUREANDHASHALGORITHM:
			/*
			* struct {
			* SignatureScheme supported_signature_algorithms<2..2^16-2>;
			* } SignatureSchemeList;
			*/
			grplen = GET_SWORD_NET(prec->rcv_msg, msglen);
			msglen += 2;
			extlen -= 2;
			lstlen -= 2;

			while (grplen > 0)
			{
				sig_alg = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;
				grplen -= 2;

				if (!pcip->signature.sig_alg)
				{
					n = sizeof(signature_set) / sizeof(sword_t);
					for (i = 0; i < n; i++)
					{
						if (sig_alg == signature_set[i].sig_alg)
						{
							if (!pcip->signature.sig_alg)
							{
								pcip->signature.sig_alg = signature_set[i].sig_alg;
								pcip->signature.pk_alg = signature_set[i].pk_alg;
								pcip->signature.md_alg = signature_set[i].md_alg;
								pcip->signature.tls_grp = signature_set[i].tls_grp;
							}
							break;
						}
					}
				}
			}
			break;
		case SSL_EXTENSION_PADDING:
			//zero length
			break;
		case SSL_EXTENSION_EXTENDEDMASTERSECRET:
			//zero length
			break;
		case SSL_EXTENSION_SESSIONTICKET:
			//zero length
			break;
		case SSL_EXTENSION_SUPPORTED_VERSION:
			n = GET_BYTE(prec->rcv_msg, msglen);
			msglen += 1;
			extlen -= 1;
			lstlen -= 1;

			pcip->ext_major_ver = prec->rcv_msg[msglen];
			pcip->ext_minor_ver = prec->rcv_msg[msglen + 1];
			msglen += n;
			extlen -= n;
			lstlen -= n;
			break;
		case SSL_EXTENSION_COOKIE:
			/*
			* struct {
			* opaque cookie<1..2^16-1>;
			} Cookie;
			*/
			n = GET_SWORD_NET(prec->rcv_msg, msglen);
			msglen += 2;
			extlen -= 2;
			lstlen -= 2;

			if (xmem_comp((void*)(pses->session_cookie), (void*)(prec->rcv_msg + msglen), n) != 0)
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
				raise_user_error(_T("_ssl_parse_client_hello"), _T("invalid session cookie"));
			}

			//
			msglen += n;
			extlen -= n;
			lstlen -= n;
			break;
		case SSL_EXTENSION_PSK_KEY_EXCHANGE_MODE:
			/*
			* struct {
			* PskKeyExchangeMode ke_modes<1..255>;
			* } PskKeyExchangeModes;
			*/
			grplen = GET_BYTE(prec->rcv_msg, msglen);
			msglen += 1;
			extlen -= 1;
			lstlen -= 1;

			pcip->psk_mode = GET_BYTE(prec->rcv_msg, msglen);
			msglen += 1;
			extlen -= 1;
			lstlen -= 1;
			break;
		case SSL_EXTENSION_CERTIFICATE_AUTHORITIES:
			/*
			* struct {
			* DistinguishedName authorities<3..2^16-1>;
			* } CertificateAuthoritiesExtension;
			*/
			grplen = GET_SWORD_NET(prec->rcv_msg, msglen);
			msglen += 2;
			extlen -= 2;
			lstlen -= 2;

			while (grplen > 0)
			{
				//opaque DistinguishedName<1..2^16-1>;
				n = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;
				grplen -= 2;

				//...
				msglen += n;
				extlen -= n;
				lstlen -= n;
				grplen -= n;
			}
			break;
		case SSL_EXTENSION_POST_HANDSHAKE_AUTH:
			//zero length
			break;
		case SSL_EXTENSION_KEY_SHARE:
			/*
			* struct {
			* KeyShareEntry client_shares<0..2^16-1>;
			* } KeyShareClientHello;
			*/
			grplen = GET_SWORD_NET(prec->rcv_msg, msglen);
			msglen += 2;
			extlen -= 2;
			lstlen -= 2;

			while (grplen > 0)
			{
				/*
				* struct {
				* NamedGroup group;
				* opaque key_exchange<1..2^16-1>;
				} KeyShareEntry;
				*/
				tls_id = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;
				grplen -= 2;

				n = sizeof(named_group) / sizeof(sword_t);
				for (i = 0; i < n; i++)
				{
					if (tls_id == named_group[i])
					{
						pcip->tls_ecp_group = tls_id;
						break;
					}
				}

				n = GET_SWORD_NET(prec->rcv_msg, msglen);
				msglen += 2;
				extlen -= 2;
				lstlen -= 2;
				grplen -= 2;

				if (pcip->tls_ecp_group == tls_id)
				{
					curve = ecp_curve_info_from_tls_id(tls_id);
					if (curve == NULL)
					{
						_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
						raise_user_error(_T("_ssl_parse_client_hello"), _T("ecp_curve_info_from_tls_id"));
					}
					if (!psec->ecdh_ctx)
					{
						psec->ecdh_ctx = (ecdh_context*)xmem_alloc(sizeof(ecdh_context));
						ecdh_init(psec->ecdh_ctx);
					}
					ret = ecdh_read_params_tls13(psec->ecdh_ctx, curve->grp_id, (prec->rcv_msg + msglen), n, pssl->f_rng, pssl->r_rng);
					if (ret != 0)
					{
						_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
						raise_user_error(_T("_ssl_parse_client_hello"), _T("ecdh_read_params_tls13"));
					}

					pcip->hello_retry = 0;
				}
				else
				{
					pcip->hello_retry = 1;
				}

				msglen += n;
				extlen -= n;
				lstlen -= n;
				grplen -= n;
			}
			break;
		default:
			//skip 
			break;
		}

		msglen += lstlen;
		extlen -= lstlen;

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

static tls13_handshake_states _ssl_write_server_hello_retry_request(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int msglen, extlen, lstlen;
	byte_t message_hash[MD_MAX_SIZE] = { 0 };
	int hlen;

	TRY_CATCH;

	/*
	* struct {
	*   ProtocolVersion legacy_version = 0x0303;
	*	Random random;
	*	opaque legacy_session_id_echo<0..32>;
	*	CipherSuite cipher_suite;
	*	uint8 legacy_compression_method = 0;
	*	Extension extensions<6..2 ^ 16 - 1>;
	* } ServerHello;
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
	xmem_copy((void*)(prec->snd_msg + msglen), (void*)tls13_hello_retry_random, SSL_RND_SIZE);
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
	PUT_SWORD_NET(prec->snd_msg, msglen, pcip->cipher.cip_id);
	msglen += 2;

	/*
	CompressionMethod
	*/
	PUT_BYTE(prec->snd_msg, msglen, 0);
	msglen++;

	// preset Extensions length to zero
	extlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(extlen));
	msglen += 2;

	// Extension type: supported_version (43)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SUPPORTED_VERSION);
	msglen += 2;
	extlen += 2;

	lstlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, lstlen);
	msglen += 2;
	extlen += 2;

	/*
	* struct {
	*	select (Handshake.msg_type) {
	*	case client_hello:
	*	ProtocolVersion versions<2..254>;
	*	case server_hello:
	*	ProtocolVersion selected_version;
	*	};
	* } SupportedVersions;
	*/

	PUT_BYTE(prec->snd_msg, msglen, SSL_MAJOR_VERSION_3);
	msglen += 1;
	extlen += 1;
	lstlen += 1;

	PUT_BYTE(prec->snd_msg, msglen, SSL_MINOR_VERSION_4);
	msglen += 1;
	extlen += 1;
	lstlen += 1;

	//reset supported_version length
	PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);

	// Extension type: cookie (44)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_COOKIE);
	msglen += 2;
	extlen += 2;

	lstlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, lstlen);
	msglen += 2;
	extlen += 2;

	//make sever cookie
	_ssl_gen_message_hash(pcip, message_hash, &hlen);
	pses->cookie_size = hlen;
	xmem_copy((void*)pses->session_cookie, (void*)(message_hash), pses->cookie_size);
	/*
	struct {
		opaque cookie<1..2^16-1>;
	} Cookie;
	*/
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_COOKIE_SIZE);
	msglen += 2;
	extlen += 2;
	lstlen += 2;

	xmem_copy((void*)(prec->snd_msg + msglen), (void*)pses->session_cookie, SSL_COOKIE_SIZE);
	msglen += SSL_COOKIE_SIZE;
	extlen += SSL_COOKIE_SIZE;
	lstlen += SSL_COOKIE_SIZE;

	//reset cookie length
	PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);

	if (pcip->psk_mode)
	{
		// Extension type: key_share (43)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_KEY_SHARE);
		msglen += 2;
		extlen += 2;

		lstlen = 0;
		PUT_SWORD_NET(prec->snd_msg, msglen, lstlen);
		msglen += 2;
		extlen += 2;

		/*
		struct {
          NamedGroup selected_group;
		} KeyShareHelloRetryRequest;
		*/

		PUT_SWORD_NET(prec->snd_msg, msglen, pcip->tls_ecp_group);
		msglen += 2;
		extlen += 2;
		lstlen += 2;

		//reset length
		PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);
	}

	//reset Extensions length
	PUT_SWORD_NET(prec->snd_msg, (msglen - extlen - 2), (unsigned short)(extlen));

	/*
	* struct {
	* HandshakeType msg_type; 1 byte
	* uint24 length; 3 bytes
	* ServerHello;
	* } Handshake;
	*/
	//handshake type
	PUT_BYTE(prec->snd_msg, 0, (byte_t)SSL_HS_SERVER_HELLO);
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_hello_retry_request"), _T("_ssl_write_snd_msg"));
	}

	pcip->hello_retry = 0;

	END_CATCH;

	return SSL_CLIENT_HELLO;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_write_server_hello(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen, extlen, lstlen;
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

	/*
	* struct {
    *   ProtocolVersion legacy_version = 0x0303; 
	*	Random random;
	*	opaque legacy_session_id_echo<0..32>;
	*	CipherSuite cipher_suite;
	*	uint8 legacy_compression_method = 0;
	*	Extension extensions<6..2 ^ 16 - 1>;
	* } ServerHello;
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
	PUT_SWORD_NET(prec->snd_msg, msglen, pcip->cipher.cip_id);
	msglen += 2;

	/*
	CompressionMethod
	*/
	PUT_BYTE(prec->snd_msg, msglen, 0);
	msglen++;

	// preset Extensions length to zero
	extlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(extlen));
	msglen += 2;

	// Extension type: supported_version (43)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SUPPORTED_VERSION);
	msglen += 2;
	extlen += 2;

	lstlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, lstlen);
	msglen += 2;
	extlen += 2;
	/*
	* struct {
	*	select (Handshake.msg_type) {
	*	case client_hello:
	*	ProtocolVersion versions<2..254>;
	*	case server_hello:
	*	ProtocolVersion selected_version;
	*	};
	* } SupportedVersions;
	*/
	PUT_BYTE(prec->snd_msg, msglen, SSL_MAJOR_VERSION_3);
	msglen += 1;
	extlen += 1;
	lstlen += 1;

	PUT_BYTE(prec->snd_msg, msglen, SSL_MINOR_VERSION_4);
	msglen += 1;
	extlen += 1;
	lstlen += 1;

	//reset SupportedVersions length
	PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);

	if (pcip->psk_mode)
	{
		// Extension type: key_share (43)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_KEY_SHARE);
		msglen += 2;
		extlen += 2;

		lstlen = 0;
		PUT_SWORD_NET(prec->snd_msg, msglen, lstlen);
		msglen += 2;
		extlen += 2;

		/*
		* struct {
        *  KeyShareEntry server_share;
		* } KeyShareServerHello;

		* struct {
		* NamedGroup group;
		* opaque key_exchange<1..2^16-1>;
		* } KeyShareEntry;
		*/

		PUT_SWORD_NET(prec->snd_msg, msglen, pcip->tls_ecp_group);
		msglen += 2;
		extlen += 2;
		lstlen += 2;
		
		n = 0;
		PUT_SWORD_NET(prec->snd_msg, msglen, n);
		msglen += 2;
		extlen += 2;
		lstlen += 2;

		n = 1024;
		if(ecdh_make_public_tls13(psec->ecdh_ctx, &n, (prec->snd_msg + msglen), n, pssl->f_rng, pssl->r_rng) != 0)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_server_hello"), _T("ecdh_make_public_tls13"));
		}
		if(ecdh_calc_secret(psec->ecdh_ctx, &pcip->dhe_length, pcip->dhe, 256, pssl->f_rng, pssl->r_rng) != 0)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_server_hello"), _T("ecdh_calc_secret"));
		}

		msglen += n;
		extlen += n;
		lstlen += n;

		//reset length
		PUT_SWORD_NET(prec->snd_msg, (msglen - n - 2), n);
		PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);
	}

	//reset Extensions length
	PUT_SWORD_NET(prec->snd_msg, (msglen - extlen - 2), (unsigned short)(extlen));

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

	return SSL_SERVER_EXTENSIONS;
	
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_write_server_encrypted_extensions(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen, extlen, lstlen, grplen;

	TRY_CATCH;

	/*
	* struct {
    * Extension extensions<0..2^16-1>;
    * } EncryptedExtensions;
	*/

	msglen = SSL_HSH_SIZE;

	// preset Extensions length to zero
	extlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(extlen));
	msglen += 2;

	if (!a_is_null(psec->host_cn))
	{
		// Extension type: Server name(0)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SERVERNAME);
		msglen += 2;
		extlen += 2;

		lstlen = 0;
		// Server name extension length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(lstlen));
		msglen += 2;
		extlen += 2;

		grplen = 0;
		// Server name list count length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(grplen));
		msglen += 2;
		extlen += 2;
		lstlen += 2;

		// Host name type
		PUT_BYTE(prec->snd_msg, msglen, 0);
		msglen++;
		extlen++;
		lstlen++;
		grplen++;

		n = a_xslen(psec->host_cn);
		// Host name length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(n));
		msglen += 2;
		extlen += 2;
		lstlen += 2;
		grplen += 2;

		// Host name
		xmem_copy(prec->snd_msg + msglen, psec->host_cn, n);
		msglen += n;
		extlen += n;
		lstlen += n;
		grplen += n;

		//reset length
		PUT_SWORD_NET(prec->snd_msg, (msglen - grplen - 2), (unsigned short)(grplen));
		PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), (unsigned short)(lstlen));
	}

	//reset Extensions length
	PUT_SWORD_NET(prec->snd_msg, (msglen - extlen - 2), (unsigned short)(extlen));

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	ServerHello;
	} Handshake;
	*/
	//handshake type
	PUT_BYTE(prec->snd_msg, 0, (byte_t)SSL_HS_ENCRYPTED_EXTENSIONS);
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, msglen - SSL_HSH_SIZE);

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _ssl_write_snd_msg(pssl))
	{
		raise_user_error(_T("_ssl_write_server_encrypted_extensions"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return (psec->verify_mode == SSL_VERIFY_NONE) ? SSL_SERVER_CERTIFICATE : SSL_CERTIFICATE_REQUEST;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_write_server_certificate_request(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen, extlen, lstlen, grplen;
	x509_crt* pcrt;

	TRY_CATCH;
	/*
	struct {
          opaque certificate_request_context<0..2^8-1>;
          Extension extensions<2..2^16-1>;
      } CertificateRequest;
	*/

	msglen = SSL_HSH_SIZE;

	/*
	* certificate_request_context length (empty)
	*/
	PUT_BYTE(prec->snd_msg, msglen, 0);
	msglen++;

	/*
	* Extension extensions
	*/
	// preset Extensions length to zero
	extlen = 0;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(extlen));
	msglen += 2;

	// Extension type: signature_algorithms(13)
	PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_SIGNATUREANDHASHALGORITHM);
	msglen += 2;
	extlen += 2;

	lstlen = 0;
	// signature_algorithms extension length
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)(lstlen));
	msglen += 2;
	extlen += 2;

	// SignatureAndHashAlgorithm
	PUT_SWORD_NET(prec->snd_msg, msglen, pcip->signature.sig_alg);
	msglen += 2;
	extlen += 2;
	lstlen += 2;

	//reset signature_algorithms extension length
	PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), (unsigned short)(lstlen));

	if (psec->chain_ca)
	{
		// Extension type: Certificate Authorities (47)
		PUT_SWORD_NET(prec->snd_msg, msglen, SSL_EXTENSION_CERTIFICATE_AUTHORITIES);
		msglen += 2;
		extlen += 2;

		lstlen = 0;
		// Certificate Authorities extension length
		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)lstlen);
		msglen += 2;
		extlen += 2;

		/*
		* struct {
		* DistinguishedName authorities<3..2^16-1>;
		* } CertificateAuthoritiesExtension;
		*/

		//DistinguishedName group length
		grplen = 0;
		PUT_SWORD_NET(prec->snd_msg, msglen, grplen);
		msglen += 2;
		extlen += 2;
		lstlen += 2;

		pcrt = psec->chain_ca;
		while (pcrt != NULL && pcrt->next != NULL)
		{
			/*
			opaque DistinguishedName<1..2^16-1>;
			*/
			n = pcrt->subject_raw.len;
			PUT_SWORD_NET(prec->snd_msg, msglen, n);
			msglen += 2;
			extlen += 2;
			lstlen += 2;
			grplen += 2;

			xmem_copy(prec->snd_msg + msglen, pcrt->subject_raw.p, n);

			msglen += n;
			extlen += n;
			lstlen += n;
			grplen += n;

			pcrt = pcrt->next;
		}

		//reset DistinguishedName group length
		PUT_SWORD_NET(prec->snd_msg, (msglen - grplen - 2), grplen);
		//reset Certificate Authorities extension length
		PUT_SWORD_NET(prec->snd_msg, (msglen - lstlen - 2), lstlen);
	}

	//Extensions end
	//reset Extensions length
	PUT_SWORD_NET(prec->snd_msg, (msglen - extlen - 2), (unsigned short)(extlen));

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

	return SSL_SERVER_CERTIFICATE;
	
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_write_server_certificate(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen, crtlen, extlen;
	x509_crt *crt;

	TRY_CATCH;

	/*
	* struct {
	* opaque certificate_request_context<0..2 ^ 8 - 1>;
	*  CertificateEntry certificate_list<0..2 ^ 24 - 1>;
	* } Certificate;
	*/

	msglen = SSL_HSH_SIZE;

	/*
	* certificate_request_context (empty)
	*/
	PUT_BYTE(prec->snd_msg, msglen, 0);
	msglen++;

	/*
	* certificate_list
	*/
	//preset certs length to zero
	crtlen = 0;
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, crtlen);
	msglen += 3;

	crt = psec->host_crt;
	while (crt != NULL && crt->version != 0)
	{
		/*
		* struct {
		* select (certificate_type) {
		* case RawPublicKey: --From RFC 7250 ASN.1_subjectPublicKeyInfo
		* opaque ASN1_subjectPublicKeyInfo<1..2 ^ 24 - 1>;
		* case X509:
		* opaque cert_data<1..2 ^ 24 - 1>;
		* };
		* Extension extensions<0..2 ^ 16 - 1>;
		* } CertificateEntry;
		*/
		n = crt->raw.len;
		if (msglen + 3 + n > SSL_PKG_SIZE)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_write_server_certificate"), _T("invalid message length"));
		}

		//peer cert length
		PUT_THREEBYTE_NET(prec->snd_msg, msglen, n);
		msglen += 3;
		crtlen += 3;

		//peer cert data
		xmem_copy(prec->snd_msg + msglen, crt->raw.p, n);
		msglen += n;
		crtlen += n;

		//zero extensions
		extlen = 0;
		PUT_DWORD_NET(prec->snd_msg, msglen, extlen);
		msglen += 2;
		crtlen += 2;

		msglen += extlen;
		crtlen += extlen;

		crt = crt->next;
	}

	//reset certificate_list length
	PUT_THREEBYTE_NET(prec->snd_msg, (msglen - crtlen - 3), crtlen);

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

	return SSL_SERVER_CERTIFICATE_VERIFY;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static int _ssl_write_server_certificate_verify(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int n, msglen;
	byte_t hash[MD_MAX_SIZE];
	int hlen;
	int sig_alg, sig_md, sig_pk;
	const md_info_t* md_info;
	void* md_ctx;

	byte_t blank_context_prefic[64] = { 0x20 };
	char server_context_string[] = "TLS 1.3, server CertificateVerify";
	byte_t zero[1] = { 0 };

	TRY_CATCH;

	//certificate verify hash = Transcript-Hash(Handshake Context, Certificate))
	_ssl_gen_handshake_hash(pcip, hash, &hlen);

	/*
	struct {
          SignatureScheme algorithm;
          opaque signature<0..2^16-1>;
      } CertificateVerify;
	*/
	msglen = SSL_HSH_SIZE;

	//algorithm
	sig_alg = TLS_ALG_RSA_PKCS1_SHA256;
	PUT_SWORD_NET(prec->snd_msg, msglen, sig_alg);
	msglen += 2;

	if (!_ssl_select_signature(sig_alg, &sig_pk, &sig_md))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_server_certificate_verify"), _T("invalid signature algorithm"));
	}

	md_info = md_info_from_type(sig_md);
	if (!md_info)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_server_certificate_verify"), _T("md_info_from_type"));
	}
	md_ctx = md_alloc(md_info);
	if (!md_ctx)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_server_certificate_verify"), _T("invalid signature context"));
	}

	md_starts(md_info, md_ctx);
	md_update(md_info, md_ctx, blank_context_prefic, 64);
	md_update(md_info, md_ctx, (byte_t*)server_context_string, a_xslen(server_context_string));
	md_update(md_info, md_ctx, zero, 1);
	md_update(md_info, md_ctx, hash, hlen);
	xmem_zero((void*)hash, md_info->size);
	md_finish(md_info, md_ctx, hash);
	md_free(md_info, md_ctx);

	if (psec->rsa_ctx == NULL)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_server_certificate_verify"), _T("invalid rsa context"));
	}

	//signature length
	n = psec->rsa_ctx->len;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)n);
	msglen += 2;

	//signature
	if (C_OK != rsa_pkcs1_sign(psec->rsa_ctx, pssl->f_rng, pssl->r_rng, RSA_PRIVATE, md_info->type, md_info->size, hash, prec->snd_msg + msglen))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_server_certificate_verify"), _T("rsa_pkcs1_sign"));
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
		raise_user_error(_T("_ssl_write_server_certificate_verify"), _T("_ssl_write_snd_msg"));
	}

	END_CATCH;

	return SSL_SERVER_FINISHED;
	
ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_write_server_finished(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->snd_record;

	int msglen;
	byte_t* mac_buf;

	byte_t hash[MD_MAX_SIZE] = { 0 };
	int hlen;
	const md_info_t* md_info;

	byte_t okm[MD_MAX_SIZE] = { 0 };
	int okm_len = 0;

	TRY_CATCH;

	//finished hash = Transcript-Hash(Handshake Context, Certificate*, CertificateVerify*))
	_ssl_gen_handshake_hash(pcip, hash, &hlen);

	//finished key
	ssl_expand(pcip->cipher.md_type, pcip->handshake_secret, pcip->handshake_secret_length, "finished", "", 0, okm, okm_len);

	//finished mac
	md_info = md_info_from_type(pcip->cipher.md_type);
	if (!md_info)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_write_server_finished"), _T("md_info_from_type"));
	}

	/*
	struct {
	opaque verify_data[Hash.length];
	} Finished;
	*/

	msglen = SSL_HSH_SIZE;
	mac_buf = prec->snd_msg + msglen;

	md_hmac(md_info, okm, okm_len, hash, hlen, mac_buf);
	msglen += md_info->size;

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

	return (psec->verify_mode == SSL_VERIFY_NONE) ? SSL_CLIENT_FINISHED : SSL_CLIENT_CERTIFICATE;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_parse_client_certificate(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int msglen, haslen, crtlen, extlen, lstlen;
	int n, type, cert;

	TRY_CATCH;

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE)
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
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CERTIFICATE)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_certificate"), _T("md_info_from_type"));
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	/*
	 struct {
          opaque certificate_request_context<0..2^8-1>;
          CertificateEntry certificate_list<0..2^24-1>;
      } Certificate;
	*/
	msglen = SSL_HSH_SIZE;

	//certificate_request_context length
	n = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	//certificate_request_context
	msglen += n;

	//certificate_list length
	crtlen = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	//the empty certificate list
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
			return SSL_CLIENT_FINISHED;
		}
	}

	psec->peer_crt = (x509_crt*)xmem_alloc(sizeof(x509_crt));

	while (crtlen)
	{
		/*
		struct {
		select (certificate_type) {
		case RawPublicKey:  --From RFC 7250 ASN.1_subjectPublicKeyInfo 
			opaque ASN1_subjectPublicKeyInfo<1..2 ^ 24 - 1>;
        case X509:
			opaque cert_data<1..2 ^ 24 - 1>;
		};
		Extension extensions<0..2 ^ 16 - 1>;
		} CertificateEntry;
		*/

		//cert_data
		n = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
		msglen += 3;
		crtlen -= 3;

		if (n < 128 || n > crtlen)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid certificate"));
		}

		if (C_OK != x509_crt_parse(psec->peer_crt, prec->rcv_msg + msglen, n))
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("x509_crt_parse"));
		}

		msglen += n;
		crtlen -= n;

		//extensions
		//extension length
		extlen = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;
		crtlen -= 2;

		crtlen -= extlen;

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

			if (!lstlen)
			{
				continue;
			}

			switch (type)
			{
			case SSL_EXTENSION_CLIENT_CERTIFICATE_TYPE:
				cert = GET_BYTE(prec->rcv_msg, msglen);

				msglen += 1;
				extlen -= 1;
				lstlen -= 1;
				break;
			default:
				//skip 
				break;
			}

			msglen += lstlen;
			extlen -= lstlen;

			if (extlen < 0)
			{
				_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
				raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid message length"));
			}
		}
	}

	if (psec->verify_mode != SSL_VERIFY_NONE)
	{
		if (psec->chain_ca == NULL)
		{
			_ssl_set_error(pses->alert_code = SSL_ALERT_UNKNOWN_CA);
			raise_user_error(_T("_ssl_parse_client_certificate"), _T("invalid ca"));
		}

		if (C_OK != x509_crt_verify(psec->peer_crt, psec->chain_ca, NULL, psec->peer_cn, &n, NULL, NULL))
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

	return SSL_CLIENT_CERTIFICATE_VERIFY;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_parse_client_certificate_verify(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int msglen, haslen, n;
	byte_t hash[MD_MAX_SIZE];
	int hlen;
	int sig_alg, sig_md, sig_pk;
	const md_info_t* md_info;
	void* md_ctx;

	byte_t blank_context_prefic[64] = { 0x20 };
	char client_context_string[] = "TLS 1.3, client CertificateVerify";
	byte_t zero[1] = { 0 };

	TRY_CATCH;

	//certificate verify hash = Transcript-Hash(Handshake Context, Certificate)
	_ssl_gen_handshake_hash(pcip, hash, &hlen);

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
	* struct {
	* SignatureScheme algorithm;
	* opaque signature<0..2^16-1>;
	* } CertificateVerify;
	*/
	msglen = SSL_HSH_SIZE;

	/*
	* algorithm
	*/
	sig_alg = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (!_ssl_select_signature(sig_alg, &sig_pk, &sig_md))
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("invalid signature algorithm"));
	}

	md_info = md_info_from_type(sig_md);
	if (!md_info)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("invalid signature type"));
	}
	md_ctx = md_alloc(md_info);
	if (!md_ctx)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("invalid signature context"));
	}

	md_starts(md_info, md_ctx);
	md_update(md_info, md_ctx, blank_context_prefic, 64);
	md_update(md_info, md_ctx, (byte_t*)client_context_string, a_xslen(client_context_string));
	md_update(md_info, md_ctx, zero, 1);
	md_update(md_info, md_ctx, hash, hlen);
	xmem_zero((void*)hash, md_info->size);
	md_finish(md_info, md_ctx, hash);
	md_free(md_info, md_ctx);

	if (psec->peer_crt == NULL)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_NO_CERTIFICATE);
		raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("invalid peer certificate"));
	}

	/*
	* signature
	*/
	n = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (n != psec->peer_crt->rsa->len)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_ILLEGAL_PARAMETER);
		raise_user_error(_T("_ssl_parse_client_certificate_verify"), _T("invalid rsa context"));
	}

	if (C_OK != rsa_pkcs1_verify(psec->peer_crt->rsa, pssl->f_rng, pssl->r_rng, RSA_PUBLIC, md_info->type, md_info->size, hash, prec->rcv_msg + msglen))
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

	return SSL_CLIENT_FINISHED;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

static tls13_handshake_states _ssl_parse_client_finished(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip = (tls13_cipher_context*)pses->cipher_context;
	ssl_security_context* psec = (ssl_security_context*)pssl->security_context;
	ssl_record_context* prec = (ssl_record_context*)pses->rcv_record;

	int mac_len;
	byte_t mac_buf[MD_MAX_SIZE];

	byte_t hash[MD_MAX_SIZE] = { 0 };
	int hlen;
	const md_info_t* md_info;

	byte_t okm[MD_MAX_SIZE] = { 0 };
	int okm_len = 0;

	TRY_CATCH;

	//finished hash = Transcript-Hash(Handshake Context, Certificate*, CertificateVerify*))
	_ssl_gen_handshake_hash(pcip, hash, &hlen);

	if (prec->rcv_msg[0] != SSL_HS_FINISHED)
	{
		if (C_OK != _ssl_read_rcv_msg(pssl))
		{
			raise_user_error(_T("_ssl_parse_client_finished"), _T("_ssl_read_rcv_msg"));
		}
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

	//finished key
	ssl_expand(pcip->cipher.md_type, pcip->handshake_secret, pcip->handshake_secret_length, "finished", "", 0, okm, okm_len);

	md_info = md_info_from_type(pcip->cipher.md_type);
	if (!md_info)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_finished"), _T("invalid signature type"));
	}
	//finished mac
	md_hmac(md_info, okm, okm_len, hash, hlen, mac_buf);
	mac_len = md_info->size;

	/*
	struct {
	opaque verify_data[Hash.length];
	} Finished;
	*/

	if (xmem_comp(prec->rcv_msg + SSL_HSH_SIZE, mac_buf, mac_len) != 0)
	{
		_ssl_set_error(pses->alert_code = SSL_ALERT_UNEXPECTED_MESSAGE);
		raise_user_error(_T("_ssl_parse_client_finished"), _T("SSL_ALERT_BAD_RECORD_MAC"));
	}

	END_CATCH;

	return SSL_HANDSHAKE_OVER;

ONERROR:
	XDK_TRACE_LAST;

	return SSL_HANDSHAKE_ERROR;
}

bool_t tls13_handshake_server(ssl_context *pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	tls13_cipher_context* pcip;

	tls13_handshake_states state = SSL_CLIENT_HELLO;

	pcip = _ssl_alloc_cipher(pssl);

	while (state != SSL_HANDSHAKE_OVER)
	{
		switch (state)
		{
		case SSL_HELLO_RETRY_REQUEST:
			break;

			/*
			*  <==   (ChangeCipherSpec) ClientHello
			*/
		case SSL_CLIENT_HELLO:
			state = _ssl_parse_client_hello(pssl);
			break;

			/*
			*  ==>   ServerHello  (  HelloRetryRequest )
			*       ServerExtensions
			*      ( CertificateRequest )
			*        ServerCertificate
			*			Finished
			*/
		case SSL_SERVER_HELLO:
			if (pcip->hello_retry)
			{
				state = _ssl_write_server_hello_retry_request(pssl);
			}
			else
			{
				_ssl_extract_early_secret(pcip);

				state = _ssl_write_server_hello(pssl);
			}
			break;
		case SSL_SERVER_EXTENSIONS:
			_ssl_extract_handshake_secret(pcip);
			_ssl_derive_handshake_key(pcip);
			_ssl_reset_sequence_number(pses);

			state = _ssl_write_server_encrypted_extensions(pssl);
			break;

		case SSL_CERTIFICATE_REQUEST:
			state = _ssl_write_server_certificate_request(pssl);
			break;

		case SSL_SERVER_CERTIFICATE:
			state = _ssl_write_server_certificate(pssl);
			break;
		case SSL_SERVER_CERTIFICATE_VERIFY:
			state = _ssl_write_server_certificate_verify(pssl);
			break;
		case SSL_SERVER_FINISHED:
			state = _ssl_write_server_finished(pssl);
			break;
			/*
			*  <== ( Certificate/Alert  )
			*      ( CertificateVerify  )
			*        ChangeCipherSpec
			*        Finished
			*/
		case SSL_CLIENT_CERTIFICATE:
			state = _ssl_parse_client_certificate(pssl);
			break;
		case SSL_CLIENT_CERTIFICATE_VERIFY:
			state = _ssl_parse_client_certificate_verify(pssl);
			break;
		case SSL_CLIENT_FINISHED:
			state = _ssl_parse_client_finished(pssl);

			_ssl_extract_master_secret(pcip);
			_ssl_derive_application_key(pcip);
			_ssl_reset_sequence_number(pses);
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
