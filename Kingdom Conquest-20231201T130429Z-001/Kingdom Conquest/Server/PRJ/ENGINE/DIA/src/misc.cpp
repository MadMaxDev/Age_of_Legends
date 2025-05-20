// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: misc.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include <stdio.h>
#include <string.h>
#include "../inc/misc.h"

//////////////////////////////////////////////////////////////////////////
// platform about network
int dia_network_init()
{
#ifdef WIN32
	WSADATA info;
	if (::WSAStartup(MAKEWORD(2,0), &info))
	{
		return -1;
	}
#endif
	return 0;
}

void dia_network_fini()
{
#ifdef WIN32
	::WSACleanup();
#endif
}

//////////////////////////////////////////////////////////////////////////
//
dia_uint64_t get_cpu_freq()
{
	dia_uint64_t t1, t2;
	t1 = get_jiffies_fromboot();
#ifdef __GNUC__
	struct timespec req;
	req.tv_sec = 1;
	req.tv_nsec = 0LL;
	nanosleep(&req, NULL);
#else
	Sleep(1000);
#endif
	t2 = get_jiffies_fromboot();
	return t2 - t1;
}

dia_uint32_t get_millisec_tick()
{
#ifdef __GNUC__
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (dia_uint32_t)(tv.tv_sec*1000 + tv.tv_usec/1000);
#else
	return timeGetTime();
#endif
}

//////////////////////////////////////////////////////////////////////////
//
dia_int32_t get_millisec_diff(dia_uint32_t t1, dia_uint32_t t2)
{
	return dia_int32_t(t1 - t2);
}

//////////////////////////////////////////////////////////////////////////
//

const char* dia_ftoa(double value, char *buf)
{
	sprintf(buf, "%G", value);
	return buf;
}

const char *dia_itoa(int value, char *buf, int type)
{
	if (type == 10)
		sprintf(buf, "%d", value);
	else if (type == 8)
		sprintf(buf, "%o", value);
	else if (type == 16)
		sprintf(buf, "%x", value);
	else
		return NULL;
	return buf;
}

const char *dia_ltoa(unsigned int value, char *buf, int type)
{
	if (type == 10)
		sprintf(buf, "%u", value);
	else if (type == 8)
		sprintf(buf, "%o", value);
	else if (type == 16)
		sprintf(buf, "%x", value);
	else
		return NULL;
	return buf;
}

const char* dia_64toa(dia_uint64_t value, char *buf, int type)
{
#ifdef __GNUC__
	if (type == 10)
		sprintf(buf, "%lld", value);
	else if (type == 8)
		sprintf(buf, "%llo", value);
	else if (type == 16)
		sprintf(buf, "%llx", value);
	else
		return NULL;
#else
	if (type == 10)
		sprintf(buf, "%I64d", value);
	else if (type == 8)
		sprintf(buf, "%I64o", value);
	else if (type == 16)
		sprintf(buf, "%I64x", value);
	else 
		return NULL;
#endif
	return buf;
}



//////////////////////////////////////////////////////////////////////////
int split_n(const char* src, char dlt)
{
	int n = 0;
	while(*src)
		if (*src++ == dlt) n++; 
	n = *(--src)==dlt? n: n+1;
	return n;
}
int split_ptr(const char* src, int idx, char dlt, char* dest, int srcLen)
{
	const char* ptr = src + idx;
	if (!*ptr)
		return 0;

	if (srcLen == 0)
	{
		while (*ptr)
			if (*ptr++ == dlt) break;

		if (*ptr)
		{
			memcpy(dest, src+idx, ptr-src-idx-1);
			dest[ptr-src-idx-1] = 0;
		}
		else
		{
			memcpy(dest, src+idx, ptr-src-idx);
			dest[ptr-src-idx] = 0;
		}
		return ptr-src-idx;
	}
	else
	{
		srcLen -= idx;
		while (*ptr && srcLen-- > 0)
			if (*ptr++ == dlt) break;

		if (*ptr && srcLen >= 0)
		{
			memcpy(dest, src+idx, ptr-src-idx-1);
			dest[ptr-src-idx-1] = 0;
		}
		else
		{
			memcpy(dest, src+idx, ptr-src-idx);
			dest[ptr-src-idx] = 0;
		}
		return ptr-src-idx;
	}
}

