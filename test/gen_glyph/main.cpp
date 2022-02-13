
#include <xdk.h>
#include <xdu.h>

static const unsigned char bitmask[8] = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };

if_context_t if_context = {0};

void gen_glyph(bool_t a)
{
	byte_t pch[UTF_LEN + 1] = { 0 };
	tchar_t str[CHS_LEN + 1] = { 0 };
 
	xfont_t xf;
	xface_t xa;
	default_xfont(&xf);
	default_xface(&xa);
	xscpy(xa.text_align, GDI_ATTR_TEXT_ALIGN_CENTER);

	dword_t dw, m;
	tchar_t fname[PATH_LEN];

	const glyph_info_t* glyph_list;
	int glyph_list_len;

	if (a)
	{
		glyph_list = a_glyph_list;
		glyph_list_len = a_alyph_list_length;
		m = ascii_code_count();
	}
	else
	{
		glyph_list = c_glyph_list;
		glyph_list_len = c_alyph_list_length;
		m = gb2312_code_count();
	}

	int i;
	for (i = 0; i < glyph_list_len; i++)
	{
		xsprintf(fname, _T("%s-%s-%s-%s-%s.gly"),
			glyph_list[i].charset,
			glyph_list[i].name,
			glyph_list[i].weight,
			glyph_list[i].style,
			glyph_list[i].size);

		xfont_from_glyph_info(&xf, &glyph_list[i]);

		xhand_t unf = xuncf_open_file(NULL, fname, FILE_OPEN_CREATE);

		tchar_t title[1024] = { 0 };
		byte_t utf_buf[1024] = { 0 };

		int n;
		n = xsprintf(title, _T("%s,%d,%d,%d,%s,%s,%s,%s\n"), glyph_list[i].charset, glyph_list[i].characters, glyph_list[i].width, glyph_list[i].height, glyph_list[i].name, glyph_list[i].weight, glyph_list[i].style, glyph_list[i].size);

#if defined(_UNICODE) || defined(UNICODE)
		dw = ucs_to_utf8(title, n, utf_buf, 1024);
#else
		dw = mbs_to_utf8(title, n, utf_buf, 1024);
#endif
		utf_buf[dw] = '\0';

		xuncf_write_file(unf, utf_buf, &dw);

		printf((char*)utf_buf);
		printf("\n");

		int w, h;
		w = glyph_list[i].width;
		h = glyph_list[i].height;

		visual_t vc_mem, vc = (*if_context.pf_create_display_context)(NULL);

		byte_t* bmp_buf = (byte_t*)xmem_alloc(w / 8 * h);

		xsize_t xs;
		xpoint_t pt;
		int k, j;
		xcolor_t xc;

		if (a)
		{
			pch[0] = (byte_t)glyph_list[i].firstchar;
		}
		else
		{
			pch[0] = GETHBYTE(glyph_list[i].firstchar);
			pch[1] = GETLBYTE(glyph_list[i].firstchar);
		}
		do
		{
#if defined(_UNICODE) || defined(UNICODE)
			n = gb2312_byte_to_ucs(pch, str);
			str[n] = _T('\0');
#else
			n = gb2312_byte_to_utf8(pch, (byte_t*)str);
			str[n] = _T('\0');
#endif

			(*if_context.pf_gdi_text_size)(vc, &xf, str, n, &xs);

			if (a)
				xsprintf(title, _T("0x%02X,%d,%d\n"), pch[0], xs.w, xs.h);
			else
				xsprintf(title, _T("0x%02X%02X,%d,%d\n"), pch[0], pch[1], xs.w, xs.h);

#if defined(_UNICODE) || defined(UNICODE)
			dw = ucs_to_utf8(title, -1, utf_buf, 1024);
#else
			dw = mbs_to_utf8(title, -1, utf_buf, 1024);
#endif

			xuncf_write_file(unf, utf_buf, &dw);

			pt.x = 0;
			pt.y = 0;

			vc_mem = (*if_context.pf_create_compatible_context)(vc, w, h);

			(*if_context.pf_gdi_text_out)(vc_mem, &xf, &pt, str, n);

			xmem_zero(bmp_buf, w / 8 * h);

			k = 0;
			xs.w = (xs.w < w) ? xs.w : w;
			xs.h = (xs.h < h) ? xs.h : h;
			pt.y = (h < xs.h) ? (h - 1) : (xs.h - 1);
			pt.y = 0;
			while (pt.y < xs.h)
			{
				pt.x = 0;
				while (pt.x < xs.w)
				{
					(*if_context.pf_gdi_get_point)(vc_mem, &xc, &pt);

					j = k * (w / 8) + pt.x / 8;
					n = pt.x % 8;

					if (xc.r | xc.g | xc.b)
						bmp_buf[j] |= ~bitmask[n];
					else
						bmp_buf[j] &= bitmask[n];

					pt.x++;
				}
				k++;
				pt.y++;
			}

			(*if_context.pf_destroy_context)(vc_mem);

			dw = 1;
			n = w / 8;
			for (k = 0; k < h; k++)
			{
				xmem_zero(utf_buf, 1024);

				for (j = 0; j < n; j++)
				{
					a_xsappend((schar_t*)utf_buf, "0x%02X", bmp_buf[k * n + j]);

					if (j != n - 1)
						a_xscat((schar_t*)utf_buf, " ");
					else
						a_xscat((schar_t*)utf_buf, "\n");
				}

				dw = a_xslen((schar_t*)utf_buf);
				xuncf_write_file(unf, utf_buf, &dw);
			}
			if (a)
				next_ascii_char(pch);
			else
				next_gb2312_char(pch);
		} while (pch[0] || pch[1]);

		(*if_context.pf_destroy_context)(vc);

		xmem_free(bmp_buf);

		xuncf_close_file(unf);
	}
}

/********************************************************************************/

#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

int main(int argc, const char * argv[]) {

    xdk_process_init(XDK_APARTMENT_PROCESS);

	xdu_impl_context(&if_context);

	xdu_impl_context_graphic(&if_context);

	xdu_impl_context_bitmap(&if_context);

	(*if_context.pf_context_startup)();

	gen_glyph(1);

	gen_glyph(0);

	(*if_context.pf_context_cleanup)();

    xdk_process_uninit();

    return 0;
}
