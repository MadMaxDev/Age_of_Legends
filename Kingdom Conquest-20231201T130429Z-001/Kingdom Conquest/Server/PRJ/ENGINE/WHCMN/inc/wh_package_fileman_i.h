// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_fileman_i.h
// Creator     : Wei Hua (κ��)
// Comment     : ���ڰ���ͨ���ļ�ϵͳ
// CreationDate: 2006-08-09
// ChangeLOG   : 2007-01-31 ��SureOpenPackage��LoadPackage������const char *cszExt����
//             : 2007-02-12 SetPackageOpenMode��Ϊ����set�Ŀ��Զ���ǰ�������
//             : 2007-03-13 ��������Debug״̬����������ļ������д�д�ַ��Ҵ򲻿��ļ���������ͱ�assert��
//             : LoadPackage�����ж��˶����Ƿ��Ѿ�������
//             : 2007-06-19 ������������ȡm_info�ķ�����������Ҫ�����趯̬�����ļ�����ģʽ
//			   : 2008-06-06	����һ���ӿڣ�����ֱ�Ӷ�ȡԭʼ��ѹ�����ݣ�by ���Ľܣ�
//			   : 2008-07-18 ����һ���ӿڣ����ڸı�m_bSearchTailFile��ֵ��by ���Ľܣ�

#ifndef	__WH_PACKAGE_FILEMAN_I_H__
#define __WH_PACKAGE_FILEMAN_I_H__

#include "wh_package_base.h"
#include "wh_package_fileman.h"
#include "whfile.h"

namespace n_whcmn
{

class	wh_package_fileman	: public whfileman
{
	// Ϊ�ӿ�ʵ�ֵ�
public:
	virtual	~wh_package_fileman();
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
	// �Լ��õ�
	// ���ذ����ļ���ԭʼ���ݣ�ѹ��������ݣ������Ľ����
	whfile *		OpenRawFile(const char *cszFName, OP_MODE_T mode);

protected:
	whfileman_package_INFO_T	m_info;
	typedef	whhash<whstr4hash, wh_package_base*, whcmnallocationobj, _whstr_hashfunc>		PATH2PACKAGE_T;
	typedef	whhash<whstr4hash, whfileman::OP_MODE_T, whcmnallocationobj, _whstr_hashfunc>	PATH2OPMODE_T;
	PATH2PACKAGE_T				m_mapPath2Package;
	PATH2OPMODE_T				m_mapPath2OPMode;
	// ������Ϊ����˵���ϸ���Ҫ�򿪵��ļ�·�����а������Բ�Ӧ�ü���ͨ��Ӳ��·������
	bool						m_bLastPathIsAboutPck;
	// ���԰Ѱ�����Ŀ¼��dir����
	class	DirKnowPackage	: public WHDirBase
	{
	private:
		whfileman	*m_pFM;
		WHDirBase	*m_pBaseDir;
		bool		m_bAutoDeleteBase;
		bool		m_bSearchTailFile;								// �Ƿ��β���ļ����ɰ��ļ�
	public:
		DirKnowPackage(whfileman *pFM, WHDirBase *pBaseDir, bool bAutoDeleteBase, bool bSearchTailFile)
			: m_pFM(pFM), m_pBaseDir(pBaseDir), m_bAutoDeleteBase(bAutoDeleteBase), m_bSearchTailFile(bSearchTailFile)
		{
		}
		~DirKnowPackage()
		{
			if( m_bAutoDeleteBase )
			{
				WHSafeSelfDestroy(m_pBaseDir);
			}
		}
		virtual	void	SelfDestroy()	{delete this;}
		virtual	void	Rewind()		{m_pBaseDir->Rewind();}
		virtual const char *	GetDir() const	{return m_pBaseDir->GetDir();}
		virtual	ENTRYINFO_T *	Read();
	};
public:
	inline whfileman_package_INFO_T	&	GetInitInfo()
	{
		return	m_info;
	}

