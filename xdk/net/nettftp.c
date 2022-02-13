/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdn tftp document

	@module	nettftp.c | implement file

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

#include "nettftp.h"

#include "../xdknet.h"
#include "../xdkimp.h"
#include "../xdkoem.h"
#include "../xdkstd.h"
#include "../xdkutil.h"

#if defined(XDK_SUPPORT_SOCK)

#define TFTP_WIN_SIZE		64

#define TFTP_MAX_TRY		3
#define TFTP_MAX_TIMO		500

typedef struct _tftp_pdu_context{
	int type;

	tchar_t method[INT_LEN + 1];
	tchar_t file[PATH_LEN + 1];
	tchar_t mode[NUM_LEN + 1];

	dword_t size;
	sword_t isdir;
	tchar_t ftime[DATE_LEN + 1];

	int errcode;
	tchar_t errtext[ERR_LEN + 1];

	int pdv_num;
	int pdv_off;
	int pdv_len;
	byte_t payload[TFTP_PDV_SIZE];
}tftp_pdu_context;

typedef struct _tftp_context{
	handle_head head;		//head for xhand_t
	int type;

	int secu;
	bio_interface* pif;

	havege_state havs;

	tftp_pdu_context snd_pdu;
	tftp_pdu_context rcv_pdu;

	int nxt_snd_num;
	int pre_rcv_num;

	int snd_eof;
	int rcv_eof;

	linear_t snd_line;
	linear_t rcv_line;

	sword_t errcode;
	tchar_t errtext[ERR_LEN + 1];
}tftp_context;

/***********************************************************************************************/

static unsigned short _tftp_port(tftp_context* pftp)
{
	unsigned short port = 0;

	while (port < UDP_MIN_PORT || port > UDP_MAX_PORT)
	{
		port = (unsigned short)havege_rand(&pftp->havs);
	}

	return port;
}

void _tftp_error(int errcode, tchar_t* errtext)
{
	switch (errcode)
	{
	case TFTP_CODE_NOTDEF:
		xscpy(errtext, TFTP_CODE_NOTDEF_TEXT);
		break;
	case TFTP_CODE_NOTFIND:
		xscpy(errtext, TFTP_CODE_NOTFIND_TEXT);
		break;
	case TFTP_CODE_REJECT:
		xscpy(errtext, TFTP_CODE_REJECT_TEXT);
		break;
	case TFTP_CODE_DSKFULL:
		xscpy(errtext, TFTP_CODE_DSKFULL_TEXT);
		break;
	case TFTP_CODE_UNTID:
		xscpy(errtext, TFTP_CODE_UNTID_TEXT);
		break;
	case TFTP_CODE_EXISTS:
		xscpy(errtext, TFTP_CODE_EXISTS_TEXT);
		break;
	case TFTP_CODE_NOUSER:
		xscpy(errtext, TFTP_CODE_NOUSER_TEXT);
		break;
	}
}

static dword_t _tftp_parse_pdu(const byte_t* buf, dword_t size, tftp_pdu_context* pdu)
{
	dword_t total = 0;
	int n, len;

	if (!size) return 0;

	pdu->type = GET_SWORD_LIT(buf, total);
	total += 2;

	switch (pdu->type)
	{
	case TFTP_PDU_HEAD:
		pdu->size = GET_DWORD_LIT(buf, total);
		total += 4;

		pdu->isdir = GET_SWORD_LIT(buf, total);
		total += 2;

		len = a_xslen((schar_t*)(buf + total));
#ifdef _UNICODE
		n = utf8_to_ucs((buf + total), len, pdu->ftime, DATE_LEN);
#else
		n = utf8_to_mbs((buf + total), len, pdu->ftime, DATE_LEN);
#endif
		pdu->ftime[n] = _T('\0');
		total += (len + 1);

		len = a_xslen((schar_t*)(buf + total));
#ifdef _UNICODE
		n = utf8_to_ucs((buf + total), len, pdu->file, PATH_LEN);
#else
		n = utf8_to_mbs((buf + total), len, pdu->file, PATH_LEN);
#endif
		pdu->file[n] = _T('\0');
		total += (len + 1);

		break;
	case TFTP_PDU_DEL:
		len = a_xslen((schar_t*)(buf + total));
#ifdef _UNICODE
		n = utf8_to_ucs((buf + total), len, pdu->file, PATH_LEN);
#else
		n = utf8_to_mbs((buf + total), len, pdu->file, PATH_LEN);
#endif
		pdu->file[n] = _T('\0');
		total += (len + 1);

		break;
	case TFTP_PDU_RRQ:
		len = a_xslen((schar_t*)(buf + total));
#ifdef _UNICODE
		n = utf8_to_ucs((buf + total), len, pdu->file, PATH_LEN);
#else
		n = utf8_to_mbs((buf + total), len, pdu->file, PATH_LEN);
#endif
		pdu->file[n] = _T('\0');
		total += (len + 1);

		len = a_xslen((schar_t*)(buf + total));
#ifdef _UNICODE
		n = utf8_to_ucs((buf + total), len, pdu->mode, NUM_LEN);
#else
		n = utf8_to_mbs((buf + total), len, pdu->mode, NUM_LEN);
#endif
		pdu->mode[n] = _T('\0');
		total += (len + 1);

		break;
	case TFTP_PDU_WRQ:
		len = a_xslen((schar_t*)(buf + total));
#ifdef _UNICODE
		n = utf8_to_ucs((buf + total), len, pdu->file, PATH_LEN);
#else
		n = utf8_to_mbs((buf + total), len, pdu->file, PATH_LEN);
#endif
		pdu->file[n] = _T('\0');
		total += (len + 1);

		len = a_xslen((schar_t*)(buf + total));
#ifdef _UNICODE
		n = utf8_to_ucs((buf + total), len, pdu->mode, NUM_LEN);
#else
		n = utf8_to_mbs((buf + total), len, pdu->mode, NUM_LEN);
#endif
		pdu->mode[n] = _T('\0');
		total += (len + 1);

		break;
	case TFTP_PDU_DATA:
		pdu->pdv_num = GET_SWORD_LIT(buf, total);
		total += 2;

		//payload
		pdu->pdv_len = size - total;
		xmem_copy((void*)(pdu->payload), (void*)(buf + total), pdu->pdv_len);
		total += pdu->pdv_len;
		break;
	case TFTP_PDU_ACK:
		pdu->pdv_num = GET_SWORD_LIT(buf, total);
		total += 2;

		break;
	case TFTP_PDU_ERR:
		pdu->errcode = GET_SWORD_LIT(buf, total);
		total += 2;

		len = a_xslen((schar_t*)(buf + total));
#ifdef _UNICODE
		n = utf8_to_ucs((buf + total), len, pdu->errtext, ERR_LEN);
#else
		n = utf8_to_mbs((buf + total), len, pdu->errtext, ERR_LEN);
#endif
		pdu->errtext[n] = _T('\0');
		total += (len + 1);

		break;
	}

	return total;
}

