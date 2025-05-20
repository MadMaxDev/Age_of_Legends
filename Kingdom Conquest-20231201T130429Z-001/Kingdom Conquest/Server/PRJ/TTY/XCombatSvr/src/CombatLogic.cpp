#include "../inc/CombatLogic.h"

#include "WHCMN/inc/whdbg.h"
#include "WHNET/inc/whnetudpGLogger.h"
#include "../../Common/inc/tty_common_def.h"

using namespace n_whcmn;
using namespace n_whnet;
using namespace n_pngs;

WHDATAPROP_MAP_BEGIN_AT_ROOT(CCombatLogic::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nCombatRand, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szBattleSoFileName, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bLogCombat, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(bool, bFakeCombat, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(float, fCounterRate, 0)
	WHDATAPROP_ON_SETVALUE_smp(unknowntype, MYSQLInfo, 0)
WHDATAPROP_MAP_END()

WHDATAPROP_MAP_BEGIN_AT_ROOT(CCombatLogic::CFGINFO_T::MySQLInfo)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlHost, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMysqlPort, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlUser, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlPass, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlDB, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlCharset, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlSocket, 0)
WHDATAPROP_MAP_END()

CCombatLogic::CCombatLogic()
	: m_tickNow(0)
	, m_pMySQL(NULL)
{

}
CCombatLogic::~CCombatLogic()
{
	CloseMySQL(m_pMySQL);
}

MYSQL*	CCombatLogic::OpenMySQL()
{
	MYSQL*	pMySQL			= mysql_init(NULL);
	int		nTimeoutOption	= 1;
	mysql_options(pMySQL, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&nTimeoutOption);

	CFGINFO_T::MySQLInfo&	mysqlInfo	= m_cfginfo.MYSQLInfo;
	unsigned long			mysqlFlag	= CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS;
	if (NULL == mysql_real_connect(pMySQL, mysqlInfo.szMysqlHost, mysqlInfo.szMysqlUser, mysqlInfo.szMysqlPass, 
		mysqlInfo.szMysqlDB, mysqlInfo.nMysqlPort, mysqlInfo.szMysqlSocket, mysqlFlag))
	{
		mysql_close(pMySQL);
		return NULL;
	}
	if (0 != mysql_set_character_set(pMySQL, mysqlInfo.szMysqlCharset))
	{
		mysql_close(pMySQL);
		return NULL;
	}

	return pMySQL;
}
void	CCombatLogic::CloseMySQL(MYSQL*& pMySQL)
{
	if (pMySQL != NULL)
	{
		mysql_close(pMySQL);
		pMySQL	= NULL;
	}
}
bool	CCombatLogic::IsMySQLHandlerValid(MYSQL* pMySQL, bool bCloseInvalidHandler)
{
	if (pMySQL == NULL)
	{
		return false;
	}
	if (mysql_ping(pMySQL) != 0)
	{
		if (bCloseInvalidHandler)
		{
			CloseMySQL(pMySQL);
		}
		return false;
	}
	return true;
}
int		CCombatLogic::Init(const char* cszCFGName)
{
	m_tickNow		= wh_gettickcount();
	int	nRst		= 0;
	if ((nRst=Init_CFG(cszCFGName)) < 0)
	{
		printf("Init_CFG,%d%s", nRst, WHLINEEND);
		return -1;
	}
	m_pMySQL		= OpenMySQL();
	if (m_pMySQL == NULL)
	{
		printf("OpenMySQL,%d%s", nRst, WHLINEEND);
		return -2;
	}
	if ((nRst=m_monoHelper.Init(m_cfginfo.szBattleSoFileName)) != 0)
	{
		printf("m_monoHelper.Init,%d%s", nRst, WHLINEEND);
		return -3;
	}

	m_vectAttackData.reserve(2*1024);
	m_vectDefenseData.reserve(2*1024);
	m_vectAttackResult.reserve(2*1024);
	m_vectDefenseResult.reserve(2*1024);
	return 0;
}
int		CCombatLogic::Init_CFG(const char* cszCFGName)
{
	// 分析文件
	WHDATAINI_CMN	ini;
	ini.addobj("CombatSvr", &m_cfginfo);
	int	nRst		= ini.analyzefile(cszCFGName);
	if (nRst < 0)
	{
		printf("Init_CFG,%d,ini.analyzefile,%s%s", nRst, ini.printerrreport(), WHLINEEND);
		return -1;
	}
	return 0;
}
int		CCombatLogic::Init_CFG_Reload(const char* cszCFGName)
{
	// 分析文件
	WHDATAINI_CMN	ini;
	ini.addobj("CombatSvr", &m_cfginfo);
	int	nRst		= ini.analyzefile(cszCFGName, false, 1);
	if (nRst < 0)
	{
		printf("Init_CFG_Reload,%d,ini.analyzefile,%s%s", nRst, ini.printerrreport(), WHLINEEND);
		return -1;
	}
	return 0;
}
int		CCombatLogic::Work()
{
	if (!IsMySQLHandlerValid(m_pMySQL, true))
	{
		m_pMySQL	= OpenMySQL();
		if (m_pMySQL == NULL)
		{
			printf("OpenMySQL failed%s", WHLINEEND);
			return -1;
		}
	}
	dia_mysql_query	q(m_pMySQL, QUERY_LEN);
	if (!q.Connected())
	{
		return -2;
	}
	else
	{
		q.SpawnQuery("SELECT event_id,account_id,obj_id,combat_type,attack_restore_percent,defense_restore_percent FROM alli_instance_combat WHERE error_flag=0 ORDER BY event_id ASC LIMIT 1");
		int	nPreRet	= 0;
		q.GetResult(nPreRet, false);
		if (nPreRet != MYSQL_QUERY_NORMAL)
		{
			return -10;
		}
		else
		{
			if (q.FetchRow())
			{
				dbgtimeuse		dtu;
				dtu.BeginMark();
				dtu.Mark();

				// 用到的一些变量
				int				nOffset		= 0;
				const char*		cszAll		= NULL;
				tty_id_t		nEventID	= 0;
				tty_id_t		nAccountID	= 0;
				tty_id_t		nObjID		= 0;
				unsigned int	nCombatType	= 0;
				int				nCombatResult		= 0;
				float			fAttackRestorePer	= 0.0;
				float			fDefenseRestorePer	= 0.0;
				int				nRand		= 0;
				char			szOne[256]	= "";
				whhash<tty_id_t, AlliInstanceCombatLog::HeroUnit>::kv_iterator	it;

				// 取出数据
				nEventID					= q.GetVal_64();
				nAccountID					= q.GetVal_64();
				nObjID						= q.GetVal_64();
				nCombatType					= q.GetVal_32();
				fAttackRestorePer			= q.GetVal_float();
				fDefenseRestorePer			= q.GetVal_float();

				switch (nCombatType)
				{
				case combat_type_alli_instance:
				case combat_type_arena:
					{
						nRand				= m_cfginfo.nCombatRand;
					}
					break;
				default:
					{
						nRand				= rand();
					}
					break;
				}
				
				// 攻击方战斗数据
				q.FreeResult();
				q.SpawnQuery("CALL alli_instance_get_attacker_combat_data(0x%"WHINT64PRFX"X)", nEventID);
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,COMBAT)"alli_instance_get_attacker_combat_data,SQLERROR,%d,0x%"WHINT64PRFX"X"
						, nPreRet, nEventID);
					return -10;
				}
				else
				{
					if (q.FetchRow())
					{
						int*		pLengths	= q.FetchLengths();
						int			nResult		= q.GetVal_32();
						if (nResult != 0)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,COMBAT)"alli_instance_get_attacker_combat_data,%d,0x%"WHINT64PRFX"X"
								, nResult, nEventID);
							return -100;
						}
						m_vectAttackData.resize(pLengths[1]+1);
						memcpy(m_vectAttackData.getbuf(), q.GetStr(), pLengths[1]+1);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,COMBAT)"alli_instance_get_attacker_combat_data,SQLERROR,NO RESULT,%d,0x%"WHINT64PRFX"X"
							, nPreRet, nEventID);
						return -100;
					}
				}

				// 防守方战斗数据
				q.FreeResult();
				q.SpawnQuery("CALL alli_instance_get_defender_combat_data(0x%"WHINT64PRFX"X)", nEventID);
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,COMBAT)"alli_instance_get_defender_combat_data,SQLERROR,%d,0x%"WHINT64PRFX"X"
						, nPreRet, nEventID);
					return -10;
				}
				else
				{
					if (q.FetchRow())
					{
						int*		pLengths	= q.FetchLengths();
						int			nResult		= q.GetVal_32();
						if (nResult != 0)
						{
							GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,COMBAT)"alli_instance_get_defender_combat_data,%d,0x%"WHINT64PRFX"X"
								, nResult, nEventID);
							return -100;
						}
						m_vectDefenseData.resize(pLengths[1]+1);
						memcpy(m_vectDefenseData.getbuf(), q.GetStr(), pLengths[1]+1);
					}
					else
					{
						GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,COMBAT)"alli_instance_get_defender_combat_data,SQLERROR,NO RESULT,%d,0x%"WHINT64PRFX"X"
							, nPreRet, nEventID);
						return -100;
					}
				}

				// 1.初始化战斗
				m_vectAttackResult.resize(0);
				m_vectDefenseResult.resize(0);
				m_mapAttackHero.clear();
				m_mapDefenseHero.clear();

				// (1)攻击方初始化
				nOffset	= 0;
				cszAll	= m_vectAttackData.getbuf();
				if (cszAll[0] != 0)
				{
					cszAll		= cszAll + 1;
					szOne[0]	= 0;
					while (wh_strsplit(&nOffset, "s", cszAll, ",", szOne) == 1)
					{
						unsigned int	nRow		= 0;
						unsigned int	nCol		= 0;
						tty_id_t		nHeroID		= 0;
						unsigned int	nLevel		= 0;
						char			szCharName[TTY_CHARACTERNAME_LEN]	= "";
						char			szHeroName[TTY_CHARACTERNAME_LEN]	= "";
						unsigned int	nProf		= 0;
						unsigned int	nArmyType	= 0;
						unsigned int	nArmyLevel	= 0;
						unsigned int	nArmyNum	= 0;

						wh_strsplit("ddsIdsdddd", szOne+1, "*", &nRow, &nCol, szCharName, &nHeroID, &nLevel, szHeroName, &nProf, &nArmyType, &nArmyLevel, &nArmyNum);

						AlliInstanceCombatLog::HeroUnit*	pHero	= (AlliInstanceCombatLog::HeroUnit*)m_mapAttackHero.getptr(nHeroID, true);
						pHero->nRow					= nRow;
						pHero->nCol					= nCol;
						pHero->nHeroID				= nHeroID;
						pHero->nHeroLevel			= nLevel;
						WH_STRNCPY0(pHero->szCharName, szCharName);
						WH_STRNCPY0(pHero->szHeroName, szHeroName);
						pHero->nArmyType			= nArmyType;
						pHero->nArmyLevel			= nArmyLevel;
						pHero->nArmyNum				= nArmyNum;
						pHero->nArmyDeathNum		= 0;			// 这个应该是演算出来的
						pHero->nArmyRestoreNum		= 0;
						pHero->nProf				= nProf;

						szOne[0]		= 0;
					}
				}

				// (2)防守方数据初始化
				nOffset			= 0;
				cszAll			= m_vectDefenseData.getbuf();
				if (cszAll[0] != 0)
				{
					cszAll		= cszAll + 1;
					szOne[0]	= 0;
					while (wh_strsplit(&nOffset, "s", cszAll, ",", szOne) == 1)
					{
						unsigned int	nRow		= 0;
						unsigned int	nCol		= 0;
						tty_id_t		nHeroID		= 0;
						unsigned int	nLevel		= 0;
						char			szCharName[TTY_CHARACTERNAME_LEN]	= "";
						char			szHeroName[TTY_CHARACTERNAME_LEN]	= "";
						unsigned int	nProf		= 0;
						unsigned int	nArmyType	= 0;
						unsigned int	nArmyLevel	= 0;
						unsigned int	nArmyNum	= 0;

						wh_strsplit("ddsIdsdddd", szOne+1, "*", &nRow, &nCol, szCharName, &nHeroID, &nLevel, szHeroName, &nProf, &nArmyType, &nArmyLevel, &nArmyNum);

						AlliInstanceCombatLog::HeroUnit*	pHero	= (AlliInstanceCombatLog::HeroUnit*)m_mapDefenseHero.getptr(nHeroID, true);
						pHero->nRow					= nRow;
						pHero->nCol					= nCol;
						pHero->nHeroID				= nHeroID;
						pHero->nHeroLevel			= nLevel;
						WH_STRNCPY0(pHero->szCharName, szCharName);
						WH_STRNCPY0(pHero->szHeroName, szHeroName);
						pHero->nArmyType			= nArmyType;
						pHero->nArmyLevel			= nArmyLevel;
						pHero->nArmyNum				= nArmyNum;
						pHero->nArmyDeathNum		= 0;			// 这个应该是演算出来的
						pHero->nArmyRestoreNum		= 0;
						pHero->nProf				= nProf;

						szOne[0]		= 0;
					}
				}

				// 2.计算战斗
				dtu.Mark();
				m_vectMonoParamAttack.resize(0);
				m_vectMonoParamDefense.resize(0);
				it				= m_mapAttackHero.begin();
				while (m_mapAttackHero.end() != it)
				{
					szOne[0]	= 0;
					AlliInstanceCombatLog::HeroUnit&	hero	= it.getvalue();
					int		nLength		= sprintf(szOne, "%"WHINT64PRFX"d*%d*%d*%d*%d"
						, hero.nHeroID, hero.nRow, hero.nCol, hero.nProf, hero.nArmyType);
					if (m_vectMonoParamAttack.size() != 0)
					{
						m_vectMonoParamAttack.push_back(',');
					}
					m_vectMonoParamAttack.pushn_back(szOne, nLength);
					++it;
				}
				m_vectMonoParamAttack.push_back(0);		// 0结尾

				it				= m_mapDefenseHero.begin();
				while (m_mapDefenseHero.end() != it)
				{
					szOne[0]	= 0;
					AlliInstanceCombatLog::HeroUnit&	hero	= it.getvalue();
					int		nLength		= sprintf(szOne, "%"WHINT64PRFX"d*%d*%d*%d*%d"
						, hero.nHeroID, hero.nRow, hero.nCol, hero.nProf, hero.nArmyType);
					if (m_vectMonoParamDefense.size() != 0)
					{
						m_vectMonoParamDefense.push_back(',');
					}
					m_vectMonoParamDefense.pushn_back(szOne, nLength);
					++it;
				}
				m_vectMonoParamDefense.push_back(0);		// 0结尾

				if (m_cfginfo.bLogCombat)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,COMBAT)"BEGIN;%d;%s;%s;%f"
						, nRand, m_vectMonoParamAttack.getbuf(), m_vectMonoParamDefense.getbuf(), m_cfginfo.fCounterRate);
				}

				const char*	cszCombatResult			= m_monoHelper.DoOneQuery(nRand, m_vectMonoParamAttack.getbuf()
					, m_vectMonoParamDefense.getbuf(), m_cfginfo.fCounterRate);
				
				if (m_cfginfo.bLogCombat)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_STORY, GLGR_STD_HDR(,COMBAT)"END;%s", cszCombatResult);
				}

				// 3.处理战斗结果
				dtu.Mark();
				nOffset		= 0;
				wh_strsplit(&nOffset, "d", cszCombatResult, ",", &nCombatResult);
				if (nCombatResult == combat_result_none)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,COMBAT)"ERROR,INPUT,%d,%s,%s,%f"
						, nRand, m_vectMonoParamAttack.getbuf(), m_vectMonoParamDefense.getbuf(), m_cfginfo.fCounterRate);
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,COMBAT)"ERROR,OUTPUT,%s", cszCombatResult);

					if (m_cfginfo.bFakeCombat)
					{
						nOffset			= 0;
						cszCombatResult	= FakeCombat();
						wh_strsplit(&nOffset, "d", cszCombatResult, ",", &nCombatResult);
					}
					else
					{
						q.FreeResult();
						q.SpawnQuery("UPDATE alli_instance_combat SET error_flag=1 WHERE event_id=0x%"WHINT64PRFX"X", nEventID);
						q.Execute(nPreRet);

						return -11;
					}
				}
				szOne[0]	= 0;
				while (wh_strsplit(&nOffset, "s", cszCombatResult, ",", szOne) == 1)
				{
					tty_id_t		nHeroID			= 0;
					unsigned int	nLeftForce		= 0;
					unsigned int	nDir			= 0;
					wh_strsplit("Idd", szOne, "*", &nHeroID, &nLeftForce, &nDir);
					AlliInstanceCombatLog::HeroUnit*	pHero		= NULL;
					switch (nDir)
					{
					case direction_left:
						{
							pHero					= m_mapAttackHero.getptr(nHeroID);
							if (pHero == NULL)
							{
								return -12;
							}
							pHero->nArmyDeathNum	= (float)(pHero->nProf>nLeftForce?pHero->nProf-nLeftForce:0)/(float)pHero->nProf*(float)pHero->nArmyNum;
							pHero->nArmyRestoreNum	= (float)pHero->nArmyDeathNum*fAttackRestorePer;
						}
						break;
					case direction_right:
						{
							pHero					= m_mapDefenseHero.getptr(nHeroID);
							if (pHero == NULL)
							{
								return -13;
							}
							pHero->nArmyDeathNum	= (float)(pHero->nProf>nLeftForce?pHero->nProf-nLeftForce:0)/(float)pHero->nProf*(float)pHero->nArmyNum;
							pHero->nArmyRestoreNum	= (float)pHero->nArmyDeathNum*fDefenseRestorePer;
						}
						break;
					}

					szOne[0]		= 0;
				}

				// 4.战报
				dtu.Mark();
				m_vectCombatLog.reserve(sizeof(AlliInstanceCombatLog) 
					+ m_mapAttackHero.size()*sizeof(AlliInstanceCombatLog::HeroUnit)
					+ m_mapDefenseHero.size()*sizeof(AlliInstanceCombatLog::HeroUnit)
					+ sizeof(AlliInstanceCombatLog::LootUnit));
				m_vectCombatLog.resize(sizeof(AlliInstanceCombatLog));
				AlliInstanceCombatLog*	pCombatLog	= (AlliInstanceCombatLog*)m_vectCombatLog.getbuf();
				pCombatLog->nAccountID				= nAccountID;
				pCombatLog->nCombatResult			= nCombatResult;
				pCombatLog->nCombatType				= nCombatType;
				pCombatLog->nObjID					= nObjID;
				pCombatLog->nRandSeed				= nRand;
				pCombatLog->nAttackHeroNum			= m_mapAttackHero.size();
				pCombatLog->nDefenseHeroNum			= m_mapDefenseHero.size();
				pCombatLog->nLootNum				= 0;
				pCombatLog->fCounterRate			= m_cfginfo.fCounterRate;
				// (1)攻击方战报
				it	= m_mapAttackHero.begin();
				while (m_mapAttackHero.end() != it)
				{
					AlliInstanceCombatLog::HeroUnit&	srcHero	= it.getvalue();
					AlliInstanceCombatLog::HeroUnit*	pHero	= (AlliInstanceCombatLog::HeroUnit*)m_vectCombatLog.pushn_back(sizeof(AlliInstanceCombatLog::HeroUnit));
					memcpy(pHero, &srcHero, sizeof(srcHero));

					int	nRealDeathNum		= pHero->nArmyDeathNum>pHero->nArmyRestoreNum?pHero->nArmyDeathNum-pHero->nArmyRestoreNum:0;
					int	nLength				= sprintf(szOne, "*%"WHINT64PRFX"d*%d*%d", pHero->nHeroID, nRealDeathNum, pHero->nArmyNum);
					m_vectAttackResult.pushn_back(szOne, nLength);

					++it;
				}
				m_vectAttackResult.push_back(0);		// 最后面加0结尾(字符串所需)

				// (2)防守方战报
				it	= m_mapDefenseHero.begin();
				while (m_mapDefenseHero.end() != it)
				{
					AlliInstanceCombatLog::HeroUnit&	srcHero	= it.getvalue();
					AlliInstanceCombatLog::HeroUnit*	pHero	= (AlliInstanceCombatLog::HeroUnit*)m_vectCombatLog.pushn_back(sizeof(AlliInstanceCombatLog::HeroUnit));
					memcpy(pHero, &srcHero, sizeof(srcHero));

					int	nRealDeathNum		= pHero->nArmyDeathNum>pHero->nArmyRestoreNum?pHero->nArmyDeathNum-pHero->nArmyRestoreNum:0;
					int	nLength				= sprintf(szOne, "*%"WHINT64PRFX"d*%d*%d", pHero->nHeroID, nRealDeathNum, pHero->nArmyNum);
					m_vectDefenseResult.pushn_back(szOne, nLength);

					++it;
				}
				m_vectDefenseResult.push_back(0);		// 最后面加0结尾(字符串所需)

				// (3)掉落
				int		nLootNum			= 0;
				char	szLootList[128]		= "";
				if (nCombatResult == combat_result_win)
				{
					switch (nCombatType)
					{
					case combat_type_alli_instance:
						{
							q.FreeResult();
							q.SpawnQuery("CALL alli_instance_gen_loot(0x%"WHINT64PRFX"X,%d)", nObjID, nCombatType);
							nPreRet	= 0;
							q.ExecuteSPWithResult(nPreRet);

							if (nPreRet != MYSQL_QUERY_NORMAL)
							{
								return -20;
							}
							else
							{
								if (q.FetchRow())
								{
									unsigned int	nExcelID	= q.GetVal_32();
									unsigned int	nNum		= q.GetVal_32();
									if (nNum>0)
									{
										int	nLength		= sprintf(szLootList, "%d*%d", nExcelID, nNum);
										szLootList[nLength]	= 0;

										nLootNum++;
										AlliInstanceCombatLog::LootUnit*	pLoot	= (AlliInstanceCombatLog::LootUnit*)m_vectCombatLog.pushn_back(sizeof(AlliInstanceCombatLog::LootUnit));
										pLoot->nType		= AlliInstanceCombatLog::LootUnit::loot_type_item;
										pLoot->nExtData		= nExcelID;
										pLoot->nNum			= nNum;
									}
								}
								else
								{
									return -21;
								}
							}
						}
						break;
					}
				}

				// 前面的操作可能已经导致pCombatLog指针失效了
				pCombatLog					= (AlliInstanceCombatLog*)m_vectCombatLog.getbuf();
				pCombatLog->nLootNum		= nLootNum;

				q.FreeResult();
				q.SpawnQuery("CALL deal_alli_instance_combat_event(0x%"WHINT64PRFX"X,%d,'%s','%s','%s','"
					, nEventID, nCombatResult, m_vectAttackResult.getbuf(), m_vectDefenseResult.getbuf(), szLootList);
				q.BinaryToString(m_vectCombatLog.size(), m_vectCombatLog.getbuf());
				q.StrMove("')");
				
				nPreRet	= 0;
				q.ExecuteSPWithResult(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					return -100;
				}
				else
				{
					if (q.FetchRow())
					{
						int	nResult	= q.GetVal_32();
					}
					else
					{
						return -101;
					}
				}
				dtu.Mark();

				if (dtu.GetDiff() >= 200)
				{
					GLOGGER2_WRITEFMT(GLOGGER_ID_ERROR, GLGR_STD_HDR(,BIGTICK)"%s,%s", __FUNCTION__, dtu.Disp());
				}
			}
			else
			{
				return 0;
			}
		}
	}
	return 1;
}
const char*		CCombatLogic::FakeCombat()
{
	static	whvector<char>	vectResultBuf;
	vectResultBuf.resize(0);

	whhash<tty_id_t, AlliInstanceCombatLog::HeroUnit>::kv_iterator	it;
	unsigned int	nAttackTotalForce	= 0;
	unsigned int	nDefenseTotalForce	= 0;
	char			szBuf[256]			= "";
	int				nLength				= 0;
	unsigned int	nCombatResult		= combat_result_none;

	it	= m_mapAttackHero.begin();
	for (; it!=m_mapAttackHero.begin(); ++it)
	{
		nAttackTotalForce	+= it.getvalue().nProf;
	}

	it	= m_mapDefenseHero.begin();
	for (; it!=m_mapDefenseHero.begin(); ++it)
	{
		nDefenseTotalForce	+= it.getvalue().nProf;
	}

	nCombatResult	= nAttackTotalForce>nDefenseTotalForce?combat_result_win:combat_result_lose;
	nLength			= sprintf(szBuf, "%d", nCombatResult);
	vectResultBuf.push_back(szBuf, nLength);

	it	= m_mapAttackHero.begin();
	for (; it!=m_mapAttackHero.begin(); ++it)
	{
		AlliInstanceCombatLog::HeroUnit&	unit	= it.getvalue();
		nLength		= sprintf(szBuf, ",0x%"WHINT64PRFX"X*%d*%d", unit.nHeroID, unit.nProf, direction_left);
		vectResultBuf.push_back(szBuf, nLength);
	}

	it	= m_mapDefenseHero.begin();
	for (; it!=m_mapDefenseHero.begin(); ++it)
	{
		AlliInstanceCombatLog::HeroUnit&	unit	= it.getvalue();
		nLength		= sprintf(szBuf, ",0x%"WHINT64PRFX"X*%d*%d", unit.nHeroID, unit.nProf, direction_right);
		vectResultBuf.push_back(szBuf, nLength);
	}

	vectResultBuf.push_back(0);
	return vectResultBuf.getbuf();
}
