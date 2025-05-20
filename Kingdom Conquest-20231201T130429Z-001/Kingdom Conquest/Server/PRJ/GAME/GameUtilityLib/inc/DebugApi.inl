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
// inline file
///////////////////////////


///////////////////////////
// 输出调试信息2
// 开始输出某错误段
// errorname	:	出错的文件名
// errorline	:	出错的行号
inline void WriteBugFileBegin(char *errorname,int errorline)
{
    WriteLogFile( BugFileName," * Bug Report Start * \n" );
    WriteLogFile( BugFileName,"ErrorName:");
    WriteLogFile( BugFileName,errorname );
    WriteLogFile( BugFileName,"\n" );
    WriteLogFile( BugFileName,"ErrorLine is:");
    WriteLogFileInt( BugFileName,errorline,"\n" );
}

// 结束输出某错误段
inline void WriteBugFile_End()
{
    WriteLogFile( BugFileName," * Bug Report End * \n" );
}

// 得到输出文件名
// name		:	输出调试信息的文件名
// nSize	:	用于取得文件名的字符串的长度，如果该值大于该字符串的真实长度则会出现致命错误
inline void WriteBugFile_GetBugFileName( char *name, int nSize )
{
	int len = strlen( BugFileName );
	if( nSize < len-1 )	len = nSize-1;
	int i=0;
	for( ; i<len; i++ )
	{
		name[i] = BugFileName[i];
	}
	name[i] = 0;
}

// 设置输出文件名
// name		:	输出调试信息的文件名
inline bool WriteBugFile_SetBugFileName( const char *name )
{
    assert ( name != NULL );
    if ( name == NULL )
        return false;
    strcpy(BugFileName,name);
    return true;
}


// 输出本行行号
// errorline	:	出错的行号
inline void WriteBugFileBreakPoint( int errorline )
{
    WriteLogFile( BugFileName,"ErrorLine is:");
    WriteLogFileInt( BugFileName,errorline,"\n" );
}

// 输出本代码所在文件名
// errorname	:	出错的文件名
inline void WriteBugFileFileName( char *errorname )
{
    WriteLogFile( BugFileName,"ErrorName:");
    WriteLogFile( BugFileName,errorname );
    WriteLogFile( BugFileName,"\n" );
}

// 输出一段信息
// nerrorint		:	要输出的信息
// strErrorString	:	分隔该信息的分隔符（在该信息后面）
inline void WriteBugFile_WriteFloat( float nerrorfloat,char *strErrorString )
{
    WriteLogFileFloat( BugFileName, nerrorfloat, strErrorString );
}

// 输出一段信息
// nerrorint		:	要输出的信息
// strErrorString	:	分隔该信息的分隔符（在该信息后面）
inline void WriteBugFile_WriteDouble( double nerrordouble,char *strErrorString )
{
    WriteLogFileDouble( BugFileName, nerrordouble, strErrorString );
}

// 输出一段信息
// nerrorint		:	要输出的信息
// strErrorString	:	分隔该信息的分隔符（在该信息后面）
inline void WriteBugFile_WriteInt( int nerrorint,char *strErrorString )
{
    WriteLogFileInt( BugFileName, nerrorint, strErrorString );
}

// 输出一段信息
// strErrorString	:	要输出的信息
inline void WriteBugFile_WriteStr( const char *  strErrorString )
{
    WriteLogFile( BugFileName, strErrorString );
}

inline void WriteBugFile_WriteStrDirect( const char *  strErrorString )
{
    WriteLogFileDirect( BugFileName, strErrorString );
}
///////////////////////////
