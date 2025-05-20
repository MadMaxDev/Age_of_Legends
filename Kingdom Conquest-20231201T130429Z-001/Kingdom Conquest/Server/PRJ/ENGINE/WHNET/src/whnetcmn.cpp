// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcmn.cpp
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵����繦��
//              : getsockname����ͨ��socket��ȡ��ַ(ֻ�а󶨳ɹ�����ܵõ�)�����ϣ����öԷ���ַ����Ҫʹ��getpeername
// CreationDate : 2003-05-14
// ChangeLOG    : 2004-09-03 �޸���cmn_get_S_addr�Ĺ��ܣ�������hash���������Ѿ�ȡ���ĵ�ַ���һЩ�õ�
//              : 2005-08-15 ��cmn_init�������˳�ʼ���������
//              : 2005-09-26 ȡ����cmn_get_S_addr�ж�l_myHash���޸�(ûʲô��Ҫ)
//              : 2009-07-01 ������cmn_get_AllMy_S_addr

#include "../inc/whnetcmn.h"
#include "../inc/whnetudpGLogger.h"
#include <WHCMN/inc/whstring.h>
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/whlock.h>
#include <WHCMN/inc/whdbg.h>
#include <WHCMN/inc/whdll.h>
#include "WHNET/inc/whnetepoll.h"
#include <stdarg.h>
#include <stdio.h>

#ifdef	__GNUC__
#include <errno.h>
#include <WHCMN/inc/whsignal.h>
#endif

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// ����������
////////////////////////////////////////////////////////////////////
namespace n_whnet
{

////////////////////////////////////////////////////////////////////
#if defined( WIN32 )				// Windows�µĴ���		{
////////////////////////////////////////////////////////////////////
// �����������������ε���cmn_init
static int	g_n_whnet_initcount = 0;
bool	cmn_init()
{
	if( g_n_whnet_initcount>0 )
	{
		// �Ѿ���ʼ������
		g_n_whnet_initcount	++;
		return	true;
	}

	// ��Ϊ�ҵó�������϶���������أ�����tick��ʼ���ͷ���������
	wh_gettickcount_calibrate();
	//wh_setstarttickref_tomakeoverflow(0);

	// ��Ϊ��һ����Ҫ������ӣ����������
	srand(wh_time());

	WORD	wVersionRequested = MAKEWORD(2, 0);	// �����Ը�����Ҫ�޸�����汾
	WSADATA	wsaData;
	int rst;

	//
	// Initialize WinSock and check version
	//
	rst	= WSAStartup(wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested)
	{
		return	false;
	}

	g_n_whnet_initcount	= 1;
	return	true;
}
bool	cmn_release()
{
	if( g_n_whnet_initcount==0 )
	{
		// �Ѿ��ͷŹ���
		return	false;
	}
	g_n_whnet_initcount	--;
	if( g_n_whnet_initcount>0 )
	{
		// ����������������˶��ͷ��ˡ�
		return	true;
	}
	if(WSACleanup())
		return	false;
	else
		return	true;
}
#endif								// EOF Windows�µĴ���	}
////////////////////////////////////////////////////////////////////
#if defined( __GNUC__ )				// Linux�µĴ���		{
////////////////////////////////////////////////////////////////////
// ���������������SIGPIPE
static void	deal_sigpipe_func(int sig)
{
	GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, "deal_sigpipe_func,,got SIGPIPE!");
}
bool	cmn_init()
{
	// ��Ϊ�ҵó�������϶���������أ�����tick��ʼ���ͷ���������
	// ����linux����Ҫ��ʼ��
	wh_gettickcount_calibrate();
	wh_setstarttickref_tomakeoverflow(0);

	// ��Ϊ��һ����Ҫ������ӣ����������
	srand(wh_time());

	// ��ȻҲ��������ΪSIG_IGN�����������Ͳ�˵�����Ƿ��й�SIGPIPE��
	signal(SIGPIPE, deal_sigpipe_func);
	printf("SIGPIPE will be caught.\r\n");

	return	true;
}
bool	cmn_release()
{
	return	true;
}
#endif								// Linux�µĴ���		}

struct	WHNET_STATIC_INFO_T
{
	whhash<whstr4hash, unsigned int>	l_myHash;
	void								*pUDPLoggerInfo;
	bool								g_bReuseAddr;
	void*								pEpollData;				// epoll�Ĺ�������
	WHNET_STATIC_INFO_T()
	: pUDPLoggerInfo(NULL)
	, g_bReuseAddr(false)
	, pEpollData(NULL)
	{
	}
};
static	WHNET_STATIC_INFO_T				l_si;
static	whlock							l_myLock;
#define	L_MYHASH						l_si.l_myHash
#define	L_MYLOCK						l_myLock
#define	G_BREUSEADDR					l_si.g_bReuseAddr

