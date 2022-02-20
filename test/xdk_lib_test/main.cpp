
#include <xdk.h>

#ifdef _OS_WINDOWS
#include <conio.h>
#endif

void _error_level2()
{
	TRY_CATCH;

	raise_user_error(_T("_error_level2"), _T("level 2 error"));

	END_CATCH;
ONERROR:

	return;
}

void _error_level1()
{
	TRY_CATCH;

	_error_level2();

	raise_user_error(_T("_error_level1"), _T("level 1 error"));

	END_CATCH;
ONERROR:

	return;
}

void test_error()
{
	TRY_CATCH;

	_error_level1();

	raise_user_error(_T("_error_level0"), _T("level 0 error"));

	END_CATCH;
ONERROR:
	XDK_TRACE_LAST;
}

void test_mem()
{
	byte_t b[20] = { 0 };

	for (int i = 0; i< 10; i++)
	{
		b[i] = '0' + i;
	}

	xmem_move(b, 10, 10);
	xmem_zero(b, 10);

	xmem_move(b + 10, 10, -10);
	xmem_zero(b + 10, 10);
}

void test_conv()
{
	schar_t* mbs_token = "A多BC字节中文";
	wchar_t* ucs_token = L"A宽节BC中文";

	byte_t utf_buf[100] = {0};
	schar_t mbs_buf[100] = {0};
	wchar_t ucs_buf[100] = {0};
	int n, len;
	dword_t k;
	
	//setlocale(P_ALL, "zh_CN.UTF-8");
	n = wctomb(mbs_buf, *ucs_token);
	n = wctomb(mbs_buf, L'A');
	n = (int)mbstowcs(NULL, mbs_token, 0);
	n = strlen(mbs_token);
	len = mblen(mbs_token, n);
	n = (int)mbstowcs(ucs_buf, mbs_token, len);
	//setlocale(P_ALL, "");

	n = mbs_to_ucs(mbs_token, -1, NULL, 100);
	n = mbs_to_ucs(mbs_token, -1, ucs_buf, n);

	n = ucs_to_utf8(ucs_token, -1, NULL, 100);
	n = ucs_to_utf8(ucs_token, -1, utf_buf, n);

	k = a_xslen(mbs_token);
	n = utf8_to_mbs((byte_t*)mbs_token, k, NULL, 100);
	n = utf8_to_mbs((byte_t*)mbs_token, k, mbs_buf, n);
}

void test_utc()
{
	xdate_t dt;
	tchar_t sz_date[UTC_LEN + 1] = { 0 };

	get_utc_date(&dt);
	format_utctime(&dt, sz_date);

	xdate_t dt2;
	parse_datetime(&dt2, sz_date);

	int rt = compare_datetime(&dt, &dt2);
}

void test_times()
{
	xdate_t dt;
	tchar_t sz_date[UTC_LEN + 1] = { 0 };
	dword_t ms;

	ms = get_times();

	ms += 24 * 60 * 60;

	utc_date_from_times(&dt, ms);
	format_utctime(&dt, sz_date);
}

void test_stamp()
{
	xdate_t dt;
	tchar_t sz_date[UTC_LEN + 1] = { 0 };
	lword_t ms;
	dword_t m, s, k;

	ms = get_timestamp();
	m = ms / (1000 * 100);
	s = ms % (1000 * 100);
	k = m & 0x0FFFFFFF;

	utc_date_from_timestamp(&dt, ms);

	format_utctime(&dt, sz_date);

	_tprintf(_T("%s\n"), sz_date);

	ms = (lword_t)m * 100000 + (lword_t)s;

	utc_date_from_timestamp(&dt, ms);

	format_utctime(&dt, sz_date);

	_tprintf(_T("%s\n"), sz_date);
}

void test_func(int a, ...)
{
	va_list args;

	va_start(args, a);

	char c = (char)va_arg(args, int);

	int b = va_arg(args, int);

	va_end(args);
}

void test_printf()
{
	char tmp[100] = { 0 };
	int len = a_xsprintf(tmp, "%d '%Y-%m-%d %H:%i') as DT", -1);

	tchar_t buf[20] = { 0 };

	//sprintf(buf,  "%c",  'W');

	//test_func(10, _T('0'), 10);

	xsprintf(buf, _T("%s%"), _T("hello"));

	//printf(buf);
}

