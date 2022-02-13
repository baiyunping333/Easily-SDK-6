/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc tkv document

	@module	tkv.c | implement file

	@devnote 张文权 2021.01 - 2021.12 v6.0
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

#include "tkv.h"
#include "tdb.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"

typedef struct _t_tk_ctx
{
	t_tk_hdr hdr;

	link_t_ptr trie;
	bool_t laze;
	t_db_t tdb;
}t_tk_ctx;


t_tk_t tkv_create(tchar_t key_feed)
{
	t_tk_ctx* pobj;

	pobj = (t_tk_ctx*)xmem_alloc(sizeof(t_tk_ctx));

	pobj->trie = create_trie_tree(key_feed);
	pobj->hdr.tag = T_OBJ_TK;

	return &(pobj->hdr);
}

void tkv_destroy(t_tk_t tkv)
{
	t_tk_ctx* pobj = (t_tk_ctx*)tkv;

	XDK_ASSERT(tkv && tkv->tag == T_OBJ_TK);

	destroy_trie_tree(pobj->trie);

	xmem_free(pobj);
}

bool_t tkv_write(t_tk_t tkv, const tchar_t* key, object_t val)
{
	t_tk_ctx* pobj = (t_tk_ctx*)tkv;
	link_t_ptr nlk;
	vword_t mask;
	variant_t kh = NULL;

	XDK_ASSERT(tkv && tkv->tag == T_OBJ_TK);

	TRY_CATCH;

	nlk = write_trie_node(pobj->trie, key, -1, NULL);

	if (pobj->tdb && !pobj->laze)
	{
		kh = variant_alloc(VV_STRING_UTF8);
		variant_from_string(kh, key, -1);

		if (!tdb_save(pobj->tdb, kh, val))
		{
			raise_user_error(_T("tkv_write"), _T("tdb_save"));
		}

		variant_free(kh);
		kh = NULL;

		mask = get_trie_node_delta(nlk);
		mask |= TKV_MASK_PERSIST;
		set_trie_node_delta(nlk, mask);
	}
	else
	{
		set_trie_node_val(nlk, val);

		mask = get_trie_node_delta(nlk);
		mask &= (~TKV_MASK_PERSIST);
		set_trie_node_delta(nlk, mask);
	}

	END_CATCH;

	return bool_true;

ONERROR:
	XDK_TRACE_LAST;

	if (kh) variant_free(kh);
	
	return bool_false;
}

bool_t tkv_update(t_tk_t tkv, const tchar_t* key, object_t val)
{
	t_tk_ctx* pobj = (t_tk_ctx*)tkv;
	link_t_ptr nlk;
	vword_t mask;
	variant_t kh = NULL;

	XDK_ASSERT(tkv && tkv->tag == T_OBJ_TK);

	TRY_CATCH;

	kh = variant_alloc(VV_STRING_UTF8);
	variant_from_string(kh, key, -1);

	nlk = get_trie_node(pobj->trie, key, -1);
	if (!nlk && pobj->tdb)
	{
		if (!tdb_clean(pobj->tdb, kh))
		{
			raise_user_error(_T("tkv_update"), _T("tdb_clean"));
		}

		nlk = write_trie_node(pobj->trie, key, -1, NULL);
	}

	if (!nlk)
	{
		raise_user_error(_T("tkv_update"), _T("not exists"));
	}

	if (pobj->tdb && !pobj->laze)
	{
		if (!tdb_save(pobj->tdb, kh, val))
		{
			raise_user_error(_T("tkv_update"), _T("tdb_save"));
		}

		mask = get_trie_node_delta(nlk);
		mask |= TKV_MASK_PERSIST;
		set_trie_node_delta(nlk, mask);
	}
	else
	{
		set_trie_node_val(nlk, val);

		mask = get_trie_node_delta(nlk);
		mask &= (~TKV_MASK_PERSIST);
		set_trie_node_delta(nlk, mask);
	}

	variant_free(kh);
	kh = NULL;

	END_CATCH;

	return bool_true;

ONERROR:
	XDK_TRACE_LAST;

	if (kh) variant_free(kh);

	return bool_false;
}

