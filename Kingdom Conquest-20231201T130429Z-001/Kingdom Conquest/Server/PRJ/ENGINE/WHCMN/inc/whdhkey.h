// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdhkey.h
// Creator      : Wei Hua (κ��)
// Comment      : 8bytes���뽻����ʹ��cryptlib��
// CreationDate : 2004-07-24
// ChangeLOG    : 

#ifndef	__WHDHKEY_H__
#define	__WHDHKEY_H__

#include <stdlib.h>

namespace n_whcmn
{

class	WHDHKEY
{
public:
	enum
	{
		DFT_KEY_LEN	= 128,											// ��ӦKEY_1024��key��
	};
	enum	KEYTYPE_T
	{
		KEY_1024	= 1,
		KEY_2048	= 2,
	};
	// ��ʼ��������ӣ��ִ���
	static	void		RandInit(const char *cszSeed=NULL, int nLen=0);
	// ����DHKEY����
	static	WHDHKEY *	Create(WHDHKEY::KEYTYPE_T nKeyType=KEY_1024);
	// ������
	virtual	~WHDHKEY()	{}
public:
	// ���key�ĳ���bytes��һ��Ӧ����128bytes��
	virtual int		GetKeyLen() const								= 0;
	// key�����ɺ�����
	virtual unsigned char *	MakeClientPubKey()						= 0;
	virtual const unsigned char *	GetClientPubKey() const			= 0;
	virtual void			SetClientPubKey(const unsigned char *pKey)
																	= 0;
	virtual unsigned char *	MakeServerPubKey()						= 0;
	virtual const unsigned char *	GetServerPubKey() const			= 0;
	virtual void			SetServerPubKey(const unsigned char *pKey)
																	= 0;
	// ����AgreedValue
	virtual unsigned char *	MakeClientAgreedValue()					= 0;
	virtual unsigned char *	MakeServerAgreedValue()					= 0;
	// ΪKey����һ������
	virtual void			MaskAgreedValue(const unsigned char *pMask, int nSize)
																	= 0;
	// ���AgreedValue�������Ǹ�Make���ŵ�һ����������
	virtual unsigned char *	GetAgreedValue()						= 0;
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHDHKEY_H__
