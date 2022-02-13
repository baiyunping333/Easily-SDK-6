/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc utility document

	@module	points.c | implement file

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

#include "points.h"

#include "../xdkimp.h"
#include "../xdkstd.h"


int ft_parse_points_from_token(xpoint_t* ppt, int max, const tchar_t* token, int len)
{
	tchar_t *key = NULL;
	tchar_t *val = NULL;
	int klen = 0;
	int vlen = 0;
	int count = 0;
	int n, total = 0;

	if (len < 0)
		len = xslen(token);

	while (n = parse_options_token((token + total), (len - total), _T(' '), _T(','), &key, &klen, &val, &vlen))
	{
		total += n;

		if (ppt)
		{
			ppt[count].fx = xsntof(key, klen);
			ppt[count].fy = xsntof(val, vlen);
		}
		count++;
	}

	return count;
}

int ft_format_points_to_token(const xpoint_t* ppt, int n, tchar_t* buf, int max)
{
	int i;
	int len, total = 0;

	for (i = 0; i < n; i++)
	{
		len = xsprintf(NULL, _T("%.1f %.1f,"), ppt[i].fx, ppt[i].fy);
		if (len + total > max)
			return total;

		if (buf)
		{
			len = xsprintf(buf + total, _T("%.1f %.1f,"), ppt[i].fx, ppt[i].fy);
		}
		total += len;
	}

	if (total && buf)
	{
		buf[total - 1] = _T('\0');
	}

	return total;
}

int pt_parse_points_from_token(xpoint_t* ppt, int max, const tchar_t* token, int len)
{
	tchar_t *key = NULL;
	tchar_t *val = NULL;
	int klen = 0;
	int vlen = 0;
	int count = 0;
	int n, total = 0;

	if (len < 0)
		len = xslen(token);

	while (n = parse_options_token((token + total), (len - total), _T(' '), _T(','), &key, &klen, &val, &vlen))
	{
		total += n;

		if (ppt)
		{
			ppt[count].x = xsntol(key, klen);
			ppt[count].y = xsntol(val, vlen);
		}
		count++;
	}

	return count;
}

int pt_format_points_to_token(const xpoint_t* ppt, int n, tchar_t* buf, int max)
{
	int i;
	int len, total = 0;

	for (i = 0; i < n; i++)
	{
		len = xsprintf(NULL, _T("%d %d,"), ppt[i].x, ppt[i].y);
		if (len + total > max)
			return total;

		if (buf)
		{
			len = xsprintf(buf + total, _T("%d %d,"), ppt[i].x, ppt[i].y);
		}
		total += len;
	}

	if (total && buf)
	{
		buf[total - 1] = _T('\0');
	}

	return total;
}

int parse_dicm_point(const tchar_t* token, int len, xpoint_t* ppt, int max)
{
	int i, n;

	if (len < 0)
		len = xslen(token);

	if (is_null(token) || !len)
		return 0;

	i = 0;
	while (*token && len && i < max)
	{
		n = 0;
		while (*token != _T('/') && *token != _T('\\') && *token != _T('\0') && n < len)
		{
			token++;
			n++;
		}
		if (ppt)
		{
			ppt[i].x = xsntol(token - n, n);
		}

		if (*token == _T('/') || *token == _T('\\'))
		{
			token++;
			n++;
		}
		len -= n;

		n = 0;
		while (*token != _T('/') && *token != _T('\\') && *token != _T('\0') && n < len)
		{
			token++;
			n++;
		}
		if (ppt)
		{
			ppt[i].y = xsntol(token - n, n);
		}

		if (*token == _T('/') || *token == _T('\\'))
		{
			token++;
			n++;
		}
		len -= n;

		i++;
	}

	return i;
}

int format_dicm_point(const xpoint_t* ppt, int count, tchar_t* buf, int max)
{
	int i, n, total = 0;;

	for (i = 0; i < count; i++)
	{
		n = xsprintf(((buf) ? (buf + total) : NULL), _T("%d/%d/"), ppt[i].x, ppt[i].y);

		if (total + n > max)
			break;

		total += n;
	}

	if (total)
	{
		buf[total - 1] = _T('\0'); //last /
		total--;
	}

	return total;
}

bool_t inside_rowcol(int row, int col, int from_row, int from_col, int to_row, int to_col)
{
	if (row < from_row || (row == from_row && col < from_col))
		return 0;

	if (row > to_row || (row == to_row && col > to_col))
		return 0;

	return 1;
}

