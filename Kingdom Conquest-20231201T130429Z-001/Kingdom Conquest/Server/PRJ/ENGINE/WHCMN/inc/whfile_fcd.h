// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whfile_fcd.h
// Creator      : Wei Hua (魏华)
// Comment      : 文件变化检测(File Change Detect)
// CreationDate : 2008-12-04 为了避免头文件嵌套从whfile_util.h中分离出来

#ifndef	__WHFILE_FCD_H__
#define	__WHFILE_FCD_H__

#include "whfile_base.h"
#include "whhash.h"

namespace n_whcmn
{

// 文件修改检查器（只通过文件时间来判断文件是否改变）
class	whfilechangedetector
{
public:
	whfilechangedetector();
	~whfilechangedetector();
	int		SetFile(const char *szFileName);
	int		Release();
	bool	IsChanged();											// 返回文件本次是否改变（同时里面可能设置最新的修改时刻，所以不能用const后缀）
private:
	char	m_szFileName[WH_MAX_PATH];
	time_t	m_nLastTime;											// 上次文件的修改时间
};

// 多个文件的修改检查器（只要有一个修改了就会返回true）
class	whmultifilechangedetector
{
public:
	whmultifilechangedetector();
	~whmultifilechangedetector();
	int		AddFile(const char *szFileName, int nExt);				// 重复的文件不会重复添加
	int		Release();
	struct	RST_T
	{
		char	*pszFileName;										// 文件名
		int		nExt;												// 对应的附加参数
	};
	whvector<RST_T> *	CheckChanged();								// 返回文件本次是否改变（同时里面可能设置最新的修改时刻，所以不能用const后缀）
																	// 如果返回为非空指针，则vector里面记录了变化的文件名的列表。返回空指针表示没有变化过
private:
	struct	UNIT_T
	{
		char	szFileName[WH_MAX_PATH];							// 文件名
		int		nExt;												// 附加参数
		time_t	nLastTime;											// 上次文件的修改时间
		inline void clear()
		{
			WHMEMSET0THIS();
		}
	};
	whvector<UNIT_T>	m_units;									// 文件信息数组
	whhashset<whstr4hash, whcmnallocationobj, _whstr_hashfunc>		m_setFNames;                                                                                                                                                                                                              
																	// 文件名表
	whvector<RST_T>		m_vectRst;									// 如果有文件变化，则CheckChanged会返回这个对象的指针
};

}		// EOF namespace n_whcmn

#endif	// __WHFILE_FCD_H__

