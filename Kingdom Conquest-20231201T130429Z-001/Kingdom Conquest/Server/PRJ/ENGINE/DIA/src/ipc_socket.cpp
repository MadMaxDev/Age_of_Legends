// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: ipc_socket.cpp
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetudpGLogger.h>

#include "../inc/ipc_socket.h"

int get_local_addr(int socket, struct sockaddr *addr, int *len)
{
#ifdef __GNUC__
	return ::getsockname(socket, addr, (socklen_t*)len);
#else
	return ::getsockname(socket, addr, len);
#endif
}
int get_peer_addr(int socket, struct sockaddr *addr, int *len)
{
#ifdef __GNUC__
	return ::getpeername(socket, addr, (socklen_t*)len);
#else
	return ::getpeername(socket, addr, len);
#endif
}

dia_ipc_socket::dia_ipc_socket()
: _socket(dia_invalid_socket)
{}
dia_ipc_socket::~dia_ipc_socket()
{
	if (get_socket() != dia_invalid_socket)
		close_all();
}


dia_socket_t dia_ipc_socket::get_socket()
{
	return _socket;
}
void dia_ipc_socket::set_socket(dia_socket_t socket)
{
	_socket = socket;
}


int dia_ipc_socket::open_stream_accept(const char *host, unsigned short port)
{
	if (open_stream() == -1)
		return -1;

	struct sockaddr_in addr;
	if (socket_addr(host, port, &addr) == -1)
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(100,DIA_IPC_SOCKET)"%s,gethostbyname,%s:%d", __FUNCTION__, host, port);
		return -2;
	}

	if (::bind(get_socket(), (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(100,DIA_IPC_SOCKET)"%s,bind,%s:%d", __FUNCTION__, host, port);
		return -3;
	}

	if (::listen(get_socket(), dia_tcp_listen_backlog) == -1)
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(100,DIA_IPC_SOCKET)"%s,listen,%d,%d", __FUNCTION__, get_socket(), dia_tcp_listen_backlog);
		return -4;
	}

	return 0;
}

int dia_ipc_socket::open_stream()
{
	if (get_socket() != dia_invalid_socket)
		return -1;

	set_socket(::socket(AF_INET, SOCK_STREAM, 0));
	if (get_socket() == dia_invalid_socket)
		return -1;

	if (set_nonblock(true) == -1)
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(100,DIA_IPC_SOCKET)"%s,set_nonblock", __FUNCTION__);
		return -1;
	}

	if (set_addrreuse(true) == -1)
	{
		GLOGGER2_WRITEFMT(n_whnet::GLOGGER_ID_ERROR, GLGR_STD_HDR(100,DIA_IPC_SOCKET)"%s,set_addrreuse", __FUNCTION__);
		return -1;
	}
	
	return 0;
}


int dia_ipc_socket::close_read()
{
	return ::shutdown(get_socket(), 0);
}
int dia_ipc_socket::close_write()
{
	return ::shutdown(get_socket(), 1);
}
int dia_ipc_socket::close_all()
{
	if (get_socket() == dia_invalid_socket)
		return -1;
#ifdef __GNUC__
	::close(get_socket());
#else
	::closesocket(get_socket());
#endif
	set_socket(dia_invalid_socket);
	return 0;
}

int dia_ipc_socket::set_socket_opt(int level, int option, void *optval, int optlen)
{
	return ::setsockopt(get_socket(), level, option, (char*)optval, optlen);
}
int dia_ipc_socket::get_socket_opt(int level, int option, void *optval, int *optlen)
{
#ifdef __GNUC__
	return ::getsockopt(get_socket(), level, option, (char*)optval, (socklen_t*)optlen);
#else
	return ::getsockopt(get_socket(), level, option, (char*)optval, optlen);
#endif
}

int dia_ipc_socket::get_local_addr(struct sockaddr *addr, int *len)
{
	return ::get_local_addr(get_socket(), addr, len);
}
int dia_ipc_socket::get_peer_addr(struct sockaddr *addr, int *len)
{
	return ::get_peer_addr(get_socket(), addr, len);
}

