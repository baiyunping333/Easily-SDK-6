﻿/*
* PCF font engine for Microwindows
* Copyright (c) 2002, 2003, 2010 Greg Haerr <greg@censoft.com>
* Copyright (c) 2001, 2002 by Century Embedded Technologies
*
* Supports dynamically loading .pcf and pcf.gz X11 fonts
*
* Written by Jordan Crouse
* Bugfixed by Greg Haerr
*
* 28.01.2003:
*   Patch for big-endian-machines by Klaus Fuerth <Fuerth.ELK@gmx.de>
* 28.06.2005:
*   Code should now work on both big- and little-endian machines.
*   Added better support for leftBearing/rightBearing in input fonts.
*    - Paul Bartholomew <oz_paulb@hotmail.com>
* 29.06.2005:
*   Fixed code that decides whether/not bits/bytes need to be swapped.
*   Now, we always put them into the bit buffer as MSB/MSB, and the code
*   that converts from the PCF input buffer to MWCOREFONT reads a
*   byte-at-a-time.
*    - Paul Bartholomew <oz_paulb@hotmail.com>
* 07.07.2005:
*   In "pcf_createfont()", removed the "unsigned long *ptr" (which
*   points into the MSB/MSB bit buffer that we've just read from
*   the .pcf file), and the "ptr += (xwidth + 1) / 2" adjustment
*   that was done per-row of input data (since this assumes that
*   "sizeof(*ptr) == 4", and also assumes each "row" of input
*   date from the .pcf file is 4-byte aligned).
*   I replaced it with an "unsigned char *p_glyph_in_rowbits", and
*   adjust it after each row by the size of an input glyph 'row'
*   ("glyph_in_rowbytes" - now returned via call to "pcf_readbitmaps()".
*   This change, along with previous change that always reads the
*   .pcf bitmap data as MSB/MSB, should make this code more compatible
*   with various .pcf input files, as well as various CPU architectures.
*    - Paul Bartholomew <oz_paulb@hotmail.com>
* 2005-07-14:
*   indention, check alloc results, use unsigned types for count loops,
*   fix when the default char is undefined (as in cuarabic12.pcf.gz).
*    - Alexander Stohr <alexander.stohr@gmx.de>
* 5/21/2010 - g haerr
*	nano-X doesn't support left and right bearing, must use width only
*	and prepare with bearing builtin.  This fixes space char width bug.
*	Set defaultchar in MWCFONT struct.
*/

#include "mwdevice.h"
#include "devfont.h"
#include "genfont.h"

/* settable params*/
#ifndef PCF_FONT_DIR
#define PCF_FONT_DIR	"fonts/pcf"
#endif

/* The user hase the option including ZLIB and being able to    */
/* directly read compressed .pcf files, or to omit it and save  */
/* space.  The following defines make life much easier          */
#if HAVE_PCFGZ_SUPPORT
#include <zlib.h>
#define FILEP gzFile
#define FOPEN(path, mode)           gzopen(path, mode)
#define FREAD(file, buffer, size)   gzread(file, buffer, size)
#define FSEEK(file, offset, whence) gzseek(file, offset, whence)
#define FCLOSE(file)                gzclose(file)
#else
#define FILEP  FILE *
#define FOPEN(path, mode)           fopen(path, mode)
#define FREAD(file, buffer, size)   fread(buffer, 1, size, file)
#define FSEEK(file, offset, whence) fseek(file, offset, whence)
#define FCLOSE(file)                fclose(file)
#endif

typedef	unsigned char(*FP_READ8)(FILEP file);
typedef	unsigned short(*FP_READ16)(FILEP file);
typedef	uint32_t(*FP_READ32)(FILEP file);

/* Handling routines for PCF fonts, use MWCOREFONT structure */
PMWFONT pcf_createfont(const char *name, MWCOORD height, MWCOORD width, int attr);
static void pcf_unloadfont(PMWFONT font);

static void	get_endian_read_funcs(uint32_t format, FP_READ8 *p_fp_read8,
	FP_READ16 *p_fp_read16, FP_READ32 *p_fp_read32);
static unsigned short	readLSB16(FILEP file);
static uint32_t			readLSB32(FILEP file);
static unsigned short	readMSB16(FILEP file);
static uint32_t			readMSB32(FILEP file);

