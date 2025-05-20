// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whchunk.h
// Creator      : Wei Hua (κ��)
// Comment      : ���������λ�Ĵ���ڴ���䣬����ͨ����Ҫ���ʸ�����Ԫ
//                ������������ô��ǿ�ʼ�����ָ��һֱ��Ч������vector�������resize���ܻᷢ���仯
//                2004-01-17 ע�⣬����chunk�����õ���list��vector���Բ���ʹ��allocator������ʱ��ʹ���ˡ�
//                ����������new�����ģ����Կ��Է���ʹ��
// CreationDate : 2004-01-16
// ChangeLog    :
//                2004-01-17 ����˷������������������Ϳ���ʹ�ù����ڴ���
//                2005-09-12 ������sequencealloc�������ڷ��������Ķ����Ԫ
//                2006-06-27 ��delete��Ϊdelete[]�ˣ���˵��new[]������deleteɾ����δ�������Ϊ���������������ʵ�ַ�ʽ���˾ͻ������⣩

#ifndef	__WHCHUNK_H__
#define	__WHCHUNK_H__

#include "./whvector.h"
#include <assert.h>

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// ģ�����͵�
////////////////////////////////////////////////////////////////////
// ����ܷ���ͷ���
template<typename _Ty, class _AllocationObj=whcmnallocationobj>
class	whchunk
{
protected:
	// ���ڴ�Ÿ���chunk��ָ��
	whvector<_Ty *, false, _AllocationObj>	m_ptrBuf;
	// ÿ��chunk�еĵ�Ԫ����
	size_t			m_nChunkSize;
public:
	// ע�⿪ʼ�����Ͳ��ܸı���
	inline void	setChunkSize(size_t nSize)
	{
		if( nSize==0 )
		{
			assert(0);
			return;
		}
		m_nChunkSize	= nSize;
	}
public:
	whchunk()
	: m_nChunkSize(100)
	{
	}
	whchunk(int nChunkSize)
	: m_nChunkSize(nChunkSize)
	{
	}
	~whchunk()
	{
		// Ϊ�˱�����ɾ��һ��
		destroy();
	}
	void	destroy()
	{
		if( m_ptrBuf.size()>0 )
		{
			// ɾ������chunk
			for(size_t i=0;i<m_ptrBuf.size();i++)
			{
				m_ptrBuf.GetAND().DeleteArray(m_ptrBuf[i], m_nChunkSize);
			}
			m_ptrBuf.clear();
		}
	}
	// ���÷������
	inline void	SetAO(_AllocationObj *pAO)
	{
		m_ptrBuf.SeAO(pAO);
	}
	// ����nNum���µ�chunk
	void	createchunk(int nNum=1)
	{
		int	i;
		for(i=0;i<nNum;i++)
		{
			// ��������ĳ�ʼ��Ӧ����New�е�����
			_Ty *	pChunk = m_ptrBuf.GetAND().NewArray((_Ty *)0, m_nChunkSize);
			assert(pChunk);
			m_ptrBuf.push_back(pChunk);
		}
	}
	// ���Ŀǰ����ʹ�õ��ܵ�Ԫ����
	inline size_t	gettotalnum() const
	{
		return	m_nChunkSize * m_ptrBuf.size();
	}
	// ������Ҫ����һ����Ԫ
	inline _Ty & operator[] (int nIdx) const
	{
		return	getunit(nIdx);
	}
	inline _Ty &	getunit(int nIdx) const
	{
		// ����Ͳ���Խ���ж��ˣ��ϲ�Ҫ��֤
		return	m_ptrBuf[nIdx/m_nChunkSize][nIdx%m_nChunkSize];
	}
	inline _Ty *	getunitptr(int nIdx) const
	{
		int	nTotalUnit = gettotalnum();
		if( nIdx<0 || nIdx>=nTotalUnit )
		{
			return	NULL;
		}
		return	&m_ptrBuf[nIdx/m_nChunkSize][nIdx%m_nChunkSize];
	}
};

