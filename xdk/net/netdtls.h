/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc dtls document

	@module	netdtls.h | interface file

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

#ifndef _NETDTLS_H
#define _NETDTLS_H

#include "../xdkdef.h"
#include "../bioinf.h"
#include "../xdkoem.h"
#include "netdef.h"
#include "ssldef.h"

#if defined(XDK_SUPPORT_SOCK)


#define DTLS_TYPE_CLIENT		0
#define DTLS_TYPE_SERVER		1
#define DTLS_TYPE_LISTEN		2

#define DTLS_CTR_SIZE		8	//control byte size
#define DTLS_HDR_SIZE		13	//message header size
#define DTLS_HSH_SIZE		4	//handshake header size
#define DTLS_MSH_SIZE		8	//handshake message header size
#define DTLS_RND_SIZE		32	//radom byte size
#define DTLS_MST_SIZE		48	//master scret byte size
#define DTLS_SES_SIZE		32	//session id size
#define DTLS_COOKIE_SIZE	32	//cooki id size
#define DTLS_CTX_SIZE		512	//encrypt/decrypt context size
#define DTLS_BLK_SIZE		256	//key block size
#define DTLS_FIN_SIZE		12 //finished hash size
#define DTLS_MAX_MAC		64	//mac byte size(16, 20 , 32, 64)
#define DTLS_MAX_IVC		16	//iv code size(8, 16)

#define DTLS_PKG_SIZE		MTU_MAX_SIZE
#define DTLS_MAX_SIZE		(16384 + 512)

#define DTLS_BASE_TIMO		(5000) //millionsecond
#define DTLS_MIN_PORT		(49152)
#define DTLS_MAX_PORT		(65535)

typedef struct _dtls_context*			dtls_context_ptr;
typedef struct _dtls_cipher_context*	dtls_cipher_context_ptr;
typedef struct _dtls_security_context*	dtls_security_context_ptr;
typedef struct _dtls_session_context*	dtls_session_context_ptr;
typedef struct _dtls_record_context*	dtls_record_context_ptr;

typedef int(*PF_DTLS_SEND)(dtls_context_ptr pdtls);
typedef int(*PF_DTLS_RECV)(dtls_context_ptr pdtls);

typedef struct _dtls_context{
	handle_head head;
	int type;

	int srv_major_ver;
	int srv_minor_ver;
	int cli_major_ver;
	int cli_minor_ver;

	bio_interface* pif;
	PF_DTLS_SEND dtls_send;
	PF_DTLS_RECV dtls_recv;

	int(*f_rng)(void *, byte_t *, dword_t);
	void* r_rng;

	dtls_security_context_ptr security_context;
	dtls_session_context_ptr session_context;
}dtls_context;

typedef struct _dtls_session_context{
	int major_ver;
	int minor_ver;

	//Session State
	int ses_size; //session id size
	byte_t ses_id[DTLS_SES_SIZE]; //session id
	int cookie_size; //for DTLS
	byte_t cookie_id[DTLS_COOKIE_SIZE]; //for DTLS cookie id

	int authen_client; //client to be authentic
	int session_resumed; //session is resumed. 0: none, 1: resumed.
	int handshake_over; //handshake step, 0 : undo, 1 : pass, -1 : close.

	void(*free_cipher_context)(dtls_session_context_ptr pses);

	dtls_cipher_context_ptr cipher_context;

	int pkg_size; //max package size
	int msg_zero; //empty message count

	sword_t snd_next_msgnum;
	dword_t snd_next_epoch;
	dword_t snd_next_seqnum;
	dtls_record_context_ptr snd_record;
	linear_t snd_linear;

	sword_t rcv_next_msgnum;
	dword_t rcv_next_epoch;
	dword_t rcv_next_seqnum;
	dtls_record_context_ptr rcv_record;
	linear_t rcv_linear;
}dtls_session_context;

typedef struct _dtls_security_context{
	int verify_mode; //verify mode

	schar_t* host_cn;
	schar_t* peer_cn;

	x509_crt* chain_ca;
	x509_crt* host_crt;
	x509_crt* peer_crt;

	rsa_context* rsa_ctx;
	dhm_context* dhm_ctx;
	ecdh_context* ecdh_ctx;
}dtls_security_context;

