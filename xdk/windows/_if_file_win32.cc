﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc file system call document

	@module	_if_file.c | windows implement file

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

#ifdef XDK_SUPPORT_FILE

res_file_t _file_open(const tchar_t* fname, dword_t fmode)
{
	HANDLE hFile = 0;
	DWORD dwAccess, dwCreate, dwFlag;

	if (fmode & FILE_OPEN_APPEND)
	{
		dwCreate = OPEN_ALWAYS;
		dwAccess = GENERIC_READ | GENERIC_WRITE;
	}
	else if (fmode & FILE_OPEN_CREATE)
	{
		dwCreate = CREATE_ALWAYS;
		dwAccess = GENERIC_READ | GENERIC_WRITE;
	}
	else if (fmode & FILE_OPEN_WRITE)
	{
		dwCreate = OPEN_EXISTING;
		dwAccess = GENERIC_READ | GENERIC_WRITE;
	}
	else
	{
		dwCreate = OPEN_EXISTING;
		dwAccess = GENERIC_READ;
	}

	if (fmode & FILE_OPEN_OVERLAP)
		dwFlag = FILE_FLAG_OVERLAPPED;
	else if (fmode & FILE_OPEN_RANDOM)
		dwFlag = FILE_FLAG_NO_BUFFERING;
	else
		dwFlag = FILE_ATTRIBUTE_NORMAL;

	hFile = CreateFile(fname, dwAccess, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, dwCreate, dwFlag, NULL);
	if ((HANDLE)hFile == INVALID_HANDLE_VALUE)
	{
		return INVALID_FILE;
	}

	if (fmode & FILE_OPEN_APPEND)
		SetFilePointer(hFile, 0, NULL, FILE_END);

	return hFile;
}

void _file_close(res_file_t fh)
{
	CloseHandle(fh);
}

bool_t _file_size(res_file_t fh, dword_t* ph, dword_t* pl)
{
	bool_t rt;

	*pl = GetFileSize(fh, (LPDWORD)ph);

	rt = (*pl == INVALID_FILE_SIZE) ? 0 : 1;

	return rt;
}

bool_t _file_write(res_file_t fh, void* buf, dword_t size, async_t* pb)
{
	LPOVERLAPPED pov = (pb && pb->type != ASYNC_BLOCK) ? (LPOVERLAPPED)pb->lapp : NULL;
	dword_t* pcb = (pb) ? &(pb->size) : NULL;

	DWORD err, dw = 0;
	ULONG_PTR up = NULL;
	LPOVERLAPPED ul = NULL;
	bool_t rt = 1;

	if (!WriteFile(fh, buf, (DWORD)size, &dw, pov))
	{
		if (!pov)
		{
			dw = 0;
			rt = 0;
			goto ITRET;
		}

		err = GetLastError();
		if (err == ERROR_HANDLE_EOF)
		{
			rt = 1;
			goto ITRET;
		}

		if (err != ERROR_IO_PENDING)
		{
			dw = 0;
			rt = 0;
			goto ITRET;
		}

		if (pb->type == ASYNC_QUEUE)
		{
			dw = 0;
			if (!GetQueuedCompletionStatus((HANDLE)pb->port, &dw, &up, &ul, pb->timo))
			{
				rt = (dw)? 1 : 0;
				goto ITRET;
			}
		}
		if (pb->type == ASYNC_EVENT)
		{
			if (WAIT_OBJECT_0 != WaitForSingleObject(pov->hEvent, pb->timo))
			{
				dw = 0;
				rt = 0;
				goto ITRET;
			}

			dw = 0;
			if (!GetOverlappedResult(fh, pov, &dw, FALSE))
			{
				rt = 0;
				goto ITRET;
			}
		}
	}

ITRET:

	if (pov && pov->hEvent) ResetEvent(pov->hEvent);

	if (pcb) *pcb = dw;

	return rt;
}

bool_t _file_flush(res_file_t fh)
{
	return (FlushFileBuffers(fh)) ? 1 : 0;
}