int dia_ipc_socket::get_last_error()
{
#ifdef __GNUC__
	return errno;
#else
	return WSAGetLastError();
#endif
}

int dia_ipc_socket::set_nonblock(bool nonblock)
{
#ifdef __GNUC__
	int flag = ::fcntl(get_socket(), F_GETFL, 0);
	if (flag == -1)
		return -1;

	if (nonblock)
		flag |= O_NONBLOCK;
	else
		flag &= ~O_NONBLOCK;
	return ::fcntl(get_socket(), F_SETFL, flag);
#else
	u_long val;
	if (nonblock)
		val = 1;
	else
		val = 0;
	return ::ioctlsocket(get_socket(), FIONBIO, &val);
#endif
}

int dia_ipc_socket::set_addrreuse(bool reuse)
{
	int opt = reuse? 1: 0;
	return set_socket_opt(SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
}

int dia_ipc_socket::set_keepalive(int millisec_idle, int millisec_interval)
{
#ifdef __GNUC__
	int keepalive = 1;
	int keepidle = millisec_idle / 1000;
	int keepinterval = millisec_interval / 1000;
	int keepcount = 1;

	if (set_socket_opt(SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(keepalive)) == -1)
		return -1;
	if (set_socket_opt(SOL_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) == -1)
		return -2;
	if (set_socket_opt(SOL_TCP, TCP_KEEPINTVL, &keepinterval, sizeof(keepinterval)) == -1)
		return -3;
	if (set_socket_opt(SOL_TCP, TCP_KEEPCNT, &keepcount, sizeof(keepcount)) == -1)
		return -4;
	return 0;
#else
	tcp_keepalive inKeepAlive = {0};
	tcp_keepalive outKeepAlive = {0};
	unsigned long ulBytesReturn = 0; 

	inKeepAlive.onoff = 1; 
	inKeepAlive.keepaliveinterval = millisec_idle;
	inKeepAlive.keepalivetime = millisec_interval;

	if (::WSAIoctl(get_socket(), SIO_KEEPALIVE_VALS, (LPVOID)&inKeepAlive, sizeof(inKeepAlive), (LPVOID)&outKeepAlive, sizeof(outKeepAlive), &ulBytesReturn, NULL, NULL)
		== SOCKET_ERROR) 
		return -1;
	else
		return 0;
#endif
}


int dia_ipc_socket::socket_addr(const char *host, unsigned short port, struct sockaddr_in *addr)
{
	if ((host == NULL) || !strcmp(host, "0") || (host[0] == 0))
	{
		addr->sin_addr.s_addr = INADDR_ANY;
	}
	else
	{
		hostent *hostinfo;
		if ((hostinfo = ::gethostbyname(host)) == NULL)
			return -1;

		addr->sin_addr = *((in_addr *)hostinfo->h_addr);;
	}

	if (port != 0)
		addr->sin_port = htons(port);
	else
		addr->sin_port = 0;

	addr->sin_family = AF_INET;
	return 0;
}


int dia_ipc_socket::connect(const char *host, unsigned short port)
{
	struct sockaddr_in addr;
	if (socket_addr(host, port, &addr) == -1)
		return -1;
	::connect(get_socket(), (struct sockaddr*)&addr, sizeof(addr));
	return 0;
}

int dia_ipc_socket::accept(dia_socket_t &socket)
{
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
#ifdef __GNUC__
	socket = ::accept(get_socket(), (struct sockaddr*)&addr, (socklen_t*)&addr_len);
#else
	socket = ::accept(get_socket(), (struct sockaddr*)&addr, &addr_len);
#endif
	if (socket == dia_invalid_socket)
		return -1;
	return 0;
}


int dia_ipc_socket::send(const char *buf, int len)
{
	return ::send(get_socket(), buf, len, 0);
}
int dia_ipc_socket::recv(char *buf, int len)
{
	return ::recv(get_socket(), buf, len, 0);
}