/* these procs used when font ASCII indexed*/
MWFONTPROCS pcf_fontprocs = {
	0,				/* can't scale*/
	MWTF_ASCII,
	NULL,			/* init*/
	pcf_createfont,
	gen_getfontinfo,
	gen_gettextsize,
	gen_gettextbits,
	pcf_unloadfont,
#if STANDALONE
	gen16_drawtext,
#else
	corefont_drawtext,
#endif
	NULL,			/* setfontsize */
	NULL,			/* setfontrotation */
	NULL,			/* setfontattr */
	NULL			/* duplicate*/
};

/* these procs used when font requires UC16 index*/
static MWFONTPROCS pcf_fontprocs16 = {
	0,				/* can't scale*/
	MWTF_UC16,		/* routines expect unicode 16 */
	NULL,			/* init*/
	pcf_createfont,
	gen_getfontinfo,
	gen16_gettextsize,
	gen_gettextbits,
	pcf_unloadfont,
	gen16_drawtext,
	NULL,			/* setfontsize */
	NULL,			/* setfontrotation */
	NULL,			/* setfontattr */
	NULL			/* duplicate*/
};

/* These are maintained statically for ease FIXME*/
static struct toc_entry *toc;
static uint32_t		 toc_size;

/* Various definitions from the Free86 PCF code */
#define PCF_FILE_VERSION	(('p'<<24)|('c'<<16)|('f'<<8)|1)
#define PCF_PROPERTIES		(1 << 0)
#define PCF_ACCELERATORS	(1 << 1)
#define PCF_METRICS		(1 << 2)
#define PCF_BITMAPS		(1 << 3)
#define PCF_INK_METRICS		(1 << 4)
#define PCF_BDF_ENCODINGS	(1 << 5)
#define PCF_SWIDTHS		(1 << 6)
#define PCF_GLYPH_NAMES		(1 << 7)
#define PCF_BDF_ACCELERATORS	(1 << 8)
#define PCF_FORMAT_MASK		0xFFFFFF00
#define PCF_DEFAULT_FORMAT	0x00000000

#define PCF_GLYPH_PAD_MASK	(3<<0)
#define PCF_GLYPH_PAD_SHIFT	(0)
#define PCF_BYTE_MASK		(1<<2)
#define PCF_BIT_MASK		(1<<3)
#define PCF_SCAN_UNIT_MASK	(3<<4)
#define PCF_SCAN_UNIT_SHIFT	(4)
#define GLYPHPADOPTIONS		4

#define PCF_LSB_FIRST		0
#define PCF_MSB_FIRST		1

/* A few structures that define the various fields within the file */
struct toc_entry {
	int type;
	int format;
	int size;
	long offset;
};

struct prop_entry {
	unsigned int name;
	unsigned char is_string;
	unsigned int value;
};

struct string_table {
	unsigned char *name;
	unsigned char *value;
};

struct metric_entry {
	short leftBearing;
	short rightBearing;
	short width;
	short ascent;
	short descent;
	short attributes;
};

struct encoding_entry {
	unsigned short min_byte2;	/* min_char or min_byte2 */
	unsigned short max_byte2;	/* max_char or max_byte2 */
	unsigned short min_byte1;	/* min_byte1 (hi order) */
	unsigned short max_byte1;	/* max_byte1 (hi order) */
	unsigned short defaultchar;
	uint32_t	   count;		/* count of map entries */
	unsigned short *map;		/* font index -> glyph index */
};

/* This is used to quickly reverse the bits in a field */
static unsigned char _reverse_byte[0x100] = {
	0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
	0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
	0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
	0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
	0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
	0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
	0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
	0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
	0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
	0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
	0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
	0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
	0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
	0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
	0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
	0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
	0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
	0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
	0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
	0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
	0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
	0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
	0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
	0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
	0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
	0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
	0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
	0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
	0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
	0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
	0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
	0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff
};

/*
*	Invert bit order within each BYTE of an array.
*/
static void
bit_order_invert(unsigned char *buf, int nbytes)
{
	for (; --nbytes >= 0; buf++)
		*buf = _reverse_byte[*buf];
}

/*
*	Invert byte order within each 16-bits of an array.
*/
static void
two_byte_swap(unsigned char *buf, int nbytes)
{
	unsigned char c;

	for (; nbytes > 0; nbytes -= 2, buf += 2) {
		c = buf[0];
		buf[0] = buf[1];
		buf[1] = c;
	}
}

