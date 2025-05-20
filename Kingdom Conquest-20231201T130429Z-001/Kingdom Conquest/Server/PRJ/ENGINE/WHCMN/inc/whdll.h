// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdll.h
// Creator      : Wei Hua (魏华)
// Comment      : 动态链接库的使用
//              : 在linux下生成动态连接库需要在连接的时候指定"-shared"参数
//              : 在windows下生成动态连接库需要生成DLL工程，并且创建DllMain函数
//              : WHDLL_Loader只能对应载入一个DLL文件，他的Release方法对应Load资源的释放
// CreationDate : 2004-11-25
// ChangeLog    :

#ifndef	__WHDLL_H__
#define	__WHDLL_H__

namespace n_whcmn
{

// 注意结构如果有构造函数就不能使用数组的方式进行初始化了
struct	WHDLL_LOAD_UNIT_T
{
	const char	*pcszFuncName;					// 函数名
	void		**ppFunc;						// 指向函数指针的指针
	void		*pDftFunc;						// 默认函数
};

class	WHDLL_Loader
{
public:
	char	m_szLastError[256];					// 这个可以用来检查究竟是什么错误
	int		m_nErrUnit;							// 可以用来检查是在载入那个func的时候出的错
public:
	static WHDLL_Loader *	Create();
	// 设置每个单元指向的函数指针为默认的函数
	static void				SetDftFunc(WHDLL_LOAD_UNIT_T *aUnit)
	{
		while( aUnit->pcszFuncName )
		{
			*aUnit->ppFunc	= aUnit->pDftFunc;
			aUnit	++;
		}
	}
	WHDLL_Loader()
	: m_nErrUnit(0)
	{
		m_szLastError[0]	= 0;
	}
	virtual ~WHDLL_Loader()	{}
public:
	// 如果不特殊说明。下面函数可能返回如下值
	enum
	{
		RST_OK							= 0,	// 成功
		RST_ERR_ALREADYLOAD				= -1,	// dll已经载入了
		RST_ERR_CANNOTOPEN				= -2,	// 无法打开dll
		RST_ERR_CANNOTCLOSE				= -3,	// 无法关闭dll
		RST_ERR_CANNOTLOADFUNC			= -4,	// 无法载入某个函数()
	};
	// aUnit数组中的最后一个一定全是NULL表示结束
	// 如果出错则pnErrUnit中记录了是那个无法载入
	virtual int		Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit)	= 0;
	virtual void *	GetFunc(const char *cszFuncName)						= 0;
	virtual int		Release()			= 0;
};

// _dftfuncptr必须是“&Func”的形式，否则在g++下会报warning
#define	WHDLL_LOAD_UNIT_DECLARE0(_funcname, _funcvar, _dftfuncptr)	\
	{_funcname, (void **)&_funcvar, (void *)_dftfuncptr},
	// EOF WHDLL_LOAD_UNIT_DECLARE0
#define	WHDLL_LOAD_UNIT_DECLARE(_funcvar, _dftfuncptr)				\
	WHDLL_LOAD_UNIT_DECLARE0(#_funcvar, _funcvar, _dftfuncptr)
	// EOF WHDLL_LOAD_UNIT_DECLARE
#define	WHDLL_LOAD_UNIT_DECLARE_LAST()								\
	{NULL, NULL, NULL},
	// EOF WHDLL_LOAD_UNIT_DECLARE_LAST

// 为了避免同一个DLL被载入多次，引入这个管理器 
class	WHDLL_Loader_Man
{
public:
	static WHDLL_Loader_Man *	Create();
	virtual ~WHDLL_Loader_Man()	{}
public:
	// Load完的DLL对象调用Release和析构就可以了
	virtual WHDLL_Loader *		Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit)
																	= 0;
};

}		// EOF namespace n_whcmn


#ifdef	WIN32
#define WH_DLL_API	__declspec(dllexport)
#else
#define	WH_DLL_API
#endif

#endif	// EOF __WHDLL_H__
