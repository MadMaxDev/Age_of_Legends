// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetNFS.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ����TCP�ļ������ļ�ϵͳ���ļ���Ϊint��������ֻ֧�ֵ��̵߳ģ�
// CreationDate : 2004-08-14
// ChangeLog    : 2005-05-25 ������whnetNFS_Client���������ܡ�
//              : 2005-08-12 ���ӵ��ļ���ɾ��ʱ���ļ�ֻ������ɾ����whnetNFS_Server::DealMsg_REQ_FILESAVE
//              :            ���ӵ��ļ�û��ȫ������ʱ���洢�κ��ļ���whnetNFS_Client::Release_Clean
//              : 2005-08-29 �����˴洢�������ļ�ʱ��CRC��־��
//              : 2005-09-26 ��¼��ɾ�����ļ�����ʱ�佨��Ŀ¼

#include "../inc/whnetNFS.h"
#include "../inc/whnetudpGLogger.h"
#include <WHCMN/inc/whcmn_def.h>
#include <WHCMN/inc/whdir.h>
#include <WHCMN/inc/whmd5.h>
#include <WHCMN/inc/whcrc.h>

// �ڲ�ʹ�õĽṹ��ָ���
namespace n_whnet
{

enum
{
	// ���ͻ�������󳤶�
	MAX_SENDBUF_SIZE			= 2000000
};
// ָ���
enum
{
	// ����
	WHNETNFS_REQ_SYSEXIT		= 1,						// �˳�����
	WHNETNFS_REQ_FILELOAD		= 2,						// �ļ�����
	WHNETNFS_REQ_FILESAVE		= 3,						// �ļ�����

	// ����
	WHNETNFS_RPL_FILELOAD		= 102,						// һ���ļ���������
	WHNETNFS_RPL_FILESAVE		= 103,						// �ļ�������
};
// �����ṹ����
// ����
// 2004-09-04 �ӣ�ԭ�����˼�pack�ˣ�����Linux�½������ݳ���
#pragma pack(1)
struct	WHNETNFS_REQ_FILELOAD_T
{
	unsigned char		nCheckOutAct;						// WHNETNFS_CHECKOUT_ACT_XXX
	whnetNFS_Client::FILEWORK_T	loadinfo;					// ������صĲ���
};
struct	WHNETNFS_REQ_FILESAVE_T
{
	unsigned char		nCheckInAct;						// WHNETNFS_CHECKIN_ACT_XXX
	whnetnfs_file		nFile;								// �ļ���
	char				data[1];							// �ļ������ļ������ɰ����ȼ���õ�
};
// ����
struct	WHNETNFS_RPL_FILELOAD_T
{
	unsigned char		nCheckOutAct;						// WHNETNFS_CHECKOUT_ACT_XXX
	whnetnfs_file		nFile;								// �ļ���
	char				data[1];							// �ļ������ļ������ɰ����ȼ���õ�
};
struct	WHNETNFS_RPL_FILESAVE_T
{
	enum
	{
		RST_OK			= 0,
		RST_ERR			= 0xFF,								// �����޷�д��
		RST_REFUSE		= 0xFE,								// �������ܾ�
	};
	unsigned char		nRst;								// RST_XXX
	unsigned char		nCheckInAct;						// WHNETNFS_CHECKIN_ACT_XXX
	whnetnfs_file		nFile;								// �ļ���
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
	// �丸�඼��������
	virtual ~whnetNFS_I_Connecter()
	{
	}
	virtual void *	QueryInterface(const char *cszIName)
	{
		// �����
		return	this;
	}
private:
	bool	CanDealMsg() const
	{
		return	m_pHost->CanDealMsg();
	}
	// �ֽ�ָ�����PNLSohu_I��ָ�����
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
	// ��������
	memcpy(&m_info, pInfo, sizeof(m_info));

	// ȷ���ļ�Ŀ¼����
	if( !whfile_ispathexisted(m_info.szWorkDir) )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::Init workdir:%s NOT existed. I make it now.", m_info.szWorkDir);
		whdir_MakeDir(m_info.szWorkDir);
	}

	// ������Բ���Ϊ���
	m_info.msgerinfo.bAutoStartThread	= false;
	// ��ʼ��tcp����server
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
	// �����TCPConnecterServer::ConnecterMan�Ĺ���
	// ����������ÿ��Connecter��DoOneWork���ܣ�������ͻᴦ����Ϣ
	DoWork(true);

