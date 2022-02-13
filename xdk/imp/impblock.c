/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory block document

	@module	impblock.c | implement file

	@devnote 张文权 2021.01 - 2021.12	v6.0
***********************************************************************/

/**********************************************************************
This program is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the xdllied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
LICENSE.GPL3 for more details.
***********************************************************************/

#include "impblock.h"

#include "../xdkimp.h"
#include "../xdkstd.h"


typedef struct _block_context{
	handle_head head;		//reserved for xhand_t

	byte_t** block;
	dword_t write_bytes;
	dword_t read_bytes;
}block_context;


xhand_t xblock_open(byte_t** pp)
{
	block_context* ppi;

	XDK_ASSERT(pp != NULL);

	ppi = (block_context*)xmem_alloc(sizeof(block_context));
	ppi->head.tag = _HANDLE_BLOCK;
	ppi->block = pp;
	ppi->read_bytes = 0;
	ppi->write_bytes = bytes_size(pp);

	return &ppi->head;
}

byte_t** xblock_handle(xhand_t block)
{
	block_context* ppi = TypePtrFromHead(block_context, block);

	XDK_ASSERT(block && block->tag == _HANDLE_BLOCK);

	return ppi->block;
}

byte_t** xblock_close(xhand_t block)
{
	block_context* ppi = TypePtrFromHead(block_context, block);
	byte_t** pp;

	XDK_ASSERT(block && block->tag == _HANDLE_BLOCK);

	pp = ppi->block;

	xmem_free(ppi);

	return pp;
}

bool_t xblock_read(xhand_t block, byte_t* buf, dword_t* pb)
{
	block_context* ppt = TypePtrFromHead(block_context, block);
	dword_t size;

	XDK_ASSERT(block && block->tag == _HANDLE_BLOCK);

	size = (*pb < (ppt->write_bytes - ppt->read_bytes)) ? (*pb) : (ppt->write_bytes - ppt->read_bytes);

	bytes_copy((byte_t**)ppt->block, ppt->read_bytes, buf, &size);
	ppt->read_bytes += (dword_t)size;

	*pb = (dword_t)size;

	return 1;
}

bool_t xblock_write(xhand_t block, const byte_t* buf, dword_t* pb)
{
	block_context* ppt = TypePtrFromHead(block_context, block);
	dword_t size;

	XDK_ASSERT(block && block->tag == _HANDLE_BLOCK);

	size = *pb;

	bytes_insert((byte_t**)ppt->block, ppt->write_bytes, buf, size);
	ppt->write_bytes += (dword_t)size;

	*pb = (dword_t)size;
	
	return 1;
}


