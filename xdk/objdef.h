/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc object handle defination document

	@module	objdef.h | interface file

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


#ifndef _OBJDEF_H
#define	_OBJDEF_H

typedef struct _handle_head{
	byte_t tag; // object handle type
	byte_t lru[3]; // object reference counter
}handle_head;

#define _HANDLE_UNKNOWN		0x00

/*driver type*/
#define _DRIVER_MONOCHROME	0x01
#define _DRIVER_GRAYSCALE	0x02
#define _DRIVER_COLOR555	0x03
#define _DRIVER_COLOR888	0x04
#define _DRIVER_COLOR8888	0x05
typedef struct _handle_head	 *driver_t;

/*device type*/
#define _DEVICE_BITMAP		0x10
#define _DEVICE_PIXMAP		0x11
typedef struct _handle_head	 *device_t;

/*visual type*/
#define _VISUAL_DISPLAY		0x20
#define _VISUAL_PRINTER		0x21
#define _VISUAL_SCRIPT		0x22
#define _VISUAL_MEMORY		0x23
typedef struct _handle_head	 *visual_t;

/*network type*/
#define _HANDLE_BLOCK		0x30
#define _HANDLE_INET		0x31
#define _HANDLE_CONS		0x32
#define _HANDLE_COMM		0x33
#define _HANDLE_PIPE		0x34
#define _HANDLE_SHARE		0x35
#define _HANDLE_CACHE		0x36
#define _HANDLE_UNCF		0x37
#define _HANDLE_UDP			0x38
#define _HANDLE_TCP			0x39
#define _HANDLE_SSL			0x3A
#define _HANDLE_SSH			0x3B
#define _HANDLE_DTLS		0x3C
#define _HANDLE_TFTP		0x3D
#define _HANDLE_MQTT		0x3E
#define _HANDLE_COAP		0x3F
typedef struct _handle_head *xhand_t;

#define _HANDLE_GLYPH		0xF1
typedef struct _handle_head	*glyph_t;

#define _HANDLE_FONT		0xF2
typedef struct _handle_head	 *font_t;

#define _HANDLE_BITMAP		0xF3
typedef struct _handle_head	 *bitmap_t;

#define _HANDLE_STREAM		0xF4
typedef struct _handle_head *stream_t;



typedef struct _memobj_head{
	byte_t tag; //memo object type
	byte_t len[3]; //memo object size in bytes
}memobj_head;

#define MEMOBJ_SIZE(obj)		(GET_THREEBYTE_LOC(((obj)->len), 0) + sizeof(memobj_head))

#define MEM_BINARY	0x00
#define MEM_VARIANT	0x01
#define MEM_STRING	0x02
#define MEM_MAP		0x03
#define MEM_VECTOR	0x04
#define MEM_MATRIX	0x04 //equal to vector
#define MEM_SET		0x05
#define MEM_DOMDOC	0x0A
#define MEM_MESSAGE	0x0B
#define MEM_QUEUE	0x0C
#define MEM_LINEAR	0x0D
#define MEM_SPINLOCK	0x0E

#define MEMENC_MASK	0x10

#define IS_OBJECT_TYPE(tag)		((tag >= 0x10 && tag <= 0x1F)? 1 : 0)

typedef struct _memobj_head **object_t;
typedef struct _memobj_head **message_t;
typedef struct _memobj_head **queue_t;
typedef struct _memobj_head *variant_t;
typedef struct _memobj_head *string_t;
typedef struct _memobj_head *map_t;
typedef struct _memobj_head *vector_t;
typedef struct _memobj_head *matrix_t;
typedef struct _memobj_head *linear_t;
typedef struct _memobj_head *spinlock_t;


#endif	/* _OBJDEF_H */