//////////////////////////////////////////////////////////////////////////
char* dia_substring_search_sunday(const char *text, const char *patt)
{
	int n, m;
	// get the length of the text and the pattern, if necessary
	n = strlen(text);
	m = strlen(patt);
	if (m == 0)
		return (char*)text;

	// construct delta shift table
	int td[128];
	for (int c = 0; c < 128; c++)
		td[c] = m + 1;
	const char* p;
	for (p=patt; *p; p++)
		td[(unsigned char)*p] = m - (p - patt);

	// start searching...
	const char *t, *tx = text;

	// the main searching loop
	while (tx + m <= text + n) {
		for (p = patt, t = tx; *p; ++p, ++t) {
			if (*p != *t) // found a mismatch
				break;
		}
		if (*p == 0) // Yes! we found it!
			return (char*)tx;
		tx += td[(unsigned char)tx[m]]; // move the pattern by a distance
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

int dia_get_code_page(const char *codepage)
{
#ifdef WIN32
	return CP_ACP;
#else
	return -1;
#endif
}

int dia_multi_2_wchar(const char *codepage, const char *inBuf, int inLen, wchar_t *outBuf, int outLen)
{
#ifdef WIN32
	return ::MultiByteToWideChar(dia_get_code_page(codepage), 0, inBuf, inLen, outBuf, outLen);
#else
	int charOutLen = outLen * sizeof(wchar_t);
	int tmpLen = charOutLen;
	iconv_t handle = iconv_open("WCHAR_T", codepage);
	if (handle == (iconv_t)(-1))
	{
		return -1;
	}

	size_t result = iconv(handle, (char**)&inBuf, (size_t*)&inLen, (char**)&outBuf, (size_t*)&charOutLen);
	if (result != 0)
	{
		iconv_close(handle);
		return -2;
	}
	
	result = tmpLen - charOutLen;
	iconv_close(handle);
	result /= sizeof(wchar_t);
	return (int)result;
#endif
}

int dia_wchar_2_multi(const char *codepage, const wchar_t *inBuf, int inLen, char *outBuf, int outLen)
{
#ifdef WIN32
	return ::WideCharToMultiByte(dia_get_code_page(codepage), 0, inBuf, inLen, outBuf, outLen, NULL, NULL);
#else
	int tmpLen = outLen;
	int tmpInLen = inLen * sizeof(wchar_t);
	iconv_t handle = iconv_open(codepage, "WCHAR_T");
	if (handle == (iconv_t)(-1))
	{
		return -1;
	}

	size_t result = iconv(handle, (char**)&inBuf, (size_t*)&tmpInLen, (char**)&outBuf, (size_t*)&outLen);
	if (result != 0)
	{
		iconv_close(handle);
		return -2;
	}

	result = tmpLen - outLen;
	iconv_close(handle);
	return (int)result;
#endif
}

int dia_wchar_2_utf8(const wchar_t* inBuf, int inLen, char* outBuf, int outLen)
{
#ifdef WIN32
	return ::WideCharToMultiByte(CP_UTF8, 0, inBuf, inLen, outBuf, outLen, NULL, NULL);
#else
	int tmpLen = outLen;
	int tmpInLen = inLen * sizeof(wchar_t);
	iconv_t handle = iconv_open("UTF8", "WCHAR_T");
	if (handle == (iconv_t)(-1))
	{
		return -1;
	}

	size_t result = iconv(handle, (char**)&inBuf, (size_t*)&tmpInLen, (char**)&outBuf, (size_t*)&outLen);
	if (result != 0)
	{
		iconv_close(handle);
		return -2;
	}

	result = tmpLen - outLen;
	iconv_close(handle);
	return (int)result;
#endif
}

int dia_utf8_2_wchar(const char* inBuf, int inLen, wchar_t* outBuf, int outLen)
{
#ifdef WIN32
	return ::MultiByteToWideChar(CP_UTF8, 0, inBuf, inLen, outBuf, outLen);
#else
	int charOutLen = outLen * sizeof(wchar_t);
	int tmpLen = charOutLen;
	iconv_t handle = iconv_open("WCHAR_T", "UTF8");
	if (handle == (iconv_t)(-1))
	{
		return -1;
	}

	size_t result = iconv(handle, (char**)&inBuf, (size_t*)&inLen, (char**)&outBuf, (size_t*)&charOutLen);
	if (result != 0)
	{
		iconv_close(handle);
		return -2;
	}

	result = tmpLen - charOutLen;
	iconv_close(handle);
	result /= sizeof(wchar_t);
	return (int)result;
#endif
}

int dia_utf8_2_multi(const char* codepage, const char* inBuf, int inLen, char* outBuf, int outLen)
{
	int tmpLen = 0;
	wchar_t* tmpBuf = new wchar_t[inLen];

	tmpLen = dia_utf8_2_wchar(inBuf, inLen, tmpBuf, inLen);
	if (tmpLen < 0)
	{
		delete [] tmpBuf;
		return -1;
	}

	tmpLen = dia_wchar_2_multi(codepage, tmpBuf, tmpLen, outBuf, outLen);
	if (tmpLen < 0)
	{
		delete [] tmpBuf;
		return -1;
	}

	delete [] tmpBuf;
	return tmpLen;
}

int dia_multi_2_utf8(const char* codepage, const char* inBuf, int inLen, char* outBuf, int outLen)
{
	int tmpLen = 0;
	wchar_t* tmpBuf = new wchar_t[inLen];

	tmpLen = dia_multi_2_wchar(codepage, inBuf, inLen, tmpBuf, inLen);
	if (tmpLen < 0)
	{
		delete [] tmpBuf;
		return -1;
	}

	tmpLen = dia_wchar_2_utf8(tmpBuf, tmpLen, outBuf, outLen);
	if (tmpLen < 0)
	{
		delete [] tmpBuf;
		return -1;
	}

	delete [] tmpBuf;
	return tmpLen;
}

//////////////////////////////////////////////////////////////////////////
int dia_utf8_stream_wc(void *str)
{
	int offset, wc;
	unsigned char c, len;
	offset = wc = 0;
	while (1)
	{
		if (!*((unsigned char*)str + offset))
			return wc;

		c = *((unsigned char*)str + offset);
		if (!(c & 0x80))
			len = 1;
		else if (c >= 0xFC)
			len = 6;
		else if (c >= 0xF8)
			len = 5;
		else if (c >= 0xF0)
			len = 4;
		else if (c >= 0xE0)
			len = 3;
		else if (c >= 0xC0)
			len = 2;
		else
			return -1;

		++wc;
		offset += len;
	}
}

int dia_utf8_stream_pop_decimal(void *str, int &offset, dia_uint32_t &val)
{
	if (!*((unsigned char*)str + offset))
		return -1;

	char tmp[3], buf[64];
	unsigned char c, len;
	buf[0] = 0;

	c = *((unsigned char*)str + offset);
	if (!(c & 0x80))
		len = 1;
	else if (c >= 0xFC)
		len = 6;
	else if (c >= 0xF8)
		len = 5;
	else if (c >= 0xF0)
		len = 4;
	else if (c >= 0xE0)
		len = 3;
	else if (c >= 0xC0)
		len = 2;
	else
		return -1;

	while (len)
	{
		c = *((unsigned char*)str + offset);
		sprintf(tmp, "%x", c);
		strcat(buf, tmp);

		++offset;
		if (!--len)
			sscanf(buf, "%x", &val);
	}
	return 0;
}
int dia_utf8_stream_pop_hex(void *str, int &offset, char *val)
{
	if (!*((unsigned char*)str + offset))
		return -1;

	char tmp[3];
	unsigned char c, len;

	c = *((unsigned char*)str + offset);
	if (!(c & 0x80))
		len = 1;
	else if (c >= 0xFC)
		len = 6;
	else if (c >= 0xF8)
		len = 5;
	else if (c >= 0xF0)
		len = 4;
	else if (c >= 0xE0)
		len = 3;
	else if (c >= 0xC0)
		len = 2;
	else
		return -1;

	while (len)
	{
		c = *((unsigned char*)str + offset);
		sprintf(tmp, "%x", c);
		strcat(val, tmp);

		++offset;
		--len;
	}
	return 0;
}



