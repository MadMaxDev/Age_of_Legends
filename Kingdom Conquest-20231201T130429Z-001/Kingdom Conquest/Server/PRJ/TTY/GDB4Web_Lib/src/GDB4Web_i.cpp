#include "../inc/GDB4Web_i.h"
#include <errno.h>

using namespace n_pngs;

WHDATAPROP_MAP_BEGIN_AT_ROOT(GDB4Web_i::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nThreadNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCmdQueueSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nCheckThreadActiveInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nFlushThreshold, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bNeedAllCntrQuitThenStop, 0)
	WHDATAPROP_ON_SETVALUE_smp(unknowntype, MYSQLInfo, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bAccountLoginNeedCheckPass, 0)
WHDATAPROP_MAP_END()

WHDATAPROP_MAP_BEGIN_AT_ROOT(GDB4Web_i::CFGINFO_T::MySQLInfo)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlHost, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMysqlPort, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlUser, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlPass, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlDB, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlCharset, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlSocket, 0)
WHDATAPROP_MAP_END()

int		GDB4Web_i::Organize()
{
	//查找TCPReceiver并设置自己为它的使用者
	m_pLogicTCPReceiver			= m_pCMN->GetLogic(PNGS_DLL_NAME_TCPRECEIVER, TCPRECEIVER_VER);
	if (m_pLogicTCPReceiver == NULL)
	{
		return -1;
	}
	PNGS_CD2TR_CONFIG_T	cfg;
	cfg.nSubCmd					= PNGS_CD2TR_CONFIG_T::SUBCMD_SET_CMDDEALER;
	if (m_pLogicTCPReceiver->CmdIn_Instant(this, PNGS_CD2TR_CONFIG, &cfg, sizeof(cfg)) < 0)
	{
		return -2;
	}
	return 0;
}

int		GDB4Web_i::Detach(bool bQuick)
{
	return 0;
}

GDB4Web_i::GDB4Web_i()
: m_pLogicTCPReceiver(NULL)
, m_nCurMySQLHandleNum(0)
, m_bStopping(false)
, m_tickStoppingBegin(0)
{
	
}

GDB4Web_i::~GDB4Web_i()
{
	//Release();
}

int		GDB4Web_i::Init(const char *cszCFGName)
{
	m_nTimeNow		= wh_time();
	m_tickNow		= wh_gettickcount();
	
	m_vectConnecters.reserve(20);

	int		iRst	= 0;
	if ((iRst=Init_CFG(cszCFGName)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_INT_RLS)"GDB4Web_i::Init,Init_CFG,%d", iRst);
		return -1;
	}

	//必须在创建线程之前先预留这么大的位置出来,不然会导致线程使用的pThreadInfo指针失效
	m_vectFreeMySQLHandle.reserve(m_cfgInfo.nThreadNum);
	m_vectThreadInfo.reserve(m_cfgInfo.nThreadNum);

	if ((iRst=m_queueCmdInMT.Init(m_cfgInfo.nCmdQueueSize)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_INT_RLS)"GDB4Web_i::Init,%d,m_queueCmdInMT.Init,%d", iRst, m_cfgInfo.nCmdQueueSize);
		return -2;
	}

	if ((iRst=ILogicBase::QueueInit(m_cfgInfo.nCmdQueueSize)) < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_INT_RLS)"GDB4Web_i::Init,%d,QueueInit,%d", iRst, m_cfgInfo.nCmdQueueSize);
		return -3;
	}

	if (InitGenAllSQLHandle() < 0)
	{
		return -4;
	}

	m_wlCheckThreadActive.setinterval(m_cfgInfo.nCheckThreadActiveInterval);

	for (int i=0; i<m_cfgInfo.nThreadNum; i++)
	{
		CreateWorkerThread();
	}

	if (Init_DB() < 0)
	{
		return -11;
	}
	return 0;
}

int		GDB4Web_i::Init_CFG(const char *cszCFGName)
{
	//分析文件
	WHDATAINI_CMN		ini;
	ini.addobj("GDB4Web", &m_cfgInfo);
	int		iRst		= ini.analyzefile(cszCFGName);
	if (iRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_INT_RLS)"GDB4Web_i::Init_CFG,%d,ini.analyzefile,%s", iRst, ini.printerrreport());
		return -1;
	}

	return 0;
}
int		GDB4Web_i::Init_DB()
{
	MYSQL*	pMySQL	= m_vectFreeMySQLHandle.getlast();
	if (pMySQL == NULL)
	{
		return -1;
	}
	dia_mysql_query q(pMySQL, QUERY_LEN);
	if (!q.Connected())
	{
		return -2;
	}
	q.SpawnQuery("call gc_init_db(0,0,%s)", SP_RESULT);
	int	nPreRet		= 0;
	q.ExecuteSPWithResult(nPreRet);
	if (nPreRet != MYSQL_QUERY_NORMAL)
	{
		return -3;
	}
	else
	{
		if (q.FetchRow())
		{
			int nRst = q.GetVal_32();
			if (nRst != 0)
			{
				return -4;
			}
		}
		else
		{
			return -5;
		}
	}
	return 0;
}

