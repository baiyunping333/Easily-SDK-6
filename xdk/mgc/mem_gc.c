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
#include "../fit/fit.h"

typedef struct _memo_context_t{
	handle_head head;

	mem_device_ptr device;
	device_t handle;

	bitmap_file_head_t bitmap_head;
	int rop; /*raster operation mode*/
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

void mgc_destroy(visual_t mgc)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	if (pgc->device)
	{
		(*(pgc->device->closeDevice))(pgc->handle);
	}

	xmem_free(pgc);
}

int mgc_get_rop(visual_t mgc)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	return pgc->rop;
}

void mgc_set_rop(visual_t mgc, int rop)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	pgc->rop = rop;
}

void mgc_get_point(visual_t mgc, xcolor_t* pxc, const xpoint_t* ppt)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	(*(pgc->device->getPoint))(pgc->handle, ppt, pxc);
}

void mgc_set_point(visual_t mgc, const xcolor_t* pxc, const xpoint_t* ppt)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	(*(pgc->device->setPoint))(pgc->handle, ppt, pxc, pgc->rop);
}

dword_t mgc_save_bytes(visual_t mgc, byte_t* buf, dword_t max)
{
	memo_context_t* pgc = (memo_context_t*)mgc;
	dword_t total = 0;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	total += xbmp_set_head(&pgc->bitmap_head, ((buf)? (buf + total) : NULL), (max - total));
	total += (*(pgc->device->getBitmap))(pgc->handle, ((buf) ? (buf + total) : NULL), (max - total));

	return total;
}

void mgc_text_metric(visual_t mgc, const xfont_t* pxf, xsize_t* pxs)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	mem_font_ptr pmf;
	font_t fnt = NULL;
	font_metrix_t fm = { 0 };

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	XDK_ASSERT(pxf != NULL && pxs != NULL);

	TRY_CATCH;

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

	(*pmf->getFontMetrix)(fnt, NULL, &fm);

	pxs->w = fm.width;
	pxs->h = fm.height;

	(*pmf->destroyFont)(fnt);
	fnt = NULL;

	END_CATCH;

	return;
ONERROR:
	if (fnt)
		(*pmf->destroyFont)(fnt);

	return;
}

void mgc_text_size(visual_t mgc, const xfont_t* pxf, const tchar_t* str, int len, xsize_t* pxs)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	mem_font_ptr pmf;
	font_t fnt = NULL;
	int n;
	xsize_t se;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	XDK_ASSERT(pxf != NULL && pxs != NULL);

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

	pxs->w = 0;
	pxs->h = 0;

	n = 0;
	while (n < len)
	{
		(*pmf->getCharSize)(fnt, (str + n), &se);

		pxs->w += se.w;
		if (pxs->h < se.h)
			pxs->h = se.h;

#if defined(_UNICODE) || defined(UNICODE)
		n += ucs_sequence(*(str + n));
#else
		n += mbs_sequence(*(str + n));
#endif
	}

	(*pmf->destroyFont)(fnt);
	fnt = NULL;

	END_CATCH;

	return;
ONERROR:
	if (fnt)
		(*pmf->destroyFont)(fnt);

	return;
}

void mgc_text_out(visual_t mgc, const xfont_t* pxf, const xpoint_t* ppt, const tchar_t* str, int len)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	dword_t total = 0;
	mem_font_ptr pmf;
	font_t fnt = NULL;
	font_metrix_t fm = { 0 };
	mem_pixmap_ptr pix = NULL;
	xcolor_t xc;
	int x, y, w, n;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	XDK_ASSERT(pxf != NULL && ppt != NULL);

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

		(*(pgc->device->drawPixmap))(pgc->handle, x, y, pix->width, pix->height, pix, 0, 0, pgc->rop);
		x += w;

#if defined(_UNICODE) || defined(UNICODE)
		n = 1;
#else
		n = mbs_sequence(*str);
