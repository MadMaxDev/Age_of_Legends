// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whhash2.h
// Creator      : Wei Hua (κ��)
// Comment      : hash���һЩ��������
//                �ο���cui��stlport��ʵ��
//                ���������治���Զ�������Ĺ��캯������Ϊ�����delete���غ��Ƕ��(��Ȼ�������)
//                ע�⣺��ʹ��ʹ�ã�hashtable���ǻ�ռ��һ���ĳ�ʼ�ڴ�
//                �ڵ�ķ��������whallocbychunk������value������������ַ�ǲ����ġ�
// Function list: _whcmn_hashfunc	��ͨ��hash�º���
//
// CreationDate : 2007-09-07 ��whhash.h��ת�ƹ���
// ChangeLog    :

#ifndef	__WHHASH2E_H__
#define	__WHHASH2E_H__

#include "whhash_base.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// �򵥵�С��ģ˫������hashtableʵ��
// ʵ�ַ������Ƚ���һ����Ӧ�����������ײ��������hashֵ�������������ײ�����������
// ����ע�⣺���������ò�Ҫ��ɾ����������Ϊɾ������ɿն�����Ҫ����ɨ������������ն���!!!!
////////////////////////////////////////////////////////////////////
template <typename _Key, typename _Val, class _HashFunc = _whcmn_hashfunc>
class	whhash2
{
public:
	struct	UNIT_T
	{
		friend	class	whhash2;
	private:
		// ��Ϊiterator�����翴��UNIT_T�ṹ������key�ǲ������ĵ�
		_Key	key;							// ��ֵ
		_Val	val;							// ��Ӧֵ
		bool	bTaken;							// ��ʾ���Ԫ���Ƿ�ʹ�ã�2007-07-11�����ƶ�������棬�������������ڶ���ļ��ʴ�һЩ��
	public:
		void	reset()							// ��alloc֮����Ҫ����һ�����
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
		inline _Val &	GetVal()				// 2007-09-17 ������ĳɷ�const�ģ����������ԱȽϷ�����޸�ֵ
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
	bool		m_b_pUnit_Should_Be_Freed;		// m_pUnit���ڲ�������ģ���ҪFree
	UNIT_T		*m_pUnit;						// ��¼����Ԫ��
	int			m_nTotal;						// ����Ԫ�صĸ���
	int			m_STEP1;
	int			m_STEP2;
	int			m_MAX;
public:
	class	iterator
	{
	private:
		friend	class whhash2;
		UNIT_T	*m_pUnit;						// ����Ӧ��Ԫ��ָ��
		UNIT_T	*m_pEnd;						// ��ʾ������Ԫ��ָ��
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
		// ��д����ᱨno postfix form of 'operator ++' found for type 'key_iterator'
		// ��ʵ������� x++�����������++x��
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
	// _STEP1��һ�η�Χ��_STEP2�Ƕ��η�Χ��_STEP1>_STEP2�����Ǳ���ģ������ǵ�ȡֵ��g_prime_list��
	// _MAX�������Լ����Ԫ�ظ����������ò�Ҫ����_STEP1/2�������ϲ���Կ϶��������ֶ�������
	int	Init(int _STEP1, int _STEP2, int _MAX)
	{
		if( _STEP1<=_STEP2 )
		{
			assert(0);
			return	-1;
		}
		// 2006-02-17 ȡ����_MAX���벻Ҫ����_STEP1/2���ж�
		// 2006-02-20 ������_MAX���벻�ܳ���_STEP1������
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
	int	Init(void *pUnit, int _STEP1, int _STEP2, int _MAX)	// ��һ�����е��ڴ棨pUnit���ڲ����ȱ���ΪGetUnitMemSize(_STEP1)���ϲ���Ҫ����������ͷ�pUnit���ڴ棩
	{
		if( _STEP1<=_STEP2 )
		{
			assert(0);
			return	-1;
		}
		// 2006-02-17 ȡ����_MAX���벻Ҫ����_STEP1/2���ж�
		// 2006-02-20 ������_MAX���벻�ܳ���_STEP1������
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
			// ����Ҫrelease��
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
	// ���ԭ��key�ʹ������޷�����
	// ���ԭ���м����ڣ����bReplaceΪ�����滻ԭ���ģ����bReplaceΪ���򷵻ؼ١�
	// pnCount����ǿ��򷵻���ײ�Ĵ��������������Ҫ������debug�������ԣ�����hash�ķ�ɢ�̶ȡ���Ϊput����Ӧ�û�Ƚ��٣�����Ӧ�ò����������ɺܴ�Ӱ�죩
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
			// �ж��Ƿ����Լ�
			if( m_pUnit[nHash1].key == key )
			{
				if( bReplace )
				{
					m_pUnit[nHash1].val	= val;
					return	iterator(m_pUnit+nHash1, getendunit());;
				}
				else
				{
					// �����ظ�
					return	end();
				}
			}
			// ������Ǿ������
			unsigned int	nHash2	= nHash % m_STEP2;
			if( nHash2==0 )	nHash2 = m_STEP2;
			// �ϲ��һ��Ҫ��֤
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
					// �ҵ���λ����
					break;
				}
				// �ж��Ƿ����Լ�
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
			// ԭ��û�У����Դ���
		}
		// ������������һ���ǿ��Բ����λ����
		m_pUnit[nHash1].bTaken	= true;
		m_pUnit[nHash1].key		= key;
		m_pUnit[nHash1].val		= val;
		m_nTotal	++;

