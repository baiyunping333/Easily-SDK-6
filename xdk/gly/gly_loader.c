/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc glyph loader document

	@module	gly_loader.c | implement file

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

#include "gly.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"

glyph_info_t a_glyph_list[a_alyph_list_length] = {
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("5"), 8, 8, 8, 0, 0x20, 0x20, 223, 1, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("5.5"), 8, 8, 8, 0, 0x20, 0x20, 223, 1, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("6.5"), 16, 16, 16, 0, 0x20, 0x20, 223, 2, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("7.5"), 16, 16, 16, 0, 0x20, 0x20, 223, 2, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("9"), 16, 16, 16, 0, 0x20, 0x20, 223, 2, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("10.5"), 16, 16, 16, 0, 0x20, 0x20, 223, 2, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("12"), 16, 16, 16, 0, 0x20, 0x20, 223, 2, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("14"), 24, 24, 24, 0, 0x20, 0x20, 223, 3, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("15"), 24, 24, 24, 0, 0x20, 0x20, 223, 3, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("16"), 24, 24, 24, 0, 0x20, 0x20, 223, 3, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("18"), 24, 24, 24, 0, 0x20, 0x20, 223, 3, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("22"), 32, 32, 32, 0, 0x20, 0x20, 223, 4, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("24"), 32, 32, 32, 0, 0x20, 0x20, 223, 4, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("26"), 40, 40, 40, 0, 0x20, 0x20, 223, 5, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("36"), 48, 48, 48, 0, 0x20, 0x20, 223, 6, NULL },
	{ _T("ASCII"), _T("Helvetica"), _T("medium"), _T("regular"), _T("42"), 56, 56, 56, 0, 0x20, 0x20, 223, 7, NULL },
};

glyph_info_t c_glyph_list[c_alyph_list_length] = {
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("5"), 8, 8, 8, 0, 0xA1A1, 0xA1A1, 8836, 1, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("5.5"), 8, 8, 8, 0, 0xA1A1, 0xA1A1, 8836, 1, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("6.5"), 16, 16, 16, 0, 0xA1A1, 0xA1A1, 8836, 2, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("7.5"), 16, 16, 16, 0, 0xA1A1, 0xA1A1, 8836, 2, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("9"), 16, 16, 16, 0, 0xA1A1, 0xA1A1, 8836, 2, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("10.5"), 16, 16, 16, 0, 0xA1A1, 0xA1A1, 8836, 2, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("12"), 16, 16, 16, 0, 0xA1A1, 0xA1A1, 8836, 2, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("14"), 24, 24, 24, 0, 0xA1A1, 0xA1A1, 8836, 3, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("15"), 24, 24, 24, 0, 0xA1A1, 0xA1A1, 8836, 3, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("16"), 24, 24, 24, 0, 0xA1A1, 0xA1A1, 8836, 3, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("18"), 24, 24, 24, 0, 0xA1A1, 0xA1A1, 8836, 3, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("22"), 32, 32, 32, 0, 0xA1A1, 0xA1A1, 8836, 4, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("24"), 32, 32, 32, 0, 0xA1A1, 0xA1A1, 8836, 4, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("26"), 40, 40, 40, 0, 0xA1A1, 0xA1A1, 8836, 5, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("36"), 48, 48, 48, 0, 0xA1A1, 0xA1A1, 8836, 6, NULL },
	{ _T("GB2312"), _T("Helvetica"), _T("medium"), _T("regular"), _T("42"), 56, 56, 56, 0, 0xA1A1, 0xA1A1, 8836, 7, NULL },
};