int compare_rowcol(int from_row, int from_col, int to_row, int to_col)
{
	if (from_row == to_row && from_col == to_col)
		return 0;
	else if (from_row == to_row && from_col < to_col)
		return -1;
	else if (from_row == to_row && from_col > to_col)
		return 1;
	else if (from_row < to_row)
		return -1;
	else
		return 1;
}

bool_t pt_inside(int x, int y, int x1, int y1, int x2, int y2)
{
	int minx, maxx, miny, maxy;

	minx = (x1 < x2) ? x1 : x2;
	maxx = (x1 > x2) ? x1 : x2;
	miny = (y1 < y2) ? y1 : y2;
	maxy = (y1 > y2) ? y1 : y2;

	return (x >= minx && x <= maxx && y >= miny && y <= maxy) ? 1 : 0;
}

bool_t ft_inside(float x, float y, float x1, float y1, float x2, float y2)
{
	float minx, maxx, miny, maxy;

	minx = (x1 < x2) ? x1 : x2;
	maxx = (x1 > x2) ? x1 : x2;
	miny = (y1 < y2) ? y1 : y2;
	maxy = (y1 > y2) ? y1 : y2;

	return (x >= minx && x <= maxx && y >= miny && y <= maxy) ? 1 : 0;
}

bool_t pt_in_rect(const xpoint_t* ppt, const xrect_t* pxr)
{
	return pt_inside(ppt->x, ppt->y, pxr->x, pxr->y, pxr->x + pxr->w, pxr->y + pxr->h);
}


bool_t ft_in_rect(const xpoint_t* ppt, const xrect_t* pxr)
{
	return ft_inside(ppt->fx, ppt->fy, pxr->fx, pxr->fy, pxr->fx + pxr->fw, pxr->fy + pxr->fh);
}

void ft_offset_point(xpoint_t* ppt, float cx, float cy)
{
	ppt->fx += cx;
	ppt->fy += cy;
}

void pt_offset_point(xpoint_t* ppt, int cx, int cy)
{
	ppt->x += cx;
	ppt->y += cy;
}

void ft_center_rect(xrect_t* pxr, float cx, float cy)
{
	pxr->fx += (pxr->fw - cx) / 2;
	pxr->fw = cx;
	pxr->fy += (pxr->fh - cy) / 2;
	pxr->fh = cy;
}

void pt_center_rect(xrect_t* pxr, int cx, int cy)
{
	pxr->x += (pxr->w - cx) / 2;
	pxr->w = cx;
	pxr->y += (pxr->h - cy) / 2;
	pxr->h = cy;
}

void pt_expand_rect(xrect_t* pxr, int cx, int cy)
{
	pxr->x -= cx;
	pxr->w += cx * 2;
	pxr->y -= cy;
	pxr->h += cy * 2;
}

void ft_expand_rect(xrect_t* pxr, float cx, float cy)
{
	pxr->fx -= cx;
	pxr->fw += cx * 2;
	pxr->fy -= cy;
	pxr->fh += cy * 2;
}

void pt_offset_rect(xrect_t* pxr, int cx, int cy)
{
	pxr->x += cx;
	pxr->y += cy;
}

void ft_offset_rect(xrect_t* pxr, float cx, float cy)
{
	pxr->fx += cx;
	pxr->fy += cy;
}

void ft_merge_rect(xrect_t* pxr, const xrect_t* pxr_nxt)
{
	float left, top, right, bottom;

	left = (pxr->fx < pxr_nxt->fx) ? pxr->fx : pxr_nxt->fx;
	top = (pxr->fy < pxr_nxt->fy) ? pxr->fy : pxr_nxt->fy;
	right = (pxr->fx + pxr->fw > pxr_nxt->fx + pxr_nxt->fw) ? (pxr->fx + pxr->fw) : (pxr_nxt->fx + pxr_nxt->fw);
	bottom = (pxr->fy + pxr->fh > pxr_nxt->fy + pxr_nxt->fh) ? (pxr->fy + pxr->fh) : (pxr_nxt->fy + pxr_nxt->fh);

	pxr->fx = left;
	pxr->fy = top;
	pxr->fw = right - left;
	pxr->fh = bottom - top;
}

void pt_merge_rect(xrect_t* pxr, const xrect_t* pxr_nxt)
{
	int left, top, right, bottom;

	left = (pxr->x < pxr_nxt->x) ? pxr->x : pxr_nxt->x;
	top = (pxr->y < pxr_nxt->y) ? pxr->y : pxr_nxt->y;
	right = (pxr->x + pxr->w > pxr_nxt->x + pxr_nxt->w) ? (pxr->x + pxr->w) : (pxr_nxt->x + pxr_nxt->w);
	bottom = (pxr->y + pxr->h > pxr_nxt->y + pxr_nxt->h) ? (pxr->y + pxr->h) : (pxr_nxt->y + pxr_nxt->h);

	pxr->x = left;
	pxr->y = top;
	pxr->w = right - left;
	pxr->h = bottom - top;
}

