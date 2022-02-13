#ifndef _MWDEVICE_H
#define _MWDEVICE_H
/*
* Copyright (c) 1999, 2000, 2001, 2002, 2003, 2005, 2007, 2010 Greg Haerr <greg@censoft.com>
* Portions Copyright (c) 2002 Koninklijke Philips Electronics
*
* Engine-level Screen, Mouse and Keyboard device driver API's and types
*
* Contents of this file are not for general export
*/

#include "mwconfig.h"
#include "mwtypes.h"


/* screen subdriver entry points: one required for each draw function*/
/* NOTE: currently used for fb driver only*/
typedef struct _mwsubdriver{
	int(*Init)(PSD psd);
	void(*DrawPixel)(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c);
	MWPIXELVAL(*ReadPixel)(PSD psd, MWCOORD x, MWCOORD y);
	void(*DrawHorzLine)(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y,
		MWPIXELVAL c);
	void(*DrawVertLine)(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2,
		MWPIXELVAL c);
	void(*FillRect)(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD x2,
		MWCOORD y2, MWPIXELVAL c);
	void(*Blit)(PSD destpsd, MWCOORD destx, MWCOORD desty, MWCOORD w,
		MWCOORD h, PSD srcpsd, MWCOORD srcx, MWCOORD srcy, int op);
	void(*DrawArea)(PSD psd, driver_gc_t *gc);
	/* Note: StretchBlit() is deprecated, use StretchBlitEx()
	void	 (*StretchBlit)(PSD destpsd, MWCOORD destx, MWCOORD desty,
	MWCOORD dstw, MWCOORD dsth, PSD srcpsd, MWCOORD srcx,
	MWCOORD srcy, MWCOORD srcw, MWCOORD srch, int op);
	*/
	void(*StretchBlitEx) (PSD dstpsd, PSD srcpsd,
		MWCOORD dest_x_start, int dest_y_start,
		MWCOORD width, int height,
		int x_denominator, int y_denominator,
		int src_x_fraction, int src_y_fraction,
		int x_step_fraction, int y_step_fraction, int op);
} SUBDRIVER, *PSUBDRIVER;

/*
* Interface to Screen Device Driver
* This structure is also allocated for memory (offscreen) drawing and blitting.
*/
typedef struct _mwscreendevice {
	MWCOORD	xres;		/* X screen res (real) */
	MWCOORD	yres;		/* Y screen res (real) */
	MWCOORD	xvirtres;	/* X drawing res (will be flipped in portrait mode) */
	MWCOORD	yvirtres;	/* Y drawing res (will be flipped in portrait mode) */
	int	planes;			/* # planes*/
	int	bpp;			/* # bpp*/
	int	linelen;		/* line length in bytes for bpp 1,2,4,8*/
	/* line length in pixels for bpp 16, 18, 24, 32*/
	int	size;			/* size of memory allocated*/
	int32_t	ncolors;	/* # screen colors*/
	int	pixtype;		/* format of pixel value*/
	int	flags;			/* device flags*/
	void *	addr;		/* address of memory allocated (memdc or fb)*/

	PSD(*Open)(PSD psd);
	void(*Close)(PSD psd);
	void(*GetScreenInfo)(PSD psd, PMWSCREENINFO psi);
	void(*SetPalette)(PSD psd, int first, int count, MWPALENTRY *pal);
	void(*DrawPixel)(PSD psd, MWCOORD x, MWCOORD y, MWPIXELVAL c);
	MWPIXELVAL(*ReadPixel)(PSD psd, MWCOORD x, MWCOORD y);
	void(*DrawHorzLine)(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y,
		MWPIXELVAL c);
	void(*DrawVertLine)(PSD psd, MWCOORD x, MWCOORD y1, MWCOORD y2,
		MWPIXELVAL c);
	void(*FillRect)(PSD psd, MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2,
		MWPIXELVAL c);
	PMWCOREFONT builtin_fonts;

	/* *void (*DrawText)(PSD psd,MWCOORD x,MWCOORD y,const MWUCHAR *str,
	int count, MWPIXELVAL fg, PMWFONT pfont);***/

	void(*Blit)(PSD destpsd, MWCOORD destx, MWCOORD desty, MWCOORD w,
		MWCOORD h, PSD srcpsd, MWCOORD srcx, MWCOORD srcy, int op);
	void(*PreSelect)(PSD psd);
	void(*DrawArea)(PSD psd, driver_gc_t *gc);
	int(*SetIOPermissions)(PSD psd);
	PSD(*AllocateMemGC)(PSD psd);
	MWBOOL(*MapMemGC)(PSD mempsd, MWCOORD w, MWCOORD h, int planes, int bpp,
		int linelen, int size, void *addr);
	void(*FreeMemGC)(PSD mempsd);
	/* Note: StretchBlit() is deprecated, use StretchBlitEx()
	void	(*StretchBlit)(PSD destpsd,MWCOORD destx,MWCOORD desty,
	MWCOORD destw,MWCOORD desth,PSD srcpsd,MWCOORD srcx,
	MWCOORD srcy,MWCOORD srcw,MWCOORD srch,int op);
	*/
	void(*SetPortrait)(PSD psd, int portraitmode);
	int	portrait;	 /* screen portrait mode*/
	PSUBDRIVER orgsubdriver; /* original subdriver for portrait modes*/
	void(*StretchBlitEx) (PSD dstpsd, PSD srcpsd,
		MWCOORD dest_x_start, MWCOORD dest_y_start,
		MWCOORD width, MWCOORD height,
		int x_denominator, int y_denominator,
		int src_x_fraction, int src_y_fraction,
		int x_step_fraction, int y_step_fraction,
		int op);
} SCREENDEVICE;

