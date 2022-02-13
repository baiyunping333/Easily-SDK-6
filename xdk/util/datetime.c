/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc utility document

	@module	datetime.c | implement file

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

#include "datetime.h"
#include "compare.h"

#include "../xdkimp.h"
#include "../xdkstd.h"

static tchar_t week_day[7][4] = { _T("Sun"), _T("Mon"), _T("Tue"), _T("Wed"), _T("Thu"), _T("Fri"), _T("Sat") };

static tchar_t year_mon[12][4] = { _T("Jan"), _T("Feb"), _T("Mar"), _T("Apr"), _T("May"), _T("Jun"), _T("Jul"), _T("Aug"), _T("Sep"), _T("Oct"), _T("Nov"), _T("Dec") };
static int mon_day[13] = { 29, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };


int format_date(const xdate_t* pmd, tchar_t* buf)
{
	return xsprintf(buf, _T("%d-%02d-%02d"), pmd->year, pmd->mon, pmd->day);
}

int format_datetime(const xdate_t* pmd, tchar_t* buf)
{
	return xsprintf(buf, _T("%d-%02d-%02d %02d:%02d:%02d"), pmd->year, pmd->mon, pmd->day, pmd->hour, pmd->min, pmd->sec);
}

int format_time(const xdate_t* pmd, tchar_t* buf)
{
	return xsprintf(buf, _T("%02d:%02d:%02d"), pmd->hour, pmd->min, pmd->sec);
}

int format_ages(const xdate_t* bday, const xdate_t* tday, tchar_t* buf)
{
	if (bday->year < tday->year)
	{
		return xsprintf(buf, _T("%d%s"), tday->year - bday->year + 1, AGES_YEAR);
	}
	else if (bday->year == tday->year)
	{
		if (bday->mon < tday->mon)
		{
			return xsprintf(buf, _T("%d%s"), tday->mon - bday->mon + 1, AGES_MONTH);
		}
		else if (bday->mon == tday->mon)
		{
			if (bday->day <= tday->day)
			{
				return xsprintf(buf, _T("%d%s"), tday->day - bday->day + 1, AGES_DAY);
			}
		}
	}

	return 0;
}

void reset_date(xdate_t* pmd)
{
	pmd->year = 1970;
	pmd->mon = 1;
	pmd->day = 1;
	pmd->hour = pmd->min = pmd->sec = 0;
	pmd->millsec = 0;
	pmd->wday = 0;
}

void parse_date(xdate_t* pmd, const tchar_t* text)
{
	tchar_t* tmp;
	int i;
	tchar_t token[5];

	pmd->year = 1970;
	pmd->mon = 1;
	pmd->day = 1;
	pmd->hour = pmd->min = pmd->sec = 0;
	pmd->millsec = 0;
	pmd->wday = 0;

	if (is_null(text))
		return;

	tmp = (tchar_t*)text;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 4)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->year = xstos(token);
	if (*tmp == _T('\0') || i == 0)
		return;

	while ((*tmp < _T('0') || *tmp > _T('9')) && *tmp != _T('\0'))
		tmp++;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 2)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->mon = xstos(token);
	if (*tmp == _T('\0') || i == 0)
		return;

	while ((*tmp < _T('0') || *tmp > _T('9')) && *tmp != _T('\0'))
		tmp++;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 2)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->day = xstos(token);
}

