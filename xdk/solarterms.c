/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc solar terms document

	@module	solarterms.c | implement file

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

#include "solarterms.h"

#include "xdkimp.h"
#include "xdkstd.h"
#include "xdkutil.h"

typedef struct _SOLARTERMS{
	tchar_t date[DATE_LEN + 1];
	tchar_t name[CHS_LEN * 2 + 1];
}SOLARTERMS;

static SOLARTERMS solar_terms[] = {
	{ _T("2020-02-04 17:03:12"), _T("立春") },
	{ _T("2020-02-19 12:56:53"), _T("雨水") },
	{ _T("2020-03-05 10:56:44"), _T("惊蛰") },
	{ _T("2020-03-20 11:49:29"), _T("春分") },
	{ _T("2020-04-04 15:38:02"), _T("清明") },
	{ _T("2020-04-19 22:45:21"), _T("谷雨") },
	{ _T("2020-05-05 08:51:16"), _T("立夏") },
	{ _T("2020-05-20 21:49:09"), _T("小满") },
	{ _T("2020-06-05 12:58:18"), _T("芒种") },
	{ _T("2020-06-21 05:43:33"), _T("夏至") },
	{ _T("2020-07-06 23:14:20"), _T("小暑") },
	{ _T("2020-07-22 16:36:44"), _T("大暑") },
	{ _T("2020-08-07 09:06:03"), _T("立秋") },
	{ _T("2020-08-22 23:44:48"), _T("处暑") },
	{ _T("2020-09-07 12:07:54"), _T("白露") },
	{ _T("2020-09-22 21:30:32"), _T("秋分") },
	{ _T("2020-10-08 03:55:07"), _T("寒露") },
	{ _T("2020-10-23 06:59:25"), _T("霜降") },
	{ _T("2020-11-07 07:13:46"), _T("立冬") },
	{ _T("2020-11-22 04:39:38"), _T("小雪") },
	{ _T("2020-12-07 00:09:21"), _T("大雪") },
	{ _T("2020-12-21 18:02:12"), _T("冬至") },
	{ _T("2021-01-05 11:23:17"), _T("小寒") },
	{ _T("2021-01-20 04:39:42"), _T("大寒") },

	{ _T("2021-02-03 22:58:39"), _T("立春") },
	{ _T("2021-02-18 18:43:49"), _T("雨水") },
	{ _T("2021-03-05 16:53:32"), _T("惊蛰") },
	{ _T("2021-03-20 17:37:19"), _T("春分") },
	{ _T("2021-04-04 21:34:58"), _T("清明") },
	{ _T("2021-04-20 04:33:14"), _T("谷雨") },
	{ _T("2021-05-05 14:47:01"), _T("立夏") },
	{ _T("2021-05-21 03:36:58"), _T("小满") },
	{ _T("2021-06-05 18:51:57"), _T("芒种") },
	{ _T("2021-06-21 11:32:00"), _T("夏至") },
	{ _T("2021-07-07 05:05:19"), _T("小暑") },
	{ _T("2021-07-22 22:26:16"), _T("大暑") },
	{ _T("2021-08-07 14:53:48"), _T("立秋") },
	{ _T("2021-08-23 05:34:48"), _T("处暑") },
	{ _T("2021-09-07 17:52:46"), _T("白露") },
	{ _T("2021-09-23 03:20:55"), _T("秋分") },
	{ _T("2021-10-08 09:38:53"), _T("寒露") },
	{ _T("2021-10-23 12:51:00"), _T("霜降") },
	{ _T("2021-11-07 12:58:37"), _T("立冬") },
	{ _T("2021-11-22 10:33:34"), _T("小雪") },
	{ _T("2021-12-07 05:56:55"), _T("大雪") },
	{ _T("2021-12-21 23:59:09"), _T("冬至") },
	{ _T("2022-01-05 17:13:54"), _T("小寒") },
	{ _T("2022-01-20 10:38:56"), _T("大寒") },

	{ _T("2022-02-04 04:50:36"), _T("立春") },
	{ _T("2022-02-19 00:42:50"), _T("雨水") },
	{ _T("2022-03-05 22:43:34"), _T("惊蛰") },
	{ _T("2022-03-20 23:33:15"), _T("春分") },
	{ _T("2022-04-05 03:20:03"), _T("清明") },
	{ _T("2022-04-20 10:24:07"), _T("谷雨") },
	{ _T("2022-05-05 20:25:46"), _T("立夏") },
	{ _T("2022-05-21 09:22:25"), _T("小满") },
	{ _T("2022-06-06 00:25:37"), _T("芒种") },
	{ _T("2022-06-21 17:13:40"), _T("夏至") },
	{ _T("2022-07-07 10:37:49"), _T("小暑") },
	{ _T("2022-07-23 04:06:49"), _T("大暑") },
	{ _T("2022-08-07 20:28:57"), _T("立秋") },
	{ _T("2022-08-23 11:15:59"), _T("处暑") },
	{ _T("2022-09-07 23:32:07"), _T("白露") },
	{ _T("2022-09-23 09:03:31"), _T("秋分") },
	{ _T("2022-10-08 15:22:16"), _T("寒露") },
	{ _T("2022-10-23 18:35:31"), _T("霜降") },
	{ _T("2022-11-07 18:45:18"), _T("立冬") },
	{ _T("2022-11-22 16:20:18"), _T("小雪") },
	{ _T("2022-12-07 11:46:04"), _T("大雪") },
	{ _T("2022-12-22 05:48:01"), _T("冬至") },
	{ _T("2023-01-05 23:04:39"), _T("小寒") },
	{ _T("2023-01-20 16:29:20"), _T("大寒") },
};

static int _find_solar_terms(int i, int j, const tchar_t* idate)
{
	int rt, k;
	xdate_t dt1, dt2;

	if (i == j)
	{
		parse_date(&dt1, idate);
		parse_date(&dt1, solar_terms[i].date);

		rt = compare_date(&dt1, &dt2);
		if (!rt)
			return i;
		else
			return -1;
	}
	else if (i < j)
	{
		k = (i + j) / 2;
		parse_date(&dt1, idate);
		parse_date(&dt1, solar_terms[k].date);
		rt = compare_date(&dt1, &dt2);
		if (!rt)
			return k;
		else if (rt > 0)
			return _find_solar_terms(k + 1, j, idate);
		else
			return _find_solar_terms(i, k - 1, idate);
	}
	else
	{
		return -1;
	}
}

void find_solar_terms(const tchar_t* idate, tchar_t* iname, tchar_t* cdate)
{
	int k, i;
	xdate_t dt1, dt2;

	if (is_null(idate))
		return;

	k = sizeof(solar_terms) / sizeof(SOLARTERMS);

	for (i = 0; i < k; i++)
	{
		parse_date(&dt1, idate);
		parse_date(&dt1, solar_terms[i].date);

		if (compare_date(&dt1, &dt2) < 0)
		{
			break;
		}
	}

	i--;

	if (i >= 0 && i < k)
	{
		if (iname)
			xscpy(iname, solar_terms[i].name);
		if (cdate)
			xscpy(cdate, solar_terms[i].date);
	}
}

