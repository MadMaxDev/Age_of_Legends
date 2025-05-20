// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whfile.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 文件功能
//              : 注意：feof是一个宏，不能加"::"前缀
// CreationDate : 2003-06-13
// ChangeLOG    : 2004-06-14 whfile_cpy增加了参数,对于源文件不存在的按正确处理.
//                2004-07-16 whfile_getfilename修正了原来获得文件名会有一个'\'号的问题
//                2004-12-14 whfile_crc_readfile在文件长度为0的情况下，认为文件内容为0（这个主要是为了将来删除文件方便）
//                2005-10-09 xxx_crc_readfile_xxx中修正了文件头损环导致长度异常的bug。同时增加停电导致文件变长不认为是错误文件的功能。
//                2006-08-11 whfile_ispathdir里面增加了对于d:或//xman/write这样情况判断结果为非目录的补充判断。
//                2006-10-19 增加了几个filexxx::Seek中对未知origin的assert
//                2007-08-01 在linux下也要过滤连续的斜杠

#include "../inc/whcmn_def.h"
#include "../inc/whfile.h"
#include "../inc/whdir.h"
#include "../inc/whtime.h"
#include "../inc/whbits.h"
#include "../inc/whstring.h"
#include "../inc/whcrc.h"

// 大家都用的头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#if defined( __GNUC__ )			// Linux下的头文件			{
#include <unistd.h>
#include <utime.h>
#endif							// EOF Linux下的头文件		}
#if defined( WIN32 )			// Windows下的头文件		{
#include <io.h>
#include <sys/utime.h>
#endif							// EOF Windows下的头文件	}

using namespace n_whcmn;