/*
*	Invert byte order within each 32-bits of an array.
*/
static void
four_byte_swap(unsigned char *buf, int nbytes)
{
	unsigned char c;

	for (; nbytes > 0; nbytes -= 4, buf += 4) {
		c = buf[0];
		buf[0] = buf[3];
		buf[3] = c;
		c = buf[1];
		buf[1] = buf[2];
		buf[2] = c;
	}
}

/* read an 8 bit byte*/
static unsigned short
readINT8(FILEP file)
{
	unsigned char b;

	FREAD(file, &b, sizeof(b));
	return b;
}


/* Get the offset of the given field */
static long
pcf_get_offset(int item)
{
	uint32_t i;

	for (i = 0; i < toc_size; i++)
		if (item == toc[i].type)
			return toc[i].offset;
	return -1;
}

#if LATER
/* Read the properties from the file */
static int
pcf_readprops(FILEP file, struct prop_entry **prop, struct string_table **strings)
{
	long offset;
	uint32_t format;
	uint32_t num_props;
	uint32_t ssize;
	uint32_t i;
	struct string_table *s;
	struct prop_entry *p;

	unsigned char *string_buffer, *spos;

	if ((offset = pcf_get_offset(PCF_PROPERTIES)) == -1)
		return -1;
	FSEEK(file, offset, SEEK_SET);

	format = readLSB32(file);
	num_props = readLSB32(file);

	p = *prop = (struct prop_entry *)malloc(num_props * sizeof(struct prop_entry));
	if (!p)
		return -1;

	for (i = 0; i < num_props; i++) {
		p[i].name = readLSB32(file);
		p[i].is_string = readINT8(file);
		p[i].value = readLSB32(file);
	}

	/* Pad to 32 bit multiples */
	if (num_props & 3)
		FSEEK(file, 4 - (num_props & 3), SEEK_CUR);

	/* Read the entire set of strings into memory */
	ssize = readLSB32(file);
	spos = string_buffer = (unsigned char *)ALLOCA(ssize);
	if (!string_buffer)
		return -1;
	FREAD(file, string_buffer, ssize);

	/* Allocate the group of strings */
	s = *strings = (struct string_table *)ALLOCA(num_props * sizeof(struct string_table));
	if (!s)
		return -1;

	for (i = 0; i < num_props; i++) {
		s[i].name = (unsigned char *)strdup(spos);
		spos += strlen(s[i].name) + 1;

		if (p[i].is_string) {
			s[i].value = (unsigned char *)strdup(spos);
			spos += strlen(s[i].value) + 1;
		}
		else
			s[i].value = 0;
	}

	FREEA(string_buffer);
	return num_props;
}
#endif

