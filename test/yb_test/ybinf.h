#ifndef _YBINF_H
#define _YBINF_H


#include "ybdef.h"

bool_t sign_in(yb_config_t* pcfg);

bool_t sign_out(yb_config_t* pcfg);

bool_t stmt_total(yb_config_t* pcfg, const tchar_t* insutype, const tchar_t* med_type);

bool_t query_psninfo(yb_config_t* pcfg, link_t_ptr rlk);

bool_t query_opspinfo(yb_config_t* pcfg, link_t_ptr rlk_record);

bool_t query_psnsum(yb_config_t* pcfg, link_t_ptr rlk_record);

bool_t ipt_reg(yb_config_t* pcfg, link_t_ptr rlk_record, tchar_t* ipt_no, tchar_t* ipt_id);

bool_t opt_reg(yb_config_t* pcfg, link_t_ptr rlk, tchar_t* opt_no, tchar_t* opt_id);

bool_t opt_reg_cancel(yb_config_t* pcfg, link_t_ptr rlk, const tchar_t* opt_no, const tchar_t* opt_id);

bool_t ipt_reg_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id);

bool_t opt_trt_up(yb_config_t* pcfg, link_t_ptr rlk, const tchar_t* opt_no, const tchar_t* opt_id);

bool_t opt_fee_up(yb_config_t* pcfg, link_t_ptr rlk, const tchar_t* opt_no, const tchar_t* opt_id, tchar_t* medfee_sumamt, tchar_t* fulamt_ownpay_amt, tchar_t* overlmt_selfpay, tchar_t* preselfpay_amt, tchar_t* inscp_scp_amt);

bool_t ipt_fee_up(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id, tchar_t* medfee_sumamt, tchar_t* fulamt_ownpay_amt, tchar_t* overlmt_selfpay, tchar_t* preselfpay_amt, tchar_t* inscp_scp_amt);

bool_t opt_fee_cancel(yb_config_t* pcfg, link_t_ptr rlk, const tchar_t* opt_no, const tchar_t* opt_id);

bool_t ipt_fee_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id);

bool_t opt_preset(yb_config_t* pcfg, link_t_ptr rlk, const tchar_t* opt_no, const tchar_t* opt_id, const tchar_t* medfee_sumamt, const tchar_t* fulamt_ownpay_amt, const tchar_t* overlmt_selfpay, const tchar_t* preselfpay_amt, const tchar_t* inscp_scp_amt);

bool_t ipt_preset(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id, const tchar_t* medfee_sumamt, const tchar_t* fulamt_ownpay_amt, const tchar_t* overlmt_selfpay, const tchar_t* preselfpay_amt, const tchar_t* inscp_scp_amt); 

bool_t opt_settle(yb_config_t* pcfg, link_t_ptr rlk, const tchar_t* opt_no, const tchar_t* opt_id, const tchar_t* medfee_sumamt, const tchar_t* fulamt_ownpay_amt, const tchar_t* overlmt_selfpay, const tchar_t* preselfpay_amt, const tchar_t* inscp_scp_amt, tchar_t* setl_id);

bool_t ipt_settle(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id, const tchar_t* medfee_sumamt, const tchar_t* fulamt_ownpay_amt, const tchar_t* overlmt_selfpay, const tchar_t* preselfpay_amt, const tchar_t* inscp_scp_amt, tchar_t* setl_id);

bool_t opt_settle_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* opt_id, const tchar_t* setl_id);

bool_t ipt_settle_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_id, const tchar_t* setl_id);

bool_t ipt_discharge(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id);

bool_t ipt_discharge_cancel(yb_config_t* pcfg, link_t_ptr rlk_record, const tchar_t* ipt_no, const tchar_t* ipt_id);

#endif