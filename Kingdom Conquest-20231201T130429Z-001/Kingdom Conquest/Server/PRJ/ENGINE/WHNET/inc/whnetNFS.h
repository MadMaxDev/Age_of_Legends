// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetNFS.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于TCP的简单网络文件系统（文件名为int整数）（只支持单线程的）
//                为了设计简单，所有文件需要小于1M。
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
	WHNETNFS_CHECKOUT_ACT_MARKONLY			= 1,			// 只是标记为CheckOut
	WHNETNFS_CHECKOUT_ACT_JUSTLOAD			= 2,			// 不CheckOut，但是获取文件
	WHNETNFS_CHECKOUT_ACT_NORMAL			= 3,			// 正常CheckOut并获取文件
	// 注意：前两个结构就是第三个
};
enum
{
	WHNETNFS_CHECKIN_ACT_UNDOCHECKOUT		= 1,			// 取消上次CheckOut
	WHNETNFS_CHECKIN_ACT_JUSTSAVE			= 2,			// 不CheckIn，但是存储文件
	WHNETNFS_CHECKIN_ACT_NORMAL				= 3,			// 正常CheckIn并存储文件
	// 注意：前两个结构就是第三个
};

class	whnetNFS_Server		: public TCPConnecterServer, public n_whcmn::whcmddealer
{
	WHCMD_DECLARE_MAP(whnetNFS_Server)
public:
	struct	INFO_T
	{
		char	szWorkDir[WH_MAX_PATH];						// 工作目录
		char	szFNFmt[64];								// 从数字变为文件名的格式（如："%08d"）
		TCPConnecterServer::INFO_T	CntrSvrInfo;			// 用于初始化m_CntrSvr
		tcpmsger::INFO_T			msgerinfo;				// 用于初始化消息通讯器
		INFO_T()
		{
			strcpy(szWorkDir, ".");							// 当前目录
			strcpy(szFNFmt, "%08d");						// 8位字符长度的字串
		}
	};
private:
	bool	m_bShouldExit;
	INFO_T	m_info;
	int		m_nCurCntrID;
	n_whcmn::whvector<char>		m_vectsend;					// 用于发送的缓冲区（默认一次取很大）
	n_whcmn::whcmd_cmd_buf_t	*m_pCmdBuf;					// 把cmd和数据分开，构造时会把m_pCmdBuf设为m_vectsend.getbuf()
	size_t						m_nDSize;					// m_pCmdBuf数据部分的长度
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
	// 由whnetNFS_I_Connecter调用的
	bool	CanDealMsg() const;
	int		DealMsg(int nIDInMan, const void *pCmd, size_t nSize);
private:
	virtual Connecter *	NewConnecter(SOCKET sock, struct sockaddr_in *paddr);
	virtual void		AfterAddConnecter(Connecter * pCntr);
private:
	// 辅助功能
	// 根据数字获得文件名字串(如果szStr为NULL则使用函数内部的static字串)
	const char *	GetFileNameStr(whnetnfs_file nFile, char *szStr=NULL);
	// 向当前正在处理的连接发送指令（一般是返回结果指令）
	int		SendCmd2CurConnecter();
	// 向制定的连接发送指令
	int		SendCmd2Connecter(int nCntrID);
private:
	// 具体消息处理相关的
	void	DealMsg_REQ_SYSEXIT(const char *szPass);
	void	DealMsg_REQ_FILELOAD(const void *pParam, size_t nSize);
	void	DealMsg_REQ_FILESAVE(const void *pParam, size_t nSize);
	// 发送一个文件到当前connecter
	void	_DealMsg_REQ_FILELOAD_SendOneFileToCurCntr(whnetnfs_file nFile, unsigned char nCheckOutAct);
};

class	whnetNFS_Client			: public whstatusholder
{
	WHSH_DECLARETHISCLASS(whnetNFS_Client);
public:
	enum
	{
		STATUS_NOTHING		= 0,				// 初始态
		STATUS_TRYCONNECT	= 1,				// 尝试连接中
		STATUS_WORKBEGIN	= 2,				// 连接成功，开工前的操作（比如把原来SHM中剩下的用户角色数据发给PNLDB）
		STATUS_WORKING		= 3,				// 连接成功，正常工作中
	};
	whstatusworker			m_statusTRYCONNECT;
	whstatusworker			m_statusWORKING;
private:
	// 给worker们调用的
	void	Worker_TRYCONNECT_Begin();
	void	Worker_TRYCONNECT_Work();
	void	Worker_WORKING_Begin();
	void	Worker_WORKING_Work();
	void	Worker_WORKING_End();
public:
	// 初始化结构
	struct	INFO_T
	{
		char	szSvrAddr[WHNET_MAXADDRSTRLEN];			// 服务器的地址
		float	fSendRate;								// 发送文件的速率
		int		nConnectTimeOut;						// 连接服务器的超时
		bool	bNoSaveWhenNoLoadAll;					// 如果没有全部载入则不存储
		bool								bInherit;	// 是否是继承fmInfo中的pBuf
		n_whcmn::whmemfileman::INFO_T		fmInfo;		// 用于初始化fileman
		n_whcmn::whfixedtimeevent::INFO_T	fteInfo;	// 用于初始化定时存盘事件
		tcpmsger::INFO_T					msgerinfo;	// 用于初始化消息通讯器
		INFO_T()
		: fSendRate(40)
		, nConnectTimeOut(5000)
		, bNoSaveWhenNoLoadAll(true)
		, bInherit(false)
		{
			strcpy(szSvrAddr, "localhost:4102");
		}
	};
	// 和存盘相关的
	enum
	{
		EXTIDX_POSINSAVELIST	= 0,					// 该文件在SaveList中对应的对象指针
		EXTIDX_TIMEEVENTID		= 1,					// 该文件在定时存盘事件列表中的ID
	};
	struct	SAVEUNIT_T
	{
		whnetnfs_file		nFile;						// 文件名
		time_t				nStartTime;					// 最初放入队列的时刻
		unsigned char		nCheckInAct;				// checkin动作，见WHNETNFS_CHECKIN_ACT_XXX
		inline bool operator == (const SAVEUNIT_T & other) const
		{
			return	other.nFile == nFile;
		}
	};
	// 文件操作请求结构
	#pragma pack(1)
	struct	FILEWORK_T									// 用于申请loadup和loadup成功后的通知（只能成功不能失败）
	{
		enum
		{
			TYPE_ARRAY	= 0,							// nFile[0]是数量，nFile[1]~是各个file的名字
			TYPE_ONE	= 1,							// nFile[0]
			TYPE_RANGE	= 2,							// [nFile[0],nFile[1])，注意这个表示nFile[0]<=且<nFile[1]
		};
		unsigned char		nType;						// 类型（见前面的TYPE_XXX）
		whnetnfs_file		nFile[1];					// 文件名数组
		int	GetTotalSize() const						// 获得本结构的总尺寸
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
			// 应该不回到这里的
			return	0;
		}
		inline int	GetFileNum() const					// 获得里面囊括的文件总数
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
			// 应该不回到这里的
			return	0;
		}
	};
	#pragma pack()
	// 用于定时产生存盘事件的定时管理器
	struct	SAVEEVENTMAN :	public n_whcmn::whfixedtimeevent
	{
	public:
		// 定时存盘都是不需要checkin的
		struct	EVENTINFO_T
		{
			whnetnfs_file	nFile;						// 文件名
		};
		whnetNFS_Client		*m_pHost;					// 由上层设置之
	private:
		// 这个要被子类实现
		virtual int	DealEvent(void *pData, int nSize)
		{
			return	m_pHost->DealSaveEvent(pData, nSize);
		}
	};
