﻿/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc bitmap document

	@module	if_bitmap_win.c | windows implement file

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

#ifdef XDU_SUPPORT_CONTEXT_BITMAP

#ifndef WINCE
#pragma comment(lib, "Msimg32.lib")
#endif

#ifdef WINCE
static int MulDiv(int a, int b, int c)
{
	return (int)((float)a * (float)b / (float)c);
}
#endif

static void _CenterRect(RECT* pRect, int src_width, int src_height)
{
	if (pRect->right - pRect->left > src_width)
	{
		pRect->left = pRect->left + (pRect->right - pRect->left - src_width) / 2;
		pRect->right = pRect->left + src_width;
	}
	if (pRect->bottom - pRect->top > src_height)
	{
		pRect->top = pRect->top + (pRect->bottom - pRect->top - src_height) / 2;
		pRect->bottom = pRect->top + src_height;
	}
}

void _destroy_bitmap(bitmap_t bmp)
{
	win32_bitmap_t* pwb = (win32_bitmap_t*)bmp;

	XDK_ASSERT(bmp->tag == _HANDLE_BITMAP);

	DeleteObject(pwb->bitmap);

	xmem_free(pwb);
}

void _get_bitmap_size(bitmap_t rb, int* pw, int* ph)
{
	win32_bitmap_t* pwb = (win32_bitmap_t*)rb;

	BITMAP bmp;

	GetObject(pwb->bitmap, sizeof(bmp), (void*)&bmp);

	*pw = bmp.bmWidth;
	*ph = bmp.bmHeight;
}

bitmap_t _create_context_bitmap(visual_t rdc)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	win32_bitmap_t* pwb;

	HDC hDC = (HDC)(ctx->context);
	HBITMAP hbmp;

	if (ctx->type != CONTEXT_MEMORY)
		return NULL;

	hbmp = (HBITMAP)GetCurrentObject(hDC, OBJ_BITMAP);
	if (!hbmp)
		return NULL;

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

	pwb->bitmap = (HBITMAP)CopyImage(hbmp, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_COPYRETURNORG);

	return (bitmap_t)&pwb->head;
}

bitmap_t _create_color_bitmap(visual_t rdc, const xcolor_t* pxc, int w, int h)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	HDC memDC;
	HBRUSH hBrush;
	HBITMAP newBmp, orgBmp;
	RECT rt;

	newBmp = CreateCompatibleBitmap(hDC, w, h);
	memDC = CreateCompatibleDC(hDC);
	orgBmp = (HBITMAP)SelectObject(memDC, newBmp);

	rt.left = rt.top = 0;
	rt.right = w;
	rt.bottom = h;

	hBrush = CreateSolidBrush(RGB(pxc->r, pxc->g, pxc->b));
	FillRect(memDC, &rt, hBrush);
	DeleteObject(hBrush);

	SelectObject(memDC, orgBmp);
	DeleteDC(memDC);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

	pwb->bitmap = newBmp;

	return (bitmap_t)&pwb->head;
}

