//////////////////////////////////////////////////////////////////////////
// web��ϵȫ�ַ�,���ڿ���ȫ�ֵ�һ��¼(GS->Global Server)
// �ع��ɵ��̵߳ĳ�����
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
// Ϊ�ӿ�ʵ��
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
		int		nCmdQueueSize;							// ָ����д�С
		int		nStatisticOnlineInterval;				// ͳ�����ߵ�ʱ�䳤��
		int		nTQChunkSize;							// ʱ����е�ÿ�δ��Է�����С
		int		nLoadNotificationTimeOut;				// ����֪ͨ�ĳ�ʱ
		int		nNotificationNum;						// ֪ͨ�����Ŀ
		int		nQueryGroupInfoTimeOut;					// ���������Ϣ�ĳ�ʱ
		int		nWriteGroupInfoTimeOut;					// д������Ϣ�ļ��ĳ�ʱ
		char	szGroupInfoFileName[WH_MAX_PATH];		// ������Ϣ�ļ���
		char	szGDBAddr[WHNET_MAXADDRSTRLEN];			// GlobalDB�������ĵ�ַ
		int		nDealAdColonyTimeOut;					// ����AdColony�ĳ�ʱ

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
	// ʱ�䵥Ԫ
	struct TQUNIT_T 
	{
		typedef	int		(GS4Web_i::*TEDEAL_T)(TQUNIT_T*);
		TEDEAL_T		tefunc;
		// ���ݲ���
		union
		{
			void*		ptr;
		}un;
	};
	// �Լ��õ�
public:
	GS4Web_i();
	~GS4Web_i();
	int			Init_CFG(const char* cszCFGName);
	int			DealCmdIn_One_PNGS_TR2CD_CMD(const void* pData, int nDSize);
	// ��������ҵ������ϵͳ������
	int			DealCmdFromBD4Web(const PNGSPACKET_BD4WEB_CMD_REQ_T* pCmd, size_t nSize);
private:
	//////////////////////////////////////////////////////////////////////////
	// ʱ���¼����
	//////////////////////////////////////////////////////////////////////////
	int			Tick_DealTE();
	// ���ڴ�������֪ͨ��ʱ
	int			TEDeal_LoadingNotification_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	int			SetTE_LoadingNotification();
	// ���ڴ������������Ϣ��ʱ
	int			TEDeal_QueryGroupInfo_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	int			SetTE_QueryGroupInfo();
	// ���ڴ���д������Ϣ�ļ���ʱ
	int			TEDeal_WriteGroupInfo_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	int			SetTE_WriteGroupInfo();
	// ���ڷ���AdColonyʱ���¼�����
	int			TEDeal_Deal_AdColony_TimeEvent_TimeOut(TQUNIT_T* pTQUnit);
	// ��������ĳ�ʱ
	int			SetTE_DealAdColonyTimeEvent();
	//////////////////////////////////////////////////////////////////////////
	// ����
	//////////////////////////////////////////////////////////////////////////
	// ��������֪ͨ
	int			ReqLoadNotification();
	// �����������ݿ����Ϣ
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
	// ����Ǹ����߳������з�����������Ϣ,��Ϊ����ʹ��TR��m_queueCmdIn,���Բ���Ҫ����
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

	// �ײ�TCPͨѶ��
	ILogicBase*			m_pLogicTCPReceiver;
	// ҵ������ģ��,����ΪNULL
	ILogicBase*			m_pLogicBD4Web;
	time_t				m_nTimeNow;
	whtick_t			m_tickNow;
	// ������װ���ָ���
	whvector<char>		m_vectrawbuf;
	whvector<int>		m_vectConnecters;					// ���������Ҫ�������ݿ�Ľ��̶�Ӧcntr id
	map<int,int>		m_mapCntrID2GroupID;				// ����ID->Group ID
	map<int,int>		m_mapGroupID2CntrID;				// Group ID->����ID
	whtimequeue			m_TQ;								// ʱ�����
	bool				m_bLoadNotificationOK;				// �Ƿ�����֪ͨ�ɹ�
	whtimequeue::ID_T	m_teidLoadNotification;				// ����֪ͨ��ʱ���¼�
	map<int, int>		m_map4NeedToSendLoadingNotify;		// ����֪ͨ���µ���������,group id->cntr id
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
	// �Ƿ��������ID��Ӧ��������
	inline	bool	IsConnectorExist(int nCntrID)
	{
		return m_mapCntrID2GroupID.find(nCntrID) != m_mapCntrID2GroupID.end();
	}
	// ��������Ƿ��Ѿ���������
	inline	bool	IsGroupExist(int nGroupID)
	{
		return m_mapGroupID2CntrID.find(nGroupID) != m_mapGroupID2CntrID.end();
	}

	void	_SendBDRplCmd(const int* pExt, char* pData, size_t nDSize);
// ����ͳ�Ƶķ�������Ϣ��
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
