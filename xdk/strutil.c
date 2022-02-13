/***********************************************************************
	Easily SDK v6.0

	(c) 2013-2016 JianDe LiFang Technology Corporation.  All Rights Reserved.

	@author ZhangWenQuan, JianDe HangZhou ZheJiang China, Mail: powersuite@hotmaol.com

	@doc str utility document

	@module	strutil.c | implement file

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

#include "strutil.h"

#include "xdkimp.h"
#include "xdkstd.h"

#define A_IS_TOKEN_SPLIT(ch) (ch == (' ') || ch == ('\t') || ch == ('\n') || ch == (',') || ch == (';') || ch == ('~') || ch == (':'))
#define W_IS_TOKEN_SPLIT(ch) (ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L',' || ch == L';' || ch == L'~' || ch == L':')

#define A_IS_BRACKET_LEFT(ch) (ch == ('(') || ch == ('{') || ch == ('['))
#define A_IS_BRACKET_RIGHT(ch) (ch == (')') || ch == ('}') || ch == (']'))
#define W_IS_BRACKET_LEFT(ch) (ch == L'(' || ch == L'{' || ch == L'[')
#define W_IS_BRACKET_RIGHT(ch) (ch == L')' || ch == L'}' || ch == L']')

int w_parse_attrset_token(const wchar_t* attrset, int len, wchar_t** keyptr, int* keylen, wchar_t** valptr, int* vallen)
{
	const wchar_t* token;
	int total = 0;

	*keyptr = *valptr = NULL;
	*keylen = *vallen = 0;

	if (len < 0)
		len = w_xslen(attrset);

	if (w_is_null(attrset) || !len)
		return 0;

	token = attrset;

	*keyptr = (wchar_t*)token;
	while (*token != L' ' && *token != L'\t' && *token != L'=' && total < len)
	{
		token++;
		total++;
	}
	*keylen = (int)(token - *keyptr);

	if (total == len)
		return total;

	//skip ' ','='
	while (*token != L'\'' && *token != L'\"' && total < len)
	{
		token++;
		total++;
	}

	if (total == len)
		return total;

	//skip '\'','\"'
	token++;
	total++;

	*valptr = (wchar_t*)token;
	while (*token != L'\'' && *token != L'\"' && total < len)
	{
		token++;
		total++;
	}
	*vallen = (int)(token - *valptr);

	//skip ' ','\'','\"'
	while (*token == L'\'' || *token == L'\"' || *token == L' ' || *token == L'\t')
	{
		token++;
		total++;
	}

	return total;
}

int a_parse_attrset_token(const schar_t* attrset, int len, schar_t** keyptr, int* keylen, schar_t** valptr, int* vallen)
{
	const schar_t* token;
	int total = 0;

	*keyptr = *valptr = NULL;
	*keylen = *vallen = 0;

	if (len < 0)
		len = a_xslen(attrset);

	if (a_is_null(attrset) || !len)
		return 0;

	token = attrset;

	*keyptr = (schar_t*)token;
	while (*token != (' ') && *token != ('\t') && *token != ('=') && total < len)
	{
		token++;
		total++;
	}
	*keylen = (int)(token - *keyptr);

	if (total == len)
		return total;

	//skip ' ','='
	while (*token != ('\'') && *token != ('\"') && total < len)
	{
		token++;
		total++;
	}

	if (total == len)
		return total;

	//skip '\'','\"'
	token++;
	total++;

	*valptr = (schar_t*)token;
	while (*token != ('\'') && *token != ('\"') && total < len)
	{
		token++;
		total++;
	}
	*vallen = (int)(token - *valptr);

	//skip ' ','\'','\"'
	while (*token == ('\'') || *token == ('\"') || *token == (' ') || *token == ('\t'))
	{
		token++;
		total++;
	}

	return total;
}

int w_parse_attrset_token_count(const wchar_t* attrset, int len)
{
	int count = 0;
	wchar_t *key, *val;
	int klen, vlen;
	int n, total = 0;

	if (len < 0)
		len = w_xslen(attrset);

	if (w_is_null(attrset) || !len)
		return 0;

	while (n = w_parse_attrset_token((attrset + total), (len - total), &key, &klen, &val, &vlen))
	{
		total += n;
		count++;
	}

	return count;
}

int a_parse_attrset_token_count(const schar_t* attrset, int len)
{
	int count = 0;
	schar_t *key, *val;
	int klen, vlen;
	int n, total = 0;

	if (len < 0)
		len = a_xslen(attrset);

	if (a_is_null(attrset) || !len)
		return 0;

	while (n = a_parse_attrset_token((attrset + total), (len - total), &key, &klen, &val, &vlen))
	{
		total += n;
		count++;
	}

	return count;
}

int w_parse_options_token(const wchar_t* options, int len, wchar_t itemfeed, wchar_t linefeed, wchar_t** keyptr, int* keylen, wchar_t** valptr, int* vallen)
{
	const wchar_t* token;
	int total = 0;

	*keyptr = *valptr = NULL;
	*keylen = *vallen = 0;

	if (len < 0)
		len = w_xslen(options);

	if (w_is_null(options) || !len)
		return 0;

	token = options;
	/*skip blank*/
	while ((*token == L' ' || *token == L'\t') && total < len)
	{
		token++;
		total++;
	}

	*keyptr = (wchar_t*)token;
	while (*token != itemfeed && *token != L'\0' && total < len)
	{
		token++;
		total++;
	}
	*keylen = (int)(token - *keyptr);

	if (total == len)
		return total;

	if (*token == itemfeed)
	{
		token++; /*skip item feed*/
		total++;
	}

	/*skip blank*/
	while ((*token == L' ' || *token == L'\t') && total < len)
	{
		token++;
		total++;
	}

	if (total == len)
		return total;

	*valptr = (wchar_t*)token;
	while (*token != linefeed && *token != L'\0' && total < len)
	{
		token++;
		total++;
	}
	*vallen = (int)(token - *valptr);

	while (*token == linefeed || *token == L'\n')
	{
		token++;
		total++;
	}

	return total;
}

