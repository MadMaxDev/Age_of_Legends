// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcmn.h
// Creator      : Wei Hua (κ��)
// Comment      : �򵥵����繦��
//                ��windows����Ҫ��ʵ��������Ŀ⣬ֻҪ�õ���������صĹ��ܾ�Ҫ������
//                #pragma comment(lib, "wsock32.lib")
//                ���� #pragma comment(lib, "WS2_32.lib") ����ʵ��ǰ��Ϳ����ˣ�
//                2004-03-08 cmn_setsockopt_int(m_sock, SOL_SOCKET, SO_RCVBUF, xxxx)�������õ����ֵ��
//                ��linux���ܵ�/proc/sys/net/core��rmem_max�����ƣ���صĻ���rmem_default��rmem_max��wmem_default��wmem_max
//                !!!!��Ҫ�ö���������cmn_getsizetorecv��ʱ���ܵõ�ȫ�������ݡ���������ͬ������udp�����ͺ�!!!!!!!! (2005-07-13�ٴη���) !!!!!!!!
//                !!!!���ң���ʹ��tcpʱ��cmn_getsizetorecv��õ����ݳߴ粻һ���ܱ�ȫ���õ����п��ܻ�С�������õ��Ĵ�С
//                addr.sin_addr.s_addr ������ã����Կ��������� :)
// CreationDate : 2003-05-14
// ChangeLog    : 2003-08-06
//                ԭ����cmn_select_rd(sock,0)�жϻ������Ƿ������ݣ�����ֱ����cmn_getsizetorecv>0�жϣ�Ҫ��������
//                ��װ�ĺ�����bool cmn_havedatatorecv���Ժ����еط����������
//                ����������жϣ���ô��ֱ��ʹ��cmn_getsizetorecvȻ�����recv�ĳ�����μ���<=0����Ϊ������
//                2003-08-15
//                ������cmn_getsizetorecv��ʱ���ܵõ�ȫ�������ݡ���������ͬ������udp�����ͺ�!!!!!!!!
//                2004-02-11
//                ��Linux�£�FD_SET�Ĳ��������-1���ͻᷢ���ڴ���ʴ������԰����е�FD_SET�ĵط����ж���һ��
//                2004-06-30
//                �����˼����ַƥ��Ŀ����жϺ���cmn_addr_match_quick
//                2005-07-01
//                ������cmn_DelETCHost��������ɾ���Ѿ���¼��host��ַ���Ӷ���������ˢ�������URL��
//                2005-07-13
//                �ٴη���cmn_getsizetorecv��������֮������⡣�����ö���
//                2007-05-29
//                �����˶�reuseaddr���õļ�������

#ifndef	__WHNETCMN_H__
#define	__WHNETCMN_H__

#include <WHCMN/inc/wh_platform.h>

#if defined( WIN32 )					// Windows�µ�ͷ		{
#define	MSG_NOSIGNAL	0
#include <wsipx.h>
#include <mstcpip.h>
// windows��û����Щ����
#ifndef	socklen_t
#define	socklen_t		int
#endif
const int SHUT_RD		= 0;
const int SHUT_WR		= 1;
const int SHUT_RDWR		= 2;
#endif									// EOF Windows�µ�ͷ	}

#if defined( __GNUC__ )					// Linux�µ�ͷ			{
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// linux��û����Щ����
typedef	int				 SOCKET;		// ע��������ܷŵ�namespace n_whnet����
#define	INVALID_SOCKET	-1
#define	SOCKET_ERROR	-1
#define	closesocket(fd)	close(fd)
#endif									// EOF Linux�µ�ͷ		}

#include <WHCMN/inc/whtime.h>
#include <WHCMN/inc/whvector.h>
#include <WHCMN/inc/whfile.h>

// ������ͬʱ����select��socket����
#define	WHNET_MAX_SOCKTOSELECT			FD_SETSIZE
// �����ַ�ִ�����󳤶�(���������Ļ�����IP��)
#define	WHNET_MAXADDRSTRLEN				256
// IP������󳤶�
#define	WHNET_MAXIPSTRLEN				32

