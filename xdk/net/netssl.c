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


static int _ssl_check_rcv_msg(ssl_context* pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->rcv_record);

	if (prec->rcv_msg_len == 0)
	{
		pses->msg_zero++;

		if (pses->msg_zero > 3)
		{
			return C_ERR;
		}
	}
	else
	{
		pses->msg_zero = 0;
	}

	return C_OK;
}

static bool_t _ssl_write_data(ssl_context* pssl, byte_t* buf, int* need)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->snd_record);

	if (!(*need))
		return 1;

	*need = (*need + prec->snd_msg_pop < SSL_PKG_SIZE) ? (*need) : (SSL_PKG_SIZE - prec->snd_msg_pop);

	xmem_copy(prec->snd_msg + prec->snd_msg_pop, buf, *need);
	prec->snd_msg_pop += (*need);

	if (prec->snd_msg_pop == SSL_PKG_SIZE)
	{
		prec->snd_msg_len = prec->snd_msg_pop;
		prec->snd_msg_type = SSL_MSG_APPLICATION_DATA;

		if (C_OK != (*pssl->ssl_send)(pssl))
			return 0;
	}

	return 1;
}

static bool_t _ssl_flush_data(ssl_context* pssl)
{
	ssl_session_context* pses = (ssl_session_context*)(pssl->session_context);
	ssl_record_context* prec = (ssl_record_context*)(pses->snd_record);

	if (!prec->snd_msg_pop)
		return 1;

	prec->snd_msg_len = prec->snd_msg_pop;
	prec->snd_msg_type = SSL_MSG_APPLICATION_DATA;

	return (C_OK == (*pssl->ssl_send)(pssl)) ? 1 : 0;
}

static bool_t _ssl_write_close(ssl_context* pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->snd_record);

	dword_t dw;

	pses->handshake_over = -1;

	prec->snd_msg_type = SSL_MSG_ALERT;
	prec->snd_msg_len = 2;
	prec->snd_msg[0] = SSL_LEVEL_WARNING;
	prec->snd_msg[1] = SSL_ALERT_CLOSE_NOTIFY;

	if (pssl->ssl_send)
	{
		return (C_OK == (*pssl->ssl_send)(pssl)) ? 1 : 0;
	}
	else
	{
		dw = SSL_HDR_SIZE + prec->snd_msg_len;

		return ((*pssl->pif->pf_write)(pssl->pif->fd, prec->snd_hdr, &dw)) ? 1 : 0;
	}
}

static bool_t _ssl_read_data(ssl_context* pssl, byte_t* buf, int* need)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->rcv_record);

	if (!(*need))
		return 1;

	if (prec->rcv_msg_pop == prec->rcv_msg_len)
	{
		if (C_OK != (*pssl->ssl_recv)(pssl))
		{
			return 0;
		}
		
		if (prec->rcv_msg_len && prec->rcv_msg_type != SSL_MSG_APPLICATION_DATA)
		{
			set_last_error(_T("_ssl_read_data"), _T("not application data"), -1);
			return 0;
		}
	}

	*need = (*need + prec->rcv_msg_pop < prec->rcv_msg_len) ? (*need) : (prec->rcv_msg_len - prec->rcv_msg_pop);
	
	xmem_copy(buf, prec->rcv_msg + prec->rcv_msg_pop, *need);
	prec->rcv_msg_pop += (*need);

	return 1;
}

