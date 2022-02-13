/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc json document

	@module	jsondoc.c | implement file

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

#include "jsondoc.h"
#include "domdoc.h"

#ifdef XDL_SUPPORT_DOC

link_t_ptr create_json_doc()
{
	link_t_ptr ptr;

	ptr = create_dom_doc();
	set_dom_node_name(ptr, DOC_JSON, -1);

	return ptr;
}

void destroy_json_doc(link_t_ptr ptr)
{
	destroy_dom_doc(ptr);
}

void clear_json_doc(link_t_ptr ptr)
{
	delete_dom_child_nodes(ptr);
}

bool_t is_json_doc(link_t_ptr ptr)
{
	return (compare_text(get_dom_node_name_ptr(ptr), -1, DOC_JSON, -1, 0) == 0) ? 1 : 0;
}

bool_t is_json_item(link_t_ptr ptr, link_t_ptr ilk)
{
	if (!ilk)
		return 0;

	return is_dom_child_node(ptr, ilk);
}

int get_json_item_count(link_t_ptr ilk)
{
	return get_dom_child_node_count(ilk);
}

link_t_ptr	get_json_item(link_t_ptr ptr, const tchar_t* iname)
{
	return find_dom_node_by_name(ptr, 1, iname, -1);
}

void delete_json_item(link_t_ptr ilk)
{
	delete_dom_node(ilk);
}

link_t_ptr insert_json_item(link_t_ptr ptr, link_t_ptr pos)
{
	link_t_ptr nlk;

	nlk = insert_dom_node(ptr, pos);

	return nlk;
}

link_t_ptr get_json_first_child_item(link_t_ptr ilk)
{
	return get_dom_first_child_node(ilk);
}

link_t_ptr get_json_last_child_item(link_t_ptr ilk)
{
	return get_dom_last_child_node(ilk);
}

link_t_ptr get_json_parent_item(link_t_ptr ilk)
{
	return get_dom_parent_node(ilk);
}

link_t_ptr get_json_next_sibling_item(link_t_ptr ilk)
{
	return get_dom_next_sibling_node(ilk);
}

link_t_ptr get_json_prev_sibling_item(link_t_ptr ilk)
{
	return get_dom_prev_sibling_node(ilk);
}


#endif //XDLDOC