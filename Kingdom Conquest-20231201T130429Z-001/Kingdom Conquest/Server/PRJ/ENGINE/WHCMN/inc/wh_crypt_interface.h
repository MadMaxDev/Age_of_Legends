// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : wh_crypt_interface.h
// Creator      : Wei Hua (κ��)
// Comment      : ͨ�ü��ܽӿڶ���(����һ���ǲ��ı䳤�ȵ�)
//              : Create�����ļ��ܹ�����Key������������������Ҫʹ��SelfDestroy�������١��Ƽ�ʹ��WHSafeSelfDestroy��
// CreationDate : 2005-08-16
// Change LOG   : 2006-05-30 ����������RSADate�࣬����ʹ��RSA����ʱ����ICryptFactory::SetParamʱ���ݹ�Կ��˽Կ��

#ifndef	__WH_CRYPT_INTERFACE_H__
#define	__WH_CRYPT_INTERFACE_H__

#include <stdlib.h>

namespace n_whcmn
{

// ���ܽӿڵĶ���
// ���ܶ��󹤳�
// ���ܹ��������ٵ�ʱ�򲢲�һ�������������������ļ���������Key������ɾ�������Ի�����Ҫ�����������Լ�ɾ����Щ������
// һ����˵����Ʒһ���������Ų��˻�
class	ICryptFactory
{
protected:
	virtual	~ICryptFactory()										{};
public:
	// ��Կ������
	class	IKeyExchanger
	{
	protected:
		virtual	~IKeyExchanger()									{};
	public:
		// ��������
		// �ڲ����ܻ��������factory��ɾ���������������ͨ��factory�ķ���ɾ���Ļ���
		virtual	void	SelfDestroy()								= 0;
		// ��ʼ��������Լ���private��public����
		virtual	int		MakeOwnData()								= 0;
		// ���public�����ݳ���(���Ӧ���ǹ̶���)
		virtual	int		GetPubSize() const							= 0;
		// ���public����(���Ǽ�¼���ڲ���Ա�����е�)
		virtual	const void *	GetPubData()						= 0;
		// ͨ���Է���public���ݺ��Լ���private���ݺϳ�Ϊkey
		// ������������˫��������MakeOwnData��A����A-PubData��B�յ���Agree1��Agree1�ڲ����ܻ�Ӱ��B��PubData���õ�Key��Ȼ��B����B-PubData��A�յ���Agree2�õ�Key
		virtual	int		Agree1(const void *pPubData, int nPubSize)	= 0;
		virtual	int		Agree2(const void *pPubData, int nPubSize)	= 0;
		// ���Ӽ�����Ϣ����������շ������ͻ��˳�ʼ����������GetKey֮ǰ���ӵ�Key����ȥ
		virtual	int		AppendCryptInfo(const void *pInfo, size_t nSize)
																	= 0;
		// ���Key����
		virtual	const void *	GetKey()							= 0;
		// ���Key����(bytes)
		virtual	int		GetKeySize()								= 0;
	};
	// ����ļ�����
	class	ICryptor
	{
	protected:
		virtual	~ICryptor()											{};
	public:
		// ��������
		// �ڲ����ܻ��������factory��ɾ���������������ͨ��factory�ķ���ɾ���Ļ���
		virtual	void	SelfDestroy()								= 0;
		// ���������������ú����л��Զ�Reset
		// ���ü�������
		virtual	int		SetEncryptKey(const void *pKey, int nKeyLen)
																	= 0;
		// ���ý�������
		virtual	int		SetDecryptKey(const void *pKey, int nKeyLen)
																	= 0;
		// ���¿�ʼ
		virtual	void	Reset()										= 0;
		// ����/���ܣ�pSrc��pDst������ͬ��
		virtual	int		Encrypt(const void *pSrc, int nSize, void *pDst)
																	= 0;
		virtual	int		Decrypt(const void *pSrc, int nSize, void *pDst)
																	= 0;
	};
public:
	// ���ò���������һ��Ҫ���õģ�
	// nKey�Ķ������factory�Լ��趨
	virtual int			SetParam(int nKey, void *pParam, int nSize)
	{
		// Ĭ���ǲ���Ҫ���ò����ģ��������������Ҳ�Ƿ��ش���
		return	-1;
	}
	// �������ʼ����Ĭ�ϲ�����ʾ����ʱ������ʼ������
	virtual void		RandInit(const char *cszSeed=NULL, int nLen=0)
																	= 0;
	// �Լ������Լ�
	virtual	void		SelfDestroy()								= 0;
	// nType��Ӧ�ò��Լ����壬��������涨��Χ�򷵻�һ��Ĭ�ϵĶ���
	// �磺CAAFS��Client֮����һ�ּ��ܷ�ʽ��CLS��Client֮������һ�ּ��ܷ�ʽ
	// ע��Key�������ͼ����������Ϳ��Բ�ͬ��Key������ֻ���ڽ�����Կ�����������Ѿ���õ���Կ���м���
	// ѡ�����ּ��ܷ�ʽ�ɷ���������
	// ����һ��Key������
	virtual	IKeyExchanger *	CreateIKeyExchanger(int nType)			= 0;
	// ����һ��������
	virtual	ICryptor *		CreateICryptor(int nType)				= 0;
};

// ����һ���ٵģ������ܵļ��ܹ��� (�����Ҫ�Ǹ����ڲ�����)
ICryptFactory *	ICryptFactory_Dft_Create();

// ����DHKey��RSA������Կ���ۼ�des���м��ܵĹ��� (һ�㶼��������м���)
ICryptFactory *	ICryptFactory_DHKeyAndDES_Create();
// DHKey�����������ü򵥵ķ��������ۼƼ���
ICryptFactory *	ICryptFactory_DHKeyAndQuickCrypt_Create();

// ������
enum
{
	CARDCODE_LEN				= 16,								// ���볤�ȣ�����������'\0\��β��
	CARDCODE_TYPE_LEN			= 4,
};
class	CardCodeGen
{
public:
	virtual ~CardCodeGen()		{}
	// ���ó�ʼ����
	// cszType					������Ʒ������2�ֽ�+�������2�ֽڣ�������ȱ��뱣֤��
	// nBatchNumber				��Ʒ���ţ����д0��ʹ�õ�ǰ�����2008-01-01��������Ϊ����
	// nStartNo					��ʼ��ţ���д�ʹ�0��ʼ��һ�����ֻ����һ�����ɵ��м�ͣ�ˣ���Ҫ������ʱ���ʹ�����������
	virtual int	SetParam(const char *cszType, unsigned short nBatchNumber=0, unsigned int nStartNo=0)
																	= 0;
	// ���ü�������
	virtual int	SetPass(const char *cszPass)						= 0;
	// ����һ����¼�����뱣֤pszBuf����С��CARDCODE_LEN+1�ֽڣ�
	// ����ɹ������ص�ǰ���ɼ�¼�����
	// ʧ�ܷ��ظ�����һ������Ϊû�п����ɵ�����ˣ�
	virtual unsigned int	Gen(char *pszBuf, int nBufLen, int nNo=0)
																	= 0;;

	static CardCodeGen *	Create();
};
// �����
class	CardCodeCheck
{
public:
	virtual ~CardCodeCheck()	{}
	// ���ý�������
	virtual int	SetPass(const char *cszPass)						= 0;
	// ����¼�ĺϷ��ԣ����⿪һЩ�ؼ�����
	struct	INFO_T
	{
		char			szType[CARDCODE_TYPE_LEN+1];
		unsigned short	nBatchNumber;
		unsigned int	nNo;
	};
	virtual int	Check(const char *pszBuf, INFO_T *pInfo)			= 0;

	static CardCodeCheck *	Create();
};

}		// EOF namespace n_whcmn

#endif	// EOF __WH_CRYPT_INTERFACE_H__