static dword_t _tftp_format_pdu(byte_t* buf, dword_t size, tftp_pdu_context* pdu)
{
	dword_t dw, total = 0;

	PUT_SWORD_LIT(buf, total, pdu->type);
	total += 2;

	switch (pdu->type)
	{
	case TFTP_PDU_HEAD:
		PUT_DWORD_LIT(buf, total, pdu->size);
		total += 4;

		PUT_SWORD_LIT(buf, total, pdu->isdir);
		total += 2;

#ifdef _UNICODE
		dw = ucs_to_utf8(pdu->ftime, -1, (buf + total), TFTP_PDV_SIZE - total);
#else
		dw = mbs_to_utf8(pdu->ftime, -1, (buf + total), TFTP_PDV_SIZE - total);
#endif
		total += dw;

		buf[total] = '\0';
		total++;

#ifdef _UNICODE
		dw = ucs_to_utf8(pdu->file, -1, (buf + total), TFTP_PDV_SIZE - total);
#else
		dw = mbs_to_utf8(pdu->file, -1, (buf + total), TFTP_PDV_SIZE - total);
#endif
		total += dw;

		buf[total] = '\0';
		total++;

		break;
	case TFTP_PDU_DEL:
#ifdef _UNICODE
		dw = ucs_to_utf8(pdu->file, -1, (buf + total), TFTP_PDV_SIZE - total);
#else
		dw = mbs_to_utf8(pdu->file, -1, (buf + total), TFTP_PDV_SIZE - total);
#endif
		total += dw;

		buf[total] = '\0';
		total++;

		break;
	case TFTP_PDU_RRQ:
#ifdef _UNICODE
		dw = ucs_to_utf8(pdu->file, -1, (buf + total), TFTP_PDV_SIZE - total);
#else
		dw = mbs_to_utf8(pdu->file, -1, (buf + total), TFTP_PDV_SIZE - total);
#endif
		total += dw;
		
		buf[total] = '\0';
		total++;

#ifdef _UNICODE
		dw = ucs_to_utf8(pdu->mode, -1, (buf + total), TFTP_PDV_SIZE - total);
#else
		dw = mbs_to_utf8(pdu->mode, -1, (buf + total), TFTP_PDV_SIZE - total);
#endif
		total += dw;

		buf[total] = '\0';
		total++;

		break;
	case TFTP_PDU_WRQ:
#ifdef _UNICODE
		dw = ucs_to_utf8(pdu->file, -1, (buf + total), TFTP_PDV_SIZE - total);
#else
		dw = mbs_to_utf8(pdu->file, -1, (buf + total), TFTP_PDV_SIZE - total);
#endif
		total += dw;

		buf[total] = '\0';
		total++;

#ifdef _UNICODE
		dw = ucs_to_utf8(pdu->mode, -1, (buf + total), TFTP_PDV_SIZE - total);
#else
		dw = mbs_to_utf8(pdu->mode, -1, (buf + total), TFTP_PDV_SIZE - total);
#endif
		total += dw;

		buf[total] = '\0';
		total++;

		break;
	case TFTP_PDU_DATA:
		PUT_SWORD_LIT(buf, total, pdu->pdv_num);
		total += 2;

		xmem_copy((void*)(buf + total), (void*)pdu->payload, pdu->pdv_len);
		total += pdu->pdv_len;
		break;
	case TFTP_PDU_ACK:
		PUT_SWORD_LIT(buf, total, pdu->pdv_num);
		total += 2;

		break;
	case TFTP_PDU_ERR:
		PUT_SWORD_LIT(buf, total, pdu->errcode);
		total += 2;

#ifdef _UNICODE
		dw = ucs_to_utf8(pdu->errtext, -1, (buf + total), TFTP_PDV_SIZE - total);
#else
		dw = mbs_to_utf8(pdu->errtext, -1, (buf + total), TFTP_PDV_SIZE - total);
#endif
		total += dw;

		buf[total] = '\0';
		total++;

		break;
	}

	return total;
}

static void _tftp_clear_pdv(tftp_pdu_context* pdu)
{
	xmem_zero((void*)pdu->payload, TFTP_PDV_SIZE);
	pdu->pdv_len = 0;
}

