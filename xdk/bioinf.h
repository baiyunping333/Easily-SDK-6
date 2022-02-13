/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc bio interface document

	@module	bioinf.h | interface file

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

#ifndef _BIOINF_H
#define _BIOINF_H

#include "xdkdef.h"

typedef bool_t(*PF_BIO_READ)(xhand_t, byte_t*, dword_t*);
typedef bool_t(*PF_BIO_WRITE)(xhand_t, const byte_t*, dword_t*);
typedef bool_t(*PF_BIO_FLUSH)(xhand_t);
typedef void(*PF_BIO_CLOSE)(xhand_t);
typedef bool_t(*PF_BIO_SETOPT)(xhand_t, int, void*, int);
typedef bool_t(*PF_BIO_GETOPT)(xhand_t, int, void*, int);
typedef unsigned short(*PF_BIO_ADDR)(xhand_t, tchar_t*);
typedef unsigned short(*PF_BIO_PEER)(xhand_t, tchar_t*);

typedef struct _bio_interface{
	xhand_t		fd;

	PF_BIO_WRITE		pf_write;
	PF_BIO_FLUSH		pf_flush;
	PF_BIO_READ			pf_read;
	PF_BIO_CLOSE		pf_close;
	PF_BIO_SETOPT		pf_setopt;
	PF_BIO_GETOPT		pf_getopt;
	PF_BIO_ADDR			pf_addr;
	PF_BIO_PEER			pf_peer;
}bio_interface;

#ifdef	__cplusplus
extern "C" {
#endif

/*
@FUNCTION get_bio_interface: get bio interface.
@INPUT xhand_t io: the io object.
@RETURN bio_interface*: if succeeds return bio interface struct.
*/
EXP_API bool_t get_bio_interface(xhand_t io, bio_interface* pio);


#ifdef	__cplusplus
}
#endif


#endif /*BIOINF_H*/