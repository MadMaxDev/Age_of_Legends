// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whprocess.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵Ľ�����غ�����
//                �������ô���ͳһwindows��linux�����ܻ��б��ϵͳ���µĽ������ʹ�÷���
// CreationDate : 2003-08-07
// ChangeLOG    : 2007-06-11 �������жϽ����Ƿ���ڵĺ�����ͬʱ��windows�µ�pid��ΪDWORD�Ķ�����HANDLE
//				: 2012-02-10 ������linux�´�������create���� by �����

#ifndef	__WHPROCESS_H__
#define	__WHPROCESS_H__

#include "../inc/whfile_base.h"
#ifdef	__GNUC__
#include <sys/types.h>
#include <unistd.h>
#endif

namespace n_whcmn{

#ifdef	WIN32
typedef DWORD				whpid_t;
#endif
#ifdef	__GNUC__
typedef pid_t				whpid_t;
#endif
const whpid_t INVALID_PID	= (whpid_t)0;

// __cmdline		�����Ǵ�������
// __workpath		�����Ĭ�Ͼ�д��NULL�����ǲ���д��""�������ǷǷ�·����
int		whprocess_create(whpid_t *__pid, const char *__cmdline, const char *__workpath);
// ����������ݴ�������
// ֻ��Linux������ʵ��
int		whprocess_create_with_arg(whpid_t *__pid, const char * __cmdline,const char * __workpath, char *  argv[]);
// ��ֹ���̣����bForceΪ������ɱ��
int		whprocess_terminate(whpid_t __pid, bool bForce=false);
bool	whprocess_waitend(whpid_t __pid, unsigned int __timeout = 0);

// ��õ�ǰ���̵�id
int		whprocess_getidentifier();

// �жϽ����Ƿ����
bool	whprocess_exists(whpid_t __pid);

// pid�ļ���صĲ���
// ����pid�ļ�
// ���__pid==0���ʾ�����Լ���pid�ļ�
int		whprocess_createpidfile(const char *__file, whpid_t __pid = INVALID_PID);
// ��pid�ļ��л�ȡpid
// ����ļ��������򷵻�INVALID_PID
whpid_t	whprocess_getfrompidfile(const char *__file);
// ����ɾ��pid�ļ��������ԵĽ��̴�����ɾ��������<0�Ĵ��������������������ͬ���õĳ���Ļ��⣩
int		whprocess_trydelpidfile(const char *__file);
// ����pid�ļ��رս��̣���ɾ��pid�ļ���
// ����
// 0	˵���ļ�ԭ���Ͳ�����
// 1	˵���ļ�ԭ���������ڱ�ɾ����
// -1	˵�����̻���������
// <-100˵���ļ�ɾ��ʱ������
int		whprocess_terminatebypidfile(const char *__file, bool bForce=false, int nWaitEnd=0);

// һ�������ڿ�ʼʱ����һ��pid�ļ����ڽ���ʱɾ��
class	whprocess_pidfile
{
private:
	char	m_szFile[WH_MAX_PATH];
public:
	whprocess_pidfile(const char *__file);
	~whprocess_pidfile();
};

}// EOF	namespace wh_genericfunc

#endif	// EOF __WHPROCESS_H__