namespace n_whnet
{

typedef	unsigned short		port_t;
//typedef unsigned int		u_int;
typedef unsigned char		u_char;

// 2004-07-08Ϊ����չ��������port2����Ӧ�ִ���ʽΪip:port[,port2]
struct	EASYADDR						// �򵥵ĵ�ַ�ṹipstr+port
{
	char	szAddrStr[WHNET_MAXADDRSTRLEN];
	port_t	port;
	port_t	port2;
	// ���port��port2����0������port~port2��Χ�ڰ�

	EASYADDR()
	{
		clear();
	}
	inline void	clear()
	{
		szAddrStr[0]	= 0;
		port			= 0;
		port2			= 0;
	}
};

// �򵥵�ַ
// nl��ͬ��addr.sin_addr.s_addr
union	whnet4byte_t
{
	unsigned int	nl;
	unsigned char	c[4];	// 0~3Ϊ��IP�����Ӧ��˳��
	void	reverse()		// �ֽڵߵ�˳��
	{
		unsigned char	tc;
		tc		= c[0];
		c[0]	= c[3];
		c[3]	= tc;
		tc		= c[1];
		c[1]	= c[2];
		c[2]	= tc;
	}
	whnet4byte_t()
	: nl(0)
	{
	}
	whnet4byte_t(const unsigned int &_nl)
	: nl(_nl)
	{
	}
	whnet4byte_t(const whnet4byte_t &_other)
	: nl(_other.nl)
	{
	}
};
inline bool	operator == (const whnet4byte_t & __addr1, const whnet4byte_t & __addr2)
{
	return	__addr1.nl == __addr2.nl;
}

// �����硰ip:port[,port2]���ĵ�ַ��������EASYADDR�ṹ��
// nAct������WHSTR_ACT_XXX�������whstring.h
void	cmn_parse_addr0(const char *szAddrStr, EASYADDR *pAddr);
void	cmn_parse_addr(const char *szAddrStr, EASYADDR *pAddr, int nAct);
// �����Լ���Host
void	cmn_parse_ETCHost(const char *szLine, void *pNotUse, int nAct);
// ����Ǹ�whdatainiӦ�õ�
void	cmn_glbsetaddr(void *var, const char *cszVal);
// ��ʹ��ʱ����ʹ������Ķ��壬����WH_LINE_PARSE_INFO_T�ṹ��
// #define	_parse_addr		((whstr_ext_func_t)cmn_parse_addr)

// ��ʼ�����磨��Ϊ�ҵĳ���󲿷ֶ���Ҫ���磬������������ﻹ����linuxʱ��У׼���������ʼ�����Ĺ��ܣ�
bool	cmn_init();
// �ս�����
bool	cmn_release();

// �򵥵������ʼ�����ս����
class	cmn_ir_obj
{
public:
	cmn_ir_obj()
	{
		cmn_init();
	}
	~cmn_ir_obj()
	{
		cmn_release();
	}
};

// ���ñ������Լ���/etc/hosts�ļ�����
// �ִ�����Ϊ	����	��ַ
int				cmn_AddETCHostsLine(const char *cszLine);
// ɾ��ָ����host
// ���ΪNULL�����ʾ�������
int				cmn_DelETCHost(const char *cszKey=NULL);
// ͨ��������������URL���õ������Ƶ�ַ
unsigned int	cmn_get_S_addr(const char *__name);
// ͨ����ñ�������������IP��ַУ���
unsigned int	cmn_get_AllMy_S_addr();
// ͨ�������Ƶ�ַ�õ�IP��
const char *	cmn_get_IP(unsigned int __addr, char *szIP=NULL);

// �õ����������е�ַ
// *__num��(I/O)����������ʱΪ__addr��������ɸ��������ʱΪ��������ĸ���
// ����:
// �����ĵ�ַ����(�����ܻ����*__num�ķ���ֵ)
int		cmn_get_host_addrs(const char *__name, unsigned int *__addrs, int *__num);
// ͨ�������Ƶ�ַ�õ�ip����port
bool	cmn_get_ipnport_by_saaddr(const struct sockaddr_in *__addr, char *__ipstr, port_t *__port);
// ͨ�������Ƶ�ַֻ�õ�ip��
const char *	cmn_get_ip_by_saaddr(const struct sockaddr_in *__addr);
// �õ�IP:PORT��ʽ�ĵ�ַ��
const char *	cmn_get_ipnportstr_by_saaddr(const struct sockaddr_in *__addr);
const char *	cmn_get_ipnportstr_by_saaddr(const struct sockaddr_in *__addr, char *szStr);
// ͨ��IP:PORT��ʽ�ĵ�ַ���õ������Ƶ�ַ
bool	cmn_get_saaddr_by_ipnportstr(struct sockaddr_in *__addr, const char *__ipportstr);
// ͨ��ip(��URL)����port�õ������Ƶ�ַ
// ���__ipstrΪ�ջ���Ϊ0�����ִ���ʾ��INADDR_ANY
bool	cmn_get_saaddr_by_ipnport(struct sockaddr_in *__addr, const char *__ipstr, const port_t __port);
bool	cmn_get_saaddr_by_ipnport(struct sockaddr_in *__addr, unsigned int __ip, const port_t __port);
// ����û��port�ĵ�ַ
bool	cmn_get_saaddr_by_ip(struct sockaddr_in *__addr, const char *__ipstr);
//
inline bool	cmn_get_saaddr_by(struct sockaddr_in *__addr, EASYADDR *__ezaddr)
{
	return	cmn_get_saaddr_by_ipnport(__addr, __ezaddr->szAddrStr, __ezaddr->port);
}
// 
void	cmn_get_saaddr_by(struct sockaddr_in *__addr, const whnet4byte_t &__ip, const port_t __port);

// �����Ƿ��������ʹ�����ڱ�ʹ�õĶ˿�(Ĭ����false)
// ��ҪӰ�������bindָ��
void	cmn_set_reuseaddr(bool bSet);
// ��õ�ǰ��reuse״̬
bool	cmn_get_reuseaddr();
// ��ԭ����reuse״̬push������bSet�����ع�ȥ��
bool	cmn_push_reuseaddr_val(bool bSet);
// ��ԭ����reuse״̬pop���������ر����ǵ�
bool	cmn_pop_reuseaddr_val();

// �󶨱���ĳ���˿ڣ����__addr��Ϊ���򷵻ض�Ӧ�ĵ�ַ
bool	cmn_bind_local(SOCKET __sock, port_t __port, struct sockaddr_in *__addr = 0);
// ��ĳ����ַ�Ͷ˿�(__ipΪ������ͬcmn_bind_local)�����__addr��Ϊ���򷵻ض�Ӧ�ĵ�ַ
// ˵�������__ip��__port���ǿգ��򷵻����Ƕ�Ӧ�ĵ�ַ��
//       ���򷵻�ϵͳ��̬�Զ�ָ���ĵ�ַ��������ַANY��Ӧ����0�����Բ����á�
bool	cmn_bind(SOCKET __sock, const char *__ip, port_t __port, struct sockaddr_in *__addr = 0);

// ��ĳ��ָ����Χ�Ƕ�̬��һ�����ö˿�
bool	cmn_randbind_local(SOCKET __sock, port_t __port0, port_t __port1, struct sockaddr_in *__addr = 0);
bool	cmn_randbind(SOCKET __sock, const char *__ip, port_t __port0, port_t __port1, struct sockaddr_in *__addr = 0);

// �Ƚ�������ַ�Ƿ�һ��
// ���غ�memcmp����
int		cmn_addr_cmp(const struct sockaddr_in *__addr1, const struct sockaddr_in *__addr2);
inline bool	operator == (const struct sockaddr_in & __addr1, const struct sockaddr_in & __addr2)
{
	return	cmn_addr_cmp(&__addr1, &__addr2) == 0;
}
inline bool	operator != (const struct sockaddr_in & __addr1, const struct sockaddr_in & __addr2)
{
	return	cmn_addr_cmp(&__addr1, &__addr2) != 0;
}
// ��ַ����
int		cmn_addr_cpy(struct sockaddr_in *__addrdst, const struct sockaddr_in *__addrsrc);

// ͨ�õĵ�ַ����(���__port2��Ϊ0��󶨵�__port1��__port2��Χ�ڵĿ��ö˿�)
SOCKET	cmn_create_socket(int __type, int __protocol, const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr);

// ���ڵ�ַ�ȽϵĹ�������
struct	_whnet_addr_hashfunc
{
	_whnet_addr_hashfunc() {}
	_whnet_addr_hashfunc(const _whnet_addr_hashfunc & hf) {}

