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

#define DTLS_WIN_SIZE		32

static int _dtls_check_rcv_msg(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);

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

static bool_t _dtls_write_data(dtls_context* pdtls, byte_t* buf, int* need)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);

	if (!(*need))
		return 1;

	*need = (*need + prec->snd_msg_pop < DTLS_PKG_SIZE) ? (*need) : (DTLS_PKG_SIZE - prec->snd_msg_pop);

	xmem_copy(prec->snd_msg + prec->snd_msg_pop, buf, *need);
	prec->snd_msg_pop += (*need);

	if (prec->snd_msg_pop == DTLS_PKG_SIZE)
	{
		prec->snd_msg_len = prec->snd_msg_pop;
		prec->snd_msg_type = SSL_MSG_APPLICATION_DATA;

		if (C_OK != (*pdtls->dtls_send)(pdtls))
			return 0;
	}

	return 1;
}

static bool_t _dtls_flush_data(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);

	if (!prec->snd_msg_pop)
		return 1;

	prec->snd_msg_len = prec->snd_msg_pop;
	prec->snd_msg_type = SSL_MSG_APPLICATION_DATA;

	return (C_OK == (*pdtls->dtls_send)(pdtls)) ? 1 : 0;
}

static bool_t _dtls_write_close(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);

	dword_t dw;

	pses->handshake_over = -1;

	prec->snd_msg_type = SSL_MSG_ALERT;
	prec->snd_msg_len = 2;
	prec->snd_msg[0] = SSL_LEVEL_WARNING;
	prec->snd_msg[1] = SSL_ALERT_CLOSE_NOTIFY;

	if (pdtls->dtls_send)
	{
		return (C_OK == (*pdtls->dtls_send)(pdtls)) ? 1 : 0;
	}
	else
	{
		dw = DTLS_HDR_SIZE + prec->snd_msg_len;

		if (!(*pdtls->pif->pf_write)(pdtls->pif->fd, prec->snd_hdr, &dw))
			return 0;

		(*pdtls->pif->pf_flush)(pdtls->pif->fd);
		return 1;
	}
}

static bool_t _dtls_read_data(dtls_context* pdtls, byte_t* buf, int* need)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);

	if (!(*need))
		return 1;

	if (prec->rcv_msg_pop == prec->rcv_msg_len)
	{
		if (C_OK != (*pdtls->dtls_recv)(pdtls))
		{
			return 0;
		}
		
		if (prec->rcv_msg_len && prec->rcv_msg_type != SSL_MSG_APPLICATION_DATA)
		{
			set_last_error(_T("_dtls_read_data"), _T("not application data"), -1);
			return 0;
		}
	}

	*need = (*need + prec->rcv_msg_pop < prec->rcv_msg_len) ? (*need) : (prec->rcv_msg_len - prec->rcv_msg_pop);
	
	xmem_copy(buf, prec->rcv_msg + prec->rcv_msg_pop, *need);
	prec->rcv_msg_pop += (*need);

	return 1;
}

static void _dtls_init(dtls_context* pdtls)
{
	dtls_session_context* pses;
	dtls_security_context* psec;
	dtls_record_context* prec;

	pdtls->cli_major_ver = DTLS_MAJOR_VERSION_1;
	pdtls->cli_minor_ver = DTLS_MINOR_VERSION_0;
	pdtls->srv_major_ver = DTLS_MAJOR_VERSION_1;
	pdtls->srv_minor_ver = DTLS_MINOR_VERSION_0;

	pdtls->f_rng = havege_random;
	pdtls->r_rng = xmem_alloc(sizeof(havege_state));
	havege_init(pdtls->r_rng);

	//initialize certificate
	pdtls->security_context = psec = (dtls_security_context*)xmem_alloc(sizeof(dtls_security_context));
	psec->verify_mode = SSL_VERIFY_NONE;

	//initialize session state
	pdtls->session_context = pses = (dtls_session_context*)xmem_alloc(sizeof(dtls_session_context));
	pses->session_resumed = 0;
	pses->authen_client = 0;
	pses->handshake_over = 0;

	if (pdtls->type == DTLS_TYPE_CLIENT)
	{
		pses->major_ver = pdtls->cli_major_ver;
		pses->minor_ver = pdtls->cli_minor_ver;
	}

	pses->pkg_size = DTLS_PKG_SIZE;

	pses->snd_linear = alloc_linear(DTLS_WIN_SIZE);
	pses->rcv_linear = alloc_linear(DTLS_WIN_SIZE);

	//initialize records
	pses->rcv_record = prec = (dtls_record_context*)xmem_alloc(sizeof(dtls_record_context));
	prec->compressed = 0;
	prec->crypted = 0;
	prec->rcv_pkg = (byte_t *)xmem_alloc(DTLS_MAX_SIZE);
	prec->rcv_ctr = prec->rcv_pkg;
	prec->rcv_hdr = prec->rcv_pkg + DTLS_CTR_SIZE;
	prec->rcv_msg = prec->rcv_pkg + DTLS_CTR_SIZE + DTLS_HDR_SIZE;

	pses->snd_record = prec = (dtls_record_context*)xmem_alloc(sizeof(dtls_record_context));
	prec->compressed = 0;
	prec->crypted = 0;
	prec->snd_pkg = (byte_t *)xmem_alloc(DTLS_MAX_SIZE);
	prec->snd_ctr = prec->snd_pkg;
	prec->snd_hdr = prec->snd_pkg + DTLS_CTR_SIZE;
	prec->snd_msg = prec->snd_pkg + DTLS_CTR_SIZE + DTLS_HDR_SIZE;
}

