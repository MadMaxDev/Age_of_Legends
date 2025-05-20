// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : lHttpGet.cpp
// Creator      : 吕宝虹
// Comment      : 简单的http GET请求的封装,可以从资源的某个点传输(服务器必须支持断点续传)
// CreationDate : 2006.07.28
// ChangeLOG    : 2006.11.23 修正几处出现错误以后状态没有转换到nothing和没有关闭套接字的bug， 增加tick中对空闲状态的检测
//				  2007.04.17 把缓冲区的分配改到构造函数中，by钟文杰
//				  2007.04.20 把接收数据过程改为非阻塞模式，by钟文杰

#include "../inc/lHttpGet.h"



namespace n_whnet
{
	using	namespace	n_whnet::lane;
	//////////////////////////////////////////////////////////////////////////

httpget::httpget( const char * host, const char * ip, pfn_onget pcallbackfn, size_t callbacksize, unsigned short int port )	:
//m_sHost(host),
m_sIP(ip),
m_uPort(port),
m_sRequest(""),
m_uCurPos(0),
m_pCallbackFun(pcallbackfn),
m_uCallbackSize(callbacksize),
m_uTotalTransSize(0),
m_TcpSock( IPv4, TCP, NULL, 0 ),
m_pExp(NULL)
//m_sBufWrPtr(NULL),
//m_sBufRdPtr(NULL)

{
	m_sBuf = new char [m_uCallbackSize];
	memset(m_sBuf,0,m_uCallbackSize);
	if ( ! m_sBuf ) {
		m_stat	= memerr;
		return;
	}

	if ( host ) {
		m_sHost = host;
	}
	else  {
		m_sHost = ip;	
	}


	m_stat = nothing;
	m_uGetedSize	 = 0;
}

//////////////////////////////////////////////////////////////////////////

httpget::~httpget()
{
	release();
}

//////////////////////////////////////////////////////////////////////////

void httpget::release()
{
	m_stat =nothing;
	if ( m_sBuf ) {
		delete m_sBuf;
		m_sBuf = NULL;
	} 
	m_TcpSock.close();
	m_select.clear();
	m_pExp	= NULL;
}

//////////////////////////////////////////////////////////////////////////

int httpget::get( const char * resource, size_t startpos, void * exp, int timeout)
{
	m_stat = connecting;

	m_uCurPos = startpos;

	//m_sBufRdPtr = m_sBufWrPtr = m_sBuf;

	int	ret;
	
	if ( ! m_TcpSock.isvalid() ) {
		//release();
		if ( m_TcpSock.open( IPv4, TCP, NULL, 0 ) < 0 ) {
			return	sockerr;
		}
	}

	// 连接主机
	ret = m_TcpSock.connect( m_sIP.c_str(), m_uPort ,timeout);
	if ( ret == 0 ) {
		release();
		return	connerr;
	}

	//m_TcpSock.setblock( false );

	// 发送get请求
	char request[4096];
	sprintf( request,
		"GET %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Accept: */*\r\n"
		"User-Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0; .NET CLR 1.1.4322)\r\n"
		"Range: bytes=%d-\r\n"
		"Referer: %s\r\n"
		"Pragma: no-cache\r\n"
		"Cache-Control: no-cache\r\n"
		"Connection: close\r\n"
		"\r\n",
		resource, m_sHost.c_str(), startpos, resource);

	unsigned int requestlen = sizeof( request );

	ret = m_TcpSock.send( request, requestlen );

	if ( ret <= 0 ) {
		release();
		return sockerr;
	}

	m_stat = prochead;

	m_pfProc	= & httpget::prochttphead;

	m_pExp		= exp;

	m_uRecvLenTmp = 0;

	return	noerr;
}

//////////////////////////////////////////////////////////////////////////

int httpget::tick()
{
	if ( nothing == m_stat ) {
		return	noerr;
	}

	int	ret = 0;

	m_select.addsock( m_TcpSock, Select::read );

	if ( ( ret = m_select.test() ) > 0 ) {

		if ( m_select.readable(m_TcpSock) ) {

				int m_uRecvOnce = m_TcpSock.recv( m_sBuf + m_uRecvLenTmp, m_uCallbackSize - m_uRecvLenTmp);

				if( m_uRecvOnce > 0)
					m_uRecvLenTmp += m_uRecvOnce;

				if ( m_uRecvOnce < 0 ) {
					if ( getneterr() == EINTR ) {
						return	noerr;
					}

#ifdef WIN32
				if (getneterr() == WSAEWOULDBLOCK)   {
					return	noerr;
				}
#else
				if (getneterr() == EWOULDBLOCK) {
					return	noerr;
				}
#endif
				release();
				return	sockerr;
			}

			else if ( m_uRecvOnce == 0 ) {
				if ( getneterr() == EINTR ) {
					return	noerr;
				}
				
				(this->*m_pfProc)();

				release();

				if ( m_uTotalTransSize != m_uGetedSize ) {
					return	connerr;	
				}
				
				return complete;
			}

			else {
				if( m_uRecvLenTmp == (int)m_uCallbackSize )
					return	(this->*m_pfProc)();
				else
					return noerr;
			} 
		}	// end m_select.readable(m_TcpSock) 
	} // end  m_select.test() > 0

	else if( ret < 0 ) {
		//m_stat	= nothing;
		release();
		return sockerr;
	}

	return noerr;
}

//////////////////////////////////////////////////////////////////////////
// 处理获取的头....
int httpget::prochttphead()
{
	char * p = m_sBuf;
	char line[1024];

	int j = 0;
	for ( int i = 0; i < m_uRecvLenTmp; i ++ ) {

		// 处理一行
		if ( * p == '\r' && *(p+1) == '\n' ) {

			line[j] = 0;
			int ret = prochttpheadline( line, j );
			switch( ret )
			{
				// 需要继续分析下一行
			case 0:

				break;

				// http头分析完毕,察看http头的结果
			case 1:
				{						
					//m_sBufRdPtr = p + 2;
					//m_sBufWrPtr = p + 2;
					// 调用一次处理数据,清空所有数据,再进入下一次数据处理
					//procgetdata();
					m_stat = procdata;
					int ret = checkhttperr();
					if ( ret != noerr ) {
						return ret;
					}

					unsigned int	geted = m_uRecvLenTmp - ( (unsigned int)(p+2) - (unsigned int)(m_sBuf) );
					m_uCurPos += geted;
					m_uGetedSize += geted;
					m_pCallbackFun( p+2, geted, false, m_pExp );
					m_uRecvLenTmp = 0;
					m_pfProc = & httpget::procgetdata;

					return	ret;
				}
				break;
			case -2:
				break;
			}

			j = 0;
			p += 2;
		}

		// 不够一行
		else {
			line[j] = *p;	      
			p ++;
			j ++;
		}
	}
	//std::cout << m_sBufRdPtr;
	//m_sBufRdPtr += m_uRecvLenTmp;

	return	noerr;
}

//////////////////////////////////////////////////////////////////////////
// 处理http头每行的数据
int	httpget::prochttpheadline( const char * line, unsigned int charnum )
{
	const char * p = line;

	// 空行
	if ( line[0] == 0 ) {
		// 头分析结束,该处理数据了.
		return 1;
	}


	// 处理
	else if ( strncmp( "HTTP/", line, 5 ) == 0 ) {
		p += 8; 
		while ( *p ) {
			if ( *p == ' ' ) {
				p ++;
				break;
			}
			p ++;
		}

		char	httpret[4] = "";
		strncpy( httpret, p, 3 );
		m_uHttpRet = ::atoi(httpret);

		//std::cout << m_uHttpRet << std::endl;
	}

	// 获取要传输的数据的长度
	else if (  strncmp( "Content-Length", line, 14 ) == 0  ) {
		p += 14; 
		while ( *p ) {
			if ( *p == ' ' ) {
				p ++;
				break;
			}
			p ++;
		}

		char	needtranslen[16] = "";
		strcpy( needtranslen, p );
		m_uTotalTransSize = ::atoi(needtranslen);

		//std::cout << m_uTotalSize << std::endl;
	}


	return	noerr;
}

//////////////////////////////////////////////////////////////////////////

// 检查从http头中获取的数据,确定get请求是否正确有效
// 把检查到的错误返回给 prochttphead(), prochttphead()把错误犯回给tick()，tick返回给外界。
int httpget::checkhttperr()
{
	if ( m_uHttpRet >= 200 && m_uHttpRet < 300 ) {
		if ( m_uTotalTransSize < m_uCurPos ) {
			return	httpunknownerr;
		}
		return	noerr;
	}

	else if ( m_uHttpRet >= 400 && m_uHttpRet < 500 ) {
		release();
		return httprequesterr;
	}

	else if ( m_uHttpRet >= 500 && m_uHttpRet <600 ) {
		release();
		return httpservererr;
	}

	else {
		release();
		return	httpunknownerr;
	}

	return noerr;
}

//////////////////////////////////////////////////////////////////////////

int httpget::procgetdata()
{
	m_uCurPos += m_uRecvLenTmp;
	m_uGetedSize += m_uRecvLenTmp;
 	//assert ( m_uGetedSize <= m_uTotalTransSize );
	m_pCallbackFun( m_sBuf, m_uRecvLenTmp, (m_uGetedSize == m_uTotalTransSize)? true : false, m_pExp );
	m_uRecvLenTmp = 0;
	return	noerr;
}

//////////////////////////////////////////////////////////////////////////

int	httpget::stop()
{
	switch( m_stat )
	{
	case nothing:
		return	noerr;	

	case connecting:
		release();
		return	noerr;

	case prochead:
		release();
		return	noerr;

	case procdata:
		release();
		return	noerr;

	default:
		return	noerr;
	}
}

};	// end namespace n_whnet