int a_parse_options_token(const schar_t* options, int len, schar_t itemfeed, schar_t linefeed, schar_t** keyptr, int* keylen, schar_t** valptr, int* vallen)
{
	const schar_t* token;
	int total = 0;

	*keyptr = *valptr = NULL;
	*keylen = *vallen = 0;

	if (len < 0)
		len = a_xslen(options);

	if (a_is_null(options) || !len)
		return 0;

	token = options;
	/*skip blank*/
	while ((*token == (' ') || *token == ('\t')) && total < len)
	{
		token++;
		total++;
	}

	*keyptr = (schar_t*)token;
	while (*token != itemfeed && *token != ('\0') && total < len)
	{
		token++;
		total++;
	}
	*keylen = (int)(token - *keyptr);

	if (total == len)
		return total;

	if (*token == itemfeed)
	{
		token++; /*skip item feed*/
		total++;
	}

	/*skip blank*/
	while ((*token == (' ') || *token == ('\t')) && total < len)
	{
		token++;
		total++;
	}

	if (total == len)
		return total;

	*valptr = (schar_t*)token;
	while (*token != linefeed && *token != ('\0') && total < len)
	{
		token++;
		total++;
	}
	*vallen = (int)(token - *valptr);

	while (*token == linefeed || *token == ('\n'))
	{
		token++;
		total++;
	}

	return total;
}

int w_parse_options_token_count(const wchar_t* options,int len,wchar_t itemfeed,wchar_t linefeed)
{
	int count = 0;
	wchar_t *key,*val;
	int klen,vlen;
	int n, total = 0;

	if (len < 0)
		len = w_xslen(options);

	if (w_is_null(options) || !len)
		return 0;

	while(n = w_parse_options_token((options + total),(len - total),itemfeed,linefeed,&key,&klen,&val,&vlen))
	{
		total += n;
		count ++;
	}

	return count;
}

int a_parse_options_token_count(const schar_t* options, int len, schar_t itemfeed, schar_t linefeed)
{
	int count = 0;
	schar_t *key, *val;
	int klen, vlen;
	int n, total = 0;

	if (len < 0)
		len = a_xslen(options);

	if (a_is_null(options) || !len)
		return 0;

	while (n = a_parse_options_token((options + total), (len - total), itemfeed, linefeed, &key, &klen, &val, &vlen))
	{
		total += n;
		count++;
	}

	return count;
}

