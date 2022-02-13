﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc horz control document

	@module	horzbox.c | implement file

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

#include "xdcbox.h"
#include "xdcimp.h"

typedef struct _horzbox_delta_t{
	res_win_t target;
}horzbox_delta_t;

#define GETHORZBOXDELTA(ph) 	(horzbox_delta_t*)widget_get_user_delta(ph)
#define SETHORZBOXDELTA(ph,ptd) widget_set_user_delta(ph,(vword_t)ptd)


/*********************************************************************************/
int hand_horzbox_create(res_win_t widget, void* data)
{
	horzbox_delta_t* ptd;

	widget_hand_create(widget);

	ptd = (horzbox_delta_t*)xmem_alloc(sizeof(horzbox_delta_t));
	xmem_zero((void*)ptd, sizeof(horzbox_delta_t));

	SETHORZBOXDELTA(widget, ptd);

	return 0;
}

void hand_horzbox_destroy(res_win_t widget)
{
	horzbox_delta_t* ptd = GETHORZBOXDELTA(widget);

	XDK_ASSERT(ptd != NULL);

	xmem_free(ptd);

	SETHORZBOXDELTA(widget, 0);

	widget_hand_destroy(widget);
}

void hand_horzbox_lbutton_down(res_win_t widget, const xpoint_t* pxp)
{
	horzbox_delta_t* ptd = GETHORZBOXDELTA(widget);
	
	widget_kill_timer(widget, 0);
}

void hand_horzbox_lbutton_up(res_win_t widget, const xpoint_t* pxp)
{
	horzbox_delta_t* ptd = GETHORZBOXDELTA(widget);
	measure_interface im = { 0 };
	xfont_t xf = { 0 };
	xpoint_t pt;
	xrect_t xr;
	int hint;
	scroll_t sc;

	if (!widget_is_valid(ptd->target))
		return;

	pt.x = pxp->x;
	pt.y = pxp->y;

	widget_point_to_tm(widget, &pt);

	widget_get_xfont(widget, &xf);

	get_canvas_measure(widget_get_canvas(widget), &im);

	hint = calc_horzbox_hint(&im, &xf, &pt);

	widget_get_scroll_info(ptd->target, 1, &sc);
	widget_get_client_rect(ptd->target, &xr);

	if (hint == HORZBOX_HINT_PAGELEFT)
	{
		widget_scroll(ptd->target, 1, -sc.max);
	}
	else if (hint == HORZBOX_HINT_LINELEFT)
	{
		widget_scroll(ptd->target, 1, -xr.w);
	}
	else if (hint == HORZBOX_HINT_LINERIGHT)
	{
		widget_scroll(ptd->target, 1, xr.w);
	}
	else if (hint == HORZBOX_HINT_PAGERIGHT)
	{
		widget_scroll(ptd->target, 1, sc.max);
	}

	widget_set_timer(widget, DEF_TIPTIME);
}

void hand_horzbox_size(res_win_t widget, int code, const xsize_t* prs)
{
	horzbox_delta_t* ptd = GETHORZBOXDELTA(widget);
	
	widget_erase(widget, NULL);
}

void hand_horzbox_timer(res_win_t widget, vword_t tid)
{
	horzbox_delta_t* ptd = GETHORZBOXDELTA(widget);

	xpoint_t pt;
	xrect_t xr;

	message_position(&pt);

	widget_get_window_rect(widget, &xr);

	if (!pt_in_rect(&pt, &xr))
	{
		widget_kill_timer(widget, tid);

		widget_close(widget, 0);
	}
}

void hand_horzbox_paint(res_win_t widget, visual_t dc, const xrect_t* pxr)
{
	horzbox_delta_t* ptd = GETHORZBOXDELTA(widget);

	xrect_t xr;
	xfont_t xf;
	xbrush_t xb;
	xpen_t xp;

	visual_t rdc;
	canvas_t canv;
	const drawing_interface* pif = NULL;
	drawing_interface ifv = {0};

	widget_get_xfont(widget, &xf);
	widget_get_xbrush(widget, &xb);
	widget_get_xpen(widget, &xp);

	canv = widget_get_canvas(widget);
	pif = widget_get_canvas_interface(widget);
	

	
	
	
	
	

	widget_get_client_rect(widget, &xr);

	rdc = begin_canvas_paint(canv, dc, xr.w, xr.h);
	get_visual_interface(rdc, &ifv);

	lighten_xbrush(&xb, DEF_SOFT_DARKEN);

	(*ifv.pf_draw_rect)(ifv.ctx, NULL, &xb, &xr);

	draw_horzbox(pif, &xf);

	

	end_canvas_paint(canv, dc, pxr);
	
}

/***************************************************************************************/
res_win_t horzbox_create(res_win_t widget, dword_t style, const xrect_t* pxr)
{
	if_event_t ev = { 0 };

	EVENT_BEGIN_DISPATH(&ev)

		EVENT_ON_CREATE(hand_horzbox_create)
		EVENT_ON_DESTROY(hand_horzbox_destroy)

		EVENT_ON_PAINT(hand_horzbox_paint)

		EVENT_ON_SIZE(hand_horzbox_size)

		EVENT_ON_LBUTTON_DOWN(hand_horzbox_lbutton_down)
		EVENT_ON_LBUTTON_UP(hand_horzbox_lbutton_up)

		EVENT_ON_TIMER(hand_horzbox_timer)

		EVENT_ON_NC_IMPLEMENT

	EVENT_END_DISPATH

	return widget_create(NULL, style, pxr, widget, &ev);
}

void horzbox_popup_size(res_win_t widget, xsize_t* pxs)
{
	horzbox_delta_t* ptd = GETHORZBOXDELTA(widget);
	measure_interface im = { 0 };
	xfont_t xf = { 0 };

	XDK_ASSERT(ptd != NULL);

	widget_get_xfont(widget, &xf);

	get_canvas_measure(widget_get_canvas(widget), &im);

	calc_horzbox_size(&im, &xf, pxs);

	widget_size_to_pt(widget, pxs);

	widget_adjust_size(widget_get_style(widget), pxs);
}

void horzbox_set_target(res_win_t widget, res_win_t target)
{
	horzbox_delta_t* ptd = GETHORZBOXDELTA(widget);

	XDK_ASSERT(ptd != NULL);

	ptd->target = target;
}

res_win_t show_horzbox(res_win_t owner)
{
	res_win_t wt;
	xrect_t xr = { 0 };
	xsize_t xs = { 0 };
	clr_mod_t clr = { 0 };

	wt = horzbox_create(owner, WD_STYLE_POPUP | WD_STYLE_NOACTIVE, &xr);

	XDK_ASSERT(wt != NULL);

	widget_get_color_mode(owner, &clr);

	widget_set_user_id(wt, IDC_HORZBOX);
	widget_set_color_mode(wt, &clr);

	horzbox_set_target(wt, owner);

	horzbox_popup_size(wt, &xs);
	widget_get_client_rect(owner, &xr);

	xr.x = xr.x + xr.w / 2 - xs.w / 2;
	xr.y = xr.y + xr.h - xs.h;
	xr.w = xs.w;
	xr.h = xs.h;

	widget_client_to_screen(owner, RECTPOINT(&xr));

	widget_move(wt, RECTPOINT(&xr));
	widget_size(wt, RECTSIZE(&xr));
	widget_set_alpha(wt, 250);

	widget_set_timer(wt, DEF_TIPTIME);

	widget_show(wt, WS_SHOW_NORMAL);

	return wt;
}