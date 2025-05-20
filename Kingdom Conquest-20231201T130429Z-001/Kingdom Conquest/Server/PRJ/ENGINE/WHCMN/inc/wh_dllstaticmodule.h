// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : wh_dllstaticmodule.h
// Creator      : Wei Hua (魏华)
// Comment      : 如果DLL需要导出一些WHCMN库里的东西（如：WHCMN_STATIC_INFO_In），就包含这个
//              : 这个文件只有DLL的工程才需要包含，把WHCMN_STATIC_INFO_In放在这里，避免多个工程都依赖WHCMN且又相互依赖，导致连接时WHCMN_STATIC_INFO_In重复定义
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