static void _ssl_init(ssl_context* pssl)
{
	ssl_session_context* pses;
	ssl_security_context* psec;
	ssl_record_context* prec;

	pssl->cli_major_ver = SSL_MAJOR_VERSION_3;
	pssl->cli_minor_ver = SSL_MINOR_VERSION_0;
	pssl->srv_major_ver = SSL_MAJOR_VERSION_3;
	pssl->srv_minor_ver = SSL_MINOR_VERSION_0;

	pssl->f_rng = havege_random;
	pssl->r_rng = xmem_alloc(sizeof(havege_state));
	havege_init(pssl->r_rng);

	//initialize certificate
	pssl->security_context = psec = (ssl_security_context*)xmem_alloc(sizeof(ssl_security_context));
	psec->verify_mode = SSL_VERIFY_NONE;

	//initialize session state
	pssl->session_context = pses = (ssl_session_context*)xmem_alloc(sizeof(ssl_session_context));
	pses->authen_client = 0;
	pses->session_resumed = 0;
	pses->handshake_over = 0;

	if (pssl->type == SSL_TYPE_CLIENT)
	{
		pses->major_ver = pssl->cli_major_ver;
		pses->minor_ver = pssl->cli_minor_ver;
	}

	//initialize records
	pses->rcv_record = prec = (ssl_record_context*)xmem_alloc(sizeof(ssl_record_context));
	prec->compressed = 0;
	prec->crypted = 0;
	prec->rcv_pkg = (byte_t *)xmem_alloc(SSL_MAX_SIZE);
	prec->rcv_ctr = prec->rcv_pkg;
	prec->rcv_hdr = prec->rcv_pkg + SSL_CTR_SIZE;
	prec->rcv_msg = prec->rcv_pkg + SSL_CTR_SIZE + SSL_HDR_SIZE;

	pses->snd_record = prec = (ssl_record_context*)xmem_alloc(sizeof(ssl_record_context));
	prec->compressed = 0;
	prec->crypted = 0;
	prec->snd_pkg = (byte_t *)xmem_alloc(SSL_MAX_SIZE);
	prec->snd_ctr = prec->snd_pkg;
	prec->snd_hdr = prec->snd_pkg + SSL_CTR_SIZE;
	prec->snd_msg = prec->snd_pkg + SSL_CTR_SIZE + SSL_HDR_SIZE;
}

static void _ssl_uninit(ssl_context* pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_security_context* psec;
	ssl_record_context* prec;

	if (pssl->r_rng)
	{
		havege_free(pssl->r_rng);
		xmem_free(pssl->r_rng);
	}
	pssl->r_rng = NULL;

	if (pses)
	{
		prec = (ssl_record_context*)(pses->rcv_record);
		if (prec)
		{
			if (prec->rcv_pkg)
			{
				xmem_free(prec->rcv_pkg);
			}
			xmem_free(prec);
		}

		prec = (ssl_record_context*)(pses->snd_record);
		if (prec)
		{
			if (prec->snd_pkg)
			{
				xmem_free(prec->snd_pkg);
			}
			xmem_free(prec);
		}

		if (pses->cipher_context)
		{
			(*pses->free_cipher_context)(pses);
		}

		xmem_free(pses);
	}
	pssl->session_context = NULL;

	psec = (ssl_security_context*)(pssl->security_context);
	if (psec)
	{
		if (psec->host_crt)
		{
			x509_crt_free(psec->host_crt);
			xmem_free(psec->host_crt);
		}
		if (psec->peer_crt)
		{
			x509_crt_free(psec->peer_crt);
			xmem_free(psec->peer_crt);
		}
		if (psec->chain_ca)
		{
			x509_crt_free(psec->chain_ca);
			xmem_free(psec->chain_ca);
		}
		if (psec->rsa_ctx)
		{
			rsa_free(psec->rsa_ctx);
			xmem_free(psec->rsa_ctx);
		}
		if (psec->dhm_ctx)
		{
			dhm_free(psec->dhm_ctx);
			xmem_free(psec->dhm_ctx);
		}
		if (psec->ecdh_ctx)
		{
			ecdh_free(psec->ecdh_ctx);
			xmem_free(psec->ecdh_ctx);
		}

		xmem_free(psec);
	}
	pssl->security_context = NULL;
}

