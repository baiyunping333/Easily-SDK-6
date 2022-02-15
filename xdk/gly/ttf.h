/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc fonts document

	@module	ttf.h | interface file

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

#ifndef _TTFDEF_H
#define _TTFDEF_H

#include "../xdkdef.h"

typedef struct _ttf_table_record_t{
	byte_t	tableTag[4];	//Table identifier.
	dword_t	checksum;	//Checksum for this table.
	dword_t	offset;		//Offset from beginning of font file.
	dword_t	length;		//Length of this table.
	
	dword_t* table;
}ttf_table_record_t;

typedef struct _ttf_file_head_t{
	dword_t	sfntVersion;	//0x00010000 or 0x4F54544F ('OTTO') — see below.
	sword_t	numTables;		//Number of tables.
	sword_t	searchRange;	//Maximum power of 2 less than or equal to numTables, times 16 ((2 * *floor(log2(numTables))) * 16, where “**” is an exponentiation operator).
	sword_t	entrySelector;	//Log2 of the maximum power of 2 less than or equal to numTables(log2(searchRange / 16), which is equal to floor(log2(numTables))).
	sword_t	rangeShift;		//numTables times 16, minus searchRange((numTables * 16) - searchRange).

	ttf_table_record_t*	pTableRecords;  //Table records array—one for each top - level table in the font
}ttf_file_head_t;

typedef struct _ttf_head_table_t{
	sword_t	majorVersion;	//Major version number of the font header table — set to 1.
	sword_t	minorVersion;	//Minor version number of the font header table — set to 0.
	dword_t	fontRevision;	//Set by font manufacturer.
	dword_t	checksumAdjustment;	//To compute : set it to 0, sum the entire font as uint32, then store 0xB1B0AFBA - sum.If the font is used as a component in a font collection file, the value of this field will be invalidated by changes to the file structure and font table directory, and must be ignored.
	dword_t	magicNumber;	//Set to 0x5F0F3CF5.
	sword_t	flags;		/*
						Bit 0 : Baseline for font at y = 0.
						Bit 1 : Left sidebearing point at x = 0 (relevant only for TrueType rasterizers) — see the note below regarding variable fonts.
						Bit 2 : Instructions may depend on point size.
						Bit 3 : Force ppem to integer values for all internal scaler math; may use fractional ppem sizes if this bit is clear.It is strongly recommended that this be set in hinted fonts.
						Bit 4: Instructions may alter advance width(the advance widths might not scale linearly).
						Bit 5 : This bit is not used in OpenType, and should not be set in order to ensure compatible behavior on all platforms.If set, it may result in different behavior for vertical layout in some platforms. (See Apple’s specification for details regarding behavior in Apple platforms.)
						Bits 6–10 : These bits are not used in Opentype and should always be cleared. (See Apple’s specification for details regarding legacy used in Apple platforms.)
						Bit 11 : Font data is “lossless” as a result of having been subjected to optimizing transformation and / or compression(such as e.g.compression mechanisms defined by ISO / IEC 14496 - 18, MicroType Express, WOFF 2.0 or similar) where the original font functionality and features are retained but the binary compatibility between input and output font files is not guaranteed.As a result of the applied transform, the DSIG table may also be invalidated.
						Bit 12 : Font converted(produce compatible metrics).
						Bit 13 : Font optimized for ClearType™.Note, fonts that rely on embedded bitmaps(EBDT) for rendering should not be considered optimized for ClearType, and therefore should keep this bit cleared.
						Bit 14 : Last Resort font.If set, indicates that the glyphs encoded in the 'cmap' subtables are simply generic symbolic representations of code point ranges and don’t truly represent support for those code points.If unset, indicates that the glyphs encoded in the 'cmap' subtables represent proper support for those code points.
						Bit 15 : Reserved, set to 0.
						*/
	sword_t	unitsPerEm;		//Set to a value from 16 to 16384. Any value in this range is valid.In fonts that have TrueType outlines, a power of 2 is recommended as this allows performance optimizations in some rasterizers.
	lword_t	created;		//Number of seconds since 12 : 00 midnight that started January 1st 1904 in GMT / UTC time zone.
	lword_t	modified;		//Number of seconds since 12 : 00 midnight that started January 1st 1904 in GMT / UTC time zone.
	short	xMin;			//For all glyph bounding boxes. (Glyphs without contours are ignored.)
	short	yMin;			//For all glyph bounding boxes. (Glyphs without contours are ignored.)
	short	xMax;			//For all glyph bounding boxes. (Glyphs without contours are ignored.)
	short	yMax;			//For all glyph bounding boxes. (Glyphs without contours are ignored.)
	sword_t	macStyle;		/*
							Bit 0 : Bold(if set to 1);
							Bit 1: Italic(if set to 1)
							Bit 2 : Underline(if set to 1)
							Bit 3 : Outline(if set to 1)
							Bit 4 : Shadow(if set to 1)
							Bit 5 : Condensed(if set to 1)
							Bit 6 : Extended(if set to 1)
							Bits 7–15 : Reserved(set to 0).
							*/
	sword_t	lowestRecPPEM;	//Smallest readable size in pixels.
	short	fontDirectionHint;	/*
								Deprecated(Set to 2). 
								0 : Fully mixed directional glyphs; 1: Only strongly left to right;2: Like 1 but also contains neutrals;
								-1: Only strongly right to left;
								-2: Like - 1 but also contains neutrals.
								(A neutral character has no inherent directionality; it is not a character with zero(0) width.Spaces and punctuation are examples of neutral characters.Non - neutral characters are those with inherent directionality.For example, Roman letters(left - to - right) and Arabic letters(right - to - left) have directionality.In a “normal” Roman font where spaces and punctuation are present, the font direction hints should be set to two(2).)
								*/
	short	indexToLocFormat;	//0 for short offsets(Offset16), 1 for long(Offset32).
	short	glyphDataFormat;	//0 for current format.
}ttf_head_table_t;