int w_get_options_value(const wchar_t* options, int len, wchar_t itemfeed, wchar_t linefeed, const wchar_t* pkey, wchar_t* buf, int max)
{
	int count = 0;
	wchar_t *key, *val;
	int klen, vlen;
	int n, total = 0;

	if (len < 0)
		len = w_xslen(options);

	if (w_is_null(options) || !len)
		return 0;

	while (n = w_parse_options_token((options + total), (len - total), itemfeed, linefeed, &key, &klen, &val, &vlen))
	{
		total += n;

		if (w_xsncmp(key, pkey, klen) == 0)
		{
			max = (max < vlen) ? max : vlen;
			if (buf)
			{
				w_xsncpy(buf, val, max);
			}
			return max;
		}
	}

	return 0;
}

int a_get_options_value(const schar_t* options, int len, schar_t itemfeed, schar_t linefeed, const schar_t* pkey, schar_t* buf, int max)
{
	int count = 0;
	schar_t *key, *val;
	int klen, vlen;
	int n, total = 0;

	if (len < 0)
		len = a_xslen(options);

	if (a_is_null(options) || !len)
		return 0;

	while (n = a_parse_options_token((options + total), (len - total), itemfeed, linefeed, &key, &klen, &val, &vlen))
	{
		total += n;

		if (a_xsncmp(key, pkey, klen) == 0)
		{
			max = (max < vlen) ? max : vlen;
			if (buf)
			{
				a_xsncpy(buf, val, max);
			}
			return max;
		}
	}

	return 0;
}

int w_parse_string_token(const wchar_t* tokens,int len, wchar_t itemfeed, wchar_t** pkey, int* pkeylen)
{
	const wchar_t* token;
	int total = 0;
	bool_t bracket = 0;

	*pkey = NULL;
	*pkeylen = 0;

	if (len < 0)
		len = w_xslen(tokens);

	if(w_is_null(tokens) || !len)
		return 0;

	token = tokens;

	*pkey = (wchar_t*)token;
	while((!bracket && *token != itemfeed) || bracket)
	{
		if (W_IS_BRACKET_LEFT(*token))
			bracket = 1;
		else if(W_IS_BRACKET_RIGHT(*token))
			bracket = 0;

		token ++;
		total++;

		if (total >= len)
			break;
	}
	*pkeylen = (int)(token - *pkey);

	if(total == len)
		return total;

	token ++; //skip item feed
	total++;

	return total;
}

int a_parse_string_token(const schar_t* tokens, int len, schar_t itemfeed, schar_t** pkey, int* pkeylen)
{
	const schar_t* token;
	int total = 0;
	bool_t bracket = 0;

	*pkey = NULL;
	*pkeylen = 0;

	if (len < 0)
		len = a_xslen(tokens);

	if (a_is_null(tokens) || !len)
		return 0;

	token = tokens;

	*pkey = (schar_t*)token;
	while ((!bracket && *token != itemfeed) || bracket)
	{
		if (A_IS_BRACKET_LEFT(*token))
			bracket = 1;
		else if (A_IS_BRACKET_RIGHT(*token))
			bracket = 0;

		token++;
		total++;

		if (total >= len)
			break;
	}
	*pkeylen = (int)(token - *pkey);

	if (total == len)
		return total;

	token++; //skip item feed
	total++;

	return total;
}

int w_parse_string_token_count(const wchar_t* tokens, int len, wchar_t itemfeed)
{
	int count = 0;
	wchar_t *key;
	int klen;
	int n, total = 0;

	if (len < 0)
		len = w_xslen(tokens);

	if (w_is_null(tokens) || !len)
		return 0;

	while(n = w_parse_string_token((tokens + total),(len - total),itemfeed,&key,&klen))
	{
		total += n;
		count ++;
	}

	return count;
}

int a_parse_string_token_count(const schar_t* tokens, int len, schar_t itemfeed)
{
	int count = 0;
	schar_t *key;
	int klen;
	int n, total = 0;

	if (len < 0)
		len = a_xslen(tokens);

	if (a_is_null(tokens) || !len)
		return 0;

	while (n = a_parse_string_token((tokens + total), (len - total), itemfeed, &key, &klen))
	{
		total += n;
		count++;
	}

	return count;
}

int a_parse_zero_token(const schar_t* tokens, schar_t** pkey, int* pkeylen)
{
	*pkey = NULL;
	*pkeylen = 0;

	if (!tokens)
		return 0;

	*pkey = (schar_t*)tokens;
	*pkeylen = a_xslen(tokens);

	return (*pkeylen + 1);
}