/* PSD flags*/
#define	PSF_SCREEN		0x0001	/* screen device*/
#define PSF_MEMORY		0x0002	/* memory device*/
#define PSF_HAVEBLIT		0x0004	/* have bitblit*/
#define PSF_HAVEOP_COPY		0x0008	/* psd->DrawArea can do area copy*/
#define PSF_ADDRMALLOC		0x0010	/* psd->addr was malloc'd*/
#define PSF_ADDRSHAREDMEM	0x0020	/* psd->addr is shared memory*/
#define PSF_HAVEOP_ALPHACOL	0x0040	/* psd->Drawarea can do alphacol*/

extern SCREENDEVICE scrdev;

#ifdef __cplusplus
extern "C" {
#endif

/* devdraw.c*/
PSD	GdOpenScreen(void);
void	GdCloseScreen(PSD psd);
int	GdSetPortraitMode(PSD psd, int portraitmode);
int	GdSetMode(int mode);
MWBOOL	GdSetUseBackground(MWBOOL flag);
MWPIXELVAL GdSetForegroundPixelVal(PSD psd, MWPIXELVAL fg);
MWPIXELVAL GdSetBackgroundPixelVal(PSD psd, MWPIXELVAL bg);
MWPIXELVAL GdSetForegroundColor(PSD psd, MWCOLORVAL fg);
MWPIXELVAL GdSetBackgroundColor(PSD psd, MWCOLORVAL bg);

void	GdResetPalette(void);
void	GdSetPalette(PSD psd,int first, int count, MWPALENTRY *palette);
int	GdGetPalette(PSD psd,int first, int count, MWPALENTRY *palette);
MWCOLORVAL GdGetColorRGB(PSD psd, MWPIXELVAL pixel);
MWPIXELVAL GdFindColor(PSD psd, MWCOLORVAL c);
MWPIXELVAL GdFindNearestColor(MWPALENTRY *pal, int size, MWCOLORVAL cr);
int	GdCaptureScreen(char *path);
void	GdGetScreenInfo(PSD psd,PMWSCREENINFO psi);
void	GdPoint(PSD psd,MWCOORD x, MWCOORD y);
void	GdLine(PSD psd,MWCOORD x1,MWCOORD y1,MWCOORD x2,MWCOORD y2,
		MWBOOL bDrawLastPoint);
void	GdRect(PSD psd,MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height);
void	GdFillRect(PSD psd,MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height);
void	GdBitmap(PSD psd,MWCOORD x,MWCOORD y,MWCOORD width,MWCOORD height,
		const MWIMAGEBITS *imagebits);
MWBOOL	GdColorInPalette(MWCOLORVAL cr,MWPALENTRY *palette,int palsize);
void	GdMakePaletteConversionTable(PSD psd,MWPALENTRY *palette,int palsize,
		MWPIXELVAL *convtable,int fLoadType);
void	GdDrawImage(PSD psd,MWCOORD x, MWCOORD y, PMWIMAGEHDR pimage);
void	GdPoly(PSD psd,int count, MWPOINT *points);
void	GdFillPoly(PSD psd,int count, MWPOINT *points);
void	GdReadArea(PSD psd,MWCOORD x,MWCOORD y,MWCOORD width,MWCOORD height,
		MWPIXELVAL *pixels);
void	GdArea(PSD psd,MWCOORD x,MWCOORD y,MWCOORD width,MWCOORD height,
		void *pixels, int pixtype);
void	GdConversionBlit(PSD psd, PMWBLITPARMS parms);
void	GdDrawAreaInternal(PSD psd, driver_gc_t *gc);
void	GdTranslateArea(MWCOORD width, MWCOORD height, void *in, int inpixtype,
		MWCOORD inpitch, void *out, int outpixtype, int outpitch);
void	GdCopyArea(PSD psd,MWCOORD srcx,MWCOORD srcy,MWCOORD width,
		MWCOORD height, MWCOORD destx, MWCOORD desty);
void	GdBlit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD width,
		MWCOORD height,PSD srcpsd,MWCOORD srcx,MWCOORD srcy,int rop);