static void _dtls_uninit(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec;
	dtls_record_context* prec;

	if (pdtls->r_rng)
	{
		havege_free(pdtls->r_rng);
		xmem_free(pdtls->r_rng);
	}
	pdtls->r_rng = NULL;

	psec = (dtls_security_context*)(pdtls->security_context);
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
		if (psec->rsa_ctx)
		{
			rsa_free(psec->rsa_ctx);
			xmem_free(psec->rsa_ctx);
		}

		xmem_free(psec);
	}
	pdtls->security_context = NULL;

	if (pses)
	{
		prec = (dtls_record_context*)(pses->rcv_record);
		if (prec)
		{
			xmem_free(prec->rcv_pkg);
		}
		xmem_free(prec);

		prec = (dtls_record_context*)(pses->snd_record);
		if (prec)
		{
			xmem_free(prec->snd_pkg);
		}
		xmem_free(prec);

		if (pses->cipher_context)
		{
			(*pses->free_cipher_context)(pses);
		}

		if (pses->snd_linear)
			free_linear(pses->snd_linear);

		if (pses->rcv_linear)
			free_linear(pses->rcv_linear);

		xmem_free(pses);
	}
	pdtls->session_context = NULL;
}

static bool_t _dtls_handshake_server(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->rcv_record);

	dword_t dw;
	int major_ver, minor_ver;

	if (!pses->handshake_over)
	{
		dw = DTLS_HDR_SIZE;
		if (!(pdtls->pif->pf_read)(pdtls->pif->fd, prec->rcv_hdr, &dw))
		{
			set_last_error(_T("_dtls_handshake_server"), _T("read message head failed"), -1);
			return 0;
		}

		if (!dw)
		{
			set_last_error(_T("_dtls_handshake_server"), _T("empty message head"), -1);
			return 0;
		}

		prec->rcv_msg_type = GET_BYTE(prec->rcv_hdr, 0);
		prec->rcv_msg_len = GET_SWORD_NET(prec->rcv_hdr, (DTLS_HDR_SIZE - 2));

		if (prec->rcv_msg_len < 1 || prec->rcv_msg_len > DTLS_MAX_SIZE)
		{
			set_last_error(_T("_dtls_handshake_server"), _T("invalid message block length"), -1);
			return 0;
		}

		dw = prec->rcv_msg_len;
		if (!(*pdtls->pif->pf_read)(pdtls->pif->fd, prec->rcv_msg, &dw))
		{
			set_last_error(_T("_dtls_handshake_server"), _T("read message block failed"), -1);
			return 0;
		}

		if (prec->rcv_msg_type != SSL_MSG_HANDSHAKE || prec->rcv_msg[0] != SSL_HS_CLIENT_HELLO)
		{
			set_last_error(_T("_dtls_handshake_server"), _T("invalid client hello message"), -1);
			return 0;
		}
		/*
		ProtocolVersion client_version;
		*/
		major_ver = prec->rcv_msg[DTLS_HSH_SIZE + DTLS_MSH_SIZE];
		minor_ver = prec->rcv_msg[DTLS_HSH_SIZE + DTLS_MSH_SIZE + 1];

		if (minor_ver == DTLS_MINOR_VERSION_0)
		{
			return dtls10_handshake_server(pdtls);
		}
		else if (minor_ver == DTLS_MINOR_VERSION_2)
		{
			return dtls12_handshake_server(pdtls);
		}
		else
		{
			set_last_error(_T("_dtls_handshake_server"), _T("minor version not support"), -1);
			return 0;
		}
	}

	return (pses->handshake_over == 1) ? 1 : 0;
}

