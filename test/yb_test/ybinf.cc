
#include "ybinf.h"
#include "ybmac.h"
#include "ybcfg.h"


bool_t sign_in(yb_config_t* pcfg)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk = loca_sign();
	if (rlk)
	{
		raise_user_error(_T("sign_in"), _T("已签到"));
	}

	tchar_t inf_time[DATE_LEN] = { 0 };
	xdate_t dt;
	get_loc_date(&dt);
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("9001"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, pcfg->mdt_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, pcfg->ins_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, _T("system"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, _T("测试"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, pcfg->hos_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, pcfg->hos_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, _T("1"));

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("signIn"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("opter_no"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mac"));
	set_json_item_value(nlk, _T("053471"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("ip"));
	set_json_item_value(nlk, _T("053471"));

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	char in_buf[1024] = { 0 };
	format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/signInSignOutService/signIn"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("sign_in"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("sign_in"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("sign_in.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("sign_in"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	rlk = insert_row(grid_sign, LINK_LAST);

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));
	link_t_ptr clk_sign_time = get_col(grid_sign, _T("sign_in_time"));
	link_t_ptr clk_sign_flag = get_col(grid_sign, _T("sign_flag"));

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("signinoutb"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("sign_no"), -1, 1) == 0)
						{
							set_cell_text(rlk, clk_sign_no, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("sign_time"), -1, 1) == 0)
						{
							set_cell_text(rlk, clk_sign_time, ivalue, -1);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				set_cell_text(rlk, clk_sign_flag, _T("0"), -1);
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;
	
	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t sign_out(yb_config_t* pcfg)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk = loca_sign();
	if (rlk == NULL)
	{
		raise_user_error(_T("sign_out"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));
	link_t_ptr clk_sign_time = get_col(grid_sign, _T("sign_out_time"));
	link_t_ptr clk_sign_flag = get_col(grid_sign, _T("sign_flag"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk, clk_sign_no, sign_no, NUM_LEN);

	tchar_t inf_time[DATE_LEN] = { 0 };
	xdate_t dt;
	get_loc_date(&dt);
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("9002"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, pcfg->mdt_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, pcfg->ins_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, pcfg->hos_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, pcfg->hos_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("signOut"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("opter_no"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	char in_buf[1024] = { 0 };
	format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/signInSignOutService/signOut"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("sign_out"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("sign_out"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("sign_out.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("sign_out"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("signoutoutb"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("sign_time"), -1, 1) == 0)
						{
							set_cell_text(rlk, clk_sign_time, ivalue, -1);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				set_cell_text(rlk, clk_sign_flag, _T("1"), -1);
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t stmt_total(yb_config_t* pcfg, const tchar_t* insutype, const tchar_t* med_type)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk = loca_sign();
	if (rlk == NULL)
	{
		raise_user_error(_T("sign_out"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));
	link_t_ptr clk_sign_time = get_col(grid_sign, _T("sign_in_time"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk, clk_sign_no, sign_no, NUM_LEN);
	tchar_t sign_time[DATE_LEN] = { 0 };
	get_cell_text(rlk, clk_sign_time, sign_time, DATE_LEN);

	xdate_t dt;
	tchar_t stmt_begndate[DATE_LEN] = { 0 };
	tchar_t stmt_enddate[DATE_LEN] = { 0 };
	parse_date(&dt, sign_time);
	xsprintf(stmt_begndate, _T("%d-%02d-%02d 00:00:00"), dt.year, dt.mon, dt.day);
	xsprintf(stmt_enddate, _T("%d-%02d-%02d 23:59:59"), dt.year, dt.mon, dt.day);

	tchar_t medfee_sumamt[NUM_LEN] = { 0 };
	xsprintf(medfee_sumamt, _T("%.2f"), 0.0);
	tchar_t fund_pay_sumamt[NUM_LEN] = { 0 };
	xsprintf(fund_pay_sumamt, _T("%.2f"), 0.0);
	tchar_t acct_pay[NUM_LEN] = { 0 };
	xsprintf(acct_pay, _T("%.2f"), 0.0);
	tchar_t fixmedins_setl_cnt[NUM_LEN] = { 0 };
	xsprintf(fixmedins_setl_cnt, _T("%d"), 0);

	tchar_t inf_time[DATE_LEN] = { 0 };
	get_loc_date(&dt);
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("3201"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, pcfg->mdt_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, pcfg->ins_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, pcfg->hos_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, pcfg->hos_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种
	set_json_item_value(nlk, insutype);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("clr_type")); //清算类别
	set_json_item_value(nlk, med_type);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("setl_optins")); //结算经办机构
	set_json_item_value(nlk, pcfg->mdt_admvs);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("stmt_begndate")); //对账开始日期
	set_json_item_value(nlk, stmt_begndate);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("stmt_enddate")); //对账结束日期
	set_json_item_value(nlk, stmt_enddate);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, medfee_sumamt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fund_pay_sumamt")); //基金支付总额
	set_json_item_value(nlk, fund_pay_sumamt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_pay")); //个人账户支付金额
	set_json_item_value(nlk, acct_pay);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_setl_cnt")); //定点医药机构结算笔数
	set_json_item_value(nlk, fixmedins_setl_cnt);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/ybSettlementStmtService/stmtTotal"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("stmt_total"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("stmt_total"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("stmt_total.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("stmt_total"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_stmt_begndate = get_col(grid_sign, _T("stmt_begndate"));
	link_t_ptr clk_stmt_enddate = get_col(grid_sign, _T("stmt_enddate"));
	link_t_ptr clk_stmt_rslt = get_col(grid_sign, _T("stmt_rslt"));
	link_t_ptr clk_stmt_rslt_dscr = get_col(grid_sign, _T("stmt_rslt_dscr"));
	link_t_ptr clk_medfee_sumamt = get_col(grid_sign, _T("medfee_sumamt"));
	link_t_ptr clk_fund_pay_sumamt = get_col(grid_sign, _T("fund_pay_sumamt"));
	link_t_ptr clk_acct_pay = get_col(grid_sign, _T("acct_pay"));
	link_t_ptr clk_fixmedins_setl_cnt = get_col(grid_sign, _T("fixmedins_setl_cnt"));

	tchar_t out_msgid[NUM_LEN] = { 0 };
	tchar_t stmt_rslt[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("stmtinfo"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("stmt_rslt"), -1, 1) == 0)
						{
							set_cell_text(rlk, clk_stmt_rslt, ivalue, -1);
							xscpy(stmt_rslt, ivalue);
						}
						else if (compare_text(iname, -1, _T("stmt_rslt_dscr"), -1, 1) == 0)
						{
							set_cell_text(rlk, clk_stmt_rslt_dscr, ivalue, -1);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				set_cell_text(rlk, clk_stmt_begndate, stmt_begndate, -1);
				set_cell_text(rlk, clk_stmt_enddate, stmt_enddate, -1);
				set_cell_text(rlk, clk_medfee_sumamt, medfee_sumamt, -1);
				set_cell_text(rlk, clk_fund_pay_sumamt, fund_pay_sumamt, -1);
				set_cell_text(rlk, clk_acct_pay, acct_pay, -1);
				set_cell_text(rlk, clk_fixmedins_setl_cnt, fixmedins_setl_cnt, -1);
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t query_psninfo(yb_config_t* pcfg, link_t_ptr rlk_record)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("query_psninfo"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_cert_no = get_col(grid_record, _T("cert_no"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("1101"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, pcfg->mdt_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, pcfg->ins_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("MBS_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, pcfg->hos_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, pcfg->hos_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, CERT_TYPE_SFZ);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_cert_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("begntime"));
	set_json_item_value(nlk, begntime);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/fsiPsnInfoService/queryPsnInfo"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("query_psninfo"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("query_psninfo"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("query_psninfo.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("query_psninfo"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_psn_name = get_col(grid_record, _T("psn_name"));
	link_t_ptr clk_gend = get_col(grid_record, _T("gend"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_acct = get_col(grid_record, _T("acct"));
	link_t_ptr clk_acct_crt = get_col(grid_record, _T("acct_crt"));
	link_t_ptr clk_acct_cal = get_col(grid_record, _T("acct_cal"));
	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_psn_type = get_col(grid_record, _T("psn_type"));

	link_t_ptr clk_psn_idet_type = get_col(grid_record, _T("psn_idet_type"));
	link_t_ptr clk_psn_memo = get_col(grid_record, _T("psn_memo"));
	link_t_ptr clk_query_info = get_col(grid_record, _T("query_info"));

	tchar_t psn_insu_stas[2] = { 0 };
	tchar_t ins_admvs[NUM_LEN] = { 0 };
	tchar_t balc[NUM_LEN] = { 0 };
	tchar_t insutype[NUM_LEN] = { 0 };
	tchar_t psn_type[NUM_LEN] = { 0 };
	tchar_t psn_idet_type[NUM_LEN] = { 0 };
	tchar_t psn_memo[ERR_LEN] = { 0 };
	double crt_balc = 0.0;
	double cal_balc = 0.0;
	const tchar_t* tmp;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("baseinfo"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("gend"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_gend, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("psn_no"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_psn_no, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("psn_name"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_psn_name, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("exp_content"), -1, 1) == 0 && !is_null(ivalue))
						{
							tmp = xsstr(ivalue, _T("crtYearBalc"));
							if (tmp)
							{
								tmp += xslen(_T("crtYearBalc"));
								while (*tmp && !is_numeric(tmp))
									tmp++;
								crt_balc = xstonum(tmp);
							}

							tmp = xsstr(ivalue, _T("calYearBalc"));
							if (tmp)
							{
								tmp += xslen(_T("calYearBalc"));
								while (*tmp && !is_numeric(tmp))
									tmp++;
								cal_balc = xstonum(tmp);
							}

							set_cell_numeric(rlk_record, clk_acct_crt, crt_balc);
							set_cell_numeric(rlk_record, clk_acct_cal, cal_balc);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}
				else if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("insuinfo"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("psn_insu_stas"), -1, 1) == 0)
						{
							xscpy(psn_insu_stas, ivalue);
						}
						else if (compare_text(iname, -1, _T("insuplc_admdvs"), -1, 1) == 0)
						{
							xscpy(ins_admvs, ivalue);
						}
						else if (compare_text(iname, -1, _T("balc"), -1, 1) == 0)
						{
							xscpy(balc, ivalue);
						}
						else if (compare_text(iname, -1, _T("insutype"), -1, 1) == 0)
						{
							xscpy(insutype, ivalue);
						}
						else if (compare_text(iname, -1, _T("psn_type"), -1, 1) == 0)
						{
							xscpy(psn_type, ivalue);
						}
						
						nlk = get_json_next_sibling_item(nlk);
					}

					set_cell_text(rlk_record, clk_ins_admvs, ins_admvs, -1);
					set_cell_text(rlk_record, clk_acct, balc, -1);
					set_cell_text(rlk_record, clk_insutype, insutype, -1);
					set_cell_text(rlk_record, clk_psn_type, psn_type, -1);
				}
				else if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("idetinfo"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("psn_idet_type"), -1, 1) == 0)
						{
							xscpy(psn_idet_type, ivalue);
							
						}
						else if (compare_text(iname, -1, _T("memo"), -1, 1) == 0)
						{
							xscpy(psn_memo, ivalue);
						}

						nlk = get_json_next_sibling_item(nlk);
					}

					set_cell_text(rlk_record, clk_psn_idet_type, psn_idet_type, -1);
					set_cell_text(rlk_record, clk_psn_memo, psn_memo, -1);
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
			set_cell_text(rlk_record, clk_query_info, get_json_item_value_ptr(nlk_row), -1);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;


	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t query_opspinfo(yb_config_t* pcfg, link_t_ptr rlk_record)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("query_opspinfo"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("5301"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, pcfg->mdt_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, pcfg->ins_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("MBS_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, pcfg->hos_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, pcfg->hos_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/fsiIntegratedQueryService/queryPsnOpspReg"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("query_opspinfo"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("query_opspinfo"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("query_opspinfo.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("query_opspinfo"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_opsp_code = get_col(grid_record, _T("opsp_code"));
	link_t_ptr clk_opsp_name = get_col(grid_record, _T("opsp_name"));
	link_t_ptr clk_query_info = get_col(grid_record, _T("query_info"));

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("feedetail"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("opsp_dise_code"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_opsp_code, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("opsp_dise_name"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_opsp_name, ivalue, -1);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
			set_cell_text(rlk_record, clk_query_info, get_json_item_value_ptr(nlk_row), -1);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t query_psnsum(yb_config_t* pcfg, link_t_ptr rlk_record)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("query_psnsum"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t sumdate[DATE_LEN] = { 0 };
	xsprintf(sumdate, _T("%d%02d"), dt.year, dt.mon);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("5206"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, pcfg->mdt_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, pcfg->ins_admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("MBS_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, pcfg->hos_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, pcfg->hos_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("cum_ym"));
	//set_json_item_value(nlk, sumdate);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/fsiIntegratedQueryService/queryFixmedinsPracPsnSum"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("query_psnsum"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("query_psnsum"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("query_psnsum"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("query_psnsum.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("query_psnsum"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_year_hifp_line_sum = get_col(grid_record, _T("year_hifp_line_sum")); //统筹起付累计
	link_t_ptr clk_year_hifp_pay_sum = get_col(grid_record, _T("year_hifp_pay_sum")); //统筹基金支出累计
	link_t_ptr clk_year_hifmi_line_sum = get_col(grid_record, _T("year_hifmi_line_sum")); //大病起付线累计
	link_t_ptr clk_year_hifmi_pay_sum = get_col(grid_record, _T("year_hifmi_pay_sum")); //大病基金支出累计
	link_t_ptr clk_year_cvlserv_pay = get_col(grid_record, _T("year_cvlserv_pay")); //公务员支出累计
	link_t_ptr clk_year_hifdm_pay = get_col(grid_record, _T("year_hifdm_pay")); //伤残基金支出累计
	link_t_ptr clk_year_maf_pay = get_col(grid_record, _T("year_maf_pay")); // 医疗救助支出累计

	link_t_ptr clk_query_info = get_col(grid_record, _T("query_info"));

	tchar_t cum_type_code[NUM_LEN] = { 0 };
	tchar_t cum_year[INT_LEN] = { 0 };
	double year_hifp_line_sum = 0.0;
	double year_hifp_pay_sum = 0.0;
	double year_hifmi_line_sum = 0.0;
	double year_hifmi_pay_sum = 0.0;
	double year_cvlserv_pay = 0.0;
	double year_hifdm_pay = 0.0;
	double year_maf_pay = 0.0;
	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("cuminfo"), -1, 1) == 0)
				{
					xszero(cum_type_code, NUM_LEN);
					xszero(cum_year, INT_LEN);

					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("cum_type_code"), -1, 1) == 0)
						{
							xscpy(cum_type_code, ivalue);
						}
						else if (compare_text(iname, -1, _T("year"), -1, 1) == 0)
						{
							xscpy(cum_year, ivalue);
						}
						else if (compare_text(iname, -1, _T("cum"), -1, 1) == 0 && xstol(cum_year) == xsntol(sumdate,4))
						{
							if (xstol(pcfg->med_type) == 11) //门诊
							{
								if (compare_text(cum_type_code, -1, _T("Q1100"), -1, 1) == 0) //普通门诊起付线累计
									year_hifp_line_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("Q1401"), -1, 1) == 0) //门慢病起付线累计
									year_hifp_line_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310101BIZ11POLC001"), -1, 1) == 0) //门诊统筹基金一段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310101BIZ1401POLC001"), -1, 1) == 0) //门慢病统筹基金一段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("TQ320101BIZ1401"), -1, 1) == 0) //门慢公务员起付累计
									year_cvlserv_pay += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("TQ320101BIZ11"), -1, 1) == 0) //门诊公务员起付线累计
									year_cvlserv_pay += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("D6101011"), -1, 1) == 0) //医疗救助累计
									year_maf_pay += xstonum(ivalue);
							}
							else //规定病种和住院
							{
								if (compare_text(cum_type_code, -1, _T("Q2100"), -1, 1) == 0) //住院起付线累计
									year_hifp_line_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310101BIZ21POLC001"), -1, 1) == 0) //职工住院统筹基金一段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310101BIZ21POLC002"), -1, 1) == 0) //职工住院统筹基金二段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310101BIZ1402POLC001"), -1, 1) == 0) //职工规定病种统筹基金一段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310101BIZ1402POLC002"), -1, 1) == 0) //职工规定病种统筹基金二段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S390101BIZ21POLC001"), -1, 1) == 0) //居民住院统筹基金一段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S390101BIZ21POLC002"), -1, 1) == 0) //居民住院统筹基金二段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S390101BIZ1402POLC001"), -1, 1) == 0) //居民规定病种统筹基金一段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S390101BIZ1402POLC002"), -1, 1) == 0) //居民规定病种统筹基金二段累计
									year_hifp_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("Q0001"), -1, 1) == 0) //大病起付累计
									year_hifmi_line_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310301TREPOLPL01"), -1, 1) == 0) //职工大病基金累计一段
									year_hifmi_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310301TREPOLPL02"), -1, 1) == 0) //职工大病基金累计二段
									year_hifmi_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310301TREPOLPL03"), -1, 1) == 0) //职工大病基金累计三段
									year_hifmi_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("S310301TREPOLPL04"), -1, 1) == 0) //职工大病基金累计四段
									year_hifmi_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("TS390201POLPL01"), -1, 1) == 0) //居民大病基金累计一段
									year_hifmi_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("TS390201POLPL02"), -1, 1) == 0) //居民大病基金累计二段
									year_hifmi_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("TS390201POLPL03"), -1, 1) == 0) //居民大病基金累计三段
									year_hifmi_pay_sum += xstonum(ivalue);
								//else if (compare_text(cum_type_code, -1, _T("TS390201POLPL04"), -1, 1) == 0) //居民大病基金累计四段
									//year_hifmi_pay_sum += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("TQ320101BIZ21"), -1, 1) == 0) //公务员起付累计住院
									year_cvlserv_pay += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("TQ320101BIZ1402"), -1, 1) == 0) //公务员起付线累计规定病种
									year_cvlserv_pay += xstonum(ivalue);
								else if (compare_text(cum_type_code, -1, _T("D350101"), -1, 1) == 0) //伤残基金累计
									year_hifdm_pay += xstonum(ivalue);
							}
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}

			if (year_hifp_line_sum != 0.0)
				set_cell_numeric(rlk_record, clk_year_hifp_line_sum, year_hifp_line_sum);
			if (year_hifp_pay_sum != 0.0)
				set_cell_numeric(rlk_record, clk_year_hifp_pay_sum, year_hifp_pay_sum);
			if (year_hifmi_line_sum != 0.0)
				set_cell_numeric(rlk_record, clk_year_hifmi_line_sum, year_hifmi_line_sum);
			if (year_hifmi_pay_sum != 0.0)
				set_cell_numeric(rlk_record, clk_year_hifmi_pay_sum, year_hifmi_pay_sum);
			if (year_cvlserv_pay != 0.0)
				set_cell_numeric(rlk_record, clk_year_cvlserv_pay, year_cvlserv_pay);
			if (year_hifdm_pay != 0.0)
				set_cell_numeric(rlk_record, clk_year_hifdm_pay, year_hifdm_pay);
			if (year_maf_pay != 0.0)
				set_cell_numeric(rlk_record, clk_year_maf_pay, year_maf_pay);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
			set_cell_text(rlk_record, clk_query_info, get_json_item_value_ptr(nlk_row), -1);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t opt_reg(yb_config_t* pcfg, link_t_ptr rlk_record, tchar_t* opt_no, tchar_t* opt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("opt_reg"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));
	
	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	xsprintf(opt_no, _T("%d%02d%02d%02d%02d%02d%03d"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec, dt.millsec);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);
	
	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs")); 
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_cert_no = get_col(grid_record, _T("cert_no"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2201"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, pcfg->hos_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, pcfg->hos_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_insutype));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("begntime"));
	set_json_item_value(nlk, begntime);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, CERT_TYPE_SFZ);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_cert_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("ipt_otp_no"));
	set_json_item_value(nlk, opt_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("atddr_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dr_name"));
	set_json_item_value(nlk, _T("医师"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dept_code"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dept_name"));
	set_json_item_value(nlk, _T("科室"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("caty"));
	set_json_item_value(nlk, CATY_QK);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/outpatientDocInfoService/outpatientRregistration"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("opt_reg"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("opt_reg"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("opt_reg.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("opt_reg"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));
	link_t_ptr clk_hos_level = get_col(grid_record, _T("hos_level"));
	link_t_ptr clk_med_type = get_col(grid_record, _T("med_type"));

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("data"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("mdtrt_id"), -1, 1) == 0)
						{
							xscpy(opt_id, ivalue);
						}
						
						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				set_cell_text(rlk_record, clk_hos_code, pcfg->hos_code, -1);
				set_cell_text(rlk_record, clk_hos_name, pcfg->hos_name, -1);
				set_cell_text(rlk_record, clk_hos_level, pcfg->hos_level, -1);

				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_reg(yb_config_t* pcfg, link_t_ptr rlk_record, tchar_t* ipt_no, tchar_t* ipt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_reg"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	xsprintf(ipt_no, _T("%d%02d%02d%02d%02d%02d%03d"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec, dt.millsec);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t diag_time[DATE_LEN] = { 0 };
	format_date(&dt, diag_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_cert_no = get_col(grid_record, _T("cert_no"));
	link_t_ptr clk_med_type = get_col(grid_record, _T("med_type"));

	set_cell_text(rlk_record, clk_med_type, MED_TYPE_ZY, -1);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2401"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, pcfg->hos_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, pcfg->hos_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("mdtrtinfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_insutype));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("coner_name")); //联系人姓名
	set_json_item_value(nlk, _T("联系人"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("tel")); //联系电话
	set_json_item_value(nlk, _T("11111111111"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("begntime"));
	set_json_item_value(nlk, begntime);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, CERT_TYPE_SFZ);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_cert_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_med_type));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("ipt_no"));
	set_json_item_value(nlk, ipt_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medrcdno")); //病历号
	set_json_item_value(nlk, ipt_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("adm_diag_dscr")); //入院诊断描述
	set_json_item_value(nlk, _T("诊断描述"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("adm_dept_codg")); //入院科室编码
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("adm_dept_name")); //入院科室名称
	set_json_item_value(nlk, _T("科室"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("adm_bed")); //入院床位
	set_json_item_value(nlk, _T("B0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dscg_maindiag_code")); //住院主诊断代码
	set_json_item_value(nlk, DIAG_CODE_TNB);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dscg_maindiag_name")); //住院主诊断名称
	set_json_item_value(nlk, _T("糖尿病"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("main_cond_dscr")); //主要病情描述
	set_json_item_value(nlk, _T("病情描述"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_codg")); //病种编码
	set_json_item_value(nlk, DISE_CODEG_TNB);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_name")); //病种名称
	set_json_item_value(nlk, _T("糖尿病"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("oprn_oprt_code")); //手术操作代码
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("oprn_oprt_name")); //手术操作名称
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fpsc_no")); //计划生育服务证号
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("matn_type")); //生育类别
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("birctrl_type")); //计划生育手术类别
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("latechb_flag")); //晚育标志
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("geso_val")); //孕周数
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fetts")); //胎次
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fetus_cnt")); //胎儿数
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("pret_flag")); //早产标志
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("birctrl_matn_date")); //计划生育手术或生育日期
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("atddr_no")); //主治医生编码
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("chfpdr_name")); //主诊医师姓名
	set_json_item_value(nlk, _T("主治医生"));

	clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("diseinfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no")); //人员编号
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_type")); //诊断类别
	set_json_item_value(nlk, DIAG_TYPE_XY);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("maindiag_flag")); //主诊断标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_srt_no")); //诊断排序号
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_code")); //诊断代码
	set_json_item_value(nlk, DIAG_CODE_TNB);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_name")); //诊断名称
	set_json_item_value(nlk, _T("糖尿病"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("adm_cond")); //入院病情
	set_json_item_value(nlk, _T("入院病情"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_dept")); //诊断科室
	set_json_item_value(nlk, _T("诊断科室"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_dor_no")); //诊断医生编码
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_dor_name")); //诊断医生编码
	set_json_item_value(nlk, _T("诊断医生"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_time")); //诊断时间
	set_json_item_value(nlk, diag_time);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/hospitalRegisterService/hospitalRegisterSave"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_reg"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_reg"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_reg.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_reg"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("result"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("mdtrt_id"), -1, 1) == 0)
						{
							xscpy(ipt_id, ivalue);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				set_cell_text(rlk_record, clk_hos_code, pcfg->hos_code, -1);
				set_cell_text(rlk_record, clk_hos_name, pcfg->hos_name, -1);

				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t opt_reg_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* opt_no, const tchar_t* opt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("opt_reg_cancel"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2202"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));
	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("ipt_otp_no"));
	set_json_item_value(nlk, opt_no);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/outpatientDocInfoService/outpatientRegistrationCancel"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("opt_reg_cancel"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("opt_reg_cancel"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("opt_reg_cancel.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("opt_reg_cancel"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_reg_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_reg_cancel"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2404"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));
	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, ipt_id);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/hospitalRegisterService/hospitalRegisterCancel"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_reg_cancel"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_reg_cancel"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_reg_cancel.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_reg_cancel"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t opt_trt_up(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* opt_no, const tchar_t* opt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("opt_trt_up"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t diag_time[DATE_LEN] = { 0 };
	format_date(&dt, diag_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));
	link_t_ptr clk_opsp_code = get_col(grid_record, _T("opsp_code"));
	link_t_ptr clk_opsp_name = get_col(grid_record, _T("opsp_name"));
	link_t_ptr clk_dise_codg = get_col(grid_record, _T("dise_codg"));
	link_t_ptr clk_dise_name = get_col(grid_record, _T("dise_name"));
	link_t_ptr clk_med_type = get_col(grid_record, _T("med_type"));

	if (is_null(get_cell_text_ptr(rlk_record, clk_opsp_code)))
		set_cell_text(rlk_record, clk_med_type, MED_TYPE_MZ, -1);
	else
		set_cell_text(rlk_record, clk_med_type, MED_TYPE_GD, -1);

	if (compare_text(get_cell_text_ptr(rlk_record, clk_med_type), -1, MED_TYPE_GD, -1, 0) == 0)
	{
		set_cell_text(rlk_record, clk_dise_codg, get_cell_text_ptr(rlk_record, clk_opsp_code), -1);
		set_cell_text(rlk_record, clk_dise_name, get_cell_text_ptr(rlk_record, clk_opsp_name), -1);
	}
	else
	{
		if (is_null(get_cell_text_ptr(rlk_record, clk_dise_codg)))
		{
			set_cell_text(rlk_record, clk_dise_codg, DISE_CODEG_TNB, -1);
			set_cell_text(rlk_record, clk_dise_name, _T("糖尿病"), -1);
		}
	}

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2203A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("mdtrtinfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_med_type));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("begntime"));
	set_json_item_value(nlk, begntime);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("main_cond_dscr"));
	set_json_item_value(nlk, _T("病情描述"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_codg"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_dise_codg));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_dise_name));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("birctrl_type"));
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("birctrl_matn_date"));
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("matn_type"));
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("geso_val"));
	set_json_item_value(nlk, _T(""));

	clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("diseinfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_type"));
	set_json_item_value(nlk, DIAG_TYPE_XY);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_srt_no"));
	set_json_item_value(nlk, _T("01"));

	tchar_t dise_code[NUM_LEN] = { 0 };
	tchar_t diag_code[NUM_LEN] = { 0 };
	tchar_t diag_name[RES_LEN] = { 0 };
	get_cell_text(rlk_record, clk_dise_codg, dise_code, NUM_LEN);

	if (compare_text(get_cell_text_ptr(rlk_record, clk_med_type), -1, MED_TYPE_GD, -1, 0) == 0)
	{
		find_dise_diag(dise_code, diag_code, diag_name);
	}
	else
	{
		xscpy(diag_code, DIAG_CODE_TNB);
		xscpy(diag_name, _T("糖尿病"));
	}

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_code"));
	set_json_item_value(nlk, diag_code);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_name"));
	set_json_item_value(nlk, diag_name);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_dept"));
	set_json_item_value(nlk, _T("023"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_dor_no"));
	set_json_item_value(nlk, _T("02"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_dor_name"));
	set_json_item_value(nlk, _T("医生姓名"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_time"));
	set_json_item_value(nlk, diag_time);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("vali_flag"));
	set_json_item_value(nlk, _T("01"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/outpatientDocInfoService/outpatientMdtrtinfoUpA"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("opt_trt_up"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("opt_trt_up"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("opt_trt_up.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("opt_trt_up"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t opt_fee_up(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* opt_no, const tchar_t* opt_id, tchar_t* medfee_sumamt, tchar_t* fulamt_ownpay_amt, tchar_t* overlmt_selfpay, tchar_t* preselfpay_amt, tchar_t* inscp_scp_amt)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("opt_fee_up"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t diag_time[DATE_LEN] = { 0 };
	format_date(&dt, diag_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));
	link_t_ptr clk_dise_codg = get_col(grid_record, _T("dise_codg"));
	link_t_ptr clk_dise_name = get_col(grid_record, _T("dise_name"));
	link_t_ptr clk_med_type = get_col(grid_record, _T("med_type"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2204"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	tchar_t fee_sn[NUM_LEN];

	int i, n = sizeof(fee_case_mz) / sizeof(fee_case_t);
	for (i = 0; i < n; i++)
	{
		ltoxs(i + 1, fee_sn, NUM_LEN);

		link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
		set_json_item_name(clk_child, _T("feedetail"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("feedetl_sn")); //单次就诊内唯一
		set_json_item_value(nlk, fee_sn);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("mdtrt_id"));
		set_json_item_value(nlk, opt_id);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("psn_no"));
		set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("chrg_bchno")); //同一收费批次号病种编号必须一致
		set_json_item_value(nlk, opt_id);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("dise_codg"));
		set_json_item_value(nlk, DISE_CODEG_TNB);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("rxno")); //外购处方时，传入外购处方的处方号；非外购处方，传入医药机构处方号
		set_json_item_value(nlk, opt_id);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("rx_circ_flag"));
		set_json_item_value(nlk, _T("0"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("fee_ocur_time"));
		set_json_item_value(nlk, inf_time);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("med_list_codg"));
		set_json_item_value(nlk, fee_case_mz[i].fee_codg); //医疗目录编码

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("medins_list_codg"));
		set_json_item_value(nlk, fee_case_mz[i].fee_codg); //医药机构目录编码

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("det_item_fee_sumamt")); //明细项目费用总额
		set_json_item_value(nlk, fee_case_mz[i].fee_sumamt);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("cnt")); //数量
		set_json_item_value(nlk, fee_case_mz[i].cnt);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("pric")); //单价
		set_json_item_value(nlk, fee_case_mz[i].pric);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("sin_dos_dscr")); //单次剂量描述
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("used_frqu_dscr")); //使用频次描述
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("prd_days")); //周期天数
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("medc_way_dscr")); //用药途径描述
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("bilg_dept_codg")); //开单科室编码
		set_json_item_value(nlk, _T("1"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("bilg_dept_name")); //开单科室名称
		set_json_item_value(nlk, _T("开单科室"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("bilg_dr_codg")); //开单医生编码
		set_json_item_value(nlk, _T("1"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("bilg_dr_name")); //开单医师姓名
		set_json_item_value(nlk, _T("开单医生"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("acord_dept_codg")); //受单科室编码
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("acord_dept_name")); //受单科室名称
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("orders_dr_code")); //受单医生编码
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("orders_dr_name")); //受单医生姓名
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("hosp_appr_flag")); //医院审批标志
		set_json_item_value(nlk, _T("1"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("tcmdrug_used_way")); //中药使用方式
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("etip_flag")); //中药使用方式
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("etip_hosp_code")); //外检医院编码
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("dscg_tkdrug_flag")); //出院带药标志
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("matn_fee_flag")); //生育费用标志
		set_json_item_value(nlk, _T(""));
	}

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/outpatientDocInfoService/outpatientFeeListUp"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("opt_fee_up"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("opt_fee_up"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("opt_fee_up.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("opt_fee_up"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };
	double f_medfee_sumamt = 0.0;
	double f_fulamt_ownpay_amt = 0.0;
	double f_overlmt_selfpay = 0.0;
	double f_preselfpay_amt = 0.0;
	double f_inscp_scp_amt = 0.0;
	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			double f_medfee_sumamt = 0.0;
			double f_fulamt_ownpay_amt = 0.0;
			double f_overlmt_selfpay = 0.0;
			double f_preselfpay_amt = 0.0;
			double f_inscp_scp_amt = 0.0;

			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("result"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("det_item_fee_sumamt"), -1, 1) == 0)
						{
							f_medfee_sumamt += xstof(ivalue);
						}
						else if (compare_text(iname, -1, _T("fulamt_ownpay_amt"), -1, 1) == 0)
						{
							f_fulamt_ownpay_amt += xstof(ivalue);
						}
						else if (compare_text(iname, -1, _T("overlmt_amt"), -1, 1) == 0)
						{
							f_overlmt_selfpay += xstof(ivalue);
						}
						else if (compare_text(iname, -1, _T("preselfpay_amt"), -1, 1) == 0)
						{
							f_preselfpay_amt += xstof(ivalue);
						}
						else if (compare_text(iname, -1, _T("inscp_scp_amt"), -1, 1) == 0)
						{
							f_inscp_scp_amt += xstof(ivalue);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}

			xsprintf(medfee_sumamt, _T("%.2f"), f_medfee_sumamt);
			xsprintf(fulamt_ownpay_amt, _T("%.2f"), f_fulamt_ownpay_amt);
			xsprintf(overlmt_selfpay, _T("%.2f"), f_overlmt_selfpay);
			xsprintf(preselfpay_amt, _T("%.2f"), f_preselfpay_amt);
			xsprintf(inscp_scp_amt, _T("%.2f"), f_inscp_scp_amt);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_fee_up(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id, tchar_t* medfee_sumamt, tchar_t* fulamt_ownpay_amt, tchar_t* overlmt_selfpay, tchar_t* preselfpay_amt, tchar_t* inscp_scp_amt)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_fee_up"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t diag_time[DATE_LEN] = { 0 };
	format_date(&dt, diag_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));
	link_t_ptr clk_dise_codg = get_col(grid_record, _T("dise_codg"));
	link_t_ptr clk_dise_name = get_col(grid_record, _T("dise_name"));
	link_t_ptr clk_med_type = get_col(grid_record, _T("med_type"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2301"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	tchar_t fee_sn[NUM_LEN];
	tchar_t fee_order[NUM_LEN];

	int i, n = sizeof(fee_case_zy) / sizeof(fee_case_t);
	for (i = 0; i < n; i++)
	{
		ltoxs(i + 1, fee_sn, NUM_LEN);
		xsprintf(fee_order, _T("%s%2d"), ipt_no, i);
		
		link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
		set_json_item_name(clk_child, _T("feedetail"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("feedetl_sn")); //单次就诊内唯一
		set_json_item_value(nlk, fee_sn);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("mdtrt_id"));
		set_json_item_value(nlk, ipt_id);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("drord_no")); //医嘱号
		set_json_item_value(nlk, fee_order);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("psn_no"));
		set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("med_type")); //医疗类别
		set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_med_type));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("fee_ocur_time")); //费用发生时间
		set_json_item_value(nlk, inf_time);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("med_list_codg")); //医疗目录编码
		set_json_item_value(nlk, fee_case_zy[i].fee_codg);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("medins_list_codg")); //医药机构目录编码
		set_json_item_value(nlk, fee_case_zy[i].fee_codg);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("det_item_fee_sumamt")); //明细项目费用总额
		set_json_item_value(nlk, fee_case_zy[i].fee_sumamt);

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("cnt"));
		set_json_item_value(nlk, fee_case_zy[i].cnt); //数量

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("pric"));
		set_json_item_value(nlk, fee_case_zy[i].pric); //单价

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("bilg_dept_codg")); //开单科室编码
		set_json_item_value(nlk, _T("1"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("bilg_dept_name")); //开单科室名称
		set_json_item_value(nlk, _T("开单科室"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("bilg_dr_codg")); //开单医生编码
		set_json_item_value(nlk, _T("1"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("bilg_dr_name")); //开单医师姓名
		set_json_item_value(nlk, _T("开单医生"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("acord_dept_codg")); //受单科室编码
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("acord_dept_name")); //受单科室名称
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("orders_dr_code")); //受单医生编码
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("orders_dr_name")); //受单医生姓名
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("hosp_appr_flag")); //医院审批标志
		set_json_item_value(nlk, _T("1"));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("tcmdrug_used_way")); //中药使用方式
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("etip_flag")); //中药使用方式
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("etip_hosp_code")); //外检医院编码
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("dscg_tkdrug_flag")); //出院带药标志
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("matn_fee_flag")); //生育费用标志
		set_json_item_value(nlk, _T(""));

		nlk = insert_json_item(clk_child, LINK_LAST);
		set_json_item_name(nlk, _T("memo")); //备注
		set_json_item_value(nlk, _T(""));
	}

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/hospFeeDtlService/feeDtlUp"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_fee_up"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_fee_up"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_fee_up.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_fee_up"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			double f_medfee_sumamt = 0.0;
			double f_fulamt_ownpay_amt = 0.0;
			double f_overlmt_selfpay = 0.0;
			double f_preselfpay_amt = 0.0;
			double f_inscp_scp_amt = 0.0;

			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("result"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("det_item_fee_sumamt"), -1, 1) == 0)
						{
							f_medfee_sumamt += xstof(ivalue);
						}
						else if (compare_text(iname, -1, _T("fulamt_ownpay_amt"), -1, 1) == 0)
						{
							f_fulamt_ownpay_amt += xstof(ivalue);
						}
						else if (compare_text(iname, -1, _T("overlmt_amt"), -1, 1) == 0)
						{
							f_overlmt_selfpay += xstof(ivalue);
						}
						else if (compare_text(iname, -1, _T("preselfpay_amt"), -1, 1) == 0)
						{
							f_preselfpay_amt += xstof(ivalue);
						}
						else if (compare_text(iname, -1, _T("inscp_scp_amt"), -1, 1) == 0)
						{
							f_inscp_scp_amt += xstof(ivalue);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}

			xsprintf(medfee_sumamt, _T("%.2f"), f_medfee_sumamt);
			xsprintf(fulamt_ownpay_amt, _T("%.2f"), f_fulamt_ownpay_amt);
			xsprintf(overlmt_selfpay, _T("%.2f"), f_overlmt_selfpay);
			xsprintf(preselfpay_amt, _T("%.2f"), f_preselfpay_amt);
			xsprintf(inscp_scp_amt, _T("%.2f"), f_inscp_scp_amt);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t opt_fee_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* opt_no, const tchar_t* opt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("opt_fee_cancel"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2205"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("chrg_bchno"));
	set_json_item_value(nlk, _T("0000"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/outpatientDocInfoService/outpatientFeeListUpCancel"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("opt_fee_cancel"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("opt_fee_cancel"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("opt_fee_cancel.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("opt_fee_cancel"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_fee_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_fee_cancel"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2302"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("feedetl_sn")); //费用明细流水号,传入“0000”时删除全部
	set_json_item_value(nlk, _T("0000"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/hospFeeDtlService/feeDtlCl"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_fee_cancel"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_fee_cancel"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_fee_cancel.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_fee_cancel"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t opt_preset(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* opt_no, const tchar_t* opt_id, const tchar_t* medfee_sumamt, const tchar_t* fulamt_ownpay_amt, const tchar_t* overlmt_selfpay, const tchar_t* preselfpay_amt, const tchar_t* inscp_scp_amt)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("opt_preset"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));
	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_cert_no = get_col(grid_record, _T("cert_no"));
	link_t_ptr clk_med_type = get_col(grid_record, _T("med_type"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2206A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, CERT_TYPE_SFZ);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no")); //就诊凭证类型为“01”时填写电子凭证令牌，为“02”时填写身份证号，为“03”时填写社会保障卡卡号
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_cert_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_med_type));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, medfee_sumamt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_setlway")); //个人结算方式
	set_json_item_value(nlk, PSN_SETLWAY_XM);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("chrg_bchno")); //收费批次号
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_used_flag")); //个人账户使用标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种类型
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_insutype));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/outpatientSettleService/preSettletmentA"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("opt_preset"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("opt_preset"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("opt_preset.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("opt_preset"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_medfee_sumamt = get_col(grid_record, _T("medfee_sumamt"));
	link_t_ptr clk_fulamt_ownpay_amt = get_col(grid_record, _T("fulamt_ownpay_amt"));
	link_t_ptr clk_overlmt_selfpay = get_col(grid_record, _T("overlmt_selfpay"));
	link_t_ptr clk_preselfpay_amt = get_col(grid_record, _T("preselfpay_amt"));
	link_t_ptr clk_inscp_scp_amt = get_col(grid_record, _T("inscp_scp_amt"));
	link_t_ptr clk_act_pay_dedc = get_col(grid_record, _T("act_pay_dedc"));
	link_t_ptr clk_hifp_pay = get_col(grid_record, _T("hifp_pay"));
	link_t_ptr clk_pool_prop_selfpay = get_col(grid_record, _T("pool_prop_selfpay"));
	link_t_ptr clk_cvlserv_pay = get_col(grid_record, _T("cvlserv_pay"));
	link_t_ptr clk_hifes_pay = get_col(grid_record, _T("hifes_pay"));
	link_t_ptr clk_hifmi_pay = get_col(grid_record, _T("hifmi_pay"));
	link_t_ptr clk_hifob_pay = get_col(grid_record, _T("hifob_pay"));
	link_t_ptr clk_maf_pay = get_col(grid_record, _T("maf_pay"));
	link_t_ptr clk_oth_pay = get_col(grid_record, _T("oth_pay"));
	link_t_ptr clk_fund_pay_sumamt = get_col(grid_record, _T("fund_pay_sumamt"));
	link_t_ptr clk_psn_part_amt = get_col(grid_record, _T("psn_part_amt"));
	link_t_ptr clk_acct_pay = get_col(grid_record, _T("acct_pay"));
	link_t_ptr clk_psn_cash_pay = get_col(grid_record, _T("psn_cash_pay"));
	link_t_ptr clk_balc = get_col(grid_record, _T("balc"));
	link_t_ptr clk_acct_mulaid_pay = get_col(grid_record, _T("acct_mulaid_pay"));
	link_t_ptr clk_hosp_part_amt = get_col(grid_record, _T("hosp_part_amt"));
	link_t_ptr clk_calc_info = get_col(grid_record, _T("calc_info"));

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("setlinfo"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("medfee_sumamt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_medfee_sumamt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("fulamt_ownpay_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_fulamt_ownpay_amt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("overlmt_selfpay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_overlmt_selfpay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("preselfpay_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_preselfpay_amt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("inscp_scp_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_inscp_scp_amt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("act_pay_dedc"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_act_pay_dedc, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifp_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifp_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("pool_prop_selfpay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_pool_prop_selfpay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("cvlserv_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_cvlserv_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifes_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifes_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifmi_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifmi_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifob_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifob_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("maf_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_maf_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("oth_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_oth_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("fund_pay_sumamt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_fund_pay_sumamt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("psn_part_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_psn_part_amt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("acct_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_acct_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("psn_cash_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_psn_cash_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("balc"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_balc, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("acct_mulaid_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_acct_mulaid_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hosp_part_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hosp_part_amt, ivalue, -1);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
			set_cell_text(rlk_record, clk_calc_info, get_json_item_value_ptr(nlk_row), -1);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_preset(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id, const tchar_t* medfee_sumamt, const tchar_t* fulamt_ownpay_amt, const tchar_t* overlmt_selfpay, const tchar_t* preselfpay_amt, const tchar_t* inscp_scp_amt)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_preset"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));
	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_cert_no = get_col(grid_record, _T("cert_no"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2303A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, CERT_TYPE_SFZ);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no")); //就诊凭证类型为“01”时填写电子凭证令牌，为“02”时填写身份证号，为“03”时填写社会保障卡卡号
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_cert_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, medfee_sumamt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_setlway")); //个人结算方式
	set_json_item_value(nlk, PSN_SETLWAY_XM);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种类型
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_insutype));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_used_flag")); //个人账户使用标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("invono")); //发票号
	set_json_item_value(nlk, ipt_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mid_setl_flag")); //中途结算标志
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fulamt_ownpay_amt")); //全自费金额
	set_json_item_value(nlk, fulamt_ownpay_amt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("overlmt_selfpay")); //超限价金额
	set_json_item_value(nlk, overlmt_selfpay);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("preselfpay_amt")); //先行自付金额
	set_json_item_value(nlk, preselfpay_amt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("inscp_scp_amt")); //符合政策范围金额
	set_json_item_value(nlk, inscp_scp_amt);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/hospSettService/preSettA"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_preset"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_preset"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_preset.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_preset"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_medfee_sumamt = get_col(grid_record, _T("medfee_sumamt"));
	link_t_ptr clk_fulamt_ownpay_amt = get_col(grid_record, _T("fulamt_ownpay_amt"));
	link_t_ptr clk_overlmt_selfpay = get_col(grid_record, _T("overlmt_selfpay"));
	link_t_ptr clk_preselfpay_amt = get_col(grid_record, _T("preselfpay_amt"));
	link_t_ptr clk_inscp_scp_amt = get_col(grid_record, _T("inscp_scp_amt"));
	link_t_ptr clk_act_pay_dedc = get_col(grid_record, _T("act_pay_dedc"));
	link_t_ptr clk_hifp_pay = get_col(grid_record, _T("hifp_pay"));
	link_t_ptr clk_pool_prop_selfpay = get_col(grid_record, _T("pool_prop_selfpay"));
	link_t_ptr clk_cvlserv_pay = get_col(grid_record, _T("cvlserv_pay"));
	link_t_ptr clk_hifes_pay = get_col(grid_record, _T("hifes_pay"));
	link_t_ptr clk_hifmi_pay = get_col(grid_record, _T("hifmi_pay"));
	link_t_ptr clk_hifob_pay = get_col(grid_record, _T("hifob_pay"));
	link_t_ptr clk_maf_pay = get_col(grid_record, _T("maf_pay"));
	link_t_ptr clk_oth_pay = get_col(grid_record, _T("oth_pay"));
	link_t_ptr clk_hifdm_pay = get_col(grid_record, _T("hifdm_pay"));
	link_t_ptr clk_othfund_pay = get_col(grid_record, _T("othfund_pay"));
	link_t_ptr clk_fund_pay_sumamt = get_col(grid_record, _T("fund_pay_sumamt"));
	link_t_ptr clk_psn_part_amt = get_col(grid_record, _T("psn_part_amt"));
	link_t_ptr clk_acct_pay = get_col(grid_record, _T("acct_pay"));
	link_t_ptr clk_psn_cash_pay = get_col(grid_record, _T("psn_cash_pay"));
	link_t_ptr clk_balc = get_col(grid_record, _T("balc"));
	link_t_ptr clk_acct_mulaid_pay = get_col(grid_record, _T("acct_mulaid_pay"));
	link_t_ptr clk_hosp_part_amt = get_col(grid_record, _T("hosp_part_amt"));
	link_t_ptr clk_calc_info = get_col(grid_record, _T("calc_info"));

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("setlinfo"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("medfee_sumamt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_medfee_sumamt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("fulamt_ownpay_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_fulamt_ownpay_amt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("overlmt_selfpay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_overlmt_selfpay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("preselfpay_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_preselfpay_amt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("inscp_scp_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_inscp_scp_amt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("act_pay_dedc"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_act_pay_dedc, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifp_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifp_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("pool_prop_selfpay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_pool_prop_selfpay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("cvlserv_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_cvlserv_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifes_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifes_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifmi_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifmi_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifob_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifob_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("maf_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_maf_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("oth_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_oth_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hifdm_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hifdm_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("othfund_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_othfund_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("fund_pay_sumamt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_fund_pay_sumamt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("psn_part_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_psn_part_amt, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("acct_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_acct_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("psn_cash_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_psn_cash_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("balc"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_balc, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("acct_mulaid_pay"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_acct_mulaid_pay, ivalue, -1);
						}
						else if (compare_text(iname, -1, _T("hosp_part_amt"), -1, 1) == 0)
						{
							set_cell_text(rlk_record, clk_hosp_part_amt, ivalue, -1);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
			set_cell_text(rlk_record, clk_calc_info, get_json_item_value_ptr(nlk_row), -1);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t opt_settle(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* opt_no, const tchar_t* opt_id, const tchar_t* medfee_sumamt, const tchar_t* fulamt_ownpay_amt, const tchar_t* overlmt_selfpay, const tchar_t* preselfpay_amt, const tchar_t* inscp_scp_amt, tchar_t* setl_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("opt_settle"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));
	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_cert_no = get_col(grid_record, _T("cert_no"));
	link_t_ptr clk_med_type = get_col(grid_record, _T("med_type"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2207A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, CERT_TYPE_SFZ);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no")); //就诊凭证类型为“01”时填写电子凭证令牌，为“02”时填写身份证号，为“03”时填写社会保障卡卡号
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_cert_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_med_type));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, medfee_sumamt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_setlway")); //个人结算方式
	set_json_item_value(nlk, PSN_SETLWAY_XM);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("chrg_bchno")); //收费批次号
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_used_flag")); //个人账户使用标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种类型
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_insutype));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("invono")); //发票号
	set_json_item_value(nlk, opt_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fulamt_ownpay_amt")); //全自费金额
	set_json_item_value(nlk, fulamt_ownpay_amt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("overlmt_selfpay")); //超限价金额
	set_json_item_value(nlk, overlmt_selfpay);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("preselfpay_amt")); //先行自付金额
	set_json_item_value(nlk, preselfpay_amt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("inscp_scp_amt")); //符合政策范围金额
	set_json_item_value(nlk, inscp_scp_amt);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/outpatientSettleService/saveSettletmentA"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("opt_settle"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("opt_settle"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("opt_settle.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("opt_settle"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_calc_info = get_col(grid_record, _T("calc_info"));

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("setlinfo"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("setl_id"), -1, 1) == 0)
						{
							xscpy(setl_id, ivalue);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
			set_cell_text(rlk_record, clk_calc_info, get_json_item_value_ptr(nlk_row), -1);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_settle(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id, const tchar_t* medfee_sumamt, const tchar_t* fulamt_ownpay_amt, const tchar_t* overlmt_selfpay, const tchar_t* preselfpay_amt, const tchar_t* inscp_scp_amt, tchar_t* setl_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_settle"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));
	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_cert_no = get_col(grid_record, _T("cert_no"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2304A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, CERT_TYPE_SFZ);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no")); //就诊凭证类型为“01”时填写电子凭证令牌，为“02”时填写身份证号，为“03”时填写社会保障卡卡号
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_cert_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, medfee_sumamt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_setlway")); //个人结算方式
	set_json_item_value(nlk, PSN_SETLWAY_XM);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种类型
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_insutype));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_used_flag")); //个人账户使用标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("invono")); //发票号
	set_json_item_value(nlk, ipt_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mid_setl_flag")); //中途结算标志
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fulamt_ownpay_amt")); //全自费金额
	set_json_item_value(nlk, fulamt_ownpay_amt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("overlmt_selfpay")); //超限价金额
	set_json_item_value(nlk, overlmt_selfpay);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("preselfpay_amt")); //先行自付金额
	set_json_item_value(nlk, preselfpay_amt);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("inscp_scp_amt")); //符合政策范围金额
	set_json_item_value(nlk, inscp_scp_amt);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/hospSettService/settA"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_settle"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_settle"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_settle.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_settle"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	link_t_ptr clk_calc_info = get_col(grid_record, _T("calc_info"));

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("output"), -1, 1) == 0)
		{
			link_t_ptr nlk_sub = get_json_first_child_item(nlk_row);
			while (nlk_sub)
			{
				if (compare_text(get_json_item_name_ptr(nlk_sub), -1, _T("setlinfo"), -1, 1) == 0)
				{
					link_t_ptr nlk = get_json_first_child_item(nlk_sub);
					while (nlk)
					{
						const tchar_t* iname = get_json_item_name_ptr(nlk);
						const tchar_t* ivalue = get_json_item_value_ptr(nlk);
						if (compare_text(iname, -1, _T("setl_id"), -1, 1) == 0)
						{
							xscpy(setl_id, ivalue);
						}

						nlk = get_json_next_sibling_item(nlk);
					}
				}

				nlk_sub = get_json_next_sibling_item(nlk_sub);
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
			set_cell_text(rlk_record, clk_calc_info, get_json_item_value_ptr(nlk_row), -1);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t opt_settle_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* opt_id, const tchar_t* setl_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("opt_settle_cancel"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2208"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("setl_id"));
	set_json_item_value(nlk, setl_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/outpatientSettleService/cancleSettletment"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("opt_settle_cancel"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("opt_settle_cancel"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("opt_settle_cancel.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("opt_settle_cancel"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_settle_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_id, const tchar_t* setl_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_settle_cancel"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2305"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("setl_id"));
	set_json_item_value(nlk, setl_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/hospSettService/settCl"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_settle_cancel"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_settle_cancel"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_settle_cancel.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_settle_cancel"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_discharge(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_discharge"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t cur_date[DATE_LEN] = { 0 };
	format_date(&dt, cur_date);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_dise_codg = get_col(grid_record, _T("dise_codg"));
	link_t_ptr clk_dise_name = get_col(grid_record, _T("dise_name"));

	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2402"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("dscginfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_insutype));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("endtime")); //就诊ID
	set_json_item_value(nlk, begntime);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_codg")); //病种编码
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_dise_codg));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_name")); //病种名称
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_dise_name));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("oprn_oprt_code")); //手术操作代码
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("oprn_oprt_name")); //手术操作名称
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fpsc_no")); //计划生育服务证号
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("matn_type")); //生育类别
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("birctrl_type")); //计划生育手术类别
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("latechb_flag")); //晚育标志
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("geso_val")); //孕周数
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fetts")); //胎次
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fetus_cnt")); //胎儿数
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("pret_flag")); //早产标志
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("birctrl_matn_date")); //计划生育手术或生育日期
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("cop_flag")); //伴有并发症标志
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dscg_dept_codg")); //出院科室编码
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dscg_dept_name")); //出院科室名称
	set_json_item_value(nlk, _T("出院科室"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dscg_bed")); //出院床位
	set_json_item_value(nlk, _T("B0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dscg_way")); //离院方式
	set_json_item_value(nlk, DSCG_WAY_CY);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("die_date")); //死亡日期
	set_json_item_value(nlk, _T(""));

	clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("diseinfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no")); //人员编号
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_type")); //诊断类别
	set_json_item_value(nlk, DIAG_TYPE_XY);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("maindiag_flag")); //主诊断标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_srt_no")); //诊断排序号
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_code")); //诊断代码
	set_json_item_value(nlk, DIAG_CODE_TNB);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_name")); //诊断名称
	set_json_item_value(nlk, _T("糖尿病"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_dept")); //诊断科室
	set_json_item_value(nlk, _T("诊断科室"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_dor_no")); //诊断医生编码
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_dor_name")); //诊断医生编码
	set_json_item_value(nlk, _T("诊断医生"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_time")); //诊断时间
	set_json_item_value(nlk, cur_date);

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/dscgService/dischargeProcess"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_discharge"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_discharge"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_discharge.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_discharge"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}

bool_t ipt_discharge_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id)
{
	link_t_ptr json = NULL;
	xhand_t xhttp = NULL;
	byte_t** pbuf = NULL;

	tchar_t err_code[NUM_LEN + 1] = { 0 };
	tchar_t err_text[ERR_LEN + 1] = { 0 };
	bool_t b_ret = 0;

	XDK_ASSERT(grid_sign != NULL);

	TRY_CATCH;

	link_t_ptr rlk_sign = loca_sign();
	if (rlk_sign == NULL)
	{
		raise_user_error(_T("ipt_discharge_cancel"), _T("未签到"));
	}

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));

	tchar_t sign_no[NUM_LEN] = { 0 };
	get_cell_text(rlk_sign, clk_sign_no, sign_no, NUM_LEN);

	xdate_t dt;
	get_loc_date(&dt);

	tchar_t begntime[DATE_LEN] = { 0 };
	format_datetime(&dt, begntime);

	tchar_t inf_time[DATE_LEN] = { 0 };
	format_datetime(&dt, inf_time);

	tchar_t in_msgid[31];
	xscpy(in_msgid, pcfg->hos_code);
	xsprintf((in_msgid + xslen(in_msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr clk_mdt_admvs = get_col(grid_record, _T("mdt_admvs"));
	link_t_ptr clk_ins_admvs = get_col(grid_record, _T("ins_admvs"));
	link_t_ptr clk_psn_no = get_col(grid_record, _T("psn_no"));
	link_t_ptr clk_hos_code = get_col(grid_record, _T("hos_code"));
	link_t_ptr clk_hos_name = get_col(grid_record, _T("hos_name"));

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2405"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, in_msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_mdt_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_ins_admvs));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("recer_sys_code"));
	set_json_item_value(nlk, _T("FSI_LOCAL"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_no"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("dev_safe_info"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("cainfo"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("signtype"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infver"));
	set_json_item_value(nlk, _T("1.0"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter"));
	set_json_item_value(nlk, pcfg->opt_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, pcfg->opt_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_code));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_hos_name));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, get_cell_text_ptr(rlk_record, clk_psn_no));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	destroy_json_doc(json);
	json = NULL;

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char hos_sec[NUM_LEN] = { 0 };
	ucs_to_gb2312(pcfg->hos_sec, -1, (byte_t*)hos_sec, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)"", (char*)hos_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	tchar_t sz_url[PATH_LEN] = { 0 };
	xsprintf(sz_url, _T("%s/fsi/api/dscgService/dischargeUndo"), pcfg->yb_addr);

	xhttp = xhttp_client(_T("POST"), sz_url);
	if (!xhttp)
	{
		raise_user_error(_T("sign_in"), _T("xhttp_client"));
	}

	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, pcfg->hos_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		raise_user_error(_T("ipt_discharge_cancel"), _T("xhttp_send_full"));
	}

	dword_t nlen = 0;
	pbuf = bytes_alloc();

	if (!xhttp_recv_full(xhttp, pbuf, &nlen))
	{
		raise_user_error(_T("ipt_discharge_cancel"), _T("xhttp_recv_full"));
	}

	xhttp_close(xhttp);
	xhttp = NULL;

	save_bytes_to_file(NULL, _T("ipt_discharge_cancel.txt"), *pbuf, nlen);

	json = create_json_doc();
	if (!parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8))
	{
		raise_user_error(_T("ipt_discharge_cancel"), _T("parse_json_doc_from_bytes"));
	}

	bytes_free(pbuf);
	pbuf = NULL;

	tchar_t out_msgid[NUM_LEN] = { 0 };

	link_t_ptr nlk_row = get_json_first_child_item(json);
	while (nlk_row)
	{
		if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("infcode"), -1, 1) == 0)
		{
			xscpy(err_code, get_json_item_value_ptr(nlk_row));
			if (xstol(err_code) == 0)
			{
				b_ret = 1;
			}
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("err_msg"), -1, 1) == 0)
		{
			xsncpy(err_text, get_json_item_value_ptr(nlk_row), ERR_LEN);
		}
		else if (compare_text(get_json_item_name_ptr(nlk_row), -1, _T("inf_refmsgid"), -1, 1) == 0)
		{
			xsncpy(out_msgid, get_json_item_value_ptr(nlk_row), NUM_LEN);
		}

		nlk_row = get_json_next_sibling_item(nlk_row);
	}

	destroy_json_doc(json);
	json = NULL;

	END_CATCH;

	get_last_error(err_code, err_text, ERR_LEN);

	return b_ret;
ONERROR:

	if (json)
		destroy_json_doc(json);

	if (pbuf)
		bytes_free(pbuf);

	if (xhttp)
		xhttp_close(xhttp);

	return bool_false;
}
