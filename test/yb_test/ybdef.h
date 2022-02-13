#ifndef _YBDEF_H
#define _YBDEF_H


#include <xdl.h>


#define CERT_TYPE_YBM	_T("01") //参保人员标识-电子凭证
#define CERT_TYPE_SFZ	_T("02") //参保人员标识-身份证
#define CERT_TYPE_YBK	_T("03") //参保人员标识-医保卡号

#define INSU_TYPE_ZG	_T("310") //城镇职工医疗保险
#define INSU_TYPE_JM	_T("390") //城乡居民医疗保险

#define MED_TYPE_MZ		_T("11") //就医类型-门诊
#define MED_TYPE_GH		_T("12") //就医类型-挂号
#define MED_TYPE_JZ		_T("13") //就医类型-急诊
#define MED_TYPE_GD		_T("140201") //就医类型-特慢病
#define MED_TYPE_MB		_T("15") //就医类型-两病
#define MED_TYPE_ZY		_T("21") //就医类型-住院
#define MED_TYPE_DB		_T("27") //就医类型-单病种
#define MED_TYPE_SS		_T("28") //就医类型-日间手术
#define MED_TYPE_YD		_T("41") //就医类型-药店购药

#define MATN_TYPE_PC	_T("1") //平产
#define MATN_TYPE_ZC	_T("2") //助娩产
#define MATN_TYPE_BC	_T("3") //剖宫产

#define BIRCTRL_TYPE_FH	_T("1") //放环
#define BIRCTRL_TYPE_QH	_T("2") //取环
#define BIRCTRL_TYPE_LC	_T("3") //流产
#define BIRCTRL_TYPE_YC	_T("4") //引产
#define BIRCTRL_TYPE_JY	_T("5") //绝育
#define BIRCTRL_TYPE_FY	_T("6") //复育

#define CATY_QK			_T("A02")
#define CATY_NK			_T("A03")
#define CATY_WK			_T("A04")

#define DIAG_TYPE_XY	_T("1") //西医诊断
#define DIAG_TYPE_ZB	_T("2") //中医主病
#define DIAG_TYPE_ZZ	_T("3") //中医主症
#define DIAG_TYPE_SS	_T("4") //手术操作

#define DISE_CODEG_TNB	_T("M01600")
#define DIAG_CODE_TNB	_T("E11.900") //糖尿病

typedef struct _dise_diag_table{
	tchar_t dise_code[NUM_LEN];
	tchar_t diag_code[NUM_LEN];
	tchar_t diag_name[RES_LEN];
}dise_diag_table;


#define PSN_SETLWAY_XM	_T("01") //按项目结算
#define PSN_SETLWAY_DE	_T("02") //按定额结算

#define DSCG_WAY_CY		_T("1") //医嘱出院
#define DSCG_WAY_ZY		_T("2") //医嘱转院
#define DSCG_WAY_SW		_T("5") //死亡出院

typedef struct _fee_case_t{
	tchar_t fee_codg[NUM_LEN];
	tchar_t fee_name[RES_LEN];
	tchar_t fee_sumamt[NUM_LEN];
	tchar_t cnt[NUM_LEN];
	tchar_t pric[NUM_LEN];
}fee_case_t;

static fee_case_t fee_case_mz[] = {
	{ _T("XB03ACY199B002010104718"), _T("右旋糖酐铁注射液"), _T("250000.00"), _T("5000"), _T("50.00") },
	{ _T("C02040205800005113250000023"), _T("植入式心脏起搏器+双腔"), _T("200000.00"), _T("20000"), _T("10.00") },
	//{ _T("003113000020000-31130000202"), _T("皮下软组织（或肿块）穿刺术"), _T("350000.00"), _T("3500"), _T("100.00") }
	{ _T("003107020070000-32040000600"), _T("无导线起搏器植入术"), _T("600000.00"), _T("6000"), _T("100.00") }
};
//C02040205800005113250000001-0000023 国械注进20143126199 植入式心脏起搏器+双腔
static fee_case_t fee_case_gd[] = {
	{ _T("XB03ACY199B002010104718"), _T("右旋糖酐铁注射液"), _T("200000.00"), _T("4000"), _T("50.00") },
	{ _T("C02040205800005113250000023"), _T("植入式心脏起搏器+双腔"), _T("200000.00"), _T("20000"), _T("10.00") },
	{ _T("003107020070000-32040000600"), _T("无导线起搏器植入术"), _T("600000.00"), _T("6000"), _T("100.00") }
};

static fee_case_t fee_case_zy[] = {
	{ _T("XB03ACY199B002010104718"), _T("右旋糖酐铁注射液"), _T("400000.00"), _T("8000"), _T("50.00") },
	{ _T("C02040205800005113250000023"), _T("植入式心脏起搏器+双腔"), _T("200000.00"), _T("20000"), _T("10.00") },
	//{ _T("003113000020000-31130000202"), _T("皮下软组织（或肿块）穿刺术"), _T("600000.00"), _T("6000"), _T("100.00") }
	{ _T("003107020070000-32040000600"), _T("无导线起搏器植入术"), _T("600000.00"), _T("6000"), _T("100.00") }
};

typedef struct _yb_config_t{
	tchar_t yb_addr[RES_LEN]; //医保服务地址头，如：http://172.16.33.244:80
	tchar_t mdt_admvs[NUM_LEN]; //就医地行政区划
	tchar_t ins_admvs[NUM_LEN]; //参保地行政区划
	tchar_t opt_code[NUM_LEN]; //经办人工号
	tchar_t opt_name[RES_LEN]; //经办人姓名
	tchar_t hos_code[NUM_LEN]; //定点机构代码
	tchar_t hos_name[RES_LEN]; //定点机构名称
	tchar_t hos_level[INT_LEN]; //定点机构等级

	tchar_t hos_key[KEY_LEN]; //签名公钥
	tchar_t hos_sec[KEY_LEN]; //签名私钥

	tchar_t med_type[INT_LEN]; //就医类型
	tchar_t psn_limt[INT_LEN]; //人员分组数
	tchar_t fee_setl[INT_LEN]; //是否结算
}yb_config_t;

typedef struct _yb_person_t{
	tchar_t cert_type[3]; //人员标识类型
	tchar_t cert_no[NUM_LEN]; //人员标识
	tchar_t psn_no[NUM_LEN]; //人员编号
	tchar_t psn_name[RES_LEN]; //人员性别
	tchar_t gend[3]; //人员性别

	tchar_t balc[NUM_LEN]; //个账余额
	tchar_t insu[NUM_LEN]; //个账余额
}yb_record_t;

#endif