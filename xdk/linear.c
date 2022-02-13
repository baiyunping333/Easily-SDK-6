/***********************************************************************
	Easily SDK 6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc linear buffer document

	@module	linear.c | implement file

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

#include "linear.h"

#include "xdkimp.h"
#include "xdkstd.h"

typedef struct _linear_frame{
		int frm_seq;
		dword_t frm_len;
		byte_t* frm_pkg;
}linear_frame;

typedef struct _linear_context{
	memobj_head head;

	int lin_win;
	int frm_top;
	linear_frame** frm_list;
}linear_context;

linear_t alloc_linear(int wins)
{
	linear_context* plc;

	plc = (linear_context*)xmem_alloc(sizeof(linear_context));
	plc->head.tag = MEM_LINEAR;

	XDK_ASSERT(wins > 0);

	plc->frm_top = 0;

	plc->lin_win = wins;
	plc->frm_list = (linear_frame**)xmem_alloc(wins * sizeof(linear_frame*));

	return &(plc->head);
}

void clear_linear(linear_t lin)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);
	linear_frame* plf;
	int i;

	XDK_ASSERT(lin != NULL && lin->tag == MEM_LINEAR);

	for (i = 0; i < plc->lin_win; i++)
	{
		plf = plc->frm_list[i];
		if (plf)
		{
			xmem_free(plf->frm_pkg);
			xmem_free(plf);
		}
		plc->frm_list[i] = NULL;
	}

	plc->frm_top = 0;
}

void free_linear(linear_t lin)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);

	XDK_ASSERT(lin != NULL && lin->tag == MEM_LINEAR);

	clear_linear(lin);

	xmem_free(plc->frm_list);
	xmem_free(plc);
}

int get_linear_window(linear_t lin)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);

	XDK_ASSERT(lin != NULL && lin->tag == MEM_LINEAR);

	return plc->lin_win;
}

int get_linear_top(linear_t lin)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);

	XDK_ASSERT(lin != NULL && lin->tag == MEM_LINEAR);

	return plc->frm_top;
}

byte_t* insert_linear_frame(linear_t lin, int seqnum, dword_t frmlen)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);
	linear_frame* plf;
	int pos, n;

	XDK_ASSERT(lin != NULL && lin->tag == MEM_LINEAR);

	pos = seqnum - plc->frm_top;

	if (pos < 0) return NULL;

	if (pos < plc->lin_win)
	{
		plf = plc->frm_list[pos];
		if (plf) return plf->frm_pkg;
	}

	plf = (linear_frame*)xmem_alloc(sizeof(linear_frame));
	plf->frm_pkg = (byte_t*)xmem_alloc(frmlen + 1);
	plf->frm_len = frmlen;
	plf->frm_seq = seqnum;

	if (pos < plc->lin_win)
	{
		plc->frm_list[pos] = plf;
		if (!pos)
		{
			plc->frm_top = seqnum;
		}
		return plf->frm_pkg;
	}

	if (pos < 2 * plc->lin_win)
	{
		n = pos + 1 - plc->lin_win;
		for (pos = 0; pos < n; pos++)
		{
			if (plc->frm_list[pos])
			{
				xmem_free((plc->frm_list[pos])->frm_pkg);
				xmem_free(plc->frm_list[pos]);
			}
			plc->frm_list[pos] = NULL;
		}

		while (plc->frm_list[n] == NULL && n < plc->lin_win)
			n++;

		xmem_move((void*)(plc->frm_list + n), (plc->lin_win - n) * sizeof(linear_frame*), 0 - n * sizeof(linear_frame*));
		pos = plc->lin_win - n;
		plc->frm_list[pos] = plf;
		plc->frm_top = plc->frm_list[0]->frm_seq;

		pos++;
		while (pos < plc->lin_win)
		{
			plc->frm_list[pos++] = NULL;
		}

		return plf->frm_pkg;
	}

	for (pos = 0; pos < plc->lin_win; pos++)
	{
		if (plc->frm_list[pos])
		{
			xmem_free((plc->frm_list[pos])->frm_pkg);
			xmem_free(plc->frm_list[pos]);
		}
		plc->frm_list[pos] = NULL;
	}

	plc->frm_list[0] = plf;
	plc->frm_top = seqnum;

	return plf->frm_pkg;
}

bool_t delete_linear_frame(linear_t lin, int seqnum)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);
	linear_frame* plf;
	int pos;

	pos = seqnum - plc->frm_top;
	if (pos < 0 || pos > plc->lin_win - 1)
		return 0;

	plf = plc->frm_list[pos];
	if (plf)
	{
		xmem_free(plf->frm_pkg);
		xmem_free(plf);
	}
	plc->frm_list[pos] = NULL;

	return 1;
}

void clean_linear_frame(linear_t lin, int seqnum)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);
	linear_frame* plf;
	int i, n;

	n = seqnum - plc->frm_top + 1;
	if (n <= 0) return;

	n = (n < plc->lin_win) ? n : plc->lin_win;

	for (i = 0; i < n; i++)
	{
		plf = plc->frm_list[i];
		if (plf)
		{
			xmem_free(plf->frm_pkg);
			xmem_free(plf);
		}
		plc->frm_list[i] = NULL;
	}

	while (plc->frm_list[n] == NULL && n < plc->lin_win)
		n++;

	xmem_move((void*)(plc->frm_list + n), (plc->lin_win - n) * sizeof(linear_frame*), 0 - n * sizeof(linear_frame*));

	i = plc->lin_win - n;
	while (i < plc->lin_win)
	{
		plc->frm_list[i] = NULL;
		i++;
	}

	plf = plc->frm_list[0];
	plc->frm_top = (plf) ? plf->frm_seq : (seqnum + 1);
}

byte_t* get_linear_frame(linear_t lin, int seqnum, dword_t* pb)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);
	linear_frame* plf;
	int pos;

	pos = seqnum - plc->frm_top;
	if (pos < 0 || pos > plc->lin_win - 1) return 0;

	plf = plc->frm_list[pos];
	if (!plf)
	{
		if (pb) *pb = 0;
		return NULL;
	}
	
	if (pb) *pb = plf->frm_len;

	return plf->frm_pkg;
}

bool_t set_linear_frame(linear_t lin, int seqnum, const byte_t* frame, dword_t size)
{
	linear_context* plc = TypePtrFromHead(linear_context, lin);
	linear_frame* plf;
	int pos;

	pos = seqnum - plc->frm_top;
	if (pos < 0 || pos > plc->lin_win - 1) return 0;

	plf = plc->frm_list[pos];
	if (!plf)
	{
		plf = (linear_frame*)xmem_alloc(sizeof(linear_frame));
		plf->frm_seq = seqnum;
		plc->frm_list[pos] = plf;
	}

	plf->frm_pkg = (byte_t*)xmem_realloc(plf->frm_pkg, size + 1);
	xmem_copy((void*)plf->frm_pkg, (void*)frame, size);
	plf->frm_len = size;

	return 1;
}

#if defined(XDK_SUPPORT_TEST)
void test_linear()
{
	linear_t lin = alloc_linear(3);

	byte_t* buf;
	int len = 10;

	int i;

	for (i = 0; i < 10; i++)
	{
		buf = insert_linear_frame(lin, i + 1, 8);

		if (!((i+1) % 3))
			clean_linear_frame(lin, i);
	}

	free_linear(lin);
}
#endif