int		cmn_AddETCHostsLine(const char *cszLine)
{
	// �ֽ��ִ�
	char			szHost[128];
	char			szAddr[WHNET_MAXADDRSTRLEN];
	if( wh_strsplit("ss", cszLine, "", szHost, szAddr)!=2 )
	{
		// ��ʽ����
		return	-1;
	}
	// ��õ�ַ
	unsigned int	nAddr	= cmn_get_S_addr(szAddr);
	whlockmutex		mtx(&L_MYLOCK);
	if( !L_MYHASH.put(szHost, nAddr) )
	{
		// ���������ԭ���Ѿ�����
		return	-2;
	}
	return		0;
}
int		cmn_DelETCHost(const char *cszKey)
{
	whlockmutex		mtx(&L_MYLOCK);
	if( !cszKey )
	{
		// �������
		L_MYHASH.clear();
		return	0;
	}
	// ɾ��ָ����
	L_MYHASH.erase(cszKey);
	return		0;
}
unsigned int	cmn_get_AllMy_S_addr()
{
	unsigned int	rst	= 0;
	struct hostent *lpHostEntry;
	lpHostEntry = gethostbyname("");
	if (lpHostEntry != NULL)
	{
		int	i	= 0;
		while( lpHostEntry->h_addr_list[i] )
		{
			rst ^= (*((struct in_addr *)lpHostEntry->h_addr_list[i])).s_addr;
			i	++;
		}
	}
	return	rst;
}
unsigned int	cmn_get_S_addr(const char *__name)
{
	whlockmutex		mtx(&L_MYLOCK);

	unsigned int	rst;
	rst = inet_addr(__name);
	if(rst == INADDR_NONE)
	{
		if( !L_MYHASH.get(__name, rst) )
		{
			struct hostent *lpHostEntry;
			lpHostEntry = gethostbyname(__name);
			if (lpHostEntry == NULL)
			{
				rst = 0;
			}
			else
			{
				rst = (*((struct in_addr *)*lpHostEntry->h_addr_list)).s_addr;
				// L_MYHASH.put(__name, rst);
			}
		}
	}
	return	rst;
}
const char *	cmn_get_IP(unsigned int __addr, char *szIP)
{
	static char		szTmpIP[WHNET_MAXIPSTRLEN];
	if( szIP == NULL )
	{
		szIP	= szTmpIP;
	}
	struct in_addr	iadr;
	iadr.s_addr	= __addr;
	strcpy( szIP, inet_ntoa(iadr) );
	return	szIP;
}

int		cmn_get_host_addrs(const char *__name, unsigned int *__addrs, int *__num)
{
	int	rst;
	
	struct hostent *lpHostEntry;
	lpHostEntry = gethostbyname(__name);
	rst	= 0;
	if(lpHostEntry )
	{
	#ifdef	WIN32
		char FAR * addr;
	#endif
	#ifdef	__GNUC__
		char * addr;
	#endif
		while( (addr = lpHostEntry->h_addr_list[rst]) != NULL )
		{
			if( __num && __addrs && rst<*__num )
			{
				__addrs[rst]	= ((struct in_addr *)addr)->s_addr;
			}
			rst	++;
		}
		if( __num && rst<*__num )
		{
			*__num	= rst;
		}
	}

	return	rst;
}

bool	cmn_get_ipnport_by_saaddr(const struct sockaddr_in *__addr, char *__ipstr, port_t *__port)
{
	if( __ipstr )
	{
		strcpy( __ipstr, inet_ntoa(__addr->sin_addr) );
	}
	if( __port )
	{
		*__port		= ntohs( __addr->sin_port );
	}
	
	return	true;
}
const char *	cmn_get_ip_by_saaddr(const struct sockaddr_in *__addr)
{
	static char	szIPStr[WHNET_MAXADDRSTRLEN];
	cmn_get_ipnport_by_saaddr(__addr, szIPStr, NULL);
	return	szIPStr;
}
const char *	cmn_get_ipnportstr_by_saaddr(const struct sockaddr_in *__addr, char *szStr)
{
	char	szIP[WHNET_MAXADDRSTRLEN];
	port_t	port;
	cmn_get_ipnport_by_saaddr(__addr, szIP, &port);
	sprintf(szStr, "%s:%d", szIP, port);
	return	szStr;
}
const char *	cmn_get_ipnportstr_by_saaddr(const struct sockaddr_in *__addr)
{
	static char	szAddrStr[WHNET_MAXADDRSTRLEN];
	return	cmn_get_ipnportstr_by_saaddr(__addr, szAddrStr);
}
bool	cmn_get_saaddr_by_ipnportstr(struct sockaddr_in *__addr, const char *__ipportstr)
{
	char	szIP[WHNET_MAXADDRSTRLEN] = "";
	int		nPort = 0;

	wh_strsplit("sd", __ipportstr, ":", szIP, &nPort);

	return	cmn_get_saaddr_by_ipnport(__addr, szIP, nPort);
}
bool	cmn_get_saaddr_by_ipnport(struct sockaddr_in *__addr, const char *__ipstr, const port_t __port)
{
	memset(__addr, 0, sizeof(*__addr));

	__addr->sin_family	= AF_INET;
	if( __ipstr && __ipstr[0] )
	{
		unsigned int	addr	= cmn_get_S_addr(__ipstr);
		__addr->sin_addr.s_addr	= addr;
		if( addr==0 )
		{
			// ˵�������Ǵ���ģ������ҪADDR_ANYҪʹ�ÿմ���
			return	false;
		}
	}
	else
	{
		__addr->sin_addr.s_addr	= INADDR_ANY;
	}
	// �˿�Ϊ0ҲҪ��һ��
	__addr->sin_port			= htons(__port);
	return	true;
}
bool	cmn_get_saaddr_by_ipnport(struct sockaddr_in *__addr, unsigned int __ip, const port_t __port)
{
	memset(__addr, 0, sizeof(*__addr));

	__addr->sin_family	= AF_INET;
	if( __ip )
	{
		__addr->sin_addr.s_addr	= __ip;
	}
	else
	{
		__addr->sin_addr.s_addr	= INADDR_ANY;
	}
	// �˿�Ϊ0ҲҪ��һ��
	__addr->sin_port			= htons(__port);
	return	true;
}
bool	cmn_get_saaddr_by_ip(struct sockaddr_in *__addr, const char *__ipstr)
{
	memset(__addr, 0, sizeof(*__addr));

	__addr->sin_family	= AF_INET;
	if( __ipstr && __ipstr[0] )
	{
		__addr->sin_addr.s_addr	= cmn_get_S_addr(__ipstr);
	}
	else
	{
		__addr->sin_addr.s_addr	= INADDR_ANY;
	}

	return	true;
}
void	cmn_get_saaddr_by(struct sockaddr_in *__addr, const whnet4byte_t &__ip, const port_t __port)
{
	memset(__addr, 0, sizeof(*__addr));
	__addr->sin_family		= AF_INET;
	__addr->sin_addr.s_addr	= __ip.nl;
	__addr->sin_port		= htons(__port);
}

