/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc code page document

	@module	acp_codepage.c | implement file

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

#include "acp.h"

#include "../xdkimp.h"
#include "../xdkstd.h"

xhand_t acp_gb2312 = NULL;
xhand_t acp_unicode = NULL;

int share_gb2312_seek_unicode(unsigned char* mbs, unsigned short* ucs)
{
	int len;
	unsigned short ch;
	int ind;
	byte_t* pb;

	len = gb2312_code_sequence(*mbs);

	if (len == 1)
	{
		if (ucs)
		{
			*ucs = MAKESHORT(mbs[0], 0);
		}
		return 1;
	}

	ch = MAKESHORT(mbs[0], mbs[1]);

	if (!acp_gb2312)
	{
		if (ucs)
		{
			*ucs = ALT_CHAR;
		}
		return 1;
	}

	ind = GB2312_CODE_INDEX(ch);

	pb = xshare_lock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t));
	if (!pb)
	{
		if (ucs)
		{
			*ucs = ALT_CHAR;
		}
		return 1;
	}

	if (ucs)
	{
		*ucs = GET_SWORD_LOC(pb, 0);
	}
	xshare_unlock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t), pb);

	return 1;
}

int share_gb2312_seek_help(const unsigned char* mbs, unsigned char* hlp)
{
	int len;
	unsigned short ch;
	unsigned char bc;
	int ind;
	byte_t* pb;

	len = gb2312_code_sequence(*mbs);
	if (len == 1)
	{
		if (*mbs >= 0x00 && *mbs <= 0x7F)
		{
			if (*mbs >= 'A' && *mbs <= 'Z')
			{
				bc = *mbs;
			}
			else if (*mbs >= 'a' && *mbs <= 'z')
			{
				bc = *mbs - 32;
			}
			else
			{
				bc = 0;
			}
		}
		else
		{
			bc = 0;
		}

		if (hlp) *hlp = bc;

		return (bc) ? 1 : 0;
	}

	if (!acp_gb2312)
	{
		if (hlp) *hlp = 0;

		return 0;
	}

	ch = MAKESHORT(mbs[0], mbs[1]);
	ind = GB2312_CODE_INDEX(ch);

	pb = xshare_lock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t));
	if (!pb)
	{
		if (hlp) *hlp = 0;

		return 0;
	}

	bc = (unsigned char)GET_SWORD_LOC(pb, 2);
	xshare_unlock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t), pb);

	if (hlp) *hlp = bc;

	return (bc) ? 1 : 0;
}

vword_t get_gb2312_code_addr(const byte_t* mbs)
{
	int len;
	unsigned short ch;
	vword_t bc;
	int ind;
	byte_t* pb;

	len = gb2312_code_sequence(*mbs);
	if (len == 1)
	{
		return 0;
	}

	if (!acp_gb2312)
	{
		return 0;
	}

	ch = MAKESHORT(mbs[0], mbs[1]);
	ind = GB2312_CODE_INDEX(ch);

	pb = xshare_lock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t));
	if (!pb)
	{
		return 0;
	}

	bc = (vword_t)GET_VOID_LOC(pb, 4);
	xshare_unlock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t), pb);

	return bc;
}

bool_t set_gb2312_code_addr(const byte_t* mbs, vword_t addr)
{
	int len;
	unsigned short ch;
	int ind;
	byte_t* pb;

	len = gb2312_code_sequence(*mbs);
	if (len == 1)
	{
		return bool_false;
	}

	if (!acp_gb2312)
	{
		return bool_false;
	}

	ch = MAKESHORT(mbs[0], mbs[1]);
	ind = GB2312_CODE_INDEX(ch);

	pb = xshare_lock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t));
	if (!pb)
	{
		return bool_false;
	}

	PUT_VOID_LOC(pb, 4, addr);
	xshare_unlock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t), pb);

	return bool_true;
}

