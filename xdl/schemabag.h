/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc schema bag document

	@module	schemabag.h | interface file

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

#ifndef _SCHEMABAG_H
#define _SCHEMABAG_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_DOC

#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION export_grid_schema: export grid col set defination to a schema document.
@INPUT link_t_ptr ptr: the grid link component.
@INPUT link_t_ptr sch: the schema link component.
@RETURN void: none.
*/
EXP_API void export_grid_schema(link_t_ptr ptr, link_t_ptr sch);

/*
@FUNCTION import_grid_schema: import grid col set defination from a schema document.
@INPUT link_t_ptr ptr: the grid link component.
@INPUT link_t_ptr sch: the schema link component.
@RETURN void: none.
*/
EXP_API void import_grid_schema(link_t_ptr ptr, link_t_ptr sch);

/*
@FUNCTION export_grid_data: export grid row set to dom document using schema defination.
@INPUT link_t_ptr ptr: the grid link component.
@INPUT link_t_ptr sch: the schema link component.
@OUTPUT link_t_ptr dom: the dom link component.
@RETURN void: none.
*/
EXP_API void export_grid_data(link_t_ptr ptr, link_t_ptr sch, link_t_ptr dom);

/*
@FUNCTION import_grid_data: import grid row set from dom document using schema defination.
@INPUT link_t_ptr ptr: the grid link component.
@INPUT link_t_ptr sch: the schema link component.
@OUTPUT link_t_ptr dom: the dom link component.
@RETURN void: none.
*/
EXP_API void import_grid_data(link_t_ptr ptr, link_t_ptr sch, link_t_ptr dom);

/*
@FUNCTION export_statis_schema: export statis yax set defination to a schema document.
@INPUT link_t_ptr ptr: the statis link component.
@INPUT link_t_ptr sch: the schema link component.
@RETURN void: none.
*/
EXP_API void export_statis_schema(link_t_ptr ptr, link_t_ptr sch);

/*
@FUNCTION export_statis_data: export statis xax set to dom document using schema defination.
@INPUT link_t_ptr ptr: the statis link component.
@INPUT link_t_ptr sch: the schema link component.
@OUTPUT link_t_ptr dom: the dom link component.
@RETURN void: none.
*/
EXP_API void export_statis_data(link_t_ptr ptr, link_t_ptr sch, link_t_ptr dom);

/*
@FUNCTION import_statis_data: import statis xax set from dom document using schema defination.
@INPUT link_t_ptr ptr: the statis link component.
@INPUT link_t_ptr sch: the schema link component.
@OUTPUT link_t_ptr dom: the dom link component.
@RETURN void: none.
*/
EXP_API void import_statis_data(link_t_ptr ptr, link_t_ptr sch, link_t_ptr dom);

/*
@FUNCTION export_form_schema: export form field set defination to a schema document.
@INPUT link_t_ptr ptr: the statis link component.
@INPUT link_t_ptr sch: the schema link component.
@RETURN void: none.
*/
EXP_API void export_form_schema(link_t_ptr ptr, link_t_ptr sch);

/*
@FUNCTION export_form_data: export form field data to dom document using schema defination.
@INPUT link_t_ptr ptr: the form link component.
@INPUT link_t_ptr sch: the schema link component.
@OUTPUT link_t_ptr dom: the dom link component.
@RETURN void: none.
*/
EXP_API void export_form_data(link_t_ptr ptr, link_t_ptr sch, link_t_ptr dom);

/*
@FUNCTION import_form_data: import form field data from dom document using schema defination.
@INPUT link_t_ptr ptr: the form link component.
@INPUT link_t_ptr sch: the schema link component.
@OUTPUT link_t_ptr dom: the dom link component.
@RETURN void: none.
*/
EXP_API void import_form_data(link_t_ptr ptr, link_t_ptr sch, link_t_ptr dom);

/*
@FUNCTION export_rich_schema: export rich anchor set defination to a schema document.
@INPUT link_t_ptr ptr: the rich link component.
@INPUT link_t_ptr sch: the schema link component.
@RETURN void: none.
*/
EXP_API void export_rich_schema(link_t_ptr ptr, link_t_ptr sch);

/*
@FUNCTION export_rich_data: export rich anchor set to dom document using schema defination.
@INPUT link_t_ptr ptr: the rich link component.
@INPUT link_t_ptr sch: the schema link component.
@OUTPUT link_t_ptr dom: the dom link component.
@RETURN void: none.
*/
EXP_API void export_rich_data(link_t_ptr ptr, link_t_ptr sch, link_t_ptr dom);

/*
@FUNCTION import_rich_data: import rich anchor set from dom document using schema defination.
@INPUT link_t_ptr ptr: the rich link component.
@INPUT link_t_ptr sch: the schema link component.
@OUTPUT link_t_ptr dom: the dom link component.
@RETURN void: none.
*/
EXP_API void import_rich_data(link_t_ptr ptr, link_t_ptr sch, link_t_ptr dom);

//EXP_API void export_dom_node(link_t_ptr node, link_t_ptr sch, link_t_ptr dom);

//EXP_API void import_dom_node(link_t_ptr node, link_t_ptr sch, link_t_ptr dom);

#ifdef	__cplusplus
}
#endif

#endif //XDLDOC

#endif //SCHEMABAG_H