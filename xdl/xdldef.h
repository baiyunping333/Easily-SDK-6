/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc defination document

	@module	xdldef.h | interface file

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


#ifndef _XDLDEF_H
#define	_XDLDEF_H

#include <xdk.h>

#if defined(_OS_WINDOWS)
#include "windows/_cfg_win32.h"
#elif defined(_OS_MACOS)
#include "macos/_cfg_macos.h"
#elif defined(_OS_LINUX)
#include "linux/_cfg_linux.h"
#endif


/*define data state*/
typedef enum{ 
	dsClean = 0, 
	dsDirty = 1, 
	dsNewClean = 2, 
	dsNewDirty = 4, 
	dsDelete = 8
}DATA_STATE;

#define dsAll		(dsClean | dsNewClean | dsDirty | dsNewDirty | dsDelete)
#define dsUpdate	(dsDirty | dsNewDirty | dsDelete)
#define dsNone		0

//define data verify code
typedef enum{
	veValid = 0,
	veNull = 1,
	veFormat = 2,
	veDatatype = 3,
	veOverflow = 4,
	veTruncate = 5,
	veCodeSystem = 6,
	veUserReject = 7
}VERIFY_CODE;

/*canvas type*/
#define _CANVAS_UNKNOWN		0x00
#define _CANVAS_DISPLAY		0x01
#define _CANVAS_PRINTER		0x02

typedef struct _handle_head *canvas_t;

/*define xdb type*/
#define _DB_UNKNOWN		0x00
#define _DB_STUB		0x01
#define _DB_ODBC		0x02
#define _DB_OCI			0x03
#define _DB_MYSQL		0x04

typedef struct _handle_head	*xdb_t;

#define UNIT_PT				_T("pt")
#define UNIT_MM				_T("mm")

#define PAPER_A4_WIDTH			210.0f
#define PAPER_A4_HEIGHT			297.0f

#define PAPER_A5_WIDTH			148.0f
#define PAPER_A5_HEIGHT			210.0f

#define PAPER_LETTER_WIDTH		210.0f
#define PAPER_LETTER_HEIGHT		280.0f

#define PAPER_MIN_WIDTH			2.6f
#define PAPER_MIN_HEIGHT		2.6f

#define PAPER_MAX_WIDTH			280.0f
#define PAPER_MAX_HEIGHT		558.7f

#define DEF_PAPER_WIDTH			PAPER_A4_WIDTH
#define DEF_PAPER_HEIGHT		PAPER_A4_HEIGHT

#define DEF_SPLIT_SPAN			1.5f
#define DEF_TOUCH_SPAN			7.0f
#define DEF_ICON_SPAN			6.8f
#define DEF_TEXT_HEIGHT			8.0f
#define DEF_TEXT_WIDTH			20.0f
#define DEF_ITEM_HEIGHT			7.0f
#define DEF_ITEM_WIDTH			15.0f
#define DEF_PLOT_HEIGHT			50.0f
#define DEF_PLOT_WIDTH			50.0f

#define DEF_SMALL_ICON		3.2f
#define DEF_MIDD_ICON		6.4f
#define DEF_LARGE_ICON		8.6f

#define DEF_PAGE_FEED		10.0f
#define DEF_CORNER_FEED		3.0f
#define DEF_SPLIT_FEED		1.0f
#define DEF_SHADOW_FEED		2.0f

#define DEF_INNER_FEED		-1
#define DEF_OUTER_FEED		1
#define DEF_FOCUS_SPAN		5

/*define object sizing type*/
#define SIZING_TOPLEFT			0x00000001
#define SIZING_TOPCENTER		0x00000002
#define SIZING_TOPRIGHT			0x00000004
#define SIZING_BOTTOMLEFT		0x00000008
#define SIZING_BOTTOMCENTER		0x00000010
#define SIZING_BOTTOMRIGHT		0x00000020
#define SIZING_LEFTCENTER		0x00000040
#define SIZING_RIGHTCENTER		0x00000080

#ifdef LANG_CN
#define WARNING_OPTIONS		_T("1~红色预警;2~橙色预警;3~黄色预警;4~蓝色预警;")
#else
#define WARNING_OPTIONS		_T("1~Red;2~Orange;3~Yellow;4~Blue;")
#endif

#define PAGENUM_NAME		_T("Page#")

#ifdef LANG_CN
#define PAGENUM_GUID		_T("第%d页")
#else
#define PAGENUM_GUID		_T("Page %d")
#endif

typedef struct _page_cator_t{
	short page;
	short indi;
	int pos;
	int point;
	tchar_t pch[CHS_LEN + 1];
}page_cator_t;

typedef struct _viewbox_t{
	int px, py, pw, ph;
}viewbox_t;

typedef struct _canvbox_t{
	float fx, fy, fw, fh;
}canvbox_t;


#include "xdlinf.h"

#endif	/* _XDLDEF_H */

