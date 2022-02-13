/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc stacktable document

	@module	stacktable.h | interface file

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

#ifndef _STACKTABLE_H
#define _STACKTABLE_H

#include "xdkdef.h"
#include "dlink.h"

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API link_t_ptr create_stack_table(void);

EXP_API void destroy_stack_table(link_t_ptr st);

EXP_API void clear_stack_table(link_t_ptr st);

EXP_API void push_stack_node(link_t_ptr st,void* data);

EXP_API void* pop_stack_node(link_t_ptr st);

EXP_API void* pick_stack_node(link_t_ptr st);

EXP_API int get_stack_node_count(link_t_ptr st);

EXP_API void* peek_stack_node(link_t_ptr st, int index);

#ifdef	__cplusplus
}
#endif

#endif /*_STACKTABLE_H*/