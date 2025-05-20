// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : lHttpGet.h
// Creator      : 吕宝虹
// Comment      : 简单的http GET请求的封装,可以从资源的某个点传输(服务器必须支持断点续传)
// CreationDate : 2006.07-28
// ChangeLOG    : 2006.11.23 修正几处出现错误以后状态没有转换到nothing和没有关闭套接字的bug, 增加tick中对空闲状态的检测
//				  2007.01.16 改变回调函数参数。增加另一个void *，在把httpget放在一个类内时，可以方便传递外面那个类的对象的指针
//								get函数增加一个参数。设置回调用接收到的void *

#ifndef		LANE_LHTTPGET_H
#define		LANE_LHTTPGET_H

#include <string>
#include "../inc/lSocket.h"

namespace n_whnet
{
	
// 需要使用者自己注册的回调函数,当接受了规定的尺寸的数据以后就自动调用这个函数,
// 第一个参数是接受到的数据的缓冲,第二个是接受到的尺寸,第三个是真个传输过程是否完毕.
typedef		int (* pfn_onget)	( const char * buf, size_t buflen, bool over, void * expptr );

class	httpget
{
public:
	
	// 当前httpget对象所处的状态,可以通过调用getcurstate()获取当前的状态
	enum	state_t
	{
		nothing,		// 什么也没做
		connecting,		// 正在进行连接
		prochead,		// 处理收到的http头
		procdata,		// 处理接受到的数据
		memerr = -1		// 内存不足
	};

	// tick()函数和get()函数可能返回的错误，
	enum	err_t
	{
		complete = 1,				// get请求完成，正确结果
		noerr	 = 0,				// 没有错误发生，get请求仍在处理
		nomemerr = -1,				// 内存不足，错误
		connerr  = -2,				// 连接错误，错误
		sockerr	 = -3,				// socket出现错误，错误
		httprequesterr	 = -4,		// http请求错误，错误
		httpservererr   = -5,		// http服务器错误，错误
		httpunknownerr = -6,		// http未知错误，错误
		unknownerr = -1024,			// 未知错误，错误
	};

	// http协议的返回值，当get()或者tick()返回错误后，可以通过gethttpret()确定http的返回值
	enum	httpret
	{
		// 成功
		http_OK							= 200,	// OK
		http_create						= 201,	//
		http_accepted					= 202,	//
		http_noconent					= 203,	//

		// 请求错误
		http_badrequest					= 400,	//
		http_unauthorized				= 401,	//
		http_forbidden					= 403,	// 
		http_notfound					= 404,	// 

		// 服务端错误
		http_internal_server_error		= 500,
		http_not_implemented			= 501,	//
		http_bad_gateway				= 502,	//
		http_service_unavailable		= 503,	// 
	};

	
	// 构造函数,并不进行连接和请求。
	// 参数:主机域名,如果没有就设为NULL。
	//		主机的ip，必须填正确。
	//		回调函数的指针。
	//		每次产生回调要求的缓冲尺寸，即当接受到callbacksize尺寸的数据以后会调用pcallbackfn指向的函数
	// 注意：产生回调时，传递的接受到的数据的尺寸可能小于callbacksize，一般只会在第一次回调和最后一次回调时才产生。
	httpget( const char * host, const char * ip, pfn_onget pcallbackfn, size_t callbacksize, unsigned short int port = 80 );

	// 析构函数。释放资源，断开连接
	~httpget();

	// 连接主机，并发送一个get请求，resource为请求的文件资源，startpos为请求的资源的开始位置（主机必须支持断点续传） timeout为connect连接超时件时间
	int	get( const char * resource, size_t startpos, void * exp,int timeout);

	// 获取本次get请求，一共会传送的数据大小
	unsigned int	gettotalsize()
	{
		return m_uTotalTransSize;	
	}
	
	// 获取已经传输的数据的大小
	unsigned int	gettransedsize()
	{
		return m_uGetedSize;	
	}

	// 停止传输
	int	stop();


	// 获取http协议的返回值，类型为httpret
	int	gethttpret()
	{
		return m_uHttpRet;	
	}

	// 获取当前的状态，返回类型为state_t
	state_t	getcurstate()
	{
		return	m_stat;
	}

	// tick函数，必须在外面的tick里调用。返回值为err_t类型。
	// 如果返回complete，说明get请求的数据传输完毕，返回noerr，说明传输仍在继续，没有错误发生。
	// 其他返回值则说明产生错误，应该终止调用。
	int tick();

	// 释放资源，断开连接
	void release();

	bool free()
	{
		return	(nothing == m_stat);
	}

private:
	// 基本信息
	std::string				m_sHost;
	std::string				m_sIP;
	unsigned short int		m_uPort;
	std::string				m_sRequest;
	size_t					m_uCurPos;
	pfn_onget				m_pCallbackFun;
	size_t					m_uCallbackSize;


	// 通信使用
	size_t					m_uTotalTransSize;
	lane::Socket			m_TcpSock;
	//unsigned int			m_uHostIP;
	lane::Select			m_select;

	char					* m_sBuf;
	//char			* m_sBufWrPtr;
	//char			* m_sBufRdPtr;
	int						m_uRecvLenTmp; // 每次收到的数据包的大小,临时变量
	size_t					m_uGetedSize;		

	unsigned int			m_uHttpRet;

	void					* m_pExp;
//////////////////////////////////////////////////////////////////////////

	state_t			m_stat;

	typedef	int (httpget:: *pfn_proc)();

	pfn_proc	m_pfProc;
	//	pfn_proc	m_pfProcData;

	int prochttphead();

	int	prochttpheadline( const char * line, unsigned int charnum );

	int checkhttperr();

	int procgetdata();

};	// end class

};	// end namespace n_whnet

#endif		// LANE_LHTTPGET_H
