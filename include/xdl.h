﻿
#ifndef _XDL_H
#define _XDL_H

#include "../xdl/xdldef.h"

#ifdef XDL_SUPPORT_BIO
#include "../xdl/xdloop.h"
#endif

#ifdef XDL_SUPPORT_DOC
#include "../xdl/xdldoc.h"
#endif

#ifdef XDL_SUPPORT_VIEW
#include "../xdl/xdlgdi.h"
#include "../xdl/xdlview.h"
#endif

#include "../xdl/xdlutil.h"

typedef	link_t_ptr	LINKPTR;
typedef xhand_t		XHANDLE;

#ifdef _OS_WINDOWS
#pragma comment(lib,"xdl.lib")
#endif

#endif //_XDL_H