void test_money()
{
	tchar_t token[NUM_LEN] = { 0 };

	format_money_chs(10.01, 0, token, NUM_LEN);
}

void test_words()
{
	//const tchar_t* str = _T("这是ABC一段字体测试 文字");
	const tchar_t* str = _T("abcd,中文汉字，$￥");
	int n, total = 0;
	tchar_t pch[CHS_LEN + 1] = { 0 };

	int len = xslen(str);

	while (n = peek_word((str + total), pch))
	{
		_tprintf(_T("%s %d\n"), pch, n);
		total += n;
	}

	_tprintf(_T("len:%d total:%d\n"), len, total);
}

void test_intset()
{
	const tchar_t* str = _T("[1,2-5,7, 9-10, 12-20]");

	int n = parse_intset(str, -1, NULL, MAX_LONG);

	int* sa = (int*)xmem_alloc(n * sizeof(int));
	parse_intset(str, -1, sa, n);

	for (int i = 0; i < n; i++)
	{
		_tprintf(_T("%d\n"), sa[i]);
	}

	xmem_free(sa);
}

void test_nums()
{
	dword_t dl = 0xFFFFFFFF;
	dword_t dh = 0;
	lword_t ll = MAKELWORD(dl, dh) + 4096;

	dword_t h = GETHDWORD(ll);
	dword_t l = GETLDWORD(ll);

	XDK_ASSERT(dl == l && dh == h);
}

typedef struct _time_hint{
	int n_mon;
	int* p_mon;
	int n_week;
	int* p_week;
	int n_day;
	int* p_day;
	int n_hour;
	int* p_hour;
	int n_min;
	int* p_min;
	int n_sec;
	int* p_sec;
}time_hint;

void test_time_hint()
{
	time_hint th = { 0 };

	th.n_mon = parse_intset(_T("[1-12]"), -1, NULL, MAX_LONG);
	th.p_mon = (int*)xmem_alloc(th.n_mon * sizeof(int));
	parse_intset(_T("[1-12]"), -1, th.p_mon, th.n_mon);

	th.n_week = parse_intset(_T("[0,2,4]"), -1, NULL, MAX_LONG);
	th.p_week = (int*)xmem_alloc(th.n_week * sizeof(int));
	parse_intset(_T("[0,2,4]"), -1, th.p_week, th.n_week);

	th.n_day = parse_intset(_T("[1,2,3,4,5,6,7]"), -1, NULL, MAX_LONG);
	th.p_day = (int*)xmem_alloc(th.n_day * sizeof(int));
	parse_intset(_T("[1,2,3,4,5,6,7]"), -1, th.p_day, th.n_day);

	th.n_hour = parse_intset(_T("[10]"), -1, NULL, MAX_LONG);
	th.p_hour = (int*)xmem_alloc(th.n_hour * sizeof(int));
	parse_intset(_T("[10]"), -1, th.p_hour, th.n_hour);

	th.n_min = parse_intset(_T("[30]"), -1, NULL, MAX_LONG);
	th.p_min = (int*)xmem_alloc(th.n_min * sizeof(int));
	parse_intset(_T("[30]"), -1, th.p_min, th.n_min);

	th.n_sec = parse_intset(_T("[45]"), -1, NULL, MAX_LONG);
	th.p_sec = (int*)xmem_alloc(th.n_sec * sizeof(int));
	parse_intset(_T("[45]"), -1, th.p_sec, th.n_sec);

	xdate_t dt1 = { 0 }, dt2 = { 0 };
	parse_datetime(&dt1, _T("2020-01-01 00:00:00"));
	parse_datetime(&dt2, _T("2020-12-31 00:00:00"));

	tchar_t token[DATE_LEN] = { 0 };
	bool_t b = 0;
	int i;

	while (compare_datetime(&dt1, &dt2) < 0)
	{
		mak_loc_week(&dt1);

		b = 0;
		for (i = 0; i < th.n_mon; i++)
		{
			if (dt1.mon == th.p_mon[i])
			{
				b = 1;
				break;
			}
		}
		if (!b && th.n_mon)
		{
			plus_months(&dt1, 1);
			dt1.day = 1;
			dt1.hour = 0;
			dt1.min = 0;
			dt1.sec = 0;
			continue;
		}

		b = 0;
		for (i = 0; i < th.n_week; i++)
		{
			if (dt1.wday == th.p_week[i])
			{
				b = 1;
				break;
			}
		}
		if (!b && th.n_week)
		{
			plus_days(&dt1, 1);
			dt1.hour = 0;
			dt1.min = 0;
			dt1.sec = 0;
			continue;
		}

		b = 0;
		for (i = 0; i < th.n_day; i++)
		{
			if (dt1.day == th.p_day[i])
			{
				b = 1;
				break;
			}
		}
		if (!b && th.n_day)
		{
			plus_days(&dt1, 1);
			dt1.hour = 0;
			dt1.min = 0;
			dt1.sec = 0;
			continue;
		}

		b = 0;
		for (i = 0; i < th.n_hour; i++)
		{
			if (dt1.hour == th.p_hour[i])
			{
				b = 1;
				break;
			}
		}
		if (!b && th.n_hour)
		{
			plus_hours(&dt1, 1);
			dt1.min = 0;
			dt1.sec = 0;
			continue;
		}

		b = 0;
		for (i = 0; i < th.n_min; i++)
		{
			if (dt1.min == th.p_min[i])
			{
				b = 1;
				break;
			}
		}
		if (!b && th.n_min)
		{
			plus_minutes(&dt1, 1);
			dt1.sec = 0;
			continue;
		}

		b = 0;
		for (i = 0; i < th.n_sec; i++)
		{
			if (dt1.sec == th.p_sec[i])
			{
				b = 1;
				break;
			}
		}
		if (!b && th.n_sec)
		{
			plus_seconds(&dt1, 1);
			continue;
		}

		mak_loc_week(&dt1);
		format_datetime(&dt1, token);
		_tprintf(_T("%s W%d\n"), token, dt1.wday);

		plus_seconds(&dt1, 1);
	}

	xmem_free(th.p_mon);
	xmem_free(th.p_week);
	xmem_free(th.p_day);
	xmem_free(th.p_hour);
	xmem_free(th.p_min);
	xmem_free(th.p_sec);
}


