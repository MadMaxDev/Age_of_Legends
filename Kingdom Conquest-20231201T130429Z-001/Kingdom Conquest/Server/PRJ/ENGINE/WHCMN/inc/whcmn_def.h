// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whcmn_def.h
// Creator      : Wei Hua (魏华)
// Comment      : whcmn公用的定义
// CreationDate : 2003-05-15
// CHANGELOG    :
//              : 2005-09-15 whmisc.h的内容全部转移到whcmn_def.h中。
//              : 2006-01-23 WHSafeSelfDestroy参数改为引用，原来一直写错了。
//              : 2006-02-05 WHRANGE_T对整数类型重新实现了一下。
//              : 2006-02-06 WHRANGE_T还是采用了简单的继承的方法，因为VC6不支持模板类偏特化，gcc不支持在结构里面对模板进行特化(是什么特化都不支持哦)。
//              : 2006-02-23 增加whsetbool_true/false函数
//              : 2006-03-09 增加了whbufferwrite/read函数，用于简单的缓冲偏移读取
//              : 2007-11-15 增加了whsafeptr的detach方法

#ifndef __WHCMN_DEF_H__
#define	__WHCMN_DEF_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>						// 没有这个RAND就没定义了
#include <string.h>

#include <malloc.h>

// 定义了这个所有和ID申请相关的ID基数都会循环增长
// 可以避免新旧ID替换时的错误
// 这个只有在正式使用时采用，平时不用为了发现ID不能及时释放的错误
// #define	WHCMN_USECYCLE_ID