private:
	INFO_T					m_info;
	tcpretryer				m_Retryer;					// 这个用于不断尝试连PNLDB
	tcpmsger				m_Msger;					// 这个用于和server通讯
	n_whcmn::whmemfileman	m_MemFileMan;
	n_whcmn::whvector<char>	m_vectsend;					// 用于发送的缓冲区（默认一次取很大）
	n_whcmn::whcmd_cmd_buf_t		*m_pCmdBuf;			// 把cmd和数据分开，构造时会把m_pCmdBuf设为m_vectsend.getbuf()
	size_t							m_nDSize;			// m_pCmdBuf数据部分的长度
	// 用于向数据库存文件的队列
	n_whcmn::whlist<SAVEUNIT_T>		m_listSave;			// 存盘队列
	n_whcmn::whinterval		m_itv;						// 用于获得tick的时间间隔
	n_whcmn::whgener		m_gener;					// 发送速率
	n_whcmn::whvector<char>	m_vectToLoad;				// 存放期望load的文件结构
	FILEWORK_T				*m_pToLoad;					// 指向m_vectToLoad
	int						m_nNumToLoad;				// 期望load的个数
	bool					m_bReleased;				// 已经被Release过了
	SAVEEVENTMAN			m_fteSave;					// 定时存盘事件
public:
	inline n_whcmn::whmemfileman *	GetFileMan()		// 上层可以使用这个对用户文件进行读写
	{
		return	&m_MemFileMan;
	}
public:
	whnetNFS_Client();
	virtual	~whnetNFS_Client();
	int		Init(INFO_T *pInfo);
	int		Init_Retryer();
	int		Release();
	int		Release_Clean();							// 将所有文件存回Server（里面会自动调用Tick）
	int		Tick();
	int		FileLoadReq(FILEWORK_T *pInfo);				// 申请从Svr载入文件
	int		FileSaveReq(FILEWORK_T *pInfo, unsigned char nCheckInAct);
														// 申请将文件存入Svr
	int		MarkDelAndClean(int nFile);					// 标记删除并清空文件(相应的定时事件不用清除，等到用的时候，发现文件改变为空，还是要存地)
	// 被SAVEEVENTMAN调用的
	int		DealSaveEvent(void *pData, int nSize);
protected:
	// 生成需要存盘的文件列表
	int		MakeListSave();
	// 在已经可以确认存盘列表为空的情况下，可以令bSearch为false
	int		WishSendFileToServer(whnetnfs_file nFile, unsigned char nCheckInAct, bool bSearch=true);
	// 尝试发送最大数量的文件
	void	_DoSendFileToServer_And_SendRecv();
	// 发送一个文件(由_DoSendFileToServer_And_SendRecv内部调用)
	int		_DoSendOneFileToServer(whnetnfs_file nFile, unsigned char nCheckInAct);
	// 向服务器发送指令
	int		SendCmd2Server();
	// Tick中调用的(原来的tick现在改为Worker_WORKING_Work了)
	int		Worker_WORKING_Work_DealServerRPL();
	int		Worker_WORKING_Work_DealServerRPL_FILELOAD(const void *pParam, size_t nSize);
	int		Worker_WORKING_Work_DealServerRPL_FILESAVE(const void *pParam, size_t nSize);
	// 注册该文件的定时存盘事件
	int		_RegFileSaveEvent(n_whcmn::whmemfile::FILEINFO_T *pFileInfo);
private:
	virtual int	FileLoadOK_Notify(FILEWORK_T *pInfo)	= 0;
														// 成功load完毕之后的通知(pInfo和FileLoadUp指令中的内容一致)
	virtual int	DoFileSave_Notify(whnetnfs_file nFile)	= 0;
														// 一般是定时存盘需要通知上层先存一次盘
};

}		// EOF namespace n_whnet

#endif
