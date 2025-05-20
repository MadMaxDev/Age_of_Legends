// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whcmd.h
// Creator      : Wei Hua (魏华)
// Comment      : 通用命令调用的接口。我参考了MFC的消息映射的实现。
// CreationDate : 2003-12-18
// 注意：
// 引用类成员函数地址时，需要写成“&classname::funcname”的形式，因为linux下需要这样
// ChangeLOG    : 2005-10-11 增加了whcmdshrink::GetDataNotRead
//              : 2006-02-10 增加了whcmn_strcmd_reganddeal的Help功能
//              : 2006-05-08 把whcmdshrink的一些比较长的函数取消了inline
//              : 2007-01-29 修改了ReadStr和ReadBigStr的成功返回值，原来是0，现在改为读到的字串的长度（不包含最后一个0结尾，虽然会自动补上0结尾）
//              : 2007-03-01 加入了whcmdshrink::Reset()方法
//              : 2008-06-18 加入了whcmdshrink::SetShrinkResize()方法

#ifndef	__WHCMD_H__
#define	__WHCMD_H__

#include "whcmn_def.h"
#include "whstring.h"
#include "whqueue.h"
#include "whthread.h"
#include "whhash.h"
#include "whbits.h"
#include <map>
#ifdef	WIN32
#include <winsock2.h>
#endif
#ifdef	__GNUC__
#include <netinet/in.h>
#endif

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 命令紧缩
////////////////////////////////////////////////////////////////////
// 注意：这里面的Read和Write在发现尺寸错处时会抛出字串异常，所以外界需要用try/catch(const char *cszErr) !!!!
// Write时Pos会自动增长。可以通过Pos获得实际写入的总长度，或者调用WriteEnd以保证shrinksize就是写入长度。
class	whcmdshrink
{
protected:
	// 指向紧缩缓冲的指针
	char	*m_pShrink;
	// 紧缩的大小(或Write可以写到的最大大小)
	int		m_nShrinkSize;
	// 原始的m_nShrinkSize（用于Reset）
	int		m_nShrinkSizeOld;
	// 当前Shink到的位置(R或W)
	int		m_nPos;
	// 是否需要对int和short转换网络字节序(NetByteOrder)
	bool	m_bNBO;
public:
	// 设置承载数据的缓冲（这个函数可以用作Reset）
	inline	void	SetShrink(char *pShrink, int nShrinkSize)
	{
		m_pShrink		= pShrink;
		m_nShrinkSize	= nShrinkSize;
		m_nShrinkSizeOld= nShrinkSize;
		m_nPos			= 0;
	}
	inline	void	SetShrinkResize(char *pShrink, int nShrinkSize)		// 这个一般是写入时发生不够，所以外部的缓冲resize一下（拷贝内容外部负责），这里面也重新设置一下引用指针
	{
		m_pShrink		= pShrink;
		m_nShrinkSize	= nShrinkSize;
		m_nShrinkSizeOld= nShrinkSize;
	}
	inline void		Reset()
	{
		m_nShrinkSize	= m_nShrinkSizeOld;
		m_nPos			= 0;
	}
	inline	void	SetNBO(bool bSet)
	{
		m_bNBO			= bSet;
	}
	// 获得缓冲指针
	inline	char *	GetShrinkBuf() const
	{
		return	m_pShrink;
	}
	// 获得紧缩后的长度
	inline	int		GetShrinkSize() const
	{
		return	m_nShrinkSize;
	}
	// 获得紧缩/或读取过程中还剩下的字节数
	inline	int		GetSizeLeft() const
	{
		return	m_nShrinkSize - m_nPos;
	}
	inline	char *	GetCurPtr() const
	{
		return	m_pShrink + m_nPos;
	}
	inline int		GetCurPos() const
	{
		return	m_nPos;
	}
public:
	whcmdshrink()
	: m_pShrink(0), m_nShrinkSize(0), m_nShrinkSizeOld(0), m_nPos(0)
	, m_bNBO(false)
	{}
	// 注：后边的Read和Write虽然有返回，但是出错都是以throw的形式报出错误，所以不会有错误的返回。
	int	Read(void *pData, int nSize)
	{
		int	nPos = m_nPos + nSize;
		if( nPos>m_nShrinkSize )
		{
			// 无法读出
			throw	"Read size err!";			// 外部需要catch这个
			return	-1;
		}
		memcpy(pData, m_pShrink+m_nPos, nSize);
		m_nPos	= nPos;
		return	nSize;
	}
	// 读到结尾，返回读到的长度
	int	ReadTillEnd(void *pData)
	{
		int	nSize	= m_nShrinkSize - m_nPos;
		if( nSize<=0 )
		{
			return	0;
		}
		memcpy(pData, m_pShrink+m_nPos, nSize);
		m_nPos	= m_nShrinkSize;
		return	nSize;
	}
	// 把指针重新放到开头
	inline void	Rewind()
	{
		m_nPos	= 0;
	}
	// 向后跳过一段数据
	int	Seek(int nSize)
	{
		m_nPos	+= nSize;
		if( m_nPos>m_nShrinkSize )
		{
			m_nPos	= m_nShrinkSize;
			throw	"Seek size err!";			// 外部需要catch这个
			return	-1;
		}
		return	0;
	}
	// 获得剩余还没有读的数据
	// 需要通过长度判断是否还有数据
	void *	GetDataNotRead(int *pnSize)
	{
		*pnSize	= m_nShrinkSize - m_nPos;
		return	m_pShrink+m_nPos;
	}
	template<typename _Ty>
	inline int	Read(_Ty *pData)
	{
		return	Read(pData, sizeof(*pData));
	}
	#define	WHCMDSHRINK_READ_NBO_DECLARE(ty, fn)	\
	inline int	Read(ty *pData)						\
	{												\
		int		rst;								\
		rst		= Read(pData, sizeof(*pData));		\
		if( m_bNBO )								\
		{											\
			*pData	= fn(*pData);					\
		}											\
		return	rst;								\
	}												\
	// EOF WHCMDSHRINK_READ_NBO_DECLARE
	WHCMDSHRINK_READ_NBO_DECLARE(int, ntohl);
	WHCMDSHRINK_READ_NBO_DECLARE(long, ntohl);
	WHCMDSHRINK_READ_NBO_DECLARE(short, ntohs);
	WHCMDSHRINK_READ_NBO_DECLARE(unsigned int, ntohl);
	WHCMDSHRINK_READ_NBO_DECLARE(unsigned long, ntohl);
	WHCMDSHRINK_READ_NBO_DECLARE(unsigned short, ntohs);
	// nMaxSize是包含最后'\0'结尾的最大长度
	// 返回读到的不包含0结尾的字串长度
	int	ReadStr(char *pszData, int nMaxSize)
	{
		char	*pShrink = m_pShrink+m_nPos;
		int		nLen = *(unsigned char*)pShrink;
		int		nPos = m_nPos + nLen+1;
		if( nPos>m_nShrinkSize || nLen>=nMaxSize )
		{
			// 无法读出
			throw	"ReadStr size err!";			// 外部需要catch这个
			return	-1;
		}
		memcpy(pszData, pShrink+1, nLen);
		pszData[nLen]	= 0;						// 添加最后一个0
		m_nPos	= nPos;
		return	nLen;
	}
	int	ReadBigStr(char *pszData, int nMaxSize)
	{
		char	*pShrink	= m_pShrink+m_nPos;
		int		nLen = *(unsigned short*)pShrink;
		int		nPos = m_nPos + nLen+sizeof(unsigned short);
		if( nPos>m_nShrinkSize || nLen>=nMaxSize )
		{
			// 无法读出
			throw	"ReadStr size err!";			// 外部需要catch这个
			return	-1;
		}
		memcpy(pszData, pShrink+sizeof(unsigned short), nLen);
		pszData[nLen]	= 0;						// 添加最后一个0
		m_nPos	= nPos;
		return	nLen;
	}
	// 可变长长度，后面可以直接用Write写入相应长度的数据
	int	ReadVSize(int *pnSize)
	{
		char	*pShrink	= m_pShrink+m_nPos;
		int		nSSize		= whbit_vn2_get(pShrink, GetSizeLeft(), (unsigned int *)pnSize);
		if( nSSize<=0 )
		{
			throw	"ReadVSize size err!";
			return	-1;
		}
		m_nPos	+= nSSize;
		return	nSSize;
	}
	// 可变长数据（返回数据长度）
	int	ReadVData(void *pData, int nMaxSize)
	{
		// 先读入长度
		int		nSize;
		int		nSSize		= ReadVSize(&nSize);
		if( nSSize<=0 )
		{
			throw	"ReadVData size err!";
			return	-1;
		}
		if( nSize>nMaxSize )
		{
			// 缓冲区不够
			// 需要把偏移指针移动回去
			m_nPos	-= nSSize;
			throw	"ReadVData buffer not enough!";
			return	-2;
		}
		// 再读入数据
		int		nDSize		= Read(pData, nSize);
		if( nDSize<=0 )
		{
			m_nPos	-= nSSize;
			throw	"ReadVData Read err!";
			return	-3;
		}
		return	nDSize;
	}

