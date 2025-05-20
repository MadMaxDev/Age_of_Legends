// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: misc.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_MISC_H__
#define __DIA_MISC_H__

#include <time.h>
#include "numeric_type.h"

#ifdef __GNUC__
#include <iconv.h>
#endif

//////////////////////////////////////////////////////////////////////////
// platform relative about network
int dia_network_init();
void dia_network_fini();

//////////////////////////////////////////////////////////////////////////
// 
inline dia_uint64_t get_jiffies_fromboot()
{
#ifdef __GNUC__
	dia_uint32_t l,h;
	dia_uint64_t ret;
	__asm__ __volatile__("RDTSC" : "=a"(l), "=d"(h));
	ret = h;
	ret	= (ret << 32) | l;
	return ret;
#else
	__asm RDTSC
#endif
}

//////////////////////////////////////////////////////////////////////////
// 
dia_uint64_t get_cpu_freq();
dia_uint32_t get_millisec_tick();

// 32bit: time-event that interval less than 24.85 days
dia_int32_t get_millisec_diff(dia_uint32_t t1, dia_uint32_t t2);

//////////////////////////////////////////////////////////////////////////
#ifdef __GNUC__
#define dia_ato64 atoll
#else
#define dia_ato64 _atoi64
#endif

const char* dia_ftoa(double value, char *buf);
const char* dia_itoa(int value, char *buf, int type);
const char* dia_ltoa(unsigned int value, char *buf, int type);
const char* dia_64toa(dia_uint64_t value, char *buf, int type);

//////////////////////////////////////////////////////////////////////////
int split_n(const char *src, char dlt);
int split_ptr(const char* src, int idx, char dlt, char* dest, int srcLen);

//////////////////////////////////////////////////////////////////////////
// substring search algorithm, call me 'sunday'
// REFERENCES: http://xushiwei.blogspot.com/2006/08/computer-scienceboyer-moore-kmpknuth.html
char* dia_substring_search_sunday(const char *text, const char *patt);

//////////////////////////////////////////////////////////////////////////
int dia_get_code_page(const char *codepage);

int dia_multi_2_wchar(const char *codepage, const char *inBuf, int inLen, wchar_t *outBuf, int outLen);
int dia_wchar_2_multi(const char *codepage, const wchar_t *inBuf, int inLen, char *outBuf, int outLen);

int dia_wchar_2_utf8(const wchar_t* inBuf, int inLen, char* outBuf, int outLen);
int dia_utf8_2_wchar(const char* inBuf, int inLen, wchar_t* outBuf, int outLen);

int dia_utf8_2_multi(const char* codepage, const char* inBuf, int inLen, char* outBuf, int outLen);
int dia_multi_2_utf8(const char* codepage, const char* inBuf, int inLen, char* outBuf, int outLen);


//////////////////////////////////////////////////////////////////////////
// 从字节流里取单字的UTF8编码
int dia_utf8_stream_wc(void *str);
int dia_utf8_stream_pop_decimal(void *str, int &offset, dia_uint32_t &val);
int dia_utf8_stream_pop_hex(void *str, int &offset, char *val);


//////////////////////////////////////////////////////////////////////////
#endif

