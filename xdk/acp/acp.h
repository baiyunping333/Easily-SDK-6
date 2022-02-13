/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc codepage document

	@module	acp.h | interface file

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

#ifndef _ACP_H
#define _ACP_H

#include "../xdkdef.h"

#define LIT_MAKESHORT(lc,hc)	((((unsigned short)(hc) << 8) & 0xFF00) | ((unsigned short)(lc) & 0x00FF))
#define LIT_GETHCHAR(sw)		(unsigned char)(((unsigned short)(sw) >> 8) & 0x00FF)
#define LIT_GETLCHAR(sw)		(unsigned char)((unsigned short)(sw) & 0x00FF)

#define BIG_MAKESHORT(lc,hc)	((((unsigned short)(lc) << 8) & 0xFF00) | ((unsigned short)(hc) & 0x00FF))
#define BIG_GETHCHAR(sw)		(unsigned char)((unsigned short)(sw) & 0x00FF)
#define BIG_GETLCHAR(sw)		(unsigned char)(((unsigned short)(sw) >> 8) & 0x00FF) 

#if defined(_WIN32) || defined(_WIN64) || defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
   defined(vax) || defined(ns32000) || defined(sun386) || \
   defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
   defined(__alpha__) || defined(__alpha)
#define ACP_BYTE_ORDER    1234
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined (BIT_ZERO_ON_LEFT) || defined(m68k) || defined(__sparc)
#define ACP_BYTE_ORDER	4321
#endif

#if ACP_BYTE_ORDER == 4321
#define MAKESHORT			BIG_MAKESHORT
#define GETLCHAR			BIG_GETLCHAR
#define GETHCHAR			BIG_GETHCHAR
#else
#define MAKESHORT			LIT_MAKESHORT
#define GETLCHAR			LIT_GETLCHAR
#define GETHCHAR			LIT_GETHCHAR
#endif

#define ALT_CHAR	0x20

#if ACP_BYTE_ORDER == 4321
#define BIGBOM		0xFEFF
#define LITBOM		0xFFFE
#define DEFBOM		BIGBOM
#else
#define BIGBOM		0xFFFE
#define LITBOM		0xFEFF
#define DEFBOM		LITBOM
#endif

/*code range*/
#define _ACP_GBKMIN			0xa1a0
#define _ACP_GBKMAX			0xfeff
#define _ACP_UCSMIN			0x00a0
#define _ACP_UCSMAX			0xffef

typedef struct acp_t{
	sword_t code;
	sword_t help;
	vword_t addr;
}acp_t;

#define SHARE_GB2312_CODEPAGE		_T("share_gb2312_codepage")
#define SHARE_UNICODE_CODEPAGE		_T("share_unicode_codepage")

extern xhand_t acp_gb2312;
extern xhand_t acp_unicode;

#ifdef XDK_SUPPORT_ACP_TABLE
LOC_API int table_unicode_seek_help(unsigned short ucs, unsigned short* hlp);
LOC_API int table_gb2312_seek_help(const unsigned char* mbs, unsigned char* hlp);
LOC_API int table_unicode_seek_gb2312(unsigned short ucs, unsigned char* mbs);
LOC_API int table_gb2312_seek_unicode(unsigned char* mbs, unsigned short* ucs);
#else
LOC_API int share_unicode_seek_help(unsigned short ucs, unsigned short* hlp);
LOC_API int share_gb2312_seek_help(const unsigned char* mbs, unsigned char* hlp);
LOC_API int share_unicode_seek_gb2312(unsigned short ucs, unsigned char* mbs);
LOC_API int share_gb2312_seek_unicode(unsigned char* mbs, unsigned short* ucs);
#endif

#ifdef __cplusplus
extern "C" {
#endif

	EXP_API int gb2312_code_sequence(byte_t b);

	EXP_API int gb2312_byte_to_unicode(const byte_t* src, wchar_t* dest);

	EXP_API int gb2312_to_unicode(const byte_t* src, dword_t slen, wchar_t* dest, int dlen);

	EXP_API int unicode_byte_to_gb2312(wchar_t ch, byte_t* dest);

	EXP_API int unicode_to_gb2312(const wchar_t* src, int slen, byte_t* dest, dword_t dlen);

	EXP_API int utf8_code_sequence(unsigned char b);

	EXP_API int utf8_byte_to_unicode(const byte_t* src, wchar_t* dest);

	EXP_API int utf8_to_unicode(const byte_t* src, dword_t slen, wchar_t* dest, int dlen);

	EXP_API int unicode_byte_to_utf8(wchar_t ch, byte_t* dest);

	EXP_API int unicode_to_utf8(const wchar_t* src, int slen, byte_t* dest, dword_t dlen);

	EXP_API int ascii_code_count(void);

	EXP_API int gb2312_code_count(void);

	EXP_API bool_t next_ascii_char(byte_t* pch);

	EXP_API bool_t next_gb2312_char(byte_t* pch);

	EXP_API vword_t get_gb2312_code_addr(const byte_t* pch);

	EXP_API bool_t set_gb2312_code_addr(const byte_t* pch, vword_t addr);

	EXP_API vword_t get_unicode_code_addr(unsigned short ucs);

	EXP_API bool_t set_unicode_code_addr(unsigned short ucs, vword_t addr);

	EXP_API bool_t acp_init(void);

	EXP_API void acp_uninit(void);

#ifdef XDK_SUPPORT_ACP_TABLE

	EXP_API void unicode_gb2312_code(int index, unsigned short* code, unsigned short* val, unsigned short* key);

	EXP_API void gb2312_unicode_code(int index, unsigned short* code, unsigned short* val, unsigned short* key);

	EXP_API bool_t save_gb2312_table(const tchar_t* fname);

	EXP_API bool_t save_unicode_table(const tchar_t* fname);
#endif

	EXP_API int w_help_code(const wchar_t* src, int len, wchar_t* buf, int max);

	EXP_API int a_help_code(const schar_t* src, int len, schar_t* buf, int max);

#ifdef _UNICODE
#define help_code			w_help_code
#else
#define help_code			a_help_code
#endif

#ifdef __cplusplus
}
#endif

#endif /*_ACP_H*/
