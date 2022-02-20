/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc fit document

	@module	fit_line.c | implement file

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

#include "fit.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"

/**********************************************************************
* Bresenham Alogorithm (x0, y0) line to (x1, y1)
* dx = |x1 - x0|, dy = |y1 - y0|
* when dy/dx < 1 then:
* p = 0, y = y1
* for x = x1 to x2 do
*    plot(x, y)
*    if(2 * (p + dy) < dx)
*	    p = p + dy
*    else
*      p = p + dy - dx, y = y + 1
*    end if
* end for
* when dy/dx > 1 then:
* p = 0, x = x1
* for y = y1 to y2 do
*    plot(x, y)
*    if(2 * (p + dx) < dy)
*	    p = p + dx
*    else
*      p = p + dx - dy, x = x + 1
*    end if
* end for
***********************************************************************/

/**********************************************************************
les = 1, neg = 1
++++++++++++++++++++++++++++++|++++++*++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|++++**+++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|+++*+++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|+i*o+++++++++++++++++++++++++++++++++++
------------------------------|-------------------------------x->-----
++++++++++++++++++++++++++++++|+i*o+++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|+++*+++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|++++**+++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|++++++*++++++++++++++++++++++++++++++++
les = 1, neg = 0
***********************************************************************/

/**********************************************************************
les = 0, neg = 1
++++++++++++++++++++++++++++++|+++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|+++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|+++++++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++++++|+++++++++++++++++++++++++++++++++++++++
------------------------------|-------------------------------x->-----
++++++++++++++++++++++++++o*i+|+o*i+++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++++*+++|+++*+++++++++++++++++++++++++++++++++++
++++++++++++++++++++++++**++++|++++**+++++++++++++++++++++++++++++++++
+++++++++++++++++++++++*++++++|++++++*++++++++++++++++++++++++++++++++
                              |les = 0, neg = 0
***********************************************************************/

