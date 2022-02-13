/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc ssl document

	@module	netssl.h | interface file

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

#ifndef _NETSSL_H
#define _NETSSL_H

#include "../xdkdef.h"
#include "../bioinf.h"
#include "../xdkoem.h"
#include "netdef.h"
#include "ssldef.h"

#if defined(XDK_SUPPORT_SOCK)


#define SSL_TYPE_CLIENT		0
#define SSL_TYPE_SERVER		1
#define SSL_TYPE_LISTEN		2

#define SSL_CTR_SIZE		8	//control byte size
#define SSL_HDR_SIZE		5	//message header size
#define SSL_HSH_SIZE		4	//handshake header size
#define SSL_RND_SIZE		32	//radom byte size
#define SSL_MST_SIZE		48	//master scret byte size
#define SSL_SES_SIZE		32	//session id size
#define SSL_CTX_SIZE		512	//encrypt/decrypt context size
#define SSL_BLK_SIZE		256	//key block size
#define SSL_FIN_SIZE		12 //finished hash size
#define SSL_MAX_MAC			64	//mac byte size(16, 20 , 32, 64)
#define SSL_MAX_IVC			16	//iv code size(8, 16)
#define SSL_COOKIE_SIZE		32	//session cookie size

#define SSL_PKG_SIZE		16384
#define SSL_MAX_SIZE		(SSL_PKG_SIZE + 512)

typedef struct _ssl_context*			ssl_context_ptr;
typedef struct _ssl_cipher_context*		ssl_cipher_context_ptr;
typedef struct _ssl_security_context*	ssl_security_context_ptr;
typedef struct _ssl_session_context*	ssl_session_context_ptr;
typedef struct _ssl_record_context*		ssl_record_context_ptr;

typedef struct _ssl_context{
	handle_head head;
	int type;

	int srv_major_ver;
	int srv_minor_ver;
	int cli_major_ver;
	int cli_minor_ver;

	bio_interface* pif;

	int(*ssl_send)(ssl_context_ptr pssl);
	int(*ssl_recv)(ssl_context_ptr pssl);

	int(*f_rng)(void *, byte_t *, dword_t);
	void* r_rng;

	ssl_security_context_ptr security_context;
	ssl_session_context_ptr session_context;
}ssl_context;

typedef struct _ssl_session_context{
	int major_ver;
	int minor_ver;

	//Session State
	int session_size; //session id size
	byte_t session_id[SSL_SES_SIZE]; //session id
	int cookie_size; //cookie size
	byte_t session_cookie[MD_MAX_SIZE]; //session cookie.

	int authen_client; //client to be authentic
	int session_resumed; //session is resumed. 0: none, 1: resumed.
	int handshake_over; //handshake step, 0 : undo, 1 : pass, -1 : close.

	void(*free_cipher_context)(ssl_session_context_ptr pses);

	ssl_cipher_context_ptr cipher_context;

	int msg_zero; //empty message count
	int pkg_size; //max package size

	int alert_code; //alert notify code

	ssl_record_context_ptr snd_record;
	ssl_record_context_ptr rcv_record;
}ssl_session_context;

typedef struct _ssl_security_context{
	int verify_mode; //certificate verify mode

	schar_t* host_cn;
	schar_t* peer_cn;

	x509_crt* chain_ca;
	x509_crt* host_crt;
	x509_crt* peer_crt;

	rsa_context* rsa_ctx;
	dhm_context* dhm_ctx;
	ecdh_context* ecdh_ctx;
}ssl_security_context;

typedef struct _ssl_record_context{
	int compressed; //record is compressed
	int crypted; //record is crypted sending
	//Record
	union{
		int snd_msg_type;
		int rcv_msg_type;
	};
	union{
		int snd_msg_len;
		int rcv_msg_len;
	};
	union{
		int snd_msg_pop;
		int rcv_msg_pop;
	};
	union{
		byte_t* snd_pkg;
		byte_t* rcv_pkg;
	};
	union{
		byte_t* snd_ctr; //counter
		byte_t* rcv_ctr; //counter
	};
	union{
		byte_t* snd_hdr;
		byte_t* rcv_hdr;
	};
	union{
		byte_t* snd_msg;
		byte_t* rcv_msg;
	};
}ssl_record_context;


