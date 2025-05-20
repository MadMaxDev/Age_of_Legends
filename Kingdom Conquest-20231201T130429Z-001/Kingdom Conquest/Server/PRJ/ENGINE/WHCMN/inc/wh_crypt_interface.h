// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : wh_crypt_interface.h
// Creator      : Wei Hua (魏华)
// Comment      : 通用加密接口定义(加密一定是不改变长度的)
//              : Create出来的加密工厂、Key交换器、加密器等需要使用SelfDestroy方法销毁。推荐使用WHSafeSelfDestroy。
// CreationDate : 2005-08-16
// Change LOG   : 2006-05-30 吕宝虹增加RSADate类，用来使用RSA加密时调用ICryptFactory::SetParam时传递公钥和私钥。

#ifndef	__WH_CRYPT_INTERFACE_H__
#define	__WH_CRYPT_INTERFACE_H__

#include <stdlib.h>

namespace n_whcmn
{

// 加密接口的定义
// 加密对象工厂
// 加密工厂在销毁的时候并不一定把所有他创建出来的加密器或者Key交换器删除。所以还是需要各个所有者自己删除这些东西。
// 一般来说，产品一旦出厂，概不退换
class	ICryptFactory
{
protected:
	virtual	~ICryptFactory()										{};
public:
	// 密钥交换器
	class	IKeyExchanger
	{
	protected:
		virtual	~IKeyExchanger()									{};
	public:
		// 自我销毁
		// 内部可能会调用他的factory的删除方法（如果必须通过factory的方法删除的话）
		virtual	void	SelfDestroy()								= 0;
		// 初始化，获得自己的private和public数据
		virtual	int		MakeOwnData()								= 0;
		// 获得public的数据长度(这个应该是固定的)
		virtual	int		GetPubSize() const							= 0;
		// 获得public数据(这是记录在内部成员变量中的)
		virtual	const void *	GetPubData()						= 0;
		// 通过对方的public数据和自己的private数据合成为key
		// 过程是这样：双方都是先MakeOwnData，A发送A-PubData，B收到后Agree1（Agree1内部可能会影响B的PubData）得到Key，然后B发送B-PubData，A收到后Agree2得到Key
		virtual	int		Agree1(const void *pPubData, int nPubSize)	= 0;
		virtual	int		Agree2(const void *pPubData, int nPubSize)	= 0;
		// 附加加密信息，这个在最终服务器客户端初始化结束后在GetKey之前附加到Key上面去
		virtual	int		AppendCryptInfo(const void *pInfo, size_t nSize)
																	= 0;
		// 获得Key内容
		virtual	const void *	GetKey()							= 0;
		// 获得Key长度(bytes)
		virtual	int		GetKeySize()								= 0;
	};
	// 具体的加密器
	class	ICryptor
	{
	protected:
		virtual	~ICryptor()											{};
	public:
		// 自我销毁
		// 内部可能会调用他的factory的删除方法（如果必须通过factory的方法删除的话）
		virtual	void	SelfDestroy()								= 0;
		// 下面两个密码设置函数中会自动Reset
		// 设置加密密码
		virtual	int		SetEncryptKey(const void *pKey, int nKeyLen)
																	= 0;
		// 设置解密密码
		virtual	int		SetDecryptKey(const void *pKey, int nKeyLen)
																	= 0;
		// 重新开始
		virtual	void	Reset()										= 0;
		// 加密/解密（pSrc和pDst可以相同）
		virtual	int		Encrypt(const void *pSrc, int nSize, void *pDst)
																	= 0;
		virtual	int		Decrypt(const void *pSrc, int nSize, void *pDst)
																	= 0;
	};
public:
	// 设置参数（不是一定要调用的）
	// nKey的定义各种factory自己设定
	virtual int			SetParam(int nKey, void *pParam, int nSize)
	{
		// 默认是不需要设置参数的，所以如果调用了也是返回错误
		return	-1;
	}
	// 随机数初始化，默认参数表示利用时间作初始化种子
	virtual void		RandInit(const char *cszSeed=NULL, int nLen=0)
																	= 0;
	// 自己销毁自己
	virtual	void		SelfDestroy()								= 0;
	// nType由应用层自己定义，如果超出规定范围则返回一个默认的对象
	// 如：CAAFS和Client之间用一种加密方式、CLS和Client之间用另一种加密方式
	// 注意Key交换器和加密器的类型可以不同，Key交换器只用于交换密钥，加密器用已经获得的密钥进行加密
	// 选择那种加密方式由服务器决定
	// 创建一个Key交换器
	virtual	IKeyExchanger *	CreateIKeyExchanger(int nType)			= 0;
	// 创建一个加密器
	virtual	ICryptor *		CreateICryptor(int nType)				= 0;
};

// 创建一个假的，不加密的加密工厂 (这个主要是给初期测试用)
ICryptFactory *	ICryptFactory_Dft_Create();

// 利用DHKey或RSA交换密钥、累计des进行加密的工厂 (一般都用这个进行加密)
ICryptFactory *	ICryptFactory_DHKeyAndDES_Create();
// DHKey交换，但是用简单的方法进行累计加密
ICryptFactory *	ICryptFactory_DHKeyAndQuickCrypt_Create();

// 生成器
enum
{
	CARDCODE_LEN				= 16,								// 卡码长度（不包括最后的'\0\结尾）
	CARDCODE_TYPE_LEN			= 4,
};
class	CardCodeGen
{
public:
	virtual ~CardCodeGen()		{}
	// 设置初始参数
	// cszType					包含产品类型码2字节+活动类型码2字节（这个长度必须保证）
	// nBatchNumber				产品批号，如果写0则使用当前相对于2008-01-01的天数作为批号
	// nStartNo					起始序号，不写就从0开始（一般这个只是在一批生成到中间停了，需要继续的时候才使用这个参数）
	virtual int	SetParam(const char *cszType, unsigned short nBatchNumber=0, unsigned int nStartNo=0)
																	= 0;
	// 设置加密密码
	virtual int	SetPass(const char *cszPass)						= 0;
	// 生成一条记录（必须保证pszBuf不得小于CARDCODE_LEN+1字节）
	// 如果成功，返回当前生成记录的序号
	// 失败返回负数（一般是因为没有可生成的序号了）
	virtual unsigned int	Gen(char *pszBuf, int nBufLen, int nNo=0)
																	= 0;;

	static CardCodeGen *	Create();
};
// 检查器
class	CardCodeCheck
{
public:
	virtual ~CardCodeCheck()	{}
	// 设置解密密码
	virtual int	SetPass(const char *cszPass)						= 0;
	// 检查记录的合法性，并解开一些关键内容
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
