// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whdataprop.cpp
// Creator: Wei Hua (魏华)
// Comment: 数据成员的字串访问接口
// CreationDate: 2003-11-08

#include "../inc/whdataprop.h"
#include "../inc/whcmn_def.h"
#include "../inc/whstring.h"

namespace n_whcmn
{

int	whdataprop_setvalue(void *pValue, int nSize, int nType, int nExt, const char *szVal)
{
	#define	DEALARR(type)		\
		if( bIsArray )													\
		{																\
			wh_strsplittoarray(szVal, ",", (type *)pValue, nExt);		\
		}
	#define	DEALCMN(type)		\
		else															\
		{																\
			wh_strsetvalue(*(type*)pValue, szVal);						\
		}
	#define	DEALCHARPTR			\
		else															\
		{																\
			wh_strsetvalue_charptr((char*)pValue, nSize, szVal);		\
		}
	#define	DEAL_ARRCHARPTR		\
		DEALARR(char *)			\
		DEALCHARPTR
	#define	DEAL_ARRCMN(type)	\
		DEALARR(type)			\
		DEALCMN(type)
	#define	CASE_CMN_TYPE(type)	\
		case	WHDATAPROP_TYPE_##type:									\
		{																\
			DEAL_ARRCMN(type);											\
		}																\
		break;

	bool	bIsArray = (WHDATAPROP_TYPE_ARRAYBIT & nType) != 0 ;
	switch(WHDATAPROP_TYPE_BASICMASK & nType)
	{
		case	WHDATAPROP_TYPE_glbsetvaluefunc:
		{
			((whdataprop_glbsetvaluefunc_t)nExt)(pValue, szVal);
		}
		break;
		case	WHDATAPROP_TYPE_charptr:
		{
			DEAL_ARRCHARPTR;
		}
		break;
		CASE_CMN_TYPE(bool)
		CASE_CMN_TYPE(whbyte)
		CASE_CMN_TYPE(short)
		CASE_CMN_TYPE(int)
		CASE_CMN_TYPE(float)
		CASE_CMN_TYPE(voidptr)
		CASE_CMN_TYPE(time_t)
		CASE_CMN_TYPE(whint64)
		CASE_CMN_TYPE(string)
		case	WHDATAPROP_TYPE_unknowntype:
			// 什么也不作，这种一般都是结构
		break;
		case	WHDATAPROP_TYPE_rgint:
		{
			int		vFrom=0, vTo=0;
			wh_strsplit("dd", szVal, ",", &vFrom, &vTo);
			((WHRANGE_T<int>*)pValue)->Set(vFrom, vTo);
		}
		break;
		case	WHDATAPROP_TYPE_rgfloat:
		{
			float	vFrom=0, vTo=0;
			wh_strsplit("ff", szVal, ",", &vFrom, &vTo);
			((WHRANGE_T<float>*)pValue)->Set(vFrom, vTo);
		}
		break;
		default:
			// 类型不对
			return	-1;
		break;
	}

	return	0;
}
const char *	whdataprop_getvalue(const void *pValue, int nSize, int nType, int nExt, char *szVal)
{
	#define	G_DEALARR(type)		\
	if( bIsArray )													\
	{																\
		return	"Array type (NotSupport)";							\
	}
	#define	G_DEALCMN(type)		\
	else															\
	{																\
		return	wh_strgetvalue(*(type*)pValue, szVal);				\
	}
	#define	G_DEALCHARPTR		\
	else															\
	{																\
		return	(const char*)pValue;								\
	}
	#define	G_DEAL_ARRCHARPTR	\
		G_DEALARR(char *)		\
		G_DEALCHARPTR
	#define	G_DEAL_ARRCMN(type)	\
		G_DEALARR(type)			\
		G_DEALCMN(type)
	#define	G_CASE_CMN_TYPE(type)	\
	case	WHDATAPROP_TYPE_##type:									\
	{																\
		G_DEAL_ARRCMN(type);										\
	}																\
	break;

	bool	bIsArray = (WHDATAPROP_TYPE_ARRAYBIT & nType) != 0 ;
	switch(WHDATAPROP_TYPE_BASICMASK & nType)
	{
		case	WHDATAPROP_TYPE_charptr:
		{
			G_DEAL_ARRCHARPTR;
		}
		break;
		G_CASE_CMN_TYPE(bool)
		G_CASE_CMN_TYPE(whbyte)
		G_CASE_CMN_TYPE(short)
		G_CASE_CMN_TYPE(int)
		G_CASE_CMN_TYPE(float)
		G_CASE_CMN_TYPE(voidptr)
		G_CASE_CMN_TYPE(time_t)
		G_CASE_CMN_TYPE(whint64)
		G_CASE_CMN_TYPE(string)
		case	WHDATAPROP_TYPE_rgint:
		{
			sprintf(szVal, "%d,%d", ((WHRANGE_T<int>*)pValue)->GetFrom(), ((WHRANGE_T<int>*)pValue)->GetTo());
		}
		break;
		case	WHDATAPROP_TYPE_rgfloat:
		{
			sprintf(szVal, "%.4f,%.4f", ((WHRANGE_T<float>*)pValue)->GetFrom(), ((WHRANGE_T<float>*)pValue)->GetTo());
		}
		break;
		default:
			// 不支持的类型
			return	"NotSupport";
		break;
	}

	return	"";
}

}	// EOF namespace n_whcmn


