// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ��(GMS�ڲ�ʵ����Ҫ������Logicģ��)
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
//                GMS��ʵ����Ҫ����ײ���������ӺͶԸ���ILogic�������Ϣ����
// Logic����    : 1. "uGMS_MAINSTRUCTURE"
//                2. "uGMS_GAMEPLAY"
// CreationDate : 2005-07-23
// Change LOG   :

#ifndef	__PNGS_GMS_H__
#define	__PNGS_GMS_H__

#include "./pngs_cmn.h"

namespace n_pngs
{

class	GMS
{
public:
	static CMN::ILogic *	Create(const char *szLogicType);
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_GMS_H__
