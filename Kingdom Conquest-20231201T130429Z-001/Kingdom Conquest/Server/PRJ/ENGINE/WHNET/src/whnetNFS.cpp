// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetNFS.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 基于TCP的简单网络文件系统（文件名为int整数）（只支持单线程的）
// CreationDate : 2004-08-14
// ChangeLog    : 2005-05-25 增加了whnetNFS_Client的重连功能。
//              : 2005-08-12 增加当文件被删除时旧文件只改名不删除。whnetNFS_Server::DealMsg_REQ_FILESAVE
//              :            增加当文件没有全部载入时不存储任何文件。whnetNFS_Client::Release_Clean
//              : 2005-08-29 增加了存储和载入文件时的CRC日志。
//              : 2005-09-26 记录被删除的文件根据时间建立目录

#include "../inc/whnetNFS.h"
#include "../inc/whnetudpGLogger.h"
#include <WHCMN/inc/whcmn_def.h>
#include <WHCMN/inc/whdir.h>
#include <WHCMN/inc/whmd5.h>
#include <WHCMN/inc/whcrc.h>

// 内部使用的结构和指令定义
namespace n_whnet
{

enum
{
	// 发送缓冲区最大长度
	MAX_SENDBUF_SIZE			= 2000000
};
// 指令定义
enum
{
	// 请求
	WHNETNFS_REQ_SYSEXIT		= 1,						// 退出程序
	WHNETNFS_REQ_FILELOAD		= 2,						// 文件载入
	WHNETNFS_REQ_FILESAVE		= 3,						// 文件保存

	// 返回
	WHNETNFS_RPL_FILELOAD		= 102,						// 一个文件的载入结果
	WHNETNFS_RPL_FILESAVE		= 103,						// 文件保存结果
};
// 参数结构定义
// 请求
// 2004-09-04 加，原来忘了加pack了，导致Linux下接收数据出错
#pragma pack(1)
struct	WHNETNFS_REQ_FILELOAD_T
{
	unsigned char		nCheckOutAct;						// WHNETNFS_CHECKOUT_ACT_XXX
	whnetNFS_Client::FILEWORK_T	loadinfo;					// 载入相关的参数
};
struct	WHNETNFS_REQ_FILESAVE_T
{
	unsigned char		nCheckInAct;						// WHNETNFS_CHECKIN_ACT_XXX
	whnetnfs_file		nFile;								// 文件名
	char				data[1];							// 文件内容文件长度由包长度计算得到
};
// 返回
struct	WHNETNFS_RPL_FILELOAD_T
{
	unsigned char		nCheckOutAct;						// WHNETNFS_CHECKOUT_ACT_XXX
	whnetnfs_file		nFile;								// 文件名
	char				data[1];							// 文件内容文件长度由包长度计算得到
};
struct	WHNETNFS_RPL_FILESAVE_T
{
	enum
	{
		RST_OK			= 0,
		RST_ERR			= 0xFF,								// 比如无法写入
		RST_REFUSE		= 0xFE,								// 操作被拒绝
	};
	unsigned char		nRst;								// RST_XXX
	unsigned char		nCheckInAct;						// WHNETNFS_CHECKIN_ACT_XXX
	whnetnfs_file		nFile;								// 文件名
};
#pragma pack()

}		// EOF namespace n_whnet

using namespace n_whcmn;
using namespace n_whnet;

////////////////////////////////////////////////////////////////////
// pnldbNET_I_Connecter
////////////////////////////////////////////////////////////////////
class	whnetNFS_I_Connecter	: public TCPConnecterMsger<tcpmsger>
{
public:
	whnetNFS_Server	*m_pHost;
public:
	whnetNFS_I_Connecter(whnetNFS_Server *pHost)
	: m_pHost(pHost)
	{}
	// 其父类都是虚析构
	virtual ~whnetNFS_I_Connecter()
	{
	}
	virtual void *	QueryInterface(const char *cszIName)
	{
		// 简单起见
		return	this;
	}
private:
	bool	CanDealMsg() const
	{
		return	m_pHost->CanDealMsg();
	}
	// 分解指令，调用PNLSohu_I的指令处理函数
	int		DealMsg(const void *pData, size_t nSize)
	{
		return	m_pHost->DealMsg(m_nIDInMan, pData, nSize);
	}
};

