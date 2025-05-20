// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whtimequeue.h
// Creator      : Wei Hua (κ��)
// Comment      : ʱ�����(����������Է��ü����49.7/2����¼�)
// CreationDate : 2005-04-07
// ChangeLOG    : 2006-08-15 ������Ҫ��������ʱ����е�clear����
//              : 2007-07-31 ��֤ͬһʱ������ע����¼��ȷ���

#ifndef	__WHTIMEQUEUE_H__
#define	__WHTIMEQUEUE_H__

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif

#include <set>
#include "./whtime.h"
#include "./whunitallocator.h"

namespace n_whcmn
{

class	whtimequeue
{
public:
	// ����
	// ��ʼ������
	struct	INFO_T
	{
		// �������������ڳ�ʼ��whsmpunitallocator
		// nChunkSize��ʾÿһ��������ٵ�Ԫ
		int				nUnitLen;
		int				nChunkSize;
		INFO_T()
			: nUnitLen(4)
			, nChunkSize(128)
		{
		}
	};
	// ID�ṹ
	// ע�⣺������IDû�б��ͷŵ�ʱ�����¸����ID������ֵ��������ԭ��ָ���ID�Ͳ����Զ��ͷ��ˡ���������Ӧ���еط��ȼ�¼��ID�������ͷš����磺һ���TEDeal������
	struct	ID_T
	{
		whtick_t		t;
		int				nRotate;		// ��ת��������������֤ͬһʱ�̵��Ⱥ�˳��
		int				idx;
		whtimequeue		*pHost;
		ID_T()
		: t(0)
		, nRotate(0)
		, idx(-1)
		, pHost(NULL)
		{
		}
		~ID_T()
		{
			// !!!!��Ҫ���������DelFromQueue()����ΪID_TҲ����ֵ�����������Կ��ܵ��¶�α�ɾ��������
			// ������Ҫӵ�������ʵ���ʱ���Լ�����quit
		}
		inline void	clear()
		{
			t		= 0;
			// nRotate	= 0;	������Բ��ùܣ�����������������д��
			idx		= -1;
			pHost	= NULL;
		}
		// �����Ӷ������˳�
		// ��Ϊ�����������Ҫ��֤���к�timequeue��صĶ��󶼱�����timequeue�ս�֮ǰ����quit������͸ɴ಻Ҫ����
		inline int	quit()
		{
			if( pHost )
			{
				// ���Del��ʵҲ�͵���ID_T��clear��
				return	pHost->Del(*this);
			}
			return	0;
		}
		inline bool	IsValid() const
		{
			return	idx>=0;
		}
		inline bool operator <	(const ID_T &other) const
		{
			int	d	= wh_tickcount_diff(t, other.t);
			// ʱ�����ȣ�ID����
			if( d!=0 )	return d<0;
			// ʱ����ȲűȽ�rotate��ͬһ��tick�ڵ�rotateһ������һ���ģ�
			if( nRotate!=other.nRotate)	return nRotate<other.nRotate;
			// rotate��ȲűȽ�idx��������˵rotate���᲻һ���ģ�������ͬһʱ�̼�����������ô���¼���
			return	idx < other.idx;
		}
		inline bool	operator == (const ID_T &other) const
		{
			return	t==other.t && idx==other.idx && nRotate==other.nRotate;
		}
	};
private:
	typedef	std::set<ID_T>	idset_t;
private:
	// ��Ա����
	INFO_T				m_info;
	idset_t				m_idset;
	whsmpunitallocator	*m_punits;
	int					m_nRotate;
public:
	whtimequeue();
	~whtimequeue();
	int		Init(INFO_T *pInfo);
	int		Release();
	// ��ն���
	void	Clear();
	// ����¼�
	int 	Add(whtick_t t, void *pUnit, whtimequeue::ID_T *pID);
	int 	AddGetRef(whtick_t t, void **ppUnit, whtimequeue::ID_T *pID);
	// ɾ���¼�
	int		Del(ID_T &id);
	// �޸�ʱ��
	int		TimeMove(whtimequeue::ID_T *pID, whtick_t newt);
	// ����ʱ��������֮ǰ�����ϵ�һ���¼���id�����ݣ�
	// ������سɹ�0����ʹ����Ϻ��Լ��ǵ�ɾ��
	int		GetUnitBeforeTime(whtick_t t, void **ppUnit, ID_T *pID);
	// ���nIdx����ID_T�е�idx
	inline char *	GetUnitPtr(int nIdx)
	{
		return	m_punits->getptr(nIdx);
	}
	// ����¼�����
	inline int		size() const
	{
		assert((int)m_idset.size() == m_punits->size());
		return	m_idset.size();
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHTIMEQUEUE_H__
