#ifndef _MWCONFIG_H
#define _MWCONFIG_H
/*
* Copyright (c) 1999, 2000, 2001, 2002, 2003, 2005, 2010 Greg Haerr <greg@censoft.com>
* Portions Copyright (c) 2002 by Koninklijke Philips Electronics N.V.
*
* Exported Microwindows engine typedefs and defines
*/

#include "../xdkdef.h"

/* Changeable limits and options*/
#define UNIFORMPALETTE	1		/* =1 for 256 entry uniform palette (required for palette alpha blending)*/
#define POLYREGIONS	1			/* =1 includes polygon regions*/
#define ANIMATEPALETTE	0		/* =1 animated palette test*/

/* the fontmapper is obsolete, according to Greg */
#define FONTMAPPER	0			/* =1 for Morten's font mapper*/
#define FASTJPEG	1			/* =1 for temp quick jpeg 8bpp*/
#if RTEMS || __ECOS || PSP
#define HAVE_MMAP       0
#else
#define HAVE_MMAP       1       /* =1 to use mmap if available*/
#endif
#define FFMINAA_HEIGHT	0		/* min height for FT antialias*/

#if !_MINIX && !RTEMS && !__ECOS
#define HAVESELECT	1			/* has select system call*/
#endif

#define TRANSLATE_ESCAPE_SEQUENCES  1		/* =1 to parse fnkeys w/tty driver*/

#ifndef HAVEFLOAT
#define HAVEFLOAT	1			/* =1 incl float, GdArcAngle*/
#endif

#ifndef DYNAMICREGIONS
#define DYNAMICREGIONS	1		/* =1 to use MWCLIPREGIONS*/
#endif

#ifndef MW_FEATURE_IMAGES
#if !((DOS_DJGPP) || (__PACIFIC__) || (DOS_TURBOC))
#define MW_FEATURE_IMAGES 1		/* =1 to enable GdLoadImage* / GdDrawImage* */
#else
#define MW_FEATURE_IMAGES 0		/* platform doesn't support images*/
#endif
#endif

#ifndef MW_FEATURE_TIMERS
#if UNIX || DOS_DJGPP || RTEMS || HAVE_TIMER_SUPPORT
#define MW_FEATURE_TIMERS 1		/* =1 to include MWTIMER support */
#else
#define MW_FEATURE_TIMERS 0		/* Other platforms do not support timers yet */
#endif
#endif

/* determine compiler capability for handling EPRINTF/DPRINTF macros*/
#ifndef MW_FEATURE_GDERROR
#if (defined(GCC_VERSION) && (GCC_VERSION >= 2093)) || (defined(__GNUC__) && (((__GNUC__ >= 2) && (__GNUC_MINOR__ >= 95)) || (__GNUC__ > 2)))
#define MW_FEATURE_GDERROR	0		/* use fprintf instead of GdError*/
#else
#define MW_FEATURE_GDERROR	1		/* use GdError for errors*/
#endif
#endif /* MW_FEATURE_GDERROR*/

#if MW_FEATURE_GDERROR
#define EPRINTF			GdError		/* error output*/
#if DEBUG
#define DPRINTF			GdError		/* debug output*/
#else
#define DPRINTF			GdErrorNull	/* no debug output*/
#endif
#else

/* GCC compiler-only macro magic to save space */
#include <stdio.h>    /* For stderr */
#define EPRINTF(str, args...)   fprintf(stderr, str, ##args)  /* error output*/
#if DEBUG
#define DPRINTF(str, args...)   fprintf(stderr, str, ##args)  /* debug output*/
#else
#define DPRINTF(str, ...)	/* no debug output*/
#endif
#endif /* MW_FEATURE_GDERROR*/

/* Sanity check: VTSWITCH involves a timer. */
#if VTSWITCH && !MW_FEATURE_TIMERS
#error VTSWITCH depends on MW_FEATURE_TIMERS - disable VTSWITCH in config or enable MW_FEATURE_TIMERS in this file
#endif

/* Which low-level psd->DrawArea routines to include. */
#define MW_FEATURE_PSDOP_COPY                   1
#define MW_FEATURE_PSDOP_ALPHAMAP               0
#define MW_FEATURE_PSDOP_ALPHACOL               1
#define MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST 1
#define MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST 1

/* Override some of the above defines, for features which are required
* for the Microwindows FreeType 2 font driver
*/
#ifdef HAVE_FREETYPE_2_SUPPORT
#undef  MW_FEATURE_PSDOP_ALPHACOL
#define MW_FEATURE_PSDOP_ALPHACOL 1
#undef  MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST
#define MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST 1
#endif

/* Operations for the Blitter/Area functions */