static void _tftp_clear_pdu(tftp_pdu_context* pdu)
{
	_tftp_clear_pdv(pdu);

	xmem_zero((void*)pdu, sizeof(tftp_pdu_context));
}


/****************************************************************************************************/

bool_t _tftp_send_request(tftp_context* ppt)
{
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len;
	byte_t* lin_buf;

	XDK_ASSERT(ppt->type == _XTFTP_TYPE_CLI);

	TRY_CATCH;

	pdu = &ppt->snd_pdu;

	if (pdu->type == TFTP_PDU_DATA)
	{
		pdu->pdv_num = ppt->nxt_snd_num;
	}
	else if (pdu->type == TFTP_PDU_ACK)
	{
		pdu->pdv_num = ppt->pre_rcv_num;
	}

	len = _tftp_format_pdu(pkg_buf, TFTP_PKG_SIZE, pdu);

	if (!(*ppt->pif->pf_write)(ppt->pif->fd, pkg_buf, &len))
	{
		raise_user_error(_T("_tftp_send_request"), _T("bio write falied"));
	}

	(*ppt->pif->pf_flush)(ppt->pif->fd);

	if (pdu->type == TFTP_PDU_DATA)
	{
		lin_buf = insert_linear_frame(ppt->snd_line, pdu->pdv_num, pdu->pdv_len);
		if (!lin_buf)
		{
			raise_user_error(_T("_tftp_send_request"), _T("linear insert falied"));
		}
		xmem_copy((void*)lin_buf, (void*)(pdu->payload), pdu->pdv_len);

		if (pdu->pdv_len < TFTP_PDV_SIZE)
			ppt->snd_eof = 1;

		xmem_zero((void*)pdu->payload, pdu->pdv_len);
		pdu->pdv_len = 0;
	}

	if (pdu->type == TFTP_PDU_WRQ || pdu->type == TFTP_PDU_DATA)
	{
		ppt->nxt_snd_num++;
	}

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	return 0;
}

bool_t _tftp_replay_request(tftp_context* ppt, int pdvnum)
{
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len;

	byte_t* lin_buf;

	XDK_ASSERT(ppt->type == _XTFTP_TYPE_CLI);

	TRY_CATCH;

	pdu = &ppt->snd_pdu;
	pdu->type = TFTP_PDU_DATA;
	pdu->pdv_num = pdvnum;

	lin_buf = get_linear_frame(ppt->snd_line, pdvnum, &pdu->pdv_len);
	if (!lin_buf)
	{
		raise_user_error(_T("_tftp_replay_request"), _T("linear get failed"));
	}
	xmem_copy((void*)(pdu->payload), (void*)lin_buf, pdu->pdv_len);

	len = _tftp_format_pdu(pkg_buf, TFTP_PKG_SIZE, pdu);

	if (!(*ppt->pif->pf_write)(ppt->pif->fd, pkg_buf, &len))
	{
		raise_user_error(_T("_tftp_replay_request"), _T("bio write failed"));
	}

	(*ppt->pif->pf_flush)(ppt->pif->fd);

	xmem_zero((void*)pdu->payload, pdu->pdv_len);
	pdu->pdv_len = 0;

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	return 0;
}

static void _tftp_clear_request(tftp_context* ppt, int pdvnum)
{
	clean_linear_frame(ppt->snd_line, pdvnum);
}

bool_t _tftp_recv_request(tftp_context* ppt)
{
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len;
	byte_t* lin_buf;

	XDK_ASSERT(ppt->type == _XTFTP_TYPE_SRV);

	TRY_CATCH;

	len = TFTP_PKG_SIZE;

	if (!(*ppt->pif->pf_read)(ppt->pif->fd, pkg_buf, &len))
	{
		raise_user_error(_T("_tftp_recv_request"), _T("bio read failed"));
	}

	pdu = &ppt->rcv_pdu;

	_tftp_clear_pdu(pdu);

	len = _tftp_parse_pdu(pkg_buf, len, pdu);

	if (!len)
	{
		raise_user_error(_T("_tftp_recv_request"), _T("empty package"));
	}

	switch (pdu->type)
	{
	case TFTP_PDU_WRQ:
		xscpy(pdu->method, TFTP_METHOD_PUT);
		break;
	case TFTP_PDU_RRQ:
		xscpy(pdu->method, TFTP_METHOD_GET);
		break;
	case TFTP_PDU_HEAD:
		xscpy(pdu->method, TFTP_METHOD_HEAD);
		break;
	case TFTP_PDU_DEL:
		xscpy(pdu->method, TFTP_METHOD_DELETE);
		break;
	default:
		break;
	}

	if (pdu->type == TFTP_PDU_ERR)
	{
		raise_user_error(_T("_tftp_recv_request"), pdu->errtext);
	}

	if (pdu->type == TFTP_PDU_DATA || pdu->type == TFTP_PDU_ACK)
	{
		lin_buf = insert_linear_frame(ppt->rcv_line, pdu->pdv_num, pdu->pdv_len);
		if (!lin_buf)
		{
			raise_user_error(_T("_tftp_recv_request"), _T("linear insert failed"));
		}
		xmem_copy((void*)(lin_buf), (void*)(pdu->payload), pdu->pdv_len);

		xmem_zero((void*)pdu->payload, pdu->pdv_len);
		pdu->pdv_len = 0;

		lin_buf = get_linear_frame(ppt->rcv_line, ppt->pre_rcv_num + 1, &pdu->pdv_len);
		if (!lin_buf)
		{
			raise_user_error(_T("_tftp_recv_request"), _T("linear get failed"));
		}
		xmem_copy((void*)(pdu->payload), (void*)lin_buf, pdu->pdv_len);

		clean_linear_frame(ppt->rcv_line, ppt->pre_rcv_num + 1);

		ppt->pre_rcv_num++;

		if ((pdu->type == TFTP_PDU_DATA && pdu->pdv_len < TFTP_PDV_SIZE) || (pdu->type == TFTP_PDU_ACK && ppt->pre_rcv_num > 1 && ppt->pre_rcv_num == ppt->nxt_snd_num - 1))
			ppt->rcv_eof = 1;
	}
	else
	{
		ppt->pre_rcv_num++;
	}

	END_CATCH;

	return 1;
ONERROR:

	return 0;
}

