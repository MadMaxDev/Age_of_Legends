// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : wh_crypt_interface.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 通用加密接口默认实现
// CreationDate : 2005-08-17
// Change LOG   : 2006-05-22 修改了IKeyExchanger_X没有使用m_pHost中参数DHSTR参数的问题。所以原来的SetParam是没有用的。

#include "../inc/wh_crypt_interface.h"
#include "../inc/whstring.h"
#include "../inc/whdes.h"
#include "../inc/whtime.h"
#include <cryptlib/randpool.h>
#include <cryptlib/dh.h>
#include <cryptlib/files.h>
#include <cryptlib/hex.h>
#include <cryptlib/base64.h>
#include <sstream>
USING_NAMESPACE(CryptoPP)

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 默认的加密工厂
////////////////////////////////////////////////////////////////////
class	ICryptFactory_Dft	: public	ICryptFactory
{
public:
	enum
	{
		XDATASIZE	= 32,
	};
private:
	class	IKeyExchanger_Dft0	: public	ICryptFactory::IKeyExchanger
	{
	public:
		// 自我销毁
		virtual	void	SelfDestroy()
		{
			// 什么也不做
			// 外界调用这个是为了删除自己，但是这个对象是ICryptFactory_Dft的一个成员对象，不用外界删除了
		}
		// 客户端先初始化，得到的数据发送给服务器
		virtual	int		MakeOwnData()
		{
			return		0;
		}
		virtual	int		GetPubSize() const
		{
			return		0;
		}
		virtual	const void *	GetPubData()
		{
			return	NULL;
		}
		// 服务器使用客户端的数据再初始化，得到的数据发送给客户端
		virtual	int		Agree1(const void *pPubData, int nPubSize)
		{
			return		0;
		}
		virtual	int		Agree2(const void *pPubData, int nPubSize)
		{
			return		0;
		}
		// 附加加密信息
		virtual	int		AppendCryptInfo(const void *pInfo, size_t nSize)
		{
			return		0;
		}
		virtual	const void *	GetKey()
		{
			return		0;
		}
		virtual	int		GetKeySize()
		{
			return		0;
		}
	};
	class	IKeyExchanger_Dft1	: public	IKeyExchanger_Dft0
	{
	public:
		static const	char 	s_szPubData[XDATASIZE];
		static const	char 	s_szKey[XDATASIZE];
		char	m_szPub[XDATASIZE];
	public:
		virtual	int		MakeOwnData()
		{
			strcpy(m_szPub, s_szPubData);
			return		0;
		}
		virtual	int		GetPubSize() const
		{
			return	sizeof(m_szPub);
		}
		virtual	const void *	GetPubData()
		{
			return	m_szPub;
		}
		// 服务器使用客户端的数据再初始化，得到的数据发送给客户端
		int		_Agree(const void *pPubData, int nPubSize)
		{
			if( strcmp((const char *)pPubData, s_szPubData)!=0 )
			{
				return	-1;
			}
			return		0;
		}
		virtual	int		Agree1(const void *pPubData, int nPubSize)
		{
			return	_Agree(pPubData, nPubSize);
		}
		virtual	int		Agree2(const void *pPubData, int nPubSize)
		{
			return	_Agree(pPubData, nPubSize);
		}
		virtual	const void *	GetKey()
		{
			return		s_szKey;
		}
		virtual	int		GetKeySize()
		{
			return		strlen(s_szKey);
		}
	};
	class	ICryptor_Dft0	: public	ICryptFactory::ICryptor
	{
	public:
		// 自我销毁
		virtual	void	SelfDestroy()
		{
			// 什么也不做
			// 外界调用这个是为了删除自己，但是这个对象是ICryptFactory_Dft的一个成员对象，不用外界删除了
		}
		virtual	int		SetEncryptKey(const void *pKey, int nKeyLen)
		{
			return		0;
		}
		virtual	int		SetDecryptKey(const void *pKey, int nKeyLen)
		{
			return		0;
		}
		virtual	void	Reset()
		{
		}
		virtual	int		Encrypt(const void *pSrc, int nSize, void *pDst)
		{
			if( pDst != pSrc )
			{
				memcpy(pDst, pSrc, nSize);
			}
			return		0;
		}
		virtual	int		Decrypt(const void *pSrc, int nSize, void *pDst)
		{
			if( pDst != pSrc )
			{
				memcpy(pDst, pSrc, nSize);
			}
			return		0;
		}
	};
	class	ICryptor_Dft1	: public	ICryptor_Dft0
	{
	private:
		char	s_szKey[XDATASIZE];
		int		s_nSize;
	public:
		virtual	int		SetEncryptKey(const void *pKey, int nKeyLen)
		{
			memcpy(s_szKey, pKey, nKeyLen);
			s_nSize		= nKeyLen;
			return		0;
		}
		virtual	int		SetDecryptKey(const void *pKey, int nKeyLen)
		{
			memcpy(s_szKey, pKey, nKeyLen);
			s_nSize		= nKeyLen;
			return		0;
		}
		virtual	void	Reset()
		{
		}
		virtual	int		Encrypt(const void *pSrc, int nSize, void *pDst)
		{
			if( pDst != pSrc )
			{
				memcpy(pDst, pSrc, nSize);
			}
			WHDES_XOR((char *)pDst, nSize, s_szKey, s_nSize);
			return		0;
		}
		virtual	int		Decrypt(const void *pSrc, int nSize, void *pDst)
		{
			return		Encrypt(pSrc, nSize, pDst);
		}
	};
private:
	IKeyExchanger_Dft0	m_ikeyexchanger_dft0;
	IKeyExchanger_Dft1	m_ikeyexchanger_dft1;
	ICryptor_Dft0		m_icryptor_dft0;
	ICryptor_Dft1		m_icryptor_dft1;
public:
	// 随机数初始化，默认参数表示利用时间作初始化种子
	virtual void		RandInit(const char *cszSeed, int nLen)
	{
		// 这里不用做什么，因为是假的，呵呵。
	}
	// 自己销毁自己
	virtual	void		SelfDestroy()
	{
		delete			this;
	}
	// 创建一个加密器
	virtual	IKeyExchanger *	CreateIKeyExchanger(int nType)
	{
		nType	%= 2;
		switch( nType )
		{
			case	1:
				return	&m_ikeyexchanger_dft1;
			case	0:
			default:
				return	&m_ikeyexchanger_dft0;
		}
	}
	// 创建一个加密器
	virtual	ICryptor *	CreateICryptor(int nType)
	{
		nType	%= 2;
		switch( nType )
		{
			case	1:
				return	&m_icryptor_dft1;
			case	0:
			default:
				return	&m_icryptor_dft0;
		}
	}
};
const	char 	ICryptFactory_Dft::IKeyExchanger_Dft1::s_szPubData[ICryptFactory_Dft::XDATASIZE]	= "IKeyExchanger_Dft1: PubData";
const	char 	ICryptFactory_Dft::IKeyExchanger_Dft1::s_szKey[ICryptFactory_Dft::XDATASIZE]		= "key123456";

