/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc gdi document

	@module	if_gdi.c | linux implement file

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

#include "../xduloc.h"
#include "../xduutil.h"

#ifdef XDU_SUPPORT_CONTEXT_GDI

static void DPtoLP(visual_t rdc, XPoint* pt,int n)
{
	int i;
	for(i = 0;i<n;i++)
	{
		pt[i].x = pt[i].x;
		pt[i].y = pt[i].y;
	}
}

static void _adjust_rect(XRectangle* prt, int src_width, int src_height, const tchar_t* horz_align, const tchar_t* vert_align)
{
	if (xscmp(horz_align, GDI_ATTR_TEXT_ALIGN_NEAR) == 0 && xscmp(vert_align, GDI_ATTR_TEXT_ALIGN_NEAR) == 0)
	{
		prt->width = (prt->width < src_width) ? prt->width : src_width;
		prt->height = (prt->height < src_height) ? prt->height : src_height;
	}
	else if (xscmp(horz_align, GDI_ATTR_TEXT_ALIGN_FAR) == 0 && xscmp(vert_align,GDI_ATTR_TEXT_ALIGN_FAR) == 0)
	{
		prt->x = (prt->width < src_width) ? prt->x : (prt->x + prt->width - src_width);
		prt->height = (prt->height < src_height) ? prt->height : src_height;
	}
	else if (xscmp(horz_align,GDI_ATTR_TEXT_ALIGN_NEAR) == 0 && xscmp(vert_align,GDI_ATTR_TEXT_ALIGN_FAR) == 0)
	{
		prt->width = (prt->width < src_width) ? prt->width : src_width;
		prt->y = (prt->height < src_height) ? prt->y : (prt->y + prt->height - src_height);
	}
	else if (xscmp(horz_align,GDI_ATTR_TEXT_ALIGN_FAR) == 0 && xscmp(vert_align,GDI_ATTR_TEXT_ALIGN_FAR) == 0)
	{
		prt->x = (prt->width < src_width) ? prt->x : (prt->x + prt->width - src_width);
		prt->y = (prt->height < src_height) ? prt->y : (prt->y + prt->height - src_height);
	}
	else if (xscmp(horz_align,GDI_ATTR_TEXT_ALIGN_CENTER) == 0 && xscmp(vert_align,GDI_ATTR_TEXT_ALIGN_CENTER) == 0)
	{
		if (prt->width > src_width)
		{
			prt->x = prt->x + (prt->width - src_width) / 2;
			prt->width = src_width;
		}
		if (prt->height > src_height)
		{
			prt->y = prt->y + (prt->height - src_height) / 2;
			prt->height = src_height;
		}
	}
	else if (xscmp(horz_align,GDI_ATTR_TEXT_ALIGN_NEAR) == 0 && xscmp(vert_align,GDI_ATTR_TEXT_ALIGN_CENTER) == 0)
	{
		prt->width = (prt->width < src_width) ? prt->width : src_width;
		prt->y = (prt->height < src_height) ? prt->y : (prt->y + (prt->height - src_height) / 2);
		prt->height = (prt->height < src_height) ? prt->height : src_height;
	}
	else if (xscmp(horz_align,GDI_ATTR_TEXT_ALIGN_FAR) == 0 && xscmp(vert_align,GDI_ATTR_TEXT_ALIGN_CENTER) == 0)
	{
		prt->x = (prt->width < src_width) ? prt->x : (prt->x + prt->width - src_width);
		prt->y = (prt->height < src_height) ? prt->y : (prt->y + (prt->height - src_height) / 2);
		prt->height = (prt->height < src_height) ? prt->height : src_height;
	}
}

static void _calc_point(const xpoint_t* pt, int r, double a, xpoint_t* pp)
{
	pp->x = pt->x + (int)((float)r * cos(a));
	pp->y = pt->y + (int)((float)r * sin(a));
}


