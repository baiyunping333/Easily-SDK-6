/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc system mbcs call document

	@module	_if_mbcs.c | linux implement file

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

#include "../xdkloc.h"

#ifdef XDK_SUPPORT_MBCS

int _gbk_to_ucs(const schar_t* gbk, int len, wchar_t* ucs, int max)
{
    int n, total = 0;

    if(len < 0)
        len = (int)strlen(gbk);
    
    setlocale(P_ALL, "zh_CN.GBK");
    
    while(len && total < max)
    {
        n = mblen(gbk, len);
        if(ucs)
        {
            mbtowc(ucs + total, gbk, n);
        }
	    total ++;
        len -= n;
        gbk += n;
    }
    
    setlocale(P_ALL, "");
    
    return total;
}

int _ucs_to_gbk(const wchar_t* ucs, int len, schar_t* gbk, int max)
{
    int n, total = 0;
    char chs[4];

	if(len < 0)
        len = (int)wcslen(ucs);
    
    setlocale(P_ALL, "zh_CN.GBK");
    
   while(len && total < max)
    {
        if(gbk)
            n = wctomb(gbk + total, *ucs);
        else
            n = wctomb(chs, *ucs);

        total += n;
        len --;
        ucs ++;
    }
    
    setlocale(P_ALL, "");

    return total;
}

int _utf_to_ucs(const schar_t* utf, int len, wchar_t* ucs, int max)
{
    int n, total = 0;

    if(len < 0)
        len = (int)strlen(utf);
    
    setlocale(P_ALL, "zh_CN.UTF-8");
    
    while(len && total < max)
    {
        n = mblen(utf, len);
        if(ucs)
        {
            mbtowc(ucs + total, utf, n);
        }
	    total ++;
        len -= n;
        utf += n;
    }
    
    setlocale(P_ALL, "");
    
    return total;
}

int _ucs_to_utf(const wchar_t* ucs, int len, schar_t* utf, int max)
{
    int n, total = 0;
    char chs[4];

    if(len < 0)
        len = (int)wcslen(ucs);
    
    setlocale(P_ALL, "zh_CN.UTF-8");
    
    while(len && total < max)
    {
        if(utf)
            n = wctomb(utf + total, *ucs);
        else
            n = wctomb(chs, *ucs);

        total += n;
        len --;
        ucs ++;
    }
   
    setlocale(P_ALL, "");
    
    return total;
}

#endif
