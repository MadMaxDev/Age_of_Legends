#ifndef __ExcelDeployHelper_H__
#define __ExcelDeployHelper_H__

#include "GameUtilityLib/inc/SoloFileFunc.h"
#include "WHCMN/inc/whdir.h"
#include "WHCMN/inc/wh_package_fileman.h"
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whdataprop.h"
#include "DIA/inc/numeric_type.h"
#include "DIA/inc/st_mysql_query.h"
#include "../../Common/inc/tty_common_excel_def.h"
#include "../../Common/inc/tty_common_def.h"

using namespace n_whcmn;
using namespace n_pngs;

#include <vector>
using namespace std;

class CExcelDeployHelper
{
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		bool				bDeployToMySQL;					// 是否需要安装到数据库
		bool				bDeployToWebUpdate;				// 是否需要安装到web的更新目录
		char				szSrcDir[WH_MAX_PATH];			// excel表格原始目录
		char				szWebUpdateDir[WH_MAX_PATH];	// web更新目录
		char				szPackagePass[32+1];
		char				szMBDir[WH_MAX_PATH];		// 服务器也需要读取MB表格(主要用于拼接字串等)

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
				strcpy(szMysqlPass, "pixeldb2006");
				strcpy(szMysqlDB, "xunxian");
				strcpy(szMysqlCharset, "gbk");
				strcpy(szMysqlSocket, "/tmp/mysql.sock");
			}
			WHDATAPROP_DECLARE_MAP(MySQLInfo);
		}	MYSQLInfo;
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);

		CFGINFO_T()
		: bDeployToMySQL(false)
		, bDeployToWebUpdate(false)
		{
			szSrcDir[0]				= 0;
			szWebUpdateDir[0]		= 0;
			szMBDir[0]				= 0;
			WH_STRNCPY0(szPackagePass, "1c705c602d095075cff92b16a9b610bb");	// EasyGame_From2013的MD5
		}
	}	m_cfginfo;
public:
	CExcelDeployHelper();
	~CExcelDeployHelper();
public:
	int		Init(const char* cszCFGName);
	int		DeployExcelTable();
private:
	bool	IsWhiteSpaceString(const char* cszStr, int nLength)
	{
		for (int i=0; i<nLength; i++)
		{
			if (strchr(WHSTR_WHITESPACE, (int)cszStr[i]) == NULL)
			{
				return false;
			}
		}
		return true;
	}
private:
	MYSQL*				m_pMySQLHandler;
	dia_mysql_query*	m_pQuery;

	char				m_szMD5[2*WHMD5LEN+1];
	// 索引文件缓冲
	whvector<unsigned char>		m_vectIndexFileBuf;

	// 使用的一些缓存,防止频繁分配内存
	whvector<unsigned char>		m_vectFileBuf;
	whvector<unsigned char>		m_vectFileCryptBuf;
	whvector<unsigned char>		m_vectKeyBuf;
	whvector<unsigned char>		m_vectKeyCryptBuf;

//	int ReadOneFile();
};

#endif