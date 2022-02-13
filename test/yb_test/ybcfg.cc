
#include "ybcfg.h"

link_t_ptr grid_sign = NULL;
link_t_ptr grid_record = NULL;
link_t_ptr grid_cache = NULL;
link_t_ptr grid_psn_type = NULL;
link_t_ptr grid_psn_idet_type = NULL;

static dise_diag_table temp_diag[] = {
	{ _T("A16.701"), _T("a15.702"), _T("肺原发性结核性复征") },
	{ _T("M00101"), _T("a15.506"), _T("气管结核，细菌学") },
	{ _T("M00101"), _T("a15.000x016"), _T("继发性肺结核涂阳培阳") },
	{ _T("M00300"), _T("b24.x01"), _T("人类免疫缺陷病毒病") },
	{ _T("M00500"), _T("c49.900x003"), _T("软组织恶性肿瘤") },
	{ _T("M00501"), _T("c22.900"), _T("肝恶性肿瘤") },
	{ _T("M00599"), _T("c16.900"), _T("胃恶性肿瘤") },
	{ _T("M01102"), _T("d61.900"), _T("再生障碍性贫血") },
	{ _T("M01200"), _T("d66.x01"), _T("血友病A型") },
	{ _T("M02100"), _T("f20.000"), _T("偏执型精神分裂症") },
	{ _T("M02101"), _T("f20.800x001"), _T("难治性精神分裂症") },
	{ _T("M02207"), _T("f84.000x001"), _T("儿童孤独症") },
	{ _T("M02218"), _T("f25.900"), _T("分裂情感性障碍") },
	{ _T("M03900"), _T("i10.x09"), _T("原发性高血压") },
	{ _T("M05300"), _T("j44.900"), _T("慢性阻塞性肺病") },
	{ _T("M07101")	_T("m32.900"), _T("系统性红斑狼疮") },
	{ _T("M07801"), _T("n18.902"), _T("慢性肾衰竭") },
	{ _T("M08300"), _T("z94.002"), _T("异体肾移植状态") },
	{ _T("M80000/3"), _T("c71.900"), _T("脑恶性肿瘤") },
	{ _T("M83100/3"), _T("k92.800x012"), _T("胰腺癌伴出血") },
	{ _T("M85000/3"), _T("k92.800x010"), _T("胆管癌伴出血") },
	{ _T("M87200/3"), _T("c43.500"), _T("躯干恶性黑色素瘤") },
	{ _T("M88000/3"), _T("c92.300"), _T("髓样肉瘤") },
	{ _T("M88320/3"), _T("c81.900x005"), _T("皮肤霍奇金淋巴瘤") },
	{ _T("M88900/3"), _T("q85.900x027"), _T("平滑肌错构瘤") },
	{ _T("M90610/3"), _T("c90.200"), _T("髓外浆细胞瘤") },
	{ _T("M91800/3"), _T("q78.400x006"), _T("先天性多发性骨软骨瘤") },
	{ _T("M93800/3"), _T("q30.800x008"), _T("鼻神经胶质瘤") },
	{ _T("M93810/3"), _T("g95.800x016"), _T("脊髓胶质细胞增生") },
	{ _T("M95130/3"), _T("c69.200"), _T("视网膜恶性肿瘤") },
	{ _T("M96500/3"), _T("c81.200"), _T("混合细胞型霍奇金淋巴瘤") },
	{ _T("M98660/3"), _T("c92.402"), _T("急性髓细胞白血病，M3型") },
	{ _T("Z94.900"), _T("z94.900"), _T("器官和组织移植状态") },
	{ _T("Z99.201"), _T("z49.101"), _T("血液透析") }
};


bool_t find_dise_diag(const tchar_t* dise_code, tchar_t* diag_code, tchar_t* diag_name)
{
	int i, n = sizeof(temp_diag) / sizeof(dise_diag_table);

	for (i = 0; i < n; i++)
	{
		if (compare_text(dise_code, -1, temp_diag[i].dise_code, -1, 1) == 0)
		{
			xscpy(diag_code, temp_diag[i].diag_code);
			xscpy(diag_name, temp_diag[i].diag_name);
			return 1;
		}
	}

	return 0;
}

