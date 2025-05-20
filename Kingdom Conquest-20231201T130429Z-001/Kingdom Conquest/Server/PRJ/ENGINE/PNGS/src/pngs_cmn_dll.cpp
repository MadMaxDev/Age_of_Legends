// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cmn_dll.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��ͨ�ù���ģ���к�DLL��صĲ���
//                �����Ϊ�˺�DLL�޹صĵ��ò������Ӻ�DLL��صĵ���(��Ҫ����linux��)
//                PNGS��Pixel Network Game Structure����д
//                ����Logic��Interface��Type��Ҫ�ھ���Ӧ���о��嶨��
// CreationDate : 2005-09-07 ��pngs_cmn.cpp�з������
// Change LOG   :

#include "../inc/pngs_cmn.h"
#include "../inc/pngs_packet_logic.h"
#include <WHCMN/inc/whdll.h>

using namespace n_whnet;
using namespace n_whcmn;

namespace n_pngs
{

int		pngs_load_logic_dll_and_send_cmd(const char *cszDLLFile, const char *cszCreateFuncName, CMN::cmd_t nCmd, const void *pData, size_t nDSize)
{
	// ����DLL
	typedef	CMNBody::ILogic *	(*fn_ILOGIC_CREATE_T)();
	fn_ILOGIC_CREATE_T		LogicCreate	= NULL;
	WHDLL_LOAD_UNIT_T		aUnits[]	=
	{
		WHDLL_LOAD_UNIT_DECLARE0(cszCreateFuncName, LogicCreate, NULL)
		WHDLL_LOAD_UNIT_DECLARE_LAST()
	};
	WHDLL_Loader::SetDftFunc(aUnits);
	WHDLL_Loader	*pLoader	= WHDLL_Loader::Create();
	if( pLoader == NULL )
	{
		assert(0);
		return	-1;
	}

	int				rst		= 0;
	CMNBody::ILogic *	pLogic	= NULL;
	try
	{
		if( pLoader->Load(cszDLLFile, aUnits)<0 )
		{
			throw	-11;
		}
		if( LogicCreate == NULL )
		{
			throw	-12;
		}
		pLogic		= LogicCreate();
		if( pLogic == NULL )
		{
			throw	-13;
		}
		if( pLogic->CmdIn_Instant(NULL, nCmd, pData, nDSize)<0 )
		{
			throw	-14;
		}
	}
	catch(int nErrCode)
	{
		rst	= nErrCode;
	}

	WHSafeSelfDestroy(pLogic);
	delete	pLoader;

	return	rst;
}

int		pngs_load_logic_dll_and_query_history(const char *cszDLLFile, const char *cszCreateFuncName, const char *cszVer)
{
	PNGSPACKET_2LOGIC_DISPHISTORY_T	Cmd;
	if( !cszVer || cszVer[0]==0 )
	{
		Cmd.szVerToQuery[0]	= 0;
	}
	else
	{
		WH_STRNCPY0(Cmd.szVerToQuery, cszVer);
	}
	return	pngs_load_logic_dll_and_send_cmd(cszDLLFile, cszCreateFuncName, PNGSPACKET_2LOGIC_DISPHISTORY, &Cmd, sizeof(Cmd));
}

}		// EOF namespace n_pngs
