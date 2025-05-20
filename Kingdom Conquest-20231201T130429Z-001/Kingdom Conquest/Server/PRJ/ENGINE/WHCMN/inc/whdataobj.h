// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataobj.h
// Creator      : Wei Hua (魏华)
// Comment      : 用于为逻辑层提供简单的属性获取和设置方法，同时隐藏底层的数据存取细节
//               （包括如何从数据源初始化对象；如何在共享内存中申请对象；如何在发生宕机事件后从共享内存中恢复数据并重新存储；如何进行增量存储等）。
//                想法：可能会采用自动生成代码。
//                注：如果确定不需要使用list成员，则whdataobjbase_t相关的基础数据对象可以直接new出来或者使用局部变量。参见tty_common_PlayerData.h中的说明。
// CreationDate : 2006-02-14
// ChangeLOG    : 2006-02-22 增加了数组类型
//              : 2006-02-27 list类型原来WriteValueToStream的时候是按照hash表的浏览顺序，这样会导致ReadValueFromStream新表中的元素顺序可能和最初加入顺序不同。所以改为按照m_pList的浏览顺序。
//              : 2006-03-08 增加了对象段结尾指令WHDATAOBJ_CMD_ENDOFOBJ。这样可以在一个流中存储多段不同对象数据。这个和老的协议是兼容的。老协议在流中只有一个对象，最后没有结尾指令。读取函数在没有数据的时候结束。
//              : 2006-03-16 给各个ReadValueFromStream函数内部增加了设置为modified的功能。这样通过流导致的更改也可用被记录下来，这样将来就可以通过流对用户数据进行在线修改。
//              :            不过要记住，用户进入服务器最开始用流初始化有需要调用ClearAllModified来清除所有更改标志。
//              : 2006-03-21 增加了whdataobjbase_t的IsModified方法，可被上层用来判断整个对象是否发生了改变。
//              : 2006-04-18 修正了原来whdataobj_listtype_prop_t中一些地方之删除了hash表中的条目没有删除list中的条目的错误。
//              :            修正了whdataobj_listtype_prop_t::ClearModifiedWithLock没有把标记为删除的条目真正删除的错误。
//              :            如果list条目被添加（定时存盘失败）、删除（定时存盘失败）、又添加，这时条目的状态将是被修改，而不是被添加。
//              : 2006-04-24 增加了list的end方法
//              : 2006-06-02 修正了writeall时把DEL的数据也发回数据库的错误
//              : 2006-06-26 在writeall时不再修改单元的状态，而是直接分情况写入单元，这样不会导致元素的修改状态被改变（这样在运行过程中进行数据dump就没问题了*^_^*）
//              : 2007-04-05 修正了whdataobj_charptrtype_utf8based_prop_t在从共享内存中恢复数据时没有立即回复UTF8内容，导致存盘数据出错的问题。
//              : 2007-05-28 增加了从stream中读取数据时可以选择不设置修改标记，并如果和本地的已经修改的发生冲突则本地更新失效的机制。
//              : 2007-06-21 增加了whdataobj_listtype_prop_t的DelAll方法，用于一次性清空整个列表。
//              : 2007-08-22 取消了hash表m_mapNameID2PropPtr，因为直接用数组就好了。而且这样有利于把auto对象也放到共享内存中去。
//              : 2007-08-24 取消了所有自动对象中的实际内存分配、释放操作，用管理器的相应方法取而代之
//              : 2007-09-29 修正了list的adjust中没有调整hash表中数值内的指针的问题
//              : 2007-11-07 取消了各个读取函数中的assert，因为现在可能这样的数据会从客户端传过来。
//              : 2007-11-21 增加了marker类型，用于给杨纮宇标记各段数据
//              : 2007-11-22 增加了IsToWrite()，修正了由于增加了marker导致数据永远都是被修改状态的bug
//              : 2007-12-11 给所有的数据类型都增加只读选项
//              : 2008-01-15 增加了list类型的逻辑尺寸，以免在标记删除但是没有删除时统计的个数不对
//              : 2008-03-04 增加了避免whdataobj_arraytype_prop_t在读取数据时下表越界的assert（当数据源增加了数据长度，而使用放没有增加时就可能发生越界的情况）
//              :            给whdataobj_arraytype_prop_t增加了自动把零元素取消modified的状态的功能SetZeroNotModified
//              : 2008-03-19 修正了whdataobj_listtype_prop_t中AddModify的元素为刚刚增加且正在存盘过程中，之后调用删除会立即把元素删除，最后没有更新DB的问题。
//              :            此问题由高薇和杨纮宇联合发现:)

#ifndef	__WHDATAOBJ_H__
#define	__WHDATAOBJ_H__

#include "whfile.h"
#include "whallocmgr2.h"
#include "whdataobj_def.h"
#include "whbits.h"