ICryptFactory *	ICryptFactory_Dft_Create()
{
	return	new	ICryptFactory_Dft;
}

////////////////////////////////////////////////////////////////////
// 利用DHKey交换密钥、累计des进行加密的工厂 (一般都用这个进行加密)
////////////////////////////////////////////////////////////////////
// 两种长度的初始内容

static const char	DH1024STR[]	= "30818702818100DA9A18547FF03B385CC16508C173A7EF4EB61CB40EF8FEF3B31F145051676166BCDC3FE6B799FC394D08C26385F9413F896E09117E46209D6923602683CEA100924A6EE695281775C619DAA94EA8CB3691B4275B0183F1D39639EBC92995FE645D6C1BC28D409E585549BBD2C5DCDD6C208B04EADD8B7A6D997F72CBAD88390F020102";	// 原始
static const char	DH2048STR[]	= "308201080282010100EB60DBD494AAFBCD2EAC6A36DB8E7DD4A2A64512A5BBB15B9BFB581C7C1CAFB647D4612973C3770C2166D75EEA695F67EA8261557591DB78BCF5A886AA5294F3AEE4D25B57C8EE8C7FE8DBF70C132CD7FFCB6F89426F807F552C5DAE2FB1F329E340094E4B30D8EF6265AB4D350E9837B151C86AC524DE4E1FC04746C668BE318275E420D51AEDDFBDF887D435CDEEF6AC81293DB45287132F8236A43AD8F4D6642D7CA6732DA06A1DE008259008C9D74403B68ADAC788CF8AB5BEFFC310DCCCD32901D1F290E5B7A993D2CF6A652AF81B6DA0FD2E70678D1AE086150E41444522F20621195AD2A1F0975652B4AF7DE5261A9FD46B9EA8B443641F3BBA695B9B020103";	// 原始
//static const char	DH1024STR[]	= "3081870281810093D63E6D235E95CFB08F46C019EF81B0FB744DAA3BF67849FEE9093A0D3B041C8C1DE764A272E2AE4C0D26ADE3337009E9D25964E4D8F7D6103CD03EE4C2E277BAADFB7A8311C64C73DCF108507233ADCE83F39585940575BB2DEEEC6CE8246ECD3CAF917385D3D4E026BC3A2FB27D908F947B6CBE5D8B1296356235CAD88613020102";
//static const char	DH2048STR[]	= "308201080282010100E2E2BAF1A726AB89FACA2B9FBA112A12B753A196D1BE7CE74282B47D7BF8342D462A217EF1E772653BE92EB5AC8922510D66780E78BA53FCFDC3518C482AD7548FFA71D6BA3A38872CAD132244739CD4A59CA2B16B75E884250ABD214D4A3C615BD4755E286F76205FD08187B9AA21E1E4243AFFD03E67542E7310F926D471996A58DA2AD244F331014315BA592F87A0897A5EFB3CD8F5571C2A350D3DB75FCC60F55CF5A30D90905C20BF674E59CA0B2CB8682B8DCDDA59CF817FE446EC15F8305F57CBC7B625B11200D6753967DE261AD8D6F7838862F16669F91BAF1C25AC9E2FB716D7437706B3253BB98F4EF039D266786B1CDCBD75AAA717DED347D93B020102";
static const int	CNDHSTRNUM	= 2;
static const char	*g_pDHSTR[CNDHSTRNUM]	=
{
	DH1024STR,
	DH2048STR,
};

