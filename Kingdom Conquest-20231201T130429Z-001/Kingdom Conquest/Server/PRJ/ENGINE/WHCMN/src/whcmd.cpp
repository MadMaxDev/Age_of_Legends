// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whcmd.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 通用命令调用的接口。我参考了MFC的消息映射的实现。
// CreationDate : 2003-12-18
// ChangeLOG    : 2006-08-09 增加了whcmn_nml_console::ThreadFunc中打印提示符时的等待，这样主处理过程中可以在输出结束之后再放开锁定，这时whcmn_nml_console::ThreadFunc才会输出提示符

#include "../inc/whcmd.h"
#include "../inc/whfile.h"
#include "../inc/whtime.h"
#include <assert.h>
#ifdef __GNUC__
#include <readline/readline.h>
#include <readline/history.h>
#endif
using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whcmddealer
////////////////////////////////////////////////////////////////////
void	whcmddealer::CallEntryDealCmd(const whcmd_entry_t *pEntry, const void *pData, size_t nSize)
{
	#define	PARAM_DECLARE(type)		assert(pData); const type *pParamVar = (type *)pData;
	switch( pEntry->nParamType )
	{
		case	WHCMDFUNC_PARAMTYPE_v:
		{
			(this->*(whcmd_fc_v_t)pEntry->pFunc)();
		}
		break;
		case	WHCMDFUNC_PARAMTYPE_p:
		{
			(this->*(whcmd_fc_p_t)pEntry->pFunc)(pData, nSize);
		}
		break;
		case	WHCMDFUNC_PARAMTYPE_s:
		{
			(this->*(whcmd_fc_c_t)pEntry->pFunc)((const char *)pData);
		}
		break;
		case	WHCMDFUNC_PARAMTYPE_i:
		{
			PARAM_DECLARE(whcmd_param_i_t);
			(this->*(whcmd_fc_i_t)pEntry->pFunc)(pParamVar->nParam);
		}
		break;
		case	WHCMDFUNC_PARAMTYPE_ii:
		{
			PARAM_DECLARE(whcmd_param_ii_t);
			(this->*(whcmd_fc_ii_t)pEntry->pFunc)(pParamVar->nParam1, pParamVar->nParam2);
		}
		break;
		case	WHCMDFUNC_PARAMTYPE_iii:
		{
			PARAM_DECLARE(whcmd_param_iii_t);
			(this->*(whcmd_fc_iii_t)pEntry->pFunc)(pParamVar->nParam1, pParamVar->nParam2, pParamVar->nParam3);
		}
		break;
	}
}

class	FindCmdWork
{
public:
	whcmddealer	*pThis;			// 指向whcmddealer，因为Work里面要用
	whcmd_t		nCmd;			// 本次将要处理的指令代码
	const void	*pData;			// 指令的数据
	size_t		nSize;			// 数据的长度
public:
	FindCmdWork(whcmddealer *_pThis, whcmd_t _nCmd, const void *_pData, size_t _nSize)
	: pThis(_pThis), nCmd(_nCmd), pData(_pData), nSize(_nSize)
	{}
	inline bool	Work(const whcmd_entry_t *pEntry)
	{
		if( nCmd == pEntry->nCmd )
		{
			pThis->CallEntryDealCmd(pEntry, pData, nSize);
			return	true;
		}
		return		false;
	}
};
void	whcmddealer::DealCmd(whcmd_t nCmd, const void *pData, size_t nSize)
{
	FindCmdWork	work(this, nCmd, pData, nSize);
	if( !SearchAndDo(work) )
	{
		DealCmd_WhenUnknown(nCmd, pData, nSize);
	}
}

class	FindFuncWork
{
public:
	whcmddealer	*pThis;			// 指向whcmddealer，因为Work里面要用
	const char	*cszFuncName;	// 本次将要处理的函数名称串
	const void	*pData;			// 指令的数据
	size_t		nSize;			// 数据的长度
public:
	FindFuncWork(whcmddealer *_pThis, const char *_cszFuncName, const void *_pData, size_t _nSize)
	: pThis(_pThis), cszFuncName(_cszFuncName), pData(_pData), nSize(_nSize)
	{}
	inline bool	Work(const whcmd_entry_t *pEntry)
	{
		if( strcmp(cszFuncName, pEntry->cszKey)==0 )
		{
			pThis->CallEntryDealCmd(pEntry, pData, nSize);
			return	true;
		}
		return		false;
	}
};
void	whcmddealer::DealCmd(const char *cszFuncName, const void *pData, size_t nSize)
{
	FindFuncWork	work(this, cszFuncName, pData, nSize);
	if( !SearchAndDo(work) )
	{
		DealCmd_WhenUnknown(cszFuncName, pData, nSize);
	}
}

