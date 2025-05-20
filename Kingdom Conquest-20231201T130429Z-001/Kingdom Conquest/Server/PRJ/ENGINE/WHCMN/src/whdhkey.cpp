// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdhkey.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 密码交换（使用了cryptlib）
// CreationDate : 2004-07-24
// ChangeLOG    : 2004-07-27 取消了Create/Agree不匹配的assert，因为在多人的网络情况下是可能发生的（比如被攻击）

#include "../inc/whdhkey.h"
#include "../inc/whtime.h"
#include <cryptlib/randpool.h>
#include <cryptlib/dh.h>
#include <cryptlib/files.h>
#include <cryptlib/hex.h>
#include <assert.h>
#include <sstream>

USING_NAMESPACE(CryptoPP)

namespace n_whcmn
{

static RandomPool &		GlobalRNG()
{
	static RandomPool randomPool;
	return randomPool;
}

static const char	DH1024STR[]	= "30818702818100DA9A18547FF03B385CC16508C173A7EF4EB61CB40EF8FEF3B31F145051676166BCDC3FE6B799FC394D08C26385F9413F896E09117E46209D6923602683CEA100924A6EE695281775C619DAA94EA8CB3691B4275B0183F1D39639EBC92995FE645D6C1BC28D409E585549BBD2C5DCDD6C208B04EADD8B7A6D997F72CBAD88390F020102";
static const char	DH2048STR[]	= "308201080282010100EB60DBD494AAFBCD2EAC6A36DB8E7DD4A2A64512A5BBB15B9BFB581C7C1CAFB647D4612973C3770C2166D75EEA695F67EA8261557591DB78BCF5A886AA5294F3AEE4D25B57C8EE8C7FE8DBF70C132CD7FFCB6F89426F807F552C5DAE2FB1F329E340094E4B30D8EF6265AB4D350E9837B151C86AC524DE4E1FC04746C668BE318275E420D51AEDDFBDF887D435CDEEF6AC81293DB45287132F8236A43AD8F4D6642D7CA6732DA06A1DE008259008C9D74403B68ADAC788CF8AB5BEFFC310DCCCD32901D1F290E5B7A993D2CF6A652AF81B6DA0FD2E70678D1AE086150E41444522F20621195AD2A1F0975652B4AF7DE5261A9FD46B9EA8B443641F3BBA695B9B020103";

class	WHDHKEY_I	: public WHDHKEY
{
private:
	enum
	{
		MAX_KEY_SIZE	= 2048/8,
	};
	DH				*m_pDH;
	int				m_nKeyLen;
	unsigned char	m_ClientPrivate[MAX_KEY_SIZE]; 
	unsigned char	m_ClientPublic[MAX_KEY_SIZE]; 
	unsigned char	m_ServerPrivate[MAX_KEY_SIZE]; 
	unsigned char	m_ServerPublic[MAX_KEY_SIZE]; 
	unsigned char	m_AgreedValue[MAX_KEY_SIZE]; 
public:
	WHDHKEY_I(KEYTYPE_T nKeyType);
	virtual	~WHDHKEY_I();
public:
	virtual int		GetKeyLen() const
	{
		return	m_nKeyLen;
	}
	virtual unsigned char *	MakeClientPubKey();
	virtual const unsigned char *	GetClientPubKey() const;
	virtual void			SetClientPubKey(const unsigned char *pKey);
	virtual unsigned char *	MakeServerPubKey();
	virtual const unsigned char *	GetServerPubKey() const;
	virtual void			SetServerPubKey(const unsigned char *pKey);
	// 生成AgreedValue
	virtual unsigned char *	MakeClientAgreedValue();
	virtual unsigned char *	MakeServerAgreedValue();
	// 为Key加上一个掩码
	virtual void			MaskAgreedValue(const unsigned char *pMask, int nSize);
	// 获得AgreedValue（无论那个Make都放到一个缓冲区）
	virtual unsigned char *	GetAgreedValue();
};

void		WHDHKEY::RandInit(const char *cszSeed, int nLen)
{
	if( cszSeed )
	{
		GlobalRNG().Put((const byte *)cszSeed, nLen);
	}
	else
	{
		char	szSeed[128];
		sprintf(szSeed, "%ld", wh_time());
		GlobalRNG().Put((const byte *)szSeed, strlen(szSeed));
	}
}

WHDHKEY *	WHDHKEY::Create(WHDHKEY::KEYTYPE_T nKeyType)
{
	return	new WHDHKEY_I(nKeyType);
}

////////////////////////////////////////////////////////////////////
// WHDHKEY_I
////////////////////////////////////////////////////////////////////
WHDHKEY_I::WHDHKEY_I(KEYTYPE_T nKeyType)
{
	const char	*pDHSTR	= DH1024STR;
	switch(nKeyType)
	{
		case	KEY_2048:
			pDHSTR		= DH2048STR;
		break;
		case	KEY_1024:
		default:
		break;
	}
	std::istringstream	ss(pDHSTR);
	FileSource		f(ss, true, new HexDecoder());
	m_pDH			= new DH(f);
	m_nKeyLen		= (int)m_pDH->AgreedValueLength();
	assert(m_nKeyLen<=MAX_KEY_SIZE);
}
WHDHKEY_I::~WHDHKEY_I()
{
	if( m_pDH )
	{
		delete	m_pDH;
		m_pDH	= NULL;
	}
}
unsigned char *	WHDHKEY_I::MakeClientPubKey()
{
	m_pDH->GenerateKeyPair(GlobalRNG(), m_ClientPrivate, m_ClientPublic);
	return	m_ClientPublic;
}
const unsigned char *	WHDHKEY_I::GetClientPubKey() const
{
	return	m_ClientPublic;
}
void			WHDHKEY_I::SetClientPubKey(const unsigned char *pKey)
{
	memcpy(m_ClientPublic, pKey, m_nKeyLen);
}
unsigned char *	WHDHKEY_I::MakeServerPubKey()
{
	m_pDH->GenerateKeyPair(GlobalRNG(), m_ServerPrivate, m_ServerPublic);
	return	m_ServerPublic;
}
const unsigned char *	WHDHKEY_I::GetServerPubKey() const
{
	return	m_ServerPublic;
}
void			WHDHKEY_I::SetServerPubKey(const unsigned char *pKey)
{
	memcpy(m_ServerPublic, pKey, m_nKeyLen);
}
unsigned char *	WHDHKEY_I::MakeClientAgreedValue()
{
	if( !m_pDH->Agree(m_AgreedValue, m_ClientPrivate, m_ServerPublic) )
	{
		return	NULL;
	}
	return	m_AgreedValue;
}
unsigned char *	WHDHKEY_I::MakeServerAgreedValue()
{
	if( !m_pDH->Agree(m_AgreedValue, m_ServerPrivate, m_ClientPublic) )
	{
		return	NULL;
	}
	return	m_AgreedValue;
}
void	WHDHKEY_I::MaskAgreedValue(const unsigned char *pMask, int nSize)
{
	int	mi	= 0;
	for(int i=0;i<m_nKeyLen;i++)
	{
		m_AgreedValue[i]	^= pMask[mi];
		mi	++;
		if( mi>=nSize )
		{
			mi	= 0;
		}
	}
}
unsigned char *	WHDHKEY_I::GetAgreedValue()
{
	return	m_AgreedValue;
}

}	// EOF namespace n_whcmn
