﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc device printer document

	@module	if_printer_win.c | windows implement file

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

#ifdef XDU_SUPPORT_CONTEXT_PRINTER

bool_t _default_printer_mode(dev_prn_t* pmod)
{
	TCHAR szPrinter[MAX_PATH];
	PRINTER_DEFAULTS pdf;
	HANDLE hPrinter;
	DEVMODE* pdev = NULL;

	TCHAR* tmp;
	int size;

	xscpy(szPrinter, _T(""));
	GetProfileString(_T("windows"), _T("device"), _T(""), szPrinter, MAX_PATH);

	tmp = _tcsstr(szPrinter, _T(","));
	if (tmp != NULL)
		*tmp = '\0';

	pdf.pDatatype = NULL;
	pdf.pDevMode = NULL;
	pdf.DesiredAccess = PRINTER_ACCESS_USE;
	if (!OpenPrinter(szPrinter, &hPrinter, NULL))
		return 0;

	size = DocumentProperties(NULL, hPrinter, szPrinter, NULL, NULL, 0);
	if (size < 0)
	{
		ClosePrinter(hPrinter);
		return 0;
	}

	pdev = (DEVMODE*)GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size));
	pdev->dmSize = sizeof(DEVMODE);//(WORD)size;

	size = DocumentProperties(NULL, hPrinter, szPrinter, pdev, NULL, DM_OUT_BUFFER);

	pmod->landscape = (pdev->dmOrientation == DMORIENT_LANDSCAPE) ? 1 : 0;
	pmod->paper_width = pdev->dmPaperWidth;
	pmod->paper_height = pdev->dmPaperLength;

	ClosePrinter(hPrinter);

	GlobalUnlock(pdev);
	GlobalFree(pdev);

	return 1;
}

bool_t _setup_printer_mode(res_win_t wnd, dev_prn_t* pmod)
{
	TCHAR szPrinter[MAX_PATH];
	PRINTER_DEFAULTS pdf;
	HANDLE hPrinter;
	DEVMODE* pdev = NULL;

	TCHAR* tmp;
	int size;

	if (pmod->devname[0] != _T('\0'))
	{
		xscpy(szPrinter, (TCHAR*)pmod->devname);
	}
	else
	{
		xscpy(szPrinter, _T(""));
		GetProfileString(_T("windows"), _T("device"), _T(""), szPrinter, MAX_PATH);

		tmp = _tcsstr(szPrinter, _T(","));
		if (tmp != NULL)
			*tmp = '\0';
	}

	pdf.pDatatype = NULL;
	pdf.pDevMode = NULL;
	pdf.DesiredAccess = PRINTER_ACCESS_USE;
	if (!OpenPrinter(szPrinter, &hPrinter, NULL))
		return 0;

	size = DocumentProperties(wnd, hPrinter, szPrinter, NULL, NULL, 0);
	if (size < 0)
	{
		ClosePrinter(hPrinter);
		return 0;
	}

	pdev = (DEVMODE*)GlobalLock(GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, size));
	pdev->dmSize = sizeof(DEVMODE);//(WORD)size;

	size = DocumentProperties(wnd, hPrinter, szPrinter, pdev, pdev, DM_IN_PROMPT | DM_IN_BUFFER | DM_OUT_BUFFER);
	
	pmod->landscape = (pdev->dmOrientation == DMORIENT_LANDSCAPE) ? 1 : 0;
	pmod->paper_width = pdev->dmPaperWidth;
	pmod->paper_height = pdev->dmPaperLength;

	ClosePrinter(hPrinter);

	GlobalUnlock(pdev);
	GlobalFree(pdev);

	return 1;
}

visual_t _create_printer_context(const dev_prn_t* pmod)
{
	HDC hDC;
	HANDLE hPrinter;
	DEVMODE* pdev;
	int size;
	tchar_t szPrinter[1024] = { 0 };
	tchar_t* tmp;
	win32_context_t* pct;

	if (pmod && _tcslen(pmod->devname))
	{
		xscpy(szPrinter, pmod->devname);
	}
	else
	{
		GetProfileString(_T("windows"), _T("device"), _T(""), szPrinter, 1024);
		tmp = (tchar_t*)xsstr(szPrinter, _T(","));
		if (tmp != NULL)
			*tmp = _T('\0');
	}

	if (!OpenPrinter(szPrinter, &hPrinter, NULL))
		return 0;

	size = DocumentProperties(NULL, hPrinter, szPrinter, NULL, NULL, 0);
	if (size < 0)
	{
		ClosePrinter(hPrinter);
		return 0;
	}

	pdev = (DEVMODE*)GlobalLock(GlobalAlloc(GHND, size));
	pdev->dmSize = sizeof(DEVMODE);

	size = DocumentProperties(NULL, hPrinter, szPrinter, pdev, NULL, DM_OUT_BUFFER);
	if (size < 0)
	{
		ClosePrinter(hPrinter);
		GlobalUnlock(pdev);
		GlobalFree(pdev);
		return 0;
	}

	size = 0;
	if (pmod && pmod->paper_width > 0 && pmod->paper_height > 0)
	{
		pdev->dmFields |= (DM_PAPERSIZE | DM_PAPERWIDTH | DM_PAPERLENGTH);
		pdev->dmPaperSize = DMPAPER_USER;
		pdev->dmPaperWidth = pmod->paper_width;
		pdev->dmPaperLength = pmod->paper_height;
		size = 1;
	}

	if (pmod && pmod->landscape)
	{
		pdev->dmFields |= DM_ORIENTATION;
		pdev->dmOrientation = DMORIENT_LANDSCAPE;
		size = 1;
	}

	if (size)
	{
		size = DocumentProperties(NULL, hPrinter, szPrinter, pdev, pdev, DM_OUT_BUFFER | DM_IN_BUFFER);
	}

	ClosePrinter(hPrinter);

	hDC = CreateDC(_T("WINSPOOL"), szPrinter, NULL, pdev);

	GlobalUnlock(pdev);
	GlobalFree(pdev);

	pct = (win32_context_t*)xmem_alloc(sizeof(win32_context_t));
	pct->context = hDC;
	pct->type = CONTEXT_PRINTER;

	return (visual_t)pct;
}

void _destroy_printer_context(visual_t rdc)
{
	win32_context_t* pct = (win32_context_t*)rdc;

	DeleteDC(pct->context);

	xmem_free(pct);
}

void _begin_page(visual_t rdc)
{
	win32_context_t* pct = (win32_context_t*)rdc;

	StartPage(pct->context);
}

void _end_page(visual_t rdc)
{
	win32_context_t* pct = (win32_context_t*)rdc;

	EndPage(pct->context);
}

void _begin_doc(visual_t rdc, const tchar_t* docname)
{
	win32_context_t* pct = (win32_context_t*)rdc;
	DOCINFO df = { 0 };

	df.cbSize = sizeof(df);
	df.fwType = 0;
	df.lpszDatatype = NULL;
	df.lpszDocName = (LPTSTR)docname;
	df.lpszOutput = NULL;

	StartDoc(pct->context, &df);
}

void _end_doc(visual_t rdc)
{
	win32_context_t* pct = (win32_context_t*)rdc;

	EndDoc(pct->context);
}

#endif //XDU_SUPPORT_CONTEXT_PRINTER
