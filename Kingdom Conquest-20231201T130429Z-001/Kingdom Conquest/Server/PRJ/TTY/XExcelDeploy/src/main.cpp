#include "../inc/ExcelDeployHelper.h"

const char*	cszPackagePass	= "tianxiawudi";

int	main(int argc, char* argv[])
{
	char	szCFG[WH_MAX_PATH]		= "excel_deploy_cfg.txt";
	if (argc == 2)
	{
		strcpy(szCFG, argv[1]);
	}

	whdir_SetCWDToExePath();

	CExcelDeployHelper	deploy;
	int		nRst		= 0;
	if ((nRst=deploy.Init(szCFG)) < 0)
	{
		printf("CExcelDeployHelper.Init,%d%s", nRst, WHLINEEND);
		return -1;
	}
	deploy.DeployExcelTable();

	return 0;
}
