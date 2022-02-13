/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc zip document

	@module	zip.c | implement file

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
#include "zlibexp.h"

#include "zlib.h"


bool_t xzlib_compress_bytes(const byte_t* src_buf, dword_t src_len, byte_t* zip_buf, dword_t* zip_len)
{
	return (Z_OK == compress(zip_buf, zip_len, src_buf, src_len)) ? 1 : 0;
}

bool_t xzlib_uncompress_bytes(const byte_t* zip_buf, dword_t zip_len, byte_t* dst_buf, dword_t* dst_len)
{
	return (Z_OK == uncompress(dst_buf, dst_len, zip_buf, zip_len)) ? 1 : 0;
}

bool_t xgzip_compress_bytes(const byte_t* src_buf, dword_t src_len, byte_t* zip_buf, dword_t* zip_len)
{
	return (Z_OK == gcompress(zip_buf, zip_len, src_buf, src_len)) ? 1 : 0;
}

bool_t xgzip_uncompress_bytes(const byte_t* zip_buf, dword_t zip_len, byte_t* dst_buf, dword_t* dst_len)
{
	return (Z_OK == guncompress(dst_buf, dst_len, zip_buf, zip_len)) ? 1 : 0;
}
