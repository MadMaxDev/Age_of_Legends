// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_fileman.h
// Creator     : Wei Hua (魏华)
// Comment     : 基于包的通用文件系统
// CreationDate: 2006-08-09

#ifndef	__WH_PACKAGE_FILEMAN_H__
#define __WH_PACKAGE_FILEMAN_H__

#include "whfile_base.h"
#include "wh_compress_interface.h"
#include "wh_crypt_interface.h"
#include "wh_package_def.h"

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// 创建基于包的通用文件系统
////////////////////////////////////////////////////////////////////
struct	whfileman_package_INFO_T
{
	enum
	{
		SEARCH_ORDER_DISK_ONLY	= 0,			// 只在硬盘上查找
		SEARCH_ORDER_PCK_ONLY	= 1,			// 只在包文件中查找
		SEARCH_ORDER_DISK1_PCK2	= 2,			// 先在硬盘上找，然后再在包中找
		SEARCH_ORDER_PCK1_DISK2	= 3,			// 先在包中找，然后再在硬盘上找
	};
	unsigned char			nSearchOrder;		// 取值为前面定义的SEARCH_ORDER_XXX
	bool					bSearchTailFile;	// 是否检查tail文件中的包
	bool					bAutoCloseFM;		// 是否自动关闭pFM
	bool					bCheckFNCaseOnErr;	// 是否在出错时检查文件名的大小写（默认应该是小写）
	bool					bCheckFileBlockNumOnOpen;
												// 是否在打开文件的时候检验文件块数是否正常（因为在程序异常的时候很可能导致文件没有写入，但是文件体已经有部分更新了，所以为了能够尽可能打开文件就不用检查了）
	bool					bNoErrOnDelFile;	// 删除文件如果出错就忽略（可能会浪费一些空间）
	whfileman				*pFM;				// 底层的文件系统（也可能是包哦）
	whfileman::OP_MODE_T	nOPMode;			// 包的打开方式（有时候也是需要只读打开的，比如游戏中的时候。创建包时应该是create或更新包的时候应该是write）
												// 不论是以什么方式打开的，如果打开文件的目标在硬盘上，则文件是可以使用任何方式打开的。
	WHCompress				*pWHCompress;		// 压缩器
	ICryptFactory::ICryptor	*pEncryptor;		// 加密器
	ICryptFactory::ICryptor	*pDecryptor;		// 解密器（注意：加密器和解密器可不能一样）
	int						nBlockSize;			// 基本文件块的大小（越大对于小文件浪费越大，越小对于大文件浪费越大）
	char					szPass[128];		// 密码（如果没有密码则后的长度为0）
	int						nPassLen;			// 密码长度
	// PACKET_OP_MODE_T数组用来设置特殊包的打开模式（应该是与上面的总模式不同的）
	struct	PACKET_OP_MODE_T
	{
		const char			*pcszPck;			// 包名字（不带后缀的）
		whfileman::OP_MODE_T	nOPMode;		// 打开方式
	};
	PACKET_OP_MODE_T		*pPOPMode;			// 指向打开方式数组的指针（以pcszPck为NULL结尾）

	whfileman_package_INFO_T()
		: nSearchOrder(SEARCH_ORDER_PCK1_DISK2)
		, bSearchTailFile(false)
		, bAutoCloseFM(false)
		, bCheckFNCaseOnErr(false)
		, bCheckFileBlockNumOnOpen(false)
		, bNoErrOnDelFile(true)
		, pFM(NULL)
		, nOPMode(whfileman::OP_MODE_BIN_READONLY)
		, pWHCompress(NULL)
		, pEncryptor(NULL), pDecryptor(NULL)
		, nBlockSize(WHPACKAGE_DFT_BLOCKSIZE)
		, nPassLen(0)
		, pPOPMode(NULL)
	{
		szPass[0]	= 0;
	}
};
whfileman *	whfileman_package_Create(whfileman_package_INFO_T *pInfo);

// 参数比较简单的创建函数
struct	whfileman_package_INFO_Easy_T
{
	unsigned char			nSearchOrder;		// 取值为前面定义的whfileman_package_INFO_T::SEARCH_ORDER_XXX
	bool					bSearchTailFile;	// 是否检查tail文件中的包
	whfileman::OP_MODE_T	nOPMode;			// 包的打开方式（有时候也是需要只读打开的，比如游戏中的时候。创建包时应该是create或更新包的时候应该是write）
	int						nCryptType;			// 加密器解密器类型（大家规定好相同的数就可以了，目前没有明确定义，默认都用0也可）
	int						nBlockSize;			// 基本文件块的大小（越大对于小文件浪费越大，越小对于大文件浪费越大，所以主要看压缩之后大小在什么样的文件比较多）
	char					szPass[128];		// 密码（如果没有密码则后的长度为0）
	int						nPassLen;			// 密码长度
	whfileman_package_INFO_Easy_T()
		: nSearchOrder(whfileman_package_INFO_T::SEARCH_ORDER_PCK1_DISK2)
		, bSearchTailFile(false)
		, nOPMode(whfileman::OP_MODE_BIN_READONLY)
		, nCryptType(0)
		, nBlockSize(WHPACKAGE_DFT_BLOCKSIZE)
		, nPassLen(0)
	{
		szPass[0]	= 0;
	}
};
whfileman *	whfileman_package_Create_Easy(whfileman_package_INFO_Easy_T *pInfo);

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_FILEMAN_H__