static XFontStruct* _create_font(const xfont_t* pxf)
{
	XFontStruct* fs;
	char font_token[1024] = {0};
	
	format_font_pattern(pxf, font_token);

	fs = XLoadQueryFont(g_display, font_token);

	return fs;
}

/************************************************************************************************/

void _gdi_init(int osv)
{

}

void _gdi_uninit(void)
{
	
}

void _gdi_draw_line(visual_t rdc,const xpen_t* pxp, const xpoint_t* ppt1, const xpoint_t* ppt2)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

	xcolor_t xc = {0};
	int l_w, l_s;
	unsigned long l_for;
	XColor ext, clr_pen = {0};
    
	XPoint pt[2];
	pt[0].x = ppt1->x;
	pt[0].y = ppt1->y;
	pt[1].x = ppt2->x;
	pt[1].y = ppt2->y;

	DPtoLP(rdc,pt,2);

	if(pxp)
	{
		parse_xcolor(&xc,pxp->color);

		clr_pen.red = XRGB(xc.r);
		clr_pen.green = XRGB(xc.g);
		clr_pen.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_pen);

		l_for = clr_pen.pixel;

		if (xscmp(pxp->style, GDI_ATTR_STROKE_STYLE_DOTTED) == 0)
			l_s = LineOnOffDash;
		else if (xscmp(pxp->style,GDI_ATTR_STROKE_STYLE_DASHED) == 0)
			l_s = LineDoubleDash;
		else
			l_s = LineSolid;
		
		l_w = xstol(pxp->size);
	}else
	{
		l_for = BlackPixel(g_display, DefaultScreen(g_display));
		l_w = 1;
		l_s = LineSolid;
	}

	XSetLineAttributes(g_display, ctx->context, l_w, l_s, CapRound, JoinRound);

    XSetForeground(g_display, ctx->context, l_for);

    XDrawLine(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, pt[1].x, pt[1].y);

	if(clr_pen.pixel)
	{
		XFreeColors(g_display, ctx->color, &(clr_pen.pixel), 1, 0);
	}
}

void _gdi_draw_polyline(visual_t rdc,const xpen_t* pxp,const xpoint_t* ppt,int n)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	xcolor_t xc = {0};
	int l_w, l_s;
	unsigned long l_p;
	XColor ext, clr_pen = {0};
    
	XPoint* pa;
	int i;
	
	if(!n) return;

	pa = (XPoint*)calloc(n, sizeof(XPoint));
	for(i =0;i<n;i++)
	{
		pa[i].x = ppt[i].x;
		pa[i].y = ppt[i].y;
	}
	DPtoLP(rdc,pa,n);

	if(pxp)
	{
		parse_xcolor(&xc,pxp->color);

		clr_pen.red = XRGB(xc.r);
		clr_pen.green = XRGB(xc.g);
		clr_pen.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_pen);

		l_p = clr_pen.pixel;

		if (xscmp(pxp->style, GDI_ATTR_STROKE_STYLE_DOTTED) == 0)
			l_s = LineOnOffDash;
		else if (xscmp(pxp->style,GDI_ATTR_STROKE_STYLE_DASHED) == 0)
			l_s = LineDoubleDash;
		else
			l_s = LineSolid;
		
		l_w = xstol(pxp->size);
	}else
	{
		l_p = BlackPixel(g_display, DefaultScreen(g_display));
		l_w = 1;
		l_s = LineSolid;
	}

	XSetLineAttributes(g_display, ctx->context, l_w, l_s, CapRound, JoinRound);

    XSetForeground(g_display, ctx->context, l_p);

    XDrawLines(g_display, ctx->device, ctx->context, pa, n, CoordModePrevious);

	free(pa);

	if(clr_pen.pixel)
	{
		XFreeColors(g_display, ctx->color, &(clr_pen.pixel), 1, 0);
	}
}

