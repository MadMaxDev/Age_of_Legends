// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: numeric_type.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2007-12-17
// changelog	:
#ifndef __DIA_NUMERIC_TYPE_H__
#define __DIA_NUMERIC_TYPE_H__

//////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#ifdef __GNUC__
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#else
#include <io.h>
#include <winsock2.h>
#include <Mstcpip.h>
#include <windows.h>
#endif


// 4 MySQL::CLIENT_MULTI_STATEMENTS
typedef unsigned long ulong;

// compiler numeric type
typedef unsigned long long dia_uint64_t;
typedef long long dia_int64_t;

typedef unsigned int dia_uint32_t;
typedef int dia_int32_t;

typedef unsigned short dia_uint16_t;
typedef short dia_int16_t;

typedef unsigned char dia_uint8_t;
typedef char dia_int8_t;

// function pointer
#ifdef __GNUC__
#define dia_gf_call
#define dia_mf_call
#else
#define dia_gf_call __stdcall
#define dia_mf_call __cdecl
#endif

// handle type
typedef int dia_handle_t;
typedef int dia_filehandle_t;

// socket
#ifdef __GNUC__
typedef int dia_socket_t;
#define dia_invalid_socket -1
#define dia_socket_wouldblock EWOULDBLOCK
#else
typedef SOCKET dia_socket_t;
#define dia_invalid_socket INVALID_SOCKET
#define dia_socket_wouldblock WSAEWOULDBLOCK
#endif

// tcp listen backlog
#define dia_tcp_listen_backlog 5

// msg spec
#define dia_msg_chunk_count 64
#define dia_msg_chunk_size 1024

// string len: host, user, passwd, etc.
#define dia_string_len 256

// 64 bit format
#ifdef __GNUC__
#define	INT64PRFX			"ll"
#else
#define	INT64PRFX			"I64"
#endif


//////////////////////////////////////////////////////////////////////////
#endif

