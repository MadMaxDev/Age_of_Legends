#include "../inc/ExcelDeployHelper.h"

WHDATAPROP_MAP_BEGIN_AT_ROOT(CExcelDeployHelper::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(bool, bDeployToMySQL, 0)
	WHDATAPROP_ON_SETVALUE_smp(bool, bDeployToWebUpdate, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szSrcDir, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szWebUpdateDir, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szPackagePass, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMBDir, 0)
	WHDATAPROP_ON_SETVALUE_smp(unknowntype, MYSQLInfo, 0)
WHDATAPROP_MAP_END()

WHDATAPROP_MAP_BEGIN_AT_ROOT(CExcelDeployHelper::CFGINFO_T::MySQLInfo)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlHost, 0)
	WHDATAPROP_ON_SETVALUE_smp(int, nMysqlPort, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlUser, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlPass, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlDB, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlCharset, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szMysqlSocket, 0)
WHDATAPROP_MAP_END()

CExcelDeployHelper::CExcelDeployHelper()
: m_pMySQLHandler(NULL)
, m_pQuery(NULL)
{
	m_szMD5[0]		= 0;
}
inline void SAFE_CLOSE_MYSQL(MYSQL*& pMySQL)
{
	if (pMySQL != NULL)
	{
		mysql_close(pMySQL);
		pMySQL	= NULL;
	}
}
CExcelDeployHelper::~CExcelDeployHelper()
{
	SAFE_CLOSE_MYSQL(m_pMySQLHandler);
	WHSafeDelete(m_pQuery);
}

int		CExcelDeployHelper::Init(const char* cszCFGName)
{
	WHDATAINI_CMN		ini;
	ini.addobj("ExcelDeploy", &m_cfginfo);
	int	nRst	= ini.analyzefile(cszCFGName);
	if (nRst < 0)
	{
		printf("Can not analyze cfgfile:%s rst:%d %s%s", cszCFGName, nRst, ini.printerrreport(), WHLINEEND);
		return -1;
	}
	if (m_cfginfo.bDeployToMySQL)
	{
		m_pMySQLHandler	= mysql_init(NULL);
		int		nTimeoutOption	= 1;
		mysql_options(m_pMySQLHandler, MYSQL_OPT_CONNECT_TIMEOUT, (const char*)&nTimeoutOption);

		CFGINFO_T::MySQLInfo&	mysqlInfo	= m_cfginfo.MYSQLInfo;
		unsigned long			mysqlFlag	= CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS;
		if (NULL == mysql_real_connect(m_pMySQLHandler, mysqlInfo.szMysqlHost, mysqlInfo.szMysqlUser, mysqlInfo.szMysqlPass, 
			mysqlInfo.szMysqlDB, mysqlInfo.nMysqlPort, mysqlInfo.szMysqlSocket, mysqlFlag))
		{
			SAFE_CLOSE_MYSQL(m_pMySQLHandler);
			return -2;
		}
		if (0 != mysql_set_character_set(m_pMySQLHandler, mysqlInfo.szMysqlCharset))
		{
			SAFE_CLOSE_MYSQL(m_pMySQLHandler);
			return -3;
		}

		m_pQuery	= new dia_mysql_query(m_pMySQLHandler, QUERY_LEN);
		if (m_pQuery == NULL)
		{
			return -4;
		}
	}

	m_vectIndexFileBuf.resize(512*1024);

	m_vectFileBuf.resize(512*1024);
	m_vectFileCryptBuf.resize(1024*1024);
	int	nKeyLen		= strlen(m_cfginfo.szPackagePass);
	m_vectKeyBuf.resize(nKeyLen);
	m_vectKeyCryptBuf.resize(nKeyLen*2);

	return 0;
}