static bool_t _dtls_handshake_client(dtls_context* pdtls)
{
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;

	if (pdtls->cli_minor_ver == DTLS_MINOR_VERSION_0)
	{
		return dtls10_handshake_client(pdtls);
	}
	else if (pdtls->cli_minor_ver == DTLS_MINOR_VERSION_2)
	{
		return dtls12_handshake_client(pdtls);
	}
	else
	{
		set_last_error(_T("_dtls_handshake_client"), _T("minor version not support"), -1);
		return 0;
	}
}

/*********************************************************************************************************/

xhand_t xdtls_cli(unsigned short port, const tchar_t* addr)
{
	dtls_context* pdtls;
	xhand_t udp;
	
	udp = xudp_cli(port, addr);
	if (!udp)
		return NULL;

	pdtls = (dtls_context*)xmem_alloc(sizeof(dtls_context));
	pdtls->head.tag = _HANDLE_DTLS;

	pdtls->type = DTLS_TYPE_CLIENT;

	_dtls_init(pdtls);

	pdtls->pif = (bio_interface*)xmem_alloc(sizeof(bio_interface));

	get_bio_interface(udp, pdtls->pif);

	return &pdtls->head;
}

xhand_t xdtls_srv(unsigned short port, const tchar_t* addr, const byte_t* pack, dword_t size)
{
	dtls_context* pdtls;
	xhand_t udp;

	udp = xudp_srv(port, addr, pack, size);
	if (!udp)
		return NULL;

	pdtls = (dtls_context*)xmem_alloc(sizeof(dtls_context));
	pdtls->head.tag = _HANDLE_DTLS;

	pdtls->type = DTLS_TYPE_SERVER;

	_dtls_init(pdtls);

	pdtls->pif = (bio_interface*)xmem_alloc(sizeof(bio_interface));

	get_bio_interface(udp, pdtls->pif);

	return &pdtls->head;
}

bool_t  xdtls_bind(xhand_t dtls, unsigned short bind)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	return xudp_bind(pdtls->pif->fd, bind);
}

void  xdtls_close(xhand_t dtls)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec = (dtls_record_context*)(pses->snd_record);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	XDK_ASSERT(pdtls->type == DTLS_TYPE_CLIENT || pdtls->type == DTLS_TYPE_SERVER);

	_dtls_flush_data(pdtls);

	if(pses->handshake_over == 1)
	{
		_dtls_write_close(pdtls);
	}

	if (pdtls->pif)
		(*pdtls->pif->pf_close)(pdtls->pif->fd);

	_dtls_uninit(pdtls);

	if (pdtls->pif)
		xmem_free(pdtls->pif);

	xmem_free(pdtls);
}

res_file_t xdtls_socket(xhand_t dtls)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	return (pdtls->pif->fd) ? xudp_socket(pdtls->pif->fd) : INVALID_FILE;
}


void xdtls_settmo(xhand_t dtls, dword_t tmo)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	xudp_settmo(pdtls->pif->fd, tmo);
}

int xdtls_type(xhand_t dtls)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	return pdtls->type;
}

bool_t xdtls_write(xhand_t dtls, const byte_t* buf, dword_t* pb)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec;

	int bys, pos;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	if (!pses->handshake_over)
	{
		if (pdtls->type == DTLS_TYPE_CLIENT && !_dtls_handshake_client(pdtls))
		{
			_dtls_write_close(pdtls);
			return 0;
		}

		if (pdtls->type == DTLS_TYPE_SERVER && !_dtls_handshake_server(pdtls))
		{
			_dtls_write_close(pdtls);
			return 0;
		}

		//clear handshake
		prec = (dtls_record_context*)(pses->rcv_record);
		prec->rcv_msg_type = 0;
		prec->rcv_msg_len = 0;
		prec->rcv_msg_pop = 0;

		prec = (dtls_record_context*)(pses->snd_record);
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
		if (!_dtls_write_data(pdtls, buf + pos, &bys))
			break;

		if (!bys)
			break;

		pos += bys;
	}

	return (pos == (int)(*pb)) ? 1 : 0;
}

bool_t xdtls_flush(xhand_t dtls)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	return _dtls_flush_data(pdtls);
}

bool_t xdtls_read(xhand_t dtls, byte_t* buf, dword_t* pb)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_record_context* prec;

	int bys, pos;
	bool_t rt = 1;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	if (!pses->handshake_over)
	{
		if (pdtls->type == DTLS_TYPE_CLIENT && !_dtls_handshake_client(pdtls))
		{
			_dtls_write_close(pdtls);
			return 0;
		}

		if (pdtls->type == DTLS_TYPE_SERVER && !_dtls_handshake_server(pdtls))
		{
			_dtls_write_close(pdtls);
			return 0;
		}

		//clear handshake
		prec = (dtls_record_context*)(pses->rcv_record);
		prec->rcv_msg_type = 0;
		prec->rcv_msg_len = 0;
		prec->rcv_msg_pop = 0;

		prec = (dtls_record_context*)(pses->snd_record);
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
		rt = _dtls_read_data(pdtls, buf + pos, &bys);
		if (!rt)
			break;

		if (!bys)
			break;

		pos += bys;
		//need break
		break;
	}

	*pb = pos;

	return (*pb)? 1 : 0;
}

