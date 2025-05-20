// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2.cpp
// Creator      : Wei Hua (魏华)
// Comment      : CNL2的对外对象创建的部分
// CreationDate : 2005-07-29
// ChangeLog    : 

#include "../inc/whnetcnl2_i.h"

using namespace	n_whnet;

WHDATAPROP_MAP_BEGIN_AT_ROOT(CNL2SlotMan::DATA_INI_INFO_T)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szBindAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(whbyte, ver, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bUseTickPack, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bUseCompression, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bCanSwitchAddr, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bLogNetStat, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bLogSendRecv, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMaxConnection, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSockSndBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSockRcvBufSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nMaxSinglePacketSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nConnectTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nConnectRSDiv, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCloseTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nDropTimeout, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nNoopDivDrop, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(charptr, szConnectAllowDeny, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nConnectAllowDenyRefreshInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nTQChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nControlOutQueueSize, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCryptorType1, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCryptorType2, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nAllocChunkSize, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nAlloc_MAXSIZENUM, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nAlloc_HASHSTEP1, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nAlloc_HASHSTEP2, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nStatInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szRSAPriKeyFile, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szRSAPass, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szAuthFile, 0)
WHDATAPROP_MAP_END()

CNL2SlotMan *	CNL2SlotMan::Create(int nType)
{
	switch( nType )
	{
		case	TYPE_UDP:
			return	new CNL2SlotMan_I_UDP;
		break;
	}
	return	NULL;
}
