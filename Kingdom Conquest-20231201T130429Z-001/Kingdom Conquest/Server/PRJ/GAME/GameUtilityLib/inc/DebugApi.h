///////////////////////////
//	DebugApi.h		:	V0012
//	Written by		:	Liu Gang
//	Compiler		:	Microsoft Visual C++ 4.0/5.0/6.0
//  Copyright (C)	:	1996-1998 Eternal Moment Studio WayAhead Corporation. All Rights Reserved.
//	Library			:	Winmm.lib
//	v0010			:	Sep. 2. 1997
//	v0011			:	Dec.31.1997
//	V0012			:	Oct.27.1999
//						if link with MFC, you must remove precomplied headers in project
//						如果与MFC程序连接，必须在工程文件中将预编译头去掉
///////////////////////////
// header file
///////////////////////////

///////////////////////////
// this files includes functions for debug program, error handlers, general includes, macros, Asserts
// 调试信息和错误处理程序
//		调试信息
//		错误处理
//		产生随机数
//		检测某程序是否已经运行
//		检测光盘驱动器号
///////////////////////////


#ifndef __DEBUGAPI_H__
#define __DEBUGAPI_H__
// 定义字符串的缺省大小
#define		_MAX_STRING		128
#define		_MAX_STRING256	256

#include <stdio.h>
#include <stdlib.h>			// for _itoa()
#include <time.h>
#include <string.h>
#include <assert.h>


///////////////////////////
// write debug string to a file, for debug release version, debug strings will be appended
// 向文件写入调试信息。当程序错误只在运行版出现，而在运行版难以调试的时候。
// filename	:	file name to output
//				将被写入的文件名。该文件如果是旧文件则追加到最后。
// str		:	debug string
//				要写入的内容（字符串）。
void WriteLogFile( const char * filename, const char * str );
void WriteLogFileDirect( const char * filename,const char * str );

// write debug integer number to a file, for debug release version, debug integer will be appended
// 向文件写入调试信息。当程序错误只在运行版出现，而在运行版难以调试的时候。
// filename	:	file name to output
//				将被写入的文件名。该文件如果是旧文件则追加到最后。
// id		:	debug number
//				要写入的内容（整数）。
// szEnd    :   string that will be printed follow the int value
//          :   要跟随在整数之后输出的字符串
void WriteLogFileInt( const char * filename, int id, char *szEnd );

// write debug float number to a file, for debug release version, debug float will be appended
// 向文件写入调试信息。当程序错误只在运行版出现，而在运行版难以调试的时候。
// filename	:	file name to output
//				将被写入的文件名。该文件如果是旧文件则追加到最后。
// id		:	debug number
//				要写入的内容（单精度浮点数）。
// szEnd    :   string that will be printed follow the int value
//              要跟随在单精度浮点数之后输出的字符串
void WriteLogFileFloat( const char * filename, float id, char *szEnd );

// write debug double number to a file, for debug release version, debug float will be appended
// 向文件写入调试信息。当程序错误只在运行版出现，而在运行版难以调试的时候。
// filename	:	file name to output
//				将被写入的文件名。该文件如果是旧文件则追加到最后。
// id		:	debug number
//				要写入的内容（双精度浮点数）。
// szEnd    :   string that will be printed follow the int value
//              要跟随在双精度浮点数之后输出的字符串
void WriteLogFileDouble( const char * filename, double id, char *szEnd );
///////////////////////////


///////////////////////////
// 输出调试信息2
extern char BugFileName[_MAX_STRING];
// 开始输出某错误段
// errorname	:	出错的文件名
// errorline	:	出错的行号
void WriteBugFileBegin(char *errorname,int errorline);
// 结束输出某错误段
void WriteBugFile_End();
// 删除调试信息文件
void WriteBugFile_DeleteFile();

// 得到输出文件名
// name		:	输出调试信息的文件名
// nSize	:	用于取得文件名的字符串的长度，如果该值大于该字符串的真实长度则会出现致命错误
void WriteBugFile_GetBugFileName( char *name, int nSize );
// 设置输出文件名
// name		:	输出调试信息的文件名
bool WriteBugFile_SetBugFileName( const char *name );

bool WriteBugFile_AutoSetBugFileName( const char * szName );

// 输出本行行号
// errorline	:	出错的行号
void WriteBugFileBreakPoint( int errorline );

// 输出本代码所在文件名
// errorname	:	出错的文件名
void WriteBugFileFileName( char *errorname );


// 输出一段信息
// nerrorint		:	要输出的信息
// strErrorString	:	分隔该信息的分隔符（在该信息后面）
void WriteBugFile_WriteInt( int nerrorint,char *strErrorString );
// 输出一段信息
// nerrorint		:	要输出的信息
// strErrorString	:	分隔该信息的分隔符（在该信息后面）
void WriteBugFile_WriteFloat( float nerrorfloat,char *strErrorString );
// 输出一段信息
// nerrorint		:	要输出的信息
// strErrorString	:	分隔该信息的分隔符（在该信息后面）
void WriteBugFile_WriteDouble( double nerrordouble,char *strErrorString );
// 输出一段信息
// strErrorString	:	要输出的信息
void WriteBugFile_WriteStr( const char * strErrorString );
///////////////////////////
///////////////////////////
#include "DebugApi.inl"
///////////////////////////
///////////////////////////
#endif // __DEBUGAPI_H__
