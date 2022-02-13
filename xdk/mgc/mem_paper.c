/***********************************************************************
	Easily SDK v6.0

	(c) 2005-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc memory paper for Monochrome image document

	@module	mprn.c | implement file

	@devnote 张文权 2021.01 - 2021.12 v6.0
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

#include "mpap.h"

#include "../xdkimp.h"
#include "../xdkstd.h"
#include "../xdkinit.h"


typedef struct _mem_paper_t{
	tchar_t form_name[MAX_FORM_NAME]; /*the form name*/
	short paper_width; /*the paper width in tenths of a millimeter*/
	short paper_height; /*the paper height in tenths of a millimeter*/

} mem_paper_t;

mem_paper_t mem_paper[] = {
	{ _T("Photo 1*1.5 Inch"), 250, 350 },
	{ _T("Photo Passport"), 330, 480 },
	{ _T("Photo 1.5*2 Inch"), 350, 530 },
	{ _T("Photo 5*3.5 Inch"), 1270, 890 },
	{ _T("Photo 6*4 Inch"), 1520, 1020 },
	{ _T("Photo 7*5 Inch"), 1780, 1270 },
	{ _T("Photo 6*8 Inch"), 1520, 2030 },
	{ _T("Photo 8*12 Inch"), 2030, 3050 },
	{ _T("Photo 10*12 Inch"), 2540, 3050 }
};

bool_t select_paper(const tchar_t* formName, dev_prn_t* devPrint)
{
	int i, n;

	n = sizeof(mem_paper) / sizeof(mem_paper_t);
	for (i = 0; i < n; i++)
	{
		if (xsicmp(formName, mem_paper[i].form_name) == 0)
		{
			devPrint->paper_width = mem_paper[i].paper_width;
			devPrint->paper_height = mem_paper[i].paper_height;

			return bool_true;
		}
	}

	set_last_error(_T("select_paper"), _T("unknown form name"), -1);

	return bool_false;
}


