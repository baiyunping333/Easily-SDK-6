/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc text bio document

	@module	textbio.h | interface file

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

#ifndef _TEXTBIO_H
#define _TEXTBIO_H

#include "xdldef.h"

#if defined(XDL_SUPPORT_BIO)


#ifdef	__cplusplus
extern "C" {
#endif

	/*
	@FUNCTION load_bytes_from_file: load a bytes content from binary file.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@OUTPUT byte_t* buf: the bytes buffer.
	@INPUT dword_t max: the bytes buffer size in characters.
	@RETURN int: if succeeds return the bytes size in bytes, fails return zero.
	*/
	EXP_API dword_t load_bytes_from_file(const secu_desc_t* psd, const tchar_t* fname, byte_t* buf, dword_t max);

	/*
	@FUNCTION save_bytes_to_file: save a bytes content to binary file using default encoding.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@INPUT const byte_t* buf: the bytes buffer.
	@INPUT dword_t len: the bytes buffer size in characters.
	@RETURN int: if succeeds return the bytes size in bytes, fails return zero.
	*/
	EXP_API dword_t save_bytes_to_file(const secu_desc_t* psd, const tchar_t* fname, const byte_t* buf, dword_t len);

	/*
	@FUNCTION load_text_from_file: load a text content from binary file.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@OUTPUT tchar_t* buf: the text buffer.
	@INPUT int max: the text buffer size in characters.
	@RETURN int: if succeeds return the text size in characters, fails return C_ERR(-1).
	*/
	EXP_API int load_text_from_file(const secu_desc_t* psd, const tchar_t* fname, tchar_t* buf, int max);

	/*
	@FUNCTION save_text_to_file: save a text content to binary file using default encoding.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@INPUT const tchar_t* buf: the text buffer.
	@INPUT int len: the text buffer size in characters.
	@RETURN int: if succeeds return the text size in characters, fails return C_ERR(-1).
	*/
	EXP_API int save_text_to_file(const secu_desc_t* psd, const tchar_t* fname, const tchar_t* buf, int len);

	/*
	@FUNCTION load_memo_from_text_file: load memo document from text file.
	@INPUT link_t_ptr ptr: the meta link component.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@RETURN bool_t: if succeeds return nonzero, fails return zero.
	*/
	EXP_API bool_t load_memo_from_text_file(link_t_ptr ptr, const secu_desc_t* psd, const tchar_t* fname);

	/*
	@FUNCTION save_memo_to_text_file: save memo document as text file.
	@INPUT link_t_ptr ptr: the meta link component.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@RETURN bool_t: if succeeds return nonzero, fails return zero.
	*/
	EXP_API bool_t save_memo_to_text_file(link_t_ptr ptr, const secu_desc_t* psd, const tchar_t* fname);

	/*
	@FUNCTION load_tag_from_text_file: load tag document from text file.
	@INPUT link_t_ptr ptr: the tag link component.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@RETURN bool_t: if succeeds return nonzero, fails return zero.
	*/
	EXP_API bool_t load_tag_from_text_file(link_t_ptr ptr, const secu_desc_t* psd, const tchar_t* fname);

	/*
	@FUNCTION save_tag_to_text_file: save tag document as text file.
	@INPUT link_t_ptr ptr: the tag link component.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@RETURN bool_t: if succeeds return nonzero, fails return zero.
	*/
	EXP_API bool_t save_tag_to_text_file(link_t_ptr ptr, const secu_desc_t* psd, const tchar_t* fname);

	/*
	@FUNCTION load_json_from_text_file: load json document from text file.
	@INPUT link_t_ptr ptr: the json link component.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@RETURN bool_t: if succeeds return nonzero, fails return zero.
	*/
	EXP_API bool_t load_json_from_text_file(link_t_ptr ptr, const secu_desc_t* psd, const tchar_t* fname);

	/*
	@FUNCTION save_json_to_text_file: save json document as text file.
	@INPUT link_t_ptr ptr: the json link component.
	@INPUT const secu_desc_t* psd: the file security struct.
	@INPUT const tchar_t* fname: the destination file path name.
	@RETURN bool_t: if succeeds return nonzero, fails return zero.
	*/
	EXP_API bool_t save_json_to_text_file(link_t_ptr ptr, const secu_desc_t* psd, const tchar_t* fname);

#ifdef	__cplusplus
}
#endif

#endif /*XDL_SUPPORT_DOC*/

#endif /*TEXTBIO_H*/