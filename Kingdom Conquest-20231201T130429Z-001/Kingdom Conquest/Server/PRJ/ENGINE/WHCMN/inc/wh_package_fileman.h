// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_fileman.h
// Creator     : Wei Hua (κ��)
// Comment     : ���ڰ���ͨ���ļ�ϵͳ
// CreationDate: 2006-08-09

#ifndef	__WH_PACKAGE_FILEMAN_H__
#define __WH_PACKAGE_FILEMAN_H__

#include "whfile_base.h"
#include "wh_compress_interface.h"
#include "wh_crypt_interface.h"
#include "wh_package_def.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// �������ڰ���ͨ���ļ�ϵͳ
////////////////////////////////////////////////////////////////////
struct	whfileman_package_INFO_T
{
	enum
	{
		SEARCH_ORDER_DISK_ONLY	= 0,			// ֻ��Ӳ���ϲ���
		SEARCH_ORDER_PCK_ONLY	= 1,			// ֻ�ڰ��ļ��в���
		SEARCH_ORDER_DISK1_PCK2	= 2,			// ����Ӳ�����ң�Ȼ�����ڰ�����
		SEARCH_ORDER_PCK1_DISK2	= 3,			// ���ڰ����ң�Ȼ������Ӳ������
	};
	unsigned char			nSearchOrder;		// ȡֵΪǰ�涨���SEARCH_ORDER_XXX
	bool					bSearchTailFile;	// �Ƿ���tail�ļ��еİ�
	bool					bAutoCloseFM;		// �Ƿ��Զ��ر�pFM
	bool					bCheckFNCaseOnErr;	// �Ƿ��ڳ���ʱ����ļ����Ĵ�Сд��Ĭ��Ӧ����Сд��
	bool					bCheckFileBlockNumOnOpen;
												// �Ƿ��ڴ��ļ���ʱ������ļ������Ƿ���������Ϊ�ڳ����쳣��ʱ��ܿ��ܵ����ļ�û��д�룬�����ļ����Ѿ��в��ָ����ˣ�����Ϊ���ܹ������ܴ��ļ��Ͳ��ü���ˣ�
	bool					bNoErrOnDelFile;	// ɾ���ļ��������ͺ��ԣ����ܻ��˷�һЩ�ռ䣩
	whfileman				*pFM;				// �ײ���ļ�ϵͳ��Ҳ�����ǰ�Ŷ��
	whfileman::OP_MODE_T	nOPMode;			// ���Ĵ򿪷�ʽ����ʱ��Ҳ����Ҫֻ���򿪵ģ�������Ϸ�е�ʱ�򡣴�����ʱӦ����create����°���ʱ��Ӧ����write��
												// ��������ʲô��ʽ�򿪵ģ�������ļ���Ŀ����Ӳ���ϣ����ļ��ǿ���ʹ���κη�ʽ�򿪵ġ�
	WHCompress				*pWHCompress;		// ѹ����
	ICryptFactory::ICryptor	*pEncryptor;		// ������
	ICryptFactory::ICryptor	*pDecryptor;		// ��������ע�⣺�������ͽ������ɲ���һ����
	int						nBlockSize;			// �����ļ���Ĵ�С��Խ�����С�ļ��˷�Խ��ԽС���ڴ��ļ��˷�Խ��
	char					szPass[128];		// ���루���û���������ĳ���Ϊ0��
	int						nPassLen;			// ���볤��
	// PACKET_OP_MODE_T������������������Ĵ�ģʽ��Ӧ�������������ģʽ��ͬ�ģ�
	struct	PACKET_OP_MODE_T
	{
		const char			*pcszPck;			// �����֣�������׺�ģ�
		whfileman::OP_MODE_T	nOPMode;		// �򿪷�ʽ
	};
	PACKET_OP_MODE_T		*pPOPMode;			// ָ��򿪷�ʽ�����ָ�루��pcszPckΪNULL��β��

	whfileman_package_INFO_T()
		: nSearchOrder(SEARCH_ORDER_PCK1_DISK2)
		, bSearchTailFile(false)
		, bAutoCloseFM(false)
		, bCheckFNCaseOnErr(false)
		, bCheckFileBlockNumOnOpen(false)
		, bNoErrOnDelFile(true)
		, pFM(NULL)
		, nOPMode(whfileman::OP_MODE_BIN_READONLY)
		, pWHCompress(NULL)
		, pEncryptor(NULL), pDecryptor(NULL)
		, nBlockSize(WHPACKAGE_DFT_BLOCKSIZE)
		, nPassLen(0)
		, pPOPMode(NULL)
	{
		szPass[0]	= 0;
	}
};
whfileman *	whfileman_package_Create(whfileman_package_INFO_T *pInfo);

// �����Ƚϼ򵥵Ĵ�������
struct	whfileman_package_INFO_Easy_T
{
	unsigned char			nSearchOrder;		// ȡֵΪǰ�涨���whfileman_package_INFO_T::SEARCH_ORDER_XXX
	bool					bSearchTailFile;	// �Ƿ���tail�ļ��еİ�
	whfileman::OP_MODE_T	nOPMode;			// ���Ĵ򿪷�ʽ����ʱ��Ҳ����Ҫֻ���򿪵ģ�������Ϸ�е�ʱ�򡣴�����ʱӦ����create����°���ʱ��Ӧ����write��
	int						nCryptType;			// ���������������ͣ���ҹ涨����ͬ�����Ϳ����ˣ�Ŀǰû����ȷ���壬Ĭ�϶���0Ҳ�ɣ�
	int						nBlockSize;			// �����ļ���Ĵ�С��Խ�����С�ļ��˷�Խ��ԽС���ڴ��ļ��˷�Խ��������Ҫ��ѹ��֮���С��ʲô�����ļ��Ƚ϶ࣩ
	char					szPass[128];		// ���루���û���������ĳ���Ϊ0��
	int						nPassLen;			// ���볤��
	whfileman_package_INFO_Easy_T()
		: nSearchOrder(whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2)
		, bSearchTailFile(false)
		, nOPMode(whfileman::OP_MODE_BIN_READONLY)
		, nCryptType(0)
		, nBlockSize(WHPACKAGE_DFT_BLOCKSIZE)
		, nPassLen(0)
	{
		szPass[0]	= 0;
	}
};
whfileman *	whfileman_package_Create_Easy(whfileman_package_INFO_Easy_T *pInfo);

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_FILEMAN_H__
