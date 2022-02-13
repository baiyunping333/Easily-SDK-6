
#include <xdl.h>
//#include <tdb.h>

#ifdef _OS_WINDOWS
#include <conio.h>
#endif

void test_path()
{
	tchar_t pfmt[] = _T("$(XSERVICE_ROOT)/api/%s");
	tchar_t *path;

	int len = printf_path(NULL, pfmt, _T("loc_api.dll"));
	path = xsalloc(len + 1);
	printf_path(path, pfmt, _T("loc_api.dll"));

	xsfree(path);
}

void test_path2()
{
	tchar_t pfmt[] = _T("$(XSERVICE_DATA)/api");
	tchar_t *path;

	int len = printf_path(NULL, pfmt);
	path = xsalloc(len + 1);
	printf_path(path, pfmt);

	xsfree(path);
}

void test_json()
{
	byte_t utf_buf[100] = { 0 };
	schar_t mbs_buf[100] = { 0 };
	dword_t n;
	
	a_xscpy((schar_t*)utf_buf, "{\"demo\":\"{the \\\"demo\\\"}\",\"in\":[1,2,3],\"out\":null}");
	n = a_xslen((schar_t*)utf_buf);

	link_t_ptr json = create_json_doc();
	parse_json_doc_from_bytes(json, utf_buf, n, _UTF8);
	save_dom_doc_to_file(json, NULL, _T("json.xml"));
	format_json_doc_to_bytes(json, (byte_t*)mbs_buf, 100, _UTF8);
	destroy_json_doc(json);
}

typedef int(__stdcall *pf_zj_hmac_SM3)(char* key, char* secret, char* unix_timestamp, char* request_body, char* outMsg);


void zj_mac_sm3(char* key, char* secret, char* unix_timestamp, char* request_body, char* outMsg)
{
	res_modu_t hlib = load_library(_T("CardReaderDLL.dll"));
	pf_zj_hmac_SM3 pf = (pf_zj_hmac_SM3)get_address(hlib, "ZJ_Hmac_SM3");

	(*pf)(key, secret, unix_timestamp, request_body, outMsg);

	free_library(hlib);
}

void gb_mac_sm3(char* key, char* secret, char* unix_timestamp, char* request_body, char* outMsg)
{
	byte_t out_bin[32];
	int i, n;
	char* sin_buf;
	
	n = a_xslen(unix_timestamp) + 1 + a_xslen(request_body);
	sin_buf = (char*)a_xsalloc(n + 1);

	a_xscpy(sin_buf, unix_timestamp);
	a_xscat(sin_buf, "\n");
	a_xscat(sin_buf, request_body);
	sm3_hmac((byte_t*)secret, a_xslen(secret), (byte_t*)sin_buf, a_xslen(sin_buf), out_bin);

	a_xsfree(sin_buf);

	a_xscpy(outMsg, unix_timestamp);
	a_xscat(outMsg, ":");
	int k = a_xslen(outMsg);

	for (i = 0; i < 32; i++)
	{
		sprintf((char*)(outMsg + k + i * 2), "%02X", out_bin[i]);
	}
}

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

const char* str_key = "H33018300564";
const char* str_sec = "fAtUGnYYI8ywzIz";
const tchar_t* ins_key = _T("H33018300564");
const tchar_t* inf_time = _T("2022-01-21 10:00:00");
const tchar_t* admvs = _T("330100");
const tchar_t* ins_code = _T("H33018300564");
const tchar_t* ins_name = _T("杭州市第二测试医院");
const tchar_t* opter_no	= _T("123");
const tchar_t* opter_name = _T("测试");
const tchar_t* cert_type = _T("02");

const tchar_t* opt_cert_no = _T("330100198312290966");
const tchar_t* opt_psn_no = _T("33010000000000000001139482");
const tchar_t* opt_psn_name = _T("沈芳");

const tchar_t* sign_in_url = _T("http://172.16.33.244/fsi/api/signInSignOutService/signIn");

void sign_in()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("9001"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, _T("1"));

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("signIn"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("opter_no"));
	set_json_item_value(nlk, opter_no);

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

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char out_buf[100] = { 0 };
	zj_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)in_buf, out_buf);
	//gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), sign_in_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("sign_in.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

//msgid	0x0050f784 L"H33018300564202201211447429999"	wchar_t[31]

// L"{\"output\":{\"signinoutb\":{\"sign_no\":\"680003\",\"sign_time\":\"2022-01-21 00:00:00\"}},\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":\"success\",\"refmsg_time\":\"20220121144742293\",\"signtype\":null,\"respond_time\":\"20220121144742303\",\"inf_refmsgid\":\"330000202201211447420159618450\"}"	wchar_t[512]

