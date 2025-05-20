// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2.h
// Creator      : Wei Hua (κ��)
// Comment      : ����UDP������ͨѶЭ��2005������
//                CNL��Clever Net Layer����д
//                һ��Man�ɹ�������������������8192��
//                ���嵥Ԫ�ĸ���������65536��
//                Noop����KeepAlive
//                Noop��client�˷��𣬾�����������������˫�������Ի��RTTֵ
//                2005-07-15 ע�⣺����Ҫ��ÿ��ѭ���ڵ���TickRecv֮�󣬵���Recv���������ݶ����ոɾ���
//                           ���������ʱ�䲻����Recv������In��������֮�󣬷��ͷ��Ͳ����ٷ����κ����ݡ�
//                           �����շ���Ϊ���ղ����κ����ݣ��Ͳ�����������ȷ�ϣ��Ӷ����·��ͷ���Զ�����ٷ��������ˡ�
//                ��Ȼ�������Recv�б��channel��Ҫȷ�ϣ�ͬʱ��Recv����֮������Slot��Ҫ����Confirm�Ϳ��Խ��������⡣
//                           Ŀǰͨ�������CNL2_RECV_TRIGER_CONFIRM��ʵʩ���ֽ������
// CreationDate : 2005-04-16
// ChangeLog    : 2005-07-29 ͻȻ�뵽���ڵ�CNL2��������̫���TCP����Ҫ�Ĳ��������������������ʵ��TCP��CNLҲ��̫���ʡ����Է���������㡣
//                2005-08-25 ������ReConfigChannel�����������趨channel����
//                2006-01-23 ��ԭ����CNL2SlotMan::INFO_T::nCryptorType��Ϊ��Χ�����ܷ�ʽΪ���������һ��type֮�󴫸��ͻ��� 
//                2006-11-29 ������nMaxNoopTime��nMinNoopTime�Ա����������ʱ���ù������¿ͻ���û���κ����ݷ���ʱ��ʱ����
//                2007-06-12 �޸���CNL2_VER=1����Ҫ��Ϊ��PNGSClient�޸��˿ͻ���Э�飬ǿ�ƴ��ͳһ��
//                2007-08-09 �޸���CNL2_VER=2����Ϊ������IP�ı��Ծɿ��Զ��������Ĺ��ܣ�
//                2007-08-14 ��CNL2��Ĭ�Ϸ��ͺͽ���buffer����Ϊ512k
//                2007-12-05 �޸��˼��ܷ�ʽ������ײ�汾Ҳ�޸�Ϊ3
//                2009-03-03 CNL2_MAX_DATAFRAMESIZE��Ϊ1024����Ϊ̨��ļ���·������MTUĬ����1400��ͷ�����»��ж�����

#ifndef	__WHNETCNL2_H__
#define	__WHNETCNL2_H__

#include "./whnetcmn.h"
#include <WHCMN/inc/whfile.h>
#include <WHCMN/inc/whtime.h>
#include <WHCMN/inc/whstring.h>
#include <WHCMN/inc/whdataini.h>
#include <WHCMN/inc/wh_crypt_interface.h>

using namespace n_whcmn;