unsigned short xdtls_addr_port(xhand_t dtls, tchar_t* addr)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	net_addr_t na = { 0 };
	unsigned short port;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	socket_addr(xdtls_socket(dtls), &na);
	conv_addr(&na, &port, addr);

	return port;
}

unsigned short xdtls_peer_port(xhand_t dtls, tchar_t* addr)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	net_addr_t na = { 0 };
	unsigned short port;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	socket_peer(xdtls_socket(dtls), &na);
	conv_addr(&na, &port, addr);

	return port;
}

bool_t xdtls_setopt(xhand_t dtls, int oid, void* opt, int len)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	switch (oid)
	{
	case SOCK_OPTION_SNDBUF:
		socket_set_sndbuf(xdtls_socket(dtls), *(int*)opt);
		return 1;
	case SOCK_OPTION_RCVBUF:
		socket_set_rcvbuf(xdtls_socket(dtls), *(int*)opt);
		return 1;
	case SOCK_OPTION_NONBLK:
		socket_set_nonblk(xdtls_socket(dtls), *(bool_t*)opt);
		return 1;
	}

	return 0;
}

void xdtls_set_host(xhand_t dtls, const tchar_t* host_cn)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;

	int len;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	XDK_ASSERT(pdtls->type == DTLS_TYPE_CLIENT);

#ifdef _UNICODE
	len = ucs_to_mbs(host_cn, -1, NULL, MAX_LONG);
	psec->host_cn = a_xsalloc(len + 1);
	ucs_to_mbs(host_cn, -1, psec->host_cn, len);
#else
	len = a_xslen(host_cn);
	psec->host_cn = a_xsnclone(host_cn, len);
#endif
}

void xdtls_set_peer(xhand_t dtls, const tchar_t* peer_cn)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;

	int len;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	XDK_ASSERT(pdtls->type == DTLS_TYPE_SERVER);

#ifdef _UNICODE
	len = ucs_to_mbs(peer_cn, -1, NULL, MAX_LONG);
	psec->peer_cn = a_xsalloc(len + 1);
	ucs_to_mbs(peer_cn, -1, psec->peer_cn, len);
#else
	len = a_xslen(peer_cn);
	psec->peer_cn = a_xsnclone(peer_cn, len);
#endif
}

bool_t xdtls_set_ca(xhand_t dtls, const byte_t* sz_cert, dword_t clen)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

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

bool_t xdtls_set_cert(xhand_t dtls, const byte_t* sz_cert, dword_t clen)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;
	dtls_security_context* psec = (dtls_security_context*)pdtls->security_context;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

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

bool_t xdtls_set_rsa(xhand_t dtls, const byte_t* sz_rsa, dword_t rlen, const tchar_t* sz_pwd, int len)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_security_context* psec = (dtls_security_context*)(pdtls->security_context);

	byte_t buf_pwd[RES_LEN + 1] = { 0 };
	dword_t dw;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

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

bool_t xdtls_set_dhm(xhand_t dtls, const byte_t* sz_dhm, dword_t dlen)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_security_context* psec = (dtls_security_context*)(pdtls->security_context);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

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

void xdtls_set_verify(xhand_t dtls, int srv_verify)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_security_context* psec = (dtls_security_context*)(pdtls->security_context);

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	XDK_ASSERT(pdtls->type == DTLS_TYPE_SERVER);

	psec->verify_mode = srv_verify;
}

void xdtls_set_version(xhand_t dtls, int cli_ver)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	XDK_ASSERT(pdtls->type == DTLS_TYPE_CLIENT);

	pdtls->cli_minor_ver = GETLBYTE(cli_ver);
	pdtls->cli_major_ver = GETHBYTE(cli_ver);

	pses->minor_ver = pdtls->cli_minor_ver;
}

void xdtls_set_package(xhand_t dtls, dword_t pkg_size)
{
	dtls_context* pdtls = TypePtrFromHead(dtls_context, dtls);
	dtls_session_context* pses = (dtls_session_context*)pdtls->session_context;

	XDK_ASSERT(dtls && dtls->tag == _HANDLE_DTLS);

	pses->pkg_size = pkg_size;
}

#endif //XDK_SUPPORT_SOCK
