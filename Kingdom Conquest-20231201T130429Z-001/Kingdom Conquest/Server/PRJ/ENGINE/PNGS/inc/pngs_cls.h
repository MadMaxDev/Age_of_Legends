// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls.h
// Creator      : Wei Hua (魏华)
// Comment      : PNGS的CLS模块
//                PNGS是Pixel Network Game Structure的缩写
//                CLS是Connection Load Server的缩写，是连接分配功能中的负载服务器
// CreationDate : 2005-07-22
// Change LOG   :

#ifndef	__PNGS_CLS_H__
#define	__PNGS_CLS_H__

#include <WHCMN/inc/wh_crypt_interface.h>
#include <WHCMN/inc/whvector.h>
#include <WHNET/inc/whnetcmn.h>

namespace n_pngs
{

class	CLS2
{
public:
	static CLS2 *	Create();
protected:
	virtual	~CLS2()	{}
public:
	// 自己销毁自己
	virtual	void	SelfDestroy()									= 0;
	// 根据配置文件初始化
	virtual	int		Init(const char *cszCFG)						= 0;
	// 重新载入配置
	virtual	int		Init_CFG_Reload(const char *cszCFG)				= 0;
	// 终结
	virtual	int		Release()										= 0;
	// 设置加密工厂(如果不设置的话内部应该使用默认的加解密工厂，即：不工作的那种)
	virtual	int		SetICryptFactory(n_whcmn::ICryptFactory *pFactory)
																	= 0;
	// 获取sockets以供上层select
	virtual size_t	GetSockets(n_whcmn::whvector<SOCKET> &vect)		= 0;
	// 工作一次
	virtual	int		Tick()											= 0;
	enum
	{
		STATUS_ERR_NETWORK				= -1,						// 工作中网络出错
		STATUS_NOTHING					= 0,
		STATUS_WORKING					= 1,						// 连接成功，工作中
	};
	virtual	int		GetStatus() const								= 0;
	// 告诉上层程序应该停止了
	virtual	bool	ShouldStop() const								= 0;
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CLS_H__
