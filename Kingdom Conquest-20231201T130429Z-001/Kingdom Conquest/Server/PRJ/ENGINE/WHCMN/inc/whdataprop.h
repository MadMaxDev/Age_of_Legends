// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataprop.h
// Creator      : Wei Hua (魏华)
// Comment      : 数据成员的字串访问接口。我参考了MFC的消息映射的实现。
//                使用WHDATAPROP_MAP_BEGIN*时，如果目标结构是类里的结构，则要用全名，即：类::结构。参考：GMS_I::CFGINFO_T
// CreationDate : 2003-11-08
// ChangeLOG    : 2006-11-27 把setvaluefunc变成有int返回值的
//              : 2007-02-06 取消了原来没用的多需函数方法相关的功能

#ifndef	__WHDATAPROP_H__
#define	__WHDATAPROP_H__

#include <stddef.h>
#include <assert.h>
#include "whcmn_def.h"
#include "whvector.h"

namespace n_whcmn
{

class	whdataprop_container;
// 下面调用类型的参数
// 函数调用类型(fc是FunctionCall的缩写)
// 用来设置值的函数(如果返回<0则说明出了非常严重的错误)
typedef	int		(whdataprop_container::*whdataprop_setvalfunc_t)(const char *cszVal);
typedef	int		(*whdataprop_glbsetvaluefunc_t)(void *var, const char *cszVal);
// 基本数据类型
enum
{
	// 需要使用函数
	WHDATAPROP_TYPE_FUNC					= 0,
	// 基本类型(这些有小写的是为了到时候和脚本结合时做类型匹配)
	WHDATAPROP_TYPE_bool					= 0x0001,
	WHDATAPROP_TYPE_whbyte					= 0x0002,
	WHDATAPROP_TYPE_short					= 0x0003,
	WHDATAPROP_TYPE_int						= 0x0004,
	WHDATAPROP_TYPE_float					= 0x0005,
	WHDATAPROP_TYPE_voidptr					= 0x0006,
	WHDATAPROP_TYPE_charptr					= 0x0007,		// 注意，char*不允许有普通的数组类型，即char szStr[10][20];不允许的
															// 除非是char *szStr[10];的形式
															// charptr的单串有长度限制功能，即如果设置的字数超过了变量本身的长度则后面会被截断(哈哈哈我可真聪明)
															// charptr数组没有检查长度的功能
	WHDATAPROP_TYPE_time_t					= 0x0008,		// 时间(秒)
	WHDATAPROP_TYPE_whint64					= 0x0009,		// 64位整数
	WHDATAPROP_TYPE_string					= 0x0010,		// std::string
	WHDATAPROP_TYPE_setvaluefunc			= 0x0011,		// 函数指针为whdataprop_setvalfunc_t型，存在pSetValFunc中，该函数可以用来通过字串设置值
	WHDATAPROP_TYPE_glbsetvaluefunc			= 0x0012,		// 非成员的函数指针，为whdataprop_glbsetvaluefunc_t型，存在nExt中，该函数可以用来通过字串设置值
	WHDATAPROP_TYPE_rgint					= 0x0021,		// 整数范围(常用的就这两种啦)
	WHDATAPROP_TYPE_rgfloat					= 0x0022,		// 浮点范围
	WHDATAPROP_TYPE_unknowntype				= 0x0080,		// 未知类型，一般用于内含结构，详见whdataini的实现
	WHDATAPROP_TYPE_mis						= 0x0081,		// 用于模拟多继承的类成员(mis代表multi-inheritation simulation)
															// 对应的成员对象一定也是从container派生出来的
	// 基本类型的掩码
	WHDATAPROP_TYPE_BASICMASK				= 0x00FF,
	// 如果这位被置表示是相应类型的数组，数组元素间用逗号分隔
	// 如：
	// 0x1002就是整型数组
	// 0x1001就是字串的数组（已经分配好的，足够大的）
	WHDATAPROP_TYPE_ARRAYBIT				= 0x0100,
};

// 根据类型设置值
int	whdataprop_setvalue(void *pValue, int nSize, int nType, int nExt, const char *szVal);
// 获得值字串
const char *	whdataprop_getvalue(const void *pValue, int nSize, int nType, int nExt, char *szVal);

struct	whdataprop_entry_t
{
	const char	*cszKey;
	int			nType;										// WHDATAPROP_TYPE_XXX
	int			nValueOffset;								// 如果是普通类型，这个指向变量相对于类头的偏移
	int			nValueSize;									// 该变量的长度(主要是保证字串不超界)
	int			nExt;										// 如果是array，最大元素数目为nExt。元素间以空格为分隔符。
															// 也可以是全局的值解析函数
	whdataprop_setvalfunc_t	pSetValFunc;					// 用来设置值的函数
	unsigned int	nReloadMask;							// 如果外界给的mask非0且和这个有重叠，则这个被设置
};
struct	whdataprop_map_t
{
	const char					* cszClassName;				// 本类的类名
	const whdataprop_map_t		* lpBaseMap;				// 指向父类的whdataprop_map_t结构
	const whdataprop_entry_t	* lpEntries;				// 本类的数据入口数组
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
	// 通过字串查找变量的真实地址
	virtual	void *	findvalueptr(const char *cszKey, int *pnType=NULL, int *pnExt=NULL, whdataprop_setvalfunc_t *pSetValFunc=NULL, int *pnSize=NULL, const whdataprop_entry_t **ppEntry=NULL);
	// 找到名字和类型都匹配的变量真实地址
	virtual	void *	findvalueptr(const char *cszKey, int nType, int *pnExt=NULL, whdataprop_setvalfunc_t *pSetValFunc=NULL, int *pnSize=NULL, const whdataprop_entry_t **ppEntry=NULL);
	// 判断是否是类的格式，即：XX::xx的格式
	virtual	bool	isclasstype(const char *cszKey, char *szClassName, char *szPropName);
	// 找到基类的映射
	virtual	const whdataprop_map_t *	findmap(const char *cszClassName);
	// 通过map找到entry
	virtual	const whdataprop_entry_t *	findentry(const whdataprop_map_t *pMap, const char *cszKeyName);
	// 找到仿类的指针
	virtual	whdataprop_container *	findcontainer(const char *cszClassName);
	// 找到MIS的指针
	virtual	void *	findinterface(const char *cszClassName);

