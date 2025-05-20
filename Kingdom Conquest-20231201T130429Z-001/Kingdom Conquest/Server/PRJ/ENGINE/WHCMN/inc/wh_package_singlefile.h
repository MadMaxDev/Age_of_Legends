// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_singlefile.h
// Creator     : Wei Hua (κ��)
// Comment     : ���ļ�ѹ���������ܣ�
// CreationDate: 2006-08-01 ������ף�й������ž�����79����^____^
// ChangeLOG   : 

#ifndef	__WH_PACKAGE_SINGLEFILE_H__
#define __WH_PACKAGE_SINGLEFILE_H__

#include "whcmn_def.h"
#include "whfile_base.h"
#include "wh_compress_interface.h"
#include "wh_crypt_interface.h"
#include "whvector.h"

namespace n_whcmn
{

class	wh_package_singlefile	: public whfile
{
public:
	// pBaseFile��ѹ���������մ洢�����ļ�
	// nOPMode�ǵײ��ļ�Ҳ���Լ��Ĵ򿪷�ʽ
	wh_package_singlefile();
	~wh_package_singlefile();
	// �Լ��õ�
	struct	INFO_T 
	{
		whfileman::OP_MODE_T	nOPMode;							// �򿪰��ķ�ʽ����ʱ��Ҳ����Ҫֻ���򿪵ģ�������Ϸ�е�ʱ��
		whfile					*pBaseFile;							// ����ײ���ļ���Ҳ������һ����ڴ��ɵ��ڴ��ļ���
		WHCompress				*pWHCompress;						// ѹ����
		ICryptFactory::ICryptor	*pEncryptor;						// ������
		ICryptFactory::ICryptor	*pDecryptor;						// ������
		int						nReserveSize;						// ����Ǵ����ļ����ʼ�ȱ���һ�����ȣ��������Լ����������ݵ���ʱ�������ڴ����
		bool					bAutoCloseBaseFile;					// �Ƿ��Զ�ɾ������raw�ļ������ϲ㲻��Ҫ�ౣ��һ���ļ�ָ������ɾ�������ļ���ʱ����Ϊ���ã�
		INFO_T()
			: nOPMode(whfileman::OP_MODE_UNDEFINED)
			, pBaseFile(NULL)
			, pWHCompress(NULL)
			, pEncryptor(NULL), pDecryptor(NULL)
			, nReserveSize(4096)
			, bAutoCloseBaseFile(false)
		{
		}
	};
	int		GetPathInfo(INFO_T *pInfo, whfileman::PATHINFO_T *pPathInfo);
																	// �����ļ���ֻ�ǻ�ȡ�ļ���Ϣ
	int		Open(INFO_T *pInfo);
	int		Close();
	inline whfile *	GetBaseFile()
	{
		return	m_info.pBaseFile;
	}
	inline bool	IsReadOnly() const
	{
		return	m_info.nOPMode == whfileman::OP_MODE_BIN_READONLY;
	}
	inline const unsigned char *	GetMD5() const
	{
		return	m_hdr.MD5;
	}
	// ��whfile�ӿ�ʵ�ֵ�
	void	SelfDestroy()
	{
		delete	this;
	}
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t);
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
	int		Write(const void *pBuf, int nSize);
	int		Flush();
	int		GetMD5(unsigned char *MD5);
protected:
	INFO_T						m_info;
	// �ļ�ͷ
	#pragma pack(1)
	struct	HDR_T
	{
		enum
		{
			VER			= 0
		};
		enum
		{
			PROP_F_COMPRESSED	= 0x01,								// ��ʾ�ļ���ѹ��
			PROP_F_CRYPTED		= 0x02,								// ��ʾ�ļ�������
		};
		char			szMagic[15];								// ħ���������ڱ�ʶ�ļ�����
		unsigned char	nProp;										// һЩ���ԣ�ȡֵ�������PROP_XXX��
		unsigned char	MD5[16];									// �ļ���MD5У��ˣ��������Ͳ���ÿ�ζ�������MD5�ˣ�
		int				nVer;										// �汾
		unsigned int	nRealSize;									// �ļ����ȣ���ûѹ��ǰ�ĳ��ȣ�
		unsigned int	nCompressedSize;							// �ļ�ѹ��֮��ĳ��ȣ�������basefile�еĳ��ȣ�
																	// ע��֮���Լ�¼��������������Ϊ����ѹ���ļ��ܱ�С�����Ǵ���ԭ�����ļ�����
		time_t			nFileTime;									// ��ѹ��Դ�ļ���ʱ�䣨ʵ���ļ��ĸ�дʱ�����ͨ���ײ��ļ���ã�

		static const char		*CSZMAGIC;

		void	reset()
		{
			WHMEMSET0THIS();
			nVer			= VER;
			strcpy(szMagic, CSZMAGIC);
		}
		bool	IsOK() const										// �жϴ��ļ��ж�����ͷ�Ƿ���ȷ
		{
			return	nVer == VER && strcmp(szMagic, CSZMAGIC)==0;
		}
	};
	#pragma pack()
	HDR_T						m_hdr;
	// ��ȡ�ļ�ʱ���ڲ�����
	whvector<char>				m_vect;
	// �ļ��Ƿ�ı�
	bool						m_bModified;
public:
	static bool	IsFileOK(whfile *f);								// �ж��ļ��Ƿ���single�ļ�
};

// ����һ��ÿ���ļ����ǵ���ѹ���ļ����ļ�������
struct	whfileman_singlefileman_INFO_T
{
	bool					bAutoCloseFM;		// �Ƿ��Զ��ر�pFM
	bool					bCanOpenCmnFile;	// �Ƿ���Դ���ͨ�ļ�������˵�򿪵Ĳ���ѹ���ļ���
	whfileman				*pFM;				// �ײ���ļ�ϵͳ��Ҳ�����ǰ�Ŷ��
	WHCompress				*pWHCompress;		// ѹ����
	ICryptFactory::ICryptor	*pEncryptor;		// ������
	ICryptFactory::ICryptor	*pDecryptor;		// ��������ע�⣺�������ͽ������ɲ���һ����
	char					szPass[128];		// ���루���û���������ĳ���Ϊ0��
	int						nPassLen;			// ���볤��
	whfileman_singlefileman_INFO_T()
		: bAutoCloseFM(false)
		, bCanOpenCmnFile(false)
	{
	}
};
whfileman *	whfileman_singlefileman_Create(whfileman_singlefileman_INFO_T *pInfo);

// �����Ƚϼ򵥵Ĵ�������
struct	whfileman_singlefileman_INFO_Easy_T
{
	bool					bCanOpenCmnFile;	// �Ƿ���Դ���ͨ�ļ�������˵�򿪵Ĳ���ѹ���ļ���
	int						nCryptType;			// ���������������ͣ���ҹ涨����ͬ�����Ϳ����ˣ�Ŀǰû����ȷ���壬Ĭ�϶���0Ҳ�ɣ�
	char					szPass[128];		// ���루���û���������ĳ���Ϊ0��
	int						nPassLen;			// ���볤��
	whfileman_singlefileman_INFO_Easy_T()
		: bCanOpenCmnFile(false)
		, nCryptType(0)
		, nPassLen(0)
	{
	}
};
whfileman *	whfileman_singlefileman_Create_Easy(whfileman_singlefileman_INFO_Easy_T *pInfo);

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_SINGLEFILE_H__
