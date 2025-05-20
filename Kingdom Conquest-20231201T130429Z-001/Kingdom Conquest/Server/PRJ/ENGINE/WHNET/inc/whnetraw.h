// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File: whnetraw.h
// Creator: Wei Hua (魏华)
// Comment: 简单的RAW socket网络功能
// CreationDate: 2003-06-03

#ifndef	__WHNETRAW_H__
#define	__WHNETRAW_H__

#include "whnetcmn.h"

namespace n_whnet
{

enum
{
	MAX_ICMP_PACKET_SIZE	= 1024,		//Maximum icmp packet size
};

// IP header
struct	IPHEADER_T
{
	unsigned char	h_len:4;			// length of the header
	unsigned char	version:4;			// Version of IP
	unsigned char	tos;				// Type of service
	unsigned short	total_len;			// total length of the packet
	unsigned short	ident;				// unique identifier
	unsigned short	frag_and_flags;		// flags
	unsigned char	ttl;				// time to live
	unsigned char	proto;				// protocol (TCP, UDP etc)
	unsigned short	checksum;			// IP checksum
	unsigned int	sourceIP;
	unsigned int	destIP;
};
// ICMP header
struct	ICMPHEADER_T{
	unsigned char	i_type;
	unsigned char	i_code;				// type sub code
	unsigned short	i_cksum;
	unsigned short	i_id;
	unsigned short	i_seq;
	unsigned int	timestamp;			// This is not the std header, but we reserve space for time
};

// 创建一个用于ICMP的socket
SOCKET	raw_icmp_create_socket();

// ping相关接口
// 填写一般不变的icmp数据
void	raw_icmp_ping_fill_cmn_data(char *__data, int __size);
// 填写变化的数据
void	raw_icmp_ping_fill_var_data(char *__data, int __size, unsigned short __seqno);
// 解释icmp数据
enum
{
	ICMP_PING_PARSE_ERR_OK				= 0,
	ICMP_PING_PARSE_ERR_SIZE_TOOSMALL	= -1,			// 包尺寸太小
	ICMP_PING_PARSE_ERR_NOTECHO			= -2,			// 这不是一个回应包
	ICMP_PING_PARSE_ERR_NOTMINE			= -3,			// 这不是我的回应包
};
struct	ICMP_PING_PARSE_RST_T
{
	IPHEADER_T		*iphdr;									// 指向ip包头
	ICMPHEADER_T	*icmphdr;								// 指向icmp包头
	unsigned int	t;										// 收到该包的延时
};
int		raw_icmp_ping_parse_data(char *__data, int __size, ICMP_PING_PARSE_RST_T *__prst);
// 计算校验核(__size是否为偶数无所谓)
unsigned short	raw_icmp_ping_chksum(unsigned short *__data, int __size);

}		// EOF namespace n_whnet

#endif	// EOF __WHNETRAW_H__
