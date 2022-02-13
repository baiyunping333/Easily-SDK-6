/*
*  Multi-precision integer library
*
*  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
*  SPDX-License-Identifier: Apache-2.0
*
*  Licensed under the Apache License, Version 2.0 (the "License"); you may
*  not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*  http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
*  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*  This file is part of mbed TLS (https://tls.mbed.org)
*/

/*
*  The following sources were referenced in the design of this Multi-precision
*  Integer library:
*
*  [1] Handbook of Applied Cryptography - 1997
*      Menezes, van Oorschot and Vanstone
*
*  [2] Multi-Precision Math
*      Tom St Denis
*      https://github.com/libtom/libtommath/blob/develop/tommath.pdf
*
*  [3] GNU Multi-Precision Arithmetic Library
*      https://gmplib.org/manual/index.html
*
*/

#include "mpi.h"

#include "../xdkimp.h"

#define ciL    (sizeof(mpi_uint))         /* chars in limb  */
#define biL    (ciL << 3)               /* bits  in limb  */
#define biH    (ciL << 2)               /* half limb size */

#define MPI_SIZE_T_MAX  ( (dword_t) -1 ) /* SIZE_T_MAX is not standard */

/*
* Convert between bits/chars and number of limbs
* Divide first in order to avoid potential overflows
*/
#define BITS_TO_LIMBS(i)  ( (i) / biL + ( (i) % biL != 0 ) )
#define CHARS_TO_LIMBS(i) ( (i) / ciL + ( (i) % ciL != 0 ) )

/* Implementation that should never be optimized out by the compiler */
static void mpi_zeroize(mpi_uint *v, dword_t n)
{
	xmem_zero(v, ciL * n);
}

/*
* Initialize one MPI
*/
void mpi_init(mpi *X)
{
	XDK_ASSERT(X != NULL);

	X->s = 1;
	X->n = 0;
	X->p = NULL;
}

/*
* Unallocate one MPI
*/
void mpi_free(mpi *X)
{
	if (X == NULL)
		return;

	if (X->p != NULL)
	{
		mpi_zeroize(X->p, X->n);
		xmem_free(X->p);
	}

	X->s = 1;
	X->n = 0;
	X->p = NULL;
}

/*
* Enlarge to the specified number of limbs
*/
int mpi_grow(mpi *X, dword_t nblimbs)
{
	mpi_uint *p;
	XDK_ASSERT(X != NULL);

	if (nblimbs > MPI_MAX_LIMBS)
	{
		set_last_error(_T("mpi_grow"), _T("ERR_MPI_ALLOC_FAILED"), -1);
		return C_ERR;
	}

	if (X->n < nblimbs)
	{
		if ((p = (mpi_uint*)xmem_alloc(nblimbs * ciL)) == NULL)
		{
			set_last_error(_T("mpi_grow"), _T("ERR_MPI_ALLOC_FAILED"), -1);
			return C_ERR;
		}

		if (X->p != NULL)
		{
			xmem_copy(p, X->p, X->n * ciL);
			mpi_zeroize(X->p, X->n);
			xmem_free(X->p);
		}

		X->n = nblimbs;
		X->p = p;
	}

	return(0);
}

/*
* Resize down as much as possible,
* while keeping at least the specified number of limbs
*/
int mpi_shrink(mpi *X, dword_t nblimbs)
{
	mpi_uint *p;
	dword_t i;
	XDK_ASSERT(X != NULL);

	if (nblimbs > MPI_MAX_LIMBS)
	{
		set_last_error(_T("mpi_shrink"), _T("ERR_MPI_ALLOC_FAILED"), -1);
		return C_ERR;
	}

	/* Actually resize up if there are currently fewer than nblimbs limbs. */
	if (X->n <= nblimbs)
		return(mpi_grow(X, nblimbs));
	/* After this point, then X->n > nblimbs and in particular X->n > 0. */

	for (i = X->n - 1; i > 0; i--)
		if (X->p[i] != 0)
			break;
	i++;

	if (i < nblimbs)
		i = nblimbs;

	if ((p = (mpi_uint*)xmem_alloc(i * ciL)) == NULL)
	{
		set_last_error(_T("mpi_shrink"), _T("ERR_MPI_ALLOC_FAILED"), -1);
		return C_ERR;
	}

	if (X->p != NULL)
	{
		xmem_copy(p, X->p, i * ciL);
		mpi_zeroize(X->p, X->n);
		xmem_free(X->p);
	}

	X->n = i;
	X->p = p;

	return(0);
}

/*
* Copy the contents of Y into X
*/
int mpi_copy(mpi *X, const mpi *Y)
{
	int ret = 0;
	dword_t i;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(Y != NULL);

	if (X == Y)
		return(0);

	if (Y->n == 0)
	{
		mpi_free(X);
		return(0);
	}

	for (i = Y->n - 1; i > 0; i--)
		if (Y->p[i] != 0)
			break;
	i++;

	X->s = Y->s;

	if (X->n < i)
	{
		MPI_CHK(mpi_grow(X, i));
	}
	else
	{
		xmem_zero(X->p + i, (X->n - i) * ciL);
	}

	xmem_copy(X->p, Y->p, i * ciL);

cleanup:

	return(ret);
}

/*
* Swap the contents of X and Y
*/
void mpi_swap(mpi *X, mpi *Y)
{
	mpi T;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(Y != NULL);

	xmem_copy(&T, X, sizeof(mpi));
	xmem_copy(X, Y, sizeof(mpi));
	xmem_copy(Y, &T, sizeof(mpi));
}

/*
* Conditionally assign X = Y, without leaking information
* about whether the assignment was made or not.
* (Leaking information about the respective sizes of X and Y is ok however.)
*/
int mpi_safe_cond_assign(mpi *X, const mpi *Y, byte_t assign)
{
	int ret = 0;
	dword_t i;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(Y != NULL);

	/* make sure assign is 0 or 1 in a time-constant manner */
	assign = (assign | (byte_t)-assign) >> 7;

	MPI_CHK(mpi_grow(X, Y->n));

	X->s = X->s * (1 - assign) + Y->s * assign;

	for (i = 0; i < Y->n; i++)
		X->p[i] = X->p[i] * (1 - assign) + Y->p[i] * assign;

	for (; i < X->n; i++)
		X->p[i] *= (1 - assign);

cleanup:
	return(ret);
}

/*
* Conditionally swap X and Y, without leaking information
* about whether the swap was made or not.
* Here it is not ok to simply swap the pointers, which whould lead to
* different memory access patterns when X and Y are used afterwards.
*/
int mpi_safe_cond_swap(mpi *X, mpi *Y, byte_t swap)
{
	int ret, s;
	dword_t i;
	mpi_uint tmp;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(Y != NULL);

	if (X == Y)
		return(0);

	/* make sure swap is 0 or 1 in a time-constant manner */
	swap = (swap | (byte_t)-swap) >> 7;

	MPI_CHK(mpi_grow(X, Y->n));
	MPI_CHK(mpi_grow(Y, X->n));

	s = X->s;
	X->s = X->s * (1 - swap) + Y->s * swap;
	Y->s = Y->s * (1 - swap) + s * swap;


	for (i = 0; i < X->n; i++)
	{
		tmp = X->p[i];
		X->p[i] = X->p[i] * (1 - swap) + Y->p[i] * swap;
		Y->p[i] = Y->p[i] * (1 - swap) + tmp * swap;
	}

cleanup:
	return(ret);
}

/*
* Set value from integer
*/
int mpi_lset(mpi *X, mpi_sint z)
{
	int ret;
	XDK_ASSERT(X != NULL);

	MPI_CHK(mpi_grow(X, 1));
	xmem_zero(X->p, X->n * ciL);

	X->p[0] = (z < 0) ? -z : z;
	X->s = (z < 0) ? -1 : 1;

cleanup:

	return(ret);
}

/*
* Get a specific bit
*/
int mpi_get_bit(const mpi *X, dword_t pos)
{
	XDK_ASSERT(X != NULL);

	if (X->n * biL <= pos)
		return(0);

	return((X->p[pos / biL] >> (pos % biL)) & 0x01);
}

/* Get a specific byte, without range checks. */
#define MPI_GET_BYTE( X, i )                                \
    ( ( ( X )->p[( i ) / ciL] >> ( ( ( i ) % ciL ) * 8 ) ) & 0xff )

