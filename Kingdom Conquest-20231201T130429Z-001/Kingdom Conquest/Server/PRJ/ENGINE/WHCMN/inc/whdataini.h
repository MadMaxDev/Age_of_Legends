// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataini.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于whdataprop的ini文件访问
//              : 配置文件的单行长度不能超过1024字节(参考MAXLINELEN的定义)
// CreationDate : 2003-11-08
// ChangeLOG    : 2005-08-03 增加了一堆using n_whcmn::xxx的定义，这样就可以让包含whdataini.h的文件随便使用whdataini::obj来定义对象了
//              : 2006-05-31 端午节哦^O^。增加了配置文件的宏定义功能。
//              : 2006-06-20 增加了配置文件的include功能。（前些日子还增加了配置文件的宏定义以及顺序四则运算（这个是通过加强whstring中的整数解析功能实现的）功能，具体日子忘啦，就在这里补上吧）。
//              : 2006-06-22 增加了单行内的注释功能。这样在一行内临时加注释或者作简短的注释就没问题了。
//              : 2007-01-18 增加了#ifdef/#ifndef/#else/#endif功能
//              : 2007-06-27 增加了m_nIgnoreBadObjLevel以便让外界可以控制忽略不认识对象的嵌套级别

#ifndef	__WHDATAINI_H__
#define	__WHDATAINI_H__

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif
#include "whdataprop.h"
#include "whvector.h"
#include "whfile.h"
#include "whfile_fcd.h"
#include "whstring.h"
#include "whhash.h"
#include <map>
#include <string>

namespace n_whcmn
{

class	whdataini
{
public:
	// 最大的行长度
	enum
	{
		MAXLINELEN						= 4096,
	};
	// 错误号
	enum
	{
		ERRNO_SUCCESS					= 0,				// 没有错误
		ERRNO_CANNOTOPENFILE			= -1,				// 不能打开文件
		ERRNO_DUPINCLUDE				= -2,				// 文件重复include
		ERRNO_BADOBJNAME				= -10,				// 错误的对象名
		ERRNO_BADLINE					= -11,				// 错误的行
	};
	int		m_nIgnoreBadObjLevel;							// 小于等于这个级别的错误对象就跳过去(外界可以直接设置这个值，默认为1)
	bool	m_bSkip;										// 是否跳过当前行
	int		m_nSkipLevel;									// 跳过的层次
	int		m_nLevel;										// 当前的层次
	int		m_errno;										// 错误号
	int		m_errline;										// 出错的行号
	whstrstack	m_ssFNames;									// 各级include的文件名栈
	whhashset<whstrptr4hash, whcmnallocationobj, _whstr_hashfunc>	m_setFNames;
															// 这个用于记录本次include链中已经被include的文件名，避免同一文件的重复include
															// 和m_ssFNames配合使用
	whmultifilechangedetector			m_mfcd;				// 用于检查是否任何访问过的文件有改变
	// 用于数据访问的对象
	class	obj	: public whdataprop_container
	{
	public:
		// 对象开始和结束时应该调用的函数
		virtual void	begin()	{}
		virtual void	end()	{}
	};
	// 对同种对象得到的列表
	template<class _Ty>										// _Ty应该是从obj继承的可以进行文件的分析的对象
	struct	objlist	: public obj
	{
		_Ty				*m_pCurObj;
		whvector<_Ty *>	m_vectObjPtr;							// 上层可以根据需要自行reserve

