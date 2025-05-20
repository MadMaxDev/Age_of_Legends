// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File: whnetraw.h
// Creator: Wei Hua (κ��)
// Comment: �򵥵�RAW socket���繦��
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

// ����һ������ICMP��socket
SOCKET	raw_icmp_create_socket();

// ping��ؽӿ�
// ��дһ�㲻���icmp����
void	raw_icmp_ping_fill_cmn_data(char *__data, int __size);
// ��д�仯������
void	raw_icmp_ping_fill_var_data(char *__data, int __size, unsigned short __seqno);
// ����icmp����
enum
{
	ICMP_PING_PARSE_ERR_OK				= 0,
	ICMP_PING_PARSE_ERR_SIZE_TOOSMALL	= -1,			// ���ߴ�̫С
	ICMP_PING_PARSE_ERR_NOTECHO			= -2,			// �ⲻ��һ����Ӧ��
	ICMP_PING_PARSE_ERR_NOTMINE			= -3,			// �ⲻ���ҵĻ�Ӧ��
};
struct	ICMP_PING_PARSE_RST_T
{
	IPHEADER_T		*iphdr;									// ָ��ip��ͷ
	ICMPHEADER_T	*icmphdr;								// ָ��icmp��ͷ
	unsigned int	t;										// �յ��ð�����ʱ
};
int		raw_icmp_ping_parse_data(char *__data, int __size, ICMP_PING_PARSE_RST_T *__prst);
// ����У���(__size�Ƿ�Ϊż������ν)
unsigned short	raw_icmp_ping_chksum(unsigned short *__data, int __size);

}		// EOF namespace n_whnet

#endif	// EOF __WHNETRAW_H__