void test_hash32()
{
	int i, j, k = 0, n = 100000;
	tchar_t kid[NUM_LEN + 1] = { 0 };

	variant_t key = variant_alloc(VV_STRING_UTF8);

	key32_t* pka = (key32_t*)xmem_alloc(sizeof(key32_t) * n);

	_tprintf(_T("hash32 test case:%d\n"), n);

	for (i = 0; i < n; i++)
	{
		xsprintf(kid, _T("key%d"), i);
		variant_from_string(key, kid, -1);

		variant_hash32(key, pka + i);

		for (j = i - 1; j >= 0; j--)
		{
			if (pka[j] == pka[i])
			{
				k++;
				_tprintf(_T("key%d collide with key%d\n"), i, j);
			}
		}
	}

	variant_free(key);
	//xmem_free(pka);

	_tprintf(_T("hash32 collide:%f percent\n"), (double)k / (double)n * 100.0);
}

void test_hash64()
{
	int i, j, k = 0, n = 1000000;
	tchar_t kid[NUM_LEN + 1] = { 0 };

	variant_t key = variant_alloc(VV_STRING_UTF8);

	key64_t* pka = (key64_t*)xmem_alloc(sizeof(key64_t) * n);

	_tprintf(_T("hash64 test case:%d\n"), n);

	for (i = 0; i < n; i++)
	{
		xsprintf(kid, _T("key%d"), i);
		variant_from_string(key, kid, -1);

		variant_hash64(key, pka + i);

		for (j = i - 1; j >= 0; j--)
		{
			if (pka[j] == pka[i])
			{
				k++;
				_tprintf(_T("key%d collide with key%d\n"), i, j);
			}
		}
	}

	variant_free(key);
	xmem_free(pka);

	_tprintf(_T("hash64 collide:%f percent\n"), (double)k / (double)n * 100.0);
}