	return	0;
}
Connecter *	whnetNFS_Server::NewConnecter(SOCKET sock, struct sockaddr_in *paddr)
{
	whnetNFS_I_Connecter	*pConnecter = new whnetNFS_I_Connecter(this);
	if( pConnecter )
	{
		// ��ʼ��Connecter
		int	rst;
		whnetNFS_I_Connecter::INFO_T	info;
		memcpy(&info.msgerINFO, &m_info.msgerinfo, sizeof(info.msgerINFO));
		info.msgerINFO.sock	= sock;	// ����ɲ�������!!!!
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
	// ���Ҳ��ддlog
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
		// ���ܺͶ�Ӧ��connecter�����Ӷ���
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

// ��Ϣ�������
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
	// ���load�������������꣬����Ҫ���㹻�󣬲������м��Ƿ��ܷ��͵����⡣
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
	// �ȼ���ļ��ж೤
	int		nFSize	= 0;
	whfile	*file	= whfile_OpenCmnFile(GetFileNameStr(nFile), "rb");
	if( !file )
	{
		// ԭ��û���ļ�����ֱ�ӷ���0�������ݼ��ɡ�
	}
	else
	{
		nFSize		= file->FileSize();
	}

	// �Ȳ�����nCheckOutAct���Ժ�������Ҫ�������ɡ�

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
			// ���ļ�����
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::_DealMsg_REQ_FILELOAD_SendOneFileToCurCntr ERROR Read FILE:%d size:%d read rst:%d!", nFile, nFSize, nRst);
			goto	ErrEnd;
		}
	}
	if( file )
	{
		delete	file;
	}
	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Server send file:%d size:%d CRC:0x%X", nFile, nFSize, n_whcmn::crc32(pRPL->data, nFSize));
	// ���͸���ǰ����
	SendCmd2CurConnecter();
	return;
