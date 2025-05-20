// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_base.cpp
// Creator     : Wei Hua (魏华)
// Comment     : 基础的包结构
// CreationDate: 2006-07-18
// ChangeLOG   : 2006-07-31 取消了一些操作上可能出现的assert，因为如果随便assert可能导致包文件重要更改信息没有保存。
//             : 2007-04-02 修正了覆盖写一个已经存在的文件导致文件损坏的问题。原因：raw文件在原来基础上写入小长度（小于一个block），会在当前页没有写满的情况下跳到下一页去写。导致原来的文件内容被留在文件头部。
//			   : 2008-06-06	增加一个接口，用于直接读取原始的压缩数据（by 钟文杰）
//			   : 2008-06-06	修正错误：rawfile::Read中，如果读取结束时文件指针刚好处于某一块末尾，块指针没有正确移动（by 钟文杰）

#include "../inc/wh_package_base.h"
#include "../inc/whmd5.h"
#include "../inc/whfile.h"
#include "../inc/whtime.h"

using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// wh_package_base::rawfile
////////////////////////////////////////////////////////////////////
wh_package_base::rawfile::rawfile(wh_package_base	*pPB)
: m_pPB(pPB)
, m_nOPMode(whfileman::OP_MODE_UNDEFINED)
, m_bShouldSaveFirstBlockExt(false)
, m_nStart(0)
, m_nCurBlock(0), m_nCurOffset(0)
{
}
wh_package_base::rawfile::~rawfile()
{
	Close();
}
int		wh_package_base::rawfile::Open(whfileman::OP_MODE_T nOPMode, blockoffset_t &nStart)
{
	// 记录打开模式
	m_nOPMode	= nOPMode;
	if( !IsReadOnly() && m_pPB->IsReadOnly() )
	{
		// 如果包不是写打开的则不能进行写打开
		return	-1;
	}
	// 判断打开模式
	if( m_nOPMode == whfileman::OP_MODE_BIN_CREATE )
	{
		// 空文件最开始虽然是什么也没有的，但是也需要一个起始块（因为外界可能只是先创建文件，然后把起始块记录在文件信息表中）
		nStart	= m_pPB->AllocBlock();
		if( nStart==0 )
		{
			return	-2;
		}
		// 写入块头
		m_CurBlockHdr.nPrev				= 0;
		m_CurBlockHdr.nNext				= 0;
		if( m_pPB->GetBaseFile()->Write(&m_CurBlockHdr) != sizeof(m_CurBlockHdr) )
		{
			return	-3;
		}
		// 首块的附加数据
		memset(&m_FirstBlockExt, 0, sizeof(FIRST_BLOCK_EXT_T));
		m_nStart						= nStart;
		m_FirstBlockExt.nEnd			= nStart;
		// 文件长度
		m_FirstBlockExt.nFileSize		= 0;
		m_FirstBlockExt.nFileTime		= wh_time();
		// 写入附加数据块
		if( m_pPB->GetBaseFile()->Write(&m_FirstBlockExt) != sizeof(FIRST_BLOCK_EXT_T) )
		{
			return	-4;
		}
		// 当前文件块
		m_nCurBlock						= GetStart();
	}
	else
	{
		m_nStart		= nStart;
		// Seek到第一块
		if( SeekToBlockAndReadHdr(nStart)<0 )
		{
			return	-11;
		}
		if( m_CurBlockHdr.nPrev != 0 )
		{
			// 说明这个不是文件的第一块
			return	-12;
		}
		// 读出附加数据
		if( m_pPB->GetBaseFile()->Read(&m_FirstBlockExt) != sizeof(FIRST_BLOCK_EXT_T) )
		{
			return	-13;
		}
#ifdef	_DEBUG
		// 校验一下文件是否正确
		// 从文件长度计算文件总块数（减一）
		int nOffsetInBlock, nBlockNum;
		CalcBlocknumAndOffsetinblock(FileSize(), &nBlockNum, &nOffsetInBlock);
		// 遍历一下链表
		while( m_CurBlockHdr.nNext != 0 )
		{
			if( (--nBlockNum)<0 )
			{
				return	-14;
			}
			if( SeekToBlockAndReadHdr(m_CurBlockHdr.nNext)<0 )
			{
				return	-15;
			}
		}
		// 如果正常的话nBlockNum应该最后剩下是0
		if( nBlockNum!=0 )
		{
			return	-16;
		}
		if( m_nCurBlock != GetEnd() )
		{
			return	-17;
		}
		// 因为后面的Read和Write都会重新Seek，所以这里就不用重新Seek到文件开头了
		// 不过应该重新把指针指向开头
		m_nCurBlock		= GetStart();
#endif
	}
	// 将文件指针指向开头
	m_nCurOffset		= 0;
	return	0;
}
int		wh_package_base::rawfile::Close()
{
	if( m_pPB )
	{
		if( m_bShouldSaveFirstBlockExt )
		{
			m_bShouldSaveFirstBlockExt	= false;
			// 应该重新保存第一块里的附加信息
			if( IsReadOnly() )
			{
				// 对于只读文件应该不会发生这样的事情的
				assert(0);
				return	-1;
			}
			// Seek到文件头
			if( SeekToBlockAndReadHdr(GetStart())<0 )
			{
				return	-2;
			}
			// 写入信息
			if( m_pPB->GetBaseFile()->Write(&m_FirstBlockExt) != sizeof(FIRST_BLOCK_EXT_T) )
			{
				return	-3;
			}
			/*
			/////
			m_pPB->GetBaseFile()->Flush();
			if( SeekToBlockAndReadHdr(GetStart())<0 )
			{
				return	-2;
			}
			if( m_pPB->GetBaseFile()->Read(&m_FirstBlockExt) != sizeof(FIRST_BLOCK_EXT_T) )
			{
				return	-3;
			}
			*/
		}
		// 为了保险把m_pPB清了吧
		m_pPB	= NULL;
	}
	return	0;
}
int		wh_package_base::rawfile::Delete()
{
	if( IsReadOnly() )
	{
		return	-1;
	}
	// 释放从Start到End的块
	if( m_pPB->FreeBlock(GetStart(), GetEnd())<0 )
	{
		return	-2;
	}
	// 清空包指针，以免后面再进行任何操作
	m_pPB	= NULL;
	// 删除自己
	delete	this;
	return	0;
}
int		wh_package_base::rawfile::Extend(int nSize)
{
	if( IsReadOnly() )
	{
		return	-1;
	}
	// 记录一下当前的block和offset
	blockoffset_t	nCurBlock	= m_nCurBlock;
	int				nCurOffset	= m_nCurOffset;
	bool			bLastByteSpecial	= false;
	// 如果当前位置是文件结尾且在接缝处，那么标记一下
	if( m_nCurOffset == FileSize() && (m_nCurOffset+sizeof(FIRST_BLOCK_EXT_T))%GetBlockDataMaxSize()==0 )
	{
		bLastByteSpecial		= true;
	}
	// 如果扩展的时候文件指针刚好在块接缝处
	int	rst	= Seek(nSize, SEEK_END);
	if( rst<0 )
	{
		return	-10+rst;
	}
	if( bLastByteSpecial )
	{
		// 因为块扩展了，所以m_nCurBlock需要变成它的下一块，块内偏移是0
		SeekToBlockAndReadHdr(nCurBlock);
		nCurBlock	= m_CurBlockHdr.nNext;
		assert(nCurBlock>0);
	}
	m_nCurBlock		= nCurBlock;
	m_nCurOffset	= nCurOffset;
	return	FileSize();
}
void	wh_package_base::rawfile::SetFileTime(time_t t)
{
	if( IsReadOnly() )
	{
		assert(0);
		return;
	}
	if( t==0 )
	{
		t		= wh_time();
	}
	m_FirstBlockExt.nFileTime	= t;
	m_bShouldSaveFirstBlockExt	= true;
}
int		wh_package_base::rawfile::FileSize() const
{
	return	m_FirstBlockExt.nFileSize;
}
time_t	wh_package_base::rawfile::FileTime() const
{
	return	m_FirstBlockExt.nFileTime;
}
int		wh_package_base::rawfile::Read(void *pBuf, int nSize)
{
	// 上层应该保证nSize>0的
	if( nSize<0 )
	{
		return	-1;
	}
	if( nSize==0 )
	{
		return	0;
	}

	if( IsEOF() )
	{
		// 到了文件结尾就不应该再读了
		return	0;
	}

	int		nTotal	= 0;

	while (nSize > 0)
	{
		// 对于新块都需要重新在主文件中Seek一下
		int	nOffsetInBlock;
		if( SeekToMyCurPos(&nOffsetInBlock)<0 )
		{
			return	-2;
		}

		// 算一下当前块内还剩余多少字节
		int		nDataSizeLeft;
		if( !IsCurBlockLastOne() )
		{
			// 是第一块或中间一块（因为nOffsetInBlock在每块中的意义都相同，在第一块内也是加上了FIRST_BLOCK_EXT_T的长度，所以都用GetBlockDataMaxSize即可）
			nDataSizeLeft	= GetBlockDataMaxSize() - nOffsetInBlock;

			if (nDataSizeLeft <= 0)
			{
				assert(0 && "理论上如果在前面判断了IsEOF()的情况下，是不会出现nDataSizeLeft为0的情况的");
				break;
			}

			int nReadSize = (nSize <= nDataSizeLeft) ? nSize : nDataSizeLeft;
			int	rst	= m_pPB->GetBaseFile()->Read(pBuf, nReadSize);
			if( rst<0 )
			{
				// 读取失败
				return	-10;
			}
			// 移动文件指针
			m_nCurOffset		+= rst;
			// 总读数
			nTotal				+= rst;
			// 缓冲区和剩余需要读的大小都改变一下
			nSize				-= rst;
			pBuf				= wh_getoffsetaddr(pBuf, rst);

			if (rst == nDataSizeLeft)
			{
				// 准备读下一块
				m_nCurBlock			= m_CurBlockHdr.nNext;
			}
		}
		else
		{
			// 是最后一块
			nDataSizeLeft	= FileSize() - m_nCurOffset;

			// 直接读完了返回即可
			int	rst	= m_pPB->GetBaseFile()->Read(pBuf, (nSize<=nDataSizeLeft) ? nSize : nDataSizeLeft);
			if( rst<0 )
			{
				// 读取失败
				return	-10;
			}
			// 移动文件指针
			m_nCurOffset		+= rst;
			// 总读数
			nTotal				+= rst;
			break;
		}
		//// 需要读取整个块剩余的数据
		//if( nDataSizeLeft>0 )
		//{
		//	int	rst	= m_pPB->GetBaseFile()->Read(pBuf, nDataSizeLeft);
		//	if( rst<0 )
		//	{
		//		// 读取失败
		//		return	-11;
		//	}
		//	// 移动文件指针
		//	m_nCurOffset		+= rst;
		//	// 总读数
		//	nTotal				+= rst;
		//	// 缓冲区和剩余需要读的大小都改变一下
		//	nSize				-= rst;
		//	pBuf				= wh_getoffsetaddr(pBuf, rst);
		//}
		//else
		//{
		//	// 理论上如果在前面判断了IsEOF()的情况下，是不会出现nDataSizeLeft为0的情况的
		//	assert(0);
		//	return	-12;
		//}
		//// 判断是否是文件结尾
		//if( m_CurBlockHdr.nNext == 0 )
		//{
		//	assert(m_nCurOffset == FileSize());
		//	// 到了，直接返回当前读到的长度
		//	return	nTotal;
		//}
		//// 准备读下一块
		//m_nCurBlock			= m_CurBlockHdr.nNext;
		//// 继续读取
	}

	return	nTotal;
}
int		wh_package_base::rawfile::Seek(int nOffset, int nOrigin)
{
	// 无论是那种Seek最终都转化为先将底层Seek到一点，然后再做相对Seek
	// Seek只会定位到特定block，而不会定位到具体的offset，所以在Read和Write内部还会先调用一下SeekToMyCurPos以保证BaseFile的指针正确
	switch( nOrigin )
	{
	case	SEEK_CUR:
		{
			// 因为底层文件只有一个，所以在打开多个文件的时候，很难保证别的文件读写操作不改变底层文件的指针，所以即便是SEEK_CUR也要先重新Seek到当前位置
			int	rst	= SeekToBlockAndReadHdr(m_nCurBlock);
			if( rst<0 )
			{
				return	-1;
			}
		}
		break;
	case	SEEK_END:
		{
			int	rst	= SeekToBlockAndReadHdr(GetEnd());
			if( rst<0 )
			{
				return	-2;
			}
			m_nCurOffset		= FileSize();
		}
		break;
	case	SEEK_SET:
	default:
		{
			int	rst	= SeekToBlockAndReadHdr(GetStart());
			if( rst<0 )
			{
				return	-3;
			}
			m_nCurOffset		= 0;
		}
		break;
	}
	if( nOffset==0 )
	{
		// 已经到地方了，不用继续找了
		return	0;
	}
	int	nDstOffset	= m_nCurOffset+nOffset;
	int	nCurBlockNum;
	int	nCurOffsetInBlock;
	CalcBlocknumAndOffsetinblock(m_nCurOffset, &nCurBlockNum, &nCurOffsetInBlock);
	int	nDstBlockNum;
	int	nDstOffsetInBlock;
	CalcBlocknumAndOffsetinblock(nDstOffset, &nDstBlockNum, &nDstOffsetInBlock);
	if( nOffset>0 )
	{
		// 首先看是否会超过文件长度
		if( nDstOffset < FileSize() )
		{
			// 一定能在当前文件现有范围内解决
			// 计算应该把指针移动到哪一块
			// nDstBlockNum肯定>=nCurBlockNum
			// 向后Seek若干个block
			for(;nCurBlockNum<nDstBlockNum;++nCurBlockNum)
			{
				assert(m_CurBlockHdr.nNext>0);
				SeekToBlockAndReadHdr(m_CurBlockHdr.nNext);
			}
			// 可以在最后一块内解决了（最后设置一下指针即可）
			m_nCurOffset	= nDstOffset;
			return	0;
		}
		// 因为会超过文件长度，所以相当于从最后一个位置开始向后再多Seek一些
		// 先Seek到结尾
		int	rst	= SeekToBlockAndReadHdr(GetEnd());
		if( rst<0 )
		{
			return	-21;
		}
		m_nCurOffset		= FileSize();
		// 看是否刚好等于文件长度
		if( nDstOffset == FileSize() )
		{
			// 刚好等于，就不用再做多余的工作了
			return	0;
		}
		// 判断文件是否只读
		if( IsReadOnly() )
		{
			// 那么就到此为止了
			return	0;
		}
		// 计算出总共需要增加多少块，申请这些块
		while( nCurBlockNum<nDstBlockNum )
		{
			m_CurBlockHdr.nNext	= m_pPB->AllocBlock();
			if( m_CurBlockHdr.nNext == 0 )
			{
				return	-31;
			}
			// 保存下一个块（也就是这个新块的）头（因为可能下一个是最后一块，就跳出循环了）
			BLOCK_HDR_T		BlockHdr;
			BlockHdr.nPrev	= m_nCurBlock;
			BlockHdr.nNext	= 0;
			if( m_pPB->GetBaseFile()->Write(&BlockHdr)!=sizeof(BlockHdr) )
			{
				return	-32;
			}
			// 当前块头存盘
			SeekToBlockHdr(m_nCurBlock);
			if( m_pPB->GetBaseFile()->Write(&m_CurBlockHdr)!=sizeof(m_CurBlockHdr) )
			{
				return	-33;
			}
			// 设下一块为当前块
			// 并Seek到它
			SeekToBlockAndReadHdr(m_CurBlockHdr.nNext);
			// 下一个
			nCurBlockNum	++;
		}
		// 现在已经到了最后一块了
		m_FirstBlockExt.nEnd		= m_nCurBlock;
		// 调整指针
		m_nCurOffset				= nDstOffset;
		// 改变文件长度
		m_FirstBlockExt.nFileSize	= m_nCurOffset;
		m_bShouldSaveFirstBlockExt	= true;
		// 修改一下文件时间
		SetFileTime(0);
	}
	else
	{
		// 向前
		if( nDstOffset<=0 )
		{
			// 那么就相当于Seek到文件头
			SeekToBlockAndReadHdr(GetStart());
			m_nCurOffset			= 0;
			return	0;
		}
		// 目的点不会超过文件头
		// 那么计算一下该向前跳过多少块
		while( nCurBlockNum>nDstBlockNum )
		{
			SeekToBlockAndReadHdr(m_CurBlockHdr.nPrev);
			nCurBlockNum			--;
		}
		// 到了，最后设置一下指针位置
		m_nCurOffset				= nDstOffset;
	}

	return	0;
}
bool	wh_package_base::rawfile::IsEOF()
{
	// 之所以是">="，是因为可能Seek的实现会导致readonly时的文件指针移动到文件长度之外（反正linux系统的fseek就是这样，不过我还没有决定这样实现）
	return	m_nCurOffset >= FileSize();
}
int		wh_package_base::rawfile::Tell()
{
	return	m_nCurOffset;
}
int		wh_package_base::rawfile::Write(const void *pBuf, int nSize)
{
	if( IsReadOnly() )
	{
		return	-1;
	}
	int	nDstOffset	= m_nCurOffset+nSize;
	// 看写的区域是否在文件长度范围内
	if( nDstOffset > FileSize() )
	{
		// 扩展文件长度（这样就不会有写的过程中扩展文件的问题了）
		// Extend不改变当前的Offset，所以就不用记录现在的m_nCurBlock、m_nCurOffset相关参数了
		if( Extend(nDstOffset-FileSize())<0 )
		{
			return	-11;
		}
	}

	// 这样就没有扩充文件的问题了
	int	nCurBlockNum;
	int	nCurOffsetInBlock;
	CalcBlocknumAndOffsetinblock(m_nCurOffset, &nCurBlockNum, &nCurOffsetInBlock);
	int	nDstBlockNum;
	int	nDstOffsetInBlock;
	CalcBlocknumAndOffsetinblock(nDstOffset, &nDstBlockNum, &nDstOffsetInBlock);
	
	// 下面仿照Read写就行
	int	nTotal	= 0;

	while( nSize>0 )
	{
		// 先将底层文件Seek到当前位置
		// 对于新块都需要重新在主文件中Seek一下
		int	nOffsetInBlock;
		if( SeekToMyCurPos(&nOffsetInBlock)<0 )
		{
			return	-12;
		}
		// 算一下当前块内还剩余多少字节
		int		nDataSizeLeft;
		if( IsCurBlockLastOne() )
		{
			// 是最后一块
			nDataSizeLeft	= FileSize() - m_nCurOffset;
		}
		else
		{
			// 是第一块或中间一块（因为nOffsetInBlock在每块中的意义都相同，在第一块内也是加上了FIRST_BLOCK_EXT_T的长度，所以都用GetBlockDataMaxSize即可）
			nDataSizeLeft	= GetBlockDataMaxSize() - nOffsetInBlock;
		}
		if( nSize<nDataSizeLeft )	// 这应该是写到最后才出现的情况
		{
			nDataSizeLeft	= nSize;
		}
		// 读取剩余的数据
		int	rst	= m_pPB->GetBaseFile()->Write(pBuf, nDataSizeLeft);
		if( rst<0 )
		{
			// 读取失败
			return	-13;
		}
		// 总读数
		nTotal				+= rst;
		nSize				-= rst;
		pBuf				= wh_getoffsetaddr(pBuf, rst);
		// 移动文件指针
		m_nCurOffset		+= rst;
		if( nSize==0 )
		{
			// 退出循环
			break;
		}
		else if( m_CurBlockHdr.nNext>0 )
		{
			// 下一块
			m_nCurBlock		= m_CurBlockHdr.nNext;
		}
	}

	// 修改一下文件时间
	SetFileTime(0);

	return	nTotal;
}
int		wh_package_base::rawfile::Flush()
{
	// 还没有考虑好做什么。如果每次都调用BaseFile的Flush会不会不好啊？
	return	0;
}
int		wh_package_base::rawfile::SeekToBlockHdr(blockoffset_t nBlock)
{
	return	m_pPB->SeekToBlockHdr(nBlock);
}
int		wh_package_base::rawfile::SeekToBlockAndReadHdr(blockoffset_t nBlock)
{
	if( m_pPB->SeekToBlockHdr(nBlock)<0 )
	{
		return	-1;
	}
	if( m_pPB->GetBaseFile()->Read(&m_CurBlockHdr) != sizeof(m_CurBlockHdr) )
	{
		return	-2;
	}
	// 这些设置是每到一个块的开头都必须做的
	m_nCurBlock			= nBlock;
	return	0;
}
int		wh_package_base::rawfile::SeekToMyCurPos(int *pnOffsetInBlock)
{
	int	nBlockNum;
	CalcBlocknumAndOffsetinblock(m_nCurOffset, &nBlockNum, pnOffsetInBlock);
	int	rst	= SeekToBlockAndReadHdr(m_nCurBlock);
	if( rst<0 )
	{
		return	rst;
	}
	if( (*pnOffsetInBlock)>0 )
	{
		rst	= m_pPB->GetBaseFile()->Seek(*pnOffsetInBlock, SEEK_CUR);
		if( rst<0 )
		{
			return	-100+rst;
		}
	}
	return	0;
}

