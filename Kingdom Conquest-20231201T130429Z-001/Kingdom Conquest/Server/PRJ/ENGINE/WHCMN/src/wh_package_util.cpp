// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_util.cpp
// Creator     : Wei Hua (魏华)
// Comment     : 包的相关操作
// CreationDate: 2006-08-14
// ChangeLog	:	2008-08-22	为手动更新包增加版本验证机制，by 钟文杰
//					2008-08-22	修正错误：使用手动更新包创建了新包，但读取配置文件时出错会导致新包数据错误的问题，by 钟文杰
//					2008-10-22	手动更新包的版本验证条件修改为只要不低于预设值即可通过，by 钟文杰

#include "../inc/wh_package_util.h"
#include "../inc/wh_package_fileman_i.h"
#include "../inc/whdataini.h"

using namespace n_whcmn;

namespace
{

struct	l_notify_cosole	: public wh_package_util::notify
{
	bool			m_bDSIsSet;
	DIR_STAT_T		m_ds;
	int				m_nFileProcessed;
	int				m_nByteProcessed;
	l_notify_cosole()
		: m_bDSIsSet(false)
		, m_nFileProcessed(0)
		, m_nByteProcessed(0)
	{
	}
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		cb_DIR_BEGIN(const char *cszDirName)
	{
		printf(">> Begin processing dir:%s ...%s", cszDirName, WHLINEEND);
		return	RST_OK_CONTINUE;
	}
	virtual	int		cb_DIR_END(const char *cszDirName)
	{
		printf("<< End processing dir:%s%s", cszDirName, WHLINEEND);
		return	RST_OK_CONTINUE;
	}
	virtual	int		cb_FILE_BEGIN(const char *cszFrom, const char *cszTo, int nFileSize)
	{
		printf("     Begin processing file from:%s to:%s size:%d ...", cszFrom, cszTo, nFileSize);
		if( m_bDSIsSet )
		{
			m_nFileProcessed	++;
			m_nByteProcessed	+= nFileSize;
			printf("[file:%d/%d byte:%d/%d] ...", m_nFileProcessed, m_ds.nTotalFile, m_nByteProcessed, m_ds.nTotalFileByte);
		}
		return	RST_OK_CONTINUE;
	}
	virtual	int		cb_FILE_END()
	{
		printf(" OVER.%s", WHLINEEND);
		return	RST_OK_CONTINUE;
	}
	virtual	int		cb_FILE_PROCESS(int nCurPos)
	{
		printf(".");
		return	RST_OK_CONTINUE;
	}
	virtual	int		cb_FILE_SKIP(const char *cszFName)
	{
		printf("     * Skip file:%s", cszFName);
		return	RST_OK_CONTINUE;
	}
	virtual	int		cb_ERROR(int nErrCode, const char *cszErrString)
	{
		printf("ERROR:%d %s%s", nErrCode, cszErrString, WHLINEEND);
		return	RST_ERR_ABORT;
	}
	virtual int		cb_MSGOUT(int nMsgCode, const char *cszMsg)
	{
		printf("MSGOUT:%d %s%s", nMsgCode, cszMsg, WHLINEEND);
		return	RST_OK_CONTINUE;
	}
	virtual int		cb_SRCDIRSTAT(const DIR_STAT_T *pInfo)
	{
		m_bDSIsSet	= true;
		memcpy(&m_ds, pInfo, sizeof(m_ds));
		printf("SRCDIRSTAT totalfile:%d byte:%d dir:%d%s", pInfo->nTotalFile, pInfo->nTotalFileByte, pInfo->nTotalDir, WHLINEEND);
		return 0;
	}
};

struct	l_dir2package	: public wh_package_util::dir2package
{
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
	virtual	int		DoIt(INFO_T *pInfo)
	{
		// 可以处理包的文件管理器
		whfileman_package_INFO_T	info;
		info.nSearchOrder			= whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY;
		info.bAutoCloseFM			= false;
		info.pFM					= pInfo->pFM;
		info.nOPMode				= whfileman::OP_MODE_BIN_CREATE;
		memcpy(info.szPass, pInfo->szPass, pInfo->nPassLen);
		info.nPassLen				= pInfo->nPassLen;
		info.nBlockSize				= pInfo->nBlockSize;
		info.pWHCompress			= pInfo->pWHCompress;
		info.pEncryptor				= pInfo->pEncryptor;
		info.pDecryptor				= pInfo->pDecryptor;
		wh_package_fileman			*pPFM	= new wh_package_fileman(&info);
		wh_package_util::dircp		*pCP	= wh_package_util::dircp::Create();
		try
		{
			// 创建空包文件
			int	rst	= pPFM->SureOpenPackage(pInfo->cszDst, wh_package_fileman::CSZPCKEXT, whfileman::OP_MODE_BIN_CREATE);
			if( rst<0 )
			{
				throw	-10000 + rst;
			}
			if( pInfo->cszSrc!=NULL && strcmp(pInfo->cszSrc, "NULL")!=0 )
			{
				// 浏览目录（通过pInfo->pFM），一个一个把文件拷贝到包中（fm）
				wh_package_util::dircp::INFO_T	cpinfo;
				cpinfo.pFMSrc				= pInfo->pFM;
				cpinfo.pFMDst				= pPFM;
				cpinfo.cszSrcDir			= pInfo->cszSrc;
				cpinfo.getcmninfofrom(pInfo);
				rst	= pCP->DoIt(&cpinfo);
			}
			WHSafeSelfDestroy(pCP);
			WHSafeSelfDestroy(pPFM);
			return	rst;
		}
		catch (int nErr)
		{
			WHSafeSelfDestroy(pCP);
			WHSafeSelfDestroy(pPFM);
			return	nErr;
		}
	}
};

struct	l_package2dir	: public wh_package_util::package2dir
{
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
	virtual	int		DoIt(INFO_T *pInfo)
	{
		// 先确保包是存在的
		if( wh_package_fileman::IsPathPackage(pInfo->pFM, pInfo->cszSrc)==NULL )
		{
			return	-10001;
		}
		// 可以处理包的文件管理器
		whfileman_package_INFO_T	info;
		info.nSearchOrder			= whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY;
		info.bAutoCloseFM			= false;
		info.pFM					= pInfo->pFM;
		info.nOPMode				= whfileman::OP_MODE_BIN_READONLY;
		memcpy(info.szPass, pInfo->szPass, pInfo->nPassLen);
		info.nPassLen				= pInfo->nPassLen;
		info.nBlockSize				= pInfo->nBlockSize;
		info.pWHCompress			= pInfo->pWHCompress;
		info.pEncryptor				= pInfo->pEncryptor;
		info.pDecryptor				= pInfo->pDecryptor;
		wh_package_fileman			*pPFM	= new wh_package_fileman(&info);
		wh_package_util::dircp		*pCP	= wh_package_util::dircp::Create();
		try
		{
			// 浏览目录（通过pInfo->pFM），一个一个把文件从包中拷贝出来
			wh_package_util::dircp::INFO_T	cpinfo;
			cpinfo.pFMSrc				= pPFM;
			cpinfo.pFMDst				= pInfo->pFM;
			cpinfo.cszSrcDir			= pInfo->cszSrc;
			cpinfo.getcmninfofrom(pInfo);
			int	rst	= pCP->DoIt(&cpinfo);
			WHSafeSelfDestroy(pCP);
			WHSafeSelfDestroy(pPFM);
			return	rst;
		}
		catch (int nErr)
		{
			WHSafeSelfDestroy(pCP);
			WHSafeSelfDestroy(pPFM);
			return	nErr;
		}
	}
};

struct	l_sf_compress		: public wh_package_util::sf_compress
{
	virtual	void	SelfDestroy()
	{
		delete	this;
	}

