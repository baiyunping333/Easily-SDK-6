/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@authen ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc dtls document

	@module	netdtls.c | implement file

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

#include "netdtls.h"

#include "../xdknet.h"
#include "../xdkimp.h"
#include "../xdkoem.h"
#include "../xdkstd.h"

#if defined(XDK_SUPPORT_SOCK)

static char dhm_G[] = "4";
static char dhm_P[] = "E4004C1F94182000103D883A448B3F802CE4B44A83301270002C20D0321CFD00" \
"11CCEF784C26A400F43DFB901BCA7538F2C6B176001CF5A0FD16D2C48B1D0C1C" \
"F6AC8E1DA6BCC3B4E1F96B0564965300FFA1D0B601EB2800F489AA512C4B248C" \
"01F76949A60BB7F00A40B1EAB64BDD48E8A700D60B7F1200FA8E77B0A979DABF";

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

#define DTLS_TRY_MAX 3

typedef struct _dtls10_ciphers_set{
	int cipher;
	int type;
	int bulk;
	int key_size;
	int mac_size;
	int iv_size;
}dtls10_ciphers_set;


static dtls10_ciphers_set client_ciphers[] = {
	{ SSL_DHE_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16 },
	{ SSL_DHE_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16 },
	{ SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8 },
	{ SSL_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16 },
	{ SSL_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16 },
	{ SSL_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8 },
};

static dtls10_ciphers_set server_ciphers[] = {
	{ SSL_DHE_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16 },
	{ SSL_DHE_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16 },
	{ SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8 },
	{ SSL_RSA_WITH_AES_256_CBC_SHA, CIPHER_BLOCK, BULK_AES, 32, 20, 16 },
	{ SSL_RSA_WITH_AES_128_CBC_SHA, CIPHER_BLOCK, BULK_AES, 16, 20, 16 },
	{ SSL_RSA_WITH_3DES_EDE_CBC_SHA, CIPHER_BLOCK, BULK_3DES, 24, 20, 8 },
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
}dtls10_handshake_states;

typedef struct _dtls10_cipher_context{
	//SecurityParameters
	int endpoint;		//ConnectionEnd: { server, client }
	int alg_prf;	//PRFAlgorithm: enum { tls_prf_sha256 } 
	int cipher_bulk;	//BulkCipherAlgorithm: enum { null, rc4, rc2, des, 3des, des40, idea, aes }
	int cipher_type;	//CipherType: { stream, block }
	int cipher;			//the selected cipher
	int key_size;		//the encrypt and decrypt key size
	int key_material_length; //the most material length for key generating
	int exportable;		//IsExportable: { true, false } 
	int alg_mac;		//MACAlgorithm:  enum { null, md5, sha }
	int mac_size;		//hash length
	int iv_size;		//IV block size
	int compress_method; //CompressionMethod: { null, (0), (255) }
	byte_t master_secret[DTLS_MST_SIZE];
	byte_t rnd_srv[DTLS_RND_SIZE]; //server_random
	byte_t rnd_cli[DTLS_RND_SIZE]; //client_random

	//Generated by SecurityParameters
	byte_t iv_enc[DTLS_MAX_IVC];
	byte_t iv_dec[DTLS_MAX_IVC];
	byte_t mac_enc[DTLS_MAX_MAC];
	byte_t mac_dec[DTLS_MAX_MAC];
	dword_t ctx_enc[DTLS_CTX_SIZE];
	dword_t ctx_dec[DTLS_CTX_SIZE];

	//Tools
	md5_context md5;
	sha1_context sha1;
}dtls10_cipher_context;

#define DTLS0_RECORD_EPOCH(prec)		(int)(GET_LWORD_NET(prec->snd_hdr, 3) >> 48)
#define DTLS0_RECORD_SEQNUM(ptr)	(GET_LWORD_NET(prec->snd_hdr, 3) & 0x0000FFFFFFFFFFFF)


#define IS_DHE_CIPHER(cipher) ((cipher == SSL_DHE_RSA_WITH_AES_256_CBC_SHA || \
								cipher == SSL_DHE_RSA_WITH_AES_128_CBC_SHA || \
								cipher == SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA) ? 1 : 0)
/***********************************************************************************************************************************/

static bool_t _dtls_choose_cipher(dtls10_cipher_context* pcip, int ciph)
{
	int i, n;
	dtls10_ciphers_set* pcs;

	if (pcip->endpoint == DTLS_TYPE_CLIENT)
	{
		n = sizeof(client_ciphers) / sizeof(dtls10_ciphers_set);
		pcs = client_ciphers;
	}
	else
	{
		n = sizeof(server_ciphers) / sizeof(dtls10_ciphers_set);
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

			return 1;
		}
	}

	set_last_error(_T("_dtls_choose_cipher"), _T("unknown cipher"), -1);

	return 0;
}

static void _dtls_derive_keys(dtls10_cipher_context* pcip, byte_t* premaster, int prelen)
{
	byte_t rndb[DTLS_RND_SIZE * 2] = { 0 };
	byte_t keyblk[DTLS_BLK_SIZE] = { 0 };

	byte_t *key_enc, *key_dec;

	//generating master security
	xmem_copy((void*)rndb, (void*)pcip->rnd_cli, DTLS_RND_SIZE);
	xmem_copy((void*)(rndb + DTLS_RND_SIZE), (void*)pcip->rnd_srv, DTLS_RND_SIZE);

	ssl_prf1(premaster, prelen, label_master_secret, rndb, DTLS_RND_SIZE * 2, pcip->master_secret, DTLS_MST_SIZE);

	// swap the client and server random values.
	xmem_copy((void*)rndb, (void*)pcip->rnd_srv, DTLS_RND_SIZE);
	xmem_copy((void*)(rndb + DTLS_RND_SIZE), (void*)pcip->rnd_cli, DTLS_RND_SIZE);

	xmem_zero(pcip->rnd_cli, sizeof(pcip->rnd_cli));
	xmem_zero(pcip->rnd_srv, sizeof(pcip->rnd_srv));

	// generate key block
	//key_block = 
	//PRF(SecurityParameters.master_secret,
	//"key expansion",
	//SecurityParameters.server_random +
	//SecurityParameters.client_random);
	ssl_prf1(pcip->master_secret, DTLS_MST_SIZE, label_key_expansion, rndb, DTLS_RND_SIZE * 2, keyblk, DTLS_BLK_SIZE);

	//the key_block is partitioned as follows:
	//client_write_MAC_secret[SecurityParameters.hash_size]
	//server_write_MAC_secret[SecurityParameters.hash_size]
	//client_write_key[SecurityParameters.key_material_length]
	//server_write_key[SecurityParameters.key_material_length]
	//client_write_IV[SecurityParameters.IV_size]
	//server_write_IV[SecurityParameters.IV_size]
	if (pcip->endpoint == DTLS_TYPE_CLIENT)
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
		xmem_copy(pcip->iv_enc, keyblk + pcip->mac_size * 2 + pcip->key_size * 2, pcip->iv_size);
		//server_write_IV for client decrypting IV
		xmem_copy(pcip->iv_dec, keyblk + pcip->mac_size * 2 + pcip->key_size * 2 + pcip->iv_size, pcip->iv_size);
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
		xmem_copy(pcip->iv_dec, keyblk + pcip->mac_size * 2 + pcip->key_size * 2, pcip->iv_size);
		//server_write_IV for server encrypting IV
		xmem_copy(pcip->iv_enc, keyblk + pcip->mac_size * 2 + pcip->key_size * 2 + pcip->iv_size, pcip->iv_size);
	}

	//initialize encrypt and decrypt context
	switch (pcip->cipher)
	{
	case SSL_RSA_WITH_3DES_EDE_CBC_SHA:
	case SSL_DHE_RSA_WITH_3DES_EDE_CBC_SHA:
		des3_set3key_enc((des3_context *)pcip->ctx_enc, key_enc); //the material size is 24 bytes
		des3_set3key_dec((des3_context *)pcip->ctx_dec, key_dec); //the material size is 24 bytes
		break;
	case SSL_RSA_WITH_AES_128_CBC_SHA:
	case SSL_RSA_WITH_AES_256_CBC_SHA:
	case SSL_DHE_RSA_WITH_AES_128_CBC_SHA:
	case SSL_DHE_RSA_WITH_AES_256_CBC_SHA:
		aes_setkey_enc((aes_context *)pcip->ctx_enc, key_enc, (pcip->key_size * 8)); //the material size is bits
		aes_setkey_dec((aes_context *)pcip->ctx_dec, key_dec, (pcip->key_size * 8)); //the material size is bits
		break;
	}
}

static int _dtls_encrypt_snd_msg(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);

	int i, padlen;
	byte_t* mac_buf;
	byte_t iv_pre[16] = { 0 };
	int iv_copy;

	mac_buf = prec->snd_msg + prec->snd_msg_len; 

	//The MAC is generated as:
	//HMAC_hash(MAC_write_secret, seq_num + TLSCompressed.type + TLSCompressed.version + TLSCompressed.length + TLSCompressed.fragment));

	if (pcip->mac_size == 16)
	{
		md5_hmac(pcip->mac_enc, pcip->mac_size, prec->snd_ctr, prec->snd_msg_len + DTLS_CTR_SIZE + DTLS_HDR_SIZE, mac_buf);
	}
	else if (pcip->mac_size == 20)
	{
		sha1_hmac(pcip->mac_enc, pcip->mac_size, prec->snd_ctr, prec->snd_msg_len + DTLS_CTR_SIZE + DTLS_HDR_SIZE, mac_buf);
	}
	else
	{
		set_last_error(_T("_dtls_encrypt_snd_msg"), _T("unknown hmac function"), -1);

		return C_ERR;
	}

	prec->snd_msg_len += pcip->mac_size;

	if (pcip->cipher_type == CIPHER_BLOCK)
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
	else
	{
		set_last_error(_T("_dtls_encrypt_snd_msg"), _T("unknown crypt cipher"), -1);

		return C_ERR;
	}

	//reset message length
	PUT_SWORD_NET(prec->snd_hdr, (DTLS_HDR_SIZE - 2), (unsigned short)prec->snd_msg_len);

	return C_OK;
}