////////////////////////////////////////////////////////////////////
// wh_package_base::dir4out
////////////////////////////////////////////////////////////////////
wh_package_base::dir4out::dir4out(STR2NODE_T *pDir, const char *cszBaseDir)
: m_pDir(pDir)
{
	strcpy(m_szBaseDir, cszBaseDir);
	m_it	= m_pDir->begin();
}
wh_package_base::dir4out::~dir4out()
{
}
int		wh_package_base::dir4out::Close()
{
	// 什么也不用做，因为没有new的东西
	return	0;
}
WHDirBase::ENTRYINFO_T *	wh_package_base::dir4out::Read()
{
	if( m_it == m_pDir->end() )
	{
		return	NULL;
	}
	STRINVECT	&siv	= m_it.getkey();
	FILENODE_T	&node	= m_it.getvalue();
	m_EntryInfo.bIsDir			= node.IsDir();
	strcpy(m_EntryInfo.szName, siv.GetPtr());
	if( !m_EntryInfo.bIsDir )
	{
		m_EntryInfo.ext.pck.nID	= node.u.f.nStartBlock;
	}
	++m_it;	// 下一个
	return	&m_EntryInfo;
}
void	wh_package_base::dir4out::Rewind()
{
	m_it	= m_pDir->begin();
}

