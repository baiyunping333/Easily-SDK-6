/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc error document

	@module	imperr.c | implement file

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

#include "imperror.h"

#include "../xdkimp.h"
#include "../xdkstd.h"

void set_last_error(const tchar_t* errcode, const tchar_t* errtext, int len)
{
#ifdef XDK_SUPPORT_ERROR
	if_zone_t* pzn;
	if_memo_t* piv;
	if_dump_t* pdu;
	err_dump_t* per;
	int n;

	piv = PROCESS_MEMO_INTERFACE;
	pdu = THREAD_DUMP_INTERFACE;
	pzn = THREAD_ZONE_INTERFACE;

	if (!piv) return;

	if (!pdu || !pdu->err_enable) return;

	if (!pzn || !pzn->if_heap) return;

#ifdef XDK_SUPPORT_MEMO_HEAP
	per = (err_dump_t*)(*piv->pf_heap_alloc)(pzn->if_heap, sizeof(err_dump_t));
#else
	per = (err_dump_t*)(*piv->pf_local_alloc)(sizeof(err_dump_t));
#endif
	n = xslen(errcode);
#ifdef XDK_SUPPORT_MEMO_HEAP
	per->err_code = (tchar_t*)(*piv->pf_heap_alloc)(pzn->if_heap, (n+1) * sizeof(tchar_t));
#else
	per->err_code = (tchar_t*)(*piv->pf_local_alloc)((n + 1) * sizeof(tchar_t));
#endif
	xsncpy(per->err_code, errcode, n);

	if (len < 0) len = xslen(errtext);

#ifdef XDK_SUPPORT_MEMO_HEAP
	per->err_text = (tchar_t*)(*piv->pf_heap_alloc)(pzn->if_heap, (len + 1) * sizeof(tchar_t));
#else
	per->err_text = (tchar_t*)(*piv->pf_local_alloc)((len + 1) * sizeof(tchar_t));
#endif
	xsncpy(per->err_text, errtext, len);

	per->err_next = pdu->err_dump;
	pdu->err_dump = per;

#endif
}

void get_last_error(tchar_t* code, tchar_t* text, int max)
{
#ifdef XDK_SUPPORT_ERROR
	if_dump_t* pdu;

	pdu = THREAD_DUMP_INTERFACE;

	if (!pdu || !pdu->err_dump)
		return;

	if (code)
	{
		xsncpy(code, pdu->err_dump->err_code, NUM_LEN);
	}
	if (text)
	{
		xsncpy(text, pdu->err_dump->err_text, max);
	}
#endif
}

void set_system_error(const tchar_t* errcode)
{
	tchar_t errtext[ERR_LEN + 1] = { 0 };

#ifdef XDK_SUPPORT_ERROR
	if_error_t* pie;

	pie = PROCESS_ERROR_INTERFACE;

	XDK_ASSERT(pie != NULL);

	(*pie->pf_error_text)(errtext, ERR_LEN);
#else
	xscpy(errtext, _T("unknown system error"));
#endif

	set_last_error(errcode, errtext, -1);
}

void xdk_trace(const tchar_t* code, const tchar_t* info)
{
#ifdef XDK_SUPPORT_ERROR
	if_error_t* pie;

	pie = PROCESS_ERROR_INTERFACE;

	XDK_ASSERT(pie != NULL);

	if (pie->pf_error_trace)
	{
		(*pie->pf_error_trace)(pie->param, code, info);
	}
	else
	{
		(*pie->pf_error_print)(code);
		(*pie->pf_error_print)(_T(" : "));
		(*pie->pf_error_print)(info);
		(*pie->pf_error_print)(_T("\n"));
	}
#endif
}

void xdk_set_track(PF_TRACK_ERROR pf, void* pa)
{
#ifdef XDK_SUPPORT_ERROR
	if_dump_t* pdu;

	pdu = THREAD_DUMP_INTERFACE;

	if (!pdu)
		return;

	pdu->err_track = pf;
	pdu->err_param = pa;
#endif
}

void xdk_trace_last()
{
#ifdef XDK_SUPPORT_ERROR
	if_zone_t* pzn;
	if_memo_t* piv;
	if_dump_t* pdu;
	err_dump_t* per;

	piv = PROCESS_MEMO_INTERFACE;
	pdu = THREAD_DUMP_INTERFACE;
	pzn = THREAD_ZONE_INTERFACE;

	if (!piv) return;

	if (!pdu) return;

	if (!pzn || !pzn->if_heap) return;

	pdu->err_enable = 0;

	while (pdu->err_dump)
	{
		per = pdu->err_dump;
		pdu->err_dump = per->err_next;

		if (pdu->err_track)
			(*pdu->err_track)(pdu->err_param, per->err_code, per->err_text);
		else
			xdk_trace(per->err_code, per->err_text);

#ifdef XDK_SUPPORT_MEMO_HEAP
		(*piv->pf_heap_free)(pzn->if_heap, per->err_code);
		(*piv->pf_heap_free)(pzn->if_heap, per->err_text);
		(*piv->pf_heap_free)(pzn->if_heap, per);
#else
		(*piv->pf_local_free)(per->err_code);
		(*piv->pf_local_free)(per->err_text);
		(*piv->pf_local_free)(per);
#endif
	}

	pdu->err_enable = 1;
#endif
}