using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whdataprop_container
////////////////////////////////////////////////////////////////////
int		whdataprop_container::setvalue(const char *cszKey, const char *cszValue, unsigned int nReloadMask)
{
	// 判断是不是类
	char	szClassName[MAX_NAME];
	char	szPropName[MAX_NAME];
	if( isclasstype(cszKey, szClassName, szPropName) )
	{
		// 看是否在继承列表中
		const whdataprop_map_t	*pMap = findmap(szClassName);
		if( pMap )
		{
			const whdataprop_entry_t	*pEntry = findentry(pMap, szPropName);
			if( pEntry )
			{
				if( nReloadMask==0 || (nReloadMask&pEntry->nReloadMask)!=0 )
				{
					void	*ptr = wh_getoffsetaddr(this, pEntry->nValueOffset);
					switch(WHDATAPROP_TYPE_BASICMASK & pEntry->nType )
					{
						case	WHDATAPROP_TYPE_setvaluefunc:
						{
							// 这个是可以用来设值的函数
							return	(this->*pEntry->pSetValFunc)(cszValue);
						}
						break;
					}
					return	whdataprop_setvalue(
							ptr
							, pEntry->nValueSize
							, pEntry->nType, pEntry->nExt
							, cszValue
							);
				}
				else
				{
					return	0;
				}
			}
		}
		// 看是否在仿类中
		whdataprop_container	*pContainer = findcontainer(szClassName);
		if( pContainer )
		{
			return	pContainer->setvalue(szPropName, cszValue, nReloadMask);
		}
		// 但是没有找到类名
		// return	-1; 然后就去后面招是否有相应的变量名吧
	}

	int		nSize, nType, nExt;
	whdataprop_setvalfunc_t	pSetValFunc;

	// 在主继承队列中找到了
	const whdataprop_entry_t	*pEntry;
	void	*ptr = findvalueptr(cszKey, &nType, &nExt, &pSetValFunc, &nSize, &pEntry);
	if( ptr )
	{
		if( nReloadMask==0 || (nReloadMask&pEntry->nReloadMask)!=0 )
		{
			switch(WHDATAPROP_TYPE_BASICMASK & nType )
			{
				case	WHDATAPROP_TYPE_setvaluefunc:
				{
					// 这个是可以用来设值的函数
					return	(this->*pSetValFunc)(cszValue);
				}
				break;
			}
			return	whdataprop_setvalue(ptr, nSize, nType, nExt, cszValue);
		}
		else
		{
			return	0;
		}
	}

	// 到各级的分功能对象中去找
	const whdataprop_map_t		*pMap		= GetPropMap();
	while(pMap)
	{
		pEntry	= pMap->lpEntries;
		// 找符合的成员
		while( pEntry->cszKey )
		{
			if( pEntry->nType==WHDATAPROP_TYPE_mis )
			{
				if( nReloadMask==0 || (nReloadMask&pEntry->nReloadMask)!=0 )
				{
					whdataprop_container	*pObj = (whdataprop_container *)wh_getoffsetaddr(this, pEntry->nValueOffset);
					if( pObj->setvalue(cszKey, cszValue, nReloadMask)==0 )
					{
						// 设置成功
						return	0;
					}
				}
			}
			pEntry	++;
		}
		// 找上级
		pMap	= pMap->lpBaseMap;
	}

	return	-1;
}
const char *	whdataprop_container::getvalue(const char *cszKey, char *szValue)
{
	// 先实现简单的
	int		nSize, nType, nExt;
	whdataprop_setvalfunc_t	pSetValFunc;

	// 在主继承队列中找到了
	void	*ptr = findvalueptr(cszKey, &nType, &nExt, &pSetValFunc, &nSize);
	if( ptr )
	{
		switch(WHDATAPROP_TYPE_BASICMASK & nType )
		{
		case	WHDATAPROP_TYPE_setvaluefunc:
			{
				// 这个是可以用来设值的函数
				return	"ThisIsAFunc";
			}
			break;
		}
		return	whdataprop_getvalue(ptr, nSize, nType, nExt, szValue);
	}
	return	"NotFound";
}
void *	whdataprop_container::findvalueptr(const char *cszKey, int *pnType, int *pnExt, whdataprop_setvalfunc_t *pSetValFunc, int *pnSize, const whdataprop_entry_t **ppEntry)
{
	const whdataprop_map_t		*pMap		= GetPropMap();
	const whdataprop_entry_t	*pEntry;

	while(pMap)
	{
		pEntry	= pMap->lpEntries;
		// 找符合的成员
		while( pEntry->cszKey )
		{
			if( strcmp(pEntry->cszKey, cszKey)==0 )
			{
				// 找到了，回家
				if( pnType )
				{
					*pnType	= pEntry->nType;
				}
				if( pnExt )
				{
					*pnExt	= pEntry->nExt;
				}
				if( pSetValFunc )
				{
					*pSetValFunc	= pEntry->pSetValFunc;
				}
				if( pnSize )
				{
					*pnSize	= pEntry->nValueSize;
				}
				if( ppEntry )
				{
					*ppEntry	= pEntry;
				}
				return	wh_getoffsetaddr(this, pEntry->nValueOffset);
			}
			pEntry	++;
		}
		// 找上级
		pMap	= pMap->lpBaseMap;
	}

	// 没有找到合适的变量名
	return	NULL;
}
void *	whdataprop_container::findvalueptr(const char *cszKey, int nType, int *pnExt, whdataprop_setvalfunc_t *pSetValFunc, int *pnSize, const whdataprop_entry_t **ppEntry)
{
	const whdataprop_map_t		*pMap		= GetPropMap();
	const whdataprop_entry_t	*pEntry;

	while(pMap)
	{
		pEntry	= pMap->lpEntries;
		// 找符合的成员
		while( pEntry->cszKey )
		{
			if( strcmp(pEntry->cszKey, cszKey)==0 && pEntry->nType==nType )
			{
				// 找到了，回家
				if( pnExt )
				{
					*pnExt	= pEntry->nExt;
				}
				if( pSetValFunc )
				{
					*pSetValFunc	= pEntry->pSetValFunc;
				}
				if( pnSize )
				{
					*pnSize	= pEntry->nValueSize;
				}
				if( ppEntry )
				{
					*ppEntry	= pEntry;
				}
				return	wh_getoffsetaddr(this, pEntry->nValueOffset);
			}
			pEntry	++;
		}
		// 找上级
		pMap	= pMap->lpBaseMap;
	}

	// 没有找到合适的变量名
	return	NULL;
}
// 2007-02-07 取消"."的类成员表达形式，以免造成误解。
bool	whdataprop_container::isclasstype(const char *cszKey, char *szClassName, char *szPropName)
{
	szPropName[0]	= 0;
	if( wh_strsplit("sa", cszKey, "::", szClassName, szPropName) < 2 )
	{
		// 不是类表示形式
		return	false;
	}
	return	true;
}
const whdataprop_map_t *	whdataprop_container::findmap(const char *cszClassName)
{
	const whdataprop_map_t		*pMap	= GetPropMap();

	while(pMap)
	{
		if( strcmp(pMap->cszClassName, cszClassName)==0 )
		{
			return	pMap;
		}
		pMap	= pMap->lpBaseMap;
	}
	return	NULL;
}
const whdataprop_entry_t *	whdataprop_container::findentry(const whdataprop_map_t *pMap, const char *cszKeyName)
{
	const whdataprop_entry_t * pEntry = pMap->lpEntries;
	while(pEntry)
	{
		if( strcmp(pEntry->cszKey, cszKeyName)==0 )
		{
			return	pEntry;
		}
		pEntry	++;
	}
	return	NULL;
}
whdataprop_container *	whdataprop_container::findcontainer(const char *cszClassName)
{
	// 先看是否不是在继承列表中
	if( findmap(cszClassName) )
	{
		return	this;
	}
	// 看是不是在仿类中
	whdataprop_container *pRst = (whdataprop_container *)findvalueptr(cszClassName, WHDATAPROP_TYPE_mis);
	if( pRst )
	{
		return	pRst;
	}
	// 看是不是在仿类的仿类中
	const whdataprop_map_t		*pMap		= GetPropMap();
	const whdataprop_entry_t	*pEntry;
	while(pMap)
	{
		pEntry	= pMap->lpEntries;
		// 找符合的成员
		while( pEntry->cszKey )
		{
			if( pEntry->nType==WHDATAPROP_TYPE_mis )
			{
				pRst	= (whdataprop_container *)wh_getoffsetaddr(this, pEntry->nValueOffset);
				pRst	= pRst->findcontainer(cszClassName);
				if( pRst )
				{
					return	pRst;
				}
			}
			pEntry	++;
		}
		// 找上级
		pMap	= pMap->lpBaseMap;
	}

	// 没有找到合适的变量名
	return	NULL;
}
void *	whdataprop_container::findinterface(const char *cszClassName)
{
	return	findcontainer(cszClassName);
}