////////////////////////////////////////////////////////////////////
// wh_package_base
////////////////////////////////////////////////////////////////////
const char	*wh_package_base::PACKAGE_HDR_T::CSZMAGIC	= "whpackage1.0";
// 检查文件是否是包文件
wh_package_base::enumFILEISWHAT	wh_package_base::FileIsWhat(whfile *fp)
{
	PACKAGE_HDR_T	m_package_hdr;
	// 读入文件头块
	if( fp->Read(&m_package_hdr)==sizeof(m_package_hdr) )
	{
		// 判断块头是否正常
		if( m_package_hdr.IsOK() )
		{
			return	wh_package_base::FILEISWHAT_PCK;
		}
	}
	// 否则看文件是否是尾部的文件
	whtailfile	tf;
	if( tf.Open(fp, false, whfileman::OP_MODE_BIN_READONLY)==0 )
	{
		if( wh_package_base::FileIsWhat(&tf)==wh_package_base::FILEISWHAT_PCK )
		{
			return	wh_package_base::FILEISWHAT_TAILPCK;
		}
	}
	// tf应该会在析构的时候关闭（不过只读也没啥好清理的）
	return		wh_package_base::FILEISWHAT_COMMON;
}
wh_package_base::enumFILEISWHAT	wh_package_base::FileIsWhat(whfileman *pFM, const char *cszFName)
{
	whsafeptr<whfile>	fp(pFM->Open(cszFName, whfileman::OP_MODE_BIN_READONLY));
	if( !fp )
	{
		return	wh_package_base::FILEISWHAT_ERROR;
	}
	return	FileIsWhat(fp);
}
// 类实现
wh_package_base::wh_package_base()
: m_bInited(false)
, m_bShouldSavePackageHdr(false)
, m_bShouldSaveFileList(false)
, m_bShouldSaveVectorFileName(false)
{
	m_vectFNAME.reserve(4096);
}
wh_package_base::~wh_package_base()
{
	Release();
}
int		wh_package_base::Init(INFO_T *pInfo)
{
	if( m_bInited )
	{
		assert(0);
		return	-1;
	}
	m_bInited	= true;
	// 保存初始化信息，后面很多地方要用，比如：GetBaseFile
	memcpy(&m_info, pInfo, sizeof(m_info));
	if( GetBaseFile()->Seek(0, SEEK_SET)!=0 )
	{
		return	-2;
	}
	unsigned char	MD5[16];
	whmd5(m_info.szPass, m_info.nPassLen, MD5);

	// 设置一下加密器和解密器的密码
	if( m_info.pEncryptor )
	{
		if( m_info.nPassLen>0 )
		{
			m_info.pEncryptor->SetEncryptKey(m_info.szPass, m_info.nPassLen);
		}
		else
		{
			m_info.pEncryptor	= NULL;
		}
	}
	if( m_info.pDecryptor )
	{
		if( m_info.nPassLen>0 )
		{
			m_info.pDecryptor->SetDecryptKey(m_info.szPass, m_info.nPassLen);
		}
		else
		{
			m_info.pDecryptor	= NULL;
		}
	}
	switch( m_info.nOPMode )
	{
	case	whfileman::OP_MODE_BIN_CREATE:
		{
			// 填写头部数据（这个必须在MakeRawBlock之前填写，因为MakeRawBlock里面会使用到m_package_hdr.nBlockSize）
			m_package_hdr.reset();
			m_package_hdr.nBlockSize	= m_info.nBlockSize;
			memcpy(m_package_hdr.PassMD5, MD5, sizeof(MD5));
			// 创建文件头块
			if( MakeRawBlock(0)<0 )
			{
				return	-11;
			}
			// 创建文件信息表和文件名表（空文件，只是为了关闭的时候能统一只做写操作而不用做创建操作）
			// 文件信息表
			wh_package_base::rawfile	*f	= NULL;
			try
			{
				f	= NewRawFile();
				if( !f )
				{
					throw	-12;
				}
				if( f->Open(whfileman::OP_MODE_BIN_CREATE, m_package_hdr.anRawfile[PACKAGE_HDR_T::RF_IDX_FINFO])<0 )
				{
					throw	-13;
				}
				WHSafeSelfDestroy(f);
				// 创建目录根节点
				m_RootNode.SetIsDir(true);
				m_RootNode.u.d.pContent	= new STR2NODE_T;
				if( !m_RootNode.u.d.pContent )
				{
					throw	-14;
				}
				// 文件名表
				f		= NewRawFile();
				if( !f )
				{
					throw	-16;
				}
				if( f->Open(whfileman::OP_MODE_BIN_CREATE, m_package_hdr.anRawfile[PACKAGE_HDR_T::RF_IDX_FNAME])<0 )
				{
					throw	-17;
				}
				WHSafeSelfDestroy(f);
				// 清空文件名vector
				m_vectFNAME.clear();
				// 因为是新创建的文件，所以最后头一定要存盘的（至少要形成空文件）
				m_bShouldSavePackageHdr		= true;
				m_bShouldSaveFileList		= true;
				m_bShouldSaveVectorFileName	= true;
			}
			catch (int nErr)
			{
				WHSafeSelfDestroy(f);
				return	nErr;
			}
		}
		break;
	case	whfileman::OP_MODE_BIN_READWRITE:
	case	whfileman::OP_MODE_BIN_READONLY:
	default:
		{
			wh_package_singlefile::INFO_T	info;
			info.nOPMode		= m_info.nOPMode;
			info.pWHCompress	= m_info.pWHCompress;
			info.pEncryptor		= m_info.pEncryptor;
			info.pDecryptor		= m_info.pDecryptor;
			// 读入文件头块
			if( GetBaseFile()->Read(&m_package_hdr)!=sizeof(m_package_hdr) )
			{
				return	-21;
			}
			// 校验头是否合法
			if( !m_package_hdr.IsOK() )
			{
				return	-22;
			}
			// 校验密码是否正确
			if( memcmp(m_package_hdr.PassMD5, MD5, sizeof(MD5))!=0 )
			{
				return	-23;
			}
			m_info.nBlockSize	= m_package_hdr.nBlockSize;
			int	nBlockNum	= GetBaseFile()->FileSize() / m_info.nBlockSize;
			// 校验文件长度是否正常
			if( m_info.bCheckFileBlockNumOnOpen )
			{
				if( nBlockNum!=m_package_hdr.nMaxBlockIdx )
				{
					return	-24;
				}
			}
			// 打开文件信息表和文件名表
			wh_package_base::rawfile	*f	= NULL;
			wh_package_singlefile		*sc	= NULL;
			try
			{
				// 应该先读文件名表（因为文件索引表中需要引用文件名）
				f	= NewRawFile();
				if( !f )
				{
					throw	-31;
				}
				if( f->Open(m_info.nOPMode, m_package_hdr.anRawfile[PACKAGE_HDR_T::RF_IDX_FNAME])<0 )
				{
					throw	-32;
				}
				sc	= new wh_package_singlefile;
				if( !sc )
				{
					throw	-33;
				}
				info.pBaseFile				= f;
				if( sc->Open(&info)<0 )
				{
					throw	-34;
				}
				m_vectFNAME.resize(sc->FileSize());
				if( sc->Read(m_vectFNAME.getbuf(), m_vectFNAME.size())!=(int)m_vectFNAME.size() )
				{
					throw	-35;
				}
				// 一个一个读出创建m_setFNAME集合
				int	i=0,j=0;
				while( j<(int)m_vectFNAME.size() )
				{
					if( m_vectFNAME[j]==0 )
					{
						STRINVECT	siv(&m_vectFNAME, i);
						if( !m_setFNAME.put(siv) )
						{
							// 说明名字重了，这个是不应该的
							assert(0);
							throw	-36;
						}
						i	= ++j;
					}
					else
					{
						++j;
					}
				}
				// 关闭文件
				WHSafeSelfDestroy(sc);
				WHSafeSelfDestroy(f);
			
				// 然后读取文件索引表
				f	= NewRawFile();
				if( !f )
				{
					return	-41;
				}
				if( f->Open(m_info.nOPMode, m_package_hdr.anRawfile[PACKAGE_HDR_T::RF_IDX_FINFO])<0 )
				{
					return	-42;
				}
				sc	= new wh_package_singlefile;
				if( !sc )
				{
					return	-43;
				}
				info.pBaseFile				= f;
				if( sc->Open(&info)<0 )
				{
					return	-44;
				}
				// 创建根节点读出内容并组织目录树
				m_RootNode.SetIsDir(true);
				m_RootNode.u.d.pContent	= new STR2NODE_T;
				if( !m_RootNode.u.d.pContent )
				{
					throw	-45;
				}
				if( ReadDirTreeFromFile(m_RootNode.u.d.pContent, sc)<0 )
				{
					throw	-46;
				}
				// 关闭文件
				WHSafeSelfDestroy(sc);
				WHSafeSelfDestroy(f);
			}
			catch (int nErr)
			{
				WHSafeSelfDestroy(sc);
				WHSafeSelfDestroy(f);
				return	nErr;
			}
		}
		break;
	}

	return	0;
}
int		wh_package_base::Release()
{
	if( !m_bInited )
	{
		return	0;
	}
	if( !IsReadOnly() )
	{
		// 保存两个基本文件
		SaveInfoFiles();
		// 保存包头
		SavePackageHdr();
	}
	// 删除各个文件节点
	FreeDirTree(m_RootNode.u.d.pContent);
	// 清空名字vector
	m_vectFNAME.clear();
	if( m_info.bAutoCloseBaseFile )
	{
		WHSafeSelfDestroy(m_info.pBaseFile);
	}
	// 成功释放了才能情况初始化标记
	m_bInited	= false;
	return	0;
}
int		wh_package_base::SavePackageHdr()
{
	if( !IsReadOnly() )
	{
		// 如果package_hdr改变了就保存之
		if( m_bShouldSavePackageHdr )
		{
			// Seek到package文件头
			if( GetBaseFile()->Seek(0, SEEK_SET)<0 )
			{
				return	-1;
			}
			// 写之
			if( GetBaseFile()->Write(&m_package_hdr)!=sizeof(m_package_hdr) )
			{
				return	-2;
			}
			// 应该在最后写成功之后才改变标记
			m_bShouldSavePackageHdr	= false;
		}
	}
	return	0;
}
int		wh_package_base::SaveInfoFiles()
{
	wh_package_base::rawfile	*f	= NULL;
	wh_package_singlefile		*sc	= NULL;
	wh_package_singlefile::INFO_T	info;
	info.nOPMode				= whfileman::OP_MODE_BIN_CREATE;
	info.pWHCompress			= m_info.pWHCompress;
	info.pEncryptor				= m_info.pEncryptor;
	info.pDecryptor				= m_info.pDecryptor;
	try
	{
		// 文件名表（如果改变了）
		if( m_bShouldSaveVectorFileName )
		{
			f	= NewRawFile();
			if( !f )
			{
				throw	-11;
			}
			if( f->Open(whfileman::OP_MODE_BIN_READWRITE, m_package_hdr.anRawfile[PACKAGE_HDR_T::RF_IDX_FNAME])<0 )
			{
				throw	-12;
			}
			info.pBaseFile	= f;
			sc	= new wh_package_singlefile;
			if( !sc )
			{
				throw	-13;
			}
			if( sc->Open(&info)<0 )
			{
				throw	-14;
			}
			if( sc->Write(m_vectFNAME.getbuf(), m_vectFNAME.size())!=(int)m_vectFNAME.size() )
			{
				throw	-15;
			}
			// 关闭文件
			WHSafeSelfDestroy(sc);
			WHSafeSelfDestroy(f);
			// 清空标记
			m_bShouldSaveVectorFileName	= false;
		}
		// 文件信息列表（如果改变了）
		if( m_bShouldSaveFileList )
		{
			f	= NewRawFile();
			if( !f )
			{
				throw	-21;
			}
			if( f->Open(whfileman::OP_MODE_BIN_READWRITE, m_package_hdr.anRawfile[PACKAGE_HDR_T::RF_IDX_FINFO])<0 )
			{
				throw	-22;
			}
			info.pBaseFile	= f;
			sc	= new wh_package_singlefile;
			if( !sc )
			{
				throw	-23;
			}
			if( sc->Open(&info)<0 )
			{
				throw	-24;
			}
			if( SaveDirTreeToFile(m_RootNode.u.d.pContent, sc)<0 )
			{
				throw	-25;
			}
			// 关闭文件
			WHSafeSelfDestroy(sc);
			WHSafeSelfDestroy(f);
			// 清空标记
			m_bShouldSaveFileList	= false;
		}
	}
	catch (int nErr)
	{
		WHSafeSelfDestroy(sc);
		WHSafeSelfDestroy(f);
		return	nErr;
	}
	return	0;
}
int		wh_package_base::Flush()
{
	// 保存头
	if( SavePackageHdr()<0 )
	{
		return	-1;
	}
	// 保存两个索引文件
	if( SaveInfoFiles()<0 )
	{
		return	-2;
	}
	if( GetBaseFile()->Flush()<0 )
	{
		return	-4;
	}
	return		0;
}
wh_package_base::blockoffset_t	wh_package_base::AllocBlock()
{
	// 先看可用表里有没有
	blockoffset_t	nIdx	= m_package_hdr.nAvailIdx;
	if( nIdx>0 )
	{
		// 从表里分
		if( SeekToBlockHdr(nIdx)<0 )
		{
			return	0;
		}
		// 读出头部
		BLOCK_HDR_T	hdr;
		if( GetBaseFile()->Read(&hdr) != sizeof(hdr) )
		{
			return	0;
		}
		// 再Seek回块头（这是规定，上层在获得块之后可能会立刻写块，不过上层可要做好互斥工作啊，如果多线程可能会有问题）
		if( GetBaseFile()->Seek(-(int)(sizeof(hdr)), SEEK_CUR)!=0 )
		{
			return	0;
		}
		// avail表指向下一个
		m_package_hdr.nAvailIdx	= hdr.nNext;
		// 标记package_hdr被修改了
		m_bShouldSavePackageHdr	= true;
		// 返回分配到的块号
		return	nIdx;
	}
	// 需要通过扩展文件分配新块
	// 记录新块号
	nIdx	= m_package_hdr.nMaxBlockIdx	++;
	// Seek扩展
	if( MakeRawBlock(nIdx)<0 )
	{
		return	0;
	}
	// 把文件指针移到新块头
	if( SeekToBlockHdr(nIdx)<0 )
	{
		return	0;
	}
	// 标记package_hdr被修改了
	m_bShouldSavePackageHdr		= true;
	return	nIdx;
}
int		wh_package_base::MakeRawBlock(blockoffset_t nBlock)
{
	if( GetBaseFile()->Seek((nBlock+1)*m_package_hdr.nBlockSize-1, SEEK_SET)!=0 )
	{
		return	-1;
	}
	// 写入最后一字节使生效
	char	c	= 0;
	if( GetBaseFile()->Write(&c)!=1 )
	{
		return	-2;
	}
	return	0;
}
int		wh_package_base::SeekToBlockHdr(blockoffset_t nBlock)
{
	if( nBlock<=0 || nBlock>=m_package_hdr.nMaxBlockIdx )
	{
		return	-1;
	}
	return	GetBaseFile()->Seek(nBlock*m_package_hdr.nBlockSize, SEEK_SET);;
}
int		wh_package_base::FreeBlock(blockoffset_t nStart, blockoffset_t nEnd)
{
	// 0就不用释放了
	if( nStart==0 )
	{
		return	0;
	}
	if( SeekToBlockHdr(nEnd)<0 )
	{
		return	-1;
	}
	// 读出End块头
	BLOCK_HDR_T	hdr;
	if( GetBaseFile()->Read(&hdr)!=sizeof(hdr) )
	{
		return	-2;
	}
	// 修改
	hdr.nNext	= m_package_hdr.nAvailIdx;
	// 保存修改过的最后End块头
	if( GetBaseFile()->Seek(-(int)sizeof(hdr), SEEK_CUR)!=0 )
	{
		return	-3;
	}
	if( GetBaseFile()->Write(&hdr)!=sizeof(hdr) )
	{
		return	-4;
	}
	// 把m_package_hdr.nAvailIdx对应块的上一块指向nEnd
	if( m_package_hdr.nAvailIdx>0 )
	{
		if( SeekToBlockHdr(m_package_hdr.nAvailIdx)<0 )
		{
			return	-11;
		}
		if( GetBaseFile()->Read(&hdr)!=sizeof(hdr) )
		{
			return	-12;
		}
		hdr.nPrev	= nEnd;
		// 保存修改过的块头
		if( GetBaseFile()->Seek(-(int)sizeof(hdr), SEEK_CUR)!=0 )
		{
			return	-13;
		}
		if( GetBaseFile()->Write(&hdr)!=sizeof(hdr) )
		{
			return	-14;
		}
	}
	// 最后修改package_hdr
	m_package_hdr.nAvailIdx	= nStart;
	m_bShouldSavePackageHdr	= true;
	return	0;
}
int		wh_package_base::ReadDirTreeFromFile(STR2NODE_T *pRoot, whfile *f)
{
	while( !f->IsEOF() )
	{
		unsigned char	nType;
		// 读入类型
		int	rst			= f->Read(&nType) ;
		if( rst==0 )
		{
			// 应该是数据已经全部读完了
			// 结束处理过程
			return	0;
		}
		if( rst!= sizeof(nType) )
		{
			// 读取文件出错
			return	-1;
		}
		switch( nType )
		{
		case	TYPE_DIREND:
			{
				// 退出本级目录（所以如果增加一个没有BEGIN对应的END就会导致读取工作提早结束）
				return	0;
			}
			break;
		case	TYPE_DIRBEGIN:
			{
				// 读入名字偏移
				unsigned int		nNameOffset;
				if( f->Read(&nNameOffset) != sizeof(nNameOffset) )
				{
					// 数据出错
					return	-2;
				}
				// 加入到本级映射表中
				STRINVECT	sivName(&m_vectFNAME, nNameOffset);
				FILENODE_T	*pNode;
				if( !pRoot->putalloc(sivName, pNode) )
				{
					// 可能是名字重了？
					return	-3;
				}
				pNode->u.d.pContent	= new STR2NODE_T;
				if( !pNode->u.d.pContent )
				{
					return	-4;
				}
				pNode->SetIsDir(true);
				// 递归读入下一级
				int	rst	= ReadDirTreeFromFile(pNode->u.d.pContent, f);
				if( rst<0 )
				{
					// 处理子目录时出错
					return	-5;
				}
				// 这个子目录处理成功了，接着读后面的数据吧，OVER
			}
			break;
		case	TYPE_FILE:
			{
				// 读入名字偏移
				unsigned int		nNameOffset;
				if( f->Read(&nNameOffset) != sizeof(nNameOffset) )
				{
					// 数据出错
					return	-12;
				}
				// 加入到本级映射表中
				STRINVECT	sivName(&m_vectFNAME, nNameOffset);
				FILENODE_T	*pNode;
				if( !pRoot->putalloc(sivName, pNode) )
				{
					// 可能是名字重了？
					return	-13;
				}
				pNode->SetIsDir(false);
				// 读入起始块索引
				if( f->Read(&pNode->u.f.nStartBlock)!=sizeof(pNode->u.f.nStartBlock) )
				{
					// 数据错误
					return	-14;
				}
			}
			break;
		default:
			{
				// 错误的类型
				return	-200;
			}
			break;
		}
	}
	return	0;
}
int		wh_package_base::SaveDirTreeToFile(STR2NODE_T *pRoot, whfile *f)
{
	if( !pRoot )
	{
		// 没有内容不用写了
		return	0;
	}
	// 遍历自己的子节点，把他们写入
	for(STR2NODE_T::kv_iterator it=pRoot->begin(); it!=pRoot->end(); ++it)
	{
		FILENODE_T		&node	= it.getvalue();
		unsigned char	nType;
		if( node.IsDir() )
		{
			// 写BEGIN
			nType		= TYPE_DIRBEGIN;
			if( f->Write(&nType)!=sizeof(nType) )
			{
				return	-11;
			}
			// 写入名字偏移
			int	nOffset	= it.getkey().GetOffset();
			if( f->Write(&nOffset)!=sizeof(nOffset) )
			{
				return	-12;
			}
			// 递归写入目录内容
			if( SaveDirTreeToFile(node.u.d.pContent, f)<0 )
			{
				return	-13;
			}
			// 写END
			nType		= TYPE_DIREND;
			if( f->Write(&nType)!=sizeof(nType) )
			{
				return	-21;
			}
		}
		else
		{
			// 写入类型
			nType		= TYPE_FILE;
			if( f->Write(&nType)!=sizeof(nType) )
			{
				return	-31;
			}
			// 写入名字偏移
			int	nOffset	= it.getkey().GetOffset();
			if( f->Write(&nOffset)!=sizeof(nOffset) )
			{
				return	-32;
			}
			// 写入开始块
			if( f->Write(&node.u.f.nStartBlock)!=sizeof(node.u.f.nStartBlock) )
			{
				return	-33;
			}
		}
	}
	return	0;
}
int		wh_package_base::FreeDirTree(STR2NODE_T *&pRoot)
{
	if( !pRoot )
	{
		// 没有内容不用释放
		return	0;
	}
	// 遍历子节点，逐层释放
	for(STR2NODE_T::kv_iterator it=pRoot->begin(); it!=pRoot->end(); ++it)
	{
		FILENODE_T		&node	= it.getvalue();
		if( node.IsDir() )
		{
			// 递归进去进行释放
			if( FreeDirTree(node.u.d.pContent)<0 )
			{
				return	-1;
			}
		}
		else
		{
			// 文件没有动态申请的内容，所以不用任何操作了
		}
	}

	// 最后释放自己
	delete	pRoot;
	pRoot	= NULL;
	return	0;
}
int		wh_package_base::DelDirTree(STR2NODE_T *&pRoot)
{
	if( !pRoot )
	{
		// 没有内容不用释放
		return	0;
	}
	// 遍历子节点，逐层释放
	STR2NODE_T::kv_iterator it=pRoot->begin();
	while( it!=pRoot->end() )
	{
		FILENODE_T	&node		= it.getvalue();
		STR2NODE_T::kv_iterator	tmpit	= it++;
		if( node.IsDir() )
		{
			// 递归进去进行释放
			if( DelDirTree(node.u.d.pContent)<0 )
			{
				return	-1;
			}
		}
		else
		{
			// 删除文件
			if( DelFileByBlockIdx(node.u.f.nStartBlock)<0 )
			{
				return	-2;
			}
		}
		// 删除这个节点
		if( !pRoot->erase(tmpit) )
		{
			return	-3;
		}
	}
	// 最后释放自己
	delete	pRoot;
	pRoot	= NULL;
	return	0;
}
int		wh_package_base::FindNodeByPathName(const char *cszPath, FINDNODEBYPATHNAME_RST_T *pRst)
{
	// 分解文件名
	char	szOne[WH_MAX_PATH];
	int		nOffset	= 0;
	pRst->pDir		= m_RootNode.u.d.pContent;
	pRst->pFN		= &m_RootNode;
	pRst->nOffset	= 0;
	while( wh_strsplit(&nOffset, "s", cszPath, "\1/\\", szOne)==1 )
	{
		// 在当前节点查找
		STR2NODE_T::kv_iterator	it	= pRst->pDir->find(szOne);
		if( it == pRst->pDir->end() )
		{
			// 提前找到部分。此时pRst->nOffset就是szOne开头的位置，pRst->pDir就是它的上层目录。
			return	FINDNODEBYPATHNAME_RST_PARTIAL;
		}
		pRst->pFN		= &it.getvalue();
		// 看看是不是最后的部分
		if( cszPath[nOffset]==0 )
		{
			// 直接退出即可
			break;
		}
		if( !pRst->pFN->IsDir() )
		{
			// 说明找到路径中间有文件，说明路径非法
			return	FINDNODEBYPATHNAME_RST_ERR_MIDDLEFILEINPATH;
		}
		// 保留本次的基础dir
		pRst->pDir		= pRst->pFN->u.d.pContent;
		// 记录下次的起始偏移
		pRst->nOffset	= nOffset;
	}
	return	FINDNODEBYPATHNAME_RST_OK;
}
wh_package_base::FILENODE_T *	wh_package_base::CreateNodeByPathName(const char *cszPath, STR2NODE_T *pRoot, bool bIsFile)
{
	// 分解文件名
	char	szOne[WH_MAX_PATH];
	int		nOffset		= 0;
	FILENODE_T	*pNode	= NULL;
	while( wh_strsplit(&nOffset, "s", cszPath, "\1/\\", szOne)==1 )
	{
		// 看名字原来是否存在
		STRSET_T::key_iterator	it	= m_setFNAME.find(szOne);
		if( it == m_setFNAME.end() )
		{
			// 说明原来没有应该创建在最后
			int	nLen	= strlen(szOne)+1;
			m_vectFNAME.push_back(szOne, nLen);
			STRINVECT	siv(&m_vectFNAME, m_vectFNAME.size()-nLen);
			it			= m_setFNAME.putgetit(siv);
			assert(it!=m_setFNAME.end());
			// 标记名字vect改变了
			m_bShouldSaveVectorFileName	= true;
		}
		// 插入
		if( !pRoot->putalloc((*it), pNode) )
		{
			// 应该是名字重了（如果是创建path则不应该出现这样的情况）
			assert(0);
			return	NULL;
		}
		// 标记文件信息表发生了改变
		m_bShouldSaveFileList	= true;
		//
		if( cszPath[nOffset]==0 )
		{
			// 说明是最后一个了
			if( !bIsFile )
			{
				pNode->u.d.pContent	= new STR2NODE_T;
				pNode->SetIsDir(true);
				pRoot	= pNode->u.d.pContent;
			}
			else
			{
				// 文件应该上层去分配block
			}
			// 直接返回
			break;
		}
		else
		{
			// 说明这个肯定是目录
			pNode->u.d.pContent	= new STR2NODE_T;
			pNode->SetIsDir(true);
			pRoot	= pNode->u.d.pContent;
		}
	}
	return	pNode;
}
int			wh_package_base::DelFileByBlockIdx(blockoffset_t nStart)
{
	// 以写方式打开文件，删除之
	rawfile		*rf		= NewRawFile();
	try
	{
		int			rst		= rf->Open(whfileman::OP_MODE_BIN_READWRITE, nStart);
		if( rst<0 )
		{
			if( m_info.bNoErrOnDelFile )
			{
				throw	0;
			}
			throw	rst;
		}
		rst					= rf->Delete();	// 这里面会删除文件指针的
		if( rst<0 )
		{
			return	-100 + rst;
		}
	}
	catch(int nRst)
	{
		WHSafeSelfDestroy(rf);
		return	nRst;
	}
	return	0;
}
whfile *	wh_package_base::OpenFile(const char *szFile, whfileman::OP_MODE_T nOPMode, blockoffset_t *pnStart)
{
	if( IsReadOnly() && !whfileman::IsReadOnlyMode(nOPMode) )
	{
		// 不能对只读包做写操作
		return	NULL;
	}
	// 先根据文件名看是否可以找到相应的raw文件
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szFile, &Rst);
	switch( rst )
	{
	case	FINDNODEBYPATHNAME_RST_OK:
	case	FINDNODEBYPATHNAME_RST_PARTIAL:
		break;
	default:
		// 没有成功找到全部或部分（可能是路径中有文件名）
		return	NULL;
	}
	// 创建底层文件
	rawfile		*rf		= NewRawFile();
	file4out	*fpo	= new file4out(this);
	try
	{
		if( !rf || !fpo )
		{
			throw	-1;
		}
		switch( nOPMode )
		{
		case	whfileman::OP_MODE_BIN_CREATE:
			{
				if( rst==FINDNODEBYPATHNAME_RST_OK )
				{
					if( Rst.pFN->IsDir() )
					{
						// 创建文件不能覆盖目录
						throw	-2;
					}
					// 原来文件存在，则需要先删除之
					if( DelFileByBlockIdx(Rst.pFN->u.f.nStartBlock)<0 )
					{
						throw	-3;
					}
				}
				else
				{
					// 创建后面部分的目录和文件
					Rst.pFN	= CreateNodeByPathName(szFile+Rst.nOffset, Rst.pDir, true);
					if( !Rst.pFN )
					{
						throw	-11;
					}
				}
				// 因为文件起始块可能改变了（虽说按照俺现在的释放方式应该不会发生改变），所以需要重新存储文件信息表
				m_bShouldSaveFileList	= true;
			}
			break;
		case	whfileman::OP_MODE_BIN_READWRITE:
		case	whfileman::OP_MODE_BIN_READONLY:
		case	whfileman::OP_MODE_BIN_RAWREAD:
			{
				if( rst==FINDNODEBYPATHNAME_RST_PARTIAL )
				{
					// 这种情况就必须文件存在
					throw	-21;
				}
				if( Rst.pFN->IsDir() )
				{
					// 目标不能是目录
					throw	-22;
				}
			}
			break;
		default:
				throw	-23;
			break;
		}
		// 打开或创建raw文件（如果是创建则会改变Rst.pFN->u.f.nStartBlock）
		int	rst	= rf->Open(nOPMode, Rst.pFN->u.f.nStartBlock);
		if( rst<0 )
		{
			throw	-31;
		}
		// 现在raw文件已经有了，可以建立上层压缩文件了
		file4out::INFO_T	info;
		info.nOPMode		= nOPMode;
		info.pBaseFile		= rf;
		info.pWHCompress	= m_info.pWHCompress;
		info.pEncryptor		= m_info.pEncryptor;
		info.pDecryptor		= m_info.pDecryptor;
		info.bAutoCloseBaseFile	= true;
		if( fpo->Open(&info)<0 )
		{
			// 因为是自动关闭basefile，所以这里就把rf清空了
			rf		= NULL;
			throw	-41;
		}
		if( pnStart )
		{
			*pnStart	= Rst.pFN->u.f.nStartBlock;
		}
		return	fpo;
	}
	catch (int)
	{
		WHSafeSelfDestroy(fpo);
		WHSafeSelfDestroy(rf);
		return	NULL;
	}
}
WHDirBase *		wh_package_base::OpenDir(const char *szDir)
{
	// 通过目录名找到相应的节点
	// 先根据文件名看是否可以找到相应的raw文件
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szDir, &Rst);
	if( rst!=FINDNODEBYPATHNAME_RST_OK )
	{
		// 没有成功找到全部（应该是路径不对）
		return	NULL;
	}
	char	szBaseDir[WH_MAX_PATH];
	sprintf(szBaseDir, "%s/%s", m_info.szDir, szDir);
	return	new dir4out(Rst.pFN->u.d.pContent, szBaseDir);
}
int			wh_package_base::MakeDir(const char *szDir)
{
	if( IsReadOnly() )
	{
		// 不能对只读包做写操作
		return	-1;
	}
	// 先找到前面有多少已经建立了
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szDir, &Rst);
	switch( rst )
	{
	case	FINDNODEBYPATHNAME_RST_OK:
		{
			// 说明已经建立了
			// 看看是不是目录
			if( Rst.pFN->IsDir() )
			{
				// 已经存在了，就不用创建了
				return	0;
			}
			else
			{
				// 原来存在的是一个文件，不能在这个地方创建目录
				return	-2;
			}
		}
		break;
	case	FINDNODEBYPATHNAME_RST_PARTIAL:
		{
			// 创建后面的部分
			FILENODE_T	*pFN	= CreateNodeByPathName(szDir+Rst.nOffset, Rst.pDir, false);
			if( !pFN )
			{
				return	-11;
			}
			else
			{
				// 创建成功
				return	0;
			}
		}
		break;
	default:
		// 没有成功找到全部或部分（可能是路径中有文件名）
		return	-100;
	}
}
int			wh_package_base::DelPath(const char *szFile)
{
	if( IsReadOnly() )
	{
		// 不能对只读包做写操作
		return	-1;
	}
	// 先根据文件名看是否可以找到相应的目录或文件
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szFile, &Rst);
	if( rst != FINDNODEBYPATHNAME_RST_OK )
	{
		// 没有成功找到
		return	-2;
	}
	if( !Rst.pFN->IsDir() )
	{
		// 按文件删除
		// 先删除文件block
		if( DelFileByBlockIdx(Rst.pFN->u.f.nStartBlock)<0 )
		{
			return	-11;
		}
	}
	else
	{
		// 是目录则按目录递归删除
		rst	= DelDirTree(Rst.pFN->u.d.pContent);
		if( rst<0 )
		{
			return	-100 + rst;
		}
	}
	// 标记文件信息表发生了改变
	m_bShouldSaveFileList	= true;
	// 删除上层的名字映射(对应的pContent已经在DelDirTree中被删除了)
	if( !Rst.pDir->erase(szFile+Rst.nOffset) )
	{
		// 都找到了应该不会删除失败的
		assert(0);
		return	-31;
	}
	return	0;
}
int			wh_package_base::GetPathInfo(const char *cszPath, whfileman::PATHINFO_T *pPathInfo)
{
	pPathInfo->nType	= 0;
	// 先根据文件名看是否可以找到相应的目录或文件
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(cszPath, &Rst);
	if( rst != FINDNODEBYPATHNAME_RST_OK )	// 想获取info那么原来文件或者目录就必须存在（不能是partial的）
	{
		// 没有成功找到
		return	-1;
	}
	if( Rst.pFN->IsDir() )
	{
		pPathInfo->nType	|= whfileman::PATHINFO_T::TYPE_DIR;
		pPathInfo->nMTime	= 0;	// 包里面的目录时间没有意义
	}
	else
	{
		// 不是目录，那么获得文件基本信息（时间、长度）
		// 创建底层文件
		rawfile		*rf		= NewRawFile();
		file4out	*fpo	= new file4out(this);
		try
		{
			if( !rf || !fpo )
			{
				throw	-20;
				if( rst==FINDNODEBYPATHNAME_RST_PARTIAL )
				{
					// 这种情况就必须文件存在
					throw	-21;
				}
				if( Rst.pFN->IsDir() )
				{
					// 目标不能是目录
					throw	-22;
				}
			}
			// 打开或创建raw文件（如果是创建则会改变Rst.pFN->u.f.nStartBlock）
			int	rst	= rf->Open(whfileman::OP_MODE_BIN_READONLY, Rst.pFN->u.f.nStartBlock);
			if( rst<0 )
			{
				throw	-31;
			}
			// 现在raw文件已经有了，可以打开上层压缩文件获取信息了
			file4out::INFO_T	info;
			info.nOPMode		= whfileman::OP_MODE_BIN_READONLY;
			info.pBaseFile		= rf;
			info.pWHCompress	= m_info.pWHCompress;
			info.pEncryptor		= m_info.pEncryptor;
			info.pDecryptor		= m_info.pDecryptor;
			info.bAutoCloseBaseFile	= true;
			if( fpo->GetPathInfo(&info, pPathInfo)<0 )
			{
				// 因为是自动关闭basefile，所以这里就把rf清空了
				rf		= NULL;
				throw	-41;
			}
			WHSafeSelfDestroy(fpo);
			return	0;
		}
		catch (int nErr)
		{
			WHSafeSelfDestroy(fpo);
			WHSafeSelfDestroy(rf);
			return	nErr;
		}
		return	0;
	}
	return	0;
}
int			wh_package_base::GetBlockList(blockoffset_t nStart, whvector<blockoffset_t> &vect)
{
	vect.clear();
	if( SeekToBlockHdr(nStart)<0 )
	{
		return	-1;
	}
	BLOCK_HDR_T		CurBlockHdr;
	if( GetBaseFile()->Read(&CurBlockHdr) != sizeof(CurBlockHdr) )
	{
		return	-2;
	}
	vect.push_back(nStart);
	while( CurBlockHdr.nNext != 0 )
	{
		vect.push_back(CurBlockHdr.nNext);
		if( (int)vect.size()>=m_package_hdr.nMaxBlockIdx )
		{
			// 给的nStart可能是非法的
			return	-2;
		}
		if( SeekToBlockHdr(CurBlockHdr.nNext)<0 )
		{
			return	-3;
		}
		if( GetBaseFile()->Read(&CurBlockHdr) != sizeof(CurBlockHdr) )
		{
			return	-4;
		}
	}
	return	0;
}

