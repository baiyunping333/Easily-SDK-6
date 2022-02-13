/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc notes document

	@module	notesdoc.h | interface file

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

#ifndef _NOTESDOC_H
#define _NOTESDOC_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_DOC

/***********************************************************************************************************/
#define set_notes_id(ptr,val)							set_dom_node_attr(ptr,ATTR_ID,-1,val,-1)

#define get_notes_id_ptr(ptr)							get_dom_node_attr_ptr(ptr,ATTR_ID,-1)

#define set_notes_time(ptr,val)							set_dom_node_attr(ptr,ATTR_TIME,-1,val,-1)

#define get_notes_time_ptr(ptr)							get_dom_node_attr_ptr(ptr,ATTR_TIME,-1)

#define set_notes_to(ptr,val)							set_dom_node_attr(ptr,ATTR_TO,-1,val,-1)

#define get_notes_to_ptr(ptr)							get_dom_node_attr_ptr(ptr,ATTR_TO,-1)

#define set_notes_from(ptr,val)							set_dom_node_attr(ptr,ATTR_FROM,-1,val,-1)

#define get_notes_from_ptr(ptr)							get_dom_node_attr_ptr(ptr,ATTR_FROM,-1)

#define set_notes_type(ptr,val)							set_dom_node_attr(ptr,ATTR_TYPE,-1,val,-1)

#define get_notes_type_ptr(ptr)							get_dom_node_attr_ptr(ptr,ATTR_TYPE,-1)

#define set_notes_text(ptr,val,len)						set_dom_node_text(ptr,val,len)

#define get_notes_text_ptr(ptr)							get_dom_node_text_ptr(ptr)

/*************************************************document node name****************************************/
#ifdef	__cplusplus
extern "C" {
#endif

EXP_API link_t_ptr create_notes_doc();

EXP_API void destroy_notes_doc(link_t_ptr ptr);

EXP_API bool_t is_notes_doc(link_t_ptr ptr);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_DOC*/

#endif //_NOTESDOC_H