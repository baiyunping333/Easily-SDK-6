/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc shape control document

	@module	shapebox.c | implement file

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

typedef struct _shapebox_delta_t{
	tchar_t shape[INT_LEN + 1];
}shapebox_delta_t;

#define GETSHAPEBOXDELTA(ph) 	(shapebox_delta_t*)widget_get_user_delta(ph)
#define SETSHAPEBOXDELTA(ph,ptd) widget_set_user_delta(ph,(vword_t)ptd)

/*********************************************************************************/
int hand_shapebox_create(res_win_t widget, void* data)
{
	shapebox_delta_t* ptd = GETSHAPEBOXDELTA(widget);

	widget_hand_create(widget);

	ptd = (shapebox_delta_t*)xmem_alloc(sizeof(shapebox_delta_t));
	xmem_zero((void*)ptd, sizeof(shapebox_delta_t));

	SETSHAPEBOXDELTA(widget, ptd);

	return 0;
}

void hand_shapebox_destroy(res_win_t widget)
{
	shapebox_delta_t* ptd = GETSHAPEBOXDELTA(widget);

	XDK_ASSERT(ptd != NULL);

	xmem_free(ptd);

	SETSHAPEBOXDELTA(widget, 0);

	widget_hand_destroy(widget);
}

void hand_shapebox_lbutton_down(res_win_t widget, const xpoint_t* pxp)
{
	shapebox_delta_t* ptd = GETSHAPEBOXDELTA(widget);
	
}

void hand_shapebox_lbutton_up(res_win_t widget, const xpoint_t* pxp)
{
	shapebox_delta_t* ptd = GETSHAPEBOXDELTA(widget);
	
}

void hand_shapebox_size(res_win_t widget, int code, const xsize_t* prs)
{
	shapebox_delta_t* ptd = GETSHAPEBOXDELTA(widget);
	
	widget_erase(widget, NULL);
}

void hand_shapebox_paint(res_win_t widget, visual_t dc, const xrect_t* pxr)
{
	shapebox_delta_t* ptd = GETSHAPEBOXDELTA(widget);
	visual_t rdc;

	xrect_t xr;
	xpen_t xp;
	xbrush_t xb;

	canvas_t canv;
	const drawing_interface* pif = NULL;
	drawing_interface ifv = {0};

	widget_get_xpen(widget, &xp);
	widget_get_xbrush(widget, &xb);

	widget_get_client_rect(widget, &xr);

	canv = widget_get_canvas(widget);
	pif = widget_get_canvas_interface(widget);
	

	rdc = begin_canvas_paint(canv, dc, xr.w, xr.h);

	get_visual_interface(rdc, &ifv);

	(*ifv.pf_draw_rect)(ifv.ctx, NULL, &xb, &xr);

	draw_shape(pif, &xp, &xb, (xrect_t*)&(pif->rect), ptd->shape);

	end_canvas_paint(canv, dc, pxr);
	
}

/***************************************************************************************/
res_win_t shapebox_create(res_win_t widget, dword_t style, const xrect_t* pxr)
{
	if_event_t ev = { 0 };

	EVENT_BEGIN_DISPATH(&ev)

		EVENT_ON_CREATE(hand_shapebox_create)
		EVENT_ON_DESTROY(hand_shapebox_destroy)

		EVENT_ON_PAINT(hand_shapebox_paint)

		EVENT_ON_SIZE(hand_shapebox_size)

		EVENT_ON_LBUTTON_DOWN(hand_shapebox_lbutton_down)
		EVENT_ON_LBUTTON_UP(hand_shapebox_lbutton_up)

		EVENT_ON_NC_IMPLEMENT

	EVENT_END_DISPATH

	return widget_create(NULL, style, pxr, widget, &ev);
}

void shapebox_set_shape(res_win_t widget, const tchar_t* shape)
{
	shapebox_delta_t* ptd = GETSHAPEBOXDELTA(widget);

	XDK_ASSERT(ptd != NULL);

	xsncpy(ptd->shape, shape, INT_LEN);

	widget_erase(widget, NULL);
}

int shapebox_get_shape(res_win_t widget, tchar_t* buf)
{
	shapebox_delta_t* ptd = GETSHAPEBOXDELTA(widget);
	int len;

	XDK_ASSERT(ptd != NULL);

	len = xslen(ptd->shape);

	if (buf)
		xscpy(buf, ptd->shape);

	return len;
}
