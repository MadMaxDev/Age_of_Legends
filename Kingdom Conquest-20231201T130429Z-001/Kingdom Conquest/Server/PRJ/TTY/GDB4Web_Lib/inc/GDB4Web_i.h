//////////////////////////////////////////////////////////////////////////
//DBS4Web����MMORPG��Web��չ���ݿ�ת��ҵ��
//�����TCPReceiver��ͬһ���߳�,���Ǳ���Ὺ������������ݿ���߳�
//////////////////////////////////////////////////////////////////////////
#ifndef __DBS4Web_i_H__
#define __DBS4Web_i_H__

#include <map>
using namespace std;

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whthread.h"
#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnettcp.h"
#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_TCPReceiver_def.h"
#include "PNGS/inc/pngs_packet_logic.h"
#include "DIA/inc/mt_mysql_connector.h"
#include "DIA/inc/st_mysql_query.h"
#include "../../Common/inc/tty_def_gs4web.h"
#include "../../Common/inc/pngs_packet_db4web_logic.h"
#include "../../Common/inc/tty_common_PlayerData4Web.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class GDB4Web_i : public CMN::ILogic
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
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect)	{return 0;}
	virtual	int		SureSend();
private:
	virtual	int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		Tick_BeforeDealCmdIn();
	virtual	int		Tick_AfterDealCmdIn();
	//�Լ��Ķ���
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		int			nThreadNum;								//�߳�����
		int			nCmdQueueSize;							//ָ����д�С
		int			nCheckThreadActiveInterval;				//��ü��һ���߳��Ƿ���Ȼ���(��λms)
		int			nFlushThreshold;						//�������Ͷ��ٸ���Ϣ��ǿ��tcpreceiver����һ��
		bool		bNeedAllCntrQuitThenStop;				//�Ƿ���Ҫ���е����Ӷ��Ͽ��˳����ֹͣ
		bool		bAccountLoginNeedCheckPass;				//�Ƿ���ҪУ������(��¼ʱ)

		struct MySQLInfo : public whdataini::obj 
		{
			char	szMysqlHost[dia_string_len];
			int		nMysqlPort;
			char	szMysqlUser[dia_string_len];
			char	szMysqlPass[dia_string_len];
			char	szMysqlDB[dia_string_len];
			char	szMysqlCharset[dia_string_len];
			char	szMysqlSocket[dia_string_len];

			MySQLInfo()
			{
				strcpy(szMysqlHost, "localhost");
				nMysqlPort		= 3306;
				strcpy(szMysqlUser, "root");
				strcpy(szMysqlPass, "pixeldb2006");
				strcpy(szMysqlDB, "xunxian");
				strcpy(szMysqlCharset, "gbk");
				strcpy(szMysqlSocket, "/tmp/mysql.sock");
			}
			WHDATAPROP_DECLARE_MAP(MySQLInfo);
		}	MYSQLInfo;

		CFGINFO_T()
			: nThreadNum(4)
			, nCmdQueueSize(20*1024*1024)
			, nCheckThreadActiveInterval(10*1000)
			, nFlushThreshold(100)
			, bNeedAllCntrQuitThenStop(false)
			, bAccountLoginNeedCheckPass(true)
		{}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	}	m_cfgInfo;
public:
	GDB4Web_i();
	virtual	~GDB4Web_i();
	int				Init_CFG(const char* cszCFGName);
	int				Init_DB();
	int				DealCmdIn_One_PNGS_TR2CD_CMD(const void* pData, int nDSize);
//Ϊ���̰߳�װ�Ľӿ�
private:
	struct THREAD_INFO_T
	{
		GDB4Web_i*		pHost;
		whtid_t			tid;

		THREAD_INFO_T()
		: pHost(NULL), tid(INVALID_TID)
		{
		}
		bool	operator == (const THREAD_INFO_T& t)
		{
			if (this == &t)
			{
				return true;
			}
			return (pHost==t.pHost) && (tid==t.tid); 
		}
	};
	//���������̺߳��̴߳��У��
	void			CreateWorkerThread();
	void			CheckThreadActive();
	//mysql���
	MYSQL*			GetSelfMySQLHandle(whtid_t tid);		//��ȡ�Ѿ������MYSQL���,���û�л��߲������򷵻�NULL
	MYSQL*			BorrowMySQLHandle(whtid_t tid);
	void			ReturnMySQLHandle(whtid_t tid, MYSQL* pMYSQL);
	MYSQL*			GenMySQLHandle();
	int				InitGenAllSQLHandle();					//��ʼ����ʱ�����������еľ��
	//��Ϣ�շ�
	int				RecvMsg_MT(whvector<char>& vectBuf);
	int				SendMsg_MT(int iCntrID, const void *pData, int nDSize);
	static void*	_GDB4Web_Worker_ThreadProc(void* pParam);
	//��Ϣ����
	int				ThreadDealCmd(void* pData, int nDSize, whtid_t tid, whvector<char>& vectRawBuf);
private:
	//�ײ�TCPͨѶ��
	ILogicBase*			m_pLogicTCPReceiver;
	time_t				m_nTimeNow;						//��tick��ʱ��
	whtick_t			m_tickNow;
	//������װ���ָ��Ļ���
	whvector<char>		m_vectrawbuf;
	whlock				m_lockMySQLHandleOp;			//����mysql�������ʱʹ�õ���
	whlock				m_lockWriteLog;					//���߳�ʱʹ�õ�logman�ǹ����,ͬһ��logger�������־ʱ��ʹ��m_szFmtStrBuf����ʱ����,���³�ͻ
	whvector<MYSQL*>	m_vectFreeMySQLHandle;			//����ֱ��ʹ�õ�MYSQL���
	int					m_nCurMySQLHandleNum;			//��ǰʹ�õ�MYSQL�������(m_vectFreeMySQLHandle.size()+m_nCurMySQLHandleNum<=nThreadNum)
	n_whcmn::whsmpqueueWL	m_queueCmdInMT;				//����ָ�����
	//n_whcmn::whsmpqueueWL	m_queueCmdOut;				//���ָ�����
	whvector<THREAD_INFO_T>	m_vectThreadInfo;			//�߳�Info
	map<whtid_t, MYSQL*>	m_mapTid2MySQLHandle;		//�߳�ID��mysql�����ӳ��,�������̵߳���֮��Ҳ�ܻ��վ��
	whlooper			m_wlCheckThreadActive;			//�̴߳�����
	whvector<int>		m_vectConnecters;				//�������GZS��Ӧcntr id
	bool				m_bStopping;					//����׼��ͣ��
	whtick_t			m_tickStoppingBegin;			//ͣ����ʼ��tick
};
}

#endif