bool_t load_config(yb_config_t* pcfg)
{
	link_t_ptr ptr_ini = NULL;
	link_t_ptr sec;
	
	TRY_CATCH;

	ptr_ini = create_proper_doc();

	if (!load_proper_from_ini_file(ptr_ini, NULL, FILE_YB_CONFIG))
	{
		raise_user_error(_T("load_config"), _T("读取配置文件失败"));
	}

	sec = get_section(ptr_ini, _T("CFG"), -1);
	if (!sec)
	{
		raise_user_error(_T("load_config"), _T("读取配置[CFG]失败"));
	}

	read_proper(ptr_ini, _T("CFG"), -1, _T("yb_addr"), -1, pcfg->yb_addr, RES_LEN);
	read_proper(ptr_ini, _T("CFG"), -1, _T("mdt_admvs"), -1, pcfg->mdt_admvs, NUM_LEN);
	read_proper(ptr_ini, _T("CFG"), -1, _T("ins_admvs"), -1, pcfg->ins_admvs, NUM_LEN);
	read_proper(ptr_ini, _T("CFG"), -1, _T("opt_code"), -1, pcfg->opt_code, NUM_LEN);
	read_proper(ptr_ini, _T("CFG"), -1, _T("opt_name"), -1, pcfg->opt_name, RES_LEN);
	read_proper(ptr_ini, _T("CFG"), -1, _T("hos_code"), -1, pcfg->hos_code, NUM_LEN);
	read_proper(ptr_ini, _T("CFG"), -1, _T("hos_name"), -1, pcfg->hos_name, RES_LEN);
	read_proper(ptr_ini, _T("CFG"), -1, _T("hos_level"), -1, pcfg->hos_level, INT_LEN);

	read_proper(ptr_ini, _T("KEY"), -1, _T("hos_key"), -1, pcfg->hos_key, KEY_LEN);
	read_proper(ptr_ini, _T("KEY"), -1, _T("hos_sec"), -1, pcfg->hos_sec, KEY_LEN);

	read_proper(ptr_ini, _T("SET"), -1, _T("med_type"), -1, pcfg->med_type, INT_LEN);
	read_proper(ptr_ini, _T("SET"), -1, _T("psn_limt"), -1, pcfg->psn_limt, INT_LEN);
	read_proper(ptr_ini, _T("SET"), -1, _T("fee_setl"), -1, pcfg->fee_setl, INT_LEN);

	destroy_proper_doc(ptr_ini);
	ptr_ini = NULL;

	END_CATCH;

	return bool_true;
ONERROR:

	if (ptr_ini)
		destroy_proper_doc(ptr_ini);

	return bool_false;
}