bitmap_t _create_pattern_bitmap(visual_t rdc, const xcolor_t* pxc_front, const xcolor_t* pxc_back, int w, int h, const tchar_t* lay)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	HDC memDC;
	HBRUSH hBrush;
	HPEN newPen, orgPen;
	HBITMAP newBmp, orgBmp;
	RECT rt;
	POINT pt[2];

	newBmp = CreateCompatibleBitmap(hDC, w, h);
	memDC = CreateCompatibleDC(hDC);
	orgBmp = (HBITMAP)SelectObject(memDC, newBmp);

	rt.left = rt.top = 0;
	rt.right = w;
	rt.bottom = h;

	hBrush = CreateSolidBrush(RGB(pxc_back->r, pxc_back->g, pxc_back->b));
	FillRect(memDC, &rt, hBrush);
	DeleteObject(hBrush);

	newPen = CreatePen(PS_SOLID, 2, RGB(pxc_front->r, pxc_front->g, pxc_front->b));
	orgPen = (HPEN)SelectObject(memDC, newPen);

	if (lay &&_tcsicmp(lay, GDI_ATTR_LAYOUT_VERT) == 0)
	{
		pt[0].x = 2;
		pt[0].y = 2;
		pt[1].x = 2;
		pt[1].y = 4;
	}
	else
	{
		pt[0].x = 0;
		pt[0].y = 2;
		pt[1].x = 4;
		pt[1].y = 2;
	}

	MoveToEx(memDC, pt[0].x, pt[0].y, NULL);
	LineTo(memDC, pt[1].x, pt[1].y);

	newPen = (HPEN)SelectObject(memDC, orgPen);
	DeleteObject(newPen);

	SelectObject(memDC, orgBmp);
	DeleteDC(memDC);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

	pwb->bitmap = newBmp;

	return (bitmap_t)&pwb->head;
}

bitmap_t _create_gradient_bitmap(visual_t rdc, const xcolor_t* pxc_brim, const xcolor_t* pxc_core, int w, int h, const tchar_t* type)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	RECT rt;
	HBITMAP newBmp, orgBmp;
	HDC memDC;
	COLORREF clr_brim, clr_core;
	TRIVERTEX tv[4];
	GRADIENT_RECT gr[2];
	unsigned int mode = 0;

	rt.left = 0;
	rt.top = 0;
	rt.right = w;
	rt.bottom = h;

	newBmp = CreateCompatibleBitmap(hDC, rt.right - rt.left, rt.bottom - rt.top);
	memDC = CreateCompatibleDC(hDC);
	orgBmp = (HBITMAP)SelectObject(memDC, newBmp);

	clr_brim = RGB(pxc_brim->r, pxc_brim->g, pxc_brim->b);
	clr_core = RGB(pxc_core->r, pxc_core->g, pxc_core->b);

	if (type && _tcsicmp(type, GDI_ATTR_GRADIENT_VERT) == 0)
	{
		tv[0].x = rt.left;
		tv[0].y = rt.top;
		tv[0].Red = GetRValue(clr_brim) << 8;
		tv[0].Green = GetGValue(clr_brim) << 8;
		tv[0].Blue = GetBValue(clr_brim) << 8;
		tv[0].Alpha = 255;

		tv[1].x = rt.right;
		tv[1].y = (rt.top + rt.bottom) / 2;
		tv[1].Red = GetRValue(clr_core) << 8;
		tv[1].Green = GetGValue(clr_core) << 8;
		tv[1].Blue = GetBValue(clr_core) << 8;
		tv[1].Alpha = 255;

		tv[2].x = rt.left;
		tv[2].y = (rt.top + rt.bottom) / 2;
		tv[2].Red = GetRValue(clr_core) << 8;
		tv[2].Green = GetGValue(clr_core) << 8;
		tv[2].Blue = GetBValue(clr_core) << 8;
		tv[2].Alpha = 255;

		tv[3].x = rt.right;
		tv[3].y = rt.bottom;
		tv[3].Red = GetRValue(clr_brim) << 8;
		tv[3].Green = GetGValue(clr_brim) << 8;
		tv[3].Blue = GetBValue(clr_brim) << 8;
		tv[3].Alpha = 255;

		mode = GRADIENT_FILL_RECT_V;
	}
	else if (type && _tcsicmp(type, GDI_ATTR_GRADIENT_HORZ) == 0)
	{
		tv[0].x = rt.left;
		tv[0].y = rt.top;
		tv[0].Red = GetRValue(clr_brim) << 8;
		tv[0].Green = GetGValue(clr_brim) << 8;
		tv[0].Blue = GetBValue(clr_brim) << 8;
		tv[0].Alpha = 255;

		tv[1].x = (rt.left + rt.right) / 2;
		tv[1].y = rt.bottom;
		tv[1].Red = GetRValue(clr_core) << 8;
		tv[1].Green = GetGValue(clr_core) << 8;
		tv[1].Blue = GetBValue(clr_core) << 8;
		tv[1].Alpha = 255;

		tv[2].x = (rt.left + rt.right) / 2;
		tv[2].y = rt.top;
		tv[2].Red = GetRValue(clr_core) << 8;
		tv[2].Green = GetGValue(clr_core) << 8;
		tv[2].Blue = GetBValue(clr_core) << 8;
		tv[2].Alpha = 255;

		tv[3].x = rt.right;
		tv[3].y = rt.bottom;
		tv[3].Red = GetRValue(clr_brim) << 8;
		tv[3].Green = GetGValue(clr_brim) << 8;
		tv[3].Blue = GetBValue(clr_brim) << 8;
		tv[3].Alpha = 255;

		mode = GRADIENT_FILL_RECT_H;
	}

	gr[0].UpperLeft = 0;
	gr[0].LowerRight = 1;
	gr[1].UpperLeft = 2;
	gr[1].LowerRight = 3;

	GradientFill(memDC, tv, 4, &gr, 2, mode);

	newBmp = (HBITMAP)SelectObject(memDC, orgBmp);
	DeleteDC(memDC);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;
	pwb->bitmap = newBmp;

	return (bitmap_t)&pwb->head;
}

