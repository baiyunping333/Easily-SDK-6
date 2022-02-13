/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc money document

	@module	money.c | implement file

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

#include "money.h"

#include "xdkimp.h"
#include "xdkstd.h"


static tchar_t chs_num[10][CHS_LEN + 1] = { _T("零"), _T("壹"), _T("贰"), _T("叁"), _T("肆"), _T("伍"), _T("陆"), _T("柒"), _T("捌"), _T("玖") };
#define CHSUNI_MAX	15
static tchar_t chs_uni[CHSUNI_MAX + 1][CHS_LEN + 1] = { _T("万"), _T("仟"), _T("佰"), _T("拾"), _T("亿"), _T("仟"), _T("佰"), _T("拾"), _T("万"), _T("仟"), _T("佰"), _T("拾"), _T("元"), _T("角"), _T("分"), _T("整") };



int format_money_chs(double dbl, int n_span, tchar_t* buf, int max)
{
	tchar_t sz_num[NUM_LEN + 1] = { 0 };
	tchar_t* token;
	int step, n, count, i, len, pt, total = 0;
	bool_t b_zero;

	if (dbl <= 0)
		return 0;

	numtoxs_dig(dbl, 2, sz_num, NUM_LEN);

	len = 0;
	token = sz_num;
	while (*token != _T('\0') && *token != _T('.'))
	{
		token++;
		len++;
	}

	len = xslen(sz_num) - len;
	if (!len) //###
	{
		*token = _T('0');
		token++;
		*token = _T('0');
		token++;
		*token = _T('\0');
	}
	else if (len == 1) //##.
	{
		*token = _T('0');
		token++;
		*token = _T('0');
		token++;
		*token = _T('\0');
	}
	else if (len == 2) //##.#
	{
		*token = *(token + 1);
		token++;
		*token = _T('0');
		token++;
		*token = _T('\0');
	}
	else if (len == 3) //##.##
	{
		*token = *(token + 1);
		token++;
		*token = *(token + 1);
		token++;
		*token = _T('\0');
	}

	len = (int)(token - sz_num);
	token = sz_num;

	//truncate
	while (len > CHSUNI_MAX)
	{
		token++;
		len--;
	}

	i = CHSUNI_MAX - len;
	step = 5;
	count = 0;
	while (i < CHSUNI_MAX)
	{
		b_zero = (count) ? 0 : 1;
		count = 0;
		for (; i < step; i++)
		{
			n = (int)(*token - _T('0'));
			if (!n_span && b_zero && !n)
			{
				token++;
				continue;
			}

			b_zero = (n) ? 0 : 1;
			if (n)
			{
				count++;
			}

			pt = xslen(chs_num[n]);
			if (total + pt > max)
				return total;

			if (buf)
			{
				xsncpy(buf + total, chs_num[n], pt);
			}
			total += pt;

			if (n_span)
			{
				if (total + n_span > max)
					return total;

				if (buf)
				{
					count = n_span;
					while (count--)
					{
						buf[total + count] = _T(' ');
					}
				}
				total += n_span;

				token++;
				continue;
			}

			if (n)
			{
				pt = xslen(chs_uni[i]);
				if (total + pt > max)
					return total;

				if (buf)
				{
					xsncpy(buf + total, chs_uni[i], pt);
				}
				total += pt;
			}

			token++;
		}

		if (!n_span && i < 13 && count && b_zero)
		{
			pt = xslen(chs_uni[i - 1]);

			if (buf)
			{
				xsncpy(buf + total - pt, chs_uni[i - 1], pt);
			}
		}

		if (!n_span && i == 13 && total)
		{
			pt = xslen(chs_uni[i - 1]);

			if (buf)
			{
				xsncpy(buf + total - pt, chs_uni[i - 1], pt);
			}

			count = 1;
		}

		if (!n_span && i == 15 && b_zero && total)
		{
			pt = xslen(chs_uni[i]);

			if (buf)
			{
				xsncpy(buf + total - pt, chs_uni[i], pt);
			}
		}

		if (i >= 13)
			step = 15;
		else if (i >= 9)
			step = 13;
		else
			step = 9;

		/*switch (i)
		{
		case 5:
		step = 9;
		break;
		case 9:
		step = 13;
		break;
		case 13:
		step = 15;
		break;
		}*/

	}

	if (buf)
	{
		buf[total] = _T('\0');
	}

	return total;
}


