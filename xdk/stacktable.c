/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc stacktable document

	@module	stacktable.c | implement file

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

#include "stacktable.h"

#include "xdkimp.h"
#include "xdkstd.h"

#define STACKTABLE_INC		32

typedef struct _stack_table_t{
	link_t lk;

	int size;
	int index;
	void** pset;
}stack_table_t;

#define StackTableFromLink(p) TypePtrFromLink(stack_table_t,p)

link_t_ptr create_stack_table(void)
{
	stack_table_t* pst;

	pst = (stack_table_t*)xmem_alloc(sizeof(stack_table_t));
	pst->lk.tag = lkStackTable;

	pst->size = 0;
	pst->index = -1;
	pst->pset = NULL;

	return &pst->lk;
}

void destroy_stack_table(link_t_ptr st)
{
	stack_table_t* pst = StackTableFromLink(st);

	XDK_ASSERT(st && st->tag == lkStackTable);

	clear_stack_table(st);
	xmem_free(pst);
}

void clear_stack_table(link_t_ptr st)
{
	stack_table_t* pst = StackTableFromLink(st);

	XDK_ASSERT(st && st->tag == lkStackTable);

	xmem_free(pst->pset);
	pst->pset = NULL;
	pst->size = 0;
	pst->index = -1;
}

void push_stack_node(link_t_ptr st, void* data)
{
	stack_table_t* pst = StackTableFromLink(st);

	XDK_ASSERT(st && st->tag == lkStackTable);

	if (pst->index + 1 == pst->size)
	{
		pst->size += STACKTABLE_INC;
		pst->pset = xmem_realloc(pst->pset, sizeof(void*) * pst->size);
	}

	pst->pset[pst->index + 1] = data;
	pst->index++;
}

void* pop_stack_node(link_t_ptr st)
{
	stack_table_t* pst = StackTableFromLink(st);
	void* data;

	XDK_ASSERT(st && st->tag == lkStackTable);

	if (pst->index < 0)
		return NULL;

	data = pst->pset[pst->index];
	pst->index--;

	if (pst->size - pst->index > STACKTABLE_INC)
	{
		pst->size -= STACKTABLE_INC;
		pst->pset = xmem_realloc(pst->pset, sizeof(void*) * pst->size);
	}

	return data;
}

void* pick_stack_node(link_t_ptr st)
{
	stack_table_t* pst = StackTableFromLink(st);
	void* data;

	XDK_ASSERT(st && st->tag == lkStackTable);

	if (pst->index < 0)
		return NULL;

	data = pst->pset[0];
	if (pst->index > 0)
	{
		xmem_move(&(pst->pset[1]), pst->index * sizeof(void*), -(int)sizeof(void*));
	}
	pst->index--;

	if (pst->size - pst->index > STACKTABLE_INC)
	{
		pst->size -= STACKTABLE_INC;
		pst->pset = xmem_realloc(pst->pset, sizeof(void*) * pst->size);
	}

	return data;
}

void* peek_stack_node(link_t_ptr st, int index)
{
	stack_table_t* pst = StackTableFromLink(st);

	XDK_ASSERT(st && st->tag == lkStackTable);

	if (index < 0)
		index = pst->index;

	if (index < 0 || index > pst->index)
		return NULL;

	return pst->pset[index];
}

int get_stack_node_count(link_t_ptr st)
{
	stack_table_t* pst = StackTableFromLink(st);

	XDK_ASSERT(st && st->tag == lkStackTable);

	return pst->index + 1;
}