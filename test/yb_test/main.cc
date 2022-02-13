
#include "ybdef.h"
#include "ybinf.h"
#include "ybcfg.h"
#include "ybmac.h"

#ifdef _OS_WINDOWS
#include <conio.h>
#endif

void test_sm3()
{
	link_t_ptr json = create_json_doc();
	
	tchar_t cn[RES_LEN], cv[RES_LEN];

	int i;
	for (i = 0; i < 1000; i++)
	{
		link_t_ptr nlk = insert_json_item(json, LINK_LAST);

		xsprintf(cn, _T("item%d"), i);
		xsprintf(cv, _T("测试%d"), i);

		set_json_item_name(nlk, cn);
		set_json_item_value(nlk, cv);
	}

	dword_t n = format_json_doc_to_bytes(json, NULL, MAX_LONG, _GB2312);
	char* gb_buf = a_xsalloc(n + 1);
	format_json_doc_to_bytes(json, (byte_t*)gb_buf, n, _GB2312);

	n = format_json_doc_to_bytes(json, NULL, MAX_LONG, _UTF8);
	char* utf_buf = a_xsalloc(n + 1);
	format_json_doc_to_bytes(json, (byte_t*)utf_buf, n, _UTF8);

	destroy_json_doc(json);

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char out_buf[100] = { 0 };
	zj_mac_sm3((char*)"H33018300564", (char*)"fAtUGnYYI8ywzIz", str_tm, (char*)gb_buf, out_buf);

	char out_sin[100] = { 0 };
	gb_mac_sm3((char*)"H33018300564", (char*)"fAtUGnYYI8ywzIz", str_tm, (char*)utf_buf, out_sin);

	a_xsfree(gb_buf);
	a_xsfree(utf_buf);

	int k = a_xslen(out_buf);

	if (xmem_comp(out_buf, out_sin, k) == 0)
		printf("sm3 test succeed!");
	else
		printf("sm3 test failed!");
}