#endif
		str += n;
		len -= n;
	}

	free_pixmap(pix);
	pix = NULL;

	(*pmf->destroyFont)(fnt);
	fnt = NULL;

	END_CATCH;

	return;
ONERROR:

	if (pix) free_pixmap(pix);
	if (fnt) (*pmf->destroyFont)(fnt);

	return;
}

void mgc_text_rect(visual_t mgc, const xfont_t* pxf, const xface_t* pxa, const tchar_t* str, int len, xrect_t* pxr)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	mem_font_ptr pmf;
	font_t fnt = NULL;
	int n=0, total = 0;
	tchar_t pch[CHS_LEN + 1] = { 0 };
	xsize_t se;
	int w, h;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	XDK_ASSERT(pxf != NULL && pxr != NULL);

	TRY_CATCH;

	len = words_count(str, len);

	pmf = select_font(MGC_FONT_FIXED);
	if (!pmf)
	{
		raise_user_error(_T("mgc_text_rect"), _T("select_font"));
	}

	fnt = (*pmf->createFont)(pxf);
	if (!fnt)
	{
		raise_user_error(_T("mgc_text_rect"), _T("createFont"));
	}

	pxr->h = 0;

	w = 0;
	h = 0;
	n = 0;
	while (n++ < len)
	{
		total += peek_word((str + total), pch);
		(*pmf->getCharSize)(fnt, pch, &se);

		if (pxr->w < se.w)
		{
			pxr->w = se.w;
		}
		if (h < se.h)
		{
			h = se.h;
		}
		if (compare_text(pxa->text_wrap, -1, GDI_ATTR_TEXT_WRAP_WORDBREAK, -1, 1) == 0)
		{
			if (w + se.w > pxr->w)
			{
				if (is_null(pxa->line_height))
					pxr->h += h;
				else
					pxr->h += (int)((float)h * xstof(pxa->line_height));

				total -= xslen(pch);
				n--;
			}
			else
			{
				w += se.w;
			}
		}
		else if (compare_text(pxa->text_wrap, -1, GDI_ATTR_TEXT_WRAP_LINEBREAK, -1, 1) == 0)
		{
			if (pch[0] == _T('\n'))
			{
				if (is_null(pxa->line_height))
					pxr->h += h;
				else
					pxr->h += (int)((float)h * xstof(pxa->line_height));
			}
			else
			{
				w += se.w;
			}
		}
		else
		{
			w += se.w;
			if (pxr->h < se.h)
				pxr->h = se.h;
		}
	}

	(*pmf->destroyFont)(fnt);
	fnt = NULL;

	END_CATCH;

	return;
ONERROR:
	if (fnt)
		(*pmf->destroyFont)(fnt);

	return;
}

