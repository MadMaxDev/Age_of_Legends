// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i.h
// Creator      : Wei Hua (κ��)
// Comment      : CNL2���ڲ�ʵ��
// CreationDate : 2005-04-17
// ChangeLog    : 2005-07-13 client�����Ӻ�ͷ�������nooptimeout����һ��
//                2005-07-13 ȡ��CNL2_CMD_NOOP_T�����Confirm���ݵ���ơ���Ϊ���ȵ�����NOOP��ʱ��Ӧ���Ѿ�����Ҫȷ���ˡ�
//                ÿ���յ����ݵ�tick�ڶ��ᵼ�±�tick���յ�������ȷ�ϱ����͡�
//                2005-08-15 ��Connect�����˸��Ӳ�������Ҫ�Ǹ�PNGS��Client��CLS�����õģ�
//                2005-08-21 ������ÿ��channel�����п�SEND�Ϳ�RECV�����ԣ����û���������������ܷ��ͻ��߽�������
//                2006-05-14 CNL2_CMD_DATA_T��������һ�ֽڣ���Ҫ������packλ���Ӵ������˴�����ݣ���dataphase��CNL2_CMD_0_T���Ƴ������
//                2007-12-03 �ѱ�tick�������͵����ݲ��ŵ�ʱ���������

#ifndef	__WHNETCNL2_I_H__
#define	__WHNETCNL2_I_H__

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif
#include "./whnetcnl2.h"
#include "./whnetudp.h"
#include "./whnetudpGLogger.h"
#include <WHCMN/inc/whunitallocator.h>
#include <WHCMN/inc/whallocator2.h>
#include <WHCMN/inc/whtimequeue.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whdbg.h>
#include <WHCMN/inc/whcrc.h>
#include <WHCMN/inc/whcmd.h>
#include <WHCMN/inc/whfile_util.h>

using namespace n_whcmn;

namespace n_whnet
{

typedef	unsigned short	cnl2idx_t;
typedef	short			cnl2idxdiff_t;
inline cnl2idxdiff_t	cnl2_idxdiff(cnl2idx_t idx1, cnl2idx_t idx2)
{
	return	(cnl2idxdiff_t)(idx1 - idx2);
}

// ���������
enum
{
	CNL2_CMD_EXTEND				= 0x00,								// ������չ
	CNL2_CMD_CONNECT			= 0x01,								// ��������
																	// CNL2_CMD_CONNECT
	CNL2_CMD_CONNECT_AGREE		= 0x02,								// ���ӳɹ�
																	// CNL2_CMD_CONNECT_AGREE_T
																	// slotidx���Ǹ���������slot
	CNL2_CMD_CONNECT_REFUSE		= 0x03,								// ���ӱ��ܾ�
																	// CNL2_CMD_CONNECT_REFUSE_T
	CNL2_CMD_CLOSE				= 0x04,								// ����ر�
																	// CNL2_CMD_CLOSE_T
	CNL2_CMD_DATA_CONFIRM		= 0x05,								// ����ͨ��������ȷ�ϡ�ֻҪ�յ��˶Է��İ��ͻ���һ��tick�ڷ��������
																	// CNL2_CMD_DATA_CONFIRM_T��ͷ������channelû�á������������channelmaskΪ׼��
																	// Confirm����Ҫ��Noop�������ڷ���ֻ��Ҫ���յ��Է���Data��������»ط���
	CNL2_CMD_DATA				= 0x06,								// data (���ݳ��Ȳ�����Ϊ0)
																	// CNL2_CMD_DATA_T��ͷ
																	// ��Ϊʵʱ��������һ�����ͣ�ֱ�Ӹ��ݳ�ʼ������channel��Ӧ��ͬ�Ĵ�������ʵ��
	CNL2_CMD_NOOP				= 0x07,								// NOOP����
																	// NOOP����û�����ݽ�������������ڻ�ü�ʱ
																	// NOOP��Զֻ����Client����Server����������
																	// NOOP��channelû�����壬Ĭ����0
																	// Client���յ�Server��Agree��ʼ��Noop������
	CNL2_CMD_SWITCHADDR			= 0x08,								// �ͻ��˸�����ַ
	CNL2_CMD_MAX				= 0x10,								// ����ָ��岻�ܳ������
};
#pragma pack(1)
struct	CNL2_CMD_0_T
{
	enum
	{
		DATAPHASE_SINGLE		= 0,								// 0	�������ݻ�����
		DATAPHASE_HEAD			= 1,								// 1	������ͷ
		DATAPHASE_TAIL			= 2,								// 2	������β
	};
	unsigned char				crc;								// crcУ��
	unsigned char				cmd			: 4;					// �����CNL2_CMD_XXX��ָ�Χ0~15
	unsigned char				reserved	: 4;					// ����
	inline void	calcsetcrc(size_t nTotalSize)
	{
		crc	= calccrc(nTotalSize);
	}
	inline bool	checkcrc(size_t nTotalSize)
	{
		return	crc ==calccrc(nTotalSize);
	}
	inline unsigned char	calccrc(size_t nTotalSize)
	{
		return	smp_crc8(((char*)this)+sizeof(crc), nTotalSize-sizeof(crc));
	}
};
struct	CNL2_CMD_EXTEND_T			: public CNL2_CMD_0_T
{
	enum
	{
		CMD_ECHO				= 1,								// ����������ԭ�����ء�һ�����ڲ�������֮��������ӳ١�
		CMD_ECHO_RST			= 2,								// ��ӦECHO�ķ��أ���ECHO�������������ݷ��ؼ��ɣ�
	};
	unsigned char				subcmd;								// ����ָ��
	char						data[1];							// ���ݲ���
};
struct	CNL2_CMD_CONNECT_T			: public CNL2_CMD_0_T
{
	unsigned char				ver;								// ���Ӱ汾��
	short						clientslotidx;						// ��ߵ�slotidx
	// �����������һ���ֽڼ�¼���ȵı䳤����(������:��Կ��������������)
};
struct	CNL2_CMD_1_T				: public CNL2_CMD_0_T
{
	unsigned short				channel		: 3;					// channel��ţ�0~7
	unsigned short				slotidx		: 13;					// slot��ţ�0~8191
};
struct	CNL2_CMD_CONNECT_AGREE_T	: public CNL2_CMD_1_T
{
	short						serverslotidx;						// ��Ӧ�Ǳߵ�slotidx
	unsigned char				cryptortype;						// ����������
	int							nNoopTimeout;						// ��Ӧ�������˵�Noop�ط���ʱ���ͻ���Ӧ�úͷ������ı���һ��
	int							nDropTimeout;						// ���߳�ʱ
	int							nMightDropTimeout;					// ���ܵĶ��ߣ���⵽���������������
	// ���������Կ��������
};
struct	CNL2_CMD_CONNECT_REFUSE_T	: public CNL2_CMD_1_T
{
	unsigned char				reason;								// �ܾ����ӵ�ԭ��
																	// CONNECTREFUSE_REASON_XXX
};
struct	CNL2_CMD_CLOSE_T			: public CNL2_CMD_1_T
{
	int							nExtData;
};
// �ٶ�Confirm������B����A��
struct	CNL2_CMD_DATA_CONFIRM_T		: public CNL2_CMD_1_T
{
	unsigned char				channelmask;						// ��ʾ������˶��ٸ�����ȷ��A��channel��ȷ������
																	// ÿ��bit����ÿ��ͨ���Ƿ���confirm����

	// �������channelmask��bit��Ӧ����ͨ����confirm����
	// ���ĳ��ͨ���Է��Ѿ������Ѿ�ȫ���յ�������Ҫ��Ӧ��confirm����
	struct	CONFIRM_T
	{
		// ע�⣺��Щ���������Ķ��Ƿ��������Ǳߵ���Ϣ
		cnl2idx_t				nInIdxConfirm;						// B��In���е�ȷ��A���������һ�����+1
		cnl2idx_t				nInIdx0;							// B��In���е��ʼԪ�ص���ţ�������A��Out��������Щ���Ա��ͷ�
		// �������һЩvn1�����ݣ���ʾ������û���յ��������յ����ĸ�����0��ʾ����
		// ��������Ĳ����������ܳ���InNum����OutNum
	};
};
struct	CNL2_CMD_DATA_T				: public CNL2_CMD_1_T
{
	enum
	{
		PROP_PACK				= 0x01,								// �Ǵ������
		PROP_COMPRESS			= 0x02,								// ��ѹ������(ֻ�д�����ݲ�����ѹ������)
		PROP_ALLNEG				= 0xFF^(PROP_PACK|PROP_COMPRESS),
																	// ǰ�����е�һ��ȡ����������֤�Ƿ��д���ģ�
	};
	cnl2idx_t					nIdx;								// �������
	unsigned char				dataphase	: 2;					// ���ݺϲ����ͣ�ָֻ��Կɿ�����
																	// �������DATAPHASE_XXX
	unsigned char				prop		: 6;					// ���ԣ��μ������PROP_XXX
};
enum
{
	CNL2_MAX_SINGLEDATASIZE			= CNL2_MAX_DATAFRAMESIZE - sizeof(CNL2_CMD_DATA_T),
																	// ������԰������������ֵ
};
struct	CNL2_CMD_NOOP_T				: CNL2_CMD_1_T
{
	whtick_t					nSendTime;							// ����ʱ��
	whtick_t					nPeerSendTime;						// �Է�����ʱ��(��������û������)
	unsigned char				nCount;								// ��ת��������(��������Ϊ0)
	// ������Ÿ���channel��nOutIdxConfirm�������ͽ��շ���nInIdxConfirmһ�ȽϾͿ���֪���Ƿ���Ҫȷ����
	// ����channelmask����Ϊ���ߵ�channelһ���Ƕ�Ӧ��
	// 2005-07-13 ȡ�������Confirm���ݵ���ơ���Ϊ���ȵ�����NOOP��ʱ��Ӧ���Ѿ�����Ҫȷ���ˡ�
	// ÿ���յ����ݵ�ʱ�򶼻ᵼ��ȷ�ϱ�����
};
struct	CNL2_CMD_SWITCHADDR_T		: CNL2_CMD_1_T
{
	enum
	{
		SUBCMD_ORDER			= 0x00,								// ����peer������ַ
		SUBCMD_REQ				= 0x01,								// ���������ַ
		SUBCMD_RPL				= 0x02,								// ͬ�������ַ
	};
	unsigned char				nSubCmd;
	unsigned short				nSeed;								// ��������Ժ���
	unsigned char				szKeyMD5[WHMD5LEN];					// ���������֤��md5
};
#pragma pack()
typedef	whchunkallocationobj<whcmnallocationobj>	CNL_WCA_T;
struct	CNL2SlotMan_I_UDP;
struct	CMDPACKUNIT_T
{
	int		nSize;
	void	*pCmd;
};
struct	CNL_SLOT_CHANNEL_T
{
	int		nSlot;
	int		nChannel;
};
struct	CNL2SLOT_I
{
	typedef	int	(CNL2SlotMan_I_UDP::*CMDDEAL_T)();
	struct	CHANNEL_T
	{
		struct	INUNIT_T
		{
			void			*pData;									// ������SlotMan��������ͷŷ���������
																	// ֻҪpData�ǿվ�˵���յ���Ӧ��������
																	// ���ݱ��ϲ���պ���Ӧ��InUnit���Ƴ����в�clear
			size_t			nSize;
			unsigned char	dataphase	: 2;						// ���ݺϲ����ͣ�ȡֵ�μ�CNL2_CMD_0_T::DATAPHASE_XXX
			unsigned char	prop		: 6;						// ���ԣ��μ�CNL2_CMD_DATA_T::PROP_XXX
			bool			decrypted;								// ��ʾ�Ƿ��Ѿ��������
			INUNIT_T()
			: pData(NULL)
			{
			}
			~INUNIT_T()
			{
				assert(pData==NULL);
			}
			void	clear(CNL2SlotMan_I_UDP *pMan);
		};
		struct	OUTUNIT_T
		{
			void	*pCmd;											// ��ָ��ͷ������(crcҲ������ˣ��������Ա������ط���ʱ�����¼���crc)
																	// ������malloc����ģ���ΪҪfree�ͷ�
			size_t	nSize;											// ָ��ߴ�
																	// ����Ѿ�ȷ�����ʱ�������ɾ���������nTimeEventID
			whtick_t					nSendTime;					// ���͵�ʱ��
			unsigned short				nRSCount;					// ReSendCount
			whtimequeue::ID_T			nTimeEventID;				// ��Ӧ�ĳ�ʱ�¼����
			OUTUNIT_T()
			: pCmd(NULL)
			{
			}
			~OUTUNIT_T()
			{
				assert(pCmd==NULL);									// ����������ǰ����clear
			}
			void	clear(CNL2SlotMan_I_UDP *pMan);
		};
		CNL2SlotMan_I_UDP				*pSlotMan;					// Slot������ָ��
		CNL2SLOT_I						*pSlot;						// ��Ӧ��slotָ��
		unsigned char					nChannel;					// Channel�����
		unsigned char					nProp;						// Channel�����ԣ��ο�:CNL2CHANNELINFO_T::PROP_XXX
		int								nType;						// CNL2_CHANNELTYPE_XXX
		// �����������еĳ��ȶ����ܳ���65536
		// ����lastsafe������˵��ֻ��Idx0���á�IdxConfirmû���ô���
		cnl2idx_t						nInIdx0;					// ����pInQueue�е�һ��Ԫ�ص��±�
		cnl2idx_t						nOutIdx0;					// ����pOutQueue�е�һ��Ԫ�ص��±�
		cnl2idx_t						nInIdxConfirm;				// ��һ��ϣ��ȷ�ϵ�In�����(Ҳ����Ŀǰ�����յ������һ�����+1)
		cnl2idx_t						nOutIdxConfirm;				// �������Ѿ�����ȷ�ϵ������һ��+1(Ҳ�����ϲ������ȡ��)
		whvectorqueue<INUNIT_T>			*pInQueue;					// ���еĶ���Ҫ���
		whvectorqueue<OUTUNIT_T>		*pOutQueue;					// ֻ��safe��Ҫ�����lastsafe��Ҫ����ĳ���Ϊ1
		whDList<CHANNEL_T *>::node		nodeHavingData;				// ˵�����channel���յ�������
		whDList<CNL_SLOT_CHANNEL_T>::node	nodeHavingDataToSend;	// ˵�����channel��Ҫ��������
		ICryptFactory::ICryptor			*pICryptorSend;				// ������(����)
		ICryptFactory::ICryptor			*pICryptorRecv;				// ������(����)
		whDList<CMDPACKUNIT_T>			dlistCmdPack;				// ��Ҫ�����ָ���б�ÿ��tick����ʱ�϶���յģ�

		CHANNEL_T();
		~CHANNEL_T();
		int	Init(CNL2CHANNELINFO_T *pInfo);
		int	Release();
		// ����InQueue��ȷ�����ݡ��������ݳ��ȡ�
		size_t	MakeInQueueConfirm_Safe(void *pData);
		size_t	MakeInQueueConfirm_LastSafe(void *pData);
		inline cnl2idxdiff_t	idxdiff(cnl2idx_t idx1, cnl2idx_t idx2)
		{
			return	cnl2_idxdiff(idx1, idx2);
		}
		void	AddCmdPackUnit(whDList<CMDPACKUNIT_T>::node *pNode);
	};
	CNL2SLOTINFO_T				slotinfo;							// ���ڸ����濴
	CHANNEL_T					channel[CNL2_CHANNELNUM_MAX];
	whDList<CHANNEL_T *>		dlistChannelWithData;				// �����ݵ�channel�ͷ����������
	// ����teid����Ŷ���
	enum
	{
		TE_IDX_CONNECT			= 0,
		TE_IDX_ACCEPT			= 0,
		TE_IDX_DROP				= 0,								// ����ǳ�����
		TE_IDX_CLOSE			= 0,
		TE_IDX_CONNECT_RS		= 1,
		TE_IDX_ACCEPT_RS		= 1,
		TE_IDX_NOOP				= 1,								// ����ǳ�����
		TE_IDX_CLOSE_RS			= 1,
		TE_IDX_MIGHTDROP		= 2,								// �����ܶ��ߡ��ĳ�ʱ
		TE_IDX_SWITCHADDRREQ	= 3,								// ���͵�ַת���ļ��
		TE_IDX_MAX				= 4,
	};
	whtimequeue::ID_T			teid[TE_IDX_MAX];					// �����������ĳ�ʱ
	// SlotMan����Щ����Recv����ָ��
	CMDDEAL_T					dealfuncRecv[CNL2_CMD_MAX];
	enum
	{
		DLISTIDX_NOOP			= 0,								// ��tick����ǰ��Ҫ����Noop���б�
		DLISTIDX_COFIRM			= 1,								// ��tick����ǰ��Ҫ����Confirm���б�
		DLISTIDX_HAVINGDATA		= 2,								// ��tick����ȡ�����ݵ�slot�б�(�ϲ�����������ȡ����)
		DLISTIDX_MAX
	};
	whDList<CNL2SLOT_I *>::node	dlistNode[DLISTIDX_MAX];			// ֮��������ô��node����Ϊһ��Slot����ͬʱ������Щ������
	// Close��Ҫ��
	bool						bClosePassive;						// �����ر�
	bool						bIsClient;							// ����������������ӵ�һ��
	unsigned char				nCloseCount;						// �رռ���(�����������1��ʱ�򣬾Ϳ��Թر���)
	unsigned char				nCryptorType;						// ���������� 2006-01-23
	// ����Ҫconfirm��channel������
	// �����ÿ���յ��Է�����������ʱ���и���
	// Ҳ��ÿ���յ��Է�NOOP����ʱ����и���
	unsigned char				nPeerNotConfirmedChannelMask;
	unsigned char				nNoopCount;
	bool						bNoopMustBeSend;					// ǿ��һ��Ҫ����Noop
																	// �����ڴ���ʱ���¼���ʱ��Ͳ��ñȽ�lastsend��
	whtick_t					nPeerNoopSendTime;
	whvector<char>				vectExtSlotData;					// ������ʱ���ӵ�����(ͨ��Connectָ���)
	int							nCloseExtDataSend;					// �ڹر�����ʱ���ӵ�(���ͳ�ȥ)
	int							nCloseExtDataRecv;					// �ڹر�����ʱ���ӵ�(���յ��Է���)
	ICryptFactory::IKeyExchanger	*pIKeyExchanger;				// ��Կ������
	int							nExt;								// ���ⲿ����������
	unsigned char				szKeyMD5[WHMD5LEN];					// ��ʼ����key��md5��
	CNL2SlotMan_I_UDP			*pSlotMan;

