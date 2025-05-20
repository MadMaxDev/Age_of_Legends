// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetGenericSvrCli.h
// Creator      : Wei Hua (κ��)
// Comment      : ͨ�õķ�����/�ͻ���ģ��
//              : ClientID��ʵ���Ͼ������ӵ�ID
//              : �ϲ���SENDDATA/RECVDATA֮���Լ�ָ���߼���ص�ͨѶЭ��
// CreationDate : 2005-03-04
// Change LOG   :

#ifndef	__WHNETGENERICSVRCLI_H__
#define	__WHNETGENERICSVRCLI_H__

#include "whnetcmn.h"

namespace n_whnet
{

class	whnetGenericSvrCli
{
public:
	enum
	{
		TYPE_SMPTCPSVR				= 10,					// ��TCP������
		TYPE_SMPTCPCLI				= 11,					// ��TCP�ͻ���
	};
public:
	static whnetGenericSvrCli *		Create(int nType);
	virtual ~whnetGenericSvrCli()	{}
public:
	virtual	int	Init(const char *cszCFG)					= 0;
	virtual	int	Release()									= 0;
	virtual	int	DoSelect(int nInterval)						= 0;
	virtual	int	Tick()										= 0;
	virtual	int	CmdIn(int nCmd, const void *pData, size_t nDSize)
															= 0;
public:
	class	CmdDeal
	{
	public:
		virtual ~CmdDeal()	{}
		virtual int	CallFunc(int nCmd, const void *pData, size_t nDSize)
															= 0;
	};
protected:
	CmdDeal							*m_pCmdDeal;			// �������������߷�������
public:
	// ����ָ��ص�����(!!!!���������Init֮ǰ���ã�������������ò�Ҫ��!!!!)
	inline void	SetCmdDeal(CmdDeal *pCmdDeal)
	{
		m_pCmdDeal	= pCmdDeal;
	}
	// (һ����������ã���������whnetGenericSvrCli_SMTCPSVR��whnetGenericSvrCli_SMTCPCLI�ȵ�)���ûص�
	inline int	RPLCmdCall(int nCmd, const void *pData, size_t nDSize)
	{
		return	m_pCmdDeal->CallFunc(nCmd, pData, nDSize);
	}

////////////////////////////////////////////////////////////
// ָ����ĸ��ֽṹ�Ͷ���{
////////////////////////////////////////////////////////////
public:
	// ��Щָ��ܳ���65535
	// REQָ�������������
	enum
	{
		// �������õ���ָ��
		SVRCMD_REQ_STARTSERVER		= 1,					// ����������
															// �ṹ��SVRCMD_REQ_STARTSERVER_T
		SVRCMD_REQ_STOPSERVER		= 2,					// ����������
															// �ṹ����
		SVRCMD_REQ_SENDDATA			= 3,					// ��ĳ���ͻ��˷�������
															// �ṹ��SVRCMD_REQ_SENDDATA_T
		SVRCMD_REQ_DISCONNECTCLIENT	= 4,					// �رտͻ��˵�����
															// �ṹ��SVRCMD_REQ_DISCONNECTCLIENT_T
		SVRCMD_REQ_SETCLIENTTAG		= 5,					// ����ĳ���ͻ��˵ı��
															// �ṹ��SVRCMD_REQ_SETCLIENTTAG_T
		SVRCMD_RPL_STATUS			= 100,					// ����������״̬֪ͨ
															// �ṹ��SVRCMD_RPL_STATUS_T
		SVRCMD_RPL_CLIENTIN			= 101,					// ֪ͨ�пͻ�������
															// �ṹ��SVRCMD_RPL_CLIENTIN_T
		SVRCMD_RPL_CLIENTOUT		= 102,					// ֪ͨ�пͻ��˶���
															// �ṹ��SVRCMD_RPL_CLIENTOUT_T
		SVRCMD_RPL_RECVDATA			= 103,					// ĳ���ͻ��˷���������
															// �ṹ��SVRCMD_RPL_RECVDATA_T
		// �ͻ����õ���ָ��
		CLICMD_REQ_CONNECT			= 1,					// ���ӷ�����
															// �ṹ��CLICMD_REQ_CONNECT_T
		CLICMD_REQ_DISCONNECT		= 2,					// �ӷ������Ͽ�
															// �ṹ����
		CLICMD_REQ_SENDDATA			= 3,					// ���������������
															// �ṹ��char[1]
		CLICMD_RPL_STATUS			= 100,					// �ͻ���״̬֪ͨ(�������ӳɹ���������ߵȵ�)
															// �ṹ��CLICMD_RPL_STATUS_T
		CLICMD_RPL_RECVDATA			= 101,					// �ӷ������������
															// �ṹ��char[1]
	};
	enum
	{
		// ������״̬
		SVRSTATUS_NOTHING			= 0,
		SVRSTATUS_STARTING			= 1,					// ������������
		SVRSTATUS_WORKING			= 10,					// ��������״̬
		SVRSTATUS_STOPING			= 20,					// �������ر���
		SVRSTATUS_INTERNALERROR		= -1,					// �ڲ�����
		SVRSTATUS_SOCKETERROR		= -2,					// �������
		// �ͻ���״̬
		CLISTATUS_NOTHING			= 0,
		CLISTATUS_CONNECTING		= 1,					// �������ӷ�����
		CLISTATUS_WORKING			= 10,					// ���ӳɹ�������������
		CLISTATUS_DISCONNECTING		= 20,					// ���ڴӷ������Ͽ�
		CLISTATUS_CONNECTTIMEOUT	= -1,					// ���ӳ�ʱ
		CLISTATUS_DISCONNECTED		= -2,					// ���������
		CLISTATUS_INTERNALERROR		= -3,					// �ڲ�����(�����ڴ桢����ĳЩ�����ʼ������)
	};
	#pragma pack(1)
	// �������õ��Ľṹ
	struct	SVRCMD_REQ_STARTSERVER_T
	{
		char	szIP[WHNET_MAXADDRSTRLEN];					// ������IP
		int		nPort;										// �����Ķ˿�
	};
	struct	SVRCMD_REQ_SENDDATA_T
	{
		int		nClientID;									// ��Ӧ��ClientID�������-1��ʾ��������(��ʱ�����TagXXX��������)
		int		nTagIdx;									// ���>=0��ʾ��ĳ��λ��Tag��Ȳ��ܷ���
		int		nTagValue;									// ��ʾ��Ӧ��TagӦ�õ��ڵ�ֵ
		char	szData[1];									// ���ݲ����Լ�����
	};
	struct	SVRCMD_REQ_DISCONNECTCLIENT_T
	{
		int		nClientID;									// ��Ӧ�����ID�������-1��ʾ��������(���������TagXXX��������)
	};
	struct	SVRCMD_REQ_SETCLIENTTAG_T
	{
		int		nClientID;
		int		nTagIdx;									// Ҫ���õ�Tag����
		int		nTagValue;									// ��ʾ��Ӧ��TagӦ�õ��ڵ�ֵ
	};
	struct	SVRCMD_RPL_STATUS_T
	{
		int		nStatus;									// ǰ�涨���SVRSTATUS_XXX
	};
	struct	SVRCMD_RPL_CLIENTIN_T
	{
		int		nClientID;									// �������ҵ�ID
		struct sockaddr_in	addr;							// ��ҵ�ַ
	};
	struct	SVRCMD_RPL_CLIENTOUT_T
	{
		enum
		{
			REASON_NOTHING		= 0,
			REASON_CLOSE		= 1,						// �ͻ�����������
			REASON_TIMEOUT		= 2,						// �ͻ��˳�ʱ
		};
		int		nClientID;									// �뿪����ҵ�ID
		int		nReason;									// �뿪��ԭ��
	};
	struct	SVRCMD_RPL_RECVDATA_T
	{
		int		nClientID;									// ��Ӧ�����ID
		char	szData[1];									// ���ݲ���
	};
	// �ͻ����õ��Ľṹ
	struct	CLICMD_REQ_CONNECT_T
	{
		char	szAddr[WHNET_MAXADDRSTRLEN];				// Ŀ���ַ
		int		nTimeOut;									// ���ӳ�ʱʱ��(����)
		CLICMD_REQ_CONNECT_T()
		: nTimeOut(5000)
		{
			szAddr[0]	= 0;
		}
	};
	struct	CLICMD_RPL_STATUS_T
	{
		int		nStatus;									// ǰ�涨���CLISTATUS_XXX
	};
	#pragma pack()
////////////////////////////////////////////////////////////
// ָ����ĸ��ֽṹ�Ͷ���}
////////////////////////////////////////////////////////////
};

}

#endif	// EOF __WHNETGENERICSVRCLI_H__
