// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : wh_dllstaticmodule.h
// Creator      : Wei Hua (κ��)
// Comment      : ���DLL��Ҫ����һЩWHCMN����Ķ������磺WHCMN_STATIC_INFO_In�����Ͱ������
//              : ����ļ�ֻ��DLL�Ĺ��̲���Ҫ��������WHCMN_STATIC_INFO_In����������������̶�����WHCMN�����໥��������������ʱWHCMN_STATIC_INFO_In�ظ�����
// CreationDate : 2005-10-21

#ifndef	__WH_DLLSTATICMODULE_H__
#define	__WH_DLLSTATICMODULE_H__

#include "whcmn_def.h"

void	_WHCMN_STATIC_INFO_In(void *pInfo);
extern "C" WH_DLL_API	void	WHCMN_STATIC_INFO_In(void *pInfo)
{
	_WHCMN_STATIC_INFO_In(pInfo);
}

struct	wh_dllstaticmodule
{
	wh_dllstaticmodule()
	{
		WHCMN_STATIC_INFO_In(NULL);
	}
};
static wh_dllstaticmodule	gs_wh_dllstaticmodule;

#endif	// EOF __WH_DLLSTATICMODULE_H__