/* Read the actual bitmaps into memory */
static int
pcf_readbitmaps(FILEP file, unsigned char **bits, int *bits_size, int *glyph_pad, uint32_t **offsets)
{
	long offset;
	uint32_t format;
	uint32_t num_glyphs;
	uint32_t pad_index;
	unsigned int i;
	int endian_bits;
	uint32_t *o;
	unsigned char *b;
	FP_READ8	f_read8;
	FP_READ16	f_read16;
	FP_READ32	f_read32;
	int	format_bit_endian, format_byte_endian;
	int	desired_bit_endian, desired_byte_endian;
	int	need_bit_reverse, need_byte_reverse;
	int	format_scan_unit, desired_scan_unit;
	uint32_t bmsize[GLYPHPADOPTIONS];


	if ((offset = pcf_get_offset(PCF_BITMAPS)) == -1)
		return -1;
	FSEEK(file, offset, SEEK_SET);

	format = readLSB32(file);
	get_endian_read_funcs(format, &f_read8, &f_read16, &f_read32);


	format_bit_endian = (format & PCF_BIT_MASK) ? PCF_MSB_FIRST : PCF_LSB_FIRST;
	format_byte_endian = (format & PCF_BYTE_MASK) ? PCF_MSB_FIRST : PCF_LSB_FIRST;
	format_scan_unit = (1 << ((format & PCF_SCAN_UNIT_MASK) >> PCF_SCAN_UNIT_SHIFT));

	desired_bit_endian = PCF_MSB_FIRST;
	desired_byte_endian = PCF_MSB_FIRST;
	desired_scan_unit = (1 << 0);

	if (format_bit_endian != desired_bit_endian)
		need_bit_reverse = 1;
	else
		need_bit_reverse = 0;

	if ((format_bit_endian == format_byte_endian) !=
		(desired_bit_endian == desired_byte_endian)) {
		/* If we want byte reverse, set need_byte_reverse to the size (1/2/4) */
		need_byte_reverse = (1 << ((desired_bit_endian == desired_byte_endian) ?
		format_scan_unit : desired_scan_unit));
	}
	else {
		need_byte_reverse = 0;
	}


	endian_bits = (format & PCF_BIT_MASK) ? PCF_MSB_FIRST : PCF_LSB_FIRST;

	num_glyphs = f_read32(file);

	o = *offsets = (uint32_t *)malloc(num_glyphs * sizeof(uint32_t));
	if (!o)
		return -1;
	for (i = 0; i < num_glyphs; ++i)
		o[i] = f_read32(file);

	for (i = 0; i < GLYPHPADOPTIONS; ++i)
		bmsize[i] = f_read32(file);

	pad_index = ((format & PCF_GLYPH_PAD_MASK) >> PCF_GLYPH_PAD_SHIFT);
	*glyph_pad = (1 << pad_index);
	*bits_size = bmsize[pad_index] ? bmsize[pad_index] : 1;

	/* alloc and read bitmap data*/
	b = *bits = (unsigned char *)malloc(*bits_size);
	if (!b)
		return -1;
	FREAD(file, b, *bits_size);

	/* convert bitmaps*/
	if (need_bit_reverse)
		bit_order_invert(b, *bits_size);

	switch (need_byte_reverse) {
	default:
	case 1:
		break;
	case 2:
		two_byte_swap(b, *bits_size);
		break;
	case 4:
		four_byte_swap(b, *bits_size);
		break;
	}

	return num_glyphs;
}

/* read character metric data*/
static uint32_t
pcf_readmetrics(FILE * file, struct metric_entry **metrics)
{
	long offset;
	uint32_t i, size;
	uint32_t format;
	struct metric_entry *m;
	FP_READ8	f_read8;
	FP_READ16	f_read16;
	FP_READ32	f_read32;

	if ((offset = pcf_get_offset(PCF_METRICS)) == -1)
		return -1;
	FSEEK(file, offset, SEEK_SET);

	format = readLSB32(file);
	get_endian_read_funcs(format, &f_read8, &f_read16, &f_read32);

	if ((format & PCF_FORMAT_MASK) == PCF_DEFAULT_FORMAT) {
		size = f_read32(file);		/* 32 bits - Number of metrics*/

		m = *metrics = (struct metric_entry *)malloc(size * sizeof(struct metric_entry));
		if (!m)
			return -1;

		for (i = 0; i < size; i++) {
			m[i].leftBearing = f_read16(file);
			m[i].rightBearing = f_read16(file);
			m[i].width = f_read16(file);
			m[i].ascent = f_read16(file);
			m[i].descent = f_read16(file);
			m[i].attributes = f_read16(file);
		}
	}
	else {
		size = f_read16(file);		/* 16 bits - Number of metrics*/

		m = *metrics = (struct metric_entry *)malloc(size * sizeof(struct metric_entry));
		if (!m)
			return -1;

		for (i = 0; i < size; i++) {
			m[i].leftBearing = f_read8(file) - 0x80;
			m[i].rightBearing = f_read8(file) - 0x80;
			m[i].width = f_read8(file) - 0x80;
			m[i].ascent = f_read8(file) - 0x80;
			m[i].descent = f_read8(file) - 0x80;
		}
	}
	return size;
}

