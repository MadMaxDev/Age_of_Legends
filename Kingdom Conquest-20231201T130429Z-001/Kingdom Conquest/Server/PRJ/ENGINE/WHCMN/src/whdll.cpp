// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdll.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 动态链接库的公共部分
// CreationDate : 2007-09-21
// ChangeLog    : 

#include "../inc/whhash.h"
#include "../inc/whdll.h"

namespace n_whcmn
{

class	WHDLL_Loader_Man_I	: public WHDLL_Loader_Man
{
protected:

	// 索引对象
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
			assert(0);	// 永远不应该调用这个函数
			return	-1;
		}
		virtual void *	GetFunc(const char *cszFuncName)
		{
			return	m_pMain->GetFunc(cszFuncName);
		}
		virtual int		Release()
		{
			// 不用释放
			return	0;
		}
	};
protected:
	typedef	whhash<whstr4hash, WHDLL_Loader *>	HASH_T;
	HASH_T	m_mapName2Loader;
public:
	~WHDLL_Loader_Man_I()
	{
		// 把所有载入的DLL都释放了吧
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
		// 先看名字是否存在
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
			// 把名字和Loader对应关系加入
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
