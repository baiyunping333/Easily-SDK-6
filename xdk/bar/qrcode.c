/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc qrcode document

	@module	qrcode.c | implement file

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

#include "qrcode.h"
#include "qrencode.h"

#define DEF_QRVER		7
#define DEF_QRLEN		128


dword_t qr_encode(const byte_t* token, dword_t len, byte_t* buf, dword_t max, int* prows, int* pcols)
{
	QRcode *qrc;
	int rows, cols;
	int i, j;
	dword_t size = 0;
	byte_t c, b;

	len = (len < DEF_QRLEN) ? len : DEF_QRLEN;

	qrc = QRcode_encodeData(len, (unsigned char*)token, DEF_QRVER, QR_ECLEVEL_M);

	if (!qrc)
		return 0;

	rows = qrc->width;

	if (rows % 8)
		cols = rows / 8 + 1;
	else
		cols = rows / 8;

	if (prows)
	{
		*prows = rows;
	}

	if (pcols)
	{
		*pcols = cols;
	}

	if (!buf)
		return rows * cols;

	for (i = 0; i < rows; i++)
	{
		for (j = 0; j < rows;)
		{
			c = 0;
			b = 0x80;

			while (b && j < rows)
			{
				if ((*(qrc->data + i * rows + j)) & 0x01)
					c |= b;

				b = b >> 1;
				j++;
			}
			buf[size++] = c;
		}
	}

	QRcode_free(qrc);

	return size;
}