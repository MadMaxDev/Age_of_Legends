// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetepoll.h
// Creator      : Yue Zhongyue
// Comment      : 简单的epoll包结构
// CreationDate : 2011-11-02
// ChangeLog    :

#ifndef __WHNETEPOLL_PACKET_H__
#define __WHNETEPOLL_PACKET_H__

namespace n_whnet
{
//密钥的最大长度
enum
{
	max_key_len		= 64,
};
#pragma pack(1)
struct epoll_msg_head_t 
{
	int		iSize;
	int		iCntrID;
};
struct epoll_msg_base_t 
{
	char	iCmd;
};
struct epoll_msg_exchange_key_t : public epoll_msg_base_t 
{
	int		iCryptType;
	int		iKATimeout;
	size_t	iKeyLen;
	char	szKey[1];
};
struct epoll_msg_ka_t : public epoll_msg_base_t 
{
};
struct epoll_msg_close_t : public epoll_msg_base_t 
{
	int		iExtData;
};
struct epoll_msg_data_t : public epoll_msg_base_t 
{
	unsigned char	iCrc;
};
#pragma pack()
enum
{
	max_epoll_msg_data_size			= 32*1024,
	min_epoll_msg_data_size			= sizeof(epoll_msg_base_t),
	max_epoll_msg_size				= max_epoll_msg_data_size+sizeof(epoll_msg_head_t),
	min_epoll_msg_size				= min_epoll_msg_data_size+sizeof(epoll_msg_head_t),
};
enum
{
	epoll_msg_exchange_key			= 1,
	epoll_msg_ka					= 2,
	epoll_msg_close					= 3,
	epoll_msg_data					= 4,
};
}

#endif
