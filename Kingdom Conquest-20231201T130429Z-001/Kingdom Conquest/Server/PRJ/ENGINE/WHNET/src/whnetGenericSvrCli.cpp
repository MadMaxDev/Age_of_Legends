// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetGenericSvrCli.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ͨ�õķ�����/�ͻ���ģ��
//              : ClientID��ʵ���Ͼ������ӵ�ID
//              : �ϲ���SENDDATA/RECVDATA֮���Լ�ָ���߼���ص�ͨѶЭ��
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
