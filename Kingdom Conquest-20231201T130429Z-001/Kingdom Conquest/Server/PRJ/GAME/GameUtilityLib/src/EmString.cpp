////////////////////
//	EmString.cpp	:	V0020
//	Written By		:	miss, Liu Gang
//	Compiler		:	Microsoft Visual C++ V5.0/6.0
//	Copyright (C)	:	1996-1999 EternalMoment Studio. All Rights Reserved.
//	V0010			:	( 1999.01.29 ) Created by miss
//	V0011			:	( 1999.02.06 ) Fixed some bugs
//	V0020			:	Oct.28.1999, Changed by Liu Gang. Add some new functions for filenames
////////////////////
// 字符串处理函数库
//		函数参数处理函数
//		文件名操作函数
//		文件操作函数
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
// 得到字符串包含的段数
// 该字符串中间以任意多个分隔符分隔
// lpString		:	被查询的字符串
// szChar		:	分隔符，缺省以空格分隔
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

	// 最后一段的计数
	if( length>0 && lpString[length-1]!=szChar )
	{
		SegmentCount++;	
	}
	return SegmentCount;
}
// 得到字符串包含的段数
// 该字符串中间以任意多个分隔符分隔
// 注意：每出现一个分隔符，就是输出一个字符串
// lpString		:	被查询的字符串
// szChar		:	分隔符，缺省以空格分隔
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

	// 最后一段的计数
	if( length>0 && lpString[length-1]!=szChar )
	{
		SegmentCount++;	
	}
	return SegmentCount;
}

// 剪切字符串
// 把源字符串中第某段的字符串剪切到目的字符串中
// lpString		:	源字符串
// szString		:	目的字符串
// nSize		:	目的字符串的长度，如果长度小于目的字符串(szString)的真实长度，则会出现致命错误
// nSegment		:	将源字符串的第几段放到目的字符串中，从0开始记数
// szChar		:	分隔符，缺省以空格分隔
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
		// 是分隔符
		if( lpString[nLoop] == szChar )
		{
			if( bGet == TRUE )
			{	// 已经到了下一段，找到了想要的字符串
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
		// 是想要的那一段内容
		if( SegmentCount == nSegment )
		{
			// 不超界
			if( nCharCount < nSize-1 )
			{
				szString[nCharCount] = lpString[nLoop];
				nCharCount++;
			}
			bGet = TRUE;
		}
		bNewSegment = FALSE;
	}
	// 找到的是最后一段
	if( nCharCount > 0 )
	{
		szString[nCharCount] = '\0';
		nCharCount = 0;
		return TRUE;
	}
	// 没有找到
	return FALSE;
}

// 剪切字符串
// 把源字符串中第某段的字符串剪切到目的字符串中
// 注意：每出现一个分隔符，就是输出一个字符串
// lpString		:	源字符串
// szString		:	目的字符串
// nSize		:	目的字符串的长度，如果长度小于目的字符串(szString)的真实长度，则会出现致命错误
// nSegment		:	将源字符串的第几段放到目的字符串中
// szChar		:	分隔符，缺省以空格分隔
BOOL CutStringTable( const char *lpString, char *szString, int nSize, int nSegment, char szChar ) // = ' ' )
{
	int SegmentCount = 0;
	int length = strlen ( lpString );
	int nCharCount = 0;
	BOOL bGet = FALSE;
	for ( int nLoop = 0; nLoop < length; nLoop++ )
	{
		// 是分隔符
		if( lpString[nLoop] == szChar )
		{
			if( bGet == TRUE )
			{	// 已经到了下一段，找到了想要的字符串
				szString[nCharCount] = '\0';
				nCharCount=0;
				return TRUE;
			}
			SegmentCount++;
			continue;
		}
		// 是想要的那一段内容
		if( SegmentCount == nSegment )
		{
			// 不超界
			if( nCharCount < nSize-1 )
			{
				szString[nCharCount] = lpString[nLoop];
				nCharCount++;
			}
			bGet = TRUE;
		}
	}
	// 找到的是最后一段
	if( nCharCount > 0 )
	{
		szString[nCharCount] = '\0';
		nCharCount = 0;
		return TRUE;
	}
	// 没有找到
	return FALSE;
}

// 得到第几段以后的字符串
// lpString		:	源字符串
// szString		:	目的字符串
// nSize		:	目的字符串的长度，如果长度小于目的字符串(szString)的真实长度，则会出现致命错误
// nSegment		:	将源字符串的第几段以后的内容放到目的字符串中
// szChar		:	分隔符，缺省以空格分隔
BOOL CutStringGetRemain( const char *lpString, char *szString, int nSize, int nSegment, char szChar ) //= ' ' )
{
	int SegmentCount = 0;
	BOOL bNewSegment = TRUE;
	int length = strlen ( lpString );
	int nCharCount = 0;
	for ( int nLoop = 0; nLoop < length; nLoop++ )
	{
		// 是分隔符
		if( lpString[nLoop] == szChar )
		{
			if( bNewSegment == FALSE )
			{
				SegmentCount++;
				bNewSegment = TRUE;
			}
			continue;
		}
		// 是想要的那一段内容
		if( SegmentCount == nSegment )
		{
			// 不超界
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
	// 找到的是最后一段
	if( nCharCount > 0 )
	{
		szString[nCharCount] = '\0';
		nCharCount = 0;
		return TRUE;
	}
	// 没有找到
	return FALSE;
}

// 把字符串转换成大写(Windows标准API中有此类函数:_strlwr, _strupr)
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
	/*不用下面的了
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
// 在一个字符串中找到另一个字符串(Windows标准API中有此类函数:_strstr)
// mystring		:	被查找的字符串
// mysubstring	:	要查找的字符串
// returns		:	找到时第一个字母的位置，-1失败
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
// 剥去文件路径
// fullname :	初始文件名
// newname	:	剥去路径的文件名
// nSize	:	剥去路径的文件名的长度，如果小于输出字符串（第二个参数）的真实长度，则会出现致命错误
// index	:	剥到剩下几层,为0是只有文件名,为1是有一层路径,为2是有两层路径，小于0或多于路径的层数无效
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

// 剥去文件名,只剩下路径
// fullname	:	剥去只剩下路径的文件名
// nSize		:	剥去路径的文件名的长度，如果小于输出字符串（第二个参数）的真实长度，则会出现致命错误
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

// 得到文件名前缀，没有后缀
// get a filename's name, without extension
// filename	:	assumed to be a valid filename, that the last 3 characters are extension
// noext	:	filename without extension
// nSize	:	剥去路径的文件名的长度，如果小于输出字符串（第二个参数）的真实长度，则会出现致命错误
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

// 得到文件名后缀
// get a filename's extension
// filename	:	assumed to be a valid filename, that the last 3 characters are extension
// ext		:	extension of the filename
// nSize	:	得到的后缀的最大长度，如果小于输出字符串（第二个参数）的真实长度，则会出现致命错误
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
// 前端包含匹配两个字符串，不同于strcmp()
// 第二个字符串是否包含在第一个字符串中，而且前面的字符串一样
// 相同则返回0
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