	int	Write(const void *pData, int nSize)
	{
		int	nPos = m_nPos + nSize;
		if( nPos>m_nShrinkSize )
		{
			// 无法写入
			throw	"Write size err!";			// 外部需要catch这个
			return	-1;
		}
		memcpy(m_pShrink+m_nPos, pData, nSize);
		m_nPos	= nPos;
		return	0;
	}
	template<typename _Ty>
	inline int	Write(const _Ty *pData)
	{
		return	Write(pData, sizeof(*pData));
	}
	#define	WHCMDSHRINK_WRITE_NBO_DECLARE(ty, fn)	\
	inline int	Write(const ty *pData)				\
	{												\
		ty	Data	= *pData;						\
		if( m_bNBO )								\
		{											\
			Data	= fn(*pData);					\
		}											\
		return	Write(&Data, sizeof(Data));			\
	}												\
	// EOF WHCMDSHRINK_WRITE_NBO_DECLARE
	WHCMDSHRINK_WRITE_NBO_DECLARE(int, htonl);
	WHCMDSHRINK_WRITE_NBO_DECLARE(long, htonl);
	WHCMDSHRINK_WRITE_NBO_DECLARE(short, htons);
	WHCMDSHRINK_WRITE_NBO_DECLARE(unsigned int, htonl);
	WHCMDSHRINK_WRITE_NBO_DECLARE(unsigned long, htonl);
	WHCMDSHRINK_WRITE_NBO_DECLARE(unsigned short, htons);
	int	WriteStr(const char *pszData)
	{
		// 注意：Write好之后没有最后一个0，所以长度不会超过原来带0结尾字串的长度
		char	*pShrink = m_pShrink+m_nPos;
		int		nLen = strlen(pszData);
		int		nPos = m_nPos + nLen+1;
		if( nPos>m_nShrinkSize || nLen>255 )
		{
			// 无法写入
			throw	"WriteStr size err!";			// 外部需要catch这个
			return	-1;
		}
		*(unsigned char *)pShrink	= nLen;
		memcpy(pShrink+1, pszData, nLen);
		m_nPos	= nPos;
		return	0;
	}
	int	WriteBigStr(const char *pszData, unsigned short nLen)
	{
		// 这个会比原来字串的长度多一个short
		char	*pShrink = m_pShrink+m_nPos;
		int		nPos = m_nPos + nLen+sizeof(unsigned short);
		if( nPos>m_nShrinkSize )
		{
			// 无法写入
			throw	"WriteStr size err!";			// 外部需要catch这个
			return	-1;
		}
		*(unsigned short *)pShrink	= nLen;
		memcpy(pShrink+sizeof(unsigned short), pszData, nLen);
		m_nPos	= nPos;
		return	0;
	}
	int	WriteBigStr(const char *pszData)
	{
		return	WriteBigStr(pszData, strlen(pszData));
	}
	// 可变长长度，后面可以直接用Write写入相应长度的数据
	int	WriteVSize(int nSize)
	{
		if( GetSizeLeft()<4+nSize )
		{
			return	-1;
		}
		char	*pShrink	= m_pShrink+m_nPos;
		int		nSSize		= whbit_vn2_set(pShrink, nSize);
		m_nPos	+= nSSize;
		return	nSSize;
	}
	// 可变长数据（返回总长度）
	int	WriteVData(const void *pData, int nSize)
	{
		// 先写入长度
		int		nSSize		= WriteVSize(nSize);
		if( nSSize<0 )
		{
			return	-1;
		}
		// 再写入数据
		int		nDSize		= Write(pData, nSize);
		if( nDSize<0 )
		{
			return	-2;
		}
		return	nSSize + nDSize;
	}
	// 结束，用于告诉外界
	inline void	WriteEnd()
	{
		m_nShrinkSize	= m_nPos;
	}
	// 辅助转换（一般用于别人peek出来的数据，想获取其中的某部分，转换为本机序）
	#define	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(ty, fn)	\
	inline ty	Translate(ty val)					\
	{												\
		if( m_bNBO )								\
		{											\
			return	fn(val);						\
		}											\
		else										\
		{											\
			return	val;							\
		}											\
	}												\
	// EOF WHCMDSHRINK_TRANSLATE_NBO_DECLARE
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(int, ntohl);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(long, ntohl);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(short, ntohs);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(unsigned int, ntohl);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(unsigned long, ntohl);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(unsigned short, ntohs);

