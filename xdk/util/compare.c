/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc utility document

	@module	compare.c | implement file

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

#include "compare.h"

#include "../xdkimp.h"
#include "../xdkstd.h"

int compare_datetime(const xdate_t* pmd1, const xdate_t* pmd2)
{
	if (!pmd1 && !pmd2)
		return 0;
	else if (pmd1 && !pmd2)
		return 1;
	else if (!pmd1 && pmd2)
		return -1;

	if (pmd1->year > pmd2->year)
		return 1;
	else if (pmd1->year < pmd2->year)
		return -1;
	else if (pmd1->mon > pmd2->mon)
		return 1;
	else if (pmd1->mon < pmd2->mon)
		return -1;
	else if (pmd1->day > pmd2->day)
		return 1;
	else if (pmd1->day < pmd2->day)
		return -1;
	else if (pmd1->hour > pmd2->hour)
		return 1;
	else if (pmd1->hour < pmd2->hour)
		return -1;
	else if (pmd1->min > pmd2->min)
		return 1;
	else if (pmd1->min < pmd2->min)
		return -1;
	else if (pmd1->sec > pmd2->sec)
		return 1;
	else if (pmd1->sec < pmd2->sec)
		return -1;
	else if (pmd1->millsec > pmd2->millsec)
		return 1;
	else if (pmd1->millsec < pmd2->millsec)
		return -1;
	else
		return 0;
}

int compare_date(const xdate_t* pmd1, const xdate_t* pmd2)
{
	if (!pmd1 && !pmd2)
		return 0;
	else if (pmd1 && !pmd2)
		return 1;
	else if (!pmd1 && pmd2)
		return -1;

	if (pmd1->year > pmd2->year)
		return 1;
	else if (pmd1->year < pmd2->year)
		return -1;
	else if (pmd1->mon > pmd2->mon)
		return 1;
	else if (pmd1->mon < pmd2->mon)
		return -1;
	else if (pmd1->day > pmd2->day)
		return 1;
	else if (pmd1->day < pmd2->day)
		return -1;
	else
		return 0;
}

int compare_time(const xdate_t* pmt1, const xdate_t* pmt2)
{
	if (!pmt1 && !pmt2)
		return 0;
	else if (pmt1 && !pmt2)
		return 1;
	else if (!pmt1 && pmt2)
		return -1;

	if (pmt1->hour > pmt2->hour)
		return 1;
	else if (pmt1->hour < pmt2->hour)
		return -1;
	else if (pmt1->min > pmt2->min)
		return 1;
	else if (pmt1->min < pmt2->min)
		return -1;
	else if (pmt1->sec > pmt2->sec)
		return 1;
	else if (pmt1->sec < pmt2->sec)
		return -1;
	else
		return 0;
}

//compare to text token by len and case options
int compare_text(const tchar_t* src, int srclen, const tchar_t* dest, int destlen, int nocase)
{
	int cmplen;
	int rt;

	if (is_null(src) && is_null(dest))
		return 0;

	if (srclen == -1)
		srclen = xslen(src);
	if (destlen == -1)
		destlen = xslen(dest);

	cmplen = (srclen < destlen) ? srclen : destlen;

	if (nocase)
		rt = xsnicmp(src, dest, cmplen);
	else
		rt = xsncmp(src, dest, cmplen);

	if (rt == 0 && srclen < destlen)
		rt = -1;
	else if (rt == 0 && srclen > destlen)
		rt = 1;

	return rt;
}

int compare_numeric(const tchar_t* szSrc, const tchar_t* szDes, int digi)
{
	double db1, db2;

	db1 = xstonum(szSrc);
	db2 = xstonum(szDes);

	if (db1 == db2)
		return 0;
	else if (db1 > db2)
		return 1;
	else
		return -1;
}


int compare_float(float f1, float f2, int prec)
{
	int c1, c2;
	int n = 1;

	while (prec >= 0)
	{
		n *= 10;
		prec--;
	}

	c1 = (int)(f1 * n);
	c2 = (int)(f2 * n);

	if (c1 > c2)
		return 1;
	else if (c1 < c2)
		return -1;
	else
		return 0;
}

int compare_double(double f1, double f2, int prec)
{
	long long c1, c2;
	int n = 1;

	while (prec >= 0)
	{
		n *= 10;
		prec--;
	}

	c1 = (long long)(f1 * n);
	c2 = (long long)(f2 * n);

	if (c1 > c2)
		return 1;
	else if (c1 < c2)
		return -1;
	else
		return 0;
}