int main(int argc, char* argv[])
{
	xdk_process_init(XDK_APARTMENT_PROCESS);

	//test_sm3();

	yb_config_t ybcfg = { 0 };

	load_config(&ybcfg);

	int med_type = xstol(ybcfg.med_type); //普通门诊11，规定病种14，住院21
	int psn_limt = xstol(ybcfg.psn_limt); //最大分组人数
	int fee_setl = xstol(ybcfg.fee_setl); //是否结算

	load_psn_type();

	load_psn_idet_type();

	load_record();

	load_cache();

	load_sign();

	if (sign_in(&ybcfg))
	{
		save_sign();
	}

	link_t_ptr clk_med = get_col(grid_cache, _T("med")); //医疗类别
	link_t_ptr clk_bxh = get_col(grid_cache, _T("bxh")); //保险号
	link_t_ptr clk_bah = get_col(grid_cache, _T("bah")); //病案号
	link_t_ptr clk_jzh = get_col(grid_cache, _T("jzh")); //就诊号
	link_t_ptr clk_jsh = get_col(grid_cache, _T("jsh")); //结算号

	tchar_t med[NUM_LEN] = { 0 };
	tchar_t cert_no[NUM_LEN] = { 0 };
	tchar_t opt_no[NUM_LEN] = { 0 };
	tchar_t opt_id[NUM_LEN] = { 0 };
	tchar_t setl_id[NUM_LEN] = { 0 };
	tchar_t fndstr[RES_LEN];

	link_t_ptr nxt_cache, rlk_cache = get_next_row(grid_cache, LINK_FIRST);
	while (rlk_cache)
	{
		get_cell_text(rlk_cache, clk_med, med, NUM_LEN);
		get_cell_text(rlk_cache, clk_bxh, cert_no, NUM_LEN);
		get_cell_text(rlk_cache, clk_bah, opt_no, NUM_LEN);
		get_cell_text(rlk_cache, clk_jzh, opt_id, NUM_LEN);
		get_cell_text(rlk_cache, clk_jsh, setl_id, NUM_LEN);

		xsprintf(fndstr, _T("cert_no == %s"), cert_no);
		link_t_ptr rlk = find_grid_row(grid_record, fndstr, LINK_FIRST, 1);
		if (!rlk)
		{
			rlk_cache = get_next_row(grid_cache, rlk_cache);
			continue;
		}
		
		if (!is_null(setl_id))
		{
			if (xstol(med) == 21)
			{
				if (ipt_settle_cancel(&ybcfg, rlk, opt_id, setl_id))
				{
					xszero(setl_id, NUM_LEN);
					set_cell_text(rlk_cache, clk_jsh, setl_id, 0);
				}
			}
			else
			{
				if (ipt_settle_cancel(&ybcfg, rlk, opt_id, setl_id))
				{
					xszero(setl_id, NUM_LEN);
					set_cell_text(rlk_cache, clk_jsh, setl_id, 0);
				}
			}
		}

		if (!is_null(opt_id))
		{
			if (xstol(med) == 21)
			{
				ipt_fee_cancel(&ybcfg, rlk, opt_no, opt_id);
				if (ipt_reg_cancel(&ybcfg, rlk, opt_no, opt_id))
				{
					xszero(opt_id, NUM_LEN);
					set_cell_text(rlk_cache, clk_jzh, opt_id, 0);
					xszero(opt_no, NUM_LEN);
					set_cell_text(rlk_cache, clk_bah, opt_no, 0);
				}
			}
			else
			{
				opt_fee_cancel(&ybcfg, rlk, opt_no, opt_id);
				if (opt_reg_cancel(&ybcfg, rlk, opt_no, opt_id))
				{
					xszero(opt_id, NUM_LEN);
					set_cell_text(rlk_cache, clk_jzh, opt_id, 0);
					xszero(opt_no, NUM_LEN);
					set_cell_text(rlk_cache, clk_bah, opt_no, 0);
				}
			}
		}

		if (is_null(setl_id) && is_null(opt_id))
		{
			nxt_cache = rlk_cache;
			rlk_cache = get_next_row(grid_cache, rlk_cache);
			delete_row(nxt_cache);
		}
		else
		{
			rlk_cache = get_next_row(grid_cache, rlk_cache);
		}
	}

	save_cache();

	int n = 0;
	bool_t b = 0;
	
	link_t_ptr clk_cert_no = get_col(grid_record, _T("cert_no"));
	link_t_ptr clk_psn_name = get_col(grid_record, _T("psn_name"));

	tchar_t fee1[NUM_LEN] = { 0 };
	tchar_t fee2[NUM_LEN] = { 0 };
	tchar_t fee3[NUM_LEN] = { 0 };
	tchar_t fee4[NUM_LEN] = { 0 };
	tchar_t fee5[NUM_LEN] = { 0 };

	link_t_ptr clk_insutype = get_col(grid_record, _T("insutype"));
	link_t_ptr clk_psn_type = get_col(grid_record, _T("psn_type"));
	link_t_ptr clk_psn_idet_type = get_col(grid_record, _T("psn_idet_type"));
	link_t_ptr clk_psn_type_memo = get_col(grid_record, _T("psn_type_memo"));
	link_t_ptr clk_psn_memo = get_col(grid_record, _T("psn_memo"));
	link_t_ptr clk_psn_flag = get_col(grid_record, _T("psn_flag"));
	tchar_t psn_flag[5] = { 0 };
	const tchar_t *psn_memo;
	int psn_cnt;
	tchar_t psn_key[NUM_LEN];

	link_t_ptr clk_opsp_code = get_col(grid_record, _T("opsp_code"));
	link_t_ptr clk_opsp_name = get_col(grid_record, _T("opsp_name"));

	tchar_t err_code[NUM_LEN + 1];
	tchar_t err_text[ERR_LEN + 1];
	int k = 0;

	link_t_ptr st_filter;
	link_t_ptr flk;

	if (psn_limt)
	{
		st_filter = create_string_table(1);
	}

	rlk_cache = NULL;

	tchar_t token[1024];
	n = 0;
	link_t_ptr nxt, rlk_record = get_next_row(grid_record, LINK_FIRST);
	while (rlk_record)
	{
		_tprintf(_T("case %d: %s "), ++n, get_cell_text_ptr(rlk_record, clk_cert_no));

		get_cell_text(rlk_record, clk_cert_no, cert_no, NUM_LEN);
		xszero(opt_no, NUM_LEN);
		xszero(opt_id, NUM_LEN);
		xszero(setl_id, NUM_LEN);

		xszero(psn_flag, 5);
		xszero(token, 1024);

		if (!med_type || is_null(get_cell_text_ptr(rlk_record, clk_insutype)))
		{
			b = query_psninfo(&ybcfg, rlk_record);
			if (b)
			{
				const tchar_t *psn_type_value = NULL;
				const tchar_t *psn_idet_type_value = NULL;

				link_t_ptr clk_code = get_col(grid_psn_type, _T("code"));
				link_t_ptr clk_value = get_col(grid_psn_type, _T("value"));
				link_t_ptr rlk_psn_type = get_next_row(grid_psn_type, LINK_FIRST);
				while (rlk_psn_type)
				{
					if (compare_text(get_cell_text_ptr(rlk_record, clk_psn_type), -1, get_cell_text_ptr(rlk_psn_type, clk_code), -1, 1) == 0)
					{
						psn_type_value = get_cell_text_ptr(rlk_psn_type, clk_value);
						break;
					}
					rlk_psn_type = get_next_row(grid_psn_type, rlk_psn_type);
				}

				clk_code = get_col(grid_psn_idet_type, _T("code"));
				clk_value = get_col(grid_psn_idet_type, _T("value"));
				link_t_ptr rlk_psn_idet_type = get_next_row(grid_psn_idet_type, LINK_FIRST);
				while (rlk_psn_idet_type)
				{
					if (compare_text(get_cell_text_ptr(rlk_record, clk_psn_idet_type), -1, get_cell_text_ptr(rlk_psn_idet_type, clk_code), -1, 1) == 0)
					{
						psn_idet_type_value = get_cell_text_ptr(rlk_psn_idet_type, clk_value);
						break;
					}
					rlk_psn_idet_type = get_next_row(grid_psn_idet_type, rlk_psn_idet_type);
				}

				if (!is_null(psn_type_value) && !is_null(psn_idet_type_value))
				{
					xsprintf(token, _T("%s %s"), psn_type_value, psn_idet_type_value);
					set_cell_text(rlk_record, clk_psn_type_memo, token, -1);
				}
				else if (!is_null(psn_type_value))
				{
					xsprintf(token, _T("%s"), psn_type_value);
					set_cell_text(rlk_record, clk_psn_type_memo, token, -1);
				}
				get_cell_text(rlk_record, clk_psn_type_memo, token, 1024);

				if (xsstr(token, _T("职工")) || xsstr(token, _T("公务员")) || xsstr(token, _T("建国")) || xsstr(token, _T("灵活就业")) || xsstr(token, _T("企业补充")))
				{
					psn_flag[0] = _T('1');
				}
				else if (xsstr(token, _T("居民")) || xsstr(token, _T("学生")) || xsstr(token, _T("少儿")))
				{
					psn_flag[0] = _T('2');
				}
				else if (xsstr(token, _T("离休")))
				{
					psn_flag[0] = _T('3');
				}
				else
				{
					psn_flag[0] = _T('0');
				}

				if (psn_flag[0] == _T('1'))
				{
					if (xsstr(token, _T("在职")))
						psn_flag[1] = _T('1');
					else
						psn_flag[1] = _T('2');
				}
				else if (psn_flag[0] == _T('2'))
				{
					if (xsstr(token, _T("一档")))
						psn_flag[1] = _T('1');
					else if (xsstr(token, _T("二档")))
						psn_flag[1] = _T('2');
					else if (xsstr(token, _T("中小学")) || xsstr(token, _T("少儿")))
						psn_flag[1] = _T('4');
					else if (xsstr(token, _T("大学")))
						psn_flag[1] = _T('5');
					else
						psn_flag[1] = _T('3');
				}
				else if (psn_flag[0] == _T('3'))
				{
					psn_flag[1] = _T('0');
				}

				if (psn_flag[0] == _T('1'))
				{
					if (is_prefix(token, _T("公务员")))
						psn_flag[2] = _T('1');
					else if (xsstr(token, _T("公务员")))
						psn_flag[2] = _T('7');
					else
						psn_flag[2] = _T('0');
					if (is_prefix(token, _T("建国前")))
						psn_flag[2] = _T('2');
					if (xsstr(token, _T("劳模")) || xsstr(token, _T("红旗手")) || xsstr(token, _T("党员")) || xsstr(token, _T("五四奖章")))
						psn_flag[2] = _T('3');
					if (xsstr(token, _T("干部")))
						psn_flag[2] = _T('4');
					if (xsstr(token, _T("残疾军人")))
						psn_flag[2] = _T('5');
					if (xsstr(token, _T("优抚")))
						psn_flag[2] = _T('6');
				}
				else if (psn_flag[0] == _T('2'))
				{
					if (xsstr(token, _T("劳模")) || xsstr(token, _T("红旗手")))
						psn_flag[2] = _T('2');
					else if (xsstr(token, _T("残疾军人")))
						psn_flag[2] = _T('5');
					else if (xsstr(token, _T("优抚")))
						psn_flag[2] = _T('6');
					else
						psn_flag[2] = _T('0');
				}
				else if (psn_flag[0] == _T('3'))
				{
					if (xsstr(token, _T("配偶")))
						psn_flag[2] = _T('2');
					else
						psn_flag[2] = _T('1');
				}

				psn_memo = get_cell_text_ptr(rlk_record, clk_psn_memo);
				if ((psn_flag[0] == _T('1') || psn_flag[0] == _T('2')) && !is_null(psn_memo))
				{
					if (xsstr(psn_memo, _T("三无")) || xsstr(psn_memo, _T("特困")))
						psn_flag[3] = _T('1');
					else if (xsstr(psn_memo, _T("残疾人")))
						psn_flag[3] = _T('2');
					else if (xsstr(psn_memo, _T("最低生活保障")))
						psn_flag[3] = _T('3');
					else if (xsstr(psn_memo, _T("边缘")))
						psn_flag[3] = _T('4');
					else if(xsstr(psn_memo, _T("残疾军人")))
						psn_flag[3] = _T('5');
					else
						psn_flag[3] = _T('0');
				}
				else
				{
					psn_flag[3] = _T('0');
				}

				set_cell_text(rlk_record, clk_psn_flag, psn_flag, -1);
			}
		}
		else
		{
			b = 1;
		}

		if (!med_type && is_null(get_cell_text_ptr(rlk_record, clk_opsp_code)))
		{
			b = query_opspinfo(&ybcfg, rlk_record);
		}
		else
		{
			b = 1;
		}

		if (++k == 10000 && !med_type)
		{
			save_record();
			k = 0;
		}

		if (!b)
		{
			set_row_state(rlk_record, dsDelete);

			nxt = rlk_record;
			rlk_record = get_next_row(grid_record, rlk_record);
			delete_row(nxt);

			_tprintf(_T("result: invalid! \n"));
			continue;
		}

		if (psn_limt)
		{
			get_cell_text(rlk_record, clk_psn_flag, psn_key, 4);
			flk = find_string_entity(st_filter, psn_key, -1);
			if (flk && (int)get_string_entity_delta(flk) >= psn_limt)
			{
				nxt = rlk_record;
				rlk_record = get_next_row(grid_record, rlk_record);
				delete_row(nxt);

				_tprintf(_T("result: skiped! \n"));
				continue;
			}

			if (!flk)
			{
				flk = write_string_entity(st_filter, psn_key, -1, NULL, 0);
			}
			psn_cnt = (int)get_string_entity_delta(flk);
			set_string_entity_delta(flk, (vword_t)(psn_cnt + 1));
		}

		//b = query_psnsum(&ybcfg, rlk_record);

		if (b && med_type)
		{
			b = query_psnsum(&ybcfg, rlk_record);

			if (med_type == 21)
				b = ipt_reg(&ybcfg, rlk_record, opt_no, opt_id);
			else
				b = opt_reg(&ybcfg, rlk_record, opt_no, opt_id);

			if (b)
			{
				rlk_cache = insert_row(grid_cache, LINK_LAST);
				set_cell_integer(rlk_cache, clk_med, med_type);
				set_cell_text(rlk_cache, clk_bxh, cert_no, -1);
				set_cell_text(rlk_cache, clk_bah, opt_no, -1);
				set_cell_text(rlk_cache, clk_jzh, opt_id, -1);
				save_cache();

				if (med_type == 21)
					b = 1;
				else
					b = opt_trt_up(&ybcfg, rlk_record, opt_no, opt_id);

				if (b)
				{
					if (med_type == 21)
						b = ipt_fee_up(&ybcfg, rlk_record, opt_no, opt_id, fee1, fee2, fee3, fee4, fee5);
					else
						b = opt_fee_up(&ybcfg, rlk_record, opt_no, opt_id, fee1, fee2, fee3, fee4, fee5);

					if (b)
					{
						if (med_type == 21)
							b = ipt_preset(&ybcfg, rlk_record, opt_no, opt_id, fee1, fee2, fee3, fee4, fee5);
						else
							b = opt_preset(&ybcfg, rlk_record, opt_no, opt_id, fee1, fee2, fee3, fee4, fee5);

						if (b)
						{
							if (xstol(ybcfg.fee_setl))
							{
								if (med_type == 21)
									b = ipt_discharge(&ybcfg, rlk_record, opt_no, opt_id);
								else
									b = 0;

								if (b)
								{
									if (med_type == 21)
										b = ipt_settle(&ybcfg, rlk_record, opt_no, opt_id, fee1, fee2, fee3, fee4, fee5, setl_id);
									else
										b = opt_settle(&ybcfg, rlk_record, opt_no, opt_id, fee1, fee2, fee3, fee4, fee5, setl_id);
								}

								if (b)
								{
									set_cell_text(rlk_cache, clk_jsh, setl_id, -1);
									save_cache();
								}

								if (med_type == 21)
									b = ipt_settle_cancel(&ybcfg, rlk_record, opt_id, setl_id);
								else
									b = opt_settle_cancel(&ybcfg, rlk_record, opt_id, setl_id);

								if (b)
								{
									set_cell_text(rlk_cache, clk_jsh, NULL, 0);
									save_cache();
								}

								if (med_type == 21)
									b = ipt_discharge_cancel(&ybcfg, rlk_record, opt_no, opt_id);
								else
									b = 0;
							}
						}
						else 
						{
							if (psn_limt)
							{
								set_row_state(rlk_record, dsDelete);
							}
						}

						if (med_type == 21)
							ipt_fee_cancel(&ybcfg, rlk_record, opt_no, opt_id);
						else
							opt_fee_cancel(&ybcfg, rlk_record, opt_no, opt_id);
					}
					else
					{
						if (psn_limt)
						{
							set_row_state(rlk_record, dsDelete);
						}
					}
				}
				else
				{
					if (psn_limt)
					{
						set_row_state(rlk_record, dsDelete);
					}
				}
			}
			else
			{
				if (psn_limt)
				{
					set_row_state(rlk_record, dsDelete);
				}
			}

			if (!is_null(opt_id))
			{
				if (med_type == 21)
					b = ipt_reg_cancel(&ybcfg, rlk_record, opt_no, opt_id);
				else 	
					b = opt_reg_cancel(&ybcfg, rlk_record, opt_no, opt_id);

				if (b)
				{
					delete_row(rlk_cache);
					rlk_cache = NULL;
					save_cache();
				}
			}
			else
			{
				b = 0;
			}
		}
		
		if (!b)
		{
			get_last_error(err_code, err_text, ERR_LEN);
			_tprintf(_T("result: %s \n"), err_text);
		}
		else
		{
			_tprintf(_T("result: succeed! \n"));
		}

		if (get_row_state(rlk_record) == dsDelete)
		{
			if (psn_limt)
			{
				get_cell_text(rlk_record, clk_psn_flag, psn_key, 4);
				flk = find_string_entity(st_filter, psn_key, -1);
				if (flk)
				{
					psn_cnt = (int)get_string_entity_delta(flk);
					set_string_entity_delta(flk, (vword_t)(psn_cnt - 1));
				}
			}

			nxt = rlk_record;
			rlk_record = get_next_row(grid_record, rlk_record);
			delete_row(nxt);
		}
		else
		{
			rlk_record = get_next_row(grid_record, rlk_record);
		}
	}

	save_record();

	clean_record();

	clear_cache();

	if (sign_out(&ybcfg))
	{
		save_sign();
	}

	clear_sign();

	if (psn_limt)
	{
		destroy_string_table(st_filter);
	}

	clear_psn_idet_type();

	clear_psn_type();

	xdk_process_uninit();

#ifdef _OS_WINDOWS
	getch();
#endif

	return 0;
}