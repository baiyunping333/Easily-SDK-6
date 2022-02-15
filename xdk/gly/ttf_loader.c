/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc TrueType document

	@module	truetype.c | implement file

	@devnote 张文权 2021.01 - 2021.12 v6.0
***********************************************************************/

/**********************************************************************
This program is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABINETY or FITNESS FOR A PARTICULAR PURPOSE.See the
LICENSE.GPL3 for more details.
***********************************************************************/

#include "ttf.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"

bool_t ttf_load_file_head(ttf_file_head_t* pfile, const byte_t* buf, dword_t size)
{
	dword_t total = 0;
	sword_t n;
	dword_t *pbegin, *pend;
	dword_t allsum = 0, chksum = 0;

	pfile->sfntVersion = GET_DWORD_NET(buf, total);
	total += 4;

	pfile->numTables = GET_SWORD_NET(buf, total);
	total += 2;

	pfile->searchRange = GET_SWORD_NET(buf, total);
	total += 2;

	pfile->entrySelector = GET_SWORD_NET(buf, total);
	total += 2;

	pfile->rangeShift = GET_SWORD_NET(buf, total);
	total += 2;

	pfile->pTableRecords = (ttf_table_record_t*)xmem_alloc(pfile->numTables * sizeof(ttf_table_record_t));

	for (n = 0; n < pfile->numTables; n++)
	{
		xmem_copy((void*)(pfile->pTableRecords[n].tableTag), (void*)(buf + total), 4);
		total += 4;

		pfile->pTableRecords[n].checksum = GET_DWORD_NET(buf, total);
		total += 4;

		pfile->pTableRecords[n].offset = GET_DWORD_NET(buf, total);
		total += 4;

		pfile->pTableRecords[n].length = GET_DWORD_NET(buf, total);
		total += 4;

		pbegin = (dword_t*)(buf + pfile->pTableRecords[n].offset);
		pend = pbegin + ((pfile->pTableRecords[n].length + 3) & ~3) / sizeof(dword_t);

		chksum = 0;
		while (pbegin < pend)
		{
			chksum += GET_DWORD_NET(((byte_t*)pbegin), 0);
			pbegin++;
		}
		allsum += chksum;

		if (pfile->pTableRecords[n].tableTag[0] == 'h' && pfile->pTableRecords[n].tableTag[1] == 'e' && pfile->pTableRecords[n].tableTag[2] == 'a' && pfile->pTableRecords[n].tableTag[3] == 'd')
		{
			NOP;
		}
		else
		{
			if (chksum != pfile->pTableRecords[n].checksum)
			{
				set_last_error(_T("ttf_load_file_head"), _T("table checksum error"), -1);
				return bool_false;
			}
		}
	}

	//pfile->pTableRecords[n].checksum = allsum;

	return bool_true;
}

void ttf_clear_file_head(ttf_file_head_t* pfile)
{
	if (pfile->pTableRecords)
	{
		xmem_free(pfile->pTableRecords);
	}

	xmem_zero((void*)pfile, sizeof(ttf_file_head_t));
}

bool_t ttf_load_head_table(const ttf_file_head_t* pfile, ttf_head_table_t* phead, const byte_t* buf, dword_t size)
{
	sword_t n;
	byte_t* pb;
	dword_t m;

	for (n = 0; n < pfile->numTables; n++)
	{
		if (pfile->pTableRecords[n].tableTag[0] == 'h' && pfile->pTableRecords[n].tableTag[1] == 'e' && pfile->pTableRecords[n].tableTag[2] == 'a' && pfile->pTableRecords[n].tableTag[3] == 'd')
			break;
	}

	if (n == pfile->numTables)
	{
		set_last_error(_T("ttf_load_head_table"), _T("head table not find"), -1);
		return bool_false;
	}

	pb = (buf + pfile->pTableRecords[n].offset);
	m = 0;

	phead->majorVersion = GET_SWORD_NET(pb, m);
	m += 2;

	phead->minorVersion = GET_SWORD_NET(pb, m);
	m += 2;

	phead->fontRevision = GET_DWORD_NET(pb, m);
	m += 4;

	phead->checksumAdjustment = GET_DWORD_NET(pb, m);
	m += 4;

	phead->magicNumber = GET_DWORD_NET(pb, m);
	m += 4;

	phead->flags = GET_SWORD_NET(pb, m);
	m += 2;

	phead->unitsPerEm = GET_SWORD_NET(pb, m);
	m += 2;

	phead->created = GET_LWORD_NET(pb, m);
	m += 8;

	phead->modified = GET_LWORD_NET(pb, m);
	m += 8;

	phead->xMin = (short)GET_SWORD_NET(pb, m);
	m += 2;

	phead->yMin = (short)GET_SWORD_NET(pb, m);
	m += 2;

	phead->xMax = (short)GET_SWORD_NET(pb, m);
	m += 2;

	phead->yMax = (short)GET_SWORD_NET(pb, m);
	m += 2;

	phead->macStyle = GET_SWORD_NET(pb, m);
	m += 2;

	phead->lowestRecPPEM = GET_SWORD_NET(pb, m);
	m += 2;

	phead->fontDirectionHint = (short)GET_SWORD_NET(pb, m);
	m += 2;

	phead->indexToLocFormat = (short)GET_SWORD_NET(pb, m);
	m += 2;

	phead->glyphDataFormat = (short)GET_SWORD_NET(pb, m);
	m += 2;

	/*if (phead->checksumAdjustment != (0xB1B0AFBA - pfile->pTableRecords[n].checksum))
	{
		set_last_error(_T("ttf_load_head_table"), _T("checksum error"), -1);
		return bool_false;
	}*/

	return bool_true;
}

void ttf_clear_head_table(ttf_head_table_t* phead)
{
	xmem_zero((void*)phead, sizeof(ttf_head_table_t));
}

