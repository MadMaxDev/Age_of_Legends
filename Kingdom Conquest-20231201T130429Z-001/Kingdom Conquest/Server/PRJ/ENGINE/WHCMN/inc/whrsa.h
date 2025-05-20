// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whrsa.h
// Creator      : Wei Hua (κ��)
// Comment      : RSA���ܡ����ܶ���
//              : 1024���ȵĹ�Կ���������ݳ��Ȳ��ܳ���86�ֽڣ����ܽ��Ϊ128�ֽڣ�Ϊ�˱��գ�һ����ܲ�Ҫ����80�ֽڣ�
// CreationDate : 2007-09-12
// ChangeLOG    : 

#ifndef __WHRSA_H__
#define __WHRSA_H__

#include <stdlib.h>

namespace n_whcmn
{

// ����˽Կ�͹�Կ
int   WHRSAGen(const void *pPass, int nPassSize, void *pKeyPri, int *pnPriLen, void *pKeyPub, int *pnPubLen, const char *cszSeed=NULL, int nKeyLen=1024);

class   WHRSA
{
protected:
	virtual ~WHRSA()	{}
public:
	virtual	void	SelfDestroy()									= 0;
	virtual int	Init(const void *pPass, int nPassSize, const void *pKey, int nKeySize, const char *cszSeed=NULL)
																	= 0;
	virtual int	InitFromKeyFile(const void *pPass, int nPassSize, const char *pszKeyFile, const char *cszSeed=NULL)
																	= 0;
	virtual int	Release()											= 0;
	virtual int	Process(const void *pSrc, int nSrcSize, void *pDst, int nDstSize)
																	= 0;
public:
	static WHRSA *	CreateEncoder();
	static WHRSA *	CreateDecoder();
};

class	WHRSACheckSum
{
protected:
	virtual ~WHRSACheckSum()	{}
public:
	virtual	void	SelfDestroy()									= 0;
	// ����һ����������rsa���ܵ�md5������������ܻ��п�300�ֽڣ�һ����1024��buffer��Ӧ��û�����⣩
	// ����0��ʾ�ɹ�
	// <0��ʾʧ��
	virtual int	Make(void *pData, int nDSize, char *pszCheckSum)	= 0;
	// У�����ݺͶ�Ӧ��md5���Ƿ�һ��
	// ����0��ʾУ��ɹ�
	// <0��ʾʧ��
	virtual int	Verify(void *pData, int nDSize, const char *pszCheckSum)
																	= 0;
public:
	// �������������Լ��ù̶���RSA��
	static WHRSACheckSum *	CreateSimple();
};

}		// EOF namespace n_whcmn

#endif  // EOF __WHRSA_H__