bool_t ft_clip_rect(xrect_t* pxr, const xrect_t* pxrClp)
{
	if (pxr->fx == pxrClp->fx && pxr->fy == pxrClp->fy)
	{
		if (pxr->fh == pxrClp->fh && pxr->fw > pxrClp->fw)
		{
			pxr->fx += pxrClp->fw;
			return 1;
		}
		else if (pxr->fw == pxrClp->fw && pxr->fh > pxrClp->fh)
		{
			pxr->fy += pxrClp->fh;
			return 1;
		}
	}
	else if (pxr->fx + pxr->fw == pxrClp->fx + pxrClp->fw && pxr->fy + pxr->fh == pxrClp->fy + pxrClp->fh)
	{
		if (pxr->fh == pxrClp->fh && pxr->fw > pxrClp->fw)
		{
			pxr->fw -= pxrClp->fw;
			return 1;
		}
		else if (pxr->fw == pxrClp->fw && pxr->fh > pxrClp->fh)
		{
			pxr->fh -= pxrClp->fh;
			return 1;
		}
	}

	return 0;
}

bool_t pt_clip_rect(xrect_t* pxr, const xrect_t* pxrClp)
{
	if (pxr->x == pxrClp->x && pxr->y == pxrClp->y)
	{
		if (pxr->h == pxrClp->h && pxr->w > pxrClp->w)
		{
			pxr->x += pxrClp->w;
			return 1;
		}
		else if (pxr->w == pxrClp->w && pxr->h > pxrClp->h)
		{
			pxr->y += pxrClp->h;
			return 1;
		}
	}
	else if (pxr->x + pxr->w == pxrClp->x + pxrClp->w && pxr->y + pxr->h == pxrClp->y + pxrClp->h)
	{
		if (pxr->h == pxrClp->h && pxr->w > pxrClp->w)
		{
			pxr->w -= pxrClp->w;
			return 1;
		}
		else if (pxr->w == pxrClp->w && pxr->h > pxrClp->h)
		{
			pxr->h -= pxrClp->h;
			return 1;
		}
	}

	return 0;
}

void ft_inter_rect(xrect_t* pxr, const xrect_t* pxr_sub)
{
	if (pxr->fx < pxr_sub->fx)
	{
		pxr->fx = pxr_sub->fx;
		pxr->fw -= (pxr_sub->fx - pxr->fx);
	}
	else
	{
		pxr->fw += (pxr_sub->fx - pxr->fx);
	}

	if (pxr->fy < pxr_sub->fy)
	{
		pxr->fy = pxr_sub->fy;
		pxr->fh -= (pxr_sub->fy - pxr->fy);
	}
	else
	{
		pxr->fh += (pxr_sub->fy - pxr->fy);
	}
}

void pt_inter_rect(xrect_t* pxr, const xrect_t* pxr_sub)
{
	if (pxr->x < pxr_sub->x)
	{
		pxr->x = pxr_sub->x;
		pxr->w -= (pxr_sub->x - pxr->x);
	}
	else
	{
		pxr->w += (pxr_sub->x - pxr->x);
	}

	if (pxr->y < pxr_sub->y)
	{
		pxr->y = pxr_sub->y;
		pxr->h -= (pxr_sub->y - pxr->y);
	}
	else
	{
		pxr->h += (pxr_sub->y - pxr->y);
	}
}

void ft_inter_square(xrect_t* pxr, const xrect_t* pxr_org)
{
	xpoint_t pt;
	float r;

	pt.fx = pxr_org->fx + pxr_org->fw / 2;
	pt.fy = pxr_org->fy + pxr_org->fh / 2;

	r = (pxr_org->fw < pxr_org->fh) ? pxr_org->fw / 2 : pxr_org->fh / 2;

	pxr->fx = pt.fx - r;
	pxr->fy = pt.fy - r;
	pxr->fw = 2 * r;
	pxr->fh = 2 * r;
}

void pt_inter_square(xrect_t* pxr, const xrect_t* pxr_org)
{
	xpoint_t pt;
	int r;

	pt.x = pxr_org->x + pxr_org->w / 2;
	pt.y = pxr_org->y + pxr_org->h / 2;

	r = (pxr_org->w < pxr_org->h) ? pxr_org->w / 2 : pxr_org->h / 2;

	pxr->x = pt.x - r;
	pxr->y = pt.y - r;
	pxr->w = 2 * r;
	pxr->h = 2 * r;
}