	#define	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(ty, fn)	\
	inline ty	TranslateHToN(ty val)					\
	{												\
		if( m_bNBO )								\
		{											\
			return	fn(val);						\
		}											\
		else										\
		{											\
			return	val;							\
		}											\
	}												\
	// EOF WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(int, htonl);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(long, htonl);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(short, htons);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(unsigned int, htonl);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(unsigned long, htonl);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(unsigned short, htons);
};

////////////////////////////////////////////////////////////////////
// 命令处理相关
////////////////////////////////////////////////////////////////////
// 命令类型
typedef	unsigned int	whcmd_t;

enum
{
	// 没有参数
	WHCMDFUNC_PARAMTYPE_v									= 0,
	// 参数是一个void指针
	WHCMDFUNC_PARAMTYPE_p									= 1,
	// 参数是一个字串
	WHCMDFUNC_PARAMTYPE_s									= 2,
	// 参数是一个整数
	WHCMDFUNC_PARAMTYPE_i									= 11,
	// 参数是两个整数
	WHCMDFUNC_PARAMTYPE_ii									= 12,
	// 参数是三个整数
	WHCMDFUNC_PARAMTYPE_iii									= 13,
};

// 如果需要传递结果，应该通过类内部的全局可见变量。
class	whcmddealer;
typedef	void (whcmddealer::*whcmd_fc_t)(void);
typedef	void (whcmddealer::*whcmd_fc_v_t)(void);
typedef	void (whcmddealer::*whcmd_fc_p_t)(const void *, size_t);
typedef	void (whcmddealer::*whcmd_fc_c_t)(const char *);
typedef	void (whcmddealer::*whcmd_fc_i_t)(int);
typedef	void (whcmddealer::*whcmd_fc_ii_t)(int, int);
typedef	void (whcmddealer::*whcmd_fc_iii_t)(int, int, int);