namespace n_whcmn
{

#ifdef	__GNUC__
	struct	S_GODDAMNLINUXWARNING
	{
		static void *whoffsetdummpptr;	// 这个只是为了不让linux报warning
	};
	// 获得数据成员偏移
	#define wh_offsetof(s,m)			(size_t)&(((s *)n_whcmn::S_GODDAMNLINUXWARNING::whoffsetdummpptr)->m)
	// 获得类成员的尺寸
	#define	wh_sizeinclass(s,m)			sizeof(((s *)n_whcmn::S_GODDAMNLINUXWARNING::whoffsetdummpptr)->m)
#else
	// 获得数据成员偏移
	#define wh_offsetof(s,m)			(size_t)&(((s *)0)->m)
	// 获得类成员的尺寸
	#define	wh_sizeinclass(s,m)			sizeof(((s *)0)->m)
#endif

#define	WHSIZEOFARRAY(pArr, num)		(sizeof(*pArr)*num)
#define	WHNUMOFARRAYUNIT(arr)			(sizeof(arr)/sizeof(arr[0]))

#define	WHMEMSET0THIS()					memset(this, 0, sizeof(*this));
template<typename _Ty>
inline void	WHMEMSET0(_Ty &v)
{
	memset(&v, 0, sizeof(v));
}
template<typename _Ty>
inline void	WHMEMSET0PTR(_Ty *p)
{
	memset(p, 0, sizeof(*p));
}

// 根据偏移获得地址指针
// （这里用const指针是为了const型的数据不用强制转换也可以被使用）
inline void *	wh_getoffsetaddr(const void *ptr, int off)
{
	return	(((char *)ptr)+off);
}
// 获得指针在数组中的序号
template<typename _Ty>
inline int		wh_getidxinarray(_Ty *first, _Ty *dummy)
{
	return	(dummy-first);
}
// 对特定类型指针的调整
template<typename _Ty>
inline void		wh_adjustaddrbyoffset(_Ty *&ptr, int off)
{
	if( !ptr )	return;	// 如果是空指针就不校正了
	ptr	= (_Ty *)wh_getoffsetaddr(ptr, off);
}
// 一个数组所有内部对象的调整
template<typename _Ty>
inline void		wh_adjustarraybyoffset(_Ty *ptr, int num, int off)
{
	for(int i=0;i<num;i++)
	{
		ptr->AdjustInnerPtr(off);
		++ptr;
	}
}

// 对比指针获得偏移
inline int		wh_calcoffset(void *first, void *next)
{
	return	(int)(((char*)next) - ((char*)first));
}

// 根据类型赋值指针
template<typename _Ty>
inline void		wh_settypedptr(_Ty *&pDst, const void *pSrc)
{
	pDst	= (_Ty *)pSrc;
}

// 获得这个变量紧后面的地址
template<typename _Ty>
inline	void *	wh_getptrnexttovar(_Ty &var)
{
	return	(&var) + 1;
}
// 根据指针类型获得紧后面的地址
template<typename _Ty>
inline	void *	wh_getptrnexttoptr(_Ty *ptr)
{
	return	(void*)(ptr + 1);
}

// 写入并修改缓冲指针变量和尺寸
inline char *	whbufferwrite(char *pBuf, int &nSizeLeft, const void *pData, int nDataSize)
{
	assert(nSizeLeft>=nDataSize);
	memcpy(pBuf, pData, nDataSize);
	nSizeLeft	-= nDataSize;
	return	pBuf + nDataSize;
}
template<typename _Ty>
inline char *	whbufferwrite(char *pBuf, int &nSizeLeft, _Ty *pData)
{
	return	whbufferwrite(pBuf, nSizeLeft, pData, sizeof(*pData));
}
// 读出并修改缓冲指针变量和尺寸
inline char *	whbufferread(char *pBuf, int &nSizeLeft, void *pData, int nDataSize)
{
	assert(nSizeLeft>=nDataSize);
	memcpy(pData, pBuf, nDataSize);
	nSizeLeft	-= nDataSize;
	return	pBuf + nDataSize;
}
template<typename _Ty>
inline char *	whbufferread(char *pBuf, int &nSizeLeft, _Ty *pData)
{
	return	whbufferread(pBuf, nSizeLeft, pData, sizeof(*pData));
}

// 定义这个是为了有时候需要定义一个什么也不做的构造函数(比如只是为了刷新虚表)，但是为了区别起特殊性，需要有一个参数
enum	NULLCONSTRUCT_T
{
	NULLCONSTRUECTOR   = 0,
};
#define	NEW_NULLCONSTRUECTOR(pMem,type)	new (pMem) type(NULLCONSTRUECTOR)

// 范围内随机(注意：随机值的个数只能有32767个)
template <typename _Ty>
struct	WHRANGE_T_BASE
{
private:
	_Ty		vFrom;
	_Ty		vDiff;
	bool	bSame;
public:
	inline const _Ty 	GetFrom() const
	{
		return	vFrom;
	}
	inline const _Ty 	GetTo() const
	{
		return	vFrom+vDiff;
	}
public:
	WHRANGE_T_BASE()
	: vFrom(0), vDiff(0), bSame(true)
	{
	}
	WHRANGE_T_BASE(_Ty _From, _Ty _To)
	{
		Set(_From, _To);
	}
	void	Set(_Ty _From, _Ty _To)
	{
		// 保证_From<=_To
		bSame	= false;
		if( _From>_To )
		{
			vFrom	= _To;
			vDiff	= _From - _To;
		}
		else if( _From<_To )
		{
			vFrom	= _From;
			vDiff	= _To - _From;
		}
		else
		{
			vFrom	= _From;
			bSame	= true;
		}
	}
	void	Set(const WHRANGE_T_BASE &other)
	{
		vFrom	= other.vFrom;
		vDiff	= other.vDiff;
		bSame	= other.bSame;
	}
	inline _Ty	GetRand() const
	{
		if( bSame )
		{
			return	vFrom;
		}
		return	vFrom+rand()%vDiff;
	};
	inline _Ty	GetRand_floattype() const
	{
		if( bSame )
		{
			return	vFrom;
		}
		return	vFrom+(vDiff*float(rand())/float(RAND_MAX));
	};
};
template <typename _Ty>
struct	WHRANGE_T	: public WHRANGE_T_BASE<_Ty>
{
	WHRANGE_T()	: WHRANGE_T_BASE<_Ty>()
	{
	}
	WHRANGE_T(_Ty _From, _Ty _To)	: WHRANGE_T_BASE<_Ty>(_From, _To)
	{
	}
};
#define	WHRANGE_T_F(ftype)									\
template <>													\
struct	WHRANGE_T<ftype>	: public WHRANGE_T_BASE<ftype>	\
{															\
	WHRANGE_T()	: WHRANGE_T_BASE<ftype>()	{}				\
	WHRANGE_T(ftype _From, ftype _To)	: WHRANGE_T_BASE<ftype>(_From, _To)	{}	\
	inline ftype	GetRand() const		{return	GetRand_floattype();}			\
};															\
// EOF WHRANGE_T_F
WHRANGE_T_F(float)
WHRANGE_T_F(double)

////////////////////////////////////////////////////////////////////
// 对指针的安全删除
// 不支持所有权转移!!!!
////////////////////////////////////////////////////////////////////
struct	WHDelMethod_Cmn
{
	WHDelMethod_Cmn() {}
	template<typename _Ty>
	inline void	operator()(_Ty *ptr)	const	{delete ptr;}
};
template<typename _Ty, class _DelMethod=WHDelMethod_Cmn>
struct	whsafeptr
{
private:
	_DelMethod	m_dm;
protected:
	_Ty	*ptr;
public:
	whsafeptr()
	: ptr(NULL)
	{
	}
	whsafeptr(_Ty *__ptr)
	: ptr(__ptr)
	{
	}
	~whsafeptr()
	{
		selfdelete();
	}
	// 注意selfdelete不能通过虚函数的方式写，因为析构的时候虚表已经被破坏了
	void	selfdelete()
	{
		if( ptr )
		{
			m_dm(ptr);
			ptr		= NULL;
		}
	}
	inline void	detach()	// 这样在特定情况下可以不删除指针（比如：safeptr只是为了出现异常情况才删除指针的，正常情况下需要new出来的指针在其他地方继续使用）
	{
		ptr	= NULL;
	}
	inline void	setptr(_Ty *__ptr)
	{
		ptr	= __ptr;
	}
	inline bool operator ! () const
	{
		return	ptr==NULL;
	}
	inline bool	operator != (const _Ty * __other) const
	{
		return	ptr != __other;
	}
	inline bool	operator == (const _Ty * __other) const
	{
		return	ptr == __other;
	}
	inline _Ty&	operator*() const
	{
		return	*get();
	}
	inline _Ty*	operator->() const
	{
		return	get();
	}
	inline _Ty*	get() const
	{
		return	ptr;
	}
	inline operator _Ty* () const
	{
		return	ptr;
	}
};

struct	whsafeptr_FILE	: public whsafeptr<FILE>
{
	explicit whsafeptr_FILE(FILE *__ptr)
	: whsafeptr<FILE>(__ptr)
	{
	}
	~whsafeptr_FILE()
	{
		selfdelete();
	}
	void	selfdelete()
	{
		if( ptr )
		{
			fclose(ptr);
			ptr	= NULL;
		}
	}
};

struct	whsafeptr_FILE_excludeSTDs	: public whsafeptr<FILE>
{
	explicit whsafeptr_FILE_excludeSTDs(FILE *__ptr)
	: whsafeptr<FILE>(__ptr)
	{
	}
	~whsafeptr_FILE_excludeSTDs()
	{
		selfdelete();
	}
	void	selfdelete()
	{
		if( ptr )
		{
			if( ptr!=stdin && ptr!=stdout && ptr!=stderr )
			{
				fclose(ptr);
			}
			ptr	= NULL;
		}
	}
};

// 安全的delete
template<typename _Ty>
void	WHSafeDelete(_Ty *&pObj)
{
	if( pObj!=NULL )
	{
		delete	pObj;
		pObj	= NULL;
	}
}

// 安全的selfdestroy
template<typename _Ty>
void	WHSafeSelfDestroy(_Ty *&pObj)
{
	if( pObj!=NULL )
	{
		pObj->SelfDestroy();
		pObj	= NULL;
	}
}
// Safe SelfDestroy
struct	WHDelMethod_SD
{
	WHDelMethod_SD() {}
	template<typename _Ty>
	inline void	operator()(_Ty *ptr)	const	{ptr->SelfDestroy();}
};
// SD mean Sefl Destroy
template<typename _Ty>
struct	whsafeSDptr	: public whsafeptr<_Ty, WHDelMethod_SD>
{
	typedef	whsafeptr<_Ty, WHDelMethod_SD>	FATHERCLASS;
	whsafeSDptr()
	{
	}
	whsafeSDptr(_Ty *__ptr)
		: FATHERCLASS(__ptr)
	{
	}
};

// 获得取整的部分数量
inline int	wh_getintpartnum(int nTotal, int nParSize, int *pnResidue)
{
	return	nTotal/nParSize + ((*pnResidue=nTotal%nParSize)!=0 ? 1:0);
}

inline void	whsetbool_true(bool &bVar)
{
	if( !bVar )
	{
		bVar	= true;
	}
}
inline void	whsetbool_false(bool &bVar)
{
	if( bVar )
	{
		bVar	= false;
	}
}

// 获取环境变量
const char *	wh_getenv(const char *cszKey);
// 设置环境变量
void			wh_setenv(const char *cszKey, const char *cszVal);

// 各种基础数据类型
typedef	unsigned char			whbyte;
#ifdef WIN32
#define	WHINT64PRFX		"I64"
typedef __int64					whint64;
typedef unsigned __int64		whuint64;
#endif
#ifdef	__GNUC__
#define	WHINT64PRFX		"ll"
typedef long long				whint64;
typedef unsigned long long		whuint64;
#endif

// 有时需要把字串作为类型来定义
template<int _SIZE>
struct	wh_fixstr_t
{
	char	s[_SIZE];
	wh_fixstr_t()
	{
		s[0]		= 0;
		s[_SIZE-1]	= 0;
	}
	wh_fixstr_t(const char *cszStr)
	{
		operator = (cszStr);
	}
	inline void	clear()
	{
		s[0]	= 0;
	}
	inline void operator = (const char *cszStr)
	{
		strncpy(s, cszStr, sizeof(s)-1);
	}
	inline void operator = (const wh_fixstr_t &other)
	{
		operator = (other.s);
	}
	inline char & operator [] (int nIdx)
	{
		return	s[nIdx];
	}
	inline operator char * ()
	{
		return	s;
	}
	inline operator const char * () const
	{
		return	s;
	}
};

// unsigned int/short/char的字节转换
union	whlsc_t
{
	unsigned int	l;
	unsigned short	s[2];
	unsigned char	c[4];
	whlsc_t()
		: l(0)
	{
	}
	whlsc_t(unsigned int _l)
		: l(_l)
	{
	}
};
// whuint64/unsigned int/short/char的字节转换
union	whllsc_t
{
	whuint64		I;
	unsigned int	l[2];
	unsigned short	s[4];
	unsigned char	c[8];
	whllsc_t()
		: I(0)
	{
	}
	whllsc_t(whuint64 _I)
		: I(_I)
	{
	}
};

}		// EOF namespace n_whcmn

