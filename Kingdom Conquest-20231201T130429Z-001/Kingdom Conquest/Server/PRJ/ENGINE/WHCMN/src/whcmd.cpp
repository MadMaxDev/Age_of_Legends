// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whcmd.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ͨ��������õĽӿڡ��Ҳο���MFC����Ϣӳ���ʵ�֡�
// CreationDate : 2003-12-18
// ChangeLOG    : 2006-08-09 ������whcmn_nml_console::ThreadFunc�д�ӡ��ʾ��ʱ�ĵȴ�����������������п������������֮���ٷſ���������ʱwhcmn_nml_console::ThreadFunc�Ż������ʾ��

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
	whcmddealer	*pThis;			// ָ��whcmddealer����ΪWork����Ҫ��
	whcmd_t		nCmd;			// ���ν�Ҫ�����ָ�����
	const void	*pData;			// ָ�������
	size_t		nSize;			// ���ݵĳ���
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
	whcmddealer	*pThis;			// ָ��whcmddealer����ΪWork����Ҫ��
	const char	*cszFuncName;	// ���ν�Ҫ����ĺ������ƴ�
	const void	*pData;			// ָ�������
	size_t		nSize;			// ���ݵĳ���
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
		// �Ѿ�������
		assert(0);
		return	-1;
	}

	return	whthread_create(&m_tid, whcmn_nml_console_ThreadFunc, this);
}
int		whcmn_nml_console::WaitThreadEnd(unsigned int nTimeout)
{
	if( m_tid==INVALID_TID )
	{
		// �Ѿ�������
		return	0;
	}
	// ����ǿ�ƹر��̣߳���������ֹͣ�����������붼�����ˣ�
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
		// �ȴ��ϲ���������
		m_cmdqueue.lock();
		// ��ʾ��
//		printf("%s", m_szPrompt);
		m_cmdqueue.unlock();
	/*	if( !fgets(m_szBuf, sizeof(m_szBuf)-1, stdin) )
		{
			// �������
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
		// �ȴ��ϲ���������
		m_cmdqueue.lock();
		// ��ʾ��
		printf("%s", m_szPrompt);
		m_cmdqueue.unlock();
		if( !fgets(m_szBuf, sizeof(m_szBuf)-1, stdin) )
		{
			// �������
			break;
		}
#endif
		// ѹ�����
		wh_strtrim(m_szBuf);
		m_cmdqueue.In(m_szBuf, strlen(m_szBuf)+1);
		if( stricmp("exit", m_szBuf)==0 )
		{
			// ������ζ����������
			break;
		}
		// �����߳��ܹ��������
		wh_sleep(100);
	}
	return	0;
}

namespace n_whcmn
{

// �������Դ�
// <randstr>:size		����size���ȵ�����ִ�
// <randbin>:size		����size���ȵ��������������
// <dupstr>:num,str		����str�ظ�num�ε��ִ�
// <dupbin>:num,hexstr	����hexstr��ʾ�Ķ����������ظ�num�εĶ���������
// <file>:filename		���ļ����ݶ���
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
		// ����0��β
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
		// ���ļ�
		if( whfile_readfile(cszStr+nKeyCmdnLen, vect)<0 )
		{
			vect.f_aprintf(64, "can not open file:%s", cszStr+nKeyCmdnLen);
			return	-1;
		}
		*pbIsStr	= false;
		return	0;
	}

	// �Ͱ���ԭ�����ִ����ؼ���
	vect.resize(strlen(cszStr)+1);
	memcpy(vect.getbuf(), cszStr, vect.size());
	return	0;
}

}

