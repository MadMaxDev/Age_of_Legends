#include "../inc/LotteryMngS.h"

#include "GameUtilityLib/inc/SoloFileFunc.h"
#include "WHCMN/inc/whdir.h"
#include "WHCMN/inc/wh_package_fileman.h"
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whdataprop.h"

using namespace n_whcmn;
using namespace n_pngs;

const char*	cszPackagePass	= "tianxiawudi";

struct CFG_INFO : public whdataini::obj
{
	char	szMBPath[WH_MAX_PATH];

	CFG_INFO()
	{
		WH_STRNCPY0(szMBPath, "../excel_tables/");
	}

	WHDATAPROP_DECLARE_MAP(CFG_INFO);
}g_cfg;

WHDATAPROP_MAP_BEGIN_AT_ROOT(CFG_INFO)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMBPath, 0)
WHDATAPROP_MAP_END()

void PrintMenu()
{
	printf("---------------------\n");
	printf("1.draw a lottery;\n");
	printf("2.exit.\n");
	printf("---------------------\n");
}

int	main(int argc, char* argv[])
{
	char	szCFG[WH_MAX_PATH]		= "lottery_simulator.txt";
	if (argc == 2)
	{
		strcpy(szCFG, argv[1]);
	}

	whdir_SetCWDToExePath();

	// 1.分析配置文件
	WHDATAINI_CMN		ini;
	ini.addobj("CFG_INFO", &g_cfg);
	int	nRst	= ini.analyzefile(szCFG);
	if (nRst < 0)
	{
		printf("Can not analyze cfgfile:%s rst:%d %s%s", szCFG, nRst, ini.printerrreport(), WHLINEEND);
		return -1;
	}
	// 2.载入MB表格
	// 创建文件管理器
	whfileman_package_INFO_Easy_T	infoeasy;
	infoeasy.nSearchOrder			= whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2;
	infoeasy.nOPMode				= whfileman::OP_MODE_BIN_READONLY;
	infoeasy.nPassLen				= strlen(cszPackagePass);
	memcpy(infoeasy.szPass, cszPackagePass, infoeasy.nPassLen);
	whfileman	*pFM				= whfileman_package_Create_Easy(&infoeasy);
	WHCMN_FILEMAN_SET(pFM);
	if (!g_ExcelTableMng.LoadTables(g_cfg.szMBPath, 1))
	{
		printf("load tables error:%s%s", g_cfg.szMBPath, WHLINEEND);
		return -2;
	}

	CLotteryMngS		lotteryMng;
	if ((nRst=lotteryMng.Init()) < 0)
	{
		printf("CLotteryMngS.Init,%d%s", nRst, WHLINEEND);
		return -1;
	}
	int		nChoice		= 0;
choose:
	PrintMenu();
	scanf("%d", &nChoice);
	switch (nChoice)
	{
	case 1:
		{
			lotteryMng.DrawOnceLottery();
			goto choose;
		}
		break;
	case 2:
		{
			exit(0);
		}
		break;
	default:
		{
			goto choose;
		}
	}
	return 0;
}