static void _ssl_test_version(ssl_context* pssl, int* major_ver, int* minor_ver)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->rcv_record);

	int haslen, msglen, seslen, ciphlen, complen, comped;
	int n, extlen, type, lstlen;

	haslen = GET_THREEBYTE_NET(prec->rcv_msg, 1);

	msglen = SSL_HSH_SIZE;

	/*
	ProtocolVersion
	*/
	*major_ver = prec->rcv_msg[msglen];
	*minor_ver = prec->rcv_msg[msglen + 1];
	msglen += 2;

	/*
	Random
	*/
	msglen += SSL_RND_SIZE;

	/*
	SessionID
	*/
	seslen = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;
	msglen += seslen;

	/*
	CipherSuite
	*/
	ciphlen = GET_SWORD_NET(prec->rcv_msg, msglen);
	msglen += 2;
	msglen += ciphlen;

	/*
	CompressionMethod
	*/
	complen = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	comped = GET_BYTE(prec->rcv_msg, msglen);
	msglen++;

	//has no extension
	if (msglen == haslen + SSL_HSH_SIZE)
		return;

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
			n = GET_BYTE(prec->rcv_msg, msglen);
			msglen += 1;
			extlen -= 1;
			lstlen -= 1;

			*major_ver = prec->rcv_msg[msglen];
			*minor_ver = prec->rcv_msg[msglen + 1];
			msglen += n;
			extlen -= n;
			lstlen -= n;
			break;
		default:
			//skip 
			break;
		}

		msglen += lstlen;
		extlen -= lstlen;

		if (extlen < 0)
			break;
	}
}

static bool_t _ssl_handshake_server(ssl_context* pssl)
{
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->rcv_record);

	dword_t dw;
	int major_ver, minor_ver;

	if (!pses->handshake_over)
	{
		dw = SSL_HDR_SIZE;
		if (!(pssl->pif->pf_read)(pssl->pif->fd, prec->rcv_hdr, &dw))
		{
			set_last_error(_T("_ssl_handshake_server"), _T("read message head failed"), -1);
			return 0;
		}

		if (!dw)
		{
			set_last_error(_T("_ssl_handshake_server"), _T("empty message head"), -1);
			return 0;
		}

		prec->rcv_msg_type = GET_BYTE(prec->rcv_hdr, 0);
		prec->rcv_msg_len = GET_SWORD_NET(prec->rcv_hdr, (SSL_HDR_SIZE - 2));

		if (prec->rcv_msg_len < 1 || prec->rcv_msg_len > SSL_MAX_SIZE)
		{
			set_last_error(_T("_ssl_handshake_server"), _T("invalid message block length"), -1);
			return 0;
		}

		dw = prec->rcv_msg_len;
		if (!(*pssl->pif->pf_read)(pssl->pif->fd, prec->rcv_msg, &dw))
		{
			set_last_error(_T("_ssl_handshake_server"), _T("read message block failed"), -1);
			return 0;
		}

		if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CLIENT_HELLO)
		{
			set_last_error(_T("_ssl_handshake_server"), _T("invalid client hello message"), -1);
			return 0;
		}
		/*
		ProtocolVersion client_version;
		*/
		major_ver = prec->rcv_msg[SSL_HSH_SIZE];
		minor_ver = prec->rcv_msg[SSL_HSH_SIZE + 1];

		if (major_ver == SSL_MAJOR_VERSION_3 && minor_ver == SSL_MINOR_VERSION_3)
		{
			_ssl_test_version(pssl, &major_ver, &minor_ver);
		}

		if (minor_ver == SSL_MINOR_VERSION_0)
		{
			return ssl30_handshake_server(pssl);
		}
		else if (minor_ver == SSL_MINOR_VERSION_1)
		{
			return tls10_handshake_server(pssl);
		}
		else if (minor_ver == SSL_MINOR_VERSION_2)
		{
			return tls11_handshake_server(pssl);
		}
		else if (minor_ver == SSL_MINOR_VERSION_3)
		{
			return tls12_handshake_server(pssl);
		}
		else if (minor_ver == SSL_MINOR_VERSION_4)
		{
			return tls13_handshake_server(pssl);
		}
		else
		{
			set_last_error(_T("_ssl_handshake_server"), _T("minor version not support"), -1);
			return 0;
		}
	}

	return (pses->handshake_over == 1) ? 1 : 0;
}

