#include "WHCMN/inc/whdir.h"
#include "WHCMN/inc/wh_package_fileman.h"
#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whdataprop.h"
#include "WHCMN/inc/whcsm.h"

using namespace n_whcmn;

struct CFGINFO_T : public whdataini::obj 
{
	char				szPass[32+1];
	char				szMBDir[WH_MAX_PATH];			// 服务器也需要读取MB表格(主要用于拼接字串等)

	CFGINFO_T()
	{
		WH_STRNCPY0(szPass, "test");
		WH_STRNCPY0(szMBDir, "/srv/www/htdocs/update/");
	}
	WHDATAPROP_DECLARE_MAP(CFGINFO_T);
}	g_cfginfo;
WHDATAPROP_MAP_BEGIN_AT_ROOT(CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szPass, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMBDir, 0)
WHDATAPROP_MAP_END()

const char*	g_cszCfgName		= "decode_cfg.txt";

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		printf("error param;XDecodeExcelTalbe filename%s", WHLINEEND);
		return -1;
	}

	WHDATAINI_CMN		ini;
	ini.addobj("DecodeExcelTable", &g_cfginfo);
	int	nRst	= ini.analyzefile(g_cszCfgName);
	if (nRst < 0)
	{
		printf("Can not analyze cfgfile:%s rst:%d %s%s", g_cszCfgName, nRst, ini.printerrreport(), WHLINEEND);
		return -1;
	}

	char	szFileFullPath[WH_MAX_PATH];
	const char*	cszFileName	= szFileFullPath;
	int		nLen			= sprintf(szFileFullPath, "%s%s", g_cfginfo.szMBDir, argv[1]);
	szFileFullPath[nLen]	= 0;

	// 创建文件管理器
	whfileman_package_INFO_Easy_T	infoeasy;
	infoeasy.nSearchOrder			= whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2;
	infoeasy.nOPMode				= whfileman::OP_MODE_BIN_READONLY;
	infoeasy.nPassLen				= strlen(g_cfginfo.szPass);
	memcpy(infoeasy.szPass, g_cfginfo.szPass, infoeasy.nPassLen);
	whfileman	*pFM				= whfileman_package_Create_Easy(&infoeasy);
	WHCMN_FILEMAN_SET(pFM);

	// 使用的一些缓存,防止频繁分配内存
	whvector<unsigned char>		vectFileBuf;
	whvector<unsigned char>		vectFileDecryptBuf;
	whvector<unsigned char>		vectKeyBuf;
	whvector<unsigned char>		vectKeyDecryptBuf;
	whvector<unsigned char>		vectNKeyDecryptBuf;
	
	vectFileBuf.resize(512*1024);
	vectFileDecryptBuf.resize(1024*1024);
	int	nKeyLen		= strlen(g_cfginfo.szPass);
	vectKeyBuf.resize(nKeyLen);
	memcpy(vectKeyBuf.getbuf(), g_cfginfo.szPass, nKeyLen);
	vectKeyDecryptBuf.resize(nKeyLen*2);
	vectNKeyDecryptBuf.resize(nKeyLen*2);

	// 读取文件
	whfile*	pFile	= whfile_OpenCmnFile(cszFileName, "rb");
	if (pFile == NULL)
	{
		printf("file not exist;%s%s", cszFileName, WHLINEEND);
		return -2;
	}
	vectFileBuf.resize(pFile->FileSize());
	pFile->Read(vectFileBuf.getbuf(), pFile->FileSize());

	// 解密
	csm		objCrypt;
	vectFileDecryptBuf.resize(vectFileBuf.size()*2);
	objCrypt.decode2(vectFileBuf.getbuf(), vectFileBuf.size(), vectKeyBuf.getbuf(), vectKeyBuf.size(), vectFileDecryptBuf.getbuf(), vectKeyDecryptBuf.getbuf(), vectNKeyDecryptBuf.getbuf());
	vectFileBuf.push_back(0);

	printf("%s%s", vectFileBuf.getbuf(), WHLINEEND);

	whfile_CloseCmnFile(pFile);

	return 0;
}
