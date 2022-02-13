/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc pacs document

	@module	netpacs.h | interface file

	@devnote 张文权 2021.01 - 2021.12	v6.0
***********************************************************************/

/**********************************************************************
This program is free software : you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
LICENSE.GPL3 for more details.
***********************************************************************/

#ifndef _NETPACS_H
#define _NETPACS_H

#include "../xdkdef.h"
#include "../bioinf.h"
#include "netdef.h"

#define DEF_PNET_PORT		104

/*pnet pdu type*/
#define PNET_PDU_ASSOCIATE_RQ	0x01
#define PNET_PDU_ASSOCIATE_AC	0x02
#define PNET_PDU_ASSOCIATE_RJ	0x03
#define PNET_PDU_DATA_TF 		0x04
#define PNET_PDU_RELEASE_RQ		0x05
#define PNET_PDU_RELEASE_RP		0x06
#define PNET_PDU_ABORT_RQ		0x07

#define PNET_PDV_SIZE_FROM_SET(dw)	(dw + 2)
#define PNET_PDU_SIZE_FROM_PDV(dw)	(dw + 4)

/*pnet options*/
#define PNET_OPT_AET_SCP			1
#define PNET_OPT_AET_SCU			2
#define PNET_OPT_SYNTAX_ABSTRACT	3
#define PNET_OPT_SYNTAX_TRANSFER	4
#define PNET_OPT_DATA_MAXINUM	5
#define PNET_OPT_CONTEXT_APPLICATION	6
#define PNET_OPT_CONTEXT_IMPLEMENT	7

#define PNET_USER_DATA_MAXINUM	16384

#define SOP_CONTEXT_IMPLEMENT	"1.2.276.0.7230010.3.0.3.6.2" 
#define SOP_CONTEXT_APPLICATION	"1.2.840.10008.3.1.1.1" 
#define SOP_SYNTAX_ABSTRACT		"1.2.840.10008.1.1"
#define SOP_SYNTAX_TRANSFER		"1.2.840.10008.1.2"

typedef enum{
	_PNET_STATUS_ASSOCIATE = 0,
	_PNET_STATUS_TRANSF = 1,
	_PNET_STATUS_PENDING = 2,
	_PNET_STATUS_RELEASE = 3
}PNET_STATUS;

typedef enum{
	_ASSOCIATE_ACCEPT = 0,
	_ASSOCIATE_USER_REJECT = 1,
	_ASSOCIATE_UNKNOWN = 2,
	_ASSOCIATE_ABSTRACT_SYNTAX_NOT_SUPPORT = 3,
	_ASSOCIATE_TRANSFER_SYNTAX_NOT_SUPPORT = 4
}PNET_ASSOCIATE_STATE;

typedef enum{
	_ASSOCIATE_SOURCE_SCU = 1,
	_ASSOCIATE_SOURCE_SCP_PDV = 2,
	_ASSOCIATE_SOURCE_SCP_PDU = 3
}PNET_ASSOCIATE_SOURCE;

typedef enum{
	_ASSOCIATE_REASON_NONE = 1,
	_ASSOCIATE_REASON_APPLICATION_CONTEXT_NAME_NOT_SUPPORT = 2,
	_ASSOCIATE_REASON_CALLING_AP_TITLE_NOT_RECONGNIZED = 3,
	_ASSOCIATE_REASON_CALLED_AP_TITLE_NOT_RECONGNIZED = 7
}PNET_ASSOCIATE_REASON;

typedef enum{
	_REJECTED_PERMANENT = 1,
	_REJECTED_TEMPORARY = 2
}PNET_REJECTED_STATE;


typedef enum{
	_PNET_TYPE_SCU = 1,
	_PNET_TYPE_SCP = 2
}PNET_TYPE;

typedef struct _pnet_pdv_head_t{
	dword_t size;
	byte_t did;
	byte_t ctl;
}pnet_pdv_head_t;

typedef struct _pnet_t{

	bio_interface* pif;

	int type;	/*connect type*/
	bool_t status;		/*connect status*/
	dword_t n_request;	/*request pdu bytes*/
	dword_t n_response;	/*response pdu bytes*/

	sword_t ver;	/*version*/
	byte_t scp[16];	/*Called App title*/
	byte_t scu[16];	/*Calling Ap title*/
	byte_t ret[32];	/*retain*/

	byte_t ack[1];	/*associate ack*/
	byte_t src[1];	/*associate source*/
	byte_t dag[1];	/*associate diagnose*/

	dword_t udm;	/*User Data Maximum Length*/

	schar_t* iid;	/*Implement Context Item Text*/
	schar_t* uid;	/*Application Context Item Text*/
	schar_t* asn;	/*Abstract Syntax*/
	schar_t* tsn;	/*Transfer Syntax*/
}pnet_t;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API pnet_t*	pnet_scu(xhand_t bio);

	EXP_API pnet_t*	pnet_scp(xhand_t bio);

	EXP_API xhand_t	pnet_bio(pnet_t* pnet);

	EXP_API void pnet_close(pnet_t* pnet);

	EXP_API int pnet_status(pnet_t* pnet);

	EXP_API bool_t pnet_send(pnet_t* pnet, pnet_pdv_head_t* pdv);

	EXP_API bool_t pnet_recv(pnet_t* pnet, pnet_pdv_head_t* pdv);

	EXP_API void pnet_set_response_bytes(pnet_t* pnet, dword_t size);

	EXP_API dword_t pnet_get_response_bytes(pnet_t* pnet);

	EXP_API dword_t pnet_get_request_bytes(pnet_t* pnet);

	EXP_API void pnet_set_request_bytes(pnet_t* pnet, dword_t size);

	EXP_API dword_t pnet_pdv_group_size(pnet_pdv_head_t* pdv, int n);

	EXP_API dword_t pnet_get_options(pnet_t* pnet, int opt, void* buf, dword_t max);

	EXP_API void pnet_set_options(pnet_t* pnet, int opt, void* buf, dword_t len);

#ifdef	__cplusplus
}
#endif


#endif /*PNET_H*/
