
#ifndef _BARCODE_H
#define	_BARCODE_H

#include "../xdkdef.h"

#ifdef	__cplusplus
extern "C" {
#endif

	EXP_API dword_t pdf417_encode(const byte_t* token, dword_t len, byte_t* buf, dword_t max, int* prows, int* pcols);

	EXP_API dword_t code128_encode(const byte_t* token, dword_t len, byte_t* buf, dword_t max);

#ifdef	__cplusplus
}
#endif


#endif	/*OEMBRC_H */