namespace n_whcmn
{

// 数据对象的属性基础(这种对象因为有虚函数，所以就不用才共享内存中申请了。申请了也可能在恢复的时候会出问题)
template<class _AllocationObj>
class	whdataobj_prop_t
{
public:
	typedef	whallocmgr2<_AllocationObj>			AM_T;
	// 这两个属性还是要公开的，因为其他对象会访问，也会改写
	WHDATAOBJ_NAMEID_T	m_nNameID;									// 和数据库通讯标明这是哪个变量
	unsigned char		m_nProp;									// 其他属性
	enum
	{
		PROP_RDONLY		= 0x01,										// 表示被设置为只读，无论任何情况下都不能存盘
	};
	// 设置只读
	void	SetRDOnly(bool bSet)
	{
		if( bSet )
		{
			whcmnbit_set(m_nProp, (unsigned char)PROP_RDONLY);
		}
		else
		{
			whcmnbit_clr(m_nProp, (unsigned char)PROP_RDONLY);
		}
	}
public:
	whdataobj_prop_t()
		: m_nNameID(0)
		, m_nProp(0)
	{
	}
	whdataobj_prop_t(const NULLCONSTRUCT_T &nc)
	{
		NOTUSE(nc);
	}
	virtual	~whdataobj_prop_t()										{};
	// 用于把内部的虚表更新
	virtual void	NewInner(void *pMem)							= 0;
	// 返回该元素是否被修改过了
	virtual bool	IsModified() const								= 0;
	// 表示改元素是否应该被写入（一般来说是等价于IsModified）
	virtual bool	IsToWrite() const
	{
		if( whcmnbit_chk(m_nProp, (unsigned char)PROP_RDONLY) )	// 如果是只读则就表示不能写
		{
			return	false;
		}
		return	IsModified();
	}
	// 清除所有的修改状态
	virtual void	ClearModified()									= 0;
	// 设置被锁定的修改状态
	virtual void	SetModifiedWithLock()							= 0;
	// 清除被锁定的修改状态
	virtual void	ClearModifiedWithLock()							= 0;
	// 清除数据值，同时也清空修改标志。用于获得一个“干净”的对象（注意，这个不是DelAll操作）
	virtual void	ClearValue()									= 0;
	// 把自己的内容写入流中
	virtual int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
																	= 0;
	// 从流中读出数据填入自己
	// 如果bSetModify为假则不设置Modify标志（即读出的数据对象会被认为是没有改变过的），并且如果本地数据已经被设置了modify则不进行修改
	virtual int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
																	= 0;
	// 调整内部指针
	virtual void	AdjustInnerPtr(AM_T *pMgr, long nOffset)		= 0;
	// 获得值的缓冲区（上层应该知道是什么类型的）
	virtual void *	GetValueBuf()									= 0;
};

// 游戏中的数据对象都从whdataobjbase_t派生
template<class _AllocationObj>
struct	whdataobjbase_t
{
protected:
	WHDATAOBJ_NAMEID_T				m_ver;							// 自己的版本
	WHDATAOBJ_NAMEID_T				m_vermin;						// 兼容的最低版本（这个用来发给接收方，以确保是否可以读取）
	bool							m_bWriteMarker;					// 是否在WriteModifiedValueToStream中记录marker（这个默认是不打开的）
public:
	// 这个是必须定义的（而且其他地方要用，就写成public的了）
	typedef	whallocmgr2<_AllocationObj>			AM_T;
	typename AM_T::template vector<whdataobj_prop_t<_AllocationObj> *>		m_vectNameID2PropPtr;
																	// m_vectNameID2PropPtr里面存着分配管理器的指针
public:
	whdataobjbase_t()
		: m_ver(0), m_vermin(0)
		, m_bWriteMarker(false)
	{
	}
	inline void	SetWriteMarker(bool bSet)
	{
		m_bWriteMarker	= bSet;
	}
	inline bool	GetWriteMarker() const
	{
		return	m_bWriteMarker;
	}
	inline WHDATAOBJ_NAMEID_T	GetVer() const
	{
		return	m_ver;
	}
	inline void	SetVer(WHDATAOBJ_NAMEID_T ver, WHDATAOBJ_NAMEID_T vermin)
	{
		m_ver		= ver;
		m_vermin	= vermin;
	}
	// 浏览对象并调用回调
	int	BrowseData(void (*pFuncNotify)(whdataobj_prop_t<_AllocationObj> *))
	{
		for(size_t i=0; i<m_vectNameID2PropPtr.size(); ++i)
		{
			(*pFuncNotify)(m_vectNameID2PropPtr[i]);
		}
		return	0;
	}
	// 写入流的头部信息
	void	_WriteToStream_Head(char *&pBufStream, int &nSizeLeft)
	{
		// 首先写入版本
		*(WHDATAOBJ_CMD_T*)pBufStream	= WHDATAOBJ_CMD_VERSION;
		pBufStream	+= sizeof(WHDATAOBJ_CMD_T);
		nSizeLeft	-= sizeof(WHDATAOBJ_CMD_T);
		*(WHDATAOBJ_NAMEID_T*)pBufStream= m_vermin;
		pBufStream	+= sizeof(WHDATAOBJ_NAMEID_T);
		nSizeLeft	-= sizeof(WHDATAOBJ_NAMEID_T);
		*(WHDATAOBJ_NAMEID_T*)pBufStream= m_ver;
		pBufStream	+= sizeof(WHDATAOBJ_NAMEID_T);
		nSizeLeft	-= sizeof(WHDATAOBJ_NAMEID_T);
	}
	// 浏览对象如果更改过就写入缓冲
	// 成功返回0，出错返回<0
	int	WriteModifiedValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll=false)
	{
		_WriteToStream_Head(pBufStream, nSizeLeft);
		// 然后写入数据
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			if( pProp->IsToWrite() || bWriteAll )
			{
				if( pProp->WriteValueToStream(pBufStream, nSizeLeft, bWriteAll)<0 )
				{
					return	-pProp->m_nNameID;
				}
			}
		}
		return	0;
	}
	// 把各个修改过的部分的统计信息打出来
	// 目前只有在stream中占用的空间量
	int	StatModified(char *pszDispBuf, int nMaxDispSize, bool bWriteAll)
	{
		char	szBufStream[32768];
		char	*pBufStream	= szBufStream;
		int		nSizeLeft	= sizeof(szBufStream);
		_WriteToStream_Head(pBufStream, nSizeLeft);
		int		nDispRst	= snprintf(pszDispBuf, nMaxDispSize, "hdr %d%s", sizeof(szBufStream)-nSizeLeft, WHLINEEND);
		pszDispBuf			+= nDispRst;
		nMaxDispSize		-= nDispRst;
		// 然后写入数据
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			if( pProp->IsModified() || bWriteAll )
			{
				pBufStream		= szBufStream;
				nSizeLeft		= sizeof(szBufStream);
				if( pProp->WriteValueToStream(pBufStream, nSizeLeft, bWriteAll)<0 )
				{
					return	-pProp->m_nNameID;
				}
				nDispRst		= snprintf(pszDispBuf, nMaxDispSize, "%d %d%s", pProp->m_nNameID, sizeof(szBufStream)-nSizeLeft, WHLINEEND);
				pszDispBuf		+= nDispRst;
				nMaxDispSize	-= nDispRst;
			}
		}
		return	0;
	}
	// 根据数据流中的数据对本身数据进行修改
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify=true)
	{
		WHDATAOBJ_CMD_T		cmd;
		WHDATAOBJ_NAMEID_T	nNameID;
		// 首先检查版本
		cmd			= *(WHDATAOBJ_CMD_T *)pBufStream;
		pBufStream	+= sizeof(cmd);
		nSizeLeft	-= sizeof(cmd);
		if( cmd != WHDATAOBJ_CMD_VERSION )
		{
			return	-1;
		}
		if( nSizeLeft<=0 )
		{
			return	-2;
		}
		nNameID		= *(WHDATAOBJ_NAMEID_T *)pBufStream;
		pBufStream	+= sizeof(nNameID);
		nSizeLeft	-= sizeof(nNameID);
		if( nNameID > m_ver )
		{
			// 说明对方不兼容自己的版本
			return	-3;
		}
		if( nSizeLeft<=0 )
		{
			return	-4;
		}
		nNameID		= *(WHDATAOBJ_NAMEID_T *)pBufStream;
		pBufStream	+= sizeof(nNameID);
		nSizeLeft	-= sizeof(nNameID);
		if( nNameID < m_vermin )
		{
			// 说明自己不兼容对方的版本
			return	-5;
		}
		if( nSizeLeft<0 )
		{
			return	-6;
		}
		if( nSizeLeft==0 )
		{
			// 没有数据（这个倒是很奇怪的事情）
			return	0;
		}
		// 
		while( nSizeLeft>0 )
		{
			cmd			= *(WHDATAOBJ_CMD_T *)pBufStream;
			pBufStream	+= sizeof(cmd);
			nSizeLeft	-= sizeof(cmd);
			if( cmd == WHDATAOBJ_CMD_ENDOFOBJ )
			{
				// 此段对象结束
				break;
			}
			nNameID		= *(WHDATAOBJ_NAMEID_T *)pBufStream;
			pBufStream	+= sizeof(nNameID);
			nSizeLeft	-= sizeof(nNameID);
			if(nSizeLeft<=0)
			{
				switch( cmd )
				{
				case	WHDATAOBJ_CMD_MARKER:
					{
						// 这个可以继续
					}
					break;
				default:
					{
						// assert(0);	因为这个数据也可能是从客户端发来的，所以不要assert了
						return	-11;
					}
					break;
				}
			}
			// 判断nNamdID是否合法（因为客户端可能发来非法的数据）
			if( !m_vectNameID2PropPtr.IsPosValid(nNameID) )
			{
				return	-20;
			}
			// 根据nNameID找到对应的prop
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[nNameID];
			if( pProp )	// 如果找不到就不用继续了，说明可能是新增加的东西，但是也不应该出错
			{
				switch( cmd )
				{
				case	WHDATAOBJ_CMD_MARKER:
				case	WHDATAOBJ_CMD_VAR_MODIFY:
				case	WHDATAOBJ_CMD_LIST_MODIFY_BEGIN:
				case	WHDATAOBJ_CMD_ARR_MODIFY_BEGIN:
					{
						if( pProp->ReadValueFromStream(pBufStream, nSizeLeft, bSetModify)<0 )
						{
							return	-21;
						}
					}
					break;
				default:
					{
						// 这个是不应该有的
						// assert(0);	因为这个数据也可能是从客户端发来的，所以不要assert了
						return	-22;
					}
					break;
				}
			}
			else
			{
				return	-23;
			}
		}
		return	0;
	}
	// 看看数据对象是否是被修改过的
	bool	IsModified()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			if( pProp->IsModified() )
			{
				return	true;
			}
		}
		return	false;
	}
	// 清除所有更改标志(这个应该是在从最开始初始化ReadValueFromStream之后调用的，以保证之后新的修改才会被标记)
	// （如果读取的时候设置了bSetModify为假，则可以不调用这个清除了）
	void	ClearAllModified()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			pProp->ClearModified();
		}
	}
	// 给所有修改状态的成员设置LOCK状态(这个应该是在发出存盘指令之后设置的)
	void	SetAllModifiedWithLock()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			pProp->SetModifiedWithLock();
		}
	}
	// 清除所有锁定的更改标志(这个应该是在成功存盘之后调用的)
	void	ClearAllModifiedWithLock()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			pProp->ClearModifiedWithLock();
		}
	}
	// 清空所有值
	void	ClearAllValue()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			pProp->ClearValue();
		}
	}
	// pGoodBuddy是一个正常的对象
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset, whdataobjbase_t *pGoodBuddy)
	{
		// 把vector内部缓冲调整一下
		m_vectNameID2PropPtr.AdjustInnerPtr(pMgr, nOffset);
		// 把每个指针都调整一下
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*&pObj	= m_vectNameID2PropPtr[i];
			wh_adjustaddrbyoffset(pObj, nOffset);
			pGoodBuddy->m_vectNameID2PropPtr[i]->NewInner(pObj);
			pObj->AdjustInnerPtr(pMgr, nOffset);
		}
	}
	void *	GetValueBuf(WHDATAOBJ_NAMEID_T nNameID)
	{
		if( nNameID>=(int)m_vectNameID2PropPtr.size() )
		{
			return	NULL;
		}
		return	m_vectNameID2PropPtr[nNameID]->GetValueBuf();
	}
	template<typename _Ty>
	void	GetValue(int nNameID, _Ty &val)
	{
		void	*ptr	= GetValueBuf(nNameID);
		if( ptr )
		{
			val	= *(_Ty *)ptr;
		}
	}
};