#define TTF_ID_COPYRIGHT		0	//Copyright notice.
#define TTF_ID_FONTFAMILY		1	//Font Family name
#define TTF_ID_FONTSUBFAMILY	2	//Font Subfamily name
#define TTF_ID_UUID				3	//Unique font identifier
#define TTF_ID_FULLNAME			4	//Full font name
#define TTF_ID_VERSION			5	//Version string
#define TTF_ID_POSTSCIPT		6	//PostScript name for the font
#define TTF_ID_TRADEMARK		7	//Trademark
#define TTF_ID_MANUFACTURER		8	//Manufacturer Name
#define TTF_ID_DESIGNER			9	//Designer;
#define TTF_ID_DESCRIPTION		10	//Description;
#define TTF_ID_VENDOR_URL		11	//URL Vendor;
#define TTF_ID_DESIGNER_URL		12	//URL Designer;
#define TTF_ID_LICENSE_DESCRIP	13	//License Description;
#define TTF_ID_LICENSE_INFO		14	//License Info URL;
#define TTF_ID_TYPOGRAPHIC_FAMILY 16	//Typographic Family name :
#define TTF_ID_TYPOGRAPHIC_SUBFAMILY 17	//Typographic Subfamily name :
#define TTF_ID_COMPATIBLEFULL	18	//Compatible Full(Macintosh only);
#define TTF_ID_SAMPLETEXT		19	//Sample text;
#define TTF_ID_POSTSCRIPT_CID	20	//PostScript CID findfont name; 
#define TTF_ID_WWS_FAMILY		21	//WWS Family Name.
#define TTF_ID_WWS_SUBFAMILY	22	//WWS Subfamily Name.
#define TTF_ID_LIGHT_BACKGROUND	23	//Light Background Palette.
#define TTF_ID_DARK_BACKGROUND	24	//Dark Background Palette.
#define TTF_ID_POSTSCRIPT_NAMEPREFIX 25	//Variations PostScript Name Prefix.

typedef struct _ttf_name_record_t{
	sword_t	platformID;		//Platform ID.
	sword_t	encodingID;		//Platform - specific encoding ID.
	sword_t	languageID;		//Language ID.
	sword_t	nameID;			//Name ID.
	sword_t	length;			//String length(in bytes).
	sword_t	stringOffset;	//String offset from start of storage area(in bytes).
	byte_t* stringData;		//String data.
}ttf_name_record_t;

typedef struct _ttf_langtag_record_t{
	sword_t	length;			//Language - tag string length(in bytes)
	sword_t	langTagOffset;	//Language - tag string offset from start of storage area(in bytes).
}ttf_langtag_record_t;

typedef struct _ttf_name_table_t{
	sword_t	version;		//Table version number(= 0 or 1).
	sword_t	count;			//Number of name records.
	sword_t	storageOffset;	//Offset to start of string storage(from start of table).
	ttf_name_record_t*	pNameRecords;	//The name records where count is the number of records.
	sword_t	langTagCount;	//Number of language - tag records, exists when verion number is 1.
	ttf_langtag_record_t*	pLangTagRecords;	//The language - tag records where langTagCount is the number of records, exists when verion number is 1.
}ttf_name_table_t;