int a_parse_zero_token_count(const schar_t* tokens)
{
	const schar_t* token;
	int len, total = 0;

	if (!tokens)
		return 0;

	token = tokens;

	do
	{
		len = a_xslen(token);
		token += (len + 1);
		
		total++;
	} while (*token != '\0');

	return total;
}

int w_parse_zero_token(const wchar_t* tokens, wchar_t** pkey, int* pkeylen)
{
	*pkey = NULL;
	*pkeylen = 0;

	if (!tokens)
		return 0;

	*pkey = (wchar_t*)tokens;
	*pkeylen = w_xslen(tokens);

	return (*pkeylen + 1);
}

int w_parse_zero_token_count(const wchar_t* tokens)
{
	const wchar_t* token;
	int len, total = 0;

	if (!tokens)
		return 0;

	token = tokens;

	do
	{
		len = w_xslen(token);
		token += (len + 1);

		total++;
	} while (*token != L'\0');

	return total;
}

int w_parse_param_name(const wchar_t* param, int len, wchar_t itemdot, wchar_t** pkey, int* plen)
{
	const wchar_t* token;
	int total = 0;
	int tag = 0;

	*pkey = NULL;
	*plen = 0;

	if (len < 0)
		len = w_xslen(param);

	if (w_is_null(param) || !len)
		return 0;

	token = param;

	while (total < len)
	{
		if (*token == L'\'' || *token == L'\"')
			tag = (tag == 0) ? 1 : 0;

		if (tag == 0 && *token == itemdot)
			break;

		token++;
		total++;
	}

	if (total == len)
		return total;

	if (*token == itemdot)
	{//skip ':'
		token++;
		total++;
	}

	*pkey = token;
	while (*(token + *plen) != L' ' && total < len)
	{
		total++;

		*plen = *plen + 1;
	}

	return total;
}

int a_parse_param_name(const schar_t* param, int len, schar_t itemdot, schar_t** pkey, int* plen)
{
	const schar_t* token;
	int total = 0;
	int tag = 0;

	*pkey = NULL;
	*plen = 0;

	if (len < 0)
		len = a_xslen(param);

	if (a_is_null(param) || !len)
		return 0;

	token = param;

	while (total < len)
	{
		if (*token == ('\'') || *token == ('\"'))
			tag = (tag == 0) ? 1 : 0;

		if (tag == 0 && *token == itemdot)
			break;

		token++;
		total++;
	}

	if (total == len)
		return total;

	if (*token == itemdot)
	{//skip ':'
		token++;
		total++;
	}

	*pkey = token;
	while (*(token + *plen) != (' ') && total < len)
	{
		total++;

		*plen = *plen + 1;
	}

	return total;
}

int w_parse_param_name_count(const wchar_t* param, int len, wchar_t itemdot)
{
	int count = 0;
	wchar_t* pkey;
	int plen;
	int n, total = 0;

	if (len < 0)
		len = w_xslen(param);

	if (w_is_null(param) || !len)
		return 0;

	while (n = w_parse_param_name((param + total), (len - total), itemdot, &pkey, &plen))
	{
		total += n;
		count++;
	}

	return count;
}

int a_parse_param_name_count(const schar_t* param, int len, schar_t itemdot)
{
	int count = 0;
	schar_t* pkey;
	int plen;
	int n, total = 0;

	if (len < 0)
		len = a_xslen(param);

	if (a_is_null(param) || !len)
		return 0;

	while (n = a_parse_param_name((param + total), (len - total), itemdot, &pkey, &plen))
	{
		total += n;
		count++;
	}

	return count;
}

dword_t w_parse_octet_string(const wchar_t* octet, int len, byte_t* buf, dword_t max)
{
	dword_t pos = 0;
	int total = 0;
	byte_t bh, bl;

	if (len < 0) len = w_xslen(octet);

	if (!len || w_is_null(octet))
		return 0;

	while (total < len)
	{
		if (octet[total] == L'0' && (octet[total + 1] == L'X' || octet[total + 1] == L'x'))
		{
			total += 2;
			continue;
		}

		if (octet[total] >= L'a' && octet[total] <= L'z')
		{
			bh = (octet[total] - L'a') + 10;
		}
		else if (octet[total] >= L'A' && octet[total] <= L'Z')
		{
			bh = (octet[total] - L'A') + 10;
		}
		else if (octet[total] >= L'0' && octet[total] <= L'9')
		{
			bh = (octet[total] - L'0');
		}
		else if (octet[total] == L'\0')
			return pos;

		total++;

		if (octet[total] >= L'a' && octet[total] <= L'z')
		{
			bl = (octet[total] - L'a') + 10;
		}
		else if (octet[total] >= L'A' && octet[total] <= L'Z')
		{
			bl = (octet[total] - L'A') + 10;
		}
		else if (octet[total] >= L'0' && octet[total] <= L'9')
		{
			bl = (octet[total] - L'0');
		}
		else if (octet[total] == L'\0')
			return pos;

		total++;

		if (pos + 1 > max)
			return pos;

		if (buf)
		{
			buf[pos] = ((bh << 4) | (bl & 0x0F));
		}
		pos++;
	}

	return pos;
}