void	cmn_set_reuseaddr(bool bSet)
{
	G_BREUSEADDR	= bSet;
}
bool	cmn_get_reuseaddr()
{
	return	G_BREUSEADDR;
}
static bool	g_bOldBREUSEADDR	= false;
bool	cmn_push_reuseaddr_val(bool bSet)
{
	g_bOldBREUSEADDR	= G_BREUSEADDR;
	G_BREUSEADDR		= bSet;
	return	g_bOldBREUSEADDR;
}
bool	cmn_pop_reuseaddr_val()
{
	bool	bOld		= G_BREUSEADDR;
	G_BREUSEADDR		= g_bOldBREUSEADDR;
	return	bOld;
}

bool	cmn_bind_local(SOCKET __sock, port_t __port, struct sockaddr_in *__addr)
{
	return	cmn_bind(__sock, NULL, __port, __addr);
}
bool	cmn_bind(SOCKET __sock, const char *__ip, port_t __port, struct sockaddr_in *__addr)
{
	struct sockaddr_in	addr;
	if( !cmn_get_saaddr_by_ipnport(&addr, __ip, __port) )
	{
		return	false;
	}
	// ��������ΪREUSEADDR
	if( G_BREUSEADDR )
	{
		if( cmn_setsockopt_int(__sock, SOL_SOCKET, SO_REUSEADDR, 1)<0 )
		{
			return	false;
		}
	}
	bool	bRst;
	bRst	= bind(__sock, (const struct sockaddr *)&addr, sizeof(addr)) == 0;
	if( bRst )
	{
		if( __addr )
		{
			if( __ip && __ip[0] && __port )
			{
				// ����ȷ�ĵ�ַ
				memcpy(__addr, &addr, sizeof(*__addr));
			}
			else
			{
				socklen_t	len = sizeof(*__addr);
				// ��ȡ��ַ(ֻ�а󶨳ɹ�����ܵõ�)
				::getsockname(__sock, (struct sockaddr *)__addr, &len);
			}
		}
	}

	return	bRst;
}
bool	cmn_randbind_local(SOCKET __sock, port_t __port0, port_t __port1, struct sockaddr_in *__addr)
{
	return	cmn_randbind(__sock, NULL, __port0, __port1, __addr);
}
bool	cmn_randbind(SOCKET __sock, const char *__ip, port_t __port0, port_t __port1, struct sockaddr_in *__addr)
{
	// ����һ���˿ڵ����
	if( __port1==0 )
	{
		return	cmn_bind(__sock, __ip, __port0, __addr);
	}

	// Ĭ�Ͻ��
	bool	bRst			= true;
	// ��¼��״̬
	cmn_push_reuseaddr_val(false);

	for(port_t port=__port0; port<=__port1; port++)
	{
		if( cmn_bind(__sock, __ip, port, __addr) )
		{
			goto	End;
		}
	}
	// ����һ����ȫ������������
	bRst			= false;
End:
	cmn_pop_reuseaddr_val();
	return			bRst;
}

int		cmn_addr_cmp(const struct sockaddr_in *__addr1, const struct sockaddr_in *__addr2)
{
	int	rst = memcmp(&__addr1->sin_port, &__addr2->sin_port, sizeof(__addr1->sin_port));
	if( rst!=0 )
	{
		return	rst;
	}
	return	memcmp(&__addr1->sin_addr.s_addr, &__addr2->sin_addr.s_addr, sizeof(&__addr1->sin_addr.s_addr));
}
int		cmn_addr_cpy(struct sockaddr_in *__addrdst, const struct sockaddr_in *__addrsrc)
{
	memcpy(__addrdst, __addrsrc, sizeof(*__addrdst));
	return	0;
}

SOCKET	cmn_create_socket(int __type, int __protocol, const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr)
{
	SOCKET	sock;

	// Create socket
	sock	= socket(AF_INET, __type, __protocol);
	if( sock == INVALID_SOCKET )
	{
		return	sock;
	}

	// bind it to local port
	if( !cmn_randbind(sock, __ip, __port1, __port2, __addr) )
	{
		goto	ErrEnd;
	}

	goto GoodEnd;
ErrEnd:
	// �����ˣ�����Ѿ�������socket��Ҫ�ر�֮
	closesocket(sock);
	sock	= INVALID_SOCKET;
GoodEnd:
	return	sock;
}

