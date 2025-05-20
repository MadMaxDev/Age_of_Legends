// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetlocalhiccup.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于UDP的局域网内的应用程序授权运行
//              : hiccup是打嗝的意思，只是随便想到这个词就这么用了:P
// CreationDate : 2006-06-29
// ChangeLog    : 

#ifndef	__WHNETLOCALHICCUP_H__
#define	__WHNETLOCALHICCUP_H__

#include "whnetcmn.h"

namespace n_whnet
{

class	whnetlocalhiccup_client
{
public:
	static whnetlocalhiccup_client *	Create();
public:
	// 初始化参数
	struct	INFO_T
	{
		char	szDst[256];				// 目标地址数组（目标地址，用逗号分隔）
		char	szPass[64];				// 发送的校验密码
		int		(*pCallBack)(const char *pszCmd, void *pParam);
										// 上层给的回调函数，在适当的时候会通知上层（比如校验成功、失败；消息提示等等）
										// 参数pszCmd是指令串，目前的返回定义有：
										// Succeed		连接并身份验证成功（只有收到这个才能继续工作）
										// Fail			向所有的地址连接失败
										// Msg <msgstr>	打印出一条提示信息
										// 参数pParam是上层传的附加参数，由上层定义并解释使用。
		void	*pCBParam;				// 就是pCallBack需要使用的pParam参数
		INFO_T();
	};
public:
	virtual	~whnetlocalhiccup_client()	{};
	virtual	int	Init(INFO_T *pInfo)		= 0;
	virtual	int	Release()				= 0;
	virtual	int	Tick()					= 0;
	virtual	SOCKET GetSocket() const	= 0;

	// 单起一个线程做这个事情
	static int	DoItInThread(INFO_T *pInfo);
	static int	StopTheThread();
};

class	whnetlocalhiccup_server
{
public:
	static whnetlocalhiccup_server *	Create();
public:
	// 初始化参数
	struct	INFO_T
	{
		char	szBindAddr[WHNET_MAXADDRSTRLEN];
		int		nMaxConnection;			// 最多的连接数量
		int		nPassTimeOut;			// 等待用户发来密码的超时
		char	szPass[64];				// 需要收到的校验密码
		INFO_T();
	};
public:
	virtual	~whnetlocalhiccup_server()	{};
	virtual	int	Init(INFO_T *pInfo)		= 0;
	virtual	int	Release()				= 0;
	virtual	int	Tick()					= 0;
	virtual	SOCKET GetSocket() const	= 0;
};

}		// EOF namespace n_whnet

#endif
