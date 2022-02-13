/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory pixmap document

	@module	mpix.c | implement file

	@devnote 张文权 2021.01 - 2021.12 v6.0
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

#include "mpix.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"

static const unsigned char bitmask[8] = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };

static int get_pixbit(mem_pixmap_ptr pmp, int x, int y)
{
	register ADDR8 addr;
	register int pos, bit;

	XDK_ASSERT(pmp != NULL);

	if (x < 0 || x >= pmp->width || y < 0 || y >= pmp->height)
		return 0;

	addr = ((ADDR8)pmp->data) + (x >> 3) + y * pmp->bytes_per_line;
	pos = x & 7;
	bit = ((*addr >> (7 - pos))) & 0x01;

	return bit;
}

static void set_pixbit(mem_pixmap_ptr pmp, int x, int y, int bit)
{
	register ADDR8 addr;
	register int pos;

	XDK_ASSERT(pmp != NULL);

	if (x < 0 || x >= pmp->width || y < 0 || y >= pmp->height)
		return;

	addr = ((ADDR8)pmp->data) + (x >> 3) + y * pmp->bytes_per_line;
	pos = x & 7;
	if (bit) bit = 1;
	bit <<= (7 - pos);

	*addr = (*addr & bitmask[pos]) | (bit & 0xFF);
}

mem_pixmap_ptr alloc_pixmap(int width, int height)
{
	mem_pixmap_ptr ppixmap;

	ppixmap = (mem_pixmap_ptr)xmem_alloc(sizeof(mem_pixmap_t));
	ppixmap->width = width;
	ppixmap->height = height;
	ppixmap->bytes_per_line = (width + 7) / 8;
	ppixmap->size = ppixmap->bytes_per_line * ppixmap->height;
	ppixmap->data = (byte_t*)xmem_alloc(ppixmap->size);

	ppixmap->setPixbit = set_pixbit;
	ppixmap->getPixbit = get_pixbit;

	return ppixmap;
}

void clean_pixmap(mem_pixmap_ptr pmp)
{
	XDK_ASSERT(pmp != NULL);

	xmem_zero((void*)pmp->data, pmp->size);
}

void free_pixmap(mem_pixmap_ptr pmp)
{
	XDK_ASSERT(pmp != NULL);

	if (pmp->data)
		xmem_free(pmp->data);

	xmem_free(pmp);
}