int		cmn_select_rd(SOCKET __sock, int __timeout)
{
	if( __sock == INVALID_SOCKET )
	{
		return	-1;
	}
	struct timeval tv;
	cmn_set_timevalfromms(tv, __timeout);
	fd_set	fsr;
	int		rst;
	FD_ZERO( &fsr );
	FD_SET( __sock, &fsr );
	rst	= select(__sock+1, &fsr, NULL, NULL, &tv);
	return	rst;
}
int		cmn_select_wr(SOCKET __sock, int __timeout)
{
	if( __sock == INVALID_SOCKET )
	{
		return	-1;
	}
	struct timeval tv;
	cmn_set_timevalfromms(tv, __timeout);
	fd_set	fsr;
	int		rst;
	FD_ZERO( &fsr );
	FD_SET( __sock, &fsr );
	rst	= select(__sock+1, NULL, &fsr, NULL, &tv);
	return	rst;
}
int		cmn_select_multi_rd(int __timeout, ...)
{
	struct timeval tv;
	cmn_set_timevalfromms(tv, __timeout);
	fd_set	fsr;
	int		rst;
	FD_ZERO( &fsr );

	va_list	arglist;
	SOCKET	sock, sockmax = 0;
	va_start( arglist, __timeout );
	while( (sock=va_arg( arglist, SOCKET)) != 0 )
	{
		if( sock!=INVALID_SOCKET )
		{
			FD_SET( sock, &fsr );
			if( sockmax<sock )
			{
				sockmax	= sock;
			}
		}
	}
	va_end( arglist );

	// û�в�����ˣ�Ұ� :(((((
	if( sockmax==0 ) 
	{
		return	0;
	}

	rst	= select(sockmax+1, &fsr, NULL, NULL, &tv);
	return	rst;
}
int		cmn_select_rd_array(SOCKET *__sockarray, int __num, int __timeout)
{
	if( __num==0 )		// ��û��socket��ʱ�򣬾Ͳ�select��ֱ��sleepһ��ʱ��
	{
		wh_sleep(__timeout>100 ? 100:__timeout);
		return	0;
	}

	struct timeval tv;
	cmn_set_timevalfromms(tv, __timeout);
	fd_set	fsr;
	FD_ZERO( &fsr );

	SOCKET	sockmax = 0;
	for(int i=0;i<__num;i++)
	{
		SOCKET	sock = __sockarray[i];
		if( sock != INVALID_SOCKET )
		{
			FD_SET( sock, &fsr );
			if( sockmax<sock )
			{
				sockmax	= sock;
			}
		}
	}

	return	select(sockmax+1, &fsr, NULL, NULL, &tv);
}
int		cmn_assemble_sock_to_select(SOCKET *__sockarray, int __num, fd_set *__fsr, SOCKET *__sockmax)
{
	FD_ZERO( __fsr );

	*__sockmax	= 0;
	for(int i=0;i<__num;i++)
	{
		SOCKET	sock = __sockarray[i];
		FD_SET( sock, __fsr );
		if( *__sockmax<sock )
		{
			*__sockmax	= sock;
		}
	}
	return	0;
}
int		cmn_select_rd_array_big(SOCKET *__sockarray, int __num, int __timeout)
{
	struct timeval tv	= {0, 0};
	fd_set	fsr;
	int		nCount		= 0;
	while( __num>0 )
	{
		int	nSize;
		if( __num>=FD_SETSIZE )
		{
			nSize		= FD_SETSIZE;
		}
		else
		{
			nSize		= __num;
			if( nCount==0 )
			{
				// ���ǰ�滹û��select��һ������ô����������ʱ�������ֱ��select0���ȵ�ʱ�䣬���ٽ����������
				cmn_set_timevalfromms(tv, __timeout);
			}
		}
		SOCKET	sockMax	= 0;
		cmn_assemble_sock_to_select(__sockarray, nSize, &fsr, &sockMax);
		int	rst			= select(sockMax+1, &fsr, NULL, NULL, &tv);
		if( rst<0 )
		{
			return		rst;
		}
		nCount			+= rst;
		// ���������
		__num			-= FD_SETSIZE;
		__sockarray		+= FD_SETSIZE;
	}
	return	nCount;
}
struct timeval &	cmn_set_timevalfromms(struct timeval &tv, int nMS)
{
	/*
	tv.tv_sec	= nMS/1000;
	tv.tv_usec	= (nMS%1000)*1000L;
	*/
	tv.tv_sec	= 0;
	tv.tv_usec	= nMS*1000L;
	return	tv;
}

int		cmn_ioctl(SOCKET __sock, int __cmd, int *__arg)
{
#if defined (WIN32)
	return	ioctlsocket(__sock, __cmd, (u_long*)__arg);
#endif
#if defined (__GNUC__)
	return	ioctl(__sock, __cmd, __arg);
#endif
}

int		cmn_getsizetorecv(SOCKET __sock)
{
	int		size;
	if( cmn_ioctl(__sock, FIONREAD, &size)<0 )
	{
		return	-1;
	}
	return	size;
}

