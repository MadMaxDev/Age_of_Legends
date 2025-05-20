// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whnet
// File: whnetraw.cpp
// Creator: Wei Hua (魏华)
// Comment: 简单的RAW socket网络功能
// CreationDate: 2003-06-04

#include "../inc/whnetraw.h"
#include <WHCMN/inc/whtime.h>
#include <assert.h>
#include <string.h>

using namespace n_whcmn;

namespace n_whnet
{

const unsigned char	ICMP_ECHO		= 8;
const unsigned char	ICMP_ECHOREPLY	= 0;
const unsigned char	ICMP_MIN		= 8;	// minimum 8 byte icmp packet (just header)

// 创建一个用于ICMP的socket
SOCKET	raw_icmp_create_socket()
{
	SOCKET	sock;

	// Create socket
	sock	= socket(AF_INET,			// Address family
		   SOCK_RAW,					// Socket type
		   IPPROTO_ICMP);				// Protocol

	return	sock;
}

void	raw_icmp_ping_fill_cmn_data(char *__data, int __size)
{
	ICMPHEADER_T	*icmp_hdr;
	char			*datapart;

	assert((size_t)__size >= sizeof(ICMPHEADER_T));

	icmp_hdr		= (ICMPHEADER_T*)__data;

	icmp_hdr->i_type	= ICMP_ECHO;
	icmp_hdr->i_code	= 0;
	icmp_hdr->i_id		= (unsigned short)cmn_getpid();
	icmp_hdr->i_cksum	= 0;
	icmp_hdr->i_seq		= 0;

	datapart			= __data + sizeof(ICMPHEADER_T);
	// Place some junk in the buffer.
	memset(datapart, 'W', __size - sizeof(ICMPHEADER_T));
}
void	raw_icmp_ping_fill_var_data(char *__data, int __size, unsigned short __seqno)
{
	ICMPHEADER_T	*icmp_hdr;
	icmp_hdr		= (ICMPHEADER_T*)__data;
	icmp_hdr->i_cksum	= 0;
	icmp_hdr->timestamp	= wh_gettickcount();
	icmp_hdr->i_seq		= __seqno;
	icmp_hdr->i_cksum	= raw_icmp_ping_chksum((unsigned short*)__data, __size);
}

int		raw_icmp_ping_parse_data(char *__data, int __size, ICMP_PING_PARSE_RST_T *__prst)
{
	IPHEADER_T		*iphdr;
	ICMPHEADER_T	*icmphdr;
	unsigned short	iphdrlen;

	iphdr		= (IPHEADER_T *)__data;
	iphdrlen	= iphdr->h_len * 4;		// number of 32-bit words *4 = bytes

	if( __size < iphdrlen + ICMP_MIN)
	{
		return	ICMP_PING_PARSE_ERR_SIZE_TOOSMALL;
	}
	icmphdr		= (ICMPHEADER_T*)(__data + iphdrlen);
	if( icmphdr->i_type != ICMP_ECHOREPLY )
	{
		return	ICMP_PING_PARSE_ERR_NOTECHO;
	}
	if( icmphdr->i_id != (unsigned short)cmn_getpid() )
	{
		return	ICMP_PING_PARSE_ERR_NOTMINE;
	}

	__prst->iphdr	= iphdr;
	__prst->icmphdr	= icmphdr;
	__prst->t		= wh_gettickcount() - icmphdr->timestamp;

	return	ICMP_PING_PARSE_ERR_OK;
}

unsigned short	raw_icmp_ping_chksum(unsigned short *__data, int __size)
{
	unsigned int	cksum = 0;

	while(__size >1)
	{
		cksum	+= *__data++;
		__size	-= sizeof(unsigned short);
	}

	if( __size )
	{
		cksum	+= *(unsigned char*)__data;
	}

	cksum		= (cksum >> 16) + (cksum & 0xffff);
	cksum		+= (cksum >>16);
	return (unsigned short)(~cksum);
}

}		// EOF namespace n_whnet

