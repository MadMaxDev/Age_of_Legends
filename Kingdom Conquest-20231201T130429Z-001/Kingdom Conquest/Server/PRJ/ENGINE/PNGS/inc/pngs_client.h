// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_client.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS��Clientģ��(���Client�Ǻ;����߼��޹صģ�ֻʵ�����Ӻͻ���ͨѶ)
//                PNGS��Pixel Network Game Structure����д
// CreationDate : 2005-08-25
// Change LOG   : 2006-11-23 ���ӵײ�Ĳ�����ܣ�Ϊ�˽��������ڲ�����������������������ƹ�����׼����
//              : 2007-06-18 �����˻��������ʱ�Ĺ���

#ifndef	__PNGS_CLIENT_H__
#define	__PNGS_CLIENT_H__

#include <WHCMN/inc/wh_crypt_interface.h>
#include <WHCMN/inc/whvector.h>
#include <WHNET/inc/whnetcmn.h>
#include "pngs_cmn.h"
#include "pngs_packet.h"

namespace n_pngs
{

class	PNGSClient
{
public:
	// ��ʼ��DLL��
	static void		DLLPool_Init();
	// �ս�DLL��
	static void		DLLPool_Release();
	// �����ͻ��˶���
	static PNGSClient *		Create();
protected:
	virtual	~PNGSClient()	{}
public:
	enum
	{
		CONTROL_OUT_CONNECT_RST			= 1,						// ���ӷ������Ľ��
		CONTROL_OUT_STATUS				= 2,						// ״̬�ı�֪ͨ
																	// �ṹCONTROL_T��nParam[0]��ֵ����Status
																	// status�Ķ��������ģ�����������Ϣ��STATUS_XXX
		CONTROL_OUT_DROP				= 3,						// ���ӹ����������ж�
																	// �ṹCONTROL_T��nParam[0]��ֵ�����ж�����
		CONTROL_OUT_QUEUEINFO			= 4,						// ��CAAFS�еȴ������е���Ϣ
		CONTROL_OUT_SERVICE_INTERMIT	= 5,						// ��������ʱ�жϣ���ȴ��ָ�
																	// �ṹCONTROL_T��nParam[0]��ֵ�������ַ����ж��ˣ���ʱû�ж��壩
		CONTROL_OUT_NETWORK_INTERMIT	= 6,						// ������ʱ�жϣ���ȴ��ָ�
	};
	// ����������Ϣ
	enum
	{
		STATUS_NOTHING					= 0,						// û��״̬������֮�����������״̬��
		STATUS_CONNECTINGCAAFS			= 1,						// ��������CAAFS��
		STATUS_WAITINGINCAAFS			= 2,						// ����CAAFS�еȴ����ӻ���
		STATUS_CONNECTINGCLS			= 3,						// ��������CLS��
		STATUS_WORKING					= 4,						// ���߳ɹ���������
		STATUS_CONNECTFAIL				= 5,						// ����ʧ��
		STATUS_DROPPED					= 6,						// �����к�CLS����
	};
	// ������Ϣ
	enum
	{
		ERR_OK							= 0,						// û�д���
		// ���Ӵ���
		ERR_CONNECT_LASTCONNECTNOTOVER	= -1,						// �ϴε����ӻ�û�ж�
		ERR_CONNECT_CNL2_CONNECTERROR	= -2,						// CNL2��Connect�������ش���
	};
	#pragma pack(push, old_pack_num, 1)
	struct	CONTROL_T
	{
		short			nCmd;										// �μ�ǰ���CONTROL_OUT_XXX
		int				nParam[1];
	};
	struct	CONTROL_OUT_CONNECT_RST_T	
	{
		enum
		{
			RST_OK						= 0,						// ���ӳɹ�
			RST_NET_ERR_CAAFS			= -1,						// �������
			RST_REFUSED_BY_CAAFS		= -2,						// ���ӱ��������ܾ�
			RST_NET_ERR_DROP_CAAFS		= -3,						// ��CAAFS�����ж�
			RST_CAAFS_REFUSEALL			= -4,						// CAAFS�ܾ����е�����
			RST_NET_ERR_CLS				= -11,						// ����CLS�������
			RST_REFUSED_BY_CLS			= -12,						// ����CLS���������ܾ�
			RST_NET_ERR_DROP_CLS		= -13,						// ��CLS�����ж�
			RST_BAD_TYPE_SERVER			= -21,						// ���ӵ�server�����Լ�ϣ�����ӵ�
			RST_BAD_VER					= -22,						// �Լ��ͷ������汾��ƥ��
		};
		short			nCmd;
		short			nRst;										// ���ӽ��
		int				nClientID;									// ����ڷ������е�ID
		struct sockaddr_in		CLSAddr;							// �����ϵ�CLS�ĵ�ַ
	};
	struct	CONTROL_OUT_QUEUEINFO_T
	{
		short			nCmd;
		unsigned char	nVIPChannel;								// ���ڵ�VIPͨ���������0�����ڿͻ�����ʾ����
		unsigned short	nNumToWait;									// ǰ�滹�ж��������Ŷ�
	};
	#pragma pack(pop, old_pack_num)
public:
	// �Լ������Լ�
	virtual	void	SelfDestroy()									= 0;