	virtual	int		DoIt(INFO_T *pInfo)
	{
		// 压缩单文件的管理器
		whfileman_singlefileman_INFO_T	info;
		info.bAutoCloseFM	= false;
		info.pFM			= pInfo->pFM;
		info.pWHCompress	= pInfo->pWHCompress;
		info.pEncryptor		= pInfo->pEncryptor;
		info.pDecryptor		= pInfo->pDecryptor;
		info.nPassLen		= pInfo->nPassLen;
		memcpy(info.szPass, pInfo->szPass, pInfo->nPassLen);
		whfileman						*fpDst	= whfileman_singlefileman_Create(&info);
		wh_package_util::dircp			*pCP	= wh_package_util::dircp::Create();
		try
		{
			// 浏览目录（通过pInfo->pFM），一个一个把文件从包中拷贝出来
			wh_package_util::dircp::INFO_T	cpinfo;
			cpinfo.pFMSrc				= pInfo->pFM;			//（就直接用pInfo中的就好）
			cpinfo.pFMDst				= fpDst;
			cpinfo.cszSrcDir			= pInfo->cszSrc;
			cpinfo.getcmninfofrom(pInfo);
			int	rst	= pCP->DoIt(&cpinfo);
			WHSafeSelfDestroy(pCP);
			WHSafeSelfDestroy(fpDst);
			return	rst;
		}
		catch (int nErr)
		{
			WHSafeSelfDestroy(pCP);
			WHSafeSelfDestroy(fpDst);
			return	nErr;
		}
	}
};

struct	l_sf_decompress		: public wh_package_util::sf_decompress
{
	virtual	void	SelfDestroy()
	{
		delete	this;
	}

