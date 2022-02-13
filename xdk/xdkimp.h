
#ifndef _XDKIMP_H
#define _XDKIMP_H


#include "imp/impjmp.h"
#include "imp/impmem.h"
#include "imp/impassert.h"
#include "imp/imperror.h"

#ifdef XDK_SUPPORT_MBCS
#include "imp/impmbcs.h"
#endif

#ifdef XDK_SUPPORT_DATE
#include "imp/impdate.h"
#endif

#ifdef XDK_SUPPORT_ASYNC
#include "imp/impasync.h"
#endif

#ifdef XDK_SUPPORT_THREAD
#include "imp/impthr.h"
#endif

#ifdef XDK_SUPPORT_TIMER
#include "imp/imptimer.h"
#endif

#ifdef XDK_SUPPORT_RANDOM
#include "imp/imprandom.h"
#endif

#ifdef XDK_SUPPORT_FILE
#include "imp/impuncf.h"
#endif

#ifdef XDK_SUPPORT_SOCK
#include "imp/impsock.h"
#endif

#ifdef XDK_SUPPORT_SHARE
#include "imp/impshare.h"
#endif

#ifdef XDK_SUPPORT_MEMO_CACHE
#include "imp/impcache.h"
#endif

#ifdef XDK_SUPPORT_PROCESS
#include "imp/impproc.h"
#endif

#ifdef XDK_SUPPORT_PIPE
#include "imp/imppipe.h"
#endif

#ifdef XDK_SUPPORT_COMM
#include "imp/impcomm.h"
#endif

#ifdef XDK_SUPPORT_CONS
#include "imp/impcons.h"
#endif

#include "imp/impblock.h"

#endif //_XDKIMP_H