void _gdi_draw_arc(visual_t rdc, const xpen_t* pxp, const xpoint_t * ppt1, const xpoint_t* ppt2, const xsize_t* pxs, bool_t sflag, bool_t lflag)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

	xcolor_t xc = {0};
	int l_w, l_s;
	XColor ext, clr_brush = {0}, clr_pen = {0};
	unsigned long l_p;

	XPoint pt[4] = {0};

	int fdeg, tdeg;
	int x, y, w, h;

	xpoint_t xp[3];
	double arcf, arct;
	int rx, ry;
    
	pt[0].x = ppt1->x;
	pt[0].y = ppt1->y;
	pt[1].x = ppt2->x;
	pt[1].y = ppt2->y;
	pt[2].x = pxs->w;
	pt[2].y = pxs->h;

	DPtoLP(rdc,pt,3);

	xp[0].x = pt[0].x;
	xp[0].y = pt[0].y;
	xp[1].x = pt[1].x;
	xp[1].y = pt[1].y;
	rx = pt[2].x;
	ry = pt[2].y;

	pt_calc_radian(sflag, lflag, rx, ry, &xp[0], &xp[1], &xp[2], &arcf, &arct);

	x = xp[2].x - rx;
	y = xp[2].y - ry;
	w = rx * 2;
	h = ry * 2;

	fdeg = (int)(arcf / XPI * 180 * 64);
	tdeg = (int)(arct / XPI * 180 * 64);

	if(pxp)
	{
		parse_xcolor(&xc,pxp->color);

		clr_pen.red = XRGB(xc.r);
		clr_pen.green = XRGB(xc.g);
		clr_pen.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_pen);

		if (xscmp(pxp->style, GDI_ATTR_STROKE_STYLE_DOTTED) == 0)
			l_s = LineOnOffDash;
		else if (xscmp(pxp->style,GDI_ATTR_STROKE_STYLE_DASHED) == 0)
			l_s = LineDoubleDash;
		else
			l_s = LineSolid;
		
		l_w = xstol(pxp->size);
		l_p = clr_pen.pixel;
	}else
	{
		l_s = LineSolid;
		l_w = 1;
		l_p = BlackPixel(g_display, DefaultScreen(g_display));
	}
	
	XSetLineAttributes(g_display, ctx->context, l_w, l_s, CapNotLast, JoinMiter);

	XSetForeground(g_display, ctx->context, l_p);
	
	XDrawArc(g_display, ctx->device, ctx->context, x, y, w, h, fdeg, tdeg);

	if(clr_pen.pixel)
	{
		XFreeColors(g_display, ctx->color, &(clr_pen.pixel), 1, 0);
	}
}

void _gdi_draw_bezier(visual_t rdc, const xpen_t* pxp, const xpoint_t* ppt1, const xpoint_t* ppt2, const xpoint_t* ppt3, const xpoint_t* ppt4)
{
    
}

void _gdi_draw_curve(visual_t rdc, const xpen_t* pxp, const xpoint_t* ppt, int n)
{
    
}

void _gdi_draw_path(visual_t rdc, const xpen_t* pxp, const xbrush_t* pxb, const tchar_t* aa, const xpoint_t* pa)
{

}

void _gdi_gradient_rect(visual_t rdc, const xcolor_t* clr_brim, const xcolor_t* clr_core, const tchar_t* gradient, const xrect_t* prt)
{
	xpen_t xp;
	xbrush_t xb;

	default_xpen(&xp);
	default_xpen(&xb);
	format_xcolor(clr_brim, xp.color);
	format_xcolor(clr_core, xb.color);
	
	_gdi_draw_rect(rdc, &xp, &xb, prt);
}

void _gdi_alphablend_rect(visual_t rdc, const xcolor_t* pxc, const xrect_t* prt, int opacity)
{
	
}

