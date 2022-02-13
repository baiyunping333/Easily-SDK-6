/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc grid bag document

	@module	gridbag.h | interface file

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

#ifndef _GRIDBAG_H
#define _GRIDBAG_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_DOC


#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION save_form_to_grid_row: save form content to a grid row.
@INPUT link_t_ptr form: the form link component.
@INPUT link_t_ptr grid: the grid link component.
@INPUT link_t_ptr row: the row link component.
@RETURN void: none.
*/
EXP_API void save_form_to_grid_row(link_t_ptr form, link_t_ptr grid, link_t_ptr row);

/*
@FUNCTION load_form_from_grid_row: load form content from a grid row.
@INPUT link_t_ptr form: the form link component.
@INPUT link_t_ptr grid: the grid link component.
@INPUT link_t_ptr row: the row link component.
@RETURN void: none.
*/
EXP_API void load_form_from_grid_row(link_t_ptr form, link_t_ptr grid, link_t_ptr row);

/*
@FUNCTION save_statis_to_grid: save statisic content to a grid row.
@INPUT link_t_ptr statis: the statis link component.
@INPUT link_t_ptr grid: the grid link component.
@INPUT link_t_ptr row: the row link component.
@RETURN void: none.
*/
EXP_API void save_statis_to_grid(link_t_ptr statis, link_t_ptr grid);

/*
@FUNCTION load_statis_from_grid: load statis content from a grid row.
@INPUT link_t_ptr statis: the statis link component.
@INPUT link_t_ptr grid: the grid link component.
@INPUT link_t_ptr row: the row link component.
@RETURN void: none.
*/
EXP_API void load_statis_from_grid(link_t_ptr statis, link_t_ptr grid);

/*
@FUNCTION load_rich_from_grid_row: load rich content from a grid row.
@INPUT link_t_ptr rich: the rich link component.
@INPUT link_t_ptr grid: the grid link component.
@INPUT link_t_ptr row: the row link component.
@RETURN void: none.
*/
EXP_API void load_rich_from_grid_row(link_t_ptr rich, link_t_ptr grid, link_t_ptr rlk);

/*
@FUNCTION save_rich_to_grid_row: save rich content to a grid row.
@INPUT link_t_ptr rich: the rich link component.
@INPUT link_t_ptr grid: the grid link component.
@INPUT link_t_ptr row: the row link component.
@RETURN void: none.
*/
EXP_API void save_rich_to_grid_row(link_t_ptr rich, link_t_ptr grid, link_t_ptr rlk);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_DOC*/

#endif /*GRIDBAG_H*/