static dword_t load_glyph_header(byte_t* buf, dword_t len, glyph_info_t* gpm)
{
	byte_t* pre;
	dword_t n, total = 0;

	//glyph encode
	pre = buf;
	n = 0;
	while (*buf != ',' && *buf != '\n' && *buf != '\0')
	{
		buf++;
		n++;
	}
#if defined(_UNICODE) || defined(UNICODE)
	utf8_to_ucs(pre, n, gpm->charset, 31);
#else
	utf8_to_mbs(pre, n, gpm->charset, 31);
#endif

	if (*buf == ',')
	{
		buf++;
		n++;
	}
	total += n;

	//glyph count
	pre = buf;
	n = 0;
	while (*buf != ',' && *buf != '\n' && *buf != '\0')
	{
		buf++;
		n++;
	}
	gpm->characters = a_xsntol(pre, n);

	if (*buf == ',')
	{
		buf++;
		n++;
	}
	total += n;

	//pixel width
	pre = buf;
	n = 0;
	while (*buf != ',' && *buf != '\n' && *buf != '\0')
	{
		buf++;
		n++;
	}
	gpm->width = a_xsntol(pre, n);

	if (*buf == ',')
	{
		buf++;
		n++;
	}
	total += n;

	//pixel heght
	pre = buf;
	n = 0;
	while (*buf != ',' && *buf != '\n' && *buf != '\0')
	{
		buf++;
		n++;
	}
	gpm->height = a_xsntol(pre, n);

	if (*buf == ',')
	{
		buf++;
		n++;
	}
	total += n;

	//name
	pre = buf;
	n = 0;
	while (*buf != ',' && *buf != '\n' && *buf != '\0')
	{
		buf++;
		n++;
	}
#if defined(_UNICODE) || defined(UNICODE)
	utf8_to_ucs(pre, n, gpm->name, 31);
#else
	utf8_to_mbs(pre, n, gpm->name, 31);
#endif

	if (*buf == ',')
	{
		buf++;
		n++;
	}
	total += n;


	//weight
	pre = buf;
	n = 0;
	while (*buf != ',' && *buf != '\n' && *buf != '\0')
	{
		buf++;
		n++;
	}
#if defined(_UNICODE) || defined(UNICODE)
	utf8_to_ucs(pre, n, gpm->weight, 31);
#else
	utf8_to_mbs(pre, n, gpm->weight, 31);
#endif

	if (*buf == ',')
	{
		buf++;
		n++;
	}
	total += n;

	//style
	pre = buf;
	n = 0;
	while (*buf != ',' && *buf != '\n' && *buf != '\0')
	{
		buf++;
		n++;
	}
#if defined(_UNICODE) || defined(UNICODE)
	utf8_to_ucs(pre, n, gpm->style, 31);
#else
	utf8_to_mbs(pre, n, gpm->style, 31);
#endif

	if (*buf == ',')
	{
		buf++;
		n++;
	}
	total += n;

	//size
	pre = buf;
	n = 0;
	while (*buf != ',' && *buf != '\n' && *buf != '\0')
	{
		buf++;
		n++;
	}
#if defined(_UNICODE) || defined(UNICODE)
	utf8_to_ucs(pre, n, gpm->size, 31);
#else
	utf8_to_mbs(pre, n, gpm->size, 31);
#endif

	if (*buf == ',')
	{
		buf++;
		n++;
	}
	total += n;

	//end header
	if (*buf == '\n')
	{
		buf++;
		total++;
	}

	return total;
}

static dword_t load_glyph_pixmap(byte_t* buf, dword_t len, glyph_info_t* gpm)
{
	byte_t* pre;
	dword_t m, n, total = 0;
	byte_t pch[3] = { 0 };
	sword_t sw;
	int w, h;
	int i, j, k;
	byte_t *pp, *pb;
	int ind;
	bool_t a;
	tchar_t sname[RES_LEN + 1] = { 0 };

	xsprintf(sname, _T("%s-%s-%s-%s-%s"),
		gpm->charset,
		gpm->name,
		gpm->weight,
		gpm->style,
		gpm->size);

	//width + pixmap
	m = (2 + gpm->bytesperline * gpm->height);
	gpm->glyph = xshare_cli(sname, m * gpm->characters, FILE_OPEN_CREATE);
	if (!gpm->glyph)
		return 0;

	pp = xshare_lock(gpm->glyph, 0, m * gpm->characters);
	if (!pp)
		return 0;

	a = xsicmp(gpm->charset, _T("GB2312"));

	while (total < len && *buf != '\0')
	{
		//char code
		pre = buf;
		n = 0;
		while (*buf != ',' && *buf != '\n' && *buf != '\0')
		{
			buf++;
			n++;
		}
		sw = a_hexntol(pre, n);
		pch[0] = GETHBYTE(sw);
		pch[1] = GETLBYTE(sw);

		if (*buf == ',')
		{
			buf++;
			n++;
		}
		total += n;

		//char width
		pre = buf;
		n = 0;
		while (*buf != ',' && *buf != '\n' && *buf != '\0')
		{
			buf++;
			n++;
		}
		w = a_xsntol(pre, n);

		if (*buf == ',')
		{
			buf++;
			n++;
		}
		total += n;

		//char height
		pre = buf;
		n = 0;
		while (*buf != ',' && *buf != '\n' && *buf != '\0')
		{
			buf++;
			n++;
		}
		h = a_xsntol(pre, n);

		if (*buf == '\n')
		{
			buf++;
			n++;
		}
		total += n;

		//glyph index
		if (a)
			ind = pch[1] - gpm->firstchar;
		else
			ind = GB2312_GLYPH_INDEX(pch);

		if (ind < 0)
		{
			ind = 0;
		}

		pb = pp + ind * m;
		PUT_SWORD_LOC(pb, 0, (sword_t)w);

		pb += 2;
		//char pixmap
		for (i = 0; i < gpm->height; i++)
		{
			k = i * gpm->bytesperline;
			for (j = 0; j < gpm->bytesperline; j++)
			{
				pre = buf;
				n = 0;
				while (*buf != ' ' && *buf != '\n' && *buf != '\0')
				{
					buf++;
					n++;
				}
				pb[k++] = (byte_t)a_hexntol(pre, n);

				if (*buf == ' ')
				{
					buf++;
					n++;
				}
				total += n;
			}

			if (*buf == '\n')
			{
				buf++;
				total++;
			}
		}
	}

	return total;
}