int		cmn_setsockopt_int(SOCKET __sock, int __level, int __optname, int __val)
{
	return	setsockopt(__sock, __level, __optname, (const char *)&__val, sizeof(__val));
}

int		cmn_setsock_nonblock(SOCKET __sock, bool __nonblock)
{
	int		arg;
	arg		= __nonblock;
	return	cmn_ioctl(__sock, FIONBIO, &arg);
}
/*
int		cmn_setsock_nonblock(SOCKET __sock, bool __nonblock)
{
	int		rst;
#if defined (WIN32)
	u_long	nonblock;
	nonblock= __nonblock;
	rst		= ioctlsocket(__sock, FIONBIO, &nonblock);
	return	rst;
#endif
#if defined (__GNUC__)
	int	flags;
	flags	= fcntl(__sock, F_GETFL, 0);
	if( __nonblock )
	{
		rst	= fcntl(__sock, F_SETFL, flags | O_NONBLOCK);
	}
	else
	{
		rst	= fcntl(__sock, F_SETFL, flags & ~O_NONBLOCK);
	}
	return	rst;
#endif
}
*/
void	cmn_clear_error()
{
#if defined (WIN32)
	// do nothing
#endif
#if defined (__GNUC__)
	errno	= 0;
#endif
}
bool	cmn_is_nonblock_error()
{
#if defined (WIN32)
	return	WSAGetLastError() == WSAEWOULDBLOCK;
#endif
#if defined (__GNUC__)
	return	errno == EINPROGRESS;
#endif
}

// ����linger
int		cmn_setsock_linger(SOCKET __sock, unsigned short __onoff, unsigned short __linger)
{
	struct linger	l;
	l.l_linger	= __linger;
	l.l_onoff	= __onoff;
	return	setsockopt(__sock, SOL_SOCKET, SO_LINGER, (const char *)&l, sizeof(l));
}

int		cmn_setkeepalive(SOCKET __sock, int __idle, int __interval)
{
// ���������:)
#ifdef WIN32
	tcp_keepalive inKeepAlive		= {0};
	tcp_keepalive outKeepAlive		= {0};
	unsigned long ulBytesReturn		= 0;

	inKeepAlive.onoff				= 1;
	inKeepAlive.keepaliveinterval	= __interval;
	inKeepAlive.keepalivetime		= __idle;

	if( ::WSAIoctl(__sock, SIO_KEEPALIVE_VALS,
		(LPVOID)&inKeepAlive, sizeof(inKeepAlive),
		(LPVOID)&outKeepAlive, sizeof(outKeepAlive),
		&ulBytesReturn, NULL, NULL) == SOCKET_ERROR
	)
	{
		return -1;
	}
#else
	//�趨KeepAlive
	if( cmn_setsockopt_int(__sock, SOL_SOCKET, SO_KEEPALIVE, 1) < 0 )
	{
		return -1;
	}

	//��ʼ�״�KeepAlive̽��ǰ��TCP�ձ�ʱ��
	if( cmn_setsockopt_int(__sock, SOL_TCP, TCP_KEEPIDLE, __idle/1000) < 0 )
	{
		return -2;
	}

	//����KeepAlive̽����ʱ����
	if( cmn_setsockopt_int(__sock, SOL_TCP, TCP_KEEPINTVL, __interval/1000) < 0 )
	{
		return -3;
	}

	//�ж��Ͽ�ǰ��KeepAlive̽�����
	if( cmn_setsockopt_int(__sock, SOL_TCP, TCP_KEEPCNT, 3) < 0 )
	{
		return -4;
	}
#endif

	return 0;
}

int		cmn_getsockerror(SOCKET __sock)
{
	return	cmn_getsockopt_int(__sock, SOL_SOCKET, SO_ERROR);
}

int		cmn_getsockopt_int(SOCKET __sock, int __level, int __optname)
{
	int	nVal = 0;
	socklen_t	nLen = sizeof(nVal);
	int	rst;
	rst	= getsockopt(__sock, __level, __optname, (char *)&nVal, &nLen);
	return	nVal;
}

// �����whprocess������Ӧ�ĺ���whprocess_getidentifier������Ϊ�˼�����ֱ��ʵ��һ����
unsigned int	cmn_getpid()
{
#if defined (WIN32)
	return	GetCurrentProcessId();
#endif
#if defined (__GNUC__)
	return	getpid();
#endif
}

// һ���ִ�ΪXX:XX:XX:XX:XX:XX��XX-XX-XX-XX-XX-XX����ʽ
int		cmn_MACStrtoBin(char *szMAC, unsigned char *szBin)
{
	int		nHex;
	char	unit[3];
	unit[2]	= 0;
	int		i;
	for(i=0;i<6;i++)
	{
		strncpy(unit, szMAC, 2);
		sscanf(unit, "%x", &nHex);
		szBin[i]	= nHex;
		szMAC		+= 3;
	}
	return	0;
}
char *	cmn_MACBintoStr(unsigned char *szBin, char *szMAC)
{
	sprintf(szMAC, "%02X:%02X:%02X:%02X:%02X:%02X"
		, szBin[0]
		, szBin[1]
		, szBin[2]
		, szBin[3]
		, szBin[4]
		, szBin[5]
		);
	return	szMAC;
}

