// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : GSMEM_i_CmdIn.cpp
// Creator      : Wei Hua (κ��)
// Comment      : GSMEM�߼�ģ���CmdIn������ʵ��
// CreationDate : 2007-09-26
// ChangeLog    :

#include "../inc/GSMEM_i.h"

using namespace n_pngs;

int		GSMEM::DealCmdIn_One_Instant(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	switch( nCmd )
	{
	case	PNGSPACKET_2MEM_CONFIG:
		{
			PNGSPACKET_2MEM_CONFIG_T	*pConfig	= (PNGSPACKET_2MEM_CONFIG_T *)pData;
			switch( pConfig->nSubCmd )
			{
			case	PNGSPACKET_2MEM_CONFIG_T::SUBCMD_GET_MEMSTATUS:
				{
					pConfig->nParam		= m_nIsInherited;
					if( !m_pSHM )
					{
						pConfig->nParam1	= 0;
					}
					else
					{
						pConfig->nParam1	= m_nMemDiff;
					}
				}
				break;
			}
		}
		break;
	case	PNGSPACKET_2MEM_GET_PTR:
		{
			PNGSPACKET_2MEM_XXX_PTR_T	*pCmd	= (PNGSPACKET_2MEM_XXX_PTR_T *)pData;
			if( strcmp(PNGS_GSMEM_NAME_AM, pCmd->pcszKey)==0 )
			{
				pCmd->ptr				= m_am;						// ��Ҳ˵��GSMEM��������ʹ����֮ǰ��ʼ��
			}
			else
			{
				// �����Ƿ���ԭ���������ָ��
				int	nIdx	= 0;
				if( m_mapKey2PtrIdx.get(pCmd->pcszKey, nIdx) )
				{
					pCmd->ptr			= m_pmyinfo->aptr[nIdx];
				}
				else
				{
					assert(0);
					pCmd->ptr			= NULL;
				}
			}
		}
		break;
	case	PNGSPACKET_2MEM_SET_PTR:
		{
			PNGSPACKET_2MEM_XXX_PTR_T	*pCmd	= (PNGSPACKET_2MEM_XXX_PTR_T *)pData;
			// �����Ƿ���ԭ���������ָ��
			int	nIdx	= 0;
			if( m_mapKey2PtrIdx.get(pCmd->pcszKey, nIdx) )
			{
				m_pmyinfo->aptr[nIdx]	= pCmd->ptr;
			}
			else
			{
				// ����������ǣ����ߵ�������Ӧ��keyû���ҵ�
				assert(0);
				pCmd->ptr				= NULL;
			}
		}
		break;
	}
	return	0;
}
int		GSMEM::DealCmdIn_One(ILogicBase *pRstAccepter, CMN::cmd_t nCmd, const void *pData, int nDSize)
{
	switch( nCmd )
	{
	case	PNGSPACKET_2LOGIC_SETMOOD:
		{
			PNGSPACKET_2LOGIC_SETMOOD_T	*pCmdSetMood	= (PNGSPACKET_2LOGIC_SETMOOD_T *)pData;
			switch( pCmdSetMood->nMood )
			{
			case	CMN::ILogic::MOOD_STOPPING:
				{
					// Ĭ�ϾͿ���ֱ��ֹͣ��
					m_nMood	= CMN::ILogic::MOOD_STOPPED;
				}
				break;
			}
		}
		break;
	}
	return	0;
}
