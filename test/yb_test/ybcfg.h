#ifndef _YBCFG_H
#define _YBCFG_H


#include "ybdef.h"

bool_t find_dise_diag(const tchar_t* dise_code, tchar_t* diag_code, tchar_t* diag_name);

#define FILE_YB_CONFIG	_T("yb_config.ini")

bool_t load_config(yb_config_t* pcfg);

#define FILE_YB_PSN_TYPE	_T("yb_psn_type.txt")
extern link_t_ptr grid_psn_type;
bool_t load_psn_type(void);
void clear_psn_type(void);

#define FILE_YB_PSN_IDET_TYPE	_T("yb_psn_idet_type.txt")
extern link_t_ptr grid_psn_idet_type;
bool_t load_psn_idet_type(void);
void clear_psn_idet_type(void);

#define FILE_YB_SIGN	_T("yb_sign.txt")
extern link_t_ptr grid_sign;
bool_t load_sign(void);
bool_t save_sign(void);
link_t_ptr loca_sign(void);
void clear_sign();

#define FILE_YB_RECORD	_T("yb_record.txt")
extern link_t_ptr grid_record;
bool_t load_record(void);
bool_t save_record(void);
void clean_record(void);

#define FILE_YB_CACHE	_T("yb_cache.txt")
extern link_t_ptr grid_cache;
bool_t load_cache(void);
bool_t save_cache(void);
void clear_cache(void);

#endif

