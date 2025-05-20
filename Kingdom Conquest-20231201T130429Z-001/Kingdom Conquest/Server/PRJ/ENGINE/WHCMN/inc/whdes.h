// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdes.h
// Creator      : Wei Hua (魏华)
// Comment      : DES加密（利用网上的源码，作者未知）
// CreationDate : 2003-11
// ChangeLOG    : 2004-07-22 增加了累积des加密模式
//              : 2006-01-24 WHMessageStreamDES原来的一个设置密钥函数改为加密解密各有一个设置。
//              : 另外，还是要注意，一个WHMessageStreamDES对象不能同时用作加密和解密，只能完成其中一个工作。
//              : 2006-08-08 嗯，上面的这个一定要注意!!!!
//              : 2007-02-02 增加了reset_before_setkey();

#ifndef __WHDES_H__
#define __WHDES_H__

#include <stdlib.h>

//#define	WHMESSAGESTREAMDES_EFFICIENT

namespace n_whcmn
{

class   WHDES
{
public:
	// Encrypt/decrypt the data in "data", according to the "key".
	// Caller is responsible for confirming the buffer size of "data"
	// points to is 8*"blocks" bytes.
	// The data encrypted/decrypted is stored in data.
	// The return code is 1:success, other:failed.

	bool	encrypt ( unsigned char key[8], unsigned char* data, int blocks = 1 );
	bool	decrypt ( unsigned char key[8], unsigned char* data, int blocks = 1 );

	// 记录一次key，多次加密（只针对8字节的数据）
	inline void	encrypt8_setkey( unsigned char key[8] )
	{
		reset_before_setkey();
		deskey( key, ENCRYPT );
	}
	inline void	encrypt8( unsigned char *data )
	{
		des_block(data, data);
	}
	inline void	decrypt8_setkey( unsigned char key[8] )
	{
		reset_before_setkey();
		deskey( key, DECRYPT );
	}
	inline void	decrypt8( unsigned char *data )
	{
		des_block(data, data);
	}

	// Encrypt/decrypt any size data,according to a special method.
	// Before calling yencrypt, copy data to a new buffer with size
	// calculated by extend.

