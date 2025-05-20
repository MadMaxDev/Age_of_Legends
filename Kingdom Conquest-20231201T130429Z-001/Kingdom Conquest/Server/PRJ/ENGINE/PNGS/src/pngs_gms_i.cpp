// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ��ĸ���Logicģ���ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-08-01
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"

using namespace n_pngs;

CMN::ILogic *	GMS::Create(const char *szLogicType)
{
	if( stricmp(szLogicType, "uGMS_MAINSTRUCTURE" )==0 )
	{
		return	new	GMS_MAINSTRUCTURE;
	}
	return	NULL;
}

