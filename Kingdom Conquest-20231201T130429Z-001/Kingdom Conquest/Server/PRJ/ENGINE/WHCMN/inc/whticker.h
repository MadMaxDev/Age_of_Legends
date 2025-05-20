// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whticker.h
// Creator      : Wei Hua (κ��)
// Comment      : ��ֵ����״̬ת����
// CreationDate : 2003-11-17

#ifndef	__WHTICKER_H__
#define	__WHTICKER_H__

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif
#include <vector>
#include <assert.h>

namespace n_whcmn
{

enum
{
	WH_TICKER_STATE_NOTHING		= 0,
	WH_TICKER_STATE_WORKING		= 1,					// ���ڹ�����
	WH_TICKER_STATE_STOPPED		= 2,					// ѭ�����˾�ͷ
};

////////////////////////////////////////////////////////////////////
// ��ֵ������ģ��
////////////////////////////////////////////////////////////////////
template <typename _T, class _Interpolate >
class	whticker	: public _Interpolate
{
public:
	// ������м�¼�������������
	int		Clean();
	// ʱ���ͷ��ʼ(ֵҲ��ͷ��ʼ)
	int		ResetTime();
	// ���һ��ֵ��Val������ʱ��ΪfTime
	// !!!!��ס����Ҫ���������!!!!
	int		AddNode(const _T & Val, float fTime);
	// ����Ԫ�ظ���
	int		GetNodeNum();
	// �����ܹ�ѭ���Ĵ���(<=0��ʾ����ѭ��)
	int		SetCycleCount(int nCount);
	// һ��tick�����ص�ǰ��״̬
	int		Tick(float fElapsed);
	// ���ص�ǰֵ(ֹͣʱͣ����ʼλ����)
	const _T &	GetVal() const;
	// �����ʱ��
	float	GetTotalTime() const;
	// ��ʱ����ָ��ֵ(Ĭ�ϱ��1)
	void	SetTotalTime(float t);
	// ������ʱ�����һ������
	void	ScaleTotalTime(float s);
	// ��¡һ������
	whticker *	Clone() const;
	// �����������¡
	int		CloneFrom(whticker *pOther);