/*
* Set a bit to a specific value of 0 or 1
*/
int mpi_set_bit(mpi *X, dword_t pos, byte_t val)
{
	int ret = 0;
	dword_t off = pos / biL;
	dword_t idx = pos % biL;
	XDK_ASSERT(X != NULL);

	if (val != 0 && val != 1)
	{
		set_last_error(_T("mpi_set_bit"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (X->n * biL <= pos)
	{
		if (val == 0)
			return(0);

		MPI_CHK(mpi_grow(X, off + 1));
	}

	X->p[off] &= ~((mpi_uint)0x01 << idx);
	X->p[off] |= (mpi_uint)val << idx;

cleanup:

	return(ret);
}

/*
* Return the number of less significant zero-bits
*/
dword_t mpi_lsb(const mpi *X)
{
	dword_t i, j, count = 0;

	XDK_ASSERT(X != NULL);

	for (i = 0; i < X->n; i++)
		for (j = 0; j < biL; j++, count++)
			if (((X->p[i] >> j) & 1) != 0)
				return(count);

	return(0);
}

/*
* Return the number of most significant bits
*/
dword_t mpi_msb(const mpi *X)
{
	dword_t i, j;

	for (i = X->n - 1; i > 0; i--)
		if (X->p[i] != 0)
			break;

	for (j = biL - 1; j >= 0; j--)
		if (((X->p[i] >> j) & 1) != 0)
			break;

	return((i * biL) + j + 1);
}

/*
* Count leading zero bits in a given integer
*/
static dword_t clz(const mpi_uint x)
{
	dword_t j;
	mpi_uint mask = (mpi_uint)1 << (biL - 1);

	for (j = 0; j < biL; j++)
	{
		if (x & mask) break;

		mask >>= 1;
	}

	return j;
}

/*
* Return the number of bits
*/
dword_t mpi_bitlen(const mpi *X)
{
	dword_t i, j;

	if (X->n == 0)
		return(0);

	for (i = X->n - 1; i > 0; i--)
		if (X->p[i] != 0)
			break;

	j = biL - clz(X->p[i]);

	return((i * biL) + j);
}

/*
* Return the total size in bytes
*/
dword_t mpi_size(const mpi *X)
{
	return((mpi_bitlen(X) + 7) >> 3);
}

/*
* Convert an ASCII character to digit value
*/
static int mpi_get_digit(mpi_uint *d, int radix, char c)
{
	*d = 255;

	if (c >= 0x30 && c <= 0x39) *d = c - 0x30;
	if (c >= 0x41 && c <= 0x46) *d = c - 0x37;
	if (c >= 0x61 && c <= 0x66) *d = c - 0x57;

	if (*d >= (mpi_uint)radix)
	{
		set_last_error(_T("mpi_get_digit"), _T("ERR_MPI_INVALID_CHARACTER"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Import from an ASCII string
*/
int mpi_read_string(mpi *X, int radix, const char *s, int slen)
{
	int ret;
	dword_t i, j, n;
	mpi_uint d;
	mpi T;

	XDK_ASSERT(X != NULL);
	XDK_ASSERT(s != NULL);

	if (s == NULL)
	{
		set_last_error(_T("mpi_read_string"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (radix < 2 || radix > 16)
	{
		set_last_error(_T("mpi_read_string"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (slen < 0)
		slen = strlen(s);

	mpi_init(&T);

	if (radix == 16)
	{
		if (slen > MPI_SIZE_T_MAX >> 2)
		{
			set_last_error(_T("mpi_read_string"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
			return C_ERR;
		}

		n = BITS_TO_LIMBS(slen << 2);

		MPI_CHK(mpi_grow(X, n));
		MPI_CHK(mpi_lset(X, 0));

		for (i = slen, j = 0; i > 0; i--, j++)
		{
			if (i == 1 && s[i - 1] == '-')
			{
				X->s = -1;
				break;
			}

			MPI_CHK(mpi_get_digit(&d, radix, s[i - 1]));
			X->p[j / (2 * ciL)] |= d << ((j % (2 * ciL)) << 2);
		}
	}
	else
	{
		MPI_CHK(mpi_lset(X, 0));

		for (i = 0; i < slen; i++)
		{
			if (i == 0 && s[i] == '-')
			{
				X->s = -1;
				continue;
			}

			MPI_CHK(mpi_get_digit(&d, radix, s[i]));
			MPI_CHK(mpi_mul_int(&T, X, radix));

			if (X->s == 1)
			{
				MPI_CHK(mpi_add_int(X, &T, d));
			}
			else
			{
				MPI_CHK(mpi_sub_int(X, &T, d));
			}
		}
	}

cleanup:

	mpi_free(&T);

	return(ret);
}

/*
* Helper to write the digits high-order first.
*/
static int mpi_write_hlp(mpi *X, int radix,
	char **p, const dword_t buflen)
{
	int ret;
	mpi_uint r;
	dword_t length = 0;
	char *p_end = *p + buflen;

	do
	{
		if (length >= buflen)
		{
			set_last_error(_T("mpi_write_hlp"), _T("ERR_MPI_BUFFER_TOO_SMALL"), -1);
			return C_ERR;
		}

		MPI_CHK(mpi_mod_int(&r, X, radix));
		MPI_CHK(mpi_div_int(X, NULL, X, radix));
		/*
		* Write the residue in the current position, as an ASCII character.
		*/
		if (r < 0xA)
			*(--p_end) = (char)('0' + r);
		else
			*(--p_end) = (char)('A' + (r - 0xA));

		length++;
	} while (mpi_cmp_int(X, 0) != 0);

	memmove(*p, p_end, length);
	*p += length;

cleanup:

	return(ret);
}

/*
* Export into an ASCII string
*/
int mpi_write_string(const mpi *X, int radix,
	char *buf, dword_t buflen, dword_t *olen)
{
	int ret = 0;
	dword_t n;
	char *p;
	mpi T;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(olen != NULL);
	XDK_ASSERT(buflen == 0 || buf != NULL);

	if (radix < 2 || radix > 16)
	{
		set_last_error(_T("mpi_write_string"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	n = mpi_bitlen(X); /* Number of bits necessary to present `n`. */
	if (radix >= 4) n >>= 1;   /* Number of 4-adic digits necessary to present
							   * `n`. If radix > 4, this might be a strict
							   * overapproximation of the number of
							   * radix-adic digits needed to present `n`. */
	if (radix >= 16) n >>= 1;   /* Number of hexadecimal digits necessary to
								* present `n`. */

	n += 1; /* Terminating null byte */
	n += 1; /* Compensate for the divisions above, which round down `n`
			* in case it's not even. */
	n += 1; /* Potential '-'-sign. */
	n += (n & 1); /* Make n even to have enough space for hexadecimal writing,
				  * which always uses an even number of hex-digits. */

	if (buflen < n)
	{
		*olen = n;

		set_last_error(_T("mpi_write_string"), _T("ERR_MPI_BUFFER_TOO_SMALL"), -1);
		return C_ERR;
	}

	p = buf;
	mpi_init(&T);

	if (X->s == -1)
	{
		*p++ = '-';
		buflen--;
	}

	if (radix == 16)
	{
		int c;
		dword_t i, j, k;

		for (i = X->n, k = 0; i > 0; i--)
		{
			for (j = ciL; j > 0; j--)
			{
				c = (X->p[i - 1] >> ((j - 1) << 3)) & 0xFF;

				if (c == 0 && k == 0 && (i + j) != 2)
					continue;

				*(p++) = "0123456789ABCDEF"[c / 16];
				*(p++) = "0123456789ABCDEF"[c % 16];
				k = 1;
			}
		}
	}
	else
	{
		MPI_CHK(mpi_copy(&T, X));

		if (T.s == -1)
			T.s = 1;

		MPI_CHK(mpi_write_hlp(&T, radix, &p, buflen));
	}

	*p++ = '\0';
	*olen = p - buf;

cleanup:

	mpi_free(&T);

	return(ret);
}


/* Convert a big-endian byte array aligned to the size of mpi_uint
* into the storage form used by mpi. */

static mpi_uint mpi_uint_bigendian_to_host_c(mpi_uint x)
{
	byte_t i;
	byte_t *x_ptr;
	mpi_uint tmp = 0;

	for (i = 0, x_ptr = (byte_t*)&x; i < ciL; i++, x_ptr++)
	{
		tmp <<= CHAR_BIT;
		tmp |= (mpi_uint)*x_ptr;
	}

	return(tmp);
}

static mpi_uint mpi_uint_bigendian_to_host(mpi_uint x)
{
#if defined(__BYTE_ORDER__)

	/* Nothing to do on bigendian systems. */
#if ( __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ )
	return(x);
#endif /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */

#if ( __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ )

	/* For GCC and Clang, have builtins for byte swapping. */
#if defined(__GNUC__) && defined(__GNUC_PREREQ)
#if __GNUC_PREREQ(4,3)
#define have_bswap
#endif
#endif

#if defined(__clang__) && defined(__has_builtin)
#if __has_builtin(__builtin_bswap32)  &&                 \
    __has_builtin(__builtin_bswap64)
#define have_bswap
#endif
#endif

#if defined(have_bswap)
	/* The compiler is hopefully able to statically evaluate this! */
	switch (sizeof(mpi_uint))
	{
	case 4:
		return(__builtin_bswap32(x));
	case 8:
		return(__builtin_bswap64(x));
	}
#endif
#endif /* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
#endif /* __BYTE_ORDER__ */

	/* Fall back to C-based reordering if we don't know the byte order
	* or we couldn't use a compiler-specific builtin. */
	return(mpi_uint_bigendian_to_host_c(x));
}

static void mpi_bigendian_to_host(mpi_uint * const p, dword_t limbs)
{
	mpi_uint *cur_limb_left;
	mpi_uint *cur_limb_right;
	if (limbs == 0)
		return;

	/*
	* Traverse limbs and
	* - adapt byte-order in each limb
	* - swap the limbs themselves.
	* For that, simultaneously traverse the limbs from left to right
	* and from right to left, as long as the left index is not bigger
	* than the right index (it's not a problem if limbs is odd and the
	* indices coincide in the last iteration).
	*/
	for (cur_limb_left = p, cur_limb_right = p + (limbs - 1);
		cur_limb_left <= cur_limb_right;
		cur_limb_left++, cur_limb_right--)
	{
		mpi_uint tmp;
		/* Note that if cur_limb_left == cur_limb_right,
		* this code effectively swaps the bytes only once. */
		tmp = mpi_uint_bigendian_to_host(*cur_limb_left);
		*cur_limb_left = mpi_uint_bigendian_to_host(*cur_limb_right);
		*cur_limb_right = tmp;
	}
}

/* Resize X to have exactly n limbs and set it to 0. */
static int mpi_resize_clear(mpi *X, dword_t limbs)
{
	if (limbs == 0)
	{
		mpi_free(X);
		return(0);
	}
	else if (X->n == limbs)
	{
		xmem_zero(X->p, limbs * ciL);
		X->s = 1;
		return(0);
	}
	else
	{
		mpi_free(X);
		return(mpi_grow(X, limbs));
	}
}

/*
* Import X from unsigned binary data, little endian
*/
int mpi_read_binary_le(mpi *X,
	const unsigned char *buf, dword_t buflen)
{
	int ret;
	dword_t i;
	dword_t const limbs = CHARS_TO_LIMBS(buflen);

	/* Ensure that target MPI has exactly the necessary number of limbs */
	MPI_CHK(mpi_resize_clear(X, limbs));

	for (i = 0; i < buflen; i++)
		X->p[i / ciL] |= ((mpi_uint)buf[i]) << ((i % ciL) << 3);

cleanup:

	/*
	* This function is also used to import keys. However, wiping the buffers
	* upon failure is not necessary because failure only can happen before any
	* input is copied.
	*/
	return(ret);
}

/*
* Import X from unsigned binary data, big endian
*/
int mpi_read_binary(mpi *X, const byte_t *buf, dword_t buflen)
{
	int ret;
	dword_t const limbs = CHARS_TO_LIMBS(buflen);
	dword_t const overhead = (limbs * ciL) - buflen;
	byte_t *Xp;

	XDK_ASSERT(X != NULL);

	if (buf == NULL)
	{
		set_last_error(_T("mpi_read_binary"), _T("ERR_MPI_BAD_PARAMETERS"), -1);
		return C_ERR;
	}

	/* Ensure that target MPI has exactly the necessary number of limbs */
	if (X->n != limbs)
	{
		mpi_free(X);
		mpi_init(X);
		MPI_CHK(mpi_grow(X, limbs));
	}
	MPI_CHK(mpi_lset(X, 0));

	/* Avoid calling `xmem_copy` with NULL source argument,
	* even if buflen is 0. */
	if (buf != NULL)
	{
		Xp = (byte_t*)X->p;
		xmem_copy(Xp + overhead, buf, buflen);

		mpi_bigendian_to_host(X->p, limbs);
	}

cleanup:

	return(ret);
}

/*
*
* Export X into unsigned binary data, little endian
*/
int mpi_write_binary_le(const mpi *X,
unsigned char *buf, dword_t buflen)
{
	dword_t stored_bytes = X->n * ciL;
	dword_t bytes_to_copy;
	dword_t i;

	if (stored_bytes < buflen)
	{
		bytes_to_copy = stored_bytes;
	}
	else
	{
		bytes_to_copy = buflen;

		/* The output buffer is smaller than the allocated size of X.
		* However X may fit if its leading bytes are zero. */
		for (i = bytes_to_copy; i < stored_bytes; i++)
		{
			if (MPI_GET_BYTE(X, i) != 0)
			{
				set_last_error(_T("mpi_write_binary"), _T("MBEDTLS_ERR_MPI_BUFFER_TOO_SMALL"), -1);
				return C_ERR;
			}
		}
	}

	for (i = 0; i < bytes_to_copy; i++)
		buf[i] = MPI_GET_BYTE(X, i);

	if (stored_bytes < buflen)
	{
		/* Write trailing 0 bytes */
		memset(buf + stored_bytes, 0, buflen - stored_bytes);
	}

	return(0);
}

/*
* Export X into unsigned binary data, big endian
*/
int mpi_write_binary(const mpi *X,
	byte_t *buf, dword_t buflen)
{
	dword_t stored_bytes;
	dword_t bytes_to_copy;
	byte_t *p;
	dword_t i;

	XDK_ASSERT(X != NULL);

	if (buf == NULL)
	{
		set_last_error(_T("mpi_write_binary"), _T("ERR_MPI_BAD_PARAMETERS"), -1);
		return C_ERR;
	}

	stored_bytes = X->n * ciL;

	if (stored_bytes < buflen)
	{
		/* There is enough space in the output buffer. Write initial
		* null bytes and record the position at which to start
		* writing the significant bytes. In this case, the execution
		* trace of this function does not depend on the value of the
		* number. */
		bytes_to_copy = stored_bytes;
		p = buf + buflen - stored_bytes;
		xmem_zero(buf, buflen - stored_bytes);
	}
	else
	{
		/* The output buffer is smaller than the allocated size of X.
		* However X may fit if its leading bytes are zero. */
		bytes_to_copy = buflen;
		p = buf;
		for (i = bytes_to_copy; i < stored_bytes; i++)
		{
			if (MPI_GET_BYTE(X, i) != 0)
			{
				set_last_error(_T("mpi_write_binary"), _T("ERR_MPI_BUFFER_TOO_SMALL"), -1);
				return C_ERR;
			}
		}
	}

	for (i = 0; i < bytes_to_copy; i++)
		p[bytes_to_copy - i - 1] = MPI_GET_BYTE(X, i);

	return(0);
}

/*
* Left-shift: X <<= count
*/
int mpi_shift_l(mpi *X, dword_t count)
{
	int ret;
	dword_t i, v0, t1;
	mpi_uint r0 = 0, r1;
	XDK_ASSERT(X != NULL);

	v0 = count / (biL);
	t1 = count & (biL - 1);

	i = mpi_bitlen(X) + count;

	if (X->n * biL < i)
		MPI_CHK(mpi_grow(X, BITS_TO_LIMBS(i)));

	ret = 0;

	/*
	* shift by count / limb_size
	*/
	if (v0 > 0)
	{
		for (i = X->n; i > v0; i--)
			X->p[i - 1] = X->p[i - v0 - 1];

		for (; i > 0; i--)
			X->p[i - 1] = 0;
	}

	/*
	* shift by count % limb_size
	*/
	if (t1 > 0)
	{
		for (i = v0; i < X->n; i++)
		{
			r1 = X->p[i] >> (biL - t1);
			X->p[i] <<= t1;
			X->p[i] |= r0;
			r0 = r1;
		}
	}

cleanup:

	return(ret);
}

/*
* Right-shift: X >>= count
*/
int mpi_shift_r(mpi *X, dword_t count)
{
	dword_t i, v0, v1;
	mpi_uint r0 = 0, r1;
	XDK_ASSERT(X != NULL);

	v0 = count / biL;
	v1 = count & (biL - 1);

	if (v0 > X->n || (v0 == X->n && v1 > 0))
		return mpi_lset(X, 0);

	/*
	* shift by count / limb_size
	*/
	if (v0 > 0)
	{
		for (i = 0; i < X->n - v0; i++)
			X->p[i] = X->p[i + v0];

		for (; i < X->n; i++)
			X->p[i] = 0;
	}

	/*
	* shift by count % limb_size
	*/
	if (v1 > 0)
	{
		for (i = X->n; i > 0; i--)
		{
			r1 = X->p[i - 1] << (biL - v1);
			X->p[i - 1] >>= v1;
			X->p[i - 1] |= r0;
			r0 = r1;
		}
	}

	return(0);
}

/*
* Compare unsigned values
*/
int mpi_cmp_abs(const mpi *X, const mpi *Y)
{
	dword_t i, j;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(Y != NULL);

	for (i = X->n; i > 0; i--)
		if (X->p[i - 1] != 0)
			break;

	for (j = Y->n; j > 0; j--)
		if (Y->p[j - 1] != 0)
			break;

	if (i == 0 && j == 0)
		return(0);

	if (i > j) return(1);
	if (j > i) return(-1);

	for (; i > 0; i--)
	{
		if (X->p[i - 1] > Y->p[i - 1]) return(1);
		if (X->p[i - 1] < Y->p[i - 1]) return(-1);
	}

	return(0);
}

/*
* Compare signed values
*/
int mpi_cmp_mpi(const mpi *X, const mpi *Y)
{
	dword_t i, j;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(Y != NULL);

	for (i = X->n; i > 0; i--)
		if (X->p[i - 1] != 0)
			break;

	for (j = Y->n; j > 0; j--)
		if (Y->p[j - 1] != 0)
			break;

	if (i == 0 && j == 0)
		return(0);

	if (i > j) return(X->s);
	if (j > i) return(-Y->s);

	if (X->s > 0 && Y->s < 0) return(1);
	if (Y->s > 0 && X->s < 0) return(-1);

	for (; i > 0; i--)
	{
		if (X->p[i - 1] > Y->p[i - 1]) return(X->s);
		if (X->p[i - 1] < Y->p[i - 1]) return(-X->s);
	}

	return(0);
}

/** Decide if an integer is less than the other, without branches.
*
* \param x         First integer.
* \param y         Second integer.
*
* \return          1 if \p x is less than \p y, 0 otherwise
*/
static unsigned ct_lt_mpi_uint(const mpi_uint x,
	const mpi_uint y)
{
	mpi_uint ret;
	mpi_uint cond;

	/*
	* Check if the most significant bits (MSB) of the operands are different.
	*/
	cond = (x ^ y);
	/*
	* If the MSB are the same then the difference x-y will be negative (and
	* have its MSB set to 1 during conversion to unsigned) if and only if x<y.
	*/
	ret = (x - y) & ~cond;
	/*
	* If the MSB are different, then the operand with the MSB of 1 is the
	* bigger. (That is if y has MSB of 1, then x<y is true and it is false if
	* the MSB of y is 0.)
	*/
	ret |= y & cond;


	ret = ret >> (biL - 1);

	return (unsigned)ret;
}

/*
* Compare signed values in constant time
*/
int mpi_lt_mpi_ct(const mpi *X, const mpi *Y,
	unsigned *ret)
{
	dword_t i;
	/* The value of any of these variables is either 0 or 1 at all times. */
	unsigned cond, done, X_is_negative, Y_is_negative;

	XDK_ASSERT(X != NULL);
	XDK_ASSERT(Y != NULL);
	XDK_ASSERT(ret != NULL);

	if (X->n != Y->n)
	{
		set_last_error(_T("mpi_lt_mpi_ct"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* Set sign_N to 1 if N >= 0, 0 if N < 0.
	* We know that N->s == 1 if N >= 0 and N->s == -1 if N < 0.
	*/
	X_is_negative = (X->s & 2) >> 1;
	Y_is_negative = (Y->s & 2) >> 1;

	/*
	* If the signs are different, then the positive operand is the bigger.
	* That is if X is negative (X_is_negative == 1), then X < Y is true and it
	* is false if X is positive (X_is_negative == 0).
	*/
	cond = (X_is_negative ^ Y_is_negative);
	*ret = cond & X_is_negative;

	/*
	* This is a constant-time function. We might have the result, but we still
	* need to go through the loop. Record if we have the result already.
	*/
	done = cond;

	for (i = X->n; i > 0; i--)
	{
		/*
		* If Y->p[i - 1] < X->p[i - 1] then X < Y is true if and only if both
		* X and Y are negative.
		*
		* Again even if we can make a decision, we just mark the result and
		* the fact that we are done and continue looping.
		*/
		cond = ct_lt_mpi_uint(Y->p[i - 1], X->p[i - 1]);
		*ret |= cond & (1 - done) & X_is_negative;
		done |= cond;

		/*
		* If X->p[i - 1] < Y->p[i - 1] then X < Y is true if and only if both
		* X and Y are positive.
		*
		* Again even if we can make a decision, we just mark the result and
		* the fact that we are done and continue looping.
		*/
		cond = ct_lt_mpi_uint(X->p[i - 1], Y->p[i - 1]);
		*ret |= cond & (1 - done) & (1 - X_is_negative);
		done |= cond;
	}

	return(0);
}

/*
* Compare signed values
*/
int mpi_cmp_int(const mpi *X, mpi_sint z)
{
	mpi Y;
	mpi_uint p[1];
	XDK_ASSERT(X != NULL);

	*p = (z < 0) ? -z : z;
	Y.s = (z < 0) ? -1 : 1;
	Y.n = 1;
	Y.p = p;

	return(mpi_cmp_mpi(X, &Y));
}

/*
* Unsigned addition: X = |A| + |B|  (HAC 14.7)
*/
int mpi_add_abs(mpi *X, const mpi *A, const mpi *B)
{
	int ret;
	dword_t i, j;
	mpi_uint *o, *p, c, tmp;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(B != NULL);

	if (X == B)
	{
		const mpi *T = A; A = X; B = T;
	}

	if (X != A)
		MPI_CHK(mpi_copy(X, A));

	/*
	* X should always be positive as a result of unsigned additions.
	*/
	X->s = 1;

	for (j = B->n; j > 0; j--)
		if (B->p[j - 1] != 0)
			break;

	MPI_CHK(mpi_grow(X, j));

	o = B->p; p = X->p; c = 0;

	/*
	* tmp is used because it might happen that p == o
	*/
	for (i = 0; i < j; i++, o++, p++)
	{
		tmp = *o;
		*p += c; c = (*p <  c);
		*p += tmp; c += (*p < tmp);
	}

	while (c != 0)
	{
		if (i >= X->n)
		{
			MPI_CHK(mpi_grow(X, i + 1));
			p = X->p + i;
		}

		*p += c; c = (*p < c); i++; p++;
	}

cleanup:

	return(ret);
}

/*
* Helper for mpi subtraction
*/
static void mpi_sub_hlp(dword_t n, mpi_uint *s, mpi_uint *d)
{
	dword_t i;
	mpi_uint c, z;

	for (i = c = 0; i < n; i++, s++, d++)
	{
		z = (*d <  c);     *d -= c;
		c = (*d < *s) + z; *d -= *s;
	}

	while (c != 0)
	{
		z = (*d < c); *d -= c;
		c = z; d++;
	}
}

/*
* Unsigned subtraction: X = |A| - |B|  (HAC 14.9)
*/
int mpi_sub_abs(mpi *X, const mpi *A, const mpi *B)
{
	mpi TB;
	int ret;
	dword_t n;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(B != NULL);

	if (mpi_cmp_abs(A, B) < 0)
	{
		set_last_error(_T("mpi_sub_abs"), _T("ERR_MPI_NEGATIVE_VALUE"), -1);
		return C_ERR;
	}

	mpi_init(&TB);

	if (X == B)
	{
		MPI_CHK(mpi_copy(&TB, B));
		B = &TB;
	}

	if (X != A)
		MPI_CHK(mpi_copy(X, A));

	/*
	* X should always be positive as a result of unsigned subtractions.
	*/
	X->s = 1;

	ret = 0;

	for (n = B->n; n > 0; n--)
		if (B->p[n - 1] != 0)
			break;

	mpi_sub_hlp(n, B->p, X->p);

cleanup:

	mpi_free(&TB);

	return(ret);
}

/*
* Signed addition: X = A + B
*/
int mpi_add_mpi(mpi *X, const mpi *A, const mpi *B)
{
	int ret, s;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(B != NULL);

	s = A->s;
	if (A->s * B->s < 0)
	{
		if (mpi_cmp_abs(A, B) >= 0)
		{
			MPI_CHK(mpi_sub_abs(X, A, B));
			X->s = s;
		}
		else
		{
			MPI_CHK(mpi_sub_abs(X, B, A));
			X->s = -s;
		}
	}
	else
	{
		MPI_CHK(mpi_add_abs(X, A, B));
		X->s = s;
	}

cleanup:

	return(ret);
}

/*
* Signed subtraction: X = A - B
*/
int mpi_sub_mpi(mpi *X, const mpi *A, const mpi *B)
{
	int ret, s;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(B != NULL);

	s = A->s;
	if (A->s * B->s > 0)
	{
		if (mpi_cmp_abs(A, B) >= 0)
		{
			MPI_CHK(mpi_sub_abs(X, A, B));
			X->s = s;
		}
		else
		{
			MPI_CHK(mpi_sub_abs(X, B, A));
			X->s = -s;
		}
	}
	else
	{
		MPI_CHK(mpi_add_abs(X, A, B));
		X->s = s;
	}

cleanup:

	return(ret);
}

/*
* Signed addition: X = A + b
*/
int mpi_add_int(mpi *X, const mpi *A, mpi_sint b)
{
	mpi _B;
	mpi_uint p[1];
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);

	p[0] = (b < 0) ? -b : b;
	_B.s = (b < 0) ? -1 : 1;
	_B.n = 1;
	_B.p = p;

	return(mpi_add_mpi(X, A, &_B));
}

/*
* Signed subtraction: X = A - b
*/
int mpi_sub_int(mpi *X, const mpi *A, mpi_sint b)
{
	mpi _B;
	mpi_uint p[1];
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);

	p[0] = (b < 0) ? -b : b;
	_B.s = (b < 0) ? -1 : 1;
	_B.n = 1;
	_B.p = p;

	return(mpi_sub_mpi(X, A, &_B));
}

/*
* Helper for mpi multiplication
*/
static
#if defined(__APPLE__) && defined(__arm__)
/*
* Apple LLVM version 4.2 (clang-425.0.24) (based on LLVM 3.2svn)
* appears to need this to prevent bad ARM code generation at -O3.
*/
__attribute__((noinline))
#endif
void mpi_mul_hlp(dword_t i, mpi_uint *s, mpi_uint *d, mpi_uint b)
{
	mpi_uint c = 0, t = 0;

#if defined(MULADDC_HUIT)
	for (; i >= 8; i -= 8)
	{
		MULADDC_INIT
			MULADDC_HUIT
			MULADDC_STOP
	}

	for (; i > 0; i--)
	{
		MULADDC_INIT
			MULADDC_CORE
			MULADDC_STOP
	}
#else /* MULADDC_HUIT */
	for (; i >= 16; i -= 16)
	{
		MULADDC_INIT
			MULADDC_CORE   MULADDC_CORE
			MULADDC_CORE   MULADDC_CORE
			MULADDC_CORE   MULADDC_CORE
			MULADDC_CORE   MULADDC_CORE

			MULADDC_CORE   MULADDC_CORE
			MULADDC_CORE   MULADDC_CORE
			MULADDC_CORE   MULADDC_CORE
			MULADDC_CORE   MULADDC_CORE
			MULADDC_STOP
	}

	for (; i >= 8; i -= 8)
	{
		MULADDC_INIT
			MULADDC_CORE   MULADDC_CORE
			MULADDC_CORE   MULADDC_CORE

			MULADDC_CORE   MULADDC_CORE
			MULADDC_CORE   MULADDC_CORE
			MULADDC_STOP
	}

	for (; i > 0; i--)
	{
		MULADDC_INIT
			MULADDC_CORE
			MULADDC_STOP
	}
#endif /* MULADDC_HUIT */

	t++;

	do {
		*d += c; c = (*d < c); d++;
	} while (c != 0);
}

/*
* Baseline multiplication: X = A * B  (HAC 14.12)
*/
int mpi_mul_mpi(mpi *X, const mpi *A, const mpi *B)
{
	int ret;
	dword_t i, j;
	mpi TA, TB;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(B != NULL);

	mpi_init(&TA); mpi_init(&TB);

	if (X == A) { MPI_CHK(mpi_copy(&TA, A)); A = &TA; }
	if (X == B) { MPI_CHK(mpi_copy(&TB, B)); B = &TB; }

	for (i = A->n; i > 0; i--)
		if (A->p[i - 1] != 0)
			break;

	for (j = B->n; j > 0; j--)
		if (B->p[j - 1] != 0)
			break;

	MPI_CHK(mpi_grow(X, i + j));
	MPI_CHK(mpi_lset(X, 0));

	for (; j > 0; j--)
		mpi_mul_hlp(i, A->p, X->p + j - 1, B->p[j - 1]);

	X->s = A->s * B->s;

cleanup:

	mpi_free(&TB); mpi_free(&TA);

	return(ret);
}

/*
* Baseline multiplication: X = A * b
*/
int mpi_mul_int(mpi *X, const mpi *A, mpi_uint b)
{
	mpi _B;
	mpi_uint p[1];
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);

	_B.s = 1;
	_B.n = 1;
	_B.p = p;
	p[0] = b;

	return(mpi_mul_mpi(X, A, &_B));
}

/*
* Unsigned integer divide - double mpi_uint dividend, u1/u0, and
* mpi_uint divisor, d
*/
static mpi_uint int_div_int(mpi_uint u1,
	mpi_uint u0, mpi_uint d, mpi_uint *r)
{
#if defined(HAVE_UDBL)
	t_udbl dividend, quotient;
#else
	const mpi_uint radix = (mpi_uint)1 << biH;
	const mpi_uint uint_halfword_mask = ((mpi_uint)1 << biH) - 1;
	mpi_uint d0, d1, q0, q1, rAX, r0, quotient;
	mpi_uint u0_msw, u0_lsw;
	dword_t s;
#endif

	/*
	* Check for overflow
	*/
	if (0 == d || u1 >= d)
	{
		if (r != NULL) *r = ~0;

		return (~0);
	}

#if defined(HAVE_UDBL)
	dividend = (t_udbl)u1 << biL;
	dividend |= (t_udbl)u0;
	quotient = dividend / d;
	if (quotient > ((t_udbl)1 << biL) - 1)
		quotient = ((t_udbl)1 << biL) - 1;

	if (r != NULL)
		*r = (mpi_uint)(dividend - (quotient * d));

	return (mpi_uint)quotient;
#else

	/*
	* Algorithm D, Section 4.3.1 - The Art of Computer Programming
	*   Vol. 2 - Seminumerical Algorithms, Knuth
	*/

	/*
	* Normalize the divisor, d, and dividend, u0, u1
	*/
	s = clz(d);
	d = d << s;

	u1 = u1 << s;
	u1 |= (u0 >> (biL - s)) & (-(mpi_sint)s >> (biL - 1));
	u0 = u0 << s;

	d1 = d >> biH;
	d0 = d & uint_halfword_mask;

	u0_msw = u0 >> biH;
	u0_lsw = u0 & uint_halfword_mask;

	/*
	* Find the first quotient and remainder
	*/
	q1 = u1 / d1;
	r0 = u1 - d1 * q1;

	while (q1 >= radix || (q1 * d0 > radix * r0 + u0_msw))
	{
		q1 -= 1;
		r0 += d1;

		if (r0 >= radix) break;
	}

	rAX = (u1 * radix) + (u0_msw - q1 * d);
	q0 = rAX / d1;
	r0 = rAX - q0 * d1;

	while (q0 >= radix || (q0 * d0 > radix * r0 + u0_lsw))
	{
		q0 -= 1;
		r0 += d1;

		if (r0 >= radix) break;
	}

	if (r != NULL)
		*r = (rAX * radix + u0_lsw - q0 * d) >> s;

	quotient = q1 * radix + q0;

	return quotient;
#endif
}

/*
* Division by mpi: A = Q * B + R  (HAC 14.20)
*/
int mpi_div_mpi(mpi *Q, mpi *R, const mpi *A,
	const mpi *B)
{
	int ret;
	dword_t i, n, t, k;
	mpi X, Y, Z, T1, T2;
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(B != NULL);

	if (mpi_cmp_int(B, 0) == 0)
	{
		set_last_error(_T("mpi_div_mpi"), _T("ERR_MPI_DIVISION_BY_ZERO"), -1);
		return C_ERR;
	}

	mpi_init(&X); mpi_init(&Y); mpi_init(&Z);
	mpi_init(&T1); mpi_init(&T2);

	if (mpi_cmp_abs(A, B) < 0)
	{
		if (Q != NULL) MPI_CHK(mpi_lset(Q, 0));
		if (R != NULL) MPI_CHK(mpi_copy(R, A));
		return(0);
	}

	MPI_CHK(mpi_copy(&X, A));
	MPI_CHK(mpi_copy(&Y, B));
	X.s = Y.s = 1;

	MPI_CHK(mpi_grow(&Z, A->n + 2));
	MPI_CHK(mpi_lset(&Z, 0));
	MPI_CHK(mpi_grow(&T1, 2));
	MPI_CHK(mpi_grow(&T2, 3));

	k = mpi_bitlen(&Y) % biL;
	if (k < biL - 1)
	{
		k = biL - 1 - k;
		MPI_CHK(mpi_shift_l(&X, k));
		MPI_CHK(mpi_shift_l(&Y, k));
	}
	else k = 0;

	n = X.n - 1;
	t = Y.n - 1;
	MPI_CHK(mpi_shift_l(&Y, biL * (n - t)));

	while (mpi_cmp_mpi(&X, &Y) >= 0)
	{
		Z.p[n - t]++;
		MPI_CHK(mpi_sub_mpi(&X, &X, &Y));
	}
	MPI_CHK(mpi_shift_r(&Y, biL * (n - t)));

	for (i = n; i > t; i--)
	{
		if (X.p[i] >= Y.p[t])
			Z.p[i - t - 1] = ~0;
		else
		{
			Z.p[i - t - 1] = int_div_int(X.p[i], X.p[i - 1],
				Y.p[t], NULL);
		}

		Z.p[i - t - 1]++;
		do
		{
			Z.p[i - t - 1]--;

			MPI_CHK(mpi_lset(&T1, 0));
			T1.p[0] = (t < 1) ? 0 : Y.p[t - 1];
			T1.p[1] = Y.p[t];
			MPI_CHK(mpi_mul_int(&T1, &T1, Z.p[i - t - 1]));

			MPI_CHK(mpi_lset(&T2, 0));
			T2.p[0] = (i < 2) ? 0 : X.p[i - 2];
			T2.p[1] = (i < 1) ? 0 : X.p[i - 1];
			T2.p[2] = X.p[i];
		} while (mpi_cmp_mpi(&T1, &T2) > 0);

		MPI_CHK(mpi_mul_int(&T1, &Y, Z.p[i - t - 1]));
		MPI_CHK(mpi_shift_l(&T1, biL * (i - t - 1)));
		MPI_CHK(mpi_sub_mpi(&X, &X, &T1));

		if (mpi_cmp_int(&X, 0) < 0)
		{
			MPI_CHK(mpi_copy(&T1, &Y));
			MPI_CHK(mpi_shift_l(&T1, biL * (i - t - 1)));
			MPI_CHK(mpi_add_mpi(&X, &X, &T1));
			Z.p[i - t - 1]--;
		}
	}

	if (Q != NULL)
	{
		MPI_CHK(mpi_copy(Q, &Z));
		Q->s = A->s * B->s;
	}

	if (R != NULL)
	{
		MPI_CHK(mpi_shift_r(&X, k));
		X.s = A->s;
		MPI_CHK(mpi_copy(R, &X));

		if (mpi_cmp_int(R, 0) == 0)
			R->s = 1;
	}

cleanup:

	mpi_free(&X); mpi_free(&Y); mpi_free(&Z);
	mpi_free(&T1); mpi_free(&T2);

	return(ret);
}

/*
* Division by int: A = Q * b + R
*/
int mpi_div_int(mpi *Q, mpi *R,
	const mpi *A,
	mpi_sint b)
{
	mpi _B;
	mpi_uint p[1];
	XDK_ASSERT(A != NULL);

	p[0] = (b < 0) ? -b : b;
	_B.s = (b < 0) ? -1 : 1;
	_B.n = 1;
	_B.p = p;

	return(mpi_div_mpi(Q, R, A, &_B));
}

/*
* Modulo: R = A mod B
*/
int mpi_mod_mpi(mpi *R, const mpi *A, const mpi *B)
{
	int ret;
	XDK_ASSERT(R != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(B != NULL);

	if (mpi_cmp_int(B, 0) < 0)
	{
		set_last_error(_T("mpi_mod_mpi"), _T("ERR_MPI_NEGATIVE_VALUE"), -1);
		return C_ERR;
	}

	MPI_CHK(mpi_div_mpi(NULL, R, A, B));

	while (mpi_cmp_int(R, 0) < 0)
		MPI_CHK(mpi_add_mpi(R, R, B));

	while (mpi_cmp_mpi(R, B) >= 0)
		MPI_CHK(mpi_sub_mpi(R, R, B));

cleanup:

	return(ret);
}

/*
* Modulo: r = A mod b
*/
int mpi_mod_int(mpi_uint *r, const mpi *A, mpi_sint b)
{
	dword_t i;
	mpi_uint x, y, z;
	XDK_ASSERT(r != NULL);
	XDK_ASSERT(A != NULL);

	if (b == 0)
	{
		set_last_error(_T("mpi_mod_int"), _T("ERR_MPI_DIVISION_BY_ZERO"), -1);
		return C_ERR;
	}

	if (b < 0)
	{
		set_last_error(_T("mpi_mod_int"), _T("ERR_MPI_NEGATIVE_VALUE"), -1);
		return C_ERR;
	}

	/*
	* handle trivial cases
	*/
	if (b == 1)
	{
		*r = 0;
		return(0);
	}

	if (b == 2)
	{
		*r = A->p[0] & 1;
		return(0);
	}

	/*
	* general case
	*/
	for (i = A->n, y = 0; i > 0; i--)
	{
		x = A->p[i - 1];
		y = (y << biH) | (x >> biH);
		z = y / b;
		y -= z * b;

		x <<= biH;
		y = (y << biH) | (x >> biH);
		z = y / b;
		y -= z * b;
	}

	/*
	* If A is negative, then the current y represents a negative value.
	* Flipping it to the positive side.
	*/
	if (A->s < 0 && y != 0)
		y = b - y;

	*r = y;

	return(0);
}

/*
* Fast Montgomery initialization (thanks to Tom St Denis)
*/
static void mpi_montg_init(mpi_uint *mm, const mpi *N)
{
	mpi_uint x, m0 = N->p[0];
	unsigned int i;

	x = m0;
	x += ((m0 + 2) & 4) << 1;

	for (i = biL; i >= 8; i /= 2)
		x *= (2 - (m0 * x));

	*mm = ~x + 1;
}

/*
* Montgomery multiplication: A = A * B * R^-1 mod N  (HAC 14.36)
*/
static int mpi_montmul(mpi *A, const mpi *B, const mpi *N, mpi_uint mm,
	const mpi *T)
{
	dword_t i, n, m;
	mpi_uint u0, u1, *d;

	if (T->n < N->n + 1 || T->p == NULL)
	{
		set_last_error(_T("mpi_montmul"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	xmem_zero(T->p, T->n * ciL);

	d = T->p;
	n = N->n;
	m = (B->n < n) ? B->n : n;

	for (i = 0; i < n; i++)
	{
		/*
		* T = (T + u0*B + u1*N) / 2^biL
		*/
		u0 = A->p[i];
		u1 = (d[0] + u0 * B->p[0]) * mm;

		mpi_mul_hlp(m, B->p, d, u0);
		mpi_mul_hlp(n, N->p, d, u1);

		*d++ = u0; d[n + 1] = 0;
	}

	xmem_copy(A->p, d, (n + 1) * ciL);

	if (mpi_cmp_abs(A, N) >= 0)
		mpi_sub_hlp(n, N->p, A->p);
	else
		/* prevent timing attacks */
		mpi_sub_hlp(n, A->p, T->p);

	return(0);
}

/*
* Montgomery reduction: A = A * R^-1 mod N
*/
static int mpi_montred(mpi *A, const mpi *N,
	mpi_uint mm, const mpi *T)
{
	mpi_uint z = 1;
	mpi U;

	U.n = U.s = (int)z;
	U.p = &z;

	return(mpi_montmul(A, &U, N, mm, T));
}

/*
* Sliding-window exponentiation: X = A^E mod N  (HAC 14.85)
*/
int mpi_exp_mod(mpi *X, const mpi *A,
	const mpi *E, const mpi *N,
	mpi *_RR)
{
	int ret;
	dword_t wbits, wsize, one = 1;
	dword_t i, j, nblimbs;
	dword_t bufsize, nbits;
	mpi_uint ei, mm, state;
	mpi RR, T, W[2 << MPI_WINDOW_SIZE], Apos;
	int neg;

	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(E != NULL);
	XDK_ASSERT(N != NULL);

	if (mpi_cmp_int(N, 0) <= 0 || (N->p[0] & 1) == 0)
	{
		set_last_error(_T("mpi_exp_mod"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	if (mpi_cmp_int(E, 0) < 0)
	{
		set_last_error(_T("mpi_exp_mod"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	/*
	* Init temps and window size
	*/
	mpi_montg_init(&mm, N);
	mpi_init(&RR); mpi_init(&T);
	mpi_init(&Apos);
	xmem_zero(W, sizeof(W));

	i = mpi_bitlen(E);

	wsize = (i > 671) ? 6 : (i > 239) ? 5 :
		(i >  79) ? 4 : (i >  23) ? 3 : 1;

#if( MPI_WINDOW_SIZE < 6 )
	if (wsize > MPI_WINDOW_SIZE)
		wsize = MPI_WINDOW_SIZE;
#endif

	j = N->n + 1;
	MPI_CHK(mpi_grow(X, j));
	MPI_CHK(mpi_grow(&W[1], j));
	MPI_CHK(mpi_grow(&T, j * 2));

	/*
	* Compensate for negative A (and correct at the end)
	*/
	neg = (A->s == -1);
	if (neg)
	{
		MPI_CHK(mpi_copy(&Apos, A));
		Apos.s = 1;
		A = &Apos;
	}

	/*
	* If 1st call, pre-compute R^2 mod N
	*/
	if (_RR == NULL || _RR->p == NULL)
	{
		MPI_CHK(mpi_lset(&RR, 1));
		MPI_CHK(mpi_shift_l(&RR, N->n * 2 * biL));
		MPI_CHK(mpi_mod_mpi(&RR, &RR, N));

		if (_RR != NULL)
			xmem_copy(_RR, &RR, sizeof(mpi));
	}
	else
		xmem_copy(&RR, _RR, sizeof(mpi));

	/*
	* W[1] = A * R^2 * R^-1 mod N = A * R mod N
	*/
	if (mpi_cmp_mpi(A, N) >= 0)
		MPI_CHK(mpi_mod_mpi(&W[1], A, N));
	else
		MPI_CHK(mpi_copy(&W[1], A));

	MPI_CHK(mpi_montmul(&W[1], &RR, N, mm, &T));

	/*
	* X = R^2 * R^-1 mod N = R mod N
	*/
	MPI_CHK(mpi_copy(X, &RR));
	MPI_CHK(mpi_montred(X, N, mm, &T));

	if (wsize > 1)
	{
		/*
		* W[1 << (wsize - 1)] = W[1] ^ (wsize - 1)
		*/
		j = one << (wsize - 1);

		MPI_CHK(mpi_grow(&W[j], N->n + 1));
		MPI_CHK(mpi_copy(&W[j], &W[1]));

		for (i = 0; i < wsize - 1; i++)
			MPI_CHK(mpi_montmul(&W[j], &W[j], N, mm, &T));

		/*
		* W[i] = W[i - 1] * W[1]
		*/
		for (i = j + 1; i < (one << wsize); i++)
		{
			MPI_CHK(mpi_grow(&W[i], N->n + 1));
			MPI_CHK(mpi_copy(&W[i], &W[i - 1]));

			MPI_CHK(mpi_montmul(&W[i], &W[1], N, mm, &T));
		}
	}

	nblimbs = E->n;
	bufsize = 0;
	nbits = 0;
	wbits = 0;
	state = 0;

	while (1)
	{
		if (bufsize == 0)
		{
			if (nblimbs == 0)
				break;

			nblimbs--;

			bufsize = sizeof(mpi_uint) << 3;
		}

		bufsize--;

		ei = (E->p[nblimbs] >> bufsize) & 1;

		/*
		* skip leading 0s
		*/
		if (ei == 0 && state == 0)
			continue;

		if (ei == 0 && state == 1)
		{
			/*
			* out of window, square X
			*/
			MPI_CHK(mpi_montmul(X, X, N, mm, &T));
			continue;
		}

		/*
		* add ei to current window
		*/
		state = 2;

		nbits++;
		wbits |= (ei << (wsize - nbits));

		if (nbits == wsize)
		{
			/*
			* X = X^wsize R^-1 mod N
			*/
			for (i = 0; i < wsize; i++)
				MPI_CHK(mpi_montmul(X, X, N, mm, &T));

			/*
			* X = X * W[wbits] R^-1 mod N
			*/
			MPI_CHK(mpi_montmul(X, &W[wbits], N, mm, &T));

			state--;
			nbits = 0;
			wbits = 0;
		}
	}

	/*
	* process the remaining bits
	*/
	for (i = 0; i < nbits; i++)
	{
		MPI_CHK(mpi_montmul(X, X, N, mm, &T));

		wbits <<= 1;

		if ((wbits & (one << wsize)) != 0)
			MPI_CHK(mpi_montmul(X, &W[1], N, mm, &T));
	}

	/*
	* X = A^E * R * R^-1 mod N = A^E mod N
	*/
	MPI_CHK(mpi_montred(X, N, mm, &T));

	if (neg && E->n != 0 && (E->p[0] & 1) != 0)
	{
		X->s = -1;
		MPI_CHK(mpi_add_mpi(X, N, X));
	}

cleanup:

	for (i = (one << (wsize - 1)); i < (one << wsize); i++)
		mpi_free(&W[i]);

	mpi_free(&W[1]); mpi_free(&T); mpi_free(&Apos);

	if (_RR == NULL || _RR->p == NULL)
		mpi_free(&RR);

	return(ret);
}

/*
* Greatest common divisor: G = gcd(A, B)  (HAC 14.54)
*/
int mpi_gcd(mpi *G, const mpi *A, const mpi *B)
{
	int ret;
	dword_t lz, lzt;
	mpi TG, TA, TB;

	XDK_ASSERT(G != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(B != NULL);

	mpi_init(&TG); mpi_init(&TA); mpi_init(&TB);

	MPI_CHK(mpi_copy(&TA, A));
	MPI_CHK(mpi_copy(&TB, B));

	lz = mpi_lsb(&TA);
	lzt = mpi_lsb(&TB);

	if (lzt < lz)
		lz = lzt;

	MPI_CHK(mpi_shift_r(&TA, lz));
	MPI_CHK(mpi_shift_r(&TB, lz));

	TA.s = TB.s = 1;

	while (mpi_cmp_int(&TA, 0) != 0)
	{
		MPI_CHK(mpi_shift_r(&TA, mpi_lsb(&TA)));
		MPI_CHK(mpi_shift_r(&TB, mpi_lsb(&TB)));

		if (mpi_cmp_mpi(&TA, &TB) >= 0)
		{
			MPI_CHK(mpi_sub_abs(&TA, &TA, &TB));
			MPI_CHK(mpi_shift_r(&TA, 1));
		}
		else
		{
			MPI_CHK(mpi_sub_abs(&TB, &TB, &TA));
			MPI_CHK(mpi_shift_r(&TB, 1));
		}
	}

	MPI_CHK(mpi_shift_l(&TB, lz));
	MPI_CHK(mpi_copy(G, &TB));

cleanup:

	mpi_free(&TG); mpi_free(&TA); mpi_free(&TB);

	return(ret);
}

/*
* Fill X with size bytes of random.
*
* Use a temporary bytes representation to make sure the result is the same
* regardless of the platform endianness (useful when f_rng is actually
* deterministic, eg for tests).
*/
int mpi_fill_random(mpi *X, dword_t size,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	dword_t const limbs = CHARS_TO_LIMBS(size);
	dword_t const overhead = (limbs * ciL) - size;
	byte_t *Xp;

	XDK_ASSERT(X != NULL);
	XDK_ASSERT(f_rng != NULL);

	/* Ensure that target MPI has exactly the necessary number of limbs */
	if (X->n != limbs)
	{
		mpi_free(X);
		mpi_init(X);
		MPI_CHK(mpi_grow(X, limbs));
	}
	MPI_CHK(mpi_lset(X, 0));

	Xp = (byte_t*)X->p;
	f_rng(p_rng, Xp + overhead, size);

	mpi_bigendian_to_host(X->p, limbs);

cleanup:
	return(ret);
}

/*
* Modular inverse: X = A^-1 mod N  (HAC 14.61 / 14.64)
*/
int mpi_inv_mod(mpi *X, const mpi *A, const mpi *N)
{
	int ret;
	mpi G, TA, TU, U1, U2, TB, TV, V1, V2;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(A != NULL);
	XDK_ASSERT(N != NULL);

	if (mpi_cmp_int(N, 1) <= 0)
	{
		set_last_error(_T("mpi_inv_mod"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	mpi_init(&TA); mpi_init(&TU); mpi_init(&U1); mpi_init(&U2);
	mpi_init(&G); mpi_init(&TB); mpi_init(&TV);
	mpi_init(&V1); mpi_init(&V2);

	MPI_CHK(mpi_gcd(&G, A, N));

	if (mpi_cmp_int(&G, 1) != 0)
	{
		set_last_error(_T("mpi_inv_mod"), _T("ERR_MPI_NOT_ACCEPTABLE"), -1);
		ret = ERR_MPI_NOT_ACCEPTABLE;
		goto cleanup;
	}

	MPI_CHK(mpi_mod_mpi(&TA, A, N));
	MPI_CHK(mpi_copy(&TU, &TA));
	MPI_CHK(mpi_copy(&TB, N));
	MPI_CHK(mpi_copy(&TV, N));

	MPI_CHK(mpi_lset(&U1, 1));
	MPI_CHK(mpi_lset(&U2, 0));
	MPI_CHK(mpi_lset(&V1, 0));
	MPI_CHK(mpi_lset(&V2, 1));

	do
	{
		while ((TU.p[0] & 1) == 0)
		{
			MPI_CHK(mpi_shift_r(&TU, 1));

			if ((U1.p[0] & 1) != 0 || (U2.p[0] & 1) != 0)
			{
				MPI_CHK(mpi_add_mpi(&U1, &U1, &TB));
				MPI_CHK(mpi_sub_mpi(&U2, &U2, &TA));
			}

			MPI_CHK(mpi_shift_r(&U1, 1));
			MPI_CHK(mpi_shift_r(&U2, 1));
		}

		while ((TV.p[0] & 1) == 0)
		{
			MPI_CHK(mpi_shift_r(&TV, 1));

			if ((V1.p[0] & 1) != 0 || (V2.p[0] & 1) != 0)
			{
				MPI_CHK(mpi_add_mpi(&V1, &V1, &TB));
				MPI_CHK(mpi_sub_mpi(&V2, &V2, &TA));
			}

			MPI_CHK(mpi_shift_r(&V1, 1));
			MPI_CHK(mpi_shift_r(&V2, 1));
		}

		if (mpi_cmp_mpi(&TU, &TV) >= 0)
		{
			MPI_CHK(mpi_sub_mpi(&TU, &TU, &TV));
			MPI_CHK(mpi_sub_mpi(&U1, &U1, &V1));
			MPI_CHK(mpi_sub_mpi(&U2, &U2, &V2));
		}
		else
		{
			MPI_CHK(mpi_sub_mpi(&TV, &TV, &TU));
			MPI_CHK(mpi_sub_mpi(&V1, &V1, &U1));
			MPI_CHK(mpi_sub_mpi(&V2, &V2, &U2));
		}
	} while (mpi_cmp_int(&TU, 0) != 0);

	while (mpi_cmp_int(&V1, 0) < 0)
		MPI_CHK(mpi_add_mpi(&V1, &V1, N));

	while (mpi_cmp_mpi(&V1, N) >= 0)
		MPI_CHK(mpi_sub_mpi(&V1, &V1, N));

	MPI_CHK(mpi_copy(X, &V1));

cleanup:

	mpi_free(&TA); mpi_free(&TU); mpi_free(&U1); mpi_free(&U2);
	mpi_free(&G); mpi_free(&TB); mpi_free(&TV);
	mpi_free(&V1); mpi_free(&V2);

	return(ret);
}

#if defined(GENPRIME)

static const int small_prime[] =
{
	3, 5, 7, 11, 13, 17, 19, 23,
	29, 31, 37, 41, 43, 47, 53, 59,
	61, 67, 71, 73, 79, 83, 89, 97,
	101, 103, 107, 109, 113, 127, 131, 137,
	139, 149, 151, 157, 163, 167, 173, 179,
	181, 191, 193, 197, 199, 211, 223, 227,
	229, 233, 239, 241, 251, 257, 263, 269,
	271, 277, 281, 283, 293, 307, 311, 313,
	317, 331, 337, 347, 349, 353, 359, 367,
	373, 379, 383, 389, 397, 401, 409, 419,
	421, 431, 433, 439, 443, 449, 457, 461,
	463, 467, 479, 487, 491, 499, 503, 509,
	521, 523, 541, 547, 557, 563, 569, 571,
	577, 587, 593, 599, 601, 607, 613, 617,
	619, 631, 641, 643, 647, 653, 659, 661,
	673, 677, 683, 691, 701, 709, 719, 727,
	733, 739, 743, 751, 757, 761, 769, 773,
	787, 797, 809, 811, 821, 823, 827, 829,
	839, 853, 857, 859, 863, 877, 881, 883,
	887, 907, 911, 919, 929, 937, 941, 947,
	953, 967, 971, 977, 983, 991, 997, -103
};

/*
* Small divisors test (X must be positive)
*
* Return values:
* 0: no small factor (possible prime, more tests needed)
* 1: certain prime
* ERR_MPI_NOT_ACCEPTABLE: certain non-prime
* other negative: error
*/
static int mpi_check_small_factors(const mpi *X)
{
	int ret = 0;
	dword_t i;
	mpi_uint r;

	if ((X->p[0] & 1) == 0)
	{
		set_last_error(_T("mpi_check_small_factors"), _T("ERR_MPI_NOT_ACCEPTABLE"), -1);
		return ERR_MPI_NOT_ACCEPTABLE;
	}

	for (i = 0; small_prime[i] > 0; i++)
	{
		if (mpi_cmp_int(X, small_prime[i]) <= 0)
			return(1);

		MPI_CHK(mpi_mod_int(&r, X, small_prime[i]));

		if (r == 0)
		{
			set_last_error(_T("mpi_check_small_factors"), _T("ERR_MPI_NOT_ACCEPTABLE"), -1);
			return ERR_MPI_NOT_ACCEPTABLE;
		}
	}

cleanup:
	return(ret);
}

/*
* Miller-Rabin pseudo-primality test  (HAC 4.24)
*/
static int mpi_miller_rabin(const mpi *X, dword_t rounds,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret, count;
	dword_t i, j, k, s;
	mpi W, R, T, A, RR;

	XDK_ASSERT(X != NULL);
	XDK_ASSERT(f_rng != NULL);

	mpi_init(&W); mpi_init(&R);
	mpi_init(&T); mpi_init(&A);
	mpi_init(&RR);

	/*
	* W = |X| - 1
	* R = W >> lsb( W )
	*/
	MPI_CHK(mpi_sub_int(&W, X, 1));
	s = mpi_lsb(&W);
	MPI_CHK(mpi_copy(&R, &W));
	MPI_CHK(mpi_shift_r(&R, s));

	for (i = 0; i < rounds; i++)
	{
		/*
		* pick a random A, 1 < A < |X| - 1
		*/
		count = 0;
		do {
			MPI_CHK(mpi_fill_random(&A, X->n * ciL, f_rng, p_rng));

			j = mpi_bitlen(&A);
			k = mpi_bitlen(&W);
			if (j > k) {
				A.p[A.n - 1] &= ((mpi_uint)1 << (k - (A.n - 1) * biL - 1)) - 1;
			}

			if (count++ > 30)
			{
				set_last_error(_T("mpi_miller_rabin"), _T("ERR_MPI_NOT_ACCEPTABLE"), -1);
				ret = ERR_MPI_NOT_ACCEPTABLE;
				goto cleanup;
			}

		} while (mpi_cmp_mpi(&A, &W) >= 0 ||
			mpi_cmp_int(&A, 1) <= 0);

		/*
		* A = A^R mod |X|
		*/
		MPI_CHK(mpi_exp_mod(&A, &A, &R, X, &RR));

		if (mpi_cmp_mpi(&A, &W) == 0 ||
			mpi_cmp_int(&A, 1) == 0)
			continue;

		j = 1;
		while (j < s && mpi_cmp_mpi(&A, &W) != 0)
		{
			/*
			* A = A * A mod |X|
			*/
			MPI_CHK(mpi_mul_mpi(&T, &A, &A));
			MPI_CHK(mpi_mod_mpi(&A, &T, X));

			if (mpi_cmp_int(&A, 1) == 0)
				break;

			j++;
		}

		/*
		* not prime if A != |X| - 1 or A == 1
		*/
		if (mpi_cmp_mpi(&A, &W) != 0 ||
			mpi_cmp_int(&A, 1) == 0)
		{
			set_last_error(_T("mpi_miller_rabin"), _T("ERR_MPI_NOT_ACCEPTABLE"), -1);
			ret = ERR_MPI_NOT_ACCEPTABLE;
			break;
		}
	}

cleanup:
	mpi_free(&W); mpi_free(&R);
	mpi_free(&T); mpi_free(&A);
	mpi_free(&RR);

	return(ret);
}

/*
* Pseudo-primality test: small factors, then Miller-Rabin
*/
int mpi_is_prime(const mpi *X, int rounds,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
	int ret;
	mpi XX;
	XDK_ASSERT(X != NULL);
	XDK_ASSERT(f_rng != NULL);

	XX.s = 1;
	XX.n = X->n;
	XX.p = X->p;

	if (mpi_cmp_int(&XX, 0) == 0 ||
		mpi_cmp_int(&XX, 1) == 0)
	{
		set_last_error(_T("mpi_is_prime"), _T("ERR_MPI_NOT_ACCEPTABLE"), -1);
		return ERR_MPI_NOT_ACCEPTABLE;
	}

	if (mpi_cmp_int(&XX, 2) == 0)
		return(0);

	if ((ret = mpi_check_small_factors(&XX)) != 0)
	{
		if (ret == 1)
			return(0);

		return(ret);
	}

	return(mpi_miller_rabin(&XX, rounds, f_rng, p_rng));
}

/*
* Prime number generation
*
* To generate an RSA key in a way recommended by FIPS 186-4, both primes must
* be either 1024 bits or 1536 bits long, and flags must contain
* MPI_GEN_PRIME_FLAG_LOW_ERR.
*/
int mpi_gen_prime(mpi *X, dword_t nbits, int flags,
	int(*f_rng)(void *, byte_t *, dword_t),
	void *p_rng)
{
#ifdef HAVE_INT64
	// ceil(2^63.5)
#define CEIL_MAXUINT_DIV_SQRT2 0xb504f333f9de6485ULL
#else
	// ceil(2^31.5)
#define CEIL_MAXUINT_DIV_SQRT2 0xb504f334U
#endif
	int ret = C_ERR;
	dword_t k, n;
	int rounds;
	mpi_uint r;
	mpi Y;

	XDK_ASSERT(X != NULL);
	XDK_ASSERT(f_rng != NULL);

	if (nbits < 3 || nbits > MPI_MAX_BITS)
	{
		set_last_error(_T("mpi_gen_prime"), _T("ERR_MPI_BAD_INPUT_DATA"), -1);
		return C_ERR;
	}

	mpi_init(&Y);

	n = BITS_TO_LIMBS(nbits);

	if ((flags & MPI_GEN_PRIME_FLAG_LOW_ERR) == 0)
	{
		/*
		* 2^-80 error probability, number of rounds chosen per HAC, table 4.4
		*/
		rounds = ((nbits >= 1300) ? 2 : (nbits >= 850) ? 3 :
			(nbits >= 650) ? 4 : (nbits >= 350) ? 8 :
			(nbits >= 250) ? 12 : (nbits >= 150) ? 18 : 27);
	}
	else
	{
		/*
		* 2^-100 error probability, number of rounds computed based on HAC,
		* fact 4.48
		*/
		rounds = ((nbits >= 1450) ? 4 : (nbits >= 1150) ? 5 :
			(nbits >= 1000) ? 6 : (nbits >= 850) ? 7 :
			(nbits >= 750) ? 8 : (nbits >= 500) ? 13 :
			(nbits >= 250) ? 28 : (nbits >= 150) ? 40 : 51);
	}

	while (1)
	{
		MPI_CHK(mpi_fill_random(X, n * ciL, f_rng, p_rng));
		/* make sure generated number is at least (nbits-1)+0.5 bits (FIPS 186-4 §B.3.3 steps 4.4, 5.5) */
		if (X->p[n - 1] < CEIL_MAXUINT_DIV_SQRT2) continue;

		k = n * biL;
		if (k > nbits) MPI_CHK(mpi_shift_r(X, k - nbits));
		X->p[0] |= 1;

		if ((flags & MPI_GEN_PRIME_FLAG_DH) == 0)
		{
			ret = mpi_is_prime(X, rounds, f_rng, p_rng);

			if (ret != ERR_MPI_NOT_ACCEPTABLE)
				goto cleanup;
		}
		else
		{
			/*
			* An necessary condition for Y and X = 2Y + 1 to be prime
			* is X = 2 mod 3 (which is equivalent to Y = 2 mod 3).
			* Make sure it is satisfied, while keeping X = 3 mod 4
			*/

			X->p[0] |= 2;

			MPI_CHK(mpi_mod_int(&r, X, 3));
			if (r == 0)
				MPI_CHK(mpi_add_int(X, X, 8));
			else if (r == 1)
				MPI_CHK(mpi_add_int(X, X, 4));

			/* Set Y = (X-1) / 2, which is X / 2 because X is odd */
			MPI_CHK(mpi_copy(&Y, X));
			MPI_CHK(mpi_shift_r(&Y, 1));

			while (1)
			{
				/*
				* First, check small factors for X and Y
				* before doing Miller-Rabin on any of them
				*/
				if ((ret = mpi_check_small_factors(X)) == 0 &&
					(ret = mpi_check_small_factors(&Y)) == 0 &&
					(ret = mpi_miller_rabin(X, rounds, f_rng, p_rng))
					== 0 &&
					(ret = mpi_miller_rabin(&Y, rounds, f_rng, p_rng))
					== 0)
					goto cleanup;

				if (ret != ERR_MPI_NOT_ACCEPTABLE)
					goto cleanup;

				/*
				* Next candidates. We want to preserve Y = (X-1) / 2 and
				* Y = 1 mod 2 and Y = 2 mod 3 (eq X = 3 mod 4 and X = 2 mod 3)
				* so up Y by 6 and X by 12.
				*/
				MPI_CHK(mpi_add_int(X, X, 12));
				MPI_CHK(mpi_add_int(&Y, &Y, 6));
			}
		}
	}

cleanup:

	mpi_free(&Y);

	return(ret);
}

#endif /* GENPRIME */



#if defined(XDK_SUPPORT_TEST)

#define GCD_PAIR_COUNT  3

static const int gcd_pairs[GCD_PAIR_COUNT][3] =
{
	{ 693, 609, 21 },
	{ 1764, 868, 28 },
	{ 768454923, 542167814, 1 }
};

/*
* Checkup routine
*/
int mpi_self_test(int verbose)
{
	int ret, i;
	mpi A, E, N, X, Y, U, V;

	mpi_init(&A); mpi_init(&E); mpi_init(&N); mpi_init(&X);
	mpi_init(&Y); mpi_init(&U); mpi_init(&V);

	MPI_CHK(mpi_read_string(&A, 16,
		"EFE021C2645FD1DC586E69184AF4A31E" \
		"D5F53E93B5F123FA41680867BA110131" \
		"944FE7952E2517337780CB0DB80E61AA" \
		"E7C8DDC6C5C6AADEB34EB38A2F40D5E6", -1));

	MPI_CHK(mpi_read_string(&E, 16,
		"B2E7EFD37075B9F03FF989C7C5051C20" \
		"34D2A323810251127E7BF8625A4F49A5" \
		"F3E27F4DA8BD59C47D6DAABA4C8127BD" \
		"5B5C25763222FEFCCFC38B832366C29E", -1));

	MPI_CHK(mpi_read_string(&N, 16,
		"0066A198186C18C10B2F5ED9B522752A" \
		"9830B69916E535C8F047518A889A43A5" \
		"94B6BED27A168D31D4A52F88925AA8F5", -1));

	MPI_CHK(mpi_mul_mpi(&X, &A, &N));

	MPI_CHK(mpi_read_string(&U, 16,
		"602AB7ECA597A3D6B56FF9829A5E8B85" \
		"9E857EA95A03512E2BAE7391688D264A" \
		"A5663B0341DB9CCFD2C4C5F421FEC814" \
		"8001B72E848A38CAE1C65F78E56ABDEF" \
		"E12D3C039B8A02D6BE593F0BBBDA56F1" \
		"ECF677152EF804370C1A305CAF3B5BF1" \
		"30879B56C61DE584A0F53A2447A51E", -1));

	if (verbose != 0)
		printf("  MPI test #1 (mul_mpi): ");

	if (mpi_cmp_mpi(&X, &U) != 0)
	{
		if (verbose != 0)
			printf("failed\n");

		ret = 1;
		goto cleanup;
	}

	if (verbose != 0)
		printf("passed\n");

	MPI_CHK(mpi_div_mpi(&X, &Y, &A, &N));

	MPI_CHK(mpi_read_string(&U, 16,
		"256567336059E52CAE22925474705F39A94", -1));

	MPI_CHK(mpi_read_string(&V, 16,
		"6613F26162223DF488E9CD48CC132C7A" \
		"0AC93C701B001B092E4E5B9F73BCD27B" \
		"9EE50D0657C77F374E903CDFA4C642", -1));

	if (verbose != 0)
		printf("  MPI test #2 (div_mpi): ");

	if (mpi_cmp_mpi(&X, &U) != 0 ||
		mpi_cmp_mpi(&Y, &V) != 0)
	{
		if (verbose != 0)
			printf("failed\n");

		ret = 1;
		goto cleanup;
	}

	if (verbose != 0)
		printf("passed\n");

	MPI_CHK(mpi_exp_mod(&X, &A, &E, &N, NULL));

	MPI_CHK(mpi_read_string(&U, 16,
		"36E139AEA55215609D2816998ED020BB" \
		"BD96C37890F65171D948E9BC7CBAA4D9" \
		"325D24D6A3C12710F10A09FA08AB87", -1));

	if (verbose != 0)
		printf("  MPI test #3 (exp_mod): ");

	if (mpi_cmp_mpi(&X, &U) != 0)
	{
		if (verbose != 0)
			printf("failed\n");

		ret = 1;
		goto cleanup;
	}

	if (verbose != 0)
		printf("passed\n");

	MPI_CHK(mpi_inv_mod(&X, &A, &N));

	MPI_CHK(mpi_read_string(&U, 16,
		"003A0AAEDD7E784FC07D8F9EC6E3BFD5" \
		"C3DBA76456363A10869622EAC2DD84EC" \
		"C5B8A74DAC4D09E03B5E0BE779F2DF61", -1));

	if (verbose != 0)
		printf("  MPI test #4 (inv_mod): ");

	if (mpi_cmp_mpi(&X, &U) != 0)
	{
		if (verbose != 0)
			printf("failed\n");

		ret = 1;
		goto cleanup;
	}

	if (verbose != 0)
		printf("passed\n");

	if (verbose != 0)
		printf("  MPI test #5 (simple gcd): ");

	for (i = 0; i < GCD_PAIR_COUNT; i++)
	{
		MPI_CHK(mpi_lset(&X, gcd_pairs[i][0]));
		MPI_CHK(mpi_lset(&Y, gcd_pairs[i][1]));

		MPI_CHK(mpi_gcd(&A, &X, &Y));

		if (mpi_cmp_int(&A, gcd_pairs[i][2]) != 0)
		{
			if (verbose != 0)
				printf("failed at %d\n", i);

			ret = 1;
			goto cleanup;
		}
	}

	if (verbose != 0)
		printf("passed\n");

cleanup:

	if (ret != 0 && verbose != 0)
		printf("Unexpected error, return code = %08X\n", ret);

	mpi_free(&A); mpi_free(&E); mpi_free(&N); mpi_free(&X);
	mpi_free(&Y); mpi_free(&U); mpi_free(&V);

	if (verbose != 0)
		printf("\n");

	return(ret);
}

#endif /* SELF_TEST */

