/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc utility document

	@module	xdlutil.c | implement file

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

#include "misce.h"
#include "docattr.h"



static tchar_t week_day[7][4] = { _T("Sun"), _T("Mon"), _T("Tue"), _T("Wed"), _T("Thu"), _T("Fri"), _T("Sat") };

static tchar_t year_mon[12][4] = { _T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"), _T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec") };
static int mon_day[13] = { 29, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static tchar_t chs_num[10][CHS_LEN + 1] = { _T("零"), _T("壹"), _T("贰"), _T("叁"), _T("肆"), _T("伍"), _T("陆"), _T("柒"), _T("捌"), _T("玖") };
#define CHSUNI_MAX	15
static tchar_t chs_uni[CHSUNI_MAX + 1][CHS_LEN + 1] = { _T("万"), _T("仟"), _T("佰"), _T("拾"), _T("亿"), _T("仟"), _T("佰"), _T("拾"), _T("万"), _T("仟"), _T("佰"), _T("拾"), _T("元"), _T("角"), _T("分"), _T("整") };


#ifdef _OS_WINDOWS
static tchar_t calen_week[CALENDAR_COL][UTF_LEN + 1] = { _T("日"), _T("一"), _T("二"), _T("三"), _T("四"), _T("五"), _T("六") };
#else
static tchar_t calen_week[CALENDAR_COL][UTF_LEN + 1] = { _T("Sun"), _T("Mon"), _T("Tue"), _T("Wed"), _T("Thu"), _T("Fri"), _T("Sat") };
#endif


int format_shield(const tchar_t* sz, tchar_t* buf, int max)
{
	int bs, n = 0;

	if (is_null(sz))
		return 0;

	while (n < max && *sz)
	{
#if defined(UNICODE) || defined(_UNICODE)
		bs = ucs_sequence(*sz);
#else
		bs = mbs_sequence(*sz);
#endif

		if (buf)
		{
			if (!n)
			{
				xsncpy((buf + n), sz, bs);
			}
			else
			{
				xscpy((buf + n), _T("*"));
			}
		}

		if (!n)
			n += bs;
		else
			n++;

		sz += bs;
	}

	return n;
}


static tchar_t week_cn[7][UTF_LEN + 1] = { _T("日"), _T("一"), _T("二"), _T("三"), _T("四"), _T("五"), _T("六") };
static tchar_t month_cn[12][UTF_LEN * 2 + 1] = { _T("一"), _T("二"), _T("三"), _T("四"), _T("五"), _T("六"), _T("七"), _T("八"), _T("九"), _T("十"), _T("十一"), _T("十二") };

void cn_date_token(const xdate_t* pdt, tchar_t* year, tchar_t* month, tchar_t* day, tchar_t* week, tchar_t* solar)
{
	tchar_t idate[DATE_LEN + 1] = { 0 };

	if (year)
		xsprintf(year, _T("%d年"), pdt->year);

	if (month)
		xsprintf(month, _T("%s月"), month_cn[pdt->mon-1]);

	if (day)
		xsprintf(day, _T("%d日"), pdt->day);

	if (week)
		xsprintf(week, _T("周%s"), week_cn[pdt->wday]);

	if (solar)
	{
		format_datetime(pdt, idate);
		find_solar_terms(idate, solar, NULL);
	}
}


int compare_data(const tchar_t* szSrc, const tchar_t* szDes, const tchar_t* datatype)
{
	int nSrc, nDes;
	double dbSrc, dbDes;
	short shSrc, shDes;
	int rt;
	xdate_t md1 = { 0 };
	xdate_t md2 = { 0 };

	if (is_null(szSrc) && is_null(szDes))
		return 0;
	else if (is_null(szSrc))
		return -1;
	else if (is_null(szDes))
		return 1;

	if (xscmp(datatype, ATTR_DATA_TYPE_BOOLEAN) == 0)
	{
		shSrc = xstos(szSrc);
		shDes = xstos(szDes);
		if (shSrc > shDes)
			rt = 1;
		else if (shSrc < shDes)
			rt = -1;
		else
			rt = 0;
		return rt;
	}
	else if (xscmp(datatype, ATTR_DATA_TYPE_INTEGER) == 0)
	{
		nSrc = xstol(szSrc);
		nDes = xstol(szDes);
		if (nSrc > nDes)
			rt = 1;
		else if (nSrc < nDes)
			rt = -1;
		else
			rt = 0;
		return rt;
	}
	else if (xscmp(datatype, ATTR_DATA_TYPE_NUMERIC) == 0)
	{
		dbSrc = xstonum(szSrc);
		dbDes = xstonum(szDes);
		if (dbSrc > dbDes)
			rt = 1;
		else if (dbSrc < dbDes)
			rt = -1;
		else
			rt = 0;
		return rt;
	}
	else if (xscmp(datatype, ATTR_DATA_TYPE_DATE) == 0)
	{
		parse_date(&md1, szSrc);
		parse_date(&md2, szDes);
		return compare_date(&md1, &md2);
	}
	else if (xscmp(datatype, ATTR_DATA_TYPE_DATETIME) == 0)
	{
		parse_datetime(&md1, szSrc);
		parse_datetime(&md2, szDes);
		return compare_datetime(&md1, &md2);
	}
	else
	{
		return xscmp(szSrc, szDes);
	}
}

int verify_text(const tchar_t* str, const tchar_t* datatype, bool_t nullable, int len, const tchar_t* min, const tchar_t* max)
{

	if (is_null(str))
		return (nullable) ? veValid : veNull;

	if (xscmp(datatype, ATTR_DATA_TYPE_BOOLEAN) == 0)
	{
		if (xslen(str) > 1)
			return veTruncate;

		if (*str != _T('1') && *str != _T('0'))
			return veDatatype;
	}
	else if (xscmp(datatype, ATTR_DATA_TYPE_STRING) == 0)
	{
		if (len && xslen(str) > len)
			return veTruncate;
	}
	else if (xscmp(datatype, ATTR_DATA_TYPE_INTEGER) == 0 || xscmp(datatype, ATTR_DATA_TYPE_NUMERIC) == 0)
	{
		if (!is_numeric(str))
			return veDatatype;
	}
	else if (xscmp(datatype, ATTR_DATA_TYPE_DATETIME) == 0 || xscmp(datatype, ATTR_DATA_TYPE_DATE) == 0)
	{
		if (!is_datetime(str))
			return veDatatype;
	}

	//verify min and max value
	if (!is_null(min))
	{
		if (compare_data(min, str, datatype) > 0)
			return veOverflow;
	}

	if (!is_null(max))
	{
		if (compare_data(max, str, datatype) < 0)
			return veOverflow;
	}

	return veValid;
}

bool_t get_param_item(const tchar_t* sz_param, const tchar_t* key, tchar_t* val, int max)
{
	const tchar_t* token;
	int len;

	len = xslen(key);

	token = sz_param;
	while ((token = xsstr(token, key)) != NULL)
	{
		if (*(token + len) == _T(':'))
		{
			token += len;
			token++;

			len = 0;
			while (*(token + len) != _T(' ') && *(token + len) != _T('\0'))
			{
				len++;
			}

			xsncpy(val, token, len);
			return 1;
		}
		token += len;
	}

	return 0;
}

int split_line(const tchar_t* token, int len)
{
	int tklen = 0, total = 0;
	bool_t glt = 0;
	const tchar_t* tkcur = token;

	if (len < 0)
		len = xslen(token);

	if (!len)
		return 0;

	while (*tkcur != _T('\r') && *tkcur != _T('\n') && *tkcur != _T('\0') && total < len)
	{
		if (*tkcur == _T('\'') || *tkcur == _T('\"'))
		{
			if (glt)
				glt = 0;
			else
				glt = 1;
		}

		tklen++;
		tkcur++;
		total++;

		if (glt)
		{
			while (*tkcur == _T('\r') || *tkcur == _T('\n'))
			{
				tklen++;
				tkcur++;
				total++;
			}
		}
	}

	return total;
}

bool_t split_xmlns(tchar_t* str, int* kat, int* klen, int* vat, int* vlen)
{
	tchar_t* token = str;

	while (*token != _T(':') && *token != _T('\0'))
	{
		token++;
	}

	if (*token == _T(':'))
	{
		*kat = 0;
		*klen = (int)(token - str);

		*vat = (int)(token - str) + 1;
		*vlen = -1;

		return 1;
	}
	else
	{
		*kat = *klen = 0;
		*vat = 0;
		*vlen = -1;

		return 0;
	}
}

const tchar_t* skip_xmlns(const tchar_t* str, int slen)
{
	tchar_t* token;

	if (slen < 0)
		slen = xslen(str);

	if (!slen)
		return NULL;

	token = (tchar_t*)(str + slen);

	while (*token != _T(':') && token != str)
	{
		token--;
	}

	if (*token == _T(':'))
	{
		token++;
	}

	return token;
}

int trim_xmlns(tchar_t* str, int slen)
{
	tchar_t* token;
	int len;

	if (slen < 0)
		slen = xslen(str);

	if (!slen)
		return 0;

	token = (tchar_t*)(str + slen);

	while (*token != _T(':') && token != str)
	{
		token--;
	}

	if (*token == _T(':'))
	{
		token++;
	}

	len = (int)(token - str);
	xsndel(str, 0, len);

	return slen - len;
}

int compare_nons(const tchar_t* src, int srclen, const tchar_t* dest, int destlen)
{
	const tchar_t *no_src, *no_dest;

	if (srclen < 0)
		srclen = xslen(src);
	no_src = skip_xmlns(src, srclen);
	srclen -= (int)(no_src - src);

	if (destlen < 0)
		destlen = xslen(dest);
	no_dest = skip_xmlns(dest, destlen);
	destlen -= (int)(no_dest - dest);

	return compare_text(no_src, srclen, no_dest, destlen, 1);
}

int printf_path(tchar_t* fpath, const tchar_t* strfmt, ...)
{
	const tchar_t* tname;
	int tlen, total = 0;
	const tchar_t* tk;

	tchar_t path[PATH_LEN + 1] = { 0 };
	tchar_t ekey[RES_LEN + 1];
	tchar_t eval[PATH_LEN + 1];
	int elen;

	va_list arg;

	tk = strfmt;
	while (tk && *tk != _T('\0'))
	{
		if (*tk == _T('$') && *(tk + 1) == _T('('))
		{
			tk += 2;
			tname = tk;
			tlen = 0;
			while (*tk != _T(')'))
			{
				tk++;
				tlen++;
			}
			if (tlen > RES_LEN)
				return 0;

			xsncpy(ekey, tname, tlen);
			ekey[tlen] = _T('\0');

			xszero(eval, PATH_LEN);
			elen = get_envvar(ekey, eval, PATH_LEN);

			xsncpy(path + total, eval, elen);
			
			total += elen;

			if (*tk == _T(')'))
			{
				tk++;
			}
		}
		else
		{
			path[total] = *tk;

			total++;
			
			tk++;
		}

		if (total > PATH_LEN)
			return 0;
	}

	path[total] = _T('\0');
	
	va_start(arg, strfmt);

	total = xsprintf_arg(fpath, path, &arg);

	va_end(arg);

	return total;
}