#ifdef	__cplusplus
extern "C" {
#endif

LOC_API bool_t ssl30_handshake_server(ssl_context* pssl);
LOC_API bool_t ssl30_handshake_client(ssl_context* pssl);
LOC_API bool_t tls10_handshake_server(ssl_context* pssl);
LOC_API bool_t tls10_handshake_client(ssl_context* pssl);
LOC_API bool_t tls11_handshake_server(ssl_context* pssl);
LOC_API bool_t tls11_handshake_client(ssl_context* pssl);
LOC_API bool_t tls12_handshake_server(ssl_context* pssl);
LOC_API bool_t tls12_handshake_client(ssl_context* pssl);
LOC_API bool_t tls13_handshake_server(ssl_context* pssl);
LOC_API bool_t tls13_handshake_client(ssl_context* pssl);

/*
@FUNCTION xssl_cli: create a SSL client.
@INPUT unsigned short port: the network port to connect.
@INPUT const tchar_t* addr: the network address to connect.
@RETURN xhand_t: if succeeds return SSL client handle, fails return NULL.
*/
EXP_API xhand_t xssl_cli(unsigned short port, const tchar_t* addr);

/*
@FUNCTION xssl_srv: create a SSL server.
@INPUT res_file_t so: the network io resource handle, it must be a socket resource handle.
@RETURN xhand_t: if succeeds return SSL server handle, fails return NULL.
*/
EXP_API xhand_t xssl_srv(res_file_t so);

/*
@FUNCTION xssl_socket: get socket resource handle.
@INPUT xhand_t ssl: the SSL handle.
@RETURN res_file_t: return the socket resource handle.
*/
EXP_API res_file_t xssl_socket(xhand_t ssl);

/*
@FUNCTION xssl_type: get socket type, it can be _XSSL_TYPE_CLI, _XSSL_TYPE_SRV.
@INPUT xhand_t ssl: the SSL handle.
@RETURN int: return the socket type.
*/
EXP_API int  xssl_type(xhand_t ssl);

/*
@FUNCTION xssl_close: close SSL handle.
@INPUT xhand_t ssl: the SSL handle.
@RETURN void: none.
*/
EXP_API void  xssl_close(xhand_t ssl);

/*
@FUNCTION xssl_write: write SSL data.
@INPUT xhand_t ssl: the SSL handle.
@INPUT const byte_t* data: the data buffer.
@INOUTPUT dword_t* pb: indicate the bytes to write and return the bytes writed.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xssl_write(xhand_t ssl, const byte_t* data, dword_t* pb);

/*
@FUNCTION xssl_flush: ensure write SSL data compeleted.
@INPUT xhand_t ssl: the SSL handle.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xssl_flush(xhand_t ssl);

/*
@FUNCTION xssl_read: read SSL data.
@INPUT xhand_t ssl: the SSL handle.
@OUTPUT byte_t* data: the data buffer.
@INOUTPUT dword_t* pb: indicate the bytes to read and return the bytes readed.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xssl_read(xhand_t ssl, byte_t* data, dword_t* pb);

/*
@FUNCTION xssl_setopt: set the socket options.
@INPUT xhand_t ssl: the ssl handle.
@INPUT int oid: the option id, eg: SOCK_OPTION_SNDBUF, SOCK_OPTION_RCVBUF, SOCK_OPTION_NONBLK.
@INPUT void* opt: the option value pointer
@INPUT int len: the value length in bytes, string value must be a zero terminated token and set len to zero.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xssl_setopt(xhand_t ssl, int oid, void* opt, int len);

/*
@FUNCTION xssl_addr_port: get SSL local address and port.
@INPUT xhand_t ssl: the SSL handle.
@OUTPUT tchar_t* addr: the string buffer.
@RETURN unsigned short: return the local port.
*/
EXP_API unsigned short xssl_addr_port(xhand_t ssl, tchar_t* addr);

/*
@FUNCTION xssl_peer_port: get SSL remote address and port.
@INPUT xhand_t ssl: the SSL handle.
@OUTPUT tchar_t* addr: the string buffer.
@RETURN unsigned short: return the remote port.
*/
EXP_API unsigned short xssl_peer_port(xhand_t ssl, tchar_t* addr);

/*
@FUNCTION xssl_set_host: set SSL host name.
@INPUT xhand_t ssl: the SSL handle.
@INPUT const tchar_t* host_cn: the host name.
@RETURN void: none.
*/
EXP_API void xssl_set_host(xhand_t ssl, const tchar_t* host_cn);

/*
@FUNCTION xssl_set_peer: set SSL peer name.
@INPUT xhand_t ssl: the SSL handle.
@INPUT const tchar_t* host_cn: the peer name.
@RETURN void: none.
*/
EXP_API void xssl_set_peer(xhand_t ssl, const tchar_t* peer_cn);

/*
@FUNCTION xssl_set_ca: set SSL root certificate.
@INPUT xhand_t ssl: the SSL handle.
@INPUT const byte_t* sz_cert: the cerificate bytes buffer.
@INPUT dword_t clen: the cert data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xssl_set_ca(xhand_t ssl, const byte_t* sz_cert, dword_t clen);

/*
@FUNCTION xssl_set_cert: set SSL owner certificate.
@INPUT xhand_t ssl: the SSL handle.
@INPUT const byte_t* sz_cert: the cerificate bytes buffer.
@INPUT dword_t clen: the cert data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xssl_set_cert(xhand_t ssl, const byte_t* sz_cert, dword_t clen);

/*
@FUNCTION xssl_set_cert: set SSL owner certificate.
@INPUT xhand_t ssl: the SSL handle.
@INPUT const byte_t* sz_rsa: the rsa key bytes buffer.
@INPUT dword_t rlen: the rsa data size in bytes.
@INPUT const byte_t* sz_pwd: the password key bytes buffer.
@INPUT dword_t plen: the password data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xssl_set_rsa(xhand_t ssl, const byte_t* sz_rsa, dword_t rlen, const tchar_t* sz_pwd, int plen);

/*
@FUNCTION xssl_set_cert: set SSL owner certificate.
@INPUT xhand_t ssl: the SSL handle.
@INPUT const byte_t* sz_dhm: the dhm key bytes buffer.
@INPUT dword_t rlen: the dhm data size in bytes.
@INPUT const byte_t* sz_pwd: the password key bytes buffer.
@INPUT dword_t plen: the password data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xssl_set_dhm(xhand_t ssl, const byte_t* sz_dhm, dword_t rlen);

/*
@FUNCTION xssl_set_auth: set SSL authorization mode.
@INPUT xhand_t ssl: the SSL handle.
@INPUT int srv_verify: the certify verify mode, it can be SSL_VERIFY_NONE, SSL_VERIFY_OPTIONAL, SSL_VERIFY_REQUIRED.
@RETURN void: none.
*/
EXP_API void xssl_set_verify(xhand_t ssl, int srv_verify);

/*
@FUNCTION xssl_set_version: set SSL/TLS maximized version
@INPUT xhand_t ssl: the SSL handle.
@INPUT int cli_ver: the client maximized version it can be SSLv30, TLSv10, TLSv11, TLSv12.
@RETURN void: none.
*/
EXP_API void xssl_set_version(xhand_t ssl, int cli_ver);

#ifdef	__cplusplus
}
#endif

#endif /*XDK_SUPPORT_SOCK*/

#endif /*IMPSOCK_H*/