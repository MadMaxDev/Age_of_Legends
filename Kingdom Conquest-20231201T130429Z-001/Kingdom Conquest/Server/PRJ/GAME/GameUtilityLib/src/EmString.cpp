////////////////////
//	EmString.cpp	:	V0020
//	Written By		:	miss, Liu Gang
//	Compiler		:	Microsoft Visual C++ V5.0/6.0
//	Copyright (C)	:	1996-1999 EternalMoment Studio. All Rights Reserved.
//	V0010			:	( 1999.01.29 ) Created by miss
//	V0011			:	( 1999.02.06 ) Fixed some bugs
//	V0020			:	Oct.28.1999, Changed by Liu Gang. Add some new functions for filenames
////////////////////
// �ַ�����������
//		��������������
//		�ļ�����������
//		�ļ���������
////////////////////


////////////////////
#include	<string.h>
#include	<assert.h>
#include	"../inc/EmString.h"

#include	<WHCMN/inc/whstring.h>
using namespace n_whcmn;

////////////////////


////////////////////
//----------------Function parameter Functions-----------------
////////////////////
// �õ��ַ��������Ķ���
// ���ַ����м����������ָ����ָ�
// lpString		:	����ѯ���ַ���
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
int GetSegmentCount( const char *lpString, char szChar/*=' '*/ )
{
	int SegmentCount = 0;
	int length = strlen ( lpString );
	BOOL bNewSegment = TRUE;
	for ( int nLoop = 0; nLoop < length; nLoop++ )
	{
		if ( lpString[nLoop] == szChar )
		{
			if( bNewSegment == FALSE )
			{
				SegmentCount++;
				bNewSegment = TRUE;
			}
			continue;
		}
		else
		{
			bNewSegment = FALSE;
		}
	}

	// ���һ�εļ���
	if( length>0 && lpString[length-1]!=szChar )
	{
		SegmentCount++;	
	}
	return SegmentCount;
}
// �õ��ַ��������Ķ���
// ���ַ����м����������ָ����ָ�
// ע�⣺ÿ����һ���ָ������������һ���ַ���
// lpString		:	����ѯ���ַ���
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
int GetSegmentCountTable( const char *lpString, char szChar ) // =' ' )
{
	int SegmentCount = 0;
	int length = strlen ( lpString );
	for ( int nLoop = 0; nLoop < length; nLoop++ )
	{
		if ( lpString[nLoop] == szChar )
		{
			SegmentCount++;
			continue;
		}
	}

	// ���һ�εļ���
	if( length>0 && lpString[length-1]!=szChar )
	{
		SegmentCount++;	
	}
	return SegmentCount;
}

// �����ַ���
// ��Դ�ַ����е�ĳ�ε��ַ������е�Ŀ���ַ�����
// lpString		:	Դ�ַ���
// szString		:	Ŀ���ַ���
// nSize		:	Ŀ���ַ����ĳ��ȣ��������С��Ŀ���ַ���(szString)����ʵ���ȣ���������������
// nSegment		:	��Դ�ַ����ĵڼ��ηŵ�Ŀ���ַ����У���0��ʼ����
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
BOOL CutString( const char *lpString, char *szString, int nSize, int nSegment, char szChar/* = ' '*/ )
{
	int SegmentCount = 0;
	BOOL bNewSegment = TRUE;
	if( lpString == NULL )
	{
		return false;
	}
	int length = strlen ( lpString );
	int nCharCount = 0;
	BOOL bGet = FALSE;
	for ( int nLoop = 0; nLoop < length; nLoop++ )
	{
		// �Ƿָ���
		if( lpString[nLoop] == szChar )
		{
			if( bGet == TRUE )
			{	// �Ѿ�������һ�Σ��ҵ�����Ҫ���ַ���
				szString[nCharCount] = '\0';
				nCharCount=0;
				return TRUE;
			}
			if( bNewSegment == FALSE )
			{
				SegmentCount++;
				bNewSegment = TRUE;
			}
			continue;
		}
		// ����Ҫ����һ������
		if( SegmentCount == nSegment )
		{
			// ������
			if( nCharCount < nSize-1 )
			{
				szString[nCharCount] = lpString[nLoop];
				nCharCount++;
			}
			bGet = TRUE;
		}
		bNewSegment = FALSE;
	}
	// �ҵ��������һ��
	if( nCharCount > 0 )
	{
		szString[nCharCount] = '\0';
		nCharCount = 0;
		return TRUE;
	}
	// û���ҵ�
	return FALSE;
}

