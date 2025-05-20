// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdes.h
// Creator      : Wei Hua (κ��)
// Comment      : DES���ܣ��������ϵ�Դ�룬����δ֪��
// CreationDate : 2003-11
// ChangeLOG    : 2004-07-22 �������ۻ�des����ģʽ
//              : 2006-01-24 WHMessageStreamDESԭ����һ��������Կ������Ϊ���ܽ��ܸ���һ�����á�
//              : ���⣬����Ҫע�⣬һ��WHMessageStreamDES������ͬʱ�������ܺͽ��ܣ�ֻ���������һ��������
//              : 2006-08-08 �ţ���������һ��Ҫע��!!!!
//              : 2007-02-02 ������reset_before_setkey();

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

	// ��¼һ��key����μ��ܣ�ֻ���8�ֽڵ����ݣ�
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
	// ��������key֮ǰ�����������һ��
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


// �򵥵ģ����䳤�ļ��ܣ������8�Ĳ��ֲ���des����ֱ�����������룩
class   WHSimpleDES	: public WHDES
{
public:
	// ���ֱ����������Ϳ�����
	unsigned char	m_key[8];
protected:
	// �����Ҫͨ����������
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

// �ۻ���DES���ܣ��ȶ���Ϣ�����ܡ�ǰһ����Ϣ�ļ��ܽ����Ӱ���һ����Ϣ�ļ��ܡ�
// ����ע�⣺һ��������ͬʱ���������������ܹ���
class   WHMessageStreamDES
{
public:
	enum
	{
		KEY_UNIT_SIZE	= 8,
		MAX_UNIT_NUM	= 64,
		MAX_KEY_SIZE	= KEY_UNIT_SIZE*MAX_UNIT_NUM,				// �����˵���֧��512�ֽڳ�����Կ��Ҳ����4096bit��
	};
private:
	// 8λ������
	int				m_nUnitNum;
	// �ܹ����ܵĸ���(��shortֻ��Ϊ�˿��Ծ���ı�)
	unsigned short	m_nCount;
	// ��Կ
	unsigned char	m_key[MAX_KEY_SIZE];
	// �ۻ������ʼֵ
	unsigned char	m_Seed0[KEY_UNIT_SIZE];
	// ʹ���е��ۻ�����
	unsigned char	m_SeedInUse[KEY_UNIT_SIZE];
	// ����������ܶ���
	WHDES			m_iDES1[MAX_UNIT_NUM];
	// ��ǰ��key�����
	int				m_nCurKeyIdx;
	// ÿ�θı�m_nCurKeyIdx�Ĳ���
	int				m_nStep;
private:
	inline void	GoCurKeyToNext()
	{
		m_nCurKeyIdx	+= m_nStep;
		// ��Ϊ��SetXXXKey�����б�֤��m_nStepһ����С��m_nUnitNum�ģ�������һ��if���ɣ�����while��
		if( m_nCurKeyIdx>=m_nUnitNum )
		{
			m_nCurKeyIdx	-= m_nUnitNum;
		}
	}
public:
	// ע�⣬nStep���������
	WHMessageStreamDES(int nStep);
	bool	SetSeed(unsigned char *pSeed0);
	// ���¿�ʼ
	void	Reset();
	// ���ü�������
	bool	SetEncryptKey(unsigned char *pKey, int nKeyLen);
	// ���ý�������
	bool	SetDecryptKey(unsigned char *pKey, int nKeyLen);
	// ����ļӽ��ܵ�pSrcData��pDstData������һ�λ�����
	// ����
	bool	Encrypt(const unsigned char *pSrcData, int nLen, unsigned char *pDstData);
	// ����
	bool	Decrypt(const unsigned char *pSrcData, int nLen, unsigned char *pDstData);
private:
	// ��������ģ��
	template<class _Act>
	bool	SetKey(unsigned char *pKey, int nKeyLen, _Act &ActSetKey);
};
// ���ټ򵥵ļ���
class   WHMessageStreamQuickCrypt
{
public:
	enum
	{
		KEY_UNIT_SIZE	= 8,
		MAX_UNIT_NUM	= 64,
		MAX_KEY_SIZE	= KEY_UNIT_SIZE*MAX_UNIT_NUM,				// �����˵���֧��512�ֽڳ�����Կ��Ҳ����4096bit��
	};
protected:
	// 8λ������
	int				m_nUnitNum;
	// �ܹ����ܵĸ���(��shortֻ��Ϊ�˿��Ծ���ı�)
	unsigned short	m_nCount;
	// ��Կ
	unsigned char	m_key[MAX_KEY_SIZE];
	// �ۻ������ʼֵ
	unsigned char	m_Seed0[KEY_UNIT_SIZE];
	// ʹ���е��ۻ�����
	unsigned char	m_SeedInUse[KEY_UNIT_SIZE];
	// ��ǰ��key�����
	unsigned char	*m_pCurKey;										// ��ǰKey����ʼλ��
	unsigned char	*m_pEndKey;										// ���ڱ�־��β��λ��
public:
	// ע�⣬nStep���������
	WHMessageStreamQuickCrypt();
	bool	SetSeed(unsigned char *pSeed0);
	// ���¿�ʼ
	void	Reset();
	// ���ü�������
	inline bool	SetEncryptKey(unsigned char *pKey, int nKeyLen)
	{
		return	SetKey(pKey, nKeyLen);
	}
	// ���ý�������
	inline bool	SetDecryptKey(unsigned char *pKey, int nKeyLen)
	{
		return	SetKey(pKey, nKeyLen);
	}
	// ����ļӽ��ܵ�pSrcData��pDstData������һ�λ�����
	// ����
	bool	Encrypt(const unsigned char *pSrcData, int nLen, unsigned char *pDstData);
	// ����
	bool	Decrypt(const unsigned char *pSrcData, int nLen, unsigned char *pDstData);
private:
	bool	SetKey(unsigned char *pKey, int nKeyLen);
private:
	inline void	GoCurKeyToNext()
	{
		m_pCurKey	+= KEY_UNIT_SIZE;
		// ��Ϊ��SetXXXKey�����б�֤��m_nStepһ����С��m_nUnitNum�ģ�������һ��if���ɣ�����while��
		if( m_pCurKey==m_pEndKey )
		{
			m_pCurKey	= m_key;
		}
	}
};


// ��һ�����ⳤ�ȵ����ݴ����һ���̵Ķ������ݴ�
// ���bAppendΪ�棬�򸽼ӵ�szTo�ϣ�����szTo�ʼ�����㣩
bool	WHDES_ConvertToFixed(const char *szFrom, int nFrom, char *szTo, int nTo, bool bAppend=false);
// ��һ�����ݴ�ͨ��һ���̵Ķ���������/����
bool	WHDES_XOR(char *szDst, int nDst, const char *szKey, int nKey);
// ���ۻ��ļ��ܽ���(AC��ʾAccumulate��szDst���������������)
bool	WHDES_XOR_AC_EN(unsigned char *szDst, int nDst, const unsigned char *szKey, int nKey);
bool	WHDES_XOR_AC_DE(unsigned char *szDst, int nDst, const unsigned char *szKey, int nKey);
// ��һ���ڴ�cszSrc�����һ��szDst
void	WHDES_MEM_XOR(unsigned char *szDst, const unsigned char *cszSrc, int nLen);

// ���ݴ�λ
void	WHDES_BYTE_PERMUTE(unsigned char *szDst, int nLen);

}		// EOF namespace n_whcmn

#endif  // EOF __WHDES_H__
