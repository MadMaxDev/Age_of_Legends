// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whgener.h
// Creator      : Wei Hua (魏华)
// Comment      : 产生器，在时间内产生一个整数
// CreationDate : 2003-12-26

#ifndef	__WHGENER_H__
#define	__WHGENER_H__

namespace n_whcmn
{

class	whgener
{
protected:
	// 生成速率(个/秒)
	float	m_fGenSpeed;
	// 应该发送的数量
	float	m_fShouldGen;
public:
	whgener() : m_fGenSpeed(0), m_fShouldGen(0)
	{}
	inline int		MakeDecision(float fInterval)
	{
		m_fShouldGen		+= m_fGenSpeed * fInterval;
		int	nDec			= int(m_fShouldGen);
		if( nDec>0 )
		{
			m_fShouldGen	-= nDec;
		}
		return	nDec;
	}
	inline void		Reset()
	{
		m_fShouldGen	= 0;
	}
	inline void		SetGenSpeed(float fSpeed)	// 这个speed是可以随时设置的
	{
		m_fGenSpeed	= fSpeed;
	}
};

}	// EOF namespace n_whcmn

#endif	// EOF __WHGENER_H__