	// ���������ļ���ʼ��
	virtual	int		Init(const char *cszCFG)						= 0;

	// �ս�
	virtual	int		Release()										= 0;

	// ���ü��ܹ���(�ϲ�ע�Ᵽ֤��CAAFS2::SelfDestroy֮����ܰ�pICryptFactory�ս��)
	virtual	int		SetICryptFactory(n_whcmn::ICryptFactory *pFactory)
																	= 0;
	// ��ö�Ӧ��socket���ϲ�select
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect)		= 0;

	// ����һ��
	virtual	int		TickRecv()										= 0;
	virtual	int		TickSend()										= 0;

	// ���ÿͻ��˰汾
	virtual	void	SetVer(const char *szVer)						= 0;

	// ��ÿͻ��˰汾
	virtual	const char *	GetVer()								= 0;

	// ���ӷ�����
	// ����ǰ�涨���ERR_XXX
	virtual	int		Connect(const char *cszAddr, unsigned char nNetworkType = NETWORKTYPE_UNKNOWN)
																	= 0;

	// �Ͽ�����
	virtual	int		Disconnect()									= 0;

	// ���ƽ�����(�磺���ӽ�������ߵȵ�)
	virtual	int		ControlOut(CONTROL_T **ppCmd, size_t *pnSize)	= 0;

	// ����ָ��
	virtual	int		Send(unsigned char nSvrIdx, unsigned char nChannel, const void *pCmd, size_t nSize)
																	= 0;
	// ����ָ���CAAFS
	virtual	int		SendCAAFS(const void *pCmd, size_t nSize)		= 0;

	// ��ȡָ��
	virtual	int		Recv(unsigned char *pnChannel, void **ppCmd, size_t *pnSize)
																	= 0;
	// ������Ĵ�����
	virtual	int		GetLastError() const							= 0;

	// ������Ĵ�����
	virtual	void	SetLastError(int nErrCode=ERR_OK)				= 0;

	// �����ļ������������������Init֮ǰ���ã�
	virtual void	SetFileMan(n_whcmn::whfileman *pFileMan)		= 0;
	// ������
	// ���PlugInָ��(�����ͨ�������ļ������)
	virtual	ILogicBase *	GetPlugIn(const char *cszPlugInName, int nVer)
																	= 0;
	// �������pingֵ�����룩
	virtual int		GetPing() const									= 0;

	// ��ʹ���߳̿��ܻ�������ʱ�������������̣߳�����ע��һ��Ҫ����������recv��send֮ǰֹͣ�̣߳����򷵻��Ǵ����
	virtual int		StartThread()									= 0;
	virtual int		StopThread()									= 0;
};

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CLIENT_H__
