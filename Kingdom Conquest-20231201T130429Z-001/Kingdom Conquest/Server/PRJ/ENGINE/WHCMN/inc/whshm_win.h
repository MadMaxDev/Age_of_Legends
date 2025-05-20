// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File     : whshm_win.h
// Creator  : Wei Hua (κ��)
// Comment  :
//          �򵥵Ĺ����ڴ湦�ܵ�WIN32ʵ�ְ汾
//          create��destroy����Ҫͨ��pipe���ʱ��ص�shmserver
//          Ϊ�˼��Ϳ������ԣ�ȫ��ʹ���ִ���Ϊ����ͷ���
//          ָ��ͷ���˵����
//          1. ����
//          ָ�create strname, size
//          ���أ�
//          �ɹ���OK
//          ʧ�ܣ�ERR code, codestr
//          ˵�������ԭ��û�У��Ҵ����ɹ����򷵻�OK��
//                ���ԭ���У��򷵻�ERR �����Ѿ�����
//          2. ����
//          ָ�destroy strname
//          �ɹ���OK
//          ʧ�ܣ�ERR code, codestr
//          ///////////////////////
//          ������룺
//          0     û�д���
//          1     ϵͳ����(���ڴ治���޷�����)
//          11    �����Ѿ�����
//          12    ���ֲ�����
// CreationDate: 2004-01-20
// ChangeLOG   : 2005��� �޸�Ϊpipe�汾

#ifdef	WIN32	// { WIN32 BEGIN

#ifndef	__WHSHM_WIN_H__
#define	__WHSHM_WIN_H__

#include "./whshm_cmn.h"

namespace	n_whcmn
{

// �ڲ�ʹ�õ�һЩ�ṹ

// �ڲ�ʹ�õ�һЩ����
// ͨ��nKey������һ������
const char *	i_whshm_makename(int nKey, char *szName);
const char *	i_whshm_makename(int nKey);
// �����ľ��嶯��(whshm_raw_create�ڲ�ͨ��shmserver����shmserverͨ�������)
int		i_whshm_raw_create(const char *cszKeyName, size_t nSize, WHSHM_RAW_INFO_T *pInfo);
int		i_whshm_raw_open(const char *cszKeyName, WHSHM_RAW_INFO_T *pInfo, void *pBaseAddr);

}				// EOF namespace n_whcmn

#endif			// EOF __WHSHM_WIN_H__

#endif			// } EOF WIN32
