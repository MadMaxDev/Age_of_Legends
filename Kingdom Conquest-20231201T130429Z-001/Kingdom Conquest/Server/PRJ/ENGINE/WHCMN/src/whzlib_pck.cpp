// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whzlib
// File: whzlib_pck.cpp
// Creator: Wei Hua (魏华)
// Comment: 和具体打包相关的功能
// CreationDate: 2003-09-04

#include "../inc/whzlib_pck_i.h"
#include "../inc/whdir.h"
#include "../inc/whdes.h"
#include "../inc/whdataini.h"
#include <stdarg.h>

using namespace n_whcmn;

namespace n_whzlib
{

struct	VER	:	public whdataini::obj
{
	string	CurVer;
	string	ValidVer;
	VER()
	{
		CurVer.empty();
		ValidVer.empty();
	}
	WHDATAPROP_DECLARE_MAP(VER)
};
WHDATAPROP_MAP_BEGIN_AT_ROOT(VER)
	WHDATAPROP_ON_SETVALUE_smp(string, CurVer, 0)
	WHDATAPROP_ON_SETVALUE_smp(string, ValidVer, 0)
WHDATAPROP_MAP_END()


////////////////////////////////////////////////////////////////////
// whzlib_pck_notify_console
////////////////////////////////////////////////////////////////////
whzlib_pck_notify_console::whzlib_pck_notify_console()
: m_nDirIndent(0), m_nIndentStep(4)
{
}
int		whzlib_pck_notify_console::myindentprintf(const char *szFmt, ...)
{
	// 打印缩进
	for(int i=0;i<m_nDirIndent;i++)
	{
		putc(' ', stdout);
	}
	// 打印真正的
	int		rst;
	va_list	arglist;
	va_start(arglist, szFmt);
	rst		= vprintf(szFmt, arglist);
	va_end(arglist);
	return	rst;
}
void	whzlib_pck_notify_console::Notify(int nCmd, void *pData, int nSize)
{
	switch(nCmd)
	{
		case	WHZLIB_PCK_NOTIFY_FILE:
		{
			WHZLIB_PCK_NOTIFY_STR_T	*pStrData = (WHZLIB_PCK_NOTIFY_STR_T *)pData;
			myindentprintf("Processing file: %s ... ", pStrData->szStr);
		}
		break;
		case	WHZLIB_PCK_NOTIFY_FILEDONE:
		{
			printf("DONE\r\n");
			m_statinfo.nTotalFile	++;
		}
		break;
		case	WHZLIB_PCK_NOTIFY_ENTERDIR:
		{
			WHZLIB_PCK_NOTIFY_STR_T	*pStrData = (WHZLIB_PCK_NOTIFY_STR_T *)pData;
			myindentprintf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\r\n");
			myindentprintf("Entering Dir  : %s ...\r\n", pStrData->szStr);
			m_nDirIndent	+= m_nIndentStep;
		}
		break;
		case	WHZLIB_PCK_NOTIFY_LEAVEDIR:
		{
			m_nDirIndent	-= m_nIndentStep;
			WHZLIB_PCK_NOTIFY_STR_T	*pStrData = (WHZLIB_PCK_NOTIFY_STR_T *)pData;
			myindentprintf("Leaving Dir   : %s ...\r\n", pStrData->szStr);
			myindentprintf("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\r\n");
			m_statinfo.nTotalDir	++;
		}
		break;
		case	WHZLIB_PCK_NOTIFY_ERR:
		{
			WHZLIB_PCK_NOTIFY_ERR_T	*pErrData = (WHZLIB_PCK_NOTIFY_ERR_T *)pData;
			printf("ERROR(%d): %s", pErrData->nErr, pErrData->szErr);
		}
		break;
	}
}
void	whzlib_pck_notify_console::NotifyStr(int nCmd, const char *szStr)
{
	WHZLIB_PCK_NOTIFY_STR_T	Data;
	Data.szStr	= szStr;
	Notify(nCmd, &Data, sizeof(Data));
}
void	whzlib_pck_notify_console::NotifyInt(int nCmd, int nInt)
{
	WHZLIB_PCK_NOTIFY_INT_T	Data;
	Data.nInt	= nInt;
	Notify(nCmd, &Data, sizeof(Data));
}
void	whzlib_pck_notify_console::NotifyErr(int nCmd, int nErr, const char *szErr)
{
	WHZLIB_PCK_NOTIFY_ERR_T	Data;
	Data.nErr	= nErr;
	Data.szErr	= szErr;
	Notify(nCmd, &Data, sizeof(Data));
}

////////////////////////////////////////////////////////////////////
// whzlib_pck_modifier
////////////////////////////////////////////////////////////////////
whzlib_pck_modifier *	whzlib_pck_modifier::Create(CREATEINFO_T *pInfo)
{
	whzlib_pck_modifier_i	*pObj = new whzlib_pck_modifier_i;
	if( pObj )
	{
		if( pObj->CreatePck(pInfo)<0 )
		{
			delete	pObj;
			pObj	= NULL;
		}
	}
	return	pObj;
}
int		whzlib_pck_modifier::CreateEmptyPck(CREATEINFO_T *pInfo)
{
	whzlib_pck_modifier	*pObj	= Create(pInfo);
	if( !pObj )
	{
		return	-1;
	}
	pObj->AppendDir("");
	pObj->AppendDirEnd();
	delete	pObj;
	return	0;
}

whzlib_pck_modifier *	whzlib_pck_modifier::Open(OPENINFO_T *pInfo)
{
	whzlib_pck_modifier_i	*pObj = new whzlib_pck_modifier_i;
	if( pObj )
	{
		if( pObj->OpenPck(pInfo)<0 )
		{
			delete	pObj;
			pObj	= NULL;
		}
	}
	return	pObj;
}

////////////////////////////////////////////////////////////////////
// WHPckMaker
////////////////////////////////////////////////////////////////////
// 下面用于从目录生成一个打包文件
class	WHPckMaker : public WHDirSearchAction
{
public:
	struct	INFO_T
	{
		const char			*szPckFile;
		const char			*szPassword;
		whzlib_pck_notify	*pNotify;
	};
private:
	whzlib_pck_modifier		*m_pPckModifier;
	whzlib_pck_notify		*m_pNotify;
public:
	WHPckMaker() : m_pPckModifier(NULL), m_pNotify(NULL)
	{
	}
	~WHPckMaker()
	{
		Release();
	}
	int	Init(INFO_T *pInfo)
	{
		whzlib_pck_modifier::CREATEINFO_T	info;
		info.szPckFile	= pInfo->szPckFile;
		m_pPckModifier	= whzlib_pck_modifier::Create(&info);
		if( !m_pPckModifier )
		{
			return	-1;
		}
		m_pPckModifier->SetPassword(pInfo->szPassword);
		m_pNotify		= pInfo->pNotify;
		return	0;
	}
	int	Release()
	{
		if( m_pPckModifier )
		{
			delete	m_pPckModifier;
			m_pPckModifier	= NULL;
		}
		m_pNotify	= NULL;
		return	0;
	}
	int		DoSearch(const char *szPath)
	{
		// 添加总根
		m_pPckModifier->AppendDir("");
		return	WHDirSearchAction::DoSearch(szPath, true);
	}
private:
	// 尾接口实现的
	int		ActionOnFile(const char *szFile)
	{
		char	szErr[256];
		GetFullPath(szFile);

		// 判断文件属性
		if( whfile_ispathsysfile(GetLastFullPath()) )
		{
			// 这个文件没有可写属性，说明可能是系统或隐藏文件，如：该死的Thumb.db
			if( m_pNotify )
			{
				sprintf(szErr, "WARNING: file %s is a system file and be skipped!\r\n"
					, GetLastFullPath()
					);
				m_pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR, WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN, szErr);
			}
			return	0;
		}

