// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetlocalhiccup_i.h
// Creator      : Wei Hua (κ��)
// Comment      : ����UDP�ľ������ڵ�Ӧ�ó�����Ȩ���е�ʵ�ֲ�����ͷ�ļ�
//              : hiccup�Ǵ��õ���˼��ֻ������뵽����ʾ���ô����:P
// CreationDate : 2006-06-29
// ChangeLog    : 

#ifndef	__WHNETLOCALHICCUP_I_H__
#define	__WHNETLOCALHICCUP_I_H__

#include "whnetlocalhiccup.h"
#include "whnetcnl2.h"
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whtimequeue.h>

using namespace n_whcmn;

namespace n_whnet
{

class	whnetlocalhiccup_client_I	: public whnetlocalhiccup_client
{
public:
	// �����ܵ��еİ�
	whnetlocalhiccup_client_I();
	// Ϊ�ӿ�ʵ�ֵķ���
	virtual	~whnetlocalhiccup_client_I();
	virtual	int	Init(INFO_T *pInfo);
	virtual	int	Release();
	virtual	int	Tick();
	virtual	SOCKET GetSocket() const
	{
		return	m_pSlotMan->GetSocket();
	}
// �Լ��õ�
private:
	// <<����>>
	// ��ʼ������
	INFO_T					m_info;
	// ������
	CNL2SlotMan				*m_pSlotMan;
	// ���ܹ���
	ICryptFactory			*m_pICryptFactory;
	// ���ӵ�slot
	int						m_nSlot;
	// ��ַ�б��ӳ�ʼ�������зֽ������
	whsmpqueue				m_queueAddr;
	// ״̬
	struct	STATUS_T
	{
		// STATUS����
		enum	TYPE_T
		{
			NOTHING						= 0,						// û��״̬
			CONNECTING					= 1,						// ���ӹ�����
			WORKING						= 2,						// ��������������
			STOPPED						= 3,						// ֹͣ������
			MAXNUM,													// ���ĸ���
		};
		// ���ִ���������
		typedef	void (whnetlocalhiccup_client_I::*F_CMN_T)();			// ��ͨ�Ĳ��������ĺ���
		typedef	void (whnetlocalhiccup_client_I::*F_TRANS_T)(STATUS_T *pNextStatus);
																	// ��һ��״̬ת��Ϊ��һ��״̬��ʱ��

		TYPE_T							nStatus;
		F_CMN_T							pBegin;						// �ڿ�ʼһ��״̬ʱ��Ҫ�ĺ���
		F_CMN_T							pWork;						// ����������
		F_TRANS_T						pTrans;						// ��һ��״̬ת��Ϊ��һ��״̬��ʱ��
	};
	STATUS_T				*m_pStatus;
	STATUS_T				m_aSTATUS[STATUS_T::MAXNUM];
private:
	// <<����>>
	// ��ʼ��STATUS����
	void	StatusObjInit();
	// ����״̬
	void	SetStatus(STATUS_T::TYPE_T nType);
	inline STATUS_T::TYPE_T	GetStatus() const
	{
		return	m_pStatus->nStatus;
	}
	// ״̬������
	void	_SF_CMN_T_DoNothing();
	void	_SF_TRANS_T_DoNothing(STATUS_T *pNextStatus);
	// CONNECTING��
	void	_SF_Begin_CONNECTING();
	void	_SF_Work_CONNECTING();
	// WORKING��
	void	_SF_Begin_WORKING();
	void	_SF_Work_WORKING();
	// STOPPED��
	void	_SF_Begin_STOPPED();
};

class	whnetlocalhiccup_server_I	: public whnetlocalhiccup_server
{
public:
	// �����ܵ��еİ�
	whnetlocalhiccup_server_I();
	// Ϊ�ӿ�ʵ�ֵķ���
	virtual	~whnetlocalhiccup_server_I();
	virtual	int	Init(INFO_T *pInfo);
	virtual	int	Release();
	virtual	int	Tick();
	virtual	SOCKET GetSocket() const
	{
		return	m_pSlotMan->GetSocket();
	}
// �Լ��õ�
private:
	// <<����>>
	// ��ʼ������
	INFO_T					m_info;
	// ������
	CNL2SlotMan				*m_pSlotMan;
	// ���ܹ���
	ICryptFactory			*m_pICryptFactory;
	// ���ӵ�Ԫ
	struct	ClientUnit
	{
		enum
		{
			PROP_AUTHOK		= 0x00000001,		// �Ѿ���ȨУ�����
			PROP_ROOT		= 0x00000002,		// ���й���Ȩ�޵�����
		};
		int					nSlot;				// ��Ӧ��slot��
		unsigned int		nProp;				// ���ԣ�ȡֵΪPROP_XXX
		void	clear()
		{
			nSlot	= 0;
			nProp	= 0;
		}
	};
	whunitallocatorFixed<ClientUnit>	m_Clients;
	struct	TQUNIT_T
	{
		typedef	void (whnetlocalhiccup_server_I::*TEDEAL_T)(TQUNIT_T *);
		TEDEAL_T	tefunc;											// ������¼��ĺ���
		// ���ݲ���
		union
		{
			int	nClientID;											// ���ID
		}un;
	};
	whtimequeue							m_TQ;
	// ��ǰ��ʱ��
	whtick_t							m_tickNow;
private:
	// <<����>>
	// ʱ���¼�����
	void	TEDeal_ClientPass_TimeOut(TQUNIT_T *pTQUnit);
	// ͨ��slot�Ż��client����ָ��
	ClientUnit *	GetClientBySlot(int nSlot);
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETLOCALHICCUP_I_H__