int		cmn_IPtoBin(const char *szIP, unsigned char *szBin)
{
	int		nPart[4];
	int		rst, i;

	rst		= wh_strsplit("dddd", szIP, "."
			, nPart+0
			, nPart+1
			, nPart+2
			, nPart+3
			);
	if( rst!=4 )
	{
		return	-1;
	}
	for(i=0;i<4;i++)
	{
		szBin[i]	= nPart[i];
	}

	return	0;
}
char *	cmn_BintoIP(unsigned char *szBin, char *szIP)
{
	static char	szIPStr[64];
	if( !szIP )
	{
		szIP	= szIPStr;
	}
	sprintf(szIP, "%d.%d.%d.%d"
		, szBin[0]
		, szBin[1]
		, szBin[2]
		, szBin[3]
		);
	return	szIP;
}

////////////////////////////////////////////////////////////////////
// ������������ȡ
////////////////////////////////////////////////////////////////////
int		cmn_getlasterror()
{
#ifdef	WIN32
	return	WSAGetLastError();
#endif
#ifdef	__GNUC__
	return	errno;
#endif
}

void	cmn_parse_addr0(const char *szAddrStr, EASYADDR *pAddr)
{
	char	szPort[128];
	pAddr->clear();
	wh_strsplit("ss", szAddrStr, ":", pAddr->szAddrStr, szPort);
	wh_strsplit("oo", szPort, ",", &pAddr->port, &pAddr->port2);
}
void	cmn_parse_addr(const char *szAddrStr, EASYADDR *pAddr, int nAct)
{
	switch(nAct)
	{
		case	WHSTR_ACT_PARSE:
		{
			cmn_parse_addr0(szAddrStr, pAddr);
		}
		break;
		case	WHSTR_ACT_PRINT:
		{
			printf("%s:%d,%d", pAddr->szAddrStr, pAddr->port, pAddr->port2);
		}
		break;
	}
}
void	cmn_parse_ETCHost(const char *szLine, void *pNotUse, int nAct)
{
	switch(nAct)
	{
		case	WHSTR_ACT_PARSE:
		{
			cmn_AddETCHostsLine(szLine);
		}
		break;
		// ��ӡ������������Ϊָ��û����
	}
}
void	cmn_glbsetaddr(void *var, const char *cszVal)
{
	cmn_parse_addr0(cszVal, (EASYADDR*)var);
}

int		cmn_getstat(char * memory,char * cpu,char * processname)
{
#if defined (WIN32)
	return -1;
#endif
#if defined (__GNUC__)
	 int mypid=0;
     mypid=getpid();
     char top_cmd[128]="";
     sprintf(top_cmd,"ps -p %d -o pid,comm,pcpu,size | grep %d ",mypid,mypid);
     
     FILE * stream;
     stream= popen(top_cmd,"r");
	 char ppid[32]="";
	 int n= fscanf(stream,"%s %s %s %s",ppid,processname,cpu,memory);
	pclose(stream);
	 return 0;
#endif

}

}		// EOF namespace n_whnet

using namespace n_whnet;

////////////////////////////////////////////////////////////////////
// ΪDLL֮����Ϣͬ��
////////////////////////////////////////////////////////////////////
void *	WHNET_STATIC_INFO_Out()
{
	n_whnet::l_si.pUDPLoggerInfo	= WHNET_UDPGLOGGER_STATIC_INFO_Out();
	n_whnet::l_si.pEpollData		= WHNET_Get_EpollSharedData();
	return	&n_whnet::l_si;
}
// ����ļ�һ���ᱻ���룬���Բ�����WHCMN_STATIC_INFO_In������
extern "C" WH_DLL_API	void	WHNET_STATIC_INFO_In(void *pInfo)
{
	if( !pInfo )
	{
		return;
	}
#ifdef	_DEBUG
	printf("WHNET_STATIC_INFO_In %p %p%s", &n_whnet::l_si, pInfo, WHLINEEND);
#endif
	n_whnet::WHNET_STATIC_INFO_T	*psi	= (n_whnet::WHNET_STATIC_INFO_T *)pInfo;
	// ��hash���ݸ��ƹ���
	L_MYHASH		= psi->l_myHash;
	// reuseaddr
	G_BREUSEADDR	= psi->g_bReuseAddr;
	// GLOGGER��ص�����
	WHNET_UDPGLOGGER_STATIC_INFO_In(psi->pUDPLoggerInfo);

	WHNET_Set_EpollSharedData(psi->pEpollData);
}