const tchar_t* sign_no = _T("680003");

const tchar_t* sign_out_url = _T("http://172.16.33.244/fsi/api/signInSignOutService/signOut");

void sign_out()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("9002"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));
	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("signOut"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("opter_no"));
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	char in_buf[1024] = { 0 };
	format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char out_buf[100] = { 0 };
	zj_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)in_buf, out_buf);
	//gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), sign_out_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, _T("H33018300564"), -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("sign_out.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"output\":{\"signoutoutb\":{\"sign_time\":\"2022-01-21 14:08:37\"}},\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":\"success\",\"refmsg_time\":\"20220121140837683\",\"signtype\":null,\"respond_time\":\"20220121140837694\",\"inf_refmsgid\":\"330000202201211408370159492622\"}"	wchar_t[512]

const tchar_t* file_up_url = _T("http://172.16.33.244/fsi/api/fileupload/upload");

void file_up()
{
	res_modu_t hlib = load_library(_T("CardReaderDLL.dll"));
	pf_zj_hmac_SM3 pf = (pf_zj_hmac_SM3)get_address(hlib, "ZJ_Hmac_SM3");

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("9101"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, _T("63000000000000000000000008"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("fsUploadIn"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("in")); //
	set_json_item_value(nlk, _T("[]"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("filename")); //
	set_json_item_value(nlk, _T("202109171261324718385317321.txt.zip"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code")); //
	set_json_item_value(nlk, ins_code);

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	char in_buf[1024] = { 0 };
	format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	(*pf)((char*)str_key, (char*)str_sec, str_tm, (char*)in_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	free_library(hlib);

	xhand_t xhttp = xhttp_client(_T("POST"), file_up_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("file_up.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

const tchar_t* file_down_url = _T("http://172.16.33.244/fsi/api/fileupload/download");

void file_down()
{
	res_modu_t hlib = load_library(_T("CardReaderDLL.dll"));
	pf_zj_hmac_SM3 pf = (pf_zj_hmac_SM3)get_address(hlib, "ZJ_Hmac_SM3");

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("9102"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, _T("63000000000000000000000008"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("fsDownloadIn"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("filename")); //
	set_json_item_value(nlk, _T("202109171261324718385317321.txt.zip"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("file_qury_no")); //
	set_json_item_value(nlk, _T("M00/00/00/ClAJnmFIJ5SAZ1oeAAC082_B5y05025219"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code")); //
	set_json_item_value(nlk, ins_code);

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	char in_buf[1024] = { 0 };
	format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	(*pf)((char*)str_key, (char*)str_sec, str_tm, (char*)in_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	free_library(hlib);

	xhand_t xhttp = xhttp_client(_T("POST"), file_down_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("file_down.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

const tchar_t* query_fixins_url = _T("http://172.16.33.244/fsi/api/fsiFixMedInsService/queryFixMedIns");

void query_fixins()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("1201"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("medinsInfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_type"));
	set_json_item_value(nlk, _T("1"));

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	char in_buf[1024] = { 0 };
	format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	zj_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)in_buf, out_buf);
	//gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);
	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), query_fixins_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("query_fixins.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"output\":{\"medinsinfo\":[{\"fixmedins_code\":\"H33018300564\",\"uscc\":null,\"fixmedins_name\":\"杭州市第二测试医院\",\"fixmedins_type\":\"1\",\"hosp_lv\":\"02\"}]},\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":null,\"refmsg_time\":\"20220121102254426\",\"signtype\":null,\"respond_time\":\"20220121102254438\",\"inf_refmsgid\":\"330000202201211022540158610021\"}"	wchar_t[512]


const tchar_t* query_psninfo_url = _T("http://172.16.33.244/fsi/api/fsiPsnInfoService/queryPsnInfo");

void query_psninfo()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("1101"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));
	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, cert_type);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no"));
	set_json_item_value(nlk, opt_cert_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("begntime"));
	set_json_item_value(nlk, _T("2020-01-01"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[4096] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	char out_buf[100] = { 0 };
	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	//char out_buf[100] = { 0 };
	//zj_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)in_buf, out_buf);
	
	dword_t n_sin = a_xslen((char*)out_buf);
	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), query_psninfo_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("query_psninfo.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"output\":{\"idetinfo\":[],\"baseinfo\":{\"certno\":\"330100198312290966\",\"psn_no\":\"33010000000000000001139482\",\"gend\":\"2\",\"exp_content\":\"{\\\"crtYearBalc\\\":0,\\\"calYearBalc\\\":0}\",\"brdy\":\"1998-01-28\",\"naty\":\"01\",\"psn_cert_type\":\"01\",\"psn_name\":\"沈芳\",\"age\":21.0},\"insuinfo\":[{\"insuplc_admdvs\":\"330100\",\"psn_insu_date\":\"2019-07-01\",\"cvlserv_flag\":\"0\",\"balc\":0,\"emp_name\":\"杭州新天地户外用品有限公司\",\"psn_type\":\"1101\",\"psn_insu_stas\":\"1\",\"insutype\":\"310\",\"paus_insu_date\":null}]},\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":null,\"...	wchar_t[512]

const tchar_t* opt_no = _T("20220121");
const tchar_t* opt_date = _T("2022-01-21");

const tchar_t* opt_reg_url = _T("http://172.16.33.244/fsi/api/outpatientDocInfoService/outpatientRregistration");

void opt_reg()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2201"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));
	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype"));
	set_json_item_value(nlk, _T("310"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("begntime"));
	set_json_item_value(nlk, _T("2022-01-21 10:00:00"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, cert_type);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no"));
	set_json_item_value(nlk, opt_cert_no);

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
	set_json_item_value(nlk, _T("A13"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[4096] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);
	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_reg_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_reg.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

//msgid	0x00eefd44 L"H33018300564202201211218479999"	wchar_t[31]

// L"{\"output\":{\"data\":{\"psn_no\":\"33010000000000000001139482\",\"mdtrt_id\":\"330000164273872766400005212141\",\"exp_content\":null,\"ipt_otp_no\":\"20220121\"}},\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":null,\"refmsg_time\":\"20220121121847653\",\"signtype\":null,\"respond_time\":\"20220121121847816\",\"inf_refmsgid\":\"330000202201211218470159199016\"}"	wchar_t[512]


const tchar_t* opt_id = _T("330000164273872766400005212141");

const tchar_t* opt_reg_cancel_url = _T("http://172.16.33.244/fsi/api/outpatientDocInfoService/outpatientRegistrationCancel");

void opt_reg_cancel()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2202"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));
	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("ipt_otp_no"));
	set_json_item_value(nlk, opt_no);


	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);
	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_reg_cancel_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_reg_cancel.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"output\":null,\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":null,\"refmsg_time\":\"20220120173329875\",\"signtype\":null,\"respond_time\":\"20220120173329926\",\"inf_refmsgid\":\"330000202201201733290154282833\"}"	wchar_t[512]


const tchar_t* opt_trt_up_url = _T("http://172.16.33.244/fsi/api/outpatientDocInfoService/outpatientMdtrtinfoUpA");

void opt_trt_up()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2203A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

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
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type"));
	set_json_item_value(nlk, _T("11"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("begntime"));
	set_json_item_value(nlk, opt_date);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("main_cond_dscr"));
	set_json_item_value(nlk, _T("病情描述"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_codg"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_name"));
	set_json_item_value(nlk, _T("病种名称"));

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
	set_json_item_value(nlk, _T("46"));

	clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("diseinfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_type"));
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_srt_no"));
	set_json_item_value(nlk, _T("02"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_code"));
	set_json_item_value(nlk, _T("02"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_name"));
	set_json_item_value(nlk, _T("诊断名称"));

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
	set_json_item_value(nlk, opt_date);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("vali_flag"));
	set_json_item_value(nlk, _T("01"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_trt_up_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_trt_up.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

//msgid	0x0086f794 L"H33018300564202201211221129999"	wchar_t[31]

// L"{\"output\":null,\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":null,\"refmsg_time\":\"20220121122113002\",\"signtype\":null,\"respond_time\":\"20220121122113067\",\"inf_refmsgid\":\"330000202201211221130159203983\"}"	wchar_t[512]

const tchar_t* opt_fee_no = _T("2022-01-21");

const tchar_t* opt_fee_up_url = _T("http://172.16.33.244/fsi/api/outpatientDocInfoService/outpatientFeeListUp");

void opt_fee_up()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2204"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("feedetail"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("feedetl_sn")); //单次就诊内唯一
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id"));
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("chrg_bchno")); //同一收费批次号病种编号必须一致
	set_json_item_value(nlk, opt_fee_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_codg"));
	set_json_item_value(nlk, _T("M01607"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("rxno")); //外购处方时，传入外购处方的处方号；非外购处方，传入医药机构处方号
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("rx_circ_flag"));
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fee_ocur_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_list_codg"));
	set_json_item_value(nlk, _T("XJ01CAA040A001010102699")); //医疗目录编码

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medins_list_codg"));
	set_json_item_value(nlk, _T("XJ01CAA040A001010102699")); //医药机构目录编码

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("det_item_fee_sumamt")); //明细项目费用总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("cnt")); //数量
	set_json_item_value(nlk, _T("10"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("pric")); //单价
	set_json_item_value(nlk, _T("120"));

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

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[4096] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);
	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_fee_up_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_fee_up.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

//8 L"{\"output\":{\"result\":[{\"bas_medn_flag\":\"0\",\"med_chrgitm_type\":\"09\",\"det_item_fee_sumamt\":1200.00,\"hi_nego_drug_flag\":\"0\",\"fulamt_ownpay_amt\":1200,\"cnt\":10,\"pric\":120,\"exp_content\":null,\"memo\":null,\"feedetl_sn\":\"33010000000000000001139482\",\"inscp_scp_amt\":0,\"drt_reim_flag\":\"0\",\"overlmt_amt\":0,\"list_sp_item_flag\":null,\"pric_uplmt_amt\":99999999.000000,\"selfpay_prop\":1.0000,\"chld_medc_flag\":null,\"preselfpay_amt\":0.00,\"lmt_used_flag\":\"0\",\"chrgitm_lv\":\"01\"}]},\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":nul...	wchar_t[512]

const tchar_t* opt_fee_cancel_url = _T("http://172.16.33.244/fsi/api/outpatientDocInfoService/outpatientFeeListUpCancel");

void opt_fee_cancel()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2205"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

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
	set_json_item_value(nlk, opt_psn_no);


	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_fee_cancel_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_fee_cancel.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"output\":null,\"infcode\":0,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":null,\"refmsg_time\":\"20220121123641800\",\"signtype\":null,\"respond_time\":\"20220121123641841\",\"inf_refmsgid\":\"330000202201211236410159309144\"}"	wchar_t[512]

const tchar_t* opt_fee_preset_url = _T("http://172.16.33.244/fsi/api/outpatientSettleService/preSettletmentA");

void opt_fee_preset()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2206A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));
	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, cert_type);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no")); //就诊凭证类型为“01”时填写电子凭证令牌，为“02”时填写身份证号，为“03”时填写社会保障卡卡号
	set_json_item_value(nlk, opt_cert_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, _T("11"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_setlway")); //个人结算方式
	set_json_item_value(nlk, _T("01"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("chrg_bchno")); //收费批次号
	set_json_item_value(nlk, opt_fee_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_used_flag")); //个人账户使用标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种类型
	set_json_item_value(nlk, _T("310"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_fee_preset_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_fee_preset.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

//msgid	0x0047fca8 L"H33018300564202201211255559999"	wchar_t[31]

// L"{\"output\":{\"setlinfo\":{\"setl_time\":null,\"cvlserv_pay\":0,\"hifdm_pay\":0,\"cvlserv_flag\":\"0\",\"med_type\":\"11\",\"exp_content\":null,\"brdy\":\"1998-01-28\",\"naty\":\"01\",\"psn_cash_pay\":1200.00,\"certno\":\"330100198312290966\",\"hifmi_pay\":0,\"psn_no\":\"33010000000000000001139482\",\"act_pay_dedc\":0,\"mdtrt_cert_type\":\"02\",\"balc\":0,\"medins_setl_id\":\"H33018300564202201211255559999\",\"psn_cert_type\":\"01\",\"acct_mulaid_pay\":0,\"clr_way\":null,\"hifob_pay\":0,\"oth_pay\":0,\"medfee_sumamt\":1200.00,\"hifes_pay\":0,\"gend\":\"2\",\"mdtrt_id\":\"330000164...	wchar_t[512]


const tchar_t* opt_fee_settle_url = _T("http://172.16.33.244/fsi/api/outpatientSettleService/saveSettletmentA");

void opt_fee_settle()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2207A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));
	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type")); //就诊凭证类型
	set_json_item_value(nlk, cert_type);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no")); //就诊凭证类型为“01”时填写电子凭证令牌，为“02”时填写身份证号，为“03”时填写社会保障卡卡号
	set_json_item_value(nlk, opt_cert_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, _T("11"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_setlway")); //个人结算方式
	set_json_item_value(nlk, _T("01"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, opt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("chrg_bchno")); //收费批次号
	set_json_item_value(nlk, opt_fee_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_used_flag")); //个人账户使用标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种类型
	set_json_item_value(nlk, _T("310"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("invono")); //发票号
	set_json_item_value(nlk, opt_fee_no); 

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fulamt_ownpay_amt")); //全自费金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("overlmt_selfpay")); //超限价金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("preselfpay_amt")); //先行自付金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("inscp_scp_amt")); //符合政策范围金额
	set_json_item_value(nlk, _T("1200"));

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_fee_settle_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[4096] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, 4096);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_fee_settle.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

//msgid	0x00a6fad4 L"H33018300564202201211306599999"	wchar_t[31]

// L"{\"output\":{\"setlinfo\":{\"setl_time\":\"2022-01-21 13:06:59\",\"cvlserv_pay\":0,\"hifdm_pay\":0,\"cvlserv_flag\":\"0\",\"med_type\":\"11\",\"exp_content\":null,\"brdy\":\"1998-01-28\",\"naty\":\"01\",\"psn_cash_pay\":1200.00,\"certno\":\"330100198312290966\",\"hifmi_pay\":0,\"psn_no\":\"33010000000000000001139482\",\"act_pay_dedc\":0,\"mdtrt_cert_type\":\"02\",\"balc\":0,\"medins_setl_id\":\"H33018300564202201211306599999\",\"psn_cert_type\":\"01\",\"acct_mulaid_pay\":0,\"clr_way\":\"1\",\"hifob_pay\":0,\"oth_pay\":0,\"medfee_sumamt\":1200.00,\"hifes_pay\":0,\"gend\":\"2\",\"mdtr...	wchar_t[512]

const tchar_t* opt_settle_id = _T("330000164274162139200002757001");

const tchar_t* opt_settle_cancel_url = _T("http://172.16.33.244/fsi/api/outpatientSettleService/cancleSettletment");

void opt_settle_cancel()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2208"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("setl_id")); //结算ID
	set_json_item_value(nlk, opt_settle_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, opt_id);

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_settle_cancel_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_settle_cancel.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

//msgid	0x007bf7d8 L"H33018300564202201211318299999"	wchar_t[31]

// L"{\"output\":{\"setlinfo\":{\"setl_time\":\"2022-01-21 13:18:29\",\"cvlserv_pay\":0.00,\"hifdm_pay\":0.00,\"cvlserv_flag\":\"0\",\"med_type\":\"11\",\"exp_content\":null,\"brdy\":\"1998-01-28\",\"naty\":\"01\",\"psn_cash_pay\":-1200.00,\"certno\":\"330100198312290966\",\"hifmi_pay\":0.00,\"psn_no\":\"33010000000000000001139482\",\"act_pay_dedc\":0.00,\"mdtrt_cert_type\":\"02\",\"balc\":0.00,\"medins_setl_id\":\"H33018300564202201211318299999\",\"psn_cert_type\":\"01\",\"acct_mulaid_pay\":0.00,\"clr_way\":\"1\",\"hifob_pay\":0.00,\"oth_pay\":0.00,\"medfee_sumamt\":-1200.00,\"hif...	wchar_t[512]

const tchar_t* opt_msg_id = _T("H33018300564202201211306599999");

const tchar_t* opt_revs_url = _T("http://172.16.33.244/fsi/api/reverseService/revsMethod");

void opt_revs()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2601"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("omsgid")); //原发送方报文ID
	set_json_item_value(nlk, opt_msg_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("oinfno")); //原交易编号
	set_json_item_value(nlk, _T("2207A"));

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_revs_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_revs.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"output\":null,\"infcode\":-1,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":\"FSI-调用业务基础子系统冲正失败:冲正的结算信息退费结算标志不为未退费，不能办理冲正交易[iptPsnSetlMgtBO_4412][outpatientSettleBO_40]\",\"refmsg_time\":\"20220121132404797\",\"signtype\":null,\"respond_time\":\"20220121132404825\",\"inf_refmsgid\":null}"	wchar_t[512]

const tchar_t* opt_stmt_url = _T("http://172.16.33.244/fsi/api/ybSettlementStmtService/stmt");

void opt_stmt()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("3201"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种
	set_json_item_value(nlk, _T("310"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("clr_type")); //清算类别
	set_json_item_value(nlk, _T("11"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("setl_optins")); //结算经办机构
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("stmt_begndate")); //对账开始日期
	set_json_item_value(nlk, _T("2022-01-21"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("stmt_enddate")); //对账结束日期
	set_json_item_value(nlk, _T("2022-01-21 23:59:59"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fund_pay_sumamt")); //基金支付总额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_pay")); //个人账户支付金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_setl_cnt")); //定点医药机构结算笔数
	set_json_item_value(nlk, _T("1"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_revs_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_stmt.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"type\":\"error\",\"infcode\":-1,\"err_msg\":\"FSI-FSI-交易报文不正确：未知交易编号！,异常流水号:1972450481\"}"	wchar_t[512]

const tchar_t* opt_stmt_detail_url = _T("http://172.16.33.244/fsi/api/ybSettlementStmtService/stmtDetail");

void opt_stmt_detail()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("3202"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("setl_optins")); //结算经办机构
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("stmt_begndate")); //对账开始日期
	set_json_item_value(nlk, _T("2022-01-21"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("stmt_enddate")); //对账结束日期
	set_json_item_value(nlk, _T("2022-01-21 23:59:59"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fund_pay_sumamt")); //基金支付总额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("cash_payamt")); //现金支付总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_setl_cnt")); //定点医药机构结算笔数
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("file_qury_no")); //文件查询号
	set_json_item_value(nlk, _T("M00/00/00/ClAJnmFQBA6AA1AqAAAA3DTX59s6978876"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), opt_revs_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("out_stmt_detail.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"type\":\"error\",\"infcode\":-1,\"err_msg\":\"FSI-FSI-交易报文不正确：未知交易编号！,异常流水号:1972050300\"}"	wchar_t[512]

const tchar_t* ipt_cert_no = _T("330100198312296188");
const tchar_t* ipt_psn_no = _T("33010000000000000001123482");
const tchar_t* ipt_psn_name = _T("鲁智深");

const tchar_t* ipt_no = _T("20220121");
const tchar_t* ipt_date = _T("2022-01-21");

const tchar_t* ipt_reg_url = _T("http://172.16.33.244/fsi/api/hospitalRegisterService/hospitalRegisterSave");

void ipt_reg()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2401"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("mdtrtinfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype"));
	set_json_item_value(nlk, _T("310"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("coner_name")); //联系人姓名
	set_json_item_value(nlk, _T("联系人"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("tel")); //联系电话
	set_json_item_value(nlk, _T("11111111111"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("begntime"));
	set_json_item_value(nlk, _T("2022-01-21 10:00:00"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, cert_type);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no"));
	set_json_item_value(nlk, opt_cert_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, _T("21"));

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
	set_json_item_value(nlk, _T("A01.000"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dscg_maindiag_name")); //住院主诊断名称
	set_json_item_value(nlk, _T("诊断名称"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("main_cond_dscr")); //主要病情描述
	set_json_item_value(nlk, _T("病情描述"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_codg")); //病种编码
	set_json_item_value(nlk, _T("BA01000"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_name")); //病种名称
	set_json_item_value(nlk, _T("病种名称"));

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
	set_json_item_value(nlk, ipt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_type")); //诊断类别
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("maindiag_flag")); //主诊断标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_srt_no")); //诊断排序号
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_code")); //诊断代码
	set_json_item_value(nlk, _T("A01.000"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_name")); //诊断名称
	set_json_item_value(nlk, _T("诊断名称"));

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
	set_json_item_value(nlk, _T("2022-01-21 10:00:00"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[4096] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);
	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_reg_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_reg.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

// L"{\"output\":null,\"infcode\":-1,\"warn_msg\":null,\"cainfo\":null,\"err_msg\":\"FSI-FMI返回：该人员是在院状态不能办理入院，在院信息：机构编号：H33018300563机构名称：杭州市第一测试医院HospitalRegisterBO\",\"refmsg_time\":\"20220121145507584\",\"signtype\":null,\"respond_time\":\"20220121145507700\",\"inf_refmsgid\":null}"	wchar_t[512]

const tchar_t* ipt_id = _T("");

const tchar_t* ipt_reg_cancel_url = _T("http://172.16.33.244/fsi/api/hospitalRegisterService/hospitalRegisterCancel");

void ipt_reg_cancel()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2404"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

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
	set_json_item_value(nlk, ipt_psn_no);

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);
	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_reg_cancel_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_reg_cancel.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

const tchar_t* ipt_fee_no = _T("2022-01-21");

const tchar_t* ipt_fee_up_url = _T("http://172.16.33.244/fsi/api/hospFeeDtlService/feeDtlUp");

void ipt_fee_up()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2301"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("feedetail"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("feedetl_sn")); //费用明细流水号,单次就诊内唯一
	set_json_item_value(nlk, opt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("init_feedetl_sn")); //原费用流水号,退单时传入被退单的费用明细流水号
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("drord_no")); //医嘱号
	set_json_item_value(nlk, ipt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no")); //人员编号
	set_json_item_value(nlk, ipt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, _T("21"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fee_ocur_time")); //费用发生时间
	set_json_item_value(nlk, _T("2022-01-21 10 :00:00"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_list_codg")); //医疗目录编码
	set_json_item_value(nlk, _T("ZA06BBX0548010100293"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medins_list_codg")); //医药机构目录编码
	set_json_item_value(nlk, _T("ZA06BBX0548010100293"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("det_item_fee_sumamt")); //明细项目费用总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("cnt"));
	set_json_item_value(nlk, _T("1")); //数量

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("pric"));
	set_json_item_value(nlk, _T("120")); //单价

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("det_item_fee_sumamt")); //明细项目费用总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("cnt")); //数量
	set_json_item_value(nlk, _T("10"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("pric")); //单价
	set_json_item_value(nlk, _T("120"));

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
	set_json_item_name(nlk, _T("acord_dept_name")); //开单科室名称
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

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[4096] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);

	dword_t n_sin = a_xslen((char*)out_buf);
	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_fee_up_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_fee_up.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

const tchar_t* ipt_fee_cancel_url = _T("http://172.16.33.244/fsi/api/hospFeeDtlService/feeDtlCl");

void ipt_fee_cancel()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2302"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

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
	set_json_item_value(nlk, ipt_psn_no);


	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_fee_cancel_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_fee_cancel.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}


const tchar_t* ipt_fee_preset_url = _T("http://172.16.33.244/fsi/api/hospSettService/preSettA");

void ipt_fee_preset()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2303A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, ipt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type"));
	set_json_item_value(nlk, cert_type);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no")); //就诊凭证类型为“01”时填写电子凭证令牌，为“02”时填写身份证号，为“03”时填写社会保障卡卡号
	set_json_item_value(nlk, ipt_cert_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_setlway")); //个人结算方式
	set_json_item_value(nlk, _T("01"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, _T("21"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种类型
	set_json_item_value(nlk, _T("310"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_used_flag")); //个人账户使用标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("invono")); //发票号
	set_json_item_value(nlk, ipt_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mid_setl_flag")); //中途结算标志
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fulamt_ownpay_amt")); //全自费金额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("overlmt_selfpay")); //超限价金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("preselfpay_amt")); //先行自付金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("inscp_scp_amt")); //符合政策范围金额
	set_json_item_value(nlk, _T("1200"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_fee_preset_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_fee_preset.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

const tchar_t* ipt_fee_settle_url = _T("http://172.16.33.244/fsi/api/hospSettService/settA");

void ipt_fee_settle()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2207A"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, ipt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_type")); //就诊凭证类型
	set_json_item_value(nlk, cert_type);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_cert_no")); //就诊凭证类型为“01”时填写电子凭证令牌，为“02”时填写身份证号，为“03”时填写社会保障卡卡号
	set_json_item_value(nlk, ipt_cert_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("med_type")); //医疗类别
	set_json_item_value(nlk, _T("11"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("medfee_sumamt")); //医疗费总额
	set_json_item_value(nlk, _T("1200"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_setlway")); //个人结算方式
	set_json_item_value(nlk, _T("01"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //险种类型
	set_json_item_value(nlk, _T("310"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("acct_used_flag")); //个人账户使用标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("invono")); //发票号
	set_json_item_value(nlk, opt_fee_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mid_setl_flag")); //中途结算标志
	set_json_item_value(nlk, _T(""));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("fulamt_ownpay_amt")); //全自费金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("overlmt_selfpay")); //超限价金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("preselfpay_amt")); //先行自付金额
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("inscp_scp_amt")); //符合政策范围金额
	set_json_item_value(nlk, _T("1200"));

	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_fee_settle_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[4096] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, 4096);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_fee_settle.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

const tchar_t* ipt_settle_id = _T("");

const tchar_t* ipt_settle_cancel_url = _T("http://172.16.33.244/fsi/api/hospSettService/settCl");

void ipt_settle_cancel()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2305"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("setl_id")); //结算ID
	set_json_item_value(nlk, ipt_settle_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, ipt_psn_no);

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_settle_cancel_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_settle_cancel.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

const tchar_t* ipt_discharge_url = _T("http://172.16.33.244/fsi/api/dscgService/dischargeProcess");

void ipt_discharge()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2402"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("dscginfo"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, ipt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("insutype")); //
	set_json_item_value(nlk, _T("310"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("endtime")); //结束时间
	set_json_item_value(nlk, _T("2022-01-21 13:00:00"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_codg")); //病种编码
	set_json_item_value(nlk, _T("BA01000"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dise_name")); //病种名称
	set_json_item_value(nlk, _T("病种名称"));

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
	set_json_item_value(nlk, _T("0"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("dscg_way")); //离院方式
	set_json_item_value(nlk, _T("2"));

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
	set_json_item_value(nlk, ipt_psn_no);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_type")); //诊断类别
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("maindiag_flag")); //主诊断标志
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_srt_no")); //诊断排序号
	set_json_item_value(nlk, _T("1"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_code")); //诊断代码
	set_json_item_value(nlk, _T("A01.000"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("diag_name")); //诊断名称
	set_json_item_value(nlk, _T("诊断名称"));

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
	set_json_item_value(nlk, _T("2022-01-21 10:00:00"));


	byte_t doc_buf[4096] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 4096, _UTF8);

	//char in_buf[4096] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 4096, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_discharge_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_discharge.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

const tchar_t* ipt_discharge_cancel_url = _T("http://172.16.33.244/fsi/api/dscgService/dischargeUndo");

void ipt_discharge_cancel()
{
	xdate_t dt;
	get_loc_date(&dt);

	tchar_t msgid[31];
	xscpy(msgid, ins_code);
	xsprintf((msgid + xslen(msgid)), _T("%d%02d%02d%02d%02d%02d9999"), dt.year, dt.mon, dt.day, dt.hour, dt.min, dt.sec);

	link_t_ptr json = create_json_doc();
	link_t_ptr nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("infno"));
	set_json_item_value(nlk, _T("2305"));

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("msgid"));
	set_json_item_value(nlk, msgid);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrtarea_admvs"));
	set_json_item_value(nlk, admvs);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("insuplc_admdvs"));
	set_json_item_value(nlk, admvs);

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
	set_json_item_value(nlk, opter_no);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("opter_name"));
	set_json_item_value(nlk, opter_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("inf_time"));
	set_json_item_value(nlk, inf_time);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_code"));
	set_json_item_value(nlk, ins_code);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("fixmedins_name"));
	set_json_item_value(nlk, ins_name);

	nlk = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk, _T("sign_no"));
	set_json_item_value(nlk, sign_no);

	link_t_ptr nlk_sub = insert_json_item(json, LINK_LAST);
	set_json_item_name(nlk_sub, _T("input"));

	link_t_ptr clk_child = insert_json_item(nlk_sub, LINK_LAST);
	set_json_item_name(clk_child, _T("data"));

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("mdtrt_id")); //就诊ID
	set_json_item_value(nlk, ipt_id);

	nlk = insert_json_item(clk_child, LINK_LAST);
	set_json_item_name(nlk, _T("psn_no"));
	set_json_item_value(nlk, ipt_psn_no);

	byte_t doc_buf[1024] = { 0 };
	dword_t n_doc = format_json_doc_to_bytes(json, doc_buf, 1024, _UTF8);

	//char in_buf[1024] = { 0 };
	//format_json_doc_to_bytes(json, (byte_t*)in_buf, 1024, _GB2312);

	destroy_json_doc(json);

	char out_buf[100] = { 0 };

	char str_tm[NUM_LEN] = { 0 };
	dword_t nt = get_times();
	a_ltoxs(nt, str_tm, NUM_LEN);

	gb_mac_sm3((char*)str_key, (char*)str_sec, str_tm, (char*)doc_buf, out_buf);
	dword_t n_sin = a_xslen((char*)out_buf);

	tchar_t sin_buf[100] = { 0 };
	n_sin = utf8_to_ucs((byte_t*)out_buf, n_sin, sin_buf, 100);

	xhand_t xhttp = xhttp_client(_T("POST"), ipt_discharge_cancel_url);
	xhttp_set_request_default_header(xhttp);
	xhttp_set_request_content_type(xhttp, HTTP_HEADER_CONTENTTYPE_APPJSON, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-key"), -1, ins_key, -1);
	xhttp_set_request_header(xhttp, _T("x-ca-signature"), -1, sin_buf, n_sin);

	tchar_t err_code[NUM_LEN] = { 0 };
	tchar_t err_text[ERR_LEN] = { 0 };

	if (!xhttp_send_full(xhttp, doc_buf, n_doc))
	{
		get_last_error(err_code, err_text, ERR_LEN);
	}

	dword_t nlen = 0;
	byte_t** pbuf = NULL;
	pbuf = bytes_alloc();

	xhttp_recv_full(xhttp, pbuf, &nlen);

	utf8_to_ucs(*pbuf, nlen, err_text, ERR_LEN);

	json = create_json_doc();
	parse_json_doc_from_bytes(json, *pbuf, nlen, _UTF8);

	save_json_to_text_file(json, NULL, _T("ipt_discharge_cancel.txt"));

	destroy_json_doc(json);

	bytes_free(pbuf);
	pbuf = NULL;

	xhttp_close(xhttp);
}

int main(int argc, char* argv[])
{
	xdk_process_init(XDK_APARTMENT_PROCESS);

	//test_path2();

	//test_conv();

	//test_sm3();

	//test_json();

	//sign_in();

	//sign_out();

	query_fixins();

	//query_psninfo();

	//out_reg();

	//out_reg_cancel();

	//out_trt_up();

	//out_fee_up();

	//out_fee_cancel();

	//out_fee_preset();

	//out_fee_settle();

	//out_settle_cancel();

	//out_revs();

	//out_stmt();

	//out_stmt_detail();

	//ipt_reg();

	xdk_process_uninit();

#ifdef _OS_WINDOWS
	getch();
#endif

	return 0;
}