	inline unsigned int	operator()(const whnet4byte_t & b4)	const
	{
		return	(b4.c[0]<<24)
				+ (b4.c[1]<<16)
				+ (b4.c[2]<<8)
				+ (b4.c[3])
				;
	}
	inline size_t	operator()(const struct sockaddr_in & addr)	const
	{
		return	operator()(*(whnet4byte_t *)&(addr.sin_addr.s_addr)) + addr.sin_port;
	}
};

// ��ַƥ��
inline bool	cmn_addr_match(const whnet4byte_t &__addr, const whnet4byte_t &__mask, const whnet4byte_t &__addrtochk)
{
	return	(__addr.nl & __mask.nl) == (__addrtochk.nl & __mask.nl);
}
inline bool	cmn_addr_match_quick(const whnet4byte_t &__addrandmask, const whnet4byte_t &__mask, const whnet4byte_t &__addrtochk)
{
	return	__addrandmask.nl == (__addrtochk.nl & __mask.nl);
}
// ��policy�ĵ�ַƥ��
class	cmn_addrmatchlist
{
public:
	// ������Ϊ�������Է��ʵ��б��Ա��ѯ���ӡ�ȵ�
	struct	POLICY_T
	{
		whnet4byte_t	addr;
		whnet4byte_t	mask;
		whnet4byte_t	addrandmask;
		POLICY_T()
		: addr(0), mask(0), addrandmask(0)
		{
		}
		POLICY_T(const whnet4byte_t &_addr, const whnet4byte_t &_mask)
		{
			setaddrmask(_addr, _mask);
		}
		inline void	setaddrmask(const whnet4byte_t &_addr, const whnet4byte_t &_mask)
		{
			addr	= _addr;
			mask	= _mask;
			addrandmask	= addr.nl & mask.nl;
		}
		inline bool operator == (const POLICY_T &_other) const
		{
			return	addr == _other.addr && mask == _other.mask;
		}
	};
	n_whcmn::whvector<POLICY_T>	m_policylist;
public:
	inline void		reserve(size_t nSize)
	{
		m_policylist.reserve(nSize);
	}
	inline size_t	policynum() const
	{
		return	m_policylist.size();
	}
public:
	int		addpolicy(const whnet4byte_t &addr, const whnet4byte_t &mask);
	int		addpolicy(const char *cszAddr, const char *cszMask);
	int		addpolicy(const char *cszAddrAndMask);
	int		delpolicy(const whnet4byte_t &addr, const whnet4byte_t &mask);
	int		delpolicy(const char *cszAddr, const char *cszMask);
	int		delpolicy(const char *cszAddrAndMask);
	int		delallpolicy();
	bool	ismatch(const whnet4byte_t &addrtochk);
	bool	ismatch(const char *cszAddrToChk);
	bool	ismatch(const struct sockaddr_in &addr);
	// ���ļ��ж���policy�б�
	// �����治��clrall�����Գ���Ҫ�ϲ���ʽ����clrall�������������ṩ�Ӷ���ļ���������ʼ����
	int		initfromfile(n_whcmn::whfile *file);
};

// ����cmn_addrmatchlist�ĵ�ַ�������ֹ
class	cmn_addr_allowdeny
{
public:
	cmn_addrmatchlist	m_mlAllow, m_mlDeny;
public:
	bool	isallowed(const whnet4byte_t &addrtochk);
	bool	isallowed(const char *cszAddrToChk);
	bool	isallowed(const struct sockaddr_in &addr);
	// �ļ��ṹ��
	// *ALLOW
	// {
	//    x.x.x.x/x.x.x.x
	//    ...
	// }
	// *DENY
	// {
	//    x.x.x.x/x.x.x.x
	//    ...
	// }
	// �жϹ����ǣ������ALLOW�У���һ�����������������DENY����һ����������ʣ�µı�����
	// ������Զ�ɾ��ԭ��������policy
	int		initfromfile(n_whcmn::whfile *file);
	int		initfromfile(const char *cszFile);
};

// 
// __timeout��λ�Ǻ���
// ����>0�����ж�����0��ʾû�У�SOCKET_ERROR��ʾsocket���� 
// ע�⣺�����ж��Ƿ�������Ӧ��ʹ��cmn_getsizetorecv��������cmn_select_rd(sock, 0)�����߽����ʱ2�����ҡ�
// ע�⣺������cmn_getsizetorecv��ʱ���ܵõ�ȫ�������ݡ���������ͬ������udp�����ͺ�!!!!!!!! (2005-07-13�ٴη���) !!!!!!!!
int		cmn_select_rd(SOCKET __sock, int __timeout = 0);
int		cmn_select_wr(SOCKET __sock, int __timeout = 0);
// is means InvalidSocket
inline int	cmn_select_rd_sleep_on_is(SOCKET __sock, int __timeout)
{
	if( __sock == INVALID_SOCKET )
	{
		n_whcmn::wh_sleep(__timeout);
		return	0;
	}
	return	cmn_select_rd(__sock, __timeout);
}
// ��sock��select_rd�����غ�ϵͳ��select���ƣ��������ݵ�socket�ĸ�����
// ע�����һ��һ��Ҫд0����ʾ��������������!!!!!!
int		cmn_select_multi_rd(int __timeout, ...);
// ��socket�������select
int		cmn_select_rd_array(SOCKET *__sockarray, int __num, int __timeout);
int		cmn_assemble_sock_to_select(SOCKET *__sockarray, int __num, fd_set *__fsr, SOCKET *__sockmax);
// ���ڳ���ϵͳ�涨������socket��select_rd
// ���������ݵ�socket�ĸ���
int		cmn_select_rd_array_big(SOCKET *__sockarray, int __num, int __timeout);
// �Ѻ��������õ�struct timeval��ȥ
struct timeval &	cmn_set_timevalfromms(struct timeval &tv, int nMS);
// ���ڶ�socket��select_rd
class	cmn_selector
{
private:
	fd_set	m_fsr, m_fsrbak;			// ÿ��select��bak�����ݿ�����ȥ����ʡ���������õ�ʱ��
										// ��������socket�Ƚ��ٵ�ʱ��û�����ƣ����ܻ������������Ƚϼ򵥡�
	SOCKET	m_sockmax, m_sockmax1;
	struct timeval				m_tv;
#ifdef	__GNUC__
	struct timeval				m_tvbak;
#endif
										// ��linux��ÿ��select�ᵼ��m_tv�����㣬������Ҫ����ԭ����
	n_whcmn::whvector<SOCKET>	m_vectsock;
protected:
	inline void	restorefsr()
	{
		if( m_vectsock.size()<=8 )
		{
			FD_ZERO( &m_fsr );
			for(size_t i=0;i<m_vectsock.size();i++)
			{
				// m_vectsock[i]��Ӧ�ñ�֤û��INVALID_SOCKET�������addsocket�б�֤��
				FD_SET( m_vectsock[i], &m_fsr );
			}
		}
		else
		{
			memcpy(&m_fsr, &m_fsrbak, sizeof(m_fsr));
		}
	}
public:
	cmn_selector()
	{
		clear();
	}
	void	clear();
	void	settimeout(int __timeout);
	int		addsocket(SOCKET __sock);
	int		removesocket(SOCKET __sock);
	int		select_rd();
	int		select_rd(int __timeout);
	int		select_rd_with_extrasocket(SOCKET s1, ...);
	int		select_rd_with_extrasocketvect(n_whcmn::whvector<SOCKET> &vect);
public:
	inline int	size() const
	{
		return	m_vectsock.size();
	}
};

// ioctl
int		cmn_ioctl(SOCKET __sock, int __cmd, int *__arg);

// ���sock�н�Ҫ��õ����ݴ�С
// ��windows�£�����udp��tcp��˵���������������ݵ��ܺ͡�
// �����Բ��������ֵ�������´�recv����ʵ���ȣ�������Ҫ���¸���recv�ķ���ֵ���£�
// ��linux�£�����udp��˵���������´�recv�����ݳ���
// ����������������ж�__sock�Ƿ���Ч��
int		cmn_getsizetorecv(SOCKET __sock);

// ����������socketѡ��ֵ
int		cmn_setsockopt_int(SOCKET __sock, int __level, int __optname, int __val);
// ����socket������״̬����Ϊ��ͬƽ̨ʵ�ֲ�ͬ�����Ե���д����
int		cmn_setsock_nonblock(SOCKET __sock, bool __nonblock);
// ��������
void	cmn_clear_error();
// �����ɷ��������
bool	cmn_is_nonblock_error();
// ����linger
int		cmn_setsock_linger(SOCKET __sock, unsigned short __onoff, unsigned short __linger);
// ����tcp��keepaliveʱ��
int		cmn_setkeepalive(SOCKET __sock, int __idle=5000, int __interval=1000);

// ��ȡsocket��صĴ����
int		cmn_getsockerror(SOCKET __sock);

// ��ȡ������socketѡ��ֵ
int		cmn_getsockopt_int(SOCKET __sock, int __level, int __optname);

// �ж�һ��socket�Ƿ���Ч
inline bool	cmn_is_validsocket(SOCKET __sock)
{
	return	INVALID_SOCKET != __sock;
}

inline void	cmn_safeclosesocket(SOCKET &__sock)
{
	if( __sock != INVALID_SOCKET )
	{
		closesocket(__sock);
		__sock	= INVALID_SOCKET;
	}
}

// ����Ķ�����Ϊ6���ֽ�
// �ִ�MAC��ַ��������ת��
int		cmn_MACStrtoBin(char *szMAC, unsigned char *szBin);
// ������ת��Ϊ�ִ�MAC��ַ(��":"�ָ�)
char *	cmn_MACBintoStr(unsigned char *szBin, char *szMAC);

// ��IP����ɶ�����(�ҵĶ����ƣ�����Ķ�Ӧ��λ���磺1.2.3.4��Ӧ4,3,2,1)��������whnet4byte_t�õ���ص�������
// ���Ӧ�ú�sin_addr.s_addr�е�����һ�£���ȻҲ�ͺ�whnet4byte_t�е�һ��
// e.g. cmn_IPtoBin(szIP, ipbytes.c);
int		cmn_IPtoBin(const char *szIP, unsigned char *szBin);
// �������Ʊ��IP��
char *	cmn_BintoIP(unsigned char *szBin, char *szIP);

// �ж�socket���Ƿ�������
inline bool	cmn_havedatatorecv(SOCKET __sock)
{
	return	cmn_getsizetorecv(__sock)>0;
}

////////////////////////////////////////////////////////////////////
// ϵͳʱ����غ���
////////////////////////////////////////////////////////////////////
// 2004-07-15 ȫ��ʹ��wh_xxx������

////////////////////////////////////////////////////////////////////
// ������Ϣ
////////////////////////////////////////////////////////////////////
// ��õ�ǰ���̵�id
unsigned int	cmn_getpid();

////////////////////////////////////////////////////////////////////
// ������������ȡ
////////////////////////////////////////////////////////////////////
int		cmn_getlasterror();
#ifdef	WIN32
// ������
#endif
///////////////////////////////////////////////////////////////////
// ��õ�ǰ���̵�ϵͳռ�� 3�������������������ע�ⷵ��ֵ-1��ʱ���ַ��������ǲ���ȫ��
//////////////////////////////////////////////////////////////////
int	    cmn_getstat(char * memory,char * cpu,char * processname);

}		// EOF namespace n_whnet





////////////////////////////////////////////////////////////////////
// ΪDLL֮����Ϣͬ��
////////////////////////////////////////////////////////////////////
void *	WHNET_STATIC_INFO_Out();
extern "C" WH_DLL_API	void	WHNET_STATIC_INFO_In(void *pInfo);

#endif	// EOF __WHNETCMN_H__
