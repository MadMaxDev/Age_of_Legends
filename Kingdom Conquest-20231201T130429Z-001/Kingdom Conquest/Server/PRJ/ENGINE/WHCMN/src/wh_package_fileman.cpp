// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_fileman.cpp
// Creator     : Wei Hua (魏华)
// Comment     : 基于包的通用文件系统
// CreationDate: 2006-08-09
// ChangeLOG   : 2007-02-28 在各种文件操作接口里面都先过滤一下文件路径
//             : 2007-03-13 把默认的包文件后缀改成小写的
//             : 2007-09-06 修正了wh_package_fileman::FindPackageOnPath中判断linux绝对路径最后会判断成""目录下bug
//			   : 2008-06-06	增加一个接口，用于直接读取原始的压缩数据（by 钟文杰）

#include "../inc/wh_package_fileman_i.h"

namespace n_whcmn
{

whfileman *	whfileman_package_Create(whfileman_package_INFO_T *pInfo)
{
	return	new wh_package_fileman(pInfo);
}
whfileman *	whfileman_package_Create_Easy(whfileman_package_INFO_Easy_T *pInfo)
{
	ICryptFactory	*pICryptFactory	= ICryptFactory_DHKeyAndDES_Create();
	whfileman_package_INFO_T	info;
	info.nSearchOrder	= pInfo->nSearchOrder;
	info.bSearchTailFile= pInfo->bSearchTailFile;
	info.bAutoCloseFM	= true;
	info.pFM			= whfileman_Cmn_Create();
	info.nOPMode		= pInfo->nOPMode;
	info.pWHCompress	= WHCompress_Create_ZLib();
	info.pEncryptor		= pICryptFactory->CreateICryptor(pInfo->nCryptType);
	info.pDecryptor		= pICryptFactory->CreateICryptor(pInfo->nCryptType);
	memcpy(info.szPass, pInfo->szPass, pInfo->nPassLen);
	info.nPassLen		= pInfo->nPassLen;
	return	new wh_package_fileman_Easy(&info, pICryptFactory);
}

}		// EOF namespace n_whcmn

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// wh_package_fileman
////////////////////////////////////////////////////////////////////
const char		wh_package_fileman::CSZPCKEXT[]		= ".dpk";
const int		wh_package_fileman::CNPCKEXTLEN		= 4;			// 除去最后的'\0'占的一字节（必须保证所有包文件的后缀长度都为这个值）
const char *	wh_package_fileman::CSZPCKEXT_All[]	=				// 所有可能的包文件后缀
{
	".dpk",
	".exe",
	NULL,
};
const int		wh_package_fileman::CNPCKEXT_DFT_IDX= 0;			// 默认后缀在CSZPCKEXT_All中的序号