void _gdi_draw_rect(visual_t rdc,const xpen_t* pxp,const xbrush_t* pxb,const xrect_t* prt)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	xcolor_t xc = {0};
	int l_w, l_s;
	XColor ext, clr_brush = {0}, clr_pen = {0};

	XPoint pt[2];
    
	pt[0].x = prt->x;
	pt[0].y = prt->y;
	pt[1].x = prt->x + prt->w;
	pt[1].y = prt->y + prt->h;

	DPtoLP(rdc,pt,2);

	if(pxb)
	{
		parse_xcolor(&xc,pxb->color);

		clr_brush.red = XRGB(xc.r);
		clr_brush.green = XRGB(xc.g);
		clr_brush.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_brush);

		XSetForeground(g_display, ctx->context, clr_brush.pixel);

		XSetFillRule(g_display, ctx->context, EvenOddRule);
		XSetFillStyle(g_display, ctx->context, FillOpaqueStippled);
    
		XFillRectangle(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, pt[1].x - pt[0].x, pt[1].y - pt[0].y);

		if(clr_brush.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_brush.pixel), 1, 0);
		}
	}

	if(pxp)
	{
		parse_xcolor(&xc,pxp->color);

		clr_pen.red = XRGB(xc.r);
		clr_pen.green = XRGB(xc.g);
		clr_pen.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_pen);

		if (xscmp(pxp->style, GDI_ATTR_STROKE_STYLE_DOTTED) == 0)
			l_s = LineOnOffDash;
		else if (xscmp(pxp->style,GDI_ATTR_STROKE_STYLE_DASHED) == 0)
			l_s = LineDoubleDash;
		else
			l_s = LineSolid;
		
		l_w = xstol(pxp->size);

		XSetLineAttributes(g_display, ctx->context, l_w, l_s, CapNotLast, JoinMiter);

		XSetForeground(g_display, ctx->context, clr_pen.pixel);
		
		XDrawRectangle(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, pt[1].x - pt[0].x, pt[1].y - pt[0].y);

		if(clr_pen.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_pen.pixel), 1, 0);
		}
	}
}

void _gdi_draw_round(visual_t rdc,const xpen_t* pxp,const xbrush_t* pxb,const xrect_t* prt)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	xcolor_t xc = {0};
	int l_w, l_s;
	XColor ext, clr_brush = {0}, clr_pen = {0};

	XPoint pa[9];
	int i;

	int r;

	r = (prt->w) / 10;
	if (r < 1)
		r = 1;
	else if (r > 6)
		r = 6;

	pa[0].x = prt->x;
	pa[0].y = prt->y + r;
	pa[1].x = prt->x + r;
	pa[1].y = prt->y;
	pa[2].x = prt->x + prt->w - r;
	pa[2].y = prt->y;
	pa[3].x = prt->x + prt->w;
	pa[3].y = prt->y + r;
	pa[4].x = prt->x;
	pa[4].y = prt->y + prt->h - r;
	pa[5].x = prt->x + prt->w - r;
	pa[5].y = prt->y + prt->h;
	pa[6].x = prt->x + r;
	pa[6].y = prt->y + prt->h;
	pa[7].x = prt->x;
	pa[7].y = prt->y + prt->h - r;
	pa[8].x = prt->x;
	pa[8].y = prt->y + r;

	DPtoLP(rdc,pa,9);

	if(pxb)
	{
		parse_xcolor(&xc,pxb->color);

		clr_brush.red = XRGB(xc.r);
		clr_brush.green = XRGB(xc.g);
		clr_brush.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_brush);

		XSetForeground(g_display, ctx->context, clr_brush.pixel);

		XSetFillRule(g_display, ctx->context, EvenOddRule);
		XSetFillStyle(g_display, ctx->context, FillOpaqueStippled);
    
		XFillPolygon(g_display, ctx->device, ctx->context, pa, 9, Nonconvex, CoordModePrevious);

		if(clr_brush.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_brush.pixel), 1, 0);
		}
	}

	if(pxp)
	{
		parse_xcolor(&xc,pxp->color);

		clr_pen.red = XRGB(xc.r);
		clr_pen.green = XRGB(xc.g);
		clr_pen.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_pen);

		if (xscmp(pxp->style, GDI_ATTR_STROKE_STYLE_DOTTED) == 0)
			l_s = LineOnOffDash;
		else if (xscmp(pxp->style,GDI_ATTR_STROKE_STYLE_DASHED) == 0)
			l_s = LineDoubleDash;
		else
			l_s = LineSolid;
		
		l_w = xstol(pxp->size);

		XSetLineAttributes(g_display, ctx->context, l_w, l_s, CapNotLast, JoinMiter);

		XSetForeground(g_display, ctx->context, clr_pen.pixel);
		
		XDrawLines(g_display, ctx->device, ctx->context, pa, 9, CoordModePrevious);

		if(clr_pen.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_pen.pixel), 1, 0);
		}
	}
}