bool_t tkv_read(t_tk_t tkv, const tchar_t* key, object_t val)
{
	t_tk_ctx* pobj = (t_tk_ctx*)tkv;
	link_t_ptr nlk;
	vword_t mask;
	variant_t kh = NULL;

	XDK_ASSERT(tkv && tkv->tag == T_OBJ_TK);

	TRY_CATCH;

	kh = variant_alloc(VV_STRING_UTF8);
	variant_from_string(kh, key, -1);

	nlk = get_trie_node(pobj->trie, key, -1);
	if (!nlk && pobj->tdb)
	{
		if (!tdb_load(pobj->tdb, kh, val))
		{
			raise_user_error(_T("tkv_read"), _T("get_trie_node"));
		}

		nlk = write_trie_node(pobj->trie, key, -1, NULL);

		mask = get_trie_node_delta(nlk);
		mask |= TKV_MASK_PERSIST;
		set_trie_node_delta(nlk, mask);

		variant_free(kh);
		kh = NULL;

		CLN_CATCH;
		return bool_true;
	}

	if (!nlk)
	{
		raise_user_error(_T("tkv_read"), _T("not exists"));
	}

	mask = get_trie_node_delta(nlk);

	if (pobj->tdb && (mask & TKV_MASK_PERSIST))
	{
		if (!tdb_load(pobj->tdb, kh, val))
		{
			raise_user_error(_T("tkv_read"), _T("get_trie_node"));
		}
	}
	else
	{
		get_trie_node_val(nlk, val);
	}

	variant_free(kh);
	kh = NULL;

	END_CATCH;

	return bool_true;

ONERROR:
	XDK_TRACE_LAST;

	if (kh) variant_free(kh);

	return bool_false;
}

bool_t tkv_delete(t_tk_t tkv, const tchar_t* key)
{
	t_tk_ctx* pobj = (t_tk_ctx*)tkv;

	variant_t kh = NULL;

	XDK_ASSERT(tkv && tkv->tag == T_OBJ_TK);

	TRY_CATCH;

	if (pobj->tdb)
	{
		kh = variant_alloc(VV_STRING_UTF8);
		variant_from_string(kh, key, -1);

		if (!tdb_clean(pobj->tdb, kh))
		{
			raise_user_error(_T("tkv_delete"), _T("tdb_clean"));
		}

		variant_free(kh);
		kh = NULL;
	}

	delete_trie_node(pobj->trie, key, -1);

	END_CATCH;

	return bool_true;

ONERROR:
	XDK_TRACE_LAST;

	if (kh) variant_free(kh);

	return bool_false;
}

void tkv_bind(t_tk_t tkv, t_db_t tdb, bool_t laze)
{
	t_tk_ctx* pobj = (t_tk_ctx*)tkv;

	pobj->tdb = tdb;
	pobj->laze = (pobj->tdb) ? laze : 0;
}

static bool_t _load_tdb_node(variant_t key, object_t val, void* p)
{
	t_tk_ctx* pobj = (t_tk_ctx*)p;
	link_t_ptr nlk;
	vword_t mask;
	tchar_t* str;
	int len;
	
	len = variant_to_string(key, NULL, MAX_LONG);
	str = xsalloc(len + 1);
	variant_to_string(key, str, len);

	nlk = write_trie_node(pobj->trie, str, len, NULL);

	mask = get_trie_node_delta(nlk);
	mask |= TKV_MASK_PERSIST;
	set_trie_node_delta(nlk, mask);

	xsfree(str);

	return 1;
}

bool_t tkv_load(t_tk_t tkv)
{
	t_tk_ctx* pobj = (t_tk_ctx*)tkv;

	XDK_ASSERT(tkv && tkv->tag == T_OBJ_TK);

	tdb_enum(pobj->tdb, _load_tdb_node, (void*)pobj);

	return bool_true;
}