#if MW_FEATURE_PSDOP_COPY
/*
* Copy an image to screen.
* Params:
* dstx, dsty  - Destination for top left of image
* dstw, dsth  - Image size
* srcx, srcy  - Start co-ordinates in source image
* src_linelen - Source image stride, in pixels
* pixels      - Image to copy from in hw display format.
* misc        - Ignored.
* gr_usebg    - Checked for PSD_COPY only.
* bg_color    - Background color to ignore if PSDOP_COPYTRANS.
* fg_color    - Ignored.
*/
#define PSDOP_COPY	0	/* if gr_usebg COPYALL else COPYTRANS*/
#define PSDOP_COPYALL	1	/* copy image bits to screen*/
#define PSDOP_COPYTRANS 2	/* copy non-background bits to screen*/
#endif /* MW_FEATURE_PSDOP_COPY */

#if MW_FEATURE_PSDOP_ALPHAMAP
/*
* Copy an image to screen, using an alpha map.
* Params:
* dstx, dsty  - Destination for top left of image
* dstw, dsth  - Image size
* srcx, srcy  - Start co-ordinates in source image
* src_linelen - Source image stride, in pixels
* pixels      - Image to copy from.  Format: same color model as display.
* misc        - Alpha map.  Format: ADDR8, entries
*               are alpha values in range 0-255.
* gr_usebg    - Ignored.  FIXME If set, should blend to bg_color.
* bg_color    - Ignored.  FIXME Should be used if gr_usebg is set.
* fg_color    - Ignored.
*/
#define PSDOP_ALPHAMAP	3
#endif /* MW_FEATURE_PSDOP_ALPHAMAP */

#if MW_FEATURE_PSDOP_ALPHACOL
/*
* Draws an alpha map to screen (e.g. an anti-aliased font).
* Params:
* dstx, dsty  - Destination for top left of image
* dstw, dsth  - Image size
* srcx, srcy  - Start co-ordinates in source alpha map
* src_linelen - Source image stride, in pixels
* data        - Alpha map.  Format: ADDR8, entries
*               are alpha values in range 0-255.
* fg_color    - The color to draw in, in the display format.
* bg_color    - Ignored.  FIXME Should be used if gr_usebg is set.
* gr_usebg    - Ignored.  FIXME If set, should blend to bg_color.
* pixels      - Ignored.
*/
#define PSDOP_ALPHACOL	4
#endif /* MW_FEATURE_PSDOP_ALPHACOL */

#if MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST
/*
* Draws a mono bitmap to screen (e.g. a mono font).
* This variant takes the bitmap as an array of bytes,
* where the Least Significant Bit in each byte is
* used to set the left-most of the eight pixels
* controlled by that byte.  I.e:
*
* [ 1 1 1 1 0 0 0 1 ] == 0x8F
*
* Params:
* dstx, dsty  - Destination for top left of image
* dstw, dsth  - Image size
* srcx, srcy  - Start co-ordinates in source alpha map
* src_linelen - Source image stride, in pixels
* pixels      - The bitmap.  Format: ADDR8, LSB is drawn first.
* fg_color    - The color to draw "1" bits in, in the display format.
* bg_color    - The color to draw "0" bits in, in the display format.
* gr_usebg    - If zero, then "0" bits are transparent.  If nonzero,
*               then "0" bits are bg_color.
*/
#define PSDOP_BITMAP_BYTES_LSB_FIRST	5
#endif /* MW_FEATURE_PSDOP_BITMAP_BYTES_LSB_FIRST */

#if MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST
/*
* Draws a mono bitmap to screen (e.g. a mono font).
* This variant takes the bitmap as an array of bytes,
* where the Most Significant Bit in each byte is
* used to set the left-most of the eight pixels
* controlled by that byte.  I.e:
*
* [ 1 1 1 1 0 0 0 1 ] == 0xF1
*
* Params:
* dstx, dsty  - Destination for top left of image
* dstw, dsth  - Image size
* srcx, srcy  - Start co-ordinates in source alpha map
* src_linelen - Source image stride, in pixels
* pixels      - The bitmap.  Format: ADDR8, MSB is drawn first.
* fg_color    - The color to draw "1" bits in, in the display format.
* bg_color    - The color to draw "0" bits in, in the display format.
* gr_usebg    - If zero, then "0" bits are transparent.  If nonzero,
*               then "0" bits are bg_color.
*/
#define PSDOP_BITMAP_BYTES_MSB_FIRST	6
#endif /* MW_FEATURE_PSDOP_BITMAP_BYTES_MSB_FIRST */

#define HAVE_FNT_SUPPORT		1
#define	HAVE_PCF_SUPPORT		1

#endif /*_MWCONFIG_H*/
