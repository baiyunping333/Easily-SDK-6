/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc endian defination document

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


#ifndef _MAKWORD_H
#define	_MAKWORD_H


#define SWAPSWORD(n)			(((sword_t)(n) & 0x00FF) << 8) | ((sword_t)(n) & 0xFF00) >> 8))
#define SWAPDWORD(n)			(((dword_t)(n) & 0x0000FFFF) << 16) | ((dword_t)(n) & 0xFFFF0000) >> 16))

#define LIT_MAKESWORD(lc,hc)	((((sword_t)(hc) << 8) & 0xFF00) | ((sword_t)(lc) & 0x00FF))
#define LIT_GETHBYTE(sw)		(byte_t)(((sword_t)(sw) >> 8) & 0x00FF)
#define LIT_GETLBYTE(sw)		(byte_t)((sword_t)(sw) & 0x00FF)

#define BIG_MAKESWORD(lc,hc)	((((sword_t)(lc) << 8) & 0xFF00) | ((sword_t)(hc) & 0x00FF))
#define BIG_GETHBYTE(sw)		(byte_t)((sword_t)(sw) & 0x00FF)
#define BIG_GETLBYTE(sw)		(byte_t)(((sword_t)(sw) >> 8) & 0x00FF) 

#define LIT_MAKEDWORD(ls,hs)	((((dword_t)(hs) << 16) & 0xFFFF0000) | ((dword_t)(ls) & 0x0000FFFF))
#define LIT_GETHSWORD(dw)		(sword_t)(((dword_t)(dw) >> 16) & 0x0000FFFF)
#define LIT_GETLSWORD(dw)		(sword_t)((dword_t)(dw) & 0x0000FFFF)

#define BIG_MAKEDWORD(ls,hs)	((((dword_t)(ls) << 16) & 0xFFFF0000) | ((dword_t)(hs) & 0x0000FFFF))
#define BIG_GETHSWORD(dw)		(sword_t)((dword_t)(dw) & 0x0000FFFF)
#define BIG_GETLSWORD(dw)		(sword_t)(((dword_t)(dw) >> 16) & 0x0000FFFF)

#define LIT_MAKELWORD(lw,hw)	((((lword_t)(hw) << 32) & 0xFFFFFFFF00000000) | ((lword_t)(lw) & 0x00000000FFFFFFFF))
#define LIT_GETHDWORD(ll)		(dword_t)(((lword_t)(ll) >> 32) & 0x00000000FFFFFFFF)
#define LIT_GETLDWORD(ll)		(dword_t)((lword_t)(ll) & 0x00000000FFFFFFFF)

#define BIG_MAKELWORD(lw,hw)	((((lword_t)(lw) << 32) & 0xFFFFFFFF00000000) | (lword_t)(hw) & 0x00000000FFFFFFFF))
#define BIG_GETHDWORD(ll)		(dword_t)((lword_t)(ll) & 0x00000000FFFFFFFF)
#define BIG_GETLDWORD(ll)		(dword_t)(((lword_t)(ll) >> 32) & 0x00000000FFFFFFFF)

#if BYTE_ORDER == BIG_ENDIAN
#define MAKELWORD			BIG_MAKELWORD
#define GETLDWORD			BIG_GETLDWORD
#define GETHDWORD			BIG_GETHDWORD

#define MAKEDWORD			BIG_MAKEDWORD
#define GETLSWORD			BIG_GETLSWORD
#define GETHSWORD			BIG_GETHSWORD

#define MAKESWORD			BIG_MAKESWORD
#define GETLBYTE			BIG_GETLBYTE
#define GETHBYTE			BIG_GETHBYTE
#else
#define MAKELWORD			LIT_MAKELWORD
#define GETLDWORD			LIT_GETLDWORD
#define GETHDWORD			LIT_GETHDWORD

#define MAKEDWORD			LIT_MAKEDWORD
#define GETLSWORD			LIT_GETLSWORD
#define GETHSWORD			LIT_GETHSWORD

#define MAKESWORD			LIT_MAKESWORD
#define GETLBYTE			LIT_GETLBYTE
#define GETHBYTE			LIT_GETHBYTE
#endif