bitmap_t _create_code128_bitmap(visual_t rdc, int w, int h, const unsigned char* bar_buf, dword_t bar_len, const tchar_t* text)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	HDC winDC,memDC;
	HBRUSH wBrush, bBrush;
	HBITMAP newBmp, orgBmp;
	HFONT newFont, orgFont;
	LOGFONT lf = { 0 };
	RECT rt;
	int unit;
	DWORD i;
	int black,span;
	
	winDC = GetDC(NULL);
	unit = GetDeviceCaps(hDC, LOGPIXELSX) / GetDeviceCaps(winDC, LOGPIXELSX);
	ReleaseDC(NULL, winDC);

	wBrush = CreateSolidBrush(RGB(255, 255, 255));
	bBrush = CreateSolidBrush(RGB(0, 0, 0));

	newBmp = CreateCompatibleBitmap(hDC, w, h);
	memDC = CreateCompatibleDC(hDC);

	SetBkMode(memDC, TRANSPARENT);

	orgBmp = (HBITMAP)SelectObject(memDC, newBmp);

	rt.left = 0;
	rt.right = w;
	rt.top = 0;
	rt.bottom = h;

	FillRect(memDC, &rt, wBrush);

	rt.left = unit;
	rt.right = unit;
	rt.top = unit;
	rt.bottom = h * 2 / 3 - unit;

	black = 0;
	for (i = 0; i < bar_len; i++)
	{
		span = (bar_buf[i] - '0');
		rt.right = rt.left + span * unit;

		black = (black) ? 0 : 1;

		if (black)
		{
			FillRect(memDC, &rt, bBrush);
		}
		
		rt.left = rt.right;
	}

	orgFont = (HFONT)GetStockObject(SYSTEM_FONT);
	GetObject(orgFont, sizeof(LOGFONT), (void*)&lf);
	lf.lfHeight = -MulDiv(10, GetDeviceCaps(hDC, LOGPIXELSY), 72);

	newFont = CreateFontIndirect(&lf);
	orgFont = (HFONT)SelectObject(memDC, newFont);

	rt.left = 0;
	rt.top = h * 2 / 3;
	rt.bottom = h;

	DrawText(memDC, text, -1, &rt, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	newFont = (HFONT)SelectObject(memDC, orgFont);
	DeleteObject(newFont);

	SelectObject(memDC, orgBmp);
	DeleteDC(memDC);

	DeleteObject(wBrush);
	DeleteObject(bBrush);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;
	pwb->bitmap = newBmp;

	return (bitmap_t)&pwb->head;
}