template<class _AllocationObj>
class	whdataobj_marker_prop_t	: public whdataobj_prop_t<_AllocationObj>
{
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
	typedef	whallocmgr2<_AllocationObj>			AM_T;
protected:
	whdataobjbase_t<_AllocationObj>				*m_pHost;		// 附着母体
public:
	whdataobj_marker_prop_t()
		: m_pHost(NULL)
	{
	}
	whdataobj_marker_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	// 用于把内部的虚表更新
	virtual void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_marker_prop_t(NULLCONSTRUECTOR);
	}
	// 返回该元素是否被修改过了（这个永远不应该影响是否Modified的判断）
	virtual bool	IsModified() const
	{
		return	false;
	}
	// 表示改元素是否应该被写入（一般来说是等价于IsModified）（但是在这里必须永远写入，只要别人有修改，这个就要写入）
	virtual bool	IsToWrite() const
	{
		return	true;
	}
	// 清除所有的修改状态
	virtual void	ClearModified()
	{
	}
	// 设置被锁定的修改状态
	virtual void	SetModifiedWithLock()
	{
	}
	// 清除被锁定的修改状态
	virtual void	ClearModifiedWithLock()
	{
	}
	// 清除数据值，同时也清空修改标志。用于获得一个“干净”的对象（注意，这个不是DelAll操作）
	virtual void	ClearValue()
	{
	}
	// 把自己的内容写入流中
	virtual int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		if( !m_pHost->GetWriteMarker() )
		{
			// 如果上层不用写marker，这个就不用写了
			return	0;
		}
		// 一般来说外界提供的buffer都是够长的
		whfile_i_mem	f(pBufStream, nSizeLeft);
		// 指令：变量更改
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_MARKER;
		f.Write(&cmd);
		// 写入NameID
		f.Write(&this->m_nNameID);
		// 更改后面可写的指针pBufStream和最大长度
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	// 从流中读出数据填入自己
	// 如果bSetModify为假则不设置Modify标志（即读出的数据对象会被认为是没有改变过的），并且如果本地数据已经被设置了modify则不进行修改
	virtual int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		return	0;
	}
	// 调整内部指针
	virtual void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// 调整附着体指针
		wh_adjustaddrbyoffset(m_pHost, nOffset);
	}
	// 获得值的缓冲区（上层应该知道是什么类型的）
	virtual void *	GetValueBuf()
	{
		return	NULL;
	}
public:
	// 自己用的
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, WHDATAOBJ_NAMEID_T nNameID)
	{
		m_pHost					= pHost;
		this->m_nNameID			= nNameID;
		// 加入映射表
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
	}
};

// 固定数据类型（包括一般类型和结构等）
template<typename _Ty, class _AllocationObj>
class	whdataobj_cmntype_prop_t	: public whdataobj_prop_t<_AllocationObj>
{
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
	typedef	whallocmgr2<_AllocationObj>			AM_T;
protected:
	enum
	{
		MDSTAT_LOCKED	= 0x80,		// 表示被设置锁定了
		MDSTAT_MODIFIED	= 0x01,		// 表示被修改了
	};
	unsigned char	m_nMDStat;		// 表示是否被修改
	whdataobjbase_t<_AllocationObj>	*m_pHost;		// 附着母体
	_Ty		*m_pVar;
public:
	whdataobj_cmntype_prop_t()
		: m_nMDStat(0)
		, m_pHost(NULL)
		, m_pVar(NULL)
	{
	}
	whdataobj_cmntype_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, _Ty *pVar, WHDATAOBJ_NAMEID_T nNameID)
	{
		m_pHost					= pHost;
		m_pVar					= pVar;
		this->m_nNameID			= nNameID;
		// 加入映射表
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
	}
	inline const _Ty &	get() const
	{
		return	*m_pVar;
	}
	inline void	set(const _Ty & val)
	{
		*m_pVar		= val;
		// 如果原来被lock了重新设置则lock状态被清除
		m_nMDStat	= MDSTAT_MODIFIED;
	}
	inline _Ty *	setbyptr()
	{
		// 如果原来被lock了重新设置则lock状态被清除
		m_nMDStat	= MDSTAT_MODIFIED;
		return		m_pVar;
	}
	bool	IsModified() const
	{
		return	(m_nMDStat & MDSTAT_MODIFIED) != 0;
	}
	void	ClearModified()
	{
		m_nMDStat	= 0;
	}
	void	SetModifiedWithLock()
	{
		if( m_nMDStat != 0  )
		{
			m_nMDStat	|= MDSTAT_LOCKED;
		}
	}
	void	ClearModifiedWithLock()
	{
		if( (m_nMDStat & MDSTAT_LOCKED) != 0 )
		{
			// 只有在LOCK过程中没有被重新修改过才能重置状态
			m_nMDStat	= 0;
		}
	}
	void	ClearValue()
	{
		memset(m_pVar, 0, sizeof(*m_pVar));
		ClearModified();
	}
	int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		// 一般来说外界提供的buffer都是够长的
		whfile_i_mem	f(pBufStream, nSizeLeft);
		// 指令：变量更改
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_VAR_MODIFY;
		f.Write(&cmd);
		// 写入NameID
		f.Write(&this->m_nNameID);
		// 写入数据(因为长度是固定的)
		f.Write(m_pVar);
		// 更改后面可写的指针pBufStream和最大长度
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		// 上层应该已经读出了指令和NameID部分
		memcpy(m_pVar, pBufStream, sizeof(*m_pVar));
		pBufStream	+= sizeof(*m_pVar);
		nSizeLeft	-= sizeof(*m_pVar);
		if( nSizeLeft>=0 )
		{
			if( bSetModify )
			{
				m_nMDStat	= MDSTAT_MODIFIED;	// 这样设置直接就可以把locked的状态清除
			}
			else
			{
				ClearModified();
			}
			return	0;
		}
		return	-1;
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// 调整附着体指针
		wh_adjustaddrbyoffset(m_pHost, nOffset);
		// 调整变量指针
		wh_adjustaddrbyoffset(m_pVar, nOffset);
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_cmntype_prop_t(NULLCONSTRUECTOR);
	}
	void *	GetValueBuf()
	{
		return	m_pVar;
	}
};

