// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whmemfilewritecache.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 基于内存文件的文件写盘/读盘提速功能
//              : 真正的写盘启用另外一个线程完成
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
