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
//						�����MFC�������ӣ������ڹ����ļ��н�Ԥ����ͷȥ��
///////////////////////////
// header file
///////////////////////////

///////////////////////////
// this files includes functions for debug program, error handlers, general includes, macros, Asserts
// ������Ϣ�ʹ��������
//		������Ϣ
//		������
//		���������
//		���ĳ�����Ƿ��Ѿ�����
//		��������������
///////////////////////////


#ifndef __DEBUGAPI_H__
#define __DEBUGAPI_H__
// �����ַ�����ȱʡ��С
#define		_MAX_STRING		128
#define		_MAX_STRING256	256

#include <stdio.h>
#include <stdlib.h>			// for _itoa()
#include <time.h>
#include <string.h>
#include <assert.h>


///////////////////////////
// write debug string to a file, for debug release version, debug strings will be appended
// ���ļ�д�������Ϣ�����������ֻ�����а���֣��������а����Ե��Ե�ʱ��
// filename	:	file name to output
//				����д����ļ��������ļ�����Ǿ��ļ���׷�ӵ����
// str		:	debug string
//				Ҫд������ݣ��ַ�������
void WriteLogFile( const char * filename, const char * str );
void WriteLogFileDirect( const char * filename,const char * str );

// write debug integer number to a file, for debug release version, debug integer will be appended
// ���ļ�д�������Ϣ�����������ֻ�����а���֣��������а����Ե��Ե�ʱ��
// filename	:	file name to output
//				����д����ļ��������ļ�����Ǿ��ļ���׷�ӵ����
// id		:	debug number
//				Ҫд������ݣ���������
// szEnd    :   string that will be printed follow the int value
//          :   Ҫ����������֮��������ַ���
void WriteLogFileInt( const char * filename, int id, char *szEnd );

// write debug float number to a file, for debug release version, debug float will be appended
// ���ļ�д�������Ϣ�����������ֻ�����а���֣��������а����Ե��Ե�ʱ��
// filename	:	file name to output
//				����д����ļ��������ļ�����Ǿ��ļ���׷�ӵ����
// id		:	debug number
//				Ҫд������ݣ������ȸ���������
// szEnd    :   string that will be printed follow the int value
//              Ҫ�����ڵ����ȸ�����֮��������ַ���
void WriteLogFileFloat( const char * filename, float id, char *szEnd );

// write debug double number to a file, for debug release version, debug float will be appended
// ���ļ�д�������Ϣ�����������ֻ�����а���֣��������а����Ե��Ե�ʱ��
// filename	:	file name to output
//				����д����ļ��������ļ�����Ǿ��ļ���׷�ӵ����
// id		:	debug number
//				Ҫд������ݣ�˫���ȸ���������
// szEnd    :   string that will be printed follow the int value
//              Ҫ������˫���ȸ�����֮��������ַ���
void WriteLogFileDouble( const char * filename, double id, char *szEnd );
///////////////////////////


///////////////////////////
// ���������Ϣ2
extern char BugFileName[_MAX_STRING];
// ��ʼ���ĳ�����
// errorname	:	������ļ���
// errorline	:	������к�
void WriteBugFileBegin(char *errorname,int errorline);
// �������ĳ�����
void WriteBugFile_End();
// ɾ��������Ϣ�ļ�
void WriteBugFile_DeleteFile();

// �õ�����ļ���
// name		:	���������Ϣ���ļ���
// nSize	:	����ȡ���ļ������ַ����ĳ��ȣ������ֵ���ڸ��ַ�������ʵ������������������
void WriteBugFile_GetBugFileName( char *name, int nSize );
// ��������ļ���
// name		:	���������Ϣ���ļ���
bool WriteBugFile_SetBugFileName( const char *name );

bool WriteBugFile_AutoSetBugFileName( const char * szName );

// ��������к�
// errorline	:	������к�
void WriteBugFileBreakPoint( int errorline );

// ��������������ļ���
// errorname	:	������ļ���
void WriteBugFileFileName( char *errorname );


// ���һ����Ϣ
// nerrorint		:	Ҫ�������Ϣ
// strErrorString	:	�ָ�����Ϣ�ķָ������ڸ���Ϣ���棩
void WriteBugFile_WriteInt( int nerrorint,char *strErrorString );
// ���һ����Ϣ
// nerrorint		:	Ҫ�������Ϣ
// strErrorString	:	�ָ�����Ϣ�ķָ������ڸ���Ϣ���棩
void WriteBugFile_WriteFloat( float nerrorfloat,char *strErrorString );
// ���һ����Ϣ
// nerrorint		:	Ҫ�������Ϣ
// strErrorString	:	�ָ�����Ϣ�ķָ������ڸ���Ϣ���棩
void WriteBugFile_WriteDouble( double nerrordouble,char *strErrorString );
// ���һ����Ϣ
// strErrorString	:	Ҫ�������Ϣ
void WriteBugFile_WriteStr( const char * strErrorString );
///////////////////////////
///////////////////////////
#include "DebugApi.inl"
///////////////////////////
///////////////////////////
#endif // __DEBUGAPI_H__