#pragma pack(1)
// 通用的指令结构
struct	whcmd_cmd_buf_t
{
	whcmd_t	nCmd;
	char	szData[1];
};
// 下面是各种具体参数的结构
struct	whcmd_param_i_t
{
	int		nParam;
};
struct	whcmd_param_ii_t
{
	int		nParam1;
	int		nParam2;
};
struct	whcmd_param_iii_t
{
	int		nParam1;
	int		nParam2;
	int		nParam3;
};
#pragma pack()
// (下面两个函数保证了_Ty结构中有szData项)
// 通过数据长度得到总长度
template<typename _Ty>
inline size_t	whcmd_gettotalsize(_Ty *pCmd, size_t nDataSize)
{
	return	sizeof(*pCmd)-sizeof(pCmd->szData) + nDataSize;
}
// 通过总长度得到数据长度
template<typename _Ty>
inline size_t	whcmd_getdatasize(_Ty *pCmd, size_t nTotalSize)
{
	return	nTotalSize - sizeof(*pCmd)+sizeof(pCmd->szData);
}
// 把指令和参数整合到一起(返回总长度)
inline size_t	whcmd_assemble(char *szBuf, whcmd_t nCmd, const void *pParam, size_t nParamSize)
{
	whcmd_cmd_buf_t	*pBuf = (whcmd_cmd_buf_t *)szBuf;
	pBuf->nCmd	= nCmd;
	memcpy(pBuf->szData, pParam, nParamSize);
	return	whcmd_gettotalsize(pBuf, nParamSize);
}

struct	whcmd_entry_t
{
	const char			*cszKey;							// 函数名
	whcmd_t				nCmd;								// 对应指令名
	int					nParamType;							// 函数类型WHCMDFUNC_PARAMTYPE_XXX
															// 主要用来标识函数参数的类型
	whcmd_fc_t			pFunc;								// 函数指针
};
struct	whcmd_map_t
{
	const char			*cszClassName;						// 本类的类名
	const whcmd_map_t	*lpBaseMap;							// 指向父类的whcmd_map_t结构
	const whcmd_entry_t	*lpEntries;							// 本类的数据入口数组
};

// whcmddealer无需初始化，所有函数注册都是静态完成的
class	whcmddealer
{
public:
	virtual ~whcmddealer()	{}
	// 通过已经找到Entry调用相应的函数(public是因为SearchAndDo中的Job对象要调用)
	void	CallEntryDealCmd(const whcmd_entry_t *pEntry, const void *pData, size_t nSize);
	// 根据nCmd找到相应的函数，并调用之
	void	DealCmd(whcmd_t nCmd, const void *pData, size_t nSize);
	// 根据cszFuncName找到相应的函数，并调用之
	void	DealCmd(const char *cszFuncName, const void *pData, size_t nSize);
protected:
	// (注：在VC中，模板函数没法写到类定义外面，所以就这里了)
	// 搜索所有Entry，
	// 在pJob->Work返回true时停止搜索，函数返回true
	// 如果没有匹配到则返回false
	template<class JOB>
	bool	SearchAndDo(JOB &Job)
	{
		const whcmd_map_t	*pMap		= GetCmdMap();
		const whcmd_entry_t	*pEntry;

		while(pMap)
		{
			pEntry	= pMap->lpEntries;
			// 找符合的成员
			while( pEntry->cszKey )
			{
				if( Job.Work(pEntry) )
				{
					return	true;
				}
				pEntry	++;
			}
			// 找上级
			pMap	= pMap->lpBaseMap;
		}

		// 没有找到合适Entry
		return	false;
	}
protected:
	virtual const whcmd_map_t * GetCmdMap() const
	{
		// 如果子类不实现就返回空指针
		return	0;
	}
	virtual void	DealCmd_WhenUnknown(whcmd_t nCmd, const void *pData, size_t nSize)
	{}
	virtual void	DealCmd_WhenUnknown(const char *cszFuncName, const void *pData, size_t nSize)
	{}
};

