/***********************************************************************
	Easily DICOM 3.x

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc dicom document

	@module	dicmbag.c | dicom document implement file

	@devnote 张文权 2018.01 - 2018.12	v1.0
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

#include "dicmbag.h"
#include "dicmdom.h"
#include "dicmctx.h"


bool_t load_dicm_doc_from_file(link_t_ptr ptr, const secu_desc_t* psd, const tchar_t* fname)
{
	file_t xf;
	bio_interface bio = { 0 };
	stream_t stm;
	bool_t b;

	xf = xfile_open(psd, fname, FILE_OPEN_READ);
	if (!xf)
		return 0;

	get_bio_interface(xf->fd, &bio);

	stm = stream_alloc(&bio);

	b = parse_dicm_doc_from_stream(ptr, DICM_OPERA_FILEHEAD | DICM_OPERA_DATASET, stm);

	stream_free(stm);
	xfile_close(xf);

	return b;
}

bool_t save_dicm_doc_to_file(link_t_ptr ptr, const secu_desc_t* psd, const tchar_t* fname)
{
	file_t xf;
	bio_interface bio = { 0 };
	stream_t stm;
	bool_t b;

	xf = xfile_open(psd, fname, FILE_OPEN_CREATE);
	if (!xf)
		return 0;

	get_bio_interface(xf->fd, &bio);

	stm = stream_alloc(&bio);

	b = format_dicm_doc_to_stream(ptr, DICM_OPERA_FILEHEAD | DICM_OPERA_DATASET, stm);

	stream_free(stm);
	xfile_close(xf);

	return b;
}


bool_t load_dicm_doc_from_bytes(link_t_ptr ptr, byte_t** pp)
{
	xhand_t blk;
	bio_interface bio = { 0 };
	stream_t stm;
	bool_t b;

	blk = xblock_open(pp);
	if (!blk)
		return 0;

	get_bio_interface(blk, &bio);

	stm = stream_alloc(&bio);

	b = parse_dicm_doc_from_stream(ptr, DICM_OPERA_DATASET, stm);

	stream_free(stm);

	xblock_close(blk);

	return b;
}

bool_t save_dicm_doc_to_bytes(link_t_ptr ptr, byte_t** pp)
{
	xhand_t blk;
	bio_interface bio = { 0 };
	stream_t stm;
	bool_t b;

	blk = xblock_open(pp);
	if (!blk)
		return 0;

	get_bio_interface(blk, &bio);

	stm = stream_alloc(&bio);

	b = format_dicm_doc_to_stream(ptr, DICM_OPERA_DATASET, stm);

	stream_free(stm);

	xblock_close(blk);

	return b;
}

bool_t load_dicm_summary_from_file(dicm_summary_t* pds, const secu_desc_t* psd, const tchar_t* fname)
{
	file_t xf;
	bio_interface bio = { 0 };
	stream_t stm;
	bool_t b;

	xf = xfile_open(psd, fname, FILE_OPEN_READ);
	if (!xf)
		return 0;

	get_bio_interface(xf->fd, &bio);

	stm = stream_alloc(&bio);

	b = parse_dicm_summary_from_stream(pds, DICM_OPERA_FILEHEAD | DICM_OPERA_DATASET, stm);

	stream_free(stm);
	xfile_close(xf);

	return b;
}