/***void	GdStretchBlit(PSD dstpsd, MWCOORD dstx, MWCOORD dsty, MWCOORD dstw,
		MWCOORD dsth, PSD srcpsd, MWCOORD srcx, MWCOORD srcy,
		MWCOORD srcw, MWCOORD srch, int rop);***/
void	GdStretchBlitEx(PSD dstpsd, MWCOORD d1_x, MWCOORD d1_y, MWCOORD d2_x,
		MWCOORD d2_y, PSD srcpsd, MWCOORD s1_x, MWCOORD s1_y,
		MWCOORD s2_x, MWCOORD s2_y, int rop);
int	GdCalcMemGCAlloc(PSD psd, unsigned int width, unsigned int height,
		int planes, int bpp, int *size, int *linelen);
void	drawbitmap(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
		const MWIMAGEBITS *imagebits);
void	drawpoint(PSD psd, MWCOORD x, MWCOORD y);
void	drawrow(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y);

/* devarc.c*/
/* requires float*/
void	GdArcAngle(PSD psd, MWCOORD x0, MWCOORD y0, MWCOORD rx, MWCOORD ry,
		MWCOORD angle1, MWCOORD angle2, int type);
/* integer only*/
void	GdArc(PSD psd, MWCOORD x0, MWCOORD y0, MWCOORD rx, MWCOORD ry,
		MWCOORD ax, MWCOORD ay, MWCOORD bx, MWCOORD by, int type);
void	GdEllipse(PSD psd,MWCOORD x, MWCOORD y, MWCOORD rx, MWCOORD ry,
		MWBOOL fill);

/* devfont.c*/
void	GdClearFontList(void);
int	GdAddFont(char *fndry, char *family, char *fontname, PMWLOGFONT lf,
		  unsigned int flags);
PMWFONT	GdSetFont(PMWFONT pfont);
PMWFONT GdCreateFont(PSD psd, const char *name, MWCOORD height, MWCOORD width,
		const PMWLOGFONT plogfont);
MWCOORD	GdSetFontSize(PMWFONT pfont, MWCOORD height, MWCOORD width);
void GdGetFontList(MWFONTLIST ***list, int *num);
void GdFreeFontList(MWFONTLIST ***list, int num);
int	GdSetFontRotation(PMWFONT pfont, int tenthdegrees);
int	GdSetFontAttr(PMWFONT pfont, int setflags, int clrflags);
void	GdDestroyFont(PMWFONT pfont);
MWBOOL	GdGetFontInfo(PMWFONT pfont, PMWFONTINFO pfontinfo);
int	GdConvertEncoding(const void *istr, MWTEXTFLAGS iflags, int cc, void *ostr,
		MWTEXTFLAGS oflags);
void	GdGetTextSize(PMWFONT pfont, const void *str, int cc, MWCOORD *pwidth,
		MWCOORD *pheight, MWCOORD *pbase, MWTEXTFLAGS flags);
int	GdGetTextSizeEx(PMWFONT pfont, const void *str, int cc,
		int nMaxExtent, int *lpnFit, int *alpDx, MWCOORD *pwidth,
		MWCOORD *pheight, MWCOORD *pbase, MWTEXTFLAGS flags);	