static int fill_neghbour(bool_t les, bool_t inc, int nx, int ny, int ps, xpoint_t* ppt)
{
	int i, mo, mi, n = 0;
	int cx, cy;

	mo = (ps - 1) / 2 + (ps - 1) % 2;//outside of baseline  dots
	mi = (ps - 1) / 2; //inside of baseline dots

	cx = ppt[n].x;
	cy = ppt[n].y;
	n++;

	if (les)
	{
		for (i = 0; i < mo; i++)
		{
			if (ny > 0)
			{
				if (inc)
				{
					ppt[n].x = ((i) ? ppt[n - 2].x : cx) + 1;
					ppt[n].y = ((i) ? ppt[n - 2].y : cy) + 1;
					n++;

					ppt[n].x = ppt[n - 1].x;
					ppt[n].y = ppt[n - 1].y - 1;
					n++;
				}
				else
				{
					ppt[n].x = ((i) ? ppt[n - 1].x : cx) + 1;
					ppt[n].y = ((i) ? ppt[n - 1].y : cy) + 1;
					n++;
				}
			}
			else if (ny < 0)
			{
				if (inc)
				{
					ppt[n].x = ((i) ? ppt[n - 2].x : cx) + 1;
					ppt[n].y = ((i) ? ppt[n - 2].y : cy) - 1;
					n++;

					ppt[n].x = ppt[n - 1].x;
					ppt[n].y = ppt[n - 1].y + 1;
					n++;
				}
				else
				{
					ppt[n].x = ((i) ? ppt[n - 1].x : cx) + 1;
					ppt[n].y = ((i) ? ppt[n - 1].y : cy) - 1;
					n++;
				}
			}
			else
			{
				ppt[n].x = ((i) ? ppt[n - 1].x : cx);
				ppt[n].y = ((i) ? ppt[n - 1].y : cy) - 1;
				n++;
			}
		}

		for (i = 0; i < mi; i++)
		{
			if (ny > 0)
			{
				if (inc)
				{
					ppt[n].x = ((i) ? ppt[n - 2].x : cx) - 1;
					ppt[n].y = ((i) ? ppt[n - 2].y : cy) - 1;
					n++;

					ppt[n].x = ppt[n - 1].x + 1;
					ppt[n].y = ppt[n - 1].y;
					n++;
				}
				else
				{
					ppt[n].x = ((i) ? ppt[n - 1].x : cx) - 1;
					ppt[n].y = ((i) ? ppt[n - 1].y : cy) - 1;
					n++;
				}
			}
			else if (ny < 0)
			{
				if (inc)
				{
					ppt[n].x = ((i) ? ppt[n - 2].x : cx) - 1;
					ppt[n].y = ((i) ? ppt[n - 2].y : cy) + 1;
					n++;

					ppt[n].x = ppt[n - 1].x + 1;
					ppt[n].y = ppt[n - 1].y;
					n++;
				}
				else
				{
					ppt[n].x = ((i) ? ppt[n - 1].x : cx) - 1;
					ppt[n].y = ((i) ? ppt[n - 1].y : cy) + 1;
					n++;
				}
			}
			else
			{
				ppt[n].x = ((i) ? ppt[n - 1].x : cx);
				ppt[n].y = ((i) ? ppt[n - 1].y : cy) + 1;
				n++;
			}
		}
	}
	else
	{
		for (i = 0; i < mo; i++)
		{
			if (nx > 0)
			{
				if (inc)
				{
					ppt[n].x = ((i) ? ppt[n - 2].x : cx) - 1;
					ppt[n].y = ((i) ? ppt[n - 2].y : cy) - 1;
					n++;

					ppt[n].x = ppt[n - 1].x;
					ppt[n].y = ppt[n - 1].y + 1;
					n++;
				}
				else
				{
					ppt[n].x = ((i) ? ppt[n - 1].x : cx) - 1;
					ppt[n].y = ((i) ? ppt[n - 1].y : cy) - 1;
					n++;
				}
			}
			else if (nx < 0)
			{
				if (inc)
				{
					ppt[n].x = ((i) ? ppt[n - 2].x : cx) - 1;
					ppt[n].y = ((i) ? ppt[n - 2].y : cy) + 1;
					n++;

					ppt[n].x = ppt[n - 1].x + 1;
					ppt[n].y = ppt[n - 1].y;
					n++;
				}
				else
				{
					ppt[n].x = ((i) ? ppt[n - 1].x : cx) - 1;
					ppt[n].y = ((i) ? ppt[n - 1].y : cy) + 1;
					n++;
				}
			}
			else
			{
				ppt[n].x = ((i) ? ppt[n - 1].x : cx) - 1;
				ppt[n].y = ((i) ? ppt[n - 1].y : cy);
				n++;
			}
		}

		for (i = 0; i < mi; i++)
		{
			if (nx > 0)
			{
				if (inc)
				{
					ppt[n].x = ((i) ? ppt[n - 2].x : cx) + 1;
					ppt[n].y = ((i) ? ppt[n - 2].y : cy) + 1;
					n++;

					ppt[n].x = ppt[n - 1].x - 1;
					ppt[n].y = ppt[n - 1].y;
					n++;
				}
				else
				{
					ppt[n].x = ((i) ? ppt[n - 1].x : cx) + 1;
					ppt[n].y = ((i) ? ppt[n - 1].y : cy) + 1;
					n++;
				}
			}
			else if (nx < 0)
			{
				if (inc)
				{
					ppt[n].x = ((i) ? ppt[n - 2].x : cx) + 1;
					ppt[n].y = ((i) ? ppt[n - 2].y : cy) - 1;
					n++;

					ppt[n].x = ppt[n - 1].x;
					ppt[n].y = ppt[n - 1].y + 1;
					n++;
				}
				else
				{
					ppt[n].x = ((i) ? ppt[n - 1].x : cx) + 1;
					ppt[n].y = ((i) ? ppt[n - 1].y : cy) - 1;
					n++;
				}
			}
			else
			{
				ppt[n].x = ((i) ? ppt[n - 1].x : cx) + 1;
				ppt[n].y = ((i) ? ppt[n - 1].y : cy);
				n++;
			}
		}
	}

	return (n-1);
}