#define GET_THREEBYTE_BIG(buf,off)		(((unsigned int)((buf)[off]) << 16) | ((unsigned int)((buf)[off+1]) << 8) | (unsigned int)((buf)[off+2]))
#define PUT_THREEBYTE_BIG(buf,off,n)	{(buf)[off] = (unsigned char)((n) >> 16);(buf)[off+1] = (unsigned char)((n) >> 8);(buf)[off+2] = (unsigned char)((n));}

#define GET_THREEBYTE_LIT(buf,off)		(((unsigned int)((buf)[off+2]) << 16) | ((unsigned int)((buf)[off+1]) << 8) | (unsigned int)((buf)[off]))
#define PUT_THREEBYTE_LIT(buf,off,n)	{(buf)[off] = (unsigned char)((n));(buf)[off+1] = (unsigned char)((n) >> 8);(buf)[off+2] = (unsigned char)((n)>>16);}

#define PUT_BYTE(buf,off,n)			((buf)[off] = (unsigned char)((n) & 0xFF))
#define PUT_SWORD_LIT(buf,off,n)	((buf)[off] = (unsigned char) ((n) & 0xFF), (buf)[off+1] = (unsigned char) (((n) >> 8) & 0xFF))
#define PUT_DWORD_LIT(buf,off,n)	((buf)[off] = (unsigned char) ((n) & 0xFF), (buf)[off+1] = (unsigned char) (((n) >> 8) & 0xFF), (buf)[off+2] = (unsigned char) (((n) >> 16) & 0xFF), (buf)[off+3] = (unsigned char) (((n) >> 24) & 0xFF))
#define PUT_LWORD_LIT(buf,off,n)    (PUT_DWORD_LIT(buf,off,LIT_GETLDWORD(n)),PUT_DWORD_LIT(buf,(off+4),LIT_GETHDWORD(n)))
#define PUT_SWORD_BIG(buf,off,n)	((buf)[off] = (unsigned char) (((n) >> 8) & 0xFF), (buf)[off+1] = (unsigned char) ((n) & 0xFF))
#define PUT_DWORD_BIG(buf,off,n)	((buf)[off] = (unsigned char) (((n) >> 24) & 0xFF), (buf)[off+1] = (unsigned char) (((n) >> 16) & 0xFF), (buf)[off+2] = (unsigned char) (((n) >> 8) & 0xFF), (buf)[off+3] = (unsigned char) ((n) & 0xFF))
#define PUT_LWORD_BIG(buf,off,n)    (PUT_DWORD_BIG(buf,off,BIG_GETLDWORD(n)),PUT_DWORD_BIG(buf,(off+4),BIG_GETHDWORD(n)))

#define GET_BYTE(buf,off)			((unsigned char)(((buf)[off]) & 0xFF))
#define GET_SWORD_LIT(buf,off)		((((unsigned short)((buf)[off + 1]) << 8) & 0xFF00) | ((unsigned short)((buf)[off]) & 0x00FF))
#define GET_DWORD_LIT(buf,off)		((((unsigned int)((buf)[off + 3]) << 24) & 0xFF000000) | (((unsigned int)((buf)[off + 2]) << 16) & 0x00FF0000)  | (((unsigned int)((buf)[off + 1]) << 8) & 0x0000FF00) | ((unsigned int)((buf)[off]) & 0x000000FF))
#define GET_LWORD_LIT(buf,off)      LIT_MAKELWORD(GET_DWORD_LIT(buf,off),GET_DWORD_LIT(buf,(off + 4)))
#define GET_SWORD_BIG(buf,off)		((((unsigned short)((buf)[off]) << 8) & 0xFF00) | ((unsigned short)((buf)[off+1]) & 0x00FF))
#define GET_DWORD_BIG(buf,off)		((((unsigned int)((buf)[off]) << 24) & 0xFF000000)  | (((unsigned int)((buf)[off + 1]) << 16) & 0x00FF0000) | (((unsigned int)((buf)[off + 2]) << 8) & 0x0000FF00) | ((unsigned int)((buf)[off + 3]) & 0x000000FF))
#define GET_LWORD_BIG(buf,off)      (BIG_MAKELWORD(GET_DWORD_BIG(buf,off),GET_DWORD_BIG(buf,(off + 4)))

