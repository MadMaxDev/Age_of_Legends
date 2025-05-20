// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File: whstatusworker.cpp
// Creator: Wei Hua (κ��)
// Comment: ���ݵ�ǰ��״̬����
// CreationDate: 2004-1-27

#include "../inc/whstatusworker.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whstatusworker
////////////////////////////////////////////////////////////////////
whstatusworker::whstatusworker(int nMyStatus, int nWORKNUM)
: nStatus(nMyStatus), apWork(2+nWORKNUM)
{
	for(size_t i=0;i<apWork.size();i++)
	{
		apWork[i]	= &whstatusholder::WHSH_WORKFUNC_DUMMY;
	}
}

////////////////////////////////////////////////////////////////////
// whstatusholder
////////////////////////////////////////////////////////////////////
whstatusholder::whstatusholder()
: m_dummyworker(whstatusworker::STATUS_NOTHING)
, m_bJustChanged(false)
{
	m_pCurWorker	= &m_dummyworker;
}
void	whstatusholder::SetStatus(whstatusworker &worker)
{
	// ���״̬�Ǳ�����m_dummyworker��
	assert(worker.nStatus != whstatusworker::STATUS_NOTHING );

	if( m_pCurWorker->nStatus == worker.nStatus )
	{
		// һ������������һ����
		return;
	}

	m_bJustChanged	= true;

	DoWorkEnd();
	m_pCurWorker	= &worker;
	DoWorkBegin();
}
void	whstatusholder::SetStatusToNULL()
{
	DoWorkEnd();
	m_pCurWorker	= &m_dummyworker;
}
