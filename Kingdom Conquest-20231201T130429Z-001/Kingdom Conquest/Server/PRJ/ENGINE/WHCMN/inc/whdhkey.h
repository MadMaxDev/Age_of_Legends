// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdhkey.h
// Creator      : Wei Hua (魏华)
// Comment      : 8bytes密码交换（使用cryptlib）
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
		DFT_KEY_LEN	= 128,											// 对应KEY_1024的key长
	};
	enum	KEYTYPE_T
	{
		KEY_1024	= 1,
		KEY_2048	= 2,
	};
	// 初始化随机种子（字串）
	static	void		RandInit(const char *cszSeed=NULL, int nLen=0);
	// 创建DHKEY对象
	static	WHDHKEY *	Create(WHDHKEY::KEYTYPE_T nKeyType=KEY_1024);
	// 虚析构
	virtual	~WHDHKEY()	{}
public:
	// 获得key的长度bytes（一般应该是128bytes）
	virtual int		GetKeyLen() const								= 0;
	// key的生成和设置
	virtual unsigned char *	MakeClientPubKey()						= 0;
	virtual const unsigned char *	GetClientPubKey() const			= 0;
	virtual void			SetClientPubKey(const unsigned char *pKey)
																	= 0;
	virtual unsigned char *	MakeServerPubKey()						= 0;
	virtual const unsigned char *	GetServerPubKey() const			= 0;
	virtual void			SetServerPubKey(const unsigned char *pKey)
																	= 0;
	// 生成AgreedValue
	virtual unsigned char *	MakeClientAgreedValue()					= 0;
	virtual unsigned char *	MakeServerAgreedValue()					= 0;
	// 为Key加上一个掩码
	virtual void			MaskAgreedValue(const unsigned char *pMask, int nSize)
																	= 0;
	// 获得AgreedValue（无论那个Make都放到一个缓冲区）
	virtual unsigned char *	GetAgreedValue()						= 0;
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHDHKEY_H__
