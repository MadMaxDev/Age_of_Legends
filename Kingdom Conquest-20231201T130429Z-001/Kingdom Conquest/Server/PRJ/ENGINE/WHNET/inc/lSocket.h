// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet,lane
// File         : lSocket.h
// Creator      : 吕宝虹
// Comment      : 简单的socket封装，select功能
// CreationDate : 2006.07-28
// ChangeLOG    : 

#ifndef		LANE_LSOCKET_H
#define		LANE_LSOCKET_H

#include "whnet.h"

namespace n_whnet
{


namespace	lane
{

#ifdef	WIN32

#else

#endif

#ifdef		WIN32	// WIN32
	typedef		SOCKET		socket_t;
	//typedef		int			socklen_t;
#	define	memzero			ZeroMemory
#	define	getneterr()		WSAGetLastError()

#else				// linux
	typedef	int				socket_t;
#	define	memzero			bzero
#	define	getneterr()		errno
#	ifndef		INVALID_SOCKET
#		define		INVALID_SOCKET		-1
#	endif
#endif				// end WIN32

	/// 传输协议
enum	ETransProto
{
	IPv4 = AF_INET,
	IPv6 = AF_INET6,
	TCP	 = SOCK_STREAM,
	UDP	 = SOCK_DGRAM,
};

class Socket
{
public:

	Socket( ETransProto family = IPv4, ETransProto proto = TCP, const char * ip = NULL, unsigned short int port = 0 );

	Socket( socket_t sock );

	int	open(  ETransProto family = IPv4, ETransProto proto = TCP, const char * ip = NULL, unsigned short int port = 0 );

	bool setblock( bool block );

	virtual	~Socket();

	int	listen( int backlog );

	int	connect( const char * ip, unsigned short int port ,int timeout);

	void close();

	Socket * accept( sockaddr * addr = NULL, socklen_t * addrlen = NULL );

	int	send( const char * buf, size_t len );

	int	recv( char * buf, size_t len );

	int	sendto( const char * buf, size_t len,  const char * ip,  unsigned short int port );

	int	sendto( const char * buf, size_t len,  const sockaddr * addr, socklen_t addrlen );

	int	recvfrom( char * buf, size_t len,  char * ip = NULL, unsigned short int * port = NULL );

	int	recvfrom( char * buf, size_t len,  sockaddr * addr = NULL, socklen_t * addrlen = NULL  );

	socket_t getsock();

	bool isvalid()
	{
		return (m_sock != INVALID_SOCKET);
	}

private:
	socket_t			m_sock;
	sockaddr_in			m_addr;

private:
	Socket( const Socket & ){}
};
	
//////////////////////////////////////////////////////////////////////////


class	Select
{
public:
	enum select_opt 
	{
		read	= 0x01,		// 0001
		write	= 0x02,		// 0010
	};


	//////////////////////////////////////////////////////////////////////////

	Select()
	{
		clear();
	}

	//////////////////////////////////////////////////////////////////////////

	~Select()
	{
		clear();
	}

	//////////////////////////////////////////////////////////////////////////

	void	clear()
	{
		FD_ZERO( &m_readset ); 
		FD_ZERO( &m_writeset );
	}

	//////////////////////////////////////////////////////////////////////////

	void addsock( Socket & sock, size_t opt  )
	{
		if ( opt & read ) {
			FD_SET( sock.getsock(), & m_readset );
		}

		if ( opt & write ) {
			FD_SET( sock.getsock(), & m_writeset );
		}
	}

	//////////////////////////////////////////////////////////////////////////

	void removesock( Socket & sock, size_t opt )
	{
		if ( opt & read ) {
			FD_CLR( sock.getsock(), & m_readset );
		}

		if ( opt & write ) {
			FD_CLR( sock.getsock(), & m_writeset );
		}
	}

	//////////////////////////////////////////////////////////////////////////

	bool readable( Socket & sock )
	{
		if ( ! FD_ISSET( sock.getsock(), & m_readset ) ) {
			return false;
		}
		return	true;
	}

	//////////////////////////////////////////////////////////////////////////

	bool writeable( Socket & sock )
	{
		if ( ! FD_ISSET( sock.getsock(), & m_writeset ) ) {
			return false;
		}
		return	true;
	}

	//////////////////////////////////////////////////////////////////////////

	int test( size_t microSec = 1000 )
	{
		struct timeval	tv;
		tv.tv_sec  = 0;
		tv.tv_usec = microSec;

		return	::select( FD_SETSIZE, & m_readset, & m_writeset, NULL, & tv );
	}

	//////////////////////////////////////////////////////////////////////////

	void init()
	{
		FD_ZERO( &m_readset ); 
		FD_ZERO( &m_writeset );
	}

	//////////////////////////////////////////////////////////////////////////


private:
	fd_set m_readset;
	fd_set m_writeset;
};


};	//end lane

};	// end namespace n_whnet

#endif		// LANE_LSOCKET_H