////////////////////////////////////////////////////////////////////
// whnetNFS_Server
////////////////////////////////////////////////////////////////////
whnetNFS_Server::whnetNFS_Server()
: m_bShouldExit(false)
, m_vectsend(MAX_SENDBUF_SIZE)
{
	m_pCmdBuf	= (whcmd_cmd_buf_t *)m_vectsend.getbuf();
	m_nDSize	= 0;
}
whnetNFS_Server::~whnetNFS_Server()
{
}
int		whnetNFS_Server::Init(INFO_T *pInfo)
{
	// 保存数据
	memcpy(&m_info, pInfo, sizeof(m_info));

	// 确保文件目录存在
	if( !whfile_ispathexisted(m_info.szWorkDir) )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::Init workdir:%s NOT existed. I make it now.", m_info.szWorkDir);
		whdir_MakeDir(m_info.szWorkDir);
	}

	// 这个绝对不能为真的
	m_info.msgerinfo.bAutoStartThread	= false;
	// 初始化tcp监听server
	if( TCPConnecterServer::Init(&m_info.CntrSvrInfo)<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::Init TCPConnecterServer::Init ERROR!");
		return	-1;
	}
	return	0;
}
int		whnetNFS_Server::Release()
{
	return	0;
}
int		whnetNFS_Server::Tick()
{
	// 这个是TCPConnecterServer::ConnecterMan的功能
	// 这里面会调用每个Connecter的DoOneWork功能，在里面就会处理消息
	DoWork(true);

	return	0;
}
Connecter *	whnetNFS_Server::NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
{
	whnetNFS_I_Connecter	*pConnecter = new whnetNFS_I_Connecter(this);
	if( pConnecter )
	{
		// 初始化Connecter
		int	rst;
		whnetNFS_I_Connecter::INFO_T	info;
		memcpy(&info.msgerINFO, &m_info.msgerinfo, sizeof(info.msgerINFO));
		info.msgerINFO.sock	= sock;	// 这个可不能忘啊!!!!
		rst	= pConnecter->Init(&info);
		if( rst<0 )
		{
			delete	pConnecter;
			pConnecter	= NULL;
		}
	}
	else
	{
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "whnetNFS_Server::NewConnecter can not create connecter!%s"
			, WHLINEEND
			);
		assert(0);
	}
	return	pConnecter;
}
void		whnetNFS_Server::AfterAddConnecter(Connecter * pCntr)
{
	// 最多也就写写log
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "whnetNFS_Server Connecter id:%d incoming", pCntr->GetIDInMan());
}
const char *	whnetNFS_Server::GetFileNameStr(whnetnfs_file nFile, char *szStr)
{
	static char	szFileFullPath[WH_MAX_PATH];
	if( !szStr )
	{
		szStr	= szFileFullPath;
	}
	char	szFileName[WH_MAX_PATH];
	sprintf(szFileName, m_info.szFNFmt, nFile);
	sprintf(szStr, "%s/%s", m_info.szWorkDir, szFileName);
	return	szStr;
}
int		whnetNFS_Server::SendCmd2CurConnecter()
{
	return	SendCmd2Connecter(m_nCurCntrID);
}
int		whnetNFS_Server::SendCmd2Connecter(int nCntrID)
{
	CONNECTOR_INFO_T	*pCntrInfo	= m_Connecters.getptr(nCntrID);
	if( !pCntrInfo )
	{
		// 可能和对应的connecter的连接断了
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "whnetNFS_Server::SendCmd2Connecter connecter[%d] disconnected!%s"
			, nCntrID
			, WHLINEEND
			);
		return	-1;
	}
	int	rst	= ((whnetNFS_I_Connecter *)pCntrInfo->pCntr)->SendMsg(m_vectsend.getbuf(), m_nDSize+sizeof(*m_pCmdBuf)-1);
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::SendCmd2Connecter dsize:%u SEND ERROR!", m_nDSize);
	}
	return	rst;
}

// 消息处理相关
WHCMD_MAP_BEGIN_AT_ROOT(whnetNFS_Server)
	WHCMD_MAP_ON_c(WHNETNFS_REQ_SYSEXIT, DealMsg_REQ_SYSEXIT)
	WHCMD_MAP_ON_p(WHNETNFS_REQ_FILELOAD, DealMsg_REQ_FILELOAD)
	WHCMD_MAP_ON_p(WHNETNFS_REQ_FILESAVE, DealMsg_REQ_FILESAVE)
WHCMD_MAP_END()