// 字串类型
template<class _AllocationObj>
class	whdataobj_charptrtype_prop_t	: public whdataobj_prop_t<_AllocationObj>
{
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
	typedef	whallocmgr2<_AllocationObj>			AM_T;
protected:
	enum
	{
		MDSTAT_LOCKED	= 0x80,		// 表示被设置锁定了
		MDSTAT_MODIFIED	= 0x01,		// 表示被修改了
	};
	unsigned char	m_nMDStat;		// 表示是否被修改
	whdataobjbase_t<_AllocationObj>	*m_pHost;	// 附着母体
	char	*m_pVar;				// 字串指针
	size_t	m_nMaxSize;				// 字串变量最大长度
public:
	whdataobj_charptrtype_prop_t()
		: m_nMDStat(0)
		, m_pHost(NULL)
		, m_pVar(NULL)
		, m_nMaxSize(0)
	{
	}
	whdataobj_charptrtype_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, char *pVar, size_t nMaxSize, WHDATAOBJ_NAMEID_T nNameID)
	{
		m_pHost					= pHost;
		m_pVar					= pVar;
		m_nMaxSize				= nMaxSize;
		this->m_nNameID			= nNameID;
		// 加入映射表
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
	}
	inline const char *	get() const
	{
		return	m_pVar;
	}
	inline void	set(const char *pStr)
	{
		strncpy(m_pVar, pStr, m_nMaxSize-1);
		m_pVar[m_nMaxSize-1]	= 0;
		m_nMDStat	= MDSTAT_MODIFIED;
	}
	inline size_t	getmaxsize() const
	{
		return	m_nMaxSize;
	}
	inline char *	setbyptr()	// 注意：这个就得外部自己管好长度了。不能超常哦。!!!!
	{
		m_nMDStat	= MDSTAT_MODIFIED;
		return		m_pVar;
	}
	bool	IsModified() const
	{
		return	(m_nMDStat & MDSTAT_MODIFIED) != 0;
	}
	void	ClearModified()
	{
		m_nMDStat	= 0;
	}
	void	SetModifiedWithLock()
	{
		if( m_nMDStat != 0  )
		{
			m_nMDStat	|= MDSTAT_LOCKED;
		}
	}
	void	ClearModifiedWithLock()
	{
		if( (m_nMDStat & MDSTAT_LOCKED) != 0 )
		{
			// 只有在LOCK过程中没有被重新修改过才能重置状态
			m_nMDStat	= 0;
		}
	}
	void	ClearValue()
	{
		m_pVar[0]	= 0;
		ClearModified();
	}
	int		_WriteValueToStream(const char *pszStr, char *&pBufStream, int &nSizeLeft, bool bWriteAll, int nMaxSize)
	{
		// 一般来说外界提供的buffer都是够长的
		whfile_i_mem	f(pBufStream, nSizeLeft);
		// 指令：变量更改
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_VAR_MODIFY;
		f.Write(&cmd);
		// 写入NameID
		f.Write(&this->m_nNameID);
		// 按写入变长字串（nMaxSize用来保证pszStr不会因为超界而导致过长）
		if( f.WriteVStr(pszStr, nMaxSize)<0 )
		{
			return	-1;
		}
		// 更改后面可写的指针pBufStream和最大长度
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		return	_WriteValueToStream(m_pVar, pBufStream, nSizeLeft, bWriteAll, m_nMaxSize);
	}
	int		_ReadValueFromStream(char *pszStr, int nMaxSize, char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		// 上层应该已经读出了指令和NameID部分
		// 下面读出后面的字串部分
		whfile_i_mem	f(pBufStream, nSizeLeft);
		if( f.ReadVStr(pszStr, nMaxSize)<=0 )
		{
			return	-1;
		}
		// 更改后面可写的指针pBufStream和最大长度
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		if( nSizeLeft>=0 )
		{
			if( bSetModify )
			{
				m_nMDStat		= MDSTAT_MODIFIED;	// 这样设置直接就可以把locked的状态清除
			}
			else
			{
				ClearModified();
			}
			return	0;
		}
		return	-1;
	}
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		return	_ReadValueFromStream(m_pVar, m_nMaxSize, pBufStream, nSizeLeft, bSetModify);
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// 调整附着体指针
		wh_adjustaddrbyoffset(m_pHost, nOffset);
		// 调整字串指针
		wh_adjustaddrbyoffset(m_pVar, nOffset);
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_charptrtype_prop_t(NULLCONSTRUECTOR);
	}
	void *	GetValueBuf()
	{
		return	m_pVar;
	}
};
// 使用时按照普通字串，但是在和流交互的时候需要变成UTF8的
template<class _AllocationObj>
class	whdataobj_charptrtype_utf8based_prop_t	: public whdataobj_charptrtype_prop_t<_AllocationObj>
{
	typedef	whdataobj_charptrtype_prop_t<_AllocationObj>	FATHERCLASS;
protected:
	typedef	whallocmgr2<_AllocationObj>			AM_T;
	typename AM_T::template vector<char>					m_vectUTF8;
public:
	whdataobj_charptrtype_utf8based_prop_t()
		: FATHERCLASS()
	{
	}
	whdataobj_charptrtype_utf8based_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
		, m_vectUTF8(nc)
	{
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, char *pVar, size_t nMaxSize, WHDATAOBJ_NAMEID_T nNameID)
	{
		FATHERCLASS::SetParam(pHost, pVar, nMaxSize, nNameID);
		m_vectUTF8.SetMgr(pHost->m_vectNameID2PropPtr.GetMgr());
		m_vectUTF8.reserve(nMaxSize*2);	// 因为单字节永远不会变成4字节的，只有双字节的字符才可能变成4字节的
		Refresh2UTF8();
	}
	void	Refresh2UTF8()
	{
		wh_char2utf8(whdataobj_charptrtype_prop_t<_AllocationObj>::m_pVar, strlen(whdataobj_charptrtype_prop_t<_AllocationObj>::m_pVar), m_vectUTF8.getbuf(), m_vectUTF8.capacity()-1);
		m_vectUTF8.resize(strlen(m_vectUTF8.getbuf()));
	}
	void	Refresh2Char()
	{
		wh_utf82char(m_vectUTF8.getbuf(), m_vectUTF8.size(), whdataobj_charptrtype_prop_t<_AllocationObj>::m_pVar, whdataobj_charptrtype_prop_t<_AllocationObj>::m_nMaxSize-1);
	}
	inline const char *	getUTF8()
	{
		return	m_vectUTF8.getbuf();
	}
	inline void	set(const char *pStr)
	{
		FATHERCLASS::set(pStr);
		// 更新UTF8的字串（使用内部的默认字符）
		Refresh2UTF8();
	}
	inline void	setUTF8(const char *pStr)
	{
		size_t	nLen	= strlen(pStr);
		if( nLen>=m_vectUTF8.capacity() )
		{
			assert(0);
			return;
		}
		memcpy(m_vectUTF8.getbuf(), pStr, nLen+1);
		m_vectUTF8.resize(nLen);
		FATHERCLASS::set(pStr);
		whdataobj_charptrtype_prop_t<_AllocationObj>::m_nMDStat	= whdataobj_charptrtype_prop_t<_AllocationObj>::MDSTAT_MODIFIED;
		// 更新普通char部分
		Refresh2Char();
	}
	// 这个是按照UTF8来写的
	int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		return	FATHERCLASS::_WriteValueToStream(m_vectUTF8.getbuf(), pBufStream, nSizeLeft, bWriteAll, m_vectUTF8.capacity()-1);
	}
	// 然后按照UTF8来读
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		int	rst	= FATHERCLASS::_ReadValueFromStream(m_vectUTF8.getbuf(), m_vectUTF8.capacity()-1, pBufStream, nSizeLeft, bSetModify);
		if( rst<0 )
		{
			return	rst;
		}
		m_vectUTF8.resize(strlen(m_vectUTF8.getbuf()));
		// 设置一下普通Char的的部分
		Refresh2Char();
		return	0;
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// 调整父类的（里面也会调整m_pHost指针的）
		FATHERCLASS::AdjustInnerPtr(pMgr, nOffset);
		// 调整自己的
		m_vectUTF8.AdjustInnerPtr(pMgr, nOffset);
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_charptrtype_utf8based_prop_t(NULLCONSTRUECTOR);
	}
};