int		CExcelDeployHelper::DeployExcelTable()
{
	// 创建文件管理器
	whfileman_package_INFO_Easy_T	infoeasy;
	infoeasy.nSearchOrder			= whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2;
	infoeasy.nOPMode				= whfileman::OP_MODE_BIN_READONLY;
	infoeasy.nPassLen				= strlen(m_cfginfo.szPackagePass);
	memcpy(infoeasy.szPass, m_cfginfo.szPackagePass, infoeasy.nPassLen);
	whfileman	*pFM				= whfileman_package_Create_Easy(&infoeasy);
	WHCMN_FILEMAN_SET(pFM);

	CExcelTableMng*	pExcelTableMng	= new CExcelTableMng();
	if (!pExcelTableMng->LoadTables(m_cfginfo.szSrcDir, 0))
	{
		printf("load tables error:%s%s", m_cfginfo.szSrcDir, WHLINEEND);
		return 0;
	}

	CExcelTable*	pIndexTable	= pExcelTableMng->GetIndexTable();
	// 看需要生成内存文件否
	whfile*	pMemIndexFile	= NULL;
	int		nMemFileSize	= 0;
	if (m_cfginfo.bDeployToWebUpdate)
	{
		pMemIndexFile		= whfile_OpenMemBufFile((char*)m_vectIndexFileBuf.getbuf(), m_vectIndexFileBuf.size());
		vector<string>		vectColNames;
		pIndexTable->GetColNames(vectColNames);
		for (int i=0; i<vectColNames.size(); i++)
		{
			nMemFileSize	+= pMemIndexFile->Write(vectColNames[i].c_str(), vectColNames[i].length());
			nMemFileSize	+= pMemIndexFile->Write("\t", 1);
		}
		nMemFileSize	+= pMemIndexFile->Write(WHLINEEND, strlen(WHLINEEND));
	}
	if (m_cfginfo.bDeployToMySQL)
	{
		// 先把index_file部署一下
		// 将mb发布到本地服务器读取目录
		string	strFileName			= "index_list.txt";
		string	strSrcFilePath		= m_cfginfo.szSrcDir + strFileName;
		string	strDstFilePath		= m_cfginfo.szMBDir + strFileName;

		whfile*	pSrcFile	= whfile_OpenCmnFile(strSrcFilePath.c_str(), "rb");
		if (pSrcFile == NULL)
		{
			printf("can not open file,file path:%s%s", strSrcFilePath.c_str(), WHLINEEND);
			return 0;
		}
		whdir_SureMakeDirForFile(strDstFilePath.c_str());
		whfile*	pDstFile	= whfile_OpenCmnFile(strDstFilePath.c_str(), "wb");
		if (pDstFile == NULL)
		{
			printf("can not create new file,file path:%s%s", strDstFilePath.c_str(), WHLINEEND);
			whfile_CloseCmnFile(pSrcFile);
			return 0;
		}

		m_vectFileBuf.resize(pSrcFile->FileSize());
		pSrcFile->Read(m_vectFileBuf.getbuf(), m_vectFileBuf.size());
		pDstFile->Write(m_vectFileBuf.getbuf(), m_vectFileBuf.size());

		whfile_CloseCmnFile(pSrcFile);
		whfile_CloseCmnFile(pDstFile);
	}
	int	nLineNum	= pIndexTable->GetCount();
	for (int i=0; i<nLineNum; i++)
	{
		SExcelLine*	pIndexLine	= pIndexTable->GetLineFromList(i);
		int		nTableID		= pIndexLine->SafeGetInt(index_col_table_id);
		int		nUseType		= pIndexLine->SafeGetInt(index_col_use_type);

		// 布置到数据库
		if (m_cfginfo.bDeployToMySQL 
			&& (nUseType==use_type_all || nUseType==use_type_server))
		{
			vector<int>	vecColType;
			pIndexLine->SafeGetArray(index_col_column_type, vecColType, true);
			string		strDBTable;
			pIndexLine->SafeGetStr(index_col_mysql_table, strDBTable, true);

			if (!IsWhiteSpaceString(strDBTable.c_str(), strDBTable.length()))
			{
				char	szDeleteSQL[1024]	= "";
				sprintf(szDeleteSQL, "DELETE FROM %s", strDBTable.c_str());
				m_pQuery->SpawnQuery(szDeleteSQL);
				int		nPreRet				= 0;
				m_pQuery->Execute(nPreRet);
				if (nPreRet != MYSQL_QUERY_NORMAL)
				{
					printf("execute error:%d.%s;%s%s", m_pQuery->GetErrno(), m_pQuery->GetError(), szDeleteSQL, WHLINEEND);
				}

				CExcelTable*	pTable	= pExcelTableMng->GetTable(nTableID);
				int				nCount	= pTable->GetCount();		// 表的行数
				for (int j=0; j<nCount; j++)
				{
					SExcelLine*	pLine		= pTable->GetLineFromList(j);
					char	szBuf[2048]		= "";
					int		n				= sprintf(szBuf , "INSERT INTO %s VALUES (", strDBTable.c_str());
					char*	pPos			= szBuf + n;
					for (int k=0; k<vecColType.size(); k++)
					{
						n	= 0;
						switch (vecColType[k])
						{
						case DEF_INT:
							{
								n		= sprintf(pPos, "%d,", pLine->SafeGetInt(k, true));
							}
							break;
						case DEF_STR:
							{
								char	szTmpBuf[256];
								const char*	pStr	= pLine->FastGetStr(k, true);
								mysql_real_escape_string(m_pMySQLHandler, szTmpBuf, pStr, strlen(pStr));
								n		= sprintf(pPos, "'%s',", szTmpBuf);
							}
							break;
						case DEF_FLOAT:
							{
								n		= sprintf(pPos, "%f,", pLine->SafeGetFloat(k, true));
							}
							break;
						}
						pPos	+= n;
					}
					*(--pPos)	= 0;
					strcat(pPos, ")");

					m_pQuery->SpawnQuery(szBuf);
					m_pQuery->Execute(nPreRet);
					if (nPreRet != MYSQL_QUERY_NORMAL)
					{
						printf("%d.%s;%s%s", m_pQuery->GetErrno(), m_pQuery->GetError(), szBuf, WHLINEEND);
					}
				}
			}

			// 将mb发布到本地服务器读取目录
			string	strFileName;
			pIndexLine->SafeGetStr(index_col_file_name, strFileName, false);
			string	strSrcFilePath		= m_cfginfo.szSrcDir + strFileName + ".txt";
			string	strDstFilePath		= m_cfginfo.szMBDir + strFileName + ".txt";

			whfile*	pSrcFile	= whfile_OpenCmnFile(strSrcFilePath.c_str(), "rb");
			if (pSrcFile == NULL)
			{
				printf("can not open file,table id:%d,file path:%s%s", nTableID, strSrcFilePath.c_str(), WHLINEEND);
				continue;
			}
			whdir_SureMakeDirForFile(strDstFilePath.c_str());
			whfile*	pDstFile	= whfile_OpenCmnFile(strDstFilePath.c_str(), "wb");
			if (pDstFile == NULL)
			{
				printf("can not create new file,table id:%d,file path:%s%s", nTableID, strDstFilePath.c_str(), WHLINEEND);
				whfile_CloseCmnFile(pSrcFile);
				continue;
			}

			m_vectFileBuf.resize(pSrcFile->FileSize());
			pSrcFile->Read(m_vectFileBuf.getbuf(), m_vectFileBuf.size());
			pDstFile->Write(m_vectFileBuf.getbuf(), m_vectFileBuf.size());

			whfile_CloseCmnFile(pSrcFile);
			whfile_CloseCmnFile(pDstFile);
		}
		
		// 布置到web更新目录
		if (m_cfginfo.bDeployToWebUpdate
			&& (nUseType==use_type_all || nUseType==use_type_client))
		{
			string	strFileName;
			pIndexLine->SafeGetStr(index_col_file_name, strFileName, false);
			if (strFileName == "")
			{
				printf("can not find file path,table id:%d%s", nTableID, WHLINEEND);
				continue;
			}
			string strSrcFilePath		= m_cfginfo.szSrcDir + strFileName + ".txt";

			whfile*	pFile	= whfile_OpenCmnFile(strSrcFilePath.c_str(), "rb");
			if (pFile == NULL)
			{
				printf("can not open file,table id:%d,file path:%s%s", nTableID, strSrcFilePath.c_str(), WHLINEEND);
				continue;
			}
			
			m_vectFileBuf.resize(pFile->FileSize());
			pFile->Read(m_vectFileBuf.getbuf(), m_vectFileBuf.size());

			// 文件加密
			{
				// 打开新文件
				string	strNewFilePath	= m_cfginfo.szWebUpdateDir + strFileName + ".txt";
				whdir_SureMakeDirForFile(strNewFilePath.c_str());
				whfile*	pNewFile		= whfile_OpenCmnFile(strNewFilePath.c_str(), "wb");
				if (pNewFile == NULL)
				{
					printf("can not create new file,table id:%d,file path:%s%s", nTableID, strNewFilePath.c_str(), WHLINEEND);
					whfile_CloseCmnFile(pFile);
					continue;
				}
				pNewFile->Write(m_vectFileBuf.getbuf(), m_vectFileBuf.size());
				whfile_CloseCmnFile(pNewFile);
			}

			for (int i=0; i<index_col_checksum; i++)
			{
				string strValue;
				pIndexLine->SafeGetStr(i, strValue, true);
				nMemFileSize	+= pMemIndexFile->Write(strValue.c_str(), strValue.length());
				nMemFileSize	+= pMemIndexFile->Write("\t", 1);
			}
			whmd5str(m_vectFileBuf.getbuf(), m_vectFileBuf.size(), m_szMD5);
			nMemFileSize	+= pMemIndexFile->Write(m_szMD5, strlen(m_szMD5));
			nMemFileSize	+= pMemIndexFile->Write(WHLINEEND, strlen(WHLINEEND));
			whfile_CloseCmnFile(pFile);
		}
	}

	// 生成新的index_file
	if (m_cfginfo.bDeployToWebUpdate)
	{
		string	strNewIndexFilePath	= m_cfginfo.szWebUpdateDir + string("index_list.txt");
		whfile*	pNewIndexFile		= whfile_OpenCmnFile(strNewIndexFilePath.c_str(), "wb");
		m_vectIndexFileBuf.resize(nMemFileSize);
		pNewIndexFile->Write(m_vectIndexFileBuf.getbuf(), m_vectIndexFileBuf.size());
		whfile_CloseCmnFile(pNewIndexFile);

		// 新的md5校验文件 
		string	strNewIndexChecksumFilePath	= m_cfginfo.szWebUpdateDir + string("index_list_checksum.txt");
		whfile*	pNewIndexChecksumFile		= whfile_OpenCmnFile(strNewIndexChecksumFilePath.c_str(), "wb");
		char	szIndexFileChecksum[WHMD5LEN*2 + 1];
		whmd5str(m_vectIndexFileBuf.getbuf(), m_vectIndexFileBuf.size(), szIndexFileChecksum);
		pNewIndexChecksumFile->Write(szIndexFileChecksum, sizeof(szIndexFileChecksum));
		whfile_CloseCmnFile(pNewIndexChecksumFile);
	}
	whfile_CloseMemBufFile(pMemIndexFile);

	// 生成新的server_list.txt
	if (m_cfginfo.bDeployToWebUpdate)
	{
		string	strServerListFilePath	= m_cfginfo.szSrcDir + string("server_list.txt");
		string	strNewServerListFilePath= m_cfginfo.szWebUpdateDir + string("server_list.txt");
		whfile*	pSrcFile	= whfile_OpenCmnFile(strServerListFilePath.c_str(), "rb");
		assert(pSrcFile!=NULL);
		whfile*	pDstFile	= whfile_OpenCmnFile(strNewServerListFilePath.c_str(), "wb");
		assert(pSrcFile!=NULL);

		m_vectIndexFileBuf.resize(pSrcFile->FileSize());
		m_vectFileCryptBuf.reserve(2*pSrcFile->FileSize());
		pSrcFile->Read(m_vectIndexFileBuf.getbuf(), m_vectIndexFileBuf.size());

		pDstFile->Write(m_vectIndexFileBuf.getbuf(), m_vectIndexFileBuf.size());
		whfile_CloseCmnFile(pDstFile);
		whfile_CloseCmnFile(pSrcFile);

		// 新的md5校验文件 
		string	strNewServerListChecksumFilePath	= m_cfginfo.szWebUpdateDir + string("server_list_checksum.txt");
		whfile*	pNewServerListChecksumFile			= whfile_OpenCmnFile(strNewServerListChecksumFilePath.c_str(), "wb");
		char	szIndexFileChecksum[WHMD5LEN*2 + 1];
		whmd5str(m_vectIndexFileBuf.getbuf(), m_vectIndexFileBuf.size(), szIndexFileChecksum);
		pNewServerListChecksumFile->Write(szIndexFileChecksum, sizeof(szIndexFileChecksum));
		whfile_CloseCmnFile(pNewServerListChecksumFile);
	}

	// ReadOneFile();
	return 0;
}