	CNL2SLOT_I();
	void	clear();												// �ڳ����ر�slot��ʱ��ʹ��
	void	ClearAllTE();
	void	ClearTE(int nIdx);
	void	ClearChannelAllTE();
	void	SetSlotMan(CNL2SlotMan_I_UDP *pMan);
};
struct	CNL2SlotMan_I_UDP				: public CNL2SlotMan
{
	// ΪCNL2SlotManʵ�ֵ�
	CNL2SlotMan_I_UDP();
	~CNL2SlotMan_I_UDP();
	virtual	void	SelfDestroy();
	virtual	int		GetMaxSingleDataSize();
	virtual	int		Init(CNL2SlotMan::INFO_T *pInfo, struct sockaddr_in *pAddr);
	virtual	int		Init_Reload(INFO_T *pInfo);
	virtual	int		ReConfigChannel(INFO_T *pInfo);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	SOCKET	GetSocket() const	{return m_sock;}
	virtual	int		DoSelect(int nMS);
	virtual	int		TickRecv();
	virtual	int		TickLogicAndSend();
	virtual	int		Listen(bool bListen);
	virtual	int	Connect(struct sockaddr_in *addr, void *pExtData, int nExtSize);
	virtual	int	Connect(const char *cszAddr, void *pExtData, int nExtSize);
	virtual	int		Close(int nSlot, int nExtData);
	virtual	int		CloseAll();
	virtual	int		Send(int nSlot, const void *pData, size_t nSize, int nChannel);
	virtual	int		Recv(int *pnSlot, int *pnChannel, void **ppData, size_t *pnSize);
	virtual	int		ControlOut(CONTROL_T **ppCmd, size_t *pnSize);
	virtual	CNL2SLOTINFO_T *	GetSlotInfo(int nSlot);
	virtual	bool	IsSlotExist(int nSlot) const;
	virtual cmn_addr_allowdeny &	GetConnectAllowDeny();
	virtual	whvector<char> *	GetConnectExtData(int nSlot);
	virtual	int		SetExtLong(int nSlot, long nExt);
	virtual	int		GetExtLong(int nSlot, long *pnExt);
	virtual	void	SetICryptFactory(n_whcmn::ICryptFactory *pICryptFactory);
	virtual	int		GetSlotNum() const;
	virtual	void	SetUseAuth();
// �Լ��õ�
// ����
	struct	TQUNIT_T;
	typedef	void (CNL2SlotMan_I_UDP::*TEDEAL_T)(TQUNIT_T *);
	#pragma pack(1)
	struct	TQUNIT_T
	{
		// !!!!ע�⣬�����¼ָ�룬����뱣֤��ɾ��slot��ʱ��Ҫ���������ص�ʱ���¼�!!!!
		CNL2SLOT_I	*pSlot;									// ��ص�slot(�����¼�Ӧ�ö��Ǻ�һ��slot��ص�)
		TEDEAL_T	tefunc;									// ������¼��ĺ���
		union
		{
			struct	RESEND_T
			{
				CNL2SLOT_I::CHANNEL_T::OUTUNIT_T	*pOutUnit;
			}resend;
		}un;
		TQUNIT_T()
		: pSlot(NULL), tefunc(NULL)
		{
		}
	};
	#pragma pack()
// ����
private:
	typedef	int (CNL2SlotMan_I_UDP::*CMDDEAL_T)();
	typedef	int (CNL2SlotMan_I_UDP::*SENDFUNC_T)(const void *pData, size_t nSize);
	typedef	void (CNL2SlotMan_I_UDP::*SLOTCHANNELCONFIRM_T)(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	typedef	void (CNL2SlotMan_I_UDP::*DEALSLOT_T)(CNL2SLOT_I *pSlot);
	typedef	n_whcmn::whhash<struct sockaddr_in, int, whcmnallocationobj, _whnet_addr_hashfunc>	ADDR2SLOT_T;
	whDList<CNL_SLOT_CHANNEL_T>						m_dlistChannelHaveDataToSend;
															// ��������Ҫ���ͳ�ȥ��channel
	typedef	unsigned short	CONNECT_PUBKEY_LEN_T;
	typedef	unsigned char	CONNECT_EXT_LEN_T;
	INFO_T	m_info;
	int		m_nConnectRSTimeout;							// ͨ��info.nConnectTimeout�������
	int		m_nNoopTimeout;									// ͨ��info.nDropTimeout���������client��������֮���ӷ���������������
	int		m_nMightDropTimeout;							// m_nNoopTimeout*2��������������ʱ�䶼û���յ��Է���������ܶԷ��Ƕ�����
	SOCKET	m_sock;
	bool	m_bListen;
	whunitallocatorFixed<CNL2SLOT_I>	m_Slots;
	ADDR2SLOT_T		m_mapaddr2slot;							// ��ַ��slotid��ӳ�䣨�����Ҫ��Ϊ�˴�������ʱ�����ظ�����slot��
															// unsigned int�и���λ��channel������λ��slotid�ĵ���λ����index���֣�
	cmn_addr_allowdeny					m_ConnectAllowDeny;	// ����������Դ��ַ������
	// ��������ڴ���ո�recvfrom�������ݣ�Ҳ�����������ϳ�Ҫ����ȥ������
	// (��ôʹ��һ��ҪС�ģ�����������������ҵ����)
	whvector<char>	m_vectrawbuf;							// �ܶ෢��ָ�ʹ�����
	whvector<char>	m_vectrawbuf_recv;						// ֱ��ͨ��recvfrom���õ�buf
	whvector<char>	m_vectpackbuf;							// ���ڴ��ָ��Ļ���
	whvector<char>	m_vectAuth;								// �����洢��Ȩ�ļ�����
	whvector<char>	m_vectRSA;								// RSA��Կ

	struct	PACK_INFO_T
	{
		int				nSlot;								// �ϴλ�û����ɽ����Slot�ţ��������0�ű�ʾ��û����ɽ�������ݣ�
		int				nChannel;							// �ϴλ�û����ɽ����Channel��
		whcmdshrink		wcs;								// ����ָ����
		PACK_INFO_T()
		: nSlot(0), nChannel(0)
		{
		}
	}				m_pack_info;
	CNL2_CMD_0_T	*m_pCmd;								// ָ��m_vectrawbuf.getbuf()
	CNL2_CMD_1_T	*m_pCmd1;								// ָ��m_vectrawbuf.getbuf()
	CNL2_CMD_DATA_T	*m_pCmdData;							// ָ��m_vectrawbuf.getbuf()
	int				m_nCmdSize;								// �ո�recvfrom�����cmd�ĳ���
															// 2005-05-27 ԭ�������size_t��������recv��ʱ����ܵõ���ֵ�������ºܴ������
	void			*m_pData;								// ��������ݰ��������ָ�����ݲ���
	size_t			m_nDataSize;							// �ո�recvfrom�����cmd�ĳ���
	struct sockaddr_in	m_addr;								// �ո�ָ���Ӧ�ĵ�ַ
	CNL2SLOT_I		*m_pSlot;								// ��ǰ���ڴ����Slot��ʹ�����ֻ��Ϊ�˱��⴫��pSlot����������������������Ҫ����С�ģ��ںܶຯ���в�Ҫ�뵱Ȼ����Ϊm_pSlot�����Լ���Ҫ���Ǹ�slot��
	CNL2SLOT_I::CHANNEL_T	*m_pChannel;					// ��ǰ���ڴ����Slot��Channel
	// ʱ�����
	whtimequeue		m_tq;									// ���ֳ�ʱ�����ӡ��������ط���
	bool			m_bUseAuth;
	// ��ǰ��ʱ��
	whtick_t		m_tickNow;								// ����Ҫ���ʺϻ�ȡһ��
	// ֪ͨ�ϲ��ָ�����
	whsmpqueue		m_queueControlOut;
	// ��ʱ���m_queueControlOut���ϲ��ָ��
	whvector<char>	m_vectControlOutCmd;
	// ��ͨ���Ĵ�����
	CMDDEAL_T				m_TickRecv_CMD_Others_DATA_Deal[CNL2_CHANNELNUM_MAX];
	SENDFUNC_T				m_Send_DATA[CNL2_CHANNELNUM_MAX];
	SLOTCHANNELCONFIRM_T	m_SCC[CNL2_CHANNELNUM_MAX];
	// �μ�DLISTIDX_XXX�Ķ���
	whDList<CNL2SLOT_I *>	m_dlistSlot[CNL2SLOT_I::DLISTIDX_MAX];
	// ���ܹ���
	ICryptFactory			*m_pICryptFactory;
	// �Լ��󶨵Ķ˿ں͵�ַ����������Ŀǰ������ֻ������ʱ�����Ѽ���������СС�ı任��
	port_t					m_nMyAddrSum;
	whtick_t				m_nLastSwitchAddrTime;			// �ϴ�ת����ַ��ʱ�䣨����ת����ʱ�䲻�ܶ���MightDropTimeout��

	// С���ڴ������
//	CNL_WCA_T				m_wca;
	// ��m_wca�õ�ԭʼ�ڴ������
	whcmnallocationobj		m_a;
	// ���AllowDeny�ļ��Ƿ񱻸ı�
	whfilechangedetector	m_fcdAllowDeny;
	// ����ͳ��
	whtimestat<whuint64>	m_statDownByte, m_statUpByte;
	whtimestat<int>			m_statDownCount, m_statUpCount;

// ����
private:
	int		Init_SetParams();
	// ������չָ��ĺ���
	void	TickRecv_CMD_EXTEND();
	// ���崦��CONNECTָ��ĺ���
	void	TickRecv_CMD_CONNECT();
	// ������ַָ��
	void	TickRecv_CMD_SWITCHADDR();

	// ��������ָ����ܺ���
	void	TickRecv_CMD_Others();
	// ��TickRecv_CMD_Others����
	// �������0��ʾ����ɹ���<0��ʾ����ʧ��
	inline int	TickRecv_CMD_Others_Call(int nCmd)
	{
		// ���÷�ʽ��֤nCmd���ᳬ���(��Ϊ4bit��������)
		return	(this->*m_pSlot->dealfuncRecv[nCmd])();
	}

	// TickRecv_CMD_Others�����õĴ������ָ��ĺ���
	int		TickRecv_CMD_Others_Unknown();
	int		TickRecv_CMD_Others_CONNECT_AGREE();
	int		TickRecv_CMD_Others_CONNECT_REFUSE();
	int		TickRecv_CMD_Others_CLOSE();
	int		TickRecv_CMD_Others_DATA_CONFIRM();

	// TickRecv_CMD_Others_DATA��slot�������ڴ���DATA�ĺ���
	int		TickRecv_CMD_Others_DATA();								// ������������
	int		TickRecv_CMD_Others_DATA_ON_ACCEPTING();				// Accept�����յ���һ�����ݰ�֮ǰ�����
	// �������ЩTickRecv_CMD_Others_DATA_Deal_XXX���Ƕ�Ӧ����ͨ���Ĵ���������TickRecv_CMD_Others_DATA�ڱ�����
	int		TickRecv_CMD_Others_DATA_Deal_NotSupport();				// ��֧�ֵ�ͨ��
	int		TickRecv_CMD_Others_DATA_Deal_RT();						// ʵʱ����
	int		TickRecv_CMD_Others_DATA_Deal_Safe();					// �ɿ�����
	int		TickRecv_CMD_Others_DATA_Deal_LastSafe();				// ����ɿ�����

	// NOOP������
	int		TickRecv_CMD_Others_NOOP_ON_ACCEPTING();				// Accept�����յ���һ�����ݰ�֮ǰ�����
	int		TickRecv_CMD_Others_NOOP();

	int		_TickRecv_FIRST_CMD_ON_ACCEPTING();

	int		TickLogic_PackAndCompression();							// �Ա�֡�����ݽ��м�������ѹ��

	int		TickLogic_TE();
	int		_TickLogic_DealSlotInDList(int nIdx, DEALSLOT_T func);
	void	_DealSlotInDList_Confirm(CNL2SLOT_I *pSlot);
	void	_DealSlotInDList_Noop(CNL2SLOT_I *pSlot);
	//
	void	TEDeal_Connect(TQUNIT_T *pTE);
	void	TEDeal_Connect_RS(TQUNIT_T *pTE);
	void	TEDeal_Accept(TQUNIT_T *pTE);
	void	TEDeal_Accept_RS(TQUNIT_T *pTE);
	void	TEDeal_Data_RS(TQUNIT_T *pTE);
	void	TEDeal_Noop(TQUNIT_T *pTE);
	void	TEDeal_Drop(TQUNIT_T *pTE);
	void	TEDeal_MightDrop(TQUNIT_T *pTE);
	void	TEDeal_SwitchAddrReq(TQUNIT_T *pTE);
	void	TEDeal_Close(TQUNIT_T *pTE);
	void	TEDeal_Close_RS(TQUNIT_T *pTE);
	void	TEDeal_StatLOG(TQUNIT_T *pTE);
	void	TEDeal_AllowDenyFileChangeCheck(TQUNIT_T *pTE);
	void	Set_TEDeal_AllowDenyFileChangeCheck();
	void	TEDeal_StatInterval(TQUNIT_T *pTE);
	void	Set_TEDeal_StatInterval();
	// ����Ȩ�ļ����뵽�ڴ�
	void	TEDeal_CA0(TQUNIT_T *pTQUnit);
	// ����auth����
	void	TEDeal_CA1(TQUNIT_T *pTQUnit);
	// ��ӡlog
	void	TEDeal_CA2(TQUNIT_T *pTQUnit);
	// �˳�����
	void	TEDeal_CA3(TQUNIT_T *pTQUnit);
	// ���漸��������
	void	SetTE_CA(TEDEAL_T pFunc, int nMS);
	// ��m_vectrawbuf�д���Ҫ����������
	void	_MakeDataToSend(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T *pChannel, const void *pData, size_t nSize, unsigned char nDataPhase, unsigned char nProp, cnl2idx_t nIdx);
	int		Send_DATA_NotSupport(const void *pData, size_t nSize);
	int		Send_DATA_RT(const void *pData, size_t nSize);
	int		Send_DATA_Safe(const void *pData, size_t nSize);
	int		Send_DATA_Safe_Pack(const void *pData, size_t nSize);	// ������ķ���
	int		_Send_DATA_Safe(const void *pData, size_t nSize, unsigned char nProp);
	int		_Send_DATA_Safe_Single(const void *pData, size_t nSize, unsigned char nDataPhase, unsigned char nProp);
																	// ����һ����һ���ݿ鲢����ʱ����б��Ϊ��tick��������
																	// safe��lastsafeģʽ�����õ��������
	int		Send_DATA_LastSafe(const void *pData, size_t nSize);
	//
	int		Close(CNL2SLOT_I *pSlot);
	enum
	{
		CLOSEONERR_UNKNOWN									= 0,
		CLOSEONERR_SlotChannelConfirm_NotSupport			= 1,
		CLOSEONERR_SlotChannelConfirm_RT					= 2,
		CLOSEONERR_TickRecv_CMD_Others_Unknown				= 3,
		CLOSEONERR_BAD_DATA_CONFIRM							= 4,
		CLOSEONERR_BAD_DATA_CONFIRM_TOOMANYSECT				= 5,	// �յ�̫���ȷ�϶�
		CLOSEONERR_BAD_DATA_RECV_TOOMANYSECT				= 6,	// �յ������ݷֶ�̫��
		CLOSEONERR_BAD_DATA_SIZE							= 8,
		CLOSEONERR_BAD_DATA_NOOP							= 9,
		CLOSEONERR_BAD_DATAPHASE							= 10,
		CLOSEONERR_TickRecv_CMD_Others_DATA_Deal_NotSupport	= 11,
		CLOSEONERR_IdxOutOfRange							= 12,
		CLOSEONERR_BAD_DATA_CONFIRM_SIZE_SMALL				= 21,	// �յ���ȷ�����ݵĳߴ��С
		CLOSEONERR_BAD_DATA_CONFIRM_BADCHANNELTYPE			= 22,	// channel���Ͳ���
		CLOSEONERR_BAD_DATA_CONFIRM_HDR_SIZE_SMALL			= 23,	// ȷ��ͷ�Ĳ��ֳߴ��С
		CLOSEONERR_BAD_DATA_CONFIRM_IDX_SMALL				= 24,	// ȷ�ϵ���Ź�С
		CLOSEONERR_BAD_DATA_CONFIRM_COUNT_BIG				= 25,	// ȷ�ϵĲ��ֹ�����
		CLOSEONERR_BAD_DATA_CONTENT							= 26,	// �������ݻ���
	};
	int		CloseOnErr(CNL2SLOT_I *pSlot, int nErrCode=CLOSEONERR_UNKNOWN, int nExt=0);
	void	SetCloseStatus(CNL2SLOT_I *pSlot, bool bClosePassive);
	void	FillSlotWithDefaultTickRecvDealFunc(CNL2SLOT_I *pSlot);
	enum
	{
		ADDSLOT_RST_OK							= 0,
		ADDSLOT_RST_HASADDR						= -1,
		ADDSLOT_RST_SLOTFULL					= -2,
		ADDSLOT_RST_MEMERR						= -3,
		ADDSLOT_RST_KEYEXCHANGER_INITERR		= -4,
	};
	// ���������slot�����洴���ĵ�ǰslot��m_pSlot
	int		AddSlot(const struct sockaddr_in &addr, int nStatus, short nPeerSlotIdx, const void *pExtData, int nExtSize, int *pnSlot, const void *pPubData, int nPubSize, int nKeyExchangerType);
	int		RemoveSlot(int nSlot);
	int		RemoveSlot(CNL2SLOT_I *pSlot);
	void	ClearSlotAllTE(CNL2SLOT_I *pSlot);
	void	ClearSlotChannelAllTE(CNL2SLOT_I *pSlot);
	inline void	ClearSlotAllTEAndChannelAllTE(CNL2SLOT_I *pSlot)
	{
		pSlot->ClearAllTE();
		pSlot->ClearChannelAllTE();
	}

	void	SendConnectAndRegTQ(CNL2SLOT_I *pSlot);
	void	SendConnectAgreeAndRegTQ(CNL2SLOT_I *pSlot);
	void	SendClose(CNL2SLOT_I *pSlot);
	void	SendNoopThisTick(CNL2SLOT_I *pSlot);					// �����ڱ�tick������ʱ�����û�з����κ�������Ӧ�÷���NOOP
	void	SendConfirmThisTick(CNL2SLOT_I *pSlot);					// �����ڱ�tick������ʱ��һ��Ҫ��Է�����dataconfirm��(����Է�����û�б�confirm��channel�Ļ�)
	void	RegNoopTQ(CNL2SLOT_I *pSlot, whtick_t nTime);
	void	RegDropTQ(CNL2SLOT_I *pSlot);
	void	RegMightDropTQ(CNL2SLOT_I *pSlot, whtick_t nTime);
	void	RegSwitchAddrReq(CNL2SLOT_I *pSlot, whtick_t nTime);
	void	RegCloseTQ(CNL2SLOT_I *pSlot);
	void	RegCloseRSTQ(CNL2SLOT_I *pSlot);

	// ������channel��Ҫ�ط���ָ�������tick��ȫ���ط�
	void	SendAllUnConfirmedDataThisTick(CNL2SLOT_I *pSlot);
	// ��ĳ��outunit������������ӳ�nDelay���ͣ����ԭ��Ԥ������ʱ������tickNow+nDelay�Ͱ�Ԥ��ʱ�䷢�ͣ�
	int		SendOutUnitThisTick(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit);
	int		SendOutUnitWithDelay(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit, int nDelay);
	// ��Է����͵�ַת������
	void	SendSwitchAddrReq(CNL2SLOT_I *pSlot);

	// ���nNumΪ0���ʾnIdx֮ǰ�Ķ���ȷ��
	// ���nNum>0���ʾnIdx֮���nNum����ȷ��
	void	SlotChannelConfirm(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	void	SlotChannelConfirm_NotSupport(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	void	SlotChannelConfirm_RT(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	void	SlotChannelConfirm_Safe(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	bool	_SlotChannelConfirm_Safe_SetConfirm(CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit);
	void	SlotChannelConfirm_LastSafe(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);

	// ͨ��������е���λ���ƶ���nIdx0
	void	SlotOutChannelMove(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx0);
	// ͨ������������ͷ��nNum������
	void	SlotInChannelMove(CNL2SLOT_I::CHANNEL_T*pChannel, int nNum);

	// ��ʼ������channel�ļ�����
	void	InitChannelCryptor(CNL2SLOT_I *pSlot);

	void *	ControlOutAlloc(size_t nSize);
	template<typename _Ty>
	inline void	ControlOutAlloc(_Ty *&ptr)
	{
		ptr	= (_Ty *)ControlOutAlloc(sizeof(_Ty));
	}
	void	ControlOutReasonAndRemoveSlot(CNL2SLOT_I *pSlot, int nCmd, int nReason);
	inline void	ControlOutCloseAndRemoveSlot(CNL2SLOT_I *pSlot, int nReason)
	{
		ControlOutReasonAndRemoveSlot(pSlot, CONTROL_OUT_SLOT_CLOSED, nReason);
	}
	inline void	ControlOutConnectRefuseAndRemoveSlot(CNL2SLOT_I *pSlot, int nReason)
	{
		ControlOutReasonAndRemoveSlot(pSlot, CONTROL_OUT_SLOT_CONNECTREFUSED, nReason);
	}

	// inline ����
	// ������crc(�����ط���Ӧ���Ѿ�������һ����)
	inline int		slot_send(CNL2SLOT_I *pSlot, const void *__cmd, int __len)
	{
		pSlot->slotinfo.nLastSend	= m_tickNow;
		return	udp_sendto(__cmd, __len, &pSlot->slotinfo.peerAddr);
	}
	inline int		udp_sendto(const void *__buf, int __len, const struct sockaddr_in *__addr)
	{
		int	rst		= n_whnet::udp_sendto(m_sock, __buf, __len, __addr);
		if( rst>0 )
		{
			// ����ͳ��
			m_statUpByte.addval(rst);
			m_statUpCount.addval(1);
		}
		return		rst;
	}
	inline int		udp_recv(void *__buf, int __len, struct sockaddr_in *__addr)
	{
		int	rst		= n_whnet::udp_recv(m_sock, __buf, __len, __addr);
		if( rst>0 )
		{
			// ����ͳ��
			m_statDownByte.addval(rst);
			m_statDownCount.addval(1);
		}
		return		rst;
	}
	inline void		maketicknow()
	{
		m_tickNow	= wh_gettickcount();
	}
	inline cnl2idxdiff_t	idxdiff(cnl2idx_t idx1, cnl2idx_t idx2)
	{
		return	cnl2_idxdiff(idx1, idx2);
	}
	inline bool		IsSlotInDList(CNL2SLOT_I *pSlot, int nIdx)
	{
		return	pSlot->dlistNode[nIdx].isinlist();
	}
	inline void		AddSlotToDList(CNL2SLOT_I *pSlot, int nIdx)
	{
		m_dlistSlot[nIdx].AddToTail(&pSlot->dlistNode[nIdx]);
	}
	inline void		AddSlotToDListIfNeeded(CNL2SLOT_I *pSlot, int nIdx)
	{
		if( IsSlotInDList(pSlot, nIdx) )
		{
			// �Ѿ����ھͲ�����
			return;
		}
		AddSlotToDList(pSlot, nIdx);
	}
	inline void		ChannelHaveDataSoAddToDList(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T *pChannel)
	{
		pChannel->nodeHavingData.leave();
		pSlot->dlistChannelWithData.AddToTail(&pChannel->nodeHavingData);
		AddSlotToDListIfNeeded(pSlot, CNL2SLOT_I::DLISTIDX_HAVINGDATA);
	}
	// ����Seed����ĳ��slot�Ķ�̬KeyMD5ֵ������ᶯ̬����pSlot->slotinfo.nSpecialSeed��
	void	MakeSlotSeedKeyMD5(CNL2SLOT_I *pSlot, unsigned char *szKeyMD5);
	// ��֤ĳ��slot�Ķ�̬KeyMD5ֵ�Ƿ���ȷ��������ж�nSeed�������pSlot->slotinfo.nSpecialSeed�������жϳɹ���Ͱ���nSeed��������
	bool	VerifySlotSeedKeyMD5(CNL2SLOT_I *pSlot, unsigned int nSeed, unsigned char *szKeyMD5);
	// �ı��Լ���socket��ַ
	// ���أ�0��ʾ��ʱ�����Ѿ��ı���ˣ����ü����ı�
	//       1��ʾ�ı���
	//       -1��ʾ����
	int		SwitchAddr(CNL2SLOT_I *pSlot);

	// С�ڴ������ͷ�
	// ���䣨���ط��䵽��ָ�룩
	inline void *	AllocData(int nSize)
	{
		//return	m_wca.Alloc(nSize);
		return	malloc(nSize);
	}
	template<typename _Ty>
	_Ty *	AllocDataByType(_Ty *&ptr)
	{
		ptr	= (_Ty *)AllocData(sizeof(_Ty));
		return	ptr;
	}
	template<typename _Ty>
	_Ty *	NewDataByType(_Ty *&ptr)
	{
		return	new (AllocDataByType(ptr)) _Ty;
	}
	friend struct CNL2SLOT_I::CHANNEL_T;
	friend struct CNL2SLOT_I::CHANNEL_T::INUNIT_T;
	friend struct CNL2SLOT_I::CHANNEL_T::OUTUNIT_T;
	inline void	FreeData(void *ptr)
	{
		//m_wca.Free(ptr);
		free(ptr);
	}
	inline void	FreeAndClearData(void *&ptr)
	{
		if( ptr )
		{
			FreeData(ptr);
			ptr	= NULL;
		}
	}
	template<typename _Ty>
	inline void 	AllocAndSetData(_Ty *&ptr, int nSize, const void *ptrSrc)
	{
		memcpy((ptr=(_Ty *)AllocData(nSize)), ptrSrc, nSize);
	}
	inline void 	SetChannelDataUnit(CNL2SLOT_I::CHANNEL_T::INUNIT_T *pInUnit, int nSize, const void *ptrSrc)
	{
		AllocAndSetData(pInUnit->pData, nSize, ptrSrc);
		pInUnit->nSize	= nSize;
	}
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETCNL2_I_H__
