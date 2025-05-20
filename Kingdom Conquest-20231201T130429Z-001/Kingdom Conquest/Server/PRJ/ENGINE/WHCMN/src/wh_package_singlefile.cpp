// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_singlefile.cpp
// Creator     : Wei Hua (κ��)
// Comment     : ���ļ�ѹ���������ܣ�
// CreationDate: 2006-08-01 ������ף�й������ž�����79����^____^
// ChangeLOG   : 2007-01-26 ��������:) �����˵��ļ�ѹ���ж����ļ�����ѹ��ʱ���п������������ߴ����д����

#include "../inc/wh_package_singlefile.h"
#include "../inc/whmd5.h"
#include "../inc/whdbg.h"
#include "../inc/whstring.h"
#include "../inc/whfile.h"
#include "../inc/whdir.h"

using namespace n_whcmn;

const char	*wh_package_singlefile::HDR_T::CSZMAGIC	= "whsc1.0";

wh_package_singlefile::wh_package_singlefile()
: m_bModified(false)
{
}
wh_package_singlefile::~wh_package_singlefile()
{
	Close();
}
bool	wh_package_singlefile::IsFileOK(whfile *f)
{
	HDR_T	hdr;
	f->Seek(0, SEEK_SET);
	// �����ļ�ͷ��У��
	if( f->Read(&hdr)!=sizeof(hdr) )
	{
		return	false;
	}
	if( !hdr.IsOK() )
	{
		return	false;
	}
	return	true;
}
// �����ļ���ֻ�ǻ�ȡ�ļ���Ϣ
int		wh_package_singlefile::GetPathInfo(INFO_T *pInfo, whfileman::PATHINFO_T *pPathInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	// Ϊ�˱�������Seek���ļ�ͷ
	GetBaseFile()->Seek(0, SEEK_SET);

	// �����ļ�ͷ��У��
	if( GetBaseFile()->Read(&m_hdr)!=sizeof(m_hdr) )
	{
		return	-11;
	}
	if( !m_hdr.IsOK() )
	{
		return	-12;
	}

	pPathInfo->nMTime			= m_hdr.nFileTime;
	pPathInfo->un.file.nFSize	= m_hdr.nRealSize;

	return	0;
}
int		wh_package_singlefile::Open(wh_package_singlefile::INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	// Ϊ�˱�������Seek���ļ�ͷ
	GetBaseFile()->Seek(0, SEEK_SET);

	switch( m_info.nOPMode )
	{
	case	whfileman::OP_MODE_BIN_RAWREAD:
		{
			m_vect.resize(GetBaseFile()->FileSize());
			GetBaseFile()->Read(m_vect.getbuf(), m_vect.size());
			m_vect.f_rewind();
		}
		break;
	case	whfileman::OP_MODE_BIN_READWRITE:
	case	whfileman::OP_MODE_BIN_READONLY:
		{
			// �����ļ�ͷ��У��
			if( GetBaseFile()->Read(&m_hdr)!=sizeof(m_hdr) )
			{
				return	-11;
			}
			if( !m_hdr.IsOK() )
			{
				return	-12;
			}
			// ����һ������ȫ����
			whvector<char>	vect(m_hdr.nCompressedSize);
			if( GetBaseFile()->Read(vect.getbuf(), vect.size())!=(int)vect.size() )
			{
				// �ļ����Ȳ���
				return	-13;
			}
			// ����
			if( (m_hdr.nProp&HDR_T::PROP_F_CRYPTED) != 0 )
			{
				if( !m_info.pDecryptor )
				{
					return	-14;
				}
				m_info.pDecryptor->Reset();
				if( m_info.pDecryptor->Decrypt(vect.getbuf(), vect.size(), vect.getbuf())<0 )
				{
					return	-15;
				}
			}
			// ��ѹ��m_vect��
			m_vect.resize(m_hdr.nRealSize);
			if( (m_hdr.nProp&HDR_T::PROP_F_COMPRESSED) != 0 )
			{
				if( !m_info.pWHCompress )
				{
					return	-20;
				}
				size_t	nRealSize	= m_hdr.nRealSize;
				int		rst			= m_info.pWHCompress->decompress(vect.getbuf(), vect.size(), m_vect.getbuf(), &nRealSize);
				if( rst!=WHCompress::RST_OK )
				{
					return	-2000 + rst;
				}
				if( nRealSize!=m_vect.size() )
				{
					return	-21;
				}
			}
			else
			{
				if( vect.size() != m_vect.size() )
				{
					// �����ѹ�������һ������ȵ�
					return	-22;
				}
				memcpy(m_vect.getbuf(), vect.getbuf(), m_vect.size());
			}
			// vector�ļ�ָ���ƶ������ݿ�ͷ
			m_vect.f_rewind();
		}
		break;
	case	whfileman::OP_MODE_BIN_CREATE:
		{
			// �����ļ�ͷ�������Ȳ���д�������д��
			m_hdr.reset();
			if( m_info.pWHCompress )
			{
				m_hdr.nProp	|= HDR_T::PROP_F_COMPRESSED;
			}
			if( m_info.pEncryptor )
			{
				m_hdr.nProp	|= HDR_T::PROP_F_CRYPTED;
			}
			m_hdr.nFileTime	= wh_time();
			// ����ļ����޸�
			m_bModified	= true;
			// ����ļ�����
			m_vect.reserve(m_info.nReserveSize);
			m_vect.clear();
		}
		break;
	default:
		// ����Ĵ�ģʽ
		return	-100;
	}

	return	0;
}
int		wh_package_singlefile::Close()
{
	if( m_bModified )
	{
		assert( !IsReadOnly() );
		// ����MD5
		whmd5(m_vect.getbuf(), m_vect.size(), m_hdr.MD5);
		// ѹ������
		m_hdr.nRealSize			= m_vect.size();
		m_hdr.nCompressedSize	= m_hdr.nRealSize*2;				// Ϊ��Ӧ��ѹ��������������԰�ѹ����������һЩ
		whvector<char>	vect(m_hdr.nCompressedSize);
		size_t	nCompressedSize	= m_hdr.nCompressedSize;
		if( (m_hdr.nProp&HDR_T::PROP_F_COMPRESSED) != 0 )
		{
			int		rst				= m_info.pWHCompress->compress(m_vect.getbuf(), m_vect.size(), vect.getbuf(), &nCompressedSize);
			if( rst!=WHCompress::RST_OK )
			{
				return	-1500 + rst;
			}
		}
		if( nCompressedSize>=m_hdr.nRealSize )
		{
			// ��ô�Ͳ���ѹ����
			whcmnbit_clr(m_hdr.nProp, (unsigned char)HDR_T::PROP_F_COMPRESSED);
			memcpy(vect.getbuf(), m_vect.getbuf(), m_hdr.nRealSize);
			nCompressedSize			= m_hdr.nRealSize;
		}
		m_hdr.nCompressedSize		= nCompressedSize;
		vect.resize(nCompressedSize);
		// ��������
		if( (m_hdr.nProp&HDR_T::PROP_F_CRYPTED) != 0 )
		{
			if( m_info.pEncryptor )	// ������ܾͱ��뱣֤�м�����
			{
				m_info.pEncryptor->Reset();
				if( m_info.pEncryptor->Encrypt(vect.getbuf(), vect.size(), vect.getbuf())<0 )
				{
					return	-1;
				}
			}
			else
			{
				return	-2;
			}
		}
		// �ļ�ͷ����
		if( GetBaseFile()->Seek(0, SEEK_SET)!=0 )
		{
			return	-10;
		}
		if( GetBaseFile()->Write(&m_hdr)!=sizeof(m_hdr) )
		{
			return	-11;
		}
		// �ļ����ݴ���
		if( GetBaseFile()->Write(vect.getbuf(), vect.size())!=(int)vect.size() )
		{
			return	-12;
		}
		// ������ı��
		m_bModified	= false;
	}
	if( m_info.bAutoCloseBaseFile )
	{
		WHSafeSelfDestroy(m_info.pBaseFile);
	}
	return	0;
}
int		wh_package_singlefile::FileSize() const
{
	return	m_vect.size();
}
time_t	wh_package_singlefile::FileTime() const
{
	return	m_hdr.nFileTime;
}
void	wh_package_singlefile::SetFileTime(time_t t)
{
	if( t==0 )
	{
		t	= wh_time();
	}
	m_hdr.nFileTime	= t;
}
int		wh_package_singlefile::Read(void *pBuf, int nSize)
{
	return	m_vect.f_read(pBuf, nSize);
}
int		wh_package_singlefile::Seek(int nOffset, int nOrigin)
{
	return	m_vect.f_seek(nOffset, nOrigin, IsReadOnly());
}
bool	wh_package_singlefile::IsEOF()
{
	// ���Ӧ��ֻ�Զ�ȡ��Ч������д�ļ�Ӧ������true������д�ļ���ʱ��m_vectҲӦ����Զ��true��
	return	m_vect.f_iseof();
}
int		wh_package_singlefile::Tell()
{
	// ��д��ʱ��offset��Զָ��vector��β��λ�ã�����ʱ��Ҳ�պ����������Ծ��������
	return	m_vect.f_tell();
}
int		wh_package_singlefile::Write(const void *pBuf, int nSize)
{
	// ֻ���ļ��ǲ���д��
	if( IsReadOnly() )
	{
		return	-1;
	}
	m_bModified	= true;
	return	m_vect.f_write(pBuf, nSize);
}
int		wh_package_singlefile::Flush()
{
	// ƽʱflush��û�õģ�ֻ���ڹرյ�ʱ�����һ����ѹ��д������ļ�
	return	-1;
}
int		wh_package_singlefile::GetMD5(unsigned char *MD5)
{
	memcpy(MD5, m_hdr.MD5, sizeof(m_hdr.MD5));
	return	0;
}