namespace n_whnet
{

// һЩ����
enum
{
	CNL2_VER					= 4,								// ��ǰ�İ汾
	CNL2_CHANNELNUM_MAX			= 8,								// ����ͨ����
	CNL2_MAX_DATAFRAMESIZE		= 1024,								// һ��UDP�������ߴ硣һ��������̫����(��̫����MTU, Maximum Transmission Unit, ��1500��UDP��IP���а�ͷ�ģ����Ի��ǽ�ԼһЩ��)
																	// һ��IP�ײ�Ϊ20�ֽڣ�UDP�ײ�Ϊ8�ֽڣ����ݵľ��ɣ�payload������Ԥ���� 
																	// 1500-20-8=1472�ֽڡ�������ݲ��ִ���1472�ֽڣ��ͻ���ַ�Ƭ���� 
																	// ����ADSL��MTU��1492��������Ҫ�ټ�8
																	// ������1464
																	// Ϊ�˱�������ˮһЩ
																	// 2005-05-10 ������ping -f -l size addr�ķ�ʽ����MTU.
																	// �Ҳ��Ե�adsl��1480����ping���Է��͵����fragment��1452
																	// Ϊ�˱��վ��ٽ�20
																	// 2009-03��Ϊ1024����Ϊ̨��ļ���·������MTUĬ����1400��ͷ�����»��ж�����
	CNL2_MAX_DATATOSEND			= 1000000,							// ����ͨ��Sendһ�η��͵��������(������ڳ������ݻ����Լ��ȷֽ���ٷ���Ȼ����Ҫ�ͻ�����������ÿ��ȷ�ϵģ��������������Ļ���̫��)
	CNL2_MAX_INOUTQUEUESIZE		= 0x8000,							// ÿ��channel
	CNL2_DFT_PING				= 200,								// Ĭ�ϵ���ʼ������ʱ
	CNL2_MAX_PING				= 2000,								// ����������ʱ(��������������������˵������ǳ������ˣ������������ʱ�������)
	CNL2_MIN_PING				= 4,								// ��С��������ʱ(�����ʱΪ0�Ļ���������)
	CNL2_MAX_SLOTEXTDATA_SIZE	= 1024,								// ���ĸ��Ӳ�������
};
enum
{
	CNL2_CHANNELTYPE_NOTHING	= 0,								// δ�����channel
	CNL2_CHANNELTYPE_RTDATA		= 1,								// ���ɿ��ģ�ʵʱ���ݣ�����֤˳��
	CNL2_CHANNELTYPE_SAFE		= 2,								// �ɿ��ģ������ȳ���
	CNL2_CHANNELTYPE_LASTSAFE	= 3,								// ��ɿ��ģ���֤˳�򣬵����м���Զ�ʧ���ݵ�
																	// ����channel��In/Out���ж���ֻ��һ��Ԫ�أ���ʼ�������б�ʾ���г��ȵĵ���Ӧ�����ᱻ����
};

// ��Ϊ��������dll�����˵��ã�������Ҫ�޶��뷽ʽ��������Ϊ�ṹ���������Ա����Ϊ4�������൱��4�ֽڶ��룩
#pragma pack(8)
struct	CNL2CHANNELINFO_T
{
	enum
	{
		PROP_CANSEND			= 0x01,								// �ɷ�������
		PROP_CANRECV			= 0x02,								// �ɽ�������
		PROP_FASTACK			= 0x04,								// ϣ������ȷ��(������Ϣͨ���Ͳ���Ҫ����ȷ��)
	};
	int		nType;													// CNL2_CHANNELTYPE_XXX
	int		nInNum;													// ������еĳ���
	int		nOutNum;												// �������еĳ���
	unsigned char	nProp;											// ����
	CNL2CHANNELINFO_T()
	: nType(CNL2_CHANNELTYPE_NOTHING)
	, nInNum(0), nOutNum(0)
	, nProp(0)
	{
	}

	void	clear()
	{
		nType	= CNL2_CHANNELTYPE_NOTHING;
		nInNum	= 0;
		nOutNum	= 0;
		nProp	= 0;
	}
};
struct	CNL2SLOTINFO_T
{
	enum
	{
		STATUS_NOTHING		= 0,
		STATUS_CONNECTING	= 1,									// ��������
		STATUS_ACCEPTING	= 2,									// ���Խ���
		STATUS_WORKING		= 3,									// ���ӳɹ�����������
		STATUS_CLOSING		= 4,									// ���ڹر���
	};
	int		nStatus;												// Ŀǰ״̬
	int		nSlot;													// Slot��ID
	short	nPeerSlotIdx;											// �Է���Slot����ţ�ע�ⲻ��ID
	struct	sockaddr_in		peerAddr;								// �Է���ַ
	int		nRTT;													// ������ʱ�����صģ�
	int		nUpBPS;													// ���д����ʣ����͵ģ�
	int		nDownBPS;												// ���д����ʣ����յģ�
	short	nLostRate;												// �����ʣ��ٷ��ʣ�
	unsigned int	nSpecialSeed;									// ���������¼�����ת����

	whtick_t		nLastSend;										// �ϴη�������ʱ��
	whtick_t		nLastRecv;										// �ϴν�������ʱ��
	void	clear()
	{
		memset(this, 0, sizeof(*this));
		nLastSend	= wh_gettickcount();
		nLastRecv	= nLastSend;
		nRTT		= CNL2_DFT_PING;
	}
	const char *	infostr(char *pszStr)										// ��ӡ���ִ�
	{
		sprintf(pszStr
			, "status:%d,slot:%d,peerslotidx:%d,peeraddr:%s"
			  ",rtt:%d,upbps:%d,downbps:%d,lostrate:%d,specialseed:%d"
			  ",lastsend:%u,lastrecv:%u"
			, nStatus, nSlot, nPeerSlotIdx, cmn_get_ipnportstr_by_saaddr(&peerAddr)
			, nRTT, nUpBPS, nDownBPS, nLostRate, nSpecialSeed
			, nLastSend, nLastRecv
			);
		return	pszStr;
	}
	inline void	calcRTT(int nNewDelay)
	{
		nRTT	= (nRTT*15 + nNewDelay) / 16;
		if( nRTT > CNL2_MAX_PING )
		{
			nRTT	= CNL2_MAX_PING;
		}
		else if( nRTT < CNL2_MIN_PING )
		{
			nRTT	= CNL2_MIN_PING;
		}
	}
};
#pragma pack()
struct	CNL2SlotMan
{
	// �ṹ
	struct	INFO_T
	{
		char				szBindAddr[WHNET_MAXADDRSTRLEN];		// �󶨵ı��ص�ַ(ȫ�յ��ִ�������������������ĳ�����ö˿�)
		unsigned char		ver;									// ����У���Ƿ���ͬ������
		bool				bUseTickPack;							// ��ͬһ֡���������ݾ�������Ϊһ��������
		bool				bUseCompression;						// �Ƿ�ʹ�ö԰���ѹ����ע�⣺��pack����ôѹ������û������ģ�
		bool				bCanSwitchAddr;							// �Ƿ��������Ϊ���ߵ�ʱ��ı��Լ��ĵ�ַ
		bool				bLogNetStat;							// �����������ͳ�Ƶ�LOG��
		bool				bLogSendRecv;							// ������ͺͽ��յİ���cmn��־��
		int					nMaxConnection;							// ����������Ŀ(������������)
		int					nSockSndBufSize;						// socket�ķ��ͻ���
		int					nSockRcvBufSize;						// socket�Ľ��ջ���
																	// ע��nSockSndBufSize��nSockRcvBufSize��windows�¿�����ĺܸߣ���������ͨ���õ�linuxĬ��ֻ���赽131072���μ�/proc/sys/net/core��rmem_max��wmem_max�����Ϊ0��ʾ�������衣
		int					nMaxSinglePacketSize;					// �߼��ϵ���һ���������ߴ�
																	// �����Ӧ�ó���CNL2_MAX_DATAFRAMESIZE*channel��OutNum����Сֵ�����������Ĵ�ָ��ָ���϶�����������������
		// ��ʱ����
		int					nConnectTimeout;						// ����һ�����ӵĳ�ʱ�������ӱ��˵ĳ�ʱ
		int					nConnectRSDiv;							// �����ط�ʱ��ΪnConnectTimeout/nConnectRSDiv
		int					nCloseTimeout;							// �ر�ʱ�ĵȴ�
		int					nDropTimeout;							// ������ʱ
		int					nNoopDivDrop;							// ���һ��ʱ��û�з���Send�ͷ���Noop
		int					nMaxNoopTime;							// ���Noopʱ�䣨���nDropTimeout���õĺܴ�Ļ�����ô���ܻᵼ��Noop��ʱ��ܳ�����������Է�û�н�����Ӧ�����õĻ����ͻᵼ�¶Է���ʱ���ߣ�
		int					nMinNoopTime;							// ��̵�Noopʱ�䣨��Ϊһ�㳬ʱ�������õ�̫С��
																	// Noop�ĳ�ʱӦ����max(min(nDropTimeout/nNoopDivDrop, nMaxNoopTime), nMinNoopTime)
		// �������ӵ�����ͽ�ֹ�ĵ�ַ�ļ��б�
		char				szConnectAllowDeny[WH_MAX_PATH];		// �ļ��������Ϊ�ձ�ʾ��ʹ��
		int					nConnectAllowDenyRefreshInterval;		// ˢ�¸��ļ����ݵļ�������룩
		// ʱ�����
		int					nTQChunkSize;							// ��Ӧwhtimequeue::INFO_T�е�nChunkSize
																	// ���ڷ����������԰�������ıȽϸߣ�����10000���ͻ��˿��Ե�һЩ������100��
		// ����֪ͨ�ϲ��ָ����г���
		int					nControlOutQueueSize;
		// ��������(��һ����Χ�����)
		int					nCryptorType1, nCryptorType2;
		// ͨ����Ϣ
		CNL2CHANNELINFO_T	channelinfo[CNL2_CHANNELNUM_MAX];
		// chunk��������Ϣ����Ҫ�������������ķ������ݰ�����ģ��������ڲ��õ���whchunkallocationobj��
		int					nAllocChunkSize;
		int					nAlloc_MAXSIZENUM;
		int					nAlloc_HASHSTEP1;
		int					nAlloc_HASHSTEP2;