/* read encoding table*/
static int
pcf_read_encoding(FILE * file, struct encoding_entry **encoding)
{
	long offset;
	uint32_t n;
	uint32_t format;
	struct encoding_entry *e;
	FP_READ8	f_read8;
	FP_READ16	f_read16;
	FP_READ32	f_read32;

	if ((offset = pcf_get_offset(PCF_BDF_ENCODINGS)) == -1)
		return -1;
	FSEEK(file, offset, SEEK_SET);

	format = readLSB32(file);
	get_endian_read_funcs(format, &f_read8, &f_read16, &f_read32);

	e = *encoding = (struct encoding_entry *)malloc(sizeof(struct encoding_entry));
	if (!e)
		return -1;
	e->min_byte2 = f_read16(file);
	e->max_byte2 = f_read16(file);
	e->min_byte1 = f_read16(file);
	e->max_byte1 = f_read16(file);
	e->defaultchar = f_read16(file);
	e->count = ((uint32_t)e->max_byte2 - (uint32_t)e->min_byte2 + 1) *
		((uint32_t)e->max_byte1 - (uint32_t)e->min_byte1 + 1);
	e->map = (unsigned short *)malloc(e->count * sizeof(unsigned short));
	if (!e->map)
		return -1;

	for (n = 0; n < e->count; ++n) {
		e->map[n] = f_read16(file);
		/*DPRINTF("ncode %x (%c) %x\n", n, n, e->map[n]);*/
	}
	//DPRINTF("size %d byte1 %d,%d byte2 %d,%d\n", e->count, e->min_byte1, e->max_byte1, e->min_byte2, e->max_byte2);
	return e->count;
}

static int
pcf_read_toc(FILE * file, struct toc_entry **toc, uint32_t *size)
{
	uint32_t i;
	uint32_t version;
	struct toc_entry *t;

	FSEEK(file, 0, SEEK_SET);

	/* Verify the version */
	version = readLSB32(file);
	if (version != PCF_FILE_VERSION)
		return -1;

	*size = readLSB32(file);
	t = *toc = (struct toc_entry *)calloc(sizeof(struct toc_entry), *size);
	if (!t)
		return -1;

	/* Read in the entire table of contents */
	for (i = 0; i<*size; ++i) {
		t[i].type = readLSB32(file);
		t[i].format = readLSB32(file);
		t[i].size = readLSB32(file);
		t[i].offset = readLSB32(file);
	}

	return 0;
}

