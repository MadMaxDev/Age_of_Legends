////////////////////
//	EmString.h		:	V0020
//	Written By		:	miss, Liu Gang
//	Compiler		:	Microsoft Visual C++ V5.0/6.0
//	Copyright (C)	:	1996-1999 EternalMoment Studio. All Rights Reserved.
//	V0010			:	( 1999.01.29 ) Created by miss
//	V0011			:	( 1999.02.06 ) Fixed some bugs
//	V0020			:	Oct.28.1999, Changed by Liu Gang. Add some new functions for filenames
////////////////////
// 字符串处理函数库
//		函数参数处理函数
//		文件名处理函数
//		文件处理函数
////////////////////


#ifndef		__EMSTRING_H__
#define		__EMSTRING_H__
////////////////////


////////////////////
#include	"DMtype.h"		// BOOL
////////////////////


////////////////////
//----------------Function parameter Functions-----------------
//		函数参数处理函数
////////////////////
// 得到字符串包含的段数
// 该字符串中间以任意多个分隔符分隔
// 注意：当有连续多个连续的分隔符时，被认为是一个分隔符
// lpString		:	被查询的字符串
// szChar		:	分隔符，缺省以空格分隔
int GetSegmentCount( const char *lpString, char szChar=' ' );
// 得到字符串包含的段数
// 该字符串中间以任意多个分隔符分隔
// 注意：每出现一个分隔符，就是输出一个字符串
// lpString		:	被查询的字符串
// szChar		:	分隔符，缺省以空格分隔
int GetSegmentCountTable( const char *lpString, char szChar=' ' );

// 剪切字符串
// 把源字符串中第某段的字符串剪切到目的字符串中
// 注意：当有连续多个连续的分隔符时，被认为是一个分隔符
// lpString		:	源字符串
// szString		:	目的字符串
// nSize		:	目的字符串的长度，如果长度小于目的字符串(szString)的真实长度，则会出现致命错误
// nSegment		:	将源字符串的第几段放到目的字符串中
// szChar		:	分隔符，缺省以空格分隔
BOOL CutString( const char *lpString, char *szString, int nSize, int nSegment, char szChar = ' ' );

// 剪切字符串
// 把源字符串中第某段的字符串剪切到目的字符串中
// 注意：每出现一个分隔符，就是输出一个字符串
// lpString		:	源字符串
// szString		:	目的字符串
// nSize		:	目的字符串的长度，如果长度小于目的字符串(szString)的真实长度，则会出现致命错误
// nSegment		:	将源字符串的第几段放到目的字符串中
// szChar		:	分隔符，缺省以空格分隔
BOOL CutStringTable( const char *lpString, char *szString, int nSize, int nSegment, char szChar = ' ' );

// 得到第几段以后的字符串
// lpString		:	源字符串
// szString		:	目的字符串
// nSize		:	目的字符串的长度，如果长度小于目的字符串(szString)的真实长度，则会出现致命错误
// nSegment		:	将源字符串的第几段以后的内容放到目的字符串中
// szChar		:	分隔符，缺省以空格分隔
BOOL CutStringGetRemain( const char *lpString, char *szString, int nSize, int nSegment, char szChar = ' ' );

// 把字符串转换成大写(Windows标准API中有此类函数:_strlwr, _strupr)
// make the string upper case
// str	:	string to be changed
void StrMakeUpper( char *str );

// Find sub string in a given string
// 在一个字符串中找到另一个字符串(Windows标准API中有此类函数:_strstr)
// mystring		:	被查找的字符串
// mysubstring	:	要查找的字符串
// returns		:	找到时第一个字母的位置，-1失败
int FindSubString( char *mystring, char *mysubstring );
////////////////////


////////////////////
//----------------File Name Functions-----------------
//		文件名处理函数
////////////////////
// 剥去文件路径
// fullname :	初始文件名
// newname	:	剥去路径的文件名
// nSize	:	剥去路径的文件名的长度，如果小于输出字符串（第二个参数）的真实长度，则会出现致命错误
// index	:	剥到剩下几层,为0是只有文件名,为1是有一层路径,为2是有两层路径，小于0或多于路径的层数无效
void MakeNakedNameIndex( const char *fullname, char *newname, int nSize, int index );

// 剥去文件名,只剩下路径
// fullname	:	剥去只剩下路径的文件名
// nSize		:	剥去路径的文件名的长度，如果小于输出字符串（第二个参数）的真实长度，则会出现致命错误
void MakeNakedPath( const char *fullname, char *pathname, int nSize );

// 得到文件名前缀，没有后缀
// get a filename's name, without extension
// filename	:	assumed to be a valid filename, that the last 3 characters are extension
// noext	:	filename without extension
// nSize	:	剥去路径的文件名的长度，如果小于输出字符串（第二个参数）的真实长度，则会出现致命错误
void GetFileNameNoExt( const char *filename, char *noext, int nSize );

// 得到文件名后缀
// get a filename's extension
// filename	:	assumed to be a valid filename, that the last 3 characters are extension
// ext		:	extension of the filename
// nSize	:	得到的后缀的最大长度，如果小于输出字符串（第二个参数）的真实长度，则会出现致命错误
void GetFileNameExt( const char *filename, char *ext, int nSize );
////////////////////


////////////////////
// 前端包含匹配两个字符串，不同于strcmp()
// 第二个字符串是否包含在第一个字符串中，而且前面的字符串一样
// 相同则返回0
int StrCompare( const char *str1, const char *str2 );
////////////////////


////////////////////
#endif	//__EMSTRING_H__