static bool_t _save_tdb_node(const tchar_t* key, link_t_ptr nlk, void* p)
{
	t_tk_ctx* pobj = (t_tk_ctx*)p;
	vword_t mask;
	variant_t kh = NULL;
	object_t ob;

	mask = get_trie_node_delta(nlk);

	if (mask & TKV_MASK_PERSIST)
		return bool_true;

	kh = variant_alloc(VV_STRING_GB2312);
	variant_from_string(kh, key, -1);

	ob = get_trie_node_val_ptr(nlk);

	if (!tdb_save(pobj->tdb, kh, ob))
		return bool_false;

	variant_free(kh);
	set_trie_node_val(nlk, NULL);

	mask |= TKV_MASK_PERSIST;
	set_trie_node_delta(nlk, mask);

	return bool_true;;
}

bool_t tkv_flush(t_tk_t tkv)
{
	t_tk_ctx* pobj = (t_tk_ctx*)tkv;
	link_t_ptr nlk;

	XDK_ASSERT(tkv && tkv->tag == T_OBJ_TK);

	nlk = enum_trie_tree(pobj->trie, _save_tdb_node, (void*)pobj);
	
	if (nlk)
	{
		set_last_error(_T("tkv_flush"), _T("enum trie tree breaked"), -1);
		return bool_false;
	}

	return bool_true;
}

#ifdef XDK_SUPPORT_TEST
static int e_count = 0;
static int v_count = 0;
static int k_count = 0;

static bool_t _check_tdb_node(variant_t key, object_t val, void* pv)
{
	t_tk_ctx* pobj = (t_tk_ctx*)pv;
	link_t_ptr nlk;
	tchar_t str[NUM_LEN + 1];
	int len;
	vword_t mask;

	len = variant_to_string(key, str, NUM_LEN);

	nlk = get_trie_node(pobj->trie, str, len);
	if (nlk)
	{
		mask = get_trie_node_delta(nlk);
		if (mask & TKV_MASK_PERSIST)
		{
			e_count++;
			_tprintf(_T("%s: value exists\n"), str);
		}
		else
		{
			v_count++;
			_tprintf(_T("%s: value persist\n"), str);
		}
	}
	else
	{
		k_count++;
		_tprintf(_T("%s: key not exist\n"), str);
	}

	return 1;
}

void test_tkv()
{
	int k, n, i, len;
	tchar_t kid[NUM_LEN + 1] = { 0 };

	t_tk_t tkv = tkv_create(_T('.'));
	t_db_t hkb = tdb_create(_T("."), _T("TrieDB"), 1);

	tkv_bind(tkv, hkb, 1);

	variant_t key = variant_alloc(VV_STRING_UTF8);
	object_t val = object_alloc();
	
	for (i = 0; i < 10000; i++)
	{
		xsprintf(kid, _T("key%d"), i);
		variant_from_string(key, kid, -1);
		object_set_variant(val, key);

		n = i;
		len = 0;
		do{
			k = n % 10;
			kid[len] = k + _T('0');
			kid[len + 1] = _T('.');
			len += 2;
			n /= 10;
		} while (n);

		if (len)
		{
			len--;
			kid[len] = _T('\0');
		}
		xsnrev(kid, len);

		tkv_write(tkv, kid, val);
	}

	variant_free(key);

	object_free(val);

	t_tk_ctx* pobj = (t_tk_ctx*)tkv;

	tdb_enum(hkb, _check_tdb_node, (void*)pobj);
	_tprintf(_T("persisted:%d, cached:%d, losted:%d\n"), e_count, v_count, k_count);

	tkv_flush(tkv);

	tdb_enum(hkb, _check_tdb_node, (void*)pobj);
	_tprintf(_T("persisted:%d, cached:%d, losted:%d\n"), e_count, v_count, k_count);

	tkv_destroy(tkv);
	tkv = NULL;

	tdb_destroy(hkb);
	hkb = NULL;
}

#endif