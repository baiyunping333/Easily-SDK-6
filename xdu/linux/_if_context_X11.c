/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc context document

	@module	if_context.c | linux implement file

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


#ifdef XDU_SUPPORT_CONTEXT
Display*     g_display = NULL;
XIM          g_xim = (XIM)0;

int _context_startup(void)
{
    int nVer = 0;
    char* dname;

    dname = getenv("DISPLAY");
    
    g_display = XOpenDisplay(dname);

    if(!g_display) return (-1);

	return nVer;
}

void _context_cleanup(void)
{
    if(g_display)
        XCloseDisplay(g_display);
    
    g_display = NULL;
}

visual_t _create_display_context(res_win_t wt)
{
    X11_context_t* ctx = NULL;
    XGCValues gv = {0};
    XWindowAttributes attr = {0};
    GC* gc = NULL;

    gc = DefaultGC(g_display, DefaultScreen(g_display));
    if(!gc) return NULL;

    XGetGCValues(g_display, gc, GCFunction | GCForeground | GCBackground | GCPlaneMask, &gv);
    gv.subwindow_mode = ClipByChildren;

    ctx = (X11_context_t*)calloc(1, sizeof(X11_context_t));
    
    ctx->type = CONTEXT_WIDGET;
    ctx->device = (wt)? wt : DefaultRootWindow(g_display);
    ctx->context = XCreateGC(g_display, ctx->device, 0, &gv);

    XGetWindowAttributes(g_display, ctx->device, &attr);

    ctx->width = attr.width;
    ctx->height = attr.height;
    ctx->color = attr.colormap;
    ctx->visual = attr.visual;
    ctx->depth = attr.depth;
    
    return &(ctx->head);
}

visual_t _create_compatible_context(visual_t rdc, int cx, int cy)
{
    X11_context_t* org = (X11_context_t*)rdc;
    XGCValues gv = {0};
    Window r;
    int x,y;
    unsigned int w,h,b,d;
    X11_context_t* ctx = NULL;
    
    XGetGeometry(g_display, org->device, &r, &x, &y, &w, &h, &b, &d);
    
    ctx = (X11_context_t*)calloc(1, sizeof(X11_context_t));

    ctx->type = CONTEXT_MEMORY;
    ctx->device = XCreatePixmap (g_display, r, cx, cy, org->depth);
    ctx->context = XCreateGC(g_display, org->device, 0, &gv);
    ctx->width = cx;
    ctx->height = cy;
    if(org->type == CONTEXT_MEMORY)
        ctx->color = XCreateColormap(g_display, DefaultRootWindow(g_display), org->visual, 0);
    else
        ctx->color = XCreateColormap(g_display, org->device, org->visual, 0);
    ctx->visual = org->visual;
    ctx->depth = org->depth;
    
    return &(ctx->head);
}

void _destroy_context(visual_t rdc)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

    if(ctx->type == CONTEXT_MEMORY && ctx->device)
        XFreePixmap(g_display, ctx->device);
    
    if(ctx->color)
        XFreeColormap(g_display, ctx->color);

    if(ctx->context)
	    XFreeGC(g_display, ctx->context);
    
    free(ctx);
}

void _get_device_caps(visual_t rdc, dev_cap_t* pcap)
{
    X11_context_t* ctx = (X11_context_t*)rdc;
    int scrn;
    
    scrn = DefaultScreen(g_display);

    pcap->horz_res = DisplayWidth(g_display, scrn);
    pcap->vert_res = DisplayHeight(g_display, scrn);
    
	pcap->horz_size = DisplayWidthMM(g_display, scrn);
	pcap->vert_size = DisplayHeightMM(g_display, scrn);

	pcap->horz_pixels = (int)((float)pcap->horz_res / (float)pcap->horz_size * MMPERINCH);
	pcap->vert_pixels = (int)((float)pcap->vert_res / (float)pcap->vert_size * MMPERINCH);

    pcap->horz_feed = 0;
    pcap->vert_feed = 0;
}

void _render_context(visual_t src, int srcx, int srcy, visual_t dst, int dstx, int dsty, int dstw, int dsth)
{
    X11_context_t* src_ctx = (X11_context_t*)src;
    X11_context_t* dst_ctx = (X11_context_t*)dst;

    XCopyArea(g_display, src_ctx->device, dst_ctx->device, src_ctx->context, srcx, srcy, dstw, dsth, dstx, dsty);
}

/*******************************************************************************************************************/

float _pt_per_mm(visual_t rdc, bool_t horz)
{
    X11_context_t* ctx = (X11_context_t*)rdc;
    int scrn;
    
    scrn = DefaultScreen(g_display);
    
    if(horz)
        return (float)((float)DisplayWidth(g_display, scrn) / (float)DisplayWidthMM(g_display, scrn));
    else
        return (float)((float)DisplayHeight(g_display, scrn) / (float)DisplayHeightMM(g_display, scrn));
}

static int _font_size(visual_t rdc, int height)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

    float fh;
    int size;
    
    fh = _pt_per_mm(rdc, 0);
    
    size = (int)(((float)height / fh) * PDPERMM);
    
    return size;
}

void _text_pt_size(visual_t rdc, const xfont_t* pxf, const tchar_t* txt, int len, xsize_t* pxs)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

    XFontStruct* pfs = NULL;
    tchar_t pattern[256] = {0};
    
    if(len < 0) len = xslen(txt);
    
    format_font_pattern(pxf, pattern);
    
    pfs = XLoadQueryFont(g_display, pattern);
    if(!pfs)
        return;
    
    pxs->w = XTextWidth(pfs, txt, len);
    pxs->h = pfs->ascent + pfs->descent;

    XFreeFont(g_display, pfs);
}

void _text_mm_size(visual_t rdc, const xfont_t* pxf, const tchar_t* txt, int len, xsize_t* pxs)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

    _text_pt_size(rdc, pxf, txt, len, pxs);
    
    pxs->fw = (float)pxs->w / _pt_per_mm(rdc, 1);
    pxs->fh = (float)pxs->h / _pt_per_mm(rdc, 0);
}

void _text_pt_metric(visual_t rdc, const xfont_t* pxf, xsize_t* pxs)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

    XFontStruct* pfs = NULL;
    tchar_t pattern[256] = {0};
    
    format_font_pattern(pxf, pattern);
    
    pfs = XLoadQueryFont(g_display, pattern);
    if(!pfs)
        return;
    
    pxs->w = (pfs->min_bounds.width + pfs->max_bounds.width) / 2;
    pxs->h = pfs->ascent + pfs->descent;
}

void _text_mm_metric(visual_t rdc, const xfont_t* pxf, xsize_t* pxs)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

    _text_pt_metric(rdc, pxf, pxs);
    
    pxs->fw = (float)pxs->w / _pt_per_mm(rdc, 1);
    pxs->fh = (float)pxs->h / _pt_per_mm(rdc, 0);
}

void _cast_pt_to_mm(visual_t rdc, bool_t horz, xspan_t* pan)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

    if(horz)
        pan->fr = (float)pan->r / _pt_per_mm(rdc, 1);
    else
        pan->fr = (float)pan->r / _pt_per_mm(rdc, 0);
}

void _cast_mm_to_pt(visual_t rdc, bool_t horz, xspan_t* pan)
{
    X11_context_t* ctx = (X11_context_t*)rdc;

    if(horz)
        pan->r = (int)(pan->fr * _pt_per_mm(rdc, 1));
    else
        pan->r = (int)(pan->fr * _pt_per_mm(rdc, 0));
}

#endif //XDU_SUPPORT_CONTEXT
