/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc ssl premaster document

	@module	sslprf.h | interface file

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

#ifndef _SSLPRF_H
#define _SSLPRF_H

#include "../xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

	/*for SSL3.0*/
	EXP_API void ssl_prf0(byte_t *secret, int slen, byte_t *random, int rlen, byte_t *dstbuf, int dlen);

	/*for TLS1.0 TLS1.1*/
	EXP_API void ssl_prf1(byte_t *secret, int slen, char *label, byte_t *random, int rlen, byte_t *dstbuf, int dlen);

	/*for TLS1.2*/
	EXP_API void ssl_prf2(byte_t *secret, int slen, char *label, byte_t *random, int rlen, byte_t *dstbuf, int dlen);

	/*for TLS1.3*/
	EXP_API void ssl_extract(int md_alg, const byte_t *ikm, int ilen, const byte_t *salt, int slen, byte_t *prk, int* plen);
	EXP_API void ssl_expand(int md_alg, const byte_t* prk, int klen, const schar_t* label, const byte_t* hash, int hlen, byte_t* okm, int olen);

#ifdef	__cplusplus
}
#endif

#endif /*XDNINET_H*/