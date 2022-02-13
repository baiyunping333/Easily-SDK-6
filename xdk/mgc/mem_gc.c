/*
 * Copyright (c) 1999-2001, 2010 Greg Haerr <greg@censoft.com>
 *
 * 1bpp Packed Linear Video Driver for Microwindows (MSB first bit order)
 *
 * 	In this driver, linelen is line byte length, not line pixel length
 */

#include "mgc.h"
#include "mdev.h"
#include "mpap.h"
#include "mfnt.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"
#include "../xdkoem.h"
#include "../xdkutil.h"

typedef struct _memo_context_t{
	handle_head head;

	mem_device_ptr device;
	device_t handle;

	bitmap_file_head_t bitmap_head;
}memo_context_t;

static const mem_device_ptr select_device(const tchar_t* devName)
{
	if (xsicmp(devName, MGC_DEVICE_BITMAP_MONOCHROME) == 0)
		return &monochrome_bitmap_device;
	else if (xsicmp(devName, MGC_DEVICE_BITMAP_GRAYSCALE) == 0)
		return &grayscale_bitmap_device;
	else if (xsicmp(devName, MGC_DEVICE_BITMAP_TRUECOLOR16) == 0)
		return &truecolor16_bitmap_device;
	else if (xsicmp(devName, MGC_DEVICE_BITMAP_TRUECOLOR24) == 0)
		return &truecolor24_bitmap_device;
	else if (xsicmp(devName, MGC_DEVICE_BITMAP_TRUECOLOR32) == 0)
		return &truecolor32_bitmap_device;
	else
	{
		set_last_error(_T("select_device"), _T("unknown memory device"), -1);
		return NULL;
	}
}

static const mem_font_ptr select_font(const tchar_t* fntName)
{
	if (xsicmp(fntName, MGC_FONT_FIXED) == 0)
		return &font_Fixed;
	else
	{
		set_last_error(_T("select_font"), _T("unknown memory font"), -1);
		return NULL;
	}
}

visual_t mgc_create(const tchar_t* devName, const tchar_t* formName, int width, int height, int dpi)
{
	memo_context_t* pgc;
	dev_prn_t prn = { 0 };
	dword_t total, pixels;

	TRY_CATCH;

	pgc = (memo_context_t*)xmem_alloc(sizeof(memo_context_t));
	pgc->head.tag = _VISUAL_MEMORY;

	pgc->device = select_device(devName);
	if (!pgc->device)
	{
		raise_user_error(_T("mgc_create"), _T("select_device"));
	}

	if (!select_paper(formName, &prn))
	{
		prn.paper_width = (int)((float)width * MMPERPT * 10.0);
		prn.paper_height = (int)((float)height * MMPERPT * 10.0);
	}

	xscpy(prn.devname, devName);

	pgc->handle = (*(pgc->device->openDevice))(&prn, dpi);
	if (!pgc->handle)
	{
		raise_user_error(_T("mgc_create"), _T("openDevice"));
	}

	(*(pgc->device->getSize))(pgc->handle, &total, &pixels);

	pgc->bitmap_head.flag = BMP_FLAG;
	pgc->bitmap_head.fsize = BMP_FILEHEADER_SIZE + total;
	pgc->bitmap_head.offset = BMP_FILEHEADER_SIZE + (total - pixels);

	END_CATCH;

	return &(pgc->head);

ONERROR:
	XDK_TRACE_LAST;

	if (pgc)
	{
		xmem_free(pgc);
	}

	return NULL;
}

void mgc_destroy(visual_t gc)
{
	memo_context_t* pgc = (memo_context_t*)gc;

	XDK_ASSERT(gc && gc->tag == _VISUAL_MEMORY);

	if (pgc->device)
	{
		(*(pgc->device->closeDevice))(pgc->handle);
	}

	xmem_free(pgc);
}

void mgc_get_point(visual_t gc, xcolor_t* pxc, const xpoint_t* ppt)
{
	memo_context_t* pgc = (memo_context_t*)gc;

	XDK_ASSERT(gc && gc->tag == _VISUAL_MEMORY);

	(*(pgc->device->getPoint))(pgc->handle, ppt, pxc);
}

void mgc_set_point(visual_t gc, const xcolor_t* pxc, const xpoint_t* ppt, int rop)
{
	memo_context_t* pgc = (memo_context_t*)gc;

	XDK_ASSERT(gc && gc->tag == _VISUAL_MEMORY);

	(*(pgc->device->setPoint))(pgc->handle, ppt, pxc, rop);
}

