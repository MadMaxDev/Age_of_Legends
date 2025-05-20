// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnethttp.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵�httpЭ��ʵ��
// CreationDate : 2004-11-23
// ChangeLog    :

#ifndef	__WHNETHTTP_H__
#define	__WHNETHTTP_H__

#include "whnetcmn.h"
#include <WHCMN/inc/whstreambuffer.h>
#include <WHCMN/inc/whtime.h>

namespace n_whnet
{

enum
{
	// ���URL
	WHNET_MAXURLSTRLEN										= WHNET_MAXADDRSTRLEN,
	// ���HTTPͷ
	WHNET_MAXHTTPHEADERLEN									= 4096,
	// HTTPͷ��key/value�ĳ���
	WHNET_MAXHTTPKEYLEN										= 32,
	WHNET_MAXHTTPVALUELEN									= 128,
};
enum
{
	// SUCCESS
	HTTP_STATUS_CODE_SUCCESS_OK								= 200,
	HTTP_STATUS_CODE_SUCCESS_CREATED						= 201,
	HTTP_STATUS_CODE_SUCCESS_ACCEPTED						= 202,
	HTTP_STATUS_CODE_SUCCESS_NOCONTENT						= 203,
	// CLIENTERROR
	HTTP_STATUS_CODE_CLIENTERROR_BADREQUEST					= 400,
	HTTP_STATUS_CODE_CLIENTERROR_UNAUTHORIZED				= 401,
	HTTP_STATUS_CODE_CLIENTERROR_FORBIDDEN					= 403,
	HTTP_STATUS_CODE_CLIENTERROR_NOTFOUND					= 404,
	HTTP_STATUS_CODE_CLIENTERROR_METHODNOTALLOWED			= 405,
	HTTP_STATUS_CODE_CLIENTERROR_NOTACCEPTABLE				= 406,
	// SERVERERROR
	HTTP_STATUS_CODE_SERVERERROR_INTERNALERROR				= 500,
	HTTP_STATUS_CODE_SERVERERROR_NOTIMPLEMENTED				= 501,
	HTTP_STATUS_CODE_SERVERERROR_SERVICEUNAVAILABLE			= 503,
};

// ����һ��http������GET
class	httpgetandclose
{
public:
	struct	INFO_T
	{
		SOCKET		sock;									// �Ѿ������϶Է�http�˿ڵ�socket
															// (���sock==INVALID_SOCKET������Ҫ���ڲ���������pcszHost����һ�£�Ȼ���ڲ���дsock���������Զ��ر�)
		bool		bAutoCloseSocket;						// �Ƿ��ڽ���ʱ�Զ��ر�
		port_t		nHostPort;								// �����Ҫ�ڲ����ӣ������host�Ķ˿ں�
		int			nConnectTimeout;						// �����Ҫ�ڲ�����host����������ӳ�ʱ
		int			nRecvTimeout;							// �ȴ��������ݵĳ�ʱ
		int			nRecvBufSize;							// ���ջ�������󳤶�
		char		szHost[WHNET_MAXURLSTRLEN];				// �Է�������ַ(��������ʱҲ��Ҫ�������)
		char		szGetWhat[WHNET_MAXURLSTRLEN];			// GET�Ĳ���
		char		szAgent[128];							// User-Agent����
		INFO_T()
		: sock(INVALID_SOCKET)
		, bAutoCloseSocket(true)
		, nHostPort(80)
		, nConnectTimeout(5000)
		, nRecvTimeout(5000)
		, nRecvBufSize(10000)
		{
			szHost[0]		= 0;
			szGetWhat[0]	= 0;
			strcpy(szAgent, "Mozilla/4.0 (compatible; MSIE 5.00; Windows 98)");
		}
	};
	struct	HTTP_INFO_T
	{
		char	szVer[32];									// ��StatusLine�õ��İ汾
		char	szStatus[256];								// Status phrase
		int		nStatus;									// Status code
		char	szServer[WHNET_MAXHTTPVALUELEN];
		char	szTransfer_Encoding[WHNET_MAXHTTPVALUELEN];
		char	szContent_Type[WHNET_MAXHTTPVALUELEN];
		int		nContentTotalLength;						// ���ݲ��ֵĳ���
		int		nContentReadLength;							// ���ݲ����Ѿ������ĳ���
		HTTP_INFO_T()
		{
			memset(this, 0, sizeof(*this));
			strcpy(szVer, "NULL");
			strcpy(szStatus, "NULL");
			nContentTotalLength	= -1;
		}
	};
	enum
	{
		STATUS_ERROR_PROTOCOL	= -2,						// Э�����(������Э�飬����Э�鷵�ش���)
		STATUS_ERROR_SOCKET		= -1,						// ��������
		STATUS_NOTHING			= 0,						// ��Ϊ
		STATUS_WORKING			= 1,						// �ȴ�������
		STATUS_END				= 10,						// ȫ���������(һ����˵�Է��Ѿ��ر���socket)
	};
private:
	typedef	int	(httpgetandclose::*STEP_FUNC_T)();
private:
	INFO_T			m_cfginfo;								// ��ʼ���Ĳ���
	HTTP_INFO_T		m_httpinfo;								// �յ���http����Ϣ
	bool			m_bReleased;							// �Ƿ��Ѿ���Release���ˣ�ֻ�б�Release�˲��ܽ�����һ��http����
	bool			m_bHeaderDealOver;						// ͷ���Ѿ���������
	bool			m_bInnerRecvOver;						// �ڲ��Ľ����Ѿ�����
	int				m_nStatus;								// ״̬
	time_t			m_lastrecvtime;							// ���һ���յ����ݵ�ʱ��
	n_whcmn::whstreambuffer	m_bufRecv;						// ����Ļ���
	STEP_FUNC_T		m_pDealRecv;							// ��Tick�еĶԸո�Recv�����ݵĽ���
	STEP_FUNC_T		m_pDealRecv_Oneline;					// ����һ�еĶ���(��Ҫ�ڴ���Headerʱ)
	char			m_szHeaderLine[WHNET_MAXHTTPHEADERLEN];	// ������ʱ���ͷ�е�һ��
public:
	httpgetandclose();
	~httpgetandclose();
	int		Init(INFO_T *pInfo);							// ���sock==INVALID_SOCKET����Init�л��Զ�ִ��һ�����������Ӳ���
															// ͬʱ�ᷢ������
	int		Release();
	int		GetStatus();									// ��õ�ǰ״̬
	void	SetStatus(int nStatus);							// ���õ�ǰ״̬
	int		Select_RD(int nInterval);						// ��socket�Ƿ�������(����ϲ�����ʱ�Ķ���������Բ��������)
	int		Tick();											// һ����Select_RD֮��͵���һ��Tick
	// nSize��pData���������
	// ����:
	// >0	ʵ���յ����ֽ���
	// 0	û������
	int		Recv(void *pData, int nSize);
	// ��ȡhttp����Ϣ
	inline HTTP_INFO_T *	GetHTTPInfo()
	{
		return	&m_httpinfo;
	}
	inline bool		IsHeaderDealOver() const
	{
		return	m_bHeaderDealOver;
	}
	inline bool		IsInnerRecvOver() const
	{
		return	m_bInnerRecvOver;
	}
	inline bool		IsReleased() const
	{
		return	m_bReleased;
	}
	inline bool		IsRecvTimeOut() const
	{
		if( m_lastrecvtime>0 )
		{
			return	n_whcmn::wh_tickcount_diff(n_whcmn::wh_gettickcount(), m_lastrecvtime) >= m_cfginfo.nRecvTimeout;
		}
		return	false;
	}
private:
	int		DealRecv_Header();								// ����HTTPͷ
	int		DealRecv_Header_StatusLine();					// ����StatusLine����
	int		DealRecv_Header_Headers();						// ����ͷ��������(General/Response/Entity Headers)
	int		DealRecv_Header_ContentLength();				// ���ͷ��û��Content-Length������п��м�16���ƴ���ʾContent-Length
	int		DealRecv_Body();
	int		GetLineOut(char *szLine, int nSize);			// �ӽ��յ��Ļ����ж���һ�д�0��β(�����еĳ��ȣ����Ȳ��������һ��0)
															// û�������򷵻�0
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETHTTP_H__
