/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc calendar document

	@module	calendarview.h | interface file

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

#ifndef _CALENDARVIEW_H
#define _CALENDARVIEW_H

#include "xdldef.h"

#ifdef XDL_SUPPORT_VIEW

typedef enum{
	CALENDAR_HINT_NONE,
	CALENDAR_HINT_YEARBTN,
	CALENDAR_HINT_MONTHBTN,
	CALENDAR_HINT_WEEKBTN,
	CALENDAR_HINT_DAYBTN,
	CALENDAR_HINT_PREVBTN,
	CALENDAR_HINT_NEXTBTN,
	CALENDAR_HINT_TODAYBTN,
	CALENDAR_HINT_DAILY
}CALENDAR_HINT_CODE;

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API void calc_calendar_daily_rect(link_t_ptr ptr, link_t_ptr ilk, xrect_t* pxr);

	EXP_API int calc_calendar_hint(link_t_ptr ptr, const xpoint_t* ppt, link_t_ptr* pilk);

	EXP_API void draw_calendar(const drawing_interface* pcanv, link_t_ptr ptr);

#ifdef	__cplusplus
}
#endif

#endif //XDLVIEW

#endif
