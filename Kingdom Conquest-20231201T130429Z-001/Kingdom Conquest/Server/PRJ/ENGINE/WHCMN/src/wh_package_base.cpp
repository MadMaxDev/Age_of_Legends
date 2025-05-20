// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_base.cpp
// Creator     : Wei Hua (κ��)
// Comment     : �����İ��ṹ
// CreationDate: 2006-07-18
// ChangeLOG   : 2006-07-31 ȡ����һЩ�����Ͽ��ܳ��ֵ�assert����Ϊ������assert���ܵ��°��ļ���Ҫ������Ϣû�б��档
//             : 2007-04-02 �����˸���дһ���Ѿ����ڵ��ļ������ļ��𻵵����⡣ԭ��raw�ļ���ԭ��������д��С���ȣ�С��һ��block�������ڵ�ǰҳû��д���������������һҳȥд������ԭ�����ļ����ݱ������ļ�ͷ����
//			   : 2008-06-06	����һ���ӿڣ�����ֱ�Ӷ�ȡԭʼ��ѹ�����ݣ�by ���Ľܣ�
//			   : 2008-06-06	��������rawfile::Read�У������ȡ����ʱ�ļ�ָ��պô���ĳһ��ĩβ����ָ��û����ȷ�ƶ���by ���Ľܣ�

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
	// ��¼��ģʽ
	m_nOPMode	= nOPMode;
	if( !IsReadOnly() && m_pPB->IsReadOnly() )
	{
		// ���������д�򿪵����ܽ���д��
		return	-1;
	}
	// �жϴ�ģʽ
	if( m_nOPMode == whfileman::OP_MODE_BIN_CREATE )
	{
		// ���ļ��ʼ��Ȼ��ʲôҲû�еģ�����Ҳ��Ҫһ����ʼ�飨��Ϊ������ֻ���ȴ����ļ���Ȼ�����ʼ���¼���ļ���Ϣ���У�
		nStart	= m_pPB->AllocBlock();
		if( nStart==0 )
		{
			return	-2;
		}
		// д���ͷ
		m_CurBlockHdr.nPrev				= 0;
		m_CurBlockHdr.nNext				= 0;
		if( m_pPB->GetBaseFile()->Write(&m_CurBlockHdr) != sizeof(m_CurBlockHdr) )
		{
			return	-3;
		}
		// �׿�ĸ�������
		memset(&m_FirstBlockExt, 0, sizeof(FIRST_BLOCK_EXT_T));
		m_nStart						= nStart;
		m_FirstBlockExt.nEnd			= nStart;
		// �ļ�����
		m_FirstBlockExt.nFileSize		= 0;
		m_FirstBlockExt.nFileTime		= wh_time();
		// д�븽�����ݿ�
		if( m_pPB->GetBaseFile()->Write(&m_FirstBlockExt) != sizeof(FIRST_BLOCK_EXT_T) )
		{
			return	-4;
		}
		// ��ǰ�ļ���
		m_nCurBlock						= GetStart();
	}
	else
	{
		m_nStart		= nStart;
		// Seek����һ��
		if( SeekToBlockAndReadHdr(nStart)<0 )
		{
			return	-11;
		}
		if( m_CurBlockHdr.nPrev != 0 )
		{
			// ˵����������ļ��ĵ�һ��
			return	-12;
		}
		// ������������
		if( m_pPB->GetBaseFile()->Read(&m_FirstBlockExt) != sizeof(FIRST_BLOCK_EXT_T) )
		{
			return	-13;
		}
#ifdef	_DEBUG
		// У��һ���ļ��Ƿ���ȷ
		// ���ļ����ȼ����ļ��ܿ�������һ��
		int nOffsetInBlock, nBlockNum;
		CalcBlocknumAndOffsetinblock(FileSize(), &nBlockNum, &nOffsetInBlock);
		// ����һ������
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
		// ��������Ļ�nBlockNumӦ�����ʣ����0
		if( nBlockNum!=0 )
		{
			return	-16;
		}
		if( m_nCurBlock != GetEnd() )
		{
			return	-17;
		}
		// ��Ϊ�����Read��Write��������Seek����������Ͳ�������Seek���ļ���ͷ��
		// ����Ӧ�����°�ָ��ָ��ͷ
		m_nCurBlock		= GetStart();
#endif
	}
	// ���ļ�ָ��ָ��ͷ
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
			// Ӧ�����±����һ����ĸ�����Ϣ
			if( IsReadOnly() )
			{
				// ����ֻ���ļ�Ӧ�ò��ᷢ�������������
				assert(0);
				return	-1;
			}
			// Seek���ļ�ͷ
			if( SeekToBlockAndReadHdr(GetStart())<0 )
			{
				return	-2;
			}
			// д����Ϣ
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
		// Ϊ�˱��հ�m_pPB���˰�
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
	// �ͷŴ�Start��End�Ŀ�
	if( m_pPB->FreeBlock(GetStart(), GetEnd())<0 )
	{
		return	-2;
	}
	// ��հ�ָ�룬��������ٽ����κβ���
	m_pPB	= NULL;
	// ɾ���Լ�
	delete	this;
	return	0;
}
int		wh_package_base::rawfile::Extend(int nSize)
{
	if( IsReadOnly() )
	{
		return	-1;
	}
	// ��¼һ�µ�ǰ��block��offset
	blockoffset_t	nCurBlock	= m_nCurBlock;
	int				nCurOffset	= m_nCurOffset;
	bool			bLastByteSpecial	= false;
	// �����ǰλ�����ļ���β���ڽӷ촦����ô���һ��
	if( m_nCurOffset == FileSize() && (m_nCurOffset+sizeof(FIRST_BLOCK_EXT_T))%GetBlockDataMaxSize()==0 )
	{
		bLastByteSpecial		= true;
	}
	// �����չ��ʱ���ļ�ָ��պ��ڿ�ӷ촦
	int	rst	= Seek(nSize, SEEK_END);
	if( rst<0 )
	{
		return	-10+rst;
	}
	if( bLastByteSpecial )
	{
		// ��Ϊ����չ�ˣ�����m_nCurBlock��Ҫ���������һ�飬����ƫ����0
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
	// �ϲ�Ӧ�ñ�֤nSize>0��
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
		// �����ļ���β�Ͳ�Ӧ���ٶ���
		return	0;
	}

	int		nTotal	= 0;

	while (nSize > 0)
	{
		// �����¿鶼��Ҫ���������ļ���Seekһ��
		int	nOffsetInBlock;
		if( SeekToMyCurPos(&nOffsetInBlock)<0 )
		{
			return	-2;
		}

		// ��һ�µ�ǰ���ڻ�ʣ������ֽ�
		int		nDataSizeLeft;
		if( !IsCurBlockLastOne() )
		{
			// �ǵ�һ����м�һ�飨��ΪnOffsetInBlock��ÿ���е����嶼��ͬ���ڵ�һ����Ҳ�Ǽ�����FIRST_BLOCK_EXT_T�ĳ��ȣ����Զ���GetBlockDataMaxSize���ɣ�
			nDataSizeLeft	= GetBlockDataMaxSize() - nOffsetInBlock;

			if (nDataSizeLeft <= 0)
			{
				assert(0 && "�����������ǰ���ж���IsEOF()������£��ǲ������nDataSizeLeftΪ0�������");
				break;
			}

			int nReadSize = (nSize <= nDataSizeLeft) ? nSize : nDataSizeLeft;
			int	rst	= m_pPB->GetBaseFile()->Read(pBuf, nReadSize);
			if( rst<0 )
			{
				// ��ȡʧ��
				return	-10;
			}
			// �ƶ��ļ�ָ��
			m_nCurOffset		+= rst;
			// �ܶ���
			nTotal				+= rst;
			// ��������ʣ����Ҫ���Ĵ�С���ı�һ��
			nSize				-= rst;
			pBuf				= wh_getoffsetaddr(pBuf, rst);

			if (rst == nDataSizeLeft)
			{
				// ׼������һ��
				m_nCurBlock			= m_CurBlockHdr.nNext;
			}
		}
		else
		{
			// �����һ��
			nDataSizeLeft	= FileSize() - m_nCurOffset;

			// ֱ�Ӷ����˷��ؼ���
			int	rst	= m_pPB->GetBaseFile()->Read(pBuf, (nSize<=nDataSizeLeft) ? nSize : nDataSizeLeft);
			if( rst<0 )
			{
				// ��ȡʧ��
				return	-10;
			}
			// �ƶ��ļ�ָ��
			m_nCurOffset		+= rst;
			// �ܶ���
			nTotal				+= rst;
			break;
		}
		//// ��Ҫ��ȡ������ʣ�������
		//if( nDataSizeLeft>0 )
		//{
		//	int	rst	= m_pPB->GetBaseFile()->Read(pBuf, nDataSizeLeft);
		//	if( rst<0 )
		//	{
		//		// ��ȡʧ��
		//		return	-11;
		//	}
		//	// �ƶ��ļ�ָ��
		//	m_nCurOffset		+= rst;
		//	// �ܶ���
		//	nTotal				+= rst;
		//	// ��������ʣ����Ҫ���Ĵ�С���ı�һ��
		//	nSize				-= rst;
		//	pBuf				= wh_getoffsetaddr(pBuf, rst);
		//}
		//else
		//{
		//	// �����������ǰ���ж���IsEOF()������£��ǲ������nDataSizeLeftΪ0�������
		//	assert(0);
		//	return	-12;
		//}
		//// �ж��Ƿ����ļ���β
		//if( m_CurBlockHdr.nNext == 0 )
		//{
		//	assert(m_nCurOffset == FileSize());
		//	// ���ˣ�ֱ�ӷ��ص�ǰ�����ĳ���
		//	return	nTotal;
		//}
		//// ׼������һ��
		//m_nCurBlock			= m_CurBlockHdr.nNext;
		//// ������ȡ
	}

	return	nTotal;
}
int		wh_package_base::rawfile::Seek(int nOffset, int nOrigin)
{
	// ����������Seek���ն�ת��Ϊ�Ƚ��ײ�Seek��һ�㣬Ȼ���������Seek
	// Seekֻ�ᶨλ���ض�block�������ᶨλ�������offset��������Read��Write�ڲ������ȵ���һ��SeekToMyCurPos�Ա�֤BaseFile��ָ����ȷ
	switch( nOrigin )
	{
	case	SEEK_CUR:
		{
			// ��Ϊ�ײ��ļ�ֻ��һ���������ڴ򿪶���ļ���ʱ�򣬺��ѱ�֤����ļ���д�������ı�ײ��ļ���ָ�룬���Լ�����SEEK_CURҲҪ������Seek����ǰλ��
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
		// �Ѿ����ط��ˣ����ü�������
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
		// ���ȿ��Ƿ�ᳬ���ļ�����
		if( nDstOffset < FileSize() )
		{
			// һ�����ڵ�ǰ�ļ����з�Χ�ڽ��
			// ����Ӧ�ð�ָ���ƶ�����һ��
			// nDstBlockNum�϶�>=nCurBlockNum
			// ���Seek���ɸ�block
			for(;nCurBlockNum<nDstBlockNum;++nCurBlockNum)
			{
				assert(m_CurBlockHdr.nNext>0);
				SeekToBlockAndReadHdr(m_CurBlockHdr.nNext);
			}
			// ���������һ���ڽ���ˣ��������һ��ָ�뼴�ɣ�
			m_nCurOffset	= nDstOffset;
			return	0;
		}
		// ��Ϊ�ᳬ���ļ����ȣ������൱�ڴ����һ��λ�ÿ�ʼ����ٶ�SeekһЩ
		// ��Seek����β
		int	rst	= SeekToBlockAndReadHdr(GetEnd());
		if( rst<0 )
		{
			return	-21;
		}
		m_nCurOffset		= FileSize();
		// ���Ƿ�պõ����ļ�����
		if( nDstOffset == FileSize() )
		{
			// �պõ��ڣ��Ͳ�����������Ĺ�����
			return	0;
		}
		// �ж��ļ��Ƿ�ֻ��
		if( IsReadOnly() )
		{
			// ��ô�͵���Ϊֹ��
			return	0;
		}
		// ������ܹ���Ҫ���Ӷ��ٿ飬������Щ��
		while( nCurBlockNum<nDstBlockNum )
		{
			m_CurBlockHdr.nNext	= m_pPB->AllocBlock();
			if( m_CurBlockHdr.nNext == 0 )
			{
				return	-31;
			}
			// ������һ���飨Ҳ��������¿�ģ�ͷ����Ϊ������һ�������һ�飬������ѭ���ˣ�
			BLOCK_HDR_T		BlockHdr;
			BlockHdr.nPrev	= m_nCurBlock;
			BlockHdr.nNext	= 0;
			if( m_pPB->GetBaseFile()->Write(&BlockHdr)!=sizeof(BlockHdr) )
			{
				return	-32;
			}
			// ��ǰ��ͷ����
			SeekToBlockHdr(m_nCurBlock);
			if( m_pPB->GetBaseFile()->Write(&m_CurBlockHdr)!=sizeof(m_CurBlockHdr) )
			{
				return	-33;
			}
			// ����һ��Ϊ��ǰ��
			// ��Seek����
			SeekToBlockAndReadHdr(m_CurBlockHdr.nNext);
			// ��һ��
			nCurBlockNum	++;
		}
		// �����Ѿ��������һ����
		m_FirstBlockExt.nEnd		= m_nCurBlock;
		// ����ָ��
		m_nCurOffset				= nDstOffset;
		// �ı��ļ�����
		m_FirstBlockExt.nFileSize	= m_nCurOffset;
		m_bShouldSaveFirstBlockExt	= true;
		// �޸�һ���ļ�ʱ��
		SetFileTime(0);
	}
	else
	{
		// ��ǰ
		if( nDstOffset<=0 )
		{
			// ��ô���൱��Seek���ļ�ͷ
			SeekToBlockAndReadHdr(GetStart());
			m_nCurOffset			= 0;
			return	0;
		}
		// Ŀ�ĵ㲻�ᳬ���ļ�ͷ
		// ��ô����һ�¸���ǰ�������ٿ�
		while( nCurBlockNum>nDstBlockNum )
		{
			SeekToBlockAndReadHdr(m_CurBlockHdr.nPrev);
			nCurBlockNum			--;
		}
		// ���ˣ��������һ��ָ��λ��
		m_nCurOffset				= nDstOffset;
	}

	return	0;
}
bool	wh_package_base::rawfile::IsEOF()
{
	// ֮������">="������Ϊ����Seek��ʵ�ֻᵼ��readonlyʱ���ļ�ָ���ƶ����ļ�����֮�⣨����linuxϵͳ��fseek���������������һ�û�о�������ʵ�֣�
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
	// ��д�������Ƿ����ļ����ȷ�Χ��
	if( nDstOffset > FileSize() )
	{
		// ��չ�ļ����ȣ������Ͳ�����д�Ĺ�������չ�ļ��������ˣ�
		// Extend���ı䵱ǰ��Offset�����ԾͲ��ü�¼���ڵ�m_nCurBlock��m_nCurOffset��ز�����
		if( Extend(nDstOffset-FileSize())<0 )
		{
			return	-11;
		}
	}

	// ������û�������ļ���������
	int	nCurBlockNum;
	int	nCurOffsetInBlock;
	CalcBlocknumAndOffsetinblock(m_nCurOffset, &nCurBlockNum, &nCurOffsetInBlock);
	int	nDstBlockNum;
	int	nDstOffsetInBlock;
	CalcBlocknumAndOffsetinblock(nDstOffset, &nDstBlockNum, &nDstOffsetInBlock);
	
	// �������Readд����
	int	nTotal	= 0;

	while( nSize>0 )
	{
		// �Ƚ��ײ��ļ�Seek����ǰλ��
		// �����¿鶼��Ҫ���������ļ���Seekһ��
		int	nOffsetInBlock;
		if( SeekToMyCurPos(&nOffsetInBlock)<0 )
		{
			return	-12;
		}
		// ��һ�µ�ǰ���ڻ�ʣ������ֽ�
		int		nDataSizeLeft;
		if( IsCurBlockLastOne() )
		{
			// �����һ��
			nDataSizeLeft	= FileSize() - m_nCurOffset;
		}
		else
		{
			// �ǵ�һ����м�һ�飨��ΪnOffsetInBlock��ÿ���е����嶼��ͬ���ڵ�һ����Ҳ�Ǽ�����FIRST_BLOCK_EXT_T�ĳ��ȣ����Զ���GetBlockDataMaxSize���ɣ�
			nDataSizeLeft	= GetBlockDataMaxSize() - nOffsetInBlock;
		}
		if( nSize<nDataSizeLeft )	// ��Ӧ����д�����ų��ֵ����
		{
			nDataSizeLeft	= nSize;
		}
		// ��ȡʣ�������
		int	rst	= m_pPB->GetBaseFile()->Write(pBuf, nDataSizeLeft);
		if( rst<0 )
		{
			// ��ȡʧ��
			return	-13;
		}
		// �ܶ���
		nTotal				+= rst;
		nSize				-= rst;
		pBuf				= wh_getoffsetaddr(pBuf, rst);
		// �ƶ��ļ�ָ��
		m_nCurOffset		+= rst;
		if( nSize==0 )
		{
			// �˳�ѭ��
			break;
		}
		else if( m_CurBlockHdr.nNext>0 )
		{
			// ��һ��
			m_nCurBlock		= m_CurBlockHdr.nNext;
		}
	}

	// �޸�һ���ļ�ʱ��
	SetFileTime(0);

	return	nTotal;
}
int		wh_package_base::rawfile::Flush()
{
	// ��û�п��Ǻ���ʲô�����ÿ�ζ�����BaseFile��Flush�᲻�᲻�ð���
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
	// ��Щ������ÿ��һ����Ŀ�ͷ����������
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
	// ʲôҲ����������Ϊû��new�Ķ���
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
	++m_it;	// ��һ��
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
// ����ļ��Ƿ��ǰ��ļ�
wh_package_base::enumFILEISWHAT	wh_package_base::FileIsWhat(whfile *fp)
{
	PACKAGE_HDR_T	m_package_hdr;
	// �����ļ�ͷ��
	if( fp->Read(&m_package_hdr)==sizeof(m_package_hdr) )
	{
		// �жϿ�ͷ�Ƿ�����
		if( m_package_hdr.IsOK() )
		{
			return	wh_package_base::FILEISWHAT_PCK;
		}
	}
	// �����ļ��Ƿ���β�����ļ�
	whtailfile	tf;
	if( tf.Open(fp, false, whfileman::OP_MODE_BIN_READONLY)==0 )
	{
		if( wh_package_base::FileIsWhat(&tf)==wh_package_base::FILEISWHAT_PCK )
		{
			return	wh_package_base::FILEISWHAT_TAILPCK;
		}
	}
	// tfӦ�û���������ʱ��رգ�����ֻ��Ҳûɶ������ģ�
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
// ��ʵ��
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
	// �����ʼ����Ϣ������ܶ�ط�Ҫ�ã����磺GetBaseFile
	memcpy(&m_info, pInfo, sizeof(m_info));
	if( GetBaseFile()->Seek(0, SEEK_SET)!=0 )
	{
		return	-2;
	}
	unsigned char	MD5[16];
	whmd5(m_info.szPass, m_info.nPassLen, MD5);

	// ����һ�¼������ͽ�����������
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
			// ��дͷ�����ݣ����������MakeRawBlock֮ǰ��д����ΪMakeRawBlock�����ʹ�õ�m_package_hdr.nBlockSize��
			m_package_hdr.reset();
			m_package_hdr.nBlockSize	= m_info.nBlockSize;
			memcpy(m_package_hdr.PassMD5, MD5, sizeof(MD5));
			// �����ļ�ͷ��
			if( MakeRawBlock(0)<0 )
			{
				return	-11;
			}
			// �����ļ���Ϣ����ļ��������ļ���ֻ��Ϊ�˹رյ�ʱ����ͳһֻ��д����������������������
			// �ļ���Ϣ��
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
				// ����Ŀ¼���ڵ�
				m_RootNode.SetIsDir(true);
				m_RootNode.u.d.pContent	= new STR2NODE_T;
				if( !m_RootNode.u.d.pContent )
				{
					throw	-14;
				}
				// �ļ�����
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
				// ����ļ���vector
				m_vectFNAME.clear();
				// ��Ϊ���´������ļ����������ͷһ��Ҫ���̵ģ�����Ҫ�γɿ��ļ���
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
			// �����ļ�ͷ��
			if( GetBaseFile()->Read(&m_package_hdr)!=sizeof(m_package_hdr) )
			{
				return	-21;
			}
			// У��ͷ�Ƿ�Ϸ�
			if( !m_package_hdr.IsOK() )
			{
				return	-22;
			}
			// У�������Ƿ���ȷ
			if( memcmp(m_package_hdr.PassMD5, MD5, sizeof(MD5))!=0 )
			{
				return	-23;
			}
			m_info.nBlockSize	= m_package_hdr.nBlockSize;
			int	nBlockNum	= GetBaseFile()->FileSize() / m_info.nBlockSize;
			// У���ļ������Ƿ�����
			if( m_info.bCheckFileBlockNumOnOpen )
			{
				if( nBlockNum!=m_package_hdr.nMaxBlockIdx )
				{
					return	-24;
				}
			}
			// ���ļ���Ϣ����ļ�����
			wh_package_base::rawfile	*f	= NULL;
			wh_package_singlefile		*sc	= NULL;
			try
			{
				// Ӧ���ȶ��ļ�������Ϊ�ļ�����������Ҫ�����ļ�����
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
				// һ��һ����������m_setFNAME����
				int	i=0,j=0;
				while( j<(int)m_vectFNAME.size() )
				{
					if( m_vectFNAME[j]==0 )
					{
						STRINVECT	siv(&m_vectFNAME, i);
						if( !m_setFNAME.put(siv) )
						{
							// ˵���������ˣ�����ǲ�Ӧ�õ�
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
				// �ر��ļ�
				WHSafeSelfDestroy(sc);
				WHSafeSelfDestroy(f);
			
				// Ȼ���ȡ�ļ�������
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
				// �������ڵ�������ݲ���֯Ŀ¼��
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
				// �ر��ļ�
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
		// �������������ļ�
		SaveInfoFiles();
		// �����ͷ
		SavePackageHdr();
	}
	// ɾ�������ļ��ڵ�
	FreeDirTree(m_RootNode.u.d.pContent);
	// �������vector
	m_vectFNAME.clear();
	if( m_info.bAutoCloseBaseFile )
	{
		WHSafeSelfDestroy(m_info.pBaseFile);
	}
	// �ɹ��ͷ��˲��������ʼ�����
	m_bInited	= false;
	return	0;
}
int		wh_package_base::SavePackageHdr()
{
	if( !IsReadOnly() )
	{
		// ���package_hdr�ı��˾ͱ���֮
		if( m_bShouldSavePackageHdr )
		{
			// Seek��package�ļ�ͷ
			if( GetBaseFile()->Seek(0, SEEK_SET)<0 )
			{
				return	-1;
			}
			// д֮
			if( GetBaseFile()->Write(&m_package_hdr)!=sizeof(m_package_hdr) )
			{
				return	-2;
			}
			// Ӧ�������д�ɹ�֮��Ÿı���
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
		// �ļ���������ı��ˣ�
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
			// �ر��ļ�
			WHSafeSelfDestroy(sc);
			WHSafeSelfDestroy(f);
			// ��ձ��
			m_bShouldSaveVectorFileName	= false;
		}
		// �ļ���Ϣ�б�����ı��ˣ�
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
			// �ر��ļ�
			WHSafeSelfDestroy(sc);
			WHSafeSelfDestroy(f);
			// ��ձ��
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
	// ����ͷ
	if( SavePackageHdr()<0 )
	{
		return	-1;
	}
	// �������������ļ�
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
	// �ȿ����ñ�����û��
	blockoffset_t	nIdx	= m_package_hdr.nAvailIdx;
	if( nIdx>0 )
	{
		// �ӱ����
		if( SeekToBlockHdr(nIdx)<0 )
		{
			return	0;
		}
		// ����ͷ��
		BLOCK_HDR_T	hdr;
		if( GetBaseFile()->Read(&hdr) != sizeof(hdr) )
		{
			return	0;
		}
		// ��Seek�ؿ�ͷ�����ǹ涨���ϲ��ڻ�ÿ�֮����ܻ�����д�飬�����ϲ��Ҫ���û��⹤������������߳̿��ܻ������⣩
		if( GetBaseFile()->Seek(-(int)(sizeof(hdr)), SEEK_CUR)!=0 )
		{
			return	0;
		}
		// avail��ָ����һ��
		m_package_hdr.nAvailIdx	= hdr.nNext;
		// ���package_hdr���޸���
		m_bShouldSavePackageHdr	= true;
		// ���ط��䵽�Ŀ��
		return	nIdx;
	}
	// ��Ҫͨ����չ�ļ������¿�
	// ��¼�¿��
	nIdx	= m_package_hdr.nMaxBlockIdx	++;
	// Seek��չ
	if( MakeRawBlock(nIdx)<0 )
	{
		return	0;
	}
	// ���ļ�ָ���Ƶ��¿�ͷ
	if( SeekToBlockHdr(nIdx)<0 )
	{
		return	0;
	}
	// ���package_hdr���޸���
	m_bShouldSavePackageHdr		= true;
	return	nIdx;
}
int		wh_package_base::MakeRawBlock(blockoffset_t nBlock)
{
	if( GetBaseFile()->Seek((nBlock+1)*m_package_hdr.nBlockSize-1, SEEK_SET)!=0 )
	{
		return	-1;
	}
	// д�����һ�ֽ�ʹ��Ч
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
	// 0�Ͳ����ͷ���
	if( nStart==0 )
	{
		return	0;
	}
	if( SeekToBlockHdr(nEnd)<0 )
	{
		return	-1;
	}
	// ����End��ͷ
	BLOCK_HDR_T	hdr;
	if( GetBaseFile()->Read(&hdr)!=sizeof(hdr) )
	{
		return	-2;
	}
	// �޸�
	hdr.nNext	= m_package_hdr.nAvailIdx;
	// �����޸Ĺ������End��ͷ
	if( GetBaseFile()->Seek(-(int)sizeof(hdr), SEEK_CUR)!=0 )
	{
		return	-3;
	}
	if( GetBaseFile()->Write(&hdr)!=sizeof(hdr) )
	{
		return	-4;
	}
	// ��m_package_hdr.nAvailIdx��Ӧ�����һ��ָ��nEnd
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
		// �����޸Ĺ��Ŀ�ͷ
		if( GetBaseFile()->Seek(-(int)sizeof(hdr), SEEK_CUR)!=0 )
		{
			return	-13;
		}
		if( GetBaseFile()->Write(&hdr)!=sizeof(hdr) )
		{
			return	-14;
		}
	}
	// ����޸�package_hdr
	m_package_hdr.nAvailIdx	= nStart;
	m_bShouldSavePackageHdr	= true;
	return	0;
}
int		wh_package_base::ReadDirTreeFromFile(STR2NODE_T *pRoot, whfile *f)
{
	while( !f->IsEOF() )
	{
		unsigned char	nType;
		// ��������
		int	rst			= f->Read(&nType) ;
		if( rst==0 )
		{
			// Ӧ���������Ѿ�ȫ��������
			// �����������
			return	0;
		}
		if( rst!= sizeof(nType) )
		{
			// ��ȡ�ļ�����
			return	-1;
		}
		switch( nType )
		{
		case	TYPE_DIREND:
			{
				// �˳�����Ŀ¼�������������һ��û��BEGIN��Ӧ��END�ͻᵼ�¶�ȡ�������������
				return	0;
			}
			break;
		case	TYPE_DIRBEGIN:
			{
				// ��������ƫ��
				unsigned int		nNameOffset;
				if( f->Read(&nNameOffset) != sizeof(nNameOffset) )
				{
					// ���ݳ���
					return	-2;
				}
				// ���뵽����ӳ�����
				STRINVECT	sivName(&m_vectFNAME, nNameOffset);
				FILENODE_T	*pNode;
				if( !pRoot->putalloc(sivName, pNode) )
				{
					// �������������ˣ�
					return	-3;
				}
				pNode->u.d.pContent	= new STR2NODE_T;
				if( !pNode->u.d.pContent )
				{
					return	-4;
				}
				pNode->SetIsDir(true);
				// �ݹ������һ��
				int	rst	= ReadDirTreeFromFile(pNode->u.d.pContent, f);
				if( rst<0 )
				{
					// ������Ŀ¼ʱ����
					return	-5;
				}
				// �����Ŀ¼����ɹ��ˣ����Ŷ���������ݰɣ�OVER
			}
			break;
		case	TYPE_FILE:
			{
				// ��������ƫ��
				unsigned int		nNameOffset;
				if( f->Read(&nNameOffset) != sizeof(nNameOffset) )
				{
					// ���ݳ���
					return	-12;
				}
				// ���뵽����ӳ�����
				STRINVECT	sivName(&m_vectFNAME, nNameOffset);
				FILENODE_T	*pNode;
				if( !pRoot->putalloc(sivName, pNode) )
				{
					// �������������ˣ�
					return	-13;
				}
				pNode->SetIsDir(false);
				// ������ʼ������
				if( f->Read(&pNode->u.f.nStartBlock)!=sizeof(pNode->u.f.nStartBlock) )
				{
					// ���ݴ���
					return	-14;
				}
			}
			break;
		default:
			{
				// ���������
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
		// û�����ݲ���д��
		return	0;
	}
	// �����Լ����ӽڵ㣬������д��
	for(STR2NODE_T::kv_iterator it=pRoot->begin(); it!=pRoot->end(); ++it)
	{
		FILENODE_T		&node	= it.getvalue();
		unsigned char	nType;
		if( node.IsDir() )
		{
			// дBEGIN
			nType		= TYPE_DIRBEGIN;
			if( f->Write(&nType)!=sizeof(nType) )
			{
				return	-11;
			}
			// д������ƫ��
			int	nOffset	= it.getkey().GetOffset();
			if( f->Write(&nOffset)!=sizeof(nOffset) )
			{
				return	-12;
			}
			// �ݹ�д��Ŀ¼����
			if( SaveDirTreeToFile(node.u.d.pContent, f)<0 )
			{
				return	-13;
			}
			// дEND
			nType		= TYPE_DIREND;
			if( f->Write(&nType)!=sizeof(nType) )
			{
				return	-21;
			}
		}
		else
		{
			// д������
			nType		= TYPE_FILE;
			if( f->Write(&nType)!=sizeof(nType) )
			{
				return	-31;
			}
			// д������ƫ��
			int	nOffset	= it.getkey().GetOffset();
			if( f->Write(&nOffset)!=sizeof(nOffset) )
			{
				return	-32;
			}
			// д�뿪ʼ��
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
		// û�����ݲ����ͷ�
		return	0;
	}
	// �����ӽڵ㣬����ͷ�
	for(STR2NODE_T::kv_iterator it=pRoot->begin(); it!=pRoot->end(); ++it)
	{
		FILENODE_T		&node	= it.getvalue();
		if( node.IsDir() )
		{
			// �ݹ��ȥ�����ͷ�
			if( FreeDirTree(node.u.d.pContent)<0 )
			{
				return	-1;
			}
		}
		else
		{
			// �ļ�û�ж�̬��������ݣ����Բ����κβ�����
		}
	}

	// ����ͷ��Լ�
	delete	pRoot;
	pRoot	= NULL;
	return	0;
}
int		wh_package_base::DelDirTree(STR2NODE_T *&pRoot)
{
	if( !pRoot )
	{
		// û�����ݲ����ͷ�
		return	0;
	}
	// �����ӽڵ㣬����ͷ�
	STR2NODE_T::kv_iterator it=pRoot->begin();
	while( it!=pRoot->end() )
	{
		FILENODE_T	&node		= it.getvalue();
		STR2NODE_T::kv_iterator	tmpit	= it++;
		if( node.IsDir() )
		{
			// �ݹ��ȥ�����ͷ�
			if( DelDirTree(node.u.d.pContent)<0 )
			{
				return	-1;
			}
		}
		else
		{
			// ɾ���ļ�
			if( DelFileByBlockIdx(node.u.f.nStartBlock)<0 )
			{
				return	-2;
			}
		}
		// ɾ������ڵ�
		if( !pRoot->erase(tmpit) )
		{
			return	-3;
		}
	}
	// ����ͷ��Լ�
	delete	pRoot;
	pRoot	= NULL;
	return	0;
}
int		wh_package_base::FindNodeByPathName(const char *cszPath, FINDNODEBYPATHNAME_RST_T *pRst)
{
	// �ֽ��ļ���
	char	szOne[WH_MAX_PATH];
	int		nOffset	= 0;
	pRst->pDir		= m_RootNode.u.d.pContent;
	pRst->pFN		= &m_RootNode;
	pRst->nOffset	= 0;
	while( wh_strsplit(&nOffset, "s", cszPath, "\1/\\", szOne)==1 )
	{
		// �ڵ�ǰ�ڵ����
		STR2NODE_T::kv_iterator	it	= pRst->pDir->find(szOne);
		if( it == pRst->pDir->end() )
		{
			// ��ǰ�ҵ����֡���ʱpRst->nOffset����szOne��ͷ��λ�ã�pRst->pDir���������ϲ�Ŀ¼��
			return	FINDNODEBYPATHNAME_RST_PARTIAL;
		}
		pRst->pFN		= &it.getvalue();
		// �����ǲ������Ĳ���
		if( cszPath[nOffset]==0 )
		{
			// ֱ���˳�����
			break;
		}
		if( !pRst->pFN->IsDir() )
		{
			// ˵���ҵ�·���м����ļ���˵��·���Ƿ�
			return	FINDNODEBYPATHNAME_RST_ERR_MIDDLEFILEINPATH;
		}
		// �������εĻ���dir
		pRst->pDir		= pRst->pFN->u.d.pContent;
		// ��¼�´ε���ʼƫ��
		pRst->nOffset	= nOffset;
	}
	return	FINDNODEBYPATHNAME_RST_OK;
}
wh_package_base::FILENODE_T *	wh_package_base::CreateNodeByPathName(const char *cszPath, STR2NODE_T *pRoot, bool bIsFile)
{
	// �ֽ��ļ���
	char	szOne[WH_MAX_PATH];
	int		nOffset		= 0;
	FILENODE_T	*pNode	= NULL;
	while( wh_strsplit(&nOffset, "s", cszPath, "\1/\\", szOne)==1 )
	{
		// ������ԭ���Ƿ����
		STRSET_T::key_iterator	it	= m_setFNAME.find(szOne);
		if( it == m_setFNAME.end() )
		{
			// ˵��ԭ��û��Ӧ�ô��������
			int	nLen	= strlen(szOne)+1;
			m_vectFNAME.push_back(szOne, nLen);
			STRINVECT	siv(&m_vectFNAME, m_vectFNAME.size()-nLen);
			it			= m_setFNAME.putgetit(siv);
			assert(it!=m_setFNAME.end());
			// �������vect�ı���
			m_bShouldSaveVectorFileName	= true;
		}
		// ����
		if( !pRoot->putalloc((*it), pNode) )
		{
			// Ӧ�����������ˣ�����Ǵ���path��Ӧ�ó��������������
			assert(0);
			return	NULL;
		}
		// ����ļ���Ϣ�����˸ı�
		m_bShouldSaveFileList	= true;
		//
		if( cszPath[nOffset]==0 )
		{
			// ˵�������һ����
			if( !bIsFile )
			{
				pNode->u.d.pContent	= new STR2NODE_T;
				pNode->SetIsDir(true);
				pRoot	= pNode->u.d.pContent;
			}
			else
			{
				// �ļ�Ӧ���ϲ�ȥ����block
			}
			// ֱ�ӷ���
			break;
		}
		else
		{
			// ˵������϶���Ŀ¼
			pNode->u.d.pContent	= new STR2NODE_T;
			pNode->SetIsDir(true);
			pRoot	= pNode->u.d.pContent;
		}
	}
	return	pNode;
}
int			wh_package_base::DelFileByBlockIdx(blockoffset_t nStart)
{
	// ��д��ʽ���ļ���ɾ��֮
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
		rst					= rf->Delete();	// �������ɾ���ļ�ָ���
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
		// ���ܶ�ֻ������д����
		return	NULL;
	}
	// �ȸ����ļ������Ƿ�����ҵ���Ӧ��raw�ļ�
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szFile, &Rst);
	switch( rst )
	{
	case	FINDNODEBYPATHNAME_RST_OK:
	case	FINDNODEBYPATHNAME_RST_PARTIAL:
		break;
	default:
		// û�гɹ��ҵ�ȫ���򲿷֣�������·�������ļ�����
		return	NULL;
	}
	// �����ײ��ļ�
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
						// �����ļ����ܸ���Ŀ¼
						throw	-2;
					}
					// ԭ���ļ����ڣ�����Ҫ��ɾ��֮
					if( DelFileByBlockIdx(Rst.pFN->u.f.nStartBlock)<0 )
					{
						throw	-3;
					}
				}
				else
				{
					// �������沿�ֵ�Ŀ¼���ļ�
					Rst.pFN	= CreateNodeByPathName(szFile+Rst.nOffset, Rst.pDir, true);
					if( !Rst.pFN )
					{
						throw	-11;
					}
				}
				// ��Ϊ�ļ���ʼ����ܸı��ˣ���˵���հ����ڵ��ͷŷ�ʽӦ�ò��ᷢ���ı䣩��������Ҫ���´洢�ļ���Ϣ��
				m_bShouldSaveFileList	= true;
			}
			break;
		case	whfileman::OP_MODE_BIN_READWRITE:
		case	whfileman::OP_MODE_BIN_READONLY:
		case	whfileman::OP_MODE_BIN_RAWREAD:
			{
				if( rst==FINDNODEBYPATHNAME_RST_PARTIAL )
				{
					// ��������ͱ����ļ�����
					throw	-21;
				}
				if( Rst.pFN->IsDir() )
				{
					// Ŀ�겻����Ŀ¼
					throw	-22;
				}
			}
			break;
		default:
				throw	-23;
			break;
		}
		// �򿪻򴴽�raw�ļ�������Ǵ������ı�Rst.pFN->u.f.nStartBlock��
		int	rst	= rf->Open(nOPMode, Rst.pFN->u.f.nStartBlock);
		if( rst<0 )
		{
			throw	-31;
		}
		// ����raw�ļ��Ѿ����ˣ����Խ����ϲ�ѹ���ļ���
		file4out::INFO_T	info;
		info.nOPMode		= nOPMode;
		info.pBaseFile		= rf;
		info.pWHCompress	= m_info.pWHCompress;
		info.pEncryptor		= m_info.pEncryptor;
		info.pDecryptor		= m_info.pDecryptor;
		info.bAutoCloseBaseFile	= true;
		if( fpo->Open(&info)<0 )
		{
			// ��Ϊ���Զ��ر�basefile����������Ͱ�rf�����
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
	// ͨ��Ŀ¼���ҵ���Ӧ�Ľڵ�
	// �ȸ����ļ������Ƿ�����ҵ���Ӧ��raw�ļ�
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szDir, &Rst);
	if( rst!=FINDNODEBYPATHNAME_RST_OK )
	{
		// û�гɹ��ҵ�ȫ����Ӧ����·�����ԣ�
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
		// ���ܶ�ֻ������д����
		return	-1;
	}
	// ���ҵ�ǰ���ж����Ѿ�������
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szDir, &Rst);
	switch( rst )
	{
	case	FINDNODEBYPATHNAME_RST_OK:
		{
			// ˵���Ѿ�������
			// �����ǲ���Ŀ¼
			if( Rst.pFN->IsDir() )
			{
				// �Ѿ������ˣ��Ͳ��ô�����
				return	0;
			}
			else
			{
				// ԭ�����ڵ���һ���ļ�������������ط�����Ŀ¼
				return	-2;
			}
		}
		break;
	case	FINDNODEBYPATHNAME_RST_PARTIAL:
		{
			// ��������Ĳ���
			FILENODE_T	*pFN	= CreateNodeByPathName(szDir+Rst.nOffset, Rst.pDir, false);
			if( !pFN )
			{
				return	-11;
			}
			else
			{
				// �����ɹ�
				return	0;
			}
		}
		break;
	default:
		// û�гɹ��ҵ�ȫ���򲿷֣�������·�������ļ�����
		return	-100;
	}
}
int			wh_package_base::DelPath(const char *szFile)
{
	if( IsReadOnly() )
	{
		// ���ܶ�ֻ������д����
		return	-1;
	}
	// �ȸ����ļ������Ƿ�����ҵ���Ӧ��Ŀ¼���ļ�
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szFile, &Rst);
	if( rst != FINDNODEBYPATHNAME_RST_OK )
	{
		// û�гɹ��ҵ�
		return	-2;
	}
	if( !Rst.pFN->IsDir() )
	{
		// ���ļ�ɾ��
		// ��ɾ���ļ�block
		if( DelFileByBlockIdx(Rst.pFN->u.f.nStartBlock)<0 )
		{
			return	-11;
		}
	}
	else
	{
		// ��Ŀ¼��Ŀ¼�ݹ�ɾ��
		rst	= DelDirTree(Rst.pFN->u.d.pContent);
		if( rst<0 )
		{
			return	-100 + rst;
		}
	}
	// ����ļ���Ϣ�����˸ı�
	m_bShouldSaveFileList	= true;
	// ɾ���ϲ������ӳ��(��Ӧ��pContent�Ѿ���DelDirTree�б�ɾ����)
	if( !Rst.pDir->erase(szFile+Rst.nOffset) )
	{
		// ���ҵ���Ӧ�ò���ɾ��ʧ�ܵ�
		assert(0);
		return	-31;
	}
	return	0;
}
int			wh_package_base::GetPathInfo(const char *cszPath, whfileman::PATHINFO_T *pPathInfo)
{
	pPathInfo->nType	= 0;
	// �ȸ����ļ������Ƿ�����ҵ���Ӧ��Ŀ¼���ļ�
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(cszPath, &Rst);
	if( rst != FINDNODEBYPATHNAME_RST_OK )	// ���ȡinfo��ôԭ���ļ�����Ŀ¼�ͱ�����ڣ�������partial�ģ�
	{
		// û�гɹ��ҵ�
		return	-1;
	}
	if( Rst.pFN->IsDir() )
	{
		pPathInfo->nType	|= whfileman::PATHINFO_T::TYPE_DIR;
		pPathInfo->nMTime	= 0;	// �������Ŀ¼ʱ��û������
	}
	else
	{
		// ����Ŀ¼����ô����ļ�������Ϣ��ʱ�䡢���ȣ�
		// �����ײ��ļ�
		rawfile		*rf		= NewRawFile();
		file4out	*fpo	= new file4out(this);
		try
		{
			if( !rf || !fpo )
			{
				throw	-20;
				if( rst==FINDNODEBYPATHNAME_RST_PARTIAL )
				{
					// ��������ͱ����ļ�����
					throw	-21;
				}
				if( Rst.pFN->IsDir() )
				{
					// Ŀ�겻����Ŀ¼
					throw	-22;
				}
			}
			// �򿪻򴴽�raw�ļ�������Ǵ������ı�Rst.pFN->u.f.nStartBlock��
			int	rst	= rf->Open(whfileman::OP_MODE_BIN_READONLY, Rst.pFN->u.f.nStartBlock);
			if( rst<0 )
			{
				throw	-31;
			}
			// ����raw�ļ��Ѿ����ˣ����Դ��ϲ�ѹ���ļ���ȡ��Ϣ��
			file4out::INFO_T	info;
			info.nOPMode		= whfileman::OP_MODE_BIN_READONLY;
			info.pBaseFile		= rf;
			info.pWHCompress	= m_info.pWHCompress;
			info.pEncryptor		= m_info.pEncryptor;
			info.pDecryptor		= m_info.pDecryptor;
			info.bAutoCloseBaseFile	= true;
			if( fpo->GetPathInfo(&info, pPathInfo)<0 )
			{
				// ��Ϊ���Զ��ر�basefile����������Ͱ�rf�����
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
			// ����nStart�����ǷǷ���
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

// ���ذ����ļ���ԭʼ���ݣ�ѹ��������ݣ������Ľ����
whfile * wh_package_base::OpenRawFile(const char *szFile, whfileman::OP_MODE_T nOPMode, blockoffset_t *pnStart/* =NULL */)
{
	if( IsReadOnly() && !whfileman::IsReadOnlyMode(nOPMode) )
	{
		// ���ܶ�ֻ������д����
		return	NULL;
	}
	// �ȸ����ļ������Ƿ�����ҵ���Ӧ��raw�ļ�
	FINDNODEBYPATHNAME_RST_T	Rst;
	int	rst	= FindNodeByPathName(szFile, &Rst);
	switch( rst )
	{
	case	FINDNODEBYPATHNAME_RST_OK:
	case	FINDNODEBYPATHNAME_RST_PARTIAL:
		break;
	default:
		// û�гɹ��ҵ�ȫ���򲿷֣�������·�������ļ�����
		return	NULL;
	}
	// �����ײ��ļ�
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
						// �����ļ����ܸ���Ŀ¼
						throw	-2;
					}
					// ԭ���ļ����ڣ�����Ҫ��ɾ��֮
					if( DelFileByBlockIdx(Rst.pFN->u.f.nStartBlock)<0 )
					{
						throw	-3;
					}
				}
				else
				{
					// �������沿�ֵ�Ŀ¼���ļ�
					Rst.pFN	= CreateNodeByPathName(szFile+Rst.nOffset, Rst.pDir, true);
					if( !Rst.pFN )
					{
						throw	-11;
					}
				}
				// ��Ϊ�ļ���ʼ����ܸı��ˣ���˵���հ����ڵ��ͷŷ�ʽӦ�ò��ᷢ���ı䣩��������Ҫ���´洢�ļ���Ϣ��
				m_bShouldSaveFileList	= true;
			}
			break;
		case	whfileman::OP_MODE_BIN_READWRITE:
		case	whfileman::OP_MODE_BIN_READONLY:
		case	whfileman::OP_MODE_BIN_RAWREAD:
			{
				if( rst==FINDNODEBYPATHNAME_RST_PARTIAL )
				{
					// ��������ͱ����ļ�����
					throw	-21;
				}
				if( Rst.pFN->IsDir() )
				{
					// Ŀ�겻����Ŀ¼
					throw	-22;
				}
			}
			break;
		default:
			throw	-23;
			break;
		}
		// �򿪻򴴽�raw�ļ�������Ǵ������ı�Rst.pFN->u.f.nStartBlock��
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

