/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc character defination document

	@module	chardef.h | interface file

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


#ifndef _CHRDEF_H
#define	_CHRDEF_H


#define IS_CONTROL_CHAR(ch)		(((int)ch >= 0 && (int)ch <= 31) || (int)ch == 127)
#define IS_NUM_FEED(ch)	 ((ch == _T(' ') || ch == _T('\t'))? 1 : 0)

/*define options string token item feed and line feed*/
#define OPT_ITEMFEED		_T('~')
#define OPT_LINEFEED		_T(';')
#define CSS_ITEMFEED		_T(':')
#define CSS_LINEFEED		_T(';')
#define TXT_ITEMFEED		_T('\t')
#define TXT_LINEFEED		_T('\r')
#define CSV_ITEMFEED		_T(',')
#define CSV_LINEFEED		_T('\n')
#define URL_ITEMFEED		_T('=')
#define URL_LINEFEED		_T('&')


/* text transfer control byte */
#define TEXT_SOH			0x01 //start of headline
#define TEXT_STX			0x02 //start of text
#define TEXT_ETX			0x03 //end of text
#define TEXT_FS				0x1c //file separator
#define TEXT_GS				0x1d //group separator
#define TEXT_RS				0x1e //record separator
#define TEXT_US				0x1f //unit separator


/*define max encode bytes*/
#ifdef _UNICODE
#define CHS_LEN		1
#else
#if DEF_MBS == _GB2312
#define CHS_LEN		2
#else
#define CHS_LEN		3
#endif
#endif

#define UTF_LEN		3

/*define some characters length*/
#define INT_LEN			16
#define NUM_LEN			48 
#define DATE_LEN		48
#define UTC_LEN			24
#define CLR_LEN			24
#define RES_LEN			64
#define KEY_LEN			128
#define BLK_LEN			32
#define META_LEN		256
#define UUID_LEN		36
#define PATH_LEN		1024
#define STYLE_LEN		2048
#define CSS_LEN			4096
#define MD5_LEN			32
#define HMAC_LEN		160
#define ETAG_LEN		64
#define ADDR_LEN		18
#define MAC_LEN			17
#define ERR_LEN			512

/*define some bytes size*/
#define MD5_SIZE		16
#define SHA1_SIZE		20
#define SHA2_SIZE		32
#define SHA4_SIZE		64
#define AES_IV_SIZE		16
#define AES_KEY_128		16
#define AES_KEY_256		32
#define DES_IV_SIZE		8
#define DES_KEY_112		16
#define DES_KEY_168		24
#define RC4_KEY_256		256
#define X509_CERT_SIZE	8192
#define RSA_KEY_SIZE	4096

#endif	/* _CHRDEF_H */

