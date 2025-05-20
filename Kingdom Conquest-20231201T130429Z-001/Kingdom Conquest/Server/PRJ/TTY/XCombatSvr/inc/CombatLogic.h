#ifndef __CombatLogic_H__
#define __CombatLogic_H__
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whtime.h"
#include "WHCMN/inc/whvector.h"
#include "WHCMN/inc/whhash.h"
#include "WHCMN/inc/whstring.h"
#include "DIA/inc/st_mysql_query.h"
#include "../../Common/inc/tty_common_PlayerData4Web.h"
#include "../../Common/inc/tty_common_db_timeevent.h"

#include "MonoHelper.h"

using namespace n_whcmn;
using namespace n_pngs;

#define		QUERY_LEN			81920

class CCombatLogic
{
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		char	szBattleSoFileName[WH_MAX_PATH];
		int		nCombatRand;		// 战斗随机数
		bool	bLogCombat;
		float	fCounterRate;
		bool	bFakeCombat;		// mono战斗逻辑出错是否使用虚拟战斗

		struct MySQLInfo : public whdataini::obj 
		{
			char	szMysqlHost[dia_string_len];
			int		nMysqlPort;
			char	szMysqlUser[dia_string_len];
			char	szMysqlPass[dia_string_len];
			char	szMysqlDB[dia_string_len];
			char	szMysqlCharset[dia_string_len];
			char	szMysqlSocket[dia_string_len];

			MySQLInfo()
			{
				strcpy(szMysqlHost, "localhost");
				nMysqlPort		= 3306;
				strcpy(szMysqlUser, "root");
				strcpy(szMysqlPass, "13974268");
				strcpy(szMysqlDB, "web_3");
				strcpy(szMysqlCharset, "utf8");
				strcpy(szMysqlSocket, "/tmp/mysql.sock");
			}
			WHDATAPROP_DECLARE_MAP(MySQLInfo);
		}	MYSQLInfo;

		CFGINFO_T()
			: nCombatRand(0)
			, bLogCombat(false)
			, fCounterRate(1.5f)
			, bFakeCombat(true)
		{
			WH_STRNCPY0(szBattleSoFileName, "ServerBattle.so");
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	} m_cfginfo;
public:
	CCombatLogic();
	~CCombatLogic();
	int		Init(const char* cszCFGName);
	int		Init_CFG(const char* cszCFGName);
	int		Init_CFG_Reload(const char* cszCFGName);
	int		Work();
private:
	MYSQL*	OpenMySQL();
	void	CloseMySQL(MYSQL*& pMySQL);
	bool	IsMySQLHandlerValid(MYSQL* pMySQL, bool bCloseInvalidHandler=true);
private:
	const char*	FakeCombat();
private:
	whtick_t	m_tickNow;
	MYSQL*		m_pMySQL;
	whvector<char>	m_vectAttackData;
	whvector<char>	m_vectDefenseData;
	whvector<char>	m_vectAttackResult;
	whvector<char>	m_vectDefenseResult;
	whvector<char>	m_vectCombatLog;

	whvector<char>	m_vectMonoParamAttack;
	whvector<char>	m_vectMonoParamDefense;

	whhash<tty_id_t, AlliInstanceCombatLog::HeroUnit>		m_mapAttackHero;
	whhash<tty_id_t, AlliInstanceCombatLog::HeroUnit>		m_mapDefenseHero;

	CMonoHelper	m_monoHelper;
private:
	// 阵营
	enum
	{
		direction_left			= 1,
		direction_right			= 2,
	};
};
#endif