// 
//static const char	DH1024STR[]	= "3081870281810093D63E6D235E95CFB08F46C019EF81B0FB744DAA3BF67849FEE9093A0D3B041C8C1DE764A272E2AE4C0D26ADE3337009E9D25964E4D8F7D6103CD03EE4C2E277BAADFB7A8311C64C73DCF108507233ADCE83F39585940575BB2DEEEC6CE8246ECD3CAF917385D3D4E026BC3A2FB27D908F947B6CBE5D8B1296356235CAD88613020102";
//static const char	DH2048STR[]	= "308201080282010100E2E2BAF1A726AB89FACA2B9FBA112A12B753A196D1BE7CE74282B47D7BF8342D462A217EF1E772653BE92EB5AC8922510D66780E78BA53FCFDC3518C482AD7548FFA71D6BA3A38872CAD132244739CD4A59CA2B16B75E884250ABD214D4A3C615BD4755E286F76205FD08187B9AA21E1E4243AFFD03E67542E7310F926D471996A58DA2AD244F331014315BA592F87A0897A5EFB3CD8F5571C2A350D3DB75FCC60F55CF5A30D90905C20BF674E59CA0B2CB8682B8DCDDA59CF817FE446EC15F8305F57CBC7B625B11200D6753967DE261AD8D6F7838862F16669F91BAF1C25AC9E2FB716D7437706B3253BB98F4EF039D266786B1CDCBD75AAA717DED347D93B020102";

