/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc date document

	@module	impdate.c | implement file

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

#include "impdate.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkutil.h"

#ifdef XDK_SUPPORT_DATE

void get_loc_date(xdate_t* pxd)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_get_loc_date)(pxd);
}

void get_utc_date(xdate_t* pxd)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_get_utc_date)(pxd);
}

bool_t mak_loc_week(xdate_t* pxd)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (*pif->pf_mak_loc_week)(pxd);
}

bool_t mak_utc_week(xdate_t* pxd)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (*pif->pf_mak_utc_week)(pxd);
}

bool_t loc_date_to_utc(xdate_t* pxd)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (*pif->pf_loc_date_to_utc)(pxd);
}

bool_t utc_date_to_loc(xdate_t* pxd)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (*pif->pf_utc_date_to_loc)(pxd);
}

dword_t get_times()
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (*pif->pf_get_times)();
}

lword_t get_ticks()
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (lword_t)(*pif->pf_get_ticks)();
}

lword_t get_timestamp()
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	return (lword_t)(*pif->pf_get_timestamp)();
}

void utc_date_from_times(xdate_t* pxd, dword_t s)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_utc_date_from_times)(pxd, s);
}

void utc_date_from_ticks(xdate_t* pxd, lword_t ts)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_utc_date_from_ticks)(pxd, (clock_t)ts);
}

void utc_date_from_timestamp(xdate_t* pxd, lword_t ts)
{
	if_date_t* pif;

	pif = PROCESS_DATE_INTERFACE;

	XDK_ASSERT(pif != NULL);

	(*pif->pf_utc_date_from_timestamp)(pxd, (stamp_t)ts);
}

#else
#include <time.h>

void get_loc_date(xdate_t* pxd)
{
	time_t t;
	struct tm *pts;

	t = time(NULL);
	pts = localtime(&t);

	pxd->year = pts->tm_year + 1900;
	pxd->mon = pts->tm_mon + 1;
	pxd->day = pts->tm_mday;
	pxd->wday = pts->tm_wday;
	pxd->hour = pts->tm_hour;
	pxd->min = pts->tm_min;
	pxd->sec = pts->tm_sec;
}

bool_t mak_loc_date(xdate_t* pxd)
{
	struct tm ts;
	struct tm* pts;
	time_t t;

	ts.tm_year = pxd->year - 1900;
	ts.tm_mon = pxd->mon - 1;
	ts.tm_mday = pxd->day;
	ts.tm_hour = pxd->hour;
	ts.tm_min = pxd->min;
	ts.tm_sec = pxd->sec;

	t = mktime(&ts);
	if (t>=0)
	{
		pts = gmtime(&t);
		pxd->wday = pts->tm_wday;
		return 1;
	}

	return 0;
}

void get_utc_date(xdate_t* pxd)
{
	time_t t;
	struct tm *pts;

	t = time(NULL);
	pts = gmtime(&t);

	pxd->year = pts->tm_year + 1900;
	pxd->mon = pts->tm_mon + 1;
	pxd->day = pts->tm_mday;
	pxd->wday = pts->tm_wday;
	pxd->hour = pts->tm_hour;
	pxd->min = pts->tm_min;
	pxd->sec = pts->tm_sec;
}

bool_t mak_utc_date(xdate_t* pxd)
{
	struct tm ts;
	struct tm* pts;
	time_t t;

	ts.tm_year = pxd->year - 1900;
	ts.tm_mon = pxd->mon - 1;
	ts.tm_mday = pxd->day;
	ts.tm_hour = pxd->hour;
	ts.tm_min = pxd->min;
	ts.tm_sec = pxd->sec;

	t = mktime(&ts);
	if (t>=0)
	{
		pts = gmtime(&t);
		pxd->wday = pts->tm_wday;
		return 1;
	}

	return 0;
}

dword_t get_times()
{
	time_t t;

	t = time(NULL);

	return (dword_t)t;
}

dword_t get_ticks()
{
	clock_t c;

	c = clock();

	return (dword_t)c;
}
#endif //XDK_SUPPORT_DATE

#ifdef XDK_SUPPORT_DATE

#ifdef _OS_WINDOWS
static tchar_t calen_week[CALENDAR_COL][UTF_LEN + 1] = { _T("日"), _T("一"), _T("二"), _T("三"), _T("四"), _T("五"), _T("六") };
#else
static tchar_t calen_week[CALENDAR_COL][UTF_LEN + 1] = { _T("Sun"), _T("Mon"), _T("Tue"), _T("Wed"), _T("Thu"), _T("Fri"), _T("Sat") };
#endif

void default_calendar(calendar_t* pca)
{
	int i, max_day, wday, weeks;

	max_day = max_mon_days(1970, 1);

	for (i = 0; i < CALENDAR_COL; i++)
	{
		xscpy(pca->calen_week[i], calen_week[i]);
	}

	xmem_zero((void*)pca->calen_days, CALENDAR_ROW * CALENDAR_COL * sizeof(int));

	wday = 4;
	weeks = 0;
	for (i = 1; i <= max_day; i++)
	{
		pca->calen_days[weeks][wday] = i;
		wday++;

		if (wday > 6)
		{
			wday = 0;
			weeks++;
		}
	}
}

void fill_calendar(calendar_t* pca, const xdate_t* pdt)
{
	int i, max_day, weeks;
	xdate_t xd = { 0 };

	xd.year = pdt->year;
	xd.mon = pdt->mon;
	xd.day = 1;

	mak_loc_week(&xd);
	max_day = max_mon_days(xd.year, xd.mon);

	for (i = 0; i < CALENDAR_COL; i++)
	{
		xscpy(pca->calen_week[i], calen_week[i]);
	}

	xmem_zero((void*)pca->calen_days, CALENDAR_ROW * CALENDAR_COL * sizeof(int));

	weeks = 0;
	for (i = 1; i <= max_day; i++)
	{
		pca->calen_days[weeks][xd.wday] = i;
		xd.wday++;

		if (xd.wday > 6)
		{
			xd.wday = 0;
			weeks++;
		}
	}
}
#endif