bitmap_t _create_pdf417_bitmap(visual_t rdc, int w, int h, const unsigned char* bar_buf, dword_t bar_len, int rows, int cols)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	HDC winDC, memDC;
	HBRUSH wBrush, bBrush;
	HBITMAP newBmp, orgBmp;
	RECT rt;
	int unit;

	unsigned char b, c;
	int len, i,j, black;

	winDC = GetDC(NULL);
	unit = GetDeviceCaps(hDC, LOGPIXELSX) / GetDeviceCaps(winDC, LOGPIXELSX);
	if (unit < 2)
		unit = 2;
	ReleaseDC(NULL, winDC);

	wBrush = CreateSolidBrush(RGB(255, 255, 255));
	bBrush = CreateSolidBrush(RGB(0, 0, 0));

	newBmp = CreateCompatibleBitmap(hDC, w, h);
	memDC = CreateCompatibleDC(hDC);

	orgBmp = (HBITMAP)SelectObject(memDC, newBmp);

	rt.left = 0;
	rt.right = w;
	rt.top = 0;
	rt.bottom = h;

	FillRect(memDC, &rt, wBrush);

	rt.left = rt.right = unit;
	rt.top = rt.bottom = unit;

	len = 0;
	black = 0;
	for (i = 0; i < rows; i++)
	{
		rt.left = rt.right = unit;
		rt.top = rt.bottom;
		rt.bottom = rt.top + 2 * unit;

		for (j = 0; j < cols; j++)
		{
			c = *(bar_buf + i * cols + j);
			b = 0x80;

			while (b)
			{
				rt.left = rt.right;
				rt.right = rt.left + unit;

				black = (c & b) ? 0 : 1;

				if (black)
				{
					FillRect(memDC, &rt, bBrush);
				}

				b = b >> 1;
			}
		}
	}
	
	SelectObject(memDC, orgBmp);
	DeleteDC(memDC);

	DeleteObject(wBrush);
	DeleteObject(bBrush);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

	pwb->bitmap = newBmp;

	return (bitmap_t)&pwb->head;
}

bitmap_t _create_qrcode_bitmap(visual_t rdc, int w, int h, const unsigned char* bar_buf, dword_t bar_len, int rows, int cols)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	HDC winDC, memDC;
	HBRUSH wBrush, bBrush;
	HBITMAP newBmp, orgBmp;
	RECT rt;
	int unit;

	unsigned char b, c;
	int len, i,j, black;

	winDC = GetDC(NULL);
	unit = GetDeviceCaps(hDC, LOGPIXELSX) / GetDeviceCaps(winDC, LOGPIXELSX);
	if (unit < 2)
		unit = 2;
	ReleaseDC(NULL, winDC);

	wBrush = CreateSolidBrush(RGB(255, 255, 255));
	bBrush = CreateSolidBrush(RGB(0, 0, 0));

	newBmp = CreateCompatibleBitmap(hDC, w, h);
	memDC = CreateCompatibleDC(hDC);

	orgBmp = (HBITMAP)SelectObject(memDC, newBmp);

	rt.left = 0;
	rt.right = w;
	rt.top = 0;
	rt.bottom = h;

	FillRect(memDC, &rt, wBrush);

	rt.left = unit;
	rt.right = w;
	rt.top = unit;
	rt.bottom = h;

	rt.left = rt.right = unit;
	rt.top = rt.bottom = unit;

	len = 0;
	black = 0;
	for (i = 0; i < rows; i++)
	{
		rt.left = rt.right = unit;
		rt.top = rt.bottom;
		rt.bottom = rt.top + unit;

		for (j = 0; j < cols; j++)
		{
			c = *(bar_buf + i * cols + j);
			b = 0x80;

			while (b)
			{
				rt.left = rt.right;
				rt.right = rt.left + unit;

				black = (c & b) ? 1 : 0;

				if (black)
				{
					FillRect(memDC, &rt, bBrush);
				}

				b = b >> 1;
			}
		}
	}

	SelectObject(memDC, orgBmp);
	DeleteDC(memDC);

	DeleteObject(wBrush);
	DeleteObject(bBrush);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

	pwb->bitmap = newBmp;

	return (bitmap_t)&pwb->head;
}

