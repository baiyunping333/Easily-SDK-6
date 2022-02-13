/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc window nc document

	@module	widgetnc.h | interface file

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

#ifndef _WIDGETNC_H
#define _WIDGETNC_H

#include "xdcdef.h"

#ifdef XDU_SUPPORT_WIDGET_NC

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API void widgetnc_on_paint(res_win_t wt, visual_t dc);

EXP_API void widgetnc_on_calcsize(res_win_t wt, border_t* pbd);

EXP_API int widgetnc_on_hittest(res_win_t wt, const xpoint_t* pxp);

EXP_API int widgetnc_on_calcscroll(res_win_t wt, bool_t horz, const xpoint_t* pxp);

EXP_API void widget_draw_scroll(res_win_t wt, bool_t horz);

#ifdef	__cplusplus
}
#endif

#endif

#endif /*WIDGETNC_H*/