void parse_datetime(xdate_t* pmd, const tchar_t* text)
{
	tchar_t* tmp;
	int i;
	tchar_t token[5];

	pmd->year = 1970;
	pmd->mon = 1;
	pmd->day = 1;
	pmd->hour = pmd->min = pmd->sec = 0;
	pmd->millsec = 0;
	pmd->wday = 0;

	if (is_null(text))
		return;

	tmp = (tchar_t*)text;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 4)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->year = xstos(token);
	if (*tmp == _T('\0') || i == 0)
		return;

	while ((*tmp < _T('0') || *tmp > _T('9')) && *tmp != _T('\0'))
		tmp++;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 2)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->mon = xstos(token);
	if (*tmp == _T('\0') || i == 0)
		return;

	while ((*tmp < _T('0') || *tmp > _T('9')) && *tmp != _T('\0'))
		tmp++;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 2)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->day = xstos(token);
	if (*tmp == _T('\0') || i == 0)
		return;

	while ((*tmp < _T('0') || *tmp > _T('9')) && *tmp != _T('\0'))
		tmp++;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 2)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->hour = xstos(token);
	if (*tmp == _T('\0') || i == 0)
		return;

	while ((*tmp < _T('0') || *tmp > _T('9')) && *tmp != _T('\0'))
		tmp++;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 2)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->min = xstos(token);
	if (*tmp == _T('\0') || i == 0)
		return;

	while ((*tmp < _T('0') || *tmp > _T('9')) && *tmp != _T('\0'))
		tmp++;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 2)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}

	pmd->sec = xstos(token);
	if (*tmp == _T('\0') || i == 0)
		return;

	while ((*tmp < _T('0') || *tmp > _T('9')) && *tmp != _T('\0'))
		tmp++;

	i = 0;
	token[i] = _T('\0');
	while (*tmp != _T('\0') && *tmp >= _T('0') && *tmp <= _T('9') && i < 3)
	{
		token[i++] = *tmp;
		token[i] = _T('\0');
		tmp++;
	}
	pmd->millsec = xstos(token);
}

int format_utctime(const xdate_t* pdt, tchar_t* buf)
{
	return xsprintf(buf, _T("%d-%02d-%02dT%02d:%02d:%02d.%03dZ"), pdt->year, pdt->mon, pdt->day, pdt->hour, pdt->min, pdt->sec, pdt->millsec);
}

int format_gmttime(const xdate_t* pdt, tchar_t* buf)
{
	return xsprintf(buf, _T("%s, %02d %s %04d %02d:%02d:%02d GMT"), week_day[pdt->wday], pdt->day, year_mon[pdt->mon - 1], pdt->year, pdt->hour, pdt->min, pdt->sec);
}

void parse_gmttime(xdate_t* pdt, const tchar_t* str)
{
	tchar_t* token = (tchar_t*)str;
	tchar_t* key;
	int i, klen;

	pdt->year = 1970;
	pdt->mon = 1;
	pdt->day = 1;
	pdt->hour = 0;
	pdt->min = 0;
	pdt->sec = 0;
	pdt->millsec = 0;
	pdt->wday = 0;

	if (is_null(str))
		return;

	key = token;
	klen = 0;
	while (*token != _T('\0') && *token != _T(',') && *token != _T(' '))
	{
		token++;
		klen++;
	}

	for (i = 0; i < 7; i++)
	{
		if (compare_text(week_day[i], -1, key, klen, 1) == 0)
		{
			pdt->wday = i;
			break;
		}
	}

	while (*token != _T('\0') && (*token == _T(',') || *token == _T(' ')))
		token++;

	key = token;
	klen = 0;
	while (*token != _T('\0') && *token != _T(',') && *token != _T(' '))
	{
		token++;
		klen++;
	}

	pdt->day = xstos(key);

	while (*token != _T('\0') && (*token == _T(',') || *token == _T(' ')))
		token++;

	key = token;
	klen = 0;
	while (*token != _T('\0') && *token != _T(',') && *token != _T(' '))
	{
		token++;
		klen++;
	}

	for (i = 0; i < 12; i++)
	{
		if (compare_text(year_mon[i], -1, key, klen, 1) == 0)
		{
			pdt->mon = i + 1;
			break;
		}
	}

	while (*token != _T('\0') && (*token == _T(',') || *token == _T(' ')))
		token++;

	key = token;
	klen = 0;
	while (*token != _T('\0') && *token != _T(',') && *token != _T(' '))
	{
		token++;
		klen++;
	}

	pdt->year = xstos(key);

	while (*token != _T('\0') && (*token == _T(',') || *token == _T(' ')))
		token++;

	key = token;
	klen = 0;
	while (*token != _T('\0') && *token != _T(':'))
	{
		token++;
		klen++;
	}

	pdt->hour = xstos(key);

	if (*token == _T(':'))
		token++;

	key = token;
	klen = 0;
	while (*token != _T('\0') && *token != _T(':'))
	{
		token++;
		klen++;
	}

	pdt->min = xstos(key);

	if (*token == _T(':'))
		token++;

	key = token;
	klen = 0;
	while (*token != _T('\0') && *token != _T(':'))
	{
		token++;
		klen++;
	}

	pdt->sec = xstos(key);
}