namespace n_whcmn
{
class	wh_singlefileman	: public whfileman
{
	// Ϊ�ӿ�ʵ�ֵ�
public:
	virtual	~wh_singlefileman();
	virtual	void	SelfDestroy();
	virtual	whfile *		Open(const char *cszFName, OP_MODE_T mode);
	virtual	WHDirBase *		OpenDir(const char *cszDName);
	virtual	int				MakeDir(const char *cszDName);
	virtual	int				SureMakeDirForFile(const char *cszFName);
	virtual	bool			IsPathExist(const char *__path);
	virtual	bool			IsPathDir(const char *__path);
	virtual	int				GetPathInfo(const char *cszPath, PATHINFO_T *pInfo);
	virtual	int				DelFile(const char *cszPath);
	virtual	int				DelDir(const char *cszPath);
protected:
	whfileman_singlefileman_INFO_T	m_info;
public:
	wh_singlefileman(whfileman_singlefileman_INFO_T *pInfo);
};
wh_singlefileman::wh_singlefileman(whfileman_singlefileman_INFO_T *pInfo)
{
	memcpy(&m_info, pInfo, sizeof(m_info));
	// ���ü��ܽ������루�������Ϊ�վ��Զ������ü��ܽ�������
	if( m_info.pEncryptor )
	{
		if( m_info.nPassLen>0 )
		{
			m_info.pEncryptor->SetEncryptKey(m_info.szPass, m_info.nPassLen);
		}
		else
		{
			m_info.pEncryptor	= NULL;
		}
	}
	if( m_info.pDecryptor )
	{
		if( m_info.nPassLen>0 )
		{
			m_info.pDecryptor->SetDecryptKey(m_info.szPass, m_info.nPassLen);
		}
		else
		{
			m_info.pDecryptor	= NULL;
		}
	}
}
wh_singlefileman::~wh_singlefileman()
{
	if( m_info.bAutoCloseFM )
	{
		WHSafeSelfDestroy(m_info.pFM);
	}
}
void	wh_singlefileman::SelfDestroy()
{
	delete	this;
}
whfile *		wh_singlefileman::Open(const char *cszFName, OP_MODE_T mode)
{
	wh_package_singlefile::INFO_T		info;
	info.nOPMode						= mode;
	info.pBaseFile						= m_info.pFM->Open(cszFName, mode);
	info.pWHCompress					= m_info.pWHCompress;
	info.pEncryptor						= m_info.pEncryptor;
	info.pDecryptor						= m_info.pDecryptor;
	info.bAutoCloseBaseFile				= true;
	if( info.pBaseFile==NULL )
	{
		return	NULL;
	}

	wh_package_singlefile	*fpSingle	= new wh_package_singlefile;
	if( fpSingle->Open(&info)<0 )
	{
		WHSafeSelfDestroy(fpSingle);
		if( m_info.bCanOpenCmnFile )
		{
			return	m_info.pFM->Open(cszFName, mode);
		}
		else
		{
			return	NULL;
		}
	}
	return	fpSingle;
}
WHDirBase *		wh_singlefileman::OpenDir(const char *cszDName)
{
	return	m_info.pFM->OpenDir(cszDName);
}
int				wh_singlefileman::MakeDir(const char *cszDName)
{
	return	m_info.pFM->MakeDir(cszDName);
}
int				wh_singlefileman::SureMakeDirForFile(const char *cszFName)
{
	return	m_info.pFM->SureMakeDirForFile(cszFName);
}
bool			wh_singlefileman::IsPathExist(const char *__path)
{
	return	m_info.pFM->IsPathExist(__path);
}
bool			wh_singlefileman::IsPathDir(const char *__path)
{
	return	m_info.pFM->IsPathDir(__path);
}
int				wh_singlefileman::GetPathInfo(const char *cszPath, PATHINFO_T *pInfo)
{
	wh_package_singlefile	*fpSingle	= new wh_package_singlefile;
	wh_package_singlefile::INFO_T		info;
	info.nOPMode						= whfileman::OP_MODE_BIN_READONLY;
	info.pBaseFile						= m_info.pFM->Open(cszPath, whfileman::OP_MODE_BIN_READONLY);
	info.pWHCompress					= m_info.pWHCompress;
	info.pEncryptor						= m_info.pEncryptor;
	info.pDecryptor						= m_info.pDecryptor;
	info.bAutoCloseBaseFile				= true;
	if( fpSingle->GetPathInfo(&info, pInfo)<0 )
	{
		WHSafeSelfDestroy(fpSingle);
		return	-1;
	}
	WHSafeSelfDestroy(fpSingle);
	return	0;
}
int				wh_singlefileman::DelFile(const char *cszPath)
{
	return	whfile_del(cszPath);
}
int				wh_singlefileman::DelDir(const char *cszPath)
{
	return	whdir_sysdeldir(cszPath);
}

whfileman *	whfileman_singlefileman_Create(whfileman_singlefileman_INFO_T *pInfo)
{
	return	new wh_singlefileman(pInfo);
}


class	wh_singlefileman_Easy	: public wh_singlefileman
{
private:
	ICryptFactory	*m_pICryptFactory;
public:
	wh_singlefileman_Easy(whfileman_singlefileman_INFO_T *pInfo, ICryptFactory *pICryptFactory)
		: wh_singlefileman(pInfo)
		, m_pICryptFactory(pICryptFactory)
	{
	}
	virtual	~wh_singlefileman_Easy()
	{
		// �ͷ��Լ���info�ڵĶ�������϶����ڸ��������֮ǰ���õģ�
		WHSafeSelfDestroy(m_info.pWHCompress);
		WHSafeSelfDestroy(m_info.pEncryptor);
		WHSafeSelfDestroy(m_info.pDecryptor);
		WHSafeSelfDestroy(m_pICryptFactory);
	}
	virtual	void	SelfDestroy()
	{
		delete	this;
	}
};
whfileman *	whfileman_singlefileman_Create_Easy(whfileman_singlefileman_INFO_Easy_T *pInfo)
{
	ICryptFactory	*pICryptFactory	= ICryptFactory_DHKeyAndDES_Create();
	whfileman_singlefileman_INFO_T	info;
	info.bAutoCloseFM	= true;
	info.bCanOpenCmnFile= pInfo->bCanOpenCmnFile;
	info.pFM			= whfileman_Cmn_Create();
	info.pWHCompress	= WHCompress_Create_ZLib();
	info.pEncryptor		= pICryptFactory->CreateICryptor(pInfo->nCryptType);
	info.pDecryptor		= pICryptFactory->CreateICryptor(pInfo->nCryptType);
	memcpy(info.szPass, pInfo->szPass, pInfo->nPassLen);
	info.nPassLen		= pInfo->nPassLen;
	return	new wh_singlefileman_Easy(&info, pICryptFactory);
}


}		// EOF namespace n_whcmn
