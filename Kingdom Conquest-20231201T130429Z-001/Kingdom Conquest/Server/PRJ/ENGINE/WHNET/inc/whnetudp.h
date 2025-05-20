// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File: whnetudp.h
// Creator: Wei Hua (κ��)
// Comment: �򵥵�UDP���繦��
// CreationDate: 2003-05-15

#ifndef	__WHNETUDP_H__
#define	__WHNETUDP_H__

#include "whnetcmn.h"
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whunitallocator.h>

namespace n_whnet
{

////////////////////////////////////////////////////////////////////
// ��ͨ����
////////////////////////////////////////////////////////////////////
// ֻ�Ǵ���һ���ͱ��ض˿ڰ󶨵�socket
// ���__addr�ǿ��򷵻ذ󶨵��ĵ�ַ
SOCKET	udp_create_socket(port_t __localport, struct sockaddr_in *__addr = 0);
// �����ͱ���ĳ��ip�Ͷ˿ڰ󶨵�socket(__ipΪ�ձ�ʾ���ض�����ĳ��������ַ��)
SOCKET	udp_create_socket(const char *__ip, port_t __localport, struct sockaddr_in *__addr = 0);
// ����socket�����һ����Χ�ڵĶ˿�
SOCKET	udp_create_rand_socket(port_t __port1, port_t __port2, struct sockaddr_in *__addr = 0);
SOCKET	udp_create_rand_socket(const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr = 0);
// ͨ���򵥵�ַ�ṹ����udp
inline SOCKET	udp_create_socket_byeasyaddr(const EASYADDR *__eaddr, struct sockaddr_in *__addr = 0)
{
	return	udp_create_rand_socket(__eaddr->szAddrStr, __eaddr->port, __eaddr->port2, __addr);
}
SOCKET	udp_create_socket_by_ipnportstr(const char *__ipnportstr, struct sockaddr_in *__addr = 0);

// �������ڱ���ͨѶ��socket�ܵ�
// �ڲ�ͬ�߳��ж�����ͨ��send�����з��ͣ���recv�����н�������
// nRecvBufSize��ʾ���ջ���Ĵ�С��0��ʾʹ��Ĭ�ϵ�(Windows��9000�ֽڣ�Linux��64k)
SOCKET	udp_create_lo(int __recvbufsize = 0);

// ��SOCKET LO����һ���¼�����
// �����¼�
bool	udp_socketevent_set(SOCKET __sock);
// ����¼�
bool	udp_socketevent_clr(SOCKET __sock);
// ����Ƿ����¼�
bool	udp_socketevent_chk(SOCKET __sock);

// ���ù���Դ��ַ��recvfrom������::recvfrom�ķ���
int		udp_recv(SOCKET __sock, void *__buf, int __len);
int		udp_recv(SOCKET __sock, void *__buf, int __len, struct sockaddr_in *__addr);

// �򵥵�sendto
int		udp_sendto(SOCKET __sock, const void *__buf, int __len, const struct sockaddr_in *__addr);

// Ϊ�˸����������ö�����
// ��λ��������15.8����ʾ15.8%�İ�Ҫ����
void	udp_set_sendto_lostratio(float fLost);
float	udp_get_sendto_lostratio();
void	udp_set_recv_lostratio(float fLost);
float	udp_get_recv_lostratio();

////////////////////////////////////////////////////////////////////
// UDP���򵥵�udp socket������������
////////////////////////////////////////////////////////////////////
class	UDP
{
protected:
	SOCKET	m_sock;
public:
	UDP(port_t nPort=0);
	UDP(const char *szIP, port_t nPort=0);
	~UDP();
	inline SOCKET	GetSock() const
	{
		return	m_sock;
	}
	inline int		SendTo(const char *pBuf, int nSize, const struct sockaddr_in *pAddr)
	{
		return	::sendto(m_sock, pBuf, nSize, 0, (struct sockaddr *)pAddr, sizeof(*pAddr));
	}
	inline int		RecvFrom(char *pBuf, int nSize, struct sockaddr_in *pAddr)
	{
		socklen_t	nLen = sizeof(*pAddr);
		return	::recvfrom(m_sock, pBuf, nSize, 0, (struct sockaddr *)pAddr, &nLen);
	}
	inline int		Select_RD(int nTimeOut)
	{
		return	cmn_select_rd(m_sock, nTimeOut);
	}
	inline int		Select_WR(int nTimeOut)
	{
		return	cmn_select_wr(m_sock, nTimeOut);
	}
};

// ����UDP��֪ͨ��������localhost��UDP��ģ���¼���������windows��Event�Ĺ��ܣ�
// ��ǰ���socketevent����
class	UDPEvent
{
protected:
	SOCKET	m_sock;
public:
	inline SOCKET	GetSocket() const
	{
		return	m_sock;
	}
	UDPEvent();
	~UDPEvent();
	int	Init();
	int	Release();
	int	AddEvent(const void *pData="e", size_t nSize=1);
	int	GetEvent(void *pData, size_t *pnSize);
	int	ClrAllEvent();
};

////////////////////////////////////////////////////////////////////
// ��־��¼��
////////////////////////////////////////////////////////////////////
// ����־��¼����Ϊ����ԭ���ĳ��򶼿����ã����ﻹ������
class	UDPLogger
{
private:
	SOCKET	m_sock;
public:
	UDPLogger();
	~UDPLogger();
	// szParam��ʽ
	// ip:port	���磺"127.0.0.1:1976"
	int	Init(const char *szParam);
	int	Release();
	int	WriteBin(const char *szMsg, int nLen);
	int	Write(const char *szMsg);
	int	WriteFmt(const char *szFmt, ...);
};

////////////////////////////////////////////////////////////////////
// ת����(����ģ�������ϵ���ʱ�Ͷ���)
////////////////////////////////////////////////////////////////////
// ��һ�������Լ��İ���������Դ��ַ���Ժ����Ŀ���ַ��Դ��ַ�䴫������
// �ϲ�ʹ�÷���
// ...
// select_rd(router->GetSocket(), 10);
// router->Tick();
// ...
class	UDPRouter
{
public:
	enum
	{
		MAX_PACKET_SIZE		= 1024,
		MAX_BUF_SIZE		= MAX_PACKET_SIZE * 2,
	};
	// ��ʼ�������ṹ
	struct	INFO_T
	{
		char	szDstIP[WHNET_MAXADDRSTRLEN];				// Ŀ���ַ
		port_t	nDstPort;									// Ŀ��˿�
		port_t	nLocalPort;									// ���ض˿�
		int		nMaxPacketNum;								// �����ۻ�����������
		int		nMaxPacketSize;								// �����������ߴ�
		int		nMinDelay, nMaxDelay;						// �ӳ�ʱ�䷶Χ(����)
		int		nLoss;										// ������(ǧ��֮m_nLoss)
	};
	// ���ݻ��嵥Ԫ�ṹ
	struct	PACKETUNIT_T
	{
		char	*szData;									// ָ������Ļ����ָ��
		int		nSize;										// ���ݴ�С
		unsigned int	nTimeToSend;						// �������ʱ�̾ͷ���
		int		nAddr;										// ��Ҫ����ĵ�ַ(��������enum��������)
	};
	enum
	{
		DSTADDR		= 0,
		SRCADDR		= 1,
	};
	enum
	{
		LOSSRANGE	= 1000,									// ˵��nLoss�ĵ�λ��ǧ��֮һ
	};
private:
	SOCKET	m_sock;											// socket
	struct sockaddr_in	m_addr[2];							// Ŀ���ַ����Դ��ַ
	bool	m_bVirgin;										// ��û�а���
	int		m_nMinDelay, m_nMaxDelay;						// �ӳ�ʱ�䷶Χ(����)
	int		m_nLoss;										// ������(ǧ��֮m_nLoss)
	n_whcmn::whunitallocatorFixed<PACKETUNIT_T>	m_Units;	// ���еȴ���ʱ���͵İ�
	int		*m_pSendList;									// ��DoSend��ʹ�õı���Ӧ�÷��͵İ�����ű�
	char	*m_tmpbuf;										// ������ʱ��ȡ�յ����ݵĻ��壬��recvfrom�õ�
	int		m_tmpsize;										// m_tmpbuf�������ɵ�������ݳ���
public:
	inline SOCKET	GetSocket() const
	{
		return	m_sock;
	}
	// ���ò���
	// ��ʱ����[nMin,nMax]����֮��
	inline void		SetDelay(int nMin, int nMax)
	{
		m_nMinDelay	= nMin;
		m_nMaxDelay	= nMax;
	}
	// �����ʱ
	inline void		GetDelay(int *pnMin, int *pnMax)
	{
		*pnMin	= m_nMinDelay;
		*pnMax	= m_nMaxDelay;
	}
	// �����ʡ�ǧ��֮nLoss��
	inline void		SetLoss(int nLoss)
	{
		m_nLoss	= nLoss;
	}
	// ��ö�����
	inline int		GetLoss()
	{
		return	m_nLoss;
	}
public:
	UDPRouter();
	~UDPRouter();
	int		Init(INFO_T *pInfo);
	int		Release();
	// һ���˶��������������룬ת�����Է�(�Ѿ���ʱ���߶�����)
	int		Tick();
private:
	// �ڲ�����
	// ����һ�����룬�ж�Ҫ������Ҫ��ʱ��
	int		DoOneRecv();
	// ����Ҫ��ʱ�İ���
	int		DoSend();
	// ������Ƿ�Ҫ����
	bool	ShouldItLost();
	// ����һ������ӳٺ���
	int		GenDelay();
};

////////////////////////////////////////////////////////////////////
// ���ڵ�ַ�ĵǼ��б�����������Ϸ�б�
// ע�⣬NOOPһ��Ҫ�����㹻ʹ���صĵ�ַӳ�䲻�ı�
////////////////////////////////////////////////////////////////////
class	UDPRegLister
{
public:
	// ����
	enum
	{
		CMD_NOOP					= 0,					// ��������
		CMD_REGIST					= 1,					// ����ע��
		CMD_UNREGIST				= 2,					// ����ע��
		CMD_REGIST_AGREE			= 11,					// ͬ��ע��
		CMD_REGIST_REFUSE			= 12,					// �ܾ�ע��
		CMD_UNREGIST_OK				= 20,					// ע�����
		CMD_DATA					= 80,					// ����֪ͨ�����麯������
	};
	// �õ��Ľṹ
	#pragma pack(1)
	struct	CMD_T
	{
		short	cmd;
		char	data[1];
	};
	#pragma pack()
	// ����Ƿ�����ע��
	enum
	{
		CHECK_REG_RST_ERR			= -1,
		CHECK_REG_RST_OK			= 0,
	};
	// ��ʼ������
	struct	INFO_T
	{
		SOCKET		sock;			// ����socket
		int			nMaxRegNum;		// ����ע�����
		int			nMaxRegData;	// ע�����ݵ���󳤶�
		int			nMaxUnRegData;	// ע�����ݵ���󳤶�
		int			nMaxWorkData;	// ������֪ͨ���ݵ���󳤶�
		int			nDropTimeOut;	// �����ô��ʱ��û�յ�NOOP������Ϊ������(����)
		INFO_T()
		: sock(INVALID_SOCKET)
		, nMaxRegNum(100), nMaxRegData(128), nMaxUnRegData(128), nMaxWorkData(128)
		, nDropTimeOut(60000)		// (Ĭ��60��)
		{ }
	};
	// ��Ԫ�ṹ
	struct	UNIT_T
	{
		enum
		{
			STATUS_DROPPED		= -1,
			STATUS_NOTHING		= 0,
			STATUS_WORKING		= 1,
		};
		int						nStatus;
		unsigned int			nCreateTime;
		unsigned int			nLastRecvTime;
		struct sockaddr_in		addr;
		n_whcmn::whvector<char>	regData;
		n_whcmn::whvector<char>	unregData;
		n_whcmn::whvector<char>	workData;		// �����е���Ϣ
		UNIT_T()
		{
			clear();
		}
		void	clear()
		{
			nStatus			= 0;
			nCreateTime		= 0;
			nLastRecvTime	= 0;
			// destroy���Խ�Լ�ڴ�
			regData.destroy();
			unregData.destroy();
			workData.destroy();
		}
	};
private:
	// ���ע�������Ƿ���ȷ
	virtual int	CheckRegist(char *szData, int nSize);
	// ��ɾ��һ����ԪǰӦ����������
	virtual int	BeforeDeleteUnit(int nID);
	// ��ÿ��NOOP��Ҫ������
	virtual int	NoopJob(int nID);
	// ����������
	virtual int	DataJob(int nID);
protected:
	INFO_T	m_info;
	n_whcmn::whunitallocatorFixed<UNIT_T>							m_Units;
	n_whcmn::whhash<struct sockaddr_in, int, n_whcmn::whcmnallocationobj, _whnet_addr_hashfunc>	m_mapAddr2ID;
	n_whcmn::whvector<int>											m_vectToRemove;
protected:
	inline int	RecvFrom(char * buf, int len, struct sockaddr_in *from, socklen_t *fromlen)
	{
		*fromlen	= sizeof(*from);
		return		::recvfrom(m_info.sock, buf, len, 0, (struct sockaddr *)from, fromlen);
	}
	inline int	SendTo(char * buf, int len, struct sockaddr_in *to)
	{
		return		::sendto(m_info.sock, buf, len, 0, (struct sockaddr *)to, sizeof(*to));
	}
	inline int	SendCmdTo(short cmd, struct sockaddr_in *to)
	{
		CMD_T	Cmd;
		Cmd.cmd	= cmd;
		return	SendTo((char *)&Cmd, sizeof(Cmd), to);
	}
	int		RemoveUnit(int nID);
public:
	inline SOCKET	GetSocket() const
	{
		return	m_info.sock;
	}
public:
	UDPRegLister();
	virtual ~UDPRegLister();
	int		Init(INFO_T *pInfo);
	int		Release();
	int		Tick();					// ������select�����Ϳ��Ե����������
};
////////////////////////////////////////////////////////////////////
// �������ĵǼ�������
////////////////////////////////////////////////////////////////////
class	UDPReger
{
public:
	enum
	{
		STATUS_DROPPED				= -2,					// ������
		STATUS_ERROR				= -1,					// ������
		STATUS_NOTHING				= 0,
		STATUS_REGING				= 1,
		STATUS_UNREGING				= 2,
		STATUS_WORKING				= 10,					//
		STATUS_WORKEND				= 20,					// UNREGҲ����ˣ����й����ɹ�����
	};
	struct	INFO_T
	{
		SOCKET						sock;					// ͨѶsocket (UDP����������Ӧ���Ѿ��ͶԷ�connect��)
		n_whcmn::WHRANGE_T<int>		rgNoopInterval;			// ����NOOP�ļ��
		int							nReqInterval;			// ����Reg��UnReg����ļ��
		int							nReqTimeOut;			// ����ʱ
		INFO_T()
		: sock(INVALID_SOCKET)
		, rgNoopInterval(18000, 22000)						// ƽ��20�루���漴��Ϊ���÷������ĸ��ظ�ƽ��һЩ��
		, nReqInterval(1000)
		, nReqTimeOut(5000)
		{ }
	};
protected:
	int		m_nStatus;
	unsigned int	m_nLastNoopTime;						// �ϴη���NOOP��ʱ��
	int		m_nNoopInterval;								// NOOP���εķ��ͼ��
	unsigned int	m_nStartReqTime;						// ����������������ʱ��
	unsigned int	m_nLastReqTime;							// �ϴη������������ʱ��
	INFO_T	m_info;
	n_whcmn::whvector<char>			m_RegData;
	n_whcmn::whvector<char>			m_UnRegData;
	n_whcmn::whvector<char>			m_WorkData;
protected:
	void	SetStatus(int nStatus);
	int		Tick_Reging();
	int		Tick_UnReging();
	int		Tick_Working();
public:
	inline SOCKET	GetSocket() const
	{
		return	m_info.sock;
	}
	inline int		GetStatus() const
	{
		return	m_nStatus;
	}
public:
	UDPReger();
	~UDPReger();
	int		Init(INFO_T *pInfo);
	int		Release();
	int		Reg(char *pData, int nSize);
	int		UnReg(char *pData, int nSize);
	int		SendData(char *pData, int nSize);
	int		Tick();
};

// ����udp��־
int		gUDPLogger_Dft_Init(const char *szParam);
int		gUDPLogger_Dft_Release();
int		gUDPLogger_Dft_WriteFmt(const char *szFmt, ...);
void	gUDPLogger_Set_PrintInScreenOnly(bool bSet);
void	gUDPLogger_Set_PrintInScreenToo(bool bSet);

}		// EOF namespace n_whnet

// ���ֻ��һ�����������ʡ�ô����ˣ����������ڳ���ʼ��ʱ����Init��ʼ�����g_UDPLogger
// 2004-06-29����Ҫ��glogger�����������������Ȳ�����
//#define	SYSLOGGER_INIT			n_whnet::gUDPLogger_Dft_Init
//#define	SYSLOGGER_RELEASE		n_whnet::gUDPLogger_Dft_Release
//#define	SYSLOGGER_WRITEFMT		n_whnet::gUDPLogger_Dft_WriteFmt
//#define	SYSLOGGER_SET_PRINTINSCREENONLY		n_whnet::gUDPLogger_Set_PrintInScreenOnly
//#define	SYSLOGGER_SET_PRINTINSCREENTOO		n_whnet::gUDPLogger_Set_PrintInScreenToo

//#define	SYSLOGGER_WRITEFMT		printf

#endif	// EOF __WHNETUDP_H__