		whzlib_file	*file = whzlib_OpenCmnFile(GetLastFullPath(), "rb");
		if( m_pNotify )
		{
			// 通知处理文件
			m_pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_FILE, GetLastFullPath());
		}
		if( !file )
		{
			// 通知错误
			if( m_pNotify )
			{
				sprintf(szErr, "Can not open file:%s !!!", GetLastFullPath());
				m_pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR, WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN, szErr);
			}
			return	-1;
		}
		int	rst	= m_pPckModifier->AppendFile(szFile, file, COMPRESSMODE_ZIP);
		whzlib_CloseCmnFile(file);
		if( m_pNotify )
		{
			m_pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_FILEDONE, GetLastFullPath());
		}
		return	rst;
	}
	int		ActionOnDir(const char *szDir)
	{
		if( m_pNotify )
		{
			m_pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_ENTERDIR, GetFullPath(szDir));
		}
		return	m_pPckModifier->AppendDir(szDir);
	}
	int		LeaveDir(const char *szDir)
	{
		if( m_pNotify )
		{
			// 这个szDir是全路径
			m_pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_LEAVEDIR, szDir);
		}
		return	m_pPckModifier->AppendDirEnd();
	}
};

int	whzlib_pck_make(const char *szDir, const char *szPckFile, const char *szPassword, whzlib_pck_notify *pNotify)
{
	WHPckMaker	pcker;
	WHPckMaker::INFO_T	info;

	info.szPckFile	= szPckFile;
	info.szPassword	= szPassword;
	info.pNotify	= pNotify;

	if( pcker.Init(&info)<0 )
	{
		return	-1;
	}

	return	pcker.DoSearch(szDir);
}

