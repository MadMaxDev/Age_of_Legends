// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstreambuffer.h
// Creator      : Wei Hua (κ��)
// Comment      : �����������塣��Out�����лص���GetBuf���ָ��ʧЧ
// CreationDate : 2003-10-06
// 

#ifndef	__WHSTREAMBUFFER_H__
#define	__WHSTREAMBUFFER_H__

#include <stdlib.h>
#include "whlock.h"

namespace n_whcmn
{

// whstreambuffer����in������ʱ���ǲ��ᵼ��ָ��仯��
class	whstreambuffer
{
protected:
	char	*m_pBuf;								// �����������ݻ�����
	size_t	m_nMaxSize;								// ���Դ�ŵ�������ݳ���
	size_t	m_nSize;								// �Ѿ���ŵ������ܳ���
	size_t	m_nBegin;								// ���ݿ�ʼ��λ��
	size_t	m_nScrollSize;							// ��ȡ�����ݵ���m_nBegin����m_nScrollSize���ͽ��������ݹ�������ͷ��
public:
	inline char *	GetBuf() const					// Ŀǰ���ݵĿ�ʼָ�룬����һ���������(Free)ǰ��Ч
	{
		return	m_pBuf + m_nBegin;
	}
	inline size_t	GetMaxSize() const
	{
		return	m_nMaxSize;
	}
	inline size_t	GetSize() const					// ����Ѿ����ڵ����ݳ���
	{
		return	m_nSize;
	}
	inline char *	GetTail() const					// ���βָ��
	{
		return	m_pBuf + m_nBegin + m_nSize;
	}
	inline size_t	GetSizeLeft() const				// ��û�������ӵ����ݳ���
	{
		return	m_nMaxSize - m_nBegin - m_nSize;
	}
public:
	whstreambuffer();
	~whstreambuffer();
	int		Init(size_t nMaxSize, size_t nTailPadding, size_t nScrollSize);
													// ��ʼ�����塣nTailPadding�����ڽ�β����������ֽڲ��á�
	int		Release();								// �ս�(�����������Զ�����)
	void	Clean();								// ��������е���������
	void *	InAlloc(size_t nSize);					// ������뻺���ڴ棬������д���ݣ���������뿽��һ��
	int		In(const void *pData, size_t nSize);	// ���볤��ΪnSize�����ݣ�����0��ʾ�ɹ���-1�������ܼ��룬������ּ��벿�ֵ����
	int		Out(void *pData, size_t *pnSize);		// ȡ�����ݣ�*pnSize������ȡ�õ����ݳ��ȣ�����0��ʾ�ɹ�������ݣ�-1��ʾû������
	void	Out(size_t nSize);						// �ͷ�����
};

class	whstreambufferWL :	public	whstreambuffer
{
private:
	whlock	m_lock;
public:
	inline void	ReInitLock()
	{
		m_lock.reinit();
	}
	inline void	Clean()
	{
		m_lock.lock();
		whstreambuffer::Clean();
		m_lock.unlock();
	}
	inline void *	InAlloc(size_t nSize)
	{
		m_lock.lock();
		void *	ptr = whstreambuffer::InAlloc(nSize);
		m_lock.unlock();
		return	ptr;
	}
	int		In(const void *pData, size_t nSize)
	{
		m_lock.lock();
		int		rst = whstreambuffer::In(pData, nSize);
		m_lock.unlock();
		return	rst;
	}
	inline int		Out(void *pData, size_t *pnSize)
	{
		m_lock.lock();
		int		rst = whstreambuffer::Out(pData, pnSize);
		m_lock.unlock();
		return	rst;
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHSTREAMBUFFER_H__