int share_unicode_seek_gb2312(unsigned short ucs, unsigned char* mbs)
{
	int ind;
	byte_t* pb;
	unsigned short ch;

	if (ucs == BIGBOM || ucs == LITBOM)
	{
		if (mbs)
		{
			mbs[0] = ALT_CHAR;
		}
		return 1;
	}

	if (ucs >= 0x0000 && ucs <= 0x007F)
	{
		if (mbs)
		{
			mbs[0] = (unsigned char)ucs;
		}
		return 1;
	}

	if (!acp_unicode)
	{
		if (mbs)
		{
			mbs[0] = ALT_CHAR;
		}
		return 1;
	}

	ind = UNICODE_CODE_INDEX(ucs);

	pb = xshare_lock(acp_unicode, ind * sizeof(acp_t), sizeof(acp_t));
	if (!pb)
	{
		if (mbs)
		{
			mbs[0] = ALT_CHAR;
		}
		return 1;
	}

	ch = GET_SWORD_LOC(pb, 0);
	if (mbs)
	{
		mbs[0] = GETLCHAR(ch);
		mbs[1] = GETHCHAR(ch);
	}
	xshare_unlock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t), pb);

	return 1;
}

int share_unicode_seek_help(unsigned short ucs, unsigned short* hlp)
{
	int ind;
	byte_t* pb;
	unsigned short bc;

	if (ucs >= 0x0000 && ucs <= 0x007F)
	{
		if (ucs >= L'A' && ucs <= L'Z')
		{
			bc = ucs;
		}
		else if (ucs >= L'a' && ucs <= L'z')
		{
			bc = ucs - 32;
		}
		else
		{
			bc = 0;
		}

		if (hlp) *hlp = bc;

		return (bc) ? 1 : 0;
	}

	if (!acp_unicode)
	{
		if (hlp) *hlp = 0;

		return 0;
	}

	ind = UNICODE_CODE_INDEX(ucs);
	pb = xshare_lock(acp_unicode, ind * sizeof(acp_t), sizeof(acp_t));
	if (!pb)
	{
		if (hlp) *hlp = 0;

		return 0;
	}
	bc = GET_SWORD_LOC(pb, 2);
	xshare_unlock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t), pb);

	if (hlp) *hlp = bc;

	return (bc) ? 1 : 0;
}

vword_t get_unicode_code_addr(unsigned short ucs)
{
	int ind;
	byte_t* pb;
	vword_t bc;

	if (ucs >= 0x0000 && ucs <= 0x007F)
	{
		return 0;
	}

	if (!acp_unicode)
	{
		return 0;
	}

	ind = UNICODE_CODE_INDEX(ucs);
	pb = xshare_lock(acp_unicode, ind * sizeof(acp_t), sizeof(acp_t));
	if (!pb)
	{
		return 0;
	}
	bc = GET_VOID_LOC(pb, 4);
	xshare_unlock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t), pb);

	return bc;
}

bool_t set_unicode_code_addr(unsigned short ucs, vword_t addr)
{
	int ind;
	byte_t* pb;

	if (ucs >= 0x0000 && ucs <= 0x007F)
	{
		return bool_false;
	}

	if (!acp_unicode)
	{
		return bool_false;
	}

	ind = UNICODE_CODE_INDEX(ucs);
	pb = xshare_lock(acp_unicode, ind * sizeof(acp_t), sizeof(acp_t));
	if (!pb)
	{
		return 0;
	}
	PUT_VOID_LOC(pb, 4, addr);
	xshare_unlock(acp_gb2312, ind * sizeof(acp_t), sizeof(acp_t), pb);

	return bool_true;
}

