// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whchunk.h
// Creator      : Wei Hua (魏华)
// Comment      : 多个包含单位的大块内存分配，可以通过需要访问各个单元
//                这样分配的最大好处是开始分配的指针一直有效而不象vector那样如果resize可能会发生变化
//                2004-01-17 注意，由于chunk里面用到了list和vector所以不好使用allocator，就暂时不使用了。
//                对象数组是new出来的，所以可以放心使用
// CreationDate : 2004-01-16
// ChangeLog    :
//                2004-01-17 添加了分配器，这样它将来就可能使用共享内存了
//                2005-09-12 增加了sequencealloc方法用于分配连续的多个单元
//                2006-06-27 把delete改为delete[]了，据说用new[]分配用delete删除是未定义的行为（如果将来编译器实现方式改了就会有问题）

#ifndef	__WHCHUNK_H__
#define	__WHCHUNK_H__

#include "./whvector.h"
#include <assert.h>

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 模板类型的
////////////////////////////////////////////////////////////////////
// 这个管分配和访问
template<typename _Ty, class _AllocationObj=whcmnallocationobj>
class	whchunk
{
protected:
	// 用于存放各个chunk的指针
	whvector<_Ty *, false, _AllocationObj>	m_ptrBuf;
	// 每个chunk中的单元数量
	size_t			m_nChunkSize;
public:
	// 注意开始申请后就不能改变了
	inline void	setChunkSize(size_t nSize)
	{
		if( nSize==0 )
		{
			assert(0);
			return;
		}
		m_nChunkSize	= nSize;
	}
public:
	whchunk()
	: m_nChunkSize(100)
	{
	}
	whchunk(int nChunkSize)
	: m_nChunkSize(nChunkSize)
	{
	}
	~whchunk()
	{
		// 为了保险再删除一次
		destroy();
	}
	void	destroy()
	{
		if( m_ptrBuf.size()>0 )
		{
			// 删除所有chunk
			for(size_t i=0;i<m_ptrBuf.size();i++)
			{
				m_ptrBuf.GetAND().DeleteArray(m_ptrBuf[i], m_nChunkSize);
			}
			m_ptrBuf.clear();
		}
	}
	// 设置分配对象
	inline void	SetAO(_AllocationObj *pAO)
	{
		m_ptrBuf.SeAO(pAO);
	}
	// 创建nNum个新的chunk
	void	createchunk(int nNum=1)
	{
		int	i;
		for(i=0;i<nNum;i++)
		{
			// 各个对象的初始化应该再New中调用了
			_Ty *	pChunk = m_ptrBuf.GetAND().NewArray((_Ty *)0, m_nChunkSize);
			assert(pChunk);
			m_ptrBuf.push_back(pChunk);
		}
	}
	// 获得目前可以使用的总单元数量
	inline size_t	gettotalnum() const
	{
		return	m_nChunkSize * m_ptrBuf.size();
	}
	// 根据需要访问一个单元
	inline _Ty & operator[] (int nIdx) const
	{
		return	getunit(nIdx);
	}
	inline _Ty &	getunit(int nIdx) const
	{
		// 这里就不作越界判断了，上层要保证
		return	m_ptrBuf[nIdx/m_nChunkSize][nIdx%m_nChunkSize];
	}
	inline _Ty *	getunitptr(int nIdx) const
	{
		int	nTotalUnit = gettotalnum();
		if( nIdx<0 || nIdx>=nTotalUnit )
		{
			return	NULL;
		}
		return	&m_ptrBuf[nIdx/m_nChunkSize][nIdx%m_nChunkSize];
	}
};