////////////////////////////////////////////////////////////
// 声明函数映射必须的各个宏
////////////////////////////////////////////////////////////
// 在需要有函数映射的类中一定要写这个
// 在类定义中需要有这句话
#define	WHCMD_DECLARE_MAP(_theClass)						\
private:													\
	typedef	_theClass	theClass;							\
	static const n_whcmn::whcmd_entry_t	_cmdEntries[];		\
	static const n_whcmn::whcmd_map_t	_cmdMap;			\
public:														\
	virtual const n_whcmn::whcmd_map_t * GetCmdMap() const;	\
	// **** EOF WHCMD_DECLARE_MAP

// 普通类的映射定义
#define	WHCMD_MAP_BEGIN0(_theClass, _baseClassCmdMapPtr)	\
	const n_whcmn::whcmd_map_t * _theClass::GetCmdMap() const				\
		{ return &_theClass::_cmdMap; }						\
	const n_whcmn::whcmd_map_t _theClass::_cmdMap	=		\
		{													\
			#_theClass, _baseClassCmdMapPtr, _theClass::_cmdEntries			\
		};													\
	const n_whcmn::whcmd_entry_t _theClass::_cmdEntries[]	=				\
	{														\
	// **** EOF GC_BEGIN_MSG_MAP0
#define	WHCMD_MAP_BEGIN(_theClass, _baseClass)				\
	WHCMD_MAP_BEGIN0(_theClass, &_baseClass::_cmdMap)		\
	// **** EOF GC_BEGIN_MSG_MAP

// 最顶类的映射定义(即需要搜索成员的最根类)
#define	WHCMD_MAP_BEGIN_AT_ROOT(_theClass)					\
	WHCMD_MAP_BEGIN0(_theClass, NULL)						\
	// **** EOF WHCMD_MAP_BEGIN_AT_ROOT

// 映射定义结束
#define WHCMD_MAP_END()										\
		{ 0, 0, 0, 0 }										\
	};														\
	// **** EOF WHCMD_MAP_END

// 最基础的映射定义
#define	WHCMD_MAP_ON0(name, cmd, type, chk_type_t, func)	\
	{ name, cmd, type, (whcmd_fc_t)(chk_type_t)&theClass::func },			\
	// **** EOF WHCMD_MAP_ON0