	// 需要有映射的变量定义(变量和特殊变量的设置函数，子类中填写的)
	//{{
	//}}
	// 需要在子类需要WHDATAPROP_DECLARE_MAP();
	virtual const whdataprop_map_t * GetPropMap() const
	{
		// 如果子类不实现就返回空指针
		return	0;
	}
};

////////////////////////////////////////////////////////////
// 声明数据或函数映射必须的各个宏
////////////////////////////////////////////////////////////
// 在需要有数据成员映射的类中一定要写这个
// 在类定义中需要有这句话
#define	WHDATAPROP_DECLARE_MAP(_theClass)					\
private:													\
	typedef	_theClass	theClass;							\
	static const whdataprop_entry_t	_propEntries[];			\
protected:													\
	static const whdataprop_map_t			propMap;		\
public:														\
	virtual const whdataprop_map_t * GetPropMap() const;	\
	// **** EOF WHDATAPROP_DECLARE_MAP

// 普通类的映射定义
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

// 最顶类的映射定义(即需要搜索成员的最根类)
#define	WHDATAPROP_MAP_BEGIN_AT_ROOT(_theClass)				\
	WHDATAPROP_MAP_BEGIN0(_theClass, NULL)					\
	// **** EOF WHDATAPROP_MAP_BEGIN_AT_ROOT

// 映射定义结束
#define WHDATAPROP_MAP_END()								\
		{ 0, 0, 0, 0, 0, 0, 0 }								\
	};														\
	// **** EOF WHDATAPROP_MAP_END