// 下面用于清除打包文件中的浪费空间（Dat和Nam文件会缩水，Idx文件各个相关索引内容会改变）
// 先整理到同目录下的
#define	SAFEOPENFILE(fp, head, ext, mod)		\
	sprintf(szFilePath, "%s.%s", head, ext);	\
	fp	= fopen(szFilePath, mod);				\
	if( !fp )									\
	{											\
		goto	ErrEnd;							\
	}											\
	// EOF SAFEOPENFILE

#define SAFECLOSEFP(fp)	if( fp ) {fclose(fp); fp = NULL;}
int	whzlib_pck_tidy(const char *szOldPckFile, const char *szNewPckFile, whzlib_pck_notify *pNotify)
{
	int		finalrst = 0;
	int		rst, i;
	infofile_hdr_t	infohdr;
	whvector<int>	vectPartIdx;
	whvector<char>	vBuf;
	// 直接读数据即可
	FILE	*fpIn_Idx = NULL, *fpIn_Dat = NULL, *fpIn_Nam = NULL;
	FILE	*fpOut_Idx = NULL, *fpOut_Dat = NULL, *fpOut_Nam = NULL;
	char	szFilePath[WH_MAX_PATH];

	whstrstack	DirStack('/');

	// 把根目录压入栈
	DirStack.push(szOldPckFile);

	// 打开各个输入文件
	SAFEOPENFILE(fpIn_Idx, szOldPckFile, cstIdxFileExt, "rb");
	SAFEOPENFILE(fpIn_Dat, szOldPckFile, cstDatFileExt, "rb");
	SAFEOPENFILE(fpIn_Nam, szOldPckFile, cstNamFileExt, "rb");
	// 打开各个输出文件
	SAFEOPENFILE(fpOut_Idx, szNewPckFile, cstIdxFileExt, "wb");
	SAFEOPENFILE(fpOut_Dat, szNewPckFile, cstDatFileExt, "wb");
	SAFEOPENFILE(fpOut_Nam, szNewPckFile, cstNamFileExt, "wb");

	// 读入文件头
	rst		= fread(&infohdr, 1, sizeof(infohdr), fpIn_Idx);
	if( rst!=sizeof(infohdr) )
	{
		// 错误的文件
		goto	ErrEnd;
	}
	// 清空浪费空间
	infohdr.hdr.nWastedSize	= 0;
	// 写入文件头
	rst		= fwrite(&infohdr, 1, sizeof(infohdr), fpOut_Idx);
	if( rst!=sizeof(infohdr) )
	{
		// 错误的文件
		goto	ErrEnd;
	}

	// 初始化缓冲
	vBuf.resize(infohdr.hdr.nUnitSize);

	// 依次读入各条文件信息，获得文件名和文件数据
	// 写入文件名和文件数，重新调整文件信息中的偏移
	// 写入新文件信息
	while(!feof(fpIn_Idx))
	{
		infofile_fileunit_t	Unit;
		rst	= fread(&Unit, 1, sizeof(Unit), fpIn_Idx);
		if( rst==0 )
		{
			// 读到文件结尾了
			break;
		}
		switch(Unit.type)
		{
			case	infofile_fileunit_t::TYPE_FILE:
			{
				// 文件的总分块数
				int	nTotalPart = whzlib_GetFilePartNum(Unit.info.file.totaldatasize, infohdr.hdr.nUnitSize);
				vectPartIdx.resize(nTotalPart);
				// 走到源文件的开始处
				fseek(fpIn_Dat, Unit.info.file.datapos, SEEK_SET);
				// 读入文件数据，写入新文件
				// 读入文件头
				datafile_file_hdr_t	hdr;
				rst		= fread(&hdr, 1, sizeof(hdr), fpIn_Dat);
				if( rst!=sizeof(hdr)
				||  hdr.hdr.nParts!=nTotalPart )
				{
					assert(0);
					return	-1;
				}
				// 更新在新文件中的数据偏移
				Unit.info.file.datapos	= ftell(fpOut_Dat);
				// 写入文件头
				rst		= fwrite(&hdr, 1, sizeof(hdr), fpOut_Dat);
				if( rst!=sizeof(hdr) )
				{
					assert(0);
					return	-1;
				}
				if( nTotalPart>0 )
				{
					// 读入分段索引
					fread(vectPartIdx.getbuf(), 1, sizeof(int)*nTotalPart, fpIn_Dat);
					// 写入分段索引
					fwrite(vectPartIdx.getbuf(), 1, sizeof(int)*nTotalPart, fpOut_Dat);
					// 依次读入各段数据，并依次写入各段数据
					for(i=0;i<nTotalPart;i++)
					{
						// (这里常用，在这里做文件读写判断即可)
						// 数据头，读入、写入
						datafile_dataunit_hdr_t	dhdr;
						rst	= fread(&dhdr, 1, sizeof(dhdr), fpIn_Dat);
						if( rst!=sizeof(dhdr) )
						{
							goto	ErrEnd;
						}
						rst	= fwrite(&dhdr, 1, sizeof(dhdr), fpOut_Dat);
						if( rst!=sizeof(dhdr) )
						{
							goto	ErrEnd;
						}
						// 数据部
						rst	= fread(vBuf.getbuf(), 1, dhdr.nSize, fpIn_Dat);
						if( rst!=(int)dhdr.nSize )
						{
							goto	ErrEnd;
						}
						rst	= fwrite(vBuf.getbuf(), 1, dhdr.nSize, fpOut_Dat);
						if( rst!=(int)dhdr.nSize )
						{
							goto	ErrEnd;
						}
					}
				}
			}
			// 然后到下面去获得文件名
			case	infofile_fileunit_t::TYPE_DIR:
			{
				// 获得文件名
				whzlib_ReadFileName(fpIn_Nam, Unit.namepos, szFilePath, sizeof(szFilePath));
				// 更新文件名偏移
				Unit.namepos	= ftell(fpOut_Nam);
				// 写入文件名
				whzlib_WriteFileName(fpOut_Nam, szFilePath);

				if( szFilePath[0] )
				{
					DirStack.push(szFilePath);
				}
				if( pNotify )
				{
					switch(Unit.type)
					{
						case	infofile_fileunit_t::TYPE_FILE:
							pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_FILE, DirStack.getwholestr());
							pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_FILEDONE, DirStack.getwholestr());
							DirStack.pop();
						break;
						case	infofile_fileunit_t::TYPE_DIR:
							pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_ENTERDIR, DirStack.getwholestr());
						break;
					}
				}
			}
			break;
			case	infofile_fileunit_t::TYPE_EOD:
			{
				if( pNotify )
				{
					pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_LEAVEDIR, DirStack.getwholestr());
					DirStack.pop();
				}
			}
			break;
		}
		// 信息数据写入
		if( fwrite(&Unit, 1, sizeof(Unit), fpOut_Idx) != sizeof(Unit) )
		{
			goto	ErrEnd;
		}
	}