bool_t _tftp_send_response(tftp_context* ppt)
{
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len;
	byte_t* lin_buf;

	XDK_ASSERT(ppt->type == _XTFTP_TYPE_SRV);

	TRY_CATCH;

	pdu = &ppt->snd_pdu;

	if (pdu->type == TFTP_PDU_DATA)
	{
		pdu->pdv_num = ppt->nxt_snd_num;
	}
	else if (pdu->type == TFTP_PDU_ACK)
	{
		pdu->pdv_num = ppt->pre_rcv_num;
	}

	len = _tftp_format_pdu(pkg_buf, TFTP_PKG_SIZE, pdu);

	if (!(*ppt->pif->pf_write)(ppt->pif->fd, pkg_buf, &len))
	{
		raise_user_error(_T("_tftp_send_response"), _T("bio write falied"));
	}

	(*ppt->pif->pf_flush)(ppt->pif->fd);

	if (pdu->type == TFTP_PDU_DATA)
	{
		lin_buf = insert_linear_frame(ppt->snd_line, pdu->pdv_num, pdu->pdv_len);
		if (!lin_buf)
		{
			raise_user_error(_T("_tftp_send_response"), _T("linear insert falied"));
		}
		xmem_copy((void*)(lin_buf), (void*)(pdu->payload), pdu->pdv_len);

		if (pdu->pdv_len < TFTP_PDV_SIZE)
			ppt->snd_eof = 1;

		ppt->nxt_snd_num++;

		xmem_zero((void*)pdu->payload, pdu->pdv_len);
		pdu->pdv_len = 0;
	}

	END_CATCH;

	return 1;
ONERROR:

	return 0;
}

bool_t _tftp_replay_response(tftp_context* ppt, int pdvnum)
{
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len;

	byte_t* lin_buf;

	XDK_ASSERT(ppt->type == _XTFTP_TYPE_SRV);

	TRY_CATCH;

	pdu = &ppt->snd_pdu;
	pdu->type = TFTP_PDU_DATA;
	pdu->pdv_num = pdvnum;

	lin_buf = get_linear_frame(ppt->snd_line, pdvnum, &pdu->pdv_len);
	if (!lin_buf)
	{
		raise_user_error(_T("_tftp_replay_response"), _T("linear get failed"));
	}
	xmem_copy((void*)(pdu->payload), (void*)lin_buf, pdu->pdv_len);

	len = _tftp_format_pdu(pkg_buf, TFTP_PKG_SIZE, pdu);

	if (!(*ppt->pif->pf_write)(ppt->pif->fd, pkg_buf, &len))
	{
		raise_user_error(_T("_tftp_replay_response"), _T("bio write failed"));
	}

	(*ppt->pif->pf_flush)(ppt->pif->fd);

	xmem_zero((void*)pdu->payload, pdu->pdv_len);
	pdu->pdv_len = 0;

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	return 0;
}

static void _tftp_clear_response(tftp_context* ppt, int pdvnum)
{
	clean_linear_frame(ppt->snd_line, pdvnum);
}

bool_t _tftp_recv_response(tftp_context* ppt)
{
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len;
	byte_t* lin_buf;

	XDK_ASSERT(ppt->type == _XTFTP_TYPE_CLI);

	TRY_CATCH;

	len = TFTP_PKG_SIZE;
	if (!(*ppt->pif->pf_read)(ppt->pif->fd, pkg_buf, &len))
	{
		raise_user_error(_T("_tftp_recv_response"), _T("bio read failed"));
	}

	pdu = &ppt->rcv_pdu;

	_tftp_clear_pdu(pdu);

	len = _tftp_parse_pdu(pkg_buf, len, pdu);

	if (!len)
	{
		raise_user_error(_T("_tftp_recv_response"), _T("empty package"));
	}

	if (pdu->type == TFTP_PDU_ERR)
	{
		raise_user_error(_T("_tftp_recv_response"), pdu->errtext);
	}

	if (pdu->type == TFTP_PDU_DATA || pdu->type == TFTP_PDU_ACK)
	{
		lin_buf = insert_linear_frame(ppt->rcv_line, pdu->pdv_num, pdu->pdv_len);
		if (!lin_buf)
		{
			raise_user_error(_T("_tftp_recv_response"), _T("linear insert failed"));
		}
		xmem_copy((void*)(lin_buf), (void*)(pdu->payload), pdu->pdv_len);

		xmem_zero((void*)pdu->payload, pdu->pdv_len);
		pdu->pdv_len = 0;

		lin_buf = get_linear_frame(ppt->rcv_line, ppt->pre_rcv_num + 1, &pdu->pdv_len);
		if (!lin_buf)
		{
			raise_user_error(_T("_tftp_recv_response"), _T("linear get failed"));
		}
		xmem_copy((void*)(pdu->payload), (void*)lin_buf, pdu->pdv_len);

		clean_linear_frame(ppt->rcv_line, ppt->pre_rcv_num + 1);

		ppt->pre_rcv_num++;

		if ((pdu->type == TFTP_PDU_DATA  && pdu->pdv_len < TFTP_PDV_SIZE) || (pdu->type == TFTP_PDU_ACK  && ppt->pre_rcv_num > 1 && ppt->pre_rcv_num == ppt->nxt_snd_num - 1))
			ppt->rcv_eof = 1;
	}

	END_CATCH;

	return 1;
ONERROR:

	return 0;
}

