// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whhash2.h
// Creator      : Wei Hua (魏华)
// Comment      : hash表的一些公共定义
//                参考了cui和stlport的实现
//                不过这里面不会自动调用类的构造函数，因为想避免delete重载后的嵌套(虽然不会出错)
//                注意：即使不使用，hashtable还是会占用一定的初始内存
//                节点的分配采用了whallocbychunk，所以value如果被分配则地址是不会变的。
// Function list: _whcmn_hashfunc	普通的hash仿函数
//
// CreationDate : 2007-09-07 从whhash.h中转移过来
// ChangeLog    :

#ifndef	__WHHASH2E_H__
#define	__WHHASH2E_H__

#include "whhash_base.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 简单的小规模双级检测的hashtable实现
// 实现方法是先进行一级对应，如果发生碰撞则计算二级hash值，如果还发生碰撞则依次向后检测
// 严重注意：这个东西最好不要有删除操作，因为删除会造成空洞，需要重新扫描整个表来填补空洞。!!!!
////////////////////////////////////////////////////////////////////
template <typename _Key, typename _Val, class _HashFunc = _whcmn_hashfunc>
class	whhash2
{
public:
	struct	UNIT_T
	{
		friend	class	whhash2;
	private:
		// 因为iterator会给外界看到UNIT_T结构，所以key是不能随便改的
		_Key	key;							// 键值
		_Val	val;							// 对应值
		bool	bTaken;							// 表示这个元素是否被使用（2007-07-11把它移动到最后面，这样可能有助于对齐的几率大一些）
	public:
		void	reset()							// 在alloc之后需要调用一下这个
		{
			bTaken	= false;
		}
		inline bool	IsTaken() const
		{
			return	bTaken;
		}
		inline const _Key &	GetKey() const
		{
			return	key;
		}
		inline _Val &	GetVal()				// 2007-09-17 把这个改成非const的，这样外界可以比较方便的修改值
		{
			return	val;
		}
		inline void	SetVal(const _Val &_val)
		{
			val	= _val;
		}
	};
private:
	_HashFunc	_hash;
	bool		m_b_pUnit_Should_Be_Freed;		// m_pUnit是内部的申请的，需要Free
	UNIT_T		*m_pUnit;						// 记录所有元素
	int			m_nTotal;						// 所有元素的个数
	int			m_STEP1;
	int			m_STEP2;
	int			m_MAX;
public:
	class	iterator
	{
	private:
		friend	class whhash2;
		UNIT_T	*m_pUnit;						// 所对应的元素指针
		UNIT_T	*m_pEnd;						// 表示结束的元素指针
	public:
		iterator()
		: m_pUnit(NULL), m_pEnd(NULL)
		{
		}
		iterator(UNIT_T *pUnit, UNIT_T *pEnd)
		: m_pUnit(pUnit), m_pEnd(pEnd)
		{
		}
		iterator(const iterator &_other)
		: m_pUnit(_other.m_pUnit), m_pEnd(_other.m_pEnd)
		{
		}
	public:
		inline bool operator == (const iterator & it) const
		{
			return	m_pUnit == it.m_pUnit;
		}
		inline bool operator != (const iterator & it) const
		{
			return	! operator == (it);
		}
		// 不写这个会报no postfix form of 'operator ++' found for type 'key_iterator'
		// 其实这个就是 x++，而下面就是++x。
		inline iterator		operator ++ (int)
		{
			iterator	old(*this);
			operator ++ ();
			return		old;
		}
		inline iterator &	operator ++ ()
		{
			while( m_pUnit != m_pEnd )
			{
				++m_pUnit;
				if( m_pUnit->IsTaken() )
				{
					break;
				}
			}
			return	*this;
		}
		inline UNIT_T & operator * ()
		{
			return	*m_pUnit;
		}
	};
	whhash2()
	: m_b_pUnit_Should_Be_Freed(false), m_pUnit(NULL)
	, m_nTotal(0)
	, m_STEP1(0), m_STEP2(0), m_MAX(0)
	{
	}
	~whhash2()
	{
		Release();
	}
	// _STEP1是一次范围、_STEP2是二次范围，_STEP1>_STEP2（这是必须的）。他们的取值见g_prime_list表。
	// _MAX是最多可以加入的元素个数，这个最好不要超过_STEP1/2（除非上层可以肯定所有数字都连续）
	int	Init(int _STEP1, int _STEP2, int _MAX)
	{
		if( _STEP1<=_STEP2 )
		{
			assert(0);
			return	-1;
		}
		// 2006-02-17 取消了_MAX必须不要超过_STEP1/2的判断
		// 2006-02-20 增加了_MAX必须不能超过_STEP1的盘算
		if( _MAX>=_STEP1 )
		{
			assert(0);
			return	-2;
		}
		m_nTotal= 0;
		m_STEP1	= _STEP1;
		m_STEP2	= _STEP2;
		m_MAX	= _MAX;
		m_pUnit	= new UNIT_T[m_STEP1];
		for(int i=0;i<m_STEP1;i++)
		{
			m_pUnit[i].reset();
		}
		m_b_pUnit_Should_Be_Freed	= true;
		return	0;
	}
	inline int	GetUnitMemSize(int nNum)
	{
		return	nNum*sizeof(UNIT_T);
	}
	int	Init(void *pUnit, int _STEP1, int _STEP2, int _MAX)	// 给一段已有的内存（pUnit的内部长度必须为GetUnitMemSize(_STEP1)，上层需要在最后自行释放pUnit的内存）
	{
		if( _STEP1<=_STEP2 )
		{
			assert(0);
			return	-1;
		}
		// 2006-02-17 取消了_MAX必须不要超过_STEP1/2的判断
		// 2006-02-20 增加了_MAX必须不能超过_STEP1的盘算
		if( _MAX>=_STEP1 )
		{
			assert(0);
			return	-2;
		}
		m_nTotal= 0;
		m_STEP1	= _STEP1;
		m_STEP2	= _STEP2;
		m_MAX	= _MAX;
		m_pUnit	= (UNIT_T *)pUnit;
		for(int i=0;i<m_STEP1;i++)
		{
			m_pUnit[i].reset();
		}
		m_b_pUnit_Should_Be_Freed	= false;
		return	0;
	}
	int	Release()
	{
		if( m_pUnit == NULL )
		{
			// 不需要release了
			return	0;
		}
		if( m_b_pUnit_Should_Be_Freed )
		{
			delete []	m_pUnit;
		}
		m_pUnit		= NULL;
		return	0;
	}
	inline unsigned int	size() const
	{
		return	m_nTotal;
	}
	inline UNIT_T *	getendunit()
	{
		return	m_pUnit+m_STEP1;
	}
	inline iterator	begin()
	{
		for(int	nBegin=0;nBegin<m_STEP1;nBegin++)
		{
			if( m_pUnit[nBegin].bTaken )
			{
				return	iterator(m_pUnit+nBegin, getendunit());
			}
		}
		return		end();
	}
	inline iterator	end()
	{
		return	iterator(getendunit(), getendunit());
	}
	// 如果原来key就存在则无法存入
	// 如果原来有键存在，如果bReplace为真则替换原来的，如果bReplace为假则返回假。
	// pnCount如果非空则返回碰撞的次数（这个数据主要用来在debug版作调试，评估hash的分散程度。因为put操作应该会比较少，所以应该不会对性能造成很大影响）
	iterator	put(const _Key &key, const _Val &val, bool bReplace=false, int *pnCount=NULL)
	{
		if( m_nTotal>=m_MAX )
		{
			assert(0);
			return	end();
		}
		unsigned int	nHash	= _hash(key);
		unsigned int	nHash1	= nHash % m_STEP1;
		#ifdef	_DEBUG
		if( pnCount )	*pnCount = 1;
		#endif
		if( m_pUnit[nHash1].bTaken )
		{
			// 判断是否是自己
			if( m_pUnit[nHash1].key == key )
			{
				if( bReplace )
				{
					m_pUnit[nHash1].val	= val;
					return	iterator(m_pUnit+nHash1, getendunit());;
				}
				else
				{
					// 发生重复
					return	end();
				}
			}
			// 如果不是就向后找
			unsigned int	nHash2	= nHash % m_STEP2;
			if( nHash2==0 )	nHash2 = m_STEP2;
			// 上层可一定要保证
			for(;;)
			{
				#ifdef	_DEBUG
				if( pnCount )	++(*pnCount);
				#endif
				nHash1	+= nHash2;
				if( nHash1>=(unsigned int)m_STEP1 )
				{
					nHash1	-= m_STEP1;
				}
				if( !m_pUnit[nHash1].bTaken )
				{
					// 找到空位置了
					break;
				}
				// 判断是否是自己
				if( m_pUnit[nHash1].key == key )
				{
					if( bReplace )
					{
						m_pUnit[nHash1].val	= val;
						return	iterator(m_pUnit+nHash1, getendunit());;
					}
					else
					{
						return	end();
					}
				}
			}
		}
		else
		{
			// 原来没有，可以存入
		}
		// 如果到了这里就一定是可以插入的位置了
		m_pUnit[nHash1].bTaken	= true;
		m_pUnit[nHash1].key		= key;
		m_pUnit[nHash1].val		= val;
		m_nTotal	++;

		return	iterator(m_pUnit+nHash1, getendunit());;
	}
	// 查找
	iterator	find(const _Key & key)
	{
		unsigned int	nHash	= _hash(key);
		unsigned int	nHash1	= nHash % m_STEP1;
		if( m_pUnit[nHash1].bTaken )
		{
			// 判断是否是自己
			if( m_pUnit[nHash1].key == key )
			{
				return	iterator(m_pUnit+nHash1, getendunit());
			}
			// 如果不是就向后找
			unsigned int	nHash2	= nHash % m_STEP2;
			if( nHash2==0 )	nHash2 = m_STEP2;
			// 上层可一定要保证
			for(;;)
			{
				nHash1	+= nHash2;
				if( nHash1>=(unsigned int)m_STEP1 )
				{
					nHash1	-= m_STEP1;
				}
				if( !m_pUnit[nHash1].bTaken )
				{
					// 到空位置了
					return	end();
				}
				// 判断是否是自己
				if( m_pUnit[nHash1].key == key )
				{
					return	iterator(m_pUnit+nHash1, getendunit());
				}
			}
		}
		else
		{
			// 没有这个记录
			return	end();
		}
	}
	inline bool	has(const _Key & key)
	{
		return	end() != find(key);
	}
	// 删除
	// 移除指定键值的单元
	bool	erase(const _Key &key)
	{
		iterator	it	= find(key);
		if( it == end() )
		{
			return	false;
		}
		return	erase(it);
	}
	// 根据iterator删除（注意，这个不要在遍历过程使用，因为他可能导致所有东西都变了!!!!）
	bool	erase(iterator &it)
	{
		if( it.m_pUnit->bTaken )
		{
			it.m_pUnit->bTaken	= false;
			m_nTotal			--;
			// 扫描整个表重新填补空洞，把非一次命中的都找出来重新插一遍。
			for(int i=0;i<m_STEP1;i++)
			{
				if( m_pUnit[i].bTaken )
				{
					unsigned int	nHash1	= _hash(m_pUnit[i].key) % m_STEP1;
					if( nHash1 == i )
					{
						// 一次命中的就不用管了
					}
					else
					{
						// 非一次命中的
						// 删除，然后重新插入
						m_pUnit[i].bTaken	= false;
						m_nTotal			--;
						put(m_pUnit[i].key, m_pUnit[i].val, false, NULL);
					}
				}
			}
			return	true;
		}
		else
		{
			assert(0);
			return	false;
		}
	}
	// 清空hash表
	void	clear()
	{
		m_nTotal	= 0;
		for(int i=0;i<m_STEP1;i++)
		{
			if( m_pUnit[i].bTaken )
			{
				m_pUnit[i].bTaken	= false;
			}
		}
	}
	// 统计hash表的命中率
	struct	STAT_T
	{
		int	nGood;								// 一次命中的
		int	nBad;								// 非一次命中的
	};
	void	stat(STAT_T *pStat)
	{
		pStat->nGood	= 0;
		pStat->nBad		= 0;
		for(int i=0;i<m_STEP1;i++)
		{
			if( m_pUnit[i].bTaken )
			{
				int	nHash1	= _hash(m_pUnit[i].key) % m_STEP1;
				if( nHash1 == i )
				{
					pStat->nGood	++;
				}
				else
				{
					pStat->nBad		++;
				}
			}
		}
	}
	// 内部指针调整
	void	AdjustInnerPtr(int nOffset)
	{
		wh_adjustaddrbyoffset(m_pUnit, nOffset);
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHHASH2E_H__