////////////////////////////////////////////////////////////////////
// C++����
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// cmn_selector
////////////////////////////////////////////////////////////////////
void	cmn_selector::clear()
{
	FD_ZERO( &m_fsr );
	FD_ZERO( &m_fsrbak );
	m_sockmax	= 0;
	m_sockmax1	= 0;
	m_vectsock.clear();
	memset(&m_tv, 0, sizeof(m_tv));
#ifdef	__GNUC__
	memset(&m_tvbak, 0, sizeof(m_tvbak));
#endif
}
void	cmn_selector::settimeout(int __timeout)
{
	cmn_set_timevalfromms(m_tv, __timeout);
#ifdef	__GNUC__
	memcpy(&m_tvbak, &m_tv, sizeof(m_tvbak));
#endif
}
int		cmn_selector::addsocket(SOCKET __sock)
{
	if( __sock == INVALID_SOCKET )
	{
		return	-1;
	}
	if( m_sockmax<__sock )
	{
		m_sockmax	= __sock;
		m_sockmax1	= __sock + 1;
	}
	FD_SET( __sock, &m_fsrbak );
	m_vectsock.push_back(__sock);
	return	0;
}
int		cmn_selector::removesocket(SOCKET __sock)
{
	FD_CLR( __sock, &m_fsrbak );
	m_vectsock.delvalue(__sock);
	return	0;
}
int		cmn_selector::select_rd()
{
	restorefsr();
#ifdef	__GNUC__
	memcpy(&m_tv, &m_tvbak, sizeof(m_tv));	// ��linux�»�ʹm_tv����
#endif
	return	select(m_sockmax1, &m_fsr, NULL, NULL, &m_tv);
}
int		cmn_selector::select_rd(int __timeout)
{
	restorefsr();
	struct timeval tv;
	cmn_set_timevalfromms(tv, __timeout);
	return	select(m_sockmax1, &m_fsr, NULL, NULL, &tv);
}
int		cmn_selector::select_rd_with_extrasocket(SOCKET s1, ...)
{
	restorefsr();
#ifdef	__GNUC__
	memcpy(&m_tv, &m_tvbak, sizeof(m_tv));	// ��linux�»�ʹm_tv����
#endif
	SOCKET	sockmax	= m_sockmax;
	va_list	arglist;
	SOCKET	sock;
	va_start( arglist, s1 );
	while( (sock=va_arg(arglist, SOCKET)) != 0 )
	{
		if( sock==INVALID_SOCKET )
		{
			continue;
		}
		FD_SET( sock, &m_fsr );
		if( sockmax<sock )
		{
			sockmax	= sock;
		}
	}
	if( s1 != INVALID_SOCKET )
	{
		FD_SET( s1, &m_fsr );
	}
	if( sockmax<s1)
	{
		sockmax		= s1;
	}
	va_end( arglist );

	return	select(sockmax+1, &m_fsr, NULL, NULL, &m_tv);
}
int		cmn_selector::select_rd_with_extrasocketvect(whvector<SOCKET> &vect)
{
	restorefsr();
#ifdef	__GNUC__
	memcpy(&m_tv, &m_tvbak, sizeof(m_tv));	// ��linux�»�ʹm_tv����
#endif
	SOCKET	sockmax	= m_sockmax;
	for(size_t i=0;i<vect.size();i++)
	{
		SOCKET	sock = vect[i];
		if( sock==INVALID_SOCKET )
		{
			continue;
		}
		FD_SET( sock, &m_fsr );
		if( sockmax<sock )
		{
			sockmax	= sock;
		}
	}
	return	select(sockmax+1, &m_fsr, NULL, NULL, &m_tv);
}

