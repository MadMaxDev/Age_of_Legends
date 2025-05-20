// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whset.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵ļ���(set/multiset)����νset���ǿ���sort��
// CreationDate : 2004-04-21
// ChangeLog    :

#ifndef	__WHSET_H__
#define	__WHSET_H__

#include "whlist.h"
#include <assert.h>

// Ч��̫������:(((((((((((((
/*
namespace n_whcmn
{

template<typename _Ty, class _Allocator=whcmnallocator>
class	whset	: public whlist<_Ty, _Allocator>
{
protected:
	// �Ƿ��ǽ���
	bool	m_bDescent;
public:
	whset()
	: whlist<_Ty, _Allocator>()
	, m_bDescent(false)
	{}
	whset(int nChunkSize)
	: whlist<_Ty, _Allocator>(nChunkSize)
	, m_bDescent(false)
	{}
	inline void	SetDescent(bool bSet)
	{
		// ������ʱ��֧��
		assert(0);
		m_bDescent	= bSet;
	}
	// ����(ע��erase������whlist��)
	bool	insert(const _Ty &val)
	{
		// �ж�ԭ����û��
		iterator	it;
		if( find(val, it) )
		{
			// ԭ���У��޷�����
			return	false;
		}
		insertbefore(it, val);
		return	0;
	}
	// ��ȡ(�����ȡ��������itref�д�ŵ���ǡ�ÿ���insertbefore��it)
	bool	find(const _Ty &val, iterator &itref)
	{
		if( size()==0 )
		{
			itref	= end();
			return	false;
		}
		// �ö��ַ�����
		int			nSLen	= size();
		iterator	itbegin	= begin();
		iterator	itend	= end();
		while( nSLen>0 )
		{
			if( nSLen == 1 )
			{
				itref	= itbegin;
				if( m_bDescent )
				{
					// ����
				}
				else
				{
					// ����
					// �жϺ�val�Ĺ�ϵ
					if( (*itref) < val )
					{
						// ���λ����һ�����ǿ���insertbefore��λ��
						++	itref;
						return	false;
					}
					else if( val < (*itref) )
					{
						// ���λ�þ��ǿ���insertbefore��λ��
						return	false;
					}
				}
				// ���
				return	true;
			}
			// ������ҷ�Χ
			int	nSplitIdx	= nSLen/2 - 1;
			// ���ҵ�Ŀ���
			itref			= itbegin;
			itref			+= nSplitIdx;
			// ���Ƿ��Ѿ����
			if( (*itref) == val )
			{
				return	true;
			}
			// ������˳��
			if( m_bDescent )
			{
				// ����
			}
			else
			{
				// ����
				if( (*itref) < val )
				{
					// ���ڸõ���һ����β�ķ�Χ��
					itbegin	= itref;
					++	itbegin;
					nSLen	-= (nSplitIdx+1);
				}
				else
				{
					// ����ͷ���õ�ķ�Χ��
					itend	= itref;
					nSLen	= nSplitIdx+1;
				}
			}
		}
		// û�ҵ�
		return	false;
	}
};
*/

}		// EOF namespace n_whcmn

#endif	// EOF __WHSET_H__