void ft_cell_rect(xrect_t* pxr, bool_t horz, int rows, int cols, int index)
{
	int row, col;

	if (horz)
	{
		row = index / cols;
		col = index % cols;
	}
	else
	{
		col = index / rows;
		row = index % rows;
	}

	pxr->fx += pxr->fw * col / cols;
	pxr->fw = pxr->fw / cols;
	pxr->fy += pxr->fh * row / rows;
	pxr->fh = pxr->fh / rows;
}

void pt_cell_rect(xrect_t* pxr, bool_t horz, int rows, int cols, int index)
{
	int row, col;

	if (horz)
	{
		row = index / cols;
		col = index % cols;
	}
	else
	{
		col = index / rows;
		row = index % rows;
	}

	pxr->x += pxr->w * col / cols;
	pxr->w = pxr->w / cols;
	pxr->y += pxr->h * row / rows;
	pxr->h = pxr->h / rows;
}

void empty_rect(xrect_t* pxr)
{
	pxr->fx = pxr->fy = pxr->fw = pxr->fh = 0;
}

bool_t rect_is_empty(const xrect_t* pxr)
{
	return (!pxr->w || !pxr->h) ? 1 : 0;
}

void pt_adjust_rect(xrect_t* pxr, int src_width, int src_height, const tchar_t* horz_align, const tchar_t* vert_align)
{
	if (xscmp(horz_align, GDI_ATTR_TEXT_ALIGN_NEAR) == 0)
	{
		pxr->w = (pxr->w < src_width) ? pxr->w : src_width;
	}
	else if (xscmp(horz_align, GDI_ATTR_TEXT_ALIGN_CENTER) == 0)
	{
		pxr->x = (pxr->w < src_width) ? pxr->x : (pxr->x + (pxr->w - src_width) / 2);
	}
	else if (xscmp(horz_align, GDI_ATTR_TEXT_ALIGN_FAR) == 0)
	{
		pxr->x = (pxr->w < src_width) ? pxr->x : (pxr->x + pxr->w - src_width);
	}

	if (xscmp(vert_align, GDI_ATTR_TEXT_ALIGN_NEAR) == 0)
	{
		pxr->h = (pxr->h < src_height) ? pxr->h : src_height;
	}
	else if (xscmp(vert_align, GDI_ATTR_TEXT_ALIGN_CENTER) == 0)
	{
		pxr->y = (pxr->h < src_height) ? pxr->y : (pxr->y + (pxr->h - src_height) / 2);
	}
	else if (xscmp(vert_align, GDI_ATTR_TEXT_ALIGN_FAR) == 0)
	{
		pxr->y = (pxr->h < src_height) ? pxr->y : (pxr->y + pxr->h - src_height);
	}
}

void ft_adjust_rect(xrect_t* pxr, float src_width, float src_height, const tchar_t* horz_align, const tchar_t* vert_align)
{
	if (xscmp(horz_align, GDI_ATTR_TEXT_ALIGN_NEAR) == 0)
	{
		pxr->fw = (pxr->fw < src_width) ? pxr->fw : src_width;
	}
	else if (xscmp(horz_align, GDI_ATTR_TEXT_ALIGN_CENTER) == 0)
	{
		pxr->fx = (pxr->fw < src_width) ? pxr->fx : (pxr->fx + (pxr->fw - src_width) / 2);
	}
	else if (xscmp(horz_align, GDI_ATTR_TEXT_ALIGN_FAR) == 0)
	{
		pxr->fx = (pxr->fw < src_width) ? pxr->fx : (pxr->fx + pxr->fw - src_width);
	}

	if (xscmp(vert_align, GDI_ATTR_TEXT_ALIGN_NEAR) == 0)
	{
		pxr->fh = (pxr->fh < src_height) ? pxr->fh : src_height;
	}
	else if (xscmp(vert_align, GDI_ATTR_TEXT_ALIGN_CENTER) == 0)
	{
		pxr->fy = (pxr->fh < src_height) ? pxr->fy : (pxr->fy + (pxr->fh - src_height) / 2);
	}
	else if (xscmp(vert_align, GDI_ATTR_TEXT_ALIGN_FAR) == 0)
	{
		pxr->fy = (pxr->fh < src_height) ? pxr->fy : (pxr->fy + pxr->fh - src_height);
	}
}

