// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdll_linux.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ��̬���ӿ��ʹ��(__GNUC__��)
// CreationDate : 2004-11-25
// ChangeLog    : 2005-11-08 RTLD_LAZY��ΪRTLD_NOW

#ifdef	__GNUC__	//				{

#include "../inc/whdll.h"
#include "../inc/whstring.h"
#include <dlfcn.h>
#include <assert.h>

using namespace n_whcmn;

class	WHDLL_Loader_LINUX	: public	WHDLL_Loader
{
public:
	WHDLL_Loader_LINUX();
	virtual ~WHDLL_Loader_LINUX();
public:
	// Ϊ����ʵ�ֵ�
	virtual int		Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit);
	virtual void *	GetFunc(const char *cszFuncName);
	virtual int		Release();
private:
	void	*m_dllhandle;
};
WHDLL_Loader *	WHDLL_Loader::Create()
{
	return	new WHDLL_Loader_LINUX;
}

////////////////////////////////////////////////////////////////////
// WHDLL_Loader_LINUX
////////////////////////////////////////////////////////////////////
WHDLL_Loader_LINUX::WHDLL_Loader_LINUX()
: m_dllhandle(NULL)
{
}
WHDLL_Loader_LINUX::~WHDLL_Loader_LINUX()
{
	Release();
}
int		WHDLL_Loader_LINUX::Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit)
{
	m_nErrUnit	= -1;

	if( cszDLLName )
	{
		if( m_dllhandle )
		{
			// �Ѿ�������
			return	RST_ERR_ALREADYLOAD;
		}

		// m_dllhandle	= dlopen(cszDLLName, RTLD_LAZY);
		// 2005-11-08 �����lazy�Ļ��ᵼ�������ʱ����Щ����undefined�Ĵ���
		m_dllhandle	= dlopen(cszDLLName, RTLD_NOW);
		if( !m_dllhandle )
		{
			// ����������ԭ��*ppFunc�е�Ĭ��ֵ
			sprintf(m_szLastError, "dlopen(%s):%s", cszDLLName, dlerror());
			fprintf(stderr, "%s%s", m_szLastError, WHLINEEND);
			return	RST_ERR_CANNOTOPEN;
		}
	}

	if( aUnit )
	{
		// ˳������
		m_nErrUnit	= 0;
		while( aUnit->pcszFuncName )
		{
			// ���뺯��
			void	*pFunc	= GetFunc(aUnit->pcszFuncName);
			if( !pFunc )
			{
				return	RST_ERR_CANNOTLOADFUNC;
			}
			*aUnit->ppFunc	= pFunc;
			// ��һ��
			aUnit		++;
			m_nErrUnit	++;
		}
	}

	return	RST_OK;
}
void *	WHDLL_Loader_LINUX::GetFunc(const char *cszFuncName)
{
	void	*pFunc	= dlsym(m_dllhandle, cszFuncName);
	char	*pszError;
	if( (pszError=dlerror()) != NULL )
	{
		sprintf(m_szLastError, "dlsym %s, err:%s", cszFuncName, pszError);
		return	NULL;
	}
	return	pFunc;
}
int		WHDLL_Loader_LINUX::Release()
{
	int	rst	= RST_OK;
	if( m_dllhandle )
	{
		if( dlclose(m_dllhandle)!=0 )
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

#endif			// EOF __GNUC__	}
