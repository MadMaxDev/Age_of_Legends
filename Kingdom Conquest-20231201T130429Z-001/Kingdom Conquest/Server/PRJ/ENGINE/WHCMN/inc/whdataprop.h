// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataprop.h
// Creator      : Wei Hua (κ��)
// Comment      : ���ݳ�Ա���ִ����ʽӿڡ��Ҳο���MFC����Ϣӳ���ʵ�֡�
//                ʹ��WHDATAPROP_MAP_BEGIN*ʱ�����Ŀ��ṹ������Ľṹ����Ҫ��ȫ����������::�ṹ���ο���GMS_I::CFGINFO_T
// CreationDate : 2003-11-08
// ChangeLOG    : 2006-11-27 ��setvaluefunc�����int����ֵ��
//              : 2007-02-06 ȡ����ԭ��û�õĶ��躯��������صĹ���

#ifndef	__WHDATAPROP_H__
#define	__WHDATAPROP_H__

#include <stddef.h>
#include <assert.h>
#include "whcmn_def.h"
#include "whvector.h"

namespace n_whcmn
{

class	whdataprop_container;
// ����������͵Ĳ���
// ������������(fc��FunctionCall����д)
// ��������ֵ�ĺ���(�������<0��˵�����˷ǳ����صĴ���)
typedef	int		(whdataprop_container::*whdataprop_setvalfunc_t)(const char *cszVal);
typedef	int		(*whdataprop_glbsetvaluefunc_t)(void *var, const char *cszVal);
// ������������
enum
{
	// ��Ҫʹ�ú���
	WHDATAPROP_TYPE_FUNC					= 0,
	// ��������(��Щ��Сд����Ϊ�˵�ʱ��ͽű����ʱ������ƥ��)
	WHDATAPROP_TYPE_bool					= 0x0001,
	WHDATAPROP_TYPE_whbyte					= 0x0002,
	WHDATAPROP_TYPE_short					= 0x0003,
	WHDATAPROP_TYPE_int						= 0x0004,
	WHDATAPROP_TYPE_float					= 0x0005,
	WHDATAPROP_TYPE_voidptr					= 0x0006,
	WHDATAPROP_TYPE_charptr					= 0x0007,		// ע�⣬char*����������ͨ���������ͣ���char szStr[10][20];�������
															// ������char *szStr[10];����ʽ
															// charptr�ĵ����г������ƹ��ܣ���������õ����������˱�������ĳ��������ᱻ�ض�(�������ҿ������)
															// charptr����û�м�鳤�ȵĹ���
	WHDATAPROP_TYPE_time_t					= 0x0008,		// ʱ��(��)
	WHDATAPROP_TYPE_whint64					= 0x0009,		// 64λ����
	WHDATAPROP_TYPE_string					= 0x0010,		// std::string
	WHDATAPROP_TYPE_setvaluefunc			= 0x0011,		// ����ָ��Ϊwhdataprop_setvalfunc_t�ͣ�����pSetValFunc�У��ú�����������ͨ���ִ�����ֵ
	WHDATAPROP_TYPE_glbsetvaluefunc			= 0x0012,		// �ǳ�Ա�ĺ���ָ�룬Ϊwhdataprop_glbsetvaluefunc_t�ͣ�����nExt�У��ú�����������ͨ���ִ�����ֵ
	WHDATAPROP_TYPE_rgint					= 0x0021,		// ������Χ(���õľ���������)
	WHDATAPROP_TYPE_rgfloat					= 0x0022,		// ���㷶Χ
	WHDATAPROP_TYPE_unknowntype				= 0x0080,		// δ֪���ͣ�һ�������ں��ṹ�����whdataini��ʵ��
	WHDATAPROP_TYPE_mis						= 0x0081,		// ����ģ���̳е����Ա(mis����multi-inheritation simulation)
															// ��Ӧ�ĳ�Ա����һ��Ҳ�Ǵ�container����������
	// �������͵�����
	WHDATAPROP_TYPE_BASICMASK				= 0x00FF,
	// �����λ���ñ�ʾ����Ӧ���͵����飬����Ԫ�ؼ��ö��ŷָ�
	// �磺
	// 0x1002������������
	// 0x1001�����ִ������飨�Ѿ�����õģ��㹻��ģ�
	WHDATAPROP_TYPE_ARRAYBIT				= 0x0100,
};

// ������������ֵ
int	whdataprop_setvalue(void *pValue, int nSize, int nType, int nExt, const char *szVal);
// ���ֵ�ִ�
const char *	whdataprop_getvalue(const void *pValue, int nSize, int nType, int nExt, char *szVal);

struct	whdataprop_entry_t
{
	const char	*cszKey;
	int			nType;										// WHDATAPROP_TYPE_XXX
	int			nValueOffset;								// �������ͨ���ͣ����ָ������������ͷ��ƫ��
	int			nValueSize;									// �ñ����ĳ���(��Ҫ�Ǳ�֤�ִ�������)
	int			nExt;										// �����array�����Ԫ����ĿΪnExt��Ԫ�ؼ��Կո�Ϊ�ָ�����
															// Ҳ������ȫ�ֵ�ֵ��������
	whdataprop_setvalfunc_t	pSetValFunc;					// ��������ֵ�ĺ���
	unsigned int	nReloadMask;							// ���������mask��0�Һ�������ص��������������
};
struct	whdataprop_map_t
{
	const char					* cszClassName;				// ���������
	const whdataprop_map_t		* lpBaseMap;				// ָ�����whdataprop_map_t�ṹ
	const whdataprop_entry_t	* lpEntries;				// ����������������
};

class	whdataprop_container
{
	enum
	{
		MAX_NAME				= 256,
	};
public:
	virtual ~whdataprop_container()							{}
	virtual int		setvalue(const char *cszKey, const char *cszValue, unsigned int nReloadMask=0);
	const char *	getvalue(const char *cszKey, char *szValue);
	// ͨ���ִ����ұ�������ʵ��ַ
	virtual	void *	findvalueptr(const char *cszKey, int *pnType=NULL, int *pnExt=NULL, whdataprop_setvalfunc_t *pSetValFunc=NULL, int *pnSize=NULL, const whdataprop_entry_t **ppEntry=NULL);
	// �ҵ����ֺ����Ͷ�ƥ��ı�����ʵ��ַ
	virtual	void *	findvalueptr(const char *cszKey, int nType, int *pnExt=NULL, whdataprop_setvalfunc_t *pSetValFunc=NULL, int *pnSize=NULL, const whdataprop_entry_t **ppEntry=NULL);
	// �ж��Ƿ�����ĸ�ʽ������XX::xx�ĸ�ʽ
	virtual	bool	isclasstype(const char *cszKey, char *szClassName, char *szPropName);
	// �ҵ������ӳ��
	virtual	const whdataprop_map_t *	findmap(const char *cszClassName);
	// ͨ��map�ҵ�entry
	virtual	const whdataprop_entry_t *	findentry(const whdataprop_map_t *pMap, const char *cszKeyName);
	// �ҵ������ָ��
	virtual	whdataprop_container *	findcontainer(const char *cszClassName);
	// �ҵ�MIS��ָ��
	virtual	void *	findinterface(const char *cszClassName);