	// ��ȡָ���Ľڵ��ֵ
	inline const _T &	GetNodeVal(int nIdx) const
	{
		return	m_vectNodes[nIdx].Val;
	}
	inline float		GetNodeTime(int nIdx) const
	{
		return	m_vectNodes[nIdx].fTime;
	}
	inline float		GetNodeTimeOnVal(const _T &Val) const
	{
		for(size_t i=0;i<m_vectNodes.size();i++)
		{
			if( m_vectNodes[i].Val == Val )
			{
				return	m_vectNodes[i].fTime;
			}
		}
		return	0;
	}

// �ڲ�ʹ�õ�
protected:
	struct	FVTNODE_T
	{
		_T		Val;
		float	fTime;
		FVTNODE_T()
		: Val(0), fTime(0)
		{ }
		FVTNODE_T(const FVTNODE_T &other)
		{
			Val		= other.Val;
			fTime	= other.fTime;
		}
		FVTNODE_T(const _T & _Val, float _fTime)
		{
			Val		= _Val;
			fTime	= _fTime;
		}
	};
	typedef	std::vector<FVTNODE_T>	VECTFVTNODE_T;
protected:
	int		m_nIdx, m_nNextIdx;								// ��ǰ���ڴ���Ľڵ���¸���Ҫ����Ľڵ����
	float	m_fTime;										// ��ǰ�ڵ��Ѿ�ʹ�õ�ʱ��
	int		m_nCount;										// ��ǰ�Ѿ�ѭ����Ȧ��
	int		m_nMaxCount;									// ���ѭ����Ȧ��
	_T		m_CurVal;										// ��ǰ��ֵ
	VECTFVTNODE_T	m_vectNodes;
public:
	virtual ~whticker();
	whticker();
};

template <typename _T, class _Interpolater>
whticker<_T, _Interpolater>::whticker()
{
	Clean();
}
template <typename _T, class _Interpolater>
whticker<_T, _Interpolater>::~whticker()
{
	Clean();
}

template <typename _T, class _Interpolater>
int	whticker<_T, _Interpolater>::Clean()
{
	m_nIdx		= 0;
	m_nNextIdx	= 1;
	m_fTime		= 0;
	m_nCount	= 0;
	m_nMaxCount	= -1;
	//m_CurVal	= 0;	�����_Tû��ʵ��=0����������������ʱ�����
	m_vectNodes.clear();
	return	0;
}
template <typename _T, class _Interpolater>
int	whticker<_T, _Interpolater>::ResetTime()
{
	m_nIdx		= 0;
	m_nNextIdx	= 1;
	m_nCount	= 0;
	m_fTime		= 0;
	m_CurVal	= m_vectNodes[0].Val;
	return	0;
}
template <typename _T, class _Interpolater>
int	whticker<_T, _Interpolater>::AddNode(const _T & Val, float fTime)
{
	m_vectNodes.push_back(FVTNODE_T(Val, fTime));
	if( m_vectNodes.size()==1 )
	{
		// �ǵ�һ��
		m_CurVal	= Val;
	}
	return	0;
}
template <typename _T, class _Interpolater>
int	whticker<_T, _Interpolater>::GetNodeNum()
{
	return	m_vectNodes.size();
}
template <typename _T, class _Interpolater>
int	whticker<_T, _Interpolater>::SetCycleCount(int nCount)
{
	m_nMaxCount	= nCount;
	return	0;
}
template <typename _T, class _Interpolater>
int	whticker<_T, _Interpolater>::Tick(float fElapsed)
{
	if( m_nMaxCount>0 && m_nCount >= m_nMaxCount )
	{
		return	WH_TICKER_STATE_STOPPED;
	}
	if( m_vectNodes.size()==0 )
	{
		return	WH_TICKER_STATE_STOPPED;
	}

	float	fDT;
	m_fTime	+= fElapsed;

	// �жϵ�ǰ����Ƿ��Ѿ�����
	if( (fDT=m_fTime-m_vectNodes[m_nIdx].fTime) >= 0 )
	{
		m_nIdx		= m_nNextIdx;
		m_nNextIdx	= (m_nNextIdx + 1) % m_vectNodes.size();
		m_fTime		= fDT;
		if( m_nNextIdx == 0 )
		{
			// һȦ��
			if( m_nMaxCount>0 )
			{
				++ m_nCount;
				if( m_nCount >= m_nMaxCount )
				{
					// ��β�����һ��ֵ
					m_CurVal	= (*(m_vectNodes.end()-1)).Val;
					return	WH_TICKER_STATE_WORKING;
				}
			}
		}
	}

	// ����ʱ��̫�̵�����
	if( m_fTime>m_vectNodes[m_nIdx].fTime )
	{
		m_fTime	= m_vectNodes[m_nIdx].fTime;
	}
	// ��ֵ
	m_CurVal	= Interpolate(
				  m_fTime
				, m_vectNodes[m_nIdx].fTime
				, m_vectNodes[m_nIdx].Val
				, m_vectNodes[m_nNextIdx].Val
				);

	return	WH_TICKER_STATE_WORKING;
}
template <typename _T, class _Interpolater>
const _T &	whticker<_T, _Interpolater>::GetVal() const
{
	return	m_CurVal;
}

template <typename _T, class _Interpolater>
float	whticker<_T, _Interpolater>::GetTotalTime() const
{
	// ���Ŀǰ����ʱ��
	float	fTotal = 0;
	for(size_t i=0;i<m_vectNodes.size();i++)
	{
		fTotal	+= m_vectNodes[i].fTime;
	}
	return	fTotal;
}

template <typename _T, class _Interpolater>
void	whticker<_T, _Interpolater>::SetTotalTime(float t)
{
	if(t>0)
	{
		ScaleTotalTime(t/GetTotalTime());
	}
}

template <typename _T, class _Interpolater>
void	whticker<_T, _Interpolater>::ScaleTotalTime(float s)
{
	for(size_t i=0;i<m_vectNodes.size();i++)
	{
		m_vectNodes[i].fTime	*= s;
	}
}

template <typename _T, class _Interpolater>
whticker<_T, _Interpolater> *	whticker<_T, _Interpolater>::Clone() const
{
	whticker<_T, _Interpolater>	*pNew = new whticker<_T, _Interpolater>;
	pNew->CloneFrom(this);
	return	pNew;
}

template <typename _T, class _Interpolater>
int		whticker<_T, _Interpolater>::CloneFrom(whticker *pOther)
{
	m_nIdx		= pOther->m_nIdx		;
	m_nNextIdx	= pOther->m_nNextIdx	;
	m_fTime		= pOther->m_fTime		;
	m_nCount	= pOther->m_nCount		;
	m_nMaxCount	= pOther->m_nMaxCount	;
	m_CurVal	= pOther->m_CurVal		;
	m_vectNodes	= pOther->m_vectNodes	;
	return	0;
}

////////////////////////////////////////////////////////////////////
// ��ֵ��
////////////////////////////////////////////////////////////////////
template <typename _T>
class	wh_Interpolater_Normal
{
protected:
	inline const _T		Interpolate(float fT, float fTotalT, const _T &begin, const _T &end) const
	{
		if( fTotalT<=0 )
		{
			if( fT==0 )
				return	begin;
			else
				return	end;
		}
		else
		{
			fT		/= fTotalT;
			return	end*fT + begin*(1-fT);
		}
	}
};

////////////////////////////////////////////////////////////////////
// ״̬ת����
////////////////////////////////////////////////////////////////////
template <typename _T>
class	wh_Interpolater_State
{
protected:
	inline const _T		Interpolate(float fT, float fTotalT, const _T &begin, const _T &end) const
	{
		if( fTotalT>0 )
			return	begin;
		else
		{
			if( fT==0 )
				return	begin;
			else
				return	end;
		}
	}
};

}	// EOF namespace n_whcmn

#endif	// EOF __WHTICKER_H__