void _gdi_draw_ellipse(visual_t rdc,const xpen_t* pxp,const xbrush_t* pxb,const xrect_t* prt)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	xcolor_t xc = {0};
	int l_w, l_s;
	XColor ext, clr_brush = {0}, clr_pen = {0};

	XPoint pt[2];
    
	pt[0].x = prt->x;
	pt[0].y = prt->y;
	pt[1].x = prt->x + prt->w;
	pt[1].y = prt->y + prt->h;

	DPtoLP(rdc,pt,2);

	if(pxb)
	{
		parse_xcolor(&xc,pxb->color);

		clr_brush.red = XRGB(xc.r);
		clr_brush.green = XRGB(xc.g);
		clr_brush.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_brush);

		XSetForeground(g_display, ctx->context, clr_brush.pixel);

		XSetFillRule(g_display, ctx->context, EvenOddRule);
		XSetFillStyle(g_display, ctx->context, FillOpaqueStippled);
    
		XFillArc(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, pt[1].x - pt[0].x, pt[1].y - pt[0].y, 0, 360 * 64);

		if(clr_brush.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_brush.pixel), 1, 0);
		}
	}

	if(pxp)
	{
		parse_xcolor(&xc,pxp->color);

		clr_pen.red = XRGB(xc.r);
		clr_pen.green = XRGB(xc.g);
		clr_pen.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_pen);

		if (xscmp(pxp->style, GDI_ATTR_STROKE_STYLE_DOTTED) == 0)
			l_s = LineOnOffDash;
		else if (xscmp(pxp->style,GDI_ATTR_STROKE_STYLE_DASHED) == 0)
			l_s = LineDoubleDash;
		else
			l_s = LineSolid;
		
		l_w = xstol(pxp->size);

		XSetLineAttributes(g_display, ctx->context, l_w, l_s, CapNotLast, JoinMiter);

		XSetForeground(g_display, ctx->context, clr_pen.pixel);
		
		XDrawArc(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, pt[1].x - pt[0].x, pt[1].y - pt[0].y, 0, 360 * 64);

		if(clr_pen.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_pen.pixel), 1, 0);
		}
	}
}

