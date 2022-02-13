#ifndef CRC_H
#define CRC_H

#include "../xdkdef.h"

#ifdef __cplusplus
extern "C" {
#endif

	EXP_API sword_t crc16(const byte_t *s, sword_t l);

	EXP_API dword_t crc32(dword_t crc, const byte_t *s, dword_t l);

	EXP_API lword_t crc64(lword_t crc, const byte_t *s, lword_t l);

#ifdef __cplusplus
}
#endif


#endif