static xhand_t load_acp_gb2312(const tchar_t* acp_file, const tchar_t* sha_file)
{
	xhand_t ah = NULL;
	xhand_t sh = NULL;
	byte_t* acp_buf = NULL;
	dword_t n_acp = 0;
	tchar_t fsize[INT_LEN + 1] = { 0 };

	byte_t *pre,*nxt;
	dword_t ind, n, total = 0;
	sword_t sw;
	byte_t pch[8] = { 0 };
	void* ph;

	TRY_CATCH;

	if (!xuncf_file_info(NULL, acp_file, NULL, fsize, NULL, NULL))
	{
		raise_user_error(_T("load_acp_gb2312"), _T("xuncf_file_info"));
	}

	ah = xuncf_open_file(NULL, acp_file, FILE_OPEN_READ);
	if (!ah)
	{
		raise_user_error(_T("load_acp_gb2312"), _T("xuncf_open_file"));
	}

	n_acp = xstol(fsize);
	acp_buf = (byte_t*)xmem_alloc(n_acp);
	
	if (!xuncf_read_file(ah, acp_buf, &n_acp))
	{
		raise_user_error(_T("load_acp_gb2312"), _T("xuncf_read_file"));
	}

	xuncf_close_file(ah);
	ah = NULL;

	n = CHS_GB2312_COUNT * sizeof(acp_t);
	sh = xshare_cli(sha_file, n, FILE_OPEN_CREATE);
	if (!sh)
	{
		raise_user_error(_T("load_acp_gb2312"), _T("xshare_cli"));
	}

	ph = xshare_lock(sh, 0, n);
	if (!ph)
	{
		raise_user_error(_T("load_acp_gb2312"), _T("xshare_lock"));
	}

	nxt = acp_buf;
	while (total < n_acp && *acp_buf != '\0')
	{
		//gb2312 code
		pre = nxt;
		n = 0;
		while (*nxt != ',' && *nxt != '\n' && *nxt != '\0')
		{
			nxt++;
			n++;
		}
		sw = a_hexntol(pre, n);
		if (*nxt == ',')
		{
			nxt++;
			n++;
		}
		total += n;

		ind = GB2312_CODE_INDEX(sw);

		//unicode code
		pre = nxt;
		n = 0;
		while (*nxt != ',' && *nxt != '\n' && *nxt != '\0')
		{
			nxt++;
			n++;
		}
		sw = a_hexntol(pre, n);
		PUT_SWORD_LOC(pch, 0, sw);
		if (*nxt == ',')
		{
			nxt++;
			n++;
		}
		total += n;

		//py code
		pre = nxt;
		n = 0;
		while (*nxt != ',' && *nxt != '\n' && *nxt != '\0')
		{
			nxt++;
			n++;
		}
		sw = ((*pre >= _T('A') && *pre <= _T('Z')) || (*pre >= _T('a') && *pre <= _T('z'))) ? (unsigned short)(pre[0]) : 0;
		PUT_SWORD_LOC(pch, 2, sw);
		if (*nxt == ',')
		{
			nxt++;
			n++;
		}
		total += n;

		if (ind < CHS_GB2312_COUNT)
		{
			xmem_copy(((byte_t*)ph + ind * sizeof(acp_t)), (void*)(pch), 4);
		}

		pre = nxt;
		n = 0;
		if (*nxt == '\n')
		{
			nxt++;
			n++;
		}
		total += n;
	}

	n = CHS_GB2312_COUNT * sizeof(acp_t);
	xshare_unlock(sh, 0, n, ph);

	xmem_free(acp_buf);
	acp_buf = NULL;

	END_CATCH;

	return sh;
ONERROR:
	if (ah) xuncf_close_file(ah);
	if (sh) xshare_close(sh);
	if (acp_buf) xmem_free(acp_buf);

	return NULL;
}