static int _dtls_decrypt_rcv_msg(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);

	int i, padlen = 0;
	byte_t* mac_buf;
	byte_t mac_tmp[32];

	if (prec->rcv_msg_len < (pcip->mac_size + pcip->iv_size))
	{
		set_last_error(_T("_dtls_decrypt_rcv_msg"), _T("message length to small"), -1);

		return C_ERR;
	}

	if (pcip->cipher_type == CIPHER_BLOCK)
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
			set_last_error(_T("_dtls_decrypt_rcv_msg"), _T("message length not multiple of IV size"), -1);

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
			set_last_error(_T("_dtls_decrypt_rcv_msg"), _T("invalid message pading length"), -1);

			return C_ERR;
		}

		prec->rcv_msg_len -= padlen;
	}
	else
	{
		set_last_error(_T("_dtls_decrypt_rcv_msg"), _T("unknown crypt cipher"), -1);

		return C_ERR;
	}

	prec->rcv_msg_len -= pcip->mac_size;
	mac_buf = prec->rcv_msg + prec->rcv_msg_len;

	//reset message length
	PUT_SWORD_NET(prec->rcv_hdr, (DTLS_HDR_SIZE - 2), (unsigned short)prec->rcv_msg_len);

	if (pcip->mac_size == 16)
		md5_hmac(pcip->mac_dec, pcip->mac_size, prec->rcv_ctr, (prec->rcv_msg_len + DTLS_CTR_SIZE + DTLS_HDR_SIZE), mac_tmp);
	else if (pcip->mac_size == 20)
		sha1_hmac(pcip->mac_dec, pcip->mac_size, prec->rcv_ctr, (prec->rcv_msg_len + DTLS_CTR_SIZE + DTLS_HDR_SIZE), mac_tmp);
	
	if (xmem_comp((void*)mac_tmp, (void*)mac_buf, pcip->mac_size) != 0)
	{
		set_last_error(_T("_dtls_decrypt_rcv_msg"), _T("message signature hash not matched"), -1);

		return C_ERR;
	}

	return C_OK;
}

static int _dtls_write_snd_msg(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);

	dword_t dw;
	int i, haslen, htype;
	byte_t* token;
	int total;

	byte_t* lin_buf;
	int lin_len, frm_off, frm_len;

	/*
	struct {
        ContentType type;
        ProtocolVersion version;
        uint16 epoch;                                     // New field
        uint48 sequence_number;                           // New field
        uint16 length;
        select (CipherSpec.cipher_type) {
          case block:  GenericBlockCipher;
        } fragment;
      } DTLSCiphertext;
	*/

	if (prec->snd_msg_type == SSL_MSG_HANDSHAKE && prec->snd_msg[0] != SSL_HS_SERVER_HELLO_DONE && prec->snd_msg[0] != SSL_HS_FINISHED)
	{
		token = prec->snd_msg;
		total = prec->snd_msg_len;
		if (total)
		{
			haslen = GET_THREEBYTE_NET(token, 1);

			//not include the first ClientHello (with none cookie) or ServerHelloVerifyRequest message 
			if (pses->snd_next_seqnum)
			{
				md5_update(&pcip->md5, token, DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
				sha1_update(&pcip->sha1, token, DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
			}

			total = haslen;
			htype = GET_BYTE(token, 0);
		}
		else
		{
			total = 0;
		}

		// split the handshake package
		frm_off = 0;
		while (total)
		{
			lin_len = (DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen - frm_off < pses->pkg_size) ? (DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen - frm_off) : pses->pkg_size;
			lin_len += (DTLS_HDR_SIZE);
			lin_buf = insert_linear_frame(pses->snd_linear, pses->snd_next_seqnum, lin_len);
			if (!lin_buf)
			{
				set_last_error(_T("_dtls_write_snd_msg"), _T("linear insert failed"), -1);
				return C_ERR;
			}
			frm_len = (lin_len - DTLS_HDR_SIZE - DTLS_HSH_SIZE - DTLS_MSH_SIZE);

			xmem_copy((void*)(lin_buf + DTLS_HDR_SIZE + DTLS_HSH_SIZE + DTLS_MSH_SIZE), (void*)(prec->snd_hdr + DTLS_HDR_SIZE + DTLS_HSH_SIZE + DTLS_MSH_SIZE + frm_off), frm_len);

			// adjust message header
			PUT_BYTE(lin_buf, 0, (byte_t)(prec->snd_msg_type));
			PUT_BYTE(lin_buf, 1, (byte_t)(pses->major_ver));
			PUT_BYTE(lin_buf, 2, (byte_t)(pses->minor_ver));
			PUT_LWORD_NET(lin_buf, 3, ((unsigned long long)pses->snd_next_epoch << 48 | pses->snd_next_seqnum));
			PUT_SWORD_NET(lin_buf, 11, (unsigned short)(lin_len - DTLS_HDR_SIZE));
			// adjust handshake
			// handshake type
			PUT_BYTE(lin_buf, 13, (byte_t)htype);
			// handshake length
			PUT_THREEBYTE_NET(lin_buf, 14, haslen);
			// message_seq
			PUT_SWORD_NET(lin_buf, 17, pses->snd_next_msgnum);
			// fragment_offset
			PUT_THREEBYTE_NET(lin_buf, 19, frm_off);
			// fragment_length
			PUT_THREEBYTE_NET(lin_buf, 22, frm_len);

			frm_off += frm_len;
			total -= frm_len;

			if (!(*pdtls->pif->pf_write)(pdtls->pif->fd, lin_buf, &lin_len))
			{
				set_last_error(_T("_dtls_write_snd_msg"), _T("write message block failed"), -1);
				return C_ERR;
			}

			(*pdtls->pif->pf_flush)(pdtls->pif->fd);

			prec->snd_msg_pop = 0;
			pses->snd_next_seqnum++;
		}

		pses->snd_next_msgnum++;
	}
	else
	{
		pses->snd_next_epoch = pses->snd_next_epoch;
		pses->snd_next_seqnum = pses->snd_next_seqnum;
		pses->snd_next_msgnum = pses->snd_next_msgnum;

		PUT_BYTE(prec->snd_hdr, 0, (byte_t)(prec->snd_msg_type));
		PUT_BYTE(prec->snd_hdr, 1, (byte_t)(pses->major_ver));
		PUT_BYTE(prec->snd_hdr, 2, (byte_t)(pses->minor_ver));
		PUT_LWORD_NET(prec->snd_hdr, 3, ((unsigned long long)pses->snd_next_epoch << 48 | pses->snd_next_seqnum));
		PUT_SWORD_NET(prec->snd_hdr, 11, (unsigned short)(prec->snd_msg_len));

		if (prec->snd_msg_type == SSL_MSG_HANDSHAKE)
		{
			token = prec->snd_msg;
			total = prec->snd_msg_len;
			while (total)
			{
				haslen = GET_THREEBYTE_NET(token, 1);

				md5_update(&pcip->md5, token, DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
				sha1_update(&pcip->sha1, token, DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);

				total -= (DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
				token += (DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
			}
		}

		if (prec->crypted)
		{
			if (C_OK != _dtls_encrypt_snd_msg(pdtls))
			{
				set_last_error(_T("_dtls_write_snd_msg"), _T("encrypt message block failed"), -1);
				return C_ERR;
			}

			//incre send message control bits
			for (i = DTLS_CTR_SIZE - 1; i >= 0; i--)
			{
				if (++prec->snd_ctr[i] != 0)
					break;
			}
		}

		dw = DTLS_HDR_SIZE + prec->snd_msg_len;
		if (!(*pdtls->pif->pf_write)(pdtls->pif->fd, prec->snd_hdr, &dw))
		{
			set_last_error(_T("_dtls_write_snd_msg"), _T("write message block failed"), -1);
			return C_ERR;
		}

		(*pdtls->pif->pf_flush)(pdtls->pif->fd);

		prec->snd_msg_pop = 0;
		pses->snd_next_seqnum++;

		if (prec->snd_msg_type == SSL_MSG_HANDSHAKE)
		{
			pses->snd_next_msgnum++;
		}
	}

	return C_OK;
}

static int _dtls_read_rcv_msg(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);

	dword_t dw;
	int i, haslen;
	byte_t* token;
	int total;

	int htype;
	byte_t* lin_buf;
	dword_t lin_len, frm_off, frm_len;
	dword_t epoch, seqnum;
	sword_t msgnum;

	/*
	struct {
        ContentType type;
        ProtocolVersion version;
        uint16 epoch;                                     // New field
        uint48 sequence_number;                           // New field
        uint16 length;
        select (CipherSpec.cipher_type) {
          case block:  GenericBlockCipher;
        } fragment;
      } DTLSCiphertext;
	*/
	//if the head already readed at handshake begain

retain:

	if (pses->major_ver)
	{
		dw = DTLS_HDR_SIZE;
		if (!(pdtls->pif->pf_read)(pdtls->pif->fd, prec->rcv_hdr, &dw))
		{
			set_last_error(_T("_dtls_read_rcv_msg"), _T("read message head failed"), -1);
			return C_ERR;
		}

		if (!dw)
		{
			xmem_zero((void*)prec->rcv_hdr, DTLS_HDR_SIZE);
			return C_OK;
		}

		if (prec->rcv_hdr[1] != DTLS_MAJOR_VERSION_1)
		{
			set_last_error(_T("_dtls_read_rcv_msg"), _T("major version mismatch"), -1);
			return C_ERR;
		}

		if (prec->rcv_hdr[2] > SSL_MINOR_VERSION_0)
		{
			set_last_error(_T("_dtls_read_rcv_msg"), _T("minor version mismatch"), -1);
			return C_ERR;
		}

		prec->rcv_msg_type = GET_BYTE(prec->rcv_hdr, 0);
		epoch = (int)(GET_LWORD_NET(prec->rcv_hdr, 3) >> 48);
		seqnum = (GET_LWORD_NET(prec->rcv_hdr, 3) & 0x0000FFFFFFFFFFFF);
		prec->rcv_msg_len = GET_SWORD_NET(prec->rcv_hdr, 11);

		if (prec->rcv_msg_len < 1 || prec->rcv_msg_len > DTLS_MAX_SIZE)
		{
			set_last_error(_T("_dtls_read_rcv_msg"), _T("invalid message block length"), -1);
			return C_ERR;
		}

		dw = prec->rcv_msg_len;
		if (!(*pdtls->pif->pf_read)(pdtls->pif->fd, prec->rcv_msg, &dw))
		{
			set_last_error(_T("_dtls_read_rcv_msg"), _T("read message block failed"), -1);
			return C_ERR;
		}
	}
	else
	{
		epoch = (int)(GET_LWORD_NET(prec->rcv_hdr, 3) >> 48);
		seqnum = (GET_LWORD_NET(prec->rcv_hdr, 3) & 0x0000FFFFFFFFFFFF);
	}

	if (prec->crypted)
	{
		if (C_OK != _dtls_decrypt_rcv_msg(pdtls))
		{
			set_last_error(_T("_dtls_read_rcv_msg"), _T("decrypt message block failed"), -1);
			return C_ERR;
		}

		//incre recv message control bits
		for (i = DTLS_CTR_SIZE - 1; i >= 0; i--)
		{
			if (++prec->rcv_ctr[i] != 0)
				break;
		}
	}

	if (prec->rcv_msg_type == SSL_MSG_HANDSHAKE)
	{
		lin_len = prec->rcv_msg_len + DTLS_HDR_SIZE;
		lin_buf = insert_linear_frame(pses->rcv_linear, seqnum, lin_len);
		if (!lin_buf)
		{
			set_last_error(_T("_dtls_read_rcv_msg"), _T("insert linear failed"), -1);
			return C_ERR;
		}
		xmem_copy((void*)lin_buf, (void*)(prec->rcv_hdr), lin_len);

		// handshake type
		htype = GET_BYTE(lin_buf, 13);
		// handshake length
		haslen = GET_THREEBYTE_NET(lin_buf, 14);
		// message_seq
		msgnum = GET_SWORD_NET(lin_buf, 17);
		// fragment_offset
		frm_off = GET_THREEBYTE_NET(lin_buf, 19);
		// fragment_length
		frm_len = GET_THREEBYTE_NET(lin_buf, 22);

		if (msgnum == pses->rcv_next_msgnum && (frm_off + frm_len) < (haslen))
		{
			prec->rcv_msg_pop = 0;
			pses->rcv_next_seqnum++;
			goto retain;
		}

		if (msgnum == pses->rcv_next_msgnum)
		{
			prec->rcv_msg_len = DTLS_HSH_SIZE + DTLS_MSH_SIZE;

			do{
				lin_buf = get_linear_frame(pses->rcv_linear, seqnum--, &lin_len);
				if (!lin_buf)
					break;

				msgnum = GET_SWORD_NET(lin_buf, 17);
				if (msgnum != pses->rcv_next_msgnum)
				{
					lin_buf = NULL;
					break;
				}

				// fragment_offset
				frm_off = GET_THREEBYTE_NET(lin_buf, 19);
				// fragment_length
				frm_len = GET_THREEBYTE_NET(lin_buf, 22);
				// merge the fragment
				xmem_copy((void*)(prec->rcv_msg + DTLS_HSH_SIZE + DTLS_MSH_SIZE + frm_off), (void*)(lin_buf + DTLS_HDR_SIZE + DTLS_HSH_SIZE + DTLS_MSH_SIZE), frm_len);
				prec->rcv_msg_len += frm_len;
			} while (frm_off);

			if (!lin_buf)
			{
				prec->rcv_msg_pop = 0;
				goto retain;
			}
			else
			{
				total = prec->rcv_msg_len;
				token = prec->rcv_msg;
				while (total)
				{
					haslen = GET_THREEBYTE_NET(token, 1);

					//reset fragment
					// fragment_offset
					PUT_THREEBYTE_NET(token, (DTLS_HSH_SIZE + 2), 0);
					// fragment_length
					PUT_THREEBYTE_NET(token, (DTLS_HSH_SIZE + 5), haslen);

					//not include the first ClientHello (with none cookie) or ServerHelloVerifyRequest message 
					if (pses->rcv_next_seqnum)
					{
						md5_update(&pcip->md5, token, DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
						sha1_update(&pcip->sha1, token, DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
					}

					total -= (DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
					token += (DTLS_HSH_SIZE + DTLS_MSH_SIZE + haslen);
				}

				prec->rcv_msg_pop = 0;
				pses->rcv_next_seqnum++;
				pses->rcv_next_msgnum++;
			}
		}
		else
		{
			prec->rcv_msg_pop = 0;
			goto retain;
		}
	}
	else
	{
		prec->rcv_msg_pop = 0;
		pses->rcv_next_seqnum++;
	}

	if (prec->rcv_msg_type == SSL_MSG_ALERT)
	{
		if (prec->rcv_msg[0] == SSL_LEVEL_FATAL)
		{
			set_last_error(_T("_dtls_read_rcv_msg"), _T("fatal alert message"), -1);
			return C_ERR;
		}

		if (prec->rcv_msg[0] == SSL_LEVEL_WARNING && prec->rcv_msg[1] == SSL_ALERT_CLOSE_NOTIFY)
		{
			pses->handshake_over = -1;
			prec->rcv_msg_len = 0;
		}
	}

	return C_OK;
}

static void _dtls_clear_flight(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;

	clear_linear(pses->snd_linear);

	clear_linear(pses->rcv_linear);
}

static bool_t _dtls_replay_flight(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	int seqnum;
	byte_t* lin_buf = NULL;
	dword_t lin_len;

	seqnum = get_linear_top(pses->snd_linear);

	do{
		lin_len = 0;
		lin_buf = get_linear_frame(pses->snd_linear, seqnum++, &lin_len);

		if (lin_buf && lin_len)
		{
			if (!(*pdtls->pif->pf_write)(pdtls->pif->fd, lin_buf, &lin_len))
			{
				return 0;
			}

			(*pdtls->pif->pf_flush)(pdtls->pif->fd);
		}
	} while (lin_buf);

	return 1;
}

static void _dtls_init_context(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls10_cipher_context* pcip;

	pses->cipher_context = (dtls_cipher_context_ptr)xmem_alloc(sizeof(dtls10_cipher_context));
	pcip = (dtls10_cipher_context*)pses->cipher_context;

	//initialize tools
	md5_starts(&pcip->md5);
	sha1_starts(&pcip->sha1);

	pcip->endpoint = pdtls->type;

	pdtls->dtls_send = _dtls_write_snd_msg;
	pdtls->dtls_recv = _dtls_read_rcv_msg;

	if (pdtls->type == DTLS_TYPE_SERVER)
	{
		pdtls->srv_major_ver = DTLS_MAJOR_VERSION_1;
		pdtls->srv_minor_ver = SSL_MINOR_VERSION_0;
	}
}

/***************************************client routing************************************************************/

static dtls10_handshake_states _dtls_write_client_hello(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int i, n;
	int msglen;
	dword_t t;

	//gen client random bytes
	t = get_times();
	PUT_DWORD_NET(pcip->rnd_cli, 0, t);

	/*for (i = 4; i < DTLS_RND_SIZE; i++)
	{
		pcip->rnd_cli[i] = (byte_t)havege_rand(&pcip->rng);
	}*/
	(*pdtls->f_rng)(pdtls->r_rng, (pcip->rnd_cli + 4), (DTLS_RND_SIZE - 4));

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ClientHello;
	} Handshake;
	*/

	//handshake type
	PUT_BYTE(prec->snd_msg, 0, (byte_t)SSL_HS_CLIENT_HELLO);
	//preset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct {
        ProtocolVersion client_version;
        Random random;
        SessionID session_id;
        opaque cookie<0..32>;                             // New field
        CipherSuite cipher_suites<2..2^16-1>;
        CompressionMethod compression_methods<1..2^8-1>;
      } ClientHello;
	*/

	/*
	ProtocolVersion
	*/
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pdtls->cli_major_ver));
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pdtls->cli_minor_ver));
	
	/*
	Random
	*/
	xmem_copy(prec->snd_msg + msglen, pcip->rnd_cli, DTLS_RND_SIZE);
	msglen += DTLS_RND_SIZE;

	/*
	SessionID
	*/
	n = pses->ses_size;
	if (n < 16 || n > 32 || pses->session_resumed == 0)
		n = 0;

	//sension id length
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)n);
	//sension id
	xmem_copy(prec->snd_msg + msglen, pses->ses_id, n);
	msglen += n;

	/*
	cookie
	*/
	//cookie id length
	n = pses->cookie_size;
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)n);
	//cookie id
	xmem_copy(prec->snd_msg + msglen, pses->cookie_id, n);
	msglen += n;

	/*
	CipherSuite
	*/
	n = sizeof(client_ciphers) / sizeof(dtls10_ciphers_set);

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

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _dtls_write_snd_msg(pdtls))
		return SSL_HANDSHAKE_ERROR;

	return  (pses->cookie_size) ? SSL_SERVER_HELLO : SSL_SERVER_HELLO_VERIFY_REQUEST;
}

