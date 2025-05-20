// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: ipc_socket.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_IPC_SOCKET_H__
#define __DIA_IPC_SOCKET_H__

#include "numeric_type.h"

// 
int get_local_addr(int socket, struct sockaddr *addr, int *len);
int get_peer_addr(int socket, struct sockaddr *addr, int *len);


class dia_ipc_socket
{
public:
	dia_ipc_socket();
	~dia_ipc_socket();

public:
	dia_socket_t get_socket();
	void set_socket(dia_socket_t socket);

	int open_stream_accept(const char *host, unsigned short port);
	int open_stream();

	int close_read();
	int close_write();
	int close_all();

	int set_socket_opt(int level, int option, void *optval, int optlen);
	int get_socket_opt(int level, int option, void *optval, int *optlen);

	int get_local_addr(struct sockaddr *addr, int *len);
	int get_peer_addr(struct sockaddr *addr, int *len);

	int get_last_error();

	int set_nonblock(bool nonblock);
	int set_addrreuse(bool reuse);
	int set_keepalive(int millisec_idle, int millisec_interval);

	int connect(const char *host, unsigned short port);
	int accept(dia_socket_t &socket);

	int send(const char *buf, int len);
	int recv(char *buf, int len);

protected:
	int socket_addr(const char *host, unsigned short port, struct sockaddr_in *addr);
	
private:
	dia_socket_t _socket;
};


#endif