int ft_quadrant(const xpoint_t* ppo, const xpoint_t* ppt, bool_t sflag)
{
	float dx, dy;

	dx = ppt->fx - ppo->fx;
	dy = ppt->fy - ppo->fy;

	if (IS_ZERO_FLOAT(dx) && IS_ZERO_FLOAT(dy))
		return 0;

	if (dx >= 0.0f && dy >= 0.0f)
		return (sflag) ? 1 : 4;
	else if (dx <= 0.0f && dy >= 0.0f)
		return (sflag) ? 2 : 3;
	else if (dx <= 0.0f && dy <= 0.0f)
		return (sflag) ? 3 : 2;
	else
		return (sflag) ? 4 : 1;
}

int pt_quadrant(const xpoint_t* ppo, const xpoint_t* ppt, bool_t sflag)
{
	int dx, dy;

	dx = ppt->x - ppo->x;
	dy = ppt->y - ppo->y;

	if (!dx && !dy)
		return 0;

	if (dx >= 0 && dy >= 0)
		return (sflag) ? 1 : 4;
	else if (dx <= 0 && dy >= 0)
		return (sflag) ? 2 : 3;
	else if (dx <= 0 && dy <= 0)
		return (sflag) ? 3 : 2;
	else
		return (sflag) ? 4 : 1;
}

void pt_calc_radian(int sweep_flag, int large_flag, int rx, int ry, const xpoint_t* ppt1, const xpoint_t* ppt2, xpoint_t* ppt, double* arc_from, double* arc_sweep)
{
	xpoint_t pt;
	double len, arc1, arc2, arc;
	int n;

	//let ppt1 as the orgin
	pt.x = ppt2->x - ppt1->x;
	pt.y = ppt2->y - ppt1->y;

	//the four quadrant of counterclockewise 
	if (pt.x >= 0 && pt.y >= 0)
		n = 4;
	else if (pt.x <= 0 && pt.y >= 0)
		n = 3;
	else if (pt.x <= 0 && pt.y <= 0)
		n = 2;
	else if (pt.x >= 0 && pt.y <= 0)
		n = 1;

	//abs number
	if (pt.x < 0)
		pt.x = 0 - pt.x;
	if (pt.y < 0)
		pt.y = 0 - pt.y;

	//the half line length (ppt1 to ppt2)
	len = sqrt(pow((double)(pt.x), 2) + pow((double)(pt.y), 2)) / 2;

	if (len >(float)rx) len = rx;

	switch (n)
	{
	case 4:
		if (sweep_flag ^ large_flag)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.x / (double)pt.y);
			arc = (arc1 - arc2);

			//the arc center point
			pt.x = -(int)((float)rx * sin(arc));
			pt.y = (int)((float)rx * cos(arc));

			if (sweep_flag) //sweep clockwise AND small arc
			{
				*arc_from = (double)(XPI / 2 - arc);
				*arc_sweep = -(double)(XPI - arc1 * 2);
			}
			else //sweep counterclockwise AND large arc
			{
				*arc_from = (double)(XPI / 2 - arc);
				*arc_sweep = (double)(XPI + arc1 * 2);
			}
		}
		else
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.y / (double)pt.x);
			arc = (arc1 - arc2);

			//the arc center point
			pt.x = (int)((float)rx * cos(arc));
			pt.y = -(int)((float)rx * sin(arc));

			if (sweep_flag) //sweep clockwise AND large arc
			{
				*arc_from = (double)(XPI + arc);
				*arc_sweep = -(double)(XPI + arc1 * 2);
			}
			else //sweep counterclockwise AND small arc
			{
				*arc_from = (double)(XPI + arc);
				*arc_sweep = (double)(XPI - arc1 * 2);
			}
		}
		break;
	case 3:
		if (sweep_flag ^ large_flag) //(sweep counterclockwise AND large arc) OR (sweep clockwise AND small arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.y / (double)pt.x);
			arc = (arc1 - arc2);

			//the arc center point
			pt.x = -(int)((float)rx * cos(arc));
			pt.y = -(int)((float)rx * sin(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(2 * XPI - arc);
				*arc_sweep = -(double)(XPI - arc1 * 2);
			}
			else
			{
				*arc_from = (float)(2 * XPI - arc);
				*arc_sweep = (double)(XPI + arc1 * 2);
			}
		}
		else  //(sweep counterclockwise AND small arc) OR (sweep clockwise AND large arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.x / (double)pt.y);
			arc = (arc1 - arc2);

			//the arc center point
			pt.x = (int)((float)rx * sin(arc));
			pt.y = (int)((float)rx * cos(arc));

			if (sweep_flag)
			{
				*arc_from = (double)((XPI / 2 + arc));
				*arc_sweep = -(double)(XPI + arc1 * 2);
			}
			else
			{
				*arc_from = (double)(XPI / 2 + arc);
				*arc_sweep = (double)(XPI - arc1 * 2);
			}
		}
		break;
	case 2:
		if (sweep_flag ^ large_flag) //(sweep counterclockwise AND large arc) OR (sweep clockwise AND small arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.x / (double)pt.y);
			arc = (arc1 - arc2);

			//the arc center point
			pt.x = (int)((float)rx * sin(arc));
			pt.y = -(int)((float)rx * cos(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(XPI * 3 / 2 - arc);
				*arc_sweep = -(double)(XPI - arc1 * 2);
			}
			else
			{
				*arc_from = (double)(XPI * 3 / 2 - arc);
				*arc_sweep = (double)(XPI + arc1 * 2);
			}
		}
		else  //(sweep counterclockwise AND small arc) OR (sweep clockwise AND large arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.y / (double)pt.x);
			arc = (arc1 - arc2);

			//the arc center point
			pt.x = -(int)((float)rx * cos(arc));
			pt.y = (int)((float)rx * sin(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(arc);
				*arc_sweep = -(double)(XPI + arc1 * 2);
			}
			else
			{
				*arc_from = (double)(arc);
				*arc_sweep = (double)(XPI - arc1 * 2);
			}
		}
		break;
	case 1:
		if (sweep_flag ^ large_flag) //(sweep counterclockwise AND large arc) OR (sweep clockwise AND small arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.y / (double)pt.x);
			arc = (arc1 - arc2);

			//the arc center point
			pt.x = (int)((float)rx * cos(arc));
			pt.y = (int)((float)rx * sin(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(XPI - arc);
				*arc_sweep = -(double)(XPI - arc1 * 2);
			}
			else
			{
				*arc_from = (double)(XPI - arc);
				*arc_sweep = (double)(XPI + arc1 * 2);
			}
		}
		else  //(sweep counterclockwise AND small arc) OR (sweep clockwise AND large arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.x / (double)pt.y);
			arc = (arc1 - arc2);

			//the arc center point
			pt.x = -(int)((float)rx * sin(arc));
			pt.y = -(int)((float)rx * cos(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(XPI * 3 / 2 + arc);
				*arc_sweep = -(double)(XPI + arc1 * 2);
			}
			else
			{
				*arc_from = (double)(XPI * 3 / 2 + arc);
				*arc_sweep = (double)(XPI - arc1 * 2);
			}
		}
		break;
	}

	//restore the orgin
	ppt->x = pt.x + ppt1->x;
	ppt->y = pt.y + ppt1->y;
}