void mgc_text_indicate(visual_t mgc, const xfont_t* pxf, const xface_t* pxa, const tchar_t* str, int len, const xrect_t* pxr, xrect_t*pa, int n)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	mem_font_ptr pmf;
	font_t fnt = NULL;
	tchar_t pch[CHS_LEN + 1] = { 0 };
	int ind, total = 0;
	xsize_t se;
	int w, h, x, y;
	int *pw = NULL;
	int *pc = NULL;
	int li, ln = 0, lc = 0;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	XDK_ASSERT(pxf != NULL && pxr != NULL);

	TRY_CATCH;

	len = words_count(str, len);
	n = (len < n) ? len : n;
	if (!n)
	{
		raise_user_error(_T("mgc_text_indicate"), _T("empty text"));
	}

	pmf = select_font(MGC_FONT_FIXED);
	if (!pmf)
	{
		raise_user_error(_T("mgc_text_indicate"), _T("select_font"));
	}

	fnt = (*pmf->createFont)(pxf);
	if (!fnt)
	{
		raise_user_error(_T("mgc_text_indicate"), _T("createFont"));
	}

	pw = (int*)xmem_alloc(sizeof(int) * len);
	pc = (int*)xmem_alloc(sizeof(int) * len);

	w = 0;
	h = 0;
	ind = 0;
	while (ind < n)
	{
		total += peek_word((str + total), pch);

		(*pmf->getCharSize)(fnt, pch, &se);

		pa[ind].x = w;
		pa[ind].y = h;
		pa[ind].w = se.w;
		pa[ind].h = se.h;
		ind ++ ;

		if (compare_text(pxa->text_wrap, -1, GDI_ATTR_TEXT_WRAP_WORDBREAK, -1, 1) == 0)
		{
			if (lc && w + se.w > pxr->w)
			{
				if (is_null(pxa->line_height))
				{
					h += se.h;
				}
				else
				{
					h += (int)((float)se.h * xstof(pxa->line_height));
				}
				pw[ln] = w;
				pc[ln] = lc;
				ln++;
				lc = 0;
				w = 0;
				ind--;
			}
			else
			{
				lc++;
				w += se.w;
			}
		}
		else if (compare_text(pxa->text_wrap, -1, GDI_ATTR_TEXT_WRAP_LINEBREAK, -1, 1) == 0)
		{
			if (pch[0] == _T('\n'))
			{
				if (is_null(pxa->line_height))
				{
					h += se.h;
				}
				else
				{
					h += (int)((float)se.h * xstof(pxa->line_height));
				}
				pw[ln] = w;
				pc[ln] = lc + 1;
				ln++;
				lc = 0;
				w = 0;
			}
			else
			{
				lc++;
				w += se.w;
			}
		}
		else
		{
			w += se.w;
			h = 0;
			ln = 0;
			lc++;
		}
	}

	pw[ln] = w;
	pc[ln] = lc;
	ln++;

	h += (int)(pa[ind - 1].h * xstof(pxa->line_height));

	ind = 0;
	for (li = 0; li < ln; li++)
	{
		if (compare_text(pxa->text_align, -1, GDI_ATTR_TEXT_ALIGN_FAR, -1, 1) == 0)
		{
			x = pxr->w - pw[li];
		}
		else if (compare_text(pxa->text_align, -1, GDI_ATTR_TEXT_ALIGN_CENTER, -1, 1) == 0)
		{
			x = (pxr->w - pw[li]) / 2;
		}
		else
		{
			x = 0;
		}

		while (pc[li]--)
		{
			pa[ind].x += pxr->x + x;
			ind++;
		}
	}

	if (compare_text(pxa->line_align, -1, GDI_ATTR_TEXT_ALIGN_FAR, -1, 1) == 0)
	{
		y = pxr->h - h;
	}
	else if (compare_text(pxa->line_align, -1, GDI_ATTR_TEXT_ALIGN_CENTER, -1, 1) == 0)
	{
		y = (pxr->h - h) / 2;
	}
	else
	{
		y = 0;
	}
	for (ind = 0; ind < n; ind++)
	{
		pa[ind].y += (pxr->y + y);
	}

	xmem_free(pw);
	pw = NULL;
	xmem_free(pc);
	pc = NULL;

	(*pmf->destroyFont)(fnt);
	fnt = NULL;

	END_CATCH;

	return;
ONERROR:

	if (pw) xmem_free(pw);
	if (pc) xmem_free(pc);
	if (fnt) (*pmf->destroyFont)(fnt);

	return;
}