bool_t _file_read(res_file_t fh, void* buf, dword_t size, async_t* pb)
{
	LPOVERLAPPED pov = (pb && pb->type != ASYNC_BLOCK) ? (LPOVERLAPPED)pb->lapp : NULL;
	dword_t* pcb = (pb) ? &(pb->size) : NULL;

	DWORD err, dw = 0;
	ULONG_PTR up = NULL;
	LPOVERLAPPED ul = NULL;
	bool_t rt = 1;

	if (!ReadFile(fh, buf, (DWORD)size, &dw, pov))
	{
		if (!pov)
		{
			dw = 0;
			rt = 0;
			goto ITRET;
		}

		err = GetLastError();
		if (err == ERROR_HANDLE_EOF)
		{
			rt = 1;
			goto ITRET;
		}

		if (err != ERROR_IO_PENDING)
		{
			dw = 0;
			rt = 0;
			goto ITRET;
		}

		if (pb->type == ASYNC_QUEUE)
		{
			dw = 0;
			if (!GetQueuedCompletionStatus((HANDLE)pb->port, &dw, &up, &ul, pb->timo))
			{
				rt = (dw)? 1 : 0;
				goto ITRET;
			}
		}
		if (pb->type == ASYNC_EVENT)
		{
			if (WAIT_OBJECT_0 != WaitForSingleObject(pov->hEvent, pb->timo))
			{
				dw = 0;
				rt = 0;
				goto ITRET;
			}

			dw = 0;
			if (!GetOverlappedResult(fh, pov, &dw, FALSE))
			{
				rt = 0;
				goto ITRET;
			}
		}
	}

ITRET:

	if (pov && pov->hEvent) ResetEvent(pov->hEvent);
	
	if (pcb) *pcb = dw;

	return rt;
}

bool_t _file_read_range(res_file_t fh, dword_t hoff, dword_t loff, void* buf, dword_t size)
{
	HANDLE mh;
	void* pBase;
	dword_t gran, dwh, dwl, poff;
	lword_t flen;
	vword_t dlen;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	gran = si.dwAllocationGranularity;

	poff = (loff % gran);
	loff = (loff / gran) * gran;
	dlen = (vword_t)poff + (vword_t)size;

	flen = MAKELWORD(loff, hoff) + (lword_t)dlen;
	dwh = GETHDWORD(flen);
	dwl = GETLDWORD(flen);

	mh = CreateFileMapping(fh, NULL, PAGE_READONLY, dwh, dwl, NULL);
	if (!mh)
	{
		return 0;
	}

	pBase = MapViewOfFile(mh, FILE_MAP_READ, hoff, loff, dlen);
	if (!pBase)
	{
		CloseHandle(mh);
		return 0;
	}

	CopyMemory(buf, (void*)((char*)pBase + poff), size);

	UnmapViewOfFile(pBase);
	CloseHandle(mh);

	return 1;
}

bool_t _file_write_range(res_file_t fh, dword_t hoff, dword_t loff, void* buf, dword_t size)
{
	HANDLE mh;
	void* pBase;
	dword_t gran, dwh, dwl, poff;
	lword_t flen;
	vword_t dlen;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	gran = si.dwAllocationGranularity;

	poff = (loff % gran);
	loff = (loff / gran) * gran;
	dlen = (vword_t)poff + (vword_t)size;

	flen = MAKELWORD(loff, hoff) + (lword_t)dlen;
	dwh = GETHDWORD(flen);
	dwl = GETLDWORD(flen);

	mh = CreateFileMapping(fh, NULL, PAGE_READWRITE, dwh, dwl, NULL);
	if (!mh)
	{
		return 0;
	}

	pBase = MapViewOfFile(mh, FILE_MAP_READ | FILE_MAP_WRITE, hoff, loff, dlen);
	if (!pBase)
	{
		CloseHandle(mh);
		return 0;
	}

	CopyMemory((void*)((char*)pBase + poff), buf, size);

	FlushViewOfFile(pBase, dlen);

	UnmapViewOfFile(pBase);
	CloseHandle(mh);

	return 1;
}

void* _file_lock_range(res_file_t fh, dword_t hoff, dword_t loff, dword_t size, bool_t write, res_file_t* ph)
{
	HANDLE mh;
	void* pBase;
	dword_t mask, gran, dwh, dwl, poff;
	lword_t flen;
	vword_t dlen;

	SYSTEM_INFO si = { 0 };

	*ph = NULL;

	GetSystemInfo(&si);
	gran = si.dwAllocationGranularity;

	poff = (loff % gran);
	loff = (loff / gran) * gran;
	dlen = (vword_t)poff + (vword_t)size;

	flen = MAKELWORD(loff, hoff) + (lword_t)dlen;
	dwh = GETHDWORD(flen);
	dwl = GETLDWORD(flen);

	mask = (write)? PAGE_READWRITE : PAGE_READONLY;

	mh = CreateFileMapping(fh, NULL, mask, dwh, dwl, NULL);
	if (!mh)
	{
		return NULL;
	}

	mask = (write)? (FILE_MAP_READ | FILE_MAP_WRITE) : FILE_MAP_READ;

	pBase = MapViewOfFile(mh, mask, hoff, loff, dlen);
	if (!pBase)
	{
		CloseHandle(mh);
		return NULL;
	}

	*ph = mh;
	return (void*)((char*)pBase + poff);
}

