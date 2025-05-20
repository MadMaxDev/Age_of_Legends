// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpProxy.h
// Creator      : Wei Hua (魏华)
// Comment      : UDP端口转发对象
//              : 将本地收到的包都转发到指定位置，一个对象只绑定监听一个端口进行转发，可以通过console或者UDP控制端口进行丢包、乱序和延迟设置
//              : 一个没有出现过的地址发送到监听端口就会创建一个FORWARD对象
// CreationDate : 2006-12-19
// ChangeLog    : 

#ifndef	__WHNETUDPPROXY_H__
#define	__WHNETUDPPROXY_H__

#include "./whnetcmn.h"

namespace n_whnet
{

// 接口定义
class	whnetudpProxy
{
public:
	static whnetudpProxy *	Create();
protected:
	virtual	~whnetudpProxy()										{};
public:
	struct	INFO_T
	{
		char		szLocalAddr[WHNET_MAXADDRSTRLEN];				// 本机监听地址
		char		szDestAddr[WHNET_MAXADDRSTRLEN];				// FORWARD目标地址
		char		szCtrlAddr[WHNET_MAXADDRSTRLEN];				// 用于接收控制包的地址
		int			nMaxFORWARD;									// 最多可以支持FORWARD来源的个数
		int			nTQChunkSize;									// 时间队列的分配块大小
		// 这之后的应该都可以临时调整（通过ResetInfo）
		int			nPacketSize;									// 单个包的最大长度
		int			nDropTimeOut;									// 如果一个FORWARD用户来源在指定时间内没有来包且没有收报，则这个FORWARD将失效（毫秒）
		int			nUpLostRate;									// 上行丢包率（即用户发来给目标的丢包率）（百分率，0~99即，10表示10%）
		int			nDownLostRate;									// 下行丢包率（即目标发回给用户的丢包率）（百分率，0~99即，10表示10%）
		int			nUpDelay0, nUpDelay1;							// 上行延迟范围（毫秒）
		int			nDownDelay0, nDownDelay1;						// 下行延迟范围（毫秒）

		INFO_T()
			: nMaxFORWARD(64)
			, nTQChunkSize(512)
			, nPacketSize(4096)
			, nDropTimeOut(20000)
			, nUpLostRate(0)
			, nDownLostRate(0)
			, nUpDelay0(0), nUpDelay1(0)
			, nDownDelay0(0), nDownDelay1(0)
		{
			// 这两个地址外界必须设置的
			szLocalAddr[0]	= 0;
			szDestAddr[0]	= 0;
			szCtrlAddr[0]	= 0;
		};
	};
public:
	// 自我销毁
	virtual	void	SelfDestroy()									= 0;
	// 初始化
	virtual	int		Init(const INFO_T *pInfo)						= 0;
	// 重新设置某些参数（如超时、丢包率、延迟）
	virtual	int		ResetInfo(const INFO_T *pInfo)					= 0;
	// 终结
	virtual	int		Release()										= 0;
	// 一次工作（nMS是给内部select的时间）
	virtual	int		Tick(int nMS)									= 0;
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETUDPPROXY_H__