void	GdText(PSD psd,MWCOORD x,MWCOORD y,const void *str,int count,
		MWTEXTFLAGS flags);
PMWFONT	GdCreateFontFromBuffer(PSD psd, const unsigned char *buffer,
		unsigned length, const char *format, MWCOORD height, MWCOORD width);
PMWFONT	GdDuplicateFont(PSD psd, PMWFONT psrcfont, MWCOORD height, MWCOORD width);


/* both devclip1.c and devclip2.c */
MWBOOL	GdClipPoint(PSD psd,MWCOORD x,MWCOORD y);
int	GdClipArea(PSD psd,MWCOORD x1, MWCOORD y1, MWCOORD x2, MWCOORD y2);
extern MWCOORD clipminx, clipminy, clipmaxx, clipmaxy;

/* devclip1.c only*/
void 	GdSetClipRects(PSD psd,int count,MWCLIPRECT *table);

/* devclip2.c only*/
void	GdSetClipRegion(PSD psd, MWCLIPREGION *reg);

/* devrgn.c - multi-rectangle region entry points*/
MWBOOL GdPtInRegion(MWCLIPREGION *rgn, MWCOORD x, MWCOORD y);
int    GdRectInRegion(MWCLIPREGION *rgn, const MWRECT *rect);
MWBOOL GdEqualRegion(MWCLIPREGION *r1, MWCLIPREGION *r2);
MWBOOL GdEmptyRegion(MWCLIPREGION *rgn);
MWCLIPREGION *GdAllocRegion(void);
MWCLIPREGION *GdAllocRectRegion(MWCOORD left,MWCOORD top,MWCOORD right,MWCOORD bottom);
MWCLIPREGION *GdAllocRectRegionIndirect(MWRECT *prc);
void GdSetRectRegion(MWCLIPREGION *rgn, MWCOORD left, MWCOORD top,
		MWCOORD right, MWCOORD bottom);
void GdSetRectRegionIndirect(MWCLIPREGION *rgn, MWRECT *prc);
void GdDestroyRegion(MWCLIPREGION *rgn);
void GdOffsetRegion(MWCLIPREGION *rgn, MWCOORD x, MWCOORD y);
int  GdGetRegionBox(MWCLIPREGION *rgn, MWRECT *prc);
void GdUnionRectWithRegion(const MWRECT *rect, MWCLIPREGION *rgn);
void GdSubtractRectFromRegion(const MWRECT *rect, MWCLIPREGION *rgn);
void GdCopyRegion(MWCLIPREGION *d, MWCLIPREGION *s);
void GdIntersectRegion(MWCLIPREGION *d, MWCLIPREGION *s1, MWCLIPREGION *s2);
void GdUnionRegion(MWCLIPREGION *d, MWCLIPREGION *s1, MWCLIPREGION *s2);
void GdSubtractRegion(MWCLIPREGION *d, MWCLIPREGION *s1, MWCLIPREGION *s2);
void GdXorRegion(MWCLIPREGION *d, MWCLIPREGION *s1, MWCLIPREGION *s2);
MWCLIPREGION *GdAllocBitmapRegion(MWIMAGEBITS *bitmap, MWCOORD width, MWCOORD height);

/* devrgn2.c*/
MWCLIPREGION *GdAllocPolygonRegion(MWPOINT *points, int count, int mode);
MWCLIPREGION *GdAllocPolyPolygonRegion(MWPOINT *points, int *count,
		int nbpolygons, int mode);

/* devstipple.c */
void	GdSetDash(uint32_t *mask, int *count);
void	GdSetStippleBitmap(MWIMAGEBITS *stipple, MWCOORD width, MWCOORD height);
void	GdSetTSOffset(int xoff, int yoff);
int	GdSetFillMode(int mode);
void	GdSetTilePixmap(PSD src, MWCOORD width, MWCOORD height);
void	ts_drawpoint(PSD psd, MWCOORD x, MWCOORD y);
void	ts_drawrow(PSD psd, MWCOORD x1, MWCOORD x2, MWCOORD y);
void	ts_fillrect(PSD psd, MWCOORD x, MWCOORD y, MWCOORD w, MWCOORD h);
void	set_ts_origin(int x, int y);

