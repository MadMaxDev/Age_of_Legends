// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdbgmem.h
// Creator      : Wei Hua (κ��)
// Comment      : �ڴ���صĵ���
//              : �ο������ʵ��
// CreationDate : 2005-12-15

#ifndef __WHDBGMEM_H__
#define	__WHDBGMEM_H__

namespace n_whcmn
{

// ����������ֻ��_DEBUG���������²�������
// �����Ƿ��ڳ��������ʱ��Dump�ڴ�й©
void	whdbg_check_leak(bool check);
// ����ʱ����break�ϵ�
// cszBreakPointFile����һ���ı��ļ�����¼����Ҫ�жϵĵط�
void	whdbg_SetBreakAlloc(const char *cszBreakPointFile=0);

}		// EOF namespace n_whcmn

#endif
