// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whfile_fcd.h
// Creator      : Wei Hua (κ��)
// Comment      : �ļ��仯���(File Change Detect)
// CreationDate : 2008-12-04 Ϊ�˱���ͷ�ļ�Ƕ�״�whfile_util.h�з������

#ifndef	__WHFILE_FCD_H__
#define	__WHFILE_FCD_H__

#include "whfile_base.h"
#include "whhash.h"

namespace n_whcmn
{

// �ļ��޸ļ������ֻͨ���ļ�ʱ�����ж��ļ��Ƿ�ı䣩
class	whfilechangedetector
{
public:
	whfilechangedetector();
	~whfilechangedetector();
	int		SetFile(const char *szFileName);
	int		Release();
	bool	IsChanged();											// �����ļ������Ƿ�ı䣨ͬʱ��������������µ��޸�ʱ�̣����Բ�����const��׺��
private:
	char	m_szFileName[WH_MAX_PATH];
	time_t	m_nLastTime;											// �ϴ��ļ����޸�ʱ��
};

// ����ļ����޸ļ������ֻҪ��һ���޸��˾ͻ᷵��true��
class	whmultifilechangedetector
{
public:
	whmultifilechangedetector();
	~whmultifilechangedetector();
	int		AddFile(const char *szFileName, int nExt);				// �ظ����ļ������ظ����
	int		Release();
	struct	RST_T
	{
		char	*pszFileName;										// �ļ���
		int		nExt;												// ��Ӧ�ĸ��Ӳ���
	};
	whvector<RST_T> *	CheckChanged();								// �����ļ������Ƿ�ı䣨ͬʱ��������������µ��޸�ʱ�̣����Բ�����const��׺��
																	// �������Ϊ�ǿ�ָ�룬��vector�����¼�˱仯���ļ������б����ؿ�ָ���ʾû�б仯��
private:
	struct	UNIT_T
	{
		char	szFileName[WH_MAX_PATH];							// �ļ���
		int		nExt;												// ���Ӳ���
		time_t	nLastTime;											// �ϴ��ļ����޸�ʱ��
		inline void clear()
		{
			WHMEMSET0THIS();
		}
	};
	whvector<UNIT_T>	m_units;									// �ļ���Ϣ����
	whhashset<whstr4hash, whcmnallocationobj, _whstr_hashfunc>		m_setFNames;                                                                                                                                                                                                              
																	// �ļ�����
	whvector<RST_T>		m_vectRst;									// ������ļ��仯����CheckChanged�᷵����������ָ��
};

}		// EOF namespace n_whcmn

#endif	// __WHFILE_FCD_H__

