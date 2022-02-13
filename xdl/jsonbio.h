/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc json bio document

	@module	jsonbio.h | interface file

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

#ifndef _JSONBIO_H
#define _JSONBIO_H

#include "xdldef.h"

#if defined(XDL_SUPPORT_BIO)

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API bool_t parse_json_doc_from_bytes(link_t_ptr json, const byte_t* str, dword_t len, int encode);

	EXP_API dword_t format_json_doc_to_bytes(link_t_ptr json, byte_t* buf, dword_t max, int encode);


	EXP_API bool_t parse_json_doc_from_string(link_t_ptr json, string_t vs);

	EXP_API bool_t format_json_doc_to_string(link_t_ptr json, string_t vs);


	EXP_API bool_t parse_json_doc_from_stream(link_t_ptr json, stream_t stm);

	EXP_API bool_t format_json_doc_to_stream(link_t_ptr json, stream_t stm);


	EXP_API bool_t parse_json_doc_from_memo(link_t_ptr json, link_t_ptr txt);

	EXP_API bool_t format_json_doc_to_memo(link_t_ptr json, link_t_ptr txt);


#ifdef	__cplusplus
}
#endif

#endif

#endif /*_JSONBIO_H*/
