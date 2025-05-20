// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_fileman.cpp
// Creator     : Wei Hua (κ��)
// Comment     : ���ڰ���ͨ���ļ�ϵͳ
// CreationDate: 2006-08-09
// ChangeLOG   : 2007-02-28 �ڸ����ļ������ӿ����涼�ȹ���һ���ļ�·��
//             : 2007-03-13 ��Ĭ�ϵİ��ļ���׺�ĳ�Сд��
//             : 2007-09-06 ������wh_package_fileman::FindPackageOnPath���ж�linux����·�������жϳ�""Ŀ¼��bug
//			   : 2008-06-06	����һ���ӿڣ�����ֱ�Ӷ�ȡԭʼ��ѹ�����ݣ�by ���Ľܣ�

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
const int		wh_package_fileman::CNPCKEXTLEN		= 4;			// ��ȥ����'\0'ռ��һ�ֽڣ����뱣֤���а��ļ��ĺ�׺���ȶ�Ϊ���ֵ��
const char *	wh_package_fileman::CSZPCKEXT_All[]	=				// ���п��ܵİ��ļ���׺
{
	".dpk",
	".exe",
	NULL,
};
const int		wh_package_fileman::CNPCKEXT_DFT_IDX= 0;			// Ĭ�Ϻ�׺��CSZPCKEXT_All�е����

//
WHDirBase::ENTRYINFO_T *	wh_package_fileman::DirKnowPackage::Read()
{
loop_read:
	ENTRYINFO_T	*pEntry	= m_pBaseDir->Read();
	if( !pEntry )
	{
		return	NULL;
	}
	if( !pEntry->bIsDir )	// �����ж��Ƿ����ģʽ��SEARCH_ORDER_DISK_ONLY����Ϊ����������¾Ͳ�������������DirKnowPackageĿ¼����
	{
		// �ж��Ƿ�����ǰ��ļ�������Ǿ͵���Ŀ¼����
		int	nLen	= strlen(pEntry->szName)-wh_package_fileman::CNPCKEXTLEN;
		const char	*pszExt	= pEntry->szName+nLen;
		int	nIdx	= wh_strcmparray(wh_package_fileman::CSZPCKEXT_All, pszExt);
		if( nIdx>=0 )
		{
			// ������ȷ�ˣ����������Ƿ���ȷ
			char	szFullPath[WH_MAX_PATH];
			sprintf(szFullPath, "%s/%s", m_pBaseDir->GetDir(), pEntry->szName);
			switch( wh_package_base::FileIsWhat(m_pFM, szFullPath) )
			{
			case	wh_package_base::FILEISWHAT_PCK:
				{
					// �������ڸ�ΪĿ¼
					pEntry->bIsDir	= true;
					pEntry->szName[nLen]	= 0;
				}
				break;
			case	wh_package_base::FILEISWHAT_TAILPCK:
				{
					if( m_bSearchTailFile )
					{
						// �������ڸ�ΪĿ¼
						pEntry->bIsDir	= true;
						pEntry->szName[nLen]	= 0;
					}
					else
					{
						// �����������ļ�
						goto	loop_read;
					}
				}
				break;
			default:
				// ������ʲôҲ����
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
	// �����������İ�����
	for(PATH2PACKAGE_T::kv_iterator it=m_mapPath2Package.begin(); it!=m_mapPath2Package.end(); ++it)
	{
		delete	it.getvalue();
	}
	m_mapPath2Package.clear();
	m_mapPath2OPMode.clear();
	// �����Ҫ���Զ�ɾ���ײ�FM
	if( m_info.bAutoCloseFM )
	{
		m_info.bAutoCloseFM	= false;
		WHSafeSelfDestroy(m_info.pFM);
	}
	return	0;
}
int		wh_package_fileman::SetPackageOpenMode(const char *cszPCKName, whfileman::OP_MODE_T nOPMode)
{
	// Ϊ�˱�����ɾ��
	whPathStrTransformer	WHPST(cszPCKName);
	m_mapPath2OPMode.erase((const char *)WHPST);
	if( nOPMode == whfileman::OP_MODE_UNDEFINED )
	{
		// �����ʾ���ĳ�������ֻ������package��û������֮ǰ�������Ч�ģ�
		return	0;
	}
	// ����
	if( !m_mapPath2OPMode.put((const char *)WHPST, nOPMode) )
	{
		return	-1;
	}
	return	0;
}
int		wh_package_fileman::FlushAllPackage()
{
	// �ҵ����е�package���󣬵��öԷ���flush����
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
			assert(0 && "�������ļ�����Сд��һ�´���");
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

	// ���ݲ���ģʽ�ж�����ʲô����ʲô
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
			assert(0 && "������Ŀ¼����Сд��һ�´���");
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
	// ���ݲ���ģʽ�ж�����ʲô����ʲô
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
	case	whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2:	// ����Ͳ��ð���Ӳ���ϵ�Ŀ¼һ������
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
			// ����������ھʹ�������Ŀ¼
			int	rst	= CreateDir_In_Pck(pszWHPST);
			switch( rst )
			{
			case	CREATEDIR_IN_PCK_RST_OK:
				return	0;
			case	CREATEDIR_IN_PCK_RST_ERR_NOPCK:
				// ������ù�
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
		// ����Ŀ¼���ڲ������ظ������ģ�
		return	MakeDir(szPath);
	}
	// ˵����ֻ��һ���ļ�������ô���ǵ�ǰ·���µ��ļ�
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
		// ָ��·������Ŀ¼����ô�Ͳ����ܴ���
		return	NULL;
	}
	return	m_info.pFM->OpenDir(cszDName);
}
WHDirBase *		wh_package_fileman::OpenDir_On_Disk(const char *cszDName)
{
	if( !m_info.pFM->IsPathDir(cszDName) )
	{
		// ָ��·������Ŀ¼����ô�Ͳ����ܴ���
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
	// ע����Ϊ����������أ����Բ���m_bLastPathIsAboutPck��֪ͨ�ϲ���
	return	CREATEDIR_IN_PCK_RST_ERR_NOPCK;
}
wh_package_base *	wh_package_fileman::FindPackageOnPath(const char *cszPath, bool bIsFile, int *pnOffset)
{
	// ��ͨ��·�����Ұ����Ӻ���ǰ��
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
	// Ȼ��һ��һ���Ұ�
	while( wss.size()>0 )
	{
		wh_package_base	*pPB	= NULL;
		// �������Ƿ��Ѿ�����
		const char	*pcszPath	= wss.getwholestr();
		if( pcszPath[0]==0 )	// ���Ϊ0˵��ԭ���Ǿ���·��������жϵ�����
		{
			break;
		}
		if( m_mapPath2Package.get(WHPATHSTRTRANSFORMER(pcszPath), pPB) )
		{
			// ������ֱ����pPB�еõ���Ӧ��package����ָ����
			return	pPB;
		}
		// ��û�����룬�������ļ��Ƿ����
		// ԭ�������ж��Ƿ���Ŀ¼������������ʧȥ�˰����ȵĻ����ˣ����ԾͲ���Ŀ¼�ж���
		//if( m_info.pFM->IsPathDir(wss.getwholestr()) )
		//{
		//	return	NULL;
		//}
		char	szFName[WH_MAX_PATH];
		int		i=0;
		while( wh_package_fileman::CSZPCKEXT_All[i]!=NULL )
		{
			// �����ļ���
			sprintf(szFName, "%s%s", wss.getwholestr(), wh_package_fileman::CSZPCKEXT_All[i]);
			if( m_info.pFM->IsPathExist(szFName) )
			{
				switch( wh_package_base::FileIsWhat(m_info.pFM, szFName) )
				{
				case	wh_package_base::FILEISWHAT_PCK:
				case	wh_package_base::FILEISWHAT_TAILPCK:
					{
						// �����ڣ��򿪰��ļ�
						return	LoadPackage(wss.getwholestr(), wh_package_fileman::CSZPCKEXT_All[i], m_info.nOPMode);
					}
					break;
				default:
					{
						if( i == wh_package_fileman::CNPCKEXT_DFT_IDX )
						{
							// �����Ĭ�Ϻ�׺���ļ���Ӧ�ô򲻿���
							return	NULL;
						}
					}
					break;
				}
			}
			i++;
		}
		// ���û�а��ļ���������ǰ��
		wss.pop(&nOffset);
	}
	// �����Ǹ�Ŀ¼�µ��ļ�������һ�㲻���ҵ�����ɣ�
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
	// ֱ������ͨ��
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
	// ֱ������ͨ��
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
	// �ڰ���ɾ��Ŀ¼���ļ���һ����
	return	DelFile_In_Pck(cszPath);
}
int		wh_package_fileman::SureOpenPackage(const char *cszPckPath, const char *cszExt, whfileman::OP_MODE_T nOPMode)
{
	if( m_mapPath2Package.has(WHPATHSTRTRANSFORMER(cszPckPath)) )
	{
		// ˵�����Ѿ�����
		return	0;
	}
	char	szFullPath[WH_MAX_PATH];
	sprintf(szFullPath, "%s%s", cszPckPath, cszExt);
	// �ж��ļ��Ƿ���ڣ�����������Ǵ����ļ���ô�Ͳ����ж��ļ��Ƿ�����ˣ�
	if( nOPMode != whfileman::OP_MODE_BIN_CREATE
	&&  m_info.pFM->IsPathExist(szFullPath) )
	{
		// �ж��Ƿ��ǰ��ļ�
		switch( wh_package_base::FileIsWhat(m_info.pFM, szFullPath) )
		{
		case	wh_package_base::FILEISWHAT_PCK:
		case	wh_package_base::FILEISWHAT_TAILPCK:
			break;
		default:
			// ���ļ��𻵣���ô�ͷ��ش�����
			return	-1;
		}
		// ����򿪷�ʽ��Ĭ�Ϲ涨����ͬ��������������ط��ʵ�����»��Զ��򿪰��ģ�����Ͳ��ö��������
		if( nOPMode==whfileman::OP_MODE_UNDEFINED )
		{
			return	0;
		}
		// �򿪷�ʽ��ͬ����ô���������ȴ�һ�°�
		if( !LoadPackage(cszPckPath, cszExt, nOPMode) )
		{
			return	-2;
		}
		return	0;
	}
	// ��֤�㼶Ŀ¼����
	if( m_info.pFM->SureMakeDirForFile(szFullPath)<0 )
	{
		return	-11;
	}
	// ����һ��˵��һ������Ҫ�����հ��ļ�
	wh_package_base::INFO_T	info;
	info.ReadFromOther(m_info);
	info.pBaseFile	= m_info.pFM->Open(szFullPath, whfileman::OP_MODE_BIN_CREATE);
	if( !info.pBaseFile )
	{
		return	-21;
	}
	info.bAutoCloseBaseFile	= true;
	strcpy(info.szDir, cszPckPath);
	info.nOPMode			= whfileman::OP_MODE_BIN_CREATE;		// ����Ϳ϶���Ҫ��create�ˣ����ÿ���nOPMode
	wh_package_base	*pPB	= new wh_package_base;
	assert(pPB);
	if( pPB->Init(&info)<0 )
	{
		delete	pPB;
		return	-22;
	}
	// �����ִ�ӳ�䣨����Ҳ�����ˣ�����ɾ���������������ʹ�þͿ���ֱ�����ˣ�
	if( !m_mapPath2Package.put(WHPATHSTRTRANSFORMER(cszPckPath), pPB) )
	{
		assert(0);	// ��Ӧ��ǰ���Ҳ��������ֲ岻��ȥ:(
		delete	pPB;
		return	-23;
	}
	return	0;
}
wh_package_base *	wh_package_fileman::LoadPackage(const char *cszPckPath, const char *cszExt, whfileman::OP_MODE_T nOPMode)
{
	whPathStrTransformer	WHPST(cszPckPath);

	// �ֿ�һ�°��Ƿ�������Ĵ�ģʽ�������Ƚ��Ƿ���ͬ�������ͬ��assert�ˣ������棩
	m_mapPath2OPMode.get((const char *)WHPST, nOPMode);

	wh_package_base	*pPB	= NULL;
	if( m_mapPath2Package.get((const char *)WHPST, pPB) )
	{
		// ������ֱ����pPB�еõ���Ӧ��package����ָ����
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
	// ������ǰ��ļ�����tail�ļ���
	switch( wh_package_base::FileIsWhat(fp) )
	{
	case	wh_package_base::FILEISWHAT_PCK:
		// ʲôҲ������������
		break;
	case	wh_package_base::FILEISWHAT_TAILPCK:
		{
			if( !m_info.bSearchTailFile )
			{
				// ����Ҫ���tail�ļ�
				return	NULL;
			}
			whtailfile	*tf	= new whtailfile;
			if( tf->Open(fp, true, nOPMode)<0 )
			{
				// ˵��Ҳ����tail�ļ�
				WHSafeSelfDestroy(tf);
				return	NULL;
			}
			// ��tail�ļ���Ϊ���Ļ����ļ�
			fp	= tf;
		}
		break;
	default:
		// �����������Ϊ�򲻿���
		return	NULL;
		break;
	}
	// �򿪰�
	pPB = new wh_package_base;
	assert(pPB);
	wh_package_base::INFO_T	info;
	info.ReadFromOther(m_info);
	info.pBaseFile			= fp;
	info.bAutoCloseBaseFile	= true;
	info.nOPMode			= nOPMode;	// 2007-11-20 ��
	strcpy(info.szDir, (const char *)WHPST);
	int	rst	= pPB->Init(&info);
	if( rst<0 )
	{
		delete	pPB;
		return	NULL;
	}
	// �����ִ�ӳ��
	if( !m_mapPath2Package.put((const char *)WHPST, pPB) )
	{
		assert(0);	// ��Ӧ��ǰ���Ҳ��������ֲ岻��ȥ:(
		delete	pPB;
		return	NULL;
	}
	// �ɹ������������
	return	pPB;
}
int		wh_package_fileman::UnloadPackage(const char *cszPckPath)
{
	// �Ȳ�һ���Ƿ����
	whPathStrTransformer	WHPST(cszPckPath);

	wh_package_base	*pPB	= NULL;
	if( !m_mapPath2Package.get((const char *)WHPST, pPB) )
	{
		// û�У�����ɹ��˰�
		return	0;
	}
	// �ر�package
	delete	pPB;
	// ������ӳ�����ɾ��
	m_mapPath2Package.erase((const char *)WHPST);
	// ������ģʽ��Ӧ����ɾ��
	m_mapPath2OPMode.erase((const char *)WHPST);
	return	0;
}
// �ж�һ��·���Ƿ��ǰ�
const char *	wh_package_fileman::IsPathPackage(whfileman *pFM, const char *cszPath)
{
	char	szFName[WH_MAX_PATH];
	int		i=0;
	while( wh_package_fileman::CSZPCKEXT_All[i]!=NULL )
	{
		// �����ļ���
		sprintf(szFName, "%s%s", cszPath, wh_package_fileman::CSZPCKEXT_All[i]);
		if( pFM->IsPathExist(szFName) )
		{
			switch( wh_package_base::FileIsWhat(pFM, szFName) )
			{
			case	wh_package_base::FILEISWHAT_PCK:
			case	wh_package_base::FILEISWHAT_TAILPCK:
				// ������
				return	wh_package_fileman::CSZPCKEXT_All[i];
			default:
				// ������ʲôҲ����
				break;
			}
		}
		i++;
	}
	return	NULL;
}

// ���ذ����ļ���ԭʼ���ݣ�ѹ��������ݣ������Ľ����
whfile * wh_package_fileman::OpenRawFile(const char *cszFName, OP_MODE_T mode)
{
	whPathStrTransformer	WHPST(cszFName);
	const char *pszWHPST	= (const char *)WHPST;

	whfile * fp = NULL;

	// ���ݲ���ģʽ�ж�����ʲô����ʲô
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
			assert(0 && "�������ļ�����Сд��һ�´���");
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
