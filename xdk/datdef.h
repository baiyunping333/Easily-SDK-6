/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc data type defination document

	@module	datdef.h | interface file

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


#ifndef _DATDEF_H
#define	_DATDEF_H


#if defined(_UNICODE) || defined(UNICODE)
typedef wchar_t			tchar_t;
#else
typedef char			tchar_t;
#endif

#ifndef schar_t
typedef char			schar_t;
#endif

#ifndef byte_t
typedef unsigned char	byte_t;
#endif

#ifndef bool_t
typedef unsigned int	bool_t;

#define bool_true		((bool_t)1)
#define bool_false		((bool_t)0)
#endif

#ifndef sword_t
typedef unsigned short	sword_t;
#endif

#ifndef dword_t
typedef unsigned int	dword_t;
#endif

#ifndef lword_t
typedef unsigned long long lword_t;
#endif

#ifdef _OS_64
typedef unsigned long long	vword_t;
#else
typedef unsigned int		vword_t;
#endif

#ifndef stamp_t
typedef long long		stamp_t;
#endif

#ifndef wait_t
typedef int				wait_t;
#endif



#endif	/* _DATDEF_H */

