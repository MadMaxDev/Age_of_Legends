// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace : n_whcmn
// File      : whdaemon.h
// Creator   : Wei Hua (κ��)
// Comment   : �ػ����̸���������������Linux��ʹ�ã�
// ChangeLOG :

#ifndef	__WHDAEMON_H__
#define	__WHDAEMON_H__

namespace n_whcmn
{

// bAutoCloseFD��ʾ�Ƿ���Ҫ���ӽ����йر����и����̴������ļ����
// ע�⣺���bAutoCloseFDΪ�棬����ʹ��whdaemon_init֮ǰ��Ҫ�����κ�socket������ᱻ�ر�!!!!
int		whdaemon_init(bool bAutoCloseFD=false);

}

#endif	// EOF __WHDAEMON_H__
