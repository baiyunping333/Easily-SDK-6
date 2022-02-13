/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk utility document

	@module	datetime.h | interface file

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

#ifndef _DATETIME_H
#define _DATETIME_H

#include "../xdkdef.h"

#ifdef LANG_CN
#define AGES_YEAR			_T("岁")
#define AGES_MONTH			_T("月")
#define AGES_DAY			_T("天")
#else
#define AGES_YEAR			_T("Years")
#define AGES_MONTH			_T("Months")
#define AGES_DAY			_T("Days")
#endif

typedef struct _period_t {
	tchar_t base[DATE_LEN + 1]; // base datetime
	tchar_t prec[2];			//time ruler, eg: 'Y','M','D','H','I','S'
	tchar_t feed[INT_LEN + 1]; // increasing step, according to time ruler
}period_t;

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void reset_date(xdate_t* pmd);

EXP_API void get_min_date(xdate_t* pdt);

EXP_API int format_date(const xdate_t* pmd,tchar_t* buf);

EXP_API int format_datetime(const xdate_t* pmd,tchar_t* buf);

EXP_API int format_time(const xdate_t* pmd, tchar_t* buf);

EXP_API void parse_date(xdate_t* pmd, const tchar_t* str);

EXP_API void parse_datetime(xdate_t* pmd, const tchar_t* str);

EXP_API int format_utctime(const xdate_t* pmd, tchar_t* buf);

EXP_API int format_gmttime(const xdate_t* pmd, tchar_t* buf);

EXP_API void parse_gmttime(xdate_t* pmd, const tchar_t* str);

EXP_API int format_datetime_ex(const xdate_t* pxd, const tchar_t* fmt, tchar_t* buf, int max);

EXP_API void parse_datetime_ex(xdate_t* pxd, const tchar_t* fmt, const tchar_t* str);

EXP_API bool_t verify_datetime(const xdate_t* pdt);

EXP_API bool_t is_datetime(const tchar_t* token);

EXP_API int max_mon_days(int year, int mon);

EXP_API int diff_years(const xdate_t* pdt1, const xdate_t* pdt2);

EXP_API int diff_months(const xdate_t* pdt1, const xdate_t* pdt2);

EXP_API int diff_days(const xdate_t* pdt1, const xdate_t* pdt2);

EXP_API int diff_hours(const xdate_t* pdt1, const xdate_t* pdt2);

EXP_API int diff_mins(const xdate_t* pdt1, const xdate_t* pdt2);

EXP_API int diff_secs(const xdate_t* pdt1, const xdate_t* pdt2);

EXP_API void plus_years(xdate_t* pdt, int years);

EXP_API void plus_months(xdate_t* pdt, int months);

EXP_API void plus_days(xdate_t* pdt, int days);

EXP_API void plus_weeks(xdate_t* pdt, int weeks);

EXP_API void plus_hours(xdate_t* pdt, int hours);

EXP_API void plus_minutes(xdate_t* pdt, int minutes);

EXP_API void plus_seconds(xdate_t* pdt, int seconds);

EXP_API void plus_millseconds(xdate_t* pdt, int ms);

EXP_API void calc_period(const period_t* ptp, tchar_t* sz_time);



#ifdef	__cplusplus
}
#endif

#endif