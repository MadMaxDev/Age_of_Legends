// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: 无
// File         : whzio.h
// Creator      : Wei Hua (魏华)
// Comment      : 对打包文件读取的包装，使用要类似stdio。
//              : 注意：不能多线程使用!!!!
//              : 鸣谢：代码中使用了Wei Dai的CryptoPP中的zlib实现。2006-01-23。
// CreationDate : 2003-11-27

#ifndef	__WHZIO_H__
#define	__WHZIO_H__

#include "whzlib.h"

#ifdef	USE_WHZIO
typedef	n_whzlib::whzlib_file			WHZFILE;
typedef	n_whcmn::WHDirBase				WHZDIR;
typedef	n_whcmn::WHDirBase::ENTRYINFO_T	WHZDIRENTRY;
enum
{
	WHZIO_SMODE_1PCK2FILE			= n_whzlib::whzlib_fileman::SEARCHMODE_1PCK2FILE,
	WHZIO_SMODE_1FILE2PCK			= n_whzlib::whzlib_fileman::SEARCHMODE_1FILE2PCK,
	WHZIO_SMODE_PCKONLY				= n_whzlib::whzlib_fileman::SEARCHMODE_PCKONLY,
	WHZIO_SMODE_FILEONLY			= n_whzlib::whzlib_fileman::SEARCHMODE_FILEONLY,
};

// 初始化(bCheckPckFirst表示是否优先从包中找文件)
bool	WHZIO_Init(int nSearchMode = WHZIO_SMODE_1PCK2FILE);
// 终结
bool	WHZIO_Release();
// 预装载包(这样可以减少程序中载入文件索引耗时太长)
int		WHZIO_LoadPckToMem(const char *szPckName);
int		WHZIO_UnLoadPckFromMem(const char *szPckName);
// 预装载文件
int		WHZIO_LoadFileToMem(const char *szFileName);
int		WHZIO_UnLoadFileFromMem(const char *szFileName);

// 设置解压密码
void	WHZIO_SetPassword(const char *szPassword);

// **** 文件操作 ****
// 打开文件
// 如果szOPMode不为空并且不为"rb"则按照文件方式打开，并且可以从里面获得FILE指针
WHZFILE *	WHZIO_Open(const char *szFileName, const char *szOPMode = NULL);
// 关闭文件
int		WHZIO_Close(WHZFILE *fp);
// 获得系统文件指针(如果有的话)
FILE *	WHZIO_GetFILEPtr(WHZFILE *fp);
// 获得文件长度
int		WHZIO_GetFileSize(const char *szFileName);
int		WHZIO_GetFileSize(WHZFILE *fp);
// 获得文件md5
int		WHZIO_GetFileMD5(WHZFILE *fp, unsigned char *MD5);
// 获得文件时间
time_t	WHZIO_GetFileTime(WHZFILE *fp);
// 判断一个文件是否存在
bool	WHZIO_IsFileExist(const char *szFileName);
// 读取文件(返回读到的字节数)
// 如果读到文件尾则返回0，出错返回<0
int		WHZIO_Read(WHZFILE *fp, void *pBuf, int nSize);
// 这个是为了和fread统一（比如在ogg的读取中的回调）
int		WHZIO_FRead(void *pBuf, size_t nSize, size_t nCount, WHZFILE *fp);
// 读取一行。返回行的字节数，行包含结束符"\n"或"\r\n"，文件里是什么就是什么。
int		WHZIO_ReadLine(WHZFILE *fp, char *pBuf, int nSize);
// 移动文件指针
// nOrigin使用和fseek参数一样的SEEK_SET、SEEK_CUR和SET_END
// 提供反向seek
// 返回0表示成功，<0表示出错了
// 如：WHZIO_Seek(fp, SEEK_SET, 100);
int		WHZIO_Seek(WHZFILE *fp, int nOffset, int nOrigin);
// 判断是否到了文件结尾
bool	WHZIO_IsEOF(WHZFILE *fp);
// 告诉当前文件指针的位置
int		WHZIO_Tell(WHZFILE *fp);

// **** 目录操作 ****
// 打开目录
WHZDIR *	WHZIO_OpenDir(const char *szDirName);
// 关闭目录
int		WHZIO_CloseDir(WHZDIR *dp);
// 读取一个目录条目，如果没有了就返回NULL
WHZDIRENTRY *	WHZIO_ReadDir(WHZDIR *dp);

#endif	// EOF USE_WHZIO

#endif	// EOF __WHZIO_H__