// 这个主要是为了写临时调试打印语句，将来想去掉的时候好找
#define	WHTMPPRINTF			printf

// 这个主要是用来给一个缓冲区连续打印字串用的
#define	WH_SMPINBUFF_PRINT	pszBuf+=rst;nSizeLeft-=rst;assert(nSizeLeft>=0);rst=snprintf

// 这个主要用于书写结构的小于操作符
#define	LESS_THAN_OP(thisvar, othervar)			\
	if( thisvar < othervar )					\
	{											\
		return	true;							\
	}											\
	else if( thisvar > othervar )				\
	{											\
		return	false;							\
	}											\
	// EOF LESSTHAN_OP
// 默认对方的主变量是other
#define	LESS_THAN_OP_SMP(var)	LESS_THAN_OP(var, other.var)

// 一般对象的lessthan可能这么写
// bool	xxx::lessthan (const _Ty &other) const
// {
// 	LESS_THAN_OP_SMP(var1)
// 	LESS_THAN_OP_SMP(var2)
// 	LESS_THAN_OP_SMP(var3)
// 	return	false;
// }

// 左移
template<typename T>
inline T wh_l_shift(const T & a, int n)
{
	return	a << n;
}
// 右移
template<typename T>
inline T wh_r_shift(const T & a, int n)
{
	return	a >> n;
}