bool_t load_sign()
{
	link_t_ptr clk, rlk;

	TRY_CATCH;

	grid_sign = create_grid_doc();

	if (xfile_info(NULL, FILE_YB_SIGN, NULL, NULL, NULL, NULL))
	{
		if (!load_grid_from_csv_file(grid_sign, 1, NULL, FILE_YB_SIGN))
		{
			raise_user_error(_T("load_sign"), _T("读取签到文件失败"));
		}
	}
	else
	{
		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("sign_no"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("sign_in_time"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("sign_out_time"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("sign_flag"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("stmt_begndate"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("stmt_enddate"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("medfee_sumamt"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("fund_pay_sumamt"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("acct_pay"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("fixmedins_setl_cnt"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("stmt_rslt"));

		clk = insert_col(grid_sign, LINK_LAST);
		set_col_name(clk, _T("stmt_rslt_dscr"));

		if (!save_grid_to_csv_file(grid_sign, 1, NULL, FILE_YB_SIGN))
		{
			raise_user_error(_T("load_sign"), _T("新建签到文件失败"));
		}
	}

	END_CATCH;

	return bool_true;
ONERROR:

	if (grid_sign)
	{
		destroy_grid_doc(grid_sign);

		grid_sign = NULL;
	}

	return bool_false;
}

bool_t save_sign()
{
	link_t_ptr clk, rlk;

	TRY_CATCH;

	if (!grid_sign)
	{
		raise_user_error(_T("save_sign"), _T("签到文件文件不存在"));
	}

	if (!save_grid_to_csv_file(grid_sign, 1, NULL, FILE_YB_SIGN))
	{
		raise_user_error(_T("save_sign"), _T("保存签到文件失败"));
	}

	END_CATCH;

	return bool_true;
ONERROR:

	return bool_false;
}

void clear_sign()
{
	if (grid_sign)
	{
		destroy_grid_doc(grid_sign);
		grid_sign = NULL;
	}
}

link_t_ptr loca_sign()
{
	XDK_ASSERT(grid_sign != NULL);

	tchar_t sign_flag[2] = { 0 };

	link_t_ptr clk_sign_no = get_col(grid_sign, _T("sign_no"));
	link_t_ptr clk_sign_flag = get_col(grid_sign, _T("sign_flag"));

	link_t_ptr rlk = get_next_row(grid_sign, LINK_FIRST);
	while (rlk)
	{
		get_cell_text(rlk, clk_sign_flag, sign_flag, 1);
		if (compare_text(sign_flag, -1, _T("1"), -1, 1) != 0)
		{
			return rlk;
		}

		rlk = get_next_row(grid_sign, rlk);
	}

	return NULL;
}

bool_t load_psn_type(void)
{
	link_t_ptr clk, rlk;

	TRY_CATCH;

	grid_psn_type = create_grid_doc();

	clk = insert_col(grid_psn_type, LINK_LAST);
	set_col_name(clk, _T("code")); //代码

	clk = insert_col(grid_psn_type, LINK_LAST);
	set_col_name(clk, _T("value")); //代码值

	if (xfile_info(NULL, FILE_YB_PSN_TYPE, NULL, NULL, NULL, NULL))
	{
		if (!load_grid_from_csv_file(grid_psn_type, 0, NULL, FILE_YB_PSN_TYPE))
		{
			raise_user_error(_T("load_psn_type"), _T("读取人员类别字典文件失败"));
		}
	}

	END_CATCH;

	return bool_true;
ONERROR:

	if (grid_psn_type)
	{
		destroy_grid_doc(grid_psn_type);

		grid_psn_type = NULL;
	}

	return bool_false;
}

void clear_psn_type(void)
{
	if (grid_psn_type)
	{
		destroy_grid_doc(grid_psn_type);

		grid_psn_type = NULL;
	}
}

bool_t load_psn_idet_type(void)
{
	link_t_ptr clk, rlk;

	TRY_CATCH;

	grid_psn_idet_type = create_grid_doc();

	clk = insert_col(grid_psn_idet_type, LINK_LAST);
	set_col_name(clk, _T("code")); //代码

	clk = insert_col(grid_psn_idet_type, LINK_LAST);
	set_col_name(clk, _T("value")); //代码值

	if (xfile_info(NULL, FILE_YB_PSN_IDET_TYPE, NULL, NULL, NULL, NULL))
	{
		if (!load_grid_from_csv_file(grid_psn_idet_type, 0, NULL, FILE_YB_PSN_IDET_TYPE))
		{
			raise_user_error(_T("load_psn_idet_type"), _T("读取身份类别字典文件失败"));
		}
	}

	END_CATCH;

	return bool_true;
ONERROR:

	if (grid_psn_idet_type)
	{
		destroy_grid_doc(grid_psn_idet_type);

		grid_psn_idet_type = NULL;
	}

	return bool_false;
}

void clear_psn_idet_type(void)
{
	if (grid_psn_idet_type)
	{
		destroy_grid_doc(grid_psn_idet_type);

		grid_psn_idet_type = NULL;
	}
}

bool_t load_record()
{
	link_t_ptr clk, rlk;

	TRY_CATCH;

	grid_record = create_grid_doc();

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("mdt_admvs")); //就医地

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("cert_no")); //身份证

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_no")); //个人编号

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_name")); //姓名

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("gend")); //性别

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("opsp_code")); //备案病种编码

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("opsp_name")); //备案病种名称

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hos_code")); //机构代码

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hos_name")); //机构名称

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hos_level")); //机构等级

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("med_type")); //就医类型

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("dise_codg")); //病种编码

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("dise_name")); //病种名称

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("oprt_code")); //手术代码

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("oprt_name")); //手术名称

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("matn_type")); //生育类别

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("birctrl_type")); //计划生育手术类别

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("ins_admvs")); //参保地

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("insutype")); //险种类型

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_type")); //人员类别

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_flag")); //人员标志

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_idet_type")); //人员身份类别

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_type_memo")); //人员类别说明

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_memo")); //身份类别说明

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("year_hifp_line_sum")); //统筹起付线累计

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("year_hifp_pay_sum")); //统筹基金支出累计

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("year_hifmi_line_sum")); //大病起付线累计

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("year_hifmi_pay_sum")); //大病基金支出累计

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("year_cvlserv_pay")); //公务员支出累计

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("year_hifdm_pay")); //伤残基金支出累计

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("year_maf_pay")); // 医疗救助支出累计

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("acct")); //个账

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("acct_crt")); //个账当年

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("acct_cal")); //个账历年

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("query_info")); //查询返回消息

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("medfee_sumamt")); //医疗费总额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("fulamt_ownpay_amt")); //全自费金额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("overlmt_selfpay")); //超限价自费费用

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("preselfpay_amt")); //先行自付金额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("inscp_scp_amt")); //符合政策范围金额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("act_pay_dedc")); //实际支付起付线

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hifp_pay")); //统筹基金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("pool_prop_selfpay")); //统筹基金支付比例

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("cvlserv_pay")); //公务员医疗补助资金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hifes_pay")); //企业补充医疗保险基金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hifmi_pay")); //大病保险资金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hifob_pay")); //职工大额医疗费用补助基金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hifdm_pay")); //伤残人员医疗保障基金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("oth_pay")); //其他支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("maf_pay")); //医疗救助基金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("othfund_pay")); //其他商保基金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("fund_pay_sumamt")); //基金支付总额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_part_amt")); //个人负担总金额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("acct_pay")); //个人账户支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("psn_cash_pay")); //个人现金支出

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("balc")); //个账余额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("acct_mulaid_pay")); //个人账户共济支付金额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("hosp_part_amt")); //医院负担金额

	clk = insert_col(grid_record, LINK_LAST);
	set_col_name(clk, _T("calc_info")); //结算返回消息

	if (xfile_info(NULL, FILE_YB_RECORD, NULL, NULL, NULL, NULL))
	{
		if (!load_grid_from_csv_file(grid_record, 0, NULL, FILE_YB_RECORD))
		{
			raise_user_error(_T("load_record"), _T("读取批量文件失败"));
		}
	}

	END_CATCH;

	return bool_true;
