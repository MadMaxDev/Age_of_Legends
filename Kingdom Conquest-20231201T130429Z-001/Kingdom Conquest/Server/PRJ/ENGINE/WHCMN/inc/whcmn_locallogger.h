//added by yuezhongyue 2011-08-05,添加本地日志备份处理
//以前使用的都是网络日志系统,由于底层使用的是udp,在局域网内有时候也会丢失一些关键日志,所以关键日志现在本地也记录下来

#ifndef __WHCMN_LOCALLOGGER_H__
#define __WHCMN_LOCALLOGGER_H__

#include "../inc/whlog.h"

namespace n_whcmn
{
int		LLogger_Init(const char* cszLocalLogPrefix);
int		LLogger_Release();
int		LLogger_Tick();
int		LLogger_WriteFmt(const char* cszFmt, ...);

//added by yuezhongyue 2011-08-10,dll间g_pLocalLogWriter指针共享
whlogwriter*	WHCMN_Get_LocalLogWriter();
void			WHCMN_Set_LocalLogWriter(whlogwriter* pLogWriter);
}

#define LLOGGER_INIT		n_whcmn::LLogger_Init
#define LLOGGER_RELEASE		n_whcmn::LLogger_Release
#define LLOGGER_TICK		n_whcmn::LLogger_Tick
#define LLOGGER_WRITEFMT	n_whcmn::LLogger_WriteFmt

#endif