/***********************************************************************************************/

xhand_t xtftp_client(const tchar_t* method, const tchar_t* url)
{
	tftp_context* pftp = NULL;

	tchar_t *potoat, *hostat, *portat, *objat, *qryat;
	int potolen, hostlen, portlen, objlen, qrylen;

	tchar_t host[META_LEN + 1] = { 0 };
	tchar_t addr[ADDR_LEN + 1] = { 0 };
	unsigned short port, bind;

	tftp_pdu_context* pdu = NULL;
	xhand_t bio = NULL;

	TRY_CATCH;

	pftp = (tftp_context*)xmem_alloc(sizeof(tftp_context));
	pftp->head.tag = _HANDLE_TFTP;

	havege_init(&pftp->havs);
	bind = _tftp_port(pftp);

	pftp->type = _XTFTP_TYPE_CLI;

	parse_url(url, &potoat, &potolen, &hostat, &hostlen, &portat, &portlen, &objat, &objlen, &qryat, &qrylen);

	if (compare_text(potoat, potolen, _T("tftps"), -1, 1) == 0)
		pftp->secu = _SECU_DTLS;
	else
		pftp->secu = _SECU_NONE;

	xsncpy(host, hostat, hostlen);
	if (is_ip(host))
	{
		xscpy(addr, host);
	}
	else
	{
		host_addr(host, addr);
	}

	port = xsntos(portat, portlen);
	if (!port)
		port = DEF_TFTP_PORT;

	if (pftp->secu == _SECU_DTLS)
	{
		bio = xdtls_cli(port, addr);
	}
	else
	{
		bio = xudp_cli(port, addr);
	}

	if (!bio)
	{
		raise_user_error(NULL, NULL);
	}

	if (pftp->secu == _SECU_DTLS)
	{
		if (!xdtls_bind(bio, bind))
		{
			raise_user_error(NULL, NULL);
		}

		xdtls_set_package(bio, TFTP_PKG_SIZE);
	}
	else
	{
		if (!xudp_bind(bio, bind))
		{
			raise_user_error(NULL, NULL);
		}

		xudp_set_package(bio, TFTP_PKG_SIZE);
	}

	pftp->pif = (bio_interface*)xmem_alloc(sizeof(bio_interface));
	get_bio_interface(bio, pftp->pif);
	bio = NULL;

	pftp->snd_line = alloc_linear(TFTP_WIN_SIZE);
	pftp->rcv_line = alloc_linear(TFTP_WIN_SIZE);

	pdu = &pftp->snd_pdu;

	xscpy(pdu->method, method);
	xsncpy(pdu->file, objat, objlen);
	xscpy(pdu->mode, _T("octet"));

	END_CATCH;

	return &pftp->head;

ONERROR:
	XDK_TRACE_LAST;

	if (bio)
		xudp_close(bio);

	if (pftp)
	{
		if (pftp->pif)
			xmem_free(pftp->pif);

		if (pftp->snd_line)
			free_linear(pftp->snd_line);
		if (pftp->rcv_line)
			free_linear(pftp->rcv_line);

		xmem_free(pftp);
	}

	return NULL;
}

xhand_t	xtftp_server(xhand_t bio)
{
	tftp_context* pftp = NULL;
	unsigned short bind;

	TRY_CATCH;

	XDK_ASSERT(bio != NULL);

	pftp = (tftp_context*)xmem_alloc(sizeof(tftp_context));
	pftp->head.tag = _HANDLE_TFTP;

	pftp->type = _XTFTP_TYPE_SRV;

	havege_init(&pftp->havs);
	bind = _tftp_port(pftp);

	pftp->secu = (bio->tag == _HANDLE_DTLS) ? _SECU_DTLS : _SECU_NONE;

	pftp->pif = (bio_interface*)xmem_alloc(sizeof(bio_interface));
	get_bio_interface(bio, pftp->pif);

	if (pftp->secu == _SECU_DTLS)
	{
		if (!xdtls_bind(bio, bind))
		{
			raise_user_error(NULL, NULL);
		}

		xdtls_set_package(bio, TFTP_PKG_SIZE);
	}
	else
	{
		if (!xudp_bind(bio, bind))
		{
			raise_user_error(NULL, NULL);
		}

		xudp_set_package(bio, TFTP_PKG_SIZE);
	}

	pftp->snd_line = alloc_linear(TFTP_WIN_SIZE);
	pftp->rcv_line = alloc_linear(TFTP_WIN_SIZE);

	END_CATCH;

	return &pftp->head;
ONERROR:
	XDK_TRACE_LAST;

	if (pftp)
	{
		if (pftp->pif)
			xmem_free(pftp->pif);

		if (pftp->snd_line)
			free_linear(pftp->snd_line);
		if (pftp->rcv_line)
			free_linear(pftp->rcv_line);

		xmem_free(pftp);
	}

	return NULL;
}

xhand_t xtftp_bio(xhand_t tftp)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	return (pftp->pif)? pftp->pif->fd : NULL;
}

