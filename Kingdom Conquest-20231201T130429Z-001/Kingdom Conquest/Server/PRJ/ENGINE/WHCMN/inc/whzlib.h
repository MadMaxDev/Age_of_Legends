// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whzlib
// File        : whzlib.h
// Creator     : Wei Hua (魏华)
// Comment     : 对zlib功能的包装和扩充，目前依赖于lzo (lzo压缩率好像比zlib差一些，不过速度快些)
// CreationDate: 2003-09-04
// ChangeLOG   :
//               2004-02-16 增加了单一文件压缩
//               2006-01-23 把uncompress改名为decompress

#ifndef	__WHZLIB_H__
#define __WHZLIB_H__

#include <stdio.h>
#include <time.h>
#include "whdir.h"

// 压缩包版本
// 当前使用的版本
#define WHZLIB_VER			0x00000002
// 支持读取的最低版本
#define WHZLIB_LOWRVER		0x00000002

// 单个压缩文件版本
// 当前使用的版本
#define WHZLIB_SC_VER		0x00000001
// 支持读取的最低版本
#define WHZLIB_SC_LOWRVER	0x00000001

namespace n_whzlib
{

enum
{
	WHZLIB_MAX_RAWDATAUNIT_SIZE	= 65536,		// 其实也可以更大，反正里面用的是uint32
};

////////////////////////////////////////////////////////////////////
// 类接口
////////////////////////////////////////////////////////////////////
// 基于zlib的文件对象
// 它只能通过whzlib_fileman::Open创建出来，通过whzlib_fileman::Close方法销毁
typedef	n_whcmn::whfile	whzlib_file;

// 基于zlib的文件读访问管理器
class	whzlib_fileman
{
public:
	enum
	{
		SEARCHMODE_1PCK2FILE			= 0,
		SEARCHMODE_1FILE2PCK			= 1,
		SEARCHMODE_PCKONLY				= 2,
		SEARCHMODE_FILEONLY				= 3,
	};
	// 创建与销毁
	static whzlib_fileman *		Create();
	virtual ~whzlib_fileman()	{}
public:
	// 是否优先从包文件中检索(默认是true，即先检查是包中的文件，如果没有再检查是否存在文件)
	virtual void	SetSearchMode(int nMode)				= 0;
	// 获得检索模式
	virtual int		GetSearchMode() const					= 0;
	// 设置解压密码
	virtual void	SetPassword(const char *szPassword)		= 0;
	// 预先加载包文件到内存
	virtual int		LoadPckToMem(const char *szPckName)		= 0;
	virtual int		UnLoadPckFromMem(const char *szPckName)	= 0;
	// 预先加载单个文件到内存
	virtual int		LoadFileToMem(const char *szFileName)	= 0;
	virtual int		UnLoadFileFromMem(const char *szFileName)
															= 0;
	// szFileName最好是全路径名(否则会按照当前路径查找，在当前路径变了时可能会出现混乱)
	// 如果szFileName是实际存在的文件，则按照文件打开
	// 如果szFileName不存在，则找到承载文件的压缩包，从包中打开
	// 如果szMode为空，或者等于"rb"则按照默认的方式打开，否则只按磁盘文件方式打开
	virtual whzlib_file *	Open(const char *szFileName, const char *szMode=NULL)
															= 0;
	// 关闭文件
	virtual bool	Close(whzlib_file *file)				= 0;
	// 获得文件长度
	virtual int		GetFileSize(const char *szFileName)		= 0;
	// 判断文件是否存在
	virtual bool	IsFileExist(const char *szFileName)		= 0;
	// 和目录相关的(使用完毕自行delete即可)
	// 打开并创建
	virtual n_whcmn::WHDirBase *	OpenDir(const char *szDirName)
															= 0;
};

// 对zlib的文件更新访问管理器
class	whzlib_filemodifyman
{
public:
	// 创建与销毁
	static whzlib_filemodifyman *	Create();
	virtual ~whzlib_filemodifyman()	{}
public:
	enum
	{
		PROPERTY_AUTOCREATEPCK		= 0x0001,				// 自动创建包(即putfile如果bShouldInPck，但没有发现包，则自动创建之)
	};
	unsigned int	m_nProperty;							// 这个可以由外界自由访问、更改
	int				m_nUnitSize;							// 单个数据块压缩前的最大长度。
public:
	whzlib_filemodifyman()
	: m_nProperty(0)
	, m_nUnitSize(WHZLIB_MAX_RAWDATAUNIT_SIZE)
	{
	}
	// 设置解压密码
	virtual void	SetPassword(const char *szPassword)		= 0;
	// 设置属性
	// 替换或添加文件
	// bShouldInPck表示应该放在包中，找不到包就是错误
	// nCompressMode=COMPRESSMODE_XXX
	virtual int		PutFile(const char *szFileName, whzlib_file *file, bool bShouldInPck, int nCompressMode)
															= 0;
	// 删除文件
	virtual int		DelFile(const char *szFileName)			= 0;
	// 删除目录
	virtual int		DelDir(const char *szDir)				= 0;

	// 将所有内容存储到硬盘上
	virtual int		FlushAll()								= 0;
};

////////////////////////////////////////////////////////////////////
// 功能函数
////////////////////////////////////////////////////////////////////
// 直接从普通文件创建whzlib_file对象
// szMode一般就是"rb"
inline whzlib_file *	whzlib_OpenCmnFile(const char *szFileName, const char *szMode)
{
	return	n_whcmn::whfile_OpenCmnFile(szFileName, szMode);
}
// 关闭普通文件对象
inline void				whzlib_CloseCmnFile(whzlib_file *file)
{
	n_whcmn::whfile_CloseCmnFile(file);
}

// 从内存创建whzlib_file对象
inline whzlib_file *	whzlib_OpenMemBufFile(char *szMemBuf, size_t nMemSize)
{
	return	n_whcmn::whfile_OpenMemBufFile(szMemBuf, nMemSize);
}
inline	void			whzlib_CloseMemBufFile(whzlib_file *file)
{
	n_whcmn::whfile_CloseMemBufFile(file);
}

// 直接内存压缩函数
// 说明：*pnDstSize在输入时存放pDst最多能容纳的字节数
// 返回值<0表示出错
// 压缩
int	whzlib_compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize);
// 解压
int	whzlib_decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize);

// 判断压缩包是否存在
bool	whzlib_ispckexist(const char *cszPck);

// 获得三个包文件的名字
struct	PCKFILENAMES_T
{
	char	szIdx[WH_MAX_PATH];
	char	szDat[WH_MAX_PATH];
	char	szNam[WH_MAX_PATH];
};
int	whzlib_getpckfilenames(const char *cszPck, PCKFILENAMES_T *pNames);

////////////////////////////////////////////////////////////////////
// 单一文件压缩（实现见whzlib_i.h）
////////////////////////////////////////////////////////////////////

// 将通用zip文件流压缩到指定的磁盘文件
int		whzlib_single_compress(whzlib_file *file, const char *cszDstFile, const char *cszPasswd);
// 源文件流为被压缩的文件格式，将其变成通用zip文件流
// 注意关闭时，要先关闭后产生的，再关闭srcfile。
whzlib_file *	whzlib_OpenSingleCompress(whzlib_file *srcfile, const char *cszPasswd);
void			whzlib_CloseSingleCompress(whzlib_file *singlefile);
// 判断一个文件流是否是single compressed (注意，这个判断会改变文件内部的偏移指针)
bool			whzlib_issinglecompressed(whzlib_file *srcfile);

}		// EOF namespace n_whzlib

#endif	// EOF __WHZLIB_H__