void test_dict()
{
	link_t_ptr dict = create_dict_table();

	variant_t var = variant_alloc(VV_STRING_UTF8);

	object_t val = object_alloc();
	tchar_t str[100] = { 0 };
	int i;

	for (i = 0; i < 10000; i++)
	{
		xsprintf(str, _T("string%d"), i);
		variant_from_string(var, str, -1);
		object_set_variant(val, var);

		link_t_ptr ent = write_dict_entity(dict, var, val);

		bool_t rt = read_dict_entity(dict, var, val);
		if (!rt)
			_tprintf(_T("%s losted\n"), str);
		else
		{
			object_get_variant(val, var);
			xszero(str, 100);
			variant_to_string(var, str, 100);
			//_tprintf(_T("%s passed\n"), str);
		}
	}

	variant_free(var);
	object_free(val);

	destroy_dict_table(dict);

	_tprintf(_T("end\n"));
}

void test_acp()
{
#ifdef XDK_SUPPORT_ACP_TABLE
	save_gb2312_table(_T("gb2312.acp"));
	save_unicode_table(_T("unicode.acp"));
#endif
}

void test_hlp()
{
	const schar_t* a_str = "T汉字 拼F音a";
	const wchar_t* w_str = L"T汉字 拼F音a";

	schar_t a_hlp[10] = { 0 };
	int n = a_help_code(a_str, -1, NULL, 10);
	a_help_code(a_str, -1, a_hlp, 10);

	wchar_t w_hlp[10];
	n = w_help_code(w_str, -1, NULL, 10);
	w_help_code(w_str, -1, w_hlp, 10);
}

void test_mgc()
{
	visual_t mgc = mgc_create(MGC_DEVICE_BITMAP_TRUECOLOR32, MGC_PAPER_P6, 100, 800, SDPI);

	xcolor_t xc;
	parse_xcolor(&xc, GDI_ATTR_RGB_RED);

	xpoint_t pt;

	int i, j;

	for (i = 0;  i < 100; i++)
	{
		for (j = 0; j < 100; j++)
		{
			pt.x = i; pt.y = j;
			xc.r = (i == j)? 1 : 0;
			xc.g = 0;// j;
			xc.b = 0;// i + j;
			if (i == j)
			{
				//mgc_set_point(mgc, &xc, &pt, ROP_COPY);
			}
		}
	}

	xfont_t xf;
	default_xfont(&xf);
	xscpy(xf.color, GDI_ATTR_RGB_LIGHTWHITE);

	const tchar_t* str = _T("abcd,中文汉字，$￥");
	//const tchar_t* str = _T("abcd");
	//const tchar_t* str = _T("啊");
	
	tchar_t fs[16][5] = { _T("5"), _T("5.5"),_T("6.5"), _T("7.5"), _T("9"), _T("10.5"), _T("12"), _T("14"), _T("15"), _T("16"), _T("18"), _T("22"), _T("24"), _T("26"), _T("36"), _T("42") };
	xsize_t xs = { 0 };

	mgc_set_rop(mgc, ROP_COPY);

	pt.x = pt.y = 0;
	for (i = 0; i < 16; i++)
	{
		xscpy(xf.size, fs[i]);
		mgc_text_size(mgc, &xf, str, -1, &xs);

		pt.y += xs.h;
		mgc_text_out(mgc, &xf, &pt, str, -1);
	}

	dword_t n = mgc_save_bytes(mgc, NULL, MAX_LONG);
	byte_t* buf = (byte_t*)xmem_alloc(n);
	mgc_save_bytes(mgc, buf, n);

	mgc_destroy(mgc);

	FILE* fp = fopen("demo.bmp", "wb+");
	fwrite(buf, n, 1, fp);
	fclose(fp);

	xmem_free(buf);
}

