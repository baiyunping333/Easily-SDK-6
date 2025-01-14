﻿
#ifndef _XDCIMP_H
#define _XDCIMP_H

#ifdef XDU_SUPPORT_SHELL
#include "impshell.h"
#endif

#ifdef XDU_SUPPORT_CONTEXT_BITMAP
#include "impbitmap.h"
#endif

#ifdef XDU_SUPPORT_CLIPBOARD
#include "impclip.h"
#endif

#ifdef XDU_SUPPORT_CONTEXT
#include "impcontext.h"
#include "gdicanv.h"
#include "gdiinf.h"
#include "printbag.h"
#endif


#ifdef XDU_SUPPORT_WIDGET
#include "impwidget.h"
#include "widgetnc.h"
#include "widgetex.h"
#endif



#endif //_XDCIMP_H