void ft_calc_radian(int sweep_flag, int large_flag, float rx, float ry, const xpoint_t* ppt1, const xpoint_t* ppt2, xpoint_t* ppt, double* arc_from, double* arc_sweep)
{
	xpoint_t pt;
	double len, arc1, arc2, arc;
	int n;

	//let ppt1 as th orgin
	pt.fx = ppt2->fx - ppt1->fx;
	pt.fy = ppt2->fy - ppt1->fy;

	//the four quadrant of counterclockewise 
	if (pt.x >= 0 && pt.y >= 0)
		n = 4;
	else if (pt.x <= 0 && pt.y >= 0)
		n = 3;
	else if (pt.x <= 0 && pt.y <= 0)
		n = 2;
	else if (pt.x >= 0 && pt.y <= 0)
		n = 1;

	//abs number
	if (pt.fx < 0)
		pt.fx = 0 - pt.fx;
	if (pt.fy < 0)
		pt.fy = 0 - pt.fy;

	//the half line length (ppt1 to ppt2)
	len = sqrt(pow((double)(pt.fx), 2) + pow((double)(pt.fy), 2)) / 2;

	if (len >(float)rx) len = rx;

	switch (n)
	{
	case 4:
		if (sweep_flag ^ large_flag) //(sweep counterclockwise AND large arc) OR (sweep clockwise AND small arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.fx / (double)pt.fy);
			arc = (arc1 - arc2);

			//the arc center point
			pt.fx = -(float)((float)rx * sin(arc));
			pt.fy = (float)((float)rx * cos(arc));

			if (sweep_flag) //sweep clockwise AND small arc
			{
				*arc_from = (double)(XPI / 2 - arc);
				*arc_sweep = -(double)(XPI - arc1 * 2);
			}
			else //sweep counterclockwise AND large arc
			{
				*arc_from = (double)(XPI / 2 - arc);
				*arc_sweep = (double)(XPI + arc1 * 2);
			}
		}
		else  //(sweep counterclockwise AND small arc) OR (sweep clockwise AND large arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.fy / (double)pt.fx);
			arc = (arc1 - arc2);

			//the arc center point
			pt.fx = (float)((float)rx * cos(arc));
			pt.fy = -(float)((float)rx * sin(arc));

			if (sweep_flag) //sweep clockwise AND large arc
			{
				*arc_from = (double)(XPI + arc);
				*arc_sweep = -(double)(XPI + arc1 * 2);
			}
			else //sweep counterclockwise AND small arc
			{
				*arc_from = (double)(XPI + arc);
				*arc_sweep = (double)(XPI - arc1 * 2);
			}
		}
		break;
	case 3:
		if (sweep_flag ^ large_flag) //(sweep counterclockwise AND large arc) OR (sweep clockwise AND small arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.fy / (double)pt.fx);
			arc = (arc1 - arc2);

			//the arc center point
			pt.fx = -(float)((float)rx * cos(arc));
			pt.fy = -(float)((float)rx * sin(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(2 * XPI - arc);
				*arc_sweep = -(double)(XPI - arc1 * 2);
			}
			else
			{
				*arc_from = (float)(2 * XPI - arc);
				*arc_sweep = (double)(XPI + arc1 * 2);
			}
		}
		else  //(sweep counterclockwise AND small arc) OR (sweep clockwise AND large arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.fx / (double)pt.fy);
			arc = (arc1 - arc2);

			//the arc center point
			pt.fx = (float)((float)rx * sin(arc));
			pt.fy = (float)((float)rx * cos(arc));

			if (sweep_flag)
			{
				*arc_from = (double)((XPI / 2 + arc));
				*arc_sweep = -(double)(XPI + arc1 * 2);
			}
			else
			{
				*arc_from = (double)(XPI / 2 + arc);
				*arc_sweep = (double)(XPI - arc1 * 2);
			}
		}
		break;
	case 2:
		if (sweep_flag ^ large_flag) //(sweep counterclockwise AND large arc) OR (sweep clockwise AND small arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.fx / (double)pt.fy);
			arc = (arc1 - arc2);

			//the arc center point
			pt.fx = (float)((float)rx * sin(arc));
			pt.fy = -(float)((float)rx * cos(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(XPI * 3 / 2 - arc);
				*arc_sweep = -(double)(XPI - arc1 * 2);
			}
			else
			{
				*arc_from = (double)(XPI * 3 / 2 - arc);
				*arc_sweep = (double)(XPI + arc1 * 2);
			}
		}
		else  //(sweep counterclockwise AND small arc) OR (sweep clockwise AND large arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.fy / (double)pt.fx);
			arc = (arc1 - arc2);

			//the arc center point
			pt.fx = -(float)((float)rx * cos(arc));
			pt.fy = (float)((float)rx * sin(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(arc);
				*arc_sweep = -(double)(XPI + arc1 * 2);
			}
			else
			{
				*arc_from = (double)(arc);
				*arc_sweep = (double)(XPI - arc1 * 2);
			}
		}
		break;
	case 1:
		if (sweep_flag ^ large_flag) //(sweep counterclockwise AND large arc) OR (sweep clockwise AND small arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.fy / (double)pt.fx);
			arc = (arc1 - arc2);

			//the arc center point
			pt.fx = (float)((float)rx * cos(arc));
			pt.fy = (float)((float)rx * sin(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(XPI - arc);
				*arc_sweep = -(double)(XPI - arc1 * 2);
			}
			else
			{
				*arc_from = (double)(XPI - arc);
				*arc_sweep = (double)(XPI + arc1 * 2);
			}
		}
		else  //(sweep counterclockwise AND small arc) OR (sweep clockwise AND large arc)
		{
			arc1 = acos(len / rx);
			arc2 = atan((double)pt.fx / (double)pt.fy);
			arc = (arc1 - arc2);

			//the arc center point
			pt.fx = -(float)((float)rx * sin(arc));
			pt.fy = -(float)((float)rx * cos(arc));

			if (sweep_flag)
			{
				*arc_from = (double)(XPI * 3 / 2 + arc);
				*arc_sweep = -(double)(XPI + arc1 * 2);
			}
			else
			{
				*arc_from = (double)(XPI * 3 / 2 + arc);
				*arc_sweep = (double)(XPI - arc1 * 2);
			}
		}
		break;
	}

	//restore the orgin
	ppt->fx = pt.fx + ppt1->fx;
	ppt->fy = pt.fy + ppt1->fy;
}

