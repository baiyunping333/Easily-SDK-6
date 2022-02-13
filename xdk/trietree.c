/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc trie tree document

	@module	trietree.c | implement file

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

#include "trietree.h"

#include "xdkimp.h"
#include "xdkoem.h"
#include "xdkstd.h"
#include "xdkutil.h"

typedef struct _trie_node_t{
	link_t lkSibling;	/*the self link component, linked to parent*/
	link_t lkChild;		/*the child node link root*/
	
	tchar_t feed;		/*key feed*/
	tchar_t* key;		/*trie key*/
	int len;			/*key length*/

	object_t val;		/*object val*/
	vword_t delta;		/*runtime val*/
}trie_node_t;

/*restore trie_treenode from self link ptr*/
#define TrieNodeFromLink(p) TypePtrFromSibling(trie_node_t,p)
/*restore trie_treenode from child link ptr*/
#define TrieNodeFromChild(p) TypePtrFromChild(trie_node_t,p)

static trie_node_t* _alloc_trie_node(const tchar_t* key, int len)
{
	trie_node_t* pbn;

	pbn = (trie_node_t*)xmem_alloc(sizeof(trie_node_t));
	pbn->lkSibling.tag = lkTrieNode;
	pbn->lkSibling.next = NULL;
	pbn->lkSibling.prev = NULL;
	init_root_link(&pbn->lkChild);

	if (len < 0)
		len = xslen(key);

	pbn->key = xsnclone(key, len);
	pbn->len = len;
	pbn->val = NULL;

	return pbn;
}

static void _free_trie_node(trie_node_t* ptt)
{
	if (ptt->val)
	{
		object_free(ptt->val);
	}
	xmem_free(ptt->key);
	xmem_free(ptt);
}

static tchar_t _get_trie_key_feed(link_t_ptr ilk)
{
	trie_node_t* ptt;

	ptt = TrieNodeFromLink(ilk);

	return ptt->feed;
}

static link_t_ptr _get_trie_parent_node(link_t_ptr ilk)
{
	trie_node_t* ptt;
	link_t_ptr parent;

	parent = get_root_link(ilk);
	if (!parent)
		return NULL;

	ptt = TrieNodeFromChild(parent);

	return &(ptt->lkSibling);
}

static link_t_ptr _get_trie_first_child_node(link_t_ptr ilk)
{
	trie_node_t* ptt;

	ptt = TrieNodeFromLink(ilk);

	return get_first_link(&ptt->lkChild);
}

static link_t_ptr _get_trie_next_sibling_node(link_t_ptr ilk)
{
	return get_next_link(ilk);
}

static void _delete_trie_node(link_t_ptr ilk)
{
	link_t_ptr parent, child;
	trie_node_t* ptt;
	trie_node_t* pup;
	link_t_ptr stack = NULL;

	while (ilk)
	{
		child = _get_trie_first_child_node(ilk);
		if (child)
		{
			if (!stack)
				stack = create_stack_table();

			push_stack_node(stack, (void*)ilk);

			ilk = child;
			continue;
		}

		parent = _get_trie_parent_node(ilk);
		if (parent)
		{
			///delete node link from parent node
			pup = TrieNodeFromLink(parent);
			delete_link(&(pup->lkChild), ilk);
		}

		ptt = TrieNodeFromLink(ilk);
		_free_trie_node(ptt);

		ilk = (stack) ? (link_t_ptr)pop_stack_node(stack) : NULL;
	}

	if (stack)
		destroy_stack_table(stack);
}

typedef struct _trie_indi_t{
	link_t_ptr parent;
	link_t_ptr child;
	tchar_t* sub;
	int len;
}trie_indi_t;

static void _find_trie_node(link_t_ptr ilk, const tchar_t* key, int len, trie_indi_t* pti)
{
	link_t_ptr child = NULL;
	trie_node_t* ptc;
	tchar_t feed;

	int rt;
	int n, total = 0;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	if (len < 0)
		len = xslen(key);

	pti->parent = ilk;
	pti->child = NULL;

	while (pti->parent)
	{
		feed = _get_trie_key_feed(pti->parent);

		pti->sub = NULL;
		pti->len = 0;

		n = parse_string_token((key + total), (len - total), feed, &(pti->sub), &(pti->len));
		total += n;
		if (!pti->len)
			break;
		
		rt = 1;
		child = _get_trie_first_child_node(pti->parent);
		while (child)
		{
			ptc = TrieNodeFromLink(child);

			rt = compare_text(ptc->key, ptc->len, pti->sub, pti->len, 0);
			if (!rt)
			{
				pti->parent = child;
				pti->child = NULL;
				break;
			}
			else if (rt > 0)
			{
				pti->child = child;
				break;
			}

			child = _get_trie_next_sibling_node(child);
		}

		if (!rt)
			continue;
		else
			break;
	}
}

link_t_ptr create_trie_tree(tchar_t kfeed)
{
	trie_node_t* pti;

	pti = _alloc_trie_node(NULL, 0);
	pti->feed = kfeed;

	return &(pti->lkSibling);
}

