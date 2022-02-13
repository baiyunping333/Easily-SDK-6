
/*
* Copyright (c) 1999, 2005 Greg Haerr <greg@censoft.com>
*
* 1bpp (2 color) standard palette definition
*/

#define MWINCLUDECOLORS

#include "mwdevice.h"

/*
* Standard palette for Everex Freestyle Palm PC
* This palette is in reverse order from some 2bpp systems.
* That is, white is pixel value 0, while black is 3.
*/
const MWPALENTRY mwstdpal2[4] = {
#ifdef INVERT2BPP
	RGBDEF(255, 255, 255),	/* white*/
	RGBDEF(192, 192, 192),	/* ltgray*/
	RGBDEF(128, 128, 128),	/* gray*/
	RGBDEF(0, 0, 0)		/* black*/
#else
	RGBDEF(0, 0, 0),	/* black*/
	RGBDEF(128, 128, 128),	/* gray*/
	RGBDEF(192, 192, 192),	/* ltgray*/
	RGBDEF(255, 255, 255)		/* white*/
#endif
};

