// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: ��
// File         : whzio.h
// Creator      : Wei Hua (κ��)
// Comment      : �Դ���ļ���ȡ�İ�װ��ʹ��Ҫ����stdio��
//              : ע�⣺���ܶ��߳�ʹ��!!!!
//              : ��л��������ʹ����Wei Dai��CryptoPP�е�zlibʵ�֡�2006-01-23��
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

// ��ʼ��(bCheckPckFirst��ʾ�Ƿ����ȴӰ������ļ�)
bool	WHZIO_Init(int nSearchMode = WHZIO_SMODE_1PCK2FILE);
// �ս�
bool	WHZIO_Release();
// Ԥװ�ذ�(�������Լ��ٳ����������ļ�������ʱ̫��)
int		WHZIO_LoadPckToMem(const char *szPckName);
int		WHZIO_UnLoadPckFromMem(const char *szPckName);
// Ԥװ���ļ�
int		WHZIO_LoadFileToMem(const char *szFileName);
int		WHZIO_UnLoadFileFromMem(const char *szFileName);

// ���ý�ѹ����
void	WHZIO_SetPassword(const char *szPassword);

// **** �ļ����� ****
// ���ļ�
// ���szOPMode��Ϊ�ղ��Ҳ�Ϊ"rb"�����ļ���ʽ�򿪣����ҿ��Դ�������FILEָ��
WHZFILE *	WHZIO_Open(const char *szFileName, const char *szOPMode = NULL);
// �ر��ļ�
int		WHZIO_Close(WHZFILE *fp);
// ���ϵͳ�ļ�ָ��(����еĻ�)
FILE *	WHZIO_GetFILEPtr(WHZFILE *fp);
// ����ļ�����
int		WHZIO_GetFileSize(const char *szFileName);
int		WHZIO_GetFileSize(WHZFILE *fp);
// ����ļ�md5
int		WHZIO_GetFileMD5(WHZFILE *fp, unsigned char *MD5);
// ����ļ�ʱ��
time_t	WHZIO_GetFileTime(WHZFILE *fp);
// �ж�һ���ļ��Ƿ����
bool	WHZIO_IsFileExist(const char *szFileName);
// ��ȡ�ļ�(���ض������ֽ���)
// ��������ļ�β�򷵻�0��������<0
int		WHZIO_Read(WHZFILE *fp, void *pBuf, int nSize);
// �����Ϊ�˺�freadͳһ��������ogg�Ķ�ȡ�еĻص���
int		WHZIO_FRead(void *pBuf, size_t nSize, size_t nCount, WHZFILE *fp);
// ��ȡһ�С������е��ֽ������а���������"\n"��"\r\n"���ļ�����ʲô����ʲô��
int		WHZIO_ReadLine(WHZFILE *fp, char *pBuf, int nSize);
// �ƶ��ļ�ָ��
// nOriginʹ�ú�fseek����һ����SEEK_SET��SEEK_CUR��SET_END
// �ṩ����seek
// ����0��ʾ�ɹ���<0��ʾ������
// �磺WHZIO_Seek(fp, SEEK_SET, 100);
int		WHZIO_Seek(WHZFILE *fp, int nOffset, int nOrigin);
// �ж��Ƿ����ļ���β
bool	WHZIO_IsEOF(WHZFILE *fp);
// ���ߵ�ǰ�ļ�ָ���λ��
int		WHZIO_Tell(WHZFILE *fp);

// **** Ŀ¼���� ****
// ��Ŀ¼
WHZDIR *	WHZIO_OpenDir(const char *szDirName);
// �ر�Ŀ¼
int		WHZIO_CloseDir(WHZDIR *dp);
// ��ȡһ��Ŀ¼��Ŀ�����û���˾ͷ���NULL
WHZDIRENTRY *	WHZIO_ReadDir(WHZDIR *dp);

#endif	// EOF USE_WHZIO

#endif	// EOF __WHZIO_H__