dword_t a_parse_octet_string(const schar_t* octet, int len, byte_t* buf, dword_t max)
{
	dword_t pos = 0;
	int total = 0;
	byte_t bh, bl;

	if (len < 0) len = a_xslen(octet);

	if (!len || a_is_null(octet))
		return 0;

	while (total < len)
	{
		if (octet[total] == '0' && (octet[total + 1] == 'X' || octet[total + 1] == 'x'))
		{
			total += 2;
			continue;
		}

		if (octet[total] >= 'a' && octet[total] <= 'z')
		{
			bh = (octet[total] - 'a') + 10;
		}
		else if (octet[total] >= 'A' && octet[total] <= 'Z')
		{
			bh = (octet[total] - 'A') + 10;
		}
		else if (octet[total] >= '0' && octet[total] <= '9')
		{
			bh = buf[pos] = (octet[total] - '0');
		}
		else if (octet[total] == '\0')
			return pos;

		total++;

		if (octet[total] >= 'a' && octet[total] <= 'z')
		{
			bl = (octet[total] - 'a') + 10;
		}
		else if (octet[total] >= 'A' && octet[total] <= 'Z')
		{
			bl = (octet[total] - 'A') + 10;
		}
		else if (octet[total] >= '0' && octet[total] <= '9')
		{
			bl = buf[pos] = (octet[total] - '0');
		}
		else if (octet[total] == '\0')
			return pos;

		total++;

		if (pos + 1 > max)
			return pos;

		if (buf)
		{
			buf[pos] = ((bh << 4) | (bl & 0x0F));
		}
		pos++;
	}

	return pos;
}

int w_format_octet_string(const byte_t* octet, dword_t len, bool_t upper, wchar_t* buf, int max)
{
	int pos = 0;
	dword_t total = 0;
	byte_t bh, bl;

	while (total < len)
	{
		bh = (octet[total] & 0xF0) >> 4;
		bl = (octet[total] & 0x0F);

		if (pos + 2 > max)
			return pos;

		if (bh >= 0 && bh < 10)
		{
			if (buf)
			{
				buf[pos] = (bh + L'0');
			}
		}
		else
		{
			if (buf)
			{
				buf[pos] = (upper) ? (bh - 10 + L'A') : (bh - 10 + L'a');
			}
		}

		pos++;

		if (bl >= 0 && bl < 10)
		{
			if (buf)
			{
				buf[pos] = (bl + L'0');
			}
		}
		else
		{
			if (buf)
			{
				buf[pos] = (upper) ? (bl - 10 + L'A') : (bl - 10 + L'a');
			}
		}

		pos++;
		total++;
	}

	return pos;
}


int a_format_octet_string(const byte_t* octet, dword_t len, bool_t upper, schar_t* buf, int max)
{
	int pos = 0;
	dword_t total = 0;
	byte_t bh, bl;

	while (total < len)
	{
		bh = (octet[total] & 0xF0) >> 4;
		bl = (octet[total] & 0x0F);

		if (pos + 2 > max)
			return pos;

		if (bh >= 0 && bh < 10)
		{
			if (buf)
			{
				buf[pos] = (bh + '0');
			}
		}
		else
		{
			if (buf)
			{
				buf[pos] = (upper) ? (bh - 10 + 'A') : (bh - 10 + 'a');
			}
		}

		pos++;

		if (bl >= 0 && bl < 10)
		{
			if (buf)
			{
				buf[pos] = (bl + '0');
			}
		}
		else
		{
			if (buf)
			{
				buf[pos] = (upper) ? (bl - 10 + 'a') : (bl - 10 + 'a');
			}
		}

		pos++;
		total++;
	}

	return pos;
}