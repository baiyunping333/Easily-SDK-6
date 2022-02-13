/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc loged defination document

	@module	xdkdef.h | interface file

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


#ifndef _LOGINF_H
#define	_LOGINF_H

#include "dlink.h"


typedef void(*PF_LOGED_TITLE)(const tchar_t*, const tchar_t*, int);
typedef void(*PF_LOGED_ERROR)(const tchar_t*, const tchar_t*, const tchar_t*, int);
typedef void(*PF_LOGED_DATA)(const tchar_t*, const byte_t*, dword_t);
typedef void(*PF_LOGED_XML)(const tchar_t*, link_t_ptr);
typedef void(*PF_LOGED_JSON)(const tchar_t*, link_t_ptr);

typedef struct _loged_interface{
	tchar_t unc[PATH_LEN + 1];

	PF_LOGED_TITLE	pf_log_title;
	PF_LOGED_ERROR	pf_log_error;
	PF_LOGED_DATA	pf_log_data;
	PF_LOGED_XML	pf_log_xml;
	PF_LOGED_JSON	pf_log_json;
}loged_interface;

typedef bool_t(*PF_PUBS_EVENT)(const tchar_t*, bool_t, link_t_ptr);
typedef bool_t(*PF_SUBS_EVENT)(const tchar_t*, bool_t, link_t_ptr);

typedef struct _event_interface{
	tchar_t url[PATH_LEN + 1];

	PF_PUBS_EVENT	pf_pubs_event;
	PF_SUBS_EVENT	pf_subs_event;
}event_interface;

typedef void(CALLBACK *PF_TRACK_ERROR)(void* param, const tchar_t* code, const tchar_t* text);


#endif	/* _LOGINF_H */

