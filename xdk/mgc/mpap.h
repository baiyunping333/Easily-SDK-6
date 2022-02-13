/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory context paper document

	@module	mpap.h | interface file

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
#ifndef _MEMPAP_H
#define _MEMPAP_H

#include "mdef.h"

#define MGC_PAPER_UN	_T("Unknown Paper")
#define MGC_PAPER_P1	_T("Photo 1*1.5 Inch")
#define MGC_PAPER_PP	_T("Photo Passport")
#define MGC_PAPER_P2	_T("Photo 1.5*2 Inch")
#define MGC_PAPER_P5	_T("Photo 5*3.5 Inch")
#define MGC_PAPER_P6	_T("Photo 6*4 Inch")
#define MGC_PAPER_P7	_T("Photo 7*5 Inch")
#define MGC_PAPER_P8	_T("Photo 6*8 Inch")
#define MGC_PAPER_P10	_T("Photo 8*12 Inch")
#define MGC_PAPER_P12	_T("Photo 10*12 Inch")

#ifdef	__cplusplus
extern "C" {
#endif

EXP_API bool_t select_paper(const tchar_t* formName, dev_prn_t* devPrint);

#ifdef	__cplusplus
}
#endif

#endif /*_MPRN_H*/