int		GDB4Web_i::Init_CFG_Reload(const char *cszCFGName)
{
	//分析文件
	WHDATAINI_CMN		ini;
	ini.addobj("GDB4Web", &m_cfgInfo);
	int		iRst		= ini.analyzefile(cszCFGName, false, 1);
	if (iRst < 0)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_INT_RLS)"GDB4Web_i::Init_CFG_Reload,%d,ini.analyzefile,%s", iRst, ini.printerrreport());
		return -1;
	}

	return 0;
}

int		GDB4Web_i::Release()
{
	map<whtid_t, MYSQL*>::iterator	it		= m_mapTid2MySQLHandle.begin();
	while (it != m_mapTid2MySQLHandle.end())
	{
		mysql_close(it->second);
		++it;
	}
	for (unsigned int i=0; i<m_vectFreeMySQLHandle.size(); i++)
	{
		mysql_close(m_vectFreeMySQLHandle[i]);
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_RT)"Main thread release, m_mapTid2MySQLHandle.size():%d,m_nCurMySQLHandleNum:%d,m_vectFreeMySQLHandle.size():%d", 
		m_mapTid2MySQLHandle.size(), m_nCurMySQLHandleNum, m_vectFreeMySQLHandle.size());
	m_mapTid2MySQLHandle.clear();
	m_vectFreeMySQLHandle.clear();
	return 0;
}

int		GDB4Web_i::SureSend()
{
	return 0;
}

int		GDB4Web_i::Tick_BeforeDealCmdIn()
{
	if (m_nMood == CMN::ILogic::MOOD_STOPPED)
	{
		return 0;
	}
	if (m_wlCheckThreadActive.check())
	{
		m_wlCheckThreadActive.reset();
		CheckThreadActive();
	}
	return 0;
}

int		GDB4Web_i::Tick_AfterDealCmdIn()
{
	if (m_bStopping)
	{	
		if ((!m_cfgInfo.bNeedAllCntrQuitThenStop || m_vectConnecters.size()==0) && m_queueCmdInMT.GetSize()==0 && m_nMood!=CMN::ILogic::MOOD_STOPPED)
		{
			m_nMood				= CMN::ILogic::MOOD_STOPPED;
			GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_RT)"GDB4Web_i::DealCmdIn_One,end now,t1:%d,t2:%d", m_tickStoppingBegin, wh_gettickcount());
		}
	}
	return 0;
}

int		GDB4Web_i::SendMsg_MT(int iCntrID, const void *pData, int nDSize)
{	
	PNGS_CD2TR_CMD_T*	pCmd	= NULL;
	if( m_pLogicTCPReceiver->CmdIn_MT_lock(this, PNGS_CD2TR_CMD, (void **)&pCmd, nDSize + sizeof(PNGS_CD2TR_CMD_T)) < 0)
	{
		return	-1;
	}
	// 拷贝全部数据过去
	pCmd->nConnecterID		= iCntrID;
	pCmd->nDSize			= nDSize;
	// 重新设置pData指向的地址
	wh_settypedptr(pCmd->pData, pCmd+1);
	// 拷贝数据部分
	memcpy((void *)pCmd->pData, pData, nDSize);
	m_pLogicTCPReceiver->CmdIn_MT_unlock();

	return 0;
}

int		GDB4Web_i::RecvMsg_MT(whvector<char>& vectBuf)
{
	m_queueCmdInMT.lock();
	size_t	nSize	= m_queueCmdInMT.whsmpqueue::PeekSize();
	vectBuf.resize(nSize);
	m_queueCmdInMT.whsmpqueue::Out(vectBuf.getbuf(), &nSize);
	m_queueCmdInMT.unlock();
	return nSize;
}

void	GDB4Web_i::CreateWorkerThread()
{
//	whtid_t			tid;
	THREAD_INFO_T*	pInfo		= m_vectThreadInfo.push_back();
	pInfo->pHost				= this;
	//!!!m_vectThreadInfo在Init的时候reserve了nThreadNum个空间,所以能保证pInfo在_DBS4Web_Worker_ThreadProc线程的生命周期内是有效的
	//如果开的线程超过了nThreadNum个,那么不能保证pInfo是有效的,切记!!!
	if (whthread_create(&pInfo->tid, _GDB4Web_Worker_ThreadProc, pInfo) < 0)
	{
		m_vectThreadInfo.erasevalue(*pInfo);
		return;
	}
//	pInfo->tid					= tid;
}