int format_datetime_ex(const xdate_t* pxd, const tchar_t* fmt, tchar_t* buf, int max)
{
	int y_count, m_count, d_count, h_count, i_count, s_count;
	const tchar_t* tk_at;
	int tk_len, total = 0;

	if (is_null(fmt))
		return format_datetime(pxd, buf);

	while (*fmt != _T('\0'))
	{
		tk_at = fmt;
		tk_len = 0;
		while (*fmt != _T('\0') && *fmt != _T('y') && *fmt != _T('Y') && *fmt != _T('m') && *fmt != _T('M') && *fmt != _T('d') && *fmt != _T('D') && *fmt != _T('h') && *fmt != _T('H') && *fmt != _T('i') && *fmt != _T('I') && *fmt != _T('s') && *fmt != _T('S'))
		{
			tk_len++;
			fmt++;
		}

		if (total + tk_len > max)
			return total;

		if (buf)
		{
			xsncpy(buf + total, tk_at, tk_len);
		}
		total += tk_len;

		y_count = 0;
		while (*fmt == _T('y') || *fmt == _T('Y'))
		{
			y_count++;
			fmt++;
		}

		if (total + y_count > max)
			return total;

		if (buf && y_count)
		{
			y_count = xsprintf(buf + total, _T("%04d"), pxd->year);
		}
		total += y_count;

		m_count = 0;
		while (*fmt == _T('m') || *fmt == _T('M'))
		{
			m_count++;
			fmt++;
		}

		if (total + m_count > max)
			return total;

		if (buf && m_count)
		{
			m_count = xsprintf(buf + total, _T("%02d"), pxd->mon);
		}
		total += m_count;

		d_count = 0;
		while (*fmt == _T('d') || *fmt == _T('D'))
		{
			d_count++;
			fmt++;
		}

		if (total + d_count > max)
			return total;

		if (buf && d_count)
		{
			d_count = xsprintf(buf + total, _T("%02d"), pxd->day);
		}
		total += d_count;

		h_count = 0;
		while (*fmt == _T('h') || *fmt == _T('H'))
		{
			h_count++;
			fmt++;
		}

		if (total + h_count > max)
			return total;

		if (buf && h_count)
		{
			h_count = xsprintf(buf + total, _T("%02d"), pxd->hour);
		}
		total += h_count;

		i_count = 0;
		while (*fmt == _T('i') || *fmt == _T('I'))
		{
			i_count++;
			fmt++;
		}

		if (total + i_count > max)
			return total;

		if (buf && i_count)
		{
			i_count = xsprintf(buf + total, _T("%02d"), pxd->min);
		}
		total += i_count;

		s_count = 0;
		while (*fmt == _T('s') || *fmt == _T('S'))
		{
			s_count++;
			fmt++;
		}

		if (total + s_count > max)
			return total;

		if (buf && s_count)
		{
			s_count = xsprintf(buf + total, _T("%02d"), pxd->sec);
		}
		total += s_count;
	}

	return total;
}

void parse_datetime_ex(xdate_t* pxd, const tchar_t* fmt, const tchar_t* str)
{
	const tchar_t* tkat;
	int tklen;
	tchar_t ch;

	if (is_null(fmt) || is_null(str))
	{
		parse_datetime(pxd, str);
		return;
	}

	while (*fmt != _T('\0'))
	{
		while (*fmt != _T('\0') && *fmt != _T('y') && *fmt != _T('Y') && *fmt != _T('m') && *fmt != _T('M') && *fmt != _T('d') && *fmt != _T('D') && *fmt != _T('h') && *fmt != _T('H') && *fmt != _T('i') && *fmt != _T('I') && *fmt != _T('s') && *fmt != _T('S'))
		{
			fmt++;
		}

		while (*str != _T('\0') && (*str < _T('0') || *str > _T('9')))
		{
			str++;
		}

		tkat = str;
		tklen = 0;
		while (*str >= _T('0') && *str <= _T('9'))
		{
			str++;
			tklen++;
		}

		if (*fmt == _T('y') || *fmt == _T('Y'))
		{
			pxd->year = xsntos(tkat, tklen);
			if (tklen < 4)
				pxd->year += 2000;
		}
		else if (*fmt == _T('m') || *fmt == _T('M'))
		{
			pxd->mon = xsntos(tkat, tklen);
		}
		else if (*fmt == _T('d') || *fmt == _T('D'))
		{
			pxd->day = xsntos(tkat, tklen);
		}
		else if (*fmt == _T('h') || *fmt == _T('H'))
		{
			pxd->hour = xsntos(tkat, tklen);
		}
		else if (*fmt == _T('i') || *fmt == _T('I'))
		{
			pxd->min = xsntos(tkat, tklen);
		}
		else if (*fmt == _T('s') || *fmt == _T('S'))
		{
			pxd->sec = xsntos(tkat, tklen);
		}

		ch = *fmt;
		fmt++;
		while (ch == *fmt)
			fmt++;

		if (*str == _T('\0'))
			break;
	}
}