#define TTF_PLATFORM_UNICODE	0	//Unicode	Various
#define TTF_PLATFORM_MACINTOSH	1	//Macintosh	Script manager code
#define TTF_PLATFORM_ISO		2	//ISO[deprecated]	ISO encoding[deprecated]
#define TTF_PLATFORM_WINDOWS	3	//Windows	Windows encoding
#define TTF_PLATFORM_CUSTOM		4	//Custom	Custom

#define TTF_ENCODING_UNICODE10	0	//Unicode 1.0 semantics—deprecated
#define TTF_ENCODING_UNICODE11	1	//Unicode 1.1 semantics—deprecated
#define TTF_ENCODING_ISO		2	//ISO / IEC 10646 semantics—deprecated
#define TTF_ENCODING_UNICODE20_BMP	3	//Unicode 2.0 and onwards semantics, Unicode BMP only
#define TTF_ENCODING_UNICODE20_FULL	4	//Unicode 2.0 and onwards semantics, Unicode full repertoire
#define TTF_ENCODING_UNICODE20_SUBTABLE14	5	//Unicode Variation Sequences—for use with subtable format 14
#define TTF_ENCODING_UNICODE20_SUBTABLE13	6	//Unicode full repertoire—for use with subtable format 13

typedef struct _ttf_encoding_subtable_t{
	sword_t	format;			//Format number is set to 0.
	sword_t	length;			//This is the length in bytes of the subtable.
	sword_t	language;		//For requirements on use of the language field, see “Use of the language field in 'cmap' subtables” in this document.
	sword_t arrayCount;
	sword_t* pCodeArray;		//
	sword_t* pGlyphIdArray;	//An array that maps character codes to glyph index values.
}ttf_encoding_subtable_t;

typedef struct _ttf_encoding_record_t{
	sword_t	platformID;		//Platform ID. 0: Unicode, 1: Macintosh, 2: ISO, 3: Windows, 4: Custom.
	sword_t	encodingID;		//Platform - specific encoding ID. 0: Unicode 1.0 semantics, 1: Unicode 1.1 semantics, 2: ISO/IEC 10646 semantics, 3: Unicode 2.0 and onwards semantics, 4: Unicode 2.0 and onwards semantics, 5: Unicode Variation Sequences, 6: Unicode full repertoire.
	dword_t	subtableOffset;	//Byte offset from beginning of table to the subtable for this encoding.

	ttf_encoding_subtable_t subtable;
}ttf_encoding_record_t;


typedef struct _ttf_cmap_table_t{
	sword_t	version;		//Table version number(0).
	sword_t	numTables;		//Number of encoding tables that follow.
	ttf_encoding_record_t*	pEncodingRecords;
}ttf_cmap_table_t;

typedef struct _ttf_maxp_table_t{
	dword_t	version;		//0x00010000 for version 1.0.
	sword_t	numGlyphs;		//The number of glyphs in the font.
	sword_t	maxPoints;		//Maximum points in a non - composite glyph.
	sword_t	maxContours;	//Maximum contours in a non - composite glyph.
	sword_t	maxCompositePoints;	//Maximum points in a composite glyph.
	sword_t	maxCompositeContours;	//Maximum contours in a composite glyph.
	sword_t	maxZones;		//1 if instructions do not use the twilight zone(Z0), or 2 if instructions do use Z0; should be set to 2 in most cases.
	sword_t	maxTwilightPoints;	//Maximum points used in Z0.
	sword_t	maxStorage;		//Number of Storage Area locations.
	sword_t	maxFunctionDefs;	//Number of FDEFs, equal to the highest function number + 1.
	sword_t	maxInstructionDefs;	//Number of IDEFs.
	sword_t	maxStackElements;	//Maximum stack depth across Font Program('fpgm' table), CVT Program('prep' table) and all glyph instructions(in the 'glyf' table).
	sword_t	maxSizeOfInstructions;	//Maximum byte count for glyph instructions.
	sword_t	maxComponentElements;	//Maximum number of components referenced at “top level” for any composite glyph.
	sword_t	maxComponentDepth;		//Maximum levels of recursion; 1 for simple components.
}ttf_maxp_table_t;

typedef struct _ttf_loca_table_t{
	dword_t* pOffsets;	//[numGlyphs + 1] The value for numGlyphs is found in the 'maxp' table.
}ttf_loca_table_t;

