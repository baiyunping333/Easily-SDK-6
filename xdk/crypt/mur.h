/**
* \file aes.h
*/
#ifndef MUR_H
#define MUR_H

#include "../xdkdef.h"

dword_t MurmurHash3_x86_32(const void * key, int len, dword_t seed);
void MurmurHash3_x86_128(const void * key, int len, dword_t seed, void * out);
void MurmurHash3_x64_128(const void * key, int len, dword_t seed, void * out);

#ifdef __cplusplus
extern "C" {
#endif

	EXP_API void murhash32(const byte_t *in, dword_t inlen, byte_t out[4]);

	EXP_API void murhash128(const byte_t *in, dword_t inlen, byte_t out[16]);

#ifdef __cplusplus
}
#endif

#endif /* oemmur.h */