//
WHDirBase::ENTRYINFO_T *	wh_package_fileman::DirKnowPackage::Read()
{
loop_read:
	ENTRYINFO_T	*pEntry	= m_pBaseDir->Read();
	if( !pEntry )
	{
		return	NULL;
	}
	if( !pEntry->bIsDir )	// 不用判断是否查找模式是SEARCH_ORDER_DISK_ONLY，因为在那种情况下就不用生成这样的DirKnowPackage目录对象
	{
		// 判断是否可能是包文件，如果是就当作目录处理
		int	nLen	= strlen(pEntry->szName)-wh_package_fileman::CNPCKEXTLEN;
		const char	*pszExt	= pEntry->szName+nLen;
		int	nIdx	= wh_strcmparray(wh_package_fileman::CSZPCKEXT_All, pszExt);
		if( nIdx>=0 )
		{
			// 名字正确了，看看内容是否正确
			char	szFullPath[WH_MAX_PATH];
			sprintf(szFullPath, "%s/%s", m_pBaseDir->GetDir(), pEntry->szName);
			switch( wh_package_base::FileIsWhat(m_pFM, szFullPath) )
			{
			case	wh_package_base::FILEISWHAT_PCK:
				{
					// 把这个入口改为目录
					pEntry->bIsDir	= true;
					pEntry->szName[nLen]	= 0;
				}
				break;
			case	wh_package_base::FILEISWHAT_TAILPCK:
				{
					if( m_bSearchTailFile )
					{
						// 把这个入口改为目录
						pEntry->bIsDir	= true;
						pEntry->szName[nLen]	= 0;
					}
					else
					{
						// 否则忽略这个文件
						goto	loop_read;
					}
				}
				break;
			default:
				// 其他的什么也不做
				break;
			}
		}
	}
	return	pEntry;
}
//
wh_package_fileman::wh_package_fileman(whfileman_package_INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	if( pInfo->pPOPMode != NULL )
	{
		whfileman_package_INFO_T::PACKET_OP_MODE_T	*pPOPMode	= pInfo->pPOPMode;
		while( pPOPMode->pcszPck != NULL )
		{
			SetPackageOpenMode(pPOPMode->pcszPck, pPOPMode->nOPMode);
			pPOPMode	++;
		}
	}
	m_bLastPathIsAboutPck	= false;
}
wh_package_fileman::~wh_package_fileman()
{
	Close();
}
int		wh_package_fileman::Close()
{
	// 清除所有载入的包对象
	for(PATH2PACKAGE_T::kv_iterator it=m_mapPath2Package.begin(); it!=m_mapPath2Package.end(); ++it)
	{
		delete	it.getvalue();
	}
	m_mapPath2Package.clear();
	m_mapPath2OPMode.clear();
	// 如果需要就自动删除底层FM
	if( m_info.bAutoCloseFM )
	{
		m_info.bAutoCloseFM	= false;
		WHSafeSelfDestroy(m_info.pFM);
	}
	return	0;
}
int		wh_package_fileman::SetPackageOpenMode(const char *cszPCKName, whfileman::OP_MODE_T nOPMode)
{
	// 为了保险先删除
	whPathStrTransformer	WHPST(cszPCKName);
	m_mapPath2OPMode.erase((const char *)WHPST);
	if( nOPMode == whfileman::OP_MODE_UNDEFINED )
	{
		// 这个表示清除某个（这个只能是在package还没有载入之前清除才有效的）
		return	0;
	}
	// 插入
	if( !m_mapPath2OPMode.put((const char *)WHPST, nOPMode) )
	{
		return	-1;
	}
	return	0;
}
int		wh_package_fileman::FlushAllPackage()
{
	// 找到所有的package对象，调用对方的flush方法
	for(PATH2PACKAGE_T::kv_iterator it=m_mapPath2Package.begin(); it!=m_mapPath2Package.end(); ++it)
	{
		it.getvalue()->Flush();
	}
	return	0;
}
void	wh_package_fileman::SelfDestroy()
{
	delete	this;
}
whfile *		wh_package_fileman::Open(const char *cszFName, OP_MODE_T mode)
{
#ifdef	_DEBUG
	whfile *fp	= _Open(cszFName, mode);
	if( !fp && m_info.bCheckFNCaseOnErr )
	{
		if( wh_checkcharrange(cszFName, 'A', 'Z') )
		{
			assert(0 && "可能是文件名大小写不一致错误");
		}
	}
	return	fp;
#else
	return	_Open(cszFName, mode);
#endif
}
whfile *		wh_package_fileman::_Open(const char *cszFName, OP_MODE_T mode)
{
	whPathStrTransformer	WHPST(cszFName);
	const char *pszWHPST	= (const char *)WHPST;

	// 根据查找模式判断先做什么后做什么
	switch( m_info.nSearchOrder )
	{
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK_ONLY:
		return	Open_On_Disk(pszWHPST, mode);
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY:
		return	Open_In_Pck(pszWHPST, mode);
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2:
		{
			whfile	*fp	= Open_On_Disk(pszWHPST, mode);
			if( fp )
			{
				return	fp;
			}
			return	Open_In_Pck(pszWHPST, mode);
		}
		break;
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2:
		{
			whfile	*fp	= Open_In_Pck(pszWHPST, mode);
			if( fp )
			{
				return	fp;
			}
			else if( m_bLastPathIsAboutPck )
			{
				return	NULL;
			}
			return	Open_On_Disk(pszWHPST, mode);
		}
		break;
	default:
		return	NULL;
	}
	return	NULL;
}
WHDirBase *		wh_package_fileman::OpenDir(const char *cszDName)
{
#ifdef	_DEBUG
	WHDirBase *fd	= _OpenDir(cszDName);
	if( !fd && m_info.bCheckFNCaseOnErr )
	{
		if( wh_checkcharrange(cszDName, 'A', 'Z') )
		{
			assert(0 && "可能是目录名大小写不一致错误");
		}
	}
	return	fd;
#else
	return	_OpenDir(cszDName);
#endif
}
WHDirBase *		wh_package_fileman::_OpenDir(const char *cszDName)
{
	whPathStrTransformer	WHPST(cszDName);
	const char *pszWHPST	= (const char *)WHPST;
	// 根据查找模式判断先做什么后做什么
	switch( m_info.nSearchOrder )
	{
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK_ONLY:
		return	OpenDir_On_Disk_Only(pszWHPST);
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY:
		return	OpenDir_In_Pck(pszWHPST);
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2:
		{
			WHDirBase	*fp	= OpenDir_On_Disk(pszWHPST);
			WHDirBase	**ppNext	= &fp;
			if( fp )
			{
				ppNext	= &fp->m_pNext;
			}
			*ppNext	= OpenDir_In_Pck(pszWHPST);
			return	new WHDirChain(fp);
		}
		break;
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2:	// 这个就不用包和硬盘上的目录一起列了
		{
			WHDirBase	*fp	= OpenDir_In_Pck(pszWHPST);
			if( fp )
			{
				return	fp;
			}
			if( m_bLastPathIsAboutPck )
			{
				return	NULL;
			}
			return	OpenDir_On_Disk(pszWHPST);
		}
		break;
	default:
		return	NULL;
	}
	return	NULL;
}
int				wh_package_fileman::MakeDir(const char *cszDName)
{
	whPathStrTransformer	WHPST(cszDName);
	const char *pszWHPST	= (const char *)WHPST;
	switch( m_info.nSearchOrder )
	{
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK_ONLY:
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2:
		return	CreateDir_On_Disk(pszWHPST);
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY:
		return	CreateDir_In_Pck(pszWHPST);
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2:
		{
			// 如果包不存在就创建磁盘目录
			int	rst	= CreateDir_In_Pck(pszWHPST);
			switch( rst )
			{
			case	CREATEDIR_IN_PCK_RST_OK:
				return	0;
			case	CREATEDIR_IN_PCK_RST_ERR_NOPCK:
				// 这个不用管
				break;
			default:
				return	-1000+rst;
			}
			rst	= CreateDir_On_Disk(pszWHPST);
			if( rst<0 )
			{
				return	-2000 + rst;
			}
			return	0;
		}
		break;
	default:
		return	-1;
	}
	return	0;
}
int				wh_package_fileman::SureMakeDirForFile(const char *cszFName)
{
	char	szPath[WH_MAX_PATH];
	if( whfile_getfilepath(cszFName, szPath) )
	{
		// 创建目录（内部不会重复创建的）
		return	MakeDir(szPath);
	}
	// 说明是只有一个文件名，那么就是当前路径下的文件
	return	0;
}
bool			wh_package_fileman::IsPathExist(const char *__path)
{
	PATHINFO_T	info;
	if( GetPathInfo(__path, &info)<0 )
	{
		return	false;
	}
	return	true;
}
bool			wh_package_fileman::IsPathDir(const char *__path)
{
	PATHINFO_T	info;
	if( GetPathInfo(__path, &info)<0 )
	{
		return	false;
	}
	return	(info.nType&PATHINFO_T::TYPE_DIR) != 0;
}
whfile *		wh_package_fileman::Open_On_Disk(const char *cszFName, OP_MODE_T mode)
{
	return	m_info.pFM->Open(cszFName, mode);
}
whfile *		wh_package_fileman::Open_In_Pck(const char *cszFName, OP_MODE_T mode)
{
	int				nOffset	= 0;
	wh_package_base	*pPB	= FindPackageOnPath(cszFName, true, &nOffset);
	if( pPB )
	{
		m_bLastPathIsAboutPck	= true;
		return	pPB->OpenFile(cszFName+nOffset, mode);
	}
	m_bLastPathIsAboutPck	= false;
	return	NULL;
}
WHDirBase *		wh_package_fileman::OpenDir_On_Disk_Only(const char *cszDName)
{
	if( !m_info.pFM->IsPathDir(cszDName) )
	{
		// 指定路径不是目录，那么就不可能打开了
		return	NULL;
	}
	return	m_info.pFM->OpenDir(cszDName);
}
WHDirBase *		wh_package_fileman::OpenDir_On_Disk(const char *cszDName)
{
	if( !m_info.pFM->IsPathDir(cszDName) )
	{
		// 指定路径不是目录，那么就不可能打开了
		return	NULL;
	}
	return	new DirKnowPackage(m_info.pFM, m_info.pFM->OpenDir(cszDName), true, m_info.bSearchTailFile);
}
int				wh_package_fileman::CreateDir_On_Disk(const char *cszDName)
{
	return	m_info.pFM->MakeDir(cszDName);
}
WHDirBase *		wh_package_fileman::OpenDir_In_Pck(const char *cszDName)
{
	int				nOffset	= 0;
	wh_package_base	*pPB	= FindPackageOnPath(cszDName, false, &nOffset);
	if( pPB )
	{
		m_bLastPathIsAboutPck	= true;
		return	pPB->OpenDir(cszDName+nOffset);
	}
	m_bLastPathIsAboutPck	= false;
	return	NULL;
}
int				wh_package_fileman::CreateDir_In_Pck(const char *cszDName)
{
	int				nOffset	= 0;
	wh_package_base	*pPB	= FindPackageOnPath(cszDName, false, &nOffset);
	if( pPB )
	{
		int	rst	= pPB->MakeDir(cszDName+nOffset);
		if( rst<0 )
		{
			return	CREATEDIR_IN_PCK_RST_ERR_OTHER + rst;
		}
		return	CREATEDIR_IN_PCK_RST_OK;
	}
	// 注：因为有了这个返回，所以不用m_bLastPathIsAboutPck来通知上层了
	return	CREATEDIR_IN_PCK_RST_ERR_NOPCK;
}
wh_package_base *	wh_package_fileman::FindPackageOnPath(const char *cszPath, bool bIsFile, int *pnOffset)
{
	// 先通过路径查找包（从后往前）
	whstrstack	wss('/');
	if( wss.reinitfrom(cszPath, "/\\")<0 )
	{
		return	NULL;
	}
	int	&nOffset	= *pnOffset;
	if( bIsFile )
	{
		wss.pop(&nOffset);
	}
	else
	{
		nOffset		= strlen(cszPath);
	}
	// 然后一级一级找包
	while( wss.size()>0 )
	{
		wh_package_base	*pPB	= NULL;
		// 看看包是否已经载入
		const char	*pcszPath	= wss.getwholestr();
		if( pcszPath[0]==0 )	// 如果为0说明原来是绝对路径，最后判断到根了
		{
			break;
		}
		if( m_mapPath2Package.get(WHPATHSTRTRANSFORMER(pcszPath), pPB) )
		{
			// 这样就直接在pPB中得到相应的package对象指针了
			return	pPB;
		}
		// 包没有载入，看看包文件是否存在
		// 原来会先判断是否是目录，但是这样就失去了包优先的机制了，所以就不做目录判断了
		//if( m_info.pFM->IsPathDir(wss.getwholestr()) )
		//{
		//	return	NULL;
		//}
		char	szFName[WH_MAX_PATH];
		int		i=0;
		while( wh_package_fileman::CSZPCKEXT_All[i]!=NULL )
		{
			// 生成文件名
			sprintf(szFName, "%s%s", wss.getwholestr(), wh_package_fileman::CSZPCKEXT_All[i]);
			if( m_info.pFM->IsPathExist(szFName) )
			{
				switch( wh_package_base::FileIsWhat(m_info.pFM, szFName) )
				{
				case	wh_package_base::FILEISWHAT_PCK:
				case	wh_package_base::FILEISWHAT_TAILPCK:
					{
						// 包存在，打开包文件
						return	LoadPackage(wss.getwholestr(), wh_package_fileman::CSZPCKEXT_All[i], m_info.nOPMode);
					}
					break;
				default:
					{
						if( i == wh_package_fileman::CNPCKEXT_DFT_IDX )
						{
							// 如果是默认后缀的文件不应该打不开的
							return	NULL;
						}
					}
					break;
				}
			}
			i++;
		}
		// 这层没有包文件，继续向前找
		wss.pop(&nOffset);
	}
	// 除非是根目录下的文件，否则一般不会找到这里吧？
	return	NULL;
}
int		wh_package_fileman::GetPathInfo(const char *cszPath, wh_package_fileman::PATHINFO_T *pInfo)
{
	whPathStrTransformer	WHPST(cszPath);
	const char *pszWHPST	= (const char *)WHPST;
	switch( m_info.nSearchOrder )
	{
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK_ONLY:
		return	GetPathInfo_On_Disk(pszWHPST, pInfo);
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2:
		{
			if( GetPathInfo_On_Disk(pszWHPST, pInfo)==0 )
			{
				return	0;
			}
			return	GetPathInfo_In_Pck(pszWHPST, pInfo);
		}
		break;
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY:
		return	GetPathInfo_In_Pck(pszWHPST, pInfo);
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2:
		{
			if( GetPathInfo_In_Pck(pszWHPST, pInfo)==0 )
			{
				return	0;
			}
			return	GetPathInfo_On_Disk(pszWHPST, pInfo);
		}
		break;
	default:
		return	-1;
	}
	return	0;
}
int		wh_package_fileman::DelFile(const char *cszPath)
{
	whPathStrTransformer	WHPST(cszPath);
	const char *pszWHPST	= (const char *)WHPST;
	switch( m_info.nSearchOrder )
	{
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK_ONLY:
		return	DelFile_On_Disk(pszWHPST);
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2:
		{
			if( DelFile_On_Disk(pszWHPST)==0 )
			{
				return	0;
			}
			return	DelFile_In_Pck(pszWHPST);
		}
		break;
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY:
		return	DelFile_In_Pck(pszWHPST);
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2:
		{
			if( DelFile_In_Pck(pszWHPST)==0 )
			{
				return	0;
			}
			return	DelFile_On_Disk(pszWHPST);
		}
		break;
	default:
		return	-1;
	}
	return	0;
}
int		wh_package_fileman::DelDir(const char *cszPath)
{
	whPathStrTransformer	WHPST(cszPath);
	const char *pszWHPST	= (const char *)WHPST;
	switch( m_info.nSearchOrder )
	{
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK_ONLY:
		return	DelDir_On_Disk(pszWHPST);
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2:
		{
			if( DelDir_On_Disk(pszWHPST)==0 )
			{
				return	0;
			}
			return	DelDir_In_Pck(pszWHPST);
		}
		break;
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY:
		return	DelDir_In_Pck(pszWHPST);
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2:
		{
			if( DelDir_In_Pck(pszWHPST)==0 )
			{
				return	0;
			}
			return	DelDir_On_Disk(pszWHPST);
		}
		break;
	default:
		return	-1;
	}
	return	0;
}
int		wh_package_fileman::GetPathInfo_On_Disk(const char *cszPath, wh_package_fileman::PATHINFO_T *pInfo)
{
	// 直接用普通的
	return	m_info.pFM->GetPathInfo(cszPath, pInfo);
}
int		wh_package_fileman::GetPathInfo_In_Pck(const char *cszPath, wh_package_fileman::PATHINFO_T *pInfo)
{
	int				nOffset	= 0;
	wh_package_base	*pPB	= FindPackageOnPath(cszPath, false, &nOffset);
	if( pPB )
	{
		return	pPB->GetPathInfo(cszPath+nOffset, pInfo);
	}
	return	-100;
}
int		wh_package_fileman::DelFile_On_Disk(const char *cszPath)
{
	// 直接用普通的
	return	m_info.pFM->DelFile(cszPath);
}
int		wh_package_fileman::DelDir_On_Disk(const char *cszPath)
{
	return	m_info.pFM->DelDir(cszPath);
}
int		wh_package_fileman::DelFile_In_Pck(const char *cszPath)
{
	int				nOffset	= 0;
	wh_package_base	*pPB	= FindPackageOnPath(cszPath, false, &nOffset);
	if( pPB )
	{
		return	pPB->DelPath(cszPath+nOffset);
	}
	return	-100;
}
int		wh_package_fileman::DelDir_In_Pck(const char *cszPath)
{
	// 在包里删除目录和文件是一样的
	return	DelFile_In_Pck(cszPath);
}
int		wh_package_fileman::SureOpenPackage(const char *cszPckPath, const char *cszExt, whfileman::OP_MODE_T nOPMode)
{
	if( m_mapPath2Package.has(WHPATHSTRTRANSFORMER(cszPckPath)) )
	{
		// 说明包已经打开了
		return	0;
	}
	char	szFullPath[WH_MAX_PATH];
	sprintf(szFullPath, "%s%s", cszPckPath, cszExt);
	// 判断文件是否存在（如果本来就是创建文件那么就不用判断文件是否存在了）
	if( nOPMode != whfileman::OP_MODE_BIN_CREATE
	&&  m_info.pFM->IsPathExist(szFullPath) )
	{
		// 判断是否是包文件
		switch( wh_package_base::FileIsWhat(m_info.pFM, szFullPath) )
		{
		case	wh_package_base::FILEISWHAT_PCK:
		case	wh_package_base::FILEISWHAT_TAILPCK:
			break;
		default:
			// 包文件损坏，那么就返回错误了
			return	-1;
		}
		// 如果打开方式和默认规定的相同，如果今后有有相关访问的情况下会自动打开包的，这里就不用多余操作了
		if( nOPMode==whfileman::OP_MODE_UNDEFINED )
		{
			return	0;
		}
		// 打开方式不同，那么就在这里先打开一下吧
		if( !LoadPackage(cszPckPath, cszExt, nOPMode) )
		{
			return	-2;
		}
		return	0;
	}
	// 保证层级目录存在
	if( m_info.pFM->SureMakeDirForFile(szFullPath)<0 )
	{
		return	-11;
	}
	// 到这一步说明一定是需要创建空包文件
	wh_package_base::INFO_T	info;
	info.ReadFromOther(m_info);
	info.pBaseFile	= m_info.pFM->Open(szFullPath, whfileman::OP_MODE_BIN_CREATE);
	if( !info.pBaseFile )
	{
		return	-21;
	}
	info.bAutoCloseBaseFile	= true;
	strcpy(info.szDir, cszPckPath);
	info.nOPMode			= whfileman::OP_MODE_BIN_CREATE;		// 这个就肯定需要是create了，不用考虑nOPMode
	wh_package_base	*pPB	= new wh_package_base;
	assert(pPB);
	if( pPB->Init(&info)<0 )
	{
		delete	pPB;
		return	-22;
	}
	// 加入字串映射（反正也创建了，不用删除，这样如果后面使用就可以直接用了）
	if( !m_mapPath2Package.put(WHPATHSTRTRANSFORMER(cszPckPath), pPB) )
	{
		assert(0);	// 不应该前面找不到这里又插不进去:(
		delete	pPB;
		return	-23;
	}
	return	0;
}
wh_package_base *	wh_package_fileman::LoadPackage(const char *cszPckPath, const char *cszExt, whfileman::OP_MODE_T nOPMode)
{
	whPathStrTransformer	WHPST(cszPckPath);

	// 现看一下包是否有特殊的打开模式（后面会比较是否相同，如果不同就assert了，看下面）
	m_mapPath2OPMode.get((const char *)WHPST, nOPMode);

	wh_package_base	*pPB	= NULL;
	if( m_mapPath2Package.get((const char *)WHPST, pPB) )
	{
		// 这样就直接在pPB中得到相应的package对象指针了
		if( pPB->GetInitInfo().nOPMode != nOPMode )
		{
			assert(0 && "wh_package_fileman::LoadPackage OPMode differ from before");
			return	NULL;
		}
		return	pPB;
	}

	char	szFullPath[WH_MAX_PATH];
	sprintf(szFullPath, "%s%s", (const char *)WHPST, cszExt);
	whfile	*fp	= Open_On_Disk(szFullPath, nOPMode);
	if( !fp )
	{
		return	NULL;
	}
	// 如果不是包文件则按照tail文件打开
	switch( wh_package_base::FileIsWhat(fp) )
	{
	case	wh_package_base::FILEISWHAT_PCK:
		// 什么也不用做，继续
		break;
	case	wh_package_base::FILEISWHAT_TAILPCK:
		{
			if( !m_info.bSearchTailFile )
			{
				// 不需要检查tail文件
				return	NULL;
			}
			whtailfile	*tf	= new whtailfile;
			if( tf->Open(fp, true, nOPMode)<0 )
			{
				// 说明也不是tail文件
				WHSafeSelfDestroy(tf);
				return	NULL;
			}
			// 把tail文件作为包的基础文件
			fp	= tf;
		}
		break;
	default:
		// 其他情况都认为打不开包
		return	NULL;
		break;
	}
	// 打开包
	pPB = new wh_package_base;
	assert(pPB);
	wh_package_base::INFO_T	info;
	info.ReadFromOther(m_info);
	info.pBaseFile			= fp;
	info.bAutoCloseBaseFile	= true;
	info.nOPMode			= nOPMode;	// 2007-11-20 加
	strcpy(info.szDir, (const char *)WHPST);
	int	rst	= pPB->Init(&info);
	if( rst<0 )
	{
		delete	pPB;
		return	NULL;
	}
	// 加入字串映射
	if( !m_mapPath2Package.put((const char *)WHPST, pPB) )
	{
		assert(0);	// 不应该前面找不到这里又插不进去:(
		delete	pPB;
		return	NULL;
	}
	// 成功载入包，返回
	return	pPB;
}
int		wh_package_fileman::UnloadPackage(const char *cszPckPath)
{
	// 先查一下是否存在
	whPathStrTransformer	WHPST(cszPckPath);

	wh_package_base	*pPB	= NULL;
	if( !m_mapPath2Package.get((const char *)WHPST, pPB) )
	{
		// 没有，就算成功了吧
		return	0;
	}
	// 关闭package
	delete	pPB;
	// 从名字映射表中删除
	m_mapPath2Package.erase((const char *)WHPST);
	// 从特殊模式对应表中删除
	m_mapPath2OPMode.erase((const char *)WHPST);
	return	0;
}
// 判断一个路径是否是包
const char *	wh_package_fileman::IsPathPackage(whfileman *pFM, const char *cszPath)
{
	char	szFName[WH_MAX_PATH];
	int		i=0;
	while( wh_package_fileman::CSZPCKEXT_All[i]!=NULL )
	{
		// 生成文件名
		sprintf(szFName, "%s%s", cszPath, wh_package_fileman::CSZPCKEXT_All[i]);
		if( pFM->IsPathExist(szFName) )
		{
			switch( wh_package_base::FileIsWhat(pFM, szFName) )
			{
			case	wh_package_base::FILEISWHAT_PCK:
			case	wh_package_base::FILEISWHAT_TAILPCK:
				// 包存在
				return	wh_package_fileman::CSZPCKEXT_All[i];
			default:
				// 其他的什么也不做
				break;
			}
		}
		i++;
	}
	return	NULL;
}

