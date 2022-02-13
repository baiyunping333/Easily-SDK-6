/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc notes document

	@module	notesdoc.c | implement file

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

#include "notesdoc.h"

#include "xdlutil.h"
#include "xdldoc.h"

#ifdef XDL_SUPPORT_DOC

link_t_ptr create_notes_doc()
{
	link_t_ptr ptr;

	ptr = create_dom_doc();
	set_dom_node_name(ptr,DOC_NOTES,-1);

	return ptr;
}

void destroy_notes_doc(link_t_ptr ptr)
{
	destroy_dom_doc(ptr);
}

bool_t is_notes_doc(link_t_ptr ptr)
{
	return  (compare_text(get_dom_node_name_ptr(ptr), -1, DOC_NOTES, -1, 1) == 0) ? 1 : 0;
}

#endif //_NOTESDOC_H
