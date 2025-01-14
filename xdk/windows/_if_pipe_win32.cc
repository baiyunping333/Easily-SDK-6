﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc pipe system call document

	@module	_if_pipe.c | windows implement file

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

#include "../xdkloc.h"

#ifdef XDK_SUPPORT_PIPE

#define PIPE_HEAD		_T("\\\\.\\pipe\\")

res_file_t _pipe_srv(const tchar_t* pname, dword_t fmode)
{
	tchar_t path[PATH_LEN + 1] = { 0 };
	HANDLE hp = NULL;
	DWORD dwAccess;
	SECURITY_ATTRIBUTES saAttr;

	if (!pname || !(*pname))
	{
		if (fmode & FILE_OPEN_WRITE)
			hp = GetStdHandle(STD_OUTPUT_HANDLE);
		else
			hp = GetStdHandle(STD_INPUT_HANDLE);

		return (hp) ? hp : INVALID_FILE;
	}

	_stprintf(path, _T("%s%s"), PIPE_HEAD, pname);

	if (fmode & FILE_OPEN_WRITE)
		dwAccess = PIPE_ACCESS_OUTBOUND;
	else
		dwAccess = PIPE_ACCESS_INBOUND;

	if (fmode & FILE_OPEN_OVERLAP)
		dwAccess |= FILE_FLAG_OVERLAPPED;
	
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = FALSE;
	saAttr.lpSecurityDescriptor = NULL;

	hp = CreateNamedPipe(path, dwAccess, PIPE_TYPE_BYTE, PIPE_UNLIMITED_INSTANCES, 4096, 4096, 0, &saAttr);
	if (hp == INVALID_HANDLE_VALUE)
	{
		return INVALID_FILE;
	}

	return hp;
}

wait_t _pipe_listen(res_file_t pip, async_t* pb)
{
	LPOVERLAPPED pov = (pb) ? (LPOVERLAPPED)pb->lapp : NULL;

	if (!ConnectNamedPipe(pip, pov))
	{
		if (GetLastError() != ERROR_PIPE_CONNECTED)
			return WAIT_TMO;
	}

	if (pov) ResetEvent(pov->hEvent);

	return WAIT_RET;
}

void _pipe_stop(res_file_t pip)
{
	FlushFileBuffers(pip);
	DisconnectNamedPipe(pip);
}

void _pipe_close(const tchar_t* pname, res_file_t pip)
{
	CloseHandle(pip);
}

res_file_t _pipe_cli(const tchar_t* pname, dword_t fmode)
{
	tchar_t path[PATH_LEN + 1] = { 0 };
	HANDLE hp = NULL;
	DWORD dwAccess;

	if (!pname || !(*pname))
	{
		if (fmode & FILE_OPEN_WRITE)
			hp = GetStdHandle(STD_OUTPUT_HANDLE);
		else
			hp = GetStdHandle(STD_INPUT_HANDLE);

		return (hp) ? hp : INVALID_FILE;
	}

	_stprintf(path, _T("%s%s"), PIPE_HEAD, pname);

	if (fmode & FILE_OPEN_WRITE)
		dwAccess = GENERIC_WRITE;
	else
		dwAccess = GENERIC_READ;

	if (fmode & FILE_OPEN_OVERLAP)
		dwAccess |= FILE_FLAG_OVERLAPPED;

	hp = CreateFile(path, dwAccess, 0, NULL, OPEN_EXISTING, 0, NULL);

	return (hp == INVALID_HANDLE_VALUE) ? INVALID_FILE : hp;
}

wait_t _pipe_wait(const tchar_t* pname, int ms)
{
	tchar_t path[PATH_LEN + 1] = { 0 };
	DWORD dw;

	if (!pname || !(*pname))
	{
		return WAIT_RET;
	}

	_stprintf(path, _T("%s%s"), PIPE_HEAD, pname);

	if (ms < 0)
		dw = NMPWAIT_WAIT_FOREVER;
	else if (!ms)
		dw = NMPWAIT_USE_DEFAULT_WAIT;
	else
		dw = ms;

	return (WaitNamedPipe(path, dw)) ? WAIT_RET : WAIT_TMO;
}

bool_t _pipe_write(res_file_t pipe, void* buf, dword_t len, async_t* pb)
{
	return _file_write(pipe, buf, len, pb);
}

bool_t _pipe_flush(res_file_t pipe)
{
	return _file_flush(pipe);
}

bool_t _pipe_read(res_file_t pipe, void* buf, dword_t size, async_t* pb)
{
	return _file_read(pipe, buf, size, pb);
}

#endif //XDK_SUPPORT_PIPE
