/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc trie tree document

	@module	trietree.h | interface file

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

#ifndef _TRIETREE_H
#define _TRIETREE_H

#include "xdkdef.h"
#include "dlink.h"

/**********************************************************************************************************************/

typedef bool_t (*ENUM_TRIETREE_NODE)(const tchar_t* key, link_t_ptr nlk, void* p);

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API link_t_ptr create_trie_tree(tchar_t kfeed);

	EXP_API void destroy_trie_tree(link_t_ptr trie);

	EXP_API link_t_ptr write_trie_node(link_t_ptr trie, const tchar_t* key, int len, object_t val);

	EXP_API link_t_ptr read_trie_node(link_t_ptr trie, const tchar_t* key, int len, object_t val);

	EXP_API void delete_trie_node(link_t_ptr trie, const tchar_t* key, int len);

	EXP_API link_t_ptr get_trie_node(link_t_ptr trie, const tchar_t* key, int len);

	EXP_API link_t_ptr get_trie_from_node(link_t_ptr node);

	EXP_API bool_t is_trie_leaf(link_t_ptr node);

	EXP_API link_t_ptr enum_trie_tree(link_t_ptr trie, ENUM_TRIETREE_NODE pf, void* param);

	EXP_API const tchar_t* get_trie_node_key_ptr(link_t_ptr node);

	EXP_API int get_trie_node_key(link_t_ptr node, tchar_t* key, int max);

	EXP_API void set_trie_node_key(link_t_ptr node, const tchar_t* key, int len);

	EXP_API const object_t get_trie_node_val_ptr(link_t_ptr node);

	EXP_API void get_trie_node_val(link_t_ptr node, object_t val);

	EXP_API void set_trie_node_val(link_t_ptr node, object_t val);

	EXP_API vword_t get_trie_node_delta(link_t_ptr node);

	EXP_API void set_trie_node_delta(link_t_ptr node, vword_t vw);

#if defined(XDK_SUPPORT_TEST)
	EXP_API void test_trie_tree();
#endif

#ifdef	__cplusplus
}
#endif

#endif /*_TRIETREE_H*/