/*
static const char	DH1024STR[]	= "30818702818100DA9A18547FF03B385CC16508C173A7EF4EB61CB40EF8FEF3B31F145051676166BCDC3FE6B799FC394D08C26385F9413F896E09117E46209D6923602683CEA100924A6EE695281775C619DAA94EA8CB3691B4275B0183F1D39639EBC92995FE645D6C1BC28D409E585549BBD2C5DCDD6C208B04EADD8B7A6D997F72CBAD88390F020102";	// 原始
static const char	DH2048STR[]	= "308201080282010100EB60DBD494AAFBCD2EAC6A36DB8E7DD4A2A64512A5BBB15B9BFB581C7C1CAFB647D4612973C3770C2166D75EEA695F67EA8261557591DB78BCF5A886AA5294F3AEE4D25B57C8EE8C7FE8DBF70C132CD7FFCB6F89426F807F552C5DAE2FB1F329E340094E4B30D8EF6265AB4D350E9837B151C86AC524DE4E1FC04746C668BE318275E420D51AEDDFBDF887D435CDEEF6AC81293DB45287132F8236A43AD8F4D6642D7CA6732DA06A1DE008259008C9D74403B68ADAC788CF8AB5BEFFC310DCCCD32901D1F290E5B7A993D2CF6A652AF81B6DA0FD2E70678D1AE086150E41444522F20621195AD2A1F0975652B4AF7DE5261A9FD46B9EA8B443641F3BBA695B9B020103";	// 原始

static const char	DH1024STR[]	=
	"MIGHAoGBAJPWPm0jXpXPsI9GwBnvgbD7dE2qO/Z4Sf7pCToNOwQcjB3nZKJy4q5M\n"
	"DSat4zNwCenSWWTk2PfWEDzQPuTC4ne6rft6gxHGTHPc8QhQcjOtzoPzlYWUBXW7\n"
	"Le7sbOgkbs08r5FzhdPU4Ca8Oi+yfZCPlHtsvl2LEpY1YjXK2IYTAgEC"
	;
static const char	DH2048STR[]	=
	"MIIBCAKCAQEA4uK68acmq4n6yiufuhEqErdToZbRvnznQoK0fXv4NC1GKiF+8edy\n"
	"ZTvpLrWsiSJRDWZ4Dni6U/z9w1GMSCrXVI/6cda6OjiHLK0TIkRznNSlnKKxa3Xo\n"
	"hCUKvSFNSjxhW9R1XihvdiBf0IGHuaoh4eQkOv/QPmdULnMQ+SbUcZlqWNoq0kTz\n"
	"MQFDFbpZL4egiXpe+zzY9VccKjUNPbdfzGD1XPWjDZCQXCC/Z05ZygssuGgrjc3a\n"
	"Wc+Bf+RG7BX4MF9Xy8e2JbESANZ1OWfeJhrY1veDiGLxZmn5G68cJayeL7cW10N3\n"
	"BrMlO7mPTvA50mZ4axzcvXWqpxfe00fZOwIBAg=="
	;
*/