typedef struct _dtls_record_context{
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
}dtls_record_context;

#ifdef	__cplusplus
extern "C" {
#endif

LOC_API bool_t dtls10_handshake_server(dtls_context* pdtls);
LOC_API bool_t dtls10_handshake_client(dtls_context* pdtls);
LOC_API bool_t dtls12_handshake_server(dtls_context* pdtls);
LOC_API bool_t dtls12_handshake_client(dtls_context* pdtls);

/*
@FUNCTION xdtls_cli: create a DTLS client.
@INPUT unsigned short port: the network port to connect.
@INPUT const tchar_t* addr: the network address to connect.
@RETURN xhand_t: if succeeds return DTLS client handle, fails return NULL.
*/
EXP_API xhand_t xdtls_cli(unsigned short port, const tchar_t* addr);

/*
@FUNCTION xdtls_srv: create a DTLS server.
@INPUT unsigned short port: the network port from client.
@INPUT const tchar_t* addr: the network address from client.
@INPUT const byte_t* pack: the network package from client.
@INPUT dword_t size: the network package size in bytes.
@RETURN xhand_t: if succeeds return DTLS server handle, fails return NULL.
*/
EXP_API xhand_t xdtls_srv(unsigned short port, const tchar_t* addr, const byte_t* pack, dword_t size);

/*
@FUNCTION xdtls_bind: bind the local port.
@INPUT xhand_t dtls: the DTLS xhandle.
@INPUT unsigned short port: the network port from binding.
@RETURN bool_t: if succeeds return nonzero.
*/
EXP_API bool_t xdtls_bind(xhand_t dtls, unsigned short bind);

/*
@FUNCTION xdtls_socket: get socket resource handle.
@INPUT xhand_t dtls: the DTLS handle.
@RETURN res_file_t: return the socket resource handle.
*/
EXP_API res_file_t xdtls_socket(xhand_t dtls);

/*
@FUNCTION xdtls_type: get socket type, it can be _XDTLS_TYPE_CLI, _XDTLS_TYPE_SRV.
@INPUT xhand_t dtls: the DTLS handle.
@RETURN int: return the socket type.
*/
EXP_API int  xdtls_type(xhand_t dtls);

/*
@FUNCTION xdtls_close: close DTLS handle.
@INPUT xhand_t dtls: the DTLS handle.
@RETURN void: none.
*/
EXP_API void  xdtls_close(xhand_t dtls);

/*
@FUNCTION xdtls_write: write DTLS data.
@INPUT xhand_t dtls: the DTLS handle.
@INPUT const byte_t* data: the data buffer.
@INOUTPUT dword_t* pb: indicate the bytes to write and return the bytes writed.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xdtls_write(xhand_t dtls, const byte_t* data, dword_t* pb);

/*
@FUNCTION xdtls_flush: ensure write DTLS data compeleted.
@INPUT xhand_t dtls: the DTLS handle.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xdtls_flush(xhand_t dtls);

/*
@FUNCTION xdtls_read: read DTLS data.
@INPUT xhand_t dtls: the DTLS handle.
@OUTPUT byte_t* data: the data buffer.
@INOUTPUT dword_t* pb: indicate the bytes to read and return the bytes readed.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xdtls_read(xhand_t dtls, byte_t* data, dword_t* pb);

/*
@FUNCTION xdtls_setopt: set the socket options.
@INPUT xhand_t dtls: the dtls handle.
@INPUT int oid: the option id, eg: SOCK_OPTION_SNDBUF, SOCK_OPTION_RCVBUF, SOCK_OPTION_NONBLK.
@INPUT void* opt: the option value pointer
@INPUT int len: the value length in bytes, string value must be a zero terminated token and set len to zero.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xdtls_setopt(xhand_t dtls, int oid, void* opt, int len);

/*
@FUNCTION xdtls_settmo: set the socket timeout.
@INPUT xhand_t dtls: the dtls handle.
@INPUT dword_t tmo: the tmieout in millsecoend.
@RETURN void: none
*/
EXP_API void xdtls_settmo(xhand_t ucp, dword_t tmo);

/*
@FUNCTION xdtls_addr_port: get DTLS local address and port.
@INPUT xhand_t dtls: the DTLS handle.
@OUTPUT tchar_t* addr: the string buffer.
@RETURN unsigned short: return the local port.
*/
EXP_API unsigned short xdtls_addr_port(xhand_t dtls, tchar_t* addr);

/*
@FUNCTION xdtls_peer_port: get DTLS remote address and port.
@INPUT xhand_t dtls: the DTLS handle.
@OUTPUT tchar_t* addr: the string buffer.
@RETURN unsigned short: return the remote port.
*/
EXP_API unsigned short xdtls_peer_port(xhand_t dtls, tchar_t* addr);

/*
@FUNCTION xdtls_set_host: set DTLS host name.
@INPUT xhand_t dtls: the DTLS handle.
@INPUT const tchar_t* host_cn: the host name.
@RETURN void: none.
*/
EXP_API void xdtls_set_host(xhand_t dtls, const tchar_t* host_cn);

/*
@FUNCTION xdtls_set_peer: set DTLS peer name.
@INPUT xhand_t dtls: the DTLS handle.
@INPUT const tchar_t* host_cn: the peer name.
@RETURN void: none.
*/
EXP_API void xdtls_set_peer(xhand_t dtls, const tchar_t* peer_cn);

/*
@FUNCTION xdtls_set_ca: set DTLS root certificate.
@INPUT xhand_t dtls: the DTLS handle.
@INPUT const byte_t* sz_cert: the cerificate bytes buffer.
@INPUT dword_t clen: the cert data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xdtls_set_ca(xhand_t dtls, const byte_t* sz_cert, dword_t clen);

/*
@FUNCTION xdtls_set_cert: set DTLS owner certificate.
@INPUT xhand_t dtls: the DTLS handle.
@INPUT const byte_t* sz_cert: the cerificate bytes buffer.
@INPUT dword_t clen: the cert data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xdtls_set_cert(xhand_t dtls, const byte_t* sz_cert, dword_t clen);

/*
@FUNCTION xdtls_set_cert: set SSL owner certificate.
@INPUT xhand_t dtls: the DTLS handle.
@INPUT const byte_t* sz_rsa: the rsa key bytes buffer.
@INPUT dword_t rlen: the rsa data size in bytes.
@INPUT const byte_t* sz_pwd: the password key bytes buffer.
@INPUT dword_t plen: the password data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xdtls_set_rsa(xhand_t dtls, const byte_t* sz_rsa, dword_t rlen, const tchar_t* sz_pwd, int plen);

/*
@FUNCTION xdtls_set_cert: set SSL owner certificate.
@INPUT xhand_t dtls: the DTLS handle.
@INPUT const byte_t* sz_dhm: the dhm key bytes buffer.
@INPUT dword_t rlen: the dhm data size in bytes.
@INPUT const byte_t* sz_pwd: the password key bytes buffer.
@INPUT dword_t plen: the password data size in bytes.
@RETURN bool_t: if succeeds return nonzero, fails return zero.
*/
EXP_API bool_t xdtls_set_dhm(xhand_t dtls, const byte_t* sz_dhm, dword_t rlen);

/*
@FUNCTION xdtls_set_auth: set DTLS authorization mode.
@INPUT xhand_t dtls: the DTLS handle.
@INPUT int srv_verify: the certify verify mode, it can be DTLS_VERIFY_NONE, DTLS_VERIFY_OPTIONAL, DTLS_VERIFY_REQUIRED.
@RETURN void: none.
*/
EXP_API void xdtls_set_verify(xhand_t dtls, int srv_verify);

/*
@FUNCTION xdtls_set_version: set DTLS/TLS maximized version
@INPUT xhand_t dtls: the DTLS handle.
@INPUT int cli_ver: the client maximized version it can be DTLSv30, TLSv10, TLSv11, TLSv12.
@RETURN void: none.
*/
EXP_API void xdtls_set_version(xhand_t dtls, int cli_ver);

EXP_API void xdtls_set_package(xhand_t dtls, dword_t pkg_size);

#ifdef	__cplusplus
}
#endif

#endif /*XDK_SUPPORT_SOCK*/

#endif /*IMPSOCK_H*/