/* devimage.c */
#if MW_FEATURE_IMAGES
int	GdLoadImageFromBuffer(PSD psd, void *buffer, int size, int flags);
void	GdDrawImageFromBuffer(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
	MWCOORD height, void *buffer, int size, int flags);
void	GdDrawImageFromFile(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
	MWCOORD height, char *path, int flags);
int	GdLoadImageFromFile(PSD psd, char *path, int flags);
void	GdDrawImageToFit(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width,
	MWCOORD height, int id);
void	GdFreeImage(int id);
MWBOOL	GdGetImageInfo(int id, PMWIMAGEINFO pii);
void	GdStretchImage(PMWIMAGEHDR src, MWCLIPRECT *srcrect, PMWIMAGEHDR dst,
	MWCLIPRECT *dstrect);
void	GdDrawImagePartToFit(PSD psd, MWCOORD x, MWCOORD y, MWCOORD width, MWCOORD height,
	MWCOORD sx, MWCOORD sy, MWCOORD swidth, MWCOORD sheight, int id);
void	GdComputeImagePitch(int bpp, int width, int *pitch, int *bytesperpixel);

/* Buffered input functions to replace stdio functions*/
typedef struct {  /* structure for reading images from buffer   */
	unsigned char *start;	/* The pointer to the beginning of the buffer */
	unsigned long offset;	/* The current offset within the buffer       */
	unsigned long size;	/* The total size of the buffer               */
} buffer_t;
void	GdImageBufferInit(buffer_t *buffer, void *startdata, int size);
void	GdImageBufferSeekTo(buffer_t *buffer, unsigned long offset);
int	GdImageBufferRead(buffer_t *buffer, void *dest, unsigned long size);
int	GdImageBufferGetChar(buffer_t *buffer);
char *	GdImageBufferGetString(buffer_t *buffer, char *dest, unsigned int size);
int	GdImageBufferEOF(buffer_t *buffer);

/* individual decoders*/
#ifdef HAVE_BMP_SUPPORT
int	GdDecodeBMP(buffer_t *src, PMWIMAGEHDR pimage);
#endif
#ifdef HAVE_JPEG_SUPPORT
int	GdDecodeJPEG(buffer_t *src, PMWIMAGEHDR pimage, PSD psd, MWBOOL fast_grayscale);
#endif
#ifdef HAVE_PNG_SUPPORT
int	GdDecodePNG(buffer_t *src, PMWIMAGEHDR pimage);
#endif
#ifdef HAVE_GIF_SUPPORT
int	GdDecodeGIF(buffer_t *src, PMWIMAGEHDR pimage);
#endif
#ifdef HAVE_PNM_SUPPORT
int	GdDecodePNM(buffer_t *src, PMWIMAGEHDR pimage);
#endif
#ifdef HAVE_XPM_SUPPORT
int	GdDecodeXPM(buffer_t *src, PMWIMAGEHDR pimage, PSD psd);
#endif
#ifdef HAVE_TIFF_SUPPORT
int	GdDecodeTIFF(char *path, PMWIMAGEHDR pimage);
#endif
#endif /* MW_FEATURE_IMAGES */

/* devlist.c*/
/* field offset*/
#define MWITEM_OFFSET(type, field)    ((long)&(((type *)0)->field))

void * 	GdItemAlloc(unsigned int size);
void	GdListAdd(PMWLISTHEAD pHead,PMWLIST pItem);
void	GdListInsert(PMWLISTHEAD pHead,PMWLIST pItem);
void	GdListRemove(PMWLISTHEAD pHead, PMWLIST pItem);
#define GdItemNew(type)	((type *)GdItemAlloc(sizeof(type)))
#define GdItemFree(ptr)	free((void *)ptr)
/* return base item address from list ptr*/
#define GdItemAddr(p,type,list)	((type *)((long)p - MWITEM_OFFSET(type,list)))

#define GdMalloc(size)                  malloc((size))
#define GdCalloc(num,size)              calloc((num),(size))
#define GdRealloc(addr,oldsize,newsize) realloc((addr),(newsize))
#define GdFree(addr)                    free((addr))
#define GdStrDup(string)                strdup((string))

#define GdFixCursor(psd)	(void*)psd
#define GdCheckCursor(psd, rx1, ry1, rx2, ry2) (void*)psd
#define GdError(str)		(void*)str
#define GdErrorNull(str)		(void*)str

#ifdef __cplusplus
}
#endif

#endif /*_PSD_H*/