End:
	SAFECLOSEFP(fpIn_Idx);
	SAFECLOSEFP(fpIn_Dat);
	SAFECLOSEFP(fpIn_Nam);
	SAFECLOSEFP(fpOut_Idx);
	SAFECLOSEFP(fpOut_Dat);
	SAFECLOSEFP(fpOut_Nam);

	return	finalrst;
ErrEnd:
	finalrst	= -1;
	goto	End;
}

// 这个和tidy的步骤类似，所以可以修改上面的函数
int	whzlib_pck_extract(const char *szPckFile, const char *szDir, const char *szPassword, whzlib_pck_notify *pNotify)
{
	int		finalrst = 0;
	int		rst, i, nIdx;
	infofile_hdr_t	infohdr;
	whvector<int>	vectPartIdx;
	whvector<char>	vBuf, vBufRaw;
	FILE	*fpIn_Idx = NULL, *fpIn_Dat = NULL, *fpIn_Nam = NULL;
	char	szFilePath[WH_MAX_PATH];		// 用于临时获取文件名
	const char	*szOutFile = NULL;				// 输出文件或目录的全路径名
	whstrstack	DirStack('/');

	// 确保目标目录存在
	if( whdir_MakeDir(szDir)<0 )
	{
		return	-1;
	}

	WHSimpleDES	des;
	if( szPassword && szPassword[0] )
	{
		WHDES_ConvertToFixed(szPassword, strlen(szPassword), (char *)des.m_key, sizeof(des.m_key));
		des.setmask(szPassword, strlen(szPassword));
	}

	// 把根目录压入栈
	DirStack.push(szDir);

	// 打开各个输入文件
	SAFEOPENFILE(fpIn_Idx, szPckFile, cstIdxFileExt, "rb");
	SAFEOPENFILE(fpIn_Dat, szPckFile, cstDatFileExt, "rb");
	SAFEOPENFILE(fpIn_Nam, szPckFile, cstNamFileExt, "rb");

	// 读入文件头
	rst		= fread(&infohdr, 1, sizeof(infohdr), fpIn_Idx);
	if( rst!=sizeof(infohdr) )
	{
		// 错误的文件
		goto	ErrEnd;
	}
	// 判断版本
	if( !infohdr.IsMagicGood()
	||  !infohdr.IsVerGood()
	)
	{
		goto	ErrEnd;
	}

	// 初始化缓冲
	vBuf.resize(infohdr.hdr.nUnitSize);
	vBufRaw.resize(infohdr.hdr.nUnitSize);

	// 依次读入各条文件信息，获得文件名和文件数据
	// 写入文件名和文件数，重新调整文件信息中的偏移
	// 写入新文件信息
	nIdx	= 0;
	while(!feof(fpIn_Idx))
	{
		infofile_fileunit_t	Unit;

		rst	= fread(&Unit, 1, sizeof(Unit), fpIn_Idx);
		if( rst==0 )
		{
			// 读到文件结尾了
			break;
		}
		// 获得文件或目录名
		switch(Unit.type)
		{
			case	infofile_fileunit_t::TYPE_FILE:
			case	infofile_fileunit_t::TYPE_DIR:
			{
				if( whzlib_ReadFileName(fpIn_Nam, Unit.namepos, szFilePath, sizeof(szFilePath))<0 )
				{
					assert(0);
					goto	ErrEnd;
				}
				if( szFilePath[0]==0 )
				{
					// 根目录就不用处理了(外面已经压入栈了)
					continue;
				}
				DirStack.push(szFilePath);
				szOutFile	= DirStack.getwholestr();
			}
			break;
		}
		// 文件或目录数据
		switch(Unit.type)
		{
			case	infofile_fileunit_t::TYPE_FILE:
			{
				if( pNotify )
				{
					pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_FILE, szOutFile);
				}
				// 文件的总分块数
				int	nTotalPart = whzlib_GetFilePartNum(Unit.info.file.totaldatasize, infohdr.hdr.nUnitSize);
				vectPartIdx.resize(nTotalPart);
				// 走到源文件的开始处
				fseek(fpIn_Dat, Unit.info.file.datapos, SEEK_SET);
				// 读入文件数据，写入新文件
				// 读入文件头
				datafile_file_hdr_t	hdr;
				rst		= fread(&hdr, 1, sizeof(hdr), fpIn_Dat);
				if( rst!=sizeof(hdr)
				||  hdr.hdr.nParts!=nTotalPart )
				{
					assert(0);
					goto	ErrEnd;
				}
				// 创建文件
				FILE	*fpOut = fopen(szOutFile, "wb");
				if( !fpOut )
				{
					if( pNotify )
					{
						char	szErr[256];
						sprintf(szErr, "Can not open output file:%s\r\n", szOutFile);
						pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR, WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN, szErr);
					}
					goto	ErrEnd;
				}
				// 读入分段索引
				if( nTotalPart>0 )
				{
					fread(vectPartIdx.getbuf(), 1, sizeof(int)*nTotalPart, fpIn_Dat);

					// 依次读入各段数据，并解压写入文件
					for(i=0;i<nTotalPart;i++)
					{
						// (这里常用，在这里做文件读写判断即可)
						// 读入数据头
						datafile_dataunit_hdr_t	dhdr;
						size_t	nRawSize = 0;
						rst	= fread(&dhdr, 1, sizeof(dhdr), fpIn_Dat);
						if( rst!=sizeof(dhdr) )
						{
							goto	ErrEnd;
						}
						// 读入数据部
						rst	= fread(vBuf.getbuf(), 1, dhdr.nSize, fpIn_Dat);
						if( rst!=(int)dhdr.nSize )
						{
							goto	ErrEnd;
						}
						if( szPassword && szPassword[0] )
						{
							// 如果有密码就解压
							des.decrypt( (unsigned char*)vBuf.getbuf(), dhdr.nSize );
						}
						// 看情况解压
						switch( dhdr.nMode )
						{
							case	COMPRESSMODE_ZIP:
							{
								nRawSize	= infohdr.hdr.nUnitSize;
								if( whzlib_decompress(vBuf.getbuf(), dhdr.nSize, vBufRaw.getbuf(), &nRawSize)<0 )
								{
									assert(0);
									goto	ErrEnd;
								}
							}
							break;
							default:
							{
								// 直接拷贝
								nRawSize	= dhdr.nSize;
								memcpy(vBufRaw.getbuf(), vBuf.getbuf(), nRawSize);
							}
							break;
						}
						// 写入数据部
						if( fwrite(vBufRaw.getbuf(), 1, nRawSize, fpOut)!=nRawSize )
						{
							assert(0);
							goto	ErrEnd;
						}
					}
				}
				// 关闭文件
				fclose(fpOut);
				// 设置文件时间
				whfile_setmtime(szOutFile, Unit.time);
				if( pNotify )
				{
					pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_FILEDONE, szOutFile);
				}
				// 对于文件就要在这里弹出
				DirStack.pop();
			}
			break;
			case	infofile_fileunit_t::TYPE_DIR:
			{
				// 创建目录
				if( pNotify )
				{
					pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_ENTERDIR, szOutFile);
				}
				if( whdir_MakeDir(szOutFile)<0 )
				{
					assert(0);
					goto	ErrEnd;
				}
			}
			break;
			case	infofile_fileunit_t::TYPE_EOD:
			{
				// 退出目录
				if( pNotify )
				{
					pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_LEAVEDIR, DirStack.getwholestr());
				}
				// 目录在整理弹出，在所有文件处理完毕后，根目录也在这里弹出了
				DirStack.pop();
			}
			break;
		}
		nIdx	++;
	}
	
