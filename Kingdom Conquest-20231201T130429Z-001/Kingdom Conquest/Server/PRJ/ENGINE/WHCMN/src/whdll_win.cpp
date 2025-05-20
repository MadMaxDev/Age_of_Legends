// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdll_win.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 动态链接库的使用(Windows版)
// CreationDate : 2004-11-25
// ChangeLog    :

#ifdef	WIN32	//				{

#include "../inc/wh_platform.h"
#include "../inc/whdll.h"
#include "../inc/whstring.h"
#include <assert.h>

using namespace n_whcmn;

class	WHDLL_Loader_WIN32	: public	WHDLL_Loader
{
public:
	WHDLL_Loader_WIN32();
	virtual ~WHDLL_Loader_WIN32();
public:
	// 为父类实现的
	virtual int		Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit);
	virtual void *	GetFunc(const char *cszFuncName);
	virtual int		Release();
private:
	HMODULE		m_dllhandle;
};
WHDLL_Loader *	WHDLL_Loader::Create()
{
	return	new WHDLL_Loader_WIN32;
}

////////////////////////////////////////////////////////////////////
// WHDLL_Loader_WIN32
////////////////////////////////////////////////////////////////////
WHDLL_Loader_WIN32::WHDLL_Loader_WIN32()
: m_dllhandle(NULL)
{
}
WHDLL_Loader_WIN32::~WHDLL_Loader_WIN32()
{
	Release();
}
int		WHDLL_Loader_WIN32::Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit)
{
	m_nErrUnit	= -1;

	if( cszDLLName )
	{
		if( m_dllhandle )
		{
			// 已经载入了
			return	RST_ERR_ALREADYLOAD;
		}

		m_dllhandle	= LoadLibrary(cszDLLName);
		if( !m_dllhandle )
		{
			// 打印错误
			LPVOID lpMsgBuf;
			FormatMessage( 
				FORMAT_MESSAGE_ALLOCATE_BUFFER | 
				FORMAT_MESSAGE_FROM_SYSTEM | 
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				0, // Default language
				(LPTSTR) &lpMsgBuf,
				0,
				NULL 
			);
			// Display the string.
			fprintf(stderr, "LoadLibrary(%s):%s", cszDLLName, (LPCTSTR)lpMsgBuf);
			// Free the buffer.
			LocalFree( lpMsgBuf );
			// 这样保留了原来*ppFunc中的默认值
			return	RST_ERR_CANNOTOPEN;
		}
	}

	// 顺序载入
	if( aUnit )
	{
		while( aUnit->pcszFuncName )
		{
			// 载入函数
			void	*pFunc	= GetFunc(aUnit->pcszFuncName);
			if( !pFunc )
			{
				return	RST_ERR_CANNOTLOADFUNC;
			}
			*aUnit->ppFunc	= pFunc;
			// 下一个
			aUnit		++;
			m_nErrUnit	++;
		}
	}

	return	RST_OK;
}
void *	WHDLL_Loader_WIN32::GetFunc(const char *cszFuncName)
{
	void	*pFunc	= GetProcAddress(m_dllhandle, cszFuncName);
	if( !pFunc )
	{
		DWORD	dwErrCode	= GetLastError();
		char	buf[1024];
		FormatMessage( 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dwErrCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) buf,
			sizeof(buf)-1,
			NULL 
		);
		wh_strtrim(buf);
		sprintf(m_szLastError, "GetProcAddress %s err:%d, %s", cszFuncName, dwErrCode, buf);
		return	NULL;
	}
	return	pFunc;
}
int		WHDLL_Loader_WIN32::Release()
{
	int	rst	= RST_OK;
	if( m_dllhandle )
	{
		if( !FreeLibrary(m_dllhandle) )
		{
			rst	= RST_ERR_CANNOTCLOSE;
		}
		else
		{
			m_dllhandle	= NULL;
		}
	}
	return	rst;
}

#endif			// EOF WIN32	}