dword_t mgc_save_bytes(visual_t gc, byte_t* buf, dword_t max)
{
	memo_context_t* pgc = (memo_context_t*)gc;
	dword_t total = 0;

	XDK_ASSERT(gc && gc->tag == _VISUAL_MEMORY);

	total += xbmp_set_head(&pgc->bitmap_head, ((buf)? (buf + total) : NULL), (max - total));
	total += (*(pgc->device->getBitmap))(pgc->handle, ((buf) ? (buf + total) : NULL), (max - total));

	return total;
}

bool_t mgc_text_size(visual_t gc, const xfont_t* pxf, const tchar_t* str, int len, xsize_t* pxs)
{
	memo_context_t* pgc = (memo_context_t*)gc;

	mem_font_ptr pmf;
	font_t fnt = NULL;
	int n;
	xsize_t se;

	XDK_ASSERT(gc && gc->tag == _VISUAL_MEMORY);

	TRY_CATCH;

	if (len < 0) len = xslen(str);

	pmf = select_font(MGC_FONT_FIXED);
	if (!pmf)
	{
		raise_user_error(_T("mgc_text_size"), _T("select_font"));
	}

	fnt = (*pmf->createFont)(pxf);
	if (!fnt)
	{
		raise_user_error(_T("mgc_text_size"), _T("createFont"));
	}

	if (pxs)
	{
		pxs->w = 0;
		pxs->h = 0;
	}

	n = 0;
	while (n < len)
	{
		(*pmf->getCharSize)(fnt, (str + n), &se);
		if (pxs)
		{
			pxs->w += se.w;
			if (pxs->h < se.h)
				pxs->h = se.h;
		}

#if defined(_UNICODE) || defined(UNICODE)
		n += ucs_sequence(*(str + n));
#else
		n += mbs_sequence(*(str + n));
#endif
	}

	(*pmf->destroyFont)(fnt);
	fnt = NULL;

	END_CATCH;

	return bool_true;
ONERROR:
	if (fnt)
		(*pmf->destroyFont)(fnt);

	return bool_false;
}

bool_t mgc_text_out(visual_t gc, const xfont_t* pxf, const xpoint_t* ppt, int rop, const tchar_t* str, int len)
{
	memo_context_t* pgc = (memo_context_t*)gc;
	dword_t total = 0;
	mem_font_ptr pmf;
	font_t fnt = NULL;
	font_metrix_t fm = { 0 };
	mem_pixmap_ptr pix = NULL;
	xcolor_t xc;
	int x, y, w, n;

	XDK_ASSERT(gc && gc->tag == _VISUAL_MEMORY);

	TRY_CATCH;

	if (len < 0) len = xslen(str);

	pmf = select_font(MGC_FONT_FIXED);
	if (!pmf)
	{
		raise_user_error(_T("mgc_text_out"), _T("select_font"));
	}

	fnt = (*pmf->createFont)(pxf);
	if (!fnt)
	{
		raise_user_error(_T("mgc_text_out"), _T("createFont"));
	}

	(*pmf->getFontMetrix)(fnt, NULL, &fm);

	pix = alloc_pixmap(fm.width, fm.height);
	if (!pix)
	{
		raise_user_error(_T("mgc_text_out"), _T("alloc_pixmap"));
	}

	x = ppt->x;
	y = ppt->y;

	parse_xcolor(&xc, pxf->color);

	while (len)
	{
		clean_pixmap(pix);
		w = (*pmf->getCharPixmap)(fnt, str, pix);
		pix->fg_color = PUT_PIXVAL(0, xc.r, xc.g, xc.b);
		pix->bg_used = 0;

		(*(pgc->device->drawPixmap))(pgc->handle, x, y, pix->width, pix->height, pix, 0, 0, rop);
		x += w;

#if defined(_UNICODE) || defined(UNICODE)
		n = 1;
#else
		n = mbs_sequence(str);
#endif
		str += n;
		len -= n;
	}

	free_pixmap(pix);
	pix = NULL;

	(*pmf->destroyFont)(fnt);
	fnt = NULL;

	END_CATCH;

	return bool_true;
ONERROR:

	if (pix) free_pixmap(pix);
	if (fnt) (*pmf->destroyFont)(fnt);

	return bool_false;
}