void _gdi_draw_pie(visual_t rdc, const xpen_t* pxp, const xbrush_t*pxb, const xrect_t* prt,  double fang, double sang)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	xcolor_t xc = {0};
	int l_w, l_s;
	XColor ext, clr_brush = {0}, clr_pen = {0};

	XPoint pt[2];

	int fdeg, tdeg;
    
	pt[0].x = prt->x;
	pt[0].y = prt->y;
	pt[1].x = prt->x + prt->w;
	pt[1].y = prt->y + prt->h;

	DPtoLP(rdc,pt,2);

	fdeg = (int)(fang / XPI * 180 * 64);
	tdeg = (int)(sang / XPI * 180 * 64);

	if(pxb)
	{
		parse_xcolor(&xc,pxb->color);

		clr_brush.red = XRGB(xc.r);
		clr_brush.green = XRGB(xc.g);
		clr_brush.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_brush);

		XSetForeground(g_display, ctx->context, clr_brush.pixel);

		XSetFillRule(g_display, ctx->context, EvenOddRule);
		XSetFillStyle(g_display, ctx->context, FillOpaqueStippled);
    
		XFillArc(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, pt[1].x - pt[0].x, pt[1].y - pt[0].y, fdeg, tdeg);

		if(clr_brush.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_brush.pixel), 1, 0);
		}
	}

	if(pxp)
	{
		parse_xcolor(&xc,pxp->color);

		clr_pen.red = XRGB(xc.r);
		clr_pen.green = XRGB(xc.g);
		clr_pen.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_pen);

		if (xscmp(pxp->style, GDI_ATTR_STROKE_STYLE_DOTTED) == 0)
			l_s = LineOnOffDash;
		else if (xscmp(pxp->style,GDI_ATTR_STROKE_STYLE_DASHED) == 0)
			l_s = LineDoubleDash;
		else
			l_s = LineSolid;
		
		l_w = xstol(pxp->size);

		XSetLineAttributes(g_display, ctx->context, l_w, l_s, CapNotLast, JoinMiter);

		XSetForeground(g_display, ctx->context, clr_pen.pixel);
		
		XDrawArc(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, pt[1].x - pt[0].x, pt[1].y - pt[0].y, fdeg, tdeg);

		if(clr_pen.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_pen.pixel), 1, 0);
		}
	}
}

void _gdi_draw_polygon(visual_t rdc, const xpen_t* pxp, const xbrush_t*pxb, const xpoint_t* ppt, int n)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

	xcolor_t xc = {0};
	int l_w, l_s;
	XColor ext, clr_brush = {0}, clr_pen = {0};

	XPoint* pa;
	int i;

	pa = (XPoint*)calloc(n + 1, sizeof(XPoint));
	for(i =0;i<n;i++)
	{
		pa[i].x = ppt[i].x;
		pa[i].y = ppt[i].y;
	}
	pa[n].x = ppt[0].x;
	pa[n].y = ppt[0].y;

	DPtoLP(rdc,pa,n + 1);

	if(pxb)
	{
		parse_xcolor(&xc,pxb->color);

		clr_brush.red = XRGB(xc.r);
		clr_brush.green = XRGB(xc.g);
		clr_brush.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_brush);

		XSetForeground(g_display, ctx->context, clr_brush.pixel);

		XSetFillRule(g_display, ctx->context, EvenOddRule);
		XSetFillStyle(g_display, ctx->context, FillOpaqueStippled);
    
		XFillPolygon(g_display, ctx->device, ctx->context, pa, n + 1, Nonconvex, CoordModePrevious);

		if(clr_brush.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_brush.pixel), 1, 0);
		}
	}

	if(pxp)
	{
		parse_xcolor(&xc,pxp->color);

		clr_pen.red = XRGB(xc.r);
		clr_pen.green = XRGB(xc.g);
		clr_pen.blue = XRGB(xc.b);

		XAllocColor(g_display, ctx->color, &clr_pen);

		if (xscmp(pxp->style, GDI_ATTR_STROKE_STYLE_DOTTED) == 0)
			l_s = LineOnOffDash;
		else if (xscmp(pxp->style,GDI_ATTR_STROKE_STYLE_DASHED) == 0)
			l_s = LineDoubleDash;
		else
			l_s = LineSolid;
		
		l_w = xstol(pxp->size);

		XSetLineAttributes(g_display, ctx->context, l_w, l_s, CapNotLast, JoinMiter);

		XSetForeground(g_display, ctx->context, clr_pen.pixel);
		
		XDrawLines(g_display, ctx->device, ctx->context, pa, n + 1, CoordModePrevious);

		if(clr_pen.pixel)
		{
			XFreeColors(g_display, ctx->color, &(clr_pen.pixel), 1, 0);
		}
	}

	free(pa);
}

