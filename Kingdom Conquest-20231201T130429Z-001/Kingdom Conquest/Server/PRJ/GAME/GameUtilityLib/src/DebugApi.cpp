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
//						�����MFC�������ӣ������ڹ����ļ��н�Ԥ����ͷȥ��
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
// ���ļ�д�������Ϣ�����������ֻ�����а���֣��������а����Ե��Ե�ʱ��
// filename	:	file name to output
//				����д����ļ��������ļ�����Ǿ��ļ���׷�ӵ����
// str		:	debug string
//				Ҫд������ݣ��ַ�������
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
// ���ļ�д�������Ϣ�����������ֻ�����а���֣��������а����Ե��Ե�ʱ��
// filename	:	file name to output
//				����д����ļ��������ļ�����Ǿ��ļ���׷�ӵ����
// id		:	debug number
//				Ҫд������ݣ���������
// szEnd    :   string that will be printed follow the int value
//          :   Ҫ����������֮��������ַ���
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
// ���ļ�д�������Ϣ�����������ֻ�����а���֣��������а����Ե��Ե�ʱ��
// filename	:	file name to output
//				����д����ļ��������ļ�����Ǿ��ļ���׷�ӵ����
// id		:	debug number
//				Ҫд������ݣ������ȸ���������
// szEnd    :   string that will be printed follow the int value
//              Ҫ�����ڵ����ȸ�����֮��������ַ���
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
// ���ļ�д�������Ϣ�����������ֻ�����а���֣��������а����Ե��Ե�ʱ��
// filename	:	file name to output
//				����д����ļ��������ļ�����Ǿ��ļ���׷�ӵ����
// id		:	debug number
//				Ҫд������ݣ�˫���ȸ���������
// szEnd    :   string that will be printed follow the int value
//              Ҫ������˫���ȸ�����֮��������ַ���
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
// ���������Ϣ2
char BugFileName[_MAX_STRING]="BugLog.ini";
///////////////////////////

bool WriteBugFile_AutoSetBugFileName( const char * szName )
{
	strcpy(BugFileName,szName);
	WriteBugFile_DeleteFile();
	return true;
}

// ɾ��������Ϣ�ļ�
void WriteBugFile_DeleteFile()
{
	whfile_del( BugFileName );
}




