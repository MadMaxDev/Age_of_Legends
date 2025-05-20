// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whcmn.cpp
// Creator: Wei Hua (魏华)
// Comment: whcmn公用的定义
// CreationDate: 2003-05-15

#include "../inc/whcmn_def.h"
#include "../inc/whtime.h"
#include "../inc/whfile.h"
#include "../inc/whthread.h"

//added by yuezhongyue 2011-08-10,dll间g_pLocalLogWriter指针共享
#include "../inc/whcmn_locallogger.h"

namespace n_whcmn
{

#ifdef	__GNUC__
void *S_GODDAMNLINUXWARNING::whoffsetdummpptr	= 0;
#endif

const char *	wh_getenv(const char *cszKey)
{
#ifdef  __GNUC__
	return	getenv(cszKey);
#endif
#ifdef	WIN32
	static char	szVal[1024];
	szVal[0]	= 0;
	GetEnvironmentVariable(cszKey, szVal, sizeof(szVal)-1);
	return	szVal;
#endif
}
// 设置环境变量
void			wh_setenv(const char *cszKey, const char *cszVal)
{
#ifdef  __GNUC__
	setenv(cszKey, cszVal, 1);
#endif
#ifdef	WIN32
	SetEnvironmentVariable(cszKey, cszVal);
#endif
}

}		// EOF namespace n_whcmn

using namespace n_whcmn;
////////////////////////////////////////////////////////////////////
// 为DLL之间信息同步
////////////////////////////////////////////////////////////////////
struct	WHCMN_STATIC_INFO_T
{
	void						*pTimeInfo;
	void						*pFileInfo;
	void						*pThreadInfo;
	unsigned char				bDbgLvl;
	int							nNotify;		// 用于内部通知最上层，现在出现了某些问题
	// （日志函数指针）为所有子Logic提供写日志的功能（这个由上层自由设置）
	int							(*fn_Log_WriteFmt)(int nID, const char *szFmt, ...);
	// 本地字符集
	char						szCharSet[16];

	//added by yuezhongyue 2011-08-10,dll间g_pLocalLogWriter指针共享
	whlogwriter*				pLocalLogWriter;

	static int					dummy_Log_WriteFmt(int nID, const char *szFmt, ...)
	{
		return	0;
	}
	WHCMN_STATIC_INFO_T()
		: pTimeInfo(NULL)
		, pFileInfo(NULL)
		, pThreadInfo(NULL)
		, bDbgLvl(0)
		, nNotify(0)
		, fn_Log_WriteFmt(&WHCMN_STATIC_INFO_T::dummy_Log_WriteFmt)
	{
		szCharSet[0]			= 0;
	}
};
static WHCMN_STATIC_INFO_T		l_si;
static WHCMN_STATIC_INFO_T		*l_psi	= &l_si;

void *	WHCMN_STATIC_INFO_Out()
{
	// 时间相关
	l_si.pTimeInfo				= WHCMN_TIME_STATIC_INFO_Out();
	// 文件系统相关
	l_si.pFileInfo				= WHCMN_FILE_STATIC_INFO_Out();
	// 线程相关
	l_si.pThreadInfo			= WHCMN_THREAD_STATIC_INFO_Out();

	//added by yuezhongyue 2011-08-10,dll间g_pLocalLogWriter指针共享
	l_si.pLocalLogWriter		= WHCMN_Get_LocalLogWriter();

	return	&l_si;
}
#ifdef	_DEBUG
unsigned char	WHCMN_GET_DBGLVL()
{
	return	l_psi->bDbgLvl;
}
void			WHCMN_SET_DBGLVL(unsigned char nLvl)
{
	l_psi->bDbgLvl	= nLvl;
}
#endif
int				WHCMN_GET_NOTIFY()
{
	return	l_psi->nNotify;
}
void			WHCMN_SET_NOTIFY(int nNotify)
{
	l_psi->nNotify	= nNotify;
}

void	WHCMN_Set_Log_WriteFmt( LOG_WRITEFMT_FUNC_T pFunc )
{
	l_psi->fn_Log_WriteFmt	= pFunc;
}
LOG_WRITEFMT_FUNC_T	WHCMN_Get_Log_WriteFmt()
{
	return	l_psi->fn_Log_WriteFmt;
}
const char *	WHCMN_getcharset()
{
	return	l_psi->szCharSet;
}
void	WHCMN_setcharset(const char *cszName)
{
	// 上面不会变态到给个过长的字串吧
	strcpy(l_psi->szCharSet, cszName);
}
void	_WHCMN_STATIC_INFO_In(void *pInfo)
{
	if( !pInfo )
	{
		return;
	}
	WHCMN_STATIC_INFO_T			*psi	= (WHCMN_STATIC_INFO_T *)pInfo;
	// 时间相关
	WHCMN_TIME_STATIC_INFO_In(psi->pTimeInfo);
	// 文件系统相关
	WHCMN_FILE_STATIC_INFO_In(psi->pFileInfo);
	// 线程相关
	WHCMN_THREAD_STATIC_INFO_In(psi->pThreadInfo);

	//added by yuezhongyue 2011-08-10,dll间g_pLocalLogWriter指针共享
	WHCMN_Set_LocalLogWriter(psi->pLocalLogWriter);

	// 自己要用的
	l_psi						= psi;
}
