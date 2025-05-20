// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cls.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CLSģ��
//                PNGS��Pixel Network Game Structure����д
//                CLS��Connection Load Server����д�������ӷ��书���еĸ��ط�����
// CreationDate : 2005-07-22
// Change LOG   :

#ifndef	__PNGS_CLS_H__
#define	__PNGS_CLS_H__

#include <WHCMN/inc/wh_crypt_interface.h>
#include <WHCMN/inc/whvector.h>
#include <WHNET/inc/whnetcmn.h>

namespace n_pngs
{

class	CLS2
{
public:
	static CLS2 *	Create();
protected:
	virtual	~CLS2()	{}
public:
	// �Լ������Լ�
	virtual	void	SelfDestroy()									= 0;
	// ���������ļ���ʼ��
	virtual	int		Init(const char *cszCFG)						= 0;
	// ������������
	virtual	int		Init_CFG_Reload(const char *cszCFG)				= 0;
	// �ս�
	virtual	int		Release()										= 0;
	// ���ü��ܹ���(��������õĻ��ڲ�Ӧ��ʹ��Ĭ�ϵļӽ��ܹ���������������������)
	virtual	int		SetICryptFactory(n_whcmn::ICryptFactory *pFactory)
																	= 0;
	// ��ȡsockets�Թ��ϲ�select
	virtual size_t	GetSockets(n_whcmn::whvector<SOCKET> &vect)		= 0;
	// ����һ��
	virtual	int		Tick()											= 0;
	enum
	{
		STATUS_ERR_NETWORK				= -1,						// �������������
		STATUS_NOTHING					= 0,
		STATUS_WORKING					= 1,						// ���ӳɹ���������
	};
	virtual	int		GetStatus() const								= 0;
	// �����ϲ����Ӧ��ֹͣ��
	virtual	bool	ShouldStop() const								= 0;
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CLS_H__