// 返回包中文件的原始数据（压缩后的数据），钟文杰添加
whfile * wh_package_fileman::OpenRawFile(const char *cszFName, OP_MODE_T mode)
{
	whPathStrTransformer	WHPST(cszFName);
	const char *pszWHPST	= (const char *)WHPST;

	whfile * fp = NULL;

	// 根据查找模式判断先做什么后做什么
	switch( m_info.nSearchOrder )
	{
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2:
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK_ONLY:
		fp = OpenRaw_In_Pck(pszWHPST, mode);
		break;
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK1_PCK2:
		{
			whfile	*fp	= OpenRaw_In_Pck(pszWHPST, mode);
			if( fp )
			{
				fp->SelfDestroy();
				fp = NULL;
			}
			else
				fp = OpenRaw_In_Pck(pszWHPST, mode);
		}
		break;
	case	whfileman_package_INFO_T::SEARCH_ORDER_DISK_ONLY:
		break;
	default:
		break;
	}

#ifdef	_DEBUG
	if( !fp && m_info.bCheckFNCaseOnErr )
	{
		if( wh_checkcharrange(cszFName, 'A', 'Z') )
		{
			assert(0 && "可能是文件名大小写不一致错误");
		}
	}
#endif

	return	fp;
}
whfile *		wh_package_fileman::OpenRaw_In_Pck(const char *cszFName, OP_MODE_T mode)
{
	int				nOffset	= 0;
	wh_package_base	*pPB	= FindPackageOnPath(cszFName, true, &nOffset);
	if( pPB )
	{
		m_bLastPathIsAboutPck	= true;
		return	pPB->OpenRawFile(cszFName+nOffset, mode);
	}
	m_bLastPathIsAboutPck	= false;
	return	NULL;
}