void	GDB4Web_i::CheckThreadActive()
{
	whvector<THREAD_INFO_T>	vectTmp;
	for (unsigned int i=0; i<m_vectThreadInfo.size(); i++)
	{
		whtid_t		tid		= m_vectThreadInfo[i].tid;
		if (whthread_isend(tid))		//线程已经失效了
		{
			map<whtid_t, MYSQL*>::iterator	it	= m_mapTid2MySQLHandle.find(tid);
			if (it != m_mapTid2MySQLHandle.end())
			{
				ReturnMySQLHandle(tid, it->second);			//回收MYSQL句柄
				m_mapTid2MySQLHandle.erase(it);
			}
			vectTmp.push_back(vectTmp[i]);
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_RT)"thread not active,thread id:0x%X, status:%d", tid, m_nMood);
		}
	}

	for (unsigned int i=0; i<vectTmp.size(); i++)
	{
		m_vectThreadInfo.erasevalue(vectTmp[i]);
		CreateWorkerThread();
	}
}

MYSQL*	GDB4Web_i::GetSelfMySQLHandle(whtid_t tid)
{
	whlockmutex		tmpLock(&m_lockMySQLHandleOp);	//会自动加锁或者释放
	MYSQL*	pMySQL	= NULL;
	map<whtid_t, MYSQL*>::iterator it = m_mapTid2MySQLHandle.find(tid);
	if (it != m_mapTid2MySQLHandle.end())
	{
		pMySQL		= it->second;
		if (mysql_ping(pMySQL) != 0)
		{
			mysql_close(pMySQL);
			m_mapTid2MySQLHandle.erase(it);
			m_nCurMySQLHandleNum--;
			pMySQL	= NULL;
		}
	}
	return pMySQL;
}

MYSQL*	GDB4Web_i::BorrowMySQLHandle(whtid_t tid)
{
	whlockmutex		tmpLock(&m_lockMySQLHandleOp);	//会自动加锁或者释放
	MYSQL*	pMySQL	= NULL;

	while (m_vectFreeMySQLHandle.size() > 0)		//尝试直到可用句柄都不能用了
	{
		pMySQL		= m_vectFreeMySQLHandle.getlast();
		m_vectFreeMySQLHandle.pop_back();
		if (mysql_ping(pMySQL) != 0)				//得确认下是否依然能用,如果不能用了就产生个新的
		{
			mysql_close(pMySQL);
			//pMySQL	= GenMySQLHandle();
			pMySQL	= NULL;
		}
		if (pMySQL != NULL)
		{
			m_mapTid2MySQLHandle.insert(map<whtid_t, MYSQL*>::value_type(tid, pMySQL));
			m_nCurMySQLHandleNum++;
			return pMySQL;
		}
	}
	if (m_nCurMySQLHandleNum < m_cfgInfo.nThreadNum)
	{
		pMySQL		= GenMySQLHandle();
		if (pMySQL != NULL)
		{
			m_mapTid2MySQLHandle.insert(map<whtid_t, MYSQL*>::value_type(tid, pMySQL));
			m_nCurMySQLHandleNum++;
			return pMySQL;
		}
	}
	return NULL;
}

void	GDB4Web_i::ReturnMySQLHandle(whtid_t tid, MYSQL* pMYSQL)
{
	whlockmutex		tmpLock(&m_lockMySQLHandleOp);	//会自动加锁或者释放
	m_mapTid2MySQLHandle.erase(tid);
	if (mysql_ping(pMYSQL) == 0)
	{
		m_vectFreeMySQLHandle.push_back(pMYSQL);
	}
	else
	{
		mysql_close(pMYSQL);						//不能用的就直接关闭了
	}
	m_nCurMySQLHandleNum--;
}