bool	whnetNFS_Server::CanDealMsg() const
{
	return	true;
}
int		whnetNFS_Server::DealMsg(int nIDInMan, const void *pCmd, size_t nSize)
{
	m_nCurCntrID	= nIDInMan;
	whcmd_cmd_buf_t	*pCmd0	= (whcmd_cmd_buf_t *)pCmd;

	whcmddealer::DealCmd(pCmd0->nCmd, pCmd0->szData, nSize-sizeof(*pCmd0)+1);

	return	0;
}
void	whnetNFS_Server::DealMsg_REQ_SYSEXIT(const char *szPass)
{
	m_bShouldExit	= true;
}
void	whnetNFS_Server::DealMsg_REQ_FILELOAD(const void *pParam, size_t nSize)
{
	WHNETNFS_REQ_FILELOAD_T	*pCmd	= (WHNETNFS_REQ_FILELOAD_T *)pParam;
	// 这个load操作会连续做完，缓冲要设足够大，不考虑中间是否不能发送的问题。
	switch( pCmd->loadinfo.nType )
	{
		case	whnetNFS_Client::FILEWORK_T::TYPE_ARRAY:
		{
			for(int i=0;i<pCmd->loadinfo.nFile[0];i++)
			{
				_DealMsg_REQ_FILELOAD_SendOneFileToCurCntr(pCmd->loadinfo.nFile[1+i], pCmd->nCheckOutAct);
			}
		}
		break;
		case	whnetNFS_Client::FILEWORK_T::TYPE_ONE:
		{
			_DealMsg_REQ_FILELOAD_SendOneFileToCurCntr(pCmd->loadinfo.nFile[0], pCmd->nCheckOutAct);
		}
		break;
		case	whnetNFS_Client::FILEWORK_T::TYPE_RANGE:
		{
			for(whnetnfs_file i=pCmd->loadinfo.nFile[0];i<pCmd->loadinfo.nFile[1];i++)
			{
				_DealMsg_REQ_FILELOAD_SendOneFileToCurCntr(i, pCmd->nCheckOutAct);
			}
		}
		break;
	}
}
void	whnetNFS_Server::_DealMsg_REQ_FILELOAD_SendOneFileToCurCntr(whnetnfs_file nFile, unsigned char nCheckOutAct)
{
	// 先检测文件有多长
	int		nFSize	= 0;
	whfile	*file	= whfile_OpenCmnFile(GetFileNameStr(nFile), "rb");
	if( !file )
	{
		// 原来没有文件。则直接发送0长度数据即可。
	}
	else
	{
		nFSize		= file->FileSize();
	}

	// 先不处理nCheckOutAct。以后真有需要了再做吧。

	m_pCmdBuf->nCmd		= WHNETNFS_RPL_FILELOAD;
	WHNETNFS_RPL_FILELOAD_T	*pRPL	= (WHNETNFS_RPL_FILELOAD_T *)m_pCmdBuf->szData;
	pRPL->nCheckOutAct	= nCheckOutAct;
	pRPL->nFile			= nFile;
	m_nDSize			= nFSize + wh_offsetof(WHNETNFS_RPL_FILELOAD_T, data);
	if( nFSize>0 )
	{
		int	nRst		= file->Read(pRPL->data, nFSize);
		if( nRst!=nFSize )
		{
			// 读文件出错
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::_DealMsg_REQ_FILELOAD_SendOneFileToCurCntr ERROR Read FILE:%d size:%d read rst:%d!", nFile, nFSize, nRst);
			goto	ErrEnd;
		}
	}
	if( file )
	{
		delete	file;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Server send file:%d size:%d CRC:0x%X", nFile, nFSize, n_whcmn::crc32(pRPL->data, nFSize));
	// 发送给当前连接
	SendCmd2CurConnecter();
	return;
ErrEnd:
	return;
}
void	whnetNFS_Server::DealMsg_REQ_FILESAVE(const void *pParam, size_t nSize)
{
	WHNETNFS_REQ_FILESAVE_T	*pREQ	= (WHNETNFS_REQ_FILESAVE_T *)pParam;

	// 用于返回的结果
	m_pCmdBuf->nCmd		= WHNETNFS_RPL_FILESAVE;
	WHNETNFS_RPL_FILESAVE_T	*pRPL	= (WHNETNFS_RPL_FILESAVE_T *)m_pCmdBuf->szData;
	pRPL->nRst			= WHNETNFS_RPL_FILESAVE_T::RST_OK;
	pRPL->nCheckInAct	= pREQ->nCheckInAct;
	pRPL->nFile			= pREQ->nFile;
	m_nDSize			= sizeof(*pRPL);

	int		nFSize;
	whfile	*file		= NULL;
	whvector<char>	vect;
	unsigned int	nCRC(0);

	// 如果是UNDOCHECKOUT则直接返回
	if( pREQ->nCheckInAct == WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Server file:%d is undo checkout.%s"
			, pREQ->nFile
			, WHLINEEND
			);
		goto	End;
	}

	char	szFileName[WH_MAX_PATH];
	GetFileNameStr(pREQ->nFile, szFileName);
	nFSize	= nSize - wh_offsetof(WHNETNFS_REQ_FILESAVE_T, data);
	int		nOldFSize;
	nOldFSize	= whfile_getfilesize(szFileName);
	if( nOldFSize < 0 )
	{
		nOldFSize	= 0;
	}
	if( nFSize==0 )
	{
		// 分解原来的文件为路径和文件名
		char	szOldFileName[WH_MAX_PATH];
		strcpy(szOldFileName, szFileName);
		const char	*pcszFPath, *pcszFName;
		if( whfile_splitfilenameandpath(szOldFileName, &pcszFPath, &pcszFName) )
		{
			// 把原来的文件改名：-del-时间的样子
			char	szNewFileName[WH_MAX_PATH];
			sprintf(szNewFileName, "%s/%s/%s-del-%s", pcszFPath, wh_getsmpdaystr(), pcszFName, wh_getsmptimestr_for_file());
			whdir_SureMakeDirForFile(szNewFileName);
			int	rst	= whfile_ren( szFileName, szNewFileName );
			if( rst<0 )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::DealMsg_REQ_FILESAVE whfile_ren(%s,%s) err:%d!"
					, szFileName, szNewFileName, rst
				);
			}
			else
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Server::DealMsg_REQ_FILESAVE FILE:%d size:%d old size:%d file rename to %s rst:%d!"
					, pREQ->nFile, nFSize, nOldFSize
					, szNewFileName
					, rst
				);
			}
		}
	}
	else if( nFSize>0 )
	{
		if( whfile_safereplace(szFileName, pREQ->data, nFSize, true)<0 )
		{
			// 写打开文件出错
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::DealMsg_REQ_FILESAVE ERROR Create FILE:%d size:%d!", pREQ->nFile, nFSize);
			pRPL->nRst		= WHNETNFS_RPL_FILESAVE_T::RST_ERR;
			goto	End;
		}
	}

	// 重新读出文件计算CRC
	vect.reserve(nFSize);
	if( whfile_readfile(szFileName, vect)<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::DealMsg_REQ_FILESAVE rereadfile:%s error!", szFileName);
	}
	else
	{
		nCRC	= n_whcmn::crc32(vect.getbuf(), vect.size());
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Server file:%d size:%d is saved. (file on disk size:%d crc:0x%X)%s"
		, pREQ->nFile, nFSize
		, vect.size(), nCRC
		, WHLINEEND
		);