End:
	SAFECLOSEFP(fpIn_Idx);
	SAFECLOSEFP(fpIn_Dat);
	SAFECLOSEFP(fpIn_Nam);

	return		finalrst;
ErrEnd:
	finalrst	= -1;
	if( pNotify )
	{
		pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR, finalrst, "ERROR!!!");
	}
	goto		End;
}

int	whzlib_pck_patch(const char *szPckFile, const char **aszPckList, const char *szDstDir, int nManProperty, const char *szPassword, whzlib_pck_notify *pNotify)
{
	whzlib_filemodifyman	*pFMan = whzlib_filemodifyman::Create();
	if( !pFMan )
	{
		return	-1;
	}
	pFMan->m_nProperty		= nManProperty;
	pFMan->SetPassword(szPassword);
	WHDirInPck::m_pFMan		= whzlib_fileman::Create();
	WHDirInPck::m_pFMan->SetSearchMode(whzlib_fileman::SEARCHMODE_PCKONLY);
	WHDirInPck::m_pFMan->SetPassword(szPassword);


	// 浏览包的SAA
	class	MySAA	: public WHDirSearchActionTmpl<WHDirInPck>
	{
	public:
		whzlib_filemodifyman	*m_pFMan;
		whzlib_pck_notify		*m_pNotify;
		bool					m_bShoudlInPck;
		whstrarr				*m_pPckNameList;
		const char				*m_szDstDir;
	public:
		MySAA()
		: m_pFMan(NULL), m_pNotify(NULL), m_bShoudlInPck(false)
		, m_pPckNameList(NULL), m_szDstDir(NULL)
		{
		}
		void	Reset()
		{
			m_bShoudlInPck	= false;
		}
	private:
		bool	CheckIfStop()
		{
			if( m_pNotify )
			{
				if( m_pNotify->ShouldQuit() )
				{
					StopSearch();
					return	true;
				}
			}
			return	false;
		}
		int		ActionOnFile(const char *szFile)
		{
			if( CheckIfStop() )
			{
				return	-1;
			}

			char	szErr[256];
			whzlib_file	*file = WHDirInPck::m_pFMan->Open(GetFullPath(szFile), "rb");
			if( m_pNotify )
			{
				// 通知处理文件
				m_pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_FILE, GetLastFullPath());
			}
			if( !file )
			{
				// 通知错误
				if( m_pNotify )
				{
					sprintf(szErr, "Can not open file in pck:%s !!!\r\n", GetLastFullPath());
					m_pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR, WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN, szErr);
				}
				return	-1;
			}
			char	szDstFile[WH_MAX_PATH];
			sprintf(szDstFile, "%s%s/%s", m_szDstDir, GetCurRelPath(), szFile);
			if( m_pNotify )
			{
				m_pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_FILEDONE, GetLastFullPath());
			}
			int	rst	= m_pFMan->PutFile(szDstFile, file, m_bShoudlInPck, COMPRESSMODE_ZIP);
			if( rst<0 )
			{
				// 通知错误
				if( m_pNotify )
				{
					sprintf(szErr, "Can not patch file :%s !!!\r\n", szDstFile);
					m_pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR, WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN, szErr);
				}
				WHDirInPck::m_pFMan->Close(file);
				return	-1;
			}
			WHDirInPck::m_pFMan->Close(file);
			return	0;
		}
		int		ActionOnDir(const char *szDir)
		{
			if( CheckIfStop() )
			{
				return	-1;
			}

			if( GetPathLevel()==1 && m_pPckNameList )
			{
				m_bShoudlInPck	= m_pPckNameList->findstr(szDir, true)>=0;
			}
			return	0;
		}
		int		LeaveDir(const char *szDir)
		{
			if( GetPathLevel()==1 )
			{
				m_bShoudlInPck	= false;
			}
			return	0;
		}
	};

	whstrarr		pcknamelist(aszPckList, 0);

	MySAA				saa;
	saa.m_pFMan			= pFMan;
	saa.m_pNotify		= pNotify;
	saa.m_pPckNameList	= &pcknamelist;
	saa.m_szDstDir		= szDstDir;
	
	// 开始patch
	int	rst = saa.DoSearch(szPckFile, true);

	delete	WHDirInPck::m_pFMan;
	delete	pFMan;
	return	rst;
}