void test_draw()
{
	visual_t mgc = mgc_create(MGC_DEVICE_BITMAP_TRUECOLOR32, MGC_PAPER_P6, 100, 800, SDPI);

	xcolor_t xc;
	parse_xcolor(&xc, GDI_ATTR_RGB_RED);

	int i, j;

	xfont_t xf;
	default_xfont(&xf);
	xscpy(xf.color, GDI_ATTR_RGB_LIGHTWHITE);
	xscpy(xf.size, _T("12"));
	xface_t xa;
	default_xface(&xa);
	xscpy(xa.text_align, GDI_ATTR_TEXT_ALIGN_NEAR);
	xscpy(xa.line_align, GDI_ATTR_TEXT_ALIGN_NEAR);
	xscpy(xa.text_wrap, GDI_ATTR_TEXT_WRAP_LINEBREAK);

	const tchar_t* str = _T("abcd,中文汉字，\n$￥");
	xsize_t xs = { 0 };

	mgc_set_rop(mgc, ROP_COPY);

	xrect_t xr;
	xr.x = 0;
	xr.y = 0;
	xr.w = 100;
	xr.h = 100;

	//mgc_text_rect(mgc, &xf, &xa, str, -1, &xr);

	mgc_draw_text(mgc, &xf, &xa, &xr, str, -1);

	xpen_t xp;
	default_xpen(&xp);
	xscpy(xp.size, _T("6"));
	xscpy(xp.color, GDI_ATTR_RGB_WHITE);
	xscpy(xp.style, GDI_ATTR_STROKE_STYLE_DOTTED);

	xpoint_t pt[5];
	pt[0].x = 50;
	pt[0].y = 100;
	pt[1].x = 80;
	pt[1].y = 50;
	pt[2].x = 150;
	pt[2].y = 100;
	pt[3].x = 80;
	pt[3].y = 150;
	pt[4].x = 50;
	pt[4].y = 100;

	//mgc_draw_polyline(mgc, &xp, pt, 5);
	mgc_draw_polygon(mgc, &xp, NULL, pt, 4);

	xscpy(xp.size, _T("4"));

	xscpy(xp.style, GDI_ATTR_STROKE_STYLE_SOLID);
	pt[0].x = 0;
	pt[0].y = 170;
	pt[1].x = 50;
	pt[1].y = 170;
	mgc_draw_line(mgc, &xp, &pt[0], &pt[1]);

	xscpy(xp.style, GDI_ATTR_STROKE_STYLE_DASHED);
	pt[0].x = 0;
	pt[0].y = 180;
	pt[1].x = 50;
	pt[1].y = 180;
	mgc_draw_line(mgc, &xp, &pt[0], &pt[1]);

	xr.x = 10;
	xr.y = 200;
	xr.w = 50;
	xr.h = 50;
	mgc_draw_rect(mgc, &xp, NULL, &xr);

	xscpy(xp.style, GDI_ATTR_STROKE_STYLE_SOLID);
	xr.x = 10;
	xr.y = 260;
	xr.w = 50;
	xr.h = 50;
	mgc_draw_rect(mgc, &xp, NULL, &xr);

	dword_t n = mgc_save_bytes(mgc, NULL, MAX_LONG);
	byte_t* buf = (byte_t*)xmem_alloc(n);
	mgc_save_bytes(mgc, buf, n);

	mgc_destroy(mgc);

	FILE* fp = fopen("draw.bmp", "wb+");
	fwrite(buf, n, 1, fp);
	fclose(fp);

	xmem_free(buf);
}

void test_printf_big5()
{
	FILE* fp = fopen("BG2UBG.KU", "rb");
	unsigned short c, u;

	FILE* fd = fopen("big.c", "w+");
	tchar_t hex[5];
	char cc[8], uc[8];
	wchar_t wc;

	while (!feof(fp))
	{
		fread(&c, sizeof(unsigned short), 1, fp);
		fread(&u, sizeof(unsigned short), 1, fp);

		gb2312_byte_to_ucs((byte_t*)&c, &wc);

		memset(hex, 0, 5);
		cc[0] = '0'; cc[1] = 'x';
		format_hexnum(c, hex, 4);
		cc[2] = (char)hex[0];
		cc[3] = (char)hex[1];
		cc[4] = (char)hex[2];
		cc[5] = (char)hex[3];
		cc[6] = ',';
		cc[7] = '\0';

		memset(hex, 0, 5);
		uc[0] = '0'; uc[1] = 'x';
		format_hexnum(wc, hex, 4);
		uc[2] = (char)hex[0];
		uc[3] = (char)hex[1];
		uc[4] = (char)hex[2];
		uc[5] = (char)hex[3];
		uc[6] = '\n';
		uc[7] = '\0';

		fwrite(cc, 1, 7, fd);
		fwrite(uc, 1, 7, fd);
	}

	fclose(fp);
	fclose(fd);
}