void xtftp_close(xhand_t tftp)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	if (pftp->pif)
	{
		(*pftp->pif->pf_close)(pftp->pif->fd);
		xmem_free(pftp->pif);
	}

	if (pftp->snd_line)
		free_linear(pftp->snd_line);
	if (pftp->rcv_line)
		free_linear(pftp->rcv_line);

	xmem_free(pftp);
}

bool_t xtftp_connect(xhand_t tftp)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);

	int TRY = TFTP_MAX_TRY;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	tftp_pdu_context* pdu;

	TRY_CATCH;

	pdu = &pftp->snd_pdu;

	if (compare_text(pdu->method, -1, TFTP_METHOD_PUT, -1, 1) == 0)
	{
		pftp->nxt_snd_num = 0;
		pftp->pre_rcv_num = -1;
		pftp->snd_eof = pftp->rcv_eof = 0;

		pdu->type = TFTP_PDU_WRQ;
		if (!_tftp_send_request(pftp))
		{
			raise_user_error(_T("xtftp_connect"), _T("send WRQ request failed"));
		}

		while (!_tftp_recv_response(pftp) && TRY--)
		{
			thread_sleep(TFTP_MAX_TIMO);
		}

		if (pftp->rcv_pdu.type != TFTP_PDU_ACK)
		{
			raise_user_error(_T("xtftp_connect"), _T("recv WRQ ACK failed"));
		}
	}
	else if (compare_text(pdu->method, -1, TFTP_METHOD_GET, -1, 1) == 0)
	{
		pftp->nxt_snd_num = 0;
		pftp->pre_rcv_num = 0;
		pftp->snd_eof = pftp->rcv_eof = 0;

		pdu->type = TFTP_PDU_RRQ;
		if (!_tftp_send_request(pftp))
		{
			raise_user_error(_T("xtftp_connect"), _T("send GET request failed"));
		}
	}
	else if (compare_text(pdu->method, -1, TFTP_METHOD_HEAD, -1, 1) == 0)
	{
		pftp->nxt_snd_num = 0;
		pftp->pre_rcv_num = -1;
		pftp->snd_eof = pftp->rcv_eof = 0;

		pdu->type = TFTP_PDU_HEAD;
		if (!_tftp_send_request(pftp))
		{
			raise_user_error(_T("xtftp_connect"), _T("send HEAD request failed"));
		}
	}
	else if (compare_text(pdu->method, -1, TFTP_METHOD_DELETE, -1, 1) == 0)
	{
		pftp->nxt_snd_num = 0;
		pftp->pre_rcv_num = -1;
		pftp->snd_eof = pftp->rcv_eof = 0;

		pdu->type = TFTP_PDU_DEL;
		if (!_tftp_send_request(pftp))
		{
			raise_user_error(_T("xtftp_connect"), _T("send DEL request failed"));
		}
	}
	else
	{
		raise_user_error(_T("xtftp_connect"), _T("invalid operator"));
	}

	END_CATCH;

	return 1;

ONERROR:
	XDK_TRACE_LAST;

	return 0;
}

bool_t	xtftp_accept(xhand_t tftp)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);

	tftp_pdu_context* pdu;
	int pdu_type;
	int TRY = TFTP_MAX_TRY;

	TRY_CATCH;

	while (TRY-- && !_tftp_recv_request(pftp))
	{
		thread_sleep(TFTP_MAX_TIMO);
	}

	if (TRY<0)
	{
		raise_user_error(_T("xtftp_accept"), _T("recv request failed"));
	}

	pdu_type = pftp->rcv_pdu.type;

	pdu = &pftp->snd_pdu;

	if (pdu_type == TFTP_PDU_WRQ)
	{
		pftp->nxt_snd_num = 0;
		pftp->pre_rcv_num = 0;
		pftp->snd_eof = pftp->rcv_eof = 0;

		pdu->type = TFTP_PDU_ACK;
		if (!_tftp_send_response(pftp))
		{
			raise_user_error(_T("xtftp_accept"), _T("send ACK failed"));
		}
	}
	else if (pdu_type == TFTP_PDU_RRQ)
	{
		pftp->nxt_snd_num = 1;
		pftp->pre_rcv_num = 0;
		pftp->snd_eof = pftp->rcv_eof = 0;
	}
	else if (pdu_type == TFTP_PDU_HEAD)
	{
		pftp->nxt_snd_num = 0;
		pftp->pre_rcv_num = 0;
		pftp->snd_eof = pftp->rcv_eof = 0;
	}
	else if (pdu_type == TFTP_PDU_DEL)
	{
		pftp->nxt_snd_num = 0;
		pftp->pre_rcv_num = 0;
		pftp->snd_eof = pftp->rcv_eof = 0;
	}

	END_CATCH;

	return 1;
ONERROR:
	XDK_TRACE_LAST;

	return 0;
}