// �����ַ���
// ��Դ�ַ����е�ĳ�ε��ַ������е�Ŀ���ַ�����
// ע�⣺ÿ����һ���ָ������������һ���ַ���
// lpString		:	Դ�ַ���
// szString		:	Ŀ���ַ���
// nSize		:	Ŀ���ַ����ĳ��ȣ��������С��Ŀ���ַ���(szString)����ʵ���ȣ���������������
// nSegment		:	��Դ�ַ����ĵڼ��ηŵ�Ŀ���ַ�����
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
BOOL CutStringTable( const char *lpString, char *szString, int nSize, int nSegment, char szChar ) // = ' ' )
{
	int SegmentCount = 0;
	int length = strlen ( lpString );
	int nCharCount = 0;
	BOOL bGet = FALSE;
	for ( int nLoop = 0; nLoop < length; nLoop++ )
	{
		// �Ƿָ���
		if( lpString[nLoop] == szChar )
		{
			if( bGet == TRUE )
			{	// �Ѿ�������һ�Σ��ҵ�����Ҫ���ַ���
				szString[nCharCount] = '\0';
				nCharCount=0;
				return TRUE;
			}
			SegmentCount++;
			continue;
		}
		// ����Ҫ����һ������
		if( SegmentCount == nSegment )
		{
			// ������
			if( nCharCount < nSize-1 )
			{
				szString[nCharCount] = lpString[nLoop];
				nCharCount++;
			}
			bGet = TRUE;
		}
	}
	// �ҵ��������һ��
	if( nCharCount > 0 )
	{
		szString[nCharCount] = '\0';
		nCharCount = 0;
		return TRUE;
	}
	// û���ҵ�
	return FALSE;
}

// �õ��ڼ����Ժ���ַ���
// lpString		:	Դ�ַ���
// szString		:	Ŀ���ַ���
// nSize		:	Ŀ���ַ����ĳ��ȣ��������С��Ŀ���ַ���(szString)����ʵ���ȣ���������������
// nSegment		:	��Դ�ַ����ĵڼ����Ժ�����ݷŵ�Ŀ���ַ�����
// szChar		:	�ָ�����ȱʡ�Կո�ָ�
BOOL CutStringGetRemain( const char *lpString, char *szString, int nSize, int nSegment, char szChar ) //= ' ' )
{
	int SegmentCount = 0;
	BOOL bNewSegment = TRUE;
	int length = strlen ( lpString );
	int nCharCount = 0;
	for ( int nLoop = 0; nLoop < length; nLoop++ )
	{
		// �Ƿָ���
		if( lpString[nLoop] == szChar )
		{
			if( bNewSegment == FALSE )
			{
				SegmentCount++;
				bNewSegment = TRUE;
			}
			continue;
		}
		// ����Ҫ����һ������
		if( SegmentCount == nSegment )
		{
			// ������
			if( nSize > (length-nLoop) )
			{
				nSize = length-nLoop;
			}
			for( nCharCount=0; nCharCount<nSize; nCharCount++ )
			{
				szString[nCharCount] = lpString[nLoop+nCharCount];
			}
			break;
		}
		bNewSegment = FALSE;
	}
	// �ҵ��������һ��
	if( nCharCount > 0 )
	{
		szString[nCharCount] = '\0';
		nCharCount = 0;
		return TRUE;
	}
	// û���ҵ�
	return FALSE;
}

// ���ַ���ת���ɴ�д(Windows��׼API���д��ຯ��:_strlwr, _strupr)
// make the string upper case
// str	:	string to be changed
void StrMakeUpper( char *str )
{
#ifdef WIN32
	_strupr(str);
#else
	int		i=0;
	while(str[i])
	{
		if( str[i]>='a' && str[i]<='z' )
		{
			str[i]	+= ('A'-'z');
		}
		i	++;
	}
#endif
	return;
	/*�����������
	int length = strlen( str );
	for( int i=0; i<length; i++ )
	{
		if( str[i]>=97 && str[i] < 123 )
		{
			str[i] -= 32;
		}
	}
	*/
}

// Find sub string in a given string
// ��һ���ַ������ҵ���һ���ַ���(Windows��׼API���д��ຯ��:_strstr)
// mystring		:	�����ҵ��ַ���
// mysubstring	:	Ҫ���ҵ��ַ���
// returns		:	�ҵ�ʱ��һ����ĸ��λ�ã�-1ʧ��
int FindSubString( char *mystring, char *mysubstring )
{
	int i, j;
	int nLen = strlen( mystring );
	int nSubLen = strlen( mysubstring );
	int nFindCount = 0;
	int nFind = -1;
	char *string=NULL;
	char *substring=NULL;
	
	wh_strlwr( mystring );
	wh_strlwr( mysubstring );
	string = mystring;
	substring = mysubstring;
	for( i=0; i<nLen; i++ )
	{
		if( string[i] == substring[0] )
		{
			nFindCount = 0;
			for( j=0; j<nSubLen; j++ )
			{
				if( string[i+j] == substring[j] )
				{
					nFindCount++;
				}
			}
			if( nFindCount == nSubLen )
			{
				nFind = i;
			}
		}
		if( nFind >= 0 )
			break;
	}
	return nFind;
}
////////////////////