		int					nStatInterval;							// ��¼ͳ�Ƶļ��

		//
		char				szRSAPriKeyFile[WH_MAX_PATH];			// RSA˽Կ�ļ�
		char				szRSAPass[64];							// RSA����
		char				szAuthFile[WH_MAX_PATH];				// ��Ȩ�ļ������ܺ�

		INFO_T()
		: ver(CNL2_VER)
		, bUseTickPack(true)
		, bUseCompression(false)
		, bCanSwitchAddr(false)
		, bLogNetStat(false)
		, bLogSendRecv(false)
		, nMaxConnection(2)
		, nSockSndBufSize(512*1024)		// 2007-08-14 ��Ĭ��buffer����Ϊ512k
		, nSockRcvBufSize(512*1024)
		, nMaxSinglePacketSize(65536)
		, nConnectTimeout(10000)
		, nConnectRSDiv(6)
		, nCloseTimeout(2000)
		, nDropTimeout(20000)
		, nNoopDivDrop(8)
		, nMaxNoopTime(40000), nMinNoopTime(20)
		, nConnectAllowDenyRefreshInterval(30*1000)
		, nTQChunkSize(100)
		, nControlOutQueueSize(10000)
		, nCryptorType1(0), nCryptorType2(9)
		, nAllocChunkSize(64*1024)
		, nAlloc_MAXSIZENUM(200)
		, nAlloc_HASHSTEP1(1543)
		, nAlloc_HASHSTEP2(97)
		, nStatInterval(20*1000)
		{
			szBindAddr[0]			= 0;
			szConnectAllowDeny[0]	= 0;
			WH_STRNCPY0(szRSAPriKeyFile, "auth-pri.key");
			WH_STRNCPY0(szRSAPass, "tmxwwudi");
			WH_STRNCPY0(szAuthFile, "auth-info.key");
		}

