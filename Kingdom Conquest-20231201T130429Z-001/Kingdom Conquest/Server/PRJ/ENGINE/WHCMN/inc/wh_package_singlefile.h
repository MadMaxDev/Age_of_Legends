// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_singlefile.h
// Creator     : Wei Hua (魏华)
// Comment     : 单文件压缩（带加密）
// CreationDate: 2006-08-01 热烈庆祝中国人民解放军建军79周年^____^
// ChangeLOG   : 

#ifndef	__WH_PACKAGE_SINGLEFILE_H__
#define __WH_PACKAGE_SINGLEFILE_H__

#include "whcmn_def.h"
#include "whfile_base.h"
#include "wh_compress_interface.h"
#include "wh_crypt_interface.h"
#include "whvector.h"

namespace n_whcmn
{

class	wh_package_singlefile	: public whfile
{
public:
	// pBaseFile是压缩数据最终存储到的文件
	// nOPMode是底层文件也是自己的打开方式
	wh_package_singlefile();
	~wh_package_singlefile();
	// 自己用的
	struct	INFO_T 
	{
		whfileman::OP_MODE_T	nOPMode;							// 打开包的方式（有时候也是需要只读打开的，比如游戏中的时候）
		whfile					*pBaseFile;							// 最最底层的文件（也可能是一大块内存变成的内存文件）
		WHCompress				*pWHCompress;						// 压缩器
		ICryptFactory::ICryptor	*pEncryptor;						// 加密器
		ICryptFactory::ICryptor	*pDecryptor;						// 解密器
		int						nReserveSize;						// 如果是创建文件，最开始先保留一个长度，这样可以减少少量数据递增时的重新内存分配
		bool					bAutoCloseBaseFile;					// 是否自动删除基础raw文件（在上层不需要多保留一个文件指针又想删除基础文件的时候尤为有用）
		INFO_T()
			: nOPMode(whfileman::OP_MODE_UNDEFINED)
			, pBaseFile(NULL)
			, pWHCompress(NULL)
			, pEncryptor(NULL), pDecryptor(NULL)
			, nReserveSize(4096)
			, bAutoCloseBaseFile(false)
		{
		}
	};
	int		GetPathInfo(INFO_T *pInfo, whfileman::PATHINFO_T *pPathInfo);
																	// 不打开文件，只是获取文件信息
	int		Open(INFO_T *pInfo);
	int		Close();
	inline whfile *	GetBaseFile()
	{
		return	m_info.pBaseFile;
	}
	inline bool	IsReadOnly() const
	{
		return	m_info.nOPMode == whfileman::OP_MODE_BIN_READONLY;
	}
	inline const unsigned char *	GetMD5() const
	{
		return	m_hdr.MD5;
	}
	// 给whfile接口实现的
	void	SelfDestroy()
	{
		delete	this;
	}
	int		FileSize() const;
	time_t	FileTime() const;
	void	SetFileTime(time_t t);
	int		Read(void *pBuf, int nSize);
	int		Seek(int nOffset, int nOrigin);
	bool	IsEOF();
	int		Tell();
	int		Write(const void *pBuf, int nSize);
	int		Flush();
	int		GetMD5(unsigned char *MD5);
protected:
	INFO_T						m_info;
	// 文件头
	#pragma pack(1)
	struct	HDR_T
	{
		enum
		{
			VER			= 0
		};
		enum
		{
			PROP_F_COMPRESSED	= 0x01,								// 表示文件被压缩
			PROP_F_CRYPTED		= 0x02,								// 表示文件被加密
		};
		char			szMagic[15];								// 魔法串，用于标识文件类型
		unsigned char	nProp;										// 一些属性（取值见上面的PROP_XXX）
		unsigned char	MD5[16];									// 文件的MD5校验核（这样它就不用每次都重新算MD5了）
		int				nVer;										// 版本
		unsigned int	nRealSize;									// 文件长度（即没压缩前的长度）
		unsigned int	nCompressedSize;							// 文件压缩之后的长度（即存在basefile中的长度）
																	// 注：之所以记录上面两个长度是为了让压缩文件能变小并覆盖存在原来的文件上面
		time_t			nFileTime;									// 被压缩源文件的时间（实际文件的改写时间可以通过底层文件获得）

		static const char		*CSZMAGIC;

		void	reset()
		{
			WHMEMSET0THIS();
			nVer			= VER;
			strcpy(szMagic, CSZMAGIC);
		}
		bool	IsOK() const										// 判断从文件中读出的头是否正确
		{
			return	nVer == VER && strcmp(szMagic, CSZMAGIC)==0;
		}
	};
	#pragma pack()
	HDR_T						m_hdr;
	// 读取文件时的内部缓冲
	whvector<char>				m_vect;
	// 文件是否改变
	bool						m_bModified;
public:
	static bool	IsFileOK(whfile *f);								// 判断文件是否是single文件
};

// 创建一个每个文件都是单个压缩文件的文件管理器
struct	whfileman_singlefileman_INFO_T
{
	bool					bAutoCloseFM;		// 是否自动关闭pFM
	bool					bCanOpenCmnFile;	// 是否可以打开普通文件（比如说打开的不是压缩文件）
	whfileman				*pFM;				// 底层的文件系统（也可能是包哦）
	WHCompress				*pWHCompress;		// 压缩器
	ICryptFactory::ICryptor	*pEncryptor;		// 加密器
	ICryptFactory::ICryptor	*pDecryptor;		// 解密器（注意：加密器和解密器可不能一样）
	char					szPass[128];		// 密码（如果没有密码则后的长度为0）
	int						nPassLen;			// 密码长度
	whfileman_singlefileman_INFO_T()
		: bAutoCloseFM(false)
		, bCanOpenCmnFile(false)
	{
	}
};
whfileman *	whfileman_singlefileman_Create(whfileman_singlefileman_INFO_T *pInfo);

// 参数比较简单的创建函数
struct	whfileman_singlefileman_INFO_Easy_T
{
	bool					bCanOpenCmnFile;	// 是否可以打开普通文件（比如说打开的不是压缩文件）
	int						nCryptType;			// 加密器解密器类型（大家规定好相同的数就可以了，目前没有明确定义，默认都用0也可）
	char					szPass[128];		// 密码（如果没有密码则后的长度为0）
	int						nPassLen;			// 密码长度
	whfileman_singlefileman_INFO_Easy_T()
		: bCanOpenCmnFile(false)
		, nCryptType(0)
		, nPassLen(0)
	{
	}
};
whfileman *	whfileman_singlefileman_Create_Easy(whfileman_singlefileman_INFO_Easy_T *pInfo);

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_SINGLEFILE_H__
