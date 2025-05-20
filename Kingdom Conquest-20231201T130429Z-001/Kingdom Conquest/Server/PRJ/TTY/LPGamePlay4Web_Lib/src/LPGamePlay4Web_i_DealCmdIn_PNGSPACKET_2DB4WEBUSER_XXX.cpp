#include "../inc/LPGamePlay4Web_i.h"

using namespace n_pngs;

/*
int		LPGamePlay4Web_i::DealCmdIn_PNGSPACKET_2DB4WEBUSER_GETACCOUNTSMPINFO_RST(PNGSPACKET_2DB4WEBUSER_GETACCOUNTSMPINFO_RST_T* pRst, int nDSize)
{
	// 获得Transaction
	whtransactionbase*	pT	= m_pTMan->GetTransaction(pRst->anExt[0]);
	if (pT != NULL)
	{
		pT->DoNext(PNGSPACKET_2DB4WEBUSER_GETACCOUNTSMPINFO_RST, pRst, nDSize);
	}
	else
	{

	}
	return 0;
}
int		LPGamePlay4Web_i::DealCmdIn_PNGSPACKET_2DB4WEBUSER_GETCHARINFO_SMP_RST(PNGSPACKET_2DB4WEBUSER_GETCHARINFO_SMP_RST_T* pRst, int nDSize)
{
	switch (pRst->nRst)
	{
	case PNGSPACKET_2DB4WEBUSER_GETCHARINFO_SMP_RST_T::RST_OK:
		{
			LPGamePlay4Web_i::PlayerUnit*	pPlayer	= GetPlayerByID(pRst->anExt[0]);
			if (pPlayer != NULL)
			{
				int	i	= pPlayer->FindIdxByCharID(pRst->nCharOnlyID);
				if (i < 0)	// 有可能上一次事务失败了,重新做了次,这样可能i>=0
				{
					// 找一个空位置
					for (int j=0; j<TTY_CHARACTERNUM_PER_ACCOUNT; ++j)
					{
						if (pPlayer->aCharInfoSmp[j].nID <= 0)
						{
							i	= j;
							break;
						}
					}
				}
				if (i >= 0)	
				{
					pPlayer->aCharInfoSmp[i].nID	= pRst->nCharOnlyID;
					pPlayer->aCharInfoSmp[i].nLvl	= pRst->nLvl;
					WH_CLEAR_STRNCPY0(pPlayer->aCharInfoSmp[i].szName, pRst->szCharName);
					pPlayer->nCharNum++;

					// 把这个数据发给玩家
					TTY_CHARCMD_GETCHARINFO_SMP_RST_T	rst;
					rst.nCmd			= TTY_LPGAMEPLAY_CLIENT_CHARCMD_RST;
					rst.nCharCmd		= TTY_CHARCMD_GETCHARINFO_SMP_RST;
					rst.nRst			= TTY_CHARCMD_RST_OK;
					rst.nCharOnlyID		= pRst->nCharOnlyID;
					rst.nLvl			= pRst->nLvl;
					rst.wExcelID		= pRst->wExcelID;
					rst.bySex			= pRst->bySex;
					wh_char2utf8(pRst->szCharName, strlen(pRst->szCharName), rst.szCharName, sizeof(rst.szCharName));

					SendCmdToClient(pPlayer->nID, &rst, sizeof(rst));
				}
				else
				{
					// 没有找到合适位置
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,CHARINFOSMP_RST)"not enough position,0x%X,%s,%s,0x%"WHINT64PRFX"X", pPlayer->nID, pPlayer->szAccount, pRst->szCharName, pRst->nCharOnlyID);
				}
			}
		}
		break;
	default:
		{

		}
		break;
	}

	if (pRst->anExt[1] != 0)
	{
		whtransactionbase*	pT	= m_pTMan->GetTransaction(pRst->anExt[1]);
		if (pT != NULL)
		{
			pT->DoNext(PNGSPACKET_2DB4WEBUSER_GETCHARINFO_SMP_RST, pRst, nDSize);
		}
	}
	return 0;
}
int		LPGamePlay4Web_i::DealCmdIn_PNGSPACKET_2DB4WEBUSER_GETCHARINFO_RST(PNGSPACKET_2DB4WEBUSER_GETCHARINFO_RST_T* pRst, int nDSize)
{
	// 获得Transaction
	whtransactionbase*	pT	= m_pTMan->GetTransaction(pRst->anExt[1]);
	if (pT != NULL)
	{
		pT->DoNext(PNGSPACKET_2DB4WEBUSER_GETCHARINFO_RST, pRst, nDSize);
	}
	else
	{

	}
	return 0;
}
*/