int fit_line(int ps, int dashdot, const xpoint_t* ppt1, const xpoint_t* ppt2, xpoint_t* ppt, int max)
{
	int x, y, m, r, dx, dy;
	int p = 0;
	int nx, ny;
	int c, n = 0;

	int mo, mx, my;

	mo = (ps - 1) / 2 + (ps - 1) % 2;//outside of baseline  dots

	dx = (ppt1->x < ppt2->x) ? (ppt2->x - ppt1->x) : (ppt1->x - ppt2->x);
	dy = (ppt1->y < ppt2->y) ? (ppt2->y - ppt1->y) : (ppt1->y - ppt2->y);

	if (dy <= dx)
	{
		if (ppt1->x < ppt2->x)
		{
			x = ppt1->x;
			m = ppt2->x;
			y = ppt1->y;
			ny = ppt1->y - ppt2->y;
			r = ppt1->y;
		}
		else
		{
			x = ppt2->x;
			m = ppt1->x;
			y = ppt2->y;
			ny = ppt2->y - ppt1->y;
			r = ppt2->y;
		}
		nx = ppt1->x - ppt2->x;

		//adjust line start and end dot
		mx = mo;
		my = 0;
		while (mx--)
		{
			if (((p + dy) << 1) < dx)
			{
				p += dy;
			}
			else
			{
				p += (dy - dx);
				my++;
			}
		}
		x -= mo;
		m += mo;
		dx += mo << 1;
		y -= my;
		dy += my << 1;

		c = (dashdot)? (dashdot * ps) : MAX_LONG;
		p = 0;
		while (x++ < m)
		{
			if (c>0 || x == m)
			{
				ppt[n].x = x - 1;
				ppt[n].y = (ny>0) ? (y - ((y - r) << 1)) : y;
				n++;
			}

			if (((p + dy) << 1) < dx)
			{
				p += dy;

				if (c>0 || x == m)
				{
					n += fill_neghbour(1, 0, nx, ny, ps, (ppt + n - 1));
				}
			}
			else
			{
				p += (dy - dx);
				y++;

				if (c>0 || x == m)
				{
					n += fill_neghbour(1, 1, nx, ny, ps, (ppt + n - 1));
				}
			}

			if (c + ps - 1 > 0)
				c --;
			else
				c = (dashdot) ? (dashdot * ps) : MAX_LONG;
		}
	}
	else
	{
		if (ppt1->y < ppt2->y)
		{
			y = ppt1->y;
			m = ppt2->y;
			x = ppt1->x;
			nx = (ppt1->x - ppt2->x);
			r = ppt1->x;
		}
		else
		{
			y = ppt2->y;
			m = ppt1->y;
			x = ppt2->x;
			nx= (ppt2->x - ppt1->x);
			r = ppt2->x;
		}
		ny = (ppt1->y - ppt2->y);

		//adjust line start and end dot
		my = mo;
		mx = 0;
		while (my--)
		{
			if (((p + dx) << 1) < dy)
			{
				p += dx;
			}
			else
			{
				p += (dx - dy);
				mx++;
			}
		}
		y -= mo;
		m += mo;
		dy += mo << 1;
		x -= mx;
		dx += mx << 1;

		c = (dashdot) ? (dashdot * ps) : MAX_LONG;
		p = 0;
		while (y++ < m)
		{
			if (c > 0 || y == m)
			{
				ppt[n].y = y - 1;
				ppt[n].x = (nx>0) ? (x - ((x - r) << 1)) : x;
				n++;
			}

			if (((p + dx) << 1) < dy)
			{
				p += dx;

				if (c > 0 || y == m)
				{
					n += fill_neghbour(0, 0, nx, ny, ps, (ppt + n - 1));
				}
			}
			else
			{
				p += (dx - dy);
				x++;

				if (c > 0 || y == m)
				{
					n += fill_neghbour(0, 1, nx, ny, ps, (ppt + n - 1));
				}
			}

			if (c + ps - 1 > 0)
				c--;
			else
				c = (dashdot) ? (dashdot * ps) : MAX_LONG;
		}
	}

	return n;
}