int max_mon_days(int year, int mon)
{
	if (year < MIN_YEAR || year > MAX_YEAR)
		return 0;

	if (mon < 1 || mon > 12)
		return 0;

	if (year % 4 == 0 && mon == 2)
	{
		return mon_day[0];
	}
	else
	{
		return mon_day[mon];
	}
}

int max_year_days(int year)
{
	if (year < MIN_YEAR || year > MAX_YEAR)
		return 0;

	if (year % 4 == 0)
	{
		return 366;
	}
	else
	{
		return 365;
	}
}

int diff_years(const xdate_t* pdt1, const xdate_t* pdt2)
{
	return pdt2->year - pdt1->year;
}

int diff_months(const xdate_t* pdt1, const xdate_t* pdt2)
{
	return (pdt2->year - pdt1->year) * 12 + (pdt2->mon - pdt1->mon);
}

int diff_days(const xdate_t* pdt1, const xdate_t* pdt2)
{
	int rt;
	int min_year, max_year, min_mon, max_mon, min_day, max_day;
	int n, days = 0;

	rt = compare_date(pdt1, pdt2);
	if (!rt)
		return 0;

	if (rt < 0)
	{
		min_year = pdt1->year;
		max_year = pdt2->year;
		min_mon = pdt1->mon;
		max_mon = pdt2->mon;
		min_day = pdt1->day;
		max_day = pdt2->day;
	}
	else
	{
		min_year = pdt2->year;
		max_year = pdt1->year;
		min_mon = pdt2->mon;
		max_mon = pdt1->mon;
		min_day = pdt2->day;
		max_day = pdt1->day;
	}

	for (n = 1; n < min_mon; n++)
	{
		days -= max_mon_days(min_year, n);
	}

	days -= min_day;

	for (n = min_year; n < max_year; n++)
	{
		days += max_year_days(n);
	}

	for (n = 1; n < max_mon; n++)
	{
		days += max_mon_days(max_year, n);
	}

	days += max_day;

	return (rt < 0) ? days : -days;
}

int diff_hours(const xdate_t* pdt1, const xdate_t* pdt2)
{
	int n;

	n = diff_days(pdt1, pdt2);

	return n * 24 + pdt2->hour - pdt1->hour;
}

int diff_mins(const xdate_t* pdt1, const xdate_t* pdt2)
{
	int n;

	n = diff_hours(pdt1, pdt2);

	return n * 60 + pdt2->min - pdt1->min;
}

int diff_secs(const xdate_t* pdt1, const xdate_t* pdt2)
{
	int n;

	n = diff_mins(pdt1, pdt2);

	return n * 60 + pdt2->sec - pdt1->sec;
}

void plus_years(xdate_t* pdt, int years)
{
	pdt->year += years;

	if (pdt->year % 4 != 0 && pdt->mon == 2 && pdt->day == mon_day[0])
	{
		pdt->day = mon_day[pdt->mon];
	}
}

void plus_months(xdate_t* pdt, int months)
{
	if (months < 0)
	{
		pdt->year += (pdt->mon - 12 + months) / 12;
		pdt->mon = 12 + (pdt->mon - 12 + months) % 12;
	}
	else
	{
		pdt->year += (pdt->mon + months) / 12;
		pdt->mon = (pdt->mon + months) % 12;
		if (!pdt->mon)
		{
			pdt->year--;
			pdt->mon = 12;
		}
	}

	if (pdt->day > mon_day[pdt->mon])
		pdt->day = mon_day[pdt->mon];
}