static bool_t _ssl_handshake_client(ssl_context* pssl)
{
	if (pssl->cli_minor_ver == SSL_MINOR_VERSION_0)
	{
		return ssl30_handshake_client(pssl);
	}
	else if (pssl->cli_minor_ver == SSL_MINOR_VERSION_1)
	{
		return tls10_handshake_client(pssl);
	}
	else if (pssl->cli_minor_ver == SSL_MINOR_VERSION_2)
	{
		return tls11_handshake_client(pssl);
	}
	else if (pssl->cli_minor_ver == SSL_MINOR_VERSION_3)
	{
		return tls12_handshake_client(pssl);
	}
	else if (pssl->cli_minor_ver == SSL_MINOR_VERSION_4)
	{
		return tls13_handshake_client(pssl);
	}
	else
	{
		set_last_error(_T("_ssl_handshake_client"), _T("minor version not support"), -1);
		return 0;
	}
}

/*********************************************************************************************************/

xhand_t xssl_cli(unsigned short port, const tchar_t* addr)
{
	ssl_context* pssl;
	xhand_t tcp;
	
	tcp = xtcp_cli(port, addr);
	if (!tcp)
		return NULL;

	pssl = (ssl_context*)xmem_alloc(sizeof(ssl_context));
	pssl->head.tag = _HANDLE_SSL;

	pssl->type = SSL_TYPE_CLIENT;

	_ssl_init(pssl);

	pssl->pif = (bio_interface*)xmem_alloc(sizeof(bio_interface));

	get_bio_interface(tcp, pssl->pif);

	return &pssl->head;
}

xhand_t xssl_srv(res_file_t so)
{
	ssl_context* pssl;
	xhand_t tcp;

	tcp = xtcp_srv(so);
	if (!tcp)
		return NULL;

	pssl = (ssl_context*)xmem_alloc(sizeof(ssl_context));
	pssl->head.tag = _HANDLE_SSL;

	pssl->type = SSL_TYPE_SERVER;

	_ssl_init(pssl);

	pssl->pif = (bio_interface*)xmem_alloc(sizeof(bio_interface));

	get_bio_interface(tcp, pssl->pif);

	return &pssl->head;
}

void  xssl_close(xhand_t ssl)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->rcv_record);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	XDK_ASSERT(pssl->type == SSL_TYPE_CLIENT || pssl->type == SSL_TYPE_SERVER);

	if (prec->snd_msg_pop)
	{
		_ssl_flush_data(pssl);
	}

	if(pses->handshake_over == 1)
	{
		_ssl_write_close(pssl);
	}

	if (pssl->pif)
		(*pssl->pif->pf_close)(pssl->pif->fd);

	_ssl_uninit(pssl);

	if (pssl->pif)
		xmem_free(pssl->pif);

	xmem_free(pssl);
}

res_file_t xssl_socket(xhand_t ssl)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	return (pssl->pif->fd) ? xtcp_socket(pssl->pif->fd) : INVALID_FILE;
}

int xssl_type(xhand_t ssl)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	return pssl->type;
}

bool_t xssl_write(xhand_t ssl, const byte_t* buf, dword_t* pb)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->rcv_record);

	int bys, pos;

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	if (!pses->handshake_over)
	{
		if (pssl->type == SSL_TYPE_CLIENT && !_ssl_handshake_client(pssl))
		{
			_ssl_write_close(pssl);
			return 0;
		}

		if (pssl->type == SSL_TYPE_SERVER && !_ssl_handshake_server(pssl))
		{
			_ssl_write_close(pssl);
			return 0;
		}

		//clear handshake
		prec->rcv_msg_type = 0;
		prec->rcv_msg_len = 0;
		prec->rcv_msg_pop = 0;

		prec->snd_msg_type = 0;
		prec->snd_msg_len = 0;
		prec->snd_msg_pop = 0;
	}

	if (pses->handshake_over < 0)
	{
		*pb = 0;
		return 1;
	}

	pos = 0;
	while (pos < (int)(*pb))
	{
		bys = *pb - pos;
		if (!_ssl_write_data(pssl, buf + pos, &bys))
			break;

		if (!bys)
			break;

		pos += bys;
	}

	return (pos == (int)(*pb)) ? 1 : 0;
}