static xhand_t load_acp_unicode(const tchar_t* acp_file, const tchar_t* sha_file)
{
	xhand_t ah = NULL;
	xhand_t sh = NULL;
	byte_t* acp_buf = NULL;
	dword_t n_acp = 0;
	tchar_t fsize[INT_LEN + 1] = { 0 };

	byte_t *pre, *nxt;
	dword_t ind, n, total = 0;
	sword_t sw;
	byte_t pch[8] = { 0 };
	void* ph;

	TRY_CATCH;

	if (!xuncf_file_info(NULL, acp_file, NULL, fsize, NULL, NULL))
	{
		raise_user_error(_T("load_acp_unicode"), _T("xuncf_file_info"));
	}

	ah = xuncf_open_file(NULL, acp_file, FILE_OPEN_READ);
	if (!ah)
	{
		raise_user_error(_T("load_acp_unicode"), _T("xuncf_open_file"));
	}

	n_acp = xstol(fsize);
	acp_buf = (byte_t*)xmem_alloc(n_acp);

	if (!xuncf_read_file(ah, acp_buf, &n_acp))
	{
		raise_user_error(_T("load_acp_unicode"), _T("xuncf_read_file"));
	}

	xuncf_close_file(ah);
	ah = NULL;

	n = CHS_UNICODE_COUNT * sizeof(acp_t);
	sh = xshare_cli(sha_file, n, FILE_OPEN_CREATE);
	if (!sh)
	{
		raise_user_error(_T("load_acp_unicode"), _T("xshare_cli"));
	}

	ph = xshare_lock(sh, 0, n);
	if (!ph)
	{
		raise_user_error(_T("load_acp_unicode"), _T("xshare_lock"));
	}

	nxt = acp_buf;
	while (total < n_acp && *acp_buf != '\0')
	{
		//unicode code
		pre = nxt;
		n = 0;
		while (*nxt != ',' && *nxt != '\n' && *nxt != '\0')
		{
			nxt++;
			n++;
		}
		sw = a_hexntol(pre, n);
		if (*nxt == ',')
		{
			nxt++;
			n++;
		}
		total += n;

		ind = UNICODE_CODE_INDEX(sw);

		//unicode code
		pre = nxt;
		n = 0;
		while (*nxt != ',' && *nxt != '\n' && *nxt != '\0')
		{
			nxt++;
			n++;
		}
		sw = a_hexntol(pre, n);
		PUT_SWORD_LOC(pch, 0, sw);
		if (*nxt == ',')
		{
			nxt++;
			n++;
		}
		total += n;

		//py code
		pre = nxt;
		n = 0;
		while (*nxt != ',' && *nxt != '\n' && *nxt != '\0')
		{
			nxt++;
			n++;
		}
		sw = ((*pre >= _T('A') && *pre <= _T('Z')) || (*pre >= _T('a') && *pre <= _T('z'))) ? (unsigned short)(pre[0]) : 0;
		PUT_SWORD_LOC(pch, 2, sw);
		if (*nxt == ',')
		{
			nxt++;
			n++;
		}
		total += n;

		if (ind < CHS_UNICODE_COUNT)
		{
			xmem_copy(((byte_t*)ph + ind * sizeof(acp_t)), (void*)(pch), 4);
		}

		pre = nxt;
		n = 0;
		if (*nxt == '\n')
		{
			nxt++;
			n++;
		}
		total += n;
	}

	n = CHS_UNICODE_COUNT * sizeof(acp_t);
	xshare_unlock(sh, 0, n, ph);

	xmem_free(acp_buf);
	acp_buf = NULL;

	END_CATCH;

	return sh;
ONERROR:
	if (ah) xuncf_close_file(ah);
	if (sh) xshare_close(sh);
	if (acp_buf) xmem_free(acp_buf);

	return NULL;
}


bool_t acp_init()
{
	tchar_t fpath[PATH_LEN + 1] = { 0 };

	get_runpath(NULL, fpath, PATH_LEN);
	if (is_null(fpath))
		xscpy(fpath, _T("acp/gb2312.acp"));
	else
		xscat(fpath, _T("/acp/gb2312.acp"));

	acp_gb2312 = load_acp_gb2312(fpath, SHARE_GB2312_CODEPAGE);

	get_runpath(NULL, fpath, PATH_LEN);
	if (is_null(fpath))
		xscpy(fpath, _T("acp/unicode.acp"));
	else
		xscat(fpath, _T("/acp/unicode.acp"));

	acp_unicode = load_acp_unicode(fpath, SHARE_UNICODE_CODEPAGE);

	return (acp_gb2312 && acp_unicode) ? bool_true : bool_false;
}

void acp_uninit()
{
	if (acp_gb2312)
	{
		xshare_close(acp_gb2312);
		acp_gb2312 = NULL;
	}

	if (acp_unicode)
	{
		xshare_close(acp_unicode);
		acp_unicode = NULL;
	}
}