void ft_calc_sector(const xpoint_t* ppt, const xspan_t* prl, const xspan_t* prs, double from, double sweep, xpoint_t* pa, int n)
{
	if (n > 0)
	{
		pa[0].fx = ppt->fx + (float)((float)prl->fr * cos(from));
		pa[0].fy = ppt->fy - (float)((float)prl->fr * sin(from));
	}

	if (n > 1)
	{
		pa[1].fx = ppt->fx + (float)((float)prl->fr * cos(from + sweep));
		pa[1].fy = ppt->fy - (float)((float)prl->fr * sin(from + sweep));
	}

	if (n > 2)
	{
		pa[2].fx = ppt->fx + (float)((float)prs->fr * cos(from + sweep));
		pa[2].fy = ppt->fy - (float)((float)prs->fr * sin(from + sweep));
	}

	if (n > 3)
	{
		pa[3].fx = ppt->fx + (float)((float)prs->fr * cos(from));
		pa[3].fy = ppt->fy - (float)((float)prs->fr * sin(from));
	}
}


void pt_calc_sector(const xpoint_t* ppt, const xspan_t* prl, const xspan_t* prs, double from, double sweep, xpoint_t* pa, int n)
{
	if (n > 0)
	{
		pa[0].x = ppt->x + (int)((float)prl->r * cos(from));
		pa[0].y = ppt->y - (int)((float)prl->r * sin(from));
	}

	if (n > 1)
	{
		pa[1].x = ppt->x + (int)((float)prl->r * cos(from + sweep));
		pa[1].y = ppt->y - (int)((float)prl->r * sin(from + sweep));
	}

	if (n > 2)
	{
		pa[2].x = ppt->x + (int)((float)prs->r * cos(from + sweep));
		pa[2].y = ppt->y - (int)((float)prs->r * sin(from + sweep));
	}

	if (n > 3)
	{
		pa[3].x = ppt->x + (int)((float)prs->r * cos(from));
		pa[3].y = ppt->y - (int)((float)prs->r * sin(from));
	}
}

