// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcmn.h
// Creator      : Wei Hua (魏华)
// Comment      : 简单的网络功能
//                在windows下需要现实包含下面的库，只要用到和网络相关的功能就要这样做
//                #pragma comment(lib, "wsock32.lib")
//                或者 #pragma comment(lib, "WS2_32.lib") （其实用前面就可以了）
//                2004-03-08 cmn_setsockopt_int(m_sock, SOL_SOCKET, SO_RCVBUF, xxxx)可以设置的最大值，
//                在linux下受到/proc/sys/net/core中rmem_max的限制，相关的还有rmem_default、rmem_max、wmem_default、wmem_max
//                !!!!重要置顶：发现用cmn_getsizetorecv有时候不能得到全部的数据。或者是在同机器上udp会有滞后。!!!!!!!! (2005-07-13再次发现) !!!!!!!!
//                !!!!而且，在使用tcp时，cmn_getsizetorecv获得的数据尺寸不一定能被全部得到，有可能会小于期望得到的大小
//                addr.sin_addr.s_addr 这个常用，所以拷贝在这里 :)
// CreationDate : 2003-05-14
// ChangeLog    : 2003-08-06
//                原来用cmn_select_rd(sock,0)判断缓冲中是否有数据，现在直接用cmn_getsizetorecv>0判断，要快两倍。
//                包装的函数是bool cmn_havedatatorecv。以后所有地方都用这个。
//                如果连续的判断，那么就直接使用cmn_getsizetorecv然后根据recv的长度逐次减到<=0，作为结束。
//                2003-08-15
//                发现用cmn_getsizetorecv有时候不能得到全部的数据。或者是在同机器上udp会有滞后。!!!!!!!!
//                2004-02-11
//                在Linux下，FD_SET的参数如果是-1，就会发生内存访问错误，所以把所有的FD_SET的地方都判断了一下
//                2004-06-30
//                增加了计算地址匹配的快算判断函数cmn_addr_match_quick
//                2005-07-01
//                增加了cmn_DelETCHost函数用于删除已经记录的host地址，从而可以重新刷新所需的URL。
//                2005-07-13
//                再次发现cmn_getsizetorecv导致数据之后的问题。申请置顶。
//                2007-05-29
//                增加了对reuseaddr设置的几个操作

#ifndef	__WHNETCMN_H__
#define	__WHNETCMN_H__

#include <WHCMN/inc/wh_platform.h>

#if defined( WIN32 )					// Windows下的头		{
#define	MSG_NOSIGNAL	0
#include <wsipx.h>
#include <mstcpip.h>
// windows下没有这些定义
#ifndef	socklen_t
#define	socklen_t		int
#endif
const int SHUT_RD		= 0;
const int SHUT_WR		= 1;
const int SHUT_RDWR		= 2;
#endif									// EOF Windows下的头	}

#if defined( __GNUC__ )					// Linux下的头			{
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

// linux下没有这些定义
typedef	int				 SOCKET;		// 注意这个不能放到namespace n_whnet里面
#define	INVALID_SOCKET	-1
#define	SOCKET_ERROR	-1
#define	closesocket(fd)	close(fd)
#endif									// EOF Linux下的头		}

#include <WHCMN/inc/whtime.h>
#include <WHCMN/inc/whvector.h>
#include <WHCMN/inc/whfile.h>

// 最多可以同时进行select的socket数量
#define	WHNET_MAX_SOCKTOSELECT			FD_SETSIZE
// 网络地址字串的最大长度(包括域名的或者是IP的)
#define	WHNET_MAXADDRSTRLEN				256
// IP串的最大长度
#define	WHNET_MAXIPSTRLEN				32

