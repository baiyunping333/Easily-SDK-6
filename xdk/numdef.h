/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc number defination document

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


#ifndef _NUMDEF_H
#define	_NUMDEF_H

#define MAXDBL      1.7976931348623158e+308
#define MINDBL      2.2250738585072014e-308
#define MAXFLT      3.402823466e+38F
#define MINFLT      1.175494351e-38F

#define XPI			3.1415926535

#define IS_ZERO_FLOAT(f)	((-MINFLT <= f && f <= MINFLT)? 1 : 0)
#define IS_ZERO_DOUBLE(d)	((-MINDBL <= d && d <= MINDBL)? 1 : 0)
#define IS_VALID_FLOAT(f)	((-MAXFLT < f && f < MAXFLT)? 1 : 0)
#define IS_VALID_DOUBLE(d)	((-MAXDBL < d && d < MAXDBL)? 1 : 0)

/*define max integer value*/
#define MAX_LONG        2147483647		//0x7fffffff
#define MIN_LONG		-2147483648		//0x80000000
#define MAX_SHORT       32767			//0x7fff
#define MIN_SHORT		-32768			//0x8000
#define MAX_CHAR		127				//0x7f
#define MIN_CHAR		-128			//0x80
#define ALT_CHAR		0x20

#define MAX_DWORD		4294967295		//0xffffffff
#define MAX_WORD		65535			//0xffff
#define MAX_BYTE		255				//0xff

#define MIN_YEAR		1901
#define MAX_YEAR		2038

/*define max numeric precision*/
#define MAX_DOUBLE_PREC	18
#define MAX_DOUBLE_DIGI	6
#define DEF_DOUBLE_PREC 8
#define MAX_FLOAT_PREC	12
#define MAX_FLOAT_DIGI	4
#define DEF_FLOAT_PREC	6


#endif	/* _NUMDEF_H */

