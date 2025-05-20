////////////////////
//	EmString.h		:	V0020
//	Written By		:	miss, Liu Gang
//	Compiler		:	Microsoft Visual C++ V5.0/6.0
//	Copyright (C)	:	1996-1999 EternalMoment Studio. All Rights Reserved.
//	V0010			:	( 1999.01.29 ) Created by miss
//	V0011			:	( 1999.02.06 ) Fixed some bugs
//	V0020			:	Oct.28.1999, Changed by Liu Gang. Add some new functions for filenames
////////////////////
// �ַ�����������
//		��������������
//		�ļ���������
//		�ļ�������
////////////////////


#ifndef		__EMSTRING_H__
#define		__EMSTRING_H__
////////////////////


////////////////////
#include	"DMtype.h"		// BOOL
////////////////////


////////////////////
//----------------Function parameter Functions-----------------
//		��������������
////////////////////
// �õ��ַ��������Ķ���
// ���ַ����м����������ָ����ָ�
// ע�⣺����������������ķָ���ʱ������Ϊ��һ���ָ���
// lpString		:	����ѯ���ַ���
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
int GetSegmentCount( const char *lpString, char szChar=' ' );
// �õ��ַ��������Ķ���
// ���ַ����м����������ָ����ָ�
// ע�⣺ÿ����һ���ָ������������һ���ַ���
// lpString		:	����ѯ���ַ���
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
int GetSegmentCountTable( const char *lpString, char szChar=' ' );

// �����ַ���
// ��Դ�ַ����е�ĳ�ε��ַ������е�Ŀ���ַ�����
// ע�⣺����������������ķָ���ʱ������Ϊ��һ���ָ���
// lpString		:	Դ�ַ���
// szString		:	Ŀ���ַ���
// nSize		:	Ŀ���ַ����ĳ��ȣ��������С��Ŀ���ַ���(szString)����ʵ���ȣ���������������
// nSegment		:	��Դ�ַ����ĵڼ��ηŵ�Ŀ���ַ�����
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
BOOL CutString( const char *lpString, char *szString, int nSize, int nSegment, char szChar = ' ' );

// �����ַ���
// ��Դ�ַ����е�ĳ�ε��ַ������е�Ŀ���ַ�����
// ע�⣺ÿ����һ���ָ������������һ���ַ���
// lpString		:	Դ�ַ���
// szString		:	Ŀ���ַ���
// nSize		:	Ŀ���ַ����ĳ��ȣ��������С��Ŀ���ַ���(szString)����ʵ���ȣ���������������
// nSegment		:	��Դ�ַ����ĵڼ��ηŵ�Ŀ���ַ�����
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
BOOL CutStringTable( const char *lpString, char *szString, int nSize, int nSegment, char szChar = ' ' );

// �õ��ڼ����Ժ���ַ���
// lpString		:	Դ�ַ���
// szString		:	Ŀ���ַ���
// nSize		:	Ŀ���ַ����ĳ��ȣ��������С��Ŀ���ַ���(szString)����ʵ���ȣ���������������
// nSegment		:	��Դ�ַ����ĵڼ����Ժ�����ݷŵ�Ŀ���ַ�����
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
BOOL CutStringGetRemain( const char *lpString, char *szString, int nSize, int nSegment, char szChar = ' ' );

// ���ַ���ת���ɴ�д(Windows��׼API���д��ຯ��:_strlwr, _strupr)
// make the string upper case
// str	:	string to be changed
void StrMakeUpper( char *str );

// Find sub string in a given string
// ��һ���ַ������ҵ���һ���ַ���(Windows��׼API���д��ຯ��:_strstr)
// mystring		:	�����ҵ��ַ���
// mysubstring	:	Ҫ���ҵ��ַ���
// returns		:	�ҵ�ʱ��һ����ĸ��λ�ã�-1ʧ��
int FindSubString( char *mystring, char *mysubstring );
////////////////////


////////////////////
//----------------File Name Functions-----------------
//		�ļ���������
////////////////////
// ��ȥ�ļ�·��
// fullname :	��ʼ�ļ���
// newname	:	��ȥ·�����ļ���
// nSize	:	��ȥ·�����ļ����ĳ��ȣ����С������ַ������ڶ�������������ʵ���ȣ���������������
// index	:	����ʣ�¼���,Ϊ0��ֻ���ļ���,Ϊ1����һ��·��,Ϊ2��������·����С��0�����·���Ĳ�����Ч
void MakeNakedNameIndex( const char *fullname, char *newname, int nSize, int index );

// ��ȥ�ļ���,ֻʣ��·��
// fullname	:	��ȥֻʣ��·�����ļ���
// nSize		:	��ȥ·�����ļ����ĳ��ȣ����С������ַ������ڶ�������������ʵ���ȣ���������������
void MakeNakedPath( const char *fullname, char *pathname, int nSize );

// �õ��ļ���ǰ׺��û�к�׺
// get a filename's name, without extension
// filename	:	assumed to be a valid filename, that the last 3 characters are extension
// noext	:	filename without extension
// nSize	:	��ȥ·�����ļ����ĳ��ȣ����С������ַ������ڶ�������������ʵ���ȣ���������������
void GetFileNameNoExt( const char *filename, char *noext, int nSize );

// �õ��ļ�����׺
// get a filename's extension
// filename	:	assumed to be a valid filename, that the last 3 characters are extension
// ext		:	extension of the filename
// nSize	:	�õ��ĺ�׺����󳤶ȣ����С������ַ������ڶ�������������ʵ���ȣ���������������
void GetFileNameExt( const char *filename, char *ext, int nSize );
////////////////////


////////////////////
// ǰ�˰���ƥ�������ַ�������ͬ��strcmp()
// �ڶ����ַ����Ƿ�����ڵ�һ���ַ����У�����ǰ����ַ���һ��
// ��ͬ�򷵻�0
int StrCompare( const char *str1, const char *str2 );
////////////////////


////////////////////
#endif	//__EMSTRING_H__

