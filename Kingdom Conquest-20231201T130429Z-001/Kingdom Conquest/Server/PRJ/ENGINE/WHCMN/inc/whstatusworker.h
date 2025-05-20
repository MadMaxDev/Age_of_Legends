// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstatusworker.h
// Creator      : Wei Hua (魏华)
// Comment      : 根据当前的状态做事
// CreationDate : 2004-1-27
// ChangeLog    :
//               2004-02-04 MAKE_WHSH_WORKFUNC中需要加上n_whcmn的修饰，否在linux好像判断比较严格
//                          会出现address of overloaded function with no contextual type的错误
//                          还有就是在linux下，指定成员函数指针必须用&CLASS:FUNC的形式，在VC下就不这么严格 (在whdataprop中有类似的情况)
//               2006-03-02 增加了IsStatusJustChanged方法

#ifndef	__WHSTATUSWORKER_H__
#define	__WHSTATUSWORKER_H__

#include "whvector.h"
#include <stdlib.h>

namespace n_whcmn
{

struct	whstatusworker;
class	whstatusholder;

// SH代表status holder
typedef	void (whstatusholder::*WHSH_WORKFUNC_T)(void);
#define	WHSH_DECLARETHISCLASS(cls)				typedef	cls	WHSH_THISCLASS
#define	MAKE_WHSH_WORKFUNC(cls, func)			((n_whcmn::WHSH_WORKFUNC_T)&cls::func)
#define	MAKE_WHSH_WORKFUNC_OFTHISCLASS(func)	((n_whcmn::WHSH_WORKFUNC_T)&WHSH_THISCLASS::func)

////////////////////////////////////////////////////////////////////
// whstatusworker 干事的
////////////////////////////////////////////////////////////////////
struct	whstatusworker
{
	enum
	{
		STATUS_NOTHING			= 0,			// 这个状态是保留的。一般子类不要设置这个状态。
	};
	int							nStatus;
	whvector<WHSH_WORKFUNC_T>	apWork;			// 0是begin，1是end，剩下的各种work

	whstatusworker(int nMyStatus, int nWORKNUM=1);
	inline void			SetBeginFunc(WHSH_WORKFUNC_T pFunc)
	{
		apWork[0]		= pFunc;
	}
	inline void			SetEndFunc(WHSH_WORKFUNC_T pFunc)
	{
		apWork[1]		= pFunc;
	}
	// 上层保证nIdx的正确性，不能超过nWORKNUM-1
	inline void			SetWorkFunc(WHSH_WORKFUNC_T pFunc, int nWorkIdx=0)
	{
		apWork[2+nWorkIdx]	= pFunc;
	}
private:
	friend	class		whstatusholder;
	inline	void		SetStatusToNothing()
	{
		nStatus			= STATUS_NOTHING;
	}
};

////////////////////////////////////////////////////////////////////
// whstatusholder 管事的
////////////////////////////////////////////////////////////////////
class	whstatusholder
{
protected:
	whstatusworker	*m_pCurWorker;
protected:
	inline void	DoIdxWork(whstatusworker *pWorker, int nIdx)
	{
		(this->*pWorker->apWork[nIdx])();
	}
private:
	friend	struct	whstatusworker;
	// 这两个设置用来避免对空指针的判断
	whstatusworker	m_dummyworker;
	void	WHSH_WORKFUNC_DUMMY(void)
	{
	}
	bool	m_bJustChanged;
public:
	whstatusholder();
	void	SetStatus(whstatusworker &worker);
	void	SetStatusToNULL();
	// 表示Status刚刚被转换过
	inline bool	IsStatusJustChanged()
	{
		if( m_bJustChanged )
		{
			m_bJustChanged	= false;
			return	true;
		}
		return	false;
	}
	inline int		GetStatus() const
	{
		return	m_pCurWorker->nStatus;
	}
	inline void		DoWorkBegin()
	{
		DoIdxWork(m_pCurWorker, 0);
	}
	inline void		DoWorkEnd()
	{
		DoIdxWork(m_pCurWorker, 1);
	}
	inline void		DoWork(int nWorkIdx=0)
	{
		DoIdxWork(m_pCurWorker, 2+nWorkIdx);
	}
};


}		// EOF namespace n_whcmn

#endif	// EOF __WHSTATUSWORKER_H__