void _file_unlock_range(res_file_t mh, dword_t hoff, dword_t loff, dword_t size, void* p)
{
	void* pBase;
	dword_t gran, dwh, dwl, poff;
	lword_t flen;
	vword_t dlen;

	SYSTEM_INFO si = { 0 };

	GetSystemInfo(&si);
	gran = si.dwAllocationGranularity;

	poff = (loff % gran);
	loff = (loff / gran) * gran;
	dlen = (vword_t)poff + (vword_t)size;

	flen = MAKELWORD(loff, hoff) + (lword_t)dlen;
	dwh = GETHDWORD(flen);
	dwl = GETLDWORD(flen);

	pBase = (void*)((BYTE*)p - poff);

	FlushViewOfFile(pBase, dlen);

	UnmapViewOfFile(pBase);
	CloseHandle(mh);
}

bool_t _file_truncate(res_file_t fh, dword_t hoff, dword_t loff)
{
	HANDLE hMap = NULL;

	hMap = CreateFileMapping(fh, NULL, PAGE_READWRITE, hoff, loff, NULL);
	if (!hMap)
	{
		return 0;
	}

	CloseHandle(hMap);

	return 1;
}

/*bool_t _file_write_range(res_file_t fh, void* buf, dword_t hoff, dword_t loff, dword_t size, dword_t* pb)
{
	bool_t rt;
	dword_t dw;

	dw = hoff;
	SetFilePointer(fh, loff, (PLONG)&dw, FILE_BEGIN);

	dw = 0;
	rt = WriteFile(fh, buf, (DWORD)size, &dw, NULL);

	if (pb)
		*pb = dw;

	return rt;
}*/


void _systime_to_xdate(const SYSTEMTIME* pst, xdate_t* pdt)
{
	pdt->year = pst->wYear;
	pdt->mon = pst->wMonth;
	pdt->day = pst->wDay;
	pdt->hour = pst->wHour;
	pdt->min = pst->wMinute;
	pdt->sec = pst->wSecond;
	pdt->wday = pst->wDayOfWeek;
}

void _systime_from_xdate(SYSTEMTIME* pst, const xdate_t* pdt)
{
	pst->wYear = pdt->year;
	pst->wMonth = pdt->mon;
	pst->wDay = pdt->day;
	pst->wHour = pdt->hour;
	pst->wMinute = pdt->min;
	pst->wSecond = pdt->sec;
	pst->wDayOfWeek = pdt->wday;
}

bool_t _file_gettime(res_file_t fh, xdate_t* pdt)
{
	SYSTEMTIME stm = { 0 };
	FILETIME ftm = { 0 };

	if (!GetFileTime(fh, NULL, NULL, &ftm))
		return 0;

	FileTimeToSystemTime(&ftm, &stm);

	_systime_to_xdate(&stm, pdt);

	return 1;
}

bool_t _file_settime(res_file_t fh, const xdate_t* pdt)
{
	SYSTEMTIME stm = { 0 };
	FILETIME ftm = { 0 };

	_systime_from_xdate(&stm, pdt);

	SystemTimeToFileTime(&stm, &ftm);

	return SetFileTime(fh, NULL, NULL, &ftm);
}

bool_t _file_delete(const tchar_t* fname)
{
	return DeleteFile(fname);
}

bool_t _file_rename(const tchar_t* fname, const tchar_t* nname)
{
	return (bool_t)MoveFile(fname, nname);
}

bool_t _file_info(const tchar_t* fname, file_info_t* pxf)
{
	WIN32_FILE_ATTRIBUTE_DATA ad = { 0 };
	SYSTEMTIME stm = { 0 };

	if (!GetFileAttributesEx(fname, GetFileExInfoStandard, &ad))
		return 0;

	pxf->low_size = ad.nFileSizeLow;
	pxf->high_size = ad.nFileSizeHigh;

	FileTimeToSystemTime(&ad.ftCreationTime, &stm);
	_systime_to_xdate(&stm, &pxf->create_time);

	FileTimeToSystemTime(&ad.ftLastAccessTime, &stm);
	_systime_to_xdate(&stm, &pxf->access_time);

	FileTimeToSystemTime(&ad.ftLastWriteTime, &stm);
	_systime_to_xdate(&stm, &pxf->write_time);

	return 1;
}

