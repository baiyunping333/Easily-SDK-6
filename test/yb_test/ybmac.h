#ifndef _YBMAC_H
#define _YBMAC_H


#include "ybdef.h"

typedef int(__stdcall *pf_zj_hmac_SM3)(char* key, char* secret, char* unix_timestamp, char* request_body, char* outMsg);

void zj_mac_sm3(char* key, char* secret, char* unix_timestamp, char* request_body, char* outMsg);

void gb_mac_sm3(char* key, char* secret, char* unix_timestamp, char* request_body, char* outMsg);



#endif