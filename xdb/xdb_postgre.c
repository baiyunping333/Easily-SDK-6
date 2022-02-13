/***********************************************************************
	Easily xdb postgres

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, China ZheJiang HangZhou JianDe, Mail: powersuite@hotmaol.com

	@doc xdb oci document

	@module	xdb_postgres.c | xdb postgres implement file

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

#include "xdbpro.h"

#include <libpq-fe.h>


#ifdef _OS_WINDOWS
#pragma comment(lib,"libpq.lib")
#endif

#define SQL_BREAK   _T(";")

#define _BOOLOID 16
#define _INT8OID 20
#define _INT2OID 21
#define _INT4OID 23
#define _NUMERICOID 1700
#define _FLOAT4OID 700
#define _FLOAT8OID 701
#define _ABSTIMEOID 702
#define _RELTIMEOID 703
#define _DATEOID 1082
#define _TIMEOID 1083
#define _TIMETZOID 1266
#define _TIMESTAMPOID 1114
#define _TIMESTAMPTZOID 1184
#define _OIDOID 2278
#define _BYTEAOID 17
#define _REGPROCOID 24
#define _XIDOID 28
#define _CIDOID 29
#define _CHAROID 18
#define _NAMEOID 19
#define _TEXTOID 25
#define _VARCHAROID 1043

typedef struct _xdb_postgres_context{
	handle_head head;

	int chs;
	PGconn* ctx;
	PGresult* res;

	bool_t trans;
	int rows;
	tchar_t err_code[NUM_LEN + 1];
	tchar_t err_text[ERR_LEN + 1];
}xdb_postgres_context;


static void dbtodt(Oid type, tchar_t* dt)
{
	switch (type) {
	case _BOOLOID:
		xscpy(dt, ATTR_DATA_TYPE_BOOLEAN);
		break;
	case _INT8OID:
		xscpy(dt, ATTR_DATA_TYPE_INTEGER);
		break;
	case _INT2OID:
	case _INT4OID:
	case _OIDOID:
	case _REGPROCOID:
	case _XIDOID:
	case _CIDOID:
		xscpy(dt, ATTR_DATA_TYPE_INTEGER);
		break;
	case _NUMERICOID:
	case _FLOAT4OID:
	case _FLOAT8OID:
		xscpy(dt, ATTR_DATA_TYPE_NUMERIC);
		break;
	case _ABSTIMEOID:
	case _RELTIMEOID:
	case _DATEOID:
		xscpy(dt, ATTR_DATA_TYPE_DATE);
		break;
	case _TIMEOID:
	case _TIMETZOID:
		xscpy(dt, ATTR_DATA_TYPE_DATETIME);
		break;
	case _TIMESTAMPOID:
	case _TIMESTAMPTZOID:
		xscpy(dt, ATTR_DATA_TYPE_DATETIME);
		break;
	case _BYTEAOID:
		xscpy(dt, ATTR_DATA_TYPE_BINARY);
		break;
	default:
		xscpy(dt, ATTR_DATA_TYPE_STRING);
		break;
	}
}

static int sqltolen(Oid type, int size, int* prec)
{
	if (prec) *prec = 0;

	switch (type) {
	case _BOOLOID:
		return 1;
	case _INT8OID:
		return 0;
	case _INT2OID:
	case _INT4OID:
	case _OIDOID:
	case _REGPROCOID:
	case _XIDOID:
	case _CIDOID:
		return 0;
	case _NUMERICOID:
		if (prec) *prec = (size - 4) % 65536;
		return (size - 4) / 65536;
	case _FLOAT4OID:
	case _FLOAT8OID:
		return 0;
	case _ABSTIMEOID:
	case _RELTIMEOID:
	case _DATEOID:
		return 0;
	case _TIMEOID:
	case _TIMETZOID:
		return 0;
	case _TIMESTAMPOID:
	case _TIMESTAMPTZOID:
		return 0;
	case _BYTEAOID:
		return 0;
	default:
		return (size < 0) ? 0 : (size - 4);
	}
}

static int execok(PGresult* res)
{
	switch (PQresultStatus(res))
	{
	case PGRES_EMPTY_QUERY:
	case PGRES_COMMAND_OK:
	case PGRES_TUPLES_OK:
		return XDB_SUCCEED;
	case PGRES_COPY_OUT:
	case PGRES_COPY_IN:
	case PGRES_COPY_BOTH:
		return XDB_PENDING;
	default:
		return XDB_FAILED;
	}
}

static int split_semi(const tchar_t* token, int len)
{
	int tklen = 0, total = 0;
	bool_t glt = 0;
	const tchar_t* tkcur = token;

	if (len < 0)
		len = xslen(token);

	if (!len)
		return 0;

	while (*tkcur != _T(';') && *tkcur != _T('\0') && total < len)
	{
		if (*tkcur == _T('\'') || *tkcur == _T('\"'))
		{
			if (glt)
				glt = 0;
			else
				glt = 1;
		}

		tklen++;
		tkcur++;
		total++;

		if (glt)
		{
			while (*tkcur == _T(';'))
			{
				tklen++;
				tkcur++;
				total++;
			}
		}
	}

	return total;
}

static void _raise_ctx_error(PGconn* ctx)
{
    tchar_t errcode[NUM_LEN + 1] = {0};
    tchar_t errtext[ERR_LEN + 1] = {0};
    
    const char* str;
    int len;
    
	str = PQerrorMessage(ctx);
    len = a_xslen(str);
    
#ifdef _UNICODE
	utf8_to_ucs((byte_t*)str, len, errtext, ERR_LEN);
#else
	utf8_to_mbs((byte_t*)str, len, errtext, ERR_LEN);
#endif

	xsprintf(errcode, _T("%d"), -1);
    
    raise_user_error(errcode, errtext);
}

static void _raise_stm_error(PGconn* ctx)
{
    tchar_t err_code[NUM_LEN + 1] = { 0 };
    tchar_t err_text[ERR_LEN + 1] = { 0 };
    
    const char* str;
    int len;
    
	str = PQerrorMessage(ctx);
    len = a_xslen(str);

#ifdef _UNICODE
    utf8_to_ucs((byte_t*)str, len, err_text, ERR_LEN);
#else
    utf8_to_mbs((byte_t*)str, len, err_text, ERR_LEN);
#endif
    
    xsprintf(err_code, _T("%d"), -1);
    
    raise_user_error(err_code, err_text);
}

static void _db_reset(xdb_postgres_context* pdb)
{
	xscpy(pdb->err_code, _T(""));
	xscpy(pdb->err_text, _T(""));

	pdb->rows = 0;
	pdb->trans = 0;
	pdb->res = NULL;
}

static void _db_tran(xdb_postgres_context* pdb)
{
	char sql[MIN_SQL_LEN] = { 0 };
	int len;

#ifdef _UNICODE
	len = ucs_to_utf8(_T("BEGIN;"), -1, (byte_t*)sql, MIN_SQL_LEN);
#else
	len = mbs_to_utf8(_T("BEGIN;"), -1, (byte_t*)sql, MIN_SQL_LEN);
#endif
	sql[len] = '\0';

	PQclear(PQexec(pdb->ctx, sql));
	
	pdb->trans = 1;
}

static void _db_commit(xdb_postgres_context* pdb)
{
	char sql[MIN_SQL_LEN] = { 0 };
	int len;

	if (!pdb->trans)
		return;

#ifdef _UNICODE
	len = ucs_to_utf8(_T("COMMIT;"), -1, (byte_t*)sql, 1024);
#else
	len = mbs_to_utf8(_T("COMMIT;"), -1, (byte_t*)sql, 1024);
#endif
	sql[len] = '\0';

	PQclear(PQexec(pdb->ctx, sql));

	pdb->trans = 0;
}

static void _db_rollback(xdb_postgres_context* pdb)
{
	char sql[MIN_SQL_LEN] = { 0 };
	int len;

	if (!pdb->trans)
		return;

#ifdef _UNICODE
	len = ucs_to_utf8(_T("ROLLBACK;"), -1, (byte_t*)sql, MIN_SQL_LEN);
#else
	len = mbs_to_utf8(_T("ROLLBACK;"), -1, (byte_t*)sql, MIN_SQL_LEN);
#endif
	sql[len] = '\0';

	PQclear(PQexec(pdb->ctx, sql));
    
	pdb->trans = 0;
}

bool_t STDCALL db_parse_dsn(const tchar_t* dsnfile, tchar_t* srv_buf, int srv_len, tchar_t* dbn_buf, int dbn_len, tchar_t* uid_buf, int uid_len, tchar_t* pwd_buf, int pwd_len)
{
	LINKPTR d_ptr = NULL;

	TRY_CATCH;

	d_ptr = create_proper_doc();
	if (!load_proper_from_ini_file(d_ptr, NULL, dsnfile))
	{
		raise_user_error(_T("-1"), _T("parse dsn file failed"));
	}

	read_proper(d_ptr, _T("POSTGRE"), -1, DSN_SERVER, -1, srv_buf, srv_len);
	read_proper(d_ptr, _T("POSTGRE"), -1, DSN_DATABASE, -1, dbn_buf, dbn_len);
	read_proper(d_ptr, _T("POSTGRE"), -1, DSN_UID, -1, uid_buf, uid_len);
	read_proper(d_ptr, _T("POSTGRE"), -1, DSN_PWD, -1, pwd_buf, pwd_len);

	destroy_proper_doc(d_ptr);
	d_ptr = NULL;

	END_CATCH;

	return 1;

ONERROR:

	if (d_ptr)
		destroy_proper_doc(d_ptr);

	return 0;
}

xdb_t STDCALL db_open_dsn(const tchar_t* dsnfile)
{
	tchar_t drv[MAX_SQL_TOKEN + 1] = { 0 };
	tchar_t srv[MAX_SQL_TOKEN + 1] = { 0 };
	tchar_t dbn[MAX_SQL_TOKEN + 1] = { 0 };
	tchar_t uid[MAX_SQL_TOKEN + 1] = { 0 };
	tchar_t pwd[MAX_SQL_TOKEN + 1] = { 0 };

	if (!db_parse_dsn(dsnfile, srv, MAX_SQL_TOKEN, dbn, MAX_SQL_TOKEN, uid, MAX_SQL_TOKEN, pwd, MAX_SQL_TOKEN))
		return NULL;

	return db_open(srv, dbn, uid, pwd);
}

xdb_t STDCALL db_open(const tchar_t* srv, const tchar_t* dbn, const tchar_t* uid, const tchar_t* pwd)
{
	xdb_postgres_context* pdb = NULL;
	PGconn *ctx = NULL;
	PGresult *res = NULL;

	char sdrv[MAX_SQL_TOKEN + 1] = { 0 };
	char ssrv[MAX_SQL_TOKEN + 1] = { 0 };
	char sdbn[MAX_SQL_TOKEN + 1] = { 0 };
	char suid[MAX_SQL_TOKEN + 1] = { 0 };
	char spwd[MAX_SQL_TOKEN + 1] = { 0 };

	TRY_CATCH;

#ifdef _UNICODE
    ucs_to_utf8(srv, -1, (byte_t*)ssrv, MAX_SQL_TOKEN);
    ucs_to_utf8(dbn, -1, (byte_t*)sdbn, MAX_SQL_TOKEN);
    ucs_to_utf8(uid, -1, (byte_t*)suid, MAX_SQL_TOKEN);
    ucs_to_utf8(pwd, -1, (byte_t*)spwd, MAX_SQL_TOKEN);
#else
    mbs_to_utf8(srv, -1, (byte_t*)ssrv, MAX_SQL_TOKEN);
    mbs_to_utf8(dbn, -1, (byte_t*)sdbn, MAX_SQL_TOKEN);
    mbs_to_utf8(uid, -1, (byte_t*)suid, MAX_SQL_TOKEN);
    mbs_to_utf8(pwd, -1, (byte_t*)spwd, MAX_SQL_TOKEN);
#endif
    
	ctx = PQsetdbLogin(ssrv, NULL, NULL, NULL, sdbn, suid, spwd);
	if (!ctx)
	{
		raise_user_error(_T("-1"), _T("Alloc context handle failed"));
	}

	if(PQsetClientEncoding(ctx, "UTF8") < 0)
	{
		_raise_ctx_error(ctx);
	}
	PQclear(res);
	res = NULL;

	res = PQexec(ctx, "SET DATESTYLE TO 'ISO'");
	PQclear(res);

	pdb = (xdb_postgres_context*)xmem_alloc(sizeof(xdb_postgres_context));
	pdb->head.tag = _DB_POSTGRE;

	pdb->ctx = ctx;

	END_CATCH;

	return (xdb_t)pdb;

ONERROR:

	if (ctx)
		PQfinish(ctx);

	return NULL;
}

void STDCALL db_close(xdb_t db)
{
    xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	
	XDK_ASSERT(pdb != NULL);

	if (pdb->ctx)
	{
		PQfinish(pdb->ctx);
	}
	
	xmem_free(pdb);
}


bool_t STDCALL db_datetime(xdb_t db, int diff, tchar_t* sz_time)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;

	PGresult *res = NULL;

    char sqlstr[MID_SQL_LEN] = {0};
	int n;
	char* token;

	XDK_ASSERT(pdb != NULL);

	TRY_CATCH;

	if (diff > 0)
		a_xsprintf(sqlstr, "select now() + interval '%d day' as DT", diff);
	else if (diff < 0)
		a_xsprintf(sqlstr, "select now() - interval '%d day' as DT", -diff);
	else
		a_xscpy(sqlstr, "select now() as DT");

	_db_reset(pdb);

	res = PQexec(pdb->ctx, sqlstr);

	if (execok(res) != XDB_SUCCEED)
	{
		_raise_ctx_error(pdb->ctx);
	}
    
	if (PQntuples(res) != 1)
	{
		_raise_ctx_error(pdb->ctx);
	}

	token = PQgetvalue(res, 0, 0);
	n = PQgetlength(res, 0, 0);
    
#ifdef _UNICODE
    n = utf8_to_ucs((byte_t*)token, n, sz_time, DATE_LEN);
#else
    n = utf8_to_mbs((byte_t*)token, n, sz_time, DATE_LEN);
#endif
	sz_time[n] = _T('\0');

	pdb->rows = 1;
    
	PQclear(res);
	res = NULL;

	END_CATCH;

	return 1;

ONERROR:

	if(res) PQclear(res);

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

	return 0;
}

bool_t STDCALL db_exec(xdb_t db, const tchar_t* sqlstr, int sqllen)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	PGresult *res = NULL;

    int row, rows;
    char* d_sql = NULL;
    int d_len;
    
	tchar_t *tkpre, *tkcur;
	int tklen, total;
	bool_t uni;

	if (sqllen < 0)
		sqllen = xslen(sqlstr);

	if (!sqllen)
	{
		return 1;
	}

	TRY_CATCH;

	XDK_ASSERT(pdb != NULL);
	
	_db_reset(pdb);

	_db_tran(pdb);

	rows = 0;
	total = 0;
	tkcur = (tchar_t*)sqlstr;
	while (*tkcur != _T('\0'))
	{
		tklen = split_line(tkcur, sqllen);

		tkpre = tkcur;
		tkcur += tklen;
		sqllen -= tklen;
		uni = 0;

		while (*tkcur == _T(' ') || *tkcur == _T('\t') || *tkcur == _T('\n') || *tkcur == _T('\r'))
		{
			if (*tkcur == _T('\r'))
				uni = 1;

			tkcur++;
			sqllen--;
		}

		if (!tklen)
		{
			continue;
		}
        
#ifdef _UNICODE
        d_len = ucs_to_utf8(tkpre, tklen, NULL, MAX_LONG);
#else
        d_len = mbs_to_utf8(tkpre, tklen, NULL, MAX_LONG);
#endif
        
        d_sql = (char*)xmem_alloc(d_len + 1);
        
#ifdef _UNICODE
        d_len = ucs_to_utf8(tkpre, tklen, (byte_t*)d_sql, d_len);
#else
        d_len = mbs_to_utf8(tkpre, tklen, (byte_t*)d_sql, d_len);
#endif
        
		res = PQexec(pdb->ctx, d_sql);

		if (execok(res) != XDB_SUCCEED)
        {
			_raise_ctx_error(pdb->ctx);
        }

        xmem_free(d_sql);
        d_sql = NULL;
        
		row = a_xstol(PQcmdTuples(res));

		PQclear(res);
		res = NULL;

		if (uni && row != 1)
		{
			raise_user_error(_T("-1"), ERR_TEXT_ROWCHANGED);
		}
       
		rows += (int)row;
	}

	_db_commit(pdb);

	pdb->rows = rows;

	END_CATCH;

	return 1;

ONERROR:

    if(d_sql)
        xmem_free(d_sql);
    
	if (res)
	{
		_db_rollback(pdb);

		PQclear(res);
	}

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

	return 0;
}

bool_t STDCALL db_update(xdb_t db, LINKPTR grid)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	PGresult *res = NULL;
    int row, rows;
    
	tchar_t *sqlstr;
	int sqllen;
    
    char* d_sql = NULL;
    int d_len;

	LINKPTR rlk;
    dword_t rs;

	XDK_ASSERT(pdb != NULL);

	TRY_CATCH;

	_db_reset(pdb);

	_db_tran(pdb);

	rows = 0;
	rlk = get_next_row(grid,LINK_FIRST);
	while(rlk)
	{
		sqllen = 0;
		rs = get_row_state(rlk);
		if(rs == dsDelete)
		{
			sqllen = format_row_delete_sql(grid, rlk, NULL, MAX_LONG);
			if(sqllen > 0)
			{
				sqlstr = (tchar_t*)xmem_alloc((sqllen + 1) * sizeof(tchar_t));
				format_row_delete_sql(grid,rlk,sqlstr,sqllen);
			}
		}else if(rs == dsNewDirty)
		{
			sqllen = format_row_insert_sql(grid, rlk, NULL, MAX_LONG);
			if(sqllen > 0)
			{
				sqlstr = (tchar_t*)xmem_alloc((sqllen + 1) * sizeof(tchar_t));
				format_row_insert_sql(grid,rlk,sqlstr,sqllen);
			}
		}else if(rs == dsDirty)
		{
			sqllen = format_row_update_sql(grid, rlk, NULL, MAX_LONG);
			if(sqllen > 0)
			{
				sqlstr = (tchar_t*)xmem_alloc((sqllen + 1) * sizeof(tchar_t));
				format_row_update_sql(grid,rlk,sqlstr,sqllen);
			}
		}
		else
		{
			sqllen = 0;
		}

		if (!sqllen)
		{
			rlk = get_next_row(grid, rlk);
			continue;
		}

#ifdef _UNICODE
        d_len = ucs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#else
        d_len = mbs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#endif
        
        d_sql = (char*)xmem_alloc(d_len + 1);
        
#ifdef _UNICODE
        d_len = ucs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#else
        d_len = mbs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#endif
        
        xmem_free(sqlstr);
        sqlstr = NULL;
        
		res = PQexec(pdb->ctx, d_sql);
		if (execok(res) != XDB_SUCCEED)
		{
			_raise_ctx_error(pdb->ctx);
		}

		xmem_free(d_sql);
		d_sql = NULL;

		row = a_xstol(PQcmdTuples(res));

		PQclear(res);
		res = NULL;
        
		if (row != 1)
		{
			raise_user_error(_T("-1"), ERR_TEXT_ROWCHANGED);
		}
		else
		{
			rows += (int)row;
		}
        
		rlk = get_next_row(grid,rlk);
	}
	
	_db_commit(pdb);

	pdb->rows = rows;

	END_CATCH;

	return 1;

ONERROR:

	if (sqlstr)
		xmem_free(sqlstr);
    
    if(d_sql)
        xmem_free(d_sql);

	if (res)
	{
		_db_rollback(pdb);

		PQclear(res);
	}

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

	return 0;
}

int STDCALL _db_fetch_row(xdb_postgres_context* pdb, LINKPTR grid)
{
	char *fname, *fvalue;
	Oid ftype;
	int n, col, cols, row, rows;

	LINKPTR clk,rlk;
	
	tchar_t colname[MAX_SQL_NAME] = { 0 };
	tchar_t coltype[RES_LEN + 1] = { 0 };
	int len = 0;
	xdate_t dt = { 0 };
    
    tchar_t* d_str = NULL;
    int d_len;

	XDK_ASSERT(pdb->res != NULL);

    cols = PQnfields(pdb->res);
  
	rows = PQntuples(pdb->res);

	if (rows < 0) return XDB_FAILED;

	for (row = 0; row < rows;row ++)
	{
		rlk = insert_row(grid, LINK_LAST);
		set_row_state(rlk, dsClean);

		for (col = 0; col < cols; col++)
		{
			fname = PQfname(pdb->res, col);
			n = a_xslen(fname);
#ifdef _UNICODE
			len = utf8_to_ucs((byte_t*)(fname), n, colname, MAX_SQL_NAME);
#else
			len = utf8_to_mbs((byte_t*)(fname), n, colname, MAX_SQL_NAME);
#endif
			colname[len] = _T('\0');

			clk = get_col(grid, colname);
			if (!clk)
			{
				continue;
			}

			ftype = PQftype(pdb->res, col);
			dbtodt(ftype, coltype);

			fvalue = PQgetvalue(pdb->res, row, col);
			n = a_xslen(fvalue);

#ifdef _UNICODE
			d_len = utf8_to_ucs((byte_t*)fvalue, n, NULL, MAX_LONG);
			d_str = xsalloc(d_len + 1);
			d_len = utf8_to_ucs((byte_t*)fvalue, n, d_str, d_len);

#else
			d_len = utf8_to_mbs((byte_t*)fvalue, n, NULL, MAX_LONG);
			d_str = xsalloc(d_len + 1);
			d_len = utf8_to_mbs((byte_t*)fvalue, n, d_str, d_len);
#endif

			set_cell_text(rlk, clk, d_str, d_len);

			xsfree(d_str);
			d_str = NULL;
		}
	}
   
	return XDB_SUCCEED;
}

bool_t STDCALL db_fetch(xdb_t db, LINKPTR grid)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	
	PGresult *res = NULL;
	int rt;

	tchar_t* sqlstr = NULL;
	int sqllen;
    
    char* d_sql = NULL;
    int d_len;

	XDK_ASSERT(pdb != NULL);

	TRY_CATCH;

	_db_reset(pdb);

	clear_grid_rowset(grid);

	sqllen = format_grid_select_sql(grid, NULL, MAX_LONG);
	if (sqllen <= 0)
	{
		raise_user_error(_T("-1"), _T("Empty sql statement"));
	}

	sqlstr = (tchar_t*)xmem_alloc((sqllen + 1) * sizeof(tchar_t));
	format_grid_select_sql(grid,sqlstr,sqllen);

#ifdef _UNICODE
    d_len = ucs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#else
    d_len = mbs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#endif
    
    d_sql = (char*)xmem_alloc(d_len + 1);
    
#ifdef _UNICODE
    d_len = ucs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#else
    d_len = mbs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#endif
    
    xmem_free(sqlstr);
    sqlstr = NULL;
    
	res = PQexec(pdb->ctx, d_sql);

	if (execok(res) != XDB_SUCCEED)
	{
		_raise_ctx_error(pdb->ctx);
	}
    
    xmem_free(d_sql);
    d_sql = NULL;
	
	rt = XDB_PENDING;
	while (rt == XDB_PENDING)
	{
		pdb->res = res;
		rt = _db_fetch_row(pdb, grid);
	}

	if (rt == XDB_FAILED)
	{
		_raise_ctx_error(pdb->ctx);
	}

	PQclear(res);
    res = NULL;
	pdb->res = NULL;

	pdb->rows = get_row_count(grid);

	END_CATCH;

	return 1;

ONERROR:
	if (sqlstr)
		xmem_free(sqlstr);
    
    if(d_sql)
        xmem_free(d_sql);

    if(res)
		PQclear(res);

	pdb->res = NULL;

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

	return 0;
}

bool_t STDCALL db_select(xdb_t db, LINKPTR grid, const tchar_t* sqlstr)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	PGresult *res = NULL;
	char* fname;
	Oid ftype;
	int fsize;
	int fprec;

	LINKPTR clk;
    int sqllen, rt;
	int i,len,cols = 0;
	tchar_t coltype[MAX_SQL_NAME] = { 0 };
	tchar_t colname[MAX_SQL_NAME] = { 0 };

    char* d_sql = NULL;
    int d_len;
    
	XDK_ASSERT(pdb != NULL);

	TRY_CATCH;
	
	_db_reset(pdb);

	clear_grid_rowset(grid);
	clear_grid_colset(grid);

	if (is_null(sqlstr))
	{
		raise_user_error(_T("-1"), _T("Empty sql statement"));
	}
    sqllen = xslen(sqlstr);
    
#ifdef _UNICODE
    d_len = ucs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#else
    d_len = mbs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#endif
    
    d_sql = (char*)xmem_alloc(d_len + 1);
    
#ifdef _UNICODE
    d_len = ucs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#else
    d_len = mbs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#endif
    
	res = PQexec(pdb->ctx, d_sql);

	if (execok(res) != XDB_SUCCEED)
	{
		_raise_ctx_error(pdb->ctx);
	}
    
    xmem_free(d_sql);
    d_sql = NULL;
    
	cols = PQnfields(res);
    
	for (i = 0; i < cols; i++)
	{
		clk = insert_col(grid, LINK_LAST);

		fname = PQfname(res, i);
		len = a_xslen(fname);
#ifdef _UNICODE
		len = utf8_to_ucs((byte_t*)(fname), len, colname, MAX_SQL_NAME);
#else
		len = utf8_to_mbs((byte_t*)(fname), len, colname, MAX_SQL_NAME);
#endif
		colname[len] = _T('\0');

        set_col_name(clk, colname);
        set_col_title(clk, colname);
        
		ftype = PQftype(res, i);
        dbtodt(ftype, coltype);
        set_col_data_type(clk, coltype);
        
		fsize = PQfmod(res, i);
		fsize = sqltolen(ftype, fsize, &fprec);
		if (fsize)
		{
			set_col_data_len(clk, fsize);
			if (fprec)
				set_col_data_dig(clk, fprec);
		}
	}

	rt = XDB_PENDING;
	while (rt == XDB_PENDING)
	{
		pdb->res = res;
		rt = _db_fetch_row(pdb, grid);
	}

	if (rt == XDB_FAILED)
	{
		_raise_ctx_error(pdb->ctx);
	}

	PQclear(res);
	res = NULL;
	pdb->res = NULL;

	pdb->rows = get_row_count(grid);

	END_CATCH;

	return 1;

ONERROR:
    
    if(d_sql)
        xmem_free(d_sql);

	if (res)
		PQclear(res);

	pdb->res = NULL;

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

	return 0;
}

bool_t STDCALL db_schema(xdb_t db, LINKPTR grid, const tchar_t* sqlstr)
{
    xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	PGresult *res = NULL;
	char* fname;
	Oid ftype;
	int fsize, fprec;

    LINKPTR clk;
    int sqllen;
    int i,len,cols = 0;
    tchar_t coltype[MAX_SQL_NAME] = { 0 };
    tchar_t colname[MAX_SQL_NAME] = { 0 };
    
	char* d_sql = NULL;
	int d_len;
    
    XDK_ASSERT(pdb != NULL);
    
    TRY_CATCH;
    
    _db_reset(pdb);
    
    clear_grid_rowset(grid);
    clear_grid_colset(grid);
    
    if (is_null(sqlstr))
    {
        raise_user_error(_T("-1"), _T("Empty sql statement"));
    }
    sqllen = xslen(sqlstr);
    
#ifdef _UNICODE
	d_len = ucs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#else
	d_len = mbs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#endif
	d_sql = (char*)xmem_alloc(d_len + 1);
#ifdef _UNICODE
	d_len = ucs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#else
	d_len = mbs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#endif

	res = PQexec(pdb->ctx, d_sql);

	if (execok(res) != XDB_SUCCEED)
	{
		_raise_ctx_error(pdb->ctx);
	}
    
	xmem_free(d_sql);
	d_sql = NULL;
    
	cols = PQnfields(res);
    
    for (i = 0; i < cols; i++)
    {
		clk = insert_col(grid, LINK_LAST);

		fname = PQfname(res, i);
		len = a_xslen(fname);
#ifdef _UNICODE
		len = utf8_to_ucs((byte_t*)(fname), len, colname, MAX_SQL_NAME);
#else
		len = utf8_to_mbs((byte_t*)(fname), len, colname, MAX_SQL_NAME);
#endif
		colname[len] = _T('\0');

		set_col_name(clk, colname);
		set_col_title(clk, colname);

		ftype = PQftype(res, i);
		dbtodt(ftype, coltype);
		set_col_data_type(clk, coltype);

		fsize = PQfmod(res, i);
		fsize = sqltolen(ftype, fsize, &fprec);
		if (fsize)
		{
			set_col_data_len(clk, fsize);
			if (fprec)
				set_col_data_dig(clk, fprec);
		}
    }
    
	PQclear(res);
	res = NULL;
    
    pdb->rows = get_row_count(grid);
    
    END_CATCH;
    
    return 1;
    
ONERROR:
    
	if (d_sql)
		xmem_free(d_sql);

	if (res)
		PQclear(res);
    
    get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);
    
    return 0;
}

static int _db_call_argv(xdb_postgres_context* pdb, const tchar_t* procname, const tchar_t* fmt, va_list* parg)
{
	PGresult *res = NULL;
	Oid* poid = NULL;
    int* plen = NULL;
    char** pbuf = NULL;
    int* pfmt = NULL;
    int i, ind;
	char* pval;
	int ret;
    
    tchar_t* token;
    tchar_t* ptr_str;
    int* ptr_int;
    double* ptr_double;

	char* d_sql = NULL;
	int d_len;
    
    TRY_CATCH;
    
    pdb->rows = 0;
    
    ind = 0;
    token = (tchar_t*)fmt;
    while (token && *token)
    {
        if (*token == '%')
            ind++;
        
        token++;
    }
    
#ifdef _UNICODE
	d_len = ucs_to_utf8(procname, -1, NULL, MAX_LONG);
#else
	d_len = mbs_to_utf8(procname, -1, NULL, MAX_LONG);
#endif
    d_len += a_xslen("select ()") + ind * 2;
    d_sql = (char*)xmem_alloc(d_len + 1);
    
    a_xsprintf(d_sql, "select %S(", procname);

    for (i = 0; i < ind; i++)
    {
        a_xsappend(d_sql, "$%d,", i);
    }

	d_len = a_xslen(d_sql);
	if (d_sql[d_len - 1] == '(')
		d_len++;

	d_sql[d_len - 1] = ')';
	d_sql[d_len] = '\0';

	poid = (Oid*)xmem_alloc((ind)* sizeof(Oid));
    plen = (int*)xmem_alloc((ind) * sizeof(int));
    pbuf = (char**)xmem_alloc((ind) * sizeof(char*));
    pfmt = (int*)xmem_alloc((ind) * sizeof(int));

    ind = 0;
    token = (tchar_t*)fmt;
    while (token && *token)
    {
        while (*token && *token != _T('%'))
            token++;
        
        if (!*token)
            break;
        
        if (*token == _T('%'))
            token++;
        
		if (*token == _T('-') || *token == _T('+'))
        {
            token++;
        }
        
        if (*token >= _T('0') && *token <= _T('9'))
            plen[ind] = xstol(token);
        
        while (*token >= _T('0') && *token <= _T('9'))
            token++;
        
        if (*token == _T('.'))
        {
            token++;
            while (*token >= _T('0') && *token <= _T('9'))
                token++;
        }
        
        switch (*token)
        {
            case _T('s'):
                ptr_str = va_arg(*parg, tchar_t*);
				if (!plen[ind])
				{
#ifdef _UNICODE
					plen[ind] = ucs_to_utf8(ptr_str, -1, NULL, MAX_LONG);
#else
					plen[ind] = mbs_to_utf8(ptr_str, -1, NULL, MAX_LONG);
#endif
				}

				if (plen[ind])
				{
					pbuf[ind] = (char*)xmem_alloc(plen[ind] + 1);

#ifdef _UNICODE
					ucs_to_utf8(ptr_str, -1, (byte_t*)(pbuf[ind]), plen[ind]);
#else
					mbs_to_utf8(ptr_str, -1, (byte_t*)(pbuf[ind]), plen[ind]);
#endif
				}
				else
				{
					pbuf[ind] = NULL;
				}

				poid[ind] = 0;
				pfmt[ind] = 0;
                break;
            case _T('d'):
                ptr_int = va_arg(*parg, int*);
                plen[ind] = sizeof(int);
                pbuf[ind] = (char*)xmem_alloc(plen[ind]);
                
                xmem_copy(pbuf[ind], ptr_int, sizeof(int));

				poid[ind] = _INT4OID;
				pfmt[ind] = 1;
                break;
            case _T('f'):
                ptr_double = va_arg(*parg, double*);
                plen[ind] = sizeof(double);
                pbuf[ind] = (char*)xmem_alloc(plen[ind]);
                
                xmem_copy(pbuf[ind], ptr_double, sizeof(double));
                
				poid[ind] = _NUMERICOID;
				pfmt[ind] = 1;
                break;
        }
        
        ind ++;
    }
    
	res = PQexec(pdb->ctx, d_sql);

	xmem_free(d_sql);
	d_sql = NULL;

	if (execok(res) != XDB_SUCCEED)
	{
		_raise_ctx_error(pdb->ctx);
	}

	if (PQntuples(res) > 0)
	{
		pval = PQgetvalue(res, 0, 0);
	}
	else
	{
		pval = NULL;
	}

	ret = a_xstol(pval);

	if (pbuf)
	{
		for (i = 0; i < ind; i++)
		{
			xmem_free(pbuf[i]);
		}
	}
    
    xmem_free(pbuf);
    xmem_free(poid);
    xmem_free(plen);
    xmem_free(pfmt);
    
	PQclear(res);
	res = NULL;
    
    END_CATCH;
    
    return ret;
    
ONERROR:
    
    if (d_sql)
        xmem_free(d_sql);
    
    if(pbuf)
    {
        for (i = 0; i < ind; i++)
        {
            xmem_free(pbuf[i]);
        }
    }
    
	xmem_free(pbuf);
	xmem_free(poid);
	xmem_free(plen);
	xmem_free(pfmt);
    
	if (res)
		PQclear(res);

    get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);
    
    return -1;
}

int db_call_argv(xdb_t db, const tchar_t* procname, const tchar_t* fmt, ...)
{
    xdb_postgres_context* pdb = (xdb_postgres_context*)db;
    
    va_list arg;
    int rt;
    
    va_start(arg,fmt);
	rt = _db_call_argv(pdb, procname, fmt, &arg);
    va_end(arg);
    
    return rt;
}

bool_t STDCALL db_call_func(xdb_t db, LINKPTR func)
{
    xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	PGresult *res = NULL;
	Oid* poid = NULL;
	int* plen = NULL;
	char** pbuf = NULL;
	int* pfmt = NULL;

    int i, ind;
	char* pval;
	int ret;
    
    char* d_sql = NULL;
	int d_len;
    
    LINKPTR flk;
    
    XDK_ASSERT(db && db->tag == _DB_POSTGRE);
    
    TRY_CATCH;
    
    _db_reset(pdb);
    
    ind = get_func_param_count(func);
    
	poid = (Oid*)xmem_alloc((ind)* sizeof(Oid));
	plen = (int*)xmem_alloc((ind)* sizeof(int));
	pbuf = (char**)xmem_alloc((ind)* sizeof(char*));
	pfmt = (int*)xmem_alloc((ind)* sizeof(int));
    
    ind = 0;
    flk = get_func_next_param(func, LINK_FIRST);
    while (flk)
    {
        if (compare_text(get_func_data_type_ptr(flk), -1, ATTR_DATA_TYPE_INTEGER, -1, 0) == 0)
        {
            plen[ind] = sizeof(int);
            pbuf[ind] = (char*)xmem_alloc(plen[ind]);
            *(int*)(pbuf[ind]) = get_func_param_integer(flk);
			poid[ind] = _INT4OID;
			pfmt[ind] = 1;
        }
        else if (compare_text(get_func_data_type_ptr(flk), -1, ATTR_DATA_TYPE_NUMERIC, -1, 0) == 0)
        {
            plen[ind] = sizeof(double);
            pbuf[ind] = (char*)xmem_alloc(plen[ind]);
            *(double*)(pbuf[ind]) = get_func_param_numeric(flk);
			poid[ind] = _FLOAT8OID;
			pfmt[ind] = 1;
        }
        else
        {
			plen[ind] = get_func_param_data_len(flk);

			if (!plen[ind])
			{
#ifdef _UNICODE
				plen[ind] = ucs_to_utf8(get_func_param_text_ptr(flk), -1, NULL, MAX_LONG);
#else
				plen[ind] = mbs_to_utf8(get_func_param_text_ptr(flk), -1, NULL, MAX_LONG);
#endif
			}

			if (plen[ind])
			{
				pbuf[ind] = (char*)xmem_alloc(plen[ind] + 1);
#ifdef _UNICODE
				ucs_to_utf8(get_func_param_text_ptr(flk), -1, (byte_t*)(pbuf[ind]), plen[ind]);
#else
				mbs_to_utf8(get_func_param_text_ptr(flk), -1, (byte_t*)(pbuf[ind]), plen[ind]);
#endif
			}else
			{
				pbuf[ind] = NULL;
			}

			poid[ind] = _VARCHAROID;
			pfmt[ind] = 0;
        }
        
        ind ++;
        flk = get_func_next_param(func, flk);
    }

#ifdef _UNICODE
	d_len = ucs_to_utf8(get_func_name_ptr(func), -1, NULL, MAX_LONG);
#else
	d_len = mbs_to_utf8(get_func_name_ptr(func), -1, NULL, MAX_LONG);
#endif

	d_len += a_xslen("select public.()") + ind * 4;
	d_sql = (char*)xmem_alloc(d_len + 1);

#ifdef _UNICODE
	a_xsprintf(d_sql, "select public.%S(", get_func_name_ptr(func));
#else
	a_xsprintf(d_sql, "select public.%s(", get_func_name_ptr(func));
#endif

	for (i = 0; i < ind; i++)
	{
		a_xsappend(d_sql, "$%d,", i+1);
	}

	d_len = a_xslen(d_sql);
	if (d_sql[d_len - 1] == '(')
		d_len++;

	d_sql[d_len - 1] = ')';
	d_len++;
	d_sql[d_len - 1] = ';';
	d_sql[d_len] = '\0';
    
	res = PQexecParams(pdb->ctx, d_sql, ind, poid, pbuf, plen, pfmt, 0);

	xmem_free(d_sql);
	d_sql = NULL;

	if (execok(res) != XDB_SUCCEED)
	{
		_raise_ctx_error(pdb->ctx);
	}

	if (PQntuples(res) > 0)
	{
		pval = PQgetvalue(res, 0, 0);
	}
	else
	{
		pval = NULL;
	}

	ret = a_xstol(pval);
	set_func_return_integer(func, ret);

	if (pbuf)
	{
		for (i = 0; i < ind; i++)
		{
			xmem_free(pbuf[i]);
		}
	}

	xmem_free(pbuf);
	xmem_free(poid);
	xmem_free(plen);
	xmem_free(pfmt);

	PQclear(res);
	res = NULL;
    
    END_CATCH;
    
    return 1;
    
ONERROR:
    
    if (d_sql)
        xmem_free(d_sql);

    if(pbuf)
    {
        for (i = 0; i < ind; i++)
        {
            xmem_free(pbuf[i]);
        }
    }
    
	xmem_free(pbuf);
	xmem_free(poid);
	xmem_free(plen);
	xmem_free(pfmt);
    
	if (res)
		PQclear(res);
    
	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

    return 0;
}

bool_t STDCALL _db_prepare(xdb_postgres_context* pdb, const tchar_t* sqlstr)
{
	PGresult *res = NULL;

    int sqllen;
	char* d_sql = NULL;
	int d_len;

	XDK_ASSERT(pdb != NULL);

	TRY_CATCH;

	_db_reset(pdb);

	if (is_null(sqlstr))
	{
		raise_user_error(_T("-1"), _T("Empty sql statement"));
	}
	sqllen = xslen(sqlstr);
    
#ifdef _UNICODE
	d_len = ucs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#else
	d_len = mbs_to_utf8(sqlstr, sqllen, NULL, MAX_LONG);
#endif
    
	d_sql = (char*)xmem_alloc(d_len + 1);
    
#ifdef _UNICODE
	d_len = ucs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#else
	d_len = mbs_to_utf8(sqlstr, sqllen, (byte_t*)d_sql, d_len);
#endif
    
	res = PQexec(pdb->ctx, d_sql);

	xmem_free(d_sql);
	d_sql = NULL;

	if (execok(res) != XDB_SUCCEED)
	{
		_raise_ctx_error(pdb->ctx);
	}

	pdb->rows = PQntuples(res);
	pdb->res = res;

	END_CATCH;

	return 1;

ONERROR:

	if (d_sql)
		xmem_free(d_sql);

	if (res)
		PQclear(res);

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

	return 0;
}

bool_t STDCALL db_export(xdb_t db, stream_t stream, const tchar_t* sqlstr)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	char* fname;
	int flen;
	char* fval;
	int vlen;

    int len, col, cols, row, rows;
    tchar_t datatype[MAX_SQL_NAME] = { 0 };
    tchar_t colname[MAX_SQL_NAME] = { 0 };

	string_t vs = NULL;

	tchar_t feed[3] = { TXT_ITEMFEED, TXT_LINEFEED, _T('\n') };

	tchar_t* sz_esc = NULL;
	int len_esc = 0;
    dword_t pos;

    tchar_t* d_str = NULL;
    int d_len;
    
	XDK_ASSERT(pdb != NULL);

	if (!pdb->res)
	{
		_db_prepare(pdb, sqlstr);
	}

	if (!stream)
	{
		return (pdb->res) ? 1 : 0;
	}

	TRY_CATCH;

	if (!pdb->res)
	{
		raise_user_error(_T("-1"), ERR_TEXT_INVALIDSTMT);
	}

	cols = PQnfields(pdb->res);

	stream_write_utfbom(stream, NULL);

	vs = string_alloc();

	for (col = 0; col < cols; col++)
	{
		fname = PQfname(pdb->res, col);
		flen = a_xslen(fname);

#ifdef _UNICODE
		len = utf8_to_ucs((byte_t*)(fname), flen, colname, MAX_SQL_NAME);
#else
		len = utf8_to_mbs((byte_t*)(fname), flen, colname, MAX_SQL_NAME);
#endif
		colname[len] = _T('\0');
        
		string_cat(vs, colname, -1);
		string_cat(vs, feed, 1);
	}
	string_cat(vs, feed + 1, 2);

	if (!stream_write_line(stream, vs, &pos))
	{
		raise_user_error(NULL, NULL);
	}

	string_empty(vs);

	rows = PQntuples(pdb->res);

	for (row = 0; row < rows;row++)
    {
		for (col = 0; col < cols; col++)
		{
			fval = PQgetvalue(pdb->res, row, col);
			vlen = PQgetlength(pdb->res, row, col);

			if (vlen)
			{
#ifdef _UNICODE
                d_len = utf8_to_ucs((byte_t*)fval, vlen, NULL, MAX_LONG);
#else
                d_len = utf8_to_mbs((byte_t*)fval, vlen, NULL, MAX_LONG);
#endif
                d_str = xsalloc(d_len + 1);
#ifdef _UNICODE
                d_len = utf8_to_ucs((byte_t*)fval, vlen, d_str, d_len);
#else
                d_len = utf8_to_mbs((byte_t*)fval, vlen, d_str, d_len);
#endif

				len_esc = csv_char_encode(d_str, d_len, NULL, MAX_LONG);
				if (len_esc != d_len)
				{
					sz_esc = xsalloc(len_esc + 1);
					csv_char_encode(d_str, d_len, sz_esc, len_esc);

					string_cat(vs, sz_esc, len_esc);
					xsfree(sz_esc);
				}
				else
				{
					string_cat(vs, d_str, d_len);
				}
                
                xsfree(d_str);
                d_str = NULL;
			}
			string_cat(vs, feed, 1);
		}

		string_cat(vs, feed + 1, 2);

		if (!stream_write_line(stream, vs, &pos))
		{
			raise_user_error(NULL, NULL);
		}

		string_empty(vs);
	}
	
	string_empty(vs);

	if (!stream_write_line(stream, vs, &pos))
	{
		raise_user_error(NULL, NULL);
	}

	string_free(vs);
	vs = NULL;

	if (!stream_flush(stream))
	{
		raise_user_error(NULL, NULL);
	}
    
	PQclear(pdb->res);
	pdb->res = NULL;

	END_CATCH;

	return 1;

ONERROR:

    if(d_str)
        xsfree(d_str);
    
	if (vs)
		string_free(vs);

	if (pdb->res)
		PQclear(pdb->res);

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

	return 0;
}

bool_t STDCALL db_import(xdb_t db, stream_t stream, const tchar_t* table)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	PGresult *res = NULL;
	Oid* poid = NULL;
	int* plen = NULL;
	char** pbuf = NULL;
	int* pfmt = NULL;
    int i, cols;

	string_t vs = NULL;
	string_t vs_sql = NULL;
	const tchar_t* token;
    const tchar_t* tkpre;
	int tklen;
	int rows;
	dword_t dw;

	tchar_t* sz_esc = NULL;
	int len_esc = 0;
    
    char* d_sql = NULL;
    int d_len;

	XDK_ASSERT(pdb != NULL);

	XDK_ASSERT(stream != NULL);

	TRY_CATCH;

	_db_reset(pdb);

	//_db_tran(pdb);

	stream_read_utfbom(stream, NULL);

	vs = string_alloc();

	dw = 0;
	if (!stream_read_line(stream, vs, &dw))
	{
		raise_user_error(_T("-1"), _T("read stream failed"));
	}

	vs_sql = string_alloc();

	string_printf(vs_sql, _T("insert into %s ("), table);

	cols = 0;
	token = string_ptr(vs);
	while (*token != TXT_LINEFEED && *token != _T('\0'))
	{
		tklen = 0;
		while (*token != TXT_ITEMFEED && *token != TXT_LINEFEED && *token != _T('\0'))
		{
			token++;
			tklen++;
		}

		string_cat(vs_sql, token - tklen, tklen);
		string_cat(vs_sql, _T(","), 1);

		if (*token == TXT_ITEMFEED)
			token++;

		cols++;
	}

	if (!cols)
	{
		raise_user_error(_T("-1"), _T("empty sql statement"));
	}
    
	tklen = string_len(vs_sql);
	string_set_char(vs_sql, tklen - 1, _T(')'));

	string_cat(vs_sql, _T(" values ("), -1);

	for (i = 0; i < cols; i++)
	{
		string_append(vs_sql, _T("$%d,"), i+1);
	}

	tklen = string_len(vs_sql);
	string_set_char(vs_sql, tklen - 1, _T(')'));
	string_cat(vs_sql, _T(";"), 1);
	tklen++;

#ifdef _UNICODE
    d_len = ucs_to_utf8(string_ptr(vs_sql),string_len(vs_sql),NULL, MAX_LONG);
#else
    d_len = mbs_to_utf8(string_ptr(vs_sql),string_len(vs_sql),NULL, MAX_LONG);
#endif
    d_sql = (char*)xmem_alloc(d_len + 1);
#ifdef _UNICODE
    d_len = ucs_to_utf8(string_ptr(vs_sql),string_len(vs_sql),(byte_t*)d_sql, d_len);
#else
    d_len = mbs_to_utf8(string_ptr(vs_sql),string_len(vs_sql),(byte_t*)d_sql, d_len);
#endif
    
    string_free(vs_sql);
    vs_sql = NULL;
    
	poid = (Oid*)xmem_alloc((cols)* sizeof(Oid));
	plen = (int*)xmem_alloc((cols)* sizeof(int));
	pbuf = (char**)xmem_alloc((cols)* sizeof(char*));
	pfmt = (int*)xmem_alloc((cols)* sizeof(int));

	rows = 0;
	string_empty(vs);

	while (1)
	{
		string_empty(vs);
		dw = 0;
		if (!stream_read_line(stream, vs, &dw))
		{
			raise_user_error(_T("-1"), _T("stream read line failed"));
		}

		if (string_len(vs) == 0)
			break;

		i = 0;
		token = string_ptr(vs);
		while (*token != TXT_LINEFEED && *token != _T('\0'))
		{
			tklen = 0;
			while (*token != TXT_ITEMFEED && *token != TXT_LINEFEED && *token != _T('\0'))
			{
				token++;
				tklen++;
			}

            if(tklen)
            {
                tkpre = token - tklen;
                len_esc = csv_char_decode(tkpre, tklen, NULL, MAX_LONG);
                if (len_esc != tklen)
                {
                    sz_esc = xsalloc(len_esc + 1);
					csv_char_decode(tkpre, tklen, sz_esc, len_esc);
                    
#ifdef _UNICODE
                    plen[i] = ucs_to_utf8(sz_esc, len_esc, NULL, MAX_LONG);
#else
                    plen[i] = mbs_to_utf8(sz_esc, len_esc, NULL, MAX_LONG);
#endif
                    pbuf[i] = (char*)xmem_alloc(plen[i] + 1);
#ifdef _UNICODE
                    plen[i] = ucs_to_utf8(sz_esc, len_esc, (byte_t*)(pbuf[i]), plen[i]);
#else
                    plen[i] = mbs_to_utf8(sz_esc, len_esc, (byte_t*)(pbuf[i]), plen[i]);
#endif
                    
                }
                else
                {
#ifdef _UNICODE
                    plen[i] = ucs_to_utf8(tkpre, tklen, NULL, MAX_LONG);
#else
                    plen[i] = mbs_to_utf8(tkpre, tklen, NULL, MAX_LONG);
#endif
                    pbuf[i] = (char*)xmem_alloc(plen[i] + 1);
#ifdef _UNICODE
                    plen[i] = ucs_to_utf8(tkpre, tklen, (byte_t*)(pbuf[i]), plen[i]);
#else
                    plen[i] = mbs_to_utf8(tkpre, tklen, (byte_t*)(pbuf[i]), plen[i]);
#endif
                }
                
				pfmt[i] = 0;
				poid[i] = 0;
            }else
            {
                plen[i] = 0;
                pbuf[i] = NULL;
				pfmt[i] = 0;
				poid[i] = 0;
            }

			if (*token == TXT_ITEMFEED)
				token++;
            
			if (++i == cols)
				break;
		}

		while (i < cols)
		{
			plen[i] = 0;
			pbuf[i] = NULL;
			pfmt[i] = 0;
			poid[i] = 0;
            
			if (++i == cols)
				break;
		}
        
		res = PQexecParams(pdb->ctx, d_sql, cols, poid, pbuf, plen, pfmt, 0);
       
		if (execok(res) != XDB_SUCCEED)
		{
			_raise_ctx_error(pdb->ctx);
		}

		for (i = 0; i < cols; i++)
		{
            if(pbuf[i])
                xmem_free(pbuf[i]);
            pbuf[i] = NULL;
			plen[i] = 0;
            pfmt[i] = 0;
			poid[i] = 0;
		}

		rows += a_xstol(PQcmdTuples(res));
	}

	_db_commit(pdb);

	PQclear(res);
	res = NULL;

	if (pbuf)
	{
		for (i = 0; i < cols; i++)
		{
			xmem_free(pbuf[i]);
		}
	}

	xmem_free(pbuf);
	xmem_free(poid);
	xmem_free(plen);
	xmem_free(pfmt);

	string_free(vs);
	vs = NULL;

	pdb->rows = rows;

	END_CATCH;

	return 1;

ONERROR:

	if (res)
	{
		_db_rollback(pdb);

		PQclear(res);
	}

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

    if(d_sql)
        xmem_free(d_sql);
    
	if (vs_sql)
		string_free(vs_sql);

	if (vs)
		string_free(vs);

	if (pbuf)
	{
		for (i = 0; i < cols; i++)
		{
			xmem_free(pbuf[i]);
		}
	}

	xmem_free(pbuf);
	xmem_free(poid);
	xmem_free(plen);
	xmem_free(pfmt);

	return 0;
}

bool_t STDCALL db_batch(xdb_t db, stream_t stream)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;
	PGresult *res = NULL;
	int rows;

	string_t vs = NULL;
	string_t vs_sql = NULL;
	dword_t dw;
    
    const tchar_t *tkcur, *tkpre;
    int tklen;
    
    char* d_sql = NULL;
    int d_len;

	XDK_ASSERT(pdb != NULL);

	XDK_ASSERT(stream != NULL);

	TRY_CATCH;

	_db_reset(pdb);

	stream_read_utfbom(stream, NULL);

	vs = string_alloc();
	vs_sql = string_alloc();

	while (1)
	{
		string_empty(vs);
		dw = 0;
		if (!stream_read_line(stream, vs, &dw))
		{
			raise_user_error(_T("-1"), _T("stream read line failed"));
		}

		if (string_len(vs) == 0)
		{
			dw = 0;

			if (string_len(vs_sql))
				goto EXECUTE;
			else
				break;
		}

        tkcur = string_ptr(vs);
        tklen = string_len(vs);

		while (*tkcur == _T(' ') || *tkcur == _T('\t') || *tkcur == _T('\r') || *tkcur == _T('\n'))
		{
			tkcur++;
			tklen--;
		}
       
		if (*tkcur == _T('-') && *(tkcur + 1) == _T('-'))
		{
			continue;
		}

		tklen = split_semi(tkcur, tklen);

		tkpre = tkcur;
		tkcur += tklen;

		string_cat(vs_sql, tkpre, tklen);

		if (*tkcur != _T(';'))
		{
			continue;
		}

		string_empty(vs);

EXECUTE:
		string_cat(vs_sql, _T(";"), 1);
		tkpre = string_ptr(vs_sql);
		tklen = string_len(vs_sql);

#ifdef _UNICODE
		d_len = ucs_to_utf8(tkpre, tklen, NULL, MAX_LONG);
#else
		d_len = mbs_to_utf8(tkpre, tklen, NULL, MAX_LONG);
#endif

		d_sql = (char*)xmem_alloc(d_len + 1);

#ifdef _UNICODE
		d_len = ucs_to_utf8(tkpre, tklen, (byte_t*)d_sql, d_len);
#else
		d_len = mbs_to_utf8(tkpre, tklen, (byte_t*)d_sql, d_len);
#endif

		string_empty(vs_sql);

		res = PQexec(pdb->ctx, d_sql);
		
		xmem_free(d_sql);
		d_sql = NULL;

		if (execok(res) != XDB_SUCCEED)
		{
			_raise_ctx_error(pdb->ctx);
		}

		rows = a_xstol(PQcmdTuples(res));

		PQclear(res);
		res = NULL;

		pdb->rows += (int)rows;

		if (!dw)
			break;
	}

	string_free(vs);
	vs = NULL;

	string_free(vs_sql);
	vs_sql = NULL;

	END_CATCH;

	return 1;

ONERROR:

    if(d_sql)
         xmem_free(d_sql);
    
	if (vs)
		string_free(vs);

	if (vs_sql)
		string_free(vs_sql);

	if (res)
		PQclear(res);

	get_last_error(pdb->err_code, pdb->err_text, ERR_LEN);

	return 0;
}

int STDCALL db_rows(xdb_t db)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;

	XDK_ASSERT(pdb != NULL);

	return pdb->rows;
}

int STDCALL db_error(xdb_t db, tchar_t* buf, int max)
{
	xdb_postgres_context* pdb = (xdb_postgres_context*)db;

	XDK_ASSERT(pdb != NULL);

	max = (max < ERR_LEN) ? max : ERR_LEN;
	if (buf)
	{
		xsncpy(buf, pdb->err_text, max);
	}

	return -1;
}
