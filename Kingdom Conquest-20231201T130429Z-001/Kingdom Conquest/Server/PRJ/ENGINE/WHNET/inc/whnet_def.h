// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_pnl
// File: whnet_def.h
// Creator: Wei Hua (魏华)
// Comment: whnet中使用的定义
// CreationDate: 2003-12-17

#ifndef	__WHNET_DEF_H__
#define	__WHNET_DEF_H__

namespace n_whnet
{

enum
{
	SEND_RST_ERR			= -1,				// 可能是前面的数据还没有发出去，不能继续发送
	SEND_RST_OK				= 0,
};
// 普通接收命令的返回
enum
{
	RECV_RST_HAVEDATA		= 1,
	RECV_RST_NODATA			= 0,
	RECV_RST_ERR			= -1,
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNET_DEF_H__

