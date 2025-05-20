// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whticker.h
// Creator      : Wei Hua (魏华)
// Comment      : 插值器、状态转换器
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
	WH_TICKER_STATE_WORKING		= 1,					// 还在工作中
	WH_TICKER_STATE_STOPPED		= 2,					// 循环到了尽头
};

////////////////////////////////////////////////////////////////////
// 插值器基本模板
////////////////////////////////////////////////////////////////////
template <typename _T, class _Interpolate >
class	whticker	: public _Interpolate
{
public:
	// 清空所有纪录，可以重新添加
	int		Clean();
	// 时间从头开始(值也从头开始)
	int		ResetTime();
	// 添加一对值，Val持续的时间为fTime
	// !!!!记住至少要加入两组点!!!!
	int		AddNode(const _T & Val, float fTime);
	// 返回元素个数
	int		GetNodeNum();
	// 设置总共循环的次数(<=0表示无穷循环)
	int		SetCycleCount(int nCount);
	// 一个tick，返回当前的状态
	int		Tick(float fElapsed);
	// 返回当前值(停止时停在起始位置上)
	const _T &	GetVal() const;
	// 获得总时间
	float	GetTotalTime() const;
	// 总时间变成指定值(默认变成1)
	void	SetTotalTime(float t);
	// 让所有时间乘以一个倍数
	void	ScaleTotalTime(float s);
	// 克隆一个对象
	whticker *	Clone() const;
	// 从其他对象克隆
	int		CloneFrom(whticker *pOther);

	// 获取指定的节点的值
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

// 内部使用的
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
	int		m_nIdx, m_nNextIdx;								// 当前正在处理的节点和下个将要处理的节点序号
	float	m_fTime;										// 当前节点已经使用的时间
	int		m_nCount;										// 当前已经循环的圈数
	int		m_nMaxCount;									// 最多循环的圈数
	_T		m_CurVal;										// 当前的值
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
	//m_CurVal	= 0;	这个在_T没有实现=0方法，或者有歧义时会出错
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
		// 是第一个
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

	// 判断当前这段是否已经走完
	if( (fDT=m_fTime-m_vectNodes[m_nIdx].fTime) >= 0 )
	{
		m_nIdx		= m_nNextIdx;
		m_nNextIdx	= (m_nNextIdx + 1) % m_vectNodes.size();
		m_fTime		= fDT;
		if( m_nNextIdx == 0 )
		{
			// 一圈了
			if( m_nMaxCount>0 )
			{
				++ m_nCount;
				if( m_nCount >= m_nMaxCount )
				{
					// 结尾到最后一个值
					m_CurVal	= (*(m_vectNodes.end()-1)).Val;
					return	WH_TICKER_STATE_WORKING;
				}
			}
		}
	}

	// 避免时间太短的情形
	if( m_fTime>m_vectNodes[m_nIdx].fTime )
	{
		m_fTime	= m_vectNodes[m_nIdx].fTime;
	}
	// 插值
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
	// 获得目前的总时间
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
// 插值器
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
// 状态转换器
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