bool_t ttf_load_name_table(const ttf_file_head_t* pfile, ttf_name_table_t* pname, const byte_t* buf, dword_t size)
{
	sword_t n, k;
	byte_t *pb;
	dword_t m;
	
	for (n = 0; n < pfile->numTables; n++)
	{
		if (pfile->pTableRecords[n].tableTag[0] == 'n' && pfile->pTableRecords[n].tableTag[1] == 'a' && pfile->pTableRecords[n].tableTag[2] == 'm' && pfile->pTableRecords[n].tableTag[3] == 'e')
			break;
	}

	if (n == pfile->numTables)
	{
		set_last_error(_T("ttf_load_head_table"), _T("head table not find"), -1);
		return bool_false;
	}

	pb = (buf + pfile->pTableRecords[n].offset);
	m = 0;

	pname->version = GET_SWORD_NET(pb, m);
	m += 2;

	pname->count = GET_SWORD_NET(pb, m);
	m += 2;

	pname->storageOffset = GET_SWORD_NET(pb, m);
	m += 2;

	pname->pNameRecords = (ttf_name_record_t*)xmem_alloc(pname->count * sizeof(ttf_name_record_t));
	for (k = 0; k < pname->count; k++)
	{
		pname->pNameRecords[k].platformID = GET_SWORD_NET(pb, m);
		m += 2;

		pname->pNameRecords[k].encodingID = GET_SWORD_NET(pb, m);
		m += 2;

		pname->pNameRecords[k].languageID = GET_SWORD_NET(pb, m);
		m += 2;

		pname->pNameRecords[k].nameID = GET_SWORD_NET(pb, m);
		m += 2;

		pname->pNameRecords[k].length = GET_SWORD_NET(pb, m);
		m += 2;

		pname->pNameRecords[k].stringOffset = GET_SWORD_NET(pb, m);
		m += 2;

		pname->pNameRecords[k].stringData = (byte_t*)xmem_alloc(pname->pNameRecords[k].length);
		xmem_copy((void*)(pname->pNameRecords[k].stringData), (void*)(pb + pname->storageOffset + pname->pNameRecords[k].stringOffset), pname->pNameRecords[k].length);
	}

	if (pname->version > 0)
	{
		pname->langTagCount = GET_SWORD_NET(pb, m);
		m += 2;

		pname->pLangTagRecords = (ttf_langtag_record_t*)xmem_alloc(pname->langTagCount * sizeof(ttf_langtag_record_t));
		for (k = 0; k < pname->langTagCount; k++)
		{
			pname->pLangTagRecords[k].length = GET_SWORD_NET(pb, m);
			m += 2;

			pname->pLangTagRecords[k].langTagOffset = GET_SWORD_NET(pb, m);
			m += 2;
		}
	}

	return bool_true;
}

void ttf_clear_name_table(ttf_name_table_t* pname)
{
	sword_t k;

	if (pname->pNameRecords)
	{
		for (k = 0; k < pname->count; k++)
		{
			xmem_free(pname->pNameRecords[k].stringData);
		}
		xmem_free(pname->pNameRecords);
	}

	if (pname->pLangTagRecords)
	{
		xmem_free(pname->pLangTagRecords);
	}

	xmem_zero((void*)pname, sizeof(ttf_name_table_t));
}