////////////////////
//----------------File Name Functions-----------------
////////////////////
// ��ȥ�ļ�·��
// fullname :	��ʼ�ļ���
// newname	:	��ȥ·�����ļ���
// nSize	:	��ȥ·�����ļ����ĳ��ȣ����С������ַ������ڶ�������������ʵ���ȣ���������������
// index	:	����ʣ�¼���,Ϊ0��ֻ���ļ���,Ϊ1����һ��·��,Ϊ2��������·����С��0�����·���Ĳ�����Ч
void MakeNakedNameIndex( const char *fullname, char *newname, int nSize, int index )
{
	int i;
	int begin = -1;
	int len = strlen( fullname );
	int counter = 0;

	assert( index >= 0 );
	for( i=len-1; i>=0; i-- )
	{
		if( fullname[i] == '\\' || fullname[i] == '/')
		{
			if( counter == index )
			{
				begin = i;
				break;
			}
			counter++;
		}
	}

	int nCount = 0;
	for( i = begin+1; i<len; i++ )
	{
		if( nCount < nSize-1 )
		{
			newname[i-begin-1] = fullname[i];
			nCount++;
		}
	}
	if( nCount > 0 )
	{
		newname[nCount] = '\0';
	}
}	// End of MakeNakedNameIndex()

// ��ȥ�ļ���,ֻʣ��·��
// fullname	:	��ȥֻʣ��·�����ļ���
// nSize		:	��ȥ·�����ļ����ĳ��ȣ����С������ַ������ڶ�������������ʵ���ȣ���������������
void MakeNakedPath( const char *fullname, char *pathname, int nSize )
{
	int i;
	int begin = -1;
	int len = strlen( fullname );
	for( i=len-1; i>=0; i-- )
	{
		if( fullname[i] == '\\' || fullname[i] == '/' )
		{
			begin = i;
			break;
		}
	}
	if( begin< 0 )
	{
		return;
	}

	if( nSize-1 < begin+1 ) begin = nSize-2;
	for( i = 0; i< begin+1; i++ )
	{
		pathname[i] = fullname[i];
	}
	pathname[i] = 0;
}	// End of MakeNakedPath()

// �õ��ļ���ǰ׺��û�к�׺
// get a filename's name, without extension
// filename	:	assumed to be a valid filename, that the last 3 characters are extension
// noext	:	filename without extension
// nSize	:	��ȥ·�����ļ����ĳ��ȣ����С������ַ������ڶ�������������ʵ���ȣ���������������
void GetFileNameNoExt( const char *filename, char *noext, int nSize )
{
	int length = strlen( filename );
	if( nSize-1 < length ) length = nSize-1;
	int i;
	for( i=length-1; i>=0; i-- )
	{
		if( filename[i] == '.' )
		{
			break;
		}
	}
	if( i == -1 )
		i = length;
	if( i >= 0 )
	{
		memcpy( noext, filename, i );
	}
	noext[i] = 0;
}	// End of GetFileNameNoExt()

// �õ��ļ�����׺
// get a filename's extension
// filename	:	assumed to be a valid filename, that the last 3 characters are extension
// ext		:	extension of the filename
// nSize	:	�õ��ĺ�׺����󳤶ȣ����С������ַ������ڶ�������������ʵ���ȣ���������������
void GetFileNameExt( const char *filename, char *ext, int nSize )
{
	int length = strlen( filename );
	for( int i=length-1; i>=0; i-- )
	{
		if( filename[i] == '.' )
		{
			int nCount = 0;
			for( int j=i+1; j<length; j++ )
			{
				if( nCount < nSize-1 )
				{
					ext[j-i-1] = filename[j];
					nCount++;
				}
			}
			if( nCount > 0 )
			{
				ext[nCount] = '\0';
			}
			break;
		}
	}
}	// End of GetFileNameExt()
////////////////////


////////////////////
// ǰ�˰���ƥ�������ַ�������ͬ��strcmp()
// �ڶ����ַ����Ƿ�����ڵ�һ���ַ����У�����ǰ����ַ���һ��
// ��ͬ�򷵻�0
BOOL StrCompare( const char *str1, const char *str2 )
{
	if( strstr( str1, str2 ) != NULL 
		&& str1[0] == str2[0] )
	{
		return 0;
	}
	return 1;
}
////////////////////
