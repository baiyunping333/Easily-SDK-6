/**
 * \file asn1.h
 *
 * \brief Generic ASN.1 parsing
 */
/*
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
#ifndef _OEMASN_H
#define	_OEMASN_H

#include "../xdkdef.h"
#include "mpi.h"

#define ERR_ASN1_UNEXPECTED_TAG		(C_ERR - 1)

/**
 * \name DER constants
 * These constants comply with the DER encoded ASN.1 type tags.
 * DER encoding uses hexadecimal representation.
 * An example DER sequence is:\n
 * - 0x02 -- tag indicating INTEGER
 * - 0x01 -- length in octets
 * - 0x05 -- value
 * Such sequences are typically read into \c ::x509_buf.
 * \{
 */
#define ASN1_BOOLEAN                 0x01
#define ASN1_INTEGER                 0x02
#define ASN1_BIT_STRING              0x03
#define ASN1_OCTET_STRING            0x04
#define ASN1_NULL                    0x05
#define ASN1_OID                     0x06
#define ASN1_UTF8_STRING             0x0C
#define ASN1_SEQUENCE                0x10
#define ASN1_SET                     0x11
#define ASN1_PRINTABLE_STRING        0x13
#define ASN1_T61_STRING              0x14
#define ASN1_IA5_STRING              0x16
#define ASN1_UTC_TIME                0x17
#define ASN1_GENERALIZED_TIME        0x18
#define ASN1_UNIVERSAL_STRING        0x1C
#define ASN1_BMP_STRING              0x1E
#define ASN1_PRIMITIVE               0x00
#define ASN1_CONSTRUCTED             0x20
#define ASN1_CONTEXT_SPECIFIC        0x80

/*
 * Bit masks for each of the components of an ASN.1 tag as specified in
 * ITU X.690 (08/2015), section 8.1 "General rules for encoding",
 * paragraph 8.1.2.2:
 *
 * Bit  8     7   6   5          1
 *     +-------+-----+------------+
 *     | Class | P/C | Tag number |
 *     +-------+-----+------------+
 */
#define ASN1_TAG_CLASS_MASK          0xC0
#define ASN1_TAG_PC_MASK             0x20
#define ASN1_TAG_VALUE_MASK          0x1F

/* \} name */
/* \} addtogroup asn1_module */

/** Returns the size of the binary string, without the trailing \\0 */
#define OID_SIZE(x) (sizeof(x) - 1)

/**
 * Compares an asn1_buf structure to a reference OID.
 *
 * Only works for 'defined' oid_str values (OID_HMAC_SHA1), you cannot use a
 * 'byte_t *oid' here!
 */
#define OID_CMP(oid_str, oid_buf)                                   \
        ( ( OID_SIZE(oid_str) != (oid_buf)->len ) ||                \
          xmem_comp( (oid_str), (oid_buf)->p, (oid_buf)->len) != 0 )

#define ASN1_CHK_ADD(g, f)                      \
    do                                                  \
	    {                                                   \
        if( ( ret = (f) ) < 0 )                         \
            return( ret );                              \
		        else                                            \
            (g) += ret;                                 \
	    } while( 0 )


/**
 * \name Functions to parse ASN.1 data structures
 * \{
 */

/**
 * Type-length-value structure that allows for ASN1 using DER.
 */
typedef struct asn1_buf
{
    int tag;                /**< ASN1 type, e.g. ASN1_UTF8_STRING. */
    dword_t len;             /**< ASN1 length, in octets. */
    byte_t *p;       /**< ASN1 data, e.g. in ASCII. */
}
asn1_buf;

/**
 * Container for ASN1 bit strings.
 */
typedef struct asn1_bitstring
{
    dword_t len;                 /**< ASN1 length, in octets. */
    byte_t unused_bits;  /**< Number of unused bits at the end of the string */
    byte_t *p;           /**< Raw ASN1 data for the bit string */
}
asn1_bitstring;

/**
 * Container for a sequence of ASN.1 items
 */
typedef struct asn1_sequence
{
    asn1_buf buf;                   /**< Buffer containing the given ASN.1 item. */
    struct asn1_sequence *next;    /**< The next entry in the sequence. */
}
asn1_sequence;

/**
 * Container for a sequence or list of 'named' ASN.1 data items
 */
typedef struct asn1_named_data
{
    asn1_buf oid;                   /**< The object identifier. */
    asn1_buf val;                   /**< The named value. */
    struct asn1_named_data *next;  /**< The next entry in the sequence. */
    byte_t next_merged;      /**< Merge next item into the current one? */
}
asn1_named_data;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief       Get the length of an ASN.1 element.
 *              Updates the pointer to immediately behind the length.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param len   The variable that will receive the value
 *
 * \return      0 if successful, ERR_ASN1_OUT_OF_DATA on reaching
 *              end of data, ERR_ASN1_INVALID_LENGTH if length is
 *              unparseable.
 */
EXP_API int asn1_get_len( byte_t **p,
                  const byte_t *end,
                  dword_t *len );