// �����ֻ��ѭ�����
template<typename _Ty, class _AllocationObj=whcmnallocationobj>
class	whallocbychunk	: public whchunk<_Ty, _AllocationObj>
{
private:
	typedef	whchunk<_Ty, _AllocationObj>	FATHERCLASS;
protected:
	// �Ѿ����������
	size_t	m_nAllocNum;
public:
	whallocbychunk()
	: FATHERCLASS()
	{
		clear0();
	}
	whallocbychunk(int nChunkSize)
	: FATHERCLASS(nChunkSize)
	{
		clear0();
	}
	inline void	clear0()
	{
		m_nAllocNum	= 0;
	}
	inline void	clear()					// ������Ϊ�˱��˺õ���(ϰ��)
	{
		clear0();
	}
	void	destroy()
	{
		FATHERCLASS::destroy();
		clear0();
	}
	// ����һ������
	_Ty *	alloc()
	{
		if( m_nAllocNum>=FATHERCLASS::gettotalnum() )
		{
			// �����µ�chunk
			FATHERCLASS::createchunk();
		}
		return	FATHERCLASS::getunitptr(m_nAllocNum++);
	}
	// ����һ�������Ķ���(����ط�������create�µ�chunk)
	// �����������ǲ�Ӧ�ö�Ӧ�ͷŵġ���Ҫ���������Եĳ�ʼ��(������ĳ����ִ�)
	_Ty *	sequencealloc(size_t nNum)
	{
		if( nNum>FATHERCLASS::m_nChunkSize )
		{
			assert(0);			// ����ĳ��ȳ�����ÿ��chunk�Ĵ�С
			return	NULL;
		}
		size_t	nCurTotal	= FATHERCLASS::gettotalnum();
		if( m_nAllocNum+nNum > nCurTotal )
		{
			// �˷�һЩ����
			m_nAllocNum		= nCurTotal;
			// �����µ�chunk
			FATHERCLASS::createchunk();
		}
		size_t	nOffset	= m_nAllocNum;
		m_nAllocNum	+= nNum;
		return	FATHERCLASS::getunitptr(nOffset);
	}
	// �������벢����
	_Ty *	sequenceallocandcopy(const _Ty *pData, size_t nNum)
	{
		_Ty	*pDst	= sequencealloc(nNum);
		if( !pDst )
		{
			assert(0);
			return	NULL;
		}
		memcpy(pDst, pData, nNum*sizeof(_Ty));
		return	pDst;
	}
	inline size_t	size() const
	{
		return	m_nAllocNum;
	}
};

////////////////////////////////////////////////////////////////////
// ��ģ�����͵�
////////////////////////////////////////////////////////////////////
// �������ͣ�ֻ���ĳ��ȵ�chunk����
class	whsmpchunk
{
public:
	// ����
	struct	INFO_T
	{
		// ÿ����Ԫ�ĳ���
		int				nUnitSize;
		// ÿ��chunk�еĵ�Ԫ����
		int				nChunkSize;
		INFO_T()
		: nUnitSize(0)
		, nChunkSize(0)
		{
		}
	};
protected:
	// ���ڴ�Ÿ���chunk��ָ��
	whvector<char *>	m_ptrBuf;
	// �ܵ�Ԫ��
	size_t				m_nTotalMaxUnit;
public:
	// ��ʼ����Ϣ(����ڿ�ʼ����֮ǰ��������)
	// !!!!�ϲ���뱣֤��ʹ��ǰ������m_info������!!!!
	INFO_T				m_info;
public:
	whsmpchunk()
	: m_nTotalMaxUnit(0)
	{
	}
	~whsmpchunk()
	{
		// Ϊ�˱�����ɾ��һ��
		destroy();
	}
	void	destroy()
	{
		if( m_ptrBuf.size()>0 )
		{
			// ɾ������chunk
			for(size_t i=0;i<m_ptrBuf.size();i++)
			{
				delete []	m_ptrBuf[i];
			}
			m_ptrBuf.clear();
		}
		m_nTotalMaxUnit	= 0;
	}
	// ����nNum���µ�chunk
	void	createchunk(int nNum=1)
	{
		// �ϲ���뱣֤������m_info������
		assert(m_info.nChunkSize>0 && m_info.nUnitSize);
		int	i;
		for(i=0;i<nNum;i++)
		{
			// ��������ĳ�ʼ��Ӧ����New�е�����
			char *	pChunk = new char[m_info.nChunkSize*m_info.nUnitSize];
			assert(pChunk);
			m_ptrBuf.push_back(pChunk);
		}
		m_nTotalMaxUnit	= m_info.nChunkSize * m_ptrBuf.size();
	}
	// ���Ŀǰ����ʹ�õ��ܵ�Ԫ����
	inline size_t	gettotalnum() const
	{
		return	m_nTotalMaxUnit; 
	}
	// ������Ҫ����һ����Ԫ
	inline char * operator[] (int nIdx) const
	{
		return	getunitptr(nIdx);
	}
	inline char *	getunitptr(int nIdx) const
	{
		// ����Ͳ���Խ���ж��ˣ��ϲ�Ҫ��֤
		return	m_ptrBuf[nIdx/m_info.nChunkSize] + nIdx%m_info.nChunkSize * m_info.nUnitSize;
	}
};

// �����ֻ��ѭ�����
class	whallocbysmpchunk	: public whsmpchunk
{
protected:
	// �Ѿ����������
	size_t	m_nAllocNum;
public:
	whallocbysmpchunk()
	{
		clear();
	}
	void	clear()
	{
		m_nAllocNum	= 0;
	}
	void	destroy()
	{
		whsmpchunk::destroy();
		clear();
	}
	inline char *	alloc()
	{
		if( m_nAllocNum>=gettotalnum() )
		{
			// �����µ�chunk
			createchunk();
		}
		return	getunitptr(m_nAllocNum++);
	}
	inline size_t	size() const
	{
		return	m_nAllocNum;
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHCHUNK_H__
