//////////////////////////////////////////////////////////////////////////
// web体系全局服,用于控制全局单一登录(GS->Global Server)
// 重构成单线程的程序了
//////////////////////////////////////////////////////////////////////////
#ifndef __GS4Web_i_H__
#define __GS4Web_i_H__

#include <map>
using namespace std;

#include "PNGS/inc/pngs_def.h"
#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_TCPReceiver_def.h"
#include "PNGS/inc/pngs_packet_logic.h"

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whtransaction.h"
#include "WHCMN/inc/whcmd.h"

#include "WHNET/inc/whnetudpGLogger.h"

#include "../../Common/inc/tty_def_gs4web.h"
#include "../../Common/inc/tty_common_BD4Web.h"
#include "../../Common/inc/pngs_packet_web_extension.h"

#include "NotificationMngS.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class CNotificationMngS;
class GS4Web_i : public CMN::ILogic
{
// 为接口实现
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy(){delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		SureSend();
private:
	virtual	int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		Tick_BeforeDealCmdIn();
	virtual	int		Tick_AfterDealCmdIn();
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		int		nCmdQueueSize;							// 指令队列大小
		int		nStatisticOnlineInterval;				// 统计在线的时间长度
		int		nTQChunkSize;							// 时间队列的每次次性分配块大小
		int		nLoadNotificationTimeOut;				// 载入通知的超时
		int		nNotificationNum;						// 通知最大数目
		int		nQueryGroupInfoTimeOut;					// 请求大区信息的超时
		int		nWriteGroupInfoTimeOut;					// 写大区信息文件的超时
		char	szGroupInfoFileName[WH_MAX_PATH];		// 大区信息文件名
		char	szGDBAddr[WHNET_MAXADDRSTRLEN];			// GlobalDB服务器的地址
		int		nDealAdColonyTimeOut;					// 处理AdColony的超时

		CFGINFO_T()
		: nCmdQueueSize(20*1024*1024)
		, nStatisticOnlineInterval(5*60*1000)
		, nTQChunkSize(100)
		, nLoadNotificationTimeOut(10*1000)
		, nNotificationNum(1000)
		, nQueryGroupInfoTimeOut(1*60*1000)
		, nWriteGroupInfoTimeOut(1*60*1000)
		, nDealAdColonyTimeOut(2*1000)
		{
			WH_STRNCPY0(szGroupInfoFileName, "group_info.txt");
			WH_STRNCPY0(szGDBAddr, "127.0.0.1:4900");
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	}	m_cfginfo;
private:
	// 时间单元
	struct TQUNIT_T 
	{
		typedef	int		(GS4Web_i::*TEDEAL_T)(TQUNIT_T*);
		TEDEAL_T		tefunc;
		// 数据部分
		union
		{
			void*		ptr;
		}un;
	};
	// 自己用的
public:
	GS4Web_i();
	~GS4Web_i();
	int			Init_CFG(const char* cszCFGName);
	int			DealCmdIn_One_PNGS_TR2CD_CMD(const void* pData, int nDSize);
	// 处理来自业务受理系统的请求
	int			DealCmdFromBD4Web(const PNGSPACKET_BD4WEB_CMD_REQ_T* pCmd, size_t nSize);
private:
	//////////////////////////////////////////////////////////////////////////
	// 时间事件相关
	//////////////////////////////////////////////////////////////////////////
	int			Tick_DealTE();
	// 用于处理载入通知超时
	int			TEDeal_LoadingNotification_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	int			SetTE_LoadingNotification();
	// 用于处理请求大区信息超时
	int			TEDeal_QueryGroupInfo_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	int			SetTE_QueryGroupInfo();
	// 用于处理写大区信息文件超时
	int			TEDeal_WriteGroupInfo_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	int			SetTE_WriteGroupInfo();
	// 用于发起AdColony时间事件处理
	int			TEDeal_Deal_AdColony_TimeEvent_TimeOut(TQUNIT_T* pTQUnit);
	// 设置上面的超时
	int			SetTE_DealAdColonyTimeEvent();
	//////////////////////////////////////////////////////////////////////////
	// 其他
	//////////////////////////////////////////////////////////////////////////
	// 请求载入通知
	int			ReqLoadNotification();
	// 处理来自数据库的消息
	int			Tick_DealGDB4WebMsg();
	inline int	G_SendMsgToGroup(int nGroupID, void* pCmd, size_t nSize, const int* pExt)
	{
		switch (nGroupID)
		{
		case GROUP_ID_ALL:
		case GROUP_ID_ALL_WITHOUT_GLOBAL:
			{
				SendMsgToAllGroup(pCmd, nSize);
			}
			break;
		case GROUP_ID_GLOBAL:
			{

			}
			break;
		default:
			{
				if (IsGroupExist(nGroupID))
				{
					SendMsgToGroup(nGroupID, pCmd, nSize);
				}
				else
				{
					BD_CMD_GROUP_NOT_EXIST_RPL_T	Cmd;
					Cmd.nCmd		= BD_CMD_GROUP_NOT_EXIST_RPL;
					Cmd.nSize		= sizeof(Cmd);
					Cmd.nGroupID	= nGroupID;
					_SendBDRplCmd(pExt, (char*)&Cmd, sizeof(Cmd));
				}
			}
			break;
		}
		return 0;
	}
	inline int	SendMsgToGroup(int nGroupID, void* pCmd, size_t nSize)
	{
		map<int,int>::iterator	it	= m_mapGroupID2CntrID.find(nGroupID);
		if (it == m_mapGroupID2CntrID.end())
		{
			return -1;
		}

		PNGS_CD2TR_CMD_T		cmd;
		cmd.nConnecterID		= it->second;
		cmd.nDSize				= nSize;
		cmd.pData				= pCmd;
		CMN_LOGIC_CMDIN_AUTO(this, m_pLogicTCPReceiver, PNGS_CD2TR_CMD, cmd);

		return 0;
	}
	// 这个是给主线程向所有服务器发送消息,因为不会使用TR的m_queueCmdIn,所以不需要上锁
	inline int	SendMsgToAllGroup(void* pCmd, size_t nSize)
	{
		map<int,int>::iterator	it	= m_mapGroupID2CntrID.begin();
		for (; it != m_mapGroupID2CntrID.end(); ++it)
		{
			PNGS_CD2TR_CMD_T		cmd;
			cmd.nConnecterID		= it->second;
			cmd.nDSize				= nSize;
			cmd.pData				= pCmd;
			CMN_LOGIC_CMDIN_AUTO(this, m_pLogicTCPReceiver, PNGS_CD2TR_CMD, cmd);
		}

		return 0;
	}
	int		SendNotificationToGroup(int nGroupID);
private:
	typedef	CMN::ILogic			FATHERCLASS;

	// 底层TCP通讯器
	ILogicBase*			m_pLogicTCPReceiver;
	// 业务受理模块,可以为NULL
	ILogicBase*			m_pLogicBD4Web;
	time_t				m_nTimeNow;
	whtick_t			m_tickNow;
	// 用于组装多个指令的
	whvector<char>		m_vectrawbuf;
	whvector<int>		m_vectConnecters;					// 已连入的需要访问数据库的进程对应cntr id
	map<int,int>		m_mapCntrID2GroupID;				// 连接ID->Group ID
	map<int,int>		m_mapGroupID2CntrID;				// Group ID->连接ID
	whtimequeue			m_TQ;								// 时间队列
	bool				m_bLoadNotificationOK;				// 是否载入通知成功
	whtimequeue::ID_T	m_teidLoadNotification;				// 载入通知的时间事件
	map<int, int>		m_map4NeedToSendLoadingNotify;		// 载入通知中新到来的连接,group id->cntr id
	CNotificationMngS*	m_pNotificationMngS;

	tcpmsger::DATA_INI_INFO_T		m_msgerDB_info;
	tcpretrymsger<tcpmsger>			m_msgerDB;

	inline	void		MakeCntrIDnGroupIDMap(int nCntrID, int nGroupID)
	{
		map<int,int>::iterator	it		= m_mapCntrID2GroupID.find(nCntrID);
		if (it != m_mapCntrID2GroupID.end())
		{
			m_mapCntrID2GroupID.erase(nCntrID);
		}
		m_mapCntrID2GroupID.insert(map<int,int>::value_type(nCntrID, nGroupID));

		it								= m_mapGroupID2CntrID.find(nGroupID);
		if (it != m_mapGroupID2CntrID.end())
		{
			m_mapGroupID2CntrID.erase(nGroupID);
		}
		m_mapGroupID2CntrID.insert(map<int,int>::value_type(nGroupID, nCntrID));
	}
	inline	int		GetCntrIDByGroupID(int nGroupID)
	{
		map<int,int>::iterator	it		= m_mapGroupID2CntrID.find(nGroupID);
		if (it != m_mapGroupID2CntrID.end())
		{
			return it->second;
		}
		return -1;
	}
	inline int		GetGroupIDByCntrID(int nCntrID)
	{
		map<int,int>::iterator	it		= m_mapCntrID2GroupID.find(nCntrID);
		if (it != m_mapCntrID2GroupID.end())
		{
			return it->second;
		}
		return -1;
	}
	inline	void	RemoveCntrIDnGroupIDMapByCntrID(int nCntrID)
	{
		map<int,int>::iterator	it		= m_mapCntrID2GroupID.find(nCntrID);
		if (it != m_mapCntrID2GroupID.end())
		{
			m_mapGroupID2CntrID.erase(it->second);
			m_mapCntrID2GroupID.erase(it);
		}
	}
	// 是否存在连接ID对应的连接器
	inline	bool	IsConnectorExist(int nCntrID)
	{
		return m_mapCntrID2GroupID.find(nCntrID) != m_mapCntrID2GroupID.end();
	}
	// 这个大区是否已经连接上了
	inline	bool	IsGroupExist(int nGroupID)
	{
		return m_mapGroupID2CntrID.find(nGroupID) != m_mapGroupID2CntrID.end();
	}

	void	_SendBDRplCmd(const int* pExt, char* pData, size_t nDSize);
// 用于统计的服务器信息等
private:
	struct GGUnit_T 
	{
		struct TERM_UNIT_T
		{
			int		nTermType;
			int		nPlayerNum;
		};
		whvector<TERM_UNIT_T>	vectTermUnits;
		int			nPlayerTotalNum;
		int			nMaxPlayerNum;
		int			nCntrID;

		GGUnit_T()
		{
			Clear();
		}
		
		void	Clear()
		{
			vectTermUnits.clear();
			nPlayerTotalNum		= 0;
			nMaxPlayerNum		= 0;
			nCntrID				= -1;
		}
	};
	GGUnit_T		m_aGGs[TTY_MAX_GG_NUM];
};
}

#endif
