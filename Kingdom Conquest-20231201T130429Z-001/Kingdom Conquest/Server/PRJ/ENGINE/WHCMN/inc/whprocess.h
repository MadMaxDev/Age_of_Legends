// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whcmn
// File         : whprocess.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的进程相关函数。
//                这个库的用处是统一windows和linux（可能还有别的系统）下的进程相关使用方法
// CreationDate : 2003-08-07
// ChangeLOG    : 2007-06-11 增加了判断进程是否存在的函数。同时把windows下的pid改为DWORD的而不是HANDLE
//				: 2012-02-10 增加了linux下带参数的create函数 by 张昊阳

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

// __cmdline		可以是带参数的
// __workpath		如果想默认就写成NULL，但是不能写成""（这样是非法路径）
int		whprocess_create(whpid_t *__pid, const char *__cmdline, const char *__workpath);
// 用这个来传递带参数的
// 只有Linux下做了实现
int		whprocess_create_with_arg(whpid_t *__pid, const char * __cmdline,const char * __workpath, char *  argv[]);
// 终止进程，如果bForce为真则暴力杀死
int		whprocess_terminate(whpid_t __pid, bool bForce=false);
bool	whprocess_waitend(whpid_t __pid, unsigned int __timeout = 0);

// 获得当前进程的id
int		whprocess_getidentifier();

// 判断进程是否存在
bool	whprocess_exists(whpid_t __pid);

// pid文件相关的操作
// 生成pid文件
// 如果__pid==0则表示生成自己的pid文件
int		whprocess_createpidfile(const char *__file, whpid_t __pid = INVALID_PID);
// 从pid文件中获取pid
// 如果文件不存在则返回INVALID_PID
whpid_t	whprocess_getfrompidfile(const char *__file);
// 尝试删除pid文件，如果相对的进程存在则不删除并返回<0的错误（这个用于启动两个相同配置的程序的互斥）
int		whprocess_trydelpidfile(const char *__file);
// 根据pid文件关闭进程（并删除pid文件）
// 返回
// 0	说明文件原来就不存在
// 1	说明文件原来存在现在被删除了
// -1	说明进程还在运行中
// <-100说明文件删除时出错了
int		whprocess_terminatebypidfile(const char *__file, bool bForce=false, int nWaitEnd=0);

// 一个对象，在开始时生成一个pid文件，在结束时删除
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