/* create font and allocate MWCOREFONT struct*/
PMWFONT pcf_createfont(const char *name, MWCOORD height, MWCOORD width, int attr)
{
	FILE *file = NULL;
	MWCOREFONT *pf = NULL;
	uint32_t i, count, offset;
	int bsize;
	int bwidth;
	int err = -1;
	struct metric_entry *metrics = NULL;
	struct encoding_entry *encoding = NULL;
	MWIMAGEBITS *output;
	unsigned char *glyphs = NULL;
	uint32_t *glyphs_offsets = NULL;
	int max_width = 0, max_descent = 0, max_ascent = 0, max_height;
	int result;
	unsigned int glyph_count;
	uint32_t *goffset = NULL;
	unsigned char *gwidth = NULL;
	int uc16;
	int glyph_pad;
	char fname[256];

	/* Try to open the file */
	file = FOPEN(name, "rb");
	if (!file) {
		sprintf(fname, "%s/%s", PCF_FONT_DIR, name);
		file = FOPEN(fname, "rb");
	}
	if (!file)
		return NULL;

	if (!(pf = (MWCOREFONT *)malloc(sizeof(MWCOREFONT))))
		goto err_exit;

	if (!(pf->cfont = (PMWCFONT)calloc(sizeof(MWCFONT), 1)))
		goto err_exit;


	/* Read the table of contents */
	if (pcf_read_toc(file, &toc, &toc_size) == -1)
		goto err_exit;

	/* Now, read in the bitmaps */
	result = pcf_readbitmaps(file, &glyphs, &bsize, &glyph_pad, &glyphs_offsets);
	if (result == -1)
		goto err_exit;

	glyph_count = result;
	//DPRINTF("glyph_count = %u (%x)\n", glyph_count, glyph_count);

	if (pcf_read_encoding(file, &encoding) == -1)
		goto err_exit;

	pf->cfont->firstchar = encoding->min_byte2 * (encoding->min_byte1 + 1);
	pf->cfont->defaultchar = encoding->defaultchar;
	//DPRINTF("firstchar %d\n", pf->cfont->firstchar);
	//DPRINTF("default char %d (%x)\n", pf->cfont->defaultchar, pf->cfont->defaultchar);

	/* Read in the metrics */
	count = pcf_readmetrics(file, &metrics);

	/* Calculate various maximum values */
	for (i = 0; i < count; i++) {
		int width = metrics[i].rightBearing;

		/* negative left bearing not handled*/
		//if (metrics[i].leftBearing < 0)
		//width += abs(metrics[i].leftBearing);

		/* handle space and other cases where width > rightBearing*/
		if (metrics[i].width > width)
			width = metrics[i].width;

		if (width > max_width)
			max_width = width;
		if (metrics[i].ascent > max_ascent)
			max_ascent = metrics[i].ascent;
		if (metrics[i].descent > max_descent)
			max_descent = metrics[i].descent;
	}
	max_height = max_ascent + max_descent;

	pf->cfont->maxwidth = max_width;
	pf->cfont->height = max_height;
	pf->cfont->ascent = max_ascent;

	/* Allocate enough room to hold all of the bits and the offsets */
	bwidth = (max_width + 15) / 16;

	pf->cfont->bits = (MWIMAGEBITS *)calloc((max_height * (sizeof(MWIMAGEBITS) * bwidth)), glyph_count);
	if (!pf->cfont->bits)
		goto err_exit;

	goffset = (uint32_t *)malloc(glyph_count * sizeof(uint32_t));
	if (!goffset)
		goto err_exit;

	gwidth = (unsigned char *)malloc(glyph_count * sizeof(unsigned char));
	if (!gwidth)
		goto err_exit;

	output = (MWIMAGEBITS *)pf->cfont->bits;
	offset = 0;

	/* copy and convert from packed BDF format to MWCFONT format*/
	for (i = 0; i < glyph_count; i++) {
		int h, w;
		int y = max_height;
		unsigned char *p_glyph_in_rowbits = glyphs + glyphs_offsets[i];
		int glyph_in_rowbytes;
		int width;		/* glyph advancement width*/
		int lwidth;		/* # words image width*/

		/* # words image width, corrected for bounding box problem*/
		int xwidth = (metrics[i].rightBearing - metrics[i].leftBearing + 15) / 16;

		/* Calculate width (advancement) for glyph*/
		width = metrics[i].rightBearing;

		/* negative left bearing not handled*/
		//if (metrics[i].leftBearing < 0)
		//width += abs(metrics[i].leftBearing);

		/* handle space and other cases where width > rightBearing*/
		if (metrics[i].width > width)
			width = metrics[i].width;
		gwidth[i] = width;

		if (metrics[i].leftBearing < 0)
		{
			(void*)0;
			//DPRINTF("glyph %d (%c) left bearing %d, right bearing %d, width %d\n", i, i, metrics[i].leftBearing, metrics[i].rightBearing, gwidth[i]);
		}

		lwidth = (width + 15) / 16;		/* word padding for gen16 routines*/

		goffset[i] = offset;
		offset += lwidth * max_height;

		glyph_in_rowbytes = xwidth * sizeof(unsigned short);
		glyph_in_rowbytes += glyph_pad - 1;
		glyph_in_rowbytes &= ~(glyph_pad - 1);

		/* fill in blank rows above glyph bitmap*/
		for (h = 0; h < (max_ascent - metrics[i].ascent); h++) {
			for (w = 0; w < lwidth; w++) {
				*output++ = 0;
			}
			y--;
		}

		/* copy glyph bits into MWCFONT format*/
		for (h = 0; h < (metrics[i].ascent + metrics[i].descent); h++) {
			int            bearing, carry_shift;
			unsigned short carry = 0;
			unsigned char	*p8;
			unsigned short	val16;

			/* leftBearing correction*/
			bearing = metrics[i].leftBearing;

			if (bearing < 0)	/* negative bearing not handled yet*/
				bearing = 0;
			carry_shift = 16 - bearing;

			for (w = 0; w < lwidth; w++) {
				if (w < xwidth) {
					p8 = p_glyph_in_rowbits + (w * sizeof(unsigned short));
					val16 = ((((unsigned short)p8[0]) << 8) | p8[1]);
				}
				else {
					val16 = 0;
				}
				*output++ = (val16 >> bearing) | carry;
				carry = val16 << carry_shift;
			}

			p_glyph_in_rowbits += glyph_in_rowbytes;

			y--;
		}

		/* fill in blank rows below glyph bitmap*/
		for (; y > 0; y--) {
			for (w = 0; w < lwidth; w++) {
				*output++ = 0;
			}
		}
	}

	/* reorder offsets and width according to encoding map */
	pf->cfont->offset = (uint32_t *)malloc(encoding->count * sizeof(uint32_t));
	if (!pf->cfont->offset)
		goto err_exit;

	pf->cfont->width = (unsigned char *)malloc(encoding->count * sizeof(unsigned char));
	if (!pf->cfont->width)
		goto err_exit;

	for (i = 0; i < encoding->count; ++i) {
		unsigned short n = encoding->map[i];

		if (n == 0xffff) {	/* map non-existent chars to default char */
			/* get default char map index*/
			n = encoding->map[encoding->defaultchar - pf->cfont->firstchar];

			/* if default is non-existent then char is empty */
			if (n == 0xffff) {
				/* casts necessary to remove const*/
				((uint32_t *)pf->cfont->offset)[i] = 0;
				((unsigned char *)pf->cfont->width)[i] = 0;
				continue;
			}
		}
		/* casts necessary to remove const*/
		((uint32_t *)pf->cfont->offset)[i] = goffset[n];
		((unsigned char *)pf->cfont->width)[i] = gwidth[n];
	}
	pf->cfont->size = encoding->count;

	uc16 = pf->cfont->firstchar > 255 || (pf->cfont->firstchar + pf->cfont->size) > 255;
	pf->fontprocs = uc16 ? &pcf_fontprocs16 : &pcf_fontprocs;
	pf->fontsize = pf->fontrotation = pf->fontattr = 0;
	pf->name = "PCF";
	err = 0;

err_exit:
	if (goffset)
		free(goffset);
	if (gwidth)
		free(gwidth);
	if (encoding) {
		if (encoding->map)
			free(encoding->map);
		free(encoding);
	}
	if (metrics)
		free(metrics);
	if (glyphs)
		free(glyphs);
	if (glyphs_offsets)
		free(glyphs_offsets);

	if (toc)
		free(toc);
	toc = 0;
	toc_size = 0;

	if (file)
		FCLOSE(file);

	if (err == 0 && pf)
		return (PMWFONT)pf;

	pcf_unloadfont((PMWFONT)pf);
	return 0;
}