	virtual	int		DoIt(INFO_T *pInfo)
	{
		// 压缩单文件的管理器
		whfileman_singlefileman_INFO_T	info;
		info.bAutoCloseFM	= false;
		info.pFM			= pInfo->pFM;
		info.pWHCompress	= pInfo->pWHCompress;
		info.pEncryptor		= pInfo->pEncryptor;
		info.pDecryptor		= pInfo->pDecryptor;
		info.nPassLen		= pInfo->nPassLen;
		memcpy(info.szPass, pInfo->szPass, pInfo->nPassLen);
		whfileman						*fpSrc	= whfileman_singlefileman_Create(&info);
		wh_package_util::dircp			*pCP	= wh_package_util::dircp::Create();
		try
		{
			// 浏览目录（通过pInfo->pFM），一个一个把文件从包中拷贝出来
			wh_package_util::dircp::INFO_T	cpinfo;
			cpinfo.pFMSrc				= fpSrc;
			cpinfo.pFMDst				= pInfo->pFM;			//（就直接用pInfo中的就好）
			cpinfo.cszSrcDir			= pInfo->cszSrc;
			cpinfo.getcmninfofrom(pInfo);
			int	rst	= pCP->DoIt(&cpinfo);
			WHSafeSelfDestroy(pCP);
			WHSafeSelfDestroy(fpSrc);
			return	rst;
		}
		catch (int nErr)
		{
			WHSafeSelfDestroy(pCP);
			WHSafeSelfDestroy(fpSrc);
			return	nErr;
		}
	}
};

bool vercmpless(const char * const ver1, const char * const ver2)
{
	unsigned int vercode1[4], vercode2[4];
	int ret = sscanf(ver1, "%u.%u.%u.%u", vercode1, vercode1+1, vercode1+2, vercode1+3);
	while (ret < 4)
	{
		vercode1[ret++] = 0;
	}

	ret = sscanf(ver2, "%u.%u.%u.%u", vercode2, vercode2+1, vercode2+2, vercode2+3);
	while (ret < 4)
	{
		vercode2[ret++] = 0;
	}

	return (vercode1[0] != vercode2[0]) ? (vercode1[0] < vercode2[0]) :
	((vercode1[1] != vercode2[1]) ? (vercode1[1] < vercode2[1]) :
	((vercode1[2] != vercode2[2]) ? (vercode1[2] < vercode2[2]) : (vercode1[3] < vercode2[3])));
}

struct	l_update		: public wh_package_util::update
{
	// 配置结构
	struct	CFGINFO_T	: public whdataini::obj
	{
		enum
		{
			ValidityStrLength = 16
		};
		l_update	*m_pUpdate;
		// 更新内容的子目录名
		char		szUpdateDir[WH_MAX_PATH];
		char		szValidityStr[ValidityStrLength];
		// 保证创建空包的动作
		WHDATAPROP_SETVALFUNC_DECLARE(PACKAGE)
		{
			return	m_pUpdate->PACKAGE(cszVal);
		}