bool_t _directory_create(const tchar_t* pname)
{
	return CreateDirectory(pname, NULL);
}

bool_t _directory_remove(const tchar_t* pname)
{
	return RemoveDirectory(pname);
}

bool_t	_directory_open(const tchar_t* path, dword_t mode)
{
	WIN32_FIND_DATA wfd;
	HANDLE hFind;
	tchar_t cur_path[PATH_LEN + 1];
	tchar_t* token = (tchar_t*)path;
	bool_t b_add;
	int len;

	b_add = ((mode & FILE_OPEN_CREATE) || (mode & FILE_OPEN_APPEND)) ? 1 : 0;

	while (*token == _T('\\') || *token == _T('/'))
		token++;

	if ((int)(token - path) == 2) //net share floder
	{
		while (*token != _T('\\') && *token != _T('/') && *token != _T('\0'))
		{
			token++;
		}

		if (*token == _T('\\') || *token == _T('/'))
			token++;

		while (*token != _T('\\') && *token != _T('/') && *token != _T('\0'))
		{
			token++;
		}

		if (*token == _T('\\') || *token == _T('/'))
			token++;
	}

	while (*token != _T('\0'))
	{
		if (b_add)
		{
			while (*token != _T('\\') && *token != _T('/') && *token != _T('\0'))
			{
				token++;
			}
		}
		else
		{
			while (*token != _T('\0'))
			{
				token++;
			}
		}

		len = (int)(token - path);
		CopyMemory(cur_path, path, len * sizeof(tchar_t));
		cur_path[len] = _T('\0');

		hFind = FindFirstFile(cur_path, &wfd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			if (b_add)
			{
				if (!CreateDirectory(cur_path, NULL))
					return 0;
			}
			else
			{
				return 0;
			}
		}
		else
		{
			FindClose(hFind);
		}

		if (*token == _T('\\') || *token == _T('/'))
			token++;
	}

	return 1;
}

#ifdef XDK_SUPPORT_FILE_FIND

res_find_t _file_find_first(const tchar_t* fpath, file_info_t* pfi)
{
	WIN32_FIND_DATA wfd;
	SYSTEMTIME stm;
	HANDLE hFind;

	hFind = FindFirstFile(fpath, &wfd);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return NULL;
	}

	pfi->low_size = wfd.nFileSizeLow;
	pfi->high_size = wfd.nFileSizeHigh;

	pfi->is_dir = ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0);

	FileTimeToSystemTime(&wfd.ftCreationTime, &stm);
	_systime_to_xdate(&stm, &pfi->create_time);

	FileTimeToSystemTime(&wfd.ftLastAccessTime, &stm);
	_systime_to_xdate(&stm, &pfi->access_time);

	FileTimeToSystemTime(&wfd.ftLastWriteTime, &stm);
	_systime_to_xdate(&stm, &pfi->write_time);

	CopyMemory(pfi->file_name, wfd.cFileName, META_LEN * sizeof(tchar_t));

	return hFind;
}

bool_t _file_find_next(res_find_t ff, file_info_t* pfi)
{
	WIN32_FIND_DATA wfd;
	SYSTEMTIME stm;

	if (!FindNextFile(ff, &wfd))
	{
		return 0;
	}

	pfi->low_size = wfd.nFileSizeLow;
	pfi->high_size = wfd.nFileSizeHigh;

	pfi->is_dir = ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0);

	FileTimeToSystemTime(&wfd.ftCreationTime, &stm);
	_systime_to_xdate(&stm, &pfi->create_time);

	FileTimeToSystemTime(&wfd.ftLastAccessTime, &stm);
	_systime_to_xdate(&stm, &pfi->access_time);

	FileTimeToSystemTime(&wfd.ftLastWriteTime, &stm);
	_systime_to_xdate(&stm, &pfi->write_time);

	CopyMemory(pfi->file_name, wfd.cFileName, META_LEN * sizeof(tchar_t));

	return 1;
}

void _file_find_close(res_find_t ff)
{
	FindClose(ff);
}
#endif /*XDK_SUPPORT_FILE_FIND*/

#endif //XDK_SUPPORT_FILE