// 用函数名做字符串名
#define	WHCMD_MAP_ON(cmd, type, chk_type_t, func)			\
	WHCMD_MAP_ON0(#func, cmd, type, chk_type_t, func)		\
	// **** EOF WHCMD_MAP_ON

// 下面是各个特定参数类型函数的定义，和WHCMDFUNC_PARAMTYPE_XXX对应
#define	WHCMD_MAP_ON_v(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_v, whcmd_fc_v_t, func)			\
	// **** EOF WHCMD_MAP_ON_v
#define	WHCMD_MAP_ON_p(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_p, whcmd_fc_p_t, func)			\
	// **** EOF WHCMD_MAP_ON_p
#define	WHCMD_MAP_ON_c(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_s, whcmd_fc_c_t, func)			\
	// **** EOF WHCMD_MAP_ON_c
#define	WHCMD_MAP_ON_i(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_i, whcmd_fc_i_t, func)			\
	// **** EOF WHCMD_MAP_ON_i
#define	WHCMD_MAP_ON_ii(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_ii, whcmd_fc_ii_t, func)			\
	// **** EOF WHCMD_MAP_ON_ii
#define	WHCMD_MAP_ON_iii(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_iii, whcmd_fc_iii_t, func)		\
	// **** EOF WHCMD_MAP_ON_iii

// 通用console命令处理线程对象
class	whcmn_nml_console
{
public:
	whsmpqueueWL	m_cmdqueue;								// 外界通过这个命令队列获得console的输入
	whtid_t			m_tid;									// console输入线程的线程id
	char			m_szPrompt[64];							// console输入提示符（默认"$ "）
	char			m_szBuf[32768];							// 一般输入都不会超过这个吧
public:
	whcmn_nml_console();
	// 启动线程(杀死线程不可以通过m_tid进行(那样只能使事情更糟)。一般应该是输入"exit"就自动结束，同时外界也可以通过m_cmdqueue中的exit来决定自己的结束)
	int		StartThread();
	// 等待结束并清空tid（线程需要有自己的结束机制，比如：收到exit）
	int		WaitThreadEnd(unsigned int nTimeout = 1000);
	// 真正被线程调用的部分
	int		ThreadFunc();
};

// 通用指令处理对象
// 说明：
// 1. 指令nCmd不要太大，因为越大内部分配的数组就越大(最好在1024以下，内部我认为超过65535就出错)
// 2. 子类最好在自己的构造函数中自己注册所有的指令
template<class _TyDealer>
class	whcmn_cmd_reganddeal
{
protected:
	typedef	int (_TyDealer::*cmd_deal_f)(const void *pData, size_t nDSize);
	struct	CMDUNIT_T
	{
		_TyDealer				*pThis;
		cmd_deal_f				pFunc;
	};
	whvector<CMDUNIT_T>			m_vectCmdDeal;
	_TyDealer					*m_pDftDealer;
	cmd_deal_f					m_pDftFunc;
public:
	int							m_nUnknownCmd;
public:
	whcmn_cmd_reganddeal()
	: m_pDftDealer(NULL)
	, m_pDftFunc(NULL)
	, m_nUnknownCmd(0)
	{
		m_vectCmdDeal.reserve(1024);
	}
	// 为m_vectCmdDeal预留空间
	inline void	Reserve(size_t nCap)
	{
		m_vectCmdDeal.reserve(nCap);
	}
	// 注册一条指令以及对应的函数
	int		RegCmd(int nCmd, _TyDealer *pThis, cmd_deal_f pFunc)
	{
		if( nCmd<0 )
		{
			// 默认函数
			m_pDftDealer	= pThis;
			m_pDftFunc		= pFunc;
			return	0;
		}
		if( nCmd>=65536 )
		{
			// 指令范围超界
			assert(0);
			return	-1;
		}
		if( nCmd+1>(int)m_vectCmdDeal.size() )
		{
			size_t	nOldSize	= m_vectCmdDeal.size();
			m_vectCmdDeal.resize(nCmd+1);
			for(int i=nOldSize;i<=nCmd;i++)
			{
				// 先把没用的都清0
				m_vectCmdDeal[i].pThis	= NULL;
				m_vectCmdDeal[i].pFunc	= NULL;
			}
		}
		if( m_vectCmdDeal[nCmd].pFunc!=NULL )
		{
			// 该指令原来已经注册过了
			assert(0);
			return	-2;
		}
		m_vectCmdDeal[nCmd].pThis		= pThis;
		m_vectCmdDeal[nCmd].pFunc		= pFunc;
		return	0;
	}
	// 调用一条指令
	int		CallFunc(int nCmd, const void *pData, size_t nDSize)
	{
		if( nCmd<0 || (size_t)nCmd>=m_vectCmdDeal.size() )
		{
			// 指令范围超界
			// 因为有可能是因为错误导致的（比如有人有意攻击），所以这里不用assert
			return	-1000;
		}

		cmd_deal_f	pFunc	= m_vectCmdDeal[nCmd].pFunc;
		// if( !pFunc ) 这样会导致VC编译出“fatal error C1001: INTERNAL COMPILER ERROR”
		if( pFunc == NULL )
		{
			// 没有这样的指令
			if( m_pDftDealer!=NULL
			&&  m_pDftFunc!=NULL
			)
			{
				// 调用默认指令
				m_nUnknownCmd	= nCmd;
				return	(m_pDftDealer->*m_pDftFunc)(pData, nDSize);
			}
			return	-1001;
		}

		return	(m_vectCmdDeal[nCmd].pThis->*pFunc)(pData, nDSize);
	}
};
// 这两个宏是为了所有用到whcmn_cmd_reganddeal的用户简便使用的
#define	WHCMN_CMD_REGANDDEAL_DECLARE0(classname, vname)		\
	whcmn_cmd_reganddeal<classname>	vname;					\
	typedef	classname	WHThisClass;						\
	// EOF WHCMN_CMD_REGANDDEAL_DECLARE0
#define	WHCMN_CMD_REGANDDEAL_REGCMD0(vname, nCmd, Func)		vname.RegCmd(nCmd, this, &WHThisClass::Func)
	// EOF WHCMN_CMD_REGANDDEAL_REGCMD0

#define	WHCMN_CMD_REGANDDEAL_DECLARE_m_rad(classname)		WHCMN_CMD_REGANDDEAL_DECLARE0(classname, m_rad)
	// EOF WHCMN_CMD_REGANDDEAL_DECLARE_m_rad
#define	WHCMN_CMD_REGANDDEAL_REGCMD(nCmd, Func)				WHCMN_CMD_REGANDDEAL_REGCMD0(m_rad, nCmd, Func)
	// EOF WHCMN_CMD_REGANDDEAL_REGCMD

// 通用字串指令处理对象
// 说明：
// 1. 默认指令不区分大小写
// 2. 子类最好在自己的构造函数中自己注册所有的指令
// 3. 需要注意的是：_TyDealer不能是函数体内部临时定义的类，否则会出类没定义的错
template<class _TyDealer, class _TyStr = whstrptr4hashNocase, class _HashFunc = _whstr_hashfuncNocase>
class	whcmn_strcmd_reganddeal
{
protected:
	typedef	int (_TyDealer::*cmd_deal_f)(const char *pszParam);
	struct	CMDUNIT_T
	{
		_TyDealer				*pThis;
		cmd_deal_f				pFunc;
		const char				*cszHelp;					// 2006-02-10 按刘毅建议添加。作为自动指令帮助。
		unsigned int			nExecMask;					// 2008-07-22 按刘毅建议添加。指令可执行判定掩码。
	};
	typedef	whhash<_TyStr, CMDUNIT_T, whcmnallocationobj, _HashFunc>	MYHASH_T;
	typedef	typename MYHASH_T::kv_iterator			MYHASHIT;
	MYHASH_T					m_mapCmd2Func;
	_TyDealer					*m_pDftDealer;
	cmd_deal_f					m_pDftFunc;
	char						m_szLINEEND[4];
public:
	char						m_szUnknownCmd[1024];		// 这个可以用来记录不能识别的指令
	bool						m_bShouldHelp;				// 在处理过程中可以设置这个表示需要上层输出help
	bool						m_bSortCmd;					// 表示在Help的时候按照cmd进行排序（如果不sort的话显式的顺序就是hash表中的顺序，这个会比较乱）
public:
	whcmn_strcmd_reganddeal()
	: m_pDftDealer(NULL)
	, m_pDftFunc(NULL)
	, m_bShouldHelp(false)
	, m_bSortCmd(true)
	{
		m_szUnknownCmd[0]		= 0;
		strcpy(m_szLINEEND, WHLINEEND);
	}
	inline void	SetLineEnd(const char *cszLineEnd)
	{
		WH_STRNCPY0(m_szLINEEND, cszLineEnd);
	}
	inline void	SetSortCmd(bool bSet)
	{
		m_bSortCmd	= bSet;
	}
	// 注册一条指令以及对应的函数
	int		RegCmd(const char *pszCmd, _TyDealer *pThis, cmd_deal_f pFunc, const char *cszHelp=NULL, unsigned int nExecMask=0xFFFFFFFF)
	{
		if( pszCmd==NULL )
		{
			// 设置默认函数
			m_pDftDealer	= pThis;
			m_pDftFunc		= pFunc;
			return	0;
		}
		CMDUNIT_T	*pUnit;
		if( !m_mapCmd2Func.putalloc(pszCmd, pUnit) )
		{
			return	-1;
		}
		pUnit->pThis		= pThis;
		pUnit->pFunc		= pFunc;
		pUnit->cszHelp		= cszHelp;
		pUnit->nExecMask	= nExecMask;
		return	0;
	}
	// 调用一条指令
	int		CallFunc(const char *pszCmd, const char *pszParam, unsigned int nExecMask=0xFFFFFFFF)
	{
		m_bShouldHelp	= false;
		CMDUNIT_T	*pUnit;
		if( !m_mapCmd2Func.get(pszCmd, &pUnit) )
		{
			if( m_pDftDealer!=NULL
			&&  m_pDftFunc!=NULL
			)
			{
				// 调用默认指令
				strcpy(m_szUnknownCmd, pszCmd);
				return	(m_pDftDealer->*m_pDftFunc)(pszParam);
			}
			return	-1001;
		}
		if( (nExecMask & pUnit->nExecMask) != 0 )
		{
			return	(pUnit->pThis->*pUnit->pFunc)(pszParam);
		}
		return		-1002;
	}
	// 输出指令帮助到Buffer（如果pszCmd为空则输出所有指令帮助）
	// 返回szBuffer指针
	const char *	Help(char *szBuffer, int nBufLen, const char *pszCmd=NULL)
	{
		char	*pszOldBuffer	= szBuffer;
		int		nOldBufLen		= nBufLen;
		bool	bIsPattern		= false;
		whsafeptr<whstrlike>	pwsl(whstrlike::Create());
		szBuffer[0]				= 0;	// 先清空字串
		if( pszCmd && strchr(pszCmd, '%') )
		{
			bIsPattern			= true;
			pwsl->SetPattern(pszCmd);
		}
		if( pszCmd == NULL || pszCmd[0]==0 || bIsPattern )
		{
			std::map<_TyStr, const char *>	sortList;
			// 列出全部指令
			for(MYHASHIT itkv=m_mapCmd2Func.begin();itkv!=m_mapCmd2Func.end();++itkv)
			{
				CMDUNIT_T	&unit	= itkv.getvalue();
				if( unit.cszHelp != NULL )
				{
					if( bIsPattern )
					{
						// 看看是否模式匹配
						if( !pwsl->IsLike(itkv.getkey()) )
						{
							continue;
						}
					}
					if( m_bSortCmd )
					{
						sortList.insert( std::make_pair(itkv.getkey(), unit.cszHelp) );
					}
					else
					{
						int	rst	= snprintf(szBuffer, nBufLen, "%s %s%s", itkv.getkey().GetPtr(), unit.cszHelp, m_szLINEEND);
						szBuffer	+= rst;
						nBufLen		-= rst;
						if( rst<=0 || nBufLen<=0 )
						{
							// 没有空间了
							break;
						}
					}
				}
			}
			if( m_bSortCmd )
			{
				// 浏览map
				for(typename std::map<_TyStr, const char *>::iterator it=sortList.begin();it!=sortList.end();++it)
				{
					int	rst	= snprintf(szBuffer, nBufLen, "%s %s%s", (*it).first.GetPtr(), (*it).second, m_szLINEEND);
					szBuffer	+= rst;
					nBufLen		-= rst;
					if( rst<=0 || nBufLen<=0 )
					{
						// 没有空间了
						break;
					}
				}
			}
			if( nBufLen == nOldBufLen )
			{
				// 说明没有找到任何指令
				snprintf(szBuffer, nBufLen, "[NO SUCH CMD]:%s", pszCmd);
			}
		}
		else
		{
			CMDUNIT_T	*pUnit;
			if( m_mapCmd2Func.get(pszCmd, &pUnit) )
			{
				// 列出相应的指令
				if( pUnit->cszHelp )
				{
					snprintf(szBuffer, nBufLen, "%s %s", pszCmd, pUnit->cszHelp);
				}
				else
				{
					snprintf(szBuffer, nBufLen, "cmd:%s [NO HELP!]", pszCmd);
				}
			}
			else
			{
				// 没有这样的指令
				snprintf(szBuffer, nBufLen, "[NO SUCH CMD]:%s", pszCmd);
			}
		}
		// 为了保险给最后加上一个0
		pszOldBuffer[nOldBufLen-1]	= 0;
		return	pszOldBuffer;
	}
};
// 这两个宏是为了所有用到whcmn_strcmd_reganddeal的用户简便使用的
#define	whcmn_strcmd_reganddeal_DECLARE0(classname, vname)		\
	whcmn_strcmd_reganddeal<classname>	vname;					\
	typedef	classname	WHThisClass;							\
	// EOF whcmn_strcmd_reganddeal_DECLARE0
#define	whcmn_strcmd_reganddeal_REGCMD0(vname, pszCmd, Func, help, mask)	vname.RegCmd(pszCmd, this, &WHThisClass::Func, help, mask)
	// EOF whcmn_strcmd_reganddeal_REGCMD0

#define	whcmn_strcmd_reganddeal_DECLARE_m_srad(classname)			whcmn_strcmd_reganddeal_DECLARE0(classname, m_srad)
	// EOF whcmn_strcmd_reganddeal_DECLARE_m_srad
#define	whcmn_strcmd_reganddeal_REGCMD_m_srad0(pszCmd, Func, help, mask)	whcmn_strcmd_reganddeal_REGCMD0(m_srad, pszCmd, Func, help, mask)
	// EOF whcmn_strcmd_reganddeal_REGCMD_m_srad0
#define	whcmn_strcmd_reganddeal_REGCMD_m_srad(pszCmd, Func, help)	whcmn_strcmd_reganddeal_REGCMD_m_srad0(pszCmd, Func, help, 0xFFFFFFFF)
	// EOF whcmn_strcmd_reganddeal_REGCMD_m_srad
#define	whcmn_strcmd_reganddeal_REGCMD_m_srad_smp0(cmd, help, mask)	whcmn_strcmd_reganddeal_REGCMD_m_srad0(#cmd, _i_srad_##cmd, help, mask)
	// EOF whcmn_strcmd_reganddeal_REGCMD
#define	whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(cmd, help)		whcmn_strcmd_reganddeal_REGCMD_m_srad(#cmd, _i_srad_##cmd, help)
	// EOF whcmn_strcmd_reganddeal_REGCMD

// 分析测试串（最终结果放到vect中）
// <randstr>:size		产生size长度的随机字串
// <randbin>:size		产生size长度的随机二进制数据
// <dupstr>:num,str		产生str重复num次的字串
// <dupbin>:num,hexstr	产生hexstr表示的二进制数据重复num次的二进制数据
// <file>:filename		把文件内容读入
int	whcmd_make_tststr(const char *cszStr, whvector<char> *pvect, bool *pbIsStr);

}		// EOF namespace n_whcmn

#endif	// EOF __WHCMD_H__