bool_t xssl_flush(xhand_t ssl)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	return _ssl_flush_data(pssl);
}

bool_t xssl_read(xhand_t ssl, byte_t* buf, dword_t* pb)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;
	ssl_record_context* prec = (ssl_record_context*)(pses->rcv_record);

	int bys, pos;
	bool_t rt = 1;

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	if (!pses->handshake_over)
	{
		if (pssl->type == SSL_TYPE_CLIENT && !_ssl_handshake_client(pssl))
		{
			_ssl_write_close(pssl);
			return 0;
		}

		if (pssl->type == SSL_TYPE_SERVER && !_ssl_handshake_server(pssl))
		{
			_ssl_write_close(pssl);
			return 0;
		}

		//clear handshake
		prec->rcv_msg_type = 0;
		prec->rcv_msg_len = 0;
		prec->rcv_msg_pop = 0;

		prec->snd_msg_type = 0;
		prec->snd_msg_len = 0;
		prec->snd_msg_pop = 0;
	}

	if (pses->handshake_over < 0)
	{
		*pb = 0;
		return 1;
	}

	pos = 0;
	while (pos < (int)(*pb))
	{
		bys = *pb - pos;
		rt = _ssl_read_data(pssl, buf + pos, &bys);
		if (!rt)
			break;

		if (!bys)
			break;

		pos += bys;
	}

	*pb = pos;

	return (*pb)? 1 : 0;
}

unsigned short xssl_addr_port(xhand_t ssl, tchar_t* addr)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	net_addr_t na = { 0 };
	unsigned short port;

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	socket_addr(xssl_socket(ssl), &na);
	conv_addr(&na, &port, addr);

	return port;
}

unsigned short xssl_peer_port(xhand_t ssl, tchar_t* addr)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	net_addr_t na = { 0 };
	unsigned short port;

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	socket_peer(xssl_socket(ssl), &na);
	conv_addr(&na, &port, addr);

	return port;
}

bool_t xssl_setopt(xhand_t ssl, int oid, void* opt, int len)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	switch (oid)
	{
	case SOCK_OPTION_SNDBUF:
		socket_set_sndbuf(xssl_socket(ssl), *(int*)opt);
		return 1;
	case SOCK_OPTION_RCVBUF:
		socket_set_rcvbuf(xssl_socket(ssl), *(int*)opt);
		return 1;
	case SOCK_OPTION_NONBLK:
		socket_set_nonblk(xssl_socket(ssl), *(bool_t*)opt);
		return 1;
	}

	return 0;
}

void xssl_set_host(xhand_t ssl, const tchar_t* host_cn)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_security_context* psec = (ssl_security_context*)(pssl->security_context);

	int len;

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	XDK_ASSERT(pssl->type == SSL_TYPE_CLIENT);

#ifdef _UNICODE
	len = ucs_to_mbs(host_cn, -1, NULL, MAX_LONG);
	psec->host_cn = a_xsalloc(len + 1);
	ucs_to_mbs(host_cn, -1, psec->host_cn, len);
#else
	len = a_xslen(host_cn);
	psec->host_cn = a_xsnclone(host_cn, len);
#endif
}

void xssl_set_peer(xhand_t ssl, const tchar_t* peer_cn)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_security_context* psec = (ssl_security_context*)(pssl->security_context);

	int len;

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	XDK_ASSERT(pssl->type == SSL_TYPE_SERVER);

#ifdef _UNICODE
	len = ucs_to_mbs(peer_cn, -1, NULL, MAX_LONG);
	psec->peer_cn = a_xsalloc(len + 1);
	ucs_to_mbs(peer_cn, -1, psec->peer_cn, len);
#else
	len = a_xslen(peer_cn);
	psec->peer_cn = a_xsnclone(peer_cn, len);
#endif
}