bool_t xtftp_recv(xhand_t tftp, byte_t* buf, dword_t* pch)
{
	tftp_context* ppt = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len, pos = 0;

	int mw, TRY = TFTP_MAX_TRY;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	TRY_CATCH;

	mw = get_linear_window(ppt->snd_line);

	pdu = &ppt->rcv_pdu;

	while (pos < *pch)
	{
		len = ((*pch - pos) < (pdu->pdv_len - pdu->pdv_off)) ? (*pch - pos) : (pdu->pdv_len - pdu->pdv_off);
		xmem_copy((void*)(buf + pos), (void*)(pdu->payload + pdu->pdv_off), len);
		pdu->pdv_off += len;
		pos += len;

		if (pos == *pch)
			break;

		TRY = TFTP_MAX_TRY;

		if ((pdu->pdv_len == pdu->pdv_off) && !ppt->rcv_eof)
		{
			pdu->pdv_off = 0;

			if (ppt->type == _XTFTP_TYPE_SRV)
			{
				while (TRY-- && !_tftp_recv_request(ppt))
				{
					thread_sleep(TFTP_MAX_TIMO);
				}

				if (TRY < 0)
				{
					raise_user_error(_T("xtftp_recv"), _T("recv timeout"));
				}

				if (pdu->type != TFTP_PDU_DATA)
				{
					raise_user_error(_T("xtftp_recv"), _T("recv data failed"));
				}

				ppt->snd_pdu.type = TFTP_PDU_ACK;

				if (!_tftp_send_response(ppt))
				{
					raise_user_error(NULL, NULL);
				}
			}
			else if (ppt->type == _XTFTP_TYPE_CLI)
			{
				while (TRY-- && !_tftp_recv_response(ppt))
				{
					thread_sleep(TFTP_MAX_TIMO);
				}

				if (TRY < 0)
				{
					raise_user_error(_T("xtftp_recv"), _T("recv timeout"));
				}

				if (pdu->type != TFTP_PDU_DATA)
				{
					raise_user_error(_T("xtftp_recv"), _T("recv data failed"));
				}

				ppt->snd_pdu.type = TFTP_PDU_ACK;

				if (!_tftp_send_request(ppt))
				{
					raise_user_error(NULL, NULL);
				}
			}
		}
		else if (!len)
		{
			break;
		}
	}

	*pch = pos;

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	*pch = 0;

	return 0;
}

bool_t xtftp_send(xhand_t tftp, const byte_t* buf, dword_t *pch)
{
	tftp_context* ppt = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len, pos = 0;

	int mw, TRY;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	TRY_CATCH;

	mw = get_linear_window(ppt->snd_line);

	pdu = &ppt->snd_pdu;

	while (pos < *pch)
	{
		pdu->pdv_len = TFTP_PDV_SIZE;

		len = ((*pch - pos) < (pdu->pdv_len - pdu->pdv_off)) ? (*pch - pos) : (pdu->pdv_len - pdu->pdv_off);
		xmem_copy((void*)(pdu->payload + pdu->pdv_off), (void*)(buf + pos), len);
		pdu->pdv_off += len;
		pos += len;

		if (pdu->pdv_off == pdu->pdv_len)
		{
			pdu->pdv_off = 0;

			if (ppt->type == _XTFTP_TYPE_SRV)
			{
				pdu->type = TFTP_PDU_DATA;
				if (!_tftp_send_response(ppt))
				{
					raise_user_error(_T("xtftp_recv"), _T("send response data failed"));
				}

				if (!((ppt->nxt_snd_num -1) % mw))
				{
					ppt->rcv_eof = 0;
					TRY = TFTP_MAX_TRY;
					while (!ppt->rcv_eof && TRY)
					{
						if (!_tftp_recv_request(ppt))
						{
							_tftp_replay_response(ppt, ppt->pre_rcv_num + 1);
							TRY--;
						}
						else
						{
							_tftp_clear_response(ppt, ppt->pre_rcv_num);
							TRY = TFTP_MAX_TRY;
						}
					}
				}
			}
			else if (ppt->type == _XTFTP_TYPE_CLI)
			{
				pdu->type = TFTP_PDU_DATA;
				if (!_tftp_send_request(ppt))
				{
					raise_user_error(_T("xtftp_recv"), _T("send request data failed"));
				}

				if (!((ppt->nxt_snd_num - 1) % mw))
				{
					ppt->rcv_eof = 0;
					TRY = TFTP_MAX_TRY;
					while (!ppt->rcv_eof && TRY)
					{
						if (!_tftp_recv_response(ppt))
						{
							_tftp_replay_request(ppt, ppt->pre_rcv_num + 1);
							TRY--;
						}
						else
						{
							_tftp_clear_request(ppt, ppt->pre_rcv_num);
							TRY = TFTP_MAX_TRY;
						}
					}
				}
			}
		}
	}

	*pch = pos;

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	*pch = 0;

	return 0;
}

bool_t xtftp_flush(xhand_t tftp)
{
	tftp_context* ppt = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t pos = 0;

	int TRY;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	TRY_CATCH;

	pdu = &ppt->snd_pdu;

	pdu->pdv_len = pdu->pdv_off;
	pdu->pdv_off = 0;

	if (ppt->type == _XTFTP_TYPE_SRV)
	{
		pdu->type = TFTP_PDU_DATA;
		if (!_tftp_send_response(ppt))
		{
			raise_user_error(_T("xtftp_recv"), _T("send response data failed"));
		}

		ppt->rcv_eof = 0;
		TRY = TFTP_MAX_TRY;
		while(!ppt->rcv_eof && TRY)
		{
			if (!_tftp_recv_request(ppt))
			{
				_tftp_replay_response(ppt, ppt->pre_rcv_num + 1);
				TRY--;
			}
			else
			{
				_tftp_clear_request(ppt, ppt->pre_rcv_num);
				TRY = TFTP_MAX_TRY;
			}
		}
	}
	else if (ppt->type == _XTFTP_TYPE_CLI)
	{
		pdu->type = TFTP_PDU_DATA;
		if (!_tftp_send_request(ppt))
		{
			raise_user_error(_T("xtftp_recv"), _T("send request data failed"));
		}

		ppt->rcv_eof = 0;
		TRY = TFTP_MAX_TRY;
		while (!ppt->rcv_eof && TRY)
		{
			if (!_tftp_recv_response(ppt))
			{
				_tftp_replay_request(ppt, ppt->pre_rcv_num + 1);
				TRY--;
			}
			else
			{
				_tftp_clear_request(ppt, ppt->pre_rcv_num);
				TRY = TFTP_MAX_TRY;
			}
		}
	}

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	return 0;
}


