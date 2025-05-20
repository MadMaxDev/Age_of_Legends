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
#include "WHCMN/inc/whcmd.h"
#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whnettcp.h"
#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_TCPReceiver_def.h"
#include "PNGS/inc/pngs_packet_logic.h"
#include "DIA/inc/mt_mysql_connector.h"
#include "DIA/inc/st_mysql_query.h"
#include "../../Common/inc/tty_def_dbs4web.h"
#include "../../Common/inc/pngs_packet_db4web_logic.h"
#include "../../Common/inc/tty_common_PlayerData4Web.h"
#include "../../Common/inc/tty_lp_client_gamecmd_packet.h"
#include "../../Common/inc/tty_common_excel_def.h"
#include "../../Common/inc/tty_common_db_timeevent.h"
#include "../../Common/inc/pngs_packet_lp_logic.h"

#include "../../Common/inc/tty_game_def.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class DBS4Web_i : public CMN::ILogic
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
		int			nSvrGrpID;								// 大区ID
		int			nThreadNum;								//线程数量
		int			nCmdQueueSize;							//指令队列大小
		int			nCheckThreadActiveInterval;				//多久检查一次线程是否依然存活(单位ms)
		int			nFlushThreshold;						//连续发送多少个消息就强制tcpreceiver发送一次
		bool		bNeedAllCntrQuitThenStop;				//是否需要所有的连接都断开了程序才停止

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
			: nSvrGrpID(0)
			, nThreadNum(4)
			, nCmdQueueSize(20*1024*1024)
			, nCheckThreadActiveInterval(10*1000)
			, nFlushThreshold(100)
			, bNeedAllCntrQuitThenStop(false)
		{}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	}	m_cfgInfo;
public:
	DBS4Web_i();
	virtual	~DBS4Web_i();
	int				Init_CFG(const char* cszCFGName);
	int				Init_DB();
	int				DealCmdIn_One_PNGS_TR2CD_CMD(const void* pData, int nDSize);
//为多线程包装的接口
private:
	struct THREAD_INFO_T
	{
		DBS4Web_i*		pHost;
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
	static void*	_DBS4Web_Worker_ThreadProc(void* pParam);
	//消息处理
	int				ThreadDealCmd(void* pData, int nDSize, whtid_t tid, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_AlliInstance(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_PositionMark(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_Arena(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_Item(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_Quest(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_Instance(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_Combat(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_Hero(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_Sync(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_WorldGoldMine(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
	int				ThreadDealCmd_FixHour(MYSQL* pMySQL, PNGS_TR2CD_CMD_T*	pCDCmd, whvector<char>& vectRawBuf);
public:
	void			PushHeroSimpleData(int nCntrID, dia_mysql_query& q, unsigned int nCombatType, tty_id_t nExtID, const char* cszHeroIDs, unsigned int nReason); // ,分隔的hero_id串
	void			PushDrug(int nCntrID, dia_mysql_query& q, const char* cszAccountIDs); // ,分隔的account_id串
	void			PushCharAtb(int nCntrID, dia_mysql_query& q, const char* cszAccountIDs, bool bSimple);
	void			PushMail(int nCntrID, dia_mysql_query& q, const char* cszAccountIDnMailIDs);
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
	whvector<THREAD_INFO_T>	m_vectThreadInfo;			//线程Info
	map<whtid_t, MYSQL*>	m_mapTid2MySQLHandle;		//线程ID到mysql句柄的映射,这样在线程当掉之后也能回收句柄
	whlooper			m_wlCheckThreadActive;			//线程存活检验
	bool				m_bStopping;					//现在准备停机
	whtick_t			m_tickStoppingBegin;			//停机开始的tick
	int					m_nLPCntrID;					//LP的连接ID,因为只有LP会访问大区数据库,其他的都通过LP转交数据库请求
};
}

// 字符计数
int dbs_wc(const char *src, char tag);
// 字符分割
int dbs_splitbychar(const char *src, char tag, int &idx, char *dst);


#endif
