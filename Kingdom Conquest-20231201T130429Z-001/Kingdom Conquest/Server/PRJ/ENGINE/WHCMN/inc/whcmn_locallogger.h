//added by yuezhongyue 2011-08-05,��ӱ�����־���ݴ���
//��ǰʹ�õĶ���������־ϵͳ,���ڵײ�ʹ�õ���udp,�ھ���������ʱ��Ҳ�ᶪʧһЩ�ؼ���־,���Թؼ���־���ڱ���Ҳ��¼����

#ifndef __WHCMN_LOCALLOGGER_H__
#define __WHCMN_LOCALLOGGER_H__

#include "../inc/whlog.h"

namespace n_whcmn
{
int		LLogger_Init(const char* cszLocalLogPrefix);
int		LLogger_Release();
int		LLogger_Tick();
int		LLogger_WriteFmt(const char* cszFmt, ...);

//added by yuezhongyue 2011-08-10,dll��g_pLocalLogWriterָ�빲��
whlogwriter*	WHCMN_Get_LocalLogWriter();
void			WHCMN_Set_LocalLogWriter(whlogwriter* pLogWriter);
}

#define LLOGGER_INIT		n_whcmn::LLogger_Init
#define LLOGGER_RELEASE		n_whcmn::LLogger_Release
#define LLOGGER_TICK		n_whcmn::LLogger_Tick
#define LLOGGER_WRITEFMT	n_whcmn::LLogger_WriteFmt

#endif