		objlist()
			: m_pCurObj(NULL)
		{
		}
		virtual ~objlist()
		{
			clear();
		}
		void	clear()										// clear的作用是可以重新加载数据
		{
			for(size_t i=0;i<m_vectObjPtr.size();i++)
			{
				delete	m_vectObjPtr[i];
			}
			m_vectObjPtr.clear();
		}
		virtual void	begin()
		{
			// 生成对象
			m_pCurObj	= new _Ty;
			m_vectObjPtr.push_back(m_pCurObj);
		}
		virtual void	end()
		{
			m_pCurObj	= NULL;
		}
		// 为whdataprop_container实现的
		virtual int		setvalue(const char *cszKey, const char *cszValue, unsigned int nReloadMask)
		{
			assert(m_pCurObj);
			return	m_pCurObj->setvalue(cszKey, cszValue, nReloadMask);
		}
		virtual	void *	findvalueptr(const char *cszKey, int *pnType, int *pnExt, whdataprop_setvalfunc_t *pSetValFunc, int *pnSize, const whdataprop_entry_t **ppEntry)
		{
			assert(m_pCurObj);
			return	m_pCurObj->findvalueptr(cszKey, pnType, pnExt, pSetValFunc, pnSize, ppEntry);
		}
		virtual	void *	findvalueptr(const char *cszKey, int nType, int *pnExt, whdataprop_setvalfunc_t *pSetValFunc, int *pnSize, const whdataprop_entry_t **ppEntry)
		{
			assert(m_pCurObj);
			return	m_pCurObj->findvalueptr(cszKey, nType, pnExt, pSetValFunc, pnSize, ppEntry);
		}
	};
	// 根据字符串提供对象指针的接口
	class	objgetter
	{
		// 在linux下必须加class修饰
		friend class	whdataini;
	protected:
		typedef	std::map<std::string, obj *>	MAP_T;
		// 字串到对应指针的映射
		MAP_T							m_mapobj;
	public:
		objgetter();
		~objgetter();
		// 加入名字和pObj的映射
		int		addobj(const char *cszName, obj *pObj);
		// 通过名字获得对应的对象指针
		obj *	getobj(const char *cszName);
	};
	// 行分析器，分析该行的作用
	class	lineanalyzer
	{
	public:
		enum
		{
			TYPE_BADLINE		= -1,						// 行写法有错误
			TYPE_NOTHING		= 0,						// 没有意义的行
			TYPE_COMMENT		= 1,						// 注释行
			TYPE_MACRODEFINE	= 2,						// 宏定义
			TYPE_INCLUDE		= 3,						// include文件（m_szParam[0]里面就是包含文件的路径，不过目前只能做到相对路径是相对于EXE的而不是相对于配置文件的）
			TYPE_IFDEF			= 4,						// #ifdef（m_szParam[0]是ifdef后跟的参数）
			TYPE_IFNDEF			= 5,						// #ifndef（m_szParam[0]是ifndef后跟的参数）
			TYPE_ELSE			= 6,						// #else
			TYPE_ENDIF			= 7,						// #endif
			TYPE_OBJENDBEGIN	= 10,						// 对象开始，同时意味着上个对象结束
			TYPE_OBJBEGIN		= 11,						// 对象开始
			TYPE_OBJEND			= 12,						// 对象结束
			TYPE_VAL			= 21,						// 赋值语句
			TYPE_IGNORE			= 31,						// 被忽略的语句（比如利用ifdef/ifndef/else/end造成的忽略）
		};
		char	m_szParam[2][MAXLINELEN];
		char	m_szLineTmp[MAXLINELEN];					// 临时保存某行的内容（比如上一行）
	public:
		virtual ~lineanalyzer()								{}
		// 内部归零，重新开始
		virtual void	reset()								{}
		// 分析一行，返回行的类型
		virtual int		analyze(const char *cszLine)		= 0;
	};
	// 默认的简单行分析器(ini类型的)
	class	dftlineanalyzer	: public lineanalyzer
	{
	public:
		int		analyze(const char *cszLine);
	};
	// 嵌套结构的行分析器
	class	cpplineanalyzer	: public lineanalyzer
	{
	public:
		typedef	std::map<std::string, std::string>	MAP_T;
		MAP_T	m_mapMACRO;
		enum
		{
			IF_IFDEF_NO						= 0,
			IF_IFDEF_YES					= 1,
			IF_IFDEF_IGNORE					= 2,			// 被上层的ifdef导致忽略的ifdef
		};
		whvector<int>						m_vectIF;		// IF结构栈（用于ifdef/ifndef/else/endif）
		cpplineanalyzer();
	public:
		void	reset();
		int		analyze(const char *cszLine);
	};
protected:
	// 对象名映射器指针
	objgetter		*m_getter;
	// 行分析器
	lineanalyzer	*m_analyzer;
	// 当前活动的对象栈
	whstack<obj *>	m_ObjStack;
	// 当前分析的行（analyzefile用的）
	char			m_szCurLine[MAXLINELEN];
	// 默认自带的管理器
	whfileman		*m_pFileMan_dft;
	// 实际的文件管理器
	whfileman		*m_pFileMan;
	// 是否忽略setvalue的错误
	bool			m_bIgnoreSetValueErr;
public:
	whdataini();
	~whdataini();
	int	setgetter(objgetter *getter);
	int	setlineanalyzer(lineanalyzer *analyzer);
	// 分析一行
	int	analyzeline(const char *cszLine, unsigned int nReloadMask);
	// 分析文件
	// 基于文件名打开文件的（如果cszFile为空或者空字串则不用分析）
	// bReset为true表示清空一些内部变量（一般来说，被include的文件在分析时bReset需要设置为false）
	// nReloadMask如果和某个属性与非0，则可以reload该属性
	int	analyzefile(const char *cszFile, bool bReset=true, unsigned int nReloadMask=0);
	// 直接使用文件对象的（这个可以用于通过网络传递来的配置文件）
	int	analyzefile(whfile *file, bool bReset=true, unsigned int nReloadMask=0);
	// 打印包含过程（主要用于查错）
	int	printincludestack(FILE *fpout);
	const char *	printincludestack(char *pStrOut, int &nMaxSize);
	inline const char *	printincludestack()
	{
		int		nMaxSize	= 0;
		return	printincludestack(NULL, nMaxSize);
	}
	// 打印错误字串（主要用于查错）
	int	printerrreport(FILE *fpout);
	const char *	printerrreport(char *pStrOut, int &nMaxSize);
	inline const char *	printerrreport()
	{
		int		nMaxSize	= 0;
		return	printerrreport(NULL, nMaxSize);
	}
	// 获得当前行内容，主要用于查错
	inline const char *	GetCurLine() const
	{
		return	m_szCurLine;
	}
	// 设置文件管理器（外面申请的要外面释放）
	inline void	SetFileMan(whfileman *pFileMan)
	{
		if( pFileMan )
		{
			m_pFileMan	= pFileMan;
		}
		else
		{
			m_pFileMan	= m_pFileMan_dft;
		}
	}
	// 重置对象占
	inline void	ResetObjStack()
	{
		m_ObjStack.clear();
	}
	// 设置是否忽略setvalue时出的错（一般都是找不到key）
	inline void	SetIgnoreSetValueErr(bool bSet)
	{
		m_bIgnoreSetValueErr	= bSet;
	}
};

template<class _Getter, class _Analyzer>
class	WHDATAINI	: public whdataini
{
private:
	_Getter		getter;
	_Analyzer	analyzer;
public:
	WHDATAINI()
	{
		setgetter(&getter);
		setlineanalyzer(&analyzer);
	}
	// 加入名字和pObj的映射
	inline int		addobj(const char *cszName, obj *pObj)
	{
		return	getter.addobj(cszName, pObj);
	}
};

typedef	WHDATAINI<whdataini::objgetter, whdataini::cpplineanalyzer>	WHDATAINI_CMN;

}		// EOF namespace n_whcmn