namespace n_whnet
{

typedef	unsigned short		port_t;
//typedef unsigned int		u_int;
typedef unsigned char		u_char;

// 2004-07-08为了扩展，增加了port2，对应字串格式为ip:port[,port2]
struct	EASYADDR						// 简单的地址结构ipstr+port
{
	char	szAddrStr[WHNET_MAXADDRSTRLEN];
	port_t	port;
	port_t	port2;
	// 如果port和port2都非0，则在port~port2范围内绑定

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

// 简单地址
// nl等同于addr.sin_addr.s_addr
union	whnet4byte_t
{
	unsigned int	nl;
	unsigned char	c[4];	// 0~3为和IP里面对应的顺序
	void	reverse()		// 字节颠倒顺序
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

// 把形如“ip:port[,port2]”的地址串解析到EASYADDR结构中
// nAct可以是WHSTR_ACT_XXX，定义见whstring.h
void	cmn_parse_addr0(const char *szAddrStr, EASYADDR *pAddr);
void	cmn_parse_addr(const char *szAddrStr, EASYADDR *pAddr, int nAct);
// 增加自己的Host
void	cmn_parse_ETCHost(const char *szLine, void *pNotUse, int nAct);
// 这个是给whdataini应用的
void	cmn_glbsetaddr(void *var, const char *cszVal);
// 在使用时可以使用下面的定义，放在WH_LINE_PARSE_INFO_T结构中
// #define	_parse_addr		((whstr_ext_func_t)cmn_parse_addr)

// 初始化网络（因为我的程序大部分都需要网络，所以这个函数里还包括linux时间校准；随机数初始化；的功能）
bool	cmn_init();
// 终结网络
bool	cmn_release();

// 简单的网络初始化和终结对象
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

// 设置本程序自己的/etc/hosts文件内容
// 字串内容为	名字	地址
int				cmn_AddETCHostsLine(const char *cszLine);
// 删除指定的host
// 如果为NULL，则表示清除所有
int				cmn_DelETCHost(const char *cszKey=NULL);
// 通过机器名（包括URL）得到二进制地址
unsigned int	cmn_get_S_addr(const char *__name);
// 通过获得本机所有网卡的IP地址校验核
unsigned int	cmn_get_AllMy_S_addr();
// 通过二进制地址得到IP串
const char *	cmn_get_IP(unsigned int __addr, char *szIP=NULL);

// 得到主机的所有地址
// *__num是(I/O)变量，输入时为__addr的最大容纳个数，输出时为真正填入的个数
// 返回:
// 真正的地址个数(它可能会大于*__num的返回值)
int		cmn_get_host_addrs(const char *__name, unsigned int *__addrs, int *__num);
// 通过二进制地址得到ip串和port
bool	cmn_get_ipnport_by_saaddr(const struct sockaddr_in *__addr, char *__ipstr, port_t *__port);
// 通过二进制地址只得到ip串
const char *	cmn_get_ip_by_saaddr(const struct sockaddr_in *__addr);
// 得到IP:PORT形式的地址串
const char *	cmn_get_ipnportstr_by_saaddr(const struct sockaddr_in *__addr);
const char *	cmn_get_ipnportstr_by_saaddr(const struct sockaddr_in *__addr, char *szStr);
// 通过IP:PORT形式的地址串得到二进制地址
bool	cmn_get_saaddr_by_ipnportstr(struct sockaddr_in *__addr, const char *__ipportstr);
// 通过ip(或URL)串和port得到二进制地址
// 如果__ipstr为空或者为0长度字串表示是INADDR_ANY
bool	cmn_get_saaddr_by_ipnport(struct sockaddr_in *__addr, const char *__ipstr, const port_t __port);
bool	cmn_get_saaddr_by_ipnport(struct sockaddr_in *__addr, unsigned int __ip, const port_t __port);
// 创建没有port的地址
bool	cmn_get_saaddr_by_ip(struct sockaddr_in *__addr, const char *__ipstr);
//
inline bool	cmn_get_saaddr_by(struct sockaddr_in *__addr, EASYADDR *__ezaddr)
{
	return	cmn_get_saaddr_by_ipnport(__addr, __ezaddr->szAddrStr, __ezaddr->port);
}
// 
void	cmn_get_saaddr_by(struct sockaddr_in *__addr, const whnet4byte_t &__ip, const port_t __port);

// 设置是否可以重新使用正在被使用的端口(默认是false)
// 主要影响下面的bind指令
void	cmn_set_reuseaddr(bool bSet);
// 获得当前的reuse状态
bool	cmn_get_reuseaddr();
// 把原来的reuse状态push，设置bSet，返回过去的
bool	cmn_push_reuseaddr_val(bool bSet);
// 把原来的reuse状态pop回来，返回被覆盖的
bool	cmn_pop_reuseaddr_val();

// 绑定本机某个端口，如果__addr不为空则返回对应的地址
bool	cmn_bind_local(SOCKET __sock, port_t __port, struct sockaddr_in *__addr = 0);
// 绑定某个地址和端口(__ip为空作用同cmn_bind_local)，如果__addr不为空则返回对应的地址
// 说明：如果__ip和__port都非空，则返回它们对应的地址。
//       否则返回系统动态自动指定的地址。不过地址ANY对应的是0，所以不可用。
bool	cmn_bind(SOCKET __sock, const char *__ip, port_t __port, struct sockaddr_in *__addr = 0);

// 在某个指定范围那动态绑定一个可用端口
bool	cmn_randbind_local(SOCKET __sock, port_t __port0, port_t __port1, struct sockaddr_in *__addr = 0);
bool	cmn_randbind(SOCKET __sock, const char *__ip, port_t __port0, port_t __port1, struct sockaddr_in *__addr = 0);

// 比较两个地址是否一致
// 返回和memcmp类似
int		cmn_addr_cmp(const struct sockaddr_in *__addr1, const struct sockaddr_in *__addr2);
inline bool	operator == (const struct sockaddr_in & __addr1, const struct sockaddr_in & __addr2)
{
	return	cmn_addr_cmp(&__addr1, &__addr2) == 0;
}
inline bool	operator != (const struct sockaddr_in & __addr1, const struct sockaddr_in & __addr2)
{
	return	cmn_addr_cmp(&__addr1, &__addr2) != 0;
}
// 地址拷贝
int		cmn_addr_cpy(struct sockaddr_in *__addrdst, const struct sockaddr_in *__addrsrc);

// 通用的地址创建(如果__port2不为0则绑定到__port1到__port2范围内的可用端口)
SOCKET	cmn_create_socket(int __type, int __protocol, const char *__ip, port_t __port1, port_t __port2, struct sockaddr_in *__addr);

// 用于地址比较的哈西函数
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

// 地址匹配
inline bool	cmn_addr_match(const whnet4byte_t &__addr, const whnet4byte_t &__mask, const whnet4byte_t &__addrtochk)
{
	return	(__addr.nl & __mask.nl) == (__addrtochk.nl & __mask.nl);
}
inline bool	cmn_addr_match_quick(const whnet4byte_t &__addrandmask, const whnet4byte_t &__mask, const whnet4byte_t &__addrtochk)
{
	return	__addrandmask.nl == (__addrtochk.nl & __mask.nl);
}
// 多policy的地址匹配
class	cmn_addrmatchlist
{
public:
	// 公开是为了外界可以访问到列表以便查询或打印等等
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
	// 从文件中读出policy列表
	// 这里面不会clrall，所以常需要上层显式调用clrall。（这样可以提供从多个文件中连续初始化）
	int		initfromfile(n_whcmn::whfile *file);
};

// 基于cmn_addrmatchlist的地址允许和阻止
class	cmn_addr_allowdeny
{
public:
	cmn_addrmatchlist	m_mlAllow, m_mlDeny;
public:
	bool	isallowed(const whnet4byte_t &addrtochk);
	bool	isallowed(const char *cszAddrToChk);
	bool	isallowed(const struct sockaddr_in &addr);
	// 文件结构：
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
	// 判断规则是，如果在ALLOW中，则一定被允许。否则，如果在DENY中则一定不被允许。剩下的被允许。
	// 里面会自动删除原来的所有policy
	int		initfromfile(n_whcmn::whfile *file);
	int		initfromfile(const char *cszFile);
};

// 
// __timeout单位是毫秒
// 返回>0代表有东西，0表示没有，SOCKET_ERROR表示socket错误 
// 注意：立即判断是否有数据应该使用cmn_getsizetorecv，而不是cmn_select_rd(sock, 0)。后者将多耗时2倍左右。
// 注意：发现用cmn_getsizetorecv有时候不能得到全部的数据。或者是在同机器上udp会有滞后。!!!!!!!! (2005-07-13再次发现) !!!!!!!!
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
// 多sock的select_rd（返回和系统的select类似，即有内容的socket的个数）
// 注意最后一个一定要写0，表示结束，否则会出错!!!!!!
int		cmn_select_multi_rd(int __timeout, ...);
// 对socket数组进行select
int		cmn_select_rd_array(SOCKET *__sockarray, int __num, int __timeout);
int		cmn_assemble_sock_to_select(SOCKET *__sockarray, int __num, fd_set *__fsr, SOCKET *__sockmax);
// 对于超出系统规定个数的socket的select_rd
// 返回有内容的socket的个数
int		cmn_select_rd_array_big(SOCKET *__sockarray, int __num, int __timeout);
// 把毫秒数设置到struct timeval中去
struct timeval &	cmn_set_timevalfromms(struct timeval &tv, int nMS);
// 用于多socket的select_rd
class	cmn_selector
{
private:
	fd_set	m_fsr, m_fsrbak;			// 每次select把bak的内容拷贝回去，旧省了重新设置的时间
										// 这样做在socket比较少的时候并没有优势，可能还会慢，不过比较简单。
	SOCKET	m_sockmax, m_sockmax1;
	struct timeval				m_tv;
#ifdef	__GNUC__
	struct timeval				m_tvbak;
#endif
										// 在linux下每次select会导致m_tv被清零，所以需要保存原来的
	n_whcmn::whvector<SOCKET>	m_vectsock;
protected:
	inline void	restorefsr()
	{
		if( m_vectsock.size()<=8 )
		{
			FD_ZERO( &m_fsr );
			for(size_t i=0;i<m_vectsock.size();i++)
			{
				// m_vectsock[i]中应该保证没有INVALID_SOCKET，这个在addsocket中保证了
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

// 获得sock中将要获得的数据大小
// 在windows下，对于udp和tcp来说，里面是所有数据的总和。
// （所以不能用这个值来当做下次recv的真实长度，还是需要重新根据recv的返回值重新）
// 在linux下，对于udp来说，里面是下次recv的数据长度
// 这个函数不能用来判断__sock是否还有效。
int		cmn_getsizetorecv(SOCKET __sock);

// 设置整数的socket选项值
int		cmn_setsockopt_int(SOCKET __sock, int __level, int __optname, int __val);
// 设置socket的阻塞状态（因为不同平台实现不同，所以单另写出）
int		cmn_setsock_nonblock(SOCKET __sock, bool __nonblock);
// 清除错误号
void	cmn_clear_error();
// 错误由非阻塞造成
bool	cmn_is_nonblock_error();
// 设置linger
int		cmn_setsock_linger(SOCKET __sock, unsigned short __onoff, unsigned short __linger);
// 设置tcp的keepalive时间
int		cmn_setkeepalive(SOCKET __sock, int __idle=5000, int __interval=1000);

// 获取socket相关的错误号
int		cmn_getsockerror(SOCKET __sock);

// 获取整数的socket选项值
int		cmn_getsockopt_int(SOCKET __sock, int __level, int __optname);

// 判断一个socket是否有效
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

// 下面的二进制为6个字节
// 字串MAC地址到二进制转换
int		cmn_MACStrtoBin(char *szMAC, unsigned char *szBin);
// 二进制转换为字串MAC地址(按":"分隔)
char *	cmn_MACBintoStr(unsigned char *szBin, char *szMAC);

// 将IP串变成二进制(我的二进制，后面的对应低位。如：1.2.3.4对应4,3,2,1)（可以用whnet4byte_t得到相关的整数）
// 这个应该和sin_addr.s_addr中的数据一致，当然也就和whnet4byte_t中的一致
// e.g. cmn_IPtoBin(szIP, ipbytes.c);
int		cmn_IPtoBin(const char *szIP, unsigned char *szBin);
// 将二进制变成IP串
char *	cmn_BintoIP(unsigned char *szBin, char *szIP);

// 判断socket中是否有数据
inline bool	cmn_havedatatorecv(SOCKET __sock)
{
	return	cmn_getsizetorecv(__sock)>0;
}

////////////////////////////////////////////////////////////////////
// 系统时间相关函数
////////////////////////////////////////////////////////////////////
// 2004-07-15 全部使用wh_xxx函数了

////////////////////////////////////////////////////////////////////
// 进程信息
////////////////////////////////////////////////////////////////////
// 获得当前进程的id
unsigned int	cmn_getpid();

////////////////////////////////////////////////////////////////////
// 错误定义与错误获取
////////////////////////////////////////////////////////////////////
int		cmn_getlasterror();
#ifdef	WIN32
// 错误定义
#endif
///////////////////////////////////////////////////////////////////
// 获得当前进程的系统占用 3个参数都是输出参数，注意返回值-1的时候字符串可能是不安全的
//////////////////////////////////////////////////////////////////
int	    cmn_getstat(char * memory,char * cpu,char * processname);

}		// EOF namespace n_whnet





////////////////////////////////////////////////////////////////////
// 为DLL之间信息同步
////////////////////////////////////////////////////////////////////
void *	WHNET_STATIC_INFO_Out();
extern "C" WH_DLL_API	void	WHNET_STATIC_INFO_In(void *pInfo);

#endif	// EOF __WHNETCMN_H__