ErrEnd:
	return;
}
void	whnetNFS_Server::DealMsg_REQ_FILESAVE(const void *pParam, size_t nSize)
{
	WHNETNFS_REQ_FILESAVE_T	*pREQ	= (WHNETNFS_REQ_FILESAVE_T *)pParam;

	// ���ڷ��صĽ��
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

	// �����UNDOCHECKOUT��ֱ�ӷ���
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
		// �ֽ�ԭ�����ļ�Ϊ·�����ļ���
		char	szOldFileName[WH_MAX_PATH];
		strcpy(szOldFileName, szFileName);
		const char	*pcszFPath, *pcszFName;
		if( whfile_splitfilenameandpath(szOldFileName, &pcszFPath, &pcszFName) )
		{
			// ��ԭ�����ļ�������-del-ʱ�������
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
			// д���ļ�����
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Server::DealMsg_REQ_FILESAVE ERROR Create FILE:%d size:%d!", pREQ->nFile, nFSize);
			pRPL->nRst		= WHNETNFS_RPL_FILESAVE_T::RST_ERR;
			goto	End;
		}
	}

	// ���¶����ļ�����CRC
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
	// ���ͽ��
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

  	// ���ø���statusWorker������
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
		// ��ʼ���ļ�ϵͳ
		if( m_MemFileMan.Init(&m_info.fmInfo)<0 )
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Init m_MemFileMan.Init ERROR!");
			return	-4;
		}
	}
	else
	{
		// �̳�ԭ�����ļ�ϵͳ
		// 2005-05-16 ԭ��(050509)��Inherit���©�� :(
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

	// ��ʱ�¼�������
	m_info.fteInfo.nMaxNum	= m_info.fmInfo.nMaxFile;
	if( m_fteSave.Init(&m_info.fteInfo)<0 )
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::Init m_fteSave.Init ERROR!");
		return	-6;
	}

	// ��ʼ��
	m_itv.reset();
	// �����ط�����
	m_gener.SetGenSpeed(m_info.fSendRate);

	m_nNumToLoad	= 0;

	// ����������
	if( Init_Retryer()<0 )
	{
		return	-7;
	}

	// �ȴ�ֱ�������ɹ�
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
	// ͨ����,���ֽ⣬Ȼ������
	m_Retryer.ClrAllDstAddr();
	int		nOffset			= 0;
	int		nCanTryCount	= 0;
	char	szDstAddr[WHNET_MAXADDRSTRLEN];
	// ���ֻ��һ����ַ��nCanTryCount���Բ���
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
		// ���ܱ�Release����
		return	-1;
	}
	m_bReleased	= true;

	int	rst=0;
	if( m_Msger.GetStatus() == tcpmsger::STATUS_WORKING )
	{
		// ��socket��������clean��ǰ�ᰡ
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

	// ���Ϸ��ͣ�ֱ��������ϻ���������ߣ�����ʱ��̫��
	while( m_MemFileMan.GetFileNum()>0
	&&     m_Msger.GetStatus() == tcpmsger::STATUS_WORKING
	)
	{
		// �����ܵķ��ͺͽ���
		_DoSendFileToServer_And_SendRecv();
		// �����ص�����
		Worker_WORKING_Work_DealServerRPL();
		// ��Ϣһ��
		m_Msger.ManualRecv(10);
		// �����Ƿ�ʱ��
		if( wh_tickcount_diff(wh_gettickcount(), m_Msger.GetLastRecvTime())>10000 )
		{
			// ��ʱ��
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
	// ���ݵ�ǰ��Status������Ӧ�Ĺ���
	whstatusholder::DoWork();

	return	0;
}
void	whnetNFS_Client::Worker_TRYCONNECT_Begin()
{
	// �رվ�����
	m_Msger.Release();
	// ��ʼ����
	m_Retryer.StartWork();
}
void	whnetNFS_Client::Worker_TRYCONNECT_Work()
{
	int	rst;
	// �������Ĺ���
	m_Retryer.Work();
	// �ж������Ƿ�ɹ�
	if( m_Retryer.GetStatus()==tcpretryer::STATUS_WORK )
	{
		// ���socket
		m_info.msgerinfo.sock				= m_Retryer.PickSocketOut();
		m_info.msgerinfo.bAutoStartThread	= false;
		// ��ʼ����sohu���ͨѶ��
		rst	= m_Msger.Init(&m_info.msgerinfo);
		if( rst<0 )
		{
			GLOGGER2_WRITEFMT(
				GLOGGER_ID_ERROR
				, "whnetNFS_Client::Worker_TRYCONNECT_Work �޷���ʼ��m_Msger!%s"
				, WHLINEEND
				);
			m_Msger.Release();
			SetStatus(m_statusTRYCONNECT);
		}
		else
		{
			GLOGGER2_WRITEFMT(
				GLOGGER_ID_CMN
				, "whnetNFS_Client ���� %s �ɹ�!%s"
				, m_Retryer.GetCurDstAddr()
				, WHLINEEND
				);
			SetStatus(m_statusWORKING);
		}
	}
	// ��Ϊm_Retryer���趨Ϊ���޴����ԣ����Բ�����Fail��״̬����
}
void	whnetNFS_Client::Worker_WORKING_Begin()
{
}
void	whnetNFS_Client::Worker_WORKING_Work()
{
	// �����ܵط����ļ���������
	_DoSendFileToServer_And_SendRecv();

	// ���ݵ�ʵ����
	m_Msger.ManualRecv();

	// ���Ƿ������
	if( m_Msger.GetStatus() == tcpmsger::STATUS_ERROR )
	{
		// ������������
		SetStatus(m_statusTRYCONNECT);
		return;
	}

	// ����server���ص�����
	Worker_WORKING_Work_DealServerRPL();

	// ���ݵ�ʵ�ʷ�
	m_Msger.ManualRecv();

	// ��ʱ����
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
	// ���ļ�д
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
		// �ɹ�д��
		// ���㲢����MD5ֵ���Ա���saveʱ�Ƚ�
		whmemfile::FILEINFO_T	*pFileInfo = pFile->GetInfo();
		whmd5(pRPL->data, nSize, (unsigned char *)pFileInfo->MD5);
		// m_nNumToLoad�ǻ�ϣ��load���ļ�����
		m_nNumToLoad	--;

		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client file:%d size:%d is load from server! still %d left to load%s"
			, pRPL->nFile, pFileInfo->nSize
			, m_nNumToLoad
			, WHLINEEND
			);

		// �Ǽ�һ����ʱ�¼�
		_RegFileSaveEvent(pFileInfo);

		if( m_nNumToLoad==0 )
		{
			// ֪ͨ�ϲ㣬�ļ�ȫ���յ�
			FileLoadOK_Notify(m_pToLoad);
		}
		else if( m_nNumToLoad<0 )
		{
			// ����ǲ�Ӧ�õ�
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
	// ���������CHECKIN��ɾ��
	WHNETNFS_RPL_FILESAVE_T	*pRPL	= (WHNETNFS_RPL_FILESAVE_T*)pParam;
	if( pRPL->nRst != WHNETNFS_RPL_FILESAVE_T::RST_OK )
	{
		// �洢û�гɹ����ļ����ܱ�ɾ��
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
			// ɾ���ļ�
			m_MemFileMan.Delete(pRPL->nFile);
			GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client file:%d is saved to server (act:%d)! File in mem is deleted now.%s"
				, pRPL->nFile
				, pRPL->nCheckInAct
				, WHLINEEND
				);
			// ��ʱû������֪ͨ�Ļ���!
		}
		break;
		case	WHNETNFS_CHECKIN_ACT_JUSTSAVE:
		{
			// ����MD5
			whmemfileman::FILEINFO_T	*pFileInfo	= m_MemFileMan.GetFileInfo(pRPL->nFile);
			if( pFileInfo )
			{
				memcpy(pFileInfo->MD5, pFileInfo->tmpMD5, sizeof(pFileInfo->MD5));
				// �����MarkDelete�����֮
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
		// ����û�յ���
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "whnetNFS_Client::FileLoadReq current m_nNumToLoad=%d NOT ZERO! Operation abort.", m_nNumToLoad);
		return	-1;
	}

	m_pCmdBuf->nCmd			= WHNETNFS_REQ_FILELOAD;
	WHNETNFS_REQ_FILELOAD_T	*pREQ	= (WHNETNFS_REQ_FILELOAD_T *)m_pCmdBuf->szData;
	pREQ->nCheckOutAct		= WHNETNFS_CHECKOUT_ACT_NORMAL;
	int	nSize				= pInfo->GetTotalSize();
	memcpy(&pREQ->loadinfo, pInfo, nSize);
	m_nDSize				= nSize+wh_offsetof(WHNETNFS_REQ_FILELOAD_T, loadinfo);

	// ����ȷ��ʲôʱ��ȫ���������
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
	// �������ص�Save�¼���FTE���������
	whmemfile::FILEINFO_T	*pFileInfo = m_MemFileMan.GetFileInfo(nFile);
	if( !pFileInfo )
	{
		return	-1;
	}
	if( pFileInfo->nProperty & whmemfile::FILEINFO_T::PROPERTY_DEL )
	{
		// �Ѿ�����ǹ���
		return	0;
	}
	return	m_MemFileMan.MarkDelAndClean(nFile);
}
int		whnetNFS_Client::DealSaveEvent(void *pData, int nSize)
{
	SAVEEVENTMAN::EVENTINFO_T	*pEI	= (SAVEEVENTMAN::EVENTINFO_T *)pData;
	// �жϸ��ļ��Ƿ����
	whmemfile::FILEINFO_T		*pFileInfo	= m_MemFileMan.GetFileInfo(pEI->nFile);
	if( !pFileInfo )
	{
		// �����ھͲ��ú����Ķ�����
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "!whnetNFS_Client::DealSaveEvent �ļ�:%d �Ѿ���ʧ! Ŀǰ����%d��Save�¼�(FTE). savelist.size=%d"
			, pEI->nFile
			, m_fteSave.GetEventNum()
			, m_listSave.size()
			);
		return	-1;
	}
	// �������һ��
	DoFileSave_Notify(pEI->nFile);

	// �ļ�����
	WishSendFileToServer(pEI->nFile, WHNETNFS_CHECKIN_ACT_JUSTSAVE, true);

	// ��������һ����ʱ�¼�
	_RegFileSaveEvent(pFileInfo);
	return	0;
}
int		whnetNFS_Client::MakeListSave()
{
	// ��õ�ǰ�����ļ����б�
	whvector<int>	vectFileList;
	m_MemFileMan.GetFileNameList(vectFileList);
	// ���savelist
	m_listSave.clear();
	// ����ǰ���е��ļ�����savelist
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
		// �ļ��Ѿ�������! 
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "whnetNFS_Client::WishSendFileToServer FILE:%d NOT EXIST!!", nFile);
		return	-1;
	}

	SAVEUNIT_T	*pSaveUnit	= NULL;
	// ��Ҫ�����ļ��Ƿ��Ѿ��ڴ洢�б���
	if( bSearch )
	{
		if( pInfo->aExtPtr[EXTIDX_POSINSAVELIST] )
		{
			// ˵��ԭ���Ѿ�������
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
		// ԭ��û�У������б�
		pSaveUnit				= m_listSave.push_back();
		pSaveUnit->nFile		= nFile;
		pSaveUnit->nStartTime	= wh_time();
		// ������������������
		pSaveUnit->nCheckInAct	= WHNETNFS_CHECKIN_ACT_JUSTSAVE;
		// ��¼�¶�Ӧ��SaveUnitָ��(���ܼ���������ͷ:)
		pInfo->aExtPtr[EXTIDX_POSINSAVELIST]	= pSaveUnit;
	}
	// �������CHECKIN_ACT
	pSaveUnit->nCheckInAct		|= nCheckInAct;

	return	0;
}
void	whnetNFS_Client::_DoSendFileToServer_And_SendRecv()
{
	// ���Է�������������ļ���ֱ���޷�ѹ�뻺����
	m_itv.tick();
	int	nNum = m_gener.MakeDecision(m_itv.getfloatInterval());
	while( m_listSave.size()>0 && nNum>0 )
	{
		whlist<SAVEUNIT_T>::iterator	it	= m_listSave.begin();
		if( _DoSendOneFileToServer((*it).nFile, (*it).nCheckInAct)<0 )
		{
			// �޷�������
			break;
		}
		nNum	--;
		// ��Ϊ�ļ��Ƚϴ�������ü�ʱ����
		m_Msger.ManualSend();
		// ��һ��
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
		// ֱ�ӷ��������server�Ϳ�����
		GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client::_DoSendOneFileToServer file:%d is undocheckout", nFile);
		return	SendCmd2Server();
	}

	whmemfile	*pFile			= m_MemFileMan.Open(nFile, whmemfileman::MODE_READ);
	if( !pFile )
	{
		// �ļ������ڣ�һ��ԭ��Ӧ�����ļ���û�з�����������Ҫ�ٷ�һ����ȥUNDO֮��
		pREQ->nCheckInAct		= WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT;
		SendCmd2Server();
		GLOGGER2_WRITEFMT(
			GLOGGER_ID_ERROR
			, "whnetNFS_Client::_DoSendOneFileToServer file:%d NOT EXIST! UNDOCHECKOUT it!"
			, nFile
			);
		// �򲻿�һ���ļ�����Ӱ��������������
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

	// �ļ����ڣ������������savelist�ж������������Ϊ�����ĸ�����Ͳ�������
	whmemfileman::FILEINFO_T	*pFileInfo		= pFile->GetInfo();
	pFileInfo->aExtPtr[EXTIDX_POSINSAVELIST]	= NULL;

	// �����ļ�
	size_t	nSize	= pFile->Read(pREQ->data, m_vectsend.size()-m_nDSize);
	m_nDSize		+= nSize;
	// �ж�һ���ļ��Ƿ�ı�
	whmd5(pREQ->data, nSize, pFile->GetInfo()->tmpMD5);
	if( (pFile->GetInfo()->nProperty & whmemfileman::FILEINFO_T::PROPERTY_DEL) == 0
	&&  memcmp(pFile->GetInfo()->tmpMD5, pFile->GetInfo()->MD5, sizeof(pFile->GetInfo()->tmpMD5))==0 )
	{
		// MD5û�б仯
		if( nCheckInAct==WHNETNFS_CHECKIN_ACT_JUSTSAVE )
		{
			// �������checkin���÷�����
		}
		else if( nCheckInAct==WHNETNFS_CHECKIN_ACT_NORMAL )
		{
			// �ļ�û�б仯��UNDOCHECKOUT
			pREQ->nCheckInAct		= WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT;
			SendCmd2Server();
		}
		// �ر��ļ�
		pFile->Close();
		// һ���Ͳ��ú���Ķ�����
		return	0;
	}

	GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, "whnetNFS_Client::_DoSendOneFileToServer file:%d size:%d"
		, nFile
		, pFile->GetFileSize()
		);

	// �ر��ļ�
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
			, "!!!! whnetNFS_Client::SendCmd2Server ���:%d%s"
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