bitmap_t _create_storage_bitmap(visual_t rdc, const tchar_t* filename)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	HANDLE handle;
	IPicture* p = NULL;
	IStream* s = NULL;
	HGLOBAL hb = NULL;
	dword_t size = 0;
	void* buf = NULL;

	WIN32_FILE_ATTRIBUTE_DATA ad = { 0 };
	HANDLE hFile;
	DWORD dw = 0;
	
	if (is_null(filename))
		return 0;

	if (!GetFileAttributesEx(filename, GetFileExInfoStandard, &ad))
		return 0;

	if (!ad.nFileSizeLow || ad.nFileSizeHigh)
		return 0;

	hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((HANDLE)hFile == INVALID_HANDLE_VALUE)
	{
		return 0;
	}
	
	size = ad.nFileSizeLow;
	hb = GlobalAlloc(GHND, size);

	if (!hb)
		return 0;

	buf = GlobalLock(hb);

	if (!ReadFile(hFile, buf, (DWORD)size, &dw, NULL))
	{
		GlobalUnlock(hb);
		GlobalFree(hb);
		return 0;
	}

	CloseHandle(hFile);

	CreateStreamOnHGlobal(hb, FALSE, &s);
	if (s == NULL)
	{
		GlobalUnlock(hb);
		GlobalFree(hb);
		return 0;
	}

	OleLoadPicture(s, 0, FALSE, IID_IPicture, (void**)&p);
	s->Release();
	GlobalUnlock(hb);
	GlobalFree(hb);

	if (p == NULL)
		return NULL;

	p->get_Handle((OLE_HANDLE *)&handle);

	handle = (HANDLE)CopyImage(handle, IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_COPYRETURNORG);

	p->Release();

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

	pwb->bitmap = (HBITMAP)handle;

	return (bitmap_t)&pwb->head;
}
/*******************************************************************************/

dword_t _get_bitmap_bytes(bitmap_t rb)
{
	win32_bitmap_t* pwb = (win32_bitmap_t*)rb;
	BITMAP bmp;
	WORD cClrBits;
	DWORD dwClrUsed;
	DWORD dwSizeImage;
	DWORD dwTotal;

	XDK_ASSERT(rb && rb->tag == _HANDLE_BITMAP);

	if (!GetObject(pwb->bitmap, sizeof(BITMAP), (LPSTR)&bmp))
		return 0;

	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else
		cClrBits = 32;

	if (cClrBits < 24)
		dwClrUsed = (1 << cClrBits);
	else
		dwClrUsed = 0;

	dwSizeImage = ((bmp.bmWidth * cClrBits + 31) & ~31) / 8 * bmp.bmHeight;

	dwTotal = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwClrUsed * sizeof(RGBQUAD) + dwSizeImage);

	return dwTotal;
}

bitmap_t _load_bitmap_from_bytes(visual_t rdc, const unsigned char* pb, dword_t bytes)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	PBITMAPINFO pbmi;
	BITMAPFILEHEADER bfh;
	LPBYTE lpBits;

	if (!pb)
		return NULL;

	if ((DWORD)bytes < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER))
		return NULL;

	CopyMemory((void*)&bfh, (void*)pb, sizeof(BITMAPFILEHEADER));

	if (bfh.bfType != 0x4d42)
		return NULL;

	if ((DWORD)bytes < bfh.bfSize)
		return NULL;

	pbmi = (PBITMAPINFO)(pb + sizeof(BITMAPFILEHEADER));
	lpBits = (LPBYTE)(pb + bfh.bfOffBits);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