		// 定义为有成员映射的结构
		WHDATAPROP_DECLARE_MAP(CFGINFO_T)

		CFGINFO_T()
		: m_pUpdate(NULL)
		{
			strcpy(szUpdateDir, "update");
		}
	}m_cfginfo;
	// DoIt的参数
	INFO_T						*m_pDoItInfo;
	// 可以处理包的文件管理器（以可写方式打开）
	wh_package_fileman			*m_pPFM;
	// 文件拷贝对象
	wh_package_util::dircp		*m_pCP;

	l_update()
		: m_pDoItInfo(NULL)
		, m_pPFM(NULL)
		, m_pCP(NULL)
	{
		m_cfginfo.m_pUpdate	= this;
	}
	~l_update()
	{
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}

	virtual	int		DoIt(INFO_T *pInfo)
	{
		m_pDoItInfo					= pInfo;
		// 可以处理包的文件管理器（以可写方式打开）
		whfileman_package_INFO_T	info;
		info.nSearchOrder			= whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2;
		info.bAutoCloseFM			= false;
		info.pFM					= pInfo->pFM;
		info.nOPMode				= whfileman::OP_MODE_BIN_READWRITE;
		memcpy(info.szPass, pInfo->szPass, pInfo->nPassLen);
		info.nPassLen				= pInfo->nPassLen;
		info.nBlockSize				= pInfo->nBlockSize;
		info.pWHCompress			= pInfo->pWHCompress;
		info.pEncryptor				= pInfo->pEncryptor;
		info.pDecryptor				= pInfo->pDecryptor;
		info.bSearchTailFile		= true;	// 这个很重要，否则就认不出来尾部文件了
		m_pPFM	= new wh_package_fileman(&info);
		// 这个是设置更新资源包本身只读，否则有可能更新资源包本身就在当前EXE内，所以用写方式打开就会失败了
		m_pPFM->SetPackageOpenMode(pInfo->cszSrc, whfileman::OP_MODE_BIN_READONLY);
		// 读出出脚本文件并执行
		char	szTmp[WH_MAX_PATH];
		sprintf(szTmp, "%s/%s", pInfo->cszSrc, "cfg.txt");
		whfile	*fp	= m_pPFM->Open(szTmp, whfileman::OP_MODE_BIN_READONLY);
		if( fp )	// 如果文件不存在也没关系啦，说明不需要有配置动作
		{
			// 解析配置文件（里面也会有一定的动作）
			int	rst	= ParseCfg(fp);
			WHSafeSelfDestroy(fp);
			if( rst<0 )
			{
				WHSafeSelfDestroy(m_pCP);
				WHSafeSelfDestroy(m_pPFM);
				char	szMsg[1024];
				sprintf(szMsg, "ParseCfg %s err:%d", szTmp, rst);
				pInfo->pNotify->cb_MSGOUT(0, szMsg);
				return	-10000 + rst;
			}

			if ( vercmpless(pInfo->pValidityStr, m_cfginfo.szValidityStr) )
			//if (strncmp(m_cfginfo.szValidityStr, pInfo->pValidityStr, CFGINFO_T::ValidityStrLength) > 0)
			{
				WHSafeSelfDestroy(m_pCP);
				WHSafeSelfDestroy(m_pPFM);
				char	szMsg[1024];
				sprintf(szMsg, "Need Client Version: %s, Current: %s", m_cfginfo.szValidityStr, pInfo->pValidityStr);
				pInfo->pNotify->cb_ERROR(-5, szMsg);
				return	-10100;
			}
		}
		// 获得更新数据目录
		sprintf(szTmp, "%s/%s", pInfo->cszSrc, m_cfginfo.szUpdateDir);
		// 进行文件拷贝
		m_pCP	= wh_package_util::dircp::Create();
		try
		{
			// 浏览目录（通过pInfo->pFM），一个一个把文件从包中拷贝出来
			wh_package_util::dircp::INFO_T	cpinfo;
			cpinfo.pFMSrc				= m_pPFM;
			cpinfo.pFMDst				= m_pPFM;
			cpinfo.cszSrcDir			= szTmp;
			cpinfo.getcmninfofrom(pInfo);
			int	rst	= m_pCP->DoIt(&cpinfo);
			WHSafeSelfDestroy(m_pCP);
			WHSafeSelfDestroy(m_pPFM);
			return	rst;
		}
		catch (int nErr)
		{
			WHSafeSelfDestroy(m_pCP);
			WHSafeSelfDestroy(m_pPFM);
			return	nErr;
		}
	}
	// 自己用的
	int	PACKAGE(const char *cszVal)
	{
		// 创建包
		char	buf[1024];
		sprintf(buf, "Make sure package %s exist ...", cszVal);
		m_pDoItInfo->pNotify->cb_MSGOUT(0, buf);
		sprintf(buf, "%s/%s", m_pDoItInfo->cszDst, cszVal);
		int	rst	= m_pPFM->SureOpenPackage(buf, wh_package_fileman::CSZPCKEXT, whfileman::OP_MODE_BIN_READWRITE);
		if( rst==0 )	return	0;
		sprintf(buf, "Create package %s err:%d", cszVal, rst);
		m_pDoItInfo->pNotify->cb_ERROR(wh_package_util::notify::ERRCODE_F_CREATE, buf);
		return	rst;
	}
	int	ParseCfg(whfile *fp)
	{
		WHDATAINI_CMN	ini;
		ini.addobj("UPDATE", &m_cfginfo);
		int	rst = ini.analyzefile(fp);
		if( rst<0 )
		{
			char	errmsg[4096];
			sprintf(errmsg, "Can not analyze cfgfile rst:%d %s%s", rst, ini.printerrreport(), WHLINEEND);
			m_pDoItInfo->pNotify->cb_ERROR(wh_package_util::notify::ERRCODE_OTHER, errmsg);
			return	-1;
		}
		return	0;
	}
};
WHDATAPROP_MAP_BEGIN_AT_ROOT(l_update::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szUpdateDir, 0)
	WHDATAPROP_ON_SETVALUE_smp(charptr, szValidityStr, 0)
	WHDATAPROP_ON_SETVALFUNC(PACKAGE)
WHDATAPROP_MAP_END()

struct	l_dircp		: public wh_package_util::dircp
{
	virtual	void		SelfDestroy()
	{
		delete	this;
	}
	virtual	int			DoIt(INFO_T *pInfo)
	{
		// 浏览src目录，一一对应到dst目录中
		class	MyDirSearch	: public WHDirSearchActionWFM
		{
		private:
			wh_package_util::dircp::INFO_T	*m_pInfo;
			whvector<char>	m_vect;
		public:
			MyDirSearch(wh_package_util::dircp::INFO_T *pInfo)
				: WHDirSearchActionWFM(pInfo->pFMSrc)
				, m_pInfo(pInfo), m_vect(pInfo->nNotifyStepBytes)
			{
				WHDirSearchActionWFM::SetStopSearchVar(m_pInfo->pbShouldStop);
				// 增加跳过的文件列表
				if( m_pInfo->apszSkipFile )
				{
					int	i	= 0;
					while( m_pInfo->apszSkipFile[i] )
					{
						WHDirSearchActionWFM::AddSkipFile(m_pInfo->apszSkipFile[i]);
						++i;
					}
				}
				if( pInfo->pszBadFileFilter )
				{
					WHDirSearchActionWFM::SetBadFilter(pInfo->pszBadFileFilter);
				}
			}
		private:
			virtual int		ActionOnFile(const char *szFile)
			{
				// 获得全路径，并获得文件属性
				whfileman::PATHINFO_T	pathinfo;
				if( m_pInfo->pFMSrc->GetPathInfo(GetFullPath(szFile), &pathinfo)<0 )
				{
					return	-1;
				}
				if( m_pInfo->nProp&wh_package_util::PROP_SKIPRDONLYFILE )
				{
					if( (pathinfo.nType&whfileman::PATHINFO_T::TYPE_RDOL) != 0 )
					{
						// 不处理了
						m_pInfo->pNotify->cb_FILE_SKIP(GetLastFullPath());
						return	0;
					}
				}
				char	errmsg[1024];
				whfile	*fpSrc	= m_pInfo->pFMSrc->Open(GetLastFullPath(), whfileman::OP_MODE_BIN_READONLY);
				if( !fpSrc )
				{
					sprintf(errmsg, "Can not open %s!", GetLastFullPath());
					if( m_pInfo->pNotify->cb_ERROR(wh_package_util::notify::ERRCODE_F_OPEN, errmsg)!=wh_package_util::notify::RST_OK_CONTINUE )
					{
						// 停止了
						return	-2;
					}
					// 继续后面的文件
					return	0;
				}
				whfile	*fpDst	= NULL;
				try
				{
					char	szDstPath[WH_MAX_PATH];
					char	szFullRelPath[WH_MAX_PATH];
					int		nLen		= strlen(GetFullRelPath()) - m_pInfo->nCutFileTail;
					memcpy(szFullRelPath, GetFullRelPath(), nLen);
					szFullRelPath[nLen]	= 0;
					if( m_pInfo->cszAppendExt )
					{
						sprintf(szDstPath, "%s%s%s", m_pInfo->cszDstDir, szFullRelPath, m_pInfo->cszAppendExt);
					}
					else
					{
						sprintf(szDstPath, "%s%s", m_pInfo->cszDstDir, szFullRelPath);
					}
					// 通知一下文件开始拷贝了
					if( m_pInfo->pNotify->cb_FILE_BEGIN(GetLastFullPath(), szDstPath, fpSrc->FileSize())!=wh_package_util::notify::RST_OK_CONTINUE )
					{
						throw	-11;
					}
					// 保证目标文件目录被创建
					if( m_pInfo->pFMDst->SureMakeDirForFile(szDstPath)<0 )
					{
						throw	-12;
					}
					// 创建目标文件
					fpDst	= m_pInfo->pFMDst->Open(szDstPath, whfileman::OP_MODE_BIN_CREATE);
					if( !fpDst )
					{
						throw	-13;
					}
					// 拷贝
					int	nTotal	= 0;
					while(!fpSrc->IsEOF())
					{
						int	rstRead	= fpSrc->Read(m_vect.getbuf(), m_vect.size());
						if( rstRead==0 )
						{
							// 结束了
							break;
						}
						if( rstRead<0 )
						{
							sprintf(errmsg, "ERROR reading file %s!", GetLastFullPath());
							if( m_pInfo->pNotify->cb_ERROR(wh_package_util::notify::ERRCODE_F_READ, errmsg)!=wh_package_util::notify::RST_OK_CONTINUE )
							{
								// 停止了
								throw	-21;
							}
							// 继续后面的文件
							throw	0;
						}
						int	rstWrite= fpDst->Write(m_vect.getbuf(), rstRead);
						if( rstWrite!=rstRead )
						{
							sprintf(errmsg, "ERROR writng file %s!", szDstPath);
							if( m_pInfo->pNotify->cb_ERROR(wh_package_util::notify::ERRCODE_F_WRITE, errmsg)!=wh_package_util::notify::RST_OK_CONTINUE )
							{
								// 停止了
								throw	-22;
							}
							// 继续后面的文件
							throw	0;
						}
						nTotal	+= rstWrite;
						if( m_pInfo->pNotify->cb_FILE_PROCESS(nTotal)!=wh_package_util::notify::RST_OK_CONTINUE )
						{
							// 可能是被cancel了
							throw	-23;
						}
					}
					// 看是否需要重新设置文件时间
					if( (m_pInfo->nProp&wh_package_util::PROP_KEEPFILETIME)!=0 )
					{
						fpDst->SetFileTime(fpSrc->FileTime());
					}
					// 结束了，关闭文件
					WHSafeSelfDestroy(fpSrc);
					WHSafeSelfDestroy(fpDst);
					// 通知上层结束了
					if( m_pInfo->pNotify->cb_FILE_END()!=wh_package_util::notify::RST_OK_CONTINUE )
					{
						// 可能是被cancel了
						throw	-30;
					}
					return	0;
				}
				catch( int nErr )
				{
					WHSafeSelfDestroy(fpSrc);
					WHSafeSelfDestroy(fpDst);
					return	nErr;
				}
				return	0;
			}
			virtual int		ActionOnDir(const char *szDir)
			{
				return	0;
			}
			virtual int		EnterDir(const char *szDir)
			{
				if( m_pInfo->pNotify->cb_DIR_BEGIN(szDir)!=wh_package_util::notify::RST_OK_CONTINUE )
				{
					// 可能是被cancel了
					return	-1;
				}
				return 0;
			}
			virtual int		LeaveDir(const char *szDir)
			{
				if( m_pInfo->pNotify->cb_DIR_END(szDir)!=wh_package_util::notify::RST_OK_CONTINUE )
				{
					// 可能是被cancel了
					return	-1;
				}
				return 0;
			}
		};

		// 现获取一下源目录的统计信息
		bool	bQuick	= true;
		if( pInfo->nGetSrcDirStatMode == wh_package_util::GETSRCDIRSTATMODE_NO )
		{

		}
		else
		{
			if( pInfo->nGetSrcDirStatMode == wh_package_util::GETSRCDIRSTATMODE_SLOW )
			{
				bQuick	= false;
			}
			pInfo->pNotify->cb_MSGOUT(0, "Calculating src dir stat ...");
			DIR_STAT_T	ds;
			ds.pNotify		= pInfo->pStatNotify;
			ds.pbShouldStop	= pInfo->pbShouldStop;
			int	rst	= WHDirSearchGetStatistics(pInfo->pFMSrc, pInfo->cszSrcDir, bQuick, &ds);
			if( rst<0 )
			{
				pInfo->pNotify->cb_ERROR(wh_package_util::notify::ERRCODE_F_READ, "Source files are probably corrupted");
				return	-30000 + rst;
			}
			// 通知上层统计信息
			pInfo->pNotify->cb_SRCDIRSTAT(&ds);
		}

		MyDirSearch	mds(pInfo);
		return	mds.DoSearch(pInfo->cszSrcDir, true);
	}
};

}		// EOF namespace n_whcmn

wh_package_util::notify *		wh_package_util::notify::CreateCmnConsole()
{
	return	new l_notify_cosole;
}
wh_package_util::dir2package *	wh_package_util::dir2package::Create()
{
	return	new l_dir2package;
}
wh_package_util::package2dir *	wh_package_util::package2dir::Create()
{
	return	new l_package2dir;
}
wh_package_util::sf_compress *	wh_package_util::sf_compress::Create()
{
	return	new l_sf_compress;
}
wh_package_util::sf_decompress *wh_package_util::sf_decompress::Create()
{
	return	new l_sf_decompress;
}
wh_package_util::update *wh_package_util::update::Create()
{
	return	new l_update;
}
wh_package_util::dircp *		wh_package_util::dircp::Create()
{
	return	new l_dircp;
}
