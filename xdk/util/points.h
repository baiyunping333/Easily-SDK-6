/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdk utility document

	@module	points.h | interface file

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

#ifndef _POINTS_H
#define _POINTS_H

#include "../xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API int ft_parse_points_from_token(xpoint_t* ppt, int max, const tchar_t* token, int len);

EXP_API int ft_format_points_to_token(const xpoint_t* ppt, int n, tchar_t* buf, int max);

EXP_API int pt_parse_points_from_token(xpoint_t* ppt, int max, const tchar_t* token, int len);

EXP_API int pt_format_points_to_token(const xpoint_t* ppt, int n, tchar_t* buf, int max);

EXP_API int parse_dicm_point(const tchar_t* token, int len, xpoint_t* ppt, int max);

EXP_API int format_dicm_point(const xpoint_t* ppt, int count, tchar_t* buf, int max);

EXP_API bool_t inside_rowcol(int row, int col, int from_row, int from_col, int to_row, int to_col);

EXP_API int compare_rowcol(int from_row, int from_col, int to_row, int to_col);

EXP_API bool_t ft_inside(float x, float y, float x1, float y1, float x2, float y2);

EXP_API bool_t pt_inside(int x, int y, int x1, int y1, int x2, int y2);

EXP_API bool_t ft_in_rect(const xpoint_t* ppt, const xrect_t* prt);

EXP_API bool_t pt_in_rect(const xpoint_t* ppt, const xrect_t* prt);

EXP_API void ft_offset_point(xpoint_t* ppt, float cx, float cy);

EXP_API void pt_offset_point(xpoint_t* ppt, int cx, int cy);

EXP_API void ft_center_rect(xrect_t* pxr, float cx, float cy);

EXP_API void pt_center_rect(xrect_t* pxr, int cx, int cy);

EXP_API void ft_expand_rect(xrect_t* pxr, float cx, float cy);

EXP_API void pt_expand_rect(xrect_t* pxr, int cx, int cy);

EXP_API void ft_offset_rect(xrect_t* pxr, float cx, float cy);

EXP_API void pt_offset_rect(xrect_t* pxr, int cx, int cy);

EXP_API void ft_merge_rect(xrect_t* pxr, const xrect_t* pxr_nxt);

EXP_API void pt_merge_rect(xrect_t* pxr, const xrect_t* pxr_nxt);

EXP_API bool_t ft_clip_rect(xrect_t* pxr, const xrect_t* pxr_sub);

EXP_API bool_t pt_clip_rect(xrect_t* pxr, const xrect_t* pxr_sub);

EXP_API void ft_inter_rect(xrect_t* pxr, const xrect_t* pxr_sub);

EXP_API void pt_inter_rect(xrect_t* pxr, const xrect_t* pxr_sub);

EXP_API void pt_inter_square(xrect_t* pxr, const xrect_t* pxr_org);

EXP_API void ft_inter_square(xrect_t* pxr, const xrect_t* pxr_org);

EXP_API void ft_cell_rect(xrect_t* pxr, bool_t horz, int rows, int cols, int index);

EXP_API void pt_cell_rect(xrect_t* pxr, bool_t horz, int rows, int cols, int index);

EXP_API bool_t rect_is_empty(const xrect_t* pxr);

EXP_API void empty_rect(xrect_t* pxr);

EXP_API void pt_adjust_rect(xrect_t* pxr, int src_width, int src_height, const tchar_t* horz_align, const tchar_t* vert_align);

EXP_API void ft_adjust_rect(xrect_t* pxr, float src_width, float src_height, const tchar_t* horz_align, const tchar_t* vert_align);

EXP_API int ft_quadrant(const xpoint_t* ppo, const xpoint_t* ppt, bool_t sflag);

EXP_API int pt_quadrant(const xpoint_t* ppo, const xpoint_t* ppt, bool_t sflag);

EXP_API void pt_calc_radian(int sweep_flag, int large_flag, int rx, int ry, const xpoint_t* ppt1, const xpoint_t* ppt2, xpoint_t* ppt, double* arc_from, double* arc_sweep);

EXP_API void ft_calc_radian(int sweep_flag, int large_flag, float rx, float ry, const xpoint_t* ppt1, const xpoint_t* ppt2, xpoint_t* ppt, double* arc_from, double* arc_sweep);

EXP_API void ft_calc_sector(const xpoint_t* ppt, const xspan_t* prl, const xspan_t* prs, double from, double sweep, xpoint_t* pa, int n);

EXP_API void pt_calc_sector(const xpoint_t* ppt, const xspan_t* prl, const xspan_t* prs, double from, double sweep, xpoint_t* pa, int n);

EXP_API void ft_calc_equilater(const xpoint_t* ppt, const xspan_t* pxn, xpoint_t* pa, int n);

EXP_API void pt_calc_equilater(const xpoint_t* ppt, const xspan_t* pxn, xpoint_t* pa, int n);

#ifdef	__cplusplus
}
#endif

#endif