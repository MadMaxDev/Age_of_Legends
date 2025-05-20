// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whzlib
// File        : whzlib.h
// Creator     : Wei Hua (κ��)
// Comment     : ��zlib���ܵİ�װ�����䣬Ŀǰ������lzo (lzoѹ���ʺ����zlib��һЩ�������ٶȿ�Щ)
// CreationDate: 2003-09-04
// ChangeLOG   :
//               2004-02-16 �����˵�һ�ļ�ѹ��
//               2006-01-23 ��uncompress����Ϊdecompress

#ifndef	__WHZLIB_H__
#define __WHZLIB_H__

#include <stdio.h>
#include <time.h>
#include "whdir.h"

// ѹ�����汾
// ��ǰʹ�õİ汾
#define WHZLIB_VER			0x00000002
// ֧�ֶ�ȡ����Ͱ汾
#define WHZLIB_LOWRVER		0x00000002

// ����ѹ���ļ��汾
// ��ǰʹ�õİ汾
#define WHZLIB_SC_VER		0x00000001
// ֧�ֶ�ȡ����Ͱ汾
#define WHZLIB_SC_LOWRVER	0x00000001

namespace n_whzlib
{

enum
{
	WHZLIB_MAX_RAWDATAUNIT_SIZE	= 65536,		// ��ʵҲ���Ը��󣬷��������õ���uint32
};

////////////////////////////////////////////////////////////////////
// ��ӿ�
////////////////////////////////////////////////////////////////////
// ����zlib���ļ�����
// ��ֻ��ͨ��whzlib_fileman::Open����������ͨ��whzlib_fileman::Close��������
typedef	n_whcmn::whfile	whzlib_file;

// ����zlib���ļ������ʹ�����
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
	// ����������
	static whzlib_fileman *		Create();
	virtual ~whzlib_fileman()	{}
public:
	// �Ƿ����ȴӰ��ļ��м���(Ĭ����true�����ȼ���ǰ��е��ļ������û���ټ���Ƿ�����ļ�)
	virtual void	SetSearchMode(int nMode)				= 0;
	// ��ü���ģʽ
	virtual int		GetSearchMode() const					= 0;
	// ���ý�ѹ����
	virtual void	SetPassword(const char *szPassword)		= 0;
	// Ԥ�ȼ��ذ��ļ����ڴ�
	virtual int		LoadPckToMem(const char *szPckName)		= 0;
	virtual int		UnLoadPckFromMem(const char *szPckName)	= 0;
	// Ԥ�ȼ��ص����ļ����ڴ�
	virtual int		LoadFileToMem(const char *szFileName)	= 0;
	virtual int		UnLoadFileFromMem(const char *szFileName)
															= 0;
	// szFileName�����ȫ·����(����ᰴ�յ�ǰ·�����ң��ڵ�ǰ·������ʱ���ܻ���ֻ���)
	// ���szFileName��ʵ�ʴ��ڵ��ļ��������ļ���
	// ���szFileName�����ڣ����ҵ������ļ���ѹ�������Ӱ��д�
	// ���szModeΪ�գ����ߵ���"rb"����Ĭ�ϵķ�ʽ�򿪣�����ֻ�������ļ���ʽ��
	virtual whzlib_file *	Open(const char *szFileName, const char *szMode=NULL)
															= 0;
	// �ر��ļ�
	virtual bool	Close(whzlib_file *file)				= 0;
	// ����ļ�����
	virtual int		GetFileSize(const char *szFileName)		= 0;
	// �ж��ļ��Ƿ����
	virtual bool	IsFileExist(const char *szFileName)		= 0;
	// ��Ŀ¼��ص�(ʹ���������delete����)
	// �򿪲�����
	virtual n_whcmn::WHDirBase *	OpenDir(const char *szDirName)
															= 0;
};