MYSQL*	GDB4Web_i::GenMySQLHandle()
{
	MYSQL*	pMySQL			= mysql_init(NULL);
	int		nTimeoutOption	= 1;
	mysql_options(pMySQL, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&nTimeoutOption);
	
	CFGINFO_T::MySQLInfo&	mysqlInfo	= m_cfgInfo.MYSQLInfo;
	unsigned long			mysqlFlag	= CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS;
	if (NULL == mysql_real_connect(pMySQL, mysqlInfo.szMysqlHost, mysqlInfo.szMysqlUser, mysqlInfo.szMysqlPass, 
		mysqlInfo.szMysqlDB, mysqlInfo.nMysqlPort, mysqlInfo.szMysqlSocket, mysqlFlag))
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1358,DBS4Web_MYSQL)"%s,,mysql_real_connect,%s,%s,%s,%s,%d,%s,%d,%s,%s",
			__FUNCTION__, mysqlInfo.szMysqlHost, mysqlInfo.szMysqlUser, mysqlInfo.szMysqlPass, mysqlInfo.szMysqlDB, mysqlInfo.nMysqlPort, mysqlInfo.szMysqlSocket, mysqlFlag, mysqlInfo.szMysqlCharset, mysql_error(pMySQL));
		mysql_close(pMySQL);
		return NULL;
	}
	if (0 != mysql_set_character_set(pMySQL, mysqlInfo.szMysqlCharset))
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(1358,DBS4Web_MYSQL)"%s,,mysql_set_character_set,%s,%s,%s,%s,%d,%s,%d,%s,%s",
			__FUNCTION__, mysqlInfo.szMysqlHost, mysqlInfo.szMysqlUser, mysqlInfo.szMysqlPass, mysqlInfo.szMysqlDB, mysqlInfo.nMysqlPort, mysqlInfo.szMysqlSocket, mysqlFlag, mysqlInfo.szMysqlCharset, mysql_error(pMySQL));
		mysql_close(pMySQL);
		return NULL;
	}
	GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_CMN, GLGR_STD_HDR(1358,DBS4Web_MYSQL)"%s,mysql connect ok,,%s,%s,%s,%s,%d,%s,%d,%s", __FUNCTION__, mysqlInfo.szMysqlHost, mysqlInfo.szMysqlUser, mysqlInfo.szMysqlPass, mysqlInfo.szMysqlDB, mysqlInfo.nMysqlPort, mysqlInfo.szMysqlSocket, mysqlFlag, mysqlInfo.szMysqlCharset);

	return pMySQL;
}

void*	GDB4Web_i::_GDB4Web_Worker_ThreadProc(void* pParam)
{
	whvector<char>	vectMsgBuf;
	whvector<char>	vectRawBuf;		//用于底层组装指令的
	vectMsgBuf.reserve(4*1024);		//先预留4k
	vectRawBuf.reserve(4*1024);		//先预留4k
	THREAD_INFO_T*	pThreadInfo		= (THREAD_INFO_T*)pParam;
	GDB4Web_i*	pHost				= pThreadInfo->pHost;			// pHost一定是有效的,在创建线程之前已经赋值好了
	whtid_t&	tid					= pThreadInfo->tid;				// tid不一定立即有效,在创建线程成功之后,可能在一个极短的时间内是无效的
	while ((pThreadInfo->tid==INVALID_TID) && (pHost->m_nMood!=CMN::ILogic::MOOD_STOPPED))	//获得pid信息之后才能正常工作
	{
		wh_sleep(10);
	}
	{
		whlockmutex		tmpLock(&pHost->m_lockWriteLog);
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_RT)"thread start to work,threadID:0x%X,status:%d,pThreadInfo:0x%X,host:0x%X", tid, pHost->m_nMood, pThreadInfo, pHost);
	}
	while (pHost->m_nMood != CMN::ILogic::MOOD_STOPPED)
	{
		if (pHost->RecvMsg_MT(vectMsgBuf) > 0)	//代表有数据
		{
			pHost->ThreadDealCmd(vectMsgBuf.getbuf(), vectMsgBuf.size(), tid, vectRawBuf);
		}
		else
		{
			wh_sleep(10);
		}
	}
	{
		whlockmutex		tmpLock(&pHost->m_lockWriteLog);
		GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_RT)"thread end normal,threadID:0x%X,status:%d,pThreadInfo:0x%X,host:0x%X", tid, pHost->m_nMood, pThreadInfo, pHost);
	}
	return NULL;
}

int		GDB4Web_i::InitGenAllSQLHandle()
{
	int		iNum	= 0;
	for (int i=0; i<m_cfgInfo.nThreadNum; i++)
	{
		MYSQL*	pMySQL	= GenMySQLHandle();
		if (pMySQL != NULL)
		{
			m_vectFreeMySQLHandle.push_back(pMySQL);
			iNum++;
		}
		else
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,GDB4Web_INT_RLS)"%s,,gen sql handle failed", __FUNCTION__);
		}
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, GLGR_STD_HDR(,GDB4Web_INT_RLS)"%s,,gen sql handle,num:%d", __FUNCTION__, iNum);
	return iNum==0?-1:0;			//一个句柄都没有算失败
}