// 列表类型
// 注意：_Ty中必须有一个_IDTy类型的ID，并需要有GetID和SetID方法；
// 还必须有GetStatus、GetStatusLow和SetStatus方法，设置的status就是上面定义的WHDATAOBJ_LISTTYPE_STATUS_XXX
// 可以成参考TTY_LISTUNIT_BASE_T的定义（tty_common_Base.h）
template<typename _Ty, typename _IDTy, class _AllocationObj>
class	whdataobj_listtype_prop_t	: public whdataobj_prop_t<_AllocationObj>
{
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
public:
	typedef	whallocmgr2<_AllocationObj>								AM_T;
	typedef	typename AM_T::template list<_Ty>						LIST_T;
	typedef	typename LIST_T::iterator								LISTITERATOR_T;
	typedef	typename LIST_T::const_iterator							LISTCONSTITERATOR_T;
	typedef	whhash<_IDTy, LISTITERATOR_T,_AllocationObj>			HASH_T;
	typedef	typename HASH_T::kv_iterator							HASHIT;
protected:
	bool		m_bModified;										// 表示是否被修改
	whdataobjbase_t<_AllocationObj>	*m_pHost;						// 附着母体
	LIST_T		*m_pList;
	LIST_T		**m_ppList;
	int			m_nLogicSize;										// 逻辑上的尺寸
private:
	// 物品ID和真实列表元素iterator的对应列表
	HASH_T		*m_pmapReal;
private:
	inline void	EraseItem(HASHIT &it)
	{
		m_pList->erase(it.getvalue());
		m_pmapReal->erase(it);
	}
public:
	whdataobj_listtype_prop_t()
		: m_bModified(false)
		, m_pHost(NULL)
		, m_pList(NULL), m_ppList(NULL)
		, m_nLogicSize(0)
		, m_pmapReal(NULL)
	{
	}
	whdataobj_listtype_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	virtual ~whdataobj_listtype_prop_t()
	{
		//modified by zhanghaoyang 2011/11/29 如果还没set到这个元素，就抛出异常了，这里会失败，所以加上这个判断，应该不会有任何影响
		if(m_pHost!=NULL)
			GetAM()->Delete(m_pmapReal);
	}
	inline AM_T *	GetAM()
	{
		return	m_pHost->m_vectNameID2PropPtr.GetMgr();
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, LIST_T *&pList, WHDATAOBJ_NAMEID_T nNameID)
	{
		m_pHost					= pHost;
		m_pList					= pList;
		m_ppList				= &pList;
		this->m_nNameID			= nNameID;
		// 生成hash表
		GetAM()->AllocByType(m_pmapReal);
		new (m_pmapReal) HASH_T(0,GetAM()->GetAO());
		// 加入映射表
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
	}
	bool	IsModified() const
	{
		return	m_bModified;
	}
	// 获得列表对象以进行列表浏览(可以获得begin和end)
	inline const LIST_T &	GetList() const
	{
		return	*m_pList;
	}
	inline int	GetLogicSize() const
	{
		return	m_nLogicSize;
	}
	// 查找(只读)
	LISTITERATOR_T	Find(const _IDTy id)
	{
		// 从Real表中查找
		HASHIT	kvit	= m_pmapReal->find(id);
		if( kvit == m_pmapReal->end() )
		{
			return	m_pList->end();
		}
		return	kvit.getvalue();
	}
	// 提供end用于比较Find的结果。不用给begin了。
	inline LISTITERATOR_T	end()
	{
		return	m_pList->end();
	}
	// 添加(返回迭代器，获取对应结构引用进行修改，注意里面的ID部分必须和这里提供的id一致，外界不能再修改了)
	// 如果要修改也需要用这个函数
	LISTITERATOR_T	AddModify(const _IDTy id, bool bSetModify=true)
	{
		// 设置主标志为更改过（因为后面会有return，所以先把在这里设置）
		if( bSetModify )
		{
			whsetbool_true(m_bModified);
		}

		// 查找看该ID是否已经存在
		HASHIT	it	= m_pmapReal->find(id);;
		if( it != m_pmapReal->end() )
		{
			LISTITERATOR_T	&listit	= it.getvalue();
			_Ty	&unit	= (*listit);
			if( bSetModify )
			{
				// 看看原来的状态
				switch( unit.GetStatusLow() )
				{
					case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN:
						// 重新设置为被修改
						unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED);
					break;
					case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED:
						// 这个说明是已经修改过的，因为原来可能被设置为LOCK状态了，所以需要清除掉原来的LOCK状态。
						unit.ClearStatusBit(WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK);
					case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD:
						if( (unit.GetStatus()&WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK) != 0 )
						{
							// 这个说明是上次更新结果之前加入的，后来又有修改了，所以需要清除掉原来的LOCK状态，并直接改为modified。
							unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED);
						}
						else
						{
							// 这个说明增加过还没有过存盘，那就继续保持ADD状态
						}
					break;
					case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL:
						// 刚刚被删除，重新又添加，所以需要被设置为修改
						// 重新设置为被修改
						// 注意：如果一个东西被添加（存盘发送失败）、删除（存盘发送失败）、又重新添加之后，其状态会变成Modified而不是Add，所以在从留中读取Modified和Add指令时处理方式应该一致（就是如果没有就添加，如果有就修改）
						unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED);
						// 逻辑尺寸增加
						m_nLogicSize	++;
					break;
					default:
						// 按理来说不应该出现其他的数值的（如果有说明状态位被别人修改了）
#ifdef	_DEBUG
						if( WHCMN_GET_DBGLVL()>=2 )
						{
							assert(0);
						}
						else
						{
							// 这个是为了保证测试的时候出问题也还能正常启动
							unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED);
						}
#endif
					break;
				}
			}
			else
			{
				// 不管原来是任何状态，都清除修改过的状态
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN);
			}
			return	listit;
		}
		else
		{
			// 在Real表中创建
			it	= m_pmapReal->put(id);
			LISTITERATOR_T	&listit	= it.getvalue();
			// 也在数据列表中创建
			m_pList->push_back(&listit);
			_Ty	&unit	= (*listit);
			if( bSetModify )
			{
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD);
			}
			else
			{
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN);
			}
			// 设置ID
			unit.SetID(id);
			// 逻辑尺寸增加
			m_nLogicSize	++;
			return	listit;
		}
	}
	// 从另外的list中拷贝过来
	int		CopyFrom(whdataobj_listtype_prop_t *pOther)
	{
		for(LISTITERATOR_T it=pOther->m_pList->begin(); it!=pOther->m_pList->end(); ++it)
		{
			_Ty	&unit	= (*it);
			if( unit.GetStatusLow() != WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL )
			{
				LISTITERATOR_T	nit	= AddModify(unit.GetID(), true);
				(*nit).CopyFrom(unit);
			}
		}
		return	0;
	}
	// 删除(返回是否存在)
	bool	Del(const _IDTy id, bool bSetModify=true)
	{
		HASHIT	it	= m_pmapReal->find(id);
		if( m_pmapReal->end() == it )
		{
			// 不存在
			return	false;
		}
		return	Del(it, bSetModify);
	}
	// 根据迭代子删除
	bool	Del(HASHIT	it, bool bSetModify=true)
	{
		_Ty	&unit	= (*it.getvalue());
		switch( unit.GetStatusLow() )
		{
		case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN:
		case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED:
			{
				// 设置为删除状态
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL);
			}
			break;
		case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD:
			{
				if( (unit.GetStatus()&WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK) != 0 )
				{
					// 说明上个添加的指令可能已经被发出了，只是没有收到返回，所以为了保险还是应该发送删除的指令
					unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL);
				}
				else
				{
					// 和上次的添加抵消了，直接删除(两个地方都要删)
					EraseItem(it);
				}
			}
			break;
		case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL:
			{
				// 这个理论上应该是不正常的，上层不该删除已经删除过一次的东西，所以返回错误
				return	false;
			}
			break;
		}
		// 逻辑尺寸减少
		m_nLogicSize	--;
		// 设置为更改过
		if( bSetModify )
		{
			whsetbool_true(m_bModified);
		}
		else
		{
			// 不设置更改就直接删除了
			EraseItem(it);
		}
		return	true;
	}
	void	DelAll()
	{
		HASHIT it	= m_pmapReal->begin(); 
		while( it != m_pmapReal->end() )
		{
			HASHIT tmpit	= it++;
			Del(tmpit, true);
		}
	}
	// 清除所有修改标志
	void	ClearModified()
	{
		// 清除所有单元的更改标志，并删除已经删除的
		HASHIT	it	= m_pmapReal->begin();
		while( it!=m_pmapReal->end() )
		{
			HASHIT	itcur(it++);
			_Ty	&unit		= (*itcur.getvalue());
			if( unit.GetStatusLow() == WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL )
			{
				// 真正删除之
				EraseItem(itcur);
			}
			else
			{
				// 修改状态为普通
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN);
			}
		}
		// 清除主标志
		whsetbool_false(m_bModified);
	}
	void	SetModifiedWithLock()
	{
		for(LISTITERATOR_T it=m_pList->begin(); it!=m_pList->end(); ++it)
		{
			_Ty	&unit	= (*it);
			if( unit.GetStatus() != 0 )
			{
				unit.SetStatusBit(WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK);
			}
		}
	}
	void	ClearModifiedWithLock()
	{
		whsetbool_false(m_bModified);
		HASHIT	it	= m_pmapReal->begin();
		while( it!=m_pmapReal->end() )
		{
			HASHIT	itcur(it++);
			_Ty	&unit		= (*itcur.getvalue());
			if( unit.GetStatus() != 0 )
			{
				if( (unit.GetStatus()&WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK) != 0 )
				{
					if( unit.GetStatusLow() == WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL )
					{
						// 是被删除的，真正删除之
						EraseItem(itcur);
					}
					else
					{
						// 是修改或添加的，所以直接清空状态
						unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN);
					}
				}
				else
				{
					// 说明在lock过程中有新增加的修改，说明整个有修改了
					whsetbool_true(m_bModified);
				}
			}
		}
	}
	void	ClearValue()
	{
		// 清空列表
		m_pmapReal->clear();
		m_pList->clear();
		// 清除主标志
		whsetbool_false(m_bModified);
	}
	int	WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		// 一般来说外界提供的buffer都是够长的
		whfile_i_mem	f(pBufStream, nSizeLeft);
		// 开始
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_LIST_MODIFY_BEGIN;
		f.Write(&cmd);
		f.Write(&this->m_nNameID);
		// 检查所有改动的、添加的、删除的
		for(LISTITERATOR_T it=m_pList->begin(); it!=m_pList->end(); ++it)
		{
			_Ty	&unit	= (*it);
			if( bWriteAll )
			{
				// 因为那边接收WriteAll的数据时会先清空所有数据
				// 但是请注意不能把删除的数据也发过去了
				if( unit.GetStatusLow() == WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL )
				{
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_DEL;
					f.Write(&cmd);
					// 写入物品ID
					_IDTy	nID	= unit.GetID();
					f.Write(&nID);
				}
				else
				{
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_ADD;
					f.Write(&cmd);
					// 写入数据(因为长度是固定的)
					f.Write(&unit);
				}
			}
			else
			{
				// 这里之所以也要取低位是因为可能有LOCK中的元素并没有收到存盘成功的返回，再次存储的时候这些东西仍然需要存储滴。
				switch( unit.GetStatusLow() )
				{
				case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED:
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_MODIFY;
					f.Write(&cmd);
					// 写入数据(因为长度是固定的)
					f.Write(&unit);
					break;
				case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD:
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_ADD;
					f.Write(&cmd);
					// 写入数据(因为长度是固定的)
					f.Write(&unit);
					break;
				case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL:
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_DEL;
					f.Write(&cmd);
					// 写入物品ID
					_IDTy	nID	= unit.GetID();
					f.Write(&nID);
					break;
				}
			}
		}
		// 结束
		cmd	= WHDATAOBJ_CMD_LIST_MODIFY_END;
		f.Write(&cmd);
		// 更改后面可写的指针pBufStream和最大长度
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		// 上层应该已经读出了指令和NameID部分
		whfile_i_mem	f(pBufStream, nSizeLeft);
		bool	bStop	= false;
		while( !bStop && f.GetSizeLeft()>0 )
		{
			WHDATAOBJ_CMD_T	cmd;
			f.Read(&cmd);
			switch(cmd)
			{
				case	WHDATAOBJ_CMD_LIST_MODIFY_END:
					bStop	= true;
				break;
				case	WHDATAOBJ_CMD_LIST_ITEM_MODIFY:
				case	WHDATAOBJ_CMD_LIST_ITEM_ADD:
				{
					// 读入后面的数据
					_Ty	data;
					f.Read(&data);
					// 进行添加
					LISTITERATOR_T	it	= AddModify(data.GetID(), bSetModify);
					if( it != m_pList->end() )
					{
						(*it).CopyFrom(data);	// 列表的数据单元结构必须实现CopyFrom方法
					}
				}
				break;
				case	WHDATAOBJ_CMD_LIST_ITEM_DEL:
				{
					// 读入ID
					_IDTy	nID;
					f.Read(&nID);
					// 删除
					Del(nID, bSetModify);
				}
				break;
			}
		}
		pBufStream	= f.GetCurBufPtr();
		nSizeLeft	= f.GetSizeLeft();
		if( nSizeLeft<0 )
		{
			return	-1;
		}

		return	0;
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// 调整附着体指针
		wh_adjustaddrbyoffset(m_pHost, nOffset);
		// 按照list来调整
		pMgr->AdjustPtrAndInner(m_pList, nOffset);
		// 然后把原来变量也改变值
		wh_adjustaddrbyoffset(m_ppList, nOffset);
		*m_ppList	= m_pList;
		// hash表也需要调整
		wh_adjustaddrbyoffset(m_pmapReal, nOffset);
		m_pmapReal->AdjustInnerPtr(nOffset);
		// 每个hash值也要调整，因为他们有内存指针
		for(HASHIT it = m_pmapReal->begin(); it != m_pmapReal->end(); ++it)
		{
			wh_adjustaddrbyoffset(it.getvalue().pNode, nOffset);
		}
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_listtype_prop_t(NULLCONSTRUECTOR);
	}
	void *	GetValueBuf()
	{
		// list不能用这个方法
		return	NULL;
	}
};