void destroy_trie_tree(link_t_ptr ptr)
{
	_delete_trie_node(ptr);
}

link_t_ptr write_trie_node(link_t_ptr ptr, const tchar_t* key, int len, object_t val)
{
	trie_node_t* ptt;
	trie_node_t* ptc;
	trie_indi_t ti = { 0 };
	tchar_t feed;

	XDK_ASSERT(ptr && ptr->tag == lkTrieNode);

	if (len < 0)
		len = xslen(key);

	feed = _get_trie_key_feed(ptr);

	_find_trie_node(ptr, key, len, &ti);

	while (ti.len)
	{
		ptt = TrieNodeFromLink(ti.parent);

		ptc = _alloc_trie_node(ti.sub, ti.len);
		ptc->feed = feed;

		if (ti.child)
			insert_link_before(&(ptt->lkChild), ti.child, &(ptc->lkSibling));
		else
			insert_link_after(&(ptt->lkChild), LINK_LAST, &(ptc->lkSibling));

		ti.sub += ti.len;
		if (*(ti.sub) == feed)
			ti.sub++;

		len -= (int)(ti.sub - key);
		key = ti.sub;
		_find_trie_node(&(ptc->lkSibling), key, len, &ti);
	}
	
	ptt = TrieNodeFromLink(ti.parent);
	ptt->lkSibling.tag = lkTrieLeaf;

	if (ptt->val && val)
		object_copy(ptt->val, val);
	else if (val)
		ptt->val = object_clone(val);
	else if (ptt->val)
		object_free(ptt->val);

	return ti.parent;
}

link_t_ptr read_trie_node(link_t_ptr ptr, const tchar_t* key, int len, object_t val)
{
	trie_node_t* ptt;
	trie_indi_t ti = { 0 };
	tchar_t feed;

	XDK_ASSERT(ptr && ptr->tag == lkTrieNode);

	if (len < 0)
		len = xslen(key);

	feed = _get_trie_key_feed(ptr);

	_find_trie_node(ptr, key, len, &ti);

	if (ti.len)
	{
		object_empty(val);
		return NULL;
	}

	ptt = TrieNodeFromLink(ti.parent);

	if (ptt->val && val)
		object_copy(val, ptt->val);
	else if (val)
		object_empty(val);

	return ti.parent;
}

void delete_trie_node(link_t_ptr ptr, const tchar_t* key, int len)
{
	trie_node_t* ptt;
	trie_indi_t ti = { 0 };
	link_t_ptr ilk;

	XDK_ASSERT(ptr && ptr->tag == lkTrieNode);

	if (len < 0)
		len = xslen(key);

	_find_trie_node(ptr, key, len, &ti);

	if (ti.len)
		return;

	if (!is_trie_leaf(ti.parent))
		return;

	//if is not last node, clear node tval
	if (_get_trie_first_child_node(ti.parent))
	{
		ptt = TrieNodeFromLink(ti.parent);
		ptt->lkSibling.tag = lkTrieNode;

		if (ptt->val) object_free(ptt->val);
		ptt->val = NULL;
		ptt->delta = 0;
		return;
	}

	ilk = ti.parent;
	ti.parent = _get_trie_parent_node(ti.parent);

	_delete_trie_node(ilk);

	//else delete node
	while (ti.parent && !is_trie_leaf(ti.parent) && !_get_trie_first_child_node(ti.parent))
	{
		ilk = ti.parent;
		ti.parent = _get_trie_parent_node(ti.parent);

		_delete_trie_node(ilk);

		if (ti.parent == ptr)
			break;
	}
}

bool_t is_trie_leaf(link_t_ptr ilk)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	return (ptt->lkSibling.tag == lkTrieLeaf)? 1 : 0;
}

link_t_ptr get_trie_node(link_t_ptr ptr, const tchar_t* key, int len)
{
	trie_indi_t ti = { 0 };

	XDK_ASSERT(ptr && ptr->tag == lkTrieNode);

	if (len < 0)
		len = xslen(key);

	_find_trie_node(ptr, key, len, &ti);

	if (ti.len)
		return NULL;

	if (!is_trie_leaf(ti.parent))
		return NULL;

	return ti.parent;
}

link_t_ptr get_trie_from_node(link_t_ptr ilk)
{
	link_t_ptr plk;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	plk = _get_trie_parent_node(ilk);
	while (plk)
	{
		ilk = plk;
		plk = _get_trie_parent_node(plk);
	}

	return ilk;
}

const tchar_t* get_trie_node_key_ptr(link_t_ptr ilk)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	return ptt->key;
}

int get_trie_node_key(link_t_ptr ilk, tchar_t* key, int max)
{
	trie_node_t* ptt;
	int len;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	len = xslen(ptt->key);
	max = (max < len) ? max : len;

	xsncpy(key, ptt->key, max);

	return max;
}

void set_trie_node_key(link_t_ptr ilk, const tchar_t* key, int len)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	xsfree(ptt->key);
	ptt->key = xsnclone(key, len);
}

