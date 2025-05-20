// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetGenericSvrCli_SMPTCP.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 基于简单的TCP传输的whnetGenericSvrCli的实现
// CreationDate : 2005-03-04
// Change LOG   :

#include "../inc/whnetGenericSvrCli_SMPTCP.h"
#include "../inc/whnetudpGLogger.h"

using namespace n_whnet;

////////////////////////////////////////////////////////////////////
// whnetGenericSvrCli_SMTCPSVR
////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(whnetGenericSvrCli_SMTCPSVR::CNTRSVRINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxNum, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nPort, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szIP, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szAllowDeny, 0)
WHDATAPROP_MAP_END()
WHDATAPROP_MAP_BEGIN_AT_ROOT(whnetGenericSvrCli_SMTCPSVR::MSGERINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nSendBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nRecvBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(short, nBegin, 0)
	WHDATAPROP_ON_SETVALUE_smp(short, nEnd, 0)
WHDATAPROP_MAP_END()
WHDATAPROP_MAP_BEGIN_AT_ROOT(whnetGenericSvrCli_SMTCPSVR::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(unknowntype, MSGERINFO, 0)
	WHDATAPROP_ON_SETVALUE_smp(unknowntype, CNTRSVRINFO, 0)
WHDATAPROP_MAP_END()
whnetGenericSvrCli_SMTCPSVR::whnetGenericSvrCli_SMTCPSVR()
: m_nStatus(SVRSTATUS_NOTHING)
{
	// 注册指令
	WHCMN_CMD_REGANDDEAL_REGCMD(SVRCMD_REQ_STARTSERVER, Cmd_SVRCMD_REQ_STARTSERVER);
	WHCMN_CMD_REGANDDEAL_REGCMD(SVRCMD_REQ_STOPSERVER, Cmd_SVRCMD_REQ_STOPSERVER);
	WHCMN_CMD_REGANDDEAL_REGCMD(SVRCMD_REQ_SENDDATA, Cmd_SVRCMD_REQ_SENDDATA);
	WHCMN_CMD_REGANDDEAL_REGCMD(SVRCMD_REQ_DISCONNECTCLIENT, Cmd_SVRCMD_REQ_DISCONNECTCLIENT);
	WHCMN_CMD_REGANDDEAL_REGCMD(SVRCMD_REQ_SETCLIENTTAG, Cmd_SVRCMD_REQ_SETCLIENTTAG);

	m_vectTmpBuf.reserve(100000);
	m_vectConnecterToRemove.reserve(32);
}
whnetGenericSvrCli_SMTCPSVR::~whnetGenericSvrCli_SMTCPSVR()
{
}
int	whnetGenericSvrCli_SMTCPSVR::Init(const char *cszCFG)
{
	WHDATAINI_CMN	ini;
	ini.addobj("TCP", &m_cfginfo);
	int	rst = ini.analyzefile(cszCFG);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetGenericSvrCli_SMTCPSVR::Init,%d,ini.analyzefile,%s %s", rst, cszCFG, ini.printerrreport());
		return	-1;
	}

	// 这个一定要设置为非自动线程的
	m_cfginfo.MSGERINFO.bAutoStartThread	= false;

	return	0;
}
int	whnetGenericSvrCli_SMTCPSVR::Release()
{
	// 停止服务器(保险起见)
	TCPConnecterServer::Release();
	return	0;
}
int	whnetGenericSvrCli_SMTCPSVR::DoSelect(int nInterval)
{
	TCPConnecterServer::DoSelect(nInterval);
	return	0;
}
int	whnetGenericSvrCli_SMTCPSVR::Tick()
{
	TCPConnecterServer::DoWork(true);
	// 删除需要删除的Connecter
	while( m_vectConnecterToRemove.size()>0 )
	{
		RemoveConnecter(m_vectConnecterToRemove.getlast());
		m_vectConnecterToRemove.pop_back();
	}
	return	0;
}
int	whnetGenericSvrCli_SMTCPSVR::CmdIn(int nCmd, const void *pData, size_t nDSize)
{
	return	m_rad.CallFunc(nCmd, pData, nDSize);
}
Connecter *	whnetGenericSvrCli_SMTCPSVR::NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
{
	I_Connecter	*pConnecter	= new I_Connecter(this);
	if( pConnecter )
	{
		// 初始化Connecter
		I_Connecter::INFO_T	info;
		memcpy(&info, m_cfginfo.MSGERINFO.GetBase(), sizeof(info));
		info.msgerINFO.sock	= sock;
		if( pConnecter->Init(&info)<0 )
		{
			delete	pConnecter;
			pConnecter	= NULL;
		}
		else
		{
			pConnecter->m_addr	= *paddr;
		}
	}
	return		pConnecter;
}
void	whnetGenericSvrCli_SMTCPSVR::AfterAddConnecter(Connecter * pCntr)
{
	I_Connecter	*pConnecter	= (I_Connecter *)pCntr->QueryInterface();
	if( !pConnecter )
	{
		//说明是自己，不用后续工作了
		return;
	}
	// 通知上层用户连入了
	SVRCMD_RPL_CLIENTIN_T	ClientIn;
	ClientIn.nClientID		= pCntr->GetIDInMan();
	ClientIn.addr			= pConnecter->m_addr;
	RPLCmdCall(SVRCMD_RPL_CLIENTIN, &ClientIn, sizeof(ClientIn));
}
void	whnetGenericSvrCli_SMTCPSVR::BeforeDeleteConnecter(int nCntrID, CONNECTOR_INFO_T *pCntrInfo)
{
	// 通知上层用户离开了
	SVRCMD_RPL_CLIENTOUT_T	ClientOut;
	ClientOut.nClientID		= nCntrID;
	RPLCmdCall(SVRCMD_RPL_CLIENTOUT, &ClientOut, sizeof(ClientOut));
}
bool	whnetGenericSvrCli_SMTCPSVR::CanDealMsg() const
{
	return	true;
}
int		whnetGenericSvrCli_SMTCPSVR::DealMsg(int nIDInMan, const void *pData, size_t nSize)
{
	// 转换为SVRCMD_RPL_RECVDATA
	m_vectTmpBuf.resize( wh_offsetof(SVRCMD_RPL_RECVDATA_T, szData) + nSize );
	SVRCMD_RPL_RECVDATA_T	*pRecv	= (SVRCMD_RPL_RECVDATA_T *)m_vectTmpBuf.getbuf();
	pRecv->nClientID		= nIDInMan;
	memcpy(pRecv->szData, pData, nSize);
	return	RPLCmdCall(SVRCMD_RPL_RECVDATA, m_vectTmpBuf.getbuf(), m_vectTmpBuf.size());
}

int	whnetGenericSvrCli_SMTCPSVR::Cmd_SVRCMD_REQ_STARTSERVER(const void *pData, size_t nDSize)
{
	SVRCMD_REQ_STARTSERVER_T	*pCmd	= (SVRCMD_REQ_STARTSERVER_T *)pData;
	if( pCmd->szIP[0] )
	{
		strcpy(m_cfginfo.CNTRSVRINFO.szIP, pCmd->szIP);
	}
	if( pCmd->nPort )
	{
		m_cfginfo.CNTRSVRINFO.nPort		= pCmd->nPort;
	}

	SetStatusAndNofity(SVRSTATUS_STARTING);

	// 初始化Server准备接收控制连接
	int	rst	= TCPConnecterServer::Init( m_cfginfo.CNTRSVRINFO.GetBase() );
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetGenericSvrCli_SMTCPSVR::Cmd_SVRCMD_REQ_STARTSERVER,%d,TCPConnecterServer::Init", rst);
		SetStatusAndNofity(SVRSTATUS_INTERNALERROR);
		return	rst;
	}

	SetStatusAndNofity(SVRSTATUS_WORKING);

	return	0;
}
int	whnetGenericSvrCli_SMTCPSVR::Cmd_SVRCMD_REQ_STOPSERVER(const void *pData, size_t nDSize)
{
	return	TCPConnecterServer::Release();
}
int	whnetGenericSvrCli_SMTCPSVR::Cmd_SVRCMD_REQ_SENDDATA(const void *pData, size_t nDSize)
{
	SVRCMD_REQ_SENDDATA_T			*pCmd	= (SVRCMD_REQ_SENDDATA_T *)pData;
	nDSize	-= wh_offsetof(SVRCMD_REQ_SENDDATA_T, szData);
	// 获得相应的客户端对象
	if( pCmd->nClientID<0 )
	{
		// 给所有人满足条件的人发送
		
		for(whunitallocatorFixed<CONNECTOR_INFO_T>::iterator it=m_Connecters.begin(); it!=m_Connecters.end(); ++it)
		{
			I_Connecter	*pConnecter	= (I_Connecter *)(*it).pCntr->QueryInterface();
			if( pConnecter )
			{
				if( pCmd->nTagIdx<0 || pConnecter->GetTag(pCmd->nTagIdx)==pCmd->nTagValue )
				{
					if( pConnecter->SendMsg(pCmd->szData, nDSize)<0 )
					{
						return	-1;
					}
				}
			}
			else
			{
				// 这个可能是Accepter
			}
		}
	}
	else
	{
		Connecter	*pTmp		= GetConnecterPtr(pCmd->nClientID);
		if( pTmp )
		{
			I_Connecter	*pConnecter	= (I_Connecter *)pTmp->QueryInterface();
			if( !pConnecter )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetGenericSvrCli_SMTCPSVR::Cmd_SVRCMD_REQ_SENDDATA,,connecter not exist,%d", pCmd->nClientID);
				return	-2;
			}
			return	pConnecter->SendMsg(pCmd->szData, nDSize);
		}
		else
		{
			return	-3;
		}
	}
	return	0;
}
int	whnetGenericSvrCli_SMTCPSVR::Cmd_SVRCMD_REQ_DISCONNECTCLIENT(const void *pData, size_t nDSize)
{
	SVRCMD_REQ_DISCONNECTCLIENT_T	*pCmd	= (SVRCMD_REQ_DISCONNECTCLIENT_T *)pData;
	// 不能立即 RemoveConnecter(pCmd->nClientID);
	// 因为这可能是在一个tick内，后面也许正要用到整个对象
	m_vectConnecterToRemove.push_back(pCmd->nClientID);
	return	0;
}
int	whnetGenericSvrCli_SMTCPSVR::Cmd_SVRCMD_REQ_SETCLIENTTAG(const void *pData, size_t nDSize)
{
	SVRCMD_REQ_SETCLIENTTAG_T	*pCmd	= (SVRCMD_REQ_SETCLIENTTAG_T *)pData;
	// 获得相应的客户端对象
	I_Connecter	*pConnecter	= (I_Connecter *)GetConnecterPtr(pCmd->nClientID)->QueryInterface();
	if( !pConnecter )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetGenericSvrCli_SMTCPSVR::Cmd_SVRCMD_REQ_SETCLIENTTAG,,connecter not exist,%d", pCmd->nClientID);
		return	-1;
	}
	pConnecter->SetTag(pCmd->nTagIdx, pCmd->nTagValue);
	return	0;
}
void	whnetGenericSvrCli_SMTCPSVR::StatusNotify()
{
	SVRCMD_RPL_STATUS_T		CmdStatus;
	CmdStatus.nStatus		= m_nStatus;
	RPLCmdCall(SVRCMD_RPL_STATUS, &CmdStatus, sizeof(CmdStatus));
}
void	whnetGenericSvrCli_SMTCPSVR::SetStatusAndNofity(int nStatus)
{
	m_nStatus	= nStatus;
	StatusNotify();
}

