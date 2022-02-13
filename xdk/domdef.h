/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc dom defination document

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


#ifndef _DOMDEF_H
#define	_DOMDEF_H

/*define root link tag*/
#define lkRoot			0xFF
/*define free link tag*/
#define lkFree			0x00

#define lkDebug			0xFE

#define lkDoc			0x01
#define lkNode			0x02

#define lkHashTable		0x03
#define lkHashEntity	0x04

#define lkListTable		0x05
#define lkListNode		0x06

#define lkStringTable	0x07
#define lkStringEntity	0x08

#define lkDictTable		0x09
#define lkDictEntity	0x0A

#define lkWordsTable	0x0B
#define lkWordsItem		0x0C

#define lkTrieNode		0x0D
#define lkTrieLeaf		0x0E

#define lkStackTable	0x0F

#define lkBinaTree		0x10
#define lkBinaNode		0x11

#define lkBplusTree		0x12
#define lkBplusIndex	0x13
#define lkBplusData		0x14

#define lkACTable		0x15

#define lkMultiTree		0x16

#define lkFileTable		0x17
#define lkLockTable		0x18

#define IS_DOM_DOC(ptr)		((ptr->tag == lkNode)? 1 : 0)
#define IS_XML_DOC(ptr)		((ptr->tag == lkDoc)? 1 : 0)


//define dom node mask
#define MSK_CDATA		0x00001000 //is cdata node
#define MSK_ARRAY		0x00002000 //is array node
#define MSK_NUMBER		0x00004000 //is number node
#define MSK_NULL		0x00008000 //is null node
#define MSK_LOCKED		0x00010000 //node locked
#define MSK_CHECKED		0x00020000 //node checked
#define MSK_HIDDEN		0x00040000 //node not visible
#define MSK_COLLAPSED	0x00080000 //node collapsed tag
#define MSK_SKIPPED		0x00100000 //node will be skiped
#define MSK_DELETED		0x00200000 //node will be deleted
#define MSK_SORTED		0x00400000 //node has been sorted
#define MSK_SPLITED		0x00800000 //node has been splited
#define MSK_FIXED		0x01000000 //node has been fixed
#define MSK_PASSWORD	0x02000000 //node has been fixed
#define MSK_DESIGN		0x80000000 //document in design mode



#endif	/* _DOMDEF_H */

