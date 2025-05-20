// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: ��
// File: whzio.cpp
// Creator: Wei Hua (κ��)
// Comment: �Դ���ļ���ȡ�İ�װ��ʹ��Ҫ����stdio
// CreationDate: 2003-11-27


#include "../inc/whzio.h"
#include <assert.h>

#ifdef	USE_WHZIO

using namespace n_whzlib;

static whzlib_fileman *g_pMan = NULL;

bool	WHZIO_Init(int nSearchMode)
{
	// ��debug������Ҫ��ʾ�û����ܶ�γ�ʼ��
	assert(!g_pMan);

	if( g_pMan )
	{
		// �Ѿ���ʼ������
		if( g_pMan->GetSearchMode() != nSearchMode )
		{
			// nSearchMode��ԭ����ƥ��
			return	false;
		}
		else
		{
			// ��ԭ��һ��������Ϊ�ɹ���
			return	true;
		}
	}

	g_pMan	= whzlib_fileman::Create();
	if( !g_pMan )
	{
		return	false;
	}
	g_pMan->SetSearchMode(nSearchMode);

	return	true;
}

bool	WHZIO_Release()
{
	if( g_pMan )
	{
		delete	g_pMan;
		g_pMan	= NULL;
	}
	return	true;
}

int		WHZIO_LoadPckToMem(const char *szPckName)
{
	assert( g_pMan );
	return	g_pMan->LoadPckToMem(szPckName);
}
int		WHZIO_UnLoadPckFromMem(const char *szPckName)
{
	assert( g_pMan );
	return	g_pMan->UnLoadPckFromMem(szPckName);
}
int		WHZIO_LoadFileToMem(const char *szFileName)
{
	assert( g_pMan );
	return	g_pMan->LoadFileToMem(szFileName);
}
int		WHZIO_UnLoadFileFromMem(const char *szFileName)
{
	assert( g_pMan );
	return	g_pMan->UnLoadFileFromMem(szFileName);
}

void	WHZIO_SetPassword(const char *szPassword)
{
	assert( g_pMan );
	g_pMan->SetPassword(szPassword);
}

WHZFILE *	WHZIO_Open(const char *szFileName, const char *szMode)
{
	assert( g_pMan );
	return	g_pMan->Open(szFileName, szMode);
}

int		WHZIO_Close(WHZFILE *fp)
{
	assert( g_pMan );
	return	g_pMan->Close(fp);
}
FILE *	WHZIO_GetFILEPtr(WHZFILE *fp)
{
	return	fp->GetFILE();
}

int		WHZIO_GetFileSize(const char *szFileName)
{
	assert( g_pMan );
	return	g_pMan->GetFileSize(szFileName);
}
int		WHZIO_GetFileSize(WHZFILE *fp)
{
	return	fp->FileSize();
}
int		WHZIO_GetFileMD5(WHZFILE *fp, unsigned char *MD5)
{
	return	fp->GetMD5(MD5);
}
time_t	WHZIO_GetFileTime(WHZFILE *fp)
{
	return	fp->FileTime();
}
bool	WHZIO_IsFileExist(const char *szFileName)
{
	assert( g_pMan );
	return	g_pMan->IsFileExist(szFileName);
}

int		WHZIO_Read(WHZFILE *fp, void *pBuf, int nSize)
{
	return	fp->Read(pBuf, nSize);
}
int		WHZIO_FRead(void *pBuf, size_t nSize, size_t nCount, WHZFILE *fp)
{
	return	fp->Read(pBuf, nSize*nCount);
}
int		WHZIO_ReadLine(WHZFILE *fp, char *pBuf, int nSize)
{
	return	fp->ReadLine(pBuf, nSize);
}
int		WHZIO_Seek(WHZFILE *fp, int nOffset, int nOrigin)
{
	return	fp->Seek(nOffset, nOrigin);
}
bool	WHZIO_IsEOF(WHZFILE *fp)
{
	return	fp->IsEOF();
}
int		WHZIO_Tell(WHZFILE *fp)
{
	return	fp->Tell();
}

WHZDIR *	WHZIO_OpenDir(const char *szDirName)
{
	assert( g_pMan );
	return	g_pMan->OpenDir(szDirName);
}
int		WHZIO_CloseDir(WHZDIR *dp)
{
	delete	dp;
	return	0;
}
WHZDIRENTRY *	WHZIO_ReadDir(WHZDIR *dp)
{
	return	dp->Read();
}

#endif	// EOF USE_WHZIO