#ifdef WINCE
	pwb->bitmap = CreateDIBSection(hDC, pbmi, DIB_RGB_COLORS, NULL, NULL, bfh.bfOffBits);
#else
	pwb->bitmap = CreateDIBitmap(hDC, (BITMAPINFOHEADER*)pbmi, CBM_INIT, lpBits, pbmi, DIB_RGB_COLORS);
#endif

	return (bitmap_t)&pwb->head;
}

dword_t _save_bitmap_to_bytes(visual_t rdc, bitmap_t rb, unsigned char* buf, dword_t max)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb = (win32_bitmap_t*)rb;

	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD    cClrBits;
	BITMAPFILEHEADER bfh;
	PBITMAPINFOHEADER pbih;
	LPBYTE lpBits;
	DWORD dwTotal;
#ifdef WINCE
	HBITMAP hSec;
	void* pbuf;
#endif

	XDK_ASSERT(rb && rb->tag == _HANDLE_BITMAP);

	if (!GetObject(pwb->bitmap, sizeof(BITMAP), (LPSTR)&bmp))
		return 0;

	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else
		cClrBits = 32;

	if (cClrBits < 24)
		pbmi = (PBITMAPINFO)xmem_alloc(sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (DWORD)(1 << cClrBits));
	else
		pbmi = (PBITMAPINFO)xmem_alloc(sizeof(BITMAPINFOHEADER));

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);
	else
		pbmi->bmiHeader.biClrUsed = 0;
	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;
	pbmi->bmiHeader.biClrImportant = 0;

	pbih = (PBITMAPINFOHEADER)pbmi;

	bfh.bfType = 0x4d42;        // 0x42 = "B" 0x4d = "M" 
	bfh.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD) + pbih->biSizeImage);
	bfh.bfReserved1 = 0;
	bfh.bfReserved2 = 0;
	bfh.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD));

	if (pbih->biSizeImage > (DWORD)max)
	{
		xmem_free(pbmi);
		return 0;
	}

	dwTotal = 0;
	if (buf)
	{
		CopyMemory((void*)(buf + dwTotal), (void*)&bfh, sizeof(BITMAPFILEHEADER));
	}
	dwTotal += sizeof(BITMAPFILEHEADER);

	if (buf)
	{
		CopyMemory((void*)(buf + dwTotal), (void*)pbih, sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD));
	}
	dwTotal += sizeof(BITMAPINFOHEADER) + pbih->biClrUsed * sizeof(RGBQUAD);

	if (buf)
	{
		lpBits = (LPBYTE)(buf + dwTotal);
	}
	else
	{
		lpBits = NULL;
	}
	dwTotal += pbih->biSizeImage;

	if (buf)
	{
#ifdef WINCE
		pbuf = NULL;
		hSec = CreateDIBSection(hDC, pbmi, DIB_RGB_COLORS, &pbuf, NULL, bfh.bfOffBits);
		if(!hSec)
		{
			xmem_free(pbmi);
			return 0;
		}
		CopyMemory((void*)lpBits, (void*)pbuf, pbih->biSizeImage);
		DeleteObject(hSec);
#else
		if (!GetDIBits(hDC, pwb->bitmap, 0, (WORD)pbih->biHeight, lpBits, pbmi, DIB_RGB_COLORS))
		{
			xmem_free(pbmi);
			return 0;
		}
#endif
	}

	xmem_free(pbmi);

	return dwTotal;
}