bool_t ttf_load_cmap_table(const ttf_file_head_t* pfile, ttf_cmap_table_t* pcmap, const byte_t* buf, dword_t size)
{
	sword_t i, j, k;
	byte_t *pb, *pf;
	dword_t m, n;

	for (i = 0; i < pfile->numTables; i++)
	{
		if (pfile->pTableRecords[i].tableTag[0] == 'c' && pfile->pTableRecords[i].tableTag[1] == 'm' && pfile->pTableRecords[i].tableTag[2] == 'a' && pfile->pTableRecords[i].tableTag[3] == 'p')
			break;
	}

	if (i == pfile->numTables)
	{
		set_last_error(_T("ttf_load_head_table"), _T("cmap table not find"), -1);
		return bool_false;
	}

	pb = (buf + pfile->pTableRecords[i].offset);
	m = 0;

	pcmap->version = GET_SWORD_NET(pb, m);
	m += 2;

	pcmap->numTables = GET_SWORD_NET(pb, m);
	m += 2;

	pcmap->pEncodingRecords = (ttf_encoding_record_t*)xmem_alloc(pcmap->numTables * sizeof(ttf_encoding_record_t));
	for (j = 0; j < pcmap->numTables; j++)
	{
		pcmap->pEncodingRecords[j].platformID = GET_SWORD_NET(pb, m);
		m += 2;

		pcmap->pEncodingRecords[j].encodingID = GET_SWORD_NET(pb, m);
		m += 2;

		pcmap->pEncodingRecords[j].subtableOffset = GET_DWORD_NET(pb, m);
		m += 4;

		pf = (pb + pcmap->pEncodingRecords[j].subtableOffset);
		n = 0;

		pcmap->pEncodingRecords[j].subtable.format = GET_SWORD_NET(pf, n);
		n += 2;

		pcmap->pEncodingRecords[j].subtable.length = GET_SWORD_NET(pf, n);
		n += 2;

		pcmap->pEncodingRecords[j].subtable.language = GET_SWORD_NET(pf, n);
		n += 2;

		if (pcmap->pEncodingRecords[j].subtable.format == 0)
		{
			/*
			uint8	glyphIdArray[256]	An array that maps character codes to glyph index values.
			*/
			pcmap->pEncodingRecords[j].subtable.arrayCount = 256;
			pcmap->pEncodingRecords[j].subtable.pCodeArray = (sword_t*)xmem_alloc(256 * sizeof(sword_t));
			pcmap->pEncodingRecords[j].subtable.pGlyphIdArray = (sword_t*)xmem_alloc(256 * sizeof(sword_t));

			for (k = 0; k < 256; k++)
			{
				pcmap->pEncodingRecords[j].subtable.pCodeArray[k] = k;
				pcmap->pEncodingRecords[j].subtable.pGlyphIdArray[k] = GET_BYTE(pf, n);
				n += 1;
			}
		}
		else if (pcmap->pEncodingRecords[j].subtable.format == 2)
		{
			/*
			uint16	subHeaderKeys[256]	Array that maps high bytes to subHeaders: value is subHeader index × 8.
			SubHeader	subHeaders[ ]	Variable-length array of SubHeader records.
			uint16	glyphIdArray[ ]	Variable-length array containing subarrays used for mapping the low byte of 2-byte characters.
			*/
			/*
			SubHeader{
				uint16	firstCode	First valid low byte for this SubHeader.
				uint16	entryCount	Number of valid low bytes for this SubHeader.
				int16	idDelta	See text below.
				uint16	idRangeOffset	See text below.
			}
			*/
			sword_t headKeys[256];
			sword_t headIndex = 0, glyphCount = 0;
			sword_t *firstCodeArray, *entryCountArray, *rangeOffsetArray, *glyphIdArray;
			short *idDeltaArray;
			sword_t x, y;

			for (k = 0; k < 256; k++)
			{
				headKeys[k] = GET_SWORD_NET(pf, n);
				n += 2;

				//the max index
				headIndex = (headIndex < headKeys[k] / 8) ? headIndex : headKeys[k] / 8;
			}

			firstCodeArray = (sword_t*)xmem_alloc((headIndex + 1) * sizeof(sword_t));
			entryCountArray = (sword_t*)xmem_alloc((headIndex + 1) * sizeof(sword_t));
			idDeltaArray = (short*)xmem_alloc((headIndex + 1) * sizeof(short));
			rangeOffsetArray = (sword_t*)xmem_alloc((headIndex + 1) * sizeof(sword_t));

			for (k = 0; k <= headIndex; k++)
			{
				firstCodeArray[k] = GET_SWORD_NET(pf, n);
				n += 2;

				entryCountArray[k] = GET_SWORD_NET(pf, n);
				n += 2;

				idDeltaArray[k] = (short)GET_SWORD_NET(pf, n);
				n += 2;

				rangeOffsetArray[k] = GET_SWORD_NET(pf, n);
				n += 2;

				glyphCount += entryCountArray[k];
			}

			pcmap->pEncodingRecords[j].subtable.arrayCount = glyphCount;
			glyphIdArray = (sword_t*)xmem_alloc((glyphCount) * sizeof(sword_t));

			for (k = 0; k < glyphCount; k++)
			{
				glyphIdArray[k] = GET_SWORD_NET(pf, n);
				n += 2;
			}

			pcmap->pEncodingRecords[j].subtable.pCodeArray = (sword_t*)xmem_alloc(glyphCount * sizeof(sword_t));
			pcmap->pEncodingRecords[j].subtable.pGlyphIdArray = (sword_t*)xmem_alloc(glyphCount * sizeof(sword_t));

			y = 0;
			for (k = 0; k <= headIndex; k++)
			{
				for (x = 0; x <= entryCountArray[k]; x++)
				{
					pcmap->pEncodingRecords[j].subtable.pCodeArray[y] = firstCodeArray[k] + x;
					pcmap->pEncodingRecords[j].subtable.pGlyphIdArray[y] = glyphIdArray[rangeOffsetArray[k] + x];
					y++;
				}
			}

			xmem_free(firstCodeArray);
			xmem_free(entryCountArray);
			xmem_free(idDeltaArray);
			xmem_free(rangeOffsetArray);
			xmem_free(glyphIdArray);
		}
		else if (pcmap->pEncodingRecords[j].subtable.format == 4)
		{
			/*
			uint16	segCountX2	2 × segCount.
			uint16	searchRange	Maximum power of 2 less than or equal to segCount, times 2 ((2**floor(log2(segCount))) * 2, where “**” is an exponentiation operator)
			uint16	entrySelector	Log2 of the maximum power of 2 less than or equal to numTables (log2(searchRange/2), which is equal to floor(log2(numTables)))
			uint16	rangeShift	segCount times 2, minus searchRange ((segCount * 2) - searchRange)
			uint16	endCode[segCount]	End characterCode for each segment, last=0xFFFF.
			uint16	reservedPad	Set to 0.
			uint16	startCode[segCount]	Start character code for each segment.
			int16	idDelta[segCount]	Delta for all character codes in segment.
			uint16	idRangeOffsets[segCount]	Offsets into glyphIdArray or 0
			uint16	glyphIdArray[ ]	Glyph index array (arbitrary length)
			*/
			sword_t segCount, searchRange, entrySelector, rangeShift, reservedPad;
			sword_t *startCodeArray, *endCodeArray, *idRangeOffsetArray, *glyphIdArray;
			short *idDeltaArray;
			sword_t glyphCount = 0;
			sword_t x, y;

			segCount = GET_SWORD_NET(pf, n) / 2;
			n += 2;

			searchRange = GET_SWORD_NET(pf, n);
			n += 2;

			entrySelector = GET_SWORD_NET(pf, n);
			n += 2;

			rangeShift = GET_SWORD_NET(pf, n);
			n += 2;

			endCodeArray = (sword_t*)xmem_alloc((segCount) * sizeof(sword_t));
			startCodeArray = (sword_t*)xmem_alloc((segCount)* sizeof(sword_t));
			idDeltaArray = (short*)xmem_alloc((segCount)* sizeof(short));
			idRangeOffsetArray = (sword_t*)xmem_alloc((segCount)* sizeof(sword_t));

			for (k = 0; k < segCount; k++)
			{
				endCodeArray[k] = GET_SWORD_NET(pf, n);
				n += 2;
			}

			reservedPad = GET_SWORD_NET(pf, n);
			n += 2;

			for (k = 0; k < segCount; k++)
			{
				startCodeArray[k] = GET_SWORD_NET(pf, n);
				n += 2;
			}

			for (k = 0; k < segCount; k++)
			{
				idDeltaArray[k] = (short)GET_SWORD_NET(pf, n);
				n += 2;
			}

			for (k = 0; k < segCount; k++)
			{
				idRangeOffsetArray[k] = GET_SWORD_NET(pf, n);
				n += 2;
			}

			for (k = 0; k < segCount; k++)
			{
				if (startCodeArray[k] != 0xFFFF)
				{
					glyphCount += endCodeArray[k] - startCodeArray[k] + 1;
				}
			}

			pcmap->pEncodingRecords[j].subtable.arrayCount = glyphCount;
			glyphIdArray = (sword_t*)xmem_alloc((glyphCount)* sizeof(sword_t));

			for (k = 0; k < glyphCount; k++)
			{
				glyphIdArray[k] = GET_SWORD_NET(pf, n);
				n += 2;
			}

			pcmap->pEncodingRecords[j].subtable.pCodeArray = (sword_t*)xmem_alloc(glyphCount * sizeof(sword_t));
			pcmap->pEncodingRecords[j].subtable.pGlyphIdArray = (sword_t*)xmem_alloc(glyphCount * sizeof(sword_t));

			y = 0;
			for (k = 0; k < segCount; k++)
			{
				if (startCodeArray[k] == 0xFFFF)
					break;

				for (x = startCodeArray[k]; x <= endCodeArray[k]; x++)
				{
					pcmap->pEncodingRecords[j].subtable.pCodeArray[y] = x;
					if (idRangeOffsetArray[k] == 0)
						pcmap->pEncodingRecords[j].subtable.pGlyphIdArray[y] = glyphIdArray[idDeltaArray[k] + (short)x];
					else
						pcmap->pEncodingRecords[j].subtable.pGlyphIdArray[y] = glyphIdArray[idRangeOffsetArray[k] / 2 + (x - startCodeArray[k])];
					y++;
				}
			}

			xmem_free(startCodeArray);
			xmem_free(endCodeArray);
			xmem_free(idRangeOffsetArray);
			xmem_free(idDeltaArray);
			xmem_free(glyphIdArray);
		}
	}

	return bool_true;
}