////////////////////////////////////////////////////////////////////
// 为DLL之间信息同步
////////////////////////////////////////////////////////////////////
#include "whdll.h"
void *	WHCMN_STATIC_INFO_Out();
extern "C" WH_DLL_API void	WHCMN_STATIC_INFO_In(void *pInfo);
#ifdef	_DEBUG
unsigned char	WHCMN_GET_DBGLVL();
void			WHCMN_SET_DBGLVL(unsigned char nLvl);
#endif
enum
{
	WHCMN_NOTIFY_NOTHING	= 0,				// 正常什么也没有的情况
	WHCMN_NOTIFY_MEMERR		= 1,				// 内存出现问题
	WHCMN_NOTIFY_SHMERR		= 2,				// 共享内存出现问题
};
int				WHCMN_GET_NOTIFY();
void			WHCMN_SET_NOTIFY(int nNotify);

// 可能用到的LOGID
enum
{
	// 一般可以把日志写到这个ID 
	WHCMN_LOG_ID_CMN				=	0,
	// 重要错误写到这里
	WHCMN_LOG_ID_ERROR				=	1,
	// 基本过程相关的写到这里(比如游戏中和用户操作相关的就放到这里，对应与原来刀剑中的client日志。不过现在有一些可以分到player和npc日志中去写)
	WHCMN_LOG_ID_STORY				=	2,
	// 致命错误
	WHCMN_LOG_ID_FATAL				=	4,
};
typedef	int (*LOG_WRITEFMT_FUNC_T)(int nID, const char *szFmt, ...);
void	WHCMN_Set_Log_WriteFmt( LOG_WRITEFMT_FUNC_T pFunc );
LOG_WRITEFMT_FUNC_T	WHCMN_Get_Log_WriteFmt();
#define	WHCMN_LOG_WRITEFMT			WHCMN_Get_Log_WriteFmt()
#define WHCMN_LOG_STD_HDR(code,keyword)		#code","#keyword","

// 获得本地字符集（默认是没有填写的）
const char *	WHCMN_getcharset();
// 设置本地字符集
void	WHCMN_setcharset(const char *cszName);

#endif	// EOF __WHCMN_DEF_H__