void plus_days(xdate_t* pdt, int days)
{
	while (days)
	{
		if (days > 0)
		{
			if (pdt->day + 1 > mon_day[pdt->mon] || (pdt->year % 4 == 0 && pdt->mon == 2 && pdt->day + 1 > mon_day[0]))
			{
				pdt->mon++;
				if (pdt->mon > 12)
				{
					pdt->year++;
					pdt->mon = 1;
				}
				pdt->day = 1;
			}
			else
			{
				pdt->day++;
			}
			days--;
		}
		else
		{
			if (pdt->day - 1 < 1)
			{
				pdt->mon--;
				if (pdt->mon < 1)
				{
					pdt->year--;
					pdt->mon = 12;
				}
				pdt->day = (pdt->year % 4 == 0 && pdt->mon == 2) ? mon_day[0] : mon_day[pdt->mon];
			}
			else
			{
				pdt->day--;
			}
			days++;
		}
	}
}

void plus_weeks(xdate_t* pdt, int weeks)
{
	plus_days(pdt, weeks * 7);
}

void plus_hours(xdate_t* pdt, int hours)
{
	int days;

	hours += pdt->hour;

	days = hours / 24;
	hours = hours % 24;

	if (days)
	{
		plus_days(pdt, days);
	}

	pdt->hour = hours;
}

void plus_minutes(xdate_t* pdt, int minutes)
{
	int hours;

	minutes += pdt->min;

	hours = minutes / 60;
	minutes = minutes % 60;

	if (hours)
	{
		plus_hours(pdt, hours);
	}

	pdt->min = minutes;
}

void plus_seconds(xdate_t* pdt, int seconds)
{
	int minutes;

	seconds += pdt->sec;

	minutes = seconds / 60;
	seconds = seconds % 60;

	if (minutes)
	{
		plus_minutes(pdt, minutes);
	}

	pdt->sec = seconds;
}

void plus_millseconds(xdate_t* pdt, int ms)
{
	int seconds;

	seconds = (pdt->millsec + ms) / 1000;

	if (seconds)
	{
		plus_seconds(pdt, seconds);
	}

	pdt->millsec = (pdt->millsec + ms) % 1000;
}

void get_min_date(xdate_t* pdt)
{
	parse_datetime(pdt, SYS_MINDATE);
}

void calc_period(const period_t* ptp, tchar_t* sz_time)
{
	xdate_t dt = { 0 };

	parse_datetime(&dt, ptp->base);

	switch (ptp->prec[0])
	{
	case _T('Y'):
	case _T('y'):
		plus_years(&dt, xstol(ptp->feed));
		break;
	case _T('M'):
	case _T('m'):
		plus_months(&dt, xstol(ptp->feed));
		break;
	case _T('D'):
	case _T('d'):
		plus_days(&dt, xstol(ptp->feed));
		break;
	case _T('H'):
	case _T('h'):
		plus_hours(&dt, xstol(ptp->feed));
		break;
	case _T('I'):
	case _T('i'):
		plus_minutes(&dt, xstol(ptp->feed));
		break;
	case _T('S'):
	case _T('s'):
		plus_seconds(&dt, xstol(ptp->feed));
		break;
	}

	format_datetime(&dt, sz_time);
}

bool_t verify_datetime(const xdate_t* pdt)
{
	if (pdt->year < MIN_YEAR || pdt->year > MAX_YEAR)
		return 0;

	if (pdt->mon < 1 || pdt->mon > 12)
		return 0;

	if (pdt->year % 4 == 0 && pdt->mon == 2)
	{
		if (pdt->day < 1 || pdt->day > mon_day[0])
			return 0;
	}
	else
	{
		if (pdt->day < 1 || pdt->day > mon_day[pdt->mon])
			return 0;
	}

	if (pdt->hour < 0 || pdt->hour > 23)
		return 0;

	if (pdt->min < 0 || pdt->min > 59)
		return 0;

	if (pdt->sec < 0 || pdt->sec > 59)
		return 0;

	return 1;
}

bool_t is_datetime(const tchar_t* token)
{
	xdate_t xd = { 0 };

	parse_datetime(&xd, token);

	return verify_datetime(&xd);
}