////////////////////////////////////////////////////////////////////
// cmn_addrmatchlist
////////////////////////////////////////////////////////////////////
int		cmn_addrmatchlist::addpolicy(const whnet4byte_t &addr, const whnet4byte_t &mask)
{
	POLICY_T	*pPolicy	= m_policylist.push_back();
	pPolicy->setaddrmask(addr, mask);
	return					0;
}
int		cmn_addrmatchlist::addpolicy(const char *cszAddr, const char *cszMask)
{
	whnet4byte_t	addr, mask;
	if( cmn_IPtoBin(cszAddr, addr.c)<0 )
	{
		return	-1;
	}
	if( cmn_IPtoBin(cszMask, mask.c)<0 )
	{
		return	-1;
	}
	return		addpolicy(addr, mask);
}
int		cmn_addrmatchlist::addpolicy(const char *cszAddrAndMask)
{
	char	szAddr[WHNET_MAXADDRSTRLEN], szMask[WHNET_MAXADDRSTRLEN];
	if( wh_strsplit("ss", cszAddrAndMask, "/", szAddr, szMask)==1 )
	{
		strcpy(szMask, "255.255.255.255");
	}
	return	addpolicy(szAddr, szMask);
}
int		cmn_addrmatchlist::delpolicy(const whnet4byte_t &addr, const whnet4byte_t &mask)
{
	return	m_policylist.delvalue(POLICY_T(addr, mask));
}
int		cmn_addrmatchlist::delpolicy(const char *cszAddr, const char *cszMask)
{
	whnet4byte_t	addr, mask;
	if( cmn_IPtoBin(cszAddr, addr.c)<0 )
	{
		return	-1;
	}
	if( cmn_IPtoBin(cszMask, mask.c)<0 )
	{
		return	-1;
	}
	return		delpolicy(addr, mask);
}
int		cmn_addrmatchlist::delpolicy(const char *cszAddrAndMask)
{
	char	szAddr[WHNET_MAXADDRSTRLEN], szMask[WHNET_MAXADDRSTRLEN];
	if( wh_strsplit("ss", cszAddrAndMask, "/", szAddr, szMask)==1 )
	{
		strcpy(szMask, "255.255.255.255");
	}
	return	delpolicy(szAddr, szMask);
}
int		cmn_addrmatchlist::delallpolicy()
{
	m_policylist.clear();
	return	0;
}
bool	cmn_addrmatchlist::ismatch(const whnet4byte_t &addrtochk)
{
	// �������еĵ�Ԫ�����Ƿ�match
	for(size_t i=0;i<m_policylist.size();i++)
	{
		if( cmn_addr_match_quick(m_policylist[i].addrandmask, m_policylist[i].mask, addrtochk) )
		{
			return	true;
		}
	}
	return			false;
}
bool	cmn_addrmatchlist::ismatch(const char *cszAddrToChk)
{
	whnet4byte_t	addr;
	if( cmn_IPtoBin(cszAddrToChk, addr.c)<0 )
	{
		return	false;
	}
	return	ismatch(addr);
}
bool	cmn_addrmatchlist::ismatch(const struct sockaddr_in &addr)
{
	return	ismatch(addr.sin_addr.s_addr);
}
int		cmn_addrmatchlist::initfromfile(whfile *file)
{
	char	buf[WHNET_MAXADDRSTRLEN*2];
	while( !file->IsEOF() )
	{
		if( file->ReadLine(buf, sizeof(buf)-1)<=0 )
		{
			// �ļ�����
			break;
		}
		wh_strtrim(buf);
		addpolicy(buf);
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// cmn_addr_allowdeny
////////////////////////////////////////////////////////////////////
bool	cmn_addr_allowdeny::isallowed(const whnet4byte_t &addrtochk)
{
	// �����DENY������ôƥ��ľ�һ������ֹ����ƥ���һ������ֹ
	if( m_mlDeny.policynum()>0 )
	{
		if( m_mlDeny.ismatch(addrtochk) )
		{
			return	false;
		}
		else
		{
			return	true;
		}
	}
	// ���Ҳû��ALLOW����������
	if( m_mlAllow.policynum()==0 )
	{
		return		true;
	}
	// �����ALLOWƥ��ĵģ���һ������
	if( m_mlAllow.ismatch(addrtochk) )
	{
		return		true;
	}
	// ʣ�µľͱ�����
	return			false;
}
bool	cmn_addr_allowdeny::isallowed(const char *cszAddrToChk)
{
	whnet4byte_t	addr;
	if( cmn_IPtoBin(cszAddrToChk, addr.c)<0 )
	{
		return	false;
	}
	return	isallowed(addr);
}
bool	cmn_addr_allowdeny::isallowed(const struct sockaddr_in &addr)
{
	return	isallowed(addr.sin_addr.s_addr);
}

const char	*CSZFUNKEY	= "ADDR";
class	cmn_addr_allowdeny_lineanalyzer	: public whdataini::cpplineanalyzer
{
private:
	// ��������壬ֱ��дwhdataini::cpplineanalyzer::analyze�ᱨ��
	// ˵ʲô�Ǿ�̬��Ա������ô����
	typedef	whdataini::cpplineanalyzer	upperclass;
public:
	int		analyze(const char *cszLine)
	{
		int	rst	= upperclass::analyze(cszLine);
		switch( rst )
		{
			case	upperclass::TYPE_OBJBEGIN:
			case	upperclass::TYPE_OBJEND:
			case	upperclass::TYPE_BADLINE:
			case	upperclass::TYPE_COMMENT:	// 2007-07-23 ��
				// ������ķ��أ��Ͳ��ü���������
				return	rst;
			break;
		}
		// ��������ǿ��а�һ�����
		if( cszLine[0] )
		{
			if( cszLine[0]=='*' )
			{
				// ���������
				return	TYPE_NOTHING;
			}
			strcpy(m_szParam[0], CSZFUNKEY);
			strcpy(m_szParam[1], cszLine);
			return	upperclass::TYPE_VAL;
		}
		//
		return		upperclass::TYPE_NOTHING;
	}
};
struct	CMN_ADDR_ALLOWDENY_INIT	: public whdataini::obj
{
	cmn_addrmatchlist	*pAM;
	int	SetVal_Addr(const char *cszVal)
	{
		return	pAM->addpolicy(cszVal);
	}
	CMN_ADDR_ALLOWDENY_INIT()
	: pAM(NULL)
	{
	}
	CMN_ADDR_ALLOWDENY_INIT(cmn_addrmatchlist *__pAM)
	: pAM(__pAM)
	{
	}
	WHDATAPROP_DECLARE_MAP(CMN_ADDR_ALLOWDENY_INIT);
};
WHDATAPROP_MAP_BEGIN_AT_ROOT(CMN_ADDR_ALLOWDENY_INIT)
	WHDATAPROP_ON_SETVALFUNC0(CSZFUNKEY, SetVal_Addr, 1)
WHDATAPROP_MAP_END()
int		cmn_addr_allowdeny::initfromfile(n_whcmn::whfile *file)
{
	m_mlAllow.delallpolicy();
	m_mlDeny.delallpolicy();

	CMN_ADDR_ALLOWDENY_INIT	Allow(&m_mlAllow), Deny(&m_mlDeny);
	WHDATAINI<whdataini::objgetter, cmn_addr_allowdeny_lineanalyzer>	ini;
	ini.addobj("*ALLOW", &Allow);
	ini.addobj("*DENY", &Deny);

	int	rst = ini.analyzefile(file);

	return	rst;
}
int		cmn_addr_allowdeny::initfromfile(const char *cszFile)
{
	whfile	*fp;
	fp		= whfile_OpenCmnFile(cszFile, "r");
	if( !fp )
	{
		return	-1;
	}
	else
	{
		int	rst	= initfromfile(fp);
		whfile_CloseCmnFile(fp);
		return	rst;
	}
}


///////////////////////////////////////////////////
// ��ȡ��ǰ���̵�ϵͳռ��
//////////////////////////////////////////////////
