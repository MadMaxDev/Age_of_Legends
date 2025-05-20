// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_client.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的Client模块(这个Client是和具体逻辑无关的，只实现连接和基本通讯)
//                PNGS是Pixel Network Game Structure的缩写
// CreationDate : 2005-08-25
// Change LOG   : 2006-11-23 增加底层的插件功能，为了将来集成内部和外来的聊天或者其他类似功能做准备。
//              : 2007-06-18 增加了获得网络延时的功能

#ifndef	__PNGS_CLIENT_H__
#define	__PNGS_CLIENT_H__

#include <WHCMN/inc/wh_crypt_interface.h>
#include <WHCMN/inc/whvector.h>
#include <WHNET/inc/whnetcmn.h>
#include "pngs_cmn.h"
#include "pngs_packet.h"

namespace n_pngs
{

class	PNGSClient
{
public:
	// 初始化DLL池
	static void		DLLPool_Init();
	// 终结DLL池
	static void		DLLPool_Release();
	// 创建客户端对象
	static PNGSClient *		Create();
protected:
	virtual	~PNGSClient()	{}
public:
	enum
	{
		CONTROL_OUT_CONNECT_RST			= 1,						// 连接服务器的结果
		CONTROL_OUT_STATUS				= 2,						// 状态改变通知
																	// 结构CONTROL_T，nParam[0]的值就是Status
																	// status的定义见下面的（玩家自身的信息）STATUS_XXX
		CONTROL_OUT_DROP				= 3,						// 连接过程中连接中断
																	// 结构CONTROL_T，nParam[0]的值就是中断理由
		CONTROL_OUT_QUEUEINFO			= 4,						// 在CAAFS中等待队列中的信息
		CONTROL_OUT_SERVICE_INTERMIT	= 5,						// 服务器临时中断，请等待恢复
																	// 结构CONTROL_T，nParam[0]的值就是那种服务中断了（暂时没有定义）
		CONTROL_OUT_NETWORK_INTERMIT	= 6,						// 网络临时中断，请等待恢复
	};
	// 玩家自身的信息
	enum
	{
		STATUS_NOTHING					= 0,						// 没有状态（断线之后就是这样的状态）
		STATUS_CONNECTINGCAAFS			= 1,						// 正在连接CAAFS中
		STATUS_WAITINGINCAAFS			= 2,						// 正在CAAFS中等待连接机会
		STATUS_CONNECTINGCLS			= 3,						// 正在连接CLS中
		STATUS_WORKING					= 4,						// 连线成功，工作中
		STATUS_CONNECTFAIL				= 5,						// 连接失败
		STATUS_DROPPED					= 6,						// 工作中和CLS断线
	};
	// 错误信息
	enum
	{
		ERR_OK							= 0,						// 没有错误
		// 连接错误
		ERR_CONNECT_LASTCONNECTNOTOVER	= -1,						// 上次的连接还没有断
		ERR_CONNECT_CNL2_CONNECTERROR	= -2,						// CNL2的Connect函数返回错误
	};
	#pragma pack(push, old_pack_num, 1)
	struct	CONTROL_T
	{
		short			nCmd;										// 参见前面的CONTROL_OUT_XXX
		int				nParam[1];
	};
	struct	CONTROL_OUT_CONNECT_RST_T	
	{
		enum
		{
			RST_OK						= 0,						// 连接成功
			RST_NET_ERR_CAAFS			= -1,						// 网络错误
			RST_REFUSED_BY_CAAFS		= -2,						// 连接被服务器拒绝
			RST_NET_ERR_DROP_CAAFS		= -3,						// 和CAAFS连接中断
			RST_CAAFS_REFUSEALL			= -4,						// CAAFS拒绝所有的连接
			RST_NET_ERR_CLS				= -11,						// 连接CLS网络错误
			RST_REFUSED_BY_CLS			= -12,						// 连接CLS被服务器拒绝
			RST_NET_ERR_DROP_CLS		= -13,						// 和CLS连接中断
			RST_BAD_TYPE_SERVER			= -21,						// 连接的server不是自己希望连接的
			RST_BAD_VER					= -22,						// 自己和服务器版本不匹配
		};
		short			nCmd;
		short			nRst;										// 连接结果
		int				nClientID;									// 玩家在服务器中的ID
		struct sockaddr_in		CLSAddr;							// 连接上的CLS的地址
	};
	struct	CONTROL_OUT_QUEUEINFO_T
	{
		short			nCmd;
		unsigned char	nVIPChannel;								// 死在的VIP通道，如果非0，则在客户端提示出来
		unsigned short	nNumToWait;									// 前面还有多少人在排队
	};
	#pragma pack(pop, old_pack_num)
public:
	// 自己销毁自己
	virtual	void	SelfDestroy()									= 0;

	// 根据配置文件初始化
	virtual	int		Init(const char *cszCFG)						= 0;

	// 终结
	virtual	int		Release()										= 0;

	// 设置加密工厂(上层注意保证在CAAFS2::SelfDestroy之后才能把pICryptFactory终结掉)
	virtual	int		SetICryptFactory(n_whcmn::ICryptFactory *pFactory)
																	= 0;
	// 获得对应的socket给上层select
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect)		= 0;

	// 工作一次
	virtual	int		TickRecv()										= 0;
	virtual	int		TickSend()										= 0;

	// 设置客户端版本
	virtual	void	SetVer(const char *szVer)						= 0;

	// 获得客户端版本
	virtual	const char *	GetVer()								= 0;

	// 连接服务器
	// 返回前面定义的ERR_XXX
	virtual	int		Connect(const char *cszAddr, unsigned char nNetworkType = NETWORKTYPE_UNKNOWN)
																	= 0;

	// 断开连接
	virtual	int		Disconnect()									= 0;

	// 控制结果输出(如：连接结果，断线等等)
	virtual	int		ControlOut(CONTROL_T **ppCmd, size_t *pnSize)	= 0;

	// 发送指令
	virtual	int		Send(unsigned char nSvrIdx, unsigned char nChannel, const void *pCmd, size_t nSize)
																	= 0;
	// 发送指令给CAAFS
	virtual	int		SendCAAFS(const void *pCmd, size_t nSize)		= 0;

	// 收取指令
	virtual	int		Recv(unsigned char *pnChannel, void **ppCmd, size_t *pnSize)
																	= 0;
	// 获得最后的错误码
	virtual	int		GetLastError() const							= 0;

	// 清除最后的错误码
	virtual	void	SetLastError(int nErrCode=ERR_OK)				= 0;

	// 设置文件管理器（这个必须在Init之前调用）
	virtual void	SetFileMan(n_whcmn::whfileman *pFileMan)		= 0;
	// 插件相关
	// 获得PlugIn指针(插件是通过配置文件载入的)
	virtual	ILogicBase *	GetPlugIn(const char *cszPlugInName, int nVer)
																	= 0;
	// 获得网络ping值（毫秒）
	virtual int		GetPing() const									= 0;

	// 当使用线程可能会阻塞的时候调用这个开启线程，但是注意一定要在真正调用recv和send之前停止线程，否则返回是错误的
	virtual int		StartThread()									= 0;
	virtual int		StopThread()									= 0;
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CLIENT_H__
