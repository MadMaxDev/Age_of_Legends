#include "../inc/DBS4Web_i.h"

using namespace n_pngs;

void	DBS4Web_i::PushHeroSimpleData(int nCntrID, dia_mysql_query& q, unsigned int nCombatType, tty_id_t nExtID, const char* cszHeroIDs, unsigned int nReason)
{
	P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T	RplCmd;
	RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
	RplCmd.nSubCmd		= CMDID_GET_HERO_SIMPLE_DATA_RPL;
	RplCmd.nID			= nExtID;
	RplCmd.nReason		= nReason;
	RplCmd.nCombatType	= nCombatType;
	RplCmd.nNum			= 0;

	if (cszHeroIDs[0] == 0)
	{
		SendMsg_MT(nCntrID, &RplCmd, sizeof(RplCmd));
		return;
	}
	q.FreeResult();
	q.SpawnQuery("SELECT hero_id,army_type,army_level,army_num,army_prof,healthstate,level,exp,status"
		" FROM hire_heros WHERE hero_id in (%s)", cszHeroIDs);
	int	nPreRet			= 0;
	q.GetResult(nPreRet, false);
	if (nPreRet != MYSQL_QUERY_NORMAL)
	{
		GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4WEB)"get_hero_simple_data,SQLERR");
	}
	else
	{
		RplCmd.nNum		= q.NumRows();
		if (RplCmd.nNum == 0)
		{
			SendMsg_MT(nCntrID, &RplCmd, sizeof(RplCmd));
			return;
		}
		whvector<char> vectBuf;
		vectBuf.resize(sizeof(P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T)+sizeof(HeroSimpleData)*RplCmd.nNum);
		P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T*	pRplCmd	= (P_DBS4WEBUSER_GET_HERO_SIMPLE_DATA_T*)vectBuf.getbuf();
		memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
		HeroSimpleData*	pHero		= (HeroSimpleData*)wh_getptrnexttoptr(pRplCmd);
		while (q.FetchRow())
		{
			pHero->nHeroID			= q.GetVal_64();
			pHero->nArmyType		= q.GetVal_32();
			pHero->nArmyLevel		= q.GetVal_32();
			pHero->nArmyNum			= q.GetVal_32();
			pHero->nProf			= q.GetVal_32();
			pHero->nHealthState		= q.GetVal_32();
			pHero->nLevel			= q.GetVal_32();
			pHero->nExp				= q.GetVal_32();
			pHero->nStatus			= q.GetVal_32();
			pHero++;
		}
		SendMsg_MT(nCntrID, vectBuf.getbuf(), vectBuf.size());
	}
}
void	DBS4Web_i::PushDrug(int nCntrID, dia_mysql_query& q, const char* cszAccountIDs)
{
	P_DBS4WEBUSER_REFRESH_DRUG_T	RplCmd;
	RplCmd.nCmd			= P_DBS4WEB_RPL_CMD;
	RplCmd.nSubCmd		= CMDID_REFRESH_DRUG_RPL;
	RplCmd.nNum			= 0;
	
	if (cszAccountIDs[0] == 0)
	{
		return;
	}

	q.FreeResult();
	q.SpawnQuery("SELECT account_id,drug FROM common_characters WHERE account_id IN (%s)", cszAccountIDs);
	int	nPreRet		= 0;
	q.GetResult(nPreRet, false);
	if (nPreRet != MYSQL_QUERY_NORMAL)
	{
	}
	else
	{
		RplCmd.nNum	= q.NumRows();
		if (RplCmd.nNum > 0)
		{
			whvector<char> vectBuf;
			vectBuf.resize(sizeof(P_DBS4WEBUSER_REFRESH_DRUG_T) + RplCmd.nNum*sizeof(P_DBS4WEBUSER_REFRESH_DRUG_T::DrugInfo));
			P_DBS4WEBUSER_REFRESH_DRUG_T*	pRplCmd	= (P_DBS4WEBUSER_REFRESH_DRUG_T*)vectBuf.getbuf();
			memcpy(pRplCmd, &RplCmd, sizeof(RplCmd));
			P_DBS4WEBUSER_REFRESH_DRUG_T::DrugInfo*	pUnit	= (P_DBS4WEBUSER_REFRESH_DRUG_T::DrugInfo*)wh_getptrnexttoptr(pRplCmd);
			while (q.FetchRow())
			{
				pUnit->nAccountID	= q.GetVal_64();
				pUnit->nDrug		= q.GetVal_32();

				pUnit++;
			}
			SendMsg_MT(nCntrID, vectBuf.getbuf(), vectBuf.size());
		}
	}
}
void	DBS4Web_i::PushCharAtb(int nCntrID, dia_mysql_query& q, const char* cszAccountIDs, bool bSimple)
{
	if (cszAccountIDs[0] == 0)
	{
		return;
	}
	whvector<char> vectBuf;
	vectBuf.resize(sizeof(P_DBS4WEBUSER_REFRESH_CHAR_ATB_T) + sizeof(CharAtb));
	P_DBS4WEBUSER_REFRESH_CHAR_ATB_T*	pRplCmd	= (P_DBS4WEBUSER_REFRESH_CHAR_ATB_T*)vectBuf.getbuf();
	pRplCmd->nCmd		= P_DBS4WEB_RPL_CMD;
	pRplCmd->nSubCmd	= CMDID_REFRESH_CHAR_ATB_RPL;
	pRplCmd->bSimple	= bSimple;
	CharAtb*	pCharAtb	= (CharAtb*)wh_getptrnexttoptr(pRplCmd);

	int	nOffset				= 0;
	tty_id_t	nAccountID	= 0;
	while (wh_strsplit(&nOffset, "I", cszAccountIDs, ",", &nAccountID) == 1)
	{
		q.FreeResult();
		q.SpawnQuery("CALL get_char_atb(0x%"WHINT64PRFX"X)", nAccountID);
		int	nPreRst		= 0;
		q.ExecuteSPWithResult(nPreRst);
		if (nPreRst != MYSQL_QUERY_NORMAL)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"refresh_char_atb,SQLERROR,0x%"WHINT64PRFX"X", nAccountID);
		}
		else
		{
			if (q.FetchRow())
			{
				pCharAtb->nAccountID		= nAccountID;
				WH_STRNCPY0(pCharAtb->szName, q.GetStr());
				pCharAtb->nLastLoginTime	= q.GetVal_32();
				pCharAtb->nLastLogoutTime	= q.GetVal_32();
				pCharAtb->nExp				= q.GetVal_32();
				pCharAtb->nLevel			= q.GetVal_32();
				pCharAtb->nDiamond			= q.GetVal_32();
				pCharAtb->nCrystal			= q.GetVal_32();
				pCharAtb->nGold				= q.GetVal_32();
				pCharAtb->nVip				= q.GetVal_32();
				pCharAtb->nSex				= q.GetVal_32();
				pCharAtb->nHeadID			= q.GetVal_32();
				pCharAtb->nPosX				= q.GetVal_32();
				pCharAtb->nPosY				= q.GetVal_32();
				pCharAtb->nDevelopment		= q.GetVal_32();
				pCharAtb->nPopulation		= q.GetVal_32();
				pCharAtb->nBuildNum			= q.GetVal_32();
				pCharAtb->nAddedBuildNum	= q.GetVal_32();
				pCharAtb->nFreeDrawLotteryNum	= q.GetVal_32();

				pCharAtb->nAllianceID		= q.GetVal_64();
				WH_STRNCPY0(pCharAtb->szAllianceName, q.GetStr());

				pCharAtb->nDrug				= q.GetVal_32();
				pCharAtb->nTradeNum			= q.GetVal_32();

				WH_STRNCPY0(pCharAtb->szSignature, q.GetStr());
				pCharAtb->nChangeNameNum	= q.GetVal_32();

				pCharAtb->nProtectTimeEndTick		= q.GetVal_32();
				pCharAtb->nAddBuildNumTimeEndTick	= q.GetVal_32();
				unsigned int nTimeNow		= wh_time();
				pCharAtb->nProtectTime		= nTimeNow>pCharAtb->nProtectTimeEndTick?0:pCharAtb->nProtectTimeEndTick-nTimeNow;
				pCharAtb->nAddBuildNumTime	= nTimeNow>pCharAtb->nAddBuildNumTimeEndTick?0:pCharAtb->nAddBuildNumTimeEndTick-nTimeNow;

				pCharAtb->nTotalBuildNum	= q.GetVal_32();
				pCharAtb->nNotificationID	= q.GetVal_32();
				pCharAtb->nAllianceDrawLotteryNum	= q.GetVal_32();
				pCharAtb->nTotalDiamond		= q.GetVal_32();
				pCharAtb->nInstanceWangzhe	= q.GetVal_32();
				pCharAtb->nInstanceZhengzhan	= q.GetVal_32();
				pCharAtb->nCup				= q.GetVal_32();
				pCharAtb->bBinded			= (bool)q.GetVal_32();
				pCharAtb->bVipDisplay		= (bool)q.GetVal_32();

				SendMsg_MT(nCntrID, vectBuf.getbuf(), vectBuf.size());
			}
		}
	}
}
void	DBS4Web_i::PushMail(int nCntrID, dia_mysql_query& q, const char* cszAccountIDnMailIDs)
{
	if (cszAccountIDnMailIDs[0] == 0)
	{
		return;
	}
	P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T	RplCmd;
	RplCmd.nCmd					= P_DBS4WEB_RPL_CMD;
	RplCmd.nSubCmd				= CMDID_GET_PRIVATE_MAIL_1_RPL;
	RplCmd.nRst					= P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T::RST_OK;
	RplCmd.nAccountID			= 0;

	tty_id_t		nAccountID	= 0;
	unsigned int	nMailID		= 0;
	int				nOffset		= 0;
	while (wh_strsplit(&nOffset, "Id", cszAccountIDnMailIDs, ",", &nAccountID, &nMailID) == 2)
	{
		q.FreeResult();
		q.SpawnQuery("SELECT m.mail_id,m.sender_id,m.flag,m.readed,m.type,uncompress(m.text),uncompress(m.ext_data),m.time,IFNULL(c.name,'') as sender_name FROM private_mails m LEFT JOIN common_characters c ON m.sender_id=c.account_id WHERE m.account_id=0x%"WHINT64PRFX"X AND mail_id=%d", nAccountID, nMailID);
		int	nPreRet		= 0;
		q.GetResult(nPreRet, false);
		if (nPreRet != MYSQL_QUERY_NORMAL)
		{
			GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_mail_1,SQLERR,0x%"WHINT64PRFX"X,%d", nAccountID, nMailID);
		}
		else
		{
			if (q.FetchRow())
			{
				whvector<char> vectBuf;
				vectBuf.resize(sizeof(P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T));
				P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T*	pRpl	= (P_DBS4WEBUSER_GET_PRIVATE_MAIL_1_T*)vectBuf.getbuf();
				memcpy(pRpl, &RplCmd, sizeof(RplCmd));
				pRpl->nAccountID		= nAccountID;

				int*	pLengths		= q.FetchLengths();
				int		nTotalMsgLen	=  wh_offsetof(PrivateMailUnit, szText) + pLengths[5] + (sizeof(int)+pLengths[6]);
				PrivateMailUnit*	pUnit		= (PrivateMailUnit*)vectBuf.pushn_back(nTotalMsgLen);

				pUnit->nTextLen			= pLengths[5];
				pUnit->nMailID			= q.GetVal_32();
				pUnit->nSenderID		= q.GetVal_64();
				pUnit->nFlag			= (char)q.GetVal_32();
				pUnit->bReaded			= (bool)q.GetVal_32();
				pUnit->nType			= (char)q.GetVal_32();
				memcpy(pUnit->szText, q.GetStr(), pUnit->nTextLen);
				int		nMsgUnitLen		= wh_offsetof(PrivateMailUnit, szText) + pUnit->nTextLen;
				int*	pExtDataLen		= (int*)wh_getoffsetaddr(pUnit, nMsgUnitLen);
				*pExtDataLen			= pLengths[6];
				void*	pExtData		= (void*)wh_getptrnexttoptr(pExtDataLen);
				memcpy(pExtData, q.GetStr(), *pExtDataLen);
				pUnit->nTime			= q.GetVal_32();
				WH_STRNCPY0(pUnit->szSender, q.GetStr());

				SendMsg_MT(nCntrID, vectBuf.getbuf(), vectBuf.size());
			}
			else
			{
				GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,DBS4Web)"get_private_mail_1,NOT EXISTS,0x%"WHINT64PRFX"X,%d", nAccountID, nMailID);
			}
		}
	}
}