// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnethttp.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵�httpЭ��ʵ��
// CreationDate : 2004-11-23
// ChangeLog    :

#include "../inc/whnethttp.h"
#include "../inc/whnettcp.h"
#include <WHCMN/inc/whtime.h>
#include <WHCMN/inc/whstring.h>

using namespace n_whnet;
using namespace n_whcmn;

httpgetandclose::httpgetandclose()
: m_bReleased(true)
, m_bHeaderDealOver(false)
, m_bInnerRecvOver(false)
, m_nStatus(STATUS_NOTHING)
, m_lastrecvtime(0)
, m_pDealRecv(NULL)
, m_pDealRecv_Oneline(NULL)
{
}
httpgetandclose::~httpgetandclose()
{
	Release();
}
int		httpgetandclose::Init(httpgetandclose::INFO_T *pInfo)
{
	memcpy(&m_cfginfo, pInfo, sizeof(m_cfginfo));
	if( m_cfginfo.sock == INVALID_SOCKET )
	{
		m_cfginfo.sock	= tcp_create_connect_socket(m_cfginfo.szHost, m_cfginfo.nHostPort, 0, m_cfginfo.nConnectTimeout);
		if( m_cfginfo.sock == INVALID_SOCKET )
		{
			// �޷�����
			return	-1;
		}
		m_cfginfo.bAutoCloseSocket	= true;
	}

	// ��������
	char	szReq[WHNET_MAXHTTPHEADERLEN];
	sprintf(szReq
		, "GET %s HTTP/1.1\r\n"
		  "Host: %s\r\n"
		  "Accept: */*\r\n"
		  "User-Agent: %s\r\n"
		  "Pragma: no-cache\r\n"
		  "Cache-Control: no-cache\r\n"
		  "Connection: close\r\n"
		  "\r\n"
		, m_cfginfo.szGetWhat
		, m_cfginfo.szHost
		, m_cfginfo.szAgent
		);

	int	nLen	= strlen(szReq);
	if( nLen != ::send(m_cfginfo.sock, szReq, nLen, 0) )
	{
		// ����ʧ��
		return	-2;
	}

	if( m_bufRecv.Init(pInfo->nRecvBufSize, 0, pInfo->nRecvBufSize/2)<0 )
	{
		return	-3;
	}

	SetStatus(STATUS_WORKING);
	m_pDealRecv			= &httpgetandclose::DealRecv_Header;
	m_pDealRecv_Oneline	= &httpgetandclose::DealRecv_Header_StatusLine;
	m_lastrecvtime		= wh_time();
	m_bHeaderDealOver	= false;
	m_bInnerRecvOver	= false;

	m_bReleased			= false;

	return	0;
}
int		httpgetandclose::Release()
{
	if( m_bReleased )
	{
		return	0;
	}

	if( m_cfginfo.bAutoCloseSocket
	&&  m_cfginfo.sock != INVALID_SOCKET
	)
	{
		closesocket(m_cfginfo.sock);
		m_cfginfo.sock	= INVALID_SOCKET;
	}

	m_bufRecv.Release();

	SetStatus(STATUS_NOTHING);
	m_bReleased	= true;

	return	0;
}
int		httpgetandclose::GetStatus()
{
	if( m_bHeaderDealOver && m_bInnerRecvOver )
	{
		m_nStatus	= STATUS_END;
	}
	return	m_nStatus;
}
void	httpgetandclose::SetStatus(int nStatus)
{
	m_nStatus	= nStatus;
}
int		httpgetandclose::Select_RD(int nInterval)
{
	if( m_bInnerRecvOver )
	{
		return	0;
	}
	int	rst	= cmn_select_rd(m_cfginfo.sock);
	if( rst<0 )
	{
		SetStatus(STATUS_ERROR_SOCKET);
		return	-1;
	}
	if( rst==0 )
	{
		return	0;
	}
	return	rst;
}
int		httpgetandclose::Tick()
{
	if( m_nStatus != STATUS_WORKING )
	{
		// ���ڹ���״̬��
		return	-1;
	}

	if( !m_bInnerRecvOver
	&&  m_bufRecv.GetSizeLeft()>0
	)
	{
		// ����û�������Խ���
		if( (cmn_select_rd(m_cfginfo.sock, 0)) > 0 )
		{
			// ��¼���һ���յ����ݵ�ʱ��(�����ж��Ƿ��ڱ�������)
			m_lastrecvtime	= wh_gettickcount();
			// ���������ȡ����
			int	size = tcp_saferecv(m_cfginfo.sock, m_bufRecv.GetTail(), m_bufRecv.GetSizeLeft());

			if( size>0 )
			{
				m_bufRecv.InAlloc(size);
			}
			else if( size==0 )
			{
				m_bInnerRecvOver	= true;
			}
			else
			{
				// socket������߱��ر���
				SetStatus(STATUS_ERROR_SOCKET);
				return	-1;
			}
		}
	}

	// �����յ�������
	(this->*m_pDealRecv)();

	return	0;
}
int		httpgetandclose::DealRecv_Header()
{
	// ����һ��
	while( GetLineOut(m_szHeaderLine, sizeof(m_szHeaderLine))>0 )
	{
		// ɾ��ͷβ�Ŀ��ַ�
		wh_strtrim(m_szHeaderLine);
		// ����֮
		if( (this->*m_pDealRecv_Oneline)()<0 )
		{
			break;
		}
		// ���ͷ���Ѿ������꣬�����
		if( m_pDealRecv != &httpgetandclose::DealRecv_Header )
		{
			break;
		}
	}
	return	0;
}
int		httpgetandclose::DealRecv_Header_StatusLine()
{
	// �ֽ�Ϊ������
	// ���磺HTTP/1.1 200 OK
	int	rst	= wh_strsplit("sda", m_szHeaderLine, "", m_httpinfo.szVer, &m_httpinfo.nStatus, m_httpinfo.szStatus);
	if( rst<3
	||  m_httpinfo.nStatus != HTTP_STATUS_CODE_SUCCESS_OK )
	{
		SetStatus(STATUS_ERROR_PROTOCOL);
		return	-1;
	}
	// ��ʼ��һЩ����
	m_httpinfo.nContentTotalLength	= -1;
	m_httpinfo.nContentReadLength	= 0;
	// Ȼ��Ϳ��Խ�������������
	m_pDealRecv_Oneline	= &httpgetandclose::DealRecv_Header_Headers;
	return	0;
}
int		httpgetandclose::DealRecv_Header_Headers()
{
	// �жϸ����Ƿ��ǿ���
	if( m_szHeaderLine[0]==0 )
	{
		if( m_httpinfo.nContentTotalLength<0 )
		{
			// ͷ��û��content-length����Ҫ�ȴ����������
			m_pDealRecv_Oneline	= &httpgetandclose::DealRecv_Header_ContentLength;
			return	0;
		}
		else
		{
			m_pDealRecv			= &httpgetandclose::DealRecv_Body;
			m_bHeaderDealOver	= true;
			return	0;
		}
	}

	// �ֽ�Ϊkey:value
	char	szKey[WHNET_MAXHTTPKEYLEN], szValue[WHNET_MAXHTTPVALUELEN];
	if( wh_strsplit("sa", m_szHeaderLine, ":", szKey, szValue)<2 )
	{
		// ����ʶ���У�����
		return	0;
	}

	if( stricmp(szKey, "Server")==0 )
	{
		strcpy(m_httpinfo.szServer, szValue);
	}
	else if( stricmp(szKey, "Transfer-Encoding")==0 )
	{
		strcpy(m_httpinfo.szTransfer_Encoding, szValue);
	}
	else if( stricmp(szKey, "Content-Type")==0 )
	{
		strcpy(m_httpinfo.szContent_Type, szValue);
	}
	else if( stricmp(szKey, "Content-Length")==0 )
	{
		// �����10������
		m_httpinfo.nContentTotalLength	= atoi(szValue);
	}

	return	0;
}
int		httpgetandclose::DealRecv_Header_ContentLength()
{
	// �еķ���������������ֺ����һ�����У������������ʵ���Ѿ�������ȥ��
	sscanf(m_szHeaderLine, "%x", &m_httpinfo.nContentTotalLength);
	m_pDealRecv				= &httpgetandclose::DealRecv_Body;
	m_bHeaderDealOver		= true;
	return	0;
}
int		httpgetandclose::DealRecv_Body()
{
	// ������body���־�ֱ����������Ϳ�����
	return	0;
}
int		httpgetandclose::Recv(void *pData, int nSize)
{
	if( !m_bHeaderDealOver )
	{
		return	0;
	}

	assert(m_httpinfo.nContentTotalLength>=0);

	if( nSize>(int)m_bufRecv.GetSize() )
	{
		nSize	= m_bufRecv.GetSize();
	}
	if( nSize>m_httpinfo.nContentTotalLength-m_httpinfo.nContentReadLength )
	{
		nSize	= m_httpinfo.nContentTotalLength-m_httpinfo.nContentReadLength;
	}
	if( nSize==0 )
	{
		return	0;
	}
	memcpy(pData, m_bufRecv.GetBuf(), nSize);
	m_bufRecv.Out(nSize);

	// ��¼�Ѷ��ĳ���
	m_httpinfo.nContentReadLength	+= nSize;

	return	nSize;
}
int		httpgetandclose::GetLineOut(char *szLine, int nSize)
{
	int	nBufSize	= m_bufRecv.GetSize();
	if( nBufSize==0 )
	{
		return	0;
	}
	int		nIdx	= 0;
	char	*pBuf	= m_bufRecv.GetBuf();
	while( nIdx<nBufSize )
	{
		if( pBuf[nIdx]=='\n' )
		{
			if( nIdx+2>=nSize )
			{
				// �������ߴ粻��
				return		-1;
			}
			// ������Ŀ��
			nSize			= nIdx+1;
			memcpy(szLine, pBuf, nSize);
			szLine[nSize]	= 0;
			// �ͷŻ���
			m_bufRecv.Out(nSize);
			return	nSize;
		}
		nIdx	++;
	}

	return	0;
}
