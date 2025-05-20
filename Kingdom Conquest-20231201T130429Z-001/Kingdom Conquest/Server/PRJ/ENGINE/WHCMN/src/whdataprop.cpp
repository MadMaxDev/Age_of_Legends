// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whdataprop.cpp
// Creator: Wei Hua (κ��)
// Comment: ���ݳ�Ա���ִ����ʽӿ�
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
			// ʲôҲ����������һ�㶼�ǽṹ
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
			// ���Ͳ���
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
			// ��֧�ֵ�����
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
	// �ж��ǲ�����
	char	szClassName[MAX_NAME];
	char	szPropName[MAX_NAME];
	if( isclasstype(cszKey, szClassName, szPropName) )
	{
		// ���Ƿ��ڼ̳��б���
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
							// ����ǿ���������ֵ�ĺ���
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
		// ���Ƿ��ڷ�����
		whdataprop_container	*pContainer = findcontainer(szClassName);
		if( pContainer )
		{
			return	pContainer->setvalue(szPropName, cszValue, nReloadMask);
		}
		// ����û���ҵ�����
		// return	-1; Ȼ���ȥ�������Ƿ�����Ӧ�ı�������
	}

	int		nSize, nType, nExt;
	whdataprop_setvalfunc_t	pSetValFunc;

	// �����̳ж������ҵ���
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
					// ����ǿ���������ֵ�ĺ���
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

	// �������ķֹ��ܶ�����ȥ��
	const whdataprop_map_t		*pMap		= GetPropMap();
	while(pMap)
	{
		pEntry	= pMap->lpEntries;
		// �ҷ��ϵĳ�Ա
		while( pEntry->cszKey )
		{
			if( pEntry->nType==WHDATAPROP_TYPE_mis )
			{
				if( nReloadMask==0 || (nReloadMask&pEntry->nReloadMask)!=0 )
				{
					whdataprop_container	*pObj = (whdataprop_container *)wh_getoffsetaddr(this, pEntry->nValueOffset);
					if( pObj->setvalue(cszKey, cszValue, nReloadMask)==0 )
					{
						// ���óɹ�
						return	0;
					}
				}
			}
			pEntry	++;
		}
		// ���ϼ�
		pMap	= pMap->lpBaseMap;
	}

	return	-1;
}
const char *	whdataprop_container::getvalue(const char *cszKey, char *szValue)
{
	// ��ʵ�ּ򵥵�
	int		nSize, nType, nExt;
	whdataprop_setvalfunc_t	pSetValFunc;

	// �����̳ж������ҵ���
	void	*ptr = findvalueptr(cszKey, &nType, &nExt, &pSetValFunc, &nSize);
	if( ptr )
	{
		switch(WHDATAPROP_TYPE_BASICMASK & nType )
		{
		case	WHDATAPROP_TYPE_setvaluefunc:
			{
				// ����ǿ���������ֵ�ĺ���
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
		// �ҷ��ϵĳ�Ա
		while( pEntry->cszKey )
		{
			if( strcmp(pEntry->cszKey, cszKey)==0 )
			{
				// �ҵ��ˣ��ؼ�
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
		// ���ϼ�
		pMap	= pMap->lpBaseMap;
	}

	// û���ҵ����ʵı�����
	return	NULL;
}
void *	whdataprop_container::findvalueptr(const char *cszKey, int nType, int *pnExt, whdataprop_setvalfunc_t *pSetValFunc, int *pnSize, const whdataprop_entry_t **ppEntry)
{
	const whdataprop_map_t		*pMap		= GetPropMap();
	const whdataprop_entry_t	*pEntry;

	while(pMap)
	{
		pEntry	= pMap->lpEntries;
		// �ҷ��ϵĳ�Ա
		while( pEntry->cszKey )
		{
			if( strcmp(pEntry->cszKey, cszKey)==0 && pEntry->nType==nType )
			{
				// �ҵ��ˣ��ؼ�
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
		// ���ϼ�
		pMap	= pMap->lpBaseMap;
	}

	// û���ҵ����ʵı�����
	return	NULL;
}
// 2007-02-07 ȡ��"."�����Ա�����ʽ�����������⡣
bool	whdataprop_container::isclasstype(const char *cszKey, char *szClassName, char *szPropName)
{
	szPropName[0]	= 0;
	if( wh_strsplit("sa", cszKey, "::", szClassName, szPropName) < 2 )
	{
		// �������ʾ��ʽ
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
	// �ȿ��Ƿ����ڼ̳��б���
	if( findmap(cszClassName) )
	{
		return	this;
	}
	// ���ǲ����ڷ�����
	whdataprop_container *pRst = (whdataprop_container *)findvalueptr(cszClassName, WHDATAPROP_TYPE_mis);
	if( pRst )
	{
		return	pRst;
	}
	// ���ǲ����ڷ���ķ�����
	const whdataprop_map_t		*pMap		= GetPropMap();
	const whdataprop_entry_t	*pEntry;
	while(pMap)
	{
		pEntry	= pMap->lpEntries;
		// �ҷ��ϵĳ�Ա
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
		// ���ϼ�
		pMap	= pMap->lpBaseMap;
	}

	// û���ҵ����ʵı�����
	return	NULL;
}
void *	whdataprop_container::findinterface(const char *cszClassName)
{
	return	findcontainer(cszClassName);
}