namespace n_whcmn
{

bool	whfile_isabspath(const char *__path)
{
#ifdef	WIN32
	if( __path[1]==':'
	//&&  (__path[2]=='\\' || __path[2]=='/')	因为如果写d:tmp也不能把它当成相对路径来算
	)
	{
		return	true;
	}
	if(__path[0]=='\\')
	{
		return	true;
	}
#endif
	if(__path[0]=='/')
	{
		return	true;
	}
	return		false;
}

bool	whfile_ispathdir(const char *__path)
{
	struct stat	st;
	int			rst;
	rst			= stat(__path, &st);
	if( rst!=0 )
	{
#ifdef	WIN32
		// 不能打开，判断一下是否是D:或//xman/write这样的，如果是加上最后的斜杠再判断
		int	nLen	= strlen(__path);
		if( __path[nLen-1]!='/' && __path[nLen-1]!='\\' )
		{
			switch( __path[1] )
			{
			case	':':
			case	'/':
			case	'\\':
				break;
			default:
				// 不是期望的那种格式
				return	false;
			}
			whvector<char>	vect(nLen+2);
			memcpy(vect.getbuf(), __path, nLen);
			vect[nLen]		= '/';
			vect[nLen+1]	= 0;
			rst				= stat(vect.getbuf(), &st);
			if( rst!=0 )
			{
				return		false;
			}
		}
		else
		{
			// 普通目录最后加斜杠也会判断不出来
			// 清除后面所有的连续斜杠
			whvector<char>	vect(nLen);
			memcpy(vect.getbuf(), __path, nLen);
			while( nLen>0 )
			{
				nLen--;
				switch( vect[nLen] )
				{
				case	'/':
				case	'\\':
					vect[nLen]	= 0;
					break;
				default:
					nLen	= 0;
					break;
				}
			}
			rst				= stat(vect.getbuf(), &st);
			if( rst!=0 )
			{
				return		false;
			}
		}
#else
		return	false;
#endif
	}
	// 存在但是不是目录
	if( (st.st_mode&S_IFDIR)==0 )
	{
		return	false;
	}
	return	true;
}

bool	whfile_ispathexisted(const char *__path)
{
	if( __path[0]==0 )
	{
		// 代表最根的目录吧
		return	true;
	}
	return	access(__path, F_OK) == 0;
}
bool	whfile_ispathreadable(const char *__path)
{
	if( __path[0]==0 )
	{
		return	false;
	}
	return	access(__path, R_OK) == 0;
}
bool	whfile_ispathwritable(const char *__path)
{
	if( __path[0]==0 )
	{
		return	false;
	}
	if( access(__path, W_OK) != 0 )
	{
		return	false;
	}

	// 判断是否是系统或隐藏文件
#ifdef	WIN32
	DWORD	dwAttr = GetFileAttributes(__path);
	if( dwAttr & (FILE_ATTRIBUTE_SYSTEM|FILE_ATTRIBUTE_READONLY) )
	{
		return	false;
	}
#endif

	return		true;
}
bool	whfile_ispathsysfile(const char *__path)
{
#ifdef	WIN32
	DWORD	dwAttr = GetFileAttributes(__path);
	if( dwAttr & FILE_ATTRIBUTE_SYSTEM )
	{
		return	true;
	}
#endif
	return		false;
}

bool	whfile_makefilewritable(const char *__path)
{
#ifdef	WIN32
	if( SetFileAttributes(__path, FILE_ATTRIBUTE_NORMAL) )
	{
		return	true;
	}
#endif
#ifdef	__GNUC__
	if( chmod(__path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)==0 )
	{
		return	true;
	}
#endif
	return	false;
}

whint64	whfile_getfilebigsize(const char *__path)
{
	struct stat	st;
	int			rst;
	rst			= stat(__path, &st);
	if( rst!=0 || (st.st_mode&S_IFDIR)!=0 )
	{
		return	-1;
	}
	return	st.st_size;
}

int		whfseek(FILE *fp, whint64 nOffset, int nOrigin)
{
	const whint64	cnMaxSmallOffset	= 0x7fffff00;
	if( nOffset<=cnMaxSmallOffset && nOffset>=-cnMaxSmallOffset )
	{
		return	::fseek(fp, (int)nOffset, nOrigin);
	}
	int		rst		= 0;
	int	nStep	= cnMaxSmallOffset;
	bool	bFwd	= true;
	if( nOffset<0 )
	{
		bFwd		= false;
		nStep		= -cnMaxSmallOffset;
		nOffset		= -nOffset;
	}
	// seek第一次
	rst	= ::fseek(fp, nStep, nOrigin);
	if( rst<0 )
	{
		return	rst;
	}
	nOffset			-= cnMaxSmallOffset;
	// 后面的就都是基于上次结果的了
	while( nOffset>0 )
	{
		if( nOffset>cnMaxSmallOffset )
		{
			rst	= ::fseek(fp, nStep, SEEK_CUR);
		}
		else
		{
			if( bFwd )
			{
				rst	= ::fseek(fp, (long)nOffset, SEEK_CUR);
			}
			else
			{
				rst	= ::fseek(fp, (long)(-nOffset), SEEK_CUR);
			}
		}
		if( rst<0 )
		{
			return	rst;
		}
		nOffset		-= cnMaxSmallOffset;
	}
	return	rst;
}

bool	whfile_getfilepath(const char *__file, char *__path)
{
	// 如果是根目录则返回true和空path，在使用时自动加"/"就变成了根目录
	for(int i=strlen(__file)-1;i>=0;i--)
	{
		switch( __file[i] )
		{
			case	'/':
			case	'\\':
				if( __path )
				{
					memcpy(__path, __file, i);
					__path[i]	= 0;
				}
				return	true;
			break;
		}
	}
	if( __path )
	{
		__path[0]	= 0;
	}
	return	false;
}
const char *	whfile_getfilename(const char *__file, char *__fname)
{
	static char	szFName[WH_MAX_PATH];
	if( !__fname )
	{
		__fname	= szFName;
	}
	if( __file[0]==0 )
	{
		__fname[0]	= 0;
		return	NULL;
	}
	// 如果是根目录则返回true和空path，在使用时自动加"/"就变成了根目录
	for(int i=strlen(__file)-1;i>=0;i--)
	{
		switch( __file[i] )
		{
			case	'/':
			case	'\\':
				strcpy(__fname, __file+i+1);
				return	__fname;
			break;
		}
	}
	strcpy(__fname, __file);
	return	__fname;
}
bool	whfile_splitfilenameandpath(char *__fullfilepath, const char **__fpath, const char **__fname)
{
	// 无论如何path都要从头开始
	*__fpath		= __fullfilepath;
	if( __fullfilepath[0]==0 )
	{
		*__fname	= __fullfilepath;
		return		true;
	}
	// 如果是根目录则返回true和空path，在使用时自动加"/"就变成了根目录
	for(int i=strlen(__fullfilepath)-1;i>=0;i--)
	{
		switch( __fullfilepath[i] )
		{
			case	'/':
			case	'\\':
				__fullfilepath[i]	= 0;
				*__fname			= __fullfilepath + i + 1;
				return	true;
			break;
		}
	}
	// 里面没有区分出路径的地方，那就认为是当前路径吧
	static char	czsPath[]	= ".";
	*__fpath	= czsPath;
	*__fname	= __fullfilepath;
	return	true;
}
const char *	whfile_makerealpath(const char *__basepath, const char *__reltivepath, char *__realpath)
{
	// 判断是否是绝对路径
	if( whfile_isabspath(__reltivepath) || __basepath[0]==0 )
	{
		strcpy(__realpath, __reltivepath);
		return	__realpath;
	}
	if( __reltivepath[0] != '.' )
	{
		// 非向前的相对路径
		sprintf(__realpath, "%s/%s", __basepath, __reltivepath);
	}
	else
	{
		// 先把__basepath拷贝到__realpath中，同时作\到/的转换
		int	i=0;
		while( __basepath[i]!=0 )
		{
			if( __basepath[i] == '\\' )
			{
				__realpath[i]	= '/';
			}
			else
			{
				__realpath[i]	= __basepath[i];
			}
			i++;
		}
		if( i==0 )
		{
			strcpy(__realpath, __reltivepath);
			return	__realpath;
		}
		i--;
		// 前向的相对路径（应该是前面有多个“../”的）
		while( (*__reltivepath)!=0 )
		{
			if( __reltivepath[0]=='.' )
			{
				if( __reltivepath[1]=='/' || __reltivepath[1]=='\\' )
				{
					__reltivepath	+= 2;
				}
				else if( __reltivepath[1]=='.'
				&& ( __reltivepath[2]=='/' || __reltivepath[2]=='\\' )
				)
				{
					__reltivepath	+= 3;
					// 把base向前推进
					while( i>0 )
					{
						if( __realpath[i--]=='/' )
						{
							break;
						}
					}
				}
			}
			else
			{
				break;
			}
		}
		__realpath[i+1]	= '/';
		strcpy(__realpath+i+2, __reltivepath);
	}
	return	__realpath;
}

int		whfile_del(const char *__file)
{
	return	unlink(__file);
}
int		whfile_ren(const char *__oldfile, const char *__newfile)
{
	// 如果旧文件存在就先删除之
	if( whfile_ispathexisted(__newfile)
	&&  whfile_del(__newfile)<0 )
	{
		// 删除失败
		return	-1;
	}
	if( rename(__oldfile, __newfile)<0 )
	{
		return	-2;
	}
	return		0;
}
int		whfile_cpy(const char *__dstfile, const char *__srcfile, bool __cptime, bool __nofileok)
{
	if( strcmp(__dstfile, __srcfile)==0 )
	{
		// 自己拷贝到自己，则直接已经完成了。不过这样一般都是由于失误造成的。
		assert(0);
		return	WHFILE_CPY_RST_ONSELF;
	}

	// 判断如果原文件不存在,则目标文件也不存在
	if( __nofileok )
	{
		if( !whfile_ispathexisted(__srcfile) )
		{
			return	WHFILE_CPY_RST_OK;
		}
	}

	// 打开两个文件
	FILE	*fpsrc, *fpdst;
	fpsrc	= ::fopen(__srcfile, "rb");
	if( !fpsrc )
	{
		return	WHFILE_CPY_RST_SRC_O_ERR;
	}
	fpdst	= ::fopen(__dstfile, "wb");
	if( !fpdst )
	{
		::fclose(fpsrc);
		return	WHFILE_CPY_RST_DST_C_ERR;
	}

	// 拷贝内容
	int		rst = WHFILE_CPY_RST_OK;
	char	buf[4096];
	size_t	size;
	while( !feof(fpsrc) )
	{
		size	= fread(buf, 1, sizeof(buf), fpsrc);
		if( size==0 )
		{
			break;
		}
		if( ::fwrite(buf, 1, size, fpdst) != size )
		{
			rst	= WHFILE_CPY_RST_DSTERR;
			break;
		}
	}

	// 关闭文件
	::fclose(fpdst);
	::fclose(fpsrc);

	// 看是否要设置时间
	if( __cptime )
	{
		whfile_setmtime(__dstfile, whfile_getmtime(__srcfile));
	}

	return	rst;
}
int		whfile_cpy_calcmd5(WHFILE_CPY_T *pParam)
{
	if( strcmp(pParam->dstfile, pParam->srcfile)==0 )
	{
		// 自己拷贝到自己，则直接已经完成了。不过这样一般都是由于失误造成的。
		assert(0);
		return	WHFILE_CPY_RST_ONSELF;
	}

	// 判断如果原文件不存在,则目标文件也不存在
	if( pParam->nofileok )
	{
		if( !whfile_ispathexisted(pParam->srcfile) )
		{
			return	WHFILE_CPY_RST_OK;
		}
	}

	// 打开两个文件
	FILE	*fpsrc, *fpdst;
	fpsrc	= ::fopen(pParam->srcfile, "rb");
	if( !fpsrc )
	{
		return	WHFILE_CPY_RST_SRC_O_ERR;
	}
	fpdst	= ::fopen(pParam->dstfile, "wb");
	if( !fpdst )
	{
		::fclose(fpsrc);
		return	WHFILE_CPY_RST_DST_C_ERR;
	}

	md5_state_t	state;
	md5_init(&state);

	// 拷贝内容
	int		rst = WHFILE_CPY_RST_OK;
	char	buf[4096];
	size_t	size;
	size_t	nTotal	= 0;
	size_t	nCur	= 0;
	while( !feof(fpsrc) )
	{
		size	= ::fread(buf, 1, sizeof(buf), fpsrc);
		if( size==0 )
		{
			break;
		}
		if( ::fwrite(buf, 1, size, fpdst) != size )
		{
			rst	= WHFILE_CPY_RST_DSTERR;
			break;
		}
		nTotal	+= size;
		if( pParam->cbstep>0 )
		{
			nCur	+= size;
			if( nCur>=pParam->cbstep )
			{
				nCur	= 0;
				(*pParam->cb_progress)(nTotal, pParam->cbparam);
			}
		}
		md5_append(&state, (const md5_byte_t *)buf, size);
	}

	md5_finish(&state, pParam->MD5);

	// 关闭文件
	::fclose(fpdst);
	::fclose(fpsrc);

	// 看是否要设置时间
	if( pParam->cptime )
	{
		whfile_setmtime(pParam->dstfile, whfile_getmtime(pParam->srcfile));
	}

	return	rst;
}
struct	CB_WHFILE_CPY_CMP_T
{
	size_t	nTotalSize;
};
static void	cb_whfile_cpy_cmp(size_t copied, void *cbparam)
{
	CB_WHFILE_CPY_CMP_T	*pInfo	= (CB_WHFILE_CPY_CMP_T *)cbparam;
	// 计算并打印百分比
	double	percentage	= (double)copied / pInfo->nTotalSize * 100;
	printf("%%%05.2lf\b\b\b\b\b\b", percentage); fflush(stdout);
}
int		whfile_cpy_cmp(WHFILE_CPY_T *pParam)
{
	// 如果没有填回调就填写默认的
	CB_WHFILE_CPY_CMP_T	info;

	// 获得文件总长度
	whint64	nTotalSize	= whfile_getfilebigsize(pParam->srcfile);
	if( nTotalSize<0 )
	{
		return	WHFILE_CPY_RST_SRC_O_ERR;
	}
	info.nTotalSize		= (size_t)nTotalSize;

	if( pParam->cbstep>0 )
	{
		if( pParam->cb_progress == NULL )
		{
			pParam->cb_progress	= cb_whfile_cpy_cmp;
			pParam->cbparam		= &info;
		}
	}

	printf("Copying %s to %s ... total:%d ", pParam->srcfile, pParam->dstfile, info.nTotalSize);
	int	rst	= whfile_cpy_calcmd5(pParam);
	if( rst<0 )
	{
		return	rst;
	}
	printf("OVER.  %ssrcfile MD5:[%s]%s", WHLINEEND, wh_hex2str(pParam->MD5, sizeof(pParam->MD5)), WHLINEEND); fflush(stdout);
	// 计算目标文件的MD5
	unsigned char	MD5[16];
	printf("dstfile MD5:"); fflush(stdout);
	printf("[%s] ", wh_hex2str(whfilemd5(pParam->dstfile, MD5), sizeof(MD5)));
	// 校验md5
	if( memcmp(MD5, pParam->MD5, sizeof(MD5))==0 )
	{
		printf("OK ^__^%s", WHLINEEND);
		return	WHFILE_CPY_RST_OK;
	}
	else
	{
		printf("ERR!%s", WHLINEEND);
		return	WHFILE_CPY_RST_DSTERR;
	}
}
int		whfile_setmtime(const char *__file, time_t t)
{
	struct utimbuf	utb;
	memset(&utb, 0, sizeof(utb));
	utb.actime	= t;
	utb.modtime	= t;
	return	utime(__file, &utb);
}
time_t	whfile_getmtime(const char *__file)
{
	struct stat	st;
	int			rst;
	rst			= stat(__file, &st);
	if( rst!=0 )
	{
		return	0;
	}
	return	st.st_mtime;
}

int		whfile_readfile(const char *__file, whvector<char> &__vectbuf)
{
	FILE	*fp;
	fp		= ::fopen(__file, "rb");
	if( !fp )
	{
		return	-1;
	}
	// 得到文件长度
	::fseek(fp, 0, SEEK_END);
	__vectbuf.resize(ftell(fp));
	__vectbuf.f_rewind();
	// 然后从头开始读
	::fseek(fp, 0, SEEK_SET);
	::fread(__vectbuf.getbuf(), 1, __vectbuf.size(), fp);
	::fclose(fp);
	return	0;
}
int		whfile_readfile(const char *__file, void *__data, size_t *__size)
{
	FILE	*fp;
	fp		= ::fopen(__file, "rb");
	if( !fp )
	{
		return	-1;
	}
	*__size	= ::fread(__data, 1, *__size, fp);
	::fclose(fp);
	return	0;
}
int		whfile_cmpdata(const char *__file, const void *__data, int __size)
{
	whfile	*fp	= whfile_OpenCmnFile(__file, "rb");
	if( !fp )
	{
		return	-1;
	}
	int	rst		= whfile_cmpdata(fp, __data, __size);
	// 关闭文件
	delete	fp;
	return	rst;
}
int		whfile_cmpdata(whfile *__file, const void *__data, int __size)
{
	char	szBuf[4096];
	char	*pszData	= (char *)__data;

	// 如果长度不同则可以直接返回
	int		nRst		= __file->FileSize() - __size;
	if( nRst!=0 )
	{
		return	nRst;
	}

	while( !__file->IsEOF() && __size>0 )
	{
		int	nReadSize	= __file->Read(szBuf, sizeof(szBuf));
		if( nReadSize==0 )
		{
			break;
		}
		int	nRst		= memcmp(szBuf, pszData, nReadSize);
		if( nRst!=0 )
		{
			// 这样返回还可以反映一些内存差异的状况，呵呵，不过没有太大必要
			return	nRst;
		}
		// 还需要比较字节数
		__size			-= nReadSize;
		pszData			+= nReadSize;
	}
	if( __size!=0 )
	{
		return	-1;
	}
	return	0;
}
// whfile_cmp的返回值参考头文件中的定义
int		whfile_cmp(whfile *__file1, whfile *__file2)
{
	if( __file1->FileSize() == __file2->FileSize() )
	{
		char	buf1[1024], buf2[1024];
		while( !__file1->IsEOF() )
		{
			int	size1	= __file1->Read(buf1, sizeof(buf1));
			int	size2	= __file2->Read(buf2, sizeof(buf2));
			if( size1<0
			||  size2<0
			||  size1!=size2
			)
			{
				return	-1;
			}
			if( memcmp(buf1, buf2, size1) != 0 )
			{
				return	1;
			}
		}
	}
	else
	{
		return	2;
	}

	return	0;
}
// whfile_cmp的返回值参考头文件中的定义
int		whfile_cmp(const char *__file1, const char *__file2)
{
	int	rst	= -1;
	whfile	*whfile1	= whfile_OpenCmnFile(__file1, "rb");
	if( !whfile1 )
	{
		return	-1;
	}
	whfile	*whfile2	= whfile_OpenCmnFile(__file2, "rb");
	if( !whfile2 )
	{
		delete	whfile1;
		return	-1;
	}

	rst		= whfile_cmp(whfile1, whfile2);

	delete	whfile1;
	delete	whfile2;
	return	rst;
}
int		whfile_touch(const char *__file, time_t __filetime)
{
	if( __filetime == 0 )
	{
		__filetime	= wh_time();
	}
	if( !whfile_ispathexisted(__file) )
	{
		// 试图创建
		if( whfile_writefile(__file, NULL, 0)<0 )
		{
			return	-1;
		}
	}

	if( whfile_setmtime(__file, __filetime)<0 )
	{
		return	-2;
	}

	return	0;
}
int		whfile_writefile(const char *__file, const void *__data, size_t __size)
{
	FILE	*fp;
	fp		= ::fopen(__file, "wb");
	if( !fp )
	{
		return	-1;
	}
	int	rst	= 0;
	if( __size>0 )
	{
		rst	= ::fwrite(__data, 1, __size, fp);
	}
	::fclose(fp);
	return	rst;
}
int		whfile_safereplace(const char *__file, const void *__data, size_t __size, bool __readagain)
{
	// 判断文件是否已经存在
	if( !whfile_ispathexisted(__file) )
	{
		// 不存在则直接写入
		return	whfile_writefile(__file, __data, __size);
	}

	// 写入临时文件
	char	szTmpFile[WH_MAX_PATH];
	sprintf(szTmpFile, "%s.~whsafereplacetmp", __file);
	if( whfile_writefile(szTmpFile, __data, __size)<0 )
	{
		return	-20;
	}
	// 读出来，看看和原来的数据是否一致
	if( __readagain )
	{
		if( whfile_cmpdata(szTmpFile, __data, __size)!=0 )
		{
			// 删除临时文件
			whfile_del(szTmpFile);
			return	-21;
		}
	}
	return	whfile_ren(szTmpFile, __file);
}

int		whfile_createfile_fill(const char *__file, unsigned char __pad, size_t __size)
{
	whfile	*fp	= whfile_OpenCmnFile(__file, "wb");
	if( !fp )
	{
		return	-1;
	}
	if( whfile_file_fill(fp, __pad, __size)<0 )
	{
		return	-2;
	}
	delete	fp;
	return	0;
}
int		whfile_file_fill(whfile *__fp, unsigned char __pad, size_t __size)
{
	return	__fp->DupWrite(&__pad, sizeof(__pad), __size);
}
int		whfile_createfile(const char *__file, size_t __size)
{
	FILE	*fp;
	fp		= ::fopen(__file, "wb");
	if( !fp )
	{
		return	-1;
	}
	if( __size>0 )
	{
		::fseek(fp, __size-1, SEEK_SET);
		char	c=0;
		::fputs(&c, fp);
	}
	::fclose(fp);
	return	0;
}
bool	whfile_ExtIsMatch(const char *cszFileName, const char *cszFilter)
{
	// cszFilter是".h|.cpp"这样的格式
	whvector<EXTINFO_UNIT_T>	vect;
	vect.reserve(8);
	if( !whfile_MakeExtFilter(cszFilter, vect) )
	{
		return	false;
	}
	return	whfile_ExtIsMatch(cszFileName, vect);
}
bool	whfile_MakeExtFilter(const char *cszFilter, whvector<EXTINFO_UNIT_T> &vectFilter)
{
	// 空串代表一切都匹配
	if( cszFilter==NULL || cszFilter[0]==0 )
	{
		return	true;
	}
	EXTINFO_UNIT_T	*pUnit	= vectFilter.push_back();
	pUnit->cszBegin	= cszFilter;
	pUnit->len		= 0;
	char	c;
	while( (c=(*cszFilter++)) != 0 )
	{
		if( c=='|' )
		{
			if( (*cszFilter)==0 )
			{
				assert(0);
				return	false;
			}
			pUnit	= vectFilter.push_back();;
			pUnit->cszBegin	= cszFilter;
			pUnit->len		= 0;
		}
		else
		{
			pUnit->len		++;
		}
	}
	return	true;
}
bool	whfile_ExtIsMatch(const char *cszFileName, whvector<EXTINFO_UNIT_T> &vectFilter)
{
	if( vectFilter.size()==0 )
	{
		return	true;
	}
	int	len	= strlen(cszFileName);
	for(size_t i=0;i<vectFilter.size();i++)
	{
		if( memcmp(cszFileName+len-vectFilter[i].len, vectFilter[i].cszBegin, vectFilter[i].len)==0 )
		{
			return	true;
		}
	}
	return	false;
}

#pragma pack(1)
struct	CRC_HDR_T
{
	enum
	{
		VER		= 0,
	};
	int	nVer;
	int	nSize;
	int	nCRC;
	int	nJunk;	// 这个只是为了凑16字节用的，让编辑的时候容易对齐