#define SIMPLE_GLYPH_ON_CURVE_POINT		0x01	//Bit 0: If set, the point is on the curve; otherwise, it is off the curve.
#define SIMPLE_GLYPH_ON_X_SHORT_VECTOR	0x02	//Bit 1: If set, the corresponding x - coordinate is 1 byte long.If not set, it is two bytes long.For the sign of this value, see the description of the X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR flag.
#define SIMPLE_GLYPH_ON_Y_SHORT_VECTOR	0x04	//Bit 2 : If set, the corresponding y - coordinate is 1 byte long.If not set, it is two bytes long.For the sign of this value, see the description of the Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR flag.
#define SIMPLE_GLYPH_ON_REPEAT_FLAG		0x08	//Bit 3 : If set, the next byte(read as unsigned) specifies the number of additional times this flag byte is to be repeated in the logical flags array — that is, the number of additional logical flag entries inserted after this entry. (In the expanded logical array, this bit is ignored.) In this way, the number of flags listed can be smaller than the number of points in the glyph description.
#define SIMPLE_GLYPH_ON_X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR	0x10	//Bit 4 : This flag has two meanings, depending on how the X_SHORT_VECTOR flag is set.If X_SHORT_VECTOR is set, this bit describes the sign of the value, with 1 equalling positive and 0 negative.If X_SHORT_VECTOR is not set and this bit is set, then the current x - coordinate is the same as the previous x - coordinate.If X_SHORT_VECTOR is not set and this bit is also not set, the current x - coordinate is a signed 16 - bit delta vector.
#define SIMPLE_GLYPH_ON_Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR	0x20	//Bit 5 : This flag has two meanings, depending on how the Y_SHORT_VECTOR flag is set.If Y_SHORT_VECTOR is set, this bit describes the sign of the value, with 1 equalling positive and 0 negative.If Y_SHORT_VECTOR is not set and this bit is set, then the current y - coordinate is the same as the previous y - coordinate.If Y_SHORT_VECTOR is not set and this bit is also not set, the current y - coordinate is a signed 16 - bit delta vector.
#define SIMPLE_GLYPH_ON_OVERLAP_SIMPLE	0x40	//Bit 6 : If set, contours in the glyph description may overlap.Use of this flag is not required in OpenType — that is, it is valid to have contours overlap without having this flag set.It may affect behaviors in some platforms, however. (See the discussion of “Overlapping contours” in Apple’s specification for details regarding behavior in Apple platforms.) When used, it must be set on the first flag byte for the glyph.See additional details below.


typedef struct _ttf_simple_glyph_contour_t{
	sword_t* pEndPtsOfContours;		// [numberOfContours]	Array of point indices for the last point of each contour, in increasing numeric order.
	sword_t	instructionLength;		// Total number of bytes for instructions.If instructionLength is zero, no instructions are present for this glyph, and this field is followed directly by the flags field.
	byte_t*	pInstructions;			// [instructionLength]	Array of instruction byte code for the glyph.
	byte_t*	pFlags;					// [variable]	Array of flag elements
	int*	pxCoordinates;			// [variable]	Contour point x - coordinates for the first point is relative to(0, 0); others are relative to previous point.
	int*	pyCoordinates;			// [variable]	Contour point y - coordinateS for the first point is relative to(0, 0); others are relative to previous point.
}ttf_simple_glyph_contour_t;