// 这个就只管循序分配
template<typename _Ty, class _AllocationObj=whcmnallocationobj>
class	whallocbychunk	: public whchunk<_Ty, _AllocationObj>
{
private:
	typedef	whchunk<_Ty, _AllocationObj>	FATHERCLASS;
protected:
	// 已经分配的数量
	size_t	m_nAllocNum;
public:
	whallocbychunk()
	: FATHERCLASS()
	{
		clear0();
	}
	whallocbychunk(int nChunkSize)
	: FATHERCLASS(nChunkSize)
	{
		clear0();
	}
	inline void	clear0()
	{
		m_nAllocNum	= 0;
	}
	inline void	clear()					// 这样是为了别人好调用(习惯)
	{
		clear0();
	}
	void	destroy()
	{
		FATHERCLASS::destroy();
		clear0();
	}
	// 申请一个对象
	_Ty *	alloc()
	{
		if( m_nAllocNum>=FATHERCLASS::gettotalnum() )
		{
			// 分配新的chunk
			FATHERCLASS::createchunk();
		}
		return	FATHERCLASS::getunitptr(m_nAllocNum++);
	}
	// 申请一堆连续的对象(如果地方不够则create新的chunk)
	// 这样的申请是不应该对应释放的。主要用于连续性的初始化(如大量的常量字串)
	_Ty *	sequencealloc(size_t nNum)
	{
		if( nNum>FATHERCLASS::m_nChunkSize )
		{
			assert(0);			// 申请的长度超过了每隔chunk的大小
			return	NULL;
		}
		size_t	nCurTotal	= FATHERCLASS::gettotalnum();
		if( m_nAllocNum+nNum > nCurTotal )
		{
			// 浪费一些内容
			m_nAllocNum		= nCurTotal;
			// 分配新的chunk
			FATHERCLASS::createchunk();
		}
		size_t	nOffset	= m_nAllocNum;
		m_nAllocNum	+= nNum;
		return	FATHERCLASS::getunitptr(nOffset);
	}
	// 连续申请并拷贝
	_Ty *	sequenceallocandcopy(const _Ty *pData, size_t nNum)
	{
		_Ty	*pDst	= sequencealloc(nNum);
		if( !pDst )
		{
			assert(0);
			return	NULL;
		}
		memcpy(pDst, pData, nNum*sizeof(_Ty));
		return	pDst;
	}
	inline size_t	size() const
	{
		return	m_nAllocNum;
	}
};

////////////////////////////////////////////////////////////////////
// 非模板类型的
////////////////////////////////////////////////////////////////////
// 不管类型，只关心长度的chunk分配
class	whsmpchunk
{
public:
	// 定义
	struct	INFO_T
	{
		// 每个单元的长度
		int				nUnitSize;
		// 每个chunk中的单元数量
		int				nChunkSize;
		INFO_T()
		: nUnitSize(0)
		, nChunkSize(0)
		{
		}
	};
protected:
	// 用于存放各个chunk的指针
	whvector<char *>	m_ptrBuf;
	// 总单元数
	size_t				m_nTotalMaxUnit;
public:
	// 初始化信息(外界在开始分配之前可以随便改)
	// !!!!上层必须保证在使用前设置了m_info的内容!!!!
	INFO_T				m_info;
public:
	whsmpchunk()
	: m_nTotalMaxUnit(0)
	{
	}
	~whsmpchunk()
	{
		// 为了保险再删除一次
		destroy();
	}
	void	destroy()
	{
		if( m_ptrBuf.size()>0 )
		{
			// 删除所有chunk
			for(size_t i=0;i<m_ptrBuf.size();i++)
			{
				delete []	m_ptrBuf[i];
			}
			m_ptrBuf.clear();
		}
		m_nTotalMaxUnit	= 0;
	}
	// 创建nNum个新的chunk
	void	createchunk(int nNum=1)
	{
		// 上层必须保证设置了m_info的内容
		assert(m_info.nChunkSize>0 && m_info.nUnitSize);
		int	i;
		for(i=0;i<nNum;i++)
		{
			// 各个对象的初始化应该再New中调用了
			char *	pChunk = new char[m_info.nChunkSize*m_info.nUnitSize];
			assert(pChunk);
			m_ptrBuf.push_back(pChunk);
		}
		m_nTotalMaxUnit	= m_info.nChunkSize * m_ptrBuf.size();
	}
	// 获得目前可以使用的总单元数量
	inline size_t	gettotalnum() const
	{
		return	m_nTotalMaxUnit; 
	}
	// 根据需要访问一个单元
	inline char * operator[] (int nIdx) const
	{
		return	getunitptr(nIdx);
	}
	inline char *	getunitptr(int nIdx) const
	{
		// 这里就不作越界判断了，上层要保证
		return	m_ptrBuf[nIdx/m_info.nChunkSize] + nIdx%m_info.nChunkSize * m_info.nUnitSize;
	}
};

// 这个就只管循序分配
class	whallocbysmpchunk	: public whsmpchunk
{
protected:
	// 已经分配的数量
	size_t	m_nAllocNum;
public:
	whallocbysmpchunk()
	{
		clear();
	}
	void	clear()
	{
		m_nAllocNum	= 0;
	}
	void	destroy()
	{
		whsmpchunk::destroy();
		clear();
	}
	inline char *	alloc()
	{
		if( m_nAllocNum>=gettotalnum() )
		{
			// 分配新的chunk
			createchunk();
		}
		return	getunitptr(m_nAllocNum++);
	}
	inline size_t	size() const
	{
		return	m_nAllocNum;
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHCHUNK_H__