////////////////////////////////////////////////////////////////////
// 数据入口定义
////////////////////////////////////////////////////////////////////
// 基本类型数据映射
#define	WHDATAPROP_ON_SETVALUE0(cszKey, nType, mVar, nExt, nReloadMask)	\
	{ cszKey, nType, wh_offsetof(theClass, mVar), wh_sizeinclass(theClass, mVar), nExt, 0, nReloadMask },	\
	// **** EOF WHDATAPROP_ON_SETVALUE0
// 这个直接用变量名做key名
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
// 声明一个用于映射的函数
////////////////////////////////////////////////////////////////////
// 开始用于映射的函数体(后面的部分需要自己写{...; 等; return xxx;})
// 注意：如果在类生命中直接写函数体，则在WHDATAPROP_FUNCCALL_DECLARE后面直接根就可以了
// 如果在外面写函数体，则需要在WHDATAPROP_FUNCCALL_BODY后面

#define	WHDATAPROP_SETVALFUNC_DECLARE(func)								\
int		func(const char *cszVal)
#define	WHDATAPROP_SETVALFUNC_BODY(className, func)						\
int		className::func(const char *cszVal)


////////////////////////////////////////////////////////////////////
// 获取和使用参数的宏(因为很常用，所以定义短一些，WDP时WHDATAPROP的缩写)
////////////////////////////////////////////////////////////////////
// 使用类型转换
#define	WDP_VAR(type, ptr)												\
	(*(type*)ptr)
// 强制将void *转换为指定类型的引用
#define	WDP_REFVAR_DECLARE(var, type, ptr)								\
	type & var	= (*(type*)ptr);
// 简单使用栈里的元素
#define	WDP_PARAM(type, idx)											\
	(*(type*)Stack[idx])
// 元素对应的类型
#define	WDP_PARAMTYPE(idx)												\
	(TStack[idx])
// 断言参数类型是否匹配
#define	WDP_PARAMTYPE_ASSERT(type, idx)									\
	if( WHDATAPROP_TYPE_##type == WHDATAPROP_TYPE_voidptr )				\
	{ assert(WHDATAPROP_TYPE_int == WDP_PARAMTYPE(idx)); }				\
	else	{assert(WHDATAPROP_TYPE_##type == WDP_PARAMTYPE(idx));}		\
	// EOF WDP_PARAMTYPE_ASSERT
// 将栈里的元素声明为一个变量引用(同时检测类型)
#define	WDP_PARAM_VAR_DECLARE0(var, type, idx, chktype)					\
	type & var	= WDP_PARAM(type, idx);									\
	WDP_PARAMTYPE_ASSERT(chktype, idx);
#define	WDP_PARAM_VAR_DECLARE(var, type, idx)							\
	WDP_PARAM_VAR_DECLARE0(var, type, idx, type);
// 这个主要是为了把整数转成浮点
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
// 自动顺序序号
#define	WDP_PARAM_VAR_DECLARE_AUTOIDX_BEGIN()							\
	int	auto_ParamIdx = 0;
#define	WDP_PARAM_VAR_DECLARE_AUTOIDX(var, type)						\
	WDP_PARAM_VAR_DECLARE(var, type, auto_ParamIdx); auto_ParamIdx++;


// 模拟多继承
// 定义一个具有一定功能的成员
#define	WDP_MIS_NAME(_ClassName)										\
	m_mis_##_ClassName
// 仿类声明(这之前一定要有直接的public或protected声明)
#define	WDP_MIS_DECLARE(_ClassName)										\
	_ClassName	WDP_MIS_NAME(_ClassName);
// 将该类成员定义到映射中
#define	WHDATAPROP_ON_MIS_CLASS(_ClassName)								\
	WHDATAPROP_ON_SETVALUE0(#_ClassName, WHDATAPROP_TYPE_mis, WDP_MIS_NAME(_ClassName), 0)

}		// EOF namespace n_whcmn

#endif	// EOF __WHDATAPROP_H__

// 使用例子
/*
请参见tst_property1.cpp
*/

