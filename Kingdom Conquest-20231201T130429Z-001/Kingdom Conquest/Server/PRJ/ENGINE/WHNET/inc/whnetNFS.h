// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetNFS.h
// Creator      : Wei Hua (κ��)
// Comment      : ����TCP�ļ������ļ�ϵͳ���ļ���Ϊint��������ֻ֧�ֵ��̵߳ģ�
//                Ϊ����Ƽ򵥣������ļ���ҪС��1M��
// CreationDate : 2004-08-14
// ChangeLog    :

#ifndef	__WHNETNFS_H__
#define	__WHNETNFS_H__

#include "../inc/whconnecter.h"
#include "../inc/whnettcp.h"
#include <WHCMN/inc/whfile.h>
#include <WHCMN/inc/whmemfile.h>
#include <WHCMN/inc/whcmd.h>
#include <WHCMN/inc/whtimeevent.h>
#include <WHCMN/inc/whgener.h>

namespace n_whnet
{

typedef	int		whnetnfs_file;

enum
{
	WHNETNFS_CHECKOUT_ACT_MARKONLY			= 1,			// ֻ�Ǳ��ΪCheckOut
	WHNETNFS_CHECKOUT_ACT_JUSTLOAD			= 2,			// ��CheckOut�����ǻ�ȡ�ļ�
	WHNETNFS_CHECKOUT_ACT_NORMAL			= 3,			// ����CheckOut����ȡ�ļ�
	// ע�⣺ǰ�����ṹ���ǵ�����
};
enum
{
	WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT		= 1,			// ȡ���ϴ�CheckOut
	WHNETNFS_CHECKIN_ACT_JUSTSAVE			= 2,			// ��CheckIn�����Ǵ洢�ļ�
	WHNETNFS_CHECKIN_ACT_NORMAL				= 3,			// ����CheckIn���洢�ļ�
	// ע�⣺ǰ�����ṹ���ǵ�����
};

class	whnetNFS_Server		: public TCPConnecterServer, public n_whcmn::whcmddealer
{
	WHCMD_DECLARE_MAP(whnetNFS_Server)
public:
	struct	INFO_T
	{
		char	szWorkDir[WH_MAX_PATH];						// ����Ŀ¼
		char	szFNFmt[64];								// �����ֱ�Ϊ�ļ����ĸ�ʽ���磺"%08d"��
		TCPConnecterServer::INFO_T	CntrSvrInfo;			// ���ڳ�ʼ��m_CntrSvr
		tcpmsger::INFO_T			msgerinfo;				// ���ڳ�ʼ����ϢͨѶ��
		INFO_T()
		{
			strcpy(szWorkDir, ".");							// ��ǰĿ¼
			strcpy(szFNFmt, "%08d");						// 8λ�ַ����ȵ��ִ�
		}
	};
private:
	bool	m_bShouldExit;
	INFO_T	m_info;
	int		m_nCurCntrID;
	n_whcmn::whvector<char>		m_vectsend;					// ���ڷ��͵Ļ�������Ĭ��һ��ȡ�ܴ�
	n_whcmn::whcmd_cmd_buf_t	*m_pCmdBuf;					// ��cmd�����ݷֿ�������ʱ���m_pCmdBuf��Ϊm_vectsend.getbuf()
	size_t						m_nDSize;					// m_pCmdBuf���ݲ��ֵĳ���
public:
	whnetNFS_Server();
	virtual	~whnetNFS_Server();
	int		Init(INFO_T *pInfo);
	int		Release();
	int		Tick();
public:
	inline bool	IsShouldExit() const
	{
		return	m_bShouldExit;
	}
	// ��whnetNFS_I_Connecter���õ�
	bool	CanDealMsg() const;
	int		DealMsg(int nIDInMan, const void *pCmd, size_t nSize);
private:
	virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	virtual void		AfterAddConnecter(Connecter * pCntr);
private:
	// ��������
	// �������ֻ���ļ����ִ�(���szStrΪNULL��ʹ�ú����ڲ���static�ִ�)
	const char *	GetFileNameStr(whnetnfs_file nFile, char *szStr=NULL);
	// ��ǰ���ڴ�������ӷ���ָ�һ���Ƿ��ؽ��ָ�
	int		SendCmd2CurConnecter();
	// ���ƶ������ӷ���ָ��
	int		SendCmd2Connecter(int nCntrID);
private:
	// ������Ϣ������ص�
	void	DealMsg_REQ_SYSEXIT(const char *szPass);
	void	DealMsg_REQ_FILELOAD(const void *pParam, size_t nSize);
	void	DealMsg_REQ_FILESAVE(const void *pParam, size_t nSize);
	// ����һ���ļ�����ǰconnecter
	void	_DealMsg_REQ_FILELOAD_SendOneFileToCurCntr(whnetnfs_file nFile, unsigned char nCheckOutAct);
};

class	whnetNFS_Client			: public whstatusholder
{
	WHSH_DECLARETHISCLASS(whnetNFS_Client);
public:
	enum
	{
		STATUS_NOTHING		= 0,				// ��ʼ̬
		STATUS_TRYCONNECT	= 1,				// ����������
		STATUS_WORKBEGIN	= 2,				// ���ӳɹ�������ǰ�Ĳ����������ԭ��SHM��ʣ�µ��û���ɫ���ݷ���PNLDB��
		STATUS_WORKING		= 3,				// ���ӳɹ�������������
	};
	whstatusworker			m_statusTRYCONNECT;
	whstatusworker			m_statusWORKING;
private:
	// ��worker�ǵ��õ�
	void	Worker_TRYCONNECT_Begin();
	void	Worker_TRYCONNECT_Work();
	void	Worker_WORKING_Begin();
	void	Worker_WORKING_Work();
	void	Worker_WORKING_End();
public:
	// ��ʼ���ṹ
	struct	INFO_T
	{
		char	szSvrAddr[WHNET_MAXADDRSTRLEN];			// �������ĵ�ַ
		float	fSendRate;								// �����ļ�������
		int		nConnectTimeOut;						// ���ӷ������ĳ�ʱ
		bool	bNoSaveWhenNoLoadAll;					// ���û��ȫ�������򲻴洢
		bool								bInherit;	// �Ƿ��Ǽ̳�fmInfo�е�pBuf
		n_whcmn::whmemfileman::INFO_T		fmInfo;		// ���ڳ�ʼ��fileman
		n_whcmn::whfixedtimeevent::INFO_T	fteInfo;	// ���ڳ�ʼ����ʱ�����¼�
		tcpmsger::INFO_T					msgerinfo;	// ���ڳ�ʼ����ϢͨѶ��
		INFO_T()
		: fSendRate(40)
		, nConnectTimeOut(5000)
		, bNoSaveWhenNoLoadAll(true)
		, bInherit(false)
		{
			strcpy(szSvrAddr, "localhost:4102");
		}
	};
	// �ʹ�����ص�
	enum
	{
		EXTIDX_POSINSAVELIST	= 0,					// ���ļ���SaveList�ж�Ӧ�Ķ���ָ��
		EXTIDX_TIMEEVENTID		= 1,					// ���ļ��ڶ�ʱ�����¼��б��е�ID
	};
	struct	SAVEUNIT_T
	{
		whnetnfs_file		nFile;						// �ļ���
		time_t				nStartTime;					// ���������е�ʱ��
		unsigned char		nCheckInAct;				// checkin��������WHNETNFS_CHECKIN_ACT_XXX
		inline bool operator == (const SAVEUNIT_T & other) const
		{
			return	other.nFile == nFile;
		}
	};
	// �ļ���������ṹ
	#pragma pack(1)
	struct	FILEWORK_T									// ��������loadup��loadup�ɹ����֪ͨ��ֻ�ܳɹ�����ʧ�ܣ�
	{
		enum
		{
			TYPE_ARRAY	= 0,							// nFile[0]��������nFile[1]~�Ǹ���file������
			TYPE_ONE	= 1,							// nFile[0]
			TYPE_RANGE	= 2,							// [nFile[0],nFile[1])��ע�������ʾnFile[0]<=��<nFile[1]
		};
		unsigned char		nType;						// ���ͣ���ǰ���TYPE_XXX��
		whnetnfs_file		nFile[1];					// �ļ�������
		int	GetTotalSize() const						// ��ñ��ṹ���ܳߴ�
		{
			switch( nType )
			{
				case	TYPE_ARRAY:
				return	sizeof(*this) + nFile[0]*sizeof(whnetnfs_file);
				case	TYPE_ONE:
				return	sizeof(*this);
				case	TYPE_RANGE:
				return	sizeof(*this) + 1*sizeof(whnetnfs_file);
			}
			// Ӧ�ò��ص������
			return	0;
		}
		inline int	GetFileNum() const					// ��������������ļ�����
		{
			switch( nType )
			{
				case	TYPE_ARRAY:
				return	nFile[0];
				case	TYPE_ONE:
				return	1;
				case	TYPE_RANGE:
				return	nFile[1] - nFile[0];
			}
			// Ӧ�ò��ص������
			return	0;
		}
	};
	#pragma pack()
	// ���ڶ�ʱ���������¼��Ķ�ʱ������
	struct	SAVEEVENTMAN :	public n_whcmn::whfixedtimeevent
	{
	public:
		// ��ʱ���̶��ǲ���Ҫcheckin��
		struct	EVENTINFO_T
		{
			whnetnfs_file	nFile;						// �ļ���
		};
		whnetNFS_Client		*m_pHost;					// ���ϲ�����֮
	private:
		// ���Ҫ������ʵ��
		virtual int	DealEvent(void *pData, int nSize)
		{
			return	m_pHost->DealSaveEvent(pData, nSize);
		}
	};
private:
	INFO_T					m_info;
	tcpretryer				m_Retryer;					// ������ڲ��ϳ�����PNLDB
	tcpmsger				m_Msger;					// ������ں�serverͨѶ
	n_whcmn::whmemfileman	m_MemFileMan;
	n_whcmn::whvector<char>	m_vectsend;					// ���ڷ��͵Ļ�������Ĭ��һ��ȡ�ܴ�
	n_whcmn::whcmd_cmd_buf_t		*m_pCmdBuf;			// ��cmd�����ݷֿ�������ʱ���m_pCmdBuf��Ϊm_vectsend.getbuf()
	size_t							m_nDSize;			// m_pCmdBuf���ݲ��ֵĳ���
	// ���������ݿ���ļ��Ķ���
	n_whcmn::whlist<SAVEUNIT_T>		m_listSave;			// ���̶���
	n_whcmn::whinterval		m_itv;						// ���ڻ��tick��ʱ����
	n_whcmn::whgener		m_gener;					// ��������
	n_whcmn::whvector<char>	m_vectToLoad;				// �������load���ļ��ṹ
	FILEWORK_T				*m_pToLoad;					// ָ��m_vectToLoad
	int						m_nNumToLoad;				// ����load�ĸ���
	bool					m_bReleased;				// �Ѿ���Release����
	SAVEEVENTMAN			m_fteSave;					// ��ʱ�����¼�
public:
	inline n_whcmn::whmemfileman *	GetFileMan()		// �ϲ����ʹ��������û��ļ����ж�д
	{
		return	&m_MemFileMan;
	}
public:
	whnetNFS_Client();
	virtual	~whnetNFS_Client();
	int		Init(INFO_T *pInfo);
	int		Init_Retryer();
	int		Release();
	int		Release_Clean();							// �������ļ����Server��������Զ�����Tick��
	int		Tick();
	int		FileLoadReq(FILEWORK_T *pInfo);				// �����Svr�����ļ�
	int		FileSaveReq(FILEWORK_T *pInfo, unsigned char nCheckInAct);
														// ���뽫�ļ�����Svr
	int		MarkDelAndClean(int nFile);					// ���ɾ��������ļ�(��Ӧ�Ķ�ʱ�¼�����������ȵ��õ�ʱ�򣬷����ļ��ı�Ϊ�գ�����Ҫ���)
	// ��SAVEEVENTMAN���õ�
	int		DealSaveEvent(void *pData, int nSize);
protected:
	// ������Ҫ���̵��ļ��б�
	int		MakeListSave();
	// ���Ѿ�����ȷ�ϴ����б�Ϊ�յ�����£�������bSearchΪfalse
	int		WishSendFileToServer(whnetnfs_file nFile, unsigned char nCheckInAct, bool bSearch=true);
	// ���Է�������������ļ�
	void	_DoSendFileToServer_And_SendRecv();
	// ����һ���ļ�(��_DoSendFileToServer_And_SendRecv�ڲ�����)
	int		_DoSendOneFileToServer(whnetnfs_file nFile, unsigned char nCheckInAct);
	// �����������ָ��
	int		SendCmd2Server();
	// Tick�е��õ�(ԭ����tick���ڸ�ΪWorker_WORKING_Work��)
	int		Worker_WORKING_Work_DealServerRPL();
	int		Worker_WORKING_Work_DealServerRPL_FILELOAD(const void *pParam, size_t nSize);
	int		Worker_WORKING_Work_DealServerRPL_FILESAVE(const void *pParam, size_t nSize);
	// ע����ļ��Ķ�ʱ�����¼�
	int		_RegFileSaveEvent(n_whcmn::whmemfile::FILEINFO_T *pFileInfo);
private:
	virtual int	FileLoadOK_Notify(FILEWORK_T *pInfo)	= 0;
														// �ɹ�load���֮���֪ͨ(pInfo��FileLoadUpָ���е�����һ��)
	virtual int	DoFileSave_Notify(whnetnfs_file nFile)	= 0;
														// һ���Ƕ�ʱ������Ҫ֪ͨ�ϲ��ȴ�һ����
};

}		// EOF namespace n_whnet

#endif
