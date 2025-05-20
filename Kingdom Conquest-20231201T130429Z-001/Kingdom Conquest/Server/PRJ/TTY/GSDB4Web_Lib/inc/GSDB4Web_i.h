//////////////////////////////////////////////////////////////////////////
//web��չ�ҽ��ڸ��������Ϻ�DBS4Web����ͨ�ŵ�ģ��
//!!!��Ҫ�������tcp����ʧЧ��ʱ������޷������ݵ����,����Ӧ�ò�����
//////////////////////////////////////////////////////////////////////////
#ifndef __GSDB4Web_i_H__
#define __GSDB4Web_i_H__

#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_packet_logic.h"
#include "WHCMN/inc/whdataini.h"
#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnettcp.h"
#include "../../Common/inc/tty_def_dbs4web.h"
#include "../../Common/inc/pngs_packet_db4web_logic.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class GSDB4Web_i : public CMN::ILogic
{
//ILogic�ӿ�
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET>& vect) {return 0;}
	virtual	int		SureSend();
private:
	virtual	int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		Tick_BeforeDealCmdIn();
	virtual	int		Tick_AfterDealCmdIn();
private:
	int				DealCmdFromDBS4Web(const void* pData, size_t nSize);			//��������DBS4Web����Ϣ
	int				CmdIn_PNGSPACKET_2DB_CONFIG(ILogicBase *pRstAccepter, PNGSPACKET_2DB4WEB_CONFIG_T *pCmd, int nSize);
//	int				Tick_DealTE();
//�Լ��Ķ���
public:
	void			Retry_Worker_WORKING_Begin();
private:
	int				ConnectToDBS4Web();
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		int			nQueueCmdInSize;							//ָ��������еĳ���
		char		szDBS4WebAddr[WHNET_MAXADDRSTRLEN];			//XDBS4Web�ļ�����ַ
		int			nConnectDBS4WebTimeout;						//����DBS4Web�ĳ�ʱ
		int			nSendRecvBufSize;							//�շ�����Ĵ�С
		int			nInterval;									//send,recv�ĳ�ʱ

		CFGINFO_T()
		: nQueueCmdInSize(20*1024*1024)
		, nConnectDBS4WebTimeout(10*1000)
		, nSendRecvBufSize(512*1024)
		, nInterval(0)
		{
			strcpy(szDBS4WebAddr, "127.0.0.1:4500");
		}
		
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	}m_cfgInfo;
	typedef	CMN::ILogic						FATHERCLASS;
public:
	GSDB4Web_i();
	virtual	~GSDB4Web_i();
	int				Init_CFG(const char* cszCFGName);
private:
	time_t			m_nTimeNow;						//��tick��ʱ��
	whtick_t		m_tickNow;
	int				m_nAppType;
	//������װ���ָ��Ļ���
	whvector<char>	m_vectrawbuf;
	
	//�����շ���,����DBS4Web
	class MYMSGER_T : public tcpretrymsger<tcpmsger>
	{
	public:
		GSDB4Web_i*	m_pHost;
	public:
		virtual	void	i_Worker_WORKING_Begin()
		{
			m_pHost->Retry_Worker_WORKING_Begin();
		}
	};
	MYMSGER_T		m_retrymsgerDBS4Web;

	//////////////////////////////////////////////////////////////////////////
	//��Ҫʹ�õ�������logicģ��
	//////////////////////////////////////////////////////////////////////////
	//ʹ��DB���
	ILogicBase*		m_pLogicDBUser;
};
}

#endif
