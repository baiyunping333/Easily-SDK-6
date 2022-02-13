/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc hkv document

	@module	hkv.c | implement file

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

#include "hkv.h"
#include "tdb.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"

typedef struct _t_hk_ctx
{
	t_hk_hdr hdr;

	link_t_ptr dict;
	bool_t laze;
	t_db_t tdb;
}t_hk_ctx;


t_hk_t hkv_create()
{
	t_hk_ctx* pobj;

	pobj = (t_hk_ctx*)xmem_alloc(sizeof(t_hk_ctx));

	pobj->dict = create_dict_table();
	pobj->hdr.tag = T_OBJ_HK;

	return &(pobj->hdr);
}

void hkv_destroy(t_hk_t hkv)
{
	t_hk_ctx* pobj = (t_hk_ctx*)hkv;

	XDK_ASSERT(hkv && hkv->tag == T_OBJ_HK);

	destroy_dict_table(pobj->dict);

	xmem_free(pobj);
}

bool_t hkv_write(t_hk_t hkv, variant_t key, object_t val)
{
	t_hk_ctx* pobj = (t_hk_ctx*)hkv;
	link_t_ptr ent;
	vword_t mask;

	XDK_ASSERT(hkv && hkv->tag == T_OBJ_HK);

	TRY_CATCH;

	ent = write_dict_entity(pobj->dict, key, NULL);

	if (pobj->tdb && !pobj->laze)
	{
		if (!tdb_save(pobj->tdb, key, val))
		{
			raise_user_error(_T("hkv_write"), _T("tdb_save"));
		}

		mask = get_dict_entity_delta(ent);
		mask |= HKV_MASK_PERSIST;
		set_dict_entity_delta(ent, mask);
	}
	else
	{
		set_dict_entity_val(ent, val);

		mask = get_dict_entity_delta(ent);
		mask &= (~HKV_MASK_PERSIST);
		set_dict_entity_delta(ent, mask);
	}

	END_CATCH;

	return bool_true;

ONERROR:
	XDK_TRACE_LAST;
	
	return bool_false;
}

bool_t hkv_update(t_hk_t hkv, variant_t key, object_t val)
{
	t_hk_ctx* pobj = (t_hk_ctx*)hkv;
	link_t_ptr ent;
	vword_t mask;

	XDK_ASSERT(hkv && hkv->tag == T_OBJ_HK);

	TRY_CATCH;

	ent = get_dict_entity(pobj->dict, key);

	if (!ent && pobj->tdb)
	{
		if (!tdb_clean(pobj->tdb, key))
		{
			raise_user_error(_T("hkv_update"), _T("tdb_clean"));
		}
	}

	if (!ent)
	{
		raise_user_error(_T("hkv_update"), _T("not exists"));
	}

	if (pobj->tdb && !pobj->laze)
	{
		if (!tdb_save(pobj->tdb, key, val))
		{
			raise_user_error(_T("hkv_update"), _T("tdb_save"));
		}

		mask = get_dict_entity_delta(ent);
		mask |= HKV_MASK_PERSIST;
		set_dict_entity_delta(ent, mask);
	}
	else
	{
		set_dict_entity_val(ent, val);

		mask = get_dict_entity_delta(ent);
		mask |= HKV_MASK_PERSIST;
		set_dict_entity_delta(ent, mask);
	}

	END_CATCH;

	return bool_true;

ONERROR:
	XDK_TRACE_LAST;

	return bool_false;
}

bool_t hkv_read(t_hk_t hkv, variant_t key, object_t val)
{
	t_hk_ctx* pobj = (t_hk_ctx*)hkv;
	link_t_ptr ent;
	vword_t mask;

	XDK_ASSERT(hkv && hkv->tag == T_OBJ_HK);

	TRY_CATCH;

	ent = get_dict_entity(pobj->dict, key);

	if (!ent && pobj->tdb)
	{
		if (!tdb_load(pobj->tdb, key, val))
		{
			raise_user_error(_T("hkv_read"), _T("tdb_load"));
		}

		ent = write_dict_entity(pobj->dict, key, NULL);

		mask = get_dict_entity_delta(ent);
		mask |= HKV_MASK_PERSIST;
		set_trie_node_delta(ent, mask);

		CLN_CATCH;
		return bool_true;
	}

	if (!ent)
	{
		raise_user_error(_T("tkv_read"), _T("not exists"));
	}

	mask = get_dict_entity_delta(ent);

	if (pobj->tdb && (mask & HKV_MASK_PERSIST))
	{
		if (!tdb_load(pobj->tdb, key, val))
		{
			raise_user_error(_T("tkv_read"), _T("get_trie_node"));
		}
	}
	else
	{
		get_dict_entity_val(ent, val);
	}

	END_CATCH;

	return bool_true;

ONERROR:
	XDK_TRACE_LAST;

	return bool_false;
}