ONERROR:

	if (grid_record)
	{
		destroy_grid_doc(grid_record);

		grid_record = NULL;
	}

	return bool_false;
}

bool_t save_record()
{
	link_t_ptr clk, rlk;

	TRY_CATCH;

	if (!grid_record)
	{
		raise_user_error(_T("save_record"), _T("记录文件文件不存在"));
	}

	if (!save_grid_to_csv_file(grid_record, 0, NULL, FILE_YB_RECORD))
	{
		raise_user_error(_T("save_record"), _T("保存记录文件失败"));
	}

	END_CATCH;

	return bool_true;
ONERROR:

	return bool_false;
}

void clean_record()
{
	if (grid_record)
	{
		destroy_grid_doc(grid_record);
		grid_record = NULL;
	}
}

bool_t load_cache()
{
	link_t_ptr clk, rlk;

	TRY_CATCH;

	grid_cache = create_grid_doc();

	clk = insert_col(grid_cache, LINK_LAST);
	set_col_name(clk, _T("med")); //医疗类别

	clk = insert_col(grid_cache, LINK_LAST);
	set_col_name(clk, _T("bxh")); //保险号

	clk = insert_col(grid_cache, LINK_LAST);
	set_col_name(clk, _T("bah")); //病案号

	clk = insert_col(grid_cache, LINK_LAST);
	set_col_name(clk, _T("jzh")); //就诊号

	clk = insert_col(grid_cache, LINK_LAST);
	set_col_name(clk, _T("jsh")); //结算号

	if (xfile_info(NULL, FILE_YB_CACHE, NULL, NULL, NULL, NULL))
	{
		if (!load_grid_from_csv_file(grid_cache, 0, NULL, FILE_YB_CACHE))
		{
			raise_user_error(_T("load_cache"), _T("读取缓存文件失败"));
		}
	}
	

	END_CATCH;

	return bool_true;
ONERROR:

	if (grid_cache)
	{
		destroy_grid_doc(grid_cache);

		grid_cache = NULL;
	}

	return bool_false;
}

bool_t save_cache()
{
	link_t_ptr clk, rlk;

	TRY_CATCH;

	if (!grid_cache)
	{
		raise_user_error(_T("save_cache"), _T("缓存文件文件不存在"));
	}

	if (!save_grid_to_csv_file(grid_cache, 0, NULL, FILE_YB_CACHE))
	{
		raise_user_error(_T("save_cache"), _T("保存缓存文件失败"));
	}

	END_CATCH;

	return bool_true;
ONERROR:

	return bool_false;
}

void clear_cache()
{
	if (grid_cache)
	{
		destroy_grid_doc(grid_cache);
		grid_cache = NULL;
	}
}