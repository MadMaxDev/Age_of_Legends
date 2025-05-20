// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs_i_DealStrCmd.cpp
// Creator      : Wei Hua (κ��)
// Comment      : CAAFS��DealStrCmd���ֵľ���ʵ��
// CreationDate : 2005-08-19
// Change LOG   :

#include "../inc/pngs_caafs_i.h"

using namespace n_pngs;

int		CAAFS2_I::InitStrCmdMap()
{
	whcmn_strcmd_reganddeal_REGCMD_m_srad(NULL, _i_srad_NULL, NULL);
	whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(setver, "vermain, versub");
	return	0;
}
int		CAAFS2_I::DealStrCmd(const char *cszStrCmd)
{
	char	szCmd[256]="", szParam[4096]="";
	wh_strsplit("sa", cszStrCmd, "", szCmd, szParam);

	// ����շ���
	m_szRstStr[0]	= 0;

	// ���ú���
	m_srad.CallFunc(szCmd, szParam);

	return	0;
}
int		CAAFS2_I::_i_srad_NULL(const char *param)
{
	// Ĭ�ϵķ���
	sprintf(m_szRstStr, "UNKNOWN CMD %s!", m_srad.m_szUnknownCmd);
	return	0;
}
int		CAAFS2_I::_i_srad_setver(const char *param)
{
	// setver ver
	strcpy(m_cfginfo.szVer, param);
	sprintf(m_szRstStr, "rst 0 (cur ver:%s)", m_cfginfo.szVer);
	return	0;
}