void
pcf_unloadfont(PMWFONT font)
{
	PMWCOREFONT pf = (PMWCOREFONT)font;
	PMWCFONT    pfc = pf->cfont;

	if (pfc) {
		if (pfc->width)
			free((char *)pf->cfont->width);
		if (pfc->offset)
			free((char *)pf->cfont->offset);
		if (pfc->bits)
			free((char *)pf->cfont->bits);

		free(pf->cfont);
	}

	free(font);
}

static void
get_endian_read_funcs(uint32_t format, FP_READ8 *p_fp_read8, FP_READ16 *p_fp_read16,
FP_READ32 *p_fp_read32)
{
	*p_fp_read8 = (FP_READ8)readINT8;

	if (format & PCF_BYTE_MASK) {
		/* MSB byte order */
		*p_fp_read16 = readMSB16;
		*p_fp_read32 = readMSB32;
	}
	else {
		/* LSB byte order */
		*p_fp_read16 = readLSB16;
		*p_fp_read32 = readLSB32;
	}
}

static unsigned short
readLSB16(FILEP file)
{
	unsigned short	ret;
	unsigned char	ch;
	int	i;

	ret = 0;
	for (i = 0; i < (16 / 8); i++) {
		ch = readINT8(file);
		ret |= (((unsigned short)ch) << (i * 8));
	}
	return ret;
}

static uint32_t
readLSB32(FILEP file)
{
	uint32_t			ret = 0;
	unsigned char	ch;
	int	i;

	for (i = 0; i < (32 / 8); i++) {
		ch = readINT8(file);
		ret |= (((uint32_t)ch) << (i * 8));
	}
	return ret;
}


static unsigned short
readMSB16(FILEP file)
{
	unsigned short	ret = 0;
	unsigned char	ch;
	int	i;

	for (i = 0; i < (16 / 8); i++) {
		ch = readINT8(file);
		ret <<= 8;
		ret |= ch;
	}
	return ret;
}

static uint32_t
readMSB32(FILEP file)
{
	uint32_t			ret;
	unsigned char	ch;
	int	i;

	ret = 0;
	for (i = 0; i < (32 / 8); i++) {
		ch = readINT8(file);
		ret <<= 8;
		ret |= ch;
	}
	return ret;
}

