
#include "ybmac.h"

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