	inline bool	IsVerOK() const
	{
		return	nVer == VER;
	}
	inline bool	IsSizeOK(size_t nFileSize) const
	{
		// 这样如果nSize为负数那么也会报错
		return	sizeof(*this) + nFileSize >= (size_t)nSize;
	}
};
#pragma pack()
int		whfile_crc_readfile(const char *__file, void *__data, size_t *__size)
{
	whfile	*fp	= whfile_OpenCmnFile(__file, "rb");
	if( !fp )
	{
		return	WHFILE_CRC_READ_NOTEXST;
	}
	int	rst	=	whfile_crc_readfile(fp, __data, __size);
	delete	fp;
	return	rst;
}
int		whfile_crc_readfile(whfile *__file, void *__data, size_t *__size)
{
	// 先读入数据长度和CRC校验位
	CRC_HDR_T	hdr;
	int		rst	= __file->Read(&hdr, sizeof(hdr));
	if( rst!=sizeof(hdr) )
	{
		if( rst==0 )
		{
			// 认为数据长度为0
			*__size	= 0;
			return	WHFILE_CRC_READ_OK;
		}
		return	WHFILE_CRC_READ_ERRFILE;
	}
	if( !hdr.IsVerOK() || !hdr.IsSizeOK(__file->FileSize()) )
	{
		return	WHFILE_CRC_READ_ERRHDR;
	}
	// 这是为了在文件后面多出数据的情况下不报错(多在停电的时候发生)
	if( *__size > (size_t)hdr.nSize )
	{
		*__size	= hdr.nSize;
	}
	// 在读入数据
	*__size	= __file->Read(__data, *__size);
	if( (*__size) != (size_t)hdr.nSize
	||  crc32((char*)__data, hdr.nSize) != (unsigned)hdr.nCRC
	)
	{
		return	WHFILE_CRC_READ_ERRCRC;
	}
	return		WHFILE_CRC_READ_OK;
}
int		whfile_crc_readfile(const char *__file, whvector<char> &vectData)
{
	whfile	*fp	= whfile_OpenCmnFile(__file, "rb");
	if( !fp )
	{
		return	WHFILE_CRC_READ_NOTEXST;
	}
	int	rst	=	whfile_crc_readfile(fp, vectData);
	delete	fp;
	return	rst;
}
int		whfile_crc_readfile(whfile *__file, whvector<char> &vectData)
{
	vectData.clear();
	// 先读入数据长度和CRC校验位
	CRC_HDR_T	hdr;
	int		rst	= __file->Read(&hdr, sizeof(hdr));
	if( rst!=sizeof(hdr) )
	{
		if( rst==0 )
		{
			// 认为数据长度为0
			return	WHFILE_CRC_READ_OK;
		}
		return	WHFILE_CRC_READ_ERRFILE;
	}
	if( !hdr.IsVerOK() || !hdr.IsSizeOK(__file->FileSize()) )
	{
		return	WHFILE_CRC_READ_ERRHDR;
	}

	// 文件长度
	vectData.resize(hdr.nSize);

	// 2005-02-06:应该没有必要seek一次了:fseek(fp, sizeof(hdr), SEEK_SET);
	// 读入数据
	size_t nSize = __file->Read(vectData.getbuf(), vectData.size());
	if( nSize != (size_t)hdr.nSize
	||  crc32(vectData.getbuf(), hdr.nSize) != (unsigned)hdr.nCRC
	)
	{
		return	WHFILE_CRC_READ_ERRCRC;
	}
	return		WHFILE_CRC_READ_OK;
}
int		whfile_crc_writefile(const char *__file, const void *__data, size_t __size, bool __writetmp)
{
	char	szTmpFile[WH_MAX_PATH]="";
	const char	*__oldfile	= NULL;
	if( __writetmp )
	{
		sprintf(szTmpFile, "%s.~whcrcfiletmp", __file);
		__oldfile	= __file;
		__file		= szTmpFile;
	}
	FILE	*fp;
	fp		= ::fopen(__file, "wb");
	if( !fp )
	{
		return	-1;
	}
	// 先写入数据长度和CRC校验位
	CRC_HDR_T	hdr;
	hdr.nVer	= CRC_HDR_T::VER;
	hdr.nSize	= __size;
	hdr.nCRC	= crc32((char*)__data, __size);
	hdr.nJunk	= 0x2e2e2e2e;
	if( ::fwrite(&hdr, 1, sizeof(hdr), fp)!=sizeof(hdr) )
	{
		goto	ErrEnd;
	}
	if( ::fwrite(__data, 1, __size, fp)!=(unsigned)__size )
	{
		goto	ErrEnd;
	}
	::fclose(fp);
	fp	= NULL;
	if( szTmpFile[0] )
	{
		// 将临时文件移动为目标文件(内部会先删除旧文件，然后rename)
		if( whfile_ren(szTmpFile, __oldfile)<0 )
		{
			goto	ErrEnd;
		}
	}
	return	0;
ErrEnd:
	if( fp )
	{
		::fclose(fp);
	}
	if( szTmpFile[0] )
	{
		// 删除临时文件
		whfile_del(szTmpFile);
	}
	return	-1;
}

int		whfile_crc_readfromvector(whvector<char> &vectFile, void *__data, size_t *__size)
{
	// 先把指针放到文件头
	vectFile.f_rewind();
	// 先读入数据长度和CRC校验位
	CRC_HDR_T	hdr;
	int		rst	= vectFile.f_read(&hdr);
	if( rst!=sizeof(hdr) )
	{
		if( rst==0 )
		{
			// 认为数据长度为0
			*__size	= 0;
			return	WHFILE_CRC_READ_OK;
		}
		return	WHFILE_CRC_READ_ERRFILE;
	}
	if( !hdr.IsVerOK() || !hdr.IsSizeOK(vectFile.size()) )
	{
		return	WHFILE_CRC_READ_ERRHDR;
	}
	// 这是为了在文件后面多出数据的情况下不报错(多在停电的时候发生)
	if( *__size > (size_t)hdr.nSize )
	{
		*__size	= hdr.nSize;
	}
	// 读入数据部分
	*__size	= vectFile.f_read(__data, *__size);
	if( (*__size) != (size_t)hdr.nSize
	||  crc32((char*)__data, hdr.nSize) != (unsigned)hdr.nCRC
	)
	{
		return	WHFILE_CRC_READ_ERRCRC;
	}
	return		WHFILE_CRC_READ_OK;
}
int		whfile_crc_readfromvector(whvector<char> &vectFile, whvector<char> &vectData)
{
	// 先把指针放到文件头
	vectFile.f_rewind();
	// 清空数据
	vectData.clear();
	// 先读入数据长度和CRC校验位
	CRC_HDR_T	hdr;
	int		rst	= vectFile.f_read(&hdr);
	if( rst!=sizeof(hdr) )
	{
		if( rst==0 )
		{
			// 认为数据长度为0
			return	WHFILE_CRC_READ_OK;
		}
		return	WHFILE_CRC_READ_ERRFILE;
	}
	if( !hdr.IsVerOK() || !hdr.IsSizeOK(vectFile.size()) )
	{
		return	WHFILE_CRC_READ_ERRHDR;
	}
	// 文件长度
	vectData.resize(hdr.nSize);
	// 读入数据
	size_t nSize = vectFile.f_read(vectData.getbuf(), vectData.size());
	if( nSize != (size_t)hdr.nSize
	||  crc32(vectData.getbuf(), hdr.nSize) != (unsigned)hdr.nCRC
	)
	{
		return	WHFILE_CRC_READ_ERRCRC;
	}
	return		WHFILE_CRC_READ_OK;
}
int		whfile_crc_writetovector(whvector<char> &vectFile, const void *__data, size_t __size)
{
	// 先写入数据长度和CRC校验位
	CRC_HDR_T	hdr;
	hdr.nVer	= CRC_HDR_T::VER;
	hdr.nSize	= __size;
	hdr.nCRC	= crc32((char*)__data, __size);
	hdr.nJunk	= 0x2e2e2e2e;

	// 这个写过程应该时不会出错的
	vectFile.clear();
	vectFile.f_write(&hdr);
	vectFile.f_write(__data, __size);

	return	0;
}

unsigned char *	whfilemd5(const char *szFile, unsigned char *szDst)
{
	// 分块读入文件，md5叠加
	whsafeptr<whfile>	fp(whfile_OpenCmnFile(szFile, "rb"));
	if( !fp )
	{
		return	NULL;
	}
	return	whfilemd5(fp, szDst);
}
unsigned char *	whfilemd5(whfile *fp, unsigned char *szDst)
{
	md5_state_t	state;

	md5_init(&state);

	char	szBuf[0x10000];
	int		nSize;
	while( !fp->IsEOF() )
	{
		nSize	= fp->Read(szBuf, sizeof(szBuf));
		if( nSize<0 )
		{
			return	NULL;
		}
		else
		{
			md5_append(&state, (const md5_byte_t *)szBuf, nSize);
		}
	}
	md5_finish(&state, szDst);
	return	szDst;
}
char *	whfilemd5str(const char *szFile, char *szDst)
{
	md5_byte_t	digest[16];
	memset(digest, 0, sizeof(digest));
	whfilemd5(szFile, digest);

	for(size_t i=0;i<16;i++)
	{
		sprintf(szDst + i*2, "%02x", digest[i]);
	}

	return		szDst;
}
char *	whfilemd5str(const char *szFile)
{
	static char	szDst[64];	// 其实33个就够了
	return	whfilemd5str(szFile, szDst);
}

////////////////////////////////////////////////////////////////////
// whfile
////////////////////////////////////////////////////////////////////
int	whfile::DupWrite(const void *pBuf, int nSize, int nDupNum)
{
	int	nTotal	= 0;
	for(int i=0;i<nDupNum;i++)
	{
		int	nThisWriteSize	= Write(pBuf, nSize);
		if( nThisWriteSize!=nSize )
		{
			return	-1;
		}
		nTotal	+= nThisWriteSize;
	}
	return	nTotal;
}
int	whfile::GetMD5(unsigned char *MD5)
{
	// 回到文件头
	Seek(0, SEEK_SET);

	md5_state_t	md5state;
	md5_init(&md5state);

	// 一段一段读取，计算md5
	char	buf[4096];
	int		size;
	while( !IsEOF() )
	{
		size	= Read(buf, sizeof(buf));
		if( size<=0 )
		{
			break;
		}
		md5_append(&md5state, (const md5_byte_t *)buf, size);
	}

	md5_finish(&md5state, MD5);

	return	0;
}
int	whfile::ReadLine(char *pBuf, int nSize)
{
	// 避免超过缓冲时最后一个0写不上
	nSize	--;
	// 一个字节一个字节地读入，直到读到结束"\n"或者文件结尾
	char	c, *pszBuf = pBuf;
	int		i = 0;
	while( nSize>i && Read(&c, 1)==1 )
	{
		pszBuf[i++]	= c;
		if( c=='\n' )
		{
			break;
		}
	}
	pszBuf[i]	= 0;
	return	i;
}
int	whfile::ReadVSize()
{
	// 读出vn2数据
	unsigned char	cBuf	= 0;
	Read(&cBuf);
	unsigned char	nIdx	= (cBuf & 0x3);
	// 拷贝有用的部分
	unsigned int	nSize	= 0;
	if( nIdx>0 )
	{
		Read(&nSize, nIdx);
		nSize		<<= 8;
		nSize		|= cBuf;
	}
	else
	{
		nSize		= cBuf;
	}
	// 移位恢复
	nSize			>>= 2;
	return	nSize;
}
int	whfile::WriteVSize(int nSize)
{
	char	bufsize[4];
	int		rst	= whbit_vn2_set(bufsize, nSize);
	if( rst<=0 )
	{
		return	rst;
	}
	// 先写入长度
	return	Write(bufsize, rst);
}
int	whfile::ReadVData(void *pData, int nMaxSize)
{
	// 读出尺寸
	int	nSize	= ReadVSize();
	if( nSize==0 )
	{
		// 后面没有数据了
		return	0;
	}
	if( nSize<0 )
	{
		return	-1;
	}
	if( nSize>nMaxSize )
	{
		// 上层就不应该继续读了
		return	-2;
	}
	return	Read(pData, nSize);
}
int	whfile::WriteVData(const void *pData, int nSize)
{
	// 写入尺寸
	if( WriteVSize(nSize)<=0 )
	{
		return	-1;
	}
	// 再写入数据
	return	Write(pData, nSize);
}
int	whfile::ReadVStr(char *szStr, int nMaxSize)
{
	int	rst	= ReadVData(szStr, nMaxSize-1);
	if( rst<0 )
	{
		return	rst;
	}
	// 最后补0结尾
	szStr[rst]	= 0;
	return	rst+1;
}
int	whfile::WriteVStr(const char *szStr, int nMaxSize)
{
	int	nLen	= strlen(szStr);
	if( nMaxSize>0 && nMaxSize<nLen )
	{
		return	-1;
	}
	return	WriteVData(szStr, nLen);
}

////////////////////////////////////////////////////////////////////
// whfile_i_std
////////////////////////////////////////////////////////////////////
whfile_i_std::whfile_i_std(FILE *fp, const char *cszFileName)
: m_fp(fp)
, m_nNewFileTime(0)
#ifdef	WIN32
, m_bJustRread(false)
, m_bJustWrite(false)
#endif
{
	strcpy(m_szFileName, cszFileName);
	::fseek(fp, 0, SEEK_END);
	m_nSize	= ftell(fp);
	::fseek(fp, 0, SEEK_SET);
}
whfile_i_std::~whfile_i_std()
{
	if( m_fp )
	{
		::fclose(m_fp);
		if( m_nNewFileTime!=0 )
		{
			// 修改文件时间
			whfile_setmtime(m_szFileName, m_nNewFileTime);
		}
	}
}
int	whfile_i_std::FileSize() const
{
	return	m_nSize;
}
time_t	whfile_i_std::FileTime() const
{
	struct stat	st;
	if( fstat(fileno(m_fp), &st)<0 )
	{
		return	0;
	}
	return	st.st_mtime;
}
void	whfile_i_std::SetFileTime(time_t t)
{
	// 因为futime不能在linux下面用，所以就用这个笨办法了
	if( t==0 )
	{
		t	= wh_time();
	}
	m_nNewFileTime	= t;
}
int	whfile_i_std::Read(void *pBuf, int nSize)
{
#ifdef	WIN32
	if (m_bJustWrite)
	{
		m_bJustWrite = false;
		Seek(0, SEEK_CUR);
	}
	m_bJustRread	= true;
#endif
	return	::fread(pBuf, 1, nSize, m_fp);
}
int	whfile_i_std::Seek(int nOffset, int nOrigin)
{
	return	::fseek(m_fp, nOffset, nOrigin);
}
bool	whfile_i_std::IsEOF()
{
	// 不用::是因为feof是个宏，郁闷ing:(
	return	feof(m_fp) != 0;
}
int		whfile_i_std::Tell()
{
	return	::ftell(m_fp);
}
int		whfile_i_std::Write(const void *pBuf, int nSize)
{
	// 在win32下如果刚刚进行了fread马上就进行fwrite则可能会导致fwrite虽然返回成功但是写入却没有效果
	// 这样fseek一下然后再写就好了
	// 参见http://msdn2.microsoft.com/en-us/library/kb1at4by.aspx
#ifdef	WIN32
	if( m_bJustRread )
	{
		m_bJustRread	= false;
		Seek(0, SEEK_CUR);
		//Seek(1, SEEK_CUR);
		//Seek(-1, SEEK_CUR);
	}
	m_bJustWrite	= true;
#endif
	nSize	= ::fwrite(pBuf, 1, nSize, m_fp);
	int	nCurPos	= Tell();
	if( nCurPos > m_nSize )
	{
		m_nSize	= nCurPos;
	}
	return	nSize;
}
int		whfile_i_std::Flush()
{
	return	::fflush(m_fp);
}

////////////////////////////////////////////////////////////////////
// whfile_i_mem 内存文件的实现
////////////////////////////////////////////////////////////////////
whfile_i_mem::whfile_i_mem(char *pszBuf, int nSize)
: m_pszStr(pszBuf), m_nSize(nSize), m_nOffset(0)
{
}
int		whfile_i_mem::FileSize() const
{
	return	m_nSize;
}
time_t	whfile_i_mem::FileTime() const
{
	return	0;
}
void	whfile_i_mem::SetFileTime(time_t t)
{
}
int		whfile_i_mem::Read(void *pBuf, int nSize)
{
	int	nDiff = GetSizeLeft();
	if( nSize>nDiff )
	{
		nSize	= nDiff;
	}
	if( nSize>0 )
	{
		memcpy(pBuf, m_pszStr+m_nOffset, nSize);
		m_nOffset	+= nSize;
	}
	return	nSize;
}
int		whfile_i_mem::Seek(int nOffset, int nOrigin)
{
	switch( nOrigin )
	{
		case	SEEK_SET:
			m_nOffset	= nOffset;
		break;
		case	SEEK_CUR:
			m_nOffset	+= nOffset;
		break;
		case	SEEK_END:
			m_nOffset	= m_nSize + nOffset;
		break;
		default:
			assert(0);
			return	-1;
		break;
	}

	if( m_nOffset<0 )
	{
		m_nOffset		= 0;
	}
	else if( m_nOffset>m_nSize )
	{
		m_nOffset		= m_nSize;
	}

	return	0;
}
bool	whfile_i_mem::IsEOF()
{
	assert(m_nOffset<=m_nSize);
	return	m_nOffset>=m_nSize;
}
int		whfile_i_mem::Tell()
{
	return	m_nOffset;
}
int		whfile_i_mem::Write(const void *pBuf, int nSize)
{
	int		nMaxSize	= GetSizeLeft();
	if( nSize>nMaxSize )
	{
		nSize			= nMaxSize;
	}
	assert(nSize>=0);
	memcpy(m_pszStr+m_nOffset, pBuf, nSize);
	m_nOffset			+= nSize;
	return	nSize;
}

////////////////////////////////////////////////////////////////////
// 文件内的文件的实现
// 因为whfile目前不支持多线程访问，所以这个也不支持多线程访问
////////////////////////////////////////////////////////////////////
whfile_i_fileinfile::whfile_i_fileinfile()
: m_file(NULL)
, m_nStartOffset(0)
, m_nSize(0)
, m_nOffset(0)
{
}
int		whfile_i_fileinfile::AssociateFile(whfile *file, int nStartOffset, int nSize)
{
	// 确定不能超界
	if( nStartOffset+nSize > file->FileSize() )
	{
		assert(0);
		return	-1;
	}

	m_file			= file;
	m_nStartOffset	= nStartOffset;
	m_nSize			= nSize;
	m_nOffset		= 0;

	return	0;
}
int		whfile_i_fileinfile::FileSize() const
{
	return	m_nSize;
}
time_t	whfile_i_fileinfile::FileTime() const
{
	return	m_file->FileTime();
}
void	whfile_i_fileinfile::SetFileTime(time_t t)
{
	m_file->SetFileTime(t);
}
int		whfile_i_fileinfile::Read(void *pBuf, int nSize)
{
	if( IsEOF() )
	{
		// 没有东西可读了
		return	0;
	}
	if( m_nOffset+nSize>=m_nSize )
	{
		nSize	= m_nSize-m_nOffset;
	}
	int	nRst	= m_file->Read(pBuf, nSize);
	if( nRst!=nSize )
	{
		// 前面限制过这里就不应该出错的
		assert(0);
		return	-1;
	}
	m_nOffset	+= nSize;
	return		nRst;
}
int		whfile_i_fileinfile::Seek(int nOffset, int nOrigin)
{
	switch( nOrigin )
	{
		case	SEEK_SET:
			m_nOffset	= nOffset;
		break;
		case	SEEK_CUR:
			m_nOffset	+= nOffset;
		break;
		case	SEEK_END:
			m_nOffset	= m_nSize + nOffset;
		break;
		default:
			assert(0);
			return	-1;
		break;
	}

	if( m_nOffset<0 )
	{
		m_nOffset		= 0;
	}
	else if( m_nOffset>m_nSize )
	{
		m_nOffset		= m_nSize;
	}

	m_file->Seek(m_nStartOffset+m_nOffset, SEEK_SET);

	return	0;
}
bool	whfile_i_fileinfile::IsEOF()
{
	assert(m_nOffset <= m_nSize);
	return	m_nOffset >= m_nSize;
}
int		whfile_i_fileinfile::Tell()
{
	assert(m_nOffset == m_file->Tell()-m_nStartOffset);
	return	m_nOffset;
}

// 用于读数据的对象
class	fileinfile	: public whfile_i_fileinfile
{
public:
	whmultifile::IFILEHDR_T	m_ifhdr;
public:
	virtual time_t	FileTime()
	{
		return	m_ifhdr.cmn.nTime;
	}
	virtual void	SetFileTime(time_t t)
	{
		if( t==0 )
		{
			t	= wh_time();
		}
		m_ifhdr.cmn.nTime	= t;
	}
	virtual int		GetMD5(unsigned char *MD5)
	{
		memcpy(MD5, m_ifhdr.cmn.MD5, sizeof(m_ifhdr.cmn.MD5));
		return	0;
	}
};
// 用于写数据的对象
class	fileappend	: public whfile
{
private:
	whmultifile		*m_multifile;
	whfile			*m_basefile;
	md5_state_t		m_md5state;
	whmultifile::IFILEHDR_T	m_ifhdr;
public:
	fileappend(whmultifile *multifile, const char *cszFileName, time_t nFileTime);
	virtual			 ~fileappend();
	void	SelfDestroy()
	{
		delete	this;
	}
	virtual int		Write(const void *pBuf, int nSize);
	virtual int		Flush()
	{
		// 补进行任何操作
		return	0;
	}
	virtual int		FileSize() const
	{
		// 不可调用
		assert(0);
		return	0;
	}
	virtual time_t	FileTime() const
	{
		// 不可调用
		assert(0);
		return	0;
	}
	virtual void	SetFileTime(time_t t)
	{
		// 不可调用
		assert(0);
	}
	virtual int		Read(void *pBuf, int nSize)
	{
		// 不可调用
		assert(0);
		return	0;
	}
	virtual int		Seek(int nOffset, int nOrigin)
	{
		// 不可调用
		assert(0);
		return	0;
	}
	virtual bool	IsEOF()
	{
		// 不可调用
		assert(0);
		return	0;
	}
	virtual int		Tell()
	{
		// 不可调用
		assert(0);
		return	0;
	}
};

fileappend::fileappend(whmultifile *multifile, const char *cszFileName, time_t nFileTime)
: m_multifile(multifile), m_basefile(multifile->m_basefile)
{
	if( strlen(cszFileName)>sizeof(m_ifhdr.cmn.szName)-1 )
	{
		// 名字太长
		assert(0);
		throw "Name too big!";
	}

	// seek到base文件尾
	m_basefile->Seek(m_multifile->m_hdr.cmn.nEndOffset, SEEK_SET);
	// 写入文件头(占位置)
	strcpy(m_ifhdr.cmn.szName, cszFileName);
	m_ifhdr.cmn.nSize	= 0;
	m_ifhdr.cmn.nTime	= nFileTime;
	m_basefile->Write(&m_ifhdr);
	// 准备md5计算
	md5_init(&m_md5state);
}
fileappend::~fileappend()
{
	md5_finish(&m_md5state, m_ifhdr.cmn.MD5);
	// 记录当前文件指针偏移
	int	nEndOffset	= m_basefile->Tell();
	// 重写该文件信息
	m_basefile->Seek(m_multifile->m_hdr.cmn.nEndOffset, SEEK_SET);
	m_basefile->Write(&m_ifhdr);
	// 更改总文件头
	m_multifile->m_hdr.cmn.nFileNum		++;
	m_multifile->m_hdr.cmn.nEndOffset	= nEndOffset;
}
int		fileappend::Write(const void *pBuf, int nSize)
{
	nSize	= m_basefile->Write(pBuf, nSize);
	if( nSize<=0 )
	{
		return	-1;
	}
	md5_append(&m_md5state, (const md5_byte_t *)pBuf, nSize);
	m_ifhdr.cmn.nSize	+= nSize;
	return	nSize;
}

////////////////////////////////////////////////////////////////////
// whmultifile
////////////////////////////////////////////////////////////////////
whmultifile::whmultifile()
: m_nOpenMode(OPENMODE_NOTOPEN)
, m_basefile(NULL)
, m_bAutoCloseBaseFile(true)
{
}
whmultifile::~whmultifile()
{
	CloseFile();
}
int		whmultifile::OpenFile(const char *cszMainFile, int nOpenMode)
{
	// 以普通文件方式打开
	char	szMode[16] = "rb";
	switch( nOpenMode )
	{
		case	OPENMODE_CREATE:
			strcpy(szMode, "wb");
		break;
		case	OPENMODE_RDONLY:
			strcpy(szMode, "rb");
		break;
		case	OPENMODE_RDWR:
			strcpy(szMode, "rb+");
		break;
		default:
			assert(0);
			return	-1;
		break;
	}
	whfile	*file	= whfile_OpenCmnFile(cszMainFile, szMode);
	if( !file )
	{
		return	-1;
	}
	return	OpenFile(file, nOpenMode, true);
}
int		whmultifile::OpenFile(whfile *file, int nOpenMode, bool bAutoCloseBaseFile)
{
	m_bAutoCloseBaseFile	= bAutoCloseBaseFile;
	m_basefile				= file;
	m_nOpenMode				= nOpenMode;

	if( m_nOpenMode == OPENMODE_CREATE )
	{
		// 如果是创建，则随便填写头
		file->Rewind();
		m_hdr.Reset();
		m_basefile->Write(&m_hdr);
	}
	else
	{
		// 如果是打开已存在的文件，则读入头
		if( file->Read(&m_hdr) != sizeof(m_hdr)
		||  !m_hdr.IsGood()
		)
		{
			// 文件头不对
			return	-1;
		}
	}

	return	0;
}
int		whmultifile::CloseFile()
{
	if( m_basefile )
	{
		if( IsModifyOpen() )
		{
			// 重写总头
			m_basefile->Rewind();
			m_basefile->Write(&m_hdr);
		}
		if( m_bAutoCloseBaseFile )
		{
			delete	m_basefile;
		}
		m_basefile	= NULL;
	}
	return	0;
}
int		whmultifile::Append(const char *cszFile, whfile *file)
{
	IFILEHDR_T	ifhdr;
	if( strlen(cszFile)>sizeof(ifhdr.cmn.szName)-1 )
	{
		// 名字太长
		return	-1;
	}

	// seek到文件尾
	m_basefile->Seek(m_hdr.cmn.nEndOffset, SEEK_SET);
	// 写入文件头(占位置)
	strcpy(ifhdr.cmn.szName, cszFile);
	ifhdr.cmn.nSize	= 0;
	ifhdr.cmn.nTime	= file->FileTime();
	m_basefile->Write(&ifhdr);
	// 写入文件内容(同时计算md5)
	char	buf[4096];
	int		nSize;
	md5_state_t	md5state;
	md5_init(&md5state);
	while( !file->IsEOF() )
	{
		nSize	= file->Read(buf, sizeof(buf));
		if( nSize<=0 )
		{
			break;
		}
		nSize	= m_basefile->Write(buf, nSize);
		if( nSize<=0 )
		{
			break;
		}
		md5_append(&md5state, (const md5_byte_t *)buf, nSize);
		ifhdr.cmn.nSize	+= nSize;
	}
	md5_finish(&md5state, ifhdr.cmn.MD5);
	// 记录当前文件指针偏移
	int	nEndOffset	= m_basefile->Tell();
	// 重写该文件信息
	m_basefile->Seek(m_hdr.cmn.nEndOffset, SEEK_SET);
	m_basefile->Write(&ifhdr);
	// 更改总文件头
	m_hdr.cmn.nFileNum		++;
	m_hdr.cmn.nEndOffset	= nEndOffset;
	return	0;
}
whfile *	whmultifile::GetFileToAppend(const char *cszFile, time_t nFileTime)
{
	fileappend	*file	= NULL;
	try
	{
		file	= new fileappend(this, cszFile, nFileTime);
	}
	catch( const char * )
	{
		assert(0);
		return	NULL;
	}
	return		file;
}
whfile *	whmultifile::GetFileToRead(const char *cszFile)
{
	// 跳过文件头
	m_basefile->Seek(sizeof(m_hdr), SEEK_SET);
	// 找所有的文件，看名字是否匹配
	IFILEHDR_T	ifhdr;
	for(int i=0;i<m_hdr.cmn.nFileNum && !m_basefile->IsEOF();i++)
	{
		// 读入头
		if( m_basefile->Read(&ifhdr) != sizeof(ifhdr) )
		{
			return	NULL;
		}
		if( strcmp(ifhdr.cmn.szName, cszFile)==0 )
		{
			fileinfile	*pfif	= new fileinfile;
			if( pfif->AssociateFile(m_basefile, m_basefile->Tell(), ifhdr.cmn.nSize)==0 )
			{
				return	pfif;
			}
			delete	pfif;
			return	NULL;
		}
		// 跳过这个文件的内容
		m_basefile->Seek(ifhdr.cmn.nSize, SEEK_CUR);
	}
	return	NULL;
}
int			whmultifile::GetFileHdrList(whvector<IFILEHDR_T> &vect)
{
	// 清空
	vect.clear();
	vect.reserve(m_hdr.cmn.nFileNum);
	// 跳过文件头
	m_basefile->Seek(sizeof(m_hdr), SEEK_SET);
	// 找所有的文件
	IFILEHDR_T	ifhdr;
	for(int i=0;i<m_hdr.cmn.nFileNum && !m_basefile->IsEOF();i++)
	{
		// 读入头
		if( m_basefile->Read(&ifhdr) != sizeof(ifhdr) )
		{
			return	-1;
		}
		vect.push_back(ifhdr);
		// 跳过这个文件的内容
		m_basefile->Seek(ifhdr.cmn.nSize, SEEK_CUR);
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// 附着在别的文件尾部的文件
////////////////////////////////////////////////////////////////////
whtailfile::whtailfile()
: m_nOPMode(whfileman::OP_MODE_UNDEFINED), m_bModified(false)
, m_bAutoCloseHostFile(false), m_hostfile(NULL)
{
}
whtailfile::~whtailfile()
{
	Close();
}
bool	whtailfile::IsFileOK(whfile *fp)
{
	TAIL_T	tail;
	// 读出结尾
	if( fp->FileSize()>=(int)sizeof(tail) )
	{
		// 尺寸合适了。看是否结尾good
		fp->Seek(-(int)sizeof(tail), SEEK_END);
		fp->Read(&tail);
		if( tail.IsGood() )
		{
			return	true;
		}
	}
	return	false;
}
int		whtailfile::Open(whfileman *pFM, const char *cszHostFile, whfileman::OP_MODE_T nOPMode)
{
	whfileman::OP_MODE_T	nHostOPMode	= whfileman::OP_MODE_BIN_READONLY;
	if( nOPMode != whfileman::OP_MODE_BIN_READONLY )
	{
		nHostOPMode			= whfileman::OP_MODE_BIN_READWRITE;
	}
	return	Open(pFM->Open(cszHostFile, nHostOPMode), true, nOPMode);
}
int		whtailfile::Open(whfile *pHostFile, bool bAutoCloseHostFile, whfileman::OP_MODE_T nOPMode)
{
	if( m_hostfile )
	{
		// 原来没有关闭
		assert(0);
		return	-1;
	}
	m_nOPMode				= nOPMode;
	m_bModified				= false;
	m_hostfile				= pHostFile;
	m_bAutoCloseHostFile	= bAutoCloseHostFile;
	if( !m_hostfile )
	{
		return	-2;
	}
	// 判断原来了的文件是否已经有了结尾
	bool	bHasTail	= false;
	if( m_hostfile->FileSize()>=(int)sizeof(m_tail) )
	{
		// 尺寸合适了。看是否结尾good
		m_hostfile->Seek(-(int)sizeof(m_tail), SEEK_END);
		m_hostfile->Read(&m_tail);
		if( m_tail.IsGood() )
		{
			// 真的正确的结尾
			bHasTail	= true;
			// 如果打开方式是创建，则需要把原来的文件长度清空
			if( nOPMode == whfileman::OP_MODE_BIN_CREATE )
			{
				m_tail.cmn.nTailSize	= 0;
			}
		}
	}
	if( !bHasTail )
	{
		if( nOPMode==whfileman::OP_MODE_BIN_READONLY )
		{
			// 没有尾巴且又是只读的，说明文件格式不对！
			return	-10;
		}
		// 主动填充m_tail结构
		m_tail.Reset();
		m_tail.cmn.nHostSize	= m_hostfile->FileSize();
		m_tail.cmn.nTailSize	= 0;
	}
	else
	{
		// 如果有尾部则先移动文件指针到开头
		Seek(0, SEEK_SET);
	}
	return	0;
}
int		whtailfile::Close()
{
	if( m_hostfile )
	{
		if( (m_nOPMode!=whfileman::OP_MODE_BIN_READONLY) && m_bModified )
		{
			m_bModified	= false;
			// 重写尾部
			m_hostfile->Seek(m_tail.cmn.nHostSize+m_tail.cmn.nTailSize, SEEK_SET);
			m_hostfile->Write(&m_tail);
			// 为了避免在创建文件时文件旧长度过大的情况，把文件尾重新在最后写一遍
			if( m_nOPMode == whfileman::OP_MODE_BIN_CREATE )
			{
				int	nOffset	= m_hostfile->Tell();
				m_hostfile->Seek(0, SEEK_END);
				if( nOffset!=m_hostfile->Tell() )
				{
					m_hostfile->Seek(-(int)(sizeof(m_tail)), SEEK_END);
					m_hostfile->Write(&m_tail);
				}
			}
		}
		// 关闭文件
		if( m_bAutoCloseHostFile )
		{
			WHSafeSelfDestroy(m_hostfile);
		}
	}
	return	0;
}
int		whtailfile::FileSize() const
{
	return	m_tail.cmn.nTailSize;
}
time_t	whtailfile::FileTime() const
{
	return	m_tail.cmn.nFTime;
}
void	whtailfile::SetFileTime(time_t t)
{
	m_tail.cmn.nFTime	= t;
}
int		whtailfile::Read(void *pBuf, int nSize)
{
	if( IsEOF() )
	{
		return	0;
	}
	// 判断不能超过文件结尾
	int	nCurOffset	= GetCurOffset();
	if( nCurOffset + nSize > m_tail.cmn.nTailSize )
	{
		nSize	= m_tail.cmn.nTailSize - nCurOffset;
	}
	return	m_hostfile->Read(pBuf, nSize);
}
int		whtailfile::Seek(int nOffset, int nOrigin)
{
	int	nCurOffset	= GetCurOffset();
	switch( nOrigin )
	{
		case	SEEK_SET:
			nCurOffset	= nOffset;
		break;
		case	SEEK_CUR:
			nCurOffset	+= nOffset;
		break;
		case	SEEK_END:
			nCurOffset	= m_tail.cmn.nTailSize + nOffset;
		break;
		default:
			assert(0);
			return	-1;
		break;
	}

	if( nCurOffset<0 )
	{
		nCurOffset		= 0;
	}
	// 不用处理尾部超界的情况（因为需要利用seek扩大文件）

	return	m_hostfile->Seek(m_tail.cmn.nHostSize + nCurOffset, SEEK_SET);
}
bool	whtailfile::IsEOF()
{
	return	GetCurOffset() == m_tail.cmn.nTailSize;
}
int		whtailfile::Tell()
{
	return	GetCurOffset();
}
int		whtailfile::Write(const void *pBuf, int nSize)
{
	m_bModified	= true;
	// 写入
	nSize		= m_hostfile->Write(pBuf, nSize);
	if( nSize>0 )
	{
		int	nCurOffset	= GetCurOffset();
		if( nCurOffset>m_tail.cmn.nTailSize )
		{
			m_tail.cmn.nTailSize	= nCurOffset;
		}
	}
	return		nSize;
}
int		whtailfile::Flush()
{
	return	m_hostfile->Flush();
}

////////////////////////////////////////////////////////////////////
// whlistinfile
////////////////////////////////////////////////////////////////////
const char *	whlistinfile::HDR_T::cszMagic	= "WHLIF";
whlistinfile::whlistinfile()
: m_bModified(false)
, m_pfile(NULL)
{
}
whlistinfile::~whlistinfile()
{
	Close();
}
int		whlistinfile::Create(whfile *file, INFO_T *pInfo)
{
	assert(!m_pfile);

	m_bModified			= false;

	m_hdr.clear();
	m_hdr.nListTypeNum	= pInfo->nMsgTypeNum;
	m_hdr.nUnitSize		= UNIT_T::GetTotalSize(pInfo->nUnitDataSize);
	m_hdr.nUnitDataSize	= pInfo->nUnitDataSize;

	// 计算总共需要的尺寸，初始化list和内存
	size_t	nTotal	= whalist::CalcTotalSize(m_hdr.nUnitSize, pInfo->nMaxMsgNum);
	m_vectFileData.resize(nTotal);
	if( m_list.Init(m_vectFileData.getbuf(), m_hdr.nUnitSize, pInfo->nMaxMsgNum)<0 )
	{
		return	-1;
	}
	m_listhdrs.resize(pInfo->nMsgTypeNum);
	for(size_t i=0;i<m_listhdrs.size();i++)
	{
		m_listhdrs[i].clear();
	}

	m_pfile				= file;
	// 只要是创建就一定是修改过的，因为需要创建文件
	m_bModified			= true;

	return	0;
}
int		whlistinfile::Open(whfile *file, int nNewMaxNum, int nNewnUnitDataSize)
{
	assert(!m_pfile);
	
	m_bModified		= false;

	int	nDSize	= file->FileSize();
	// 读入头
	if( file->Read(&m_hdr) != sizeof(m_hdr) )
	{
		return	-1;
	}
	if( !m_hdr.isgood() )
	{
		// 错误的文件
		return	-2;
	}
	nDSize	-= sizeof(m_hdr);
	// 读入各个表头
	m_listhdrs.resize(m_hdr.nListTypeNum);
	if( file->Read(m_listhdrs.getbuf(), m_listhdrs.totalbytes()) != (int)m_listhdrs.totalbytes() )
	{
		return	-3;
	}
	nDSize	-= m_listhdrs.totalbytes();
	// 读入列表数据
	m_vectFileData.resize( nDSize );
	if( file->Read(m_vectFileData.getbuf(), m_vectFileData.size()) != (int)m_vectFileData.size() )
	{
		return	-4;
	}
	if( m_list.Inherit(m_vectFileData.getbuf())<0 )
	{
		return	-5;
	}

	// 校验CRC
	if( m_hdr.nCRC16 != 0 )
	{
		if( m_hdr.nCRC16 != CalcCRC() )
		{
			return	-11;
		}
	}

	if( nNewMaxNum>(int)m_list.GetHdr()->nMaxNum )
	{
		// 先扩展m_vectFileData
		size_t	nTotal	= whalist::CalcTotalSize(m_hdr.nUnitSize, nNewMaxNum);
		m_vectFileData.resize(nTotal);
		// 不需要Reset()
		if( m_list.InheritEnlarge(m_vectFileData.getbuf(), nNewMaxNum, 0)<0 )
		{
			return	-21;
		}
		// 不用设置m_bModified，因为如果没有增加新消息则不用设置
	}
	if( nNewnUnitDataSize>m_hdr.nUnitDataSize )
	{
		int	nNewUnitSize	= UNIT_T::GetTotalSize(nNewnUnitDataSize);
		// 先扩展m_vectFileData
		size_t	nTotal	= whalist::CalcTotalSize(nNewUnitSize, m_list.GetHdr()->nMaxNum);
		m_vectFileData.resize(nTotal);
		if( m_list.InheritEnlarge(m_vectFileData.getbuf(), 0, nNewUnitSize)<0 )
		{
			return	-22;
		}
		m_hdr.nUnitSize		= nNewUnitSize;
		m_hdr.nUnitDataSize	= nNewnUnitDataSize;
		// 要设置m_bModified，因为hdr改变了
		m_bModified			= true;
	}

	if( whalist::CalcTotalSize(m_list.GetHdr()->nDataUnitSize, m_list.GetHdr()->nMaxNum)
		> m_vectFileData.size()
	)
	{
		// 文件尺寸不对（太小）（大一点是可以的，因为也许将来向在后面加别的咚咚）
		return	-23;
	}

	m_pfile	= file;

	return	0;
}
int		whlistinfile::Close()
{
	if( m_pfile )
	{
		// 看文件是否有修改
		if( m_bModified )
		{
			// 重新计算CRC
			m_hdr.nCRC16	= CalcCRC();
			int		rst;
			m_bModified	= false;
			rst		= m_pfile->Rewind();
			rst		= m_pfile->Write(&m_hdr);
			rst		= m_pfile->Write(m_listhdrs.getbuf(), m_listhdrs.totalbytes());
			rst		= m_pfile->Write(m_vectFileData.getbuf(), m_vectFileData.size());
		}
		// 关闭文件由上层去做
		m_pfile	= NULL;
	}
	return	0;
}
unsigned short	whlistinfile::CalcCRC() const
{
	unsigned short	nCRC	= crc16((char*)m_listhdrs.getbuf(), m_listhdrs.totalbytes())
							^ crc16((char*)m_list.GetHdr(), m_list.CalcTotalSize());
	return	nCRC;
}
int		whlistinfile::verifydata()
{
	return	VERIFY_RST_OK;
}

bool	whlistinfile::checkandkick(int nTypeToAdd)
{
	if( m_list.GetAvailNum() > 0 )
	{
		return	true;
	}

	// 看看比自己小的类型中是否有可以被顶替的
	int	i, rst;
	// 先看已经读过的条目(所有类型的)
	for(i=0;i<m_hdr.nListTypeNum;i++)
	{
		// 这个一定会得到的
		LISTHDR_T	*pLH	= GetListHdr(i);
		if( pLH->ReadList.nTotal>0 )
		{
			// 删除这个列表中的第一个
			rst	= delinner(pLH->ReadList.nHead);
			assert(rst==0);
			// 按理说删除应该是不会不成功的
			return	true;
		}
	}
	// 再看没读过的条目(不过不能顶掉和自己类型相同的，所以要减1)
	for(i=0;i<nTypeToAdd;i++)
	{
		// 这个一定会得到的
		LISTHDR_T	*pLH	= GetListHdr(i);
		if( pLH->UnReadList.nTotal>0 )
		{
			// 删除这个列表中的第一个
			rst	= delinner(pLH->UnReadList.nHead);
			assert(rst==0);
			// 按理说删除应该是不会不成功的
			return	true;
		}
	}

	return	false;
}
int		whlistinfile::push_back(whlistinfile::ONELIST_T *pOL, int nIdxInner)
{
	UNIT_T	*pUnit	= (UNIT_T *)m_list.GetDataUnitPtr(nIdxInner);
	if( !pUnit )
	{
		return	-1;
	}

	m_list.SetNextOf(nIdxInner, whalist::INVALIDIDX);
	pUnit->nPrev	= pOL->nTail;
	if( pOL->nTail == whalist::INVALIDIDX )
	{
		// 原来是空表
		pOL->nHead	= nIdxInner;
		// 总数变为1
		pOL->nTotal	= 1;
	}
	else
	{
		m_list.SetNextOf(pOL->nTail, nIdxInner);
		// 总数增加
		pOL->nTotal	++;
	}
	pOL->nTail		= nIdxInner;

	return			0;
}
/*
int		whlistinfile::push_front(whlistinfile::ONELIST_T *pOL, int nIdxInner)
{
	UNIT_T	*pUnit		= (UNIT_T *)m_list.GetDataUnitPtr(nIdxInner);
	if( !pUnit )
	{
		return	-1;
	}

	pUnit->nPrev		= whalist::INVALIDIDX;
	m_list.SetNextOf(nIdxInner, pOL->nHead);
	if( pOL->nTail == whalist::INVALIDIDX )
	{
		// 原来是空表
		pOL->nTail		= nIdxInner;
		// 总数变为1
		pOL->nTotal	= 1;
	}
	else
	{
		((UNIT_T *)m_list.GetDataUnitPtr(pOL->nHead))->nPrev	= nIdxInner;
		// 总数增加
		pOL->nTotal	++;
	}
	pOL->nHead		= nIdxInner;

	return			0;
}
*/
int		whlistinfile::remove(whlistinfile::ONELIST_T *pOL, int nIdxInner)
{
	UNIT_T	*pUnit	= (UNIT_T *)m_list.GetDataUnitPtr(nIdxInner);
	if( !pUnit )
	{
		return	-1;
	}
	int	nPrev	= pUnit->nPrev;
	int	nNext	= m_list.GetNextOf(nIdxInner);
	if( nPrev == whalist::INVALIDIDX )
	{
		assert(pOL->nHead == nIdxInner);
		pOL->nHead	= nNext;
	}
	else
	{
		m_list.SetNextOf(nPrev, nNext);
	}
	if( nNext == whalist::INVALIDIDX )
	{
		assert(pOL->nTail == nIdxInner);
		pOL->nTail	= nPrev;
	}
	else
	{
		((UNIT_T *)m_list.GetDataUnitPtr(nNext))->nPrev	= nPrev;
	}
	pOL->nTotal	--;

	return	0;
}

int		whlistinfile::refillinfo(INFO_T *pInfo)
{
	pInfo->nMaxMsgNum		= m_list.GetHdr()->nMaxNum;
	pInfo->nMsgTypeNum		= m_hdr.nListTypeNum;
	pInfo->nUnitDataSize	= m_hdr.nUnitDataSize;
	return	0;
}
int		whlistinfile::savebadfile()
{
	// 为了保险，把旧文件内容按时间文件名存储。清空当前所有的文件内容。
	m_pfile->Rewind();
	// 写文件
	char	szFName[WH_MAX_PATH];
	sprintf(szFName, "~whlif_bad_%s", wh_getsmptimestr_for_file());
	whfile	*pOut	= whfile_OpenCmnFile(szFName, "wb");
	if( pOut )
	{
		while( !m_pfile->IsEOF() )
		{
			char	buf[4096];
			int		nSize	= m_pfile->Read(buf, sizeof(buf));
			if( nSize<=0 )
			{
				break;
			}
			pOut->Write(buf, nSize);
		}
		delete	pOut;
	}
	// 保留旧文件指针
	whfile	*pfile	= m_pfile;
	// 填写初始化信息
	INFO_T	info;
	if( refillinfo(&info)<0 )
	{
		return	-1;
	}
	// 清空文件
	m_bModified	= false;
	if( Close()<0 )
	{
		return	-2;
	}
	// 重新创建文件
	if( Create(pfile, &info)<0 )
	{
		return	-3;
	}
	return	0;
}
int		whlistinfile::push_back(int nType, void *pData, int nSize)
{
	LISTHDR_T	*pLH	= GetListHdr(nType);
	if( !pLH )
	{
		return	-1;
	}

	// 先看是否需要顶掉东西
	if( !checkandkick(nType) )
	{
		return	-2;
	}

	int		nIdx	= m_list.Alloc();

	// 拷贝数据
	assert( nSize <= m_hdr.nUnitDataSize );
	UNIT_T	*pUnit	= (UNIT_T *)m_list.GetDataUnitPtr(nIdx);
	// // 在前面有checkandkick的前提下，如果pUnit为空，则说明可能文件损坏了
	if( !pUnit )
	{
		// 为了保险，把旧文件内容按时间文件名存储。清空当前所有的文件内容。
		if( savebadfile()<0 )
		{
			// 如果出错就返回，否则可以继续进行下面的操作
			return	-3;
		}
		nIdx		= m_list.Alloc();
		pUnit		= (UNIT_T *)m_list.GetDataUnitPtr(nIdx);
		if( !pUnit )
		{
			// 如果还不行就见鬼了
			assert(0);
			return	-4;
		}
	}

	pUnit->Reset(nType);
	memcpy(pUnit->data, pData, nSize);

	push_back(&pLH->UnReadList, nIdx);

	m_bModified	= true;
	return	0;
}
int		whlistinfile::getbegin(int nType)
{
	LISTHDR_T	*pLH	= GetListHdr(nType);
	if( !pLH )
	{
		return	-1;
	}
	pLH->reset();
	m_bModified	= true;
	return	0;
}
int		whlistinfile::getnext(int nType, void *pData, int *pnIdxInner)
{
	LISTHDR_T	*pLH	= GetListHdr(nType);
	if( !pLH )
	{
		return	-1;
	}

	int	nIdxInner	= whalist::INVALIDIDX;
	// 先看是否有未读的
	UNIT_T	*pUnit	= NULL;
	if( pLH->UnReadList.nTotal>0 )
	{
		nIdxInner	= pLH->UnReadList.nHead;
		pUnit		= (UNIT_T *)m_list.GetDataUnitPtr(nIdxInner);
	}
	else if( pLH->ReadList.nTotal>0 )
	{
		nIdxInner	= pLH->nCurIdxInner;
		pUnit		= (UNIT_T *)m_list.GetDataUnitPtr(nIdxInner);
		if( pUnit )
		{
			pLH->nCurIdxInner	= pUnit->nPrev;
		}
	}
	else
	{
		// 两个表都空，则没有返回
		return		-1;
	}

	if( !pUnit )
	{
		// 没有东西了
		return	-1;
	}

	// 能读到东西文件一定更新了。因为nCurIdxInner序号会变化。
	m_bModified	= true;

	*pnIdxInner	= nIdxInner;
	memcpy(pData, pUnit->data, m_hdr.nUnitDataSize);

	if( !pUnit->IsRead() )
	{
		// 原来没有读过。从未读表移动到已读表
		remove( &pLH->UnReadList, nIdxInner );
		push_back( &pLH->ReadList, nIdxInner );
		// 设置为已读
		pUnit->SetRead();
	}

	return	0;
}
int		whlistinfile::delinner(int nIdxInner)
{
	UNIT_T	*pUnit		= (UNIT_T *)m_list.GetDataUnitPtr(nIdxInner);
	if( !pUnit )
	{
		return	-1;
	}

	LISTHDR_T	*pLH	= GetListHdr(pUnit->nType);
	if( !pLH )
	{
		return	-1;
	}
	ONELIST_T	*pOL;
	if( pUnit->IsRead() )
	{
		pOL		= &pLH->ReadList;
	}
	else
	{
		pOL		= &pLH->UnReadList;
	}

	remove(pOL, nIdxInner);

	m_list.Free(nIdxInner);
	m_bModified	= true;

	return	0;
}
int		whlistinfile::unreadinner(int nIdxInner)
{
	UNIT_T	*pUnit		= (UNIT_T *)m_list.GetDataUnitPtr(nIdxInner);
	if( !pUnit )
	{
		return	-1;
	}

	LISTHDR_T	*pLH	= GetListHdr(pUnit->nType);
	if( !pLH )
	{
		return	-1;
	}
	if( pUnit->IsRead() )
	{
		// 从已读表移动到未读表
		remove( &pLH->ReadList, nIdxInner );
		push_back( &pLH->UnReadList, nIdxInner );
		// 设置为已读
		pUnit->SetUnRead();
		//
		m_bModified	= true;
	}

	return	0;
}

////////////////////////////////////////////////////////////////////
// 基本功能函数
////////////////////////////////////////////////////////////////////
whfile *	whfile_OpenCmnFile(const char *szFileName, const char *szMode)
{
	FILE	*fp = ::fopen(szFileName, szMode);
	if( !fp )
	{
		return	NULL;
	}
	whfile_i_std	*file = new whfile_i_std(fp, szFileName);
	return	file;
}
void		whfile_CloseCmnFile(whfile *file)
{
	delete	file;
}

whfile *	whfile_OpenMemBufFile(char *szMemBuf, size_t nMemSize)
{
	return	new whfile_i_mem(szMemBuf, nMemSize);
}
void			whfile_CloseMemBufFile(whfile *file)
{
	delete	file;
}

unsigned char	*whfile_CalcMD5(whfile *file, unsigned char *MD5)
{
	md5_state_t	state;

	md5_init(&state);

	char	szBuf[0x10000];
	int		nSize;
	file->Rewind();
	while( !file->IsEOF() )
	{
		nSize	= file->Read(szBuf, sizeof(szBuf));
		if( nSize<=0 )
		{
			// 文件出错
			break;
		}
		else
		{
			md5_append(&state, (const md5_byte_t *)szBuf, nSize);
		}
	}

	md5_finish(&state, MD5);

	return	MD5;
}

class	whfileman_Cmn	: public whfileman
{
public:
	virtual	void		SelfDestroy()
	{
		delete	this;
	}
	virtual	whfile *	Open(const char*cszFName, OP_MODE_T mode)
	{
		const char	*cszMode	= "rb";
		switch( mode )
		{
		case	OP_MODE_BIN_READONLY:
			cszMode	= "rb";
			break;
		case	OP_MODE_BIN_CREATE:
			cszMode	= "wb+";	// 2006-07-26 改wb为wb+，因为我一般Create之后也可能会读
			break;
		case	OP_MODE_BIN_READWRITE:
			cszMode	= "rb+";
			break;
		default:
			// 不认识的打开模式
			return	NULL;
		}
		return	whfile_OpenCmnFile(cszFName, cszMode);
	}
	virtual	WHDirBase *		OpenDir(const char *cszDName)
	{
		WHDir	*pDir	= new WHDir;
		if( pDir )
		{
			if( pDir->Open(cszDName)<0 )
			{
				delete	pDir;
				return	NULL;
			}
		}
		return	pDir;
	}
	virtual	int				MakeDir(const char *cszDName)
	{
		// 确保目录存在
		if( whfile_ispathexisted(cszDName) )
		{
			if( whfile_ispathdir(cszDName) )
			{
				// 已经存在了
				return	0;
			}
			// 说明目标是个文件
			return	-1;
		}
		// 创建目录
		if( whdir_MakeDir(cszDName)<0 )
		{
			return	-2;
		}
		return	0;
	}
	virtual	int				SureMakeDirForFile(const char *cszFName)
	{
		return	whdir_SureMakeDirForFile(cszFName);
	}
	virtual	bool			IsPathExist(const char *__path)
	{
		return	whfile_ispathexisted(__path);
	}
	virtual	bool			IsPathDir(const char *__path)
	{
		return	whfile_ispathdir(__path);
	}
	virtual	int				GetPathInfo(const char *cszPath, PATHINFO_T *pInfo)
	{
		struct stat		st;
		int				rst;
		rst				= stat(cszPath, &st);
		pInfo->nType	= 0;
		if( rst!=0 )
		{
#ifdef	WIN32
			// 不能打开，判断一下是否是D:或//xman/write这样的，如果是加上最后的斜杠再判断
			int	nLen	= strlen(cszPath);
			if( cszPath[nLen-1]!='/' && cszPath[nLen-1]!='\\' )
			{
				switch( cszPath[1] )
				{
				case	':':
				case	'/':
				case	'\\':
					break;
				default:
					// 不是期望的那种格式
					return	-1;
				}
				whvector<char>	vect(nLen+2);
				memcpy(vect.getbuf(), cszPath, nLen);
				vect[nLen]		= '/';
				vect[nLen+1]	= 0;
				rst				= stat(vect.getbuf(), &st);
				if( rst!=0 )
				{
					return		-2;
				}
			}
			else
			{
				return	-3;
			}
#else
			return	-10;
#endif
		}
		// 存在，判断是不是目录
		if( (st.st_mode&S_IFDIR)!=0 )
		{
			pInfo->nType	|= PATHINFO_T::TYPE_DIR;
		}
		else
		{
			// 不是目录那么就获得文件大小
			pInfo->un.file.nFSize	= st.st_size;
		}
		// 没有写权限
		if( (st.st_mode&S_IWRITE)==0 )
		{
			pInfo->nType	|= PATHINFO_T::TYPE_RDOL;
		}
		// 时间
		pInfo->nMTime		= st.st_mtime;
		return	0;
	}
	virtual	int				DelFile(const char *cszPath)
	{
		return	whfile_del(cszPath);
	}
	virtual	int				DelDir(const char *cszPath)
	{
		return	whdir_sysdeldir(cszPath);
	}
};
whfileman *	whfileman_Cmn_Create()
{
	return	new whfileman_Cmn;
}

void	whPathStrTransformer::SetStr(const char *szStr)
{
	// 先判断是否是绝对路径
	if( whfile_isabspath(szStr) )
	{
		// 对于WIN32系统，绝对路径中一般一定有'\'
		m_vectStr.resize(strlen(szStr)+1);			// 这样包括最后的0结尾
		memcpy(m_vectStr.getbuf(), szStr, m_vectStr.size());
	}
	else
	{
		// 既然是相对路径就补全
		// 获得当前的工作路径
		m_vectStr.resize(WH_MAX_PATH);
		if( whdir_GetCWD(m_vectStr.getbuf(), m_vectStr.size())==NULL )
		{
			// 案例来说不应该这样的
			m_pcszStr	= NULL;
			return;
		}
		int	nLenBase	= strlen(m_vectStr.getbuf());
		int	nLenFile	= strlen(szStr);
		m_vectStr.getbuf()[nLenBase]	= '/';
		memcpy(m_vectStr.getbuf()+nLenBase+1, szStr, nLenFile+1);
		m_vectStr.resize(nLenBase+1+nLenFile+1);	// 这样包括最后的0结尾
	}
#ifdef	WIN32
	// 如果是window则把所有的'\'替换为'/'
	// 把除了第一个以外的连续的'/'变成一个'/'，头两个字符中的'\'需要都变成'/'
	if( m_vectStr.size()>2 )
	{
		char	*pBuf	= m_vectStr.getbuf();
		wh_strchrreplace(pBuf, 2, '\\', '/');
		pBuf	+= 2;
		wh_strcontinuouspatternreplace(pBuf, m_vectStr.size()-2, pBuf, "/\\", '/');
	}
#else
	// 把所有的连续的'/'变成一个'/'
	if( m_vectStr.size()>1 )
	{
		char	*pBuf	= m_vectStr.getbuf();
		wh_strcontinuouspatternreplace(pBuf, m_vectStr.size(), pBuf, "/", '/');
	}
#endif
	// 把../变成上级目录
	whvector<char *>	vectPtr;
	vectPtr.reserve(8);
	int		nDotCount	= 0;
	char	*pC			= m_vectStr.getbuf();
	char	*pCDst		= m_vectStr.getbuf();
	bool	bStop		= false;
	while(!bStop)
	{
		switch( (*pC) )
		{
		case	'.':
			{
				nDotCount	++;
			}
			break;
		case	'/':
		case	0:
			{
				if( nDotCount == 0 )
				{
					vectPtr.push_back(pCDst);
				}
				else
				{
					while( (--nDotCount)>0 )
					{
						vectPtr.pop_back();
					}
					if( vectPtr.size()==0 )
					{
						pCDst	= m_vectStr.getbuf();
					}
					else
					{
						pCDst	= vectPtr.getlast();
					}
				}
				if( (*pC)==0 )
				{
					// 最后0结尾
					*pCDst	= 0;
					bStop	= true;
				}
			}
			break;
		default:
			{
				nDotCount	= 0;
			}
			break;
		}
		*pCDst++	= *pC++;
	}
	//
	m_pcszStr	= m_vectStr.getbuf();
}

#pragma pack(1)
struct	WHCMN_FILE_STATIC_INFO_T
{
	whfileman	*pFM;
	WHCMN_FILE_STATIC_INFO_T()
	: pFM(NULL)
	{
	}
};
#pragma pack()
static WHCMN_FILE_STATIC_INFO_T		l_si;
static WHCMN_FILE_STATIC_INFO_T		*l_psi	= &l_si;
void *	WHCMN_FILE_STATIC_INFO_Out()
{
	return	&l_si;
};
void	WHCMN_FILE_STATIC_INFO_In(void *pInfo)
{
#ifdef	_DEBUG
	printf("WHCMN_FILE_STATIC_INFO_In %p %p%s", l_psi, pInfo, WHLINEEND);
#endif
	l_psi	= (WHCMN_FILE_STATIC_INFO_T *)pInfo;
}
whfileman *	WHCMN_FILEMAN_GET()
{
	return	l_psi->pFM;
}
void	WHCMN_FILEMAN_SET(whfileman *pFM)
{
	l_psi->pFM	= pFM;
}

}		// EOF namespace n_whcmn

