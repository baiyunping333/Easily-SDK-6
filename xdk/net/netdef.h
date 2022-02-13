/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc network defination document

	@module	netdef.h | interface file

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


#ifndef _NETDEF_H
#define	_NETDEF_H


/* default net package size */
#define MTU_MAX_SIZE		1492
#define MTU_MID_SIZE		576
#define MTU_MIN_SIZE		46

typedef enum{
	_SECU_NONE = 0,
	_SECU_SSL = 1,
	_SECU_SSH = 2,
	_SECU_DTLS = 3
}NET_SECU;



#endif	/* _NETDEF_H */

