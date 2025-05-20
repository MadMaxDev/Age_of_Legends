// copyright (c) pixelgame, http://www.pixelgame.com
// all rights reserved.
// namespace	:
// file			: protocol_type.h
// creator		: yanghongyu
// comment		:
// creationdate	: 2008-05-09
// changelog	:
#ifndef __DIA_PROTOCOL_TYPE_H__
#define __DIA_PROTOCOL_TYPE_H__

#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
enum
{
	DIA_NIO_HANDLE_KEEPALIVE				= 0x0000,
	DIA_NIO_HANDLE_TIMEOUT					= 0x0001,
	DIA_NIO_HANDLE_CLOSED					= 0x0002,

	DIA_NIO_HANDSHAKE_ACTIVE				= 0x0003,
	DIA_NIO_HANDSHAKE_PASSIVE				= 0x0004,

	DIA_NIO_HANDSHAKE_COMPLETED				= 0x0005,
};

//////////////////////////////////////////////////////////////////////////
struct DER_CFG
{
	dia_uint32_t keepalive_idle;
	dia_uint32_t keepalive_interval;

	dia_uint32_t handler_max;
	dia_uint32_t nio_buf_len;

	char tcp_listen_host[256];
	dia_uint16_t tcp_listen_port;

	char dll_name[256];
	bool dll_multi_thread;
	char dll_create_func[256];

	void *plugin_cfg;

	int ah_freq;
	int rename_freq;

	dia_uint32_t manual_keepalive_idle;
	dia_uint32_t manual_keepalive_interval;
};


//////////////////////////////////////////////////////////////////////////
struct DMMQ_HEAD
{
	dia_socket_t _id;
	dia_uint16_t _opcode;
	dia_uint32_t _len;
};
class dia_stream_nio;
struct STREAM_NIO_CLOSED
{
	dia_stream_nio *_peer;
};


//////////////////////////////////////////////////////////////////////////
#pragma pack()
#endif

