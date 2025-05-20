// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whrsa.h
// Creator      : Wei Hua (魏华)
// Comment      : RSA加密、解密对象
//              : 1024长度的公钥，加密数据长度不能超过86字节，加密结果为128字节（为了保险，一般加密不要超过80字节）
// CreationDate : 2007-09-12
// ChangeLOG    : 

#ifndef __WHRSA_H__
#define __WHRSA_H__

#include <stdlib.h>

namespace n_whcmn
{

// 生成私钥和公钥
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
	// 根据一段数据生成rsa加密的md5串（这个串可能会有快300字节，一般用1024的buffer存应该没有问题）
	// 返回0表示成功
	// <0表示失败
	virtual int	Make(void *pData, int nDSize, char *pszCheckSum)	= 0;
	// 校验数据和对应的md5串是否一致
	// 返回0表示校验成功
	// <0表示失败
	virtual int	Verify(void *pData, int nDSize, const char *pszCheckSum)
																	= 0;
public:
	// 派生类里面是自己用固定的RSA键
	static WHRSACheckSum *	CreateSimple();
};

}		// EOF namespace n_whcmn

#endif  // EOF __WHRSA_H__