bool_t hkv_delete(t_hk_t hkv, variant_t key)
{
	t_hk_ctx* pobj = (t_hk_ctx*)hkv;

	XDK_ASSERT(hkv && hkv->tag == T_OBJ_HK);

	TRY_CATCH;

	if (pobj->tdb)
	{
		if (!tdb_clean(pobj->tdb, key))
		{
			raise_user_error(_T("hkv_delete"), _T("tdb_clean"));
		}
	}

	delete_dict_entity(pobj->dict, key);

	END_CATCH;

	return bool_true;

ONERROR:
	XDK_TRACE_LAST;

	return bool_false;
}

void hkv_bind(t_hk_t hkv, t_db_t tdb, bool_t laze)
{
	t_hk_ctx* pobj = (t_hk_ctx*)hkv;

	pobj->tdb = tdb;
	pobj->laze = (pobj->tdb) ? laze : 0;
}

static bool_t _load_tdb_node(variant_t key, object_t val, void* p)
{
	t_hk_ctx* pobj = (t_hk_ctx*)p;
	link_t_ptr ent;
	vword_t mask;

	ent = write_dict_entity(pobj->dict, key, NULL);

	mask = get_dict_entity_delta(ent);
	mask |= HKV_MASK_PERSIST;
	set_dict_entity_delta(ent, mask);

	return 1;
}

bool_t hkv_load(t_hk_t hkv)
{
	t_hk_ctx* pobj = (t_hk_ctx*)hkv;

	XDK_ASSERT(hkv && hkv->tag == T_OBJ_HK);

	tdb_enum(pobj->tdb, _load_tdb_node, (void*)pobj);

	return bool_true;
}

static bool_t _save_tdb_node(variant_t key, link_t_ptr ent, void* p)
{
	t_hk_ctx* pobj = (t_hk_ctx*)p;
	vword_t mask;
	variant_t kh = NULL;
	object_t ob;

	mask = get_dict_entity_delta(ent);

	if (mask & HKV_MASK_PERSIST)
		return bool_true;

	ob = get_dict_entity_val_ptr(ent);

	if (!tdb_save(pobj->tdb, key, ob))
		return bool_false;

	mask |= HKV_MASK_PERSIST;
	set_dict_entity_delta(ent, mask);

	return bool_true;;
}

bool_t hkv_flush(t_hk_t hkv)
{
	t_hk_ctx* pobj = (t_hk_ctx*)hkv;
	link_t_ptr ent;

	XDK_ASSERT(hkv && hkv->tag == T_OBJ_HK);

	ent = enum_dict_entity(pobj->dict, _save_tdb_node, (void*)pobj);

	if (ent)
	{
		set_last_error(_T("hkv_flush"), _T("enum dict table breaked"), -1);
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
	link_t_ptr dict = (link_t_ptr)pv;
	link_t_ptr ent;
	vword_t mask;
	tchar_t str[NUM_LEN + 1];

	variant_to_string(key, str, NUM_LEN);

	ent = get_dict_entity(dict, key);
	if (ent)
	{
		mask = get_dict_entity_delta(ent);
		if (mask & HKV_MASK_PERSIST)
		{
			e_count++;
			_tprintf(_T("%s: value persist\n"), str);
		}
		else
		{
			v_count++;
			_tprintf(_T("%s: value exists\n"), str);
		}
	}
	else
	{
		k_count++;
		_tprintf(_T("%s: value losted\n"), str);
	}

	return 1;
}

void test_hkv()
{
	t_db_t tdb = tdb_create(_T("."), _T("HashDB"), 1);
	t_hk_t hkv = hkv_create();
	
	hkv_bind(hkv, tdb, 1);

	int i;
	tchar_t kid[NUM_LEN + 1] = { 0 };
	variant_t key = variant_alloc(VV_STRING_UTF8);
	object_t val = object_alloc();
	
	for (i = 0; i < 100; i++)
	{
		xsprintf(kid, _T("key%d"), i);
		variant_from_string(key, kid, -1);

		object_set_variant(val, key);

		hkv_write(hkv, key, val);
	}

	variant_free(key);
	object_free(val);

	t_hk_ctx* pobj = (t_hk_ctx*)hkv;

	tdb_enum(tdb, _check_tdb_node, (void*)pobj->dict);
	_tprintf(_T("persisted:%d, cached:%d, losted:%d\n"), e_count, v_count, k_count);

	hkv_flush(hkv);

	tdb_enum(tdb, _check_tdb_node, (void*)pobj->dict);
	_tprintf(_T("persisted:%d, cached:%d, losted:%d\n"), e_count, v_count, k_count);

	hkv_destroy(hkv);
	hkv = NULL;

	tdb_destroy(tdb);
	tdb = NULL;
}

#endif