////////////////////////////////////////////////////////////////////
// whnetGenericSvrCli_SMTCPCLI
////////////////////////////////////////////////////////////////////
WHDATAPROP_MAP_BEGIN_AT_ROOT(whnetGenericSvrCli_SMTCPCLI::MSGERINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(int, nSendBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nRecvBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(short, nBegin, 0)
	WHDATAPROP_ON_SETVALUE_smp(short, nEnd, 0)
WHDATAPROP_MAP_END()
WHDATAPROP_MAP_BEGIN_AT_ROOT(whnetGenericSvrCli_SMTCPCLI::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(unknowntype, MSGERINFO, 0)
WHDATAPROP_MAP_END()
whnetGenericSvrCli_SMTCPCLI::whnetGenericSvrCli_SMTCPCLI()
: m_nStatus(CLISTATUS_NOTHING)
, m_nTimeOutTime(0)
{
	// 注册指令
	WHCMN_CMD_REGANDDEAL_REGCMD(CLICMD_REQ_CONNECT, Cmd_CLICMD_REQ_CONNECT);
	WHCMN_CMD_REGANDDEAL_REGCMD(CLICMD_REQ_DISCONNECT, Cmd_CLICMD_REQ_DISCONNECT);
	WHCMN_CMD_REGANDDEAL_REGCMD(CLICMD_REQ_SENDDATA, Cmd_CLICMD_REQ_SENDDATA);

	m_vectTmpBuf.reserve(100000);
}
whnetGenericSvrCli_SMTCPCLI::~whnetGenericSvrCli_SMTCPCLI()
{
}
int	whnetGenericSvrCli_SMTCPCLI::Init(const char *cszCFG)
{
	WHDATAINI_CMN	ini;
	ini.addobj("TCP", &m_cfginfo);
	int	rst = ini.analyzefile(cszCFG);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetGenericSvrCli_SMTCPCLI::Init,%d,ini.analyzefile,%s %s", rst, cszCFG, ini.printerrreport());
		return	-1;
	}

	// 这个一定要设置为非自动线程的
	m_cfginfo.MSGERINFO.bAutoStartThread	= false;

	return	0;
}
int	whnetGenericSvrCli_SMTCPCLI::Release()
{
	m_msger.Release();
	return	0;
}
int	whnetGenericSvrCli_SMTCPCLI::DoSelect(int nInterval)
{
	if( m_cfginfo.MSGERINFO.sock == INVALID_SOCKET )
	{
		wh_sleep(nInterval);
		return	0;
	}
	return	cmn_select_rd(m_cfginfo.MSGERINFO.sock, nInterval);
}
int	whnetGenericSvrCli_SMTCPCLI::Tick()
{
	switch( m_nStatus )
	{
		case	CLISTATUS_WORKING:
		{
			// 正常的工作
			// 先收
			m_msger.ManualRecv();
			// 检查是否断线
			if( m_msger.GetStatus() != tcpmsger::STATUS_WORKING )
			{
				SetStatusAndNofity(CLISTATUS_DISCONNECTED);
			}
			else
			{
				// 再处理
				// 看是否有收到东西
				size_t		nSize;
				const void	*pCmd;
				while( (pCmd=m_msger.PeekMsg(&nSize)) != NULL )
				{
					RPLCmdCall(CLICMD_RPL_RECVDATA, pCmd, nSize);
					m_msger.FreeMsg();
				}
			}
			// 再发送
			m_msger.ManualSend();
		}
		break;
		case	CLISTATUS_CONNECTING:
		{
			// 判断是否连接成功
			switch( tcp_check_connecting_socket(m_cfginfo.MSGERINFO.sock) )
			{
				case	CONNECTING_SOCKET_CHKRST_CONNTECTING:
				{
					if( wh_tickcount_diff(wh_gettickcount(), m_nTimeOutTime)>=0 )
					{
						// 超时了
						SetStatusAndNofity(CLISTATUS_CONNECTTIMEOUT);
					}
				}
				break;
				case	CONNECTING_SOCKET_CHKRST_OK:
				{
					// 连接成功
					// 初始化通讯器
					int	rst	= m_msger.Init( m_cfginfo.MSGERINFO.GetBase() );
					if( rst<0 )
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetGenericSvrCli_SMTCPCLI::Tick,%d,m_msger.Init", rst);
						SetStatusAndNofity(CLISTATUS_INTERNALERROR);
					}
					else
					{
						// 正式开始工作
						SetStatusAndNofity(CLISTATUS_WORKING);
					}
				}
				break;
				case	CONNECTING_SOCKET_CHKRST_ERR:
				{
					SetStatusAndNofity(CLISTATUS_DISCONNECTED);
				}
				break;
			}
		}
		break;
	}
	return	0;
}
int	whnetGenericSvrCli_SMTCPCLI::CmdIn(int nCmd, const void *pData, size_t nDSize)
{
	return	m_rad.CallFunc(nCmd, pData, nDSize);
}
int	whnetGenericSvrCli_SMTCPCLI::Cmd_CLICMD_REQ_CONNECT(const void *pData, size_t nDSize)
{
	if( m_cfginfo.MSGERINFO.sock != INVALID_SOCKET )
	{
		// 已经连接了，需要断线后才能重新连接
		return	-1;
	}

	CLICMD_REQ_CONNECT_T	*pCmd	= (CLICMD_REQ_CONNECT_T *)pData;

	// 建立连接
	m_cfginfo.MSGERINFO.sock		= tcp_create_connecting_socket(pCmd->szAddr, 0);
	if( m_cfginfo.MSGERINFO.sock == INVALID_SOCKET )
	{
		// 无法创建socket
		return	-2;
	}

	// 设置状态并通知上层
	SetStatusAndNofity(CLISTATUS_CONNECTING);
	m_nTimeOutTime	= wh_gettickcount() + pCmd->nTimeOut;

	return	0;
}
int	whnetGenericSvrCli_SMTCPCLI::Cmd_CLICMD_REQ_DISCONNECT(const void *pData, size_t nDSize)
{
	if( m_cfginfo.MSGERINFO.sock == INVALID_SOCKET )
	{
		// 已经释放过了
		return	0;
	}
	// 释放对象(里面会关闭连接)
	m_msger.Release();
	m_cfginfo.MSGERINFO.sock	= INVALID_SOCKET;
	SetStatusAndNofity(CLISTATUS_DISCONNECTED);

	return	0;
}
int	whnetGenericSvrCli_SMTCPCLI::Cmd_CLICMD_REQ_SENDDATA(const void *pData, size_t nDSize)
{
	if( m_nStatus != CLISTATUS_WORKING )
	{
		// 不在工作状态
		return	-1;
	}
	if( m_msger.SendMsg(pData, nDSize)<0 )
	{
		return	-2;
	}
	return	0;
}
void	whnetGenericSvrCli_SMTCPCLI::StatusNotify()
{
	CLICMD_RPL_STATUS_T		CmdStatus;
	CmdStatus.nStatus		= m_nStatus;
	RPLCmdCall(CLICMD_RPL_STATUS, &CmdStatus, sizeof(CmdStatus));
}
void	whnetGenericSvrCli_SMTCPCLI::SetStatusAndNofity(int nStatus)
{
	switch( nStatus )
	{
		case	CLISTATUS_CONNECTTIMEOUT:
		case	CLISTATUS_DISCONNECTED:
		case	CLISTATUS_INTERNALERROR:
			// 终结通讯器
			m_msger.Release();
			closesocket(m_cfginfo.MSGERINFO.sock);
			m_cfginfo.MSGERINFO.sock	= INVALID_SOCKET;
		break;
	}

	m_nStatus	= nStatus;
	
	StatusNotify();
}