////////////////////////////////////////////////////////////////////
// whcmn_nml_console
////////////////////////////////////////////////////////////////////
whcmn_nml_console::whcmn_nml_console()
: m_tid(INVALID_TID)
{
	strcpy(m_szPrompt, "$ ");
	m_cmdqueue.Init(65536);
}
static void * whcmn_nml_console_ThreadFunc(void *ptr)
{
	whcmn_nml_console	*pConsole	= (whcmn_nml_console *)ptr;
	pConsole->ThreadFunc();
	return	0;
}
int		whcmn_nml_console::StartThread()
{
	if( m_tid!=INVALID_TID )
	{
		// 已经开过了
		assert(0);
		return	-1;
	}

	return	whthread_create(&m_tid, whcmn_nml_console_ThreadFunc, this);
}
int		whcmn_nml_console::WaitThreadEnd(unsigned int nTimeout)
{
	if( m_tid==INVALID_TID )
	{
		// 已经结束了
		return	0;
	}
	// 不能强制关闭线程（那样不会停止，而且连输入都不行了）
	whthread_waitend(m_tid, nTimeout);
	whthread_closetid(m_tid);
	m_tid	= INVALID_TID;
	return	0;
}
int		whcmn_nml_console::ThreadFunc()
{
	while( 1 )
	{
#ifdef __GNUC__
		// 等待上层允许输入
		m_cmdqueue.lock();
		// 提示符
//		printf("%s", m_szPrompt);
		m_cmdqueue.unlock();
	/*	if( !fgets(m_szBuf, sizeof(m_szBuf)-1, stdin) )
		{
			// 输入错误
			break;
		}
	*/
		char * tmp=readline(m_szPrompt);
		if (tmp== 0)
			break;
		add_history(tmp);	
		memset(m_szBuf,0,sizeof(m_szBuf));
		strcpy(m_szBuf,tmp);
#endif
#ifdef WIN32
		// 等待上层允许输入
		m_cmdqueue.lock();
		// 提示符
		printf("%s", m_szPrompt);
		m_cmdqueue.unlock();
		if( !fgets(m_szBuf, sizeof(m_szBuf)-1, stdin) )
		{
			// 输入错误
			break;
		}
#endif
		// 压入队列
		wh_strtrim(m_szBuf);
		m_cmdqueue.In(m_szBuf, strlen(m_szBuf)+1);
		if( stricmp("exit", m_szBuf)==0 )
		{
			// 无论如何都用这个结束
			break;
		}
		// 让主线程能够获得数据
		wh_sleep(100);
	}
	return	0;
}

namespace n_whcmn
{

// 分析测试串
// <randstr>:size		产生size长度的随机字串
// <randbin>:size		产生size长度的随机二进制数据
// <dupstr>:num,str		产生str重复num次的字串
// <dupbin>:num,hexstr	产生hexstr表示的二进制数据重复num次的二进制数据
// <file>:filename		把文件内容读入
int	whcmd_make_tststr(const char *cszStr, whvector<char> *pvect, bool *pbIsStr)
{
	whvector<char>	&vect	= *pvect;
	*pbIsStr	= true;

	const char	*cszKeyCmd	= NULL;
	int			nKeyCmdnLen	= 0;
	cszKeyCmd	= "<randstr>:";
	nKeyCmdnLen	= strlen(cszKeyCmd);
	if( memcmp(cszStr, cszKeyCmd, nKeyCmdnLen)==0 )
	{
		int	nSize	= whstr2int(cszStr+nKeyCmdnLen);
		vect.resize(nSize+1);
		wh_randstr(vect.getbuf(), nSize);
		return	0;
	}

	cszKeyCmd	= "<randbin>:";
	nKeyCmdnLen	= strlen(cszKeyCmd);
	if( memcmp(cszStr, cszKeyCmd, nKeyCmdnLen)==0 )
	{
		int	nSize	= whstr2int(cszStr+nKeyCmdnLen);
		vect.resize(nSize);
		wh_randhex(vect.getbuf(), nSize);
		*pbIsStr	= false;
		return	0;
	}

	cszKeyCmd	= "<dupstr>:";
	nKeyCmdnLen	= strlen(cszKeyCmd);
	if( memcmp(cszStr, cszKeyCmd, nKeyCmdnLen)==0 )
	{
		int		nNum		= 0;
		char	szStr[1024]	= "";
		wh_strsplit("da", cszStr+nKeyCmdnLen, ",", &nNum, szStr);
		int		nStrLen		= strlen(szStr);
		vect.resize(nNum*nStrLen+1);
		char	*pszBuf		= vect.getbuf();
		for(int i=0;i<nNum;i++)
		{
			memcpy(pszBuf, szStr, nStrLen);
			pszBuf	+= nStrLen;
		}
		// 最后的0结尾
		*pszBuf		= 0;
		return	0;
	}

	cszKeyCmd	= "<dupbin>:";
	nKeyCmdnLen	= strlen(cszKeyCmd);
	if( memcmp(cszStr, cszKeyCmd, nKeyCmdnLen)==0 )
	{
		int		nNum		= 0;
		char	szStr[1024]	= "";
		wh_strsplit("da", cszStr+nKeyCmdnLen, ",", &nNum, szStr);
		int		nLen		= wh_str2hex(szStr, (unsigned char*)szStr);
		vect.resize(nNum*nLen);
		char	*pszBuf		= vect.getbuf();
		for(int i=0;i<nNum;i++)
		{
			memcpy(pszBuf, szStr, nLen);
			pszBuf	+= nLen;
		}
		*pbIsStr	= false;
		return	0;
	}

	cszKeyCmd	= "<file>:";
	nKeyCmdnLen	= strlen(cszKeyCmd);
	if( memcmp(cszStr, cszKeyCmd, nKeyCmdnLen)==0 )
	{
		// 打开文件
		if( whfile_readfile(cszStr+nKeyCmdnLen, vect)<0 )
		{
			vect.f_aprintf(64, "can not open file:%s", cszStr+nKeyCmdnLen);
			return	-1;
		}
		*pbIsStr	= false;
		return	0;
	}

	// 就按照原来的字串返回即可
	vect.resize(strlen(cszStr)+1);
	memcpy(vect.getbuf(), cszStr, vect.size());
	return	0;
}

}