		void	clearallchannelinfo()
		{
			for(int i=0;i<CNL2_CHANNELNUM_MAX;i++)
			{
				channelinfo[i].clear();
			}
		}
	};
	// ���Ϊ�˿���ͨ�������ļ��Զ����
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
	// ����
	enum
	{
		TYPE_UDP			= 0,
		TYPE_TCP			= 1,
	};
	// ��ס���Ĭ��ֵ��Զ��Ҫ���ˣ���Ϊ�ܶ�ط�������
	// ���Create�����Ķ�����Ҫ����SelfDestroyɾ�����Ƽ�ʹ��WHSafeSelfDestroy��
	static	CNL2SlotMan *	Create(int nType=TYPE_UDP);
protected:
	virtual	~CNL2SlotMan()	{};
public:
	// ��������
	virtual	void	SelfDestroy()									= 0;
	// һ֡�����Send�����ݳ���(���Է��͸��࣬�������ܻᵼ���ڲ���Ƭ)
	virtual	int		GetMaxSingleDataSize()							= 0;
	// ��ʼ�������ر��ذ󶨵�ַ
	virtual	int		Init(INFO_T *pInfo, struct sockaddr_in *pAddr)	= 0;
	virtual	int		Init_Reload(INFO_T *pInfo)						= 0;
	// ��������channel����(�����Ҫ�������ɴ����Ӷ�Channel��Ҫ��ͬ���磺��CAAFS��ʱ��ֻ��һ��Channel����CLS��ʱ����Ҫ4��)
	// ����Ҫ�м�����������õ�ʱ����뱣֤ǰ�������slot���Ѿ��ر���
	virtual	int		ReConfigChannel(INFO_T *pInfo)					= 0;
	// �ս�
	virtual	int		Release()										= 0;
	// ��ÿɱ������select��SOCKET��append��vector��β��(�����ϲ��ʼʹ��ǰ��Ҫvect.clearһ��)
	// �����Ҫ�Ǹ����ͳһselect��(��Ϊʵ���Ͼ�һ��socket)
	virtual	int		GetSockets(whvector<SOCKET> &vect)				= 0;
	// ������ڼ��ʹ��
	virtual	SOCKET	GetSocket() const								= 0;
	// �ڲ�select
	virtual	int		DoSelect(int nMS)								= 0;
	// ������յ�����
	virtual	int		TickRecv()										= 0;
	// �����ڲ��߼��ͷ���
	virtual	int		TickLogicAndSend()								= 0;
	// �����Ƿ�listen��Ĭ��listen��false�������������ӣ�
	virtual	int		Listen(bool bListen)							= 0;
	// ����Ŀ���ַ���������>0���ʾslotid
	// �������ݵĳ��Ȳ��ܳ���255
	virtual	int	Connect(struct sockaddr_in *addr, void *pExtData=NULL, int nExtSize=0)
																	= 0;
	virtual	int	Connect(const char *cszAddr, void *pExtData=NULL, int nExtSize=0)
																	= 0;
	// �ر�����
	virtual	int		Close(int nSlot, int nExtData=0)				= 0;
	// �ر���������
	virtual	int		CloseAll()										= 0;
	// ��������
	virtual	int		Send(int nSlot, const void *pData, size_t nSize, int nChannel)
																	= 0;
	// *ppDataָ������ڲ���һ��vector����
	// ����0��ʾ������
	// ע�⣺����Ҫ��ÿ��ѭ���ڵ���TickRecv֮�󣬵���Recv���������ݶ����ոɾ�
	// ���������ʱ�䲻����Recv������In��������֮�󣬷��ͷ��Ͳ����ٷ����κ�����
	// �����շ���Ϊ���ղ����κ����ݣ��Ͳ�����������ȷ�ϣ��Ӷ����·��ͷ���Զ�����ٷ���������
	virtual	int		Recv(int *pnSlot, int *pnChannel, void **ppData, size_t *pnSize)
																	= 0;
	// ָ���
	enum
	{
		CONTROL_OUT_SLOT_ACCEPTED		= 1,						// Slot�������
																	// data[0]Ϊslot��
		CONTROL_OUT_SLOT_CONNECTED		= 2,						// Slot�������
																	// data[0]Ϊslot��
		CONTROL_OUT_SLOT_CLOSED			= 3,						// Slot�ر�(�����ǳ�ʱ�������ر�)
																	// CONTROL_OUT_SLOT_CLOSE_REASON_T
		CONTROL_OUT_SLOT_CONNECTREFUSED	= 4,						// Slot���ӱ��ܾ�
																	// CONTROL_OUT_SLOT_CLOSE_REASON_T
		CONTROL_OUT_SHOULD_STOP			= 5,						// ֪ͨ�ϲ�ر�
																	// ���޲���
	};
	enum
	{
		// �����Ķ���CLS_GMS_CLIENT_DROP_T::REMOVEREASON_XXX
		// ע���������޸���Ҳ��Ҫ�ڹ����ĵط��޸�
		SLOTCLOSE_REASON_NOTHING		= 0x00,						// δ֪����
		SLOTCLOSE_REASON_INITIATIVE		= 0x01,						// �����ر�
		SLOTCLOSE_REASON_PASSIVE		= 0x02,						// �����ر�
		SLOTCLOSE_REASON_CLOSETimeout	= 0x03,						// �ر�ʱ��ʱ
		SLOTCLOSE_REASON_DROP			= 0x04,						// ��ʱ�ر�
		SLOTCLOSE_REASON_CONNECTTimeout	= 0x05,						// ���ӹ����г�ʱ�ر�
		SLOTCLOSE_REASON_ACCEPTTimeout	= 0x06,						// ���ܹ����г�ʱ�ر�
		SLOTCLOSE_REASON_KEYNOTAGREE	= 0x07,						// KEY��������
	};
	enum
	{
		CONNECTREFUSE_REASON_NOTHING		= 0x00,					// û��ԭ�򣬾��ǿ��㲻˳��
		CONNECTREFUSE_REASON_BADVER			= 0x01,					// �汾����
		CONNECTREFUSE_REASON_SLOTFULL		= 0x02,					// Slot�Ѿ����䵽���ֵ
		CONNECTREFUSE_REASON_BADDATA		= 0x03,					// ���ݴ���
		CONNECTREFUSE_REASON_INTERNALERROR	= 0xFF,					// �ڲ�����(�����ڴ�������)
	};
	#pragma pack(1)
	struct	CONTROL_T
	{
		int			nCmd;
		int			data[1];
	};
	struct	CONTROL_OUT_SLOT_CLOSE_REASON_T
	{
		int			nCmd;
		int			nSlot;											// ����о��Ƕ�ӦSlot��ID
		struct	sockaddr_in		peerAddr;							// �Է���ַ
		unsigned char	nReason;									// SLOTCLOSE_REASON_XXX
																	// CONNECTREFUSE_REASON_XXX
		int				nExtData;									// �رյĸ�������
		long			nSlotExt;									// Slot��������(��Ϊ�����close�����Ҫɾ����ص��ϲ��߼�����)
	};
	#pragma pack()
	// ����ڲ�֪ͨ������ĳ��slot�Ѿ����ߣ�
	// ����0��ʾ�����ݣ�-1��ʾû����
	virtual	int		ControlOut(CONTROL_T **ppCmd, size_t *pnSize)	= 0;
	// ���SLOT��Ϣ(�������޸��䲿������)
	virtual	CNL2SLOTINFO_T *	GetSlotInfo(int nSlot)				= 0;
	// �ж�slot�Ƿ����
	virtual	bool	IsSlotExist(int nSlot) const					= 0;
	// ��õ�ַ�������
	virtual cmn_addr_allowdeny &	GetConnectAllowDeny()			= 0;
	// ������Ӹ�������
	virtual	whvector<char> *	GetConnectExtData(int nSlot)		= 0;
	// ���ù�������
	virtual	int		SetExtLong(int nSlot, long nExt)				= 0;
	// ��ȡ��������
	virtual	int		GetExtLong(int nSlot, long *pnExt)				= 0;
	// ���ü��ܹ���(�ϲ�ע�Ᵽ֤��SlotMan::Release֮����ܰ�pICryptFactory�ս��)
	virtual	void	SetICryptFactory(ICryptFactory *pICryptFactory)
																	= 0;
	// ��õ�ǰ��slot����
	virtual	int		GetSlotNum() const								= 0;

	// ����ʹ��auth�ļ�
	virtual	void	SetUseAuth()									= 0;
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETCNL2_H__
