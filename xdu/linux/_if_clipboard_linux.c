/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc clipboard document

	@module	if_clipboard.c | linux implement file

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

#include "../xduloc.h"

#ifdef XDU_SUPPORT_CLIPBOARD

bool_t _clipboard_put(res_win_t win, int fmt, const byte_t* data, dword_t size)
{
	XEvent event;
	XSelectionRequestEvent ev = {0};
	XSelectionRequestEvent *xsr;
	Atom atom_clipb, atom_target, atom_text, atom_utf8;
	int ret;

	XPeekEvent(g_display, &event);

	if(event.type != SelectionRequest)
		return bool_false;
	
	xsr = &event.xselectionrequest;

	ev.type = SelectionNotify;
	ev.display = xsr->display;
	ev.requestor = xsr->requestor;
	ev.selection = xsr->selection;
	ev.time = xsr->time;
	ev.target = xsr->target;
	ev.property = xsr->property;

	atom_clipb = XInternAtom(g_display, "CLIPBOARD", 0);

	XSetSelectionOwner (g_display, atom_clipb, win, 0);

	if(XGetSelectionOwner (g_display, atom_clipb) != win)
		return bool_false;

	atom_target = XInternAtom(g_display, "TARGETS", 0);
	atom_text = XInternAtom(g_display, "TEXT", 0);
	atom_utf8 = XInternAtom(g_display, "UTF8_STRING", 1);

	if (ev.target == atom_target)
	{
		ret = XChangeProperty (ev.display, ev.requestor, ev.property, XA_ATOM, 32, PropModeReplace, (unsigned char*)&atom_utf8, 1);
	}
	else if (ev.target == XA_STRING || ev.target == atom_text) 
	{
		ret = XChangeProperty(ev.display, ev.requestor, ev.property, XA_STRING, 8, PropModeReplace, data, size);
	}
	else if (ev.target == atom_utf8)
	{
		ret = XChangeProperty(ev.display, ev.requestor, ev.property, atom_utf8, 8, PropModeReplace, data, size);
	}else
	{
		ev.property = None;
		ret = 0;
	}
	
	if((!ret & 2))
	{
		XSendEvent (g_display, ev.requestor, 0, 0, (XEvent *)&ev);
	}

	return bool_true;
}

dword_t _clipboard_get(res_win_t win, int fmt, byte_t* buf, dword_t max)
{
	XEvent ev = {0};
	int format;
	unsigned long N, size;
	unsigned char * data = NULL;
	dword_t ret;
	Atom atom_clipb, atom_target, atom_utf8, target;

	XPeekEvent(g_display, &ev);

	if(ev.type != SelectionNotify)
		return 0;

	atom_clipb = XInternAtom(g_display, "CLIPBOARD", 0);

	if(ev.xselection.selection != atom_clipb) 
		return 0;

	if(ev.xselection.property == None)
		return 0;

	atom_target = XInternAtom(g_display, "TARGETS", 0);
	atom_utf8 = XInternAtom(g_display, "UTF8_STRING", 1);

	XGetWindowProperty(ev.xselection.display, ev.xselection.requestor, ev.xselection.property, 0L,(~0L), 0, AnyPropertyType, &target, &format, &size, &N,(unsigned char**)&data);

	if(target == atom_utf8 || target == XA_STRING) 
	{
		if(buf)
		{
			ret = (max < (dword_t)size)? max : (dword_t)size;
			memcpy((void*)buf, (void*)data, ret);
		}else
		{
			ret = (dword_t)size;
		}
		
	}else
	{
		ret = 0;
	}

	if(data)  XFree(data);

	return ret;
}

#endif //XDU_SUPPORT_CLIPBOARD