class	ICryptFactory_DHKeyAndDES	: public	ICryptFactory
{
private:
	class	IKeyExchanger_X		: public	ICryptFactory::IKeyExchanger
	{
	protected:
		enum
		{
			MAX_KEY_SIZE	= 2048/8,
		};
		RandomPool			&m_RP;
		DH					*m_pDH;
		int					m_nKeyLen;
		unsigned char		m_Private[MAX_KEY_SIZE]; 
		unsigned char		m_Public[MAX_KEY_SIZE]; 
		unsigned char		m_AgreedValue[MAX_KEY_SIZE]; 
		ICryptFactory_DHKeyAndDES	*m_pHost;
	public:
		IKeyExchanger_X(ICryptFactory_DHKeyAndDES *pHost, int nType, RandomPool &RP)
		: m_RP(RP)
		, m_pHost(pHost)
		{
			nType	%= m_pHost->m_nDHSTRNUM;
			const char	*pDHSTR	= m_pHost->m_pDHSTR[nType];
			std::istringstream	ss(pDHSTR);
			//FileSource		f(ss, true, new Base64Decoder());
			FileSource		f(ss, true, new HexDecoder());
			m_pDH			= new DH(f);
			m_nKeyLen		= (int)m_pDH->AgreedValueLength();
			assert(m_nKeyLen<=MAX_KEY_SIZE);
		}
		~IKeyExchanger_X()
		{
			delete	m_pDH;
		}
		virtual	void	SelfDestroy()
		{
			delete	this;
		}
		virtual	int		MakeOwnData()
		{
			m_pDH->GenerateKeyPair(m_RP, m_Private, m_Public);
			return	0;
		}
		virtual	int		GetPubSize() const
		{
			return	m_nKeyLen;
		}
		virtual	const void *	GetPubData()
		{
			return	m_Public;
		}
		virtual	int		_Agree(const void *pPubData, int nPubSize)
		{
			if( nPubSize != m_nKeyLen )
			{
				return	-1;
			}
			if( !m_pDH->Agree(m_AgreedValue, m_Private, (const byte *)pPubData) )
			{
				return	-2;
			}
			return	0;
		}
		virtual	int		Agree1(const void *pPubData, int nPubSize)
		{
			return	_Agree(pPubData, nPubSize);
		}
		virtual	int		Agree2(const void *pPubData, int nPubSize)
		{
			return	_Agree(pPubData, nPubSize);
		}
		virtual	int		AppendCryptInfo(const void *pInfo, size_t nSize)
		{
			// 如果pInfo过长那么后面的就不用了
			int	mi	= 0;
			unsigned char	*pMask	= (unsigned char *)pInfo;
			for(int i=0;i<m_nKeyLen;i++)
			{
				m_AgreedValue[i]	^= pMask[mi];
				mi	++;
				if( mi>=(int)nSize )
				{
					mi	= 0;
				}
			}
			return	0;
		}
		virtual	const void *	GetKey()
		{
			return	m_AgreedValue;
		}
		virtual	int		GetKeySize()
		{
			return	m_nKeyLen;
		}
	};
	class	ICryptor_X	: public	ICryptFactory::ICryptor
	{
	private:
		WHMessageStreamDES	m_des;
	public:
		ICryptor_X(int nType)
		: m_des(nType)
		{
		}
		virtual	void	SelfDestroy()
		{
			delete	this;
		}
		virtual	int		SetEncryptKey(const void *pKey, int nKeyLen)
		{
			Reset();
			return		m_des.SetEncryptKey((unsigned char *)pKey, nKeyLen);
		}
		virtual	int		SetDecryptKey(const void *pKey, int nKeyLen)
		{
			Reset();
			return		m_des.SetDecryptKey((unsigned char *)pKey, nKeyLen);
		}
		virtual	void	Reset()
		{
			m_des.Reset();
		}
		virtual	int		Encrypt(const void *pSrc, int nSize, void *pDst)
		{
			return		m_des.Encrypt((const unsigned char *)pSrc, nSize, (unsigned char *)pDst);
		}
		virtual	int		Decrypt(const void *pSrc, int nSize, void *pDst)
		{
			return		m_des.Decrypt((const unsigned char *)pSrc, nSize, (unsigned char *)pDst);
		}
	};
private:
	friend class		ICryptFactory_DHKeyAndDES::IKeyExchanger_X;	// 'friend' cannot be used during type definition
	RandomPool			m_RP;
	const char			**m_pDHSTR;
	int					m_nDHSTRNUM;
public:
	ICryptFactory_DHKeyAndDES()
	{
		m_pDHSTR		= g_pDHSTR;
		m_nDHSTRNUM		= CNDHSTRNUM;
	}
	~ICryptFactory_DHKeyAndDES()
	{
	}
	virtual int			SetParam(int nKey, void *pParam, int nSize)
	{
		switch( nKey )
		{
		case	1:
			{
				// pParam必须指向一个在整个程序生存周期内都存在的指针数组
				// nSize就临时用来表示数组中元素的个数
				m_pDHSTR	= (const char **)pParam;
				m_nDHSTRNUM	= nSize;
			}
			break;
		default:
			break;
		}
		return	0;
	}
	// 随机数初始化，默认参数表示利用时间作初始化种子
	virtual void		RandInit(const char *cszSeed, int nLen)
	{
		if( cszSeed )
		{
			m_RP.Put((const byte *)cszSeed, nLen);
		}
		else
		{
			char	szSeed[64];
			sprintf(szSeed, "%ld%u", time(NULL), wh_gettickcount());
			m_RP.Put((const byte *)szSeed, strlen(szSeed));
		}
	}
	// 自己销毁自己
	virtual	void		SelfDestroy()
	{
		delete			this;
	}
	// 创建一个加密器
	virtual	IKeyExchanger *	CreateIKeyExchanger(int nType)
	{
		return	new IKeyExchanger_X(this, nType, m_RP);
	}
	// 创建一个加密器
	virtual	ICryptor *	CreateICryptor(int nType)
	{
		enum
		{
			NUM	= 5,
		};
		static const	int	CNT[NUM]	= {1,3,5,7,11};
		nType	%= NUM;
		return	new ICryptor_X(CNT[nType]);
	}
};

ICryptFactory *	ICryptFactory_DHKeyAndDES_Create()
{
	return	new	ICryptFactory_DHKeyAndDES;
}

}		// EOF namespace n_whcmn
