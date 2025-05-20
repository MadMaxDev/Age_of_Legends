// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_gms_i_GMS_MAINSTRUCTURE_CmdIn.cpp
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��GMSģ���ڲ���GMS_MAINSTRUCTUREģ����CmdIn������ʵ��
//                PNGS��Pixel Network Game Structure����д
//                GMS��Game Master Server����д�����߼��������е��ܿط�����
// CreationDate : 2005-09-02
// Change LOG   :

#include "../inc/pngs_gms_i_GMS_MAINSTRUCTURE.h"
#include "../inc/pngs_packet_logic.h"
#include "../inc/pngs_packet_gms_logic.h"

using namespace n_pngs;

int		GMS_MAINSTRUCTURE::InitTstStrMap()
{
	whcmn_strcmd_reganddeal_REGCMD_m_srad(NULL, _i_srad_NULL, NULL);
	whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(help, "[cmd]");
	whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(getclsinfo, "clsid // ��ȡCLS��Ϣ�����clsidΪ0���ʾ����cls��");
	whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(setclsinfo, "clsid key=val[,key=val] // ����CLS��Ϣ");
	return	0;
}
int		GMS_MAINSTRUCTURE::DealTstStr(const char *cszTstStr, char *szRstStr, size_t *pnSize)
{
	szRstStr[0]	= 0;
	m_pszRstStr	= szRstStr;
	m_pnRstSize	= pnSize;

	char	cmd[256]="";
	char	param[4096]="";
	wh_strsplit("sa", cszTstStr, "", cmd, param);

	int	rst	= 0;
	rst	= m_srad.CallFunc(cmd, param);

	if( m_srad.m_bShouldHelp )
	{
		_i_srad_help(cmd);
	}
	*pnSize	= strlen(szRstStr)+1;

	return	rst;
}
int		GMS_MAINSTRUCTURE::_i_srad_NULL(const char *param)
{
	snprintf(m_pszRstStr, *m_pnRstSize, "Not implemented: %s", m_srad.m_szUnknownCmd);
	return	0;
}
int		GMS_MAINSTRUCTURE::_i_srad_help(const char *param)
{
	m_srad.Help(m_pszRstStr, *m_pnRstSize, param);
	return	0;
}
int		GMS_MAINSTRUCTURE::_i_srad_getclsinfo(const char *param)
{
	int		nID		= whstr2int(param);
	CLSUnit	*pCLS	= m_pSHMData->m_CLSs.GetPtrByID(nID);
	if( pCLS )
	{
	}
	else
	{
		// ��ӡ���е�CLS
	}
	return	0;
}
int		GMS_MAINSTRUCTURE::_i_srad_setclsinfo(const char *param)
{
	return	0;
}