#ifdef XDU_SUPPORT_SHELL
bitmap_t _load_bitmap_from_icon(visual_t rdc, const tchar_t* iname)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	HICON hIcon;
	ICONINFO ico = { 0 };

	int w, h;
	RECT rt;
	HDC hdc;
	HBITMAP hbmp, horg;
	HBRUSH hBrush;

	if (_tcsicmp(iname, ICON_QUESTION) == 0)
		hIcon = LoadIcon(NULL, IDI_QUESTION);
	else if (_tcsicmp(iname, ICON_EXCLAMATION) == 0)
		hIcon = LoadIcon(NULL, IDI_EXCLAMATION);
	else if (_tcsicmp(iname, ICON_INFORMATION) == 0)
		hIcon = LoadIcon(NULL, IDI_EXCLAMATION);
	else if (_tcsicmp(iname, ICON_WARING) == 0)
		hIcon = LoadIcon(NULL, IDI_WARNING);
	else if (_tcsicmp(iname, ICON_ERROR) == 0)
		hIcon = LoadIcon(NULL, IDI_ERROR);
	else if (_tcsicmp(iname, ICON_HAND) == 0)
		hIcon = LoadIcon(NULL, IDI_HAND);
	else if (_tcsicmp(iname, ICON_ASTERISK) == 0)
		hIcon = LoadIcon(NULL, IDI_ASTERISK);
	else
		hIcon = LoadIcon(NULL, IDI_APPLICATION);

	GetIconInfo(hIcon, &ico);

	w = ico.xHotspot * 2;
	h = ico.yHotspot * 2;

	rt.top = rt.bottom = 0;
	rt.right = w;
	rt.bottom = h;

	hdc = CreateCompatibleDC(hDC);
	hbmp = CreateCompatibleBitmap(hDC, w, h);
	horg = (HBITMAP)SelectObject(hdc, hbmp);

	hBrush = CreateSolidBrush(RGB(250, 250, 250));

	FillRect(hdc, &rt, hBrush);

	DrawIcon(hdc, 0, 0, hIcon);

	DeleteObject(hBrush);
	hbmp = (HBITMAP)SelectObject(hdc, horg);
	DeleteDC(hdc);

	DeleteObject(ico.hbmColor);
	DeleteObject(ico.hbmMask);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

	pwb->bitmap = (HBITMAP)hbmp;

	return (bitmap_t)&pwb->head;
}

bitmap_t _load_bitmap_from_thumb(visual_t rdc, const tchar_t* file)
{
	win32_context_t* ctx = (win32_context_t*)rdc;
	HDC hDC = (HDC)(ctx->context);
	win32_bitmap_t* pwb;

	SHFILEINFO shi = { 0 };
	ICONINFO ico = { 0 };

	RECT rt;
	int w, h;
	HDC hdc;
	HBITMAP hbmp, horg;
	HBRUSH hBrush;

	SHGetFileInfo(file, 0, &shi, sizeof(shi), SHGFI_ICON);

	if (!shi.hIcon)
		return NULL;

	GetIconInfo(shi.hIcon, &ico);

	w = ico.xHotspot * 2;
	h = ico.yHotspot * 2;

	rt.top = rt.bottom = 0;
	rt.right = w;
	rt.bottom = h;

	hdc = CreateCompatibleDC(hDC);
	hbmp = CreateCompatibleBitmap(hDC, w, h);
	horg = (HBITMAP)SelectObject(hdc, hbmp);

	hBrush = CreateSolidBrush(RGB(250, 250, 250));

	FillRect(hdc, &rt, hBrush);

	DrawIcon(hdc, 0, 0, shi.hIcon);

	DeleteObject(hBrush);
	hbmp = (HBITMAP)SelectObject(hdc, horg);
	DeleteDC(hdc);

	DeleteObject(ico.hbmColor);
	DeleteObject(ico.hbmMask);

	pwb = (win32_bitmap_t*)xmem_alloc(sizeof(win32_bitmap_t));
	pwb->head.tag = _HANDLE_BITMAP;

	pwb->bitmap = (HBITMAP)hbmp;

	return (bitmap_t)&pwb->head;
}
#endif //XDU_SUPPORT_SHELL

#endif //XDU_SUPPORT_CONTEXT_BITMAP