void ttf_clear_cmap_table(ttf_cmap_table_t* pcmap)
{
	sword_t k;

	if (pcmap->pEncodingRecords)
	{
		for (k = 0; k < pcmap->numTables; k++)
		{
			xmem_free(pcmap->pEncodingRecords[k].subtable.pCodeArray);
			xmem_free(pcmap->pEncodingRecords[k].subtable.pGlyphIdArray);
		}
		xmem_free(pcmap->pEncodingRecords);
	}

	xmem_zero((void*)pcmap, sizeof(ttf_cmap_table_t));
}

bool_t ttf_load_maxp_table(const ttf_file_head_t* pfile, ttf_maxp_table_t* pmaxp, const byte_t* buf, dword_t size)
{
	sword_t i;
	byte_t *pb;
	dword_t m;

	for (i = 0; i < pfile->numTables; i++)
	{
		if (pfile->pTableRecords[i].tableTag[0] == 'm' && pfile->pTableRecords[i].tableTag[1] == 'a' && pfile->pTableRecords[i].tableTag[2] == 'x' && pfile->pTableRecords[i].tableTag[3] == 'p')
			break;
	}

	if (i == pfile->numTables)
	{
		set_last_error(_T("ttf_load_head_table"), _T("maxp table not find"), -1);
		return bool_false;
	}

	pb = (buf + pfile->pTableRecords[i].offset);
	m = 0;

	pmaxp->version = GET_DWORD_NET(pb, m);
	m += 4;

	pmaxp->numGlyphs = GET_SWORD_NET(pb, m);
	m += 2;

	if (pmaxp->version > 0x00005000)
	{
		pmaxp->maxPoints = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxContours = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxCompositePoints = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxCompositeContours = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxZones = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxTwilightPoints = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxStorage = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxFunctionDefs = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxInstructionDefs = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxStackElements = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxSizeOfInstructions = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxComponentElements = GET_SWORD_NET(pb, m);
		m += 2;

		pmaxp->maxComponentDepth = GET_SWORD_NET(pb, m);
		m += 2;
	}

	return bool_true;
}

void ttf_clear_maxp_table(ttf_maxp_table_t* pmaxp)
{
	xmem_zero((void*)pmaxp, sizeof(ttf_maxp_table_t));
}

bool_t ttf_load_loca_table(const ttf_file_head_t* pfile, int localFormat, int numGlyphs, ttf_loca_table_t* ploca, const byte_t* buf, dword_t size)
{
	sword_t i, k;
	byte_t *pb;
	dword_t m;

	for (i = 0; i < pfile->numTables; i++)
	{
		if (pfile->pTableRecords[i].tableTag[0] == 'l' && pfile->pTableRecords[i].tableTag[1] == 'o' && pfile->pTableRecords[i].tableTag[2] == 'c' && pfile->pTableRecords[i].tableTag[3] == 'a')
			break;
	}

	if (i == pfile->numTables)
	{
		set_last_error(_T("ttf_load_loca_table"), _T("loca table not find"), -1);
		return bool_false;
	}

	pb = (buf + pfile->pTableRecords[i].offset);
	m = 0;

	ploca->pOffsets = (dword_t*)xmem_alloc((numGlyphs + 1) * sizeof(dword_t));

	for (k = 0; k < numGlyphs; k++)
	{
		if (localFormat)
		{
			ploca->pOffsets[k] = GET_DWORD_NET(pb, m);
			m += 4;
		}
		else
		{
			ploca->pOffsets[k] = GET_SWORD_NET(pb, m) * 2;
			m += 2;
		}
	}

	return bool_true;
}

void ttf_clear_loca_table(ttf_loca_table_t* ploca)
{
	if (ploca->pOffsets)
	{
		xmem_free(ploca->pOffsets);
	}
	xmem_zero((void*)ploca, sizeof(ttf_loca_table_t));
}

