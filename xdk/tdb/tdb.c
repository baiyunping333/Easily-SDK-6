/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc tdb document

	@module	tdb.c | implement file

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

#include "tdb.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"

typedef struct _t_db_ctx
{
	t_db_hdr hdr;

	link_t_ptr ind_table;
	link_t_ptr dat_table;

	link_t_ptr tree;

	tchar_t dpath[PATH_LEN + 1];
	tchar_t dname[KEY_LEN + 1];

}t_db_ctx;

t_db_t tdb_create(const tchar_t* dpath, const tchar_t* dname, dword_t dmode)
{
	t_db_ctx* pobj = NULL;
	tchar_t fpath[PATH_LEN + 1] = { 0 };

	TRY_CATCH;

	pobj = (t_db_ctx*)xmem_alloc(sizeof(t_db_ctx));

	xsncpy(pobj->dpath, dpath, PATH_LEN);
	xsncpy(pobj->dname, dname, KEY_LEN);

	xsprintf(fpath, _T("%s/%s.ind"), pobj->dpath, pobj->dname);
	pobj->ind_table = create_file_table(fpath, BLOCK_SIZE_4096, dmode);
	if (!(pobj->ind_table))
	{
		raise_user_error(_T("tdb_create"), fpath);
	}

	xsprintf(fpath, _T("%s/%s.dat"), pobj->dpath, pobj->dname);
	pobj->dat_table = create_file_table(fpath, BLOCK_SIZE_512, dmode);
	if (!(pobj->dat_table))
	{
		raise_user_error(_T("tdb_create"), fpath);
	}
	
	pobj->tree = create_bplus_file_table(pobj->ind_table, pobj->dat_table);
	if (!(pobj->tree))
	{
		raise_user_error(_T("tdb_create"), _T("create bplus tree failed"));
	}

	END_CATCH;

	pobj->hdr.tag = T_OBJ_DB;
	return &(pobj->hdr);

ONERROR:
	XDK_TRACE_LAST;

	if (pobj)
	{
		if (pobj->ind_table)
			destroy_file_table(pobj->ind_table);
		if (pobj->dat_table)
			destroy_file_table(pobj->dat_table);
		if (pobj->tree)
			destroy_bplus_tree(pobj->tree);

		xmem_free(pobj);
	}

	return NULL;
}

void tdb_destroy(t_db_t hdb)
{
	t_db_ctx* pobj = (t_db_ctx*)hdb;

	XDK_ASSERT(hdb && hdb->tag == T_OBJ_DB);

	destroy_bplus_tree(pobj->tree);

	destroy_file_table(pobj->ind_table);

	destroy_file_table(pobj->dat_table);

	xmem_free(pobj);
}

bool_t tdb_save(t_db_t hdb, variant_t key, object_t val)
{
	t_db_ctx* pobj = (t_db_ctx*)hdb;

	XDK_ASSERT(hdb && hdb->tag == T_OBJ_DB);

	return insert_bplus_entity(pobj->tree, key, val);
}

bool_t tdb_load(t_db_t hdb, variant_t key, object_t val)
{
	t_db_ctx* pobj = (t_db_ctx*)hdb;

	XDK_ASSERT(hdb && hdb->tag == T_OBJ_DB);

	return find_bplus_entity(pobj->tree, key, val);
}

bool_t tdb_clean(t_db_t hdb, variant_t key)
{
	t_db_ctx* pobj = (t_db_ctx*)hdb;

	XDK_ASSERT(hdb && hdb->tag == T_OBJ_DB);

	return delete_bplus_entity(pobj->tree, key);
}

void tdb_enum(t_db_t hdb, ENUM_BPLUSTREE_ENTITY pf, void* param)
{
	t_db_ctx* pobj = (t_db_ctx*)hdb;

	XDK_ASSERT(hdb && hdb->tag == T_OBJ_DB);

	enum_bplus_entity(pobj->tree, pf, param);
}