void xtftp_abort(xhand_t tftp, int errcode)
{
	tftp_context* ppt = TypePtrFromHead(tftp_context, tftp);

	tftp_pdu_context* pdu;

	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t len;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	TRY_CATCH;

	pdu = &ppt->snd_pdu;

	pdu->errcode = errcode;

	_tftp_error(pdu->errcode, pdu->errtext);

	_tftp_clear_pdv(pdu);

	pdu->type = TFTP_PDU_ERR;

	len = _tftp_format_pdu(pkg_buf, TFTP_PKG_SIZE, pdu);

	if (!(*ppt->pif->pf_write)(ppt->pif->fd, pkg_buf, &len))
	{
		raise_user_error(NULL, NULL);
	}

	(*ppt->pif->pf_flush)(ppt->pif->fd);

	END_CATCH;

	return;
ONERROR:

	return;
}

bool_t xtftp_head(xhand_t tftp)
{
	tftp_context* ppt = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;
	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t pos = 0;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	TRY_CATCH;

	pdu = &ppt->snd_pdu;

	if (ppt->type == _XTFTP_TYPE_SRV)
	{
		pdu->type = TFTP_PDU_HEAD;
		if (!_tftp_send_response(ppt))
		{
			raise_user_error(NULL, NULL);
		}
	}
	else if (ppt->type == _XTFTP_TYPE_CLI)
	{
		if (!_tftp_recv_response(ppt))
		{
			raise_user_error(NULL, NULL);
		}

		if (ppt->rcv_pdu.type != TFTP_PDU_HEAD)
		{
			raise_user_error(_T("xtftp_recv"), _T("invalid ack package type"));
		}
	}

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	return 0;
}

bool_t xtftp_delete(xhand_t tftp)
{
	tftp_context* ppt = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;
	byte_t pkg_buf[TFTP_PKG_SIZE] = { 0 };
	dword_t pos = 0;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	TRY_CATCH;

	pdu = &ppt->snd_pdu;

	if (ppt->type == _XTFTP_TYPE_SRV)
	{
		pdu->type = TFTP_PDU_ACK;
		if (!_tftp_send_response(ppt))
		{
			raise_user_error(NULL, NULL);
		}
	}
	else if (ppt->type == _XTFTP_TYPE_CLI)
	{
		if (!_tftp_recv_response(ppt))
		{
			raise_user_error(NULL, NULL);
		}

		if (ppt->rcv_pdu.type != TFTP_PDU_ACK)
		{
			raise_user_error(_T("xtftp_recv"), _T("invalid ack package type"));
		}
	}

	END_CATCH;

	return 1;
ONERROR:

	XDK_TRACE_LAST;

	return 0;
}


int xtftp_method(xhand_t tftp, tchar_t* buf, int max)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	int len;
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	if (pftp->type == _XTFTP_TYPE_SRV)
		pdu = &pftp->rcv_pdu;
	else
		pdu = &pftp->snd_pdu;

	len = xslen(pdu->method);
	len = (len < max) ? len : max;

	if (buf)
	{
		xsncpy(buf, pdu->method, len);
	}

	return len;
}

void xtftp_set_isdir(xhand_t tftp, bool_t isdir)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	pdu = &pftp->snd_pdu;

	pdu->isdir = (sword_t)isdir;
}

bool_t xtftp_get_isdir(xhand_t tftp)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	if (pftp->type == _XTFTP_TYPE_SRV)
		pdu = &pftp->rcv_pdu;
	else
		pdu = &pftp->snd_pdu;

	return (bool_t)pdu->isdir;
}

void xtftp_set_filesize(xhand_t tftp, dword_t size)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	pdu = &pftp->snd_pdu;

	pdu->size = size;
}

dword_t xtftp_get_filesize(xhand_t tftp)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	if (pftp->type == _XTFTP_TYPE_SRV)
		pdu = &pftp->rcv_pdu;
	else
		pdu = &pftp->snd_pdu;

	return pdu->size;
}

void xtftp_set_filetime(xhand_t tftp, const tchar_t* ftime)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	pdu = &pftp->snd_pdu;

	xscpy(pdu->ftime, ftime);
}

void xtftp_get_filetime(xhand_t tftp, tchar_t* ftime)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	if (pftp->type == _XTFTP_TYPE_SRV)
		pdu = &pftp->rcv_pdu;
	else
		pdu = &pftp->snd_pdu;

	xscpy(ftime, pdu->ftime);
}

void xtftp_set_filename(xhand_t tftp, const tchar_t* fname)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	pdu = &pftp->snd_pdu;

	xscpy(pdu->file, fname);
}

void xtftp_get_filename(xhand_t tftp, tchar_t* fname)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);
	tftp_pdu_context* pdu;

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	if (pftp->type == _XTFTP_TYPE_SRV)
		pdu = &pftp->rcv_pdu;
	else
		pdu = &pftp->snd_pdu;

	xscpy(fname, pdu->file);
}

void xtftp_settmo(xhand_t tftp, dword_t tmo)
{
	tftp_context* pftp = TypePtrFromHead(tftp_context, tftp);

	XDK_ASSERT(tftp && tftp->tag == _HANDLE_TFTP);

	if (pftp->secu == _SECU_DTLS)
	{
		xdtls_settmo(pftp->pif->fd, tmo);
	}
	else
	{
		xudp_settmo(pftp->pif->fd, tmo);
	}
}

#endif /*XDK_SUPPORT_SOCK*/
