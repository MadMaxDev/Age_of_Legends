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
// inline file
///////////////////////////


///////////////////////////
// ���������Ϣ2
// ��ʼ���ĳ�����
// errorname	:	������ļ���
// errorline	:	������к�
inline void WriteBugFileBegin(char *errorname,int errorline)
{
    WriteLogFile( BugFileName," * Bug Report Start * \n" );
    WriteLogFile( BugFileName,"ErrorName:");
    WriteLogFile( BugFileName,errorname );
    WriteLogFile( BugFileName,"\n" );
    WriteLogFile( BugFileName,"ErrorLine is:");
    WriteLogFileInt( BugFileName,errorline,"\n" );
}

// �������ĳ�����
inline void WriteBugFile_End()
{
    WriteLogFile( BugFileName," * Bug Report End * \n" );
}

// �õ�����ļ���
// name		:	���������Ϣ���ļ���
// nSize	:	����ȡ���ļ������ַ����ĳ��ȣ������ֵ���ڸ��ַ�������ʵ������������������
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

// ��������ļ���
// name		:	���������Ϣ���ļ���
inline bool WriteBugFile_SetBugFileName( const char *name )
{
    assert ( name != NULL );
    if ( name == NULL )
        return false;
    strcpy(BugFileName,name);
    return true;
}


// ��������к�
// errorline	:	������к�
inline void WriteBugFileBreakPoint( int errorline )
{
    WriteLogFile( BugFileName,"ErrorLine is:");
    WriteLogFileInt( BugFileName,errorline,"\n" );
}

// ��������������ļ���
// errorname	:	������ļ���
inline void WriteBugFileFileName( char *errorname )
{
    WriteLogFile( BugFileName,"ErrorName:");
    WriteLogFile( BugFileName,errorname );
    WriteLogFile( BugFileName,"\n" );
}

// ���һ����Ϣ
// nerrorint		:	Ҫ�������Ϣ
// strErrorString	:	�ָ�����Ϣ�ķָ������ڸ���Ϣ���棩
inline void WriteBugFile_WriteFloat( float nerrorfloat,char *strErrorString )
{
    WriteLogFileFloat( BugFileName, nerrorfloat, strErrorString );
}

// ���һ����Ϣ
// nerrorint		:	Ҫ�������Ϣ
// strErrorString	:	�ָ�����Ϣ�ķָ������ڸ���Ϣ���棩
inline void WriteBugFile_WriteDouble( double nerrordouble,char *strErrorString )
{
    WriteLogFileDouble( BugFileName, nerrordouble, strErrorString );
}

// ���һ����Ϣ
// nerrorint		:	Ҫ�������Ϣ
// strErrorString	:	�ָ�����Ϣ�ķָ������ڸ���Ϣ���棩
inline void WriteBugFile_WriteInt( int nerrorint,char *strErrorString )
{
    WriteLogFileInt( BugFileName, nerrorint, strErrorString );
}

// ���һ����Ϣ
// strErrorString	:	Ҫ�������Ϣ
inline void WriteBugFile_WriteStr( const char *  strErrorString )
{
    WriteLogFile( BugFileName, strErrorString );
}

inline void WriteBugFile_WriteStrDirect( const char *  strErrorString )
{
    WriteLogFileDirect( BugFileName, strErrorString );
}
///////////////////////////
