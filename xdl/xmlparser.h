/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xml parser document

	@module	xmlparser.h | interface file

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

#ifndef _XMLPARSER_H
#define _XMLPARSER_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_DOC

#ifdef	__cplusplus
extern "C" {
#endif

	LOC_API bool_t parse_xml_doc_from_object(link_t_ptr ptr, opera_interface* pbo);

	LOC_API bool_t parse_dom_doc_from_object(link_t_ptr ptr, opera_interface* pbo);

	LOC_API bool_t format_xml_doc_to_object(link_t_ptr ptr, opera_interface* pbo);

	LOC_API bool_t format_dom_doc_to_object(link_t_ptr ptr, opera_interface* pbo);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_DOC*/

#endif	/*_XMLPARSER_H*/