#define GET_SWORD_NET		GET_SWORD_BIG
#define PUT_SWORD_NET		PUT_SWORD_BIG
#define GET_SWORD_NET		GET_SWORD_BIG
#define PUT_DWORD_NET		PUT_DWORD_BIG
#define GET_DWORD_NET		GET_DWORD_BIG
#define PUT_LWORD_NET		PUT_LWORD_BIG
#define GET_LWORD_NET		GET_LWORD_BIG
#define PUT_THREEBYTE_NET	PUT_THREEBYTE_BIG
#define GET_THREEBYTE_NET	GET_THREEBYTE_BIG

#if BYTE_ORDER == BIG_ENDIAN
#define PUT_SWORD_LOC		PUT_SWORD_BIG
#define GET_SWORD_LOC		GET_SWORD_BIG
#define PUT_DWORD_LOC		PUT_DWORD_BIG
#define GET_DWORD_LOC		GET_DWORD_BIG
#define PUT_LWORD_LOC		PUT_LWORD_BIG
#define GET_LWORD_LOC		GET_LWORD_BIG
#define PUT_THREEBYTE_LOC	PUT_THREEBYTE_BIG
#define GET_THREEBYTE_LOC	GET_THREEBYTE_BIG
#else
#define PUT_SWORD_LOC		PUT_SWORD_LIT
#define GET_SWORD_LOC		GET_SWORD_LIT
#define PUT_DWORD_LOC		PUT_DWORD_LIT
#define GET_DWORD_LOC		GET_DWORD_LIT
#define PUT_LWORD_LOC		PUT_LWORD_LIT
#define GET_LWORD_LOC		GET_LWORD_LIT
#define PUT_THREEBYTE_LOC	PUT_THREEBYTE_LIT
#define GET_THREEBYTE_LOC	GET_THREEBYTE_LIT
#endif

#ifdef _OS_64
#define LIT_MAKESIZE(lw,hw)		((((size_t)(hw) << 32) & 0xFFFFFFFF00000000) | ((size_t)(lw) & 0x00000000FFFFFFFF))
#define LIT_GETSIZEH(ll)		(unsigned int)(((size_t)(ll) >> 32) & 0x00000000FFFFFFFF)
#define LIT_GETSIZEL(ll)		(unsigned int)((size_t)(ll) & 0x00000000FFFFFFFF)

#define BIG_MAKESIZE(lw,hw)		((((size_t)(lw) << 32) & 0xFFFFFFFF00000000) | (size_t)(hw) & 0x00000000FFFFFFFF))
#define BIG_GETSIZEH(ll)		(unsigned int)((size_t)(ll) & 0x00000000FFFFFFFF)
#define BIG_GETSIZEL(ll)		(unsigned int)(((size_t)(ll) >> 32) & 0x00000000FFFFFFFF)

#if BYTE_ORDER == BIG_ENDIAN
#define MAKESIZE			BIG_MAKESIZE
#define GETSIZEH			BIG_GETSIZEH
#define GETSIZEL			BIG_GETSIZEL
#else
#define MAKESIZE			LIT_MAKESIZE
#define GETSIZEH			LIT_GETSIZEH
#define GETSIZEL			LIT_GETSIZEL
#endif

#else
#define MAKESIZE(l,h)		((size_t)l)
#define GETSIZEH(ll)		((unsigned int)0)
#define GETSIZEL(ll)		((unsigned int)(ll))
#endif /*_OS_64*/

#ifdef _OS_64
#define VOID_SIZE       8
#define GET_VOID_NET	GET_LWORD_NET
#define GET_VOID_LOC	GET_LWORD_LOC
#define PUT_VOID_NET	PUT_LWORD_NET
#define PUT_VOID_LOC	PUT_LWORD_LOC
#else
#define VOID_SIZE       4
#define GET_VOID_NET	GET_DWORD_NET
#define GET_VOID_LOC	GET_DWORD_LOC
#define PUT_VOID_NET	PUT_DWORD_NET
#define PUT_VOID_LOC	PUT_DWORD_LOC
#endif


#endif	/* _ENDIAN_H */