void mgc_draw_text(visual_t mgc, const xfont_t* pxf, const xface_t* pxa, const xrect_t* pxr, const tchar_t* str, int len)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	mem_font_ptr pmf;
	font_t fnt = NULL;
	font_metrix_t fm = { 0 };
	mem_pixmap_ptr pix = NULL;
	xcolor_t xc;
	tchar_t pch[CHS_LEN + 1] = { 0 };
	int n = 0, total = 0;
	xrect_t xr = { 0 };
	xrect_t* pa = NULL;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);

	XDK_ASSERT(pxf != NULL && pxr != NULL);

	TRY_CATCH;

	len = words_count(str, len);
	if (!len)
	{
		raise_user_error(_T("mgc_draw_text"), _T("empty text"));
	}

	pmf = select_font(MGC_FONT_FIXED);
	if (!pmf)
	{
		raise_user_error(_T("mgc_draw_text"), _T("select_font"));
	}

	fnt = (*pmf->createFont)(pxf);
	if (!fnt)
	{
		raise_user_error(_T("mgc_draw_text"), _T("createFont"));
	}

	(*pmf->getFontMetrix)(fnt, NULL, &fm);

	pix = alloc_pixmap(fm.width, fm.height);
	if (!pix)
	{
		raise_user_error(_T("mgc_draw_text"), _T("alloc_pixmap"));
	}

	pa = (xrect_t*)xmem_alloc(sizeof(xrect_t) * len);
	mgc_text_indicate(mgc, pxf, pxa, str, -1, pxr, pa, len);

	parse_xcolor(&xc, pxf->color);

	while (n < len)
	{
		total += peek_word((str + total), pch);

		clean_pixmap(pix);
		(*pmf->getCharPixmap)(fnt, pch, pix);
		pix->fg_color = PUT_PIXVAL(0, xc.r, xc.g, xc.b);
		pix->bg_used = 0;

		if (pt_in_rect(RECTPOINT(&(pa[n])), pxr))
		{
			(*(pgc->device->drawPixmap))(pgc->handle, pa[n].x, pa[n].y, pix->width, pix->height, pix, 0, 0, pgc->rop);
		}
		n++;
	}

	xmem_free(pa);
	pa = NULL;

	free_pixmap(pix);
	pix = NULL;

	(*pmf->destroyFont)(fnt);
	fnt = NULL;

	END_CATCH;

	return;
ONERROR:

	if (pa) xmem_free(pa);
	if (pix) free_pixmap(pix);
	if (fnt) (*pmf->destroyFont)(fnt);

	return;
}

void mgc_draw_line(visual_t mgc, const xpen_t* pxp, const xpoint_t* ppt1, const xpoint_t* ppt2)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);
	XDK_ASSERT(ppt1 != NULL && ppt2 != NULL && pxp != NULL);

	xcolor_t xc;
	int mo, mi;
	int x, y, w, h;
	xpoint_t* ppt;
	int n, fs, ds;

	fs = is_null(pxp->size) ? 1 : xstol(pxp->size);
	parse_xcolor(&xc, pxp->color);

	if (compare_text(pxp->style, -1, GDI_ATTR_STROKE_STYLE_DOTTED, -1, 1) == 0)
		ds = PEN_DOT;
	else if (compare_text(pxp->style, -1, GDI_ATTR_STROKE_STYLE_DASHED, -1, 1) == 0)
		ds = PEN_DASH;
	else
		ds = PEN_SOLID;

	if (is_null(pxp->size))
	{
		mo = 0;
		mi = 0;
	}
	else
	{
		mo = (fs - 1) / 2 + (fs - 1) % 2;//outside of baseline  dots
		mi = (fs - 1) / 2; //inside of baseline dots
	}

	if (ppt1->y == ppt2->y)
	{
		if (ds == PEN_SOLID)
		{
			x = (ppt1->x < ppt2->x) ? (ppt1->x - mo) : (ppt2->x - mo);
			y = ppt1->y - mo;
			w = (ppt1->x < ppt2->x) ? (ppt2->x - ppt1->x + (mo << 1)) : (ppt1->x - ppt2->x + (mo << 1));
			h = 1 + mo + mi;
			(*(pgc->device->fillPoints))(pgc->handle, x, y, w, h, &xc, pgc->rop);
		}
		else
		{
			n = ((ppt1->x < ppt2->x) ? (ppt2->x - ppt1->x) : (ppt1->x - ppt2->x));
			n = fs * (n + 2 * fs);
			ppt = (xpoint_t*)xmem_alloc(n * sizeof(xpoint_t));
			n = fit_line(fs, ds, ppt1, ppt2, ppt, n);
			(*(pgc->device->drawPoints))(pgc->handle, ppt, n, &xc, 1, pgc->rop);
			xmem_free(ppt);
		}
	}
	else if (ppt1->x == ppt2->x)
	{
		if (ds == PEN_SOLID)
		{
			x = ppt1->x - mo;
			y = (ppt1->y < ppt2->y) ? (ppt1->y - mo) : (ppt2->y - mo);
			w = 1 + mo + mi;
			h = (ppt1->y < ppt2->y) ? (ppt2->y - ppt1->y + (mo << 1)) : (ppt1->y - ppt2->y + (mo << 1));
			(*(pgc->device->fillPoints))(pgc->handle, x, y, w, h, &xc, pgc->rop);
		}
		else
		{
			n = ((ppt1->y < ppt2->y) ? (ppt2->y - ppt1->y) : (ppt1->y - ppt2->y));
			n = fs * (n + 2 * fs);
			ppt = (xpoint_t*)xmem_alloc(n * sizeof(xpoint_t));
			n = fit_line(fs, ds, ppt1, ppt2, ppt, n);
			(*(pgc->device->drawPoints))(pgc->handle, ppt, n, &xc, 1, pgc->rop);
			xmem_free(ppt);
		}
	}
	else
	{
		n = ((ppt1->x < ppt2->x) ? (ppt2->x - ppt1->x) : (ppt1->x - ppt2->x)) + ((ppt1->y < ppt2->y) ? (ppt2->y - ppt1->y) : (ppt1->y - ppt2->y));
		n = fs * (n + 2 * fs);
		ppt = (xpoint_t*)xmem_alloc(n * sizeof(xpoint_t));
		n = fit_line(fs, ds, ppt1, ppt2, ppt, n);
		(*(pgc->device->drawPoints))(pgc->handle, ppt, n, &xc, 1, pgc->rop);
		xmem_free(ppt);
	}
}