// 数组类型
template<typename _Ty, class _AllocationObj>
class	whdataobj_arraytype_prop_t	:	public whdataobj_prop_t<_AllocationObj>
{
protected:
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
	bool	m_bModified;			// 表示是否被修改
	whdataobjbase_t<_AllocationObj>	*m_pHost;		// 附着母体
	_Ty		*m_pArray;				// 数组头指针
	int		m_nArraySize;			// 数组长度
	enum
	{
		MDSTAT_LOCKED	= 0x80,		// 表示被设置锁定了
		MDSTAT_MODIFIED	= 0x01,		// 表示被修改了
	};
	unsigned char	*m_pnStatus;	// 对应Array每个元素的状态标志
	// 设置某个位置被修改
	inline void	SetArrayUnitModified(int nIdx)
	{
		m_pnStatus[nIdx]		= MDSTAT_MODIFIED;
	}
	// 设置连续位置被修改
	inline void	SetArrayUnitModified(int nIdx, int nSpan)
	{
		for(int i=0;i<nSpan;i++)
		{
			m_pnStatus[nIdx+i]	= MDSTAT_MODIFIED;
		}
	}
	// 清除某个位置的修改标记
	inline void	ClrArrayUnitModified(int nIdx)
	{
		m_pnStatus[nIdx]		= 0;
	}
	// 连续清除某个位置的修改标记
	inline void	ClrArrayUnitModified(int nIdx, int nSpan)
	{
		for(int i=0;i<nSpan;i++)
		{
			m_pnStatus[nIdx+i]	= 0;
		}
	}
public:
	typedef	whallocmgr2<_AllocationObj>			AM_T;
	whdataobj_arraytype_prop_t()
		: m_bModified(false)
		, m_pHost(NULL)
		, m_pArray(NULL)
		, m_nArraySize(0)
		, m_pnStatus(NULL)
	{
	}
	whdataobj_arraytype_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	~whdataobj_arraytype_prop_t()
	{
		if( m_pnStatus )
		{
			m_pHost->m_vectNameID2PropPtr.GetMgr()->Free(m_pnStatus);
			m_pnStatus			= NULL;
		}
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, _Ty *pArray, int nArraySize, WHDATAOBJ_NAMEID_T nNameID)
	{
		assert(nArraySize<0x10000);
		m_pHost					= pHost;
		m_pArray				= pArray;
		m_nArraySize			= nArraySize;
		this->m_nNameID				= nNameID;
		// 加入映射表
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
		// 申请对应的标记位
		if( m_pnStatus )
		{
			// 按理来说是不应该已经有的，但是不排除某些时候用一个对象重复设置关联基础数据的情况发生
			m_pHost->m_vectNameID2PropPtr.GetMgr()->Free(m_pnStatus);
		}
		m_pnStatus				= (unsigned char *)m_pHost->m_vectNameID2PropPtr.GetMgr()->Alloc(sizeof(unsigned char)*nArraySize);
		assert(m_pnStatus);
		memset(m_pnStatus, 0, nArraySize);
	}
	inline const _Ty *	getarray() const		// 获得数组指针开头，如果要大量读取可以用这个方法
	{
		return	m_pArray;
	}
	inline const _Ty &	get(int nIdx) const		// 获得具体数组成员(如果超界上层负责)
	{
		return	m_pArray[nIdx];
	}
	inline int	size() const					// 获得数组长度
	{
		return	m_nArraySize;
	}
	inline void	set(int nIdx, const _Ty &val)
	{
		m_pArray[nIdx]		= val;
		SetArrayUnitModified(nIdx);
		whsetbool_true(m_bModified);
	}
	// nIdx是起始下标
	// nSpan是连续元素的个数
	_Ty *	setbyptr(int nIdx, int nSpan)
	{
#ifdef	_DEBUG
		assert(nIdx>=0 && nIdx+nSpan<=m_nArraySize);
#endif
		SetArrayUnitModified(nIdx, nSpan);
		whsetbool_true(m_bModified);
		return	m_pArray + nIdx;
	}
	// 设置一个元素
	_Ty *	setbyptr(int nIdx)
	{
		SetArrayUnitModified(nIdx);
		whsetbool_true(m_bModified);
		return	m_pArray + nIdx;
	}
	// 自动把0的部分置为非改写的状态
	void	SetZeroNotModified()
	{
		bool	bModified	= false;
		for(int i=0;i<m_nArraySize;i++)
		{
			if( m_pnStatus[i] )
			{
				if( m_pArray[i]==0 )
				{
					m_pnStatus[i]	= 0;
				}
				else
				{
					bModified	= true;
				}
			}
		}
		if( bModified )
		{
			whsetbool_true(m_bModified);
		}
		else
		{
			whsetbool_false(m_bModified);
		}
	}
	// 从另外的list中拷贝过来
	int		CopyFrom(whdataobj_arraytype_prop_t *pOther)
	{
		for(int i=0;i<pOther->m_nArraySize;i++)
		{
			if( pOther->get(i) != 0 )
			{
				set(i, pOther->get(i));
			}
		}
		return	0;
	}
	bool	IsModified() const
	{
		return	m_bModified;
	}
	void	ClearModified()
	{
		whsetbool_false(m_bModified);
		memset(m_pnStatus, 0, WHSIZEOFARRAY(m_pnStatus,m_nArraySize));
	}
	void	SetModifiedWithLock()
	{
		// 把所有修改过的设置LOCK标志
		for(int i=0;i<m_nArraySize;i++)
		{
			if( m_pnStatus[i] != 0 )
			{
				m_pnStatus[i]	|= MDSTAT_LOCKED;
			}
		}
	}
	void	ClearModifiedWithLock()
	{
		whsetbool_false(m_bModified);
		// 把所有有LOCK标志的Modified状态清除
		for(int i=0;i<m_nArraySize;i++)
		{
			if( m_pnStatus[i] != 0 )
			{
				if( (m_pnStatus[i]&MDSTAT_LOCKED) != 0 )
				{
					// 清除旧状态
					m_pnStatus[i]	= 0;
				}
				else
				{
					// 说明有新修改的
					whsetbool_true(m_bModified);
				}
			}
		}
	}
	void	ClearValue()
	{
		memset(m_pArray, 0, WHSIZEOFARRAY(m_pArray,m_nArraySize));
		ClearModified();
	}
	int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		// 一般来说外界提供的buffer都是够长的
		whfile_i_mem	f(pBufStream, nSizeLeft);
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_ARR_MODIFY_BEGIN;
		f.Write(&cmd);
		f.Write(&this->m_nNameID);
		if( bWriteAll )
		{
			// 从0开始连续m_nArraySize个被输出
			WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_ARR_ITEM_MODIFYN;
			f.Write(&cmd);
			// 起始偏移(相对于-1位置的)
			f.WriteVSize(1);
			// 数组长度
			f.WriteVSize(m_nArraySize);
			// 数据
			f.Write(m_pArray, WHSIZEOFARRAY(m_pArray, m_nArraySize));
		}
		else
		{
			// 开始
			// 检查所有被更改的
			int		nLastIdx	= -1;
			int		i			= 0;
			bool	bSearchModi	= false;
			while(i<(int)m_nArraySize)
			{
				int	nCount	= 0;
				if( bSearchModi )
				{
					// 查找连续的被修改的块
					{
						int	j	= i;
						while( j<m_nArraySize && m_pnStatus[j]!=0 )
						{
							++nCount;
							++j;
						}
					}
					if( nCount==0 )
					{
						// 在有i<m_nArraySize的限制下不可能出现这样的情况的
						assert(0);
						break;
					}
					int	nDiff	= i - nLastIdx;
					if( nCount == 1 )
					{
						WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_ARR_ITEM_MODIFY1;
						f.Write(&cmd);
						f.WriteVSize(nDiff);
						f.Write(&m_pArray[i]);
					}
					else
					{
						WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_ARR_ITEM_MODIFYN;
						f.Write(&cmd);
						f.WriteVSize(nDiff);
						f.WriteVSize(nCount);
						f.Write(&m_pArray[i], WHSIZEOFARRAY(m_pArray, nCount));
					}
					i			+= nCount;
					nLastIdx	= i;
					bSearchModi	= false;
				}
				else
				{
					// 查找连续的没被修改的块
					{
						int	j	= i;
						while( j<m_nArraySize && m_pnStatus[j]==0 )
						{
							++nCount;
							++j;
						}
					}
					// 后续
					i			+= nCount;
					bSearchModi	= true;
				}
			}
		}
		// 结束
		cmd	= WHDATAOBJ_CMD_ARR_MODIFY_END;
		f.Write(&cmd);
		// 更改后面可写的指针pBufStream和最大长度
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		// 上层应该已经读出了指令和NameID部分
		whfile_i_mem	f(pBufStream, nSizeLeft);
		bool	bStop	= false;
		int		nIdx	= -1;
		while( !bStop && f.GetSizeLeft()>0 )
		{
			WHDATAOBJ_CMD_T	cmd;
			f.Read(&cmd);
			switch(cmd)
			{
				case	WHDATAOBJ_CMD_ARR_MODIFY_END:
					bStop	= true;
				break;
				case	WHDATAOBJ_CMD_ARR_ITEM_MODIFY1:
				{
					// 读出偏移
					int	nDiff	= f.ReadVSize();
					if( nDiff<=0 )
					{
						return	-1;
					}
					nIdx	+= nDiff;
					if( nIdx>=m_nArraySize )
					{
						return	-2;
					}
					// 读出数据
					f.Read(&m_pArray[nIdx]);
					// 设置修改
					if( bSetModify )
					{
						SetArrayUnitModified(nIdx);
					}
					else
					{
						ClrArrayUnitModified(nIdx);
					}
					// 下一个起始位置
					++nIdx;
				}
				break;
				case	WHDATAOBJ_CMD_ARR_ITEM_MODIFYN:
				{
					// 读出偏移
					int	nDiff	= f.ReadVSize();
					if( nDiff<=0 )
					{
						return	-11;
					}
					nIdx	+= nDiff;
					if( nIdx>=m_nArraySize )
					{
						return	-12;
					}
					// 读出个数
					int	nCount	= f.ReadVSize();
					if( nCount<=0 )
					{
						return	-13;
					}
					// 读出数据
					assert(nIdx+nCount<=m_nArraySize);
					f.Read(&m_pArray[nIdx], WHSIZEOFARRAY(m_pArray, nCount));
					// 设置修改
					if( bSetModify )
					{
						SetArrayUnitModified(nIdx, nCount);
					}
					else
					{
						ClrArrayUnitModified(nIdx, nCount);
					}
					// 下一个起始位置
					nIdx	+= nCount;
				}
				break;
			}
		}
		// 更改后面可写的指针pBufStream和最大长度
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		if( nSizeLeft>=0 )
		{
			if( bSetModify )
			{
				whsetbool_true(m_bModified);
			}
			return	0;
		}
		return	-1;
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// 调整附着体指针
		wh_adjustaddrbyoffset(m_pHost, nOffset);
		// 数组指针
		wh_adjustaddrbyoffset(m_pArray, nOffset);
		wh_adjustaddrbyoffset(m_pnStatus, nOffset);
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_arraytype_prop_t(NULLCONSTRUECTOR);
	}
	void *	GetValueBuf()
	{
		return	m_pArray;
	}
};

}		// EOF namespace n_whcmn

