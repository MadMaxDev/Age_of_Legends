// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gzs.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GZSģ��
//                PNGS��Pixel Network Game Structure����д
//                GZS��Game Zone Server����д�����߼��������е��ܿط�����
//                GZS��ʵ����Ҫ����ײ���������ӺͶԸ���ILogic�������Ϣ����
// CreationDate : 2005-07-25
// Change LOG   :

#ifndef	__PNGS_GZS_H__
#define	__PNGS_GZS_H__

#include "./pngs_cmn.h"

namespace n_pngs
{

class	GZS
{
public:
	static CMN::ILogic *	Create(const char *szLogicType);
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_GZS_H__