void mgc_draw_rect(visual_t mgc, const xpen_t* pxp, const xbrush_t* pxb, const xrect_t* pxr)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);
	XDK_ASSERT(pxr != NULL);

	if (!pxp && !pxb)
		return;

	xcolor_t xc;
	int mo, mi;
	int x, y, w, h;
	int n, fs, ds;
	xpoint_t* ppt;
	xpoint_t pt[2];

	fs = is_null(pxp->size) ? 1 : xstol(pxp->size);
	parse_xcolor(&xc, pxp->color);

	if (compare_text(pxp->style, -1, GDI_ATTR_STROKE_STYLE_DOTTED, -1, 1) == 0)
		ds = PEN_DOT;
	else if (compare_text(pxp->style, -1, GDI_ATTR_STROKE_STYLE_DASHED, -1, 1) == 0)
		ds = PEN_DASH;
	else
		ds = PEN_SOLID;

	if (is_null(pxp->size))
	{
		mo = 0;
		mi = 0;
	}
	else
	{
		mo = (fs - 1) / 2 + (fs - 1) % 2;//outside of baseline  dots
		mi = (fs - 1) / 2; //inside of baseline dots
	}

	if (ds == PEN_SOLID)
	{
		x = pxr->x - mo;
		y = pxr->y - mo;
		w = pxr->w + (mo << 1) + 1;
		h = 1 + mo + mi;
		(*(pgc->device->fillPoints))(pgc->handle, x, y, w, h, &xc, pgc->rop);

		x = pxr->x + pxr->w - mi;
		y = pxr->y + mi;
		w = 1 + mo + mi;
		h = pxr->h - (mi << 1);
		(*(pgc->device->fillPoints))(pgc->handle, x, y, w, h, &xc, pgc->rop);

		x = pxr->x - mo;
		y = pxr->y + pxr->h - mi;
		w = pxr->w + (mo << 1) + 1;
		h = 1 + mo + mi;
		(*(pgc->device->fillPoints))(pgc->handle, x, y, w, h, &xc, pgc->rop);

		x = pxr->x - mo;
		y = pxr->y + mi;
		w = 1 + mo + mi;
		h = pxr->h - (mi << 1);
		(*(pgc->device->fillPoints))(pgc->handle, x, y, w, h, &xc, pgc->rop);
	}
	else
	{
		n = pxr->w;
		n = fs * (n + 2 * fs);
		ppt = (xpoint_t*)xmem_alloc(n * sizeof(xpoint_t));
		pt[0].x = pxr->x;
		pt[0].y = pxr->y;
		pt[1].x = pxr->x + pxr->w;
		pt[1].y = pxr->y;
		n = fit_line(fs, ds, &pt[0], &pt[1], ppt, n);
		(*(pgc->device->drawPoints))(pgc->handle, ppt, n, &xc, 1, pgc->rop);
		xmem_free(ppt);

		n = pxr->h;
		n = fs * (n + 2 * fs);
		ppt = (xpoint_t*)xmem_alloc(n * sizeof(xpoint_t));
		pt[0].x = pxr->x + pxr->w;
		pt[0].y = pxr->y;
		pt[1].x = pxr->x + pxr->w;
		pt[1].y = pxr->y + pxr->h;
		n = fit_line(fs, ds, &pt[0], &pt[1], ppt, n);
		(*(pgc->device->drawPoints))(pgc->handle, ppt, n, &xc, 1, pgc->rop);
		xmem_free(ppt);

		n = pxr->w;
		n = fs * (n + 2 * fs);
		ppt = (xpoint_t*)xmem_alloc(n * sizeof(xpoint_t));
		pt[0].x = pxr->x + pxr->w + 1;
		pt[0].y = pxr->y + pxr->h;
		pt[1].x = pxr->x + 1;
		pt[1].y = pxr->y + pxr->h;
		n = fit_line(fs, ds, &pt[0], &pt[1], ppt, n);
		(*(pgc->device->drawPoints))(pgc->handle, ppt, n, &xc, 1, pgc->rop);
		xmem_free(ppt);

		n = pxr->h;
		n = fs * (n + 2 * fs);
		ppt = (xpoint_t*)xmem_alloc(n * sizeof(xpoint_t));
		pt[0].x = pxr->x;
		pt[0].y = pxr->y + pxr->h + 1;
		pt[1].x = pxr->x;
		pt[1].y = pxr->y + 1;
		n = fit_line(fs, ds, &pt[0], &pt[1], ppt, n);
		(*(pgc->device->drawPoints))(pgc->handle, ppt, n, &xc, 1, pgc->rop);
		xmem_free(ppt);
	}
}

void mgc_draw_polyline(visual_t mgc, const xpen_t* pxp, const xpoint_t* ppt, int n)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);
	XDK_ASSERT(pxp != NULL);

	int i;

	for (i = 0; i < n - 1; i++)
	{
		mgc_draw_line(mgc, pxp, &ppt[i], &ppt[i + 1]);
	}
}

void mgc_draw_polygon(visual_t mgc, const xpen_t* pxp, const xbrush_t*pxb, const xpoint_t* ppt, int n)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);
	XDK_ASSERT(pxp != NULL);

	int i;

	for (i = 0; i < n - 1; i++)
	{
		mgc_draw_line(mgc, pxp, &ppt[i], &ppt[i + 1]);
	}

	if (n > 1)
	{
		mgc_draw_line(mgc, pxp, &ppt[n-1], &ppt[0]);
	}
}

void mgc_draw_triangle(visual_t mgc, const xpen_t* pxp, const xbrush_t* pxb, const xrect_t* pxr, const tchar_t* orient)
{
	memo_context_t* pgc = (memo_context_t*)mgc;

	XDK_ASSERT(mgc && mgc->tag == _VISUAL_MEMORY);
	XDK_ASSERT(pxr != NULL);

	if (!pxp && !pxb)
		return;


}