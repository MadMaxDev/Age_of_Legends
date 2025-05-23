//////////////////////////////////////////////////////////////////////////
//DBS4Web用于MMORPG的Web扩展数据库转存业务
//本身和TCPReceiver在同一个线程,但是本身会开启多个访问数据库的线程
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
//ILogic接口
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
	//自己的东西
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		int			nThreadNum;								//线程数量
		int			nCmdQueueSize;							//指令队列大小
		int			nCheckThreadActiveInterval;				//多久检查一次线程是否依然存活(单位ms)
		int			nFlushThreshold;						//连续发送多少个消息就强制tcpreceiver发送一次
		bool		bNeedAllCntrQuitThenStop;				//是否需要所有的连接都断开了程序才停止
		bool		bAccountLoginNeedCheckPass;				//是否需要校验密码(登录时)

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
//为多线程包装的接口
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
	//创建工作线程和线程存活校验
	void			CreateWorkerThread();
	void			CheckThreadActive();
	//mysql句柄
	MYSQL*			GetSelfMySQLHandle(whtid_t tid);		//获取已经申请的MYSQL句柄,如果没有或者不可用则返回NULL
	MYSQL*			BorrowMySQLHandle(whtid_t tid);
	void			ReturnMySQLHandle(whtid_t tid, MYSQL* pMYSQL);
	MYSQL*			GenMySQLHandle();
	int				InitGenAllSQLHandle();					//初始化的时候先生成所有的句柄
	//消息收发
	int				RecvMsg_MT(whvector<char>& vectBuf);
	int				SendMsg_MT(int iCntrID, const void *pData, int nDSize);
	static void*	_GDB4Web_Worker_ThreadProc(void* pParam);
	//消息处理
	int				ThreadDealCmd(void* pData, int nDSize, whtid_t tid, whvector<char>& vectRawBuf);
private:
	//底层TCP通讯器
	ILogicBase*			m_pLogicTCPReceiver;
	time_t				m_nTimeNow;						//本tick的时刻
	whtick_t			m_tickNow;
	//用于组装多个指令的缓冲
	whvector<char>		m_vectrawbuf;
	whlock				m_lockMySQLHandleOp;			//关于mysql句柄操作时使用的锁
	whlock				m_lockWriteLog;					//多线程时使用的logman是共享的,同一个logger在输出日志时会使用m_szFmtStrBuf做临时缓冲,导致冲突
	whvector<MYSQL*>	m_vectFreeMySQLHandle;			//可以直接使用的MYSQL句柄
	int					m_nCurMySQLHandleNum;			//当前使用的MYSQL句柄数量(m_vectFreeMySQLHandle.size()+m_nCurMySQLHandleNum<=nThreadNum)
	n_whcmn::whsmpqueueWL	m_queueCmdInMT;				//输入指令队列
	//n_whcmn::whsmpqueueWL	m_queueCmdOut;				//输出指令队列
	whvector<THREAD_INFO_T>	m_vectThreadInfo;			//线程Info
	map<whtid_t, MYSQL*>	m_mapTid2MySQLHandle;		//线程ID到mysql句柄的映射,这样在线程当掉之后也能回收句柄
	whlooper			m_wlCheckThreadActive;			//线程存活检验
	whvector<int>		m_vectConnecters;				//已连入的GZS对应cntr id
	bool				m_bStopping;					//现在准备停机
	whtick_t			m_tickStoppingBegin;			//停机开始的tick
};
}

#endif
