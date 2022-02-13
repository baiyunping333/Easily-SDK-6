/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc wordstable document

	@module	wordstable.h | interface file

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

#ifndef _WORDSTABLE_H
#define _WORDSTABLE_H

#include "xdkdef.h"
#include "dlink.h"

/********************************************************************************/
#define set_words_item_hidden(ilk, b)		set_words_item_delta(ilk, (vword_t)b)

#define get_words_item_hidden(ilk)			(bool_t)get_words_item_delta(ilk)
/*********************************************************************************/

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API link_t_ptr create_words_table(int order);

EXP_API void destroy_words_table(link_t_ptr ptr);

EXP_API void clear_words_table(link_t_ptr ptr);

EXP_API bool_t is_words_table(link_t_ptr ptr);

EXP_API bool_t is_words_item(link_t_ptr ptr, link_t_ptr ilk);

EXP_API int get_words_item_count(link_t_ptr ptr);

EXP_API link_t_ptr insert_words_item(link_t_ptr ptr, const tchar_t* val, int len);

EXP_API link_t_ptr get_words_item(link_t_ptr ptr, const tchar_t* val, int len);

EXP_API void delete_words_item(link_t_ptr ptr, link_t_ptr pos);

EXP_API const tchar_t* get_words_item_text_ptr(link_t_ptr ilk);

EXP_API void set_words_item_delta(link_t_ptr ilk, vword_t data);

EXP_API vword_t get_words_item_delta(link_t_ptr ilk);

EXP_API link_t_ptr get_words_next_item(link_t_ptr ptr,link_t_ptr pos);

EXP_API link_t_ptr get_words_prev_item(link_t_ptr ptr,link_t_ptr pos);

EXP_API link_t_ptr get_words_item_at(link_t_ptr ptr, int index);

EXP_API int get_words_item_index(link_t_ptr ptr, link_t_ptr ilk);

EXP_API link_t_ptr get_words_next_visible_item(link_t_ptr ptr, link_t_ptr pos);

EXP_API link_t_ptr get_words_prev_visible_item(link_t_ptr ptr, link_t_ptr pos);

EXP_API int get_words_visible_item_count(link_t_ptr ptr);

EXP_API void words_table_parse_tokens(link_t_ptr ptr,const tchar_t* tokens,int len,tchar_t feed);

EXP_API int words_table_format_tokens(link_t_ptr ptr,tchar_t* buf,int max,tchar_t feed);

#ifdef	__cplusplus
}
#endif

#endif //_WORDSTABLE_H
