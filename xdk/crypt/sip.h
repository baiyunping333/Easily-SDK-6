/**
* \file aes.h
*/
#ifndef SIP_H
#define SIP_H

#include "../xdkdef.h"

lword_t siphash(const byte_t *in, dword_t inlen, const byte_t *k);

lword_t siphash_nocase(const byte_t *in, dword_t inlen, const byte_t *k);

#ifdef __cplusplus
extern "C" {
#endif

	EXP_API void siphash64(const byte_t *in, dword_t inlen, byte_t out[8]);

#ifdef __cplusplus
}
#endif

#endif /* oemsip.h */

