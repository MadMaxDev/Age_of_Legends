// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet,lane
// File         : lSocket.cpp
// Creator      : 吕宝虹
// Comment      : 简单的socket封装，select功能
// CreationDate : 2006.07-28
// ChangeLOG    : 2007.04.03 by魏华。把所有hton和ntoh前面的::都去掉了，否则在release版下面编译出错 expected id-expression before '(' token。原因可能是因为两层名字空间嵌套造成的。以后再研究。
//				  2007.04.20 把接收数据过程改为非阻塞模式，by钟文杰

#include "../inc/lSocket.h"

namespace n_whnet
{

namespace	lane
{




////////////////////////////////////////////////////////////////////////////
static	size_t	nInitCount = 0;

static	bool	init()
{
#ifdef WIN32
	if ( nInitCount == 0 ) {
		WSADATA info;
		if ( ::WSAStartup(MAKEWORD(1,1), &info) ) {
			return false;
		}
	}
#endif		

	nInitCount ++;
	return	true;
}

//////////////////////////////////////////////////////////////////////////

static	void	release()
{
	nInitCount --;
#ifdef WIN32
	if ( ! nInitCount ) {
		::WSACleanup(); 
	}
#endif		

}

//////////////////////////////////////////////////////////////////////////

Socket::Socket( ETransProto family, ETransProto proto, const char * ip, unsigned short int port )
{
	if ( ! init() ) {
		assert(0);
	}

	m_sock = ::socket( family, proto, 0 );
	if ( m_sock == INVALID_SOCKET ) {
		return;
	}
	

	if ( ! ip ) {
		m_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	}
	else {
		m_addr.sin_addr.s_addr	= ::inet_addr(ip);
	}

	m_addr.sin_family		= family ;
	m_addr.sin_port			= htons(port);

	int ret = ::bind( m_sock, (sockaddr *)&m_addr, sizeof(sockaddr_in) ); 
	if ( ret != 0 ) {
	//	assert(0);
		return ;
	}
}

//////////////////////////////////////////////////////////////////////////

int	Socket::open(  ETransProto family, ETransProto proto, const char * ip, unsigned short int port )
{
	m_sock = ::socket( family, proto, 0 );
	if ( m_sock == INVALID_SOCKET ) {
		return	-1;
	}
	

	if ( ! ip ) {
		m_addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	}
	else {
		m_addr.sin_addr.s_addr	= ::inet_addr(ip);
	}

	m_addr.sin_family		= family ;
	m_addr.sin_port			= htons(port);

	int ret = ::bind( m_sock, (sockaddr *)&m_addr, sizeof(sockaddr_in) ); 
	if ( ret != 0 ) {
	//	assert(0);
		return -1;
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////

Socket::Socket( socket_t sock )
{
	if ( ! init() ) {
		assert(0);
	}

	m_sock = sock;
}

//////////////////////////////////////////////////////////////////////////

Socket::~Socket()
{
#ifdef	WIN32	
	::closesocket( m_sock );
#else	
	::close(m_sock);
#endif
	lane::release();
}

//////////////////////////////////////////////////////////////////////////

bool Socket::setblock( bool block )
{

#ifdef	WIN32

u_long	opt = block ? 0 : 1;

	if ( ::ioctlsocket( m_sock, FIONBIO, &opt) != 0 ) {
		return false;
	}

#else

	if ( block ) {
		int val = ::fcntl( m_sock, F_GETFL, 0 );
		if ( val < 0 ) {
			return false;
		}
		else {
			if ( ::fcntl( m_sock, F_SETFL, val & ~O_NONBLOCK ) != 0 ) {
				return	false;
			}
		}
	}
	else {
		if ( ::fcntl( m_sock, F_SETFL, O_NONBLOCK ) != 0 ) {
			return false;
		}
	}

#endif

	return true;
}

//////////////////////////////////////////////////////////////////////////

int	Socket::listen( int backlog )
{
	return	::listen( m_sock, backlog );
}

//////////////////////////////////////////////////////////////////////////

int	Socket::connect( const char * ip, unsigned short int port ,int timeout)
{
	if ( ! ip ) {
		return	-1024;
	}
	sockaddr_in		dstaddr;
	memzero( &dstaddr, sizeof(dstaddr) );
	dstaddr.sin_addr.s_addr	= ::inet_addr(ip);
	dstaddr.sin_family			= IPv4;
	dstaddr.sin_port			= htons(port);

	bool	bErr;
	int		rst;
	bErr	= false;

	cmn_setsock_nonblock(m_sock, true);		// {	SOCKET begin noblock
	// 为了cmn_is_nonblock_error()能够获得正确的错误号
	cmn_clear_error();
	// connect to server
	rst		= ::connect(m_sock, (const struct sockaddr *)&dstaddr, sizeof(dstaddr));
	// rst must be SOCKET_ERROR
	if( rst == SOCKET_ERROR )
	{
		if( cmn_is_nonblock_error() )
		{
			// 等待超时
			rst	= cmn_select_wr(m_sock, timeout);
			if( rst<=0
			||  cmn_getsockerror(m_sock)!=0 )
			{
				// 超时或者socket错误
				bErr	= true;
			}
		}
		else
		{
			// 错误
			bErr		= true;
		}
	}
	cmn_setsock_nonblock(m_sock, false);		// }	SOCKET block again

	if( bErr )
	{
		// 出错了，如果已经生成了socket则要关闭之
		close();
		m_sock	= INVALID_SOCKET;
	}
	return	rst;
	/*if ( ! ip ) {
		return	-1024;
	}
	sockaddr_in		addr;
	memzero( &addr, sizeof(addr) );
	addr.sin_addr.s_addr	= ::inet_addr(ip);
	addr.sin_family			= IPv4;
	addr.sin_port			= htons(port);

	return	::connect( m_sock, (sockaddr *)&addr, sizeof(addr) );*/
}

//////////////////////////////////////////////////////////////////////////

void Socket::close()
{
	if ( m_sock != INVALID_SOCKET ) {

#ifdef	WIN32	
		::closesocket( m_sock );
#else	
		::close(m_sock);
#endif	

		m_sock = INVALID_SOCKET;
	}
}

//////////////////////////////////////////////////////////////////////////

Socket * Socket::accept( sockaddr * addr, socklen_t * addrlen )
{
	socket_t	sock = ::accept( m_sock, addr, addrlen );

	if ( sock == INVALID_SOCKET ) {
		return	NULL;
	}

	return	new		Socket( sock );
}

//////////////////////////////////////////////////////////////////////////

int	Socket::sendto( const char * buf, size_t len,  const char * ip, unsigned short int port )
{
	sockaddr_in	addr;
	memzero( &addr, sizeof(addr) );
	addr.sin_family = IPv4;
	addr.sin_addr.s_addr = ::inet_addr(ip);
	addr.sin_port		 = htons(port);

	return	::sendto( m_sock, buf, len, 0, (sockaddr *)&addr, sizeof(sockaddr_in) );
}

//////////////////////////////////////////////////////////////////////////

int	Socket::sendto( const char * buf, size_t len, const sockaddr * addr, socklen_t addrlen )
{
	return	::sendto( m_sock, buf, len, 0, addr, addrlen );
}

//////////////////////////////////////////////////////////////////////////

int	Socket::recvfrom( char * buf, size_t len,  char * ip, unsigned short int * port  )
{
	if ( ip == NULL ) {
		return ::recvfrom( m_sock, buf, len, 0, NULL, NULL );
	}

	sockaddr	addr;
	socklen_t	addrlen = sizeof(addr);

	memzero( &addr, sizeof(addr) );
	
	int	ret = ::recvfrom( m_sock, buf, len, 0, &addr, & addrlen );
	if ( ret <= 0 ) {
		ret = getneterr();
		return	ret;
	}
	else {
		char * rip = inet_ntoa( ((sockaddr_in *)&addr)->sin_addr );
		strcpy( ip, rip );
		* port = ntohs(  ((sockaddr_in *)&addr)->sin_port ) ;
	}
	return	ret;
}

//////////////////////////////////////////////////////////////////////////

int	Socket::recvfrom( char * buf, size_t len,  sockaddr * addr, socklen_t * addrlen )
{
	return ::recvfrom( m_sock, buf, len, 0, addr, addrlen );
}

//////////////////////////////////////////////////////////////////////////

int	Socket::send( const char * buf, size_t len )
{
	int left = len,	ret = 0;
	const	char * p = buf;

	while ( left > 0 )	{

		if ( (ret = ::send( m_sock, p, left, 0)) < 0 ) {

			if ( getneterr() == EINTR ) {
				ret = 0;
			}
			else {
				return	-1;
			}
		}

		left = left - ret;
		p += ret;

	}
	return	len - left;
}

//////////////////////////////////////////////////////////////////////////

int	Socket::recv( char * buf, size_t len )
{
	int left = len,	ret = 0;
	char * p = buf;

	if ( (ret = ::recv( m_sock, p, left, 0)) < 0 ) {
		if ( getneterr() == EINTR ) {
			ret = 0;
		}

		else {
			return	-1;
		}
	}

	return	ret;
}

//////////////////////////////////////////////////////////////////////////

socket_t Socket::getsock()
{
	return	m_sock;
}

//////////////////////////////////////////////////////////////////////////

};// end lane

};	// end namespace n_whnet