void test_ttf()
{
	FILE* fp = fopen("arial.ttf", "rb");

	fseek(fp, 0, SEEK_END);
	long int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	byte_t* buf = (byte_t*)xmem_alloc(size);
	fread(buf, 1, size, fp);
	fclose(fp);

	ttf_file_head_t ttf_file = { 0 };
	ttf_load_file_head(&ttf_file, buf, size);

	ttf_head_table_t ttf_head = { 0 };
	ttf_load_head_table(&ttf_file, &ttf_head, buf, size);
	int locaFormat = ttf_head.indexToLocFormat;
	ttf_clear_head_table(&ttf_head);

	ttf_name_table_t ttf_name = { 0 };
	ttf_load_name_table(&ttf_file, &ttf_name, buf, size);
	ttf_clear_name_table(&ttf_name);

	ttf_cmap_table_t ttf_cmap = { 0 };
	ttf_load_cmap_table(&ttf_file, &ttf_cmap, buf, size);
	int i;
	for (i = 0; i < ttf_cmap.numTables; i++)
	{

	}
	ttf_clear_cmap_table(&ttf_cmap);

	ttf_maxp_table_t ttf_maxp = { 0 };
	ttf_load_maxp_table(&ttf_file, &ttf_maxp, buf, size);
	int numGlyphs = ttf_maxp.numGlyphs;
	ttf_clear_maxp_table(&ttf_maxp);

	ttf_loca_table_t ttf_loca = { 0 };
	ttf_load_loca_table(&ttf_file, locaFormat, numGlyphs, &ttf_loca, buf, size);

	ttf_glyf_table_t* pglyf = (ttf_glyf_table_t*)xmem_alloc((numGlyphs + 1) * sizeof(ttf_glyf_table_t));
	ttf_load_glyf_table(&ttf_file, &ttf_loca, pglyf, numGlyphs, buf, size);
	for (i = 0; i <= numGlyphs; i++)
	{
		ttf_clear_glyf_table(&pglyf[i]);
	}
	xmem_free(pglyf);

	ttf_clear_loca_table(&ttf_loca);
	ttf_clear_file_head(&ttf_file);

	xmem_free(buf);
}

int main(int argc, char* argv[])
{
	xdk_process_init(XDK_APARTMENT_PROCESS);

	//test_error();

	//test_mem();

	//test_conv();

	//test_utc();

	//test_stamp();

	//test_nuid();

	//test_printf();

	//test_money();

	//test_vector();

	//test_matrix();

	//test_set();

	//test_map();

	//test_dict();

	//test_string_array();

	//test_integer_array();

	//test_numeric_array();

	//test_intset();

	//test_words();

	//test_nums();

	//test_time_hint();

	//test_variant();

	//test_message();

	//test_queue();

	//test_object();

	//test_ac_table();

	//test_dict_table();

	//test_lock_table();

	//test_file_table_alloc(_T("demo"), FILETABLE_SHARE);

	//test_file_table_write(_T("demo"), FILETABLE_SHARE | FILETABLE_DIRECT);

	//test_bplus_tree_none_table();

	//test_bplus_tree_file_table(_T("demo"), FILETABLE_SHARE);

	//test_tkv();

	//test_hash32();

	//test_hash64();

	//test_linear();

	//test_trie_tree();

	//test_tkv();

	//test_hkv();

	//aes_self_test(1);

	//arc4_self_test(1);

	//asn1_self_test(1);

	//base64_self_test(1);

	//ctr_drbg_self_test(1);

	//hmac_drbg_self_test(1);

	//des_self_test(1);

	//md2_self_test(1);

	//md4_self_test(1);

	//md5_self_test(1);

	//ripemd160_self_test(1);

	//sha1_self_test(1);

	//sha256_self_test(1);

	//sha512_self_test(1);

	//sm3_self_test(1);

	//test_hkdf(1);

	//timing_self_test(1);

	//mpi_self_test(1);

	//rsa_self_test(1);

	//rsa_test_parse(1);

	//ecp_self_test(1);

	//ecp_test_parse(1);

	//entropy_self_test(1);

	//ecdh_x25519_test(1);

	//ecdh_test(1);

	//chacha20_self_test(1);

	//poly1305_self_test(1);

	//chachapoly_self_test(1);

	//gcm_self_test(1);

	//dhm_self_test(1);

	//x509_self_test(1);

	//test_acp();

	//test_hlp();

	//test_mgc();

test_draw();

//test_ttf();

//test_printf_big5();

xdk_process_uninit();

#ifdef _OS_WINDOWS
	getch();
#endif

	return 0;
}