void _gdi_draw_sector(visual_t rdc, const xpen_t* pxp, const xbrush_t* pxb, const xpoint_t* ppt, const xspan_t* prl, const xspan_t* prs, double from, double sweep)
{

}

void _gdi_draw_text(visual_t rdc,const xfont_t* pxf,const xface_t* pxa,const xrect_t* prt,const tchar_t* txt,int len)
{
	X11_context_t* ctx = (X11_context_t*)rdc;
	XFontStruct* pfs = NULL;

	xcolor_t xc = {0};
	int l_w, l_s;
	XColor ext, clr_font = {0};

	XPoint pt[2];
    
	pt[0].x = prt->x;
	pt[0].y = prt->y;
	pt[1].x = prt->x + prt->w;
	pt[1].y = prt->y + prt->h;

	DPtoLP(rdc,pt,2);

	if(pxf)
	{
		pfs = _create_font(pxf);
	}

	if(pfs)
	{
		XSetFont(g_display, ctx->context, pfs->fid);
	}

	parse_xcolor(&xc,pxf->color);

	clr_font.red = XRGB(xc.r);
	clr_font.green = XRGB(xc.g);
	clr_font.blue = XRGB(xc.b);

	XAllocColor(g_display, ctx->color, &clr_font);

	XSetForeground(g_display, ctx->context, clr_font.pixel);

	XSetFillRule(g_display, ctx->context, EvenOddRule);
	XSetFillStyle(g_display, ctx->context, FillOpaqueStippled);
    
	if(len < 0) len = xslen(txt);

	XDrawString(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, txt, len);

	if(pfs)
	{
		XFreeFont(g_display, pfs);
	}

	if(clr_font.pixel)
	{
		XFreeColors(g_display, ctx->color, &(clr_font.pixel), 1, 0);
	}
}

void _gdi_text_out(visual_t rdc, const xfont_t* pxf, const xpoint_t* ppt, const tchar_t* txt, int len)
{
	X11_context_t* ctx = (X11_context_t*)rdc;
	XFontStruct* pfs = NULL;

	xcolor_t xc = {0};
	int l_w, l_s;
	XColor ext, clr_font = {0};

	XPoint pt[2];
    
	pt[0].x = ppt->x;
	pt[0].y = ppt->y;
	pt[1].x = ppt->x;
	pt[1].y = ppt->y;

	DPtoLP(rdc,pt,2);

	if(pxf)
	{
		pfs = _create_font(pxf);
	}

	if(pfs)
	{
		XSetFont(g_display, ctx->context, pfs->fid);
	}

	parse_xcolor(&xc,pxf->color);

	clr_font.red = XRGB(xc.r);
	clr_font.green = XRGB(xc.g);
	clr_font.blue = XRGB(xc.b);

	XAllocColor(g_display, ctx->color, &clr_font);

	XSetForeground(g_display, ctx->context, clr_font.pixel);

	XSetFillRule(g_display, ctx->context, EvenOddRule);
	XSetFillStyle(g_display, ctx->context, FillOpaqueStippled);
    
	if(len < 0) len = xslen(txt);

	XDrawString(g_display, ctx->device, ctx->context, pt[0].x, pt[0].y, txt, len);

	if(pfs)
	{
		XFreeFont(g_display, pfs);
	}

	if(clr_font.pixel)
	{
		XFreeColors(g_display, ctx->color, &(clr_font.pixel), 1, 0);
	}
}

void _gdi_exclip_rect(visual_t rdc, const xrect_t* pxr)
{

}

void _gdi_inclip_rect(visual_t rdc, const xrect_t* pxr)
{
	
}