#ifdef	__GNUC__
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L
#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
static int	MessageBox(void *ptr, const char *szText, const char *szCaption, int nType)
{
	return	0;
}
#endif
int	whzlib_pck_checkver(const char *szPckFile,const char *szDstDir,const char *szPassword,whzlib_pck_notify *pNotify)
{
	whzlib_pck_reader_i reader;
	whzlib_file_i_pck	*pckFile;
	FILE				*fp;
	char				buffer[1024];
	string				text,line;
	char				szErr[255];
	int					pos = 0,lastpos = 0;

	whdataini						ini;
	whdataini::dftlineanalyzer		analyzer;
	whdataini::objgetter			getter4pck;
	whdataini::objgetter			getter4file;

	VER					ver4file;
	VER					ver4pck;

	reader.SetPassword(szPassword);

	ini.setlineanalyzer(&analyzer);
	ini.setgetter(&getter4file);
	getter4file.addobj("[Ver]",&ver4file);

	//处理目标目录下的ver.txt
	memset(buffer,0,1024);
	sprintf(buffer,"%s\\%s",szDstDir,"CVer.txt");
	fp = fopen(buffer,"r");
	if(!fp)
	{
		//ver文件未找到
		memset(szErr,0,255);
		sprintf(szErr, "当前目录中找不到版本信息文件CVer.txt!\r\n");
		pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR,WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN,szErr);
		sprintf(szErr, "当前目录中找不到版本信息文件CVer.txt!\r\n继续更新吗?第一次更新请点击确定.");
		int rst = MessageBox(NULL,szErr,NULL,MB_YESNO);
		if(rst == IDYES)	return 0;
		return -1;
	}
	memset(buffer,0,1024);
	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);
	fread(buffer,1,size,fp);
	text = buffer;
	ini.ResetObjStack();
	while((pos = text.find("\n",lastpos)) >= 0)
	{
		line = text.substr(lastpos,pos - lastpos);
		memset(&analyzer.m_szParam[1],0,1024);
		ini.analyzeline(line.c_str(), 0);
		lastpos = pos + 1;
	}
	if(lastpos < (int)text.length())
	{
		line = text.substr(lastpos,text.length() - lastpos);
		if(line.length() > 0)
		{
			ini.analyzeline(line.c_str(), 0);
		}
	}
	ini.setgetter(&getter4pck);
	getter4pck.addobj("[Ver]",&ver4pck);

	//处理包中的ver.txt
	reader.OpenPck(szPckFile);
	pckFile = reader.OpenFile("CVer.txt");
	if(!pckFile)
	{
		//ver文件未找到
		memset(szErr,0,255);
		sprintf(szErr, "更新包中找不到版本信息文件CVer.txt.\r\n");
		pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR,WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN,szErr);
		sprintf(szErr, "更新包中找不到版本信息文件CVer.txt.\r\n是否继续更新?");
		int rst = MessageBox(NULL,szErr,NULL,MB_YESNO);
		if(rst == IDYES)	return 0;
		return -1;
	}
	memset(buffer,0,1024);
	pckFile->Read(buffer,pckFile->FileSize());
	delete	pckFile;
	text = buffer;
	ini.ResetObjStack();
	pos = lastpos =0;
	while((pos = text.find("\n",lastpos)) >= 0)
	{
		line = text.substr(lastpos,pos - lastpos);
		memset(&analyzer.m_szParam[1],0,1024);
		ini.analyzeline(line.c_str(), 0);
		lastpos = pos + 1;
	}
	if(lastpos < (int)text.length())
	{
		line = text.substr(lastpos,text.length() - lastpos);
		if(line.length() > 0)
		{
			ini.analyzeline(line.c_str(), 0);
		}
	}
	memset(szErr,0,255);
	//说明没有要求,所有版本都可以更新
	if(stricmp(ver4pck.ValidVer.c_str(),"")==0)	return 0;
	//后面是版本判断
	if(stricmp(ver4pck.CurVer.c_str(),"")==0 || stricmp(ver4file.CurVer.c_str(),"")==0)
	{
		sprintf(szErr, "版本文件: CVer.txt 中找不到正确的版本信息!\r\n");
		pNotify->NotifyErr(WHZLIB_PCK_NOTIFY_ERR,WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN,szErr);
		sprintf(szErr, "版本文件: CVer.txt 中找不到正确的版本信息!\r\n是否继续更新?");
		int rst = MessageBox(NULL,szErr,NULL,MB_YESNO);
		if(rst == IDYES)	return 0;
		return -1;
	}
	if(stricmp(ver4file.CurVer.c_str(),ver4pck.CurVer.c_str())==0)
	{
		sprintf(szErr, "当前版本已经是最新的版本!\r\n");
		pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_NOERR_WORKDONE,szErr);
		sprintf(szErr, "当前版本已经是最新的版本!\r\n是否继续更新?");
		int rst = MessageBox(NULL,szErr,NULL,MB_YESNO);
		if(rst == IDYES)	return 0;
		return -1;
	}
	if(stricmp(ver4file.CurVer.c_str(),ver4pck.CurVer.c_str())>0)
	{
		sprintf(szErr, "更新包中的文件版本比现有文件旧,请下载最新的更新包!\r\n");
		pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_NOERR_WORKDONE,szErr);
		sprintf(szErr, "更新包中的文件版本比现有文件旧,请下载最新的更新包!\r\n是否强制更新?");
		int rst = MessageBox(NULL,szErr,NULL,MB_YESNO);
		if(rst == IDYES)	return 0;
		return -1;
	}
	if(stricmp(ver4file.CurVer.c_str(),ver4pck.ValidVer.c_str())<0)
	{
		sprintf(szErr, "您的游戏版本太旧,请先更新到: %s 再执行本升级包!\r\n",ver4pck.ValidVer.c_str());
		pNotify->NotifyStr(WHZLIB_PCK_NOTIFY_NOERR_WORKDONE,szErr);
		sprintf(szErr, "您的游戏版本太旧,请先更新到: %s 再执行本升级包!\r\n点击确定退出.",ver4pck.ValidVer.c_str());
		MessageBox(NULL,szErr,"Exit",MB_OK);
		return -1;
	}

	else if(stricmp(ver4pck.CurVer.c_str(),ver4file.CurVer.c_str())>0 && stricmp(ver4pck.ValidVer.c_str(),ver4file.CurVer.c_str())<=0)
	{
		return 0;
	}

	return -1;
}

}		// EOF namespace n_whzlib
