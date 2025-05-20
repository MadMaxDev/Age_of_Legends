// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whsignal.h
// Creator      : Wei Hua (κ��)
// Comment      : ��signal�İ�װ��Ϊ����2000��linux�¶��á�
//                ֻҪ������whsignal_init���Ϳ���ʹ��signal�������ض��źŵĴ�����
//                ������˵��signalһ�㶼�Ǹ�����̨�����õģ�windows�����ֱ������Ϣ�ɡ�
//                ��Ҫע�⣺������߳��н������룬����ctrl-c���ʹ�����޷���������!!!! ����Ҳ�ã���������ctrl-c�˾��ǲ�ϣ���ټ����ˡ�
// CreationDate : 2004-02-20
// ChangeLOG    : 2006-08-16 ������whsingal_set_exitfunc������������windows��������ctrl-c���߹ر�ť�Ϳ��Ա��ػ���

#ifndef	__WHSIGNAL_H__
#define	__WHSIGNAL_H__

#include <signal.h>

namespace	n_whcmn
{

// ��ʹ��signal�ĳ�ʼ������ҪΪ����windows�¿��Բ���TERM��Ϣ��
int		whsignal_init();

// ���ϣ����һЩ������ֱ��ʹ������ĺ���
// �����Ϳ�����ctrl-c��ر�Ӧ�ó����ʱ�����ָ���ĺ���
int		whsingal_set_exitfunc(void (*sigfunc)(int sig));

}				// EOF namespace n_whcmn

#endif	// EOF __WHSIGNAL_H__