/**
 * \brief       Get the tag and length of the tag. Check for the requested tag.
 *              Updates the pointer to immediately behind the tag and length.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param len   The variable that will receive the length
 * \param tag   The expected tag
 *
 * \return      0 if successful, ERR_ASN1_UNEXPECTED_TAG if tag did
 *              not match requested tag, or another specific ASN.1 error code.
 */
EXP_API int asn1_get_tag(byte_t **p,
                  const byte_t *end,
                  dword_t *len, int tag );

/**
 * \brief       Retrieve a boolean ASN.1 tag and its value.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param val   The variable that will receive the value
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
EXP_API int asn1_get_bool(byte_t **p,
                   const byte_t *end,
                   int *val );

/**
 * \brief       Retrieve an integer ASN.1 tag and its value.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param val   The variable that will receive the value
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
EXP_API int asn1_get_int(byte_t **p,
                  const byte_t *end,
                  int *val );

/**
 * \brief       Retrieve a bitstring ASN.1 tag and its value.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param bs    The variable that will receive the value
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
EXP_API int asn1_get_bitstring(byte_t **p, const byte_t *end,
                        asn1_bitstring *bs);

/**
 * \brief       Retrieve a bitstring ASN.1 tag without unused bits and its
 *              value.
 *              Updates the pointer to the beginning of the bit/octet string.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param len   Length of the actual bit/octect string in bytes
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
EXP_API int asn1_get_bitstring_null(byte_t **p, const byte_t *end,
                             dword_t *len );

/**
 * \brief       Parses and splits an ASN.1 "SEQUENCE OF <tag>"
 *              Updated the pointer to immediately behind the full sequence tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param cur   First variable in the chain to fill
 * \param tag   Type of sequence
 *
 * \return      0 if successful or a specific ASN.1 error code.
 */
EXP_API int asn1_get_sequence_of(byte_t **p,
                          const byte_t *end,
                          asn1_sequence *cur,
                          int tag);

/**
 * \brief       Retrieve a MPI value from an integer ASN.1 tag.
 *              Updates the pointer to immediately behind the full tag.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param X     The MPI that will receive the value
 *
 * \return      0 if successful or a specific ASN.1 or MPI error code.
 */
EXP_API int asn1_get_mpi(byte_t **p,
                  const byte_t *end,
                  mpi *X );

/**
 * \brief       Retrieve an AlgorithmIdentifier ASN.1 sequence.
 *              Updates the pointer to immediately behind the full
 *              AlgorithmIdentifier.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param alg   The buffer to receive the OID
 * \param params The buffer to receive the params (if any)
 *
 * \return      0 if successful or a specific ASN.1 or MPI error code.
 */
EXP_API int asn1_get_alg(byte_t **p,
                  const byte_t *end,
                  asn1_buf *alg, asn1_buf *params );

/**
 * \brief       Retrieve an AlgorithmIdentifier ASN.1 sequence with NULL or no
 *              params.
 *              Updates the pointer to immediately behind the full
 *              AlgorithmIdentifier.
 *
 * \param p     The position in the ASN.1 data
 * \param end   End of data
 * \param alg   The buffer to receive the OID
 *
 * \return      0 if successful or a specific ASN.1 or MPI error code.
 */
EXP_API int asn1_get_alg_null(byte_t **p,
                       const byte_t *end,
                       asn1_buf *alg );

/**
 * \brief       Find a specific named_data entry in a sequence or list based on
 *              the OID.
 *
 * \param list  The list to seek through
 * \param oid   The OID to look for
 * \param len   Size of the OID
 *
 * \return      NULL if not found, or a pointer to the existing entry.
 */
EXP_API asn1_named_data *asn1_find_named_data(asn1_named_data *list,
                                       const char *oid, dword_t len );

/**
 * \brief       Free a asn1_named_data entry
 *
 * \param entry The named data entry to free
 */
EXP_API void asn1_free_named_data(asn1_named_data *entry);

/**
 * \brief       Free all entries in a asn1_named_data list
 *              Head will be set to NULL
 *
 * \param head  Pointer to the head of the list of named data entries to free
 */
EXP_API void asn1_free_named_data_list(asn1_named_data **head);

