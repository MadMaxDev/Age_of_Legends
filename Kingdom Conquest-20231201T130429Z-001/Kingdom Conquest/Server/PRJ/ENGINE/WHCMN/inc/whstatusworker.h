// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstatusworker.h
// Creator      : Wei Hua (κ��)
// Comment      : ���ݵ�ǰ��״̬����
// CreationDate : 2004-1-27
// ChangeLog    :
//               2004-02-04 MAKE_WHSH_WORKFUNC����Ҫ����n_whcmn�����Σ�����linux�����жϱȽ��ϸ�
//                          �����address of overloaded function with no contextual type�Ĵ���
//                          ���о�����linux�£�ָ����Ա����ָ�������&CLASS:FUNC����ʽ����VC�¾Ͳ���ô�ϸ� (��whdataprop�������Ƶ����)
//               2006-03-02 ������IsStatusJustChanged����

#ifndef	__WHSTATUSWORKER_H__
#define	__WHSTATUSWORKER_H__

#include "whvector.h"
#include <stdlib.h>

namespace n_whcmn
{

struct	whstatusworker;
class	whstatusholder;

// SH����status holder
typedef	void (whstatusholder::*WHSH_WORKFUNC_T)(void);
#define	WHSH_DECLARETHISCLASS(cls)				typedef	cls	WHSH_THISCLASS
#define	MAKE_WHSH_WORKFUNC(cls, func)			((n_whcmn::WHSH_WORKFUNC_T)&cls::func)
#define	MAKE_WHSH_WORKFUNC_OFTHISCLASS(func)	((n_whcmn::WHSH_WORKFUNC_T)&WHSH_THISCLASS::func)

////////////////////////////////////////////////////////////////////
// whstatusworker ���µ�
////////////////////////////////////////////////////////////////////
struct	whstatusworker
{
	enum
	{
		STATUS_NOTHING			= 0,			// ���״̬�Ǳ����ġ�һ�����಻Ҫ�������״̬��
	};
	int							nStatus;
	whvector<WHSH_WORKFUNC_T>	apWork;			// 0��begin��1��end��ʣ�µĸ���work

	whstatusworker(int nMyStatus, int nWORKNUM=1);
	inline void			SetBeginFunc(WHSH_WORKFUNC_T pFunc)
	{
		apWork[0]		= pFunc;
	}
	inline void			SetEndFunc(WHSH_WORKFUNC_T pFunc)
	{
		apWork[1]		= pFunc;
	}
	// �ϲ㱣֤nIdx����ȷ�ԣ����ܳ���nWORKNUM-1
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
// whstatusholder ���µ�
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
	// ������������������Կ�ָ����ж�
	whstatusworker	m_dummyworker;
	void	WHSH_WORKFUNC_DUMMY(void)
	{
	}
	bool	m_bJustChanged;
public:
	whstatusholder();
	void	SetStatus(whstatusworker &worker);
	void	SetStatusToNULL();
	// ��ʾStatus�ոձ�ת����
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