	// ��Ҫ��ӳ��ı�������(������������������ú�������������д��)
	//{{
	//}}
	// ��Ҫ��������ҪWHDATAPROP_DECLARE_MAP();
	virtual const whdataprop_map_t * GetPropMap() const
	{
		// ������಻ʵ�־ͷ��ؿ�ָ��
		return	0;
	}
};

////////////////////////////////////////////////////////////
// �������ݻ���ӳ�����ĸ�����
////////////////////////////////////////////////////////////
// ����Ҫ�����ݳ�Աӳ�������һ��Ҫд���
// ���ඨ������Ҫ����仰
#define	WHDATAPROP_DECLARE_MAP(_theClass)					\
private:													\
	typedef	_theClass	theClass;							\
	static const whdataprop_entry_t	_propEntries[];			\
protected:													\
	static const whdataprop_map_t			propMap;		\
public:														\
	virtual const whdataprop_map_t * GetPropMap() const;	\
	// **** EOF WHDATAPROP_DECLARE_MAP

// ��ͨ���ӳ�䶨��
#define	WHDATAPROP_MAP_BEGIN0(_theClass, _baseClasspropMapPtr)				\
	const whdataprop_map_t * _theClass::GetPropMap() const	\
		{ return &_theClass::propMap; }						\
	const whdataprop_map_t _theClass::propMap	=			\
		{													\
			#_theClass, _baseClasspropMapPtr, _theClass::_propEntries		\
		};													\
	const whdataprop_entry_t _theClass::_propEntries[]	=	\
	{														\
	// **** EOF WHDATAPROP_MAP_BEGIN0
#define	WHDATAPROP_MAP_BEGIN(_theClass, _baseClass)			\
	WHDATAPROP_MAP_BEGIN0(_theClass, &_baseClass::propMap)	\
	// **** EOF WHDATAPROP_MAP_BEGIN

// ����ӳ�䶨��(����Ҫ������Ա�������)
#define	WHDATAPROP_MAP_BEGIN_AT_ROOT(_theClass)				\
	WHDATAPROP_MAP_BEGIN0(_theClass, NULL)					\
	// **** EOF WHDATAPROP_MAP_BEGIN_AT_ROOT

// ӳ�䶨�����
#define WHDATAPROP_MAP_END()								\
		{ 0, 0, 0, 0, 0, 0, 0 }								\
	};														\
	// **** EOF WHDATAPROP_MAP_END

////////////////////////////////////////////////////////////////////
// ������ڶ���
////////////////////////////////////////////////////////////////////
// ������������ӳ��
#define	WHDATAPROP_ON_SETVALUE0(cszKey, nType, mVar, nExt, nReloadMask)	\
	{ cszKey, nType, wh_offsetof(theClass, mVar), wh_sizeinclass(theClass, mVar), nExt, 0, nReloadMask },	\
	// **** EOF WHDATAPROP_ON_SETVALUE0
// ���ֱ���ñ�������key��
#define	WHDATAPROP_ON_SETVALUE(nType, mVar, nExt, nReloadMask)			\
	WHDATAPROP_ON_SETVALUE0(#mVar, nType, mVar, nExt, nReloadMask)		\
	// **** EOF WHDATAPROP_ON_SETVALUE
#define	WHDATAPROP_ON_SETVALUE_smp(nRealType, mVar, nExt)	\
	WHDATAPROP_ON_SETVALUE(WHDATAPROP_TYPE_##nRealType, mVar, nExt, 0)	\
	// **** EOF WHDATAPROP_ON_SETVALUE
#define	WHDATAPROP_ON_SETVALUE_smp_reload1(nRealType, mVar, nExt)		\
	WHDATAPROP_ON_SETVALUE(WHDATAPROP_TYPE_##nRealType, mVar, nExt, 1)	\
	// **** EOF WHDATAPROP_ON_SETVALUE

#define	WHDATAPROP_ON_SETVALFUNC0(cszKey, func, nReloadMask)			\
	{ cszKey, WHDATAPROP_TYPE_setvaluefunc, 0, 0, 0, (whdataprop_setvalfunc_t)&theClass::func, nReloadMask },	\
	// **** EOF WHDATAPROP_ON_SETVALFUNC0
#define	WHDATAPROP_ON_SETVALFUNC(func)									\
	WHDATAPROP_ON_SETVALFUNC0(#func, func, 0)							\
	// **** EOF WHDATAPROP_ON_SETVALFUNC
#define	WHDATAPROP_ON_SETVALFUNC_reload1(func)							\
	WHDATAPROP_ON_SETVALFUNC0(#func, func, 1)							\
	// **** EOF WHDATAPROP_ON_SETVALFUNC

#define	WHDATAPROP_ON_GLBSETVALFUNC0(cszKey, mVar, func)				\
	{ cszKey, WHDATAPROP_TYPE_glbsetvaluefunc, wh_offsetof(theClass, mVar), wh_sizeinclass(theClass, mVar), (int)func, 0 },	\
	// **** EOF WHDATAPROP_ON_GLBSETVALFUNC0
#define	WHDATAPROP_ON_GLBSETVALFUNC(mVar, func)							\
	WHDATAPROP_ON_GLBSETVALFUNC0(#mVar, mVar, func)						\
	// **** EOF WHDATAPROP_ON_GLBSETVALFUNC

////////////////////////////////////////////////////////////////////
// ����һ������ӳ��ĺ���
////////////////////////////////////////////////////////////////////
// ��ʼ����ӳ��ĺ�����(����Ĳ�����Ҫ�Լ�д{...; ��; return xxx;})
// ע�⣺�������������ֱ��д�����壬����WHDATAPROP_FUNCCALL_DECLARE����ֱ�Ӹ��Ϳ�����
// ���������д�����壬����Ҫ��WHDATAPROP_FUNCCALL_BODY����

#define	WHDATAPROP_SETVALFUNC_DECLARE(func)								\
int		func(const char *cszVal)
#define	WHDATAPROP_SETVALFUNC_BODY(className, func)						\
int		className::func(const char *cszVal)


////////////////////////////////////////////////////////////////////
// ��ȡ��ʹ�ò����ĺ�(��Ϊ�ܳ��ã����Զ����һЩ��WDPʱWHDATAPROP����д)
////////////////////////////////////////////////////////////////////
// ʹ������ת��
#define	WDP_VAR(type, ptr)												\
	(*(type*)ptr)
// ǿ�ƽ�void *ת��Ϊָ�����͵�����
#define	WDP_REFVAR_DECLARE(var, type, ptr)								\
	type & var	= (*(type*)ptr);
// ��ʹ��ջ���Ԫ��
#define	WDP_PARAM(type, idx)											\
	(*(type*)Stack[idx])
// Ԫ�ض�Ӧ������
#define	WDP_PARAMTYPE(idx)												\
	(TStack[idx])
// ���Բ��������Ƿ�ƥ��
#define	WDP_PARAMTYPE_ASSERT(type, idx)									\
	if( WHDATAPROP_TYPE_##type == WHDATAPROP_TYPE_voidptr )				\
	{ assert(WHDATAPROP_TYPE_int == WDP_PARAMTYPE(idx)); }				\
	else	{assert(WHDATAPROP_TYPE_##type == WDP_PARAMTYPE(idx));}		\
	// EOF WDP_PARAMTYPE_ASSERT
// ��ջ���Ԫ������Ϊһ����������(ͬʱ�������)
#define	WDP_PARAM_VAR_DECLARE0(var, type, idx, chktype)					\
	type & var	= WDP_PARAM(type, idx);									\
	WDP_PARAMTYPE_ASSERT(chktype, idx);
#define	WDP_PARAM_VAR_DECLARE(var, type, idx)							\
	WDP_PARAM_VAR_DECLARE0(var, type, idx, type);
// �����Ҫ��Ϊ�˰�����ת�ɸ���
#define	WDP_PARAM_VAR_float_DECLARE(var, idx)							\
	float var	= WDP_PARAM(float, idx);								\
	if( WHDATAPROP_TYPE_int == WDP_PARAMTYPE(idx) )						\
	{																	\
		var		= WDP_PARAM(int, idx);									\
	}																	\
	else																\
	{																	\
		WDP_PARAMTYPE_ASSERT(float, idx);								\
	}																	\
	// EOF WDP_PARAM_VAR_float_DECLARE
// �Զ�˳�����
#define	WDP_PARAM_VAR_DECLARE_AUTOIDX_BEGIN()							\
	int	auto_ParamIdx = 0;
#define	WDP_PARAM_VAR_DECLARE_AUTOIDX(var, type)						\
	WDP_PARAM_VAR_DECLARE(var, type, auto_ParamIdx); auto_ParamIdx++;


// ģ���̳�
// ����һ������һ�����ܵĳ�Ա
#define	WDP_MIS_NAME(_ClassName)										\
	m_mis_##_ClassName
// ��������(��֮ǰһ��Ҫ��ֱ�ӵ�public��protected����)
#define	WDP_MIS_DECLARE(_ClassName)										\
	_ClassName	WDP_MIS_NAME(_ClassName);
// �������Ա���嵽ӳ����
#define	WHDATAPROP_ON_MIS_CLASS(_ClassName)								\
	WHDATAPROP_ON_SETVALUE0(#_ClassName, WHDATAPROP_TYPE_mis, WDP_MIS_NAME(_ClassName), 0)

}		// EOF namespace n_whcmn

#endif	// EOF __WHDATAPROP_H__

// ʹ������
/*
��μ�tst_property1.cpp
*/