	bool	yencrypt ( unsigned char key[8], unsigned char* data, int size );
	bool	ydecrypt ( unsigned char key[8], unsigned char* in, int blocks, int* size = 0 );
	int		extend ( int size ) { return (size/8+1)*8; };

private:
	void des(unsigned char* in, unsigned char* out, int blocks);
	void des_block(unsigned char* in, unsigned char* out);

private:
	unsigned int KnL[32];
	enum Mode { ENCRYPT, DECRYPT };
	void deskey(unsigned char key[8], Mode md);
	void usekey(unsigned int *);
	void cookey(unsigned int *);
	// 在设置新key之前最好这样设置一下
	void	reset_before_setkey();

private:
	void scrunch(unsigned char *, unsigned int *);
	void unscrun(unsigned int *, unsigned char *);
	void desfunc(unsigned int *, unsigned int *);

private:
	static const unsigned char Df_Key[24];
	static const unsigned short bytebit[8];
	static const unsigned int bigbyte[24];
	static const unsigned char pc1[56];
	static const unsigned char totrot[16];
	static const unsigned char pc2[48];
	static const unsigned int SP1[64];
	static const unsigned int SP2[64];
	static const unsigned int SP3[64];
	static const unsigned int SP4[64];
	static const unsigned int SP5[64];
	static const unsigned int SP6[64];
	static const unsigned int SP7[64];
	static const unsigned int SP8[64];
};


// 简单的，不变长的加密（最后不足8的部分不用des，而直接用密码掩码）
class   WHSimpleDES	: public WHDES
{
public:
	// 外界直接设置这个就可以了
	unsigned char	m_key[8];
protected:
	// 这个需要通过函数设置
	const char		*m_pMask;
	int				m_nMaskLen;
public:
	WHSimpleDES();
	bool	encrypt(unsigned char* data, int len);
	bool	decrypt(unsigned char* data, int len);
	inline void	setmask(const char *pMask, int nMaskLen)
	{
		m_pMask		= pMask;
		m_nMaskLen	= nMaskLen;
	}
	inline void	clrmask()
	{
		m_pMask		= NULL;
		m_nMaskLen	= 0;
	}
};

// 累积的DES加密，既对消息流加密。前一个消息的加密结果会影响后一个消息的加密。
// 不过注意：一个对象不能同时即作加密又作解密工作
class   WHMessageStreamDES
{
public:
	enum
	{
		KEY_UNIT_SIZE	= 8,
		MAX_UNIT_NUM	= 64,
		MAX_KEY_SIZE	= KEY_UNIT_SIZE*MAX_UNIT_NUM,				// 这就是说最多支持512字节长的密钥，也就是4096bit。
	};
private:
	// 8位组数量
	int				m_nUnitNum;
	// 总共加密的个数(用short只是为了可以尽快改变)
	unsigned short	m_nCount;
	// 密钥
	unsigned char	m_key[MAX_KEY_SIZE];
	// 累积掩码初始值
	unsigned char	m_Seed0[KEY_UNIT_SIZE];
	// 使用中的累积掩码
	unsigned char	m_SeedInUse[KEY_UNIT_SIZE];
	// 多个基本加密对象
	WHDES			m_iDES1[MAX_UNIT_NUM];
	// 当前的key的序号
	int				m_nCurKeyIdx;
	// 每次改变m_nCurKeyIdx的步长
	int				m_nStep;
private:
	inline void	GoCurKeyToNext()
	{
		m_nCurKeyIdx	+= m_nStep;
		// 因为在SetXXXKey函数中保证了m_nStep一定会小于m_nUnitNum的，所以用一次if即可，不用while了
		if( m_nCurKeyIdx>=m_nUnitNum )
		{
			m_nCurKeyIdx	-= m_nUnitNum;
		}
	}
public:
	// 注意，nStep最好是素数
	WHMessageStreamDES(int nStep);
	bool	SetSeed(unsigned char *pSeed0);
	// 重新开始
	void	Reset();
	// 设置加密密码
	bool	SetEncryptKey(unsigned char *pKey, int nKeyLen);
	// 设置解密密码
	bool	SetDecryptKey(unsigned char *pKey, int nKeyLen);
	// 下面的加解密的pSrcData和pDstData可以是一段缓冲区
	// 加密
	bool	Encrypt(const unsigned char *pSrcData, int nLen, unsigned char *pDstData);
	// 解密
	bool	Decrypt(const unsigned char *pSrcData, int nLen, unsigned char *pDstData);
private:
	// 设置密码模板
	template<class _Act>
	bool	SetKey(unsigned char *pKey, int nKeyLen, _Act &ActSetKey);
};
// 快速简单的加密
class   WHMessageStreamQuickCrypt
{
public:
	enum
	{
		KEY_UNIT_SIZE	= 8,
		MAX_UNIT_NUM	= 64,
		MAX_KEY_SIZE	= KEY_UNIT_SIZE*MAX_UNIT_NUM,				// 这就是说最多支持512字节长的密钥，也就是4096bit。
	};
protected:
	// 8位组数量
	int				m_nUnitNum;
	// 总共加密的个数(用short只是为了可以尽快改变)
	unsigned short	m_nCount;
	// 密钥
	unsigned char	m_key[MAX_KEY_SIZE];
	// 累积掩码初始值
	unsigned char	m_Seed0[KEY_UNIT_SIZE];
	// 使用中的累积掩码
	unsigned char	m_SeedInUse[KEY_UNIT_SIZE];
	// 当前的key的序号
	unsigned char	*m_pCurKey;										// 当前Key的起始位置
	unsigned char	*m_pEndKey;										// 用于标志结尾的位置
public:
	// 注意，nStep最好是素数
	WHMessageStreamQuickCrypt();
	bool	SetSeed(unsigned char *pSeed0);
	// 重新开始
	void	Reset();
	// 设置加密密码
	inline bool	SetEncryptKey(unsigned char *pKey, int nKeyLen)
	{
		return	SetKey(pKey, nKeyLen);
	}
	// 设置解密密码
	inline bool	SetDecryptKey(unsigned char *pKey, int nKeyLen)
	{
		return	SetKey(pKey, nKeyLen);
	}
	// 下面的加解密的pSrcData和pDstData可以是一段缓冲区
	// 加密
	bool	Encrypt(const unsigned char *pSrcData, int nLen, unsigned char *pDstData);
	// 解密
	bool	Decrypt(const unsigned char *pSrcData, int nLen, unsigned char *pDstData);
private:
	bool	SetKey(unsigned char *pKey, int nKeyLen);
private:
	inline void	GoCurKeyToNext()
	{
		m_pCurKey	+= KEY_UNIT_SIZE;
		// 因为在SetXXXKey函数中保证了m_nStep一定会小于m_nUnitNum的，所以用一次if即可，不用while了
		if( m_pCurKey==m_pEndKey )
		{
			m_pCurKey	= m_key;
		}
	}
};


// 将一个任意长度的数据串变成一个短的定长数据串
// 如果bAppend为真，则附加到szTo上（否则szTo最开始会清零）
bool	WHDES_ConvertToFixed(const char *szFrom, int nFrom, char *szTo, int nTo, bool bAppend=false);
// 将一个数据串通过一个短的定长串加密/解密
bool	WHDES_XOR(char *szDst, int nDst, const char *szKey, int nKey);
// 带累积的加密解密(AC表示Accumulate；szDst即是输入又是输出)
bool	WHDES_XOR_AC_EN(unsigned char *szDst, int nDst, const unsigned char *szKey, int nKey);
bool	WHDES_XOR_AC_DE(unsigned char *szDst, int nDst, const unsigned char *szKey, int nKey);
// 将一段内存cszSrc异或到另一段szDst
void	WHDES_MEM_XOR(unsigned char *szDst, const unsigned char *cszSrc, int nLen);

// 数据错位
void	WHDES_BYTE_PERMUTE(unsigned char *szDst, int nLen);

}		// EOF namespace n_whcmn

#endif  // EOF __WHDES_H__