bool_t ttf_load_glyf_table(const ttf_file_head_t* pfile, const ttf_loca_table_t* ploca, ttf_glyf_table_t* pglyf, int numGlyphs, const byte_t* buf, dword_t size)
{
	sword_t i, j, k;
	byte_t *pb, *pf;
	dword_t m, n;
	byte_t a;
	void* off_glyf;

	for (i = 0; i < pfile->numTables; i++)
	{
		if (pfile->pTableRecords[i].tableTag[0] == 'g' && pfile->pTableRecords[i].tableTag[1] == 'l' && pfile->pTableRecords[i].tableTag[2] == 'y' && pfile->pTableRecords[i].tableTag[3] == 'f')
			break;
	}

	if (i == pfile->numTables)
	{
		set_last_error(_T("ttf_load_glyf_table"), _T("glyf table not find"), -1);
		return bool_false;
	}

	off_glyf = (void*)(buf + pfile->pTableRecords[i].offset);

	for (i = 0; i < (sword_t)(numGlyphs); i++)
	{
		pb = (byte_t*)off_glyf + ploca->pOffsets[i];
		m = 0;
		pglyf++;
		if (ploca->pOffsets[i] == ploca->pOffsets[i + 1])
		{
			pglyf->simpleContour = NULL;
			continue;
		}

		pglyf->numberOfContours = GET_SWORD_NET(pb, m);
		m += 2;

		pglyf->xMin = GET_SWORD_NET(pb, m);
		m += 2;

		pglyf->yMin = GET_SWORD_NET(pb, m);
		m += 2;

		pglyf->xMax = GET_SWORD_NET(pb, m);
		m += 2;

		pglyf->yMax = GET_SWORD_NET(pb, m);
		m += 2;

		if (pglyf->numberOfContours >= 0)
		{
			ttf_simple_glyph_contour_t* pcon = (ttf_simple_glyph_contour_t*)xmem_alloc(sizeof(ttf_simple_glyph_contour_t));
			sword_t pns = 0;

			pcon->pEndPtsOfContours = (sword_t*)xmem_alloc(pglyf->numberOfContours * sizeof(sword_t));
			for (k = 0; k < pglyf->numberOfContours; k++)
			{
				pcon->pEndPtsOfContours[k] = GET_SWORD_NET(pb, m);
				m += 2;

				pns = pcon->pEndPtsOfContours[k] + 1;
			}

			pcon->instructionLength = GET_SWORD_NET(pb, m);
			m += 2;

			pcon->pInstructions = (byte_t*)xmem_alloc(pcon->instructionLength);
			for (k = 0; k < pcon->instructionLength; k++)
			{
				pcon->pInstructions[k] = GET_BYTE(pb, m);
				m++;
			}

			pcon->pFlags = (byte_t*)xmem_alloc(pns);
			for (k = 0; k < pns; k++)
			{
				pcon->pFlags[k] = GET_BYTE(pb, m);
				m++;

				if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_REPEAT_FLAG)
				{
					a = GET_BYTE(pb, m);
					m++;

					pcon->pFlags[k] &= ~SIMPLE_GLYPH_ON_REPEAT_FLAG;

					while (a)
					{
						pcon->pFlags[k + a] = pcon->pFlags[k];
						a--;
						k++;
					}
				}
			}

			pcon->pxCoordinates = (int*)xmem_alloc(pns * sizeof(int));
			for (k = 0; k < pns; k++)
			{
				if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_X_SHORT_VECTOR)
				{
					if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR)
					{
						pcon->pxCoordinates[k] = (unsigned char)GET_BYTE(pb, m);
						m += 1;
					}
					else
					{
						pcon->pxCoordinates[k] = (char)GET_BYTE(pb, m);
						m += 1;
					}
				}
				else
				{
					if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR)
					{
						pcon->pxCoordinates[k] = (k) ? pcon->pxCoordinates[k - 1] : 0;
					}
					else
					{
						pcon->pxCoordinates[k] = (short)GET_SWORD_NET(pb, m);
						m += 2;
					}
				}
			}

			pcon->pyCoordinates = (int*)xmem_alloc(pns * sizeof(int));
			for (k = 0; k < pns; k++)
			{
				if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_Y_SHORT_VECTOR)
				{
					if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR)
					{
						pcon->pyCoordinates[k] = (unsigned char)GET_BYTE(pb, m);
						m += 1;
					}
					else
					{
						pcon->pyCoordinates[k] = (char)GET_BYTE(pb, m);
						m += 1;
					}
				}
				else
				{
					if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR)
					{
						pcon->pyCoordinates[k] = (k) ? pcon->pyCoordinates[k - 1] : 0;
					}
					else
					{
						pcon->pyCoordinates[k] = (short)GET_SWORD_NET(pb, m);
						m += 2;
					}
				}
			}

			pglyf->simpleContour = pcon;
		}
		else
		{
			ttf_composite_glyph_contour_t* pcon;
			ttf_composite_glyph_contour_t* pre;
			sword_t pns = 0;

			while (1)
			{
				pcon = (ttf_composite_glyph_contour_t*)xmem_alloc(sizeof(ttf_composite_glyph_contour_t));

				pcon->flags = GET_SWORD_NET(pb, m);
				m += 2;

				pcon->glyphIndex = GET_SWORD_NET(pb, m);
				m += 2;

				if (pcon->flags & COMPOSITE_GLYPH_ARG_1_AND_2_ARE_WORDS)
				{
					pcon->argument1 = GET_SWORD_NET(pb, m);
					m += 2;

					pcon->argument2 = GET_SWORD_NET(pb, m);
					m += 2;
				}
				else
				{
					pcon->argument1 = GET_BYTE(pb, m);
					m += 1;

					pcon->argument2 = GET_BYTE(pb, m);
					m += 1;
				}

				if (pcon->flags & COMPOSITE_GLYPH_WE_HAVE_A_SCALE) {
					//F2Dot14  scale; 
					pcon->xx = (short)GET_SWORD_NET(pb, m) * 4;
					m += 2;

					pcon->yy = pcon->xx;
				}
				else if (pcon->flags & COMPOSITE_GLYPH_WE_HAVE_AN_X_AND_Y_SCALE) {
					//F2Dot14  xscale;
					//F2Dot14  yscale;
					pcon->xx = (short)GET_SWORD_NET(pb, m) * 4;
					m += 2;

					pcon->yy = (short)GET_SWORD_NET(pb, m) * 4;
					m += 2;
				}
				else if (pcon->flags & COMPOSITE_GLYPH_WE_HAVE_A_TWO_BY_TWO) {
					//F2Dot14  xscale;
					//F2Dot14  scale01;
					//F2Dot14  scale10;
					//F2Dot14  yscale; 
					pcon->xx = (short)GET_SWORD_NET(pb, m) * 4;
					m += 2;

					pcon->yx = (short)GET_SWORD_NET(pb, m) * 4;
					m += 2;

					pcon->xy = (short)GET_SWORD_NET(pb, m) * 4;
					m += 2;

					pcon->yy = (short)GET_SWORD_NET(pb, m) * 4;
					m += 2;
				}

				if (pglyf->compositeContour == NULL)
				{
					pglyf->compositeContour = pcon;
					pre = pcon;
				}
				else
				{
					pre->next = pcon;
					pre = pcon;
				}

				if (!(pcon->flags & COMPOSITE_GLYPH_MORE_COMPONENTS))
					break;
			}
		}
	}

	return bool_true;
}

