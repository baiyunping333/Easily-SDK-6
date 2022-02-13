/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc svg interface document

	@module	svginf.c | implement file

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

#include "svginf.h"
#include "svggdi.h"
#include "svgcanv.h"

#include "xdlview.h"

#if defined(XDL_SUPPORT_GDI)

void svg_get_canvas_interface(canvas_t canv, drawing_interface* pif)
{
	pif->ctx = (void*)canv;

	pif->pf_get_measure = svg_get_measure;

	pif->pf_rect_tm_to_pt = svg_rect_tm_to_pt;
	pif->pf_rect_pt_to_tm = svg_rect_pt_to_tm;
	pif->pf_size_tm_to_pt = svg_size_tm_to_pt;
	pif->pf_size_pt_to_tm = svg_size_pt_to_tm;
	pif->pf_point_tm_to_pt = svg_point_tm_to_pt;
	pif->pf_point_pt_to_tm = svg_point_pt_to_tm;

	pif->pf_draw_line = svg_draw_line;
	pif->pf_draw_bezier = svg_draw_bezier;
	pif->pf_draw_curve = svg_draw_curve;
	pif->pf_draw_arc = svg_draw_arc;
	pif->pf_draw_polyline = svg_draw_polyline;

	pif->pf_draw_sector = svg_draw_sector;
	pif->pf_draw_pie = svg_draw_pie;
	pif->pf_draw_triangle = svg_draw_triangle;
	pif->pf_draw_rect = svg_draw_rect;
	pif->pf_draw_round = svg_draw_round;
	pif->pf_draw_ellipse = svg_draw_ellipse;
	pif->pf_draw_polygon = svg_draw_polygon;
	pif->pf_draw_equilagon = svg_draw_equilagon;
	pif->pf_draw_path = svg_draw_path;

	pif->pf_text_metric = svg_text_metric;
	pif->pf_text_size = svg_text_size;
	pif->pf_draw_text = svg_draw_text;
	pif->pf_text_out = svg_text_out;
	pif->pf_multi_line = svg_multi_line;

	pif->pf_color_out = svg_color_out;
	pif->pf_draw_image = svg_draw_image;
	pif->pf_draw_icon = svg_draw_icon;
	pif->pf_draw_thumb = svg_draw_thumb;

	pif->pf_get_visual_interface = svg_get_visual_interface;
	pif->pf_get_visual_handle = svg_get_canvas_visual;

	parse_xcolor(&pif->mode.clr_bkg, GDI_ATTR_RGB_WHITE);
	parse_xcolor(&pif->mode.clr_frg, GDI_ATTR_RGB_GRAY);
	parse_xcolor(&pif->mode.clr_txt, GDI_ATTR_RGB_BLACK);
	parse_xcolor(&pif->mode.clr_msk, GDI_ATTR_RGB_WHITE);
	parse_xcolor(&pif->mode.clr_ico, GDI_ATTR_RGB_BLACK);
}

void svg_get_visual_interface(visual_t visual, drawing_interface* pif)
{
	pif->ctx = (void*)visual;

	pif->pf_get_measure = svg_get_measure_raw;

	pif->pf_draw_line = svg_draw_line_raw;
	pif->pf_draw_bezier = svg_draw_bezier_raw;
	pif->pf_draw_curve = svg_draw_curve_raw;
	pif->pf_draw_arc = svg_draw_arc_raw;
	pif->pf_draw_polyline = svg_draw_polyline_raw;

	pif->pf_draw_rect = svg_draw_rect_raw;
	pif->pf_draw_triangle = svg_draw_triangle_raw;
	pif->pf_draw_round = svg_draw_round_raw;
	pif->pf_draw_ellipse = svg_draw_ellipse_raw;
	pif->pf_draw_pie = svg_draw_pie_raw;
	pif->pf_draw_sector = svg_draw_sector_raw;
	pif->pf_draw_polygon = svg_draw_polygon_raw;
	pif->pf_draw_equilagon = svg_draw_equilagon_raw;
	pif->pf_draw_path = svg_draw_path_raw;

	pif->pf_text_metric = svg_text_metric_raw;
	pif->pf_text_size = svg_text_size_raw;
	pif->pf_draw_text = svg_draw_text_raw;
	pif->pf_text_out = svg_text_out_raw;
	pif->pf_multi_line = svg_multi_line_raw;

	pif->pf_color_out = svg_color_out_raw;
	pif->pf_draw_image = svg_draw_image_raw;
	pif->pf_draw_icon = svg_draw_icon_raw;
}

#endif /*XDL_SUPPORT_VIEW*/