// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpGLogger.h
// Creator      : Wei Hua (κ��)
// Comment      : ����UDP��ͨ����־��¼��
// CreationDate : 2003-05-21
// ChangeLOG    : 2005-09-07 ������Log��MT��֧��
//              : 2006-11-30 ������GLOGGER_ID_FATAL
//              : 2007-02-28 ������GLOGGER_ID_DATARECOVER
//              : ����GLOGGER_ID_HACK, GLOGGER_ID_FATAL, GLOGGER_ID_DATARECOVER�������µ�ID
//              : ͬʱΪ�����÷��㣬���Ҳ�����Ӧ����־�����ʱ��Ĭ�ϰ���־�����CMN��
//              : 2007-03-02 ��GLogger_Init��Ĭ�ϲ�����8��Ϊ16
//              : ������GLoggerMan::GetLogger�������ڲ�ʵ������Logger��Ų����ڵ�ʱ���Զ��л���Ĭ��Logger�Ĺ��ܣ���Ȼ���û��Ĭ��Logger��ô�ͳ���ɣ�
//              : 2007-04-05 ���ϵ�GLOGGER��ص�ID�ͺ궼ȡ����
//              : 2007-04-10 ������GLogger_WriteFmtStd

#ifndef	__WHNETUDPGLOGGER_H__
#define	__WHNETUDPGLOGGER_H__

#include "whnetcmn.h"
#include <WHCMN/inc/whlog.h>
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whtime.h>
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whlock.h>
#include <stdarg.h>

using n_whcmn::whtick_t;
using n_whcmn::whlooper;
using n_whcmn::whlogwriter;
using n_whcmn::whunitallocatorFixed;

namespace n_whnet
{

// GLOG���õ��Ľṹ�������
enum
{
	GLOG_MAX_TAG				= 32,			// exe��ǩ����󳤶�
};
// ����
enum
{
	GLOG_CMD_REQ_KEEPALIVE		= 0x00,			// keepalive���������ӡ�
												// ע�⣺�������msg��η��Ͷ���ָ��ʱ���ڷ���һ�Ρ�
												// �ṹ��GLOG_CMD_INT_T
	GLOG_CMD_REQ_CONNECT		= 0x01,			// ���ӷ�����
												// �ṹ��GLOG_CMD_REQ_MSG_T
	GLOG_CMD_REQ_DISCONNECT		= 0x02,			// �ӷ������Ͽ�
												// �ṹ��GLOG_CMD_INT_T
	GLOG_CMD_REQ_MSG			= 0x03,			// һ����Ϣ
												// �ṹ��GLOG_CMD_REQ_MSG_T
	GLOG_CMD_REQ_COUNT			= 0x04,			// ��Ϣ����
												// �ṹ��GLOG_CMD_REQ_COUNT_T
	GLOG_CMD_REQ_CONTROL		= 0x10,			// ����ָ��
												// �ṹ��GLOG_CMD_REQ_CONTROL_T
};
// ����
enum
{
	GLOG_CMD_RPL_CONNECT		= GLOG_CMD_REQ_CONNECT,
												// �ṹ��GLOG_CMD_RPL_CONNECT_T
};
#pragma pack(1)
// ����
struct	GLOG_CMD_T
{
	unsigned char	cmd;
};
struct	GLOG_CMD_INT_T
{
	unsigned char	cmd;
	int				nParam;
};
// ����
struct	GLOG_CMD_REQ_CONNECT_T
{
	unsigned char		cmd;
	char				szExeTag[GLOG_MAX_TAG];	// Exe��ʶ����Ҫ���ڹ���ʽ�У���ʶ��Դ��
};
struct	GLOG_CMD_REQ_MSG_T
{
	unsigned char		cmd;
	int					nID;					// �Լ���ID
	char				szMsg[1];				// ���ݲ�(���ȿ���ͨ���ܳ����������)
												// (������ִ�����������Ϣ��֤������0��β)
};
struct	GLOG_CMD_REQ_COUNT_T
{
	unsigned char		cmd;
	int					nID;					// �Լ���ID
	unsigned int		nCount;					// �ѷ��͵���Ϣ�ļ���
};
struct	GLOG_CMD_REQ_CONTROL_T
{
	unsigned char		cmd;
	char				szCmdStr[1];			// ���ݲ�(���ȿ���ͨ���ܳ����������)
};
// �������
struct	GLOG_CMD_RPL_CONNECT_T
{
	enum
	{
		RST_OK				= 0x00,				// �ɹ�
		RST_ERR				= 0xFF,				// δ֪�Ĵ���
		RST_ERR_NOROOM		= 0xFE,				// û�еط��ˡ����Ѿ�����Server��logger���������˹涨ֵ��
	};
	unsigned char		cmd;
	unsigned char		rst;					// ���
	int					nID;					// ����ɹ����᷵������ID
};
#pragma pack()

// ����־��¼������
class	GLogServerer
{
public:
	struct	INFO_T
	{
		int					nSelectInterval;	// ѡ��ļ��
		int					nKeepAliveTimeOut;	// KeepAlive�ĳ�ʱ(��)
		int					nMaxLogger;			// �������ɵ�������־����(�ļ���������������ͬ)
		char				szBindAddr[WHNET_MAXADDRSTRLEN];
												// �󶨵�����˿ڼ���
		bool				bAppendTagName;		// ��exe�����ַŵ�ÿһ��
		bool				bAppendSourceID;	// ����Դ��ID�ŵ�Name����
		bool				bRawWrite;			// û���κ����ε�д
		INFO_T()
		: nSelectInterval(100)
		, nKeepAliveTimeOut(60)
		, nMaxLogger(100)
		, bAppendTagName(true)
		, bAppendSourceID(false)
		, bRawWrite(false)
		{
			strcpy(szBindAddr, ":2004");
		}
	};
protected:
	// һ����¼��Դ��Ԫ
	struct	LoggerUnit
	{
		int				nID;					// �Լ���ID
		whtick_t		nLastKATick;			// �ϴ��յ�KeepAlive����ʱ�䣨��λ���룩
		struct sockaddr_in	addr;				// ��Դ��ַ
		char			szExeTag[GLOG_MAX_TAG + 64];	// Exe��ʶ(�����һЩ��׺)
		bool			bCanBeRemoved;			// ���Ա��Ƴ��������û�пռ������¿��԰������Ķ����Ƴ�
		unsigned int	nCount;					// �յ�����Ϣ���������ڼ���Ƿ񶪰�
		void	clear()
		{
			memset(this, 0, sizeof(*this));
		}
	};

	SOCKET					m_sock;				// ���ڼ�����socket
	struct sockaddr_in		m_fromaddr;			// Tick�е�ǰ������������������ַ
	whunitallocatorFixed<LoggerUnit>	m_Loggers;
												// ����LoggerUnit�ķ���
	INFO_T					m_info;				// ��¼��ʼ����Ϣ
	int						m_nKATimeOut;		// KeepAlive�ĳ�ʱ����λ���룩

	n_whcmn::whlogwriter	m_logwriter;		// ��־��д��
public:
	GLogServerer();
	~GLogServerer();
	int		Init(INFO_T *pInfo, whlogwriter::INFO_T	*pLogWRInfo);
	int		Release();
	int		Tick();								// һ�ι�������
public:
	// ����ֱ�Ӳٿ�logwriter
	inline n_whcmn::whlogwriter *	GetLogWriter()
	{
		return	&m_logwriter;
	}
private:
	int		Tick_KeepAlive();
	// �����������
	int		Tick_REQ_KEEPALIVE(GLOG_CMD_INT_T *pCmd);
	int		Tick_REQ_CONNECT(GLOG_CMD_REQ_CONNECT_T *pCmd);
	int		Tick_REQ_DISCONNECT(GLOG_CMD_INT_T *pCmd);
	int		Tick_REQ_MSG(GLOG_CMD_REQ_MSG_T *pCmd, int nSize);
	int		Tick_GLOG_CMD_REQ_COUNT(GLOG_CMD_REQ_COUNT_T *pCmd, int nSize);
	// �ڲ�����
	int		RemoveLoggerUnit(LoggerUnit *pLoggerUnit);
	int		RemoveLoggerUnit(int nID);
	// ����ָ���ǰ��ַ(���ոշ������ݵĵ�ַ)
	int		SendToCurAddr(const void *pCmd, size_t nSize);
};

// ����־��¼��
class	GLogger
{
public:
	struct	INFO_T
	{
		char	szDstAddr[WHNET_MAXADDRSTRLEN];
		char	szExeTag[GLOG_MAX_TAG];
		int		nKeepAliveInterval;							// KeepAlive��ʱ����(��)
		int		nCountCheckInterval;						// ����Ƿ񶪰���ʱ����(����)
		int		nConnectTimeOut;							// ���ӷ������ĳ�ʱ(����)
		bool	bSupportMT;									// �Ƿ�֧�ֶ��߳�
		INFO_T()
		: nKeepAliveInterval(22)
		, nCountCheckInterval(1000)
		, nConnectTimeOut(1000)
		, bSupportMT(false)
		{
			szDstAddr[0]	= 0;
			szExeTag[0]		= 0;
		}
	};
private:
	INFO_T		m_info;										// ��ʼ����Ϣ
	SOCKET		m_sock;										// ͨѶ��socket
	int			m_nID;										// ����GLogServerer��ȡ��ID
	char		m_szFmtStrBuf[whlogwriter::MAXMSGLEN];		// �������ɸ�ʽ�ı�
	char		m_szMsgBuf[whlogwriter::MAXMSGLEN];			// �������Ҳ����ô��
	whlooper	m_keepaliveloop;							// ���ڷ���keepalive��ʱ
	whlooper	m_checkcountloop;							// ���ڶ�ʱ����Ƿ񶪰�
	int			m_nCount;									// ��Ϣ���������ڼ���Ƿ񶪰�
	n_whcmn::whlock		*m_pLock;							// �����̼߳����
public:
	inline SOCKET	GetSock() const
	{
		return	m_sock;
	}
public:
	GLogger();
	~GLogger();
	enum
	{
		INITRST_OK			= 0,							// �ɹ�
		INITRST_ERR_SOCK	= -1,							// �޷�����socket
		INITRST_ERR_ADDR	= -2,							// �޷�������ַ
		INITRST_ERR_SVR		= -3,							// ������־����������(�����������ڻ��߷��������س���)
	};
	// Init�ķ��ؿ���Ϊ�����enum��INITRST_XXX
	int		Init(INFO_T *pInfo);							// ��ʼ����szDstAddrΪLOG��¼Server�ĵ�ַ�����磺ip:port��i.e."127.0.0.1:1976"
	int		Release();										// �սᡣ
	int		Tick();											// ��Ҫ������keepalive
	int		WriteBin(const void *pData, size_t nSize);		// д���������ݣ���������Զ����Ʒ�ʽ��
	int		WriteLine(const char *szStr);					// д���ı��У�����������ı���ʽ��
	int		WriteFmtLine(const char *szFmt, ...);			// д���ʽ�ı�������������ı���ʽ��
	int		WriteFmtLine0(const char *szFmt, va_list arglist);
private:
	// ���غ�send����һ��
	int		RawWrite(const void *pData, size_t nSize);		// ��Ŀ�ĵ�ַ���Ͱ�
};

// ��logger�Ĺ�����
class	GLoggerMan
{
private:
	struct	GLOGGERUNIT_T
	{
		GLogger			*pLogger;
		GLogger::INFO_T	info;								// ���ɸ�GLogger����Ϣ
		GLOGGERUNIT_T()
		: pLogger(NULL)
		{
		}
	};
	n_whcmn::whvector<GLOGGERUNIT_T, true>		m_vectUnits;
	int		m_nDftLoggerIdx;								// ���ĳ��Logger�����ڣ���Ĭ�ϰѶ�Ӧ����־д�����Logger���棨Ĭ����0��
public:
	GLoggerMan(int nNum);
	~GLoggerMan();
	int		AddLogger(int nID, GLogger::INFO_T *pInfo);
	int		RemoveAllLogger();
	int		WriteStr(int nID, const char *szStr);
	int		WriteFmt(int nID, const char *szFmt, ...);
	int		WriteFmt0(int nID, const char *szFmt, va_list arglist);
	int		Tick();
	inline void	SetDftLoggerIdx(int nIdx)
	{
		assert(nIdx>=0 && nIdx<(int)m_vectUnits.size());
		m_nDftLoggerIdx	= nIdx;
	}
private:
	GLogger *	GetLogger(int nID);
};

// ����GLoggerMan��ȫ��Log����
int		GLogger_Init(int nNum=16);
int		GLogger_Release();
int		GLogger_Add(int nID, GLogger::INFO_T *pInfo);
// GLogger_AddMulti�л�ı�pCmnInfo������
int		GLogger_AddMulti(GLogger::INFO_T *pCmnInfo, const char *cszAddrs);
int		GLogger_WriteFmt(int nID, const char *szFmt, ...);
int		GLogger_WriteStr(int nID, const char *szStr);
int		GLogger_Tick();
void	GLogger_Set_PrintInScreenOnly(bool bSet);
void	GLogger_Set_PrintInScreenToo(bool bSet);

// ���þɵĶ���
enum
{
	// һ����԰���־д�����ID 
	GLOGGER_ID_CMN					=	0,
	// ��Ҫ����д������
	GLOGGER_ID_ERROR				=	1,
	// ����������ص�д������(������Ϸ�к��û�������صľͷŵ������Ӧ��ԭ�������е�client��־������������һЩ���Էֵ�player��npc��־��ȥд)
	GLOGGER_ID_STORY				=	2,
	// Ԥ��֮��Ĵ��󣬿�����hack���µ�
	GLOGGER_ID_HACK					=	3,
	// �ǳ����صĴ��󣨱���debug��Ҫassert�����Ĵ���
	GLOGGER_ID_FATAL				=	4,
	// ��¼�ؼ��ԵĻָ������õ���־
	GLOGGER_ID_DATARECOVER			=	5,
	// ��Ʒ���أ������ʺŵ�login / logout / billing
	GLOGGER_ID_BILLING				=	6,
	// ���ɫ���ݺͽ�ɫ�佻����أ�������ɫlogin / logout / new / delete / exchange/ task��
	GLOGGER_ID_PLAYER				=	7,
	// ս�������־
	GLOGGER_ID_FIGHT				=	8,
	// �������أ��������ɡ���á����������ס��ϳɵ�
	GLOGGER_ID_ITEM					=	9,
	// NPC�����־
	GLOGGER_ID_NPC					=	10,
	// ���л���أ������л�Ĵ���/����/��ɢ/��Ȩ
	GLOGGER_ID_CLAN					=	11,
	// ��ɫ�����Ļ�������˽��/�ڸ���Ƶ�����˵��
	GLOGGER_ID_TALK					=	12,
	// GM �����л
	GLOGGER_ID_GM					=	13,
	// �û�������־
	GLOGGER_ID_SURVEY				=	14,
};

////////////////////////////////////////////////////////////////////
// ΪDLL֮����Ϣͬ��
////////////////////////////////////////////////////////////////////
void *	WHNET_UDPGLOGGER_STATIC_INFO_Out();
void	WHNET_UDPGLOGGER_STATIC_INFO_In(void *pInfo);

}		// EOF namespace n_whnet