End:
	if( file )
	{
		delete	file;
	}
	// 发送结果
	SendCmd2CurConnecter();
	return;
}

////////////////////////////////////////////////////////////////////
// whnetNFS_Client
////////////////////////////////////////////////////////////////////
whnetNFS_Client::whnetNFS_Client()
: 
  m_statusTRYCONNECT(STATUS_TRYCONNECT)
, m_statusWORKING(STATUS_WORKING)
, m_vectsend(MAX_SENDBUF_SIZE)
, m_vectToLoad(10000)
, m_nNumToLoad(0)
, m_bReleased(true)
{
	m_pCmdBuf	= (whcmd_cmd_buf_t *)m_vectsend.getbuf();
	m_nDSize	= 0;
	m_pToLoad	= (FILEWORK_T *)m_vectToLoad.getbuf();
	m_fteSave.m_pHost	= this;

  	// 设置各个statusWorker的内容
	m_statusTRYCONNECT.SetBeginFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_TRYCONNECT_Begin));
	m_statusTRYCONNECT.SetWorkFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_TRYCONNECT_Work));
	m_statusWORKING.SetBeginFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_WORKING_Begin));
	m_statusWORKING.SetWorkFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_WORKING_Work));
	m_statusWORKING.SetEndFunc(MAKE_WHSH_WORKFUNC_OFTHISCLASS(Worker_WORKING_End));
}
whnetNFS_Client::~whnetNFS_Client()
{
}
int		whnetNFS_Client::Init(INFO_T *pInfo)
{
	m_bReleased	= false;

	memcpy(&m_info, pInfo, sizeof(m_info));

	if( !m_info.bInherit )
	{
		// 初始化文件系统
		if( m_MemFileMan.Init(&m_info.fmInfo)<0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Init m_MemFileMan.Init ERROR!");
			return	-4;
		}
	}
	else
	{
		// 继承原来的文件系统
		// 2005-05-16 原来(050509)把Inherit这句漏了 :(
		if( m_MemFileMan.Inherit(m_info.fmInfo.pBuf)<0 )
		{
			if( m_MemFileMan.GetMemInfo() && m_MemFileMan.IsMarkedClean() )
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Init m_MemFileMan.Inherit Old IS MarkedClean! Delete it manually and then restart Please.");
				return	-5;
			}
			else
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Init m_MemFileMan.Inherit ERROR! Can not Inherit!");
				return	-6;
			}
		}
	}

	// 定时事件管理器
	m_info.fteInfo.nMaxNum	= m_info.fmInfo.nMaxFile;
	if( m_fteSave.Init(&m_info.fteInfo)<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Init m_fteSave.Init ERROR!");
		return	-6;
	}

	// 初始化
	m_itv.reset();
	// 设置重发速率
	m_gener.SetGenSpeed(m_info.fSendRate);

	m_nNumToLoad	= 0;

	// 创建重连器
	if( Init_Retryer()<0 )
	{
		return	-7;
	}

	// 等待直到重连成功
	SetStatus(m_statusTRYCONNECT);
	whtick_t	nNow	= wh_gettickcount();
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "whnetNFS_Client::Init try connecting %s ...", m_info.szSvrAddr);
	while( m_Msger.GetStatus() != tcpmsger::STATUS_WORKING )
	{
		if( wh_tickcount_diff(wh_gettickcount(), nNow) >= m_info.nConnectTimeOut )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Init can NOT connect %s !", m_info.szSvrAddr);
			return	-8;
		}
		wh_sleep(10);
		Tick();
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_CMN, "whnetNFS_Client::Init %s connected", m_info.szSvrAddr);

	return	0;
}
int		whnetNFS_Client::Init_Retryer()
{
	// 通过“,”分解，然后设置
	m_Retryer.ClrAllDstAddr();
	int		nOffset			= 0;
	int		nCanTryCount	= 0;
	char	szDstAddr[WHNET_MAXADDRSTRLEN];
	// 如果只有一个地址则nCanTryCount可以不输
	while( wh_strsplit(&nOffset, "sd", m_info.szSvrAddr, ",", szDstAddr, &nCanTryCount)>=1 )
	{
		m_Retryer.AddDstAddr(szDstAddr, nCanTryCount);
	}

	return	0;
}
int		whnetNFS_Client::Release()
{
	if( m_bReleased )
	{
		// 不能被Release两次
		return	-1;
	}
	m_bReleased	= true;

	int	rst=0;
	if( m_Msger.GetStatus() == tcpmsger::STATUS_WORKING )
	{
		// 有socket存在是能clean的前提啊
		rst	= Release_Clean();
	}

	m_Msger.Release();

	m_fteSave.Release();

	return	rst;
}
int		whnetNFS_Client::Release_Clean()
{
	if( m_info.bNoSaveWhenNoLoadAll && m_nNumToLoad>0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client::Release_Clean CurFileNum:%d m_nNumToLoad:%d not all files loaded. Save operation aborted.", m_MemFileMan.GetFileNum(), m_nNumToLoad);
		m_MemFileMan.Reset();
		return	0;
	}

	int	rst=0;

	MakeListSave();

	m_Msger.SetLastRecvTime(wh_gettickcount());

	GLOGGER2_SET_PRINTINSCREENTOO(true);
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client::Release_Clean Begin %d file to clean ...%s"
		, m_MemFileMan.GetFileNum()
		, WHLINEEND
		);

	// 不断发送，直到发送完毕或者网络断线，或者时间太长
	while( m_MemFileMan.GetFileNum()>0
	&&     m_Msger.GetStatus() == tcpmsger::STATUS_WORKING
	)
	{
		// 最大可能的发送和接收
		_DoSendFileToServer_And_SendRecv();
		// 处理返回的数据
		Worker_WORKING_Work_DealServerRPL();
		// 休息一下
		m_Msger.ManualRecv(10);
		// 看看是否超时了
		if( wh_tickcount_diff(wh_gettickcount(), m_Msger.GetLastRecvTime())>10000 )
		{
			// 超时了
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Release_Clean recv timeout!");
			rst		= -1;
			goto	End;
		}
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client::Release_Clean End.%s", WHLINEEND);

End:
	GLOGGER2_SET_PRINTINSCREENTOO(false);
	return	rst;
}
int		whnetNFS_Client::Tick()
{
	// 根据当前的Status进行相应的工作
	whstatusholder::DoWork();

	return	0;
}
void	whnetNFS_Client::Worker_TRYCONNECT_Begin()
{
	// 关闭旧连接
	m_Msger.Release();
	// 开始重试
	m_Retryer.StartWork();
}
void	whnetNFS_Client::Worker_TRYCONNECT_Work()
{
	int	rst;
	// 做重联的工作
	m_Retryer.Work();
	// 判断重联是否成功
	if( m_Retryer.GetStatus()==tcpretryer::STATUS_WORK )
	{
		// 获得socket
		m_info.msgerinfo.sock				= m_Retryer.PickSocketOut();
		m_info.msgerinfo.bAutoStartThread	= false;
		// 初始化和sohu间的通讯器
		rst	= m_Msger.Init(&m_info.msgerinfo);
		if( rst<0 )
		{
			GLOGGER2_WRITEFMT(
				GLOGGER_ID_ERROR
				, "whnetNFS_Client::Worker_TRYCONNECT_Work 无法初始化m_Msger!%s"
				, WHLINEEND
				);
			m_Msger.Release();
			SetStatus(m_statusTRYCONNECT);
		}
		else
		{
			GLOGGER2_WRITEFMT(
				GLOGGER_ID_CMN
				, "whnetNFS_Client 连接 %s 成功!%s"
				, m_Retryer.GetCurDstAddr()
				, WHLINEEND
				);
			SetStatus(m_statusWORKING);
		}
	}
	// 因为m_Retryer被设定为无限次重试，所以不会有Fail的状态出现
}
void	whnetNFS_Client::Worker_WORKING_Begin()
{
}
void	whnetNFS_Client::Worker_WORKING_Work()
{
	// 最大可能地发送文件到服务器
	_DoSendFileToServer_And_SendRecv();

	// 数据的实际收
	m_Msger.ManualRecv();

	// 看是否出错了
	if( m_Msger.GetStatus() == tcpmsger::STATUS_ERROR )
	{
		// 尝试重新连接
		SetStatus(m_statusTRYCONNECT);
		return;
	}

	// 处理server返回的数据
	Worker_WORKING_Work_DealServerRPL();

	// 数据的实际发
	m_Msger.ManualRecv();

	// 定时存盘
	m_fteSave.Tick();
}
void	whnetNFS_Client::Worker_WORKING_End()
{
	m_Msger.Release();
}
int		whnetNFS_Client::Worker_WORKING_Work_DealServerRPL()
{
	size_t					nSize;
	const whcmd_cmd_buf_t	*pCmd;
	while( (pCmd=(const whcmd_cmd_buf_t *)m_Msger.PeekMsg(&nSize)) != NULL )
	{
		nSize	-= wh_offsetof(whcmd_cmd_buf_t, szData);
		switch( pCmd->nCmd )
		{
			case	WHNETNFS_RPL_FILELOAD:
				Worker_WORKING_Work_DealServerRPL_FILELOAD(pCmd->szData, nSize);
			break;
			case	WHNETNFS_RPL_FILESAVE:
				Worker_WORKING_Work_DealServerRPL_FILESAVE(pCmd->szData, nSize);
			break;
		}
		m_Msger.FreeMsg();
	}
	return	0;
}
int		whnetNFS_Client::Worker_WORKING_Work_DealServerRPL_FILELOAD(const void *pParam, size_t nSize)
{
	WHNETNFS_RPL_FILELOAD_T	*pRPL	= (WHNETNFS_RPL_FILELOAD_T*)pParam;
	// 打开文件写
	whmemfile	*pFile				= m_MemFileMan.Open(pRPL->nFile, whmemfileman::MODE_CREATE);
	if( !pFile )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Worker_WORKING_Work_DealServerRPL_FILELOAD Can NOT create file:%d", pRPL->nFile);
		return	-1;
	}

	nSize	-= wh_offsetof(WHNETNFS_RPL_FILELOAD_T, data);
	if( pFile->Write(pRPL->data, nSize) != nSize )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Worker_WORKING_Work_DealServerRPL_FILELOAD write file:%d ERROR!", pRPL->nFile);
	}
	else
	{
		// 成功写入
		// 计算并存入MD5值，以便在save时比较
		whmemfile::FILEINFO_T	*pFileInfo = pFile->GetInfo();
		whmd5(pRPL->data, nSize, (unsigned char *)pFileInfo->MD5);
		// m_nNumToLoad是还希望load的文件数量
		m_nNumToLoad	--;

		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client file:%d size:%d is load from server! still %d left to load%s"
			, pRPL->nFile, pFileInfo->nSize
			, m_nNumToLoad
			, WHLINEEND
			);

		// 登记一个定时事件
		_RegFileSaveEvent(pFileInfo);

		if( m_nNumToLoad==0 )
		{
			// 通知上层，文件全部收到
			FileLoadOK_Notify(m_pToLoad);
		}
		else if( m_nNumToLoad<0 )
		{
			// 这个是不应该的
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Worker_WORKING_Work_DealServerRPL_FILELOAD TOO MANY Recv file:%d m_nNumToLoad=%d!%s"
				, pRPL->nFile
				, m_nNumToLoad
				, WHLINEEND
				);
		}
	}

	pFile->Close();

	return	0;
}
int		whnetNFS_Client::Worker_WORKING_Work_DealServerRPL_FILESAVE(const void *pParam, size_t nSize)
{
	// 如果请求是CHECKIN则删除
	WHNETNFS_RPL_FILESAVE_T	*pRPL	= (WHNETNFS_RPL_FILESAVE_T*)pParam;
	if( pRPL->nRst != WHNETNFS_RPL_FILESAVE_T::RST_OK )
	{
		// 存储没有成功，文件不能被删除
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Worker_WORKING_Work_DealServerRPL_FILESAVE file:%d SAVE ERROR! rst:%d"
			, pRPL->nFile
			, pRPL->nRst
			);
		return	0;
	}

	switch( pRPL->nCheckInAct )
	{
		case	WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT:
		case	WHNETNFS_CHECKIN_ACT_NORMAL:
		{
			// 删除文件
			m_MemFileMan.Delete(pRPL->nFile);
			GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client file:%d is saved to server (act:%d)! File in mem is deleted now.%s"
				, pRPL->nFile
				, pRPL->nCheckInAct
				, WHLINEEND
				);
			// 暂时没有向上通知的机制!
		}
		break;
		case	WHNETNFS_CHECKIN_ACT_JUSTSAVE:
		{
			// 更新MD5
			whmemfileman::FILEINFO_T	*pFileInfo	= m_MemFileMan.GetFileInfo(pRPL->nFile);
			if( pFileInfo )
			{
				memcpy(pFileInfo->MD5, pFileInfo->tmpMD5, sizeof(pFileInfo->MD5));
				// 如果有MarkDelete则清空之
				if( pFileInfo->nProperty & whmemfileman::FILEINFO_T::PROPERTY_DEL )
				{
					pFileInfo->nProperty	^= whmemfileman::FILEINFO_T::PROPERTY_DEL;
				}
			}
			GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client file:%d is saved to server!%s"
				, pRPL->nFile
				, WHLINEEND
				);
		}
		break;
	}
	return	0;
}
int		whnetNFS_Client::FileLoadReq(FILEWORK_T *pInfo)
{
	if( m_nNumToLoad>0 )
	{
		// 还有没收到的
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::FileLoadReq current m_nNumToLoad=%d NOT ZERO! Operation abort.", m_nNumToLoad);
		return	-1;
	}

	m_pCmdBuf->nCmd			= WHNETNFS_REQ_FILELOAD;
	WHNETNFS_REQ_FILELOAD_T	*pREQ	= (WHNETNFS_REQ_FILELOAD_T *)m_pCmdBuf->szData;
	pREQ->nCheckOutAct		= WHNETNFS_CHECKOUT_ACT_NORMAL;
	int	nSize				= pInfo->GetTotalSize();
	memcpy(&pREQ->loadinfo, pInfo, nSize);
	m_nDSize				= nSize+wh_offsetof(WHNETNFS_REQ_FILELOAD_T, loadinfo);

	// 用于确定什么时候全部接收完毕
	m_nNumToLoad			= pInfo->GetFileNum();
	memcpy(m_pToLoad, pInfo, nSize);

	return	SendCmd2Server();
}
int		whnetNFS_Client::FileSaveReq(FILEWORK_T *pInfo, unsigned char nCheckInAct)
{
	switch( pInfo->nType )
	{
		case	FILEWORK_T::TYPE_ARRAY:
		{
			for(int i=1;i<=pInfo->nFile[0];i++)
			{
				WishSendFileToServer(pInfo->nFile[i], nCheckInAct, true);
			}
		}
		break;
		case	FILEWORK_T::TYPE_ONE:
		{
			WishSendFileToServer(pInfo->nFile[0], nCheckInAct, true);
		}
		break;
		case	FILEWORK_T::TYPE_RANGE:
		{
			for(whnetnfs_file i=pInfo->nFile[0];i<=pInfo->nFile[1];i++)
			{
				WishSendFileToServer(i, nCheckInAct, true);
			}
		}
		break;
	}
	return	0;
}
int		whnetNFS_Client::MarkDelAndClean(int nFile)
{
	// 如果有相关的Save事件和FTE则清除他们
	whmemfile::FILEINFO_T	*pFileInfo = m_MemFileMan.GetFileInfo(nFile);
	if( !pFileInfo )
	{
		return	-1;
	}
	if( pFileInfo->nProperty & whmemfile::FILEINFO_T::PROPERTY_DEL )
	{
		// 已经被标记过了
		return	0;
	}
	return	m_MemFileMan.MarkDelAndClean(nFile);
}
int		whnetNFS_Client::DealSaveEvent(void *pData, int nSize)
{
	SAVEEVENTMAN::EVENTINFO_T	*pEI	= (SAVEEVENTMAN::EVENTINFO_T *)pData;
	// 判断该文件是否存在
	whmemfile::FILEINFO_T		*pFileInfo	= m_MemFileMan.GetFileInfo(pEI->nFile);
	if( !pFileInfo )
	{
		// 不存在就不用后续的动作了
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "!whnetNFS_Client::DealSaveEvent 文件:%d 已经消失! 目前还有%d个Save事件(FTE). savelist.size=%d"
			, pEI->nFile
			, m_fteSave.GetEventNum()
			, m_listSave.size()
			);
		return	-1;
	}
	// 让刘毅存一下
	DoFileSave_Notify(pEI->nFile);

	// 文件存盘
	WishSendFileToServer(pEI->nFile, WHNETNFS_CHECKIN_ACT_JUSTSAVE, true);

	// 则启动下一个定时事件
	_RegFileSaveEvent(pFileInfo);
	return	0;
}
int		whnetNFS_Client::MakeListSave()
{
	// 获得当前所有文件的列表
	whvector<int>	vectFileList;
	m_MemFileMan.GetFileNameList(vectFileList);
	// 清空savelist
	m_listSave.clear();
	// 将当前所有的文件放入savelist
	for(size_t i=0;i<vectFileList.size();i++)
	{
		WishSendFileToServer(vectFileList[i], WHNETNFS_CHECKIN_ACT_NORMAL, false);
	}
	return	0;
}
int		whnetNFS_Client::WishSendFileToServer(whnetnfs_file nFile, unsigned char nCheckInAct, bool bSearch)
{
	whmemfileman::FILEINFO_T	*pInfo	= m_MemFileMan.GetFileInfo(nFile);
	if( !pInfo )
	{
		// 文件已经不存在! 
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "whnetNFS_Client::WishSendFileToServer FILE:%d NOT EXIST!!", nFile);
		return	-1;
	}

	SAVEUNIT_T	*pSaveUnit	= NULL;
	// 需要检查该文件是否已经在存储列表中
	if( bSearch )
	{
		if( pInfo->aExtPtr[EXTIDX_POSINSAVELIST] )
		{
			// 说明原来已经在里面
			pSaveUnit		= (SAVEUNIT_T *)pInfo->aExtPtr[EXTIDX_POSINSAVELIST];
			GLOGGER2_WRITEFMT(
				GLOGGER_ID_STORY
				, "whnetNFS_Client::WishSendFileToServer %d already in save list, unit:%p%s"
				, nFile
				, pSaveUnit
				, WHLINEEND
				);
		}
	}
	if( !pSaveUnit )
	{
		// 原来没有，加入列表
		pSaveUnit				= m_listSave.push_back();
		pSaveUnit->nFile		= nFile;
		pSaveUnit->nStartTime	= wh_time();
		// 这个必须先置最低请求
		pSaveUnit->nCheckInAct	= WHNETNFS_CHECKIN_ACT_JUSTSAVE;
		// 记录下对应的SaveUnit指针(功能见本函数开头:)
		pInfo->aExtPtr[EXTIDX_POSINSAVELIST]	= pSaveUnit;
	}
	// 结合两个CHECKIN_ACT
	pSaveUnit->nCheckInAct		|= nCheckInAct;

	return	0;
}
void	whnetNFS_Client::_DoSendFileToServer_And_SendRecv()
{
	// 尝试发送最大数量的文件，直到无法压入缓冲区
	m_itv.tick();
	int	nNum = m_gener.MakeDecision(m_itv.getfloatInterval());
	while( m_listSave.size()>0 && nNum>0 )
	{
		whlist<SAVEUNIT_T>::iterator	it	= m_listSave.begin();
		if( _DoSendOneFileToServer((*it).nFile, (*it).nCheckInAct)<0 )
		{
			// 无法发送了
			break;
		}
		nNum	--;
		// 因为文件比较大，所以最好及时发送
		m_Msger.ManualSend();
		// 下一个
		m_listSave.pop_front();
	}
	m_Msger.ManualSend();
	m_Msger.ManualRecv();
}
int	whnetNFS_Client::_DoSendOneFileToServer(whnetnfs_file nFile, unsigned char nCheckInAct)
{
	m_pCmdBuf->nCmd				= WHNETNFS_REQ_FILESAVE;
	WHNETNFS_REQ_FILESAVE_T		*pREQ	= (WHNETNFS_REQ_FILESAVE_T *)m_pCmdBuf->szData;
	pREQ->nFile					= nFile;
	m_nDSize					= wh_offsetof(WHNETNFS_REQ_FILESAVE_T, data);
	pREQ->nCheckInAct			= nCheckInAct;
	if( nCheckInAct == WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT )
	{
		// 直接发送这个给server就可以了
		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client::_DoSendOneFileToServer file:%d is undocheckout", nFile);
		return	SendCmd2Server();
	}

	whmemfile	*pFile			= m_MemFileMan.Open(nFile, whmemfileman::MODE_READ);
	if( !pFile )
	{
		// 文件不存在（一般原因应该是文件就没有发过来，所以要再发一个过去UNDO之）
		pREQ->nCheckInAct		= WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT;
		SendCmd2Server();
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "whnetNFS_Client::_DoSendOneFileToServer file:%d NOT EXIST! UNDOCHECKOUT it!"
			, nFile
			);
		// 打不开一个文件不能影响后面的其他动作
		return	0;
	}
	if( pFile->GetFileSize()>m_vectsend.size() )
	{
		pREQ->nCheckInAct		= WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT;
		SendCmd2Server();
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "whnetNFS_Client::_DoSendOneFileToServer file:%d size:%d TOO BIG! UNDOCHECKOUT it! and Delete it!"
			, nFile
			, pFile->GetFileSize()
			);
		pFile->Close();
		m_MemFileMan.Delete(nFile);
		return	0;
	}

	// 文件存在，则先清除它对savelist中对象的索引，因为过后哪个对象就不存在了
	whmemfileman::FILEINFO_T	*pFileInfo		= pFile->GetInfo();
	pFileInfo->aExtPtr[EXTIDX_POSINSAVELIST]	= NULL;

	// 读入文件
	size_t	nSize	= pFile->Read(pREQ->data, m_vectsend.size()-m_nDSize);
	m_nDSize		+= nSize;
	// 判断一下文件是否改变
	whmd5(pREQ->data, nSize, pFile->GetInfo()->tmpMD5);
	if( (pFile->GetInfo()->nProperty & whmemfileman::FILEINFO_T::PROPERTY_DEL) == 0
	&&  memcmp(pFile->GetInfo()->tmpMD5, pFile->GetInfo()->MD5, sizeof(pFile->GetInfo()->tmpMD5))==0 )
	{
		// MD5没有变化
		if( nCheckInAct==WHNETNFS_CHECKIN_ACT_JUSTSAVE )
		{
			// 如果不是checkin则不用发送了
		}
		else if( nCheckInAct==WHNETNFS_CHECKIN_ACT_NORMAL )
		{
			// 文件没有变化则UNDOCHECKOUT
			pREQ->nCheckInAct		= WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT;
			SendCmd2Server();
		}
		// 关闭文件
		pFile->Close();
		// 一样就不用后面的东西了
		return	0;
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client::_DoSendOneFileToServer file:%d size:%d"
		, nFile
		, pFile->GetFileSize()
		);

	// 关闭文件
	pFile->Close();
	return	SendCmd2Server();
}
int		whnetNFS_Client::SendCmd2Server()
{
	int		rst = m_Msger.SendMsg(m_pCmdBuf, m_nDSize + wh_offsetof(whcmd_cmd_buf_t, szData));
	if( rst<0 )
	{
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "!!!! whnetNFS_Client::SendCmd2Server 结果:%d%s"
			, rst
			, WHLINEEND
			);
	}
	return	rst;
}
int		whnetNFS_Client::_RegFileSaveEvent(whmemfile::FILEINFO_T *pFileInfo)
{
	SAVEEVENTMAN::EVENTINFO_T	ei;
	ei.nFile	= pFileInfo->nName;
	int	nTEID	= m_fteSave.AddEvent(&ei, sizeof(ei));
	//GLOGGER2_WRITEFMT(
	//	GLOGGER_ID_STORY
	//	, "whnetNFS_Client::_RegFileSaveEvent for file:%d, nTEID:%d%s"
	//	, pFileInfo->nName
	//	, nTEID
	//	, WHLINEEND
	//	);
	pFileInfo->aExtPtr[EXTIDX_TIMEEVENTID]	= (void *)nTEID;
	if( nTEID<0 )
	{
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "!!whnetNFS_Client::_RegFileSaveEvent Can not reg for file:%d%s"
			, pFileInfo->nName
			, WHLINEEND
			);
		return	-1;
	}
	return		0;
}