static dtls10_handshake_states _dtls_parse_server_hello_verify_request(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen, haslen;
	int num, off, len;

	if (C_OK != _dtls_read_rcv_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ServerHello;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_HELLO_VERIFY_REQUEST)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("not handshake message"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (prec->rcv_msg_len < 38)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("handshake hello message block too short"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct {
        ProtocolVersion server_version;
        opaque cookie<0..32>;
      } HelloVerifyRequest;
	*/

	/*
	ProtocolVersion
	*/
	pdtls->srv_major_ver = GET_BYTE(prec->rcv_msg, msglen);
	pdtls->srv_minor_ver = GET_BYTE(prec->rcv_msg, msglen + 1);
	msglen += 2;

	if (pdtls->srv_major_ver != pdtls->cli_major_ver)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("handshake major version mistach"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (pdtls->srv_minor_ver > SSL_MINOR_VERSION_0)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("handshake minor version mistach"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	pses->major_ver = pdtls->cli_major_ver;
	pses->minor_ver = (pdtls->cli_minor_ver < pdtls->srv_minor_ver) ? pdtls->cli_minor_ver : pdtls->srv_minor_ver;

	/*
	cookie
	*/
	pses->cookie_size = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;
	xmem_copy((void*)(pses->cookie_id), (void*)(prec->rcv_msg + msglen), pses->cookie_size);
	msglen += pses->cookie_size;

	return SSL_CLIENT_HELLO;
}

static dtls10_handshake_states _dtls_parse_server_hello(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	dword_t t;
	int ciph;
	int msglen, haslen, seslen, extlen;
	int num, off, len;

	if (C_OK != _dtls_read_rcv_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ServerHello;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_SERVER_HELLO)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("not handshake message"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (prec->rcv_msg_len < 38)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("handshake hello message block too short"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct {
		ProtocolVersion server_version;
		Random random;
		SessionID session_id;
		CipherSuite cipher_suite;
		CompressionMethod compression_method;
	} ServerHello;
	*/

	/*
	ProtocolVersion
	*/
	pdtls->srv_major_ver = GET_BYTE(prec->rcv_msg, msglen);
	pdtls->srv_minor_ver = GET_BYTE(prec->rcv_msg, msglen + 1);
	msglen += 2;

	if (pdtls->srv_major_ver != pdtls->cli_major_ver)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("handshake major version mistach"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (pdtls->srv_minor_ver > SSL_MINOR_VERSION_0)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("handshake minor version mistach"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	pses->major_ver = pdtls->cli_major_ver;
	pses->minor_ver = (pdtls->cli_minor_ver < pdtls->srv_minor_ver) ? pdtls->cli_minor_ver : pdtls->srv_minor_ver;

	/*
	Random
	*/
	t = GET_DWORD_NET(prec->rcv_msg, msglen);
	xmem_copy(pcip->rnd_srv, prec->rcv_msg + msglen, DTLS_RND_SIZE);
	msglen += DTLS_RND_SIZE;

	/*
	SessionID
	*/
	seslen = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	/*
	CipherSuite
	*/
	ciph = GET_SWORD_NET(prec->rcv_msg, msglen + seslen);

	if (pses->session_resumed == 0 || pcip->cipher != ciph || a_xslen(pses->ses_id) != seslen || xmem_comp(pses->ses_id, prec->rcv_msg + msglen, seslen) != 0)
	{
		pses->session_resumed = 0;
		xmem_copy(pses->ses_id, prec->rcv_msg + msglen, seslen);
		pses->ses_size = seslen;

		if (!_dtls_choose_cipher((dtls10_cipher_context*)pcip, ciph))
		{
			set_last_error(_T("_dtls_parse_server_hello"), _T("unknown cipher type"), -1);
			return SSL_HANDSHAKE_ERROR;
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
	if (msglen == haslen + DTLS_HSH_SIZE)
		return SSL_SERVER_CERTIFICATE;
	
	//extension length
	extlen = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	//skip extension
	msglen += extlen;

	if (haslen + DTLS_HSH_SIZE != msglen)
	{
		set_last_error(_T("_dtls_parse_server_hello"), _T("invalid server hello message session block"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	return SSL_SERVER_CERTIFICATE;
}

static dtls10_handshake_states _dtls_parse_server_certificate(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int  ret, n;
	int msglen, haslen, crtlen;
	int num, off, len;

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE)
	{
		if (C_OK != _dtls_read_rcv_msg(pdtls))
		{
			return SSL_HANDSHAKE_ERROR;
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Certificate;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CERTIFICATE)
	{
		set_last_error(_T("_dtls_parse_server_certificate"), _T("invalid certificate message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	opaque ASN.1Cert<1..2^24-1>;

	struct {
	ASN.1Cert certificate_list<0..2^24-1>;
	} Certificate;
	*/

	crtlen = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	if (haslen != 11 + crtlen)
	{
		set_last_error(_T("_dtls_parse_server_certificate"), _T("invalid certificate block size"), -1);
		return SSL_HANDSHAKE_ERROR;
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
			set_last_error(_T("_dtls_parse_server_certificate"), _T("invalid certificate size"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (C_OK != x509_crt_parse(psec->peer_crt, prec->rcv_msg + msglen, n))
		{
			set_last_error(_T("_dtls_parse_server_certificate"), _T("invalid certificate context"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		msglen += n;
		crtlen -= n;
	}

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	if (psec->verify_mode != SSL_VERIFY_NONE)
	{
		if (psec->chain_ca == NULL)
		{
			set_last_error(_T("_dtls_parse_server_certificate"), _T("CA chian empty"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (psec->verify_mode == SSL_VERIFY_REQUIRED)
		{
			if (C_OK != x509_crt_verify(psec->peer_crt, psec->chain_ca, NULL, psec->peer_cn, &ret, NULL, NULL))
			{
				set_last_error(_T("_dtls_parse_server_certificate"), _T("certificate verify failed"), -1);
				return SSL_HANDSHAKE_ERROR;
			}
		}
	}

	return (IS_DHE_CIPHER(pcip->cipher)) ? SSL_SERVER_KEY_EXCHANGE : SSL_CERTIFICATE_REQUEST;
}

static int _dtls_parse_server_key_exchange(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	byte_t *p, *end;
	byte_t hash[36];
	md5_context md5;
	sha1_context sha1;
	int n, haslen, msglen;
	int num, off, len;

	if (prec->rcv_msg[0] != SSL_HS_SERVER_KEY_EXCHANGE)
	{
		if (C_OK != _dtls_read_rcv_msg(pdtls))
		{
			return SSL_HANDSHAKE_ERROR;
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ServerKeyExchange;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_SERVER_KEY_EXCHANGE)
	{
		set_last_error(_T("0"), _T("invalid server key exchange message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct {
	select (KeyExchangeAlgorithm) {
	case diffie_hellman:
	ServerDHParams params;
	Signature signed_params;
	case rsa:
	ServerRSAParams params;
	Signature signed_params;
	};
	} ServerKeyExchange;
	*/

	p = prec->rcv_msg + msglen;
	end = prec->rcv_msg + DTLS_HSH_SIZE + haslen;

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
			set_last_error(_T("_dtls_parse_server_key_exchange"), _T("server key exchange read dhm params error"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (psec->dhm_ctx->len < 64 || psec->dhm_ctx->len > 256)
		{
			set_last_error(_T("_dtls_parse_server_key_exchange"), _T("invalid server key exchange message context length"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
	}
	else
	{
		/*
		struct {
		opaque rsa_modulus<1..2^16-1>;
		opaque rsa_exponent<1..2^16-1>;
		} ServerRSAParams;
		*/
		if (!psec->rsa_ctx)
		{
			psec->rsa_ctx = (rsa_context*)xmem_alloc(sizeof(rsa_context));
			rsa_init(psec->rsa_ctx, 0, 0);
		}

		if (C_OK != rsa_import_pubkey(psec->rsa_ctx, &p, end, 1))
		{
			set_last_error(_T("_dtls_parse_server_key_exchange"), _T("server key exchange read rsa params error"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (psec->rsa_ctx->len > 1024)
		{
			set_last_error(_T("_dtls_parse_server_key_exchange"), _T("invalid server key exchange message context length"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
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

	n = GET_SWORD_NET(p, 0);
	p += 2;

	if ((int)(end - p) != psec->peer_crt->rsa->len)
	{
		set_last_error(_T("_dtls_parse_server_key_exchange"), _T("invalid server key exchange message context rsa key"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	n = (haslen - DTLS_MSH_SIZE) - (end - p) - 2;

	md5_starts(&md5);
	md5_update(&md5, pcip->rnd_cli, DTLS_RND_SIZE);
	md5_update(&md5, pcip->rnd_srv, DTLS_RND_SIZE);
	md5_update(&md5, prec->rcv_msg + msglen, n);
	md5_finish(&md5, hash);

	sha1_starts(&sha1);
	sha1_update(&sha1, pcip->rnd_cli, DTLS_RND_SIZE);
	sha1_update(&sha1, pcip->rnd_srv, DTLS_RND_SIZE);
	sha1_update(&sha1, prec->rcv_msg + msglen, n);
	sha1_finish(&sha1, hash + 16);

	if (C_OK != rsa_pkcs1_verify(psec->peer_crt->rsa, pdtls->f_rng, pdtls->r_rng, RSA_PUBLIC, RSA_HASH_NONE, 36, hash, p))
	{
		set_last_error(_T("_dtls_parse_server_key_exchange"), _T("invalid server key exchange message context verify"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	msglen += (2 + n + psec->peer_crt->rsa->len);

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	return SSL_CERTIFICATE_REQUEST;
}

static int _dtls_parse_server_certificate_request(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int haslen, msglen;
	int n, crttype, dsnlen;
	int num, off, len;

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE_REQUEST && prec->rcv_msg[0] != SSL_HS_SERVER_HELLO_DONE)
	{
		if (C_OK != _dtls_read_rcv_msg(pdtls))
			return SSL_HANDSHAKE_ERROR;
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	CertificateRequest;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE)
	{
		set_last_error(_T("_dtls_parse_server_certificate_request"), _T("invalid certificate request message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	pses->authen_client = (prec->rcv_msg[0] == SSL_HS_CERTIFICATE_REQUEST) ? 1 : 0;
	if (pses->authen_client == 0)
	{
		return SSL_SERVER_HELLO_DONE;
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	opaque DistinguishedName<1..2^16-1>;

	struct {
	ClientCertificateType certificate_types<1..2^8-1>;
	DistinguishedName certificate_authorities<3..2^16-1>;
	} CertificateRequest;
	*/

	//certificate_types count
	n = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	//certificate_types
	while (n--)
	{
		crttype = GET_BYTE(prec->rcv_msg, msglen);
		msglen++;
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
			set_last_error(_T("_dtls_parse_server_certificate_request"), _T("invalid certificate size"), -1);
			return SSL_HANDSHAKE_ERROR;
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

	return SSL_SERVER_HELLO_DONE;
}

static int _dtls_parse_server_hello_done(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen;
	int num, off, len;

	if (prec->rcv_msg[0] != SSL_HS_SERVER_HELLO_DONE)
	{
		if (C_OK != _dtls_read_rcv_msg(pdtls))
		{
			return SSL_HANDSHAKE_ERROR;
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ServerHelloDone;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_SERVER_HELLO_DONE)
	{
		set_last_error(_T("_dtls_parse_server_hello_done"), _T("invalid server hello message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct { } ServerHelloDone;
	*/

	return (pses->authen_client) ? SSL_CLIENT_CERTIFICATE : SSL_CLIENT_KEY_EXCHANGE;
}

static dtls10_handshake_states _dtls_write_client_certificate(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int n, lenpos, crtlen, msglen;
	x509_crt *crt;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Certificate;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_CERTIFICATE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

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
		if (msglen + 3 + n > DTLS_PKG_SIZE)
		{
			set_last_error(_T("_dtls_write_client_certificate"), _T("message package overwrited"), -1);
			return SSL_HANDSHAKE_ERROR;
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
	
	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	return (C_OK == _dtls_write_snd_msg(pdtls)) ? SSL_CLIENT_KEY_EXCHANGE : SSL_HANDSHAKE_ERROR;
}

static int _dtls_write_client_key_exchange(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int pos, n, msglen;
	byte_t premaster[DTLS_BLK_SIZE] = {0};
	int prelen = DTLS_MST_SIZE;
	size_t m;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ClientKeyExchange;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_CLIENT_KEY_EXCHANGE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct {
		select (KeyExchangeAlgorithm) {
		case rsa: EncryptedPreMasterSecret;
		case diffie_hellman: ClientDiffieHellmanPublic;
		} exchange_keys;
	} ClientKeyExchange;
	*/

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
		if (C_OK != dhm_make_public(psec->dhm_ctx, (int)m, prec->snd_msg + msglen, n, pdtls->f_rng, pdtls->r_rng))
		{
			set_last_error(_T("_dtls_write_client_key_exchange"), _T("make public dhm error"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
		msglen += n;

		//if n changed, reset key len
		PUT_SWORD_NET(prec->snd_msg, pos, (unsigned short)n);

		prelen = psec->dhm_ctx->len;

		if (C_OK != dhm_calc_secret(psec->dhm_ctx, premaster, prelen, &prelen, pdtls->f_rng, pdtls->r_rng))
		{
			set_last_error(_T("_dtls_write_client_key_exchange"), _T("cacl dhm secret error"), -1);
			return SSL_HANDSHAKE_ERROR;
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
		prelen = DTLS_MST_SIZE;

		//for (n = 2; n < prelen; n++)
		//	premaster[n] = (byte_t)havege_rand(&pcip->rng);
		(*pdtls->f_rng)(pdtls->r_rng, (premaster + 2), (prelen - 2));

		n = psec->peer_crt->rsa->len;

		PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)n);
		msglen += 2;

		if (C_OK != rsa_pkcs1_encrypt(psec->peer_crt->rsa, pdtls->f_rng, pdtls->r_rng, RSA_PUBLIC, prelen, premaster, prec->snd_msg + msglen))
		{
			set_last_error(_T("_dtls_write_client_key_exchange"), _T("rsa encrypt pre master secret error"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
		msglen += n;
	}

	_dtls_derive_keys((dtls10_cipher_context*)pcip, premaster, prelen);

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _dtls_write_snd_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	return (pses->authen_client) ? SSL_CERTIFICATE_VERIFY : SSL_CLIENT_CHANGE_CIPHER_SPEC;
}

static int _dtls_write_client_certificate_verify(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int n, msglen;
	byte_t hash[36];
	md5_context md5;
	sha1_context sha1;

	xmem_copy(&md5, &pcip->md5, sizeof(md5_context));
	xmem_copy(&sha1, &pcip->sha1, sizeof(sha1_context));

	md5_finish(&md5, hash);
	sha1_finish(&sha1, hash + 16);

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	CertificateVerify;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_CERTIFICATE_VERIFY;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct {
	Signature signature;
	} CertificateVerify;
	*/

	if (psec->rsa_ctx == NULL)
	{
		set_last_error(_T("_dtls_write_client_certificate_verify"), _T("no private key"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	n = psec->rsa_ctx->len;
	PUT_SWORD_NET(prec->snd_msg, msglen, (unsigned short)n);
	msglen += 2;

	if (C_OK != rsa_pkcs1_sign(psec->rsa_ctx, pdtls->f_rng, pdtls->r_rng, RSA_PRIVATE, RSA_HASH_NONE, 36, hash, prec->snd_msg + msglen))
	{
		set_last_error(_T("_dtls_write_client_certificate_verify"), _T("rsa signature hash error"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	msglen += n;

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	return (C_OK == _dtls_write_snd_msg(pdtls)) ? SSL_CLIENT_CHANGE_CIPHER_SPEC : SSL_HANDSHAKE_ERROR;
}

static dtls10_handshake_states _dtls_write_client_change_cipher_spec(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int i;
	/*
	struct {
		enum { change_cipher_spec(1), (255) } type;
	} ChangeCipherSpec;
	*/

	prec->snd_msg_type = SSL_MSG_CHANGE_CIPHER_SPEC;
	prec->snd_msg_len = 1;
	prec->snd_msg[0] = 1;

	//clear send message control bits
	for (i = DTLS_CTR_SIZE - 1; i >= 0; i--)
	{
		prec->snd_ctr[i] = 0;
	}

	if (C_OK != _dtls_write_snd_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	//after send change cipher all record must be crypted sending
	prec->crypted = 1;

	return SSL_CLIENT_FINISHED;
}

static dtls10_handshake_states _dtls_write_client_finished(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen;
	md5_context  md5;
	sha1_context sha1;
	byte_t padbuf[48] = {0};
	byte_t* mac_buf;

	xmem_copy(&md5, &pcip->md5, sizeof(md5_context));
	xmem_copy(&sha1, &pcip->sha1, sizeof(sha1_context));

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Finished;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_FINISHED;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct {
	opaque verify_data[12];
	} Finished;
	*/

	mac_buf = prec->snd_msg + msglen;

	md5_finish(&md5, padbuf); //16 bytes
	sha1_finish(&sha1, padbuf + 16); //20 bytes

	ssl_prf1(pcip->master_secret, DTLS_MST_SIZE, label_client_finished, padbuf, 36, mac_buf, 12);

	msglen += 12;

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _dtls_write_snd_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	return (pses->session_resumed) ? SSL_HANDSHAKE_OVER : SSL_SERVER_CHANGE_CIPHER_SPEC;
}

static dtls10_handshake_states _dtls_parse_server_change_cipher_spec(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int i;
	/*
	struct {
	enum { change_cipher_spec(1), (255) } type;
	} ChangeCipherSpec;
	*/

	if (C_OK != _dtls_read_rcv_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	if (prec->rcv_msg_type != SSL_MSG_CHANGE_CIPHER_SPEC)
	{
		set_last_error(_T("_dtls_parse_server_change_cipher_spec"), _T("invalid change cipher spec message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (prec->rcv_msg_len != 1 || prec->rcv_msg[0] != 1)
	{
		set_last_error(_T("_dtls_parse_server_change_cipher_spec"), _T("invalid change cipher spec message context"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	//clear recv message control bits
	for (i = DTLS_CTR_SIZE - 1; i >= 0; i--)
	{
		prec->rcv_ctr[i] = 0;
	}

	//after recv change cipher all record must be crypted recving
	prec->crypted = 1;

	return SSL_SERVER_FINISHED;
}

static dtls10_handshake_states _dtls_parse_server_finished(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	md5_context  md5;
	sha1_context sha1;
	byte_t padbuf[48] = { 0 };
	byte_t mac_buf[36] = { 0 };
	int hash_len, msglen;
	int num, off, len;

	xmem_copy(&md5, &pcip->md5, sizeof(md5_context));
	xmem_copy(&sha1, &pcip->sha1, sizeof(sha1_context));

	md5_finish(&md5, padbuf);
	sha1_finish(&sha1, padbuf + 16);

	ssl_prf1(pcip->master_secret, DTLS_MST_SIZE, label_server_finished, padbuf, 36, mac_buf, 12);
	hash_len = 12;

	if (C_OK != _dtls_read_rcv_msg(pdtls))
		return SSL_HANDSHAKE_ERROR;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Finished;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_FINISHED)
	{
		set_last_error(_T("_dtls_parse_server_finished"), _T("invalid finished message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct {
	opaque verify_data[12];
	} Finished;
	*/
	if (xmem_comp(prec->rcv_msg + msglen, mac_buf, hash_len) != 0)
	{
		set_last_error(_T("_dtls_parse_server_finished"), _T("invalid finished message hash"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	return (pses->session_resumed) ? SSL_CLIENT_CHANGE_CIPHER_SPEC : SSL_HANDSHAKE_OVER;
}

bool_t dtls10_handshake_client(dtls_context *pdtls)
{
	dtls_session_context* pses;
	dtls10_handshake_states state = SSL_HELLO_REQUEST;
	int TRY_MAX = DTLS_TRY_MAX;

	/* Flight 1 with none cookie
	*  ==>   ClientHello
	*  <==   ServerHelloVerifyRequest
	/* Flight 2 with cookie
	*  ==>   ClientHello
	*  <==   ServerHello
	*  <==   Certificate
	*  <==   ( ServerKeyExchange  )
	*  <==   ( CertificateRequest )
	*  <==   ServerHelloDone
	/* Flight 3
	*  ==>  ( Certificate/Alert  )
	*  ==>   ClientKeyExchange
	*  ==>   ( CertificateVerify  )
	*  ==>   ChangeCipherSpec
	*  ==>   Finished
	*  <==   ChangeCipherSpec
	*  <==   Finished
	*/

	while (state != SSL_HANDSHAKE_OVER)
	{
		switch (state)
		{
		case SSL_HELLO_REQUEST:
			_dtls_init_context(pdtls);

			state = SSL_CLIENT_HELLO;
			break;
		case SSL_CLIENT_HELLO:
			//Flight 1, 2 begain
			state = _dtls_write_client_hello(pdtls);
			break;
		case SSL_SERVER_HELLO_VERIFY_REQUEST:
			//Flight 1 end
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_server_hello_verify_request(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				thread_yield();

				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			_dtls_clear_flight(pdtls);
			break;
		case SSL_SERVER_HELLO:
			//Flight 2 continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_server_hello(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				thread_yield();

				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_SERVER_CERTIFICATE:
			//Flight 2 continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_server_certificate(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				thread_yield();

				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_SERVER_KEY_EXCHANGE:
			//Flight 2 continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_server_key_exchange(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				thread_yield();

				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_CERTIFICATE_REQUEST:
			//Flight 2 continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_server_certificate_request(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				thread_yield();

				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_SERVER_HELLO_DONE:
			//Flight 2 end
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_server_hello_done(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				thread_yield();

				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			_dtls_clear_flight(pdtls);
			break;
		case SSL_CLIENT_CERTIFICATE:
			//Flight 3 begin
			state = _dtls_write_client_certificate(pdtls);
			break;
		case SSL_CLIENT_KEY_EXCHANGE:
			//Flight 3 continue
			state = _dtls_write_client_key_exchange(pdtls);
			break;
		case SSL_CERTIFICATE_VERIFY:
			//Flight 3 continue
			state = _dtls_write_client_certificate_verify(pdtls);
			break;
		case SSL_CLIENT_CHANGE_CIPHER_SPEC:
			//Flight 3 continue
			state = _dtls_write_client_change_cipher_spec(pdtls);
			break;
		case SSL_CLIENT_FINISHED:
			//Flight 3 continue
			state = _dtls_write_client_finished(pdtls);
			break;
		case SSL_SERVER_CHANGE_CIPHER_SPEC:
			//Flight 3 continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_server_change_cipher_spec(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				thread_yield();

				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_SERVER_FINISHED:
			//Flight 3 end
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_server_finished(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				thread_yield();

				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			_dtls_clear_flight(pdtls);
			break;
		}
		
		if (state == SSL_HANDSHAKE_ERROR)
			break;
	}

	pses = (dtls_session_context*)pdtls->session_context;
	pses->handshake_over = (state == SSL_HANDSHAKE_OVER) ? 1 : -1;

	return (pses->handshake_over == 1)? 1 : 0;
}

/***************************************server routing************************************************************/

static dtls10_handshake_states _dtls_parse_client_hello(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen, haslen, ciphlen, complen, comped, extlen;
	int i, j, n;
	int ciph;
	byte_t* ciph_buf;
	int num, off, len;

	byte_t cookie[32] = { 0 };

	if (C_OK != _dtls_read_rcv_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ClientHello;
	} Handshake;
	*/

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CLIENT_HELLO)
	{
		set_last_error(_T("_dtls_parse_client_hello"), _T("invalid client hello message"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct {
	ProtocolVersion client_version;
	Random random;
	SessionID session_id;
	opaque cookie<0..32>;                             // New field
	CipherSuite cipher_suites<2..2^16-1>;
	CompressionMethod compression_methods<1..2^8-1>;
	} ClientHello;
	*/

	/*
	ProtocolVersion
	*/
	pdtls->cli_major_ver = prec->rcv_msg[msglen];
	pdtls->cli_minor_ver = prec->rcv_msg[msglen + 1];
	msglen += 2;

	if (pdtls->srv_major_ver != pdtls->cli_major_ver)
	{
		set_last_error(_T("_dtls_parse_client_hello"), _T("handshake major version mistech"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (pdtls->cli_minor_ver < SSL_MINOR_VERSION_1)
	{
		set_last_error(_T("_dtls_parse_client_hello"), _T("handshake minor version unsupported"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	pses->major_ver = pdtls->cli_major_ver;
	pses->minor_ver = (pdtls->srv_minor_ver < pdtls->cli_minor_ver) ? pdtls->srv_minor_ver : pdtls->cli_minor_ver;

	/*
	Random
	*/
	xmem_copy(pcip->rnd_cli, prec->rcv_msg + msglen, DTLS_RND_SIZE);
	msglen += DTLS_RND_SIZE;

	/*
	SessionID
	*/
	pses->ses_size = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	if (pses->ses_size < 0 || pses->ses_size > 32)
	{
		set_last_error(_T("_dtls_parse_client_hello"), _T("invalid client hello session id length"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	xmem_copy(pses->ses_id, prec->rcv_msg + msglen, pses->ses_size);
	msglen += pses->ses_size;

	/*
	cookie
	*/
	pses->cookie_size = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	if (pses->cookie_size < 0 || pses->cookie_size > 32)
	{
		set_last_error(_T("_dtls_parse_client_hello"), _T("invalid client hello cookie length"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (pses->cookie_size)
	{
		xmem_copy((void*)cookie, (void*)(prec->rcv_msg + msglen), pses->cookie_size);
		msglen += pses->cookie_size;

		if (xmem_comp((void*)(cookie), (void*)pses->cookie_id, pses->cookie_size) != 0)
		{
			set_last_error(_T("_dtls_parse_client_hello"), _T("invalid client hello cookie"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
	}

	/*
	CipherSuite
	*/
	ciphlen = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (ciphlen < 2 || ciphlen > 256 || (ciphlen % 2) != 0)
	{
		set_last_error(_T("_dtls_parse_client_hello"), _T("invalid client hello session cipher list length"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	ciph_buf = prec->rcv_msg + msglen;
	ciph = 0;
	n = sizeof(server_ciphers) / sizeof(dtls10_ciphers_set);
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

	if (!_dtls_choose_cipher((dtls10_cipher_context*)pcip, ciph))
	{
		set_last_error(_T("_dtls_parse_client_hello"), _T("unknown cipher type"), -1);
		return SSL_HANDSHAKE_ERROR;
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
		set_last_error(_T("_dtls_parse_client_hello"), _T("invalid client hello session compress length"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	//has no extension
	if (msglen > haslen + DTLS_HSH_SIZE)
	{
		//extension length
		extlen = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;

		//skip extension
		msglen += extlen;

		if (haslen + DTLS_HSH_SIZE != msglen)
		{
			set_last_error(_T("_dtls_parse_server_hello"), _T("invalid server hello message session block"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
	}

	return (pses->cookie_size) ? SSL_SERVER_HELLO : SSL_SERVER_HELLO_VERIFY_REQUEST;
}

static dtls10_handshake_states _dtls_write_server_hello_verify_request(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen;

	//generate cookie
	pses->cookie_size = 32;
	/*for (i = 0; i < pses->cookie_size; i++)
	{
		pses->cookie_id[i] = (byte_t)havege_rand(&pcip->rng);
	}*/
	(*pdtls->f_rng)(pdtls->r_rng, pses->cookie_id, pses->cookie_size);

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ServerHello;
	} Handshake;
	*/
	//handshake type
	PUT_BYTE(prec->snd_msg, 0, (byte_t)SSL_HS_HELLO_VERIFY_REQUEST);
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct {
        ProtocolVersion server_version;
        opaque cookie<0..32>;
      } HelloVerifyRequest;
	*/

	/*
	ProtocolVersion
	*/
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pses->major_ver));
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pses->minor_ver));

	/*
	cookie
	*/
	PUT_BYTE(prec->snd_msg, msglen, (byte_t)(pses->cookie_size));
	msglen++;
	xmem_copy((void*)(prec->snd_msg + msglen), (void*)pses->cookie_id, pses->cookie_size);
	msglen += pses->cookie_size;

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	return (C_OK == _dtls_write_snd_msg(pdtls)) ? SSL_CLIENT_HELLO : SSL_HANDSHAKE_ERROR;
}

static dtls10_handshake_states _dtls_write_server_hello(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen;
	dword_t t;

	//gen server random bits
	t = get_times();
	PUT_DWORD_NET(pcip->rnd_srv, 0, t);

	/*for (i = 4; i < DTLS_RND_SIZE; i++)
	{
		pcip->rnd_srv[i] = (byte_t)havege_rand(&pcip->rng);
	}*/
	(*pdtls->f_rng)(pdtls->r_rng, (pcip->rnd_srv + 4), (DTLS_RND_SIZE - 4));

	//gen server session id
	pses->ses_size = 32;
	/*for (i = 0; i < pses->ses_size; i++)
	{
		pses->ses_id[i] = (byte_t)havege_rand(&pcip->rng);
	}*/
	(*pdtls->f_rng)(pdtls->r_rng, pses->ses_id, pses->ses_size);

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ServerHello;
	} Handshake;
	*/
	//handshake type
	PUT_BYTE(prec->snd_msg, 0, (byte_t)SSL_HS_SERVER_HELLO);
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct {
	ProtocolVersion server_version;
	Random random;
	SessionID session_id;
	CipherSuite cipher_suite;
	CompressionMethod compression_method;
	} ServerHello;
	*/

	/*
	ProtocolVersion
	*/
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pses->major_ver));
	PUT_BYTE(prec->snd_msg, msglen++, (byte_t)(pses->minor_ver));

	/*
	Random
	*/
	xmem_copy(prec->snd_msg + msglen, pcip->rnd_srv, DTLS_RND_SIZE);
	msglen += DTLS_RND_SIZE;

	/*
	SessionID
	*/
	PUT_BYTE(prec->snd_msg, msglen, (byte_t)(pses->ses_size));
	msglen++;
	xmem_copy(prec->snd_msg + msglen, pses->ses_id, pses->ses_size);
	msglen += pses->ses_size;

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

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	return (C_OK == _dtls_write_snd_msg(pdtls)) ? SSL_SERVER_CERTIFICATE : SSL_HANDSHAKE_ERROR;
}

static dtls10_handshake_states _dtls_write_server_certificate(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int n, lenpos, crtlen, msglen;
	x509_crt *crt;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Certificate;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_CERTIFICATE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	opaque ASN.1Cert<1..2^24-1>;

	struct {
	ASN.1Cert certificate_list<0..2^24-1>;
	} Certificate;
	*/

	//preset certs length to zero
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	lenpos = msglen;
	msglen += 3;

	if (psec->host_crt == NULL)
	{
		set_last_error(_T("_dtls_write_server_certificate"), _T("empty server certificate"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	crtlen = 0;
	crt = psec->host_crt;
	while (crt != NULL && crt->version != 0)
	{
		n = crt->raw.len;
		if (msglen + 3 + n > DTLS_PKG_SIZE)
		{
			set_last_error(_T("_dtls_write_server_certificate"), _T("message package overwrited"), -1);
			return SSL_HANDSHAKE_ERROR;
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

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _dtls_write_snd_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	if (IS_DHE_CIPHER(pcip->cipher))
		return SSL_SERVER_KEY_EXCHANGE;
	else
		return (psec->verify_mode == SSL_VERIFY_NONE) ? SSL_SERVER_HELLO_DONE : SSL_CERTIFICATE_REQUEST;
}

static dtls10_handshake_states _dtls_write_server_key_exchange(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int n, msglen;
	size_t m;
	byte_t hash[36] = { 0 };
	md5_context md5;
	sha1_context sha1;
	mpi K;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Signature;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_SERVER_KEY_EXCHANGE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct {
	select (KeyExchangeAlgorithm) {
	case diffie_hellman:
	ServerDHParams params;
	Signature signed_params;
	case rsa:
	ServerRSAParams params;
	Signature signed_params;
	};
	} ServerKeyExchange;
	*/

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
		n = 0;
		m = mpi_size(&(psec->dhm_ctx->P)); //256
		if (C_OK != dhm_make_params(psec->dhm_ctx, (int)m, prec->snd_msg + msglen, &n, pdtls->f_rng, pdtls->r_rng))
		{
			set_last_error(_T("_dtls_write_server_key_exchange"), _T("make dhm params faild"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
		msglen += n;
	}
	else
	{
		/*
		struct {
		opaque rsa_modulus<1..2^16-1>;
		opaque rsa_exponent<1..2^16-1>;
		} ServerRSAParams;
		*/
		if (!psec->rsa_ctx)
		{
			psec->rsa_ctx = (rsa_context*)xmem_alloc(sizeof(rsa_context));

			rsa_init(psec->rsa_ctx, 0, 0);
			mpi_init(&K);

			mpi_read_string(&K, 16, rsa_N, sizeof(rsa_N) - 1);
			rsa_import(psec->rsa_ctx, &K, NULL, NULL, NULL, NULL);
			mpi_read_string(&K, 16, rsa_P, sizeof(rsa_P) - 1);
			rsa_import(psec->rsa_ctx, NULL, &K, NULL, NULL, NULL);
			mpi_read_string(&K, 16, rsa_Q, sizeof(rsa_Q) - 1);
			rsa_import(psec->rsa_ctx, NULL, NULL, &K, NULL, NULL);
			mpi_read_string(&K, 16, rsa_D, sizeof(rsa_D) - 1);
			rsa_import(psec->rsa_ctx, NULL, NULL, NULL, &K, NULL);
			mpi_read_string(&K, 16, rsa_E, sizeof(rsa_E) - 1);
			rsa_import(psec->rsa_ctx, NULL, NULL, NULL, NULL, &K);

			rsa_complete(psec->rsa_ctx);
			mpi_free(&K);

			n = 0;
			if (C_OK != rsa_export_pubkey(psec->rsa_ctx, prec->snd_msg + msglen, &n, 1))
			{
				set_last_error(_T("_dtls_write_server_key_exchange"), _T("make rsa params faild"), -1);
				return SSL_HANDSHAKE_ERROR;
			}
			msglen += n;
		}
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

	md5_starts(&md5);
	md5_update(&md5, pcip->rnd_cli, DTLS_RND_SIZE);
	md5_update(&md5, pcip->rnd_srv, DTLS_RND_SIZE);
	md5_update(&md5, prec->snd_msg + msglen - n, n);
	md5_finish(&md5, hash);

	sha1_starts(&sha1);
	sha1_update(&sha1, pcip->rnd_cli, DTLS_RND_SIZE);
	sha1_update(&sha1, pcip->rnd_srv, DTLS_RND_SIZE);
	sha1_update(&sha1, prec->snd_msg + msglen - n, n);
	sha1_finish(&sha1, hash + 16);

	PUT_SWORD_NET(prec->snd_msg, msglen, psec->rsa_ctx->len);
	msglen += 2;

	if (C_OK != rsa_pkcs1_sign(psec->rsa_ctx, pdtls->f_rng, pdtls->r_rng, RSA_PRIVATE, RSA_HASH_NONE, 36, hash, prec->snd_msg + msglen))
	{
		set_last_error(_T("_dtls_write_server_key_exchange"), _T("rsa pkcs1 sign failed"), -1);
		return SSL_HANDSHAKE_ERROR;
	}
	msglen += (psec->rsa_ctx->len);

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _dtls_write_snd_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	return (psec->verify_mode == SSL_VERIFY_NONE) ? SSL_SERVER_HELLO_DONE : SSL_CERTIFICATE_REQUEST;
}

static dtls10_handshake_states _dtls_write_server_certificate_request(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int n, lenpos, crtlen, msglen;
	x509_crt *crt;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	CertificateRequest;
	} Handshake;
	*/
	prec->snd_msg[0] = SSL_HS_CERTIFICATE_REQUEST;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	opaque DistinguishedName<1..2^16-1>;

	struct {
	ClientCertificateType certificate_types<1..2^8-1>;
	DistinguishedName certificate_authorities<3..2^16-1>;
	} CertificateRequest;
	*/

	//cert type count
	PUT_BYTE(prec->snd_msg, msglen, 1);
	msglen++;

	//cert type
	PUT_BYTE(prec->snd_msg, msglen, SSL_CERTIFICATE_TYPE_RSA);
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

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	return (C_OK == _dtls_write_snd_msg(pdtls)) ? SSL_SERVER_HELLO_DONE : SSL_HANDSHAKE_ERROR;
}

static dtls10_handshake_states _dtls_write_server_hello_done(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ServerHelloDone;
	} Handshake;
	*/
	prec->snd_msg[0] = SSL_HS_SERVER_HELLO_DONE;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct { } ServerHelloDone;
	*/

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _dtls_write_snd_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	return (psec->verify_mode == SSL_VERIFY_NONE) ? SSL_CLIENT_KEY_EXCHANGE : SSL_CLIENT_CERTIFICATE;
}

static dtls10_handshake_states _dtls_parse_client_certificate(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen, haslen, crtlen;
	int n, ret;
	int num, off, len;

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE && (prec->rcv_msg[0] != SSL_LEVEL_WARNING || prec->snd_msg[1] != SSL_ALERT_NO_CERTIFICATE))
	{
		if (C_OK != _dtls_read_rcv_msg(pdtls))
		{
			return SSL_HANDSHAKE_ERROR;
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Certificate;
	} Handshake;
	*/
	if (prec->rcv_msg_type == SSL_MSG_ALERT && prec->rcv_msg[0] == SSL_LEVEL_WARNING && prec->snd_msg[1] == SSL_ALERT_NO_CERTIFICATE)
	{
		if (psec->verify_mode == SSL_VERIFY_REQUIRED)
		{
			set_last_error(_T("_ssl_parse_client_certificate"), _T("client has no certificate"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
		else
		{
			pses->authen_client = 0;
			return SSL_CLIENT_KEY_EXCHANGE;
		}
	}

	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CERTIFICATE)
	{
		set_last_error(_T("_dtls_parse_client_certificate"), _T("invalid certificate message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	opaque ASN.1Cert<1..2^24-1>;

	struct {
	ASN.1Cert certificate_list<0..2^24-1>;
	} Certificate;
	*/

	crtlen = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	if (haslen != 3 + crtlen)
	{
		set_last_error(_T("_dtls_parse_client_certificate"), _T("invalid certificate block size"), -1);
		return SSL_HANDSHAKE_ERROR;
	}
	//empty certificate
	if (!crtlen)
	{
		if (psec->verify_mode == SSL_VERIFY_REQUIRED)
		{
			set_last_error(_T("_ssl_parse_client_certificate"), _T("client has no certificate"), -1);
			return SSL_HANDSHAKE_ERROR;
		}
		else
		{
			pses->authen_client = 0;
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
			set_last_error(_T("_dtls_parse_client_certificate"), _T("invalid certificate size"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (C_OK != x509_crt_parse(psec->peer_crt, prec->rcv_msg + msglen, n))
		{
			set_last_error(_T("_dtls_parse_client_certificate"), _T("invalid certificate context"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		msglen += n;
		crtlen -= n;
	}

	if (psec->verify_mode != SSL_VERIFY_NONE)
	{
		if (psec->chain_ca == NULL)
		{
			set_last_error(_T("_dtls_parse_client_certificate"), _T("CA chian empty"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (C_OK != x509_crt_verify(psec->peer_crt, psec->chain_ca, NULL, psec->peer_cn, &ret, NULL, NULL))
		{
			if (psec->verify_mode == SSL_VERIFY_REQUIRED)
			{
				set_last_error(_T("_dtls_parse_client_certificate"), _T("certificate verify failed"), -1);
				return SSL_HANDSHAKE_ERROR;
			}
		}
	}

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	//need client certificate verify
	pses->authen_client = 1;

	return SSL_CLIENT_KEY_EXCHANGE;
}

static dtls10_handshake_states _dtls_parse_client_key_exchange(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int haslen, n, msglen;
	byte_t premaster[DTLS_BLK_SIZE] = { 0 };
	int prelen = DTLS_MST_SIZE;
	int num, off, len;

	if (prec->rcv_msg[0] != SSL_HS_CLIENT_KEY_EXCHANGE)
	{
		if (C_OK != _dtls_read_rcv_msg(pdtls))
		{
			return SSL_HANDSHAKE_ERROR;
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	ClientKeyExchange;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CLIENT_KEY_EXCHANGE)
	{
		set_last_error(_T("_dtls_parse_client_key_exchange"), _T("invalid client key exchange message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct {
	select (KeyExchangeAlgorithm) {
	case rsa: EncryptedPreMasterSecret;
	case diffie_hellman: ClientDiffieHellmanPublic;
	} exchange_keys;
	} ClientKeyExchange;
	*/

	if (IS_DHE_CIPHER(pcip->cipher))
	{
		//Receive G^Y mod P, premaster = (G^Y)^X mod P
		n = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;

		if (n < 1 || n + 2 != (haslen - DTLS_MSH_SIZE))
		{
			set_last_error(_T("_dtls_parse_client_key_exchange"), _T("invalid client key exchange length"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (C_OK != dhm_read_public(psec->dhm_ctx, prec->rcv_msg + msglen, n))
		{
			set_last_error(_T("_dtls_parse_client_key_exchange"), _T("invalid client key exchange context"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		prelen = psec->dhm_ctx->len;

		if (C_OK != dhm_calc_secret(psec->dhm_ctx, premaster, prelen, &prelen, pdtls->f_rng, pdtls->r_rng))
		{
			set_last_error(_T("_dtls_parse_client_key_exchange"), _T("create premaster failed"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		msglen += n;
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
		prelen = DTLS_MST_SIZE;

		n = GET_SWORD_NET(prec->rcv_msg, msglen);
		msglen += 2;

		if (n != psec->rsa_ctx->len)
		{
			set_last_error(_T("_dtls_parse_client_key_exchange"), _T("invalid client key exchange key length"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (C_OK != rsa_pkcs1_decrypt(psec->rsa_ctx, pdtls->f_rng, pdtls->r_rng, RSA_PRIVATE, &prelen, prec->rcv_msg + msglen, premaster, prelen))
		{
			set_last_error(_T("_dtls_parse_client_key_exchange"), _T("decrypt client key exchange key failed"), -1);
			return SSL_HANDSHAKE_ERROR;
		}

		if (prelen != DTLS_MST_SIZE)// || premaster[0] != pses->major_ver || premaster[1] != pses->minor_ver)
		{
			/*
			* Protection against Bleichenbacher's attack:
			* invalid PKCS#1 v1.5 padding must not cause
			* the connection to end immediately; instead,
			* send a bad_record_mac later in the handshake.
			*/
			//for (i = 0; i < prelen; i++)
			//	premaster[i] = (byte_t)havege_rand(&pcip->rng);
			(*pdtls->f_rng)(pdtls->r_rng, premaster, prelen);
		}

		msglen += n;
	}

	_dtls_derive_keys((dtls10_cipher_context*)pcip, premaster, prelen);

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}

	return (pses->authen_client) ? SSL_CERTIFICATE_VERIFY : SSL_CLIENT_CHANGE_CIPHER_SPEC;
}

static dtls10_handshake_states _dtls_parse_client_certificate_verify(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen, haslen, n;
	md5_context md5;
	sha1_context sha1;
	byte_t hash[36];
	int num, off, len;

	XDK_ASSERT(psec->peer_crt != NULL);

	xmem_copy(&md5, &pcip->md5, sizeof(md5_context));
	xmem_copy(&sha1, &pcip->sha1, sizeof(sha1_context));

	md5_finish(&md5, hash);
	sha1_finish(&sha1, hash + 16);

	if (prec->rcv_msg[0] != SSL_HS_CERTIFICATE_VERIFY)
	{
		if (C_OK != _dtls_read_rcv_msg(pdtls))
		{
			return SSL_HANDSHAKE_ERROR;
		}
	}

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	CertificateVerify;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CERTIFICATE_VERIFY)
	{
		set_last_error(_T("_dtls_parse_client_certificate_verify"), _T("invalid certificate verify message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}
	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);
	haslen += DTLS_MSH_SIZE;
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct {
	Signature signature;
	} CertificateVerify;
	*/

	n = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;

	if (n != psec->peer_crt->rsa->len)
	{
		set_last_error(_T("_dtls_parse_client_certificate_verify"), _T("invalid certificate verify message length"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (C_OK != rsa_pkcs1_verify(psec->peer_crt->rsa, pdtls->f_rng, pdtls->r_rng, RSA_PUBLIC, RSA_HASH_NONE, 36, hash, prec->rcv_msg + msglen))
	{
		set_last_error(_T("_dtls_parse_client_certificate_verify"), _T("invalid certificate verify message context"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	msglen += n;

	//if multiply handshake message
	if (prec->rcv_msg_len > msglen)
	{
		xmem_move((prec->rcv_msg + msglen), (prec->rcv_msg_len - msglen), -msglen);
		prec->rcv_msg_len -= msglen;
	}
	
	return SSL_CLIENT_CHANGE_CIPHER_SPEC;
}

static dtls10_handshake_states _dtls_parse_client_change_cipher_spec(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int i;
	/*
	struct {
		enum { change_cipher_spec(1), (255) } type;
	} ChangeCipherSpec;
	*/

	if (C_OK != _dtls_read_rcv_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	if (prec->rcv_msg_type != SSL_MSG_CHANGE_CIPHER_SPEC)
	{
		set_last_error(_T("_dtls_parse_client_change_cipher_spec"), _T("invalid change cipher spec message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	if (prec->rcv_msg_len != 1 || prec->rcv_msg[0] != 1)
	{
		set_last_error(_T("_dtls_parse_client_change_cipher_spec"), _T("invalid change cipher spec message context"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	//clear recv message control bits
	for (i = DTLS_CTR_SIZE - 1; i >= 0; i--)
	{
		prec->rcv_ctr[i] = 0;
	}

	//after read change cipher all record must be crypted recving
	prec->crypted = 1;

	return SSL_CLIENT_FINISHED;
}

static dtls10_handshake_states _dtls_parse_client_finished(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int hash_len, msglen;
	md5_context  md5;
	sha1_context sha1;
	byte_t padbuf[48] = { 0 };
	byte_t mac_buf[36] = { 0 };
	int num, off, len;

	xmem_copy(&md5, &pcip->md5, sizeof(md5_context));
	xmem_copy(&sha1, &pcip->sha1, sizeof(sha1_context));

	/*
	PRF(master_secret, finished_label, MD5(handshake_messages) + SHA-1(handshake_messages)) [0..11];
	*/
	md5_finish(&md5, padbuf);
	sha1_finish(&sha1, padbuf + 16);
	ssl_prf1(pcip->master_secret, DTLS_MST_SIZE, label_client_finished, padbuf, 36, mac_buf, 12);

	hash_len = 12;

	if (C_OK != _dtls_read_rcv_msg(pdtls))
		return SSL_HANDSHAKE_ERROR;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Finished;
	} Handshake;
	*/
	if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_FINISHED)
	{
		set_last_error(_T("_dtls_parse_client_finished"), _T("invalid finished message type"), -1);
		return SSL_HANDSHAKE_ERROR;
	}
	msglen = DTLS_HSH_SIZE;

	//message_seq
	num = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	//fragment_offset
	off = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;
	//fragment_length
	len = GET_THREEBYTE_NET(prec->rcv_msg, msglen);
	msglen += 3;

	/*
	struct {
	opaque verify_data[12];
	} Finished;
	*/
	if (xmem_comp(prec->rcv_msg + msglen, mac_buf, hash_len) != 0)
	{
		set_last_error(_T("_dtls_parse_client_finished"), _T("invalid finished message hash"), -1);
		return SSL_HANDSHAKE_ERROR;
	}

	return (pses->session_resumed) ? SSL_HANDSHAKE_OVER : SSL_SERVER_CHANGE_CIPHER_SPEC;
}

static dtls10_handshake_states _dtls_write_server_change_cipher_spec(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int i;
	/*
	struct {
		enum { change_cipher_spec(1), (255) } type;
	} ChangeCipherSpec;
	*/

	prec->snd_msg_type = SSL_MSG_CHANGE_CIPHER_SPEC;
	prec->snd_msg_len = 1;
	prec->snd_msg[0] = 1;

	//clear send message control bits
	for (i = DTLS_CTR_SIZE - 1; i >= 0; i--)
	{
		prec->snd_ctr[i] = 0;
	}

	if (C_OK != _dtls_write_snd_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	//after write change cipher all record must be crypted sending
	prec->crypted = 1;

	return SSL_SERVER_FINISHED;
}

static dtls10_handshake_states _dtls_write_server_finished(dtls_context *pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);
	dtls10_cipher_context* pcip = (dtls10_cipher_context*)pses->cipher_context;

	int msglen;
	md5_context  md5;
	sha1_context sha1;
	byte_t padbuf[48] = { 0 };
	byte_t* mac_buf;

	/*
	struct {
	HandshakeType msg_type; 1 byte
	uint24 length; 3 bytes
	uint16 message_seq;                               // New field
	uint24 fragment_offset;                           // New field
	uint24 fragment_length;                           // New field
	Finished;
	} Handshake;
	*/
	//handshake type
	prec->snd_msg[0] = SSL_HS_FINISHED;
	//handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, 0);

	msglen = DTLS_HSH_SIZE;

	// message_seq
	PUT_SWORD_NET(prec->snd_msg, msglen, pses->snd_next_msgnum);
	msglen += 2;
	// fragment_offset
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;
	// preset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, msglen, 0);
	msglen += 3;

	/*
	struct {
	opaque verify_data[12];
	} Finished;
	*/
	xmem_copy(&md5, &pcip->md5, sizeof(md5_context));
	xmem_copy(&sha1, &pcip->sha1, sizeof(sha1_context));

	mac_buf = prec->snd_msg + msglen;

	/*
	PRF(master_secret, finished_label, MD5(handshake_messages) + SHA-1(handshake_messages)) [0..11];
	*/
	md5_finish(&md5, padbuf);
	sha1_finish(&sha1, padbuf + 16);
	ssl_prf1(pcip->master_secret, DTLS_MST_SIZE, label_server_finished, padbuf, 36, mac_buf, 12);

	msglen += 12;

	//reset handshake length
	PUT_THREEBYTE_NET(prec->snd_msg, 1, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));
	//reset fragment_length
	PUT_THREEBYTE_NET(prec->snd_msg, 9, (msglen - DTLS_HSH_SIZE - DTLS_MSH_SIZE));

	prec->snd_msg_len = msglen;
	prec->snd_msg_type = SSL_MSG_HANDSHAKE;

	if (C_OK != _dtls_write_snd_msg(pdtls))
	{
		return SSL_HANDSHAKE_ERROR;
	}

	return (pses->session_resumed) ? SSL_CLIENT_CHANGE_CIPHER_SPEC : SSL_HANDSHAKE_OVER;
}

bool_t dtls10_handshake_server(dtls_context *pdtls)
{
	dtls_session_context* pses;
	dtls10_handshake_states state = SSL_HELLO_REQUEST;
	int TRY_MAX = DTLS_TRY_MAX;

	/* Flight 1
	*  <==   ClientHello
	*  ==>   ServerHelloVerifyRequest
	*/
	/* Flight 2 begin
	*  <==   ClientHello
	*  ==>   ServerHello
	*  ==>   Certificate
	*  ==>   ( ServerKeyExchange  )
	*  ==>   ( CertificateRequest )
	*  ==>   ServerHelloDone
	*/
	/* Flight 3 begin
	*  <==	 ( Certificate/Alert  )
	*  <==   ClientKeyExchange
	*  <==   ( CertificateVerify  )
	*  <==   ChangeCipherSpec
	*  <==   Finished
	*  ==>   ChangeCipherSpec
	*  ==>   Finished
	*/

	while (state != SSL_HANDSHAKE_OVER)
	{
		switch (state)
		{
		case SSL_HELLO_REQUEST:
			_dtls_init_context(pdtls);

			state = SSL_CLIENT_HELLO;
			break;
		case SSL_CLIENT_HELLO:
			//Flight 1, 2 begin
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_client_hello(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			_dtls_clear_flight(pdtls);
			break;
		case SSL_SERVER_HELLO_VERIFY_REQUEST:
			//Flight 1 continue
			state = _dtls_write_server_hello_verify_request(pdtls);
			break;
		case SSL_SERVER_HELLO:
			//Flight 2 continue
			state = _dtls_write_server_hello(pdtls);
			break;
		case SSL_SERVER_CERTIFICATE:
			//Flight 2 continue
			state = _dtls_write_server_certificate(pdtls);
			break;
		case SSL_SERVER_KEY_EXCHANGE:
			//Flight 2 continue
			state = _dtls_write_server_key_exchange(pdtls);
			break;
		case SSL_CERTIFICATE_REQUEST:
			//Flight 2 continue
			state = _dtls_write_server_certificate_request(pdtls);
			break;
		case SSL_SERVER_HELLO_DONE:
			//Flight 2 continue
			state = _dtls_write_server_hello_done(pdtls);
			break;
		case SSL_CLIENT_CERTIFICATE:
			//Flight 3 begin
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_client_certificate(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_CLIENT_KEY_EXCHANGE:
			//Flight 3 begin or continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_client_key_exchange(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_CERTIFICATE_VERIFY:
			//Flight 3 continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_client_certificate_verify(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_CLIENT_CHANGE_CIPHER_SPEC:
			//Flight 3 continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_client_change_cipher_spec(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			break;
		case SSL_CLIENT_FINISHED:
			//Flight 3 continue
			TRY_MAX = DTLS_TRY_MAX;
			while (TRY_MAX && (state = _dtls_parse_client_finished(pdtls)) == SSL_HANDSHAKE_ERROR)
			{
				if (!_dtls_replay_flight(pdtls))
					TRY_MAX = 0;
				else
					TRY_MAX--;
			}
			_dtls_clear_flight(pdtls);
			break;
		case SSL_SERVER_CHANGE_CIPHER_SPEC:
			//Flight 3 continue
			state = _dtls_write_server_change_cipher_spec(pdtls);
			break;
		case SSL_SERVER_FINISHED:
			//Flight 3 continue
			state = _dtls_write_server_finished(pdtls);
			break;
		}

		if (state == SSL_HANDSHAKE_ERROR)
			break;
	}
	
	pses = (dtls_session_context*)pdtls->session_context;
	pses->handshake_over = (state == SSL_HANDSHAKE_OVER) ? 1 : -1;

	return (pses->handshake_over == 1)? 1 : 0;
}



#endif //XDK_SUPPORT_SOCK
