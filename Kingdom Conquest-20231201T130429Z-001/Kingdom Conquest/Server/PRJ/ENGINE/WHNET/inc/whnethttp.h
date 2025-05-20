// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnethttp.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的http协议实现
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
	// 最长的URL
	WHNET_MAXURLSTRLEN										= WHNET_MAXADDRSTRLEN,
	// 最长的HTTP头
	WHNET_MAXHTTPHEADERLEN									= 4096,
	// HTTP头中key/value的长度
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

// 进行一次http短连接GET
class	httpgetandclose
{
public:
	struct	INFO_T
	{
		SOCKET		sock;									// 已经连接上对方http端口的socket
															// (如果sock==INVALID_SOCKET，则需要在内部主动根据pcszHost连接一下，然后内部填写sock，并设置自动关闭)
		bool		bAutoCloseSocket;						// 是否在结束时自动关闭
		port_t		nHostPort;								// 如果需要内部连接，这个是host的端口号
		int			nConnectTimeout;						// 如果需要内部连接host，这个是连接超时
		int			nRecvTimeout;							// 等待接收数据的超时
		int			nRecvBufSize;							// 接收缓冲区最大长度
		char		szHost[WHNET_MAXURLSTRLEN];				// 对方主机地址(发送请求时也需要这个参数)
		char		szGetWhat[WHNET_MAXURLSTRLEN];			// GET的参数
		char		szAgent[128];							// User-Agent参数
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
		char	szVer[32];									// 从StatusLine得到的版本
		char	szStatus[256];								// Status phrase
		int		nStatus;									// Status code
		char	szServer[WHNET_MAXHTTPVALUELEN];
		char	szTransfer_Encoding[WHNET_MAXHTTPVALUELEN];
		char	szContent_Type[WHNET_MAXHTTPVALUELEN];
		int		nContentTotalLength;						// 内容部分的长度
		int		nContentReadLength;							// 内容部分已经读到的长度
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
		STATUS_ERROR_PROTOCOL	= -2,						// 协议出错(不符合协议，或者协议返回错误)
		STATUS_ERROR_SOCKET		= -1,						// 网络问题
		STATUS_NOTHING			= 0,						// 无为
		STATUS_WORKING			= 1,						// 等待返回中
		STATUS_END				= 10,						// 全部接收完毕(一般来说对方已经关闭了socket)
	};
private:
	typedef	int	(httpgetandclose::*STEP_FUNC_T)();
private:
	INFO_T			m_cfginfo;								// 初始化的参数
	HTTP_INFO_T		m_httpinfo;								// 收到的http的信息
	bool			m_bReleased;							// 是否已经被Release过了（只有被Release了才能接受下一个http请求）
	bool			m_bHeaderDealOver;						// 头部已经解析完了
	bool			m_bInnerRecvOver;						// 内部的接收已经结束
	int				m_nStatus;								// 状态
	time_t			m_lastrecvtime;							// 最后一次收到数据的时间
	n_whcmn::whstreambuffer	m_bufRecv;						// 输入的缓冲
	STEP_FUNC_T		m_pDealRecv;							// 在Tick中的对刚刚Recv到数据的解析
	STEP_FUNC_T		m_pDealRecv_Oneline;					// 处理一行的动作(主要在处理Header时)
	char			m_szHeaderLine[WHNET_MAXHTTPHEADERLEN];	// 用于临时存放头中的一行
public:
	httpgetandclose();
	~httpgetandclose();
	int		Init(INFO_T *pInfo);							// 如果sock==INVALID_SOCKET，则Init中会自动执行一个阻塞的连接操作
															// 同时会发送请求
	int		Release();
	int		GetStatus();									// 获得当前状态
	void	SetStatus(int nStatus);							// 设置当前状态
	int		Select_RD(int nInterval);						// 看socket是否有输入(如果上层有延时的东西，则可以不调用这个)
	int		Tick();											// 一般在Select_RD之后就调用一下Tick
	// nSize是pData的最大容量
	// 返回:
	// >0	实际收到的字节数
	// 0	没有数据
	int		Recv(void *pData, int nSize);
	// 获取http的信息
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
	int		DealRecv_Header();								// 处理HTTP头
	int		DealRecv_Header_StatusLine();					// 处理StatusLine部分
	int		DealRecv_Header_Headers();						// 处理头其他部分(General/Response/Entity Headers)
	int		DealRecv_Header_ContentLength();				// 如果头部没有Content-Length，则会有空行加16进制串表示Content-Length
	int		DealRecv_Body();
	int		GetLineOut(char *szLine, int nSize);			// 从接收到的缓冲中读出一行带0结尾(返回行的长度，长度不包括最后一个0)
															// 没有数据则返回0
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETHTTP_H__