	inline void setSearchTailFile(bool bSearchTailFile)
	{
		m_info.bSearchTailFile = bSearchTailFile;
	}
public:
	wh_package_fileman(whfileman_package_INFO_T *pInfo);
	int		SureOpenPackage(const char *cszPckPath, const char *cszExt, whfileman::OP_MODE_T nOPMode=whfileman::OP_MODE_UNDEFINED);
																	// ȷ���򿪣����ԭ��û�оʹ�������
																	// ����cszPckPath�ǲ�������׺�ģ�
																	// nOPMode�����OP_MODE_UNDEFINED��ʹ��Ĭ�ϵĴ򿪷�ʽ��
																	// ���������ʹ�����ַ�ʽ�򿪣�������Ĭ�϶���ֻ�����������д��ʽ��ĳЩ����
																	// ע�������OP_MODE_CREATE�����Զ�����µĿհ�
	wh_package_base *	LoadPackage(const char *cszPckPath, const char *cszExt, whfileman::OP_MODE_T nOPMode);
																	// �Թ̶�ģʽ����ָ�����ļ�������������ӳ��
																	// ����cszPckPath�ǲ�������׺��
	int		UnloadPackage(const char *cszPckPath);
	int		Close();												// �������л�������
	int		SetPackageOpenMode(const char *cszPCKName, whfileman::OP_MODE_T nOPMode);
																	// ����ĳ��������Ĵ�ģʽ�����������ͬ�����֣�����Ļ��ǰ��ĵ�������
	int		FlushAllPackage();										// �����еİ�flushһ��
private:
	// ֻ��Ӳ���ϲ��Ҵ�
	whfile *		Open_On_Disk(const char *cszFName, OP_MODE_T mode);
	WHDirBase *		OpenDir_On_Disk_Only(const char *cszDName);
	WHDirBase *		OpenDir_On_Disk(const char *cszDName);
	int				CreateDir_On_Disk(const char *cszDName);
	int				GetPathInfo_On_Disk(const char *cszPath, PATHINFO_T *pInfo);
	int				DelFile_On_Disk(const char *cszPath);
	int				DelDir_On_Disk(const char *cszPath);
	// ֻ�ڰ��в��Ҵ�
	whfile *		Open_In_Pck(const char *cszFName, OP_MODE_T mode);
	WHDirBase *		OpenDir_In_Pck(const char *cszDName);
	// ���ذ����ļ���ԭʼ���ݣ�ѹ��������ݣ������Ľ����
	whfile *		OpenRaw_In_Pck(const char *cszFName, OP_MODE_T mode);
	enum
	{
		CREATEDIR_IN_PCK_RST_OK					= 0,
		CREATEDIR_IN_PCK_RST_ERR_NOPCK			= -1,				// û�а�
		CREATEDIR_IN_PCK_RST_ERR_OTHER			= -10,				// �������󣨻���������
	};
	int				CreateDir_In_Pck(const char *cszDName);
	int				GetPathInfo_In_Pck(const char *cszPath, PATHINFO_T *pInfo);
	int				DelFile_In_Pck(const char *cszPath);
	int				DelDir_In_Pck(const char *cszPath);
	// ����·���ϵİ�������ҵ���*pnOffset�е���δ��ɲ�����԰���·����
	// bIsFile��ʾcszPath��ʾ����һ���ļ�������·���������Ļ������ȵ��������ļ���
	wh_package_base *	FindPackageOnPath(const char *cszPath, bool bIsFile, int *pnOffset);
	// Open��OpenDir���ڲ�ʵ�ֲ���
	whfile *		_Open(const char *cszFName, OP_MODE_T mode);
	WHDirBase *		_OpenDir(const char *cszDName);
public:
	static const char	CSZPCKEXT[];								// Ĭ�ϵİ��ļ���׺
	static const int	CNPCKEXTLEN;								// Ĭ�ϵİ��ļ���׺����
	static const char	*CSZPCKEXT_All[];							// ���п��ܵİ��ļ���׺
	static const int	CNPCKEXT_DFT_IDX;							// Ĭ�Ϻ�׺��CSZPCKEXT_All�е����
	static const char *	IsPathPackage(whfileman *pFM, const char *cszPath);	// �ж�һ��·���Ƿ��ǰ�������ǰ����򷵻غ�׺��
};

class	wh_package_fileman_Easy	: public wh_package_fileman
{
private:
	ICryptFactory	*m_pICryptFactory;
public:
	wh_package_fileman_Easy(whfileman_package_INFO_T *pInfo, ICryptFactory *pICryptFactory)
		: wh_package_fileman(pInfo)
		, m_pICryptFactory(pICryptFactory)
	{
	}
	virtual	~wh_package_fileman_Easy()
	{
		// ���ȵ��ø����Close
		wh_package_fileman::Close();
		// Ȼ���ͷ��Լ���info�ڵĶ���
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

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_FILEMAN_I_H__