bool_t xssl_set_ca(xhand_t ssl, const byte_t* sz_cert, dword_t clen)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_security_context* psec = (ssl_security_context*)(pssl->security_context);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	if (clen)
	{
		if (!psec->chain_ca)
			psec->chain_ca = (x509_crt*)xmem_alloc(sizeof(x509_crt));

		if (C_OK != x509_crt_parse(psec->chain_ca, sz_cert, clen))
		{
			x509_crt_free(psec->chain_ca);
			xmem_free(psec->chain_ca);
			psec->chain_ca = NULL;

			return 0;
		}
	}

	return 1;
}

bool_t xssl_set_cert(xhand_t ssl, const byte_t* sz_cert, dword_t clen)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_security_context* psec = (ssl_security_context*)(pssl->security_context);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	if (clen)
	{
		if (!psec->host_crt)
			psec->host_crt = (x509_crt*)xmem_alloc(sizeof(x509_crt));

		if (C_OK != x509_crt_parse(psec->host_crt, sz_cert, clen))
		{
			x509_crt_free(psec->host_crt);
			xmem_free(psec->host_crt);
			psec->host_crt = NULL;

			return 0;
		}
	}

	return 1;
}

bool_t xssl_set_rsa(xhand_t ssl, const byte_t* sz_rsa, dword_t rlen, const tchar_t* sz_pwd, int len)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_security_context* psec = (ssl_security_context*)(pssl->security_context);

	byte_t buf_pwd[RES_LEN + 1] = { 0 };
	dword_t dw;

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	if (rlen)
	{
		if (len < 0)
			len = xslen(sz_pwd);

#ifdef _UNICODE
		dw = ucs_to_utf8(sz_pwd, len, buf_pwd, RES_LEN);
#else
		dw = (len < RES_LEN) ? len : RES_LEN;
		a_xsncpy((schar_t*)buf_pwd, sz_pwd, dw);
#endif

		if (!psec->rsa_ctx)
			psec->rsa_ctx = (rsa_context*)xmem_alloc(sizeof(rsa_context));

		if (C_OK != rsa_parse_key(psec->rsa_ctx, sz_rsa, rlen, buf_pwd, dw))
		{
			rsa_free(psec->rsa_ctx);
			xmem_free(psec->rsa_ctx);
			psec->rsa_ctx = NULL;

			return 0;
		}
	}

	return 1;
}

bool_t xssl_set_dhm(xhand_t ssl, const byte_t* sz_dhm, dword_t dlen)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_security_context* psec = (ssl_security_context*)(pssl->security_context);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	if (dlen)
	{
		if (!psec->dhm_ctx)
			psec->dhm_ctx = (dhm_context*)xmem_alloc(sizeof(dhm_context));

		if (C_OK != dhm_parse_dhm(psec->dhm_ctx, sz_dhm, dlen))
		{
			dhm_free(psec->dhm_ctx);
			xmem_free(psec->dhm_ctx);
			psec->dhm_ctx = NULL;

			return 0;
		}
	}

	return 1;
}

void xssl_set_verify(xhand_t ssl, int srv_verify)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_security_context* psec = (ssl_security_context*)(pssl->security_context);

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	psec->verify_mode = srv_verify;
}

void xssl_set_version(xhand_t ssl, int cli_ver)
{
	ssl_context* pssl = TypePtrFromHead(ssl_context, ssl);
	ssl_session_context* pses = (ssl_session_context*)pssl->session_context;

	XDK_ASSERT(ssl && ssl->tag == _HANDLE_SSL);

	XDK_ASSERT(pssl->type == SSL_TYPE_CLIENT);

	pssl->cli_minor_ver = GETLBYTE(cli_ver);
	pssl->cli_major_ver = GETHBYTE(cli_ver);

	pses->major_ver = pssl->cli_major_ver;
	pses->minor_ver = (pssl->cli_minor_ver == SSL_MINOR_VERSION_4) ? SSL_MINOR_VERSION_3 : pssl->cli_minor_ver;
}

#endif //XDK_SUPPORT_SOCK