void ttf_clear_glyf_table(ttf_glyf_table_t* pglyf)
{
	if (pglyf->numberOfContours < 0)
	{
		ttf_composite_glyph_contour_t* pre;

		while ((pre = pglyf->compositeContour) != NULL)
		{
			pglyf->compositeContour = pre->next;

			xmem_free(pre);
		}
	}
	else
	{
		if (pglyf->simpleContour)
		{
			xmem_free(pglyf->simpleContour->pEndPtsOfContours);
			xmem_free(pglyf->simpleContour->pFlags);
			xmem_free(pglyf->simpleContour->pInstructions);
			xmem_free(pglyf->simpleContour->pxCoordinates);
			xmem_free(pglyf->simpleContour->pyCoordinates);
		}
		xmem_free(pglyf->simpleContour);
	}

	xmem_zero((void*)pglyf, sizeof(ttf_glyf_table_t));
}

dword_t ttf_load_head(ttf_file_head_t* pfile, const byte_t* buf, dword_t size)
{
	dword_t m, total = 0;
	sword_t n, k, a;
	dword_t *pbegin, *pend;
	dword_t chksum = 0;
	byte_t* pb;
	void* off_glyf;

	ttf_head_table_t* pt_head;
	ttf_name_table_t* pt_name;
	ttf_cmap_table_t* pt_cmap;
	ttf_maxp_table_t* pt_maxp;
	ttf_loca_table_t* pt_loca;

	pfile->sfntVersion = GET_DWORD_NET(buf, total);
	total += 4;

	pfile->numTables = GET_SWORD_NET(buf, total);
	total += 2;

	pfile->searchRange = GET_SWORD_NET(buf, total);
	total += 2;

	pfile->entrySelector = GET_SWORD_NET(buf, total);
	total += 2;

	pfile->rangeShift = GET_SWORD_NET(buf, total);
	total += 2;

	pfile->pTableRecords = (ttf_table_record_t*)xmem_alloc(pfile->numTables * sizeof(ttf_table_record_t));

	for (n = 0; n < pfile->numTables; n++)
	{
		xmem_copy((void*)(pfile->pTableRecords[n].tableTag), (void*)(buf + total), 4);
		total += 4;

		pfile->pTableRecords[n].checksum = GET_DWORD_NET(buf, total);
		total += 4;

		pfile->pTableRecords[n].offset = GET_DWORD_NET(buf, total);
		total += 4;

		pfile->pTableRecords[n].length = GET_DWORD_NET(buf, total);
		total += 4;

		pbegin = (dword_t*)(buf + pfile->pTableRecords[n].offset);
		pend = pbegin + ((pfile->pTableRecords[n].length + 3) & ~3) / sizeof(dword_t);

		chksum = 0;
		while (pbegin < pend)
		{
			chksum += GET_DWORD_NET(((byte_t*)pbegin), 0);
			pbegin++;
		}

		if (pfile->pTableRecords[n].tableTag[0] == 'h' && pfile->pTableRecords[n].tableTag[1] == 'e' && pfile->pTableRecords[n].tableTag[2] == 'a' && pfile->pTableRecords[n].tableTag[3] == 'd')
		{

		}
		else
		{
			if (chksum != pfile->pTableRecords[n].checksum)
			{
				set_last_error(_T("ttf_load_head"), _T("table checksum error"), -1);
				//return 0;
			}
		}
	}

	for (n = 0; n < pfile->numTables; n++)
	{
		if (pfile->pTableRecords[n].tableTag[0] == 'h' && pfile->pTableRecords[n].tableTag[1] == 'e' && pfile->pTableRecords[n].tableTag[2] == 'a' && pfile->pTableRecords[n].tableTag[3] == 'd')
		{
			pfile->pTableRecords->table = (dword_t*)xmem_alloc(sizeof(ttf_head_table_t));
			pt_head = (ttf_head_table_t*)pfile->pTableRecords->table;

			pb = (buf + pfile->pTableRecords[n].offset);
			m = 0;

			pt_head->majorVersion = GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->minorVersion = GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->fontRevision = GET_DWORD_NET(pb, m);
			m += 4;

			pt_head->checksumAdjustment = GET_DWORD_NET(pb, m);
			m += 4;

			pt_head->magicNumber = GET_DWORD_NET(pb, m);
			m += 4;

			pt_head->flags = GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->unitsPerEm = GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->created = GET_LWORD_NET(pb, m);
			m += 8;

			pt_head->modified = GET_LWORD_NET(pb, m);
			m += 8;

			pt_head->xMin = (short)GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->yMin = (short)GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->xMax = (short)GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->yMax = (short)GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->macStyle = GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->lowestRecPPEM = GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->fontDirectionHint = (short)GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->indexToLocFormat = (short)GET_SWORD_NET(pb, m);
			m += 2;

			pt_head->glyphDataFormat = (short)GET_SWORD_NET(pb, m);
			m += 2;
		}
		else if (pfile->pTableRecords[n].tableTag[0] == 'n' && pfile->pTableRecords[n].tableTag[1] == 'a' && pfile->pTableRecords[n].tableTag[2] == 'm' && pfile->pTableRecords[n].tableTag[3] == 'e')
		{
			pfile->pTableRecords->table = (dword_t*)xmem_alloc(sizeof(ttf_name_table_t));
			pt_name = (ttf_name_table_t*)pfile->pTableRecords->table;

			pb = (buf + pfile->pTableRecords[n].offset);
			m = 0;

			pt_name->version = GET_SWORD_NET(pb, m);
			m += 2;

			pt_name->count = GET_SWORD_NET(pb, m);
			m += 2;

			pt_name->storageOffset = GET_SWORD_NET(pb, m);
			m += 2;

			pt_name->pNameRecords = (ttf_name_record_t*)xmem_alloc(pt_name->count * sizeof(ttf_name_record_t));
			for (k = 0; k < pt_name->count; k++)
			{
				pt_name->pNameRecords[k].platformID = GET_SWORD_NET(pb, m);
				m += 2;

				pt_name->pNameRecords[k].encodingID = GET_SWORD_NET(pb, m);
				m += 2;

				pt_name->pNameRecords[k].languageID = GET_SWORD_NET(pb, m);
				m += 2;

				pt_name->pNameRecords[k].nameID = GET_SWORD_NET(pb, m);
				m += 2;

				pt_name->pNameRecords[k].length = GET_SWORD_NET(pb, m);
				m += 2;

				pt_name->pNameRecords[k].stringOffset = GET_SWORD_NET(pb, m);
				m += 2;
			}

			if (pt_name->version > 0)
			{
				pt_name->langTagCount = GET_SWORD_NET(pb, m);
				m += 2;

				pt_name->pLangTagRecords = (ttf_langtag_record_t*)xmem_alloc(pt_name->langTagCount * sizeof(ttf_langtag_record_t));
				for (k = 0; k < pt_name->langTagCount; k++)
				{
					pt_name->pLangTagRecords[k].length = GET_SWORD_NET(pb, m);
					m += 2;

					pt_name->pLangTagRecords[k].langTagOffset = GET_SWORD_NET(pb, m);
					m += 2;
				}
			}
		}
		else if (pfile->pTableRecords[n].tableTag[0] == 'c' && pfile->pTableRecords[n].tableTag[1] == 'm' && pfile->pTableRecords[n].tableTag[2] == 'a' && pfile->pTableRecords[n].tableTag[3] == 'p')
		{
			pfile->pTableRecords->table = (dword_t*)xmem_alloc(sizeof(ttf_cmap_table_t));
			pt_cmap = (ttf_cmap_table_t*)pfile->pTableRecords->table;

			pb = (buf + pfile->pTableRecords[n].offset);
			m = 0;

			pt_cmap->version = GET_SWORD_NET(pb, m);
			m += 2;

			pt_cmap->numTables = GET_SWORD_NET(pb, m);
			m += 2;

			pt_cmap->pEncodingRecords = (ttf_encoding_record_t*)xmem_alloc(pt_cmap->numTables * sizeof(ttf_encoding_record_t));
			for (k = 0; k < pt_cmap->numTables; k++)
			{
				pt_cmap->pEncodingRecords[k].platformID = GET_SWORD_NET(pb, m);
				m += 2;

				pt_cmap->pEncodingRecords[k].encodingID = GET_SWORD_NET(pb, m);
				m += 2;

				pt_cmap->pEncodingRecords[k].subtableOffset = GET_DWORD_NET(pb, m);
				m += 4;
			}
		}
		else if (pfile->pTableRecords[n].tableTag[0] == 'm' && pfile->pTableRecords[n].tableTag[1] == 'a' && pfile->pTableRecords[n].tableTag[2] == 'x' && pfile->pTableRecords[n].tableTag[3] == 'p')
		{
			pfile->pTableRecords->table = (dword_t*)xmem_alloc(sizeof(ttf_maxp_table_t));
			pt_maxp = (ttf_maxp_table_t*)pfile->pTableRecords->table;

			pb = (buf + pfile->pTableRecords[n].offset);
			m = 0;

			pt_maxp->version = GET_DWORD_NET(pb, m);
			m += 4;

			pt_maxp->numGlyphs = GET_SWORD_NET(pb, m);
			m += 2;

			if (pt_maxp->version > 0x00005000)
			{
				pt_maxp->maxPoints = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxContours = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxCompositePoints = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxCompositeContours = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxZones = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxTwilightPoints = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxStorage = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxFunctionDefs = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxInstructionDefs = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxStackElements = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxSizeOfInstructions = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxComponentElements = GET_SWORD_NET(pb, m);
				m += 2;

				pt_maxp->maxComponentDepth = GET_SWORD_NET(pb, m);
				m += 2;
			}
		}
	}

	for (n = 0; n < pfile->numTables; n++)
	{
		if (pfile->pTableRecords[n].tableTag[0] == 'l' && pfile->pTableRecords[n].tableTag[1] == 'o' && pfile->pTableRecords[n].tableTag[2] == 'c' && pfile->pTableRecords[n].tableTag[3] == 'a')
		{
			pfile->pTableRecords->table = (dword_t*)xmem_alloc(sizeof(ttf_loca_table_t));
			pt_loca = (ttf_loca_table_t*)pfile->pTableRecords->table;

			pb = (buf + pfile->pTableRecords[n].offset);
			m = 0;

			pt_loca->pOffsets = (dword_t*)xmem_alloc((pt_maxp->numGlyphs + 1) * sizeof(dword_t));

			for (k = 0; k < pt_maxp->numGlyphs; k++)
			{
				if (pt_head->indexToLocFormat)
				{
					pt_loca->pOffsets[k] = GET_DWORD_NET(pb, m) * sizeof(dword_t);
					m += 4;
				}
				else
				{
					pt_loca->pOffsets[k] = GET_SWORD_NET(pb, m) * sizeof(sword_t);
					m += 2;
				}
			}
		}
		else if (pfile->pTableRecords[n].tableTag[0] == 'g' && pfile->pTableRecords[n].tableTag[1] == 'l' && pfile->pTableRecords[n].tableTag[2] == 'y' && pfile->pTableRecords[n].tableTag[3] == 'f')
		{
			off_glyf = (void*)(buf + pfile->pTableRecords[n].offset);
		}
	}
	
	for (n = 0; n <= pt_maxp->numGlyphs; n++)
	{
		ttf_glyf_table_t* pt_glyf = (ttf_glyf_table_t*)xmem_alloc(sizeof(ttf_glyf_table_t));

		pb = (byte_t*)off_glyf + pt_loca->pOffsets[n];
		m = 0;

		pt_glyf->numberOfContours = GET_SWORD_NET(pb, m);
		m += 2;

		pt_glyf->xMin = GET_SWORD_NET(pb, m);
		m += 2;

		pt_glyf->yMin = GET_SWORD_NET(pb, m);
		m += 2;

		pt_glyf->xMax = GET_SWORD_NET(pb, m);
		m += 2;

		pt_glyf->yMax = GET_SWORD_NET(pb, m);
		m += 2;

		if (pt_glyf->numberOfContours >= 0)
		{
			ttf_simple_glyph_contour_t* pcon = (ttf_simple_glyph_contour_t*)xmem_alloc(sizeof(ttf_simple_glyph_contour_t));
			sword_t pns = 0;

			pcon->pEndPtsOfContours = (sword_t*)xmem_alloc(pt_glyf->numberOfContours * sizeof(sword_t));
			for (k = 0; k < pt_glyf->numberOfContours; k++)
			{
				pcon->pEndPtsOfContours[k] = GET_SWORD_NET(pb, m);
				m += 2;

				pns = pcon->pEndPtsOfContours[k] + 1;
			}

			pcon->instructionLength = GET_SWORD_NET(pb, m);
			m += 2;

			pcon->pInstructions = (byte_t*)xmem_alloc(pcon->instructionLength);
			for (k = 0; k < pcon->instructionLength; k++)
			{
				pcon->pInstructions[k] = GET_BYTE(pb, m);
				m++;
			}

			pcon->pFlags = (byte_t*)xmem_alloc(pns);
			for (k = 0; k < pns; k++)
			{
				pcon->pFlags[k] = GET_BYTE(pb, m);
				m++;

				if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_REPEAT_FLAG)
				{
					a = GET_BYTE(pb, m);
					m++;

					pcon->pFlags[k] &= ~SIMPLE_GLYPH_ON_REPEAT_FLAG;

					while (a)
					{
						pcon->pFlags[k + a] = pcon->pFlags[k];
						a--;
						k++;
					}
				}
			}

			pcon->pxCoordinates = (int*)xmem_alloc(pns * sizeof(int));
			for (k = 0; k < pns; k++)
			{
				if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_X_SHORT_VECTOR)
				{
					if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR)
					{
						pcon->pxCoordinates[k] = (unsigned char)GET_BYTE(pb, m);
						m += 1;
					}
					else
					{
						pcon->pxCoordinates[k] = (char)GET_BYTE(pb, m);
						m += 1;
					}
				}
				else
				{
					if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR)
					{
						pcon->pxCoordinates[k] = (k) ? pcon->pxCoordinates[k - 1] : 0;
					}
					else
					{
						pcon->pxCoordinates[k] = (short)GET_SWORD_NET(pb, m);
						m += 2;
					}
				}
			}

			pcon->pyCoordinates = (int*)xmem_alloc(pns * sizeof(int));
			for (k = 0; k < pns; k++)
			{
				if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_Y_SHORT_VECTOR)
				{
					if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR)
					{
						pcon->pyCoordinates[k] = (unsigned char)GET_BYTE(pb, m);
						m += 1;
					}
					else
					{
						pcon->pyCoordinates[k] = (char)GET_BYTE(pb, m);
						m += 1;
					}
				}
				else
				{
					if (pcon->pFlags[k] & SIMPLE_GLYPH_ON_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR)
					{
						pcon->pyCoordinates[k] = (k) ? pcon->pyCoordinates[k - 1] : 0;
					}
					else
					{
						pcon->pyCoordinates[k] = (short)GET_SWORD_NET(pb, m);
						m += 2;
					}
				}
			}
		}
		else
		{
			ttf_composite_glyph_contour_t* pcon = (ttf_composite_glyph_contour_t*)xmem_alloc(sizeof(ttf_composite_glyph_contour_t));
			sword_t pns = 0;

			while (1)
			{
				pcon->flags = GET_SWORD_NET(pb, m);
				m += 2;

				pcon->glyphIndex = GET_SWORD_NET(pb, m);
				m += 2;

				if (pcon->flags & COMPOSITE_GLYPH_ARG_1_AND_2_ARE_WORDS)
				{
					pcon->argument1 = GET_SWORD_NET(pb, m);
					m += 2;

					pcon->argument2 = GET_SWORD_NET(pb, m);
					m += 2;
				}
				else
				{
					pcon->argument1 = GET_BYTE(pb, m);
					m += 1;

					pcon->argument2 = GET_BYTE(pb, m);
					m += 1;
				}

				if (pcon->flags & COMPOSITE_GLYPH_WE_HAVE_A_SCALE) {
					//F2Dot14  scale; 
				}
				else if (pcon->flags & COMPOSITE_GLYPH_WE_HAVE_AN_X_AND_Y_SCALE) {
					//F2Dot14  xscale;
					//F2Dot14  yscale;
				}
				else if (pcon->flags & COMPOSITE_GLYPH_WE_HAVE_A_TWO_BY_TWO) {
					//F2Dot14  xscale;
					//F2Dot14  scale01;
					//F2Dot14  scale10;
					//F2Dot14  yscale; 
				}

				if (!(pcon->flags & COMPOSITE_GLYPH_MORE_COMPONENTS))
					break;
			}
		}
	}

	return total;
}
