// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetGenericSvrCli.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 通用的服务器/客户端模板
//              : ClientID：实际上就是连接的ID
//              : 上层在SENDDATA/RECVDATA之上自己指定逻辑相关的通讯协议
// CreationDate : 2005-03-07
// Change LOG   :

#include "../inc/whnetGenericSvrCli_SMPTCP.h"

using namespace n_whnet;

whnetGenericSvrCli *		whnetGenericSvrCli::Create(int nType)
{
	switch( nType )
	{
		case	TYPE_SMPTCPSVR:
		return	new whnetGenericSvrCli_SMTCPSVR;
		case	TYPE_SMPTCPCLI:
		return	new whnetGenericSvrCli_SMTCPCLI;
	}
	return		NULL;
}