// 用在whdataini::objgetter::get中，通过函数参数cszName生成对象
#define	WHDATAINI_NEW_OBJ0(mType, className)			\
	if( strcmp(cszName, mType)==0 )						\
	{													\
		return	new className;							\
	}													\
	// EOF WHDATAINI_NEW_OBJ0
#define	WHDATAINI_NEW_OBJ(className)					\
	WHDATAINI_NEW_OBJ0(#className, className)			\
	// EOF WHDATAINI_NEW_OBJ0

////////////////////////////////////////////////////////////////////
// 这个主要是为了有些本来没有prop功能的类被继承后获得prop功能
////////////////////////////////////////////////////////////////////
#define	WHDATAINI_SORC_DECLARE0(sorc, STRUCTNAME, BASENAME)			\
sorc	STRUCTNAME	: public whdataini::obj, public BASENAME		\
{																	\
	inline BASENAME *	GetBase() const								\
	{																\
		return	(BASENAME *)this;									\
	}																\
	inline BASENAME &	GetBaseRef()								\
	{																\
	return	*GetBase();												\
	}																\
	WHDATAPROP_DECLARE_MAP(STRUCTNAME)								\
};																	\
// EOF WHDATAINI_SORC_DECLARE0
#define	WHDATAINI_STRUCT_DECLARE(STRUCTNAME, BASENAME)				\
	WHDATAINI_SORC_DECLARE0(struct, STRUCTNAME, BASENAME)			\
// EOF WHDATAINI_STRUCT_DECLARE
#define	WHDATAINI_CLASS_DECLARE(STRUCTNAME, BASENAME)				\
	WHDATAINI_SORC_DECLARE0(class, STRUCTNAME, BASENAME)			\
// EOF WHDATAINI_STRUCT_DECLARE


#define	WHDATAINI_SORC_DECLARE_BEGIN0(sorc, STRUCTNAME, BASENAME)	\
sorc	STRUCTNAME	: public whdataini::obj, public BASENAME		\
{																	\
	inline BASENAME *	GetBase() const								\
	{																\
		return	(BASENAME *)this;									\
	}																\
	// EOF WHDATAINI_SORC_DECLARE_BEGIN0
#define	WHDATAINI_SORC_DECLARE_END0(STRUCTNAME)						\
	WHDATAPROP_DECLARE_MAP(STRUCTNAME)								\
};																	\
	// EOF WHDATAINI_SORC_DECLARE_END0
#define	WHDATAINI_STRUCT_DECLARE_BEGIN(STRUCTNAME, BASENAME)		\
	WHDATAINI_SORC_DECLARE_BEGIN0(struct, STRUCTNAME, BASENAME)		\
	// EOF WHDATAINI_STRUCT_DECLARE_BEGIN
#define	WHDATAINI_STRUCT_DECLARE_END(STRUCTNAME)					\
	WHDATAINI_SORC_DECLARE_END0(STRUCTNAME)							\
	// EOF WHDATAINI_SORC_DECLARE_END

// 写了这些就可以让包含whdataini.h的文件随便使用whdataini::obj来定义对象了
using n_whcmn::whdataini;
using n_whcmn::whdataprop_entry_t;
using n_whcmn::whdataprop_map_t;

#endif	// EOF __WHDATAINI_H__