const object_t get_trie_node_val_ptr(link_t_ptr ilk)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	return ptt->val;
}

void get_trie_node_val(link_t_ptr ilk, object_t val)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	if (ptt->val)
		object_copy(val, ptt->val);
	else
		object_empty(val);
}

void set_trie_node_val(link_t_ptr ilk, object_t val)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	if (ptt->val && val)
		object_copy(ptt->val, val);
	else if (val)
		ptt->val = object_clone(val);
}

void attach_trie_node_val(link_t_ptr ilk, object_t val)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	if (ptt->val)
		object_free(ptt->val);

	ptt->val = val;
}

object_t detach_trie_node_val(link_t_ptr ilk)
{
	trie_node_t* ptt;
	object_t val;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	val = ptt->val;
	ptt->val = NULL;

	return val;
}

vword_t get_trie_node_delta(link_t_ptr ilk)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	return ptt->delta;
}

void set_trie_node_delta(link_t_ptr ilk, vword_t vw)
{
	trie_node_t* ptt;

	XDK_ASSERT(ilk && (ilk->tag == lkTrieNode || ilk->tag == lkTrieLeaf));

	ptt = TrieNodeFromLink(ilk);

	ptt->delta = vw;
}

link_t_ptr enum_trie_tree(link_t_ptr ptr, ENUM_TRIETREE_NODE pf, void* param)
{
	trie_node_t* ptt;
	trie_node_t* ptc;
	link_t_ptr plk, nlk;
	int i, n;

	link_t_ptr st = NULL;
	tchar_t* key = NULL;
	int len = 0;
	bool_t rt;

	nlk = _get_trie_first_child_node(ptr);
	while (nlk)
	{
		if (is_trie_leaf(nlk) && pf)
		{
			ptc = TrieNodeFromLink(nlk);

			n = (st) ? get_stack_node_count(st) : 0;
			for (i = 0; i < n; i++)
			{
				plk = (link_t_ptr)peek_stack_node(st, i);
				ptt = TrieNodeFromLink(plk);

				len += (xslen(ptt->key) + 1);
				key = xsrealloc(key, len + 1);
				xscat(key, ptt->key);
				xsncat(key, &(ptt->feed), 1);
			}

			len += xslen(ptc->key);
			key = xsrealloc(key, len + 1);
			xscat(key, ptc->key);

			rt = (*pf)(key, nlk, param);

			xsfree(key);
			key = NULL;
			len = 0;

			if (!rt)
				break;
		}

		if (_get_trie_first_child_node(nlk))
		{
			if (!st)
				st = create_stack_table();
			push_stack_node(st, (void*)nlk);

			nlk = _get_trie_first_child_node(nlk);
			continue;
		}

		while (nlk)
		{
			nlk = _get_trie_next_sibling_node(nlk);
			if (nlk)
				break;
			nlk = (st) ? (link_t_ptr)pop_stack_node(st) : NULL;
		}
	}

	if (st)
		destroy_stack_table(st);

	return nlk;
}

#if defined(XDK_SUPPORT_TEST)
static bool_t print_leaf(const tchar_t* key, link_t_ptr nlk, void* p)
{
	object_t ob = get_trie_node_val_ptr(nlk);

	string_t vs = string_alloc();
	object_get_string(ob, vs);

	_tprintf(_T("%s %s\n"), key, string_ptr(vs));

	string_free(vs);

	return 1;
}

void test_trie_tree()
{
	object_t v = object_alloc();

	string_t vs = string_alloc();
	string_cpy(vs, _T("trie"), -1);
	object_set_string(v, vs);
	string_free(vs);

	link_t_ptr ptr = create_trie_tree(_T('.'));

	link_t_ptr ilk = write_trie_node(ptr, _T("1.111.1111"), -1, v);

	ilk = write_trie_node(ptr, _T("1.111"), -1, v);

	ilk = write_trie_node(ptr, _T("1.11.111"), -1, v);

	ilk = write_trie_node(ptr, _T("1.11.222"), -1, v);

	ilk = write_trie_node(ptr, _T("1.11.111.11"), -1, v);

	ilk = write_trie_node(ptr, _T("1.1.11.11"), -1, v);

	ilk = write_trie_node(ptr, _T("1.2.3"), -1, v);

	enum_trie_tree(ptr, print_leaf, NULL);

	delete_trie_node(ptr, _T("1.111"), -1);

	delete_trie_node(ptr, _T("1.11.111.11"), -1);

	delete_trie_node(ptr, _T("1.2.3.4"), -1);

	delete_trie_node(ptr, _T("1.2.3"), -1);

	delete_trie_node(ptr, _T("1.11.111"), -1);

	delete_trie_node(ptr, _T("1.11.222"), -1);

	delete_trie_node(ptr, _T("1.1.11.11"), -1);

	delete_trie_node(ptr, _T("1.111.1111"), -1);

	enum_trie_tree(ptr, print_leaf, NULL);

	object_free(v);

	destroy_trie_tree(ptr);
}


#endif