﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc xdc initialize document

	@module	xdcinit.h | interface file

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

#ifndef _XDCINIT_H
#define	_XDCINIT_H

#include "xdcdef.h"


typedef struct _xdc_mou_t{
	bool_t		if_ok;

#ifdef XDU_SUPPORT_BLUT
	if_blut_t if_blut;
#endif

#ifdef XDU_SUPPORT_CONTEXT
	int	ctx_ver;
	if_context_t if_context;
#endif

#ifdef XDU_SUPPORT_CLIPBOARD
	if_clipboard_t	if_clipboard;
#endif

#ifdef XDU_SUPPORT_WIDGET
	if_widget_t		if_widget;
#endif

#ifdef XDU_SUPPORT_SHELL
	if_shell_t		if_shell;
#endif

	}xdc_mou_t;

#ifdef	__cplusplus
	extern "C" {
#endif

	extern xdc_mou_t g_xdc_mou;

#define XDC_MOUNTED					((g_xdc_mou.if_ok)? 1 : 0)


#ifdef XDU_SUPPORT_CONTEXT
#define PROCESS_CONTEXT_INTERFACE	((g_xdc_mou.if_ok)? (if_context_t*)(&g_xdc_mou.if_context) : NULL)
#define XDC_CONTEXT_VERSION			(g_xdc_mou.ctx_ver)
#endif

#ifdef XDU_SUPPORT_CLIPBOARD
#define PROCESS_CLIPBOARD_INTERFACE		((g_xdc_mou.if_ok)? (if_clipboard_t*)(&g_xdc_mou.if_clipboard) : NULL)
#endif

#ifdef XDU_SUPPORT_WIDGET
#define PROCESS_WIDGET_INTERFACE		((g_xdc_mou.if_ok)? (if_widget_t*)(&g_xdc_mou.if_widget) : NULL)
#endif

#ifdef XDU_SUPPORT_SHELL
#define PROCESS_SHELL_INTERFACE			((g_xdc_mou.if_ok)? (if_shell_t*)(&g_xdc_mou.if_shell) : NULL)
#endif


	EXP_API	void	xdc_process_init(void);

	EXP_API void	xdc_process_uninit(void);

#ifdef	__cplusplus
}
#endif

#endif	/* _XDCINIT_H */