void _gdi_text_rect(visual_t rdc, const xfont_t* pxf, const xface_t* pxa, const tchar_t* txt, int len, xrect_t* prt)
{
	X11_context_t* ctx = (X11_context_t*)rdc;
	XFontStruct* pfs = NULL;
	XCharStruct chs = {0};
	int direct = 0, ascent = 0, descent = 0;

	if(pxf)
	{
		pfs = _create_font(pxf);
	}

	if(!pfs) return;
	
	if(len < 0) len = xslen(txt);

	prt->w = XTextWidth(pfs, txt, len);

	XTextExtents(pfs, txt, len, &direct, &ascent, &descent, &chs);

	prt->h = ascent + descent;

	if(pfs)
	{
		XFreeFont(g_display, pfs);
	}
}

void _gdi_text_size(visual_t rdc, const xfont_t* pxf, const tchar_t* txt, int len, xsize_t* pxs)
{
	X11_context_t* ctx = (X11_context_t*)rdc;

	XFontStruct* pfs = NULL;
	XCharStruct chs = {0};
	int direct = 0, ascent = 0, descent = 0;

	if(pxf)
	{
		pfs = _create_font(pxf);
	}

	if(!pfs) return;
	
	if(len < 0) len = xslen(txt);

	XTextExtents(pfs, txt, len, &direct, &ascent, &descent, &chs);

	pxs->w = chs.width;
	pxs->h = ascent + descent;

	if(pfs)
	{
		XFreeFont(g_display, pfs);
	}
}

void _gdi_text_metric(visual_t rdc, const xfont_t* pxf, xsize_t* pxs)
{
	X11_context_t* ctx = (X11_context_t*)rdc;
	XFontStruct* pfs = NULL;
	XCharStruct chs = {0};
	int direct = 0, ascent = 0, descent = 0;

	if(pxf)
	{
		pfs = _create_font(pxf);
	}

	if(!pfs) return;
	
	XTextExtents(pfs, "aj", 2, &direct, &ascent, &descent, &chs);

	pxs->w = chs.width / 2;
	pxs->h = (ascent + descent) / 2;

	if(pfs)
	{
		XFreeFont(g_display, pfs);
	}
}

#ifdef XDU_SUPPORT_CONTEXT_BITMAP
void _gdi_draw_image(visual_t rdc,bitmap_t rbm,const tchar_t* clr,const xrect_t* prt)
{
    X11_context_t* ctx = (X11_context_t*)rdc;
	X11_bitmap_t* bmp = (X11_bitmap_t*)rbm;

	XImage* pmi = (XImage*)bmp->image;

	XRectangle xr;
	XPoint pt[2];

	xr.x = prt->x;
	xr.y = prt->y;
	xr.width = prt->w;
	xr.height = prt->h;

	_adjust_rect(&xr, pmi->width, pmi->height, GDI_ATTR_TEXT_ALIGN_CENTER, GDI_ATTR_TEXT_ALIGN_CENTER);
    
	pt[0].x = xr.x;
	pt[0].y = xr.y;
	pt[1].x = xr.x + xr.width;
	pt[1].y = xr.y + xr.height;

	DPtoLP(rdc,pt,2);

	XPutImage(g_display, ctx->device, ctx->context, pmi, 0, 0, pt[0].x, pt[0].y, pt[1].x - pt[0].x, pt[1].y - pt[0].y);
}

void _gdi_draw_bitmap(visual_t rdc, bitmap_t rbm, const xpoint_t* ppt)
{
	X11_context_t* ctx = (X11_context_t*)rdc;
	X11_bitmap_t* bmp = (X11_bitmap_t*)rbm;

	XImage* pmi = (XImage*)bmp->image;

	XPoint pt[1];
    
	pt[0].x = ppt->x;
	pt[0].y = ppt->y;

	XPutImage(g_display, ctx->device, ctx->context, pmi, 0, 0, pt[0].x, pt[0].y, pmi->width, pmi->height);
}
#endif

#ifdef XDU_SUPPORT_CONTEXT_REGION
void _gdi_fill_region(visual_t rdc, const xbrush_t* pxb, res_rgn_t rgn)
{
    
}
#endif

#endif //XDU_SUPPORT_CONTEXT_GRAPHIC