bool_t load_glyph_info(const tchar_t* fname, glyph_info_t* gpm)
{
	tchar_t fsize[INT_LEN + 1] = { 0 };
	xhand_t fhand = NULL;
	dword_t dw, off;
	byte_t* buf = NULL;

	TRY_CATCH;

	if (!xuncf_file_info(NULL, fname, NULL, fsize, NULL, NULL))
	{
		raise_user_error(_T("load_glyph"), _T("xuncf_file_info"));
	}

	fhand = xuncf_open_file(NULL, fname, FILE_OPEN_READ);
	if (!fhand)
	{
		raise_user_error(_T("load_glyph"), _T("xuncf_open_file"));
	}

	dw = xstol(fsize);
	buf = (byte_t*)xmem_alloc(dw);

	if (!xuncf_read_file(fhand, buf, &dw))
	{
		raise_user_error(_T("load_glyph"), _T("xuncf_read_file"));
	}

	xuncf_close_file(fhand);
	fhand = NULL;

	off = load_glyph_header(buf, dw, gpm);

	load_glyph_pixmap((buf + off), (dw - off), gpm);

	xmem_free(buf);
	buf = NULL;

	END_CATCH;

	return bool_true;
ONERROR:
	if (fhand) xuncf_close_file(fhand);
	if (buf) xmem_free(buf);

	return bool_false;
}

bool_t gly_init()
{
	int n,i;
	tchar_t fpath[PATH_LEN + 1] = { 0 };
	tchar_t fname[PATH_LEN + 1] = { 0 };
	tchar_t token[RES_LEN + 1] = { 0 };

	get_runpath(NULL, fpath, PATH_LEN);

	n = sizeof(c_glyph_list) / sizeof(glyph_info_t);
	for (i = 0; i < n; i++)
	{
		xsprintf(token, _T("%s-%s-%s-%s-%s"), 
			c_glyph_list[i].charset,
			c_glyph_list[i].name, 
			c_glyph_list[i].weight, 
			c_glyph_list[i].style, 
			c_glyph_list[i].size);

		if (is_null(fpath))
			xsprintf(fname, _T("gly/%s.gly"), token);
		else
			xsprintf(fname, _T("%s/gly/%s.gly"), fpath, token);

		load_glyph_info(fname, &(c_glyph_list[i]));
	}

	n = sizeof(a_glyph_list) / sizeof(glyph_info_t);
	for (i = 0; i < n; i++)
	{
		xsprintf(token, _T("%s-%s-%s-%s-%s"),
			a_glyph_list[i].charset,
			a_glyph_list[i].name,
			a_glyph_list[i].weight,
			a_glyph_list[i].style,
			a_glyph_list[i].size);

		if (is_null(fpath))
			xsprintf(fname, _T("gly/%s.gly"), token);
		else
			xsprintf(fname, _T("%s/gly/%s.gly"), fpath, token);

		load_glyph_info(fname, &(a_glyph_list[i]));
	}

	return bool_true;
}

void gly_uninit()
{
	int n, i;

	n = sizeof(c_glyph_list) / sizeof(glyph_info_t);

	for (i = 0; i < n; i++)
	{
		if (c_glyph_list[i].glyph)
		{
			xshare_close(c_glyph_list[i].glyph);
			c_glyph_list[i].glyph = NULL;
		}
	}

	n = sizeof(a_glyph_list) / sizeof(glyph_info_t);

	for (i = 0; i < n; i++)
	{
		if (a_glyph_list[i].glyph)
		{
			xshare_close(a_glyph_list[i].glyph);
			a_glyph_list[i].glyph = NULL;
		}
	}
}