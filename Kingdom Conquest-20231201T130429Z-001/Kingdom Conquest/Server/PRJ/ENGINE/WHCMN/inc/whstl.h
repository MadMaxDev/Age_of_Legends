// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whstl.h
// Creator      : Wei Hua (κ��)
// Comment      : ���ڸ���stl�ĺ����ͷ���
// CreationDate : 2003-06-13

#ifndef	__WHSTL_H__
#define	__WHSTL_H__

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif
#include <map>
#include <set>
#include "whvector.h"

namespace n_whcmn
{

// ���Ϊ��ʵ��Ѹ�ٲ�ֵ
template<typename _TyKey, typename _TyVal>
class	whmultimap
{
public:
	typedef	std::set<_TyVal>							VALSET;
	typedef	std::map<_TyKey, VALSET>					MAPSET;
	typedef typename VALSET::iterator					VSIT;
	typedef typename MAPSET::iterator					MSIT;
	struct	iterator
	{
		MSIT				itmap;		// mapλ��
		VSIT				itset;		// setλ��
	};
protected:
	MAPSET					m_map;
public:
	whmultimap()
	{
	}
	~whmultimap()
	{
	}
	// ���__key/__val��
	bool	add(const _TyKey & __key, const _TyVal & __val)
	{
		// �ж�����Ƿ����
		MSIT	itSet;
		bool	bFound;
		if( !(bFound=searchKey(__key, itSet)) )
		{
			// ������key
			m_map.insert(std::make_pair(__key, VALSET()));
			bFound	= searchKey(__key, itSet);
		}
		if( !bFound )
		{
			// �޷����
			return	false;
		}
		// ���ֵ�Ƿ��Ѿ�����
		if( itSet->second.find(__val) != itSet->second.end() )
		{
			// �������Ѿ�����
			return	false;
		}

		itSet->second.insert(__val);
		return		true;
	}
	// ɾ��__key/__val��
	bool	remove(const _TyKey & __key, const _TyVal & __val)
	{
		iterator	it;
		if( searchKeyVal(__key, __val, it) )
		{
			it.itmap->second.erase((*it.itset));
			if( it.itmap->second.size()==0 )
			{
				m_map.erase(it.itmap);
			}
			return	true;
		}
		return		false;
	}
	// ɾ��__key��Ӧ�����м�¼
	bool	remove(const _TyKey & __key)
	{
		// �ж���key�Ƿ����
		MSIT	itSet;
		if( !searchKey(__key, itSet) )
		{
			// ������
			return	false;
		}
		// ɾ����Key������set
		m_map.erase(itSet);
		return		true;
	}
	// ���__key/__val���Ƿ���ڣ����Ϊ�潫λ�÷���__it��
	bool	searchKeyVal(const _TyKey & __key, const _TyVal & __val, iterator & __it)
	{
		if( searchKey(__key, __it.itmap) )
		{
			__it.itset	= __it.itmap->second.find(__val);
			if( __it.itset != __it.itmap->second.end() )
			{
				return	true;
			}
		}
		return	false;
	}
	// ���__key�Ƿ���ڣ�������ڷ���set
	bool	searchKey(const _TyKey __key, MSIT &__it)
	{
		__it	= m_map.find(__key);
		if( __it==m_map.end() )
		{
			return	false;
		}
		return		true;
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHSTL_H__