/**
 * \brief           Write a length field in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param len       The length value to write.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_len(byte_t **p, byte_t *start,
                            dword_t len );
/**
 * \brief           Write an ASN.1 tag in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param tag       The tag to write.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_tag(byte_t **p, byte_t *start,
                            byte_t tag );

/**
 * \brief           Write raw buffer data.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param buf       The data buffer to write.
 * \param size      The length of the data buffer.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_raw_buffer(byte_t **p, byte_t *start,
                                   const byte_t *buf, dword_t size );

/**
 * \brief           Write a arbitrary-precision number (#MBEDTLS_ASN1_INTEGER)
 *                  in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param X         The MPI to write.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_mpi(byte_t **p, byte_t *start,
                            const mpi *X );

/**
 * \brief           Write a NULL tag (#MBEDTLS_ASN1_NULL) with zero data
 *                  in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_null(byte_t **p, byte_t *start);

/**
 * \brief           Write an OID tag (#MBEDTLS_ASN1_OID) and data
 *                  in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param oid       The OID to write.
 * \param oid_len   The length of the OID.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_oid(byte_t **p, byte_t *start,
                            const char *oid, dword_t oid_len );

/**
 * \brief           Write an AlgorithmIdentifier sequence in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param oid       The OID of the algorithm to write.
 * \param oid_len   The length of the algorithm's OID.
 * \param par_len   The length of the parameters, which must be already written.
 *                  If 0, NULL parameters are added
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_algorithm_identifier(byte_t **p,
                                             byte_t *start,
                                             const char *oid, dword_t oid_len,
                                             dword_t par_len );

/**
 * \brief           Write a boolean tag (#MBEDTLS_ASN1_BOOLEAN) and value
 *                  in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param boolean   The boolean value to write, either \c 0 or \c 1.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_bool(byte_t **p, byte_t *start,
                             int boolean );

/**
 * \brief           Write an int tag (#MBEDTLS_ASN1_INTEGER) and value
 *                  in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param val       The integer value to write.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative \c MBEDTLS_ERR_ASN1_XXX error code on failure.
 */
EXP_API int asn1_write_int(byte_t **p, byte_t *start, int val);

/**
 * \brief           Write a string in ASN.1 format using a specific
 *                  string encoding tag.

 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param tag       The string encoding tag to write, e.g.
 *                  #MBEDTLS_ASN1_UTF8_STRING.
 * \param text      The string to write.
 * \param text_len  The length of \p text in bytes (which might
 *                  be strictly larger than the number of characters).
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative error code on failure.
 */
EXP_API int asn1_write_tagged_string(byte_t **p, byte_t *start,
                                      int tag, const char *text,
                                      dword_t text_len );

/**
 * \brief           Write a string in ASN.1 format using the PrintableString
 *                  string encoding tag (#MBEDTLS_ASN1_PRINTABLE_STRING).
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param text      The string to write.
 * \param text_len  The length of \p text in bytes (which might
 *                  be strictly larger than the number of characters).
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative error code on failure.
 */
EXP_API int asn1_write_printable_string(byte_t **p,
                                         byte_t *start,
                                         const char *text, dword_t text_len );

/**
 * \brief           Write a UTF8 string in ASN.1 format using the UTF8String
 *                  string encoding tag (#MBEDTLS_ASN1_PRINTABLE_STRING).
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param text      The string to write.
 * \param text_len  The length of \p text in bytes (which might
 *                  be strictly larger than the number of characters).
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative error code on failure.
 */
EXP_API int asn1_write_utf8_string(byte_t **p, byte_t *start,
                                    const char *text, dword_t text_len );

/**
 * \brief           Write a string in ASN.1 format using the IA5String
 *                  string encoding tag (#MBEDTLS_ASN1_IA5_STRING).
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param text      The string to write.
 * \param text_len  The length of \p text in bytes (which might
 *                  be strictly larger than the number of characters).
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative error code on failure.
 */
EXP_API int asn1_write_ia5_string(byte_t **p, byte_t *start,
                                   const char *text, dword_t text_len );

/**
 * \brief           Write a bitstring tag (#MBEDTLS_ASN1_BIT_STRING) and
 *                  value in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param buf       The bitstring to write.
 * \param bits      The total number of bits in the bitstring.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative error code on failure.
 */
EXP_API int asn1_write_bitstring(byte_t **p, byte_t *start,
                                  const byte_t *buf, dword_t bits );

/**
 * \brief           Write an octet string tag (#MBEDTLS_ASN1_OCTET_STRING)
 *                  and value in ASN.1 format.
 *
 * \note            This function works backwards in data buffer.
 *
 * \param p         The reference to the current position pointer.
 * \param start     The start of the buffer, for bounds-checking.
 * \param buf       The buffer holding the data to write.
 * \param size      The length of the data buffer \p buf.
 *
 * \return          The number of bytes written to \p p on success.
 * \return          A negative error code on failure.
 */
EXP_API int asn1_write_octet_string(byte_t **p, byte_t *start,
                                     const byte_t *buf, dword_t size );

/**
 * \brief           Create or find a specific named_data entry for writing in a
 *                  sequence or list based on the OID. If not already in there,
 *                  a new entry is added to the head of the list.
 *                  Warning: Destructive behaviour for the val data!
 *
 * \param list      The pointer to the location of the head of the list to seek
 *                  through (will be updated in case of a new entry).
 * \param oid       The OID to look for.
 * \param oid_len   The size of the OID.
 * \param val       The data to store (can be \c NULL if you want to fill
 *                  it by hand).
 * \param val_len   The minimum length of the data buffer needed.
 *
 * \return          A pointer to the new / existing entry on success.
 * \return          \c NULL if if there was a memory allocation error.
 */
EXP_API asn1_named_data *asn1_store_named_data(asn1_named_data **list,
                                        const char *oid, dword_t oid_len,
                                        const byte_t *val,
                                        dword_t val_len );




#ifdef __cplusplus
}
#endif


#endif /*_ASN1_H*/
