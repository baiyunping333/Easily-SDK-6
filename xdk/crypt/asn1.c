
/*
*  Generic ASN.1 parsing
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

#include "asn1.h"

#include "../xdkimp.h"

/*
* ASN.1 DER decoding routines
*/
int asn1_get_len(byte_t **p,
	const byte_t *end,
	dword_t *len)
{
	if ((end - *p) < 1)
	{
		set_last_error(_T("asn1_get_len"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
		return C_ERR;
	}

	if ((**p & 0x80) == 0)
		*len = *(*p)++;
	else
	{
		switch (**p & 0x7F)
		{
		case 1:
			if ((end - *p) < 2)
			{
				set_last_error(_T("asn1_get_len"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
				return C_ERR;
			}

			*len = (*p)[1];
			(*p) += 2;
			break;

		case 2:
			if ((end - *p) < 3)
			{
				set_last_error(_T("asn1_get_len"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
				return C_ERR;
			}

			*len = ((dword_t)(*p)[1] << 8) | (*p)[2];
			(*p) += 3;
			break;

		case 3:
			if ((end - *p) < 4)
			{
				set_last_error(_T("asn1_get_len"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
				return C_ERR;
			}

			*len = ((dword_t)(*p)[1] << 16) |
				((dword_t)(*p)[2] << 8) | (*p)[3];
			(*p) += 4;
			break;

		case 4:
			if ((end - *p) < 5)
			{
				set_last_error(_T("asn1_get_len"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
				return C_ERR;
			}

			*len = ((dword_t)(*p)[1] << 24) | ((dword_t)(*p)[2] << 16) |
				((dword_t)(*p)[3] << 8) | (*p)[4];
			(*p) += 5;
			break;

		default:
			set_last_error(_T("asn1_get_len"), _T("ERR_ASN1_INVALID_LENGTH"), -1);
			return C_ERR;
		}
	}

	if (*len > (dword_t)(end - *p))
	{
		set_last_error(_T("asn1_get_len"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
		return C_ERR;
	}

	return(0);
}

int asn1_get_tag(byte_t **p,
	const byte_t *end,
	dword_t *len, int tag)
{
	if ((end - *p) < 1)
	{
		set_last_error(_T("asn1_get_tag"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
		return C_ERR;
	}

	if (**p != tag)
	{
		set_last_error(_T("asn1_get_tag"), _T("ERR_ASN1_UNEXPECTED_TAG"), -1);
		return ERR_ASN1_UNEXPECTED_TAG;
	}

	(*p)++;

	return(asn1_get_len(p, end, len));
}

int asn1_get_bool(byte_t **p,
	const byte_t *end,
	int *val)
{
	int ret;
	dword_t len;

	if ((ret = asn1_get_tag(p, end, &len, ASN1_BOOLEAN)) != 0)
		return(ret);

	if (len != 1)
	{
		set_last_error(_T("asn1_get_bool"), _T("ERR_ASN1_INVALID_LENGTH"), -1);
		return C_ERR;
	}

	*val = (**p != 0) ? 1 : 0;
	(*p)++;

	return(0);
}

int asn1_get_int(byte_t **p,
	const byte_t *end,
	int *val)
{
	int ret;
	dword_t len;

	if ((ret = asn1_get_tag(p, end, &len, ASN1_INTEGER)) != 0)
		return(ret);

	if (len == 0 || len > sizeof(int) || (**p & 0x80) != 0)
	{
		set_last_error(_T("asn1_get_int"), _T("ERR_ASN1_INVALID_LENGTH"), -1);
		return C_ERR;
	}

	*val = 0;

	while (len-- > 0)
	{
		*val = (*val << 8) | **p;
		(*p)++;
	}

	return(0);
}

int asn1_get_mpi(byte_t **p,
	const byte_t *end,
	mpi *X)
{
	int ret;
	dword_t len;

	if ((ret = asn1_get_tag(p, end, &len, ASN1_INTEGER)) != 0)
		return(ret);

	ret = mpi_read_binary(X, *p, len);

	*p += len;

	return(ret);
}

int asn1_get_bitstring(byte_t **p, const byte_t *end,
	asn1_bitstring *bs)
{
	int ret;

	/* Certificate type is a single byte bitstring */
	if ((ret = asn1_get_tag(p, end, &bs->len, ASN1_BIT_STRING)) != 0)
		return(ret);

	/* Check length, subtract one for actual bit string length */
	if (bs->len < 1)
	{
		set_last_error(_T("asn1_get_bitstring"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
		return C_ERR;
	}

	bs->len -= 1;

	/* Get number of unused bits, ensure unused bits <= 7 */
	bs->unused_bits = **p;
	if (bs->unused_bits > 7)
	{
		set_last_error(_T("asn1_get_bitstring"), _T("ERR_ASN1_INVALID_LENGTH"), -1);
		return C_ERR;
	}

	(*p)++;

	/* Get actual bitstring */
	bs->p = *p;
	*p += bs->len;

	if (*p != end)
	{
		set_last_error(_T("asn1_get_bitstring"), _T("ERR_ASN1_LENGTH_MISMATCH"), -1);
		return C_ERR;
	}

	return(0);
}

/*
* Get a bit string without unused bits
*/
int asn1_get_bitstring_null(byte_t **p, const byte_t *end,
	dword_t *len)
{
	int ret;

	if ((ret = asn1_get_tag(p, end, len, ASN1_BIT_STRING)) != 0)
		return(ret);

	if ((*len)-- < 2 || *(*p)++ != 0)
	{
		set_last_error(_T("asn1_get_bitstring_null"), _T("ERR_ASN1_INVALID_DATA"), -1);
		return C_ERR;
	}

	return(0);
}


/*
*  Parses and splits an ASN.1 "SEQUENCE OF <tag>"
*/
int asn1_get_sequence_of(byte_t **p,
	const byte_t *end,
	asn1_sequence *cur,
	int tag)
{
	int ret;
	dword_t len;
	asn1_buf *buf;

	/* Get main sequence tag */
	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
		return(ret);

	if (*p + len != end)
	{
		set_last_error(_T("asn1_get_sequence_of"), _T("ERR_ASN1_LENGTH_MISMATCH"), -1);
		return C_ERR;
	}

	while (*p < end)
	{
		buf = &(cur->buf);
		buf->tag = **p;

		if ((ret = asn1_get_tag(p, end, &buf->len, tag)) != 0)
			return(ret);

		buf->p = *p;
		*p += buf->len;

		/* Allocate and assign next pointer */
		if (*p < end)
		{
			cur->next = (asn1_sequence*)xmem_alloc(sizeof(asn1_sequence));

			if (cur->next == NULL)
			{
				set_last_error(_T("asn1_get_sequence_of"), _T("ERR_ASN1_ALLOC_FAILED"), -1);
				return C_ERR;
			}

			cur = cur->next;
		}
	}

	/* Set final sequence entry's next pointer to NULL */
	cur->next = NULL;

	if (*p != end)
	{
		set_last_error(_T("asn1_get_sequence_of"), _T("ERR_ASN1_LENGTH_MISMATCH"), -1);
		return C_ERR;
	}

	return(0);
}

int asn1_get_alg(byte_t **p,
	const byte_t *end,
	asn1_buf *alg, asn1_buf *params)
{
	int ret;
	dword_t len;

	if ((ret = asn1_get_tag(p, end, &len,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE)) != 0)
		return(ret);

	if ((end - *p) < 1)
	{
		set_last_error(_T("asn1_get_alg"), _T("ERR_ASN1_OUT_OF_DATA"), -1);
		return C_ERR;
	}

	alg->tag = **p;
	end = *p + len;

	if ((ret = asn1_get_tag(p, end, &alg->len, ASN1_OID)) != 0)
		return(ret);

	alg->p = *p;
	*p += alg->len;

	if (*p == end)
	{
		xmem_zero(params, sizeof(asn1_buf));
		return(0);
	}

	params->tag = **p;
	(*p)++;

	if ((ret = asn1_get_len(p, end, &params->len)) != 0)
		return(ret);

	params->p = *p;
	*p += params->len;

	if (*p != end)
	{
		set_last_error(_T("asn1_get_alg"), _T("ERR_ASN1_LENGTH_MISMATCH"), -1);
		return C_ERR;
	}

	return(0);
}

int asn1_get_alg_null(byte_t **p,
	const byte_t *end,
	asn1_buf *alg)
{
	int ret;
	asn1_buf params;

	xmem_zero(&params, sizeof(asn1_buf));

	if ((ret = asn1_get_alg(p, end, alg, &params)) != 0)
		return(ret);

	if ((params.tag != ASN1_NULL && params.tag != 0) || params.len != 0)
	{
		set_last_error(_T("asn1_get_alg_null"), _T("ERR_ASN1_INVALID_DATA"), -1);
		return C_ERR;
	}

	return(0);
}

void asn1_free_named_data(asn1_named_data *cur)
{
	if (cur == NULL)
		return;

	xmem_free(cur->oid.p);
	xmem_free(cur->val.p);

	xmem_zero(cur, sizeof(asn1_named_data));
}

void asn1_free_named_data_list(asn1_named_data **head)
{
	asn1_named_data *cur;

	while ((cur = *head) != NULL)
	{
		*head = cur->next;
		asn1_free_named_data(cur);
		xmem_free(cur);
	}
}

asn1_named_data *asn1_find_named_data(asn1_named_data *list,
	const char *oid, dword_t len)
{
	while (list != NULL)
	{
		if (list->oid.len == len &&
			memcmp(list->oid.p, oid, len) == 0)
		{
			break;
		}

		list = list->next;
	}

	return(list);
}

#define ASN1_CHK_ADD(g, f)                      \
    do                                          \
		    {                                       \
        if( ( ret = (f) ) < 0 )                 \
            return( ret );                      \
						        else                            \
            (g) += ret;                         \
		    } while( 0 )

int asn1_write_len(byte_t **p, byte_t *start, dword_t len)
{
	if (len < 0x80)
	{
		if (*p - start < 1)
		{
			set_last_error(_T("asn1_write_len"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
			return C_ERR;
		}

		*--(*p) = (byte_t)len;
		return(1);
	}

	if (len <= 0xFF)
	{
		if (*p - start < 2)
		{
			set_last_error(_T("asn1_write_len"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
			return C_ERR;
		}

		*--(*p) = (byte_t)len;
		*--(*p) = 0x81;
		return(2);
	}

	if (len <= 0xFFFF)
	{
		if (*p - start < 3)
		{
			set_last_error(_T("asn1_write_len"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
			return C_ERR;
		}

		*--(*p) = (len)& 0xFF;
		*--(*p) = (len >> 8) & 0xFF;
		*--(*p) = 0x82;
		return(3);
	}

	if (len <= 0xFFFFFF)
	{
		if (*p - start < 4)
		{
			set_last_error(_T("asn1_write_len"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
			return C_ERR;
		}

		*--(*p) = (len)& 0xFF;
		*--(*p) = (len >> 8) & 0xFF;
		*--(*p) = (len >> 16) & 0xFF;
		*--(*p) = 0x83;
		return(4);
	}

#if SIZE_MAX > 0xFFFFFFFF
	if (len <= 0xFFFFFFFF)
#endif
	{
		if (*p - start < 5)
		{
			set_last_error(_T("asn1_write_len"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
			return C_ERR;
		}

		*--(*p) = (len)& 0xFF;
		*--(*p) = (len >> 8) & 0xFF;
		*--(*p) = (len >> 16) & 0xFF;
		*--(*p) = (len >> 24) & 0xFF;
		*--(*p) = 0x84;
		return(5);
	}

#if SIZE_MAX > 0xFFFFFFFF
	set_last_error(_T("asn1_write_len"), _T("ERR_ASN1_INVALID_LENGTH"), -1);
	return C_ERR;
#endif

	return (0);
}

int asn1_write_tag(byte_t **p, byte_t *start, byte_t tag)
{
	if (*p - start < 1)
	{
		set_last_error(_T("asn1_write_tag"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
		return C_ERR;
	}

	*--(*p) = tag;

	return(1);
}

int asn1_write_raw_buffer(byte_t **p, byte_t *start,
	const byte_t *buf, dword_t size)
{
	dword_t len = 0;

	if (*p < start || (dword_t)(*p - start) < size)
	{
		set_last_error(_T("asn1_write_raw_buffer"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
		return C_ERR;
	}

	len = size;
	(*p) -= len;
	xmem_copy(*p, buf, len);

	return((int)len);
}

int asn1_write_mpi(byte_t **p, byte_t *start, const mpi *X)
{
	int ret;
	dword_t len = 0;

	// Write the MPI
	//
	len = mpi_size(X);

	if (*p < start || (dword_t)(*p - start) < len)
	{
		set_last_error(_T("asn1_write_mpi"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
		return C_ERR;
	}

	(*p) -= len;
	MPI_CHK(mpi_write_binary(X, *p, len));

	// DER format assumes 2s complement for numbers, so the leftmost bit
	// should be 0 for positive numbers and 1 for negative numbers.
	//
	if (X->s == 1 && **p & 0x80)
	{
		if (*p - start < 1)
		{
			set_last_error(_T("asn1_write_mpi"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
			return C_ERR;
		}

		*--(*p) = 0x00;
		len += 1;
	}

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_INTEGER));

	ret = (int)len;

cleanup:
	return(ret);
}

int asn1_write_null(byte_t **p, byte_t *start)
{
	int ret;
	dword_t len = 0;

	// Write NULL
	//
	ASN1_CHK_ADD(len, asn1_write_len(p, start, 0));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_NULL));

	return((int)len);
}

int asn1_write_oid(byte_t **p, byte_t *start,
	const char *oid, dword_t oid_len)
{
	int ret;
	dword_t len = 0;

	ASN1_CHK_ADD(len, asn1_write_raw_buffer(p, start,
		(const byte_t *)oid, oid_len));
	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_OID));

	return((int)len);
}

int asn1_write_algorithm_identifier(byte_t **p, byte_t *start,
	const char *oid, dword_t oid_len,
	dword_t par_len)
{
	int ret;
	dword_t len = 0;

	if (par_len == 0)
		ASN1_CHK_ADD(len, asn1_write_null(p, start));
	else
		len += par_len;

	ASN1_CHK_ADD(len, asn1_write_oid(p, start, oid, oid_len));

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start,
		ASN1_CONSTRUCTED | ASN1_SEQUENCE));

	return((int)len);
}

int asn1_write_bool(byte_t **p, byte_t *start, int boolean)
{
	int ret;
	dword_t len = 0;

	if (*p - start < 1)
	{
		set_last_error(_T("asn1_write_bool"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
		return C_ERR;
	}

	*--(*p) = (boolean) ? 255 : 0;
	len++;

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_BOOLEAN));

	return((int)len);
}

int asn1_write_int(byte_t **p, byte_t *start, int val)
{
	int ret;
	dword_t len = 0;

	if (*p - start < 1)
	{
		set_last_error(_T("asn1_write_int"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
		return C_ERR;
	}

	len += 1;
	*--(*p) = val;

	if (val > 0 && **p & 0x80)
	{
		if (*p - start < 1)
		{
			set_last_error(_T("asn1_write_int"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
			return C_ERR;
		}

		*--(*p) = 0x00;
		len += 1;
	}

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_INTEGER));

	return((int)len);
}

int asn1_write_tagged_string(byte_t **p, byte_t *start, int tag,
	const char *text, dword_t text_len)
{
	int ret;
	dword_t len = 0;

	ASN1_CHK_ADD(len, asn1_write_raw_buffer(p, start,
		(const byte_t *)text, text_len));

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, tag));

	return((int)len);
}

int asn1_write_utf8_string(byte_t **p, byte_t *start,
	const char *text, dword_t text_len)
{
	return(asn1_write_tagged_string(p, start, ASN1_UTF8_STRING, text, text_len));
}

int asn1_write_printable_string(byte_t **p, byte_t *start,
	const char *text, dword_t text_len)
{
	return(asn1_write_tagged_string(p, start, ASN1_PRINTABLE_STRING, text, text_len));
}

int asn1_write_ia5_string(byte_t **p, byte_t *start,
	const char *text, dword_t text_len)
{
	return(asn1_write_tagged_string(p, start, ASN1_IA5_STRING, text, text_len));
}

int asn1_write_bitstring(byte_t **p, byte_t *start,
	const byte_t *buf, dword_t bits)
{
	int ret;
	dword_t len = 0;
	dword_t unused_bits, byte_len;

	byte_len = (bits + 7) / 8;
	unused_bits = (byte_len * 8) - bits;

	if (*p < start || (dword_t)(*p - start) < byte_len + 1)
	{
		set_last_error(_T("asn1_write_bitstring"), _T("ERR_ASN1_BUF_TOO_SMALL"), -1);
		return C_ERR;
	}

	len = byte_len + 1;

	/* Write the bitstring. Ensure the unused bits are zeroed */
	if (byte_len > 0)
	{
		byte_len--;
		*--(*p) = buf[byte_len] & ~((0x1 << unused_bits) - 1);
		(*p) -= byte_len;
		xmem_copy(*p, buf, byte_len);
	}

	/* Write unused bits */
	*--(*p) = (byte_t)unused_bits;

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_BIT_STRING));

	return((int)len);
}

int asn1_write_octet_string(byte_t **p, byte_t *start,
	const byte_t *buf, dword_t size)
{
	int ret;
	dword_t len = 0;

	ASN1_CHK_ADD(len, asn1_write_raw_buffer(p, start, buf, size));

	ASN1_CHK_ADD(len, asn1_write_len(p, start, len));
	ASN1_CHK_ADD(len, asn1_write_tag(p, start, ASN1_OCTET_STRING));

	return((int)len);
}

asn1_named_data *asn1_store_named_data(
	asn1_named_data **head,
	const char *oid, dword_t oid_len,
	const byte_t *val,
	dword_t val_len)
{
	asn1_named_data *cur;

	if ((cur = asn1_find_named_data(*head, oid, oid_len)) == NULL)
	{
		// Add new entry if not present yet based on OID
		//
		cur = (asn1_named_data*)xmem_alloc(sizeof(asn1_named_data));
		if (cur == NULL)
			return(NULL);

		cur->oid.len = oid_len;
		cur->oid.p = xmem_alloc(oid_len);
		if (cur->oid.p == NULL)
		{
			xmem_free(cur);
			return(NULL);
		}

		xmem_copy(cur->oid.p, oid, oid_len);

		cur->val.len = val_len;
		cur->val.p = xmem_alloc(val_len);
		if (cur->val.p == NULL)
		{
			xmem_free(cur->oid.p);
			xmem_free(cur);
			return(NULL);
		}

		cur->next = *head;
		*head = cur;
	}
	else if (cur->val.len < val_len)
	{
		/*
		* Enlarge existing value buffer if needed
		* Preserve old data until the allocation succeeded, to leave list in
		* a consistent state in case allocation fails.
		*/
		void *p = xmem_alloc(val_len);
		if (p == NULL)
			return(NULL);

		xmem_free(cur->val.p);
		cur->val.p = p;
		cur->val.len = val_len;
	}

	if (val != NULL)
		xmem_copy(cur->val.p, val, val_len);

	return(cur);
}


#if defined(XDK_SUPPORT_TEST)

void test_asn1()
{
	/*int n;
	int total = 0;
	byte_t tmp[1024];

	n = MAX_BYTE - 1;
	total += asn1_write_integer(tmp + total, n);
	printf("write: %d\n", n);

	n = MAX_SHORT - 1;
	total += asn1_write_integer(tmp + total, n);
	printf("write: %d\n", n);

	n = MAX_LONG - 1;
	total += asn1_write_integer(tmp + total, n);
	printf("write: %d\n", n);

	total = 0;

	total += asn1_read_integer(tmp + total, &n);
	if (n == MAX_BYTE - 1)
		printf("read: %d\n", n);
	else
		printf("read error: %d\n", n);

	total += asn1_read_integer(tmp + total, &n);
	if (n == MAX_SHORT - 1)
		printf("read: %d\n", n);
	else
		printf("read error: %d\n", n);

	total += asn1_read_integer(tmp + total, &n);
	if (n == MAX_LONG - 1)
		printf("read: %d\n", n);
	else
		printf("read error: %d\n", n);

	total = 0;

	total += asn1_write_null(tmp);
	printf("write: NULL\n", n);

	const char str[] = "OCTET TOKEN";
	total += asn1_write_octet_string((tmp + total), str, a_xslen(str));
	printf("write: %s\n", str);

	total = 0;

	total += asn1_read_null((tmp + total));
	if (total)
		printf("rad: NULL\n", n);
	else
		printf("read error: NULL\n", n);

	byte_t* buf = { 0 };
	total += asn1_read_octet_string((tmp + total), &buf, &n);
	printf("rad: %s\n", buf);*/
}

#endif