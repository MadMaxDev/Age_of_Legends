// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2_i.h
// Creator      : Wei Hua (魏华)
// Comment      : CNL2的内部实现
// CreationDate : 2005-04-17
// ChangeLog    : 2005-07-13 client在连接后和服务器的nooptimeout保持一致
//                2005-07-13 取消CNL2_CMD_NOOP_T后面跟Confirm数据的设计。因为当等到发送NOOP的时候应该已经不需要确认了。
//                每次收到数据的tick内都会导致本tick内收到的数据确认被发送。
//                2005-08-15 给Connect增加了附加参数（主要是给PNGS中Client和CLS连接用的）
//                2005-08-21 增加了每个channel都会有可SEND和可RECV的属性，如果没有这样的属性则不能发送或者接受数据
//                2006-05-14 CNL2_CMD_DATA_T中增加了一字节，主要增加了pack位（从此增加了打包数据），dataphase从CNL2_CMD_0_T中移出到这里。
//                2007-12-03 把本tick立即发送的数据不放到时间队列中了

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

// 基本命令定义
enum
{
	CNL2_CMD_EXTEND				= 0x00,								// 用于扩展
	CNL2_CMD_CONNECT			= 0x01,								// 请求连接
																	// CNL2_CMD_CONNECT
	CNL2_CMD_CONNECT_AGREE		= 0x02,								// 连接成功
																	// CNL2_CMD_CONNECT_AGREE_T
																	// slotidx就是给请求分配的slot
	CNL2_CMD_CONNECT_REFUSE		= 0x03,								// 连接被拒绝
																	// CNL2_CMD_CONNECT_REFUSE_T
	CNL2_CMD_CLOSE				= 0x04,								// 请求关闭
																	// CNL2_CMD_CLOSE_T
	CNL2_CMD_DATA_CONFIRM		= 0x05,								// 各个通道的数据确认。只要收到了对方的包就会在一个tick内发送这个。
																	// CNL2_CMD_DATA_CONFIRM_T开头，不过channel没用。后面的数据以channelmask为准。
																	// Confirm不需要象Noop那样定期发，只需要在收到对方的Data包的情况下回发。
	CNL2_CMD_DATA				= 0x06,								// data (数据长度不可以为0)
																	// CNL2_CMD_DATA_T开头
																	// 不为实时数据增加一个类型，直接根据初始化各个channel对应不同的处理函数来实现
	CNL2_CMD_NOOP				= 0x07,								// NOOP请求
																	// NOOP是在没有数据交互的情况下用于获得及时
																	// NOOP永远只能由Client发向Server，来回三次
																	// NOOP的channel没有意义，默认是0
																	// Client在收到Server的Agree后开始打开Noop计数器
	CNL2_CMD_SWITCHADDR			= 0x08,								// 客户端更换地址
	CNL2_CMD_MAX				= 0x10,								// 所有指令定义不能超过这个
};
#pragma pack(1)
struct	CNL2_CMD_0_T
{
	enum
	{
		DATAPHASE_SINGLE		= 0,								// 0	单独数据或大据中
		DATAPHASE_HEAD			= 1,								// 1	大数据头
		DATAPHASE_TAIL			= 2,								// 2	大数据尾
	};
	unsigned char				crc;								// crc校验
	unsigned char				cmd			: 4;					// 上面的CNL2_CMD_XXX，指令范围0~15
	unsigned char				reserved	: 4;					// 保留
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
		CMD_ECHO				= 1,								// 将所有数据原样返回。一般用于测量两机之间的网络延迟。
		CMD_ECHO_RST			= 2,								// 对应ECHO的返回（把ECHO所带的所有数据返回即可）
	};
	unsigned char				subcmd;								// 附加指令
	char						data[1];							// 数据部分
};
struct	CNL2_CMD_CONNECT_T			: public CNL2_CMD_0_T
{
	unsigned char				ver;								// 连接版本号
	short						clientslotidx;						// 这边的slotidx
	// 后面跟着若干一个字节记录长度的变长数据(依次是:密钥交换、附加数据)
};
struct	CNL2_CMD_1_T				: public CNL2_CMD_0_T
{
	unsigned short				channel		: 3;					// channel序号：0~7
	unsigned short				slotidx		: 13;					// slot序号：0~8191
};
struct	CNL2_CMD_CONNECT_AGREE_T	: public CNL2_CMD_1_T
{
	short						serverslotidx;						// 对应那边的slotidx
	unsigned char				cryptortype;						// 加密器类型
	int							nNoopTimeout;						// 对应服务器端的Noop重发超时，客户端应该和服务器的保持一致
	int							nDropTimeout;						// 断线超时
	int							nMightDropTimeout;					// 可能的断线（侦测到这个还可以重连）
	// 后面跟着密钥交换数据
};
struct	CNL2_CMD_CONNECT_REFUSE_T	: public CNL2_CMD_1_T
{
	unsigned char				reason;								// 拒绝连接的原因
																	// CONNECTREFUSE_REASON_XXX
};
struct	CNL2_CMD_CLOSE_T			: public CNL2_CMD_1_T
{
	int							nExtData;
};
// 假定Confirm包是由B发向A的
struct	CNL2_CMD_DATA_CONFIRM_T		: public CNL2_CMD_1_T
{
	unsigned char				channelmask;						// 表示后面跟了多少个用于确认A的channel的确认数据
																	// 每个bit代表每个通道是否有confirm数据

	// 后面跟着channelmask各bit对应各个通道的confirm数据
	// 如果某个通道对方已经数据已经全部收到，则不需要对应的confirm数据
	struct	CONFIRM_T
	{
		// 注意：这些数据描述的都是发来数据那边的信息
		cnl2idx_t				nInIdxConfirm;						// B的In队列的确认A的连续最后一个序号+1
		cnl2idx_t				nInIdx0;							// B的In队列的最开始元素的序号，代表了A的Out队列中那些可以被释放
		// 后面就是一些vn1的数据，表示连续“没有收到”、“收到”的个数，0表示结束
		// 这个连续的部分数量不能超过InNum或者OutNum
	};
};
struct	CNL2_CMD_DATA_T				: public CNL2_CMD_1_T
{
	enum
	{
		PROP_PACK				= 0x01,								// 是打包数据
		PROP_COMPRESS			= 0x02,								// 是压缩数据(只有打包数据才能是压缩数据)
		PROP_ALLNEG				= 0xFF^(PROP_PACK|PROP_COMPRESS),
																	// 前面所有的一起取反（用于验证是否有错误的）
	};
	cnl2idx_t					nIdx;								// 数据序号
	unsigned char				dataphase	: 2;					// 数据合并类型，只指针对可靠数据
																	// 见上面的DATAPHASE_XXX
	unsigned char				prop		: 6;					// 属性，参见上面的PROP_XXX
};
enum
{
	CNL2_MAX_SINGLEDATASIZE			= CNL2_MAX_DATAFRAMESIZE - sizeof(CNL2_CMD_DATA_T),
																	// 后面可以包括的数据最大值
};
struct	CNL2_CMD_NOOP_T				: CNL2_CMD_1_T
{
	whtick_t					nSendTime;							// 发送时刻
	whtick_t					nPeerSendTime;						// 对方发送时刻(发起包这个没有意义)
	unsigned char				nCount;								// 周转次数计数(发起者置为0)
	// 后面跟着各个channel的nOutIdxConfirm，这样和接收方的nInIdxConfirm一比较就可以知道是否还需要确认了
	// 不用channelmask，因为两边的channel一定是对应的
	// 2005-07-13 取消后面跟Confirm数据的设计。因为当等到发送NOOP的时候应该已经不需要确认了。
	// 每次收到数据的时候都会导致确认被发送
};
struct	CNL2_CMD_SWITCHADDR_T		: CNL2_CMD_1_T
{
	enum
	{
		SUBCMD_ORDER			= 0x00,								// 命令peer更换地址
		SUBCMD_REQ				= 0x01,								// 请求更换地址
		SUBCMD_RPL				= 0x02,								// 同意更换地址
	};
	unsigned char				nSubCmd;
	unsigned short				nSeed;								// 这个留给以后用
	unsigned char				szKeyMD5[WHMD5LEN];					// 用于身份验证的md5
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
			void			*pData;									// 必须用SlotMan的申请和释放方法来处理
																	// 只要pData非空就说明收到相应的数据了
																	// 数据被上层接收后，相应的InUnit会移出队列并clear
			size_t			nSize;
			unsigned char	dataphase	: 2;						// 数据合并类型，取值参见CNL2_CMD_0_T::DATAPHASE_XXX
			unsigned char	prop		: 6;						// 属性，参见CNL2_CMD_DATA_T::PROP_XXX
			bool			decrypted;								// 表示是否已经解过密了
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
			void	*pCmd;											// 带指令头的数据(crc也计算好了，这样可以避免在重发的时候重新计算crc)
																	// 必须是malloc分配的，因为要free释放
			size_t	nSize;											// 指令尺寸
																	// 如果已经确认则从时间队列中删除，并清空nTimeEventID
			whtick_t					nSendTime;					// 发送的时刻
			unsigned short				nRSCount;					// ReSendCount
			whtimequeue::ID_T			nTimeEventID;				// 对应的超时事件序号
			OUTUNIT_T()
			: pCmd(NULL)
			{
			}
			~OUTUNIT_T()
			{
				assert(pCmd==NULL);									// 必须在析构前调用clear
			}
			void	clear(CNL2SlotMan_I_UDP *pMan);
		};
		CNL2SlotMan_I_UDP				*pSlotMan;					// Slot管理器指针
		CNL2SLOT_I						*pSlot;						// 对应的slot指针
		unsigned char					nChannel;					// Channel的序号
		unsigned char					nProp;						// Channel的属性，参考:CNL2CHANNELINFO_T::PROP_XXX
		int								nType;						// CNL2_CHANNELTYPE_XXX
		// 下面两个队列的长度都不能超过65536
		// 对于lastsafe队列来说，只有Idx0有用。IdxConfirm没有用处。
		cnl2idx_t						nInIdx0;					// 代表pInQueue中第一个元素的下标
		cnl2idx_t						nOutIdx0;					// 代表pOutQueue中第一个元素的下标
		cnl2idx_t						nInIdxConfirm;				// 下一个希望确认的In的序号(也就是目前连续收到的最后一个序号+1)
		cnl2idx_t						nOutIdxConfirm;				// 发出的已经连续确认到的最后一个+1(也就是上层可以收取到)
		whvectorqueue<INUNIT_T>			*pInQueue;					// 所有的都需要这个
		whvectorqueue<OUTUNIT_T>		*pOutQueue;					// 只有safe需要这个，lastsafe需要这个的长度为1
		whDList<CHANNEL_T *>::node		nodeHavingData;				// 说明这个channel接收到了数据
		whDList<CNL_SLOT_CHANNEL_T>::node	nodeHavingDataToSend;	// 说明这个channel需要发送数据
		ICryptFactory::ICryptor			*pICryptorSend;				// 加密器(发送)
		ICryptFactory::ICryptor			*pICryptorRecv;				// 加密器(接收)
		whDList<CMDPACKUNIT_T>			dlistCmdPack;				// 需要打包的指令列表（每个tick结束时肯定清空的）

		CHANNEL_T();
		~CHANNEL_T();
		int	Init(CNL2CHANNELINFO_T *pInfo);
		int	Release();
		// 生成InQueue的确认数据。返回数据长度。
		size_t	MakeInQueueConfirm_Safe(void *pData);
		size_t	MakeInQueueConfirm_LastSafe(void *pData);
		inline cnl2idxdiff_t	idxdiff(cnl2idx_t idx1, cnl2idx_t idx2)
		{
			return	cnl2_idxdiff(idx1, idx2);
		}
		void	AddCmdPackUnit(whDList<CMDPACKUNIT_T>::node *pNode);
	};
	CNL2SLOTINFO_T				slotinfo;							// 用于给外面看
	CHANNEL_T					channel[CNL2_CHANNELNUM_MAX];
	whDList<CHANNEL_T *>		dlistChannelWithData;				// 有数据的channel就放入这个队列
	// 下面teid的序号定义
	enum
	{
		TE_IDX_CONNECT			= 0,
		TE_IDX_ACCEPT			= 0,
		TE_IDX_DROP				= 0,								// 这个是常发的
		TE_IDX_CLOSE			= 0,
		TE_IDX_CONNECT_RS		= 1,
		TE_IDX_ACCEPT_RS		= 1,
		TE_IDX_NOOP				= 1,								// 这个是常发的
		TE_IDX_CLOSE_RS			= 1,
		TE_IDX_MIGHTDROP		= 2,								// “可能断线”的超时
		TE_IDX_SWITCHADDRREQ	= 3,								// 发送地址转换的间隔
		TE_IDX_MAX				= 4,
	};
	whtimequeue::ID_T			teid[TE_IDX_MAX];					// 上面各种情况的超时
	// SlotMan用这些处理Recv到的指令
	CMDDEAL_T					dealfuncRecv[CNL2_CMD_MAX];
	enum
	{
		DLISTIDX_NOOP			= 0,								// 本tick结束前需要发送Noop的列表
		DLISTIDX_COFIRM			= 1,								// 本tick结束前需要发送Confirm的列表
		DLISTIDX_HAVINGDATA		= 2,								// 本tick内收取到数据的slot列表(上层根据这个来收取数据)
		DLISTIDX_MAX
	};
	whDList<CNL2SLOT_I *>::node	dlistNode[DLISTIDX_MAX];			// 之所以用这么多node是因为一个Slot可能同时处于这些队列中
	// Close需要的
	bool						bClosePassive;						// 被动关闭
	bool						bIsClient;							// 这个是主动发起连接的一方
	unsigned char				nCloseCount;						// 关闭计数(当这个数大于1的时候，就可以关闭了)
	unsigned char				nCryptorType;						// 加密器类型 2006-01-23
	// 还需要confirm的channel的掩码
	// 这个在每次收到对方发来的数据时进行更改
	// 也在每次收到对方NOOP包的时候进行更改
	unsigned char				nPeerNotConfirmedChannelMask;
	unsigned char				nNoopCount;
	bool						bNoopMustBeSend;					// 强制一定要发送Noop
																	// 这样在处理时间事件的时候就不用比较lastsend了
	whtick_t					nPeerNoopSendTime;
	whvector<char>				vectExtSlotData;					// 在连接时附加的数据(通过Connect指令发出)
	int							nCloseExtDataSend;					// 在关闭连接时附加的(发送出去)
	int							nCloseExtDataRecv;					// 在关闭连接时附加的(接收到对方的)
	ICryptFactory::IKeyExchanger	*pIKeyExchanger;				// 密钥交换器
	int							nExt;								// 和外部关联的数据
	unsigned char				szKeyMD5[WHMD5LEN];					// 初始加密key的md5码
	CNL2SlotMan_I_UDP			*pSlotMan;

	CNL2SLOT_I();
	void	clear();												// 在撤销关闭slot的时候使用
	void	ClearAllTE();
	void	ClearTE(int nIdx);
	void	ClearChannelAllTE();
	void	SetSlotMan(CNL2SlotMan_I_UDP *pMan);
};
struct	CNL2SlotMan_I_UDP				: public CNL2SlotMan
{
	// 为CNL2SlotMan实现的
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
// 自己用的
// 定义
	struct	TQUNIT_T;
	typedef	void (CNL2SlotMan_I_UDP::*TEDEAL_T)(TQUNIT_T *);
	#pragma pack(1)
	struct	TQUNIT_T
	{
		// !!!!注意，这里记录指针，则必须保证在删除slot的时候要清除所有相关的时间事件!!!!
		CNL2SLOT_I	*pSlot;									// 相关的slot(所有事件应该都是和一个slot相关的)
		TEDEAL_T	tefunc;									// 处理该事件的函数
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
// 数据
private:
	typedef	int (CNL2SlotMan_I_UDP::*CMDDEAL_T)();
	typedef	int (CNL2SlotMan_I_UDP::*SENDFUNC_T)(const void *pData, size_t nSize);
	typedef	void (CNL2SlotMan_I_UDP::*SLOTCHANNELCONFIRM_T)(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	typedef	void (CNL2SlotMan_I_UDP::*DEALSLOT_T)(CNL2SLOT_I *pSlot);
	typedef	n_whcmn::whhash<struct sockaddr_in, int, whcmnallocationobj, _whnet_addr_hashfunc>	ADDR2SLOT_T;
	whDList<CNL_SLOT_CHANNEL_T>						m_dlistChannelHaveDataToSend;
															// 有数据需要发送出去的channel
	typedef	unsigned short	CONNECT_PUBKEY_LEN_T;
	typedef	unsigned char	CONNECT_EXT_LEN_T;
	INFO_T	m_info;
	int		m_nConnectRSTimeout;							// 通过info.nConnectTimeout计算出来
	int		m_nNoopTimeout;									// 通过info.nDropTimeout计算出来，client的在连接之后会从服务器获得这个参数
	int		m_nMightDropTimeout;							// m_nNoopTimeout*2，如果超过了这个时间都没有收到对方来包则可能对方是断线了
	SOCKET	m_sock;
	bool	m_bListen;
	whunitallocatorFixed<CNL2SLOT_I>	m_Slots;
	ADDR2SLOT_T		m_mapaddr2slot;							// 地址和slotid的映射（这个主要是为了处理连接时不会重复生成slot）
															// unsigned int中高两位是channel，低两位是slotid的低两位（即index部分）
	cmn_addr_allowdeny					m_ConnectAllowDeny;	// 对于连接来源地址的限制
	// 下面的用于处理刚刚recvfrom到的数据，也可以用它来合成要发出去的数据
	// (这么使用一定要小心，避免和其他变量混乱的情况)
	whvector<char>	m_vectrawbuf;							// 很多发送指令都使用这个
	whvector<char>	m_vectrawbuf_recv;						// 直接通过recvfrom调用的buf
	whvector<char>	m_vectpackbuf;							// 用于打包指令的缓冲
	whvector<char>	m_vectAuth;								// 用来存储授权文件内容
	whvector<char>	m_vectRSA;								// RSA密钥

	struct	PACK_INFO_T
	{
		int				nSlot;								// 上次还没有完成解包的Slot号（这个大于0才表示有没有完成解包的数据）
		int				nChannel;							// 上次还没有完成解包的Channel号
		whcmdshrink		wcs;								// 用于指令解包
		PACK_INFO_T()
		: nSlot(0), nChannel(0)
		{
		}
	}				m_pack_info;
	CNL2_CMD_0_T	*m_pCmd;								// 指向m_vectrawbuf.getbuf()
	CNL2_CMD_1_T	*m_pCmd1;								// 指向m_vectrawbuf.getbuf()
	CNL2_CMD_DATA_T	*m_pCmdData;							// 指向m_vectrawbuf.getbuf()
	int				m_nCmdSize;								// 刚刚recvfrom读入的cmd的长度
															// 2005-05-27 原来这个是size_t，但是在recv的时候可能得到负值，而导致很大的数字
	void			*m_pData;								// 如果是数据包，则这个指向数据部分
	size_t			m_nDataSize;							// 刚刚recvfrom读入的cmd的长度
	struct sockaddr_in	m_addr;								// 刚刚指令对应的地址
	CNL2SLOT_I		*m_pSlot;								// 当前正在处理的Slot（使用这个只是为了避免传递pSlot到各个函数，但是这样做要格外小心，在很多函数中不要想当然的认为m_pSlot就是自己需要的那个slot）
	CNL2SLOT_I::CHANNEL_T	*m_pChannel;					// 当前正在处理的Slot的Channel
	// 时间队列
	whtimequeue		m_tq;									// 各种超时（连接、断连、重发）
	bool			m_bUseAuth;
	// 当前的时刻
	whtick_t		m_tickNow;								// 在需要的适合获取一次
	// 通知上层的指令队列
	whsmpqueue		m_queueControlOut;
	// 临时存放m_queueControlOut给上层的指令
	whvector<char>	m_vectControlOutCmd;
	// 各通道的处理函数
	CMDDEAL_T				m_TickRecv_CMD_Others_DATA_Deal[CNL2_CHANNELNUM_MAX];
	SENDFUNC_T				m_Send_DATA[CNL2_CHANNELNUM_MAX];
	SLOTCHANNELCONFIRM_T	m_SCC[CNL2_CHANNELNUM_MAX];
	// 参见DLISTIDX_XXX的定义
	whDList<CNL2SLOT_I *>	m_dlistSlot[CNL2SLOT_I::DLISTIDX_MAX];
	// 加密工厂
	ICryptFactory			*m_pICryptFactory;
	// 自己绑定的端口和地址的运算结果（目前的作用只是连接时用来把加密密码做小小的变换）
	port_t					m_nMyAddrSum;
	whtick_t				m_nLastSwitchAddrTime;			// 上次转换地址的时间（两次转换的时间不能短于MightDropTimeout）

	// 小块内存分配器
//	CNL_WCA_T				m_wca;
	// 给m_wca用的原始内存分配器
	whcmnallocationobj		m_a;
	// 检查AllowDeny文件是否被改变
	whfilechangedetector	m_fcdAllowDeny;
	// 流量统计
	whtimestat<whuint64>	m_statDownByte, m_statUpByte;
	whtimestat<int>			m_statDownCount, m_statUpCount;

// 函数
private:
	int		Init_SetParams();
	// 处理扩展指令的函数
	void	TickRecv_CMD_EXTEND();
	// 具体处理CONNECT指令的函数
	void	TickRecv_CMD_CONNECT();
	// 处理换地址指令
	void	TickRecv_CMD_SWITCHADDR();

	// 处理其他指令的总函数
	void	TickRecv_CMD_Others();
	// 被TickRecv_CMD_Others调用
	// 如果返回0表示处理成功，<0表示处理失败
	inline int	TickRecv_CMD_Others_Call(int nCmd)
	{
		// 调用方式保证nCmd不会超界的(因为4bit限制死了)
		return	(this->*m_pSlot->dealfuncRecv[nCmd])();
	}

	// TickRecv_CMD_Others所调用的处理具体指令的函数
	int		TickRecv_CMD_Others_Unknown();
	int		TickRecv_CMD_Others_CONNECT_AGREE();
	int		TickRecv_CMD_Others_CONNECT_REFUSE();
	int		TickRecv_CMD_Others_CLOSE();
	int		TickRecv_CMD_Others_DATA_CONFIRM();

	// TickRecv_CMD_Others_DATA是slot本身用于处理DATA的函数
	int		TickRecv_CMD_Others_DATA();								// 正常处理数据
	int		TickRecv_CMD_Others_DATA_ON_ACCEPTING();				// Accept方在收到第一个数据包之前用这个
	// 后面的这些TickRecv_CMD_Others_DATA_Deal_XXX都是对应具体通道的处理函数，在TickRecv_CMD_Others_DATA内被调用
	int		TickRecv_CMD_Others_DATA_Deal_NotSupport();				// 不支持的通道
	int		TickRecv_CMD_Others_DATA_Deal_RT();						// 实时数据
	int		TickRecv_CMD_Others_DATA_Deal_Safe();					// 可靠数据
	int		TickRecv_CMD_Others_DATA_Deal_LastSafe();				// 靠后可靠数据

	// NOOP处理函数
	int		TickRecv_CMD_Others_NOOP_ON_ACCEPTING();				// Accept方在收到第一个数据包之前用这个
	int		TickRecv_CMD_Others_NOOP();

	int		_TickRecv_FIRST_CMD_ON_ACCEPTING();

	int		TickLogic_PackAndCompression();							// 对本帧的数据进行集体打包和压缩

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
	// 把授权文件载入到内存
	void	TEDeal_CA0(TQUNIT_T *pTQUnit);
	// 读入auth数据
	void	TEDeal_CA1(TQUNIT_T *pTQUnit);
	// 打印log
	void	TEDeal_CA2(TQUNIT_T *pTQUnit);
	// 退出程序
	void	TEDeal_CA3(TQUNIT_T *pTQUnit);
	// 上面几个的设置
	void	SetTE_CA(TEDEAL_T pFunc, int nMS);
	// 在m_vectrawbuf中创建要发出的数据
	void	_MakeDataToSend(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T *pChannel, const void *pData, size_t nSize, unsigned char nDataPhase, unsigned char nProp, cnl2idx_t nIdx);
	int		Send_DATA_NotSupport(const void *pData, size_t nSize);
	int		Send_DATA_RT(const void *pData, size_t nSize);
	int		Send_DATA_Safe(const void *pData, size_t nSize);
	int		Send_DATA_Safe_Pack(const void *pData, size_t nSize);	// 带打包的发送
	int		_Send_DATA_Safe(const void *pData, size_t nSize, unsigned char nProp);
	int		_Send_DATA_Safe_Single(const void *pData, size_t nSize, unsigned char nDataPhase, unsigned char nProp);
																	// 加入一个单一数据块并放入时间队列标记为本tick立即发送
																	// safe和lastsafe模式都会用到这个方法
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
		CLOSEONERR_BAD_DATA_CONFIRM_TOOMANYSECT				= 5,	// 收到太多的确认段
		CLOSEONERR_BAD_DATA_RECV_TOOMANYSECT				= 6,	// 收到的数据分段太多
		CLOSEONERR_BAD_DATA_SIZE							= 8,
		CLOSEONERR_BAD_DATA_NOOP							= 9,
		CLOSEONERR_BAD_DATAPHASE							= 10,
		CLOSEONERR_TickRecv_CMD_Others_DATA_Deal_NotSupport	= 11,
		CLOSEONERR_IdxOutOfRange							= 12,
		CLOSEONERR_BAD_DATA_CONFIRM_SIZE_SMALL				= 21,	// 收到的确认数据的尺寸过小
		CLOSEONERR_BAD_DATA_CONFIRM_BADCHANNELTYPE			= 22,	// channel类型不对
		CLOSEONERR_BAD_DATA_CONFIRM_HDR_SIZE_SMALL			= 23,	// 确认头的部分尺寸过小
		CLOSEONERR_BAD_DATA_CONFIRM_IDX_SMALL				= 24,	// 确认的序号过小
		CLOSEONERR_BAD_DATA_CONFIRM_COUNT_BIG				= 25,	// 确认的部分过多了
		CLOSEONERR_BAD_DATA_CONTENT							= 26,	// 数据内容混乱
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
	// 创建并添加slot，里面创建的当前slot是m_pSlot
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
	void	SendNoopThisTick(CNL2SLOT_I *pSlot);					// 标明在本tick结束的时候如果没有发送任何数据则应该发送NOOP
	void	SendConfirmThisTick(CNL2SLOT_I *pSlot);					// 标明在本tick结束的时候一定要向对方发送dataconfirm包(如果对方还有没有被confirm的channel的话)
	void	RegNoopTQ(CNL2SLOT_I *pSlot, whtick_t nTime);
	void	RegDropTQ(CNL2SLOT_I *pSlot);
	void	RegMightDropTQ(CNL2SLOT_I *pSlot, whtick_t nTime);
	void	RegSwitchAddrReq(CNL2SLOT_I *pSlot, whtick_t nTime);
	void	RegCloseTQ(CNL2SLOT_I *pSlot);
	void	RegCloseRSTQ(CNL2SLOT_I *pSlot);

	// 把所有channel需要重发的指令在这个tick内全部重发
	void	SendAllUnConfirmedDataThisTick(CNL2SLOT_I *pSlot);
	// 将某个outunit立即或者最多延迟nDelay发送（如果原来预定发送时间早于tickNow+nDelay就按预定时间发送）
	int		SendOutUnitThisTick(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit);
	int		SendOutUnitWithDelay(CNL2SLOT_I *pSlot, CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit, int nDelay);
	// 向对方发送地址转换请求
	void	SendSwitchAddrReq(CNL2SLOT_I *pSlot);

	// 如果nNum为0则表示nIdx之前的都被确认
	// 如果nNum>0则表示nIdx之后的nNum个被确认
	void	SlotChannelConfirm(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	void	SlotChannelConfirm_NotSupport(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	void	SlotChannelConfirm_RT(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	void	SlotChannelConfirm_Safe(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);
	bool	_SlotChannelConfirm_Safe_SetConfirm(CNL2SLOT_I::CHANNEL_T::OUTUNIT_T *pOutUnit);
	void	SlotChannelConfirm_LastSafe(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx, int nNum);

	// 通道输出队列的首位置移动到nIdx0
	void	SlotOutChannelMove(CNL2SLOT_I *pSlot, int nChannel, cnl2idx_t nIdx0);
	// 通道输入队列清除头的nNum个数据
	void	SlotInChannelMove(CNL2SLOT_I::CHANNEL_T*pChannel, int nNum);

	// 初始化各个channel的加密器
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

	// inline 部分
	// 不计算crc(用于重发，应该已经计算了一次了)
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
			// 流量统计
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
			// 流量统计
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
			// 已经存在就不用了
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
	// 根据Seed计算某个slot的动态KeyMD5值（里面会动态增加pSlot->slotinfo.nSpecialSeed）
	void	MakeSlotSeedKeyMD5(CNL2SLOT_I *pSlot, unsigned char *szKeyMD5);
	// 验证某个slot的动态KeyMD5值是否正确（里面会判断nSeed必须大于pSlot->slotinfo.nSpecialSeed，并且判断成功后就把新nSeed赋给它）
	bool	VerifySlotSeedKeyMD5(CNL2SLOT_I *pSlot, unsigned int nSeed, unsigned char *szKeyMD5);
	// 改变自己的socket地址
	// 返回：0表示短时间内已经改变过了，不用继续改变
	//       1表示改变了
	//       -1表示出错
	int		SwitchAddr(CNL2SLOT_I *pSlot);

	// 小内存分配和释放
	// 分配（返回分配到的指针）
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