// �мǲ�Ҫ����Щ���������κμ�����صĶ�������i++֮��ģ���Ϊ����һ����ע���ˣ��Ͳ��������

// ���þɵĶ���
#define	GLOGGER2_INIT						n_whnet::GLogger_Init
#define	GLOGGER2_RELEASE					n_whnet::GLogger_Release
#define	GLOGGER2_ADD						n_whnet::GLogger_Add
#define	GLOGGER2_ADDMULTI					n_whnet::GLogger_AddMulti
#define	GLOGGER2_WRITEFMT					n_whnet::GLogger_WriteFmt
#define	GLOGGER2_WRITESTR					n_whnet::GLogger_WriteStr
#define	GLOGGER2_TICK						n_whnet::GLogger_Tick
#define	GLOGGER2_SET_PRINTINSCREENONLY		n_whnet::GLogger_Set_PrintInScreenOnly
#define	GLOGGER2_SET_PRINTINSCREENTOO		n_whnet::GLogger_Set_PrintInScreenToo
#define	GLOGGER2_INFO_T						n_whnet::GLogger::INFO_T
// ר����������־��ʽ��ǰ�����Ӵ���͹ؼ��ֵ�
#define GLGR_STD_HDR(code,keyword)			#code","#keyword","

#endif	// EOF __WHNETUDPGLOGGER_H__
