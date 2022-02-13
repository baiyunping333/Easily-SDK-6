/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc encode defination document

	@module	xdkdef.h | interface file

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


#ifndef _ENCODE_H
#define	_ENCODE_H

/*define gb2312 range*/
#define MIN_CHS_GB2312		0xA1A1
#define MAX_CHS_GB2312		0xFEFF
#define CHS_GB2312_COUNT	8836
#define GB2312_CODE_INDEX(sw)		 ((GETHBYTE(sw) - 161) * 94 + GETLBYTE(sw) - 161)

/*define unicode range*/
#define MIN_CHS_UNICODE		0x4E00
#define MAX_CHS_UNICODE		0x9FA5
#define CHS_UNICODE_COUNT	20902
#define UNICODE_CODE_INDEX(sw)		 (sw - MIN_CHS_UNICODE)

/*define unicode prefix*/
#ifndef GBKBOM
#define GBKBOM		0xFFFF
#endif

#if BYTE_ORDER == BIG_ENDIAN
#ifndef BIGBOM
#define BIGBOM		0xFEFF
#endif
#ifndef LITBOM
#define LITBOM		0xFFFE
#endif
#ifndef UTFBOM
#define UTFBOM		0xEFBBBF
#endif
#ifndef DEFBOM
#define DEFBOM		BIGBOM
#endif
#else
#ifndef BIGBOM
#define BIGBOM		0xFFFE
#endif
#ifndef LITBOM
#define LITBOM		0xFEFF
#endif
#ifndef UTFBOM
#define UTFBOM		0xBFBBEF
#endif
#ifndef DEFBOM
#define DEFBOM		LITBOM
#endif
#endif

#define PUT_ENCODE(buf, off, enc)	(buf[off] = (unsigned char)(enc >> 16), buf[off + 1] = (unsigned char)(enc >> 8), buf[off + 2] = (unsigned char)(enc))
#define GET_ENCODE(buf, off)		(int)((unsigned int)buf[off] << 16 | (unsigned int)buf[off + 1] << 8 | (unsigned int)buf[off + 2])

#define _UNKNOWN        0x0000
#define _GB2312         GBKBOM
#define _UTF8           UTFBOM
#define _UTF16_LIT      LITBOM
#define _UTF16_BIG      BIGBOM
#if BYTE_ORDER == BIG_ENDIAN
#define _UCS2           _UTF16_BIG
#else
#define _UCS2           _UTF16_LIT
#endif


#ifdef _OS_WINDOWS
#define DEF_MBS			_GB2312
#else
#define DEF_MBS			_UTF8
#endif

#if BYTE_ORDER == BIG_ENDIAN
#define DEF_UCS			_UTF16_BIG
#else
#define DEF_UCS			_UTF16_LIT
#endif

#define CHARSET_GB2312		_T("gb2312")
#define CHARSET_UTF8		_T("utf-8")
#define CHARSET_UTF16		_T("utf-16")



#endif	/* _ENCODE_H */

