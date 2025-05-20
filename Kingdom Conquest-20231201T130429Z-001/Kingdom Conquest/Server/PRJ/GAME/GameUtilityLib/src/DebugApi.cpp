///////////////////////////
//	DebugApi.cpp	:	V0012
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
// implementation file
///////////////////////////


///////////////////////////
#include "../inc/DebugApi.h"

#include <WHCMN/inc/whfile.h>
#include <WHCMN/inc/whtime.h>
using namespace n_whcmn;


///////////////////////////
// write debug string to a file, for debug release version, debug strings will be appended
// 向文件写入调试信息。当程序错误只在运行版出现，而在运行版难以调试的时候。
// filename	:	file name to output
//				将被写入的文件名。该文件如果是旧文件则追加到最后。
// str		:	debug string
//				要写入的内容（字符串）。
void WriteLogFile( const char * filename, const char * str )
{
	FILE *fp;
	fp = fopen( filename, "at" );
	if( !fp )
		return;
	char tmpbuf[128];
    
	wh_getsmptimestr(time(NULL),tmpbuf);
	fprintf(fp, "%s",tmpbuf);

    fprintf( fp, "	%s",str );

	fclose( fp );
}

void WriteLogFileDirect( const char * filename, const char * str )
{
	FILE *fp;
	fp = fopen( filename, "at" );
	if( !fp )
		return;
    fprintf( fp, "	%s",str );
	fclose( fp );
}

// write debug integer number to a file, for debug release version, debug integer will be appended
// 向文件写入调试信息。当程序错误只在运行版出现，而在运行版难以调试的时候。
// filename	:	file name to output
//				将被写入的文件名。该文件如果是旧文件则追加到最后。
// id		:	debug number
//				要写入的内容（整数）。
// szEnd    :   string that will be printed follow the int value
//          :   要跟随在整数之后输出的字符串
void WriteLogFileInt( const char * filename, int id, char *szEnd )
{
	FILE * fp;
	fp = fopen( filename, "at" );
	if( !fp )
		return;
	fprintf( fp,"%d%s", id, szEnd );
	fclose( fp );
}

// write debug float number to a file, for debug release version, debug float will be appended
// 向文件写入调试信息。当程序错误只在运行版出现，而在运行版难以调试的时候。
// filename	:	file name to output
//				将被写入的文件名。该文件如果是旧文件则追加到最后。
// id		:	debug number
//				要写入的内容（单精度浮点数）。
// szEnd    :   string that will be printed follow the int value
//              要跟随在单精度浮点数之后输出的字符串
void WriteLogFileFloat( const char * filename, float id, char *szEnd )
{
	FILE * fp;
	fp = fopen( filename, "at" );
	if( !fp )
		return;
	fprintf( fp,"%f%s", id, szEnd );
	fclose( fp );
}

// write debug float number to a file, for debug release version, debug float will be appended
// 向文件写入调试信息。当程序错误只在运行版出现，而在运行版难以调试的时候。
// filename	:	file name to output
//				将被写入的文件名。该文件如果是旧文件则追加到最后。
// id		:	debug number
//				要写入的内容（双精度浮点数）。
// szEnd    :   string that will be printed follow the int value
//              要跟随在双精度浮点数之后输出的字符串
void WriteLogFileDouble( const char * filename, double id, char *szEnd )
{
	FILE * fp;
	fp = fopen( filename, "at" );
	if( !fp )
		return;
	fprintf( fp,"%e%s", id, szEnd );
	fclose( fp );
}
///////////////////////////


///////////////////////////
// 输出调试信息2
char BugFileName[_MAX_STRING]="BugLog.ini";
///////////////////////////

bool WriteBugFile_AutoSetBugFileName( const char * szName )
{
	strcpy(BugFileName,szName);
	WriteBugFile_DeleteFile();
	return true;
}

// 删除调试信息文件
void WriteBugFile_DeleteFile()
{
	whfile_del( BugFileName );
}