// ��zlib���ļ����·��ʹ�����
class	whzlib_filemodifyman
{
public:
	// ����������
	static whzlib_filemodifyman *	Create();
	virtual ~whzlib_filemodifyman()	{}
public:
	enum
	{
		PROPERTY_AUTOCREATEPCK		= 0x0001,				// �Զ�������(��putfile���bShouldInPck����û�з��ְ������Զ�����֮)
	};
	unsigned int	m_nProperty;							// ���������������ɷ��ʡ�����
	int				m_nUnitSize;							// �������ݿ�ѹ��ǰ����󳤶ȡ�
public:
	whzlib_filemodifyman()
	: m_nProperty(0)
	, m_nUnitSize(WHZLIB_MAX_RAWDATAUNIT_SIZE)
	{
	}
	// ���ý�ѹ����
	virtual void	SetPassword(const char *szPassword)		= 0;
	// ��������
	// �滻������ļ�
	// bShouldInPck��ʾӦ�÷��ڰ��У��Ҳ��������Ǵ���
	// nCompressMode=COMPRESSMODE_XXX
	virtual int		PutFile(const char *szFileName, whzlib_file *file, bool bShouldInPck, int nCompressMode)
															= 0;
	// ɾ���ļ�
	virtual int		DelFile(const char *szFileName)			= 0;
	// ɾ��Ŀ¼
	virtual int		DelDir(const char *szDir)				= 0;

	// ���������ݴ洢��Ӳ����
	virtual int		FlushAll()								= 0;
};

////////////////////////////////////////////////////////////////////
// ���ܺ���
////////////////////////////////////////////////////////////////////
// ֱ�Ӵ���ͨ�ļ�����whzlib_file����
// szModeһ�����"rb"
inline whzlib_file *	whzlib_OpenCmnFile(const char *szFileName, const char *szMode)
{
	return	n_whcmn::whfile_OpenCmnFile(szFileName, szMode);
}
// �ر���ͨ�ļ�����
inline void				whzlib_CloseCmnFile(whzlib_file *file)
{
	n_whcmn::whfile_CloseCmnFile(file);
}

// ���ڴ洴��whzlib_file����
inline whzlib_file *	whzlib_OpenMemBufFile(char *szMemBuf, size_t nMemSize)
{
	return	n_whcmn::whfile_OpenMemBufFile(szMemBuf, nMemSize);
}
inline	void			whzlib_CloseMemBufFile(whzlib_file *file)
{
	n_whcmn::whfile_CloseMemBufFile(file);
}

// ֱ���ڴ�ѹ������
// ˵����*pnDstSize������ʱ���pDst��������ɵ��ֽ���
// ����ֵ<0��ʾ����
// ѹ��
int	whzlib_compress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize);
// ��ѹ
int	whzlib_decompress(const void *pSrc, size_t nSrcSize, void *pDst, size_t *pnDstSize);

// �ж�ѹ�����Ƿ����
bool	whzlib_ispckexist(const char *cszPck);

// ����������ļ�������
struct	PCKFILENAMES_T
{
	char	szIdx[WH_MAX_PATH];
	char	szDat[WH_MAX_PATH];
	char	szNam[WH_MAX_PATH];
};
int	whzlib_getpckfilenames(const char *cszPck, PCKFILENAMES_T *pNames);

////////////////////////////////////////////////////////////////////
// ��һ�ļ�ѹ����ʵ�ּ�whzlib_i.h��
////////////////////////////////////////////////////////////////////

// ��ͨ��zip�ļ���ѹ����ָ���Ĵ����ļ�
int		whzlib_single_compress(whzlib_file *file, const char *cszDstFile, const char *cszPasswd);
// Դ�ļ���Ϊ��ѹ�����ļ���ʽ��������ͨ��zip�ļ���
// ע��ر�ʱ��Ҫ�ȹرպ�����ģ��ٹر�srcfile��
whzlib_file *	whzlib_OpenSingleCompress(whzlib_file *srcfile, const char *cszPasswd);
void			whzlib_CloseSingleCompress(whzlib_file *singlefile);
// �ж�һ���ļ����Ƿ���single compressed (ע�⣬����жϻ�ı��ļ��ڲ���ƫ��ָ��)
bool			whzlib_issinglecompressed(whzlib_file *srcfile);

}		// EOF namespace n_whzlib

#endif	// EOF __WHZLIB_H__