#define COMPOSITE_GLYPH_ARG_1_AND_2_ARE_WORDS		0x0001		//Bit 0: If this is set, the arguments are 16 - bit(uint16 or int16); otherwise, they are bytes(uint8 or int8).
#define COMPOSITE_GLYPH_ARGS_ARE_XY_VALUES			0x0002		//Bit 1: If this is set, the arguments are signed xy values; otherwise, they are unsigned point numbers.
#define COMPOSITE_GLYPH_ROUND_XY_TO_GRID			0x0004		//Bit 2: For the xy values if the preceding is true.
#define COMPOSITE_GLYPH_WE_HAVE_A_SCALE				0x0008		//Bit 3 : This indicates that there is a simple scale for the component.Otherwise, scale = 1.0.
#define COMPOSITE_GLYPH_MORE_COMPONENTS				0x0020		//Bit 5 : Indicates at least one more glyph after this one.
#define COMPOSITE_GLYPH_WE_HAVE_AN_X_AND_Y_SCALE	0x0040		//Bit 6 : The x direction will use a different scale from the y direction.
#define COMPOSITE_GLYPH_WE_HAVE_A_TWO_BY_TWO		0x0080		//Bit 7 : There is a 2 by 2 transformation that will be used to scale the component.
#define COMPOSITE_GLYPH_WE_HAVE_INSTRUCTIONS		0x0100		//Bit 8 : Following the last component are instructions for the composite character.
#define COMPOSITE_GLYPH_USE_MY_METRICS				0x0200		//Bit 9 : If set, this forces the aw and lsb(and rsb) for the composite to be equal to those from this original glyph.This works for hinted and unhinted characters.
#define COMPOSITE_GLYPH_OVERLAP_COMPOUND			0x0400		//Bit 10 : If set, the components of the compound glyph overlap.Use of this flag is not required in OpenType — that is, it is valid to have components overlap without having this flag set.It may affect behaviors in some platforms, however. (See Apple’s specification for details regarding behavior in Apple platforms.) When used, it must be set on the flag word for the first component.See additional remarks, above, for the similar OVERLAP_SIMPLE flag used in simple - glyph descriptions.
#define COMPOSITE_GLYPH_SCALED_COMPONENT_OFFSET		0x0800		//Bit 11 : The composite is designed to have the component offset scaled.
#define COMPOSITE_GLYPH_UNSCALED_COMPONENT_OFFSET	0x1000		//Bit 12 : The composite is designed not to have the component offset scaled.

typedef struct _ttf_composite_glyph_contour_t* ttf_composite_glyph_contour_ptr;
typedef struct _ttf_composite_glyph_contour_t{
	sword_t	flags;			//component flag
	sword_t	glyphIndex;		//glyph index of component
	int		argument1;		//x - offset for component or point number; type depends on bits 0 and 1 in component flags
	int		argument2;		//y - offset for component or point number; type depends on bits 0 and 1 in component flags

	int xx;
	int yy;
	int xy;
	int yx;
	ttf_composite_glyph_contour_ptr next;
}ttf_composite_glyph_contour_t;

typedef struct _ttf_glyf_table_t{
	short	numberOfContours;	//If the number of contours is greater than or equal to zero, this is a simple glyph.If negative, this is a composite glyph — the value - 1 should be used for composite glyphs.
	short	xMin;				//Minimum x for coordinate data.
	short	yMin;				//Minimum y for coordinate data.
	short	xMax;				//Maximum x for coordinate data.
	short	yMax;				//Maximum y for coordinate data.

	bool_t isSimpleContour;
	union{
		ttf_simple_glyph_contour_t* simpleContour;
		ttf_composite_glyph_contour_t* compositeContour;
	};
}ttf_glyf_table_t;

#ifdef __cplusplus
extern "C" {
#endif

	EXP_API bool_t ttf_load_file_head(ttf_file_head_t* pfile, const byte_t* buf, dword_t size);

	EXP_API void ttf_clear_file_head(ttf_file_head_t* pfile);

	EXP_API bool_t ttf_load_head_table(const ttf_file_head_t* pfile, ttf_head_table_t* phead, const byte_t* buf, dword_t size);

	EXP_API void ttf_clear_head_table(ttf_head_table_t* phead);

	EXP_API bool_t ttf_load_name_table(const ttf_file_head_t* pfile, ttf_name_table_t* pname, const byte_t* buf, dword_t size);

	EXP_API void ttf_clear_name_table(ttf_name_table_t* pname);

	EXP_API bool_t ttf_load_cmap_table(const ttf_file_head_t* pfile, ttf_cmap_table_t* pcmap, const byte_t* buf, dword_t size);

	EXP_API void ttf_clear_cmap_table(ttf_cmap_table_t* pcmap);

	EXP_API bool_t ttf_load_maxp_table(const ttf_file_head_t* pfile, ttf_maxp_table_t* pmaxp, const byte_t* buf, dword_t size);

	EXP_API void ttf_clear_maxp_table(ttf_maxp_table_t* pmaxp);

	EXP_API bool_t ttf_load_loca_table(const ttf_file_head_t* pfile, int localFormat, int numGlyphs, ttf_loca_table_t* ploca, const byte_t* buf, dword_t size);

	EXP_API void ttf_clear_loca_table(ttf_loca_table_t* ploca);

	EXP_API bool_t ttf_load_glyf_table(const ttf_file_head_t* pfile, const ttf_loca_table_t* ploca, ttf_glyf_table_t* pglyf, int numGlyphs, const byte_t* buf, dword_t size);

	EXP_API void ttf_clear_glyf_table(ttf_glyf_table_t* pglyf);

#ifdef __cplusplus
}
#endif

#endif /*_FNTDEF_H*/
