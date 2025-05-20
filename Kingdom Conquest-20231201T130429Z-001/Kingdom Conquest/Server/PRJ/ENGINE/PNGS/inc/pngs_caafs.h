// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_caafs.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��CAAFSģ��
//                PNGS��Pixel Network Game Structure����д
//                CAAFS��Connection Allocation and Address Filter Server����д�������ӷ��书���е����ӷ���͵�ַ���˷�����
// CreationDate : 2005-07-23
// Change LOG   :

#ifndef	__PNGS_CAAFS_H__
#define	__PNGS_CAAFS_H__

#include <WHCMN/inc/wh_crypt_interface.h>
#include <WHCMN/inc/whvector.h>
#include <WHNET/inc/whnetcmn.h>

namespace n_pngs
{

class	CAAFS2
{
public:
	static CAAFS2 *	Create();
protected:
	virtual~CAAFS2()		{}
public:
	// �Լ������Լ�
	virtual	void	SelfDestroy()									= 0;
	// ���������ļ���ʼ��
	virtual	int		Init(const char *cszCFG)						= 0;
	// ������������
	virtual	int		Init_CFG_Reload(const char *cszCFG)				= 0;
	// �ս�
	virtual	int		Release()										= 0;
	// ���ü��ܹ���(�ϲ�ע�Ᵽ֤��CAAFS2::SelfDestroy֮����ܰ�pICryptFactory�ս��)
	virtual	int		SetICryptFactory(n_whcmn::ICryptFactory *pFactory)
																	= 0;
	// ��ȡsockets�Թ��ϲ�select
	virtual size_t	GetSockets(n_whcmn::whvector<SOCKET> &vect)		= 0;
	// ����һ��
	virtual	int		Tick()											= 0;
	// ��ǰ����״̬
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

#endif	// EOF __PNGS_CAAFS_H__
