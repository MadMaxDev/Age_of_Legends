// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdll.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ��̬���ӿ�Ĺ�������
// CreationDate : 2007-09-21
// ChangeLog    : 

#include "../inc/whhash.h"
#include "../inc/whdll.h"

namespace n_whcmn
{

class	WHDLL_Loader_Man_I	: public WHDLL_Loader_Man
{
protected:

	// ��������
	class	WHDLL_Loader_Ref	: public WHDLL_Loader
	{
	protected:
		WHDLL_Loader	*m_pMain;
	public:
		WHDLL_Loader_Ref(WHDLL_Loader *pMain)
			: m_pMain(pMain)
		{
			assert(m_pMain);
		}
		~WHDLL_Loader_Ref()
		{
		}
		virtual int		Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit)
		{
			assert(0);	// ��Զ��Ӧ�õ����������
			return	-1;
		}
		virtual void *	GetFunc(const char *cszFuncName)
		{
			return	m_pMain->GetFunc(cszFuncName);
		}
		virtual int		Release()
		{
			// �����ͷ�
			return	0;
		}
	};
protected:
	typedef	whhash<whstr4hash, WHDLL_Loader *>	HASH_T;
	HASH_T	m_mapName2Loader;
public:
	~WHDLL_Loader_Man_I()
	{
		// �����������DLL���ͷ��˰�
		Release();
	}
	int		Release()
	{
		for(HASH_T::kv_iterator it=m_mapName2Loader.begin(); it!=m_mapName2Loader.end(); ++it)
		{
			delete	it.getvalue();
		}
		m_mapName2Loader.clear();
		return	0;
	}
	virtual WHDLL_Loader *		Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit)
	{
		// �ȿ������Ƿ����
		WHDLL_Loader	*pMain	= NULL;
		if( !m_mapName2Loader.get(cszDLLName, pMain) )
		{
			pMain	= WHDLL_Loader::Create();
			int	rst	= pMain->Load(cszDLLName, aUnit);
			if( rst<0 )
			{
				delete	pMain;
				return	NULL;
			}
			// �����ֺ�Loader��Ӧ��ϵ����
			m_mapName2Loader.put(cszDLLName, pMain);
		}
		else
		{
			int	rst	= pMain->Load(NULL, aUnit);
			if( rst<0 )
			{
				return	NULL;
			}
		}
		return	new WHDLL_Loader_Ref(pMain);
	}
};

WHDLL_Loader_Man *	WHDLL_Loader_Man::Create()
{
	return	new WHDLL_Loader_Man_I;
}

}	// EOF namespace n_whcmn