// 返回包中文件的原始数据（压缩后的数据），钟文杰添加
whfile * wh_package_base::OpenRawFile(const char *szFile, whfileman::OP_MODE_T nOPMode, blockoffset_t *pnStart/* =NULL */)
{
	if( IsReadOnly() && !whfileman::IsReadOnlyMode(nOPMode) )
	{
		// 不能对只读包做写操作
		return	NULL;
	}
	// 先根据文件名看是否可以找到相应的raw文件
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szFile, &Rst);
	switch( rst )
	{
	case	FINDNODEBYPATHNAME_RST_OK:
	case	FINDNODEBYPATHNAME_RST_PARTIAL:
		break;
	default:
		// 没有成功找到全部或部分（可能是路径中有文件名）
		return	NULL;
	}
	// 创建底层文件
	rawfile		*rf		= NewRawFile();
	try
	{
		if( !rf )
		{
			throw	-1;
		}
		switch( nOPMode )
		{
		case	whfileman::OP_MODE_BIN_CREATE:
			{
				if( rst==FINDNODEBYPATHNAME_RST_OK )
				{
					if( Rst.pFN->IsDir() )
					{
						// 创建文件不能覆盖目录
						throw	-2;
					}
					// 原来文件存在，则需要先删除之
					if( DelFileByBlockIdx(Rst.pFN->u.f.nStartBlock)<0 )
					{
						throw	-3;
					}
				}
				else
				{
					// 创建后面部分的目录和文件
					Rst.pFN	= CreateNodeByPathName(szFile+Rst.nOffset, Rst.pDir, true);
					if( !Rst.pFN )
					{
						throw	-11;
					}
				}
				// 因为文件起始块可能改变了（虽说按照俺现在的释放方式应该不会发生改变），所以需要重新存储文件信息表
				m_bShouldSaveFileList	= true;
			}
			break;
		case	whfileman::OP_MODE_BIN_READWRITE:
		case	whfileman::OP_MODE_BIN_READONLY:
		case	whfileman::OP_MODE_BIN_RAWREAD:
			{
				if( rst==FINDNODEBYPATHNAME_RST_PARTIAL )
				{
					// 这种情况就必须文件存在
					throw	-21;
				}
				if( Rst.pFN->IsDir() )
				{
					// 目标不能是目录
					throw	-22;
				}
			}
			break;
		default:
			throw	-23;
			break;
		}
		// 打开或创建raw文件（如果是创建则会改变Rst.pFN->u.f.nStartBlock）
		int	rst	= rf->Open(nOPMode, Rst.pFN->u.f.nStartBlock);
		if( rst<0 )
		{
			throw	-31;
		}
		return	rf;
	}
	catch (int)
	{
		WHSafeSelfDestroy(rf);
		return	NULL;
	}
}