// 和设置关联相关的（注意：设置str可以直接用array的方式）
#define	AUTO_DATA_SetParam( a,b )		a.SetParam(this, &pBase->a, b)
#define	AUTO_DATA_SetParam_smp(a)		AUTO_DATA_SetParam(a,enum_##a)
#define	AUTO_DATA_SetParamPtr( ptr,b )		ptr.SetParam(this, pBase->ptr, b)
#define	AUTO_DATA_SetParamPtr_smp( ptr )	AUTO_DATA_SetParamPtr(ptr, enum_##ptr)
#define	AUTO_DATA_SetParamArray( a,b )	a.SetParam(this, pBase->a,WHNUMOFARRAYUNIT(pBase->a), b)
#define	AUTO_DATA_SetParamArray_smp(a)	AUTO_DATA_SetParamArray(a,enum_##a)
#define	AUTO_DATA_SetMarker( a,b )		a.SetParam(this, b)
#define	AUTO_DATA_SetMarker_smp( a )	AUTO_DATA_SetMarker(a,enum_##a)

// 和打印元素相关的（之前使用者应该定义rst，并且置初值为0）
// 这个主要是用来给一个缓冲区连续打印字串用的
#define	AUTO_DATA_SMPPRINT	assert(rst>=0);pszBuf+=rst;nSizeLeft-=rst;assert(nSizeLeft>=0);rst=snprintf
#define	AUTO_DATA_DispCmn( t, a )		AUTO_DATA_SMPPRINT( pszBuf, nSizeLeft, "%s:"t"%s",#a,a.get(), WHLINEEND)
#define	AUTO_DATA_DispInt( a )			AUTO_DATA_DispCmn("%d", a)
#define	AUTO_DATA_DispInt64( a )		AUTO_DATA_DispCmn("0x%"WHINT64PRFX"X", a)
#define	AUTO_DATA_DispTime( a )			AUTO_DATA_SMPPRINT( pszBuf, nSizeLeft, "%s:%lu%s",#a,a.get(), WHLINEEND)
#define	AUTO_DATA_DispFloat( a )		AUTO_DATA_SMPPRINT( pszBuf, nSizeLeft, "%s:%.4f%s",#a,a.get(), WHLINEEND)
#define	AUTO_DATA_DispStr( a )			AUTO_DATA_SMPPRINT( pszBuf, nSizeLeft, "%s:%s%s",#a,a.get(), WHLINEEND)
#define	AUTO_DATA_DispArray0( t, a )	AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, "%s ",#a );{for(int i=0;i<a.size();++i) {if( a.get(i)!=0 ) {AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, "[%d]."t" ", i, a.get(i) );}}}AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, WHLINEEND )
#define	AUTO_DATA_DispArray( a )		AUTO_DATA_DispArray0("%d", a)
#define	AUTO_DATA_DispStrArray( a )		AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, "%s ",#a );{for(int i=0;i<a.size();++i) {if( a.get(i)[0]!=0 ) {AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, "[%d].%s%s", i, (const char *)a.get(i), WHLINEEND);}}}AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, WHLINEEND )

#endif