void pt_calc_equilater(const xpoint_t* ppt, const xspan_t* pxn, xpoint_t* pa, int n)
{
	double a;
	int r;
	int i, j;

	if (n < 3) return;

	r = pxn->r;
	a = 2 * XPI / n;

	if (n % 2)
	{
		pa[0].x = ppt->x;
		pa[0].y = ppt->y - r;

		for (i = 1; i <= n / 2; i++)
		{
			pa[i].x = ppt->x + (int)(r * cos(a * i + XPI / 2));
			pa[i].y = ppt->y - (int)(r * sin(a * i + XPI / 2));
		}

		for (j = i; j < n; j++)
		{
			pa[j].x = ppt->x - (pa[i - 1].x - ppt->x);
			pa[j].y = pa[i - 1].y;
			i--;
		}
	}
	else
	{
		for (i = 0; i < n / 2; i++)
		{
			pa[i].x = ppt->x + (int)(r * cos(a * i + a / 2 + XPI / 2));
			pa[i].y = ppt->y - (int)(r * sin(a * i + a / 2 + XPI / 2));
		}

		for (j = i; j < n; j++)
		{
			pa[j].x = ppt->x - (pa[i - 1].x - ppt->x);
			pa[j].y = pa[i - 1].y;
			i--;
		}
	}
}

void ft_calc_equilater(const xpoint_t* ppt, const xspan_t* pxn, xpoint_t* pa, int n)
{
	double a;
	float fr;
	int i, j;

	if (n < 3) return;

	fr = pxn->fr;
	a = 2 * XPI / n;

	if (n % 2)
	{
		pa[0].fx = ppt->fx;
		pa[0].fy = ppt->fy - fr;

		for (i = 1; i <= n / 2; i++)
		{
			pa[i].fx = ppt->fx + (int)(fr * cos(a * i + XPI / 2));
			pa[i].fy = ppt->fy - (int)(fr * sin(a * i + XPI / 2));
		}

		for (j = i; j < n; j++)
		{
			pa[j].fx = ppt->fx - (pa[i - 1].fx - ppt->fx);
			pa[j].fy = pa[i - 1].fy;
			i--;
		}
	}
	else
	{
		for (i = 0; i < n / 2; i++)
		{
			pa[i].fx = ppt->fx + (int)(fr * cos(a * i + a / 2 + XPI / 2));
			pa[i].fy = ppt->fy - (int)(fr * sin(a * i + a / 2 + XPI / 2));
		}

		for (j = i; j < n; j++)
		{
			pa[j].fx = ppt->fx - (pa[i - 1].fx - ppt->fx);
			pa[j].fy = pa[i - 1].fy;
			i--;
		}
	}
}