		return	iterator(m_pUnit+nHash1, getendunit());;
	}
	// ����
	iterator	find(const _Key & key)
	{
		unsigned int	nHash	= _hash(key);
		unsigned int	nHash1	= nHash % m_STEP1;
		if( m_pUnit[nHash1].bTaken )
		{
			// �ж��Ƿ����Լ�
			if( m_pUnit[nHash1].key == key )
			{
				return	iterator(m_pUnit+nHash1, getendunit());
			}
			// ������Ǿ������
			unsigned int	nHash2	= nHash % m_STEP2;
			if( nHash2==0 )	nHash2 = m_STEP2;
			// �ϲ��һ��Ҫ��֤
			for(;;)
			{
				nHash1	+= nHash2;
				if( nHash1>=(unsigned int)m_STEP1 )
				{
					nHash1	-= m_STEP1;
				}
				if( !m_pUnit[nHash1].bTaken )
				{
					// ����λ����
					return	end();
				}
				// �ж��Ƿ����Լ�
				if( m_pUnit[nHash1].key == key )
				{
					return	iterator(m_pUnit+nHash1, getendunit());
				}
			}
		}
		else
		{
			// û�������¼
			return	end();
		}
	}
	inline bool	has(const _Key & key)
	{
		return	end() != find(key);
	}
	// ɾ��
	// �Ƴ�ָ����ֵ�ĵ�Ԫ
	bool	erase(const _Key &key)
	{
		iterator	it	= find(key);
		if( it == end() )
		{
			return	false;
		}
		return	erase(it);
	}
	// ����iteratorɾ����ע�⣬�����Ҫ�ڱ�������ʹ�ã���Ϊ�����ܵ������ж���������!!!!��
	bool	erase(iterator &it)
	{
		if( it.m_pUnit->bTaken )
		{
			it.m_pUnit->bTaken	= false;
			m_nTotal			--;
			// ɨ��������������ն����ѷ�һ�����еĶ��ҳ������²�һ�顣
			for(int i=0;i<m_STEP1;i++)
			{
				if( m_pUnit[i].bTaken )
				{
					unsigned int	nHash1	= _hash(m_pUnit[i].key) % m_STEP1;
					if( nHash1 == i )
					{
						// һ�����еľͲ��ù���
					}
					else
					{
						// ��һ�����е�
						// ɾ����Ȼ�����²���
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
	// ���hash��
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
	// ͳ��hash���������
	struct	STAT_T
	{
		int	nGood;								// һ�����е�
		int	nBad;								// ��һ�����е�
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
	// �ڲ�ָ�����
	void	AdjustInnerPtr(int nOffset)
	{
		wh_adjustaddrbyoffset(m_pUnit, nOffset);
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHHASH2E_H__
