// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfilewritecache.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �����ڴ��ļ����ļ�д��/�������ٹ���
//              : ������д����������һ���߳����
// CreationDate : 2005-02-06
// ChangeLOG    : 

#include "../inc/whmemfilewritecache_i.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whmemfilewritecache
////////////////////////////////////////////////////////////////////
whmemfilewritecache *	whmemfilewritecache::Create()
{
	return	new whmemfilewritecache_i;
}
