// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetcnl2.h
// Creator      : Wei Hua (魏华)
// Comment      : 基于UDP的网络通讯协议2005升级版
//                CNL是Clever Net Layer的缩写
//                一个Man可管理的最大连接数不超过8192个
//                缓冲单元的个数不超过65536个
//                Noop就是KeepAlive
//                Noop由client端发起，经过三次往复结束，双方都可以获得RTT值
//                2005-07-15 注意：必须要在每个循环内调用TickRecv之后，调用Recv把所有数据都接收干净。
//                           否则如果长时间不调用Recv，而在In队列满了之后，发送方就不能再发送任何数据。
//                           而接收方因为接收不到任何数据，就不能主动发送确认，从而导致发送方永远不能再发送数据了。
//                当然，如果在Recv中标记channel需要确认，同时在Recv结束之后设置Slot需要发送Confirm就可以解决这个问题。
//                           目前通过定义宏CNL2_RECV_TRIGER_CONFIRM来实施这种解决方案
// CreationDate : 2005-04-16
// ChangeLog    : 2005-07-29 突然想到现在的CNL2中增加了太多的TCP不需要的参数，所以在这个基础上实现TCP的CNL也许不太合适。所以放弃这个打算。
//                2005-08-25 增加了ReConfigChannel函数以重新设定channel属性
//                2006-01-23 把原来的CNL2SlotMan::INFO_T::nCryptorType改为范围，加密方式为服务器随机一个type之后传给客户端 
//                2006-11-29 增加了nMaxNoopTime和nMinNoopTime以避免服务器超时设置过长导致客户端没有任何数据发出时超时掉线
//                2007-06-12 修改了CNL2_VER=1（主要是为了PNGSClient修改了客户端协议，强制大家统一）
//                2007-08-09 修改了CNL2_VER=2（因为增加了IP改变仍旧可以断线重连的功能）
//                2007-08-14 把CNL2的默认发送和接收buffer设置为512k
//                2007-12-05 修改了加密方式，网络底层版本也修改为3
//                2009-03-03 CNL2_MAX_DATAFRAMESIZE改为1024，因为台湾的家用路由器的MTU默认是1400出头，导致会有丢包。

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

// 一些常量
enum
{
	CNL2_VER					= 4,								// 当前的版本
	CNL2_CHANNELNUM_MAX			= 8,								// 最多的通道数
	CNL2_MAX_DATAFRAMESIZE		= 1024,								// 一个UDP包的最大尺寸。一个包不能太大了(以太网的MTU, Maximum Transmission Unit, 是1500，UDP和IP都有包头的，所以还是节约一些啦)
																	// 一般IP首部为20字节，UDP首部为8字节，数据的净荷（payload）部分预留是 
																	// 1500-20-8=1472字节。如果数据部分大于1472字节，就会出现分片现象。 
																	// 不过ADSL的MTU是1492，所以需要再减8
																	// 所以是1464
																	// 为了保险再缩水一些
																	// 2005-05-10 可以用ping -f -l size addr的方式测试MTU.
																	// 我测试的adsl是1480，即ping可以发送的最大fragment是1452
																	// 为了保险就再降20
																	// 2009-03改为1024，因为台湾的家用路由器的MTU默认是1400出头，导致会有丢包。
	CNL2_MAX_DATATOSEND			= 1000000,							// 可以通过Send一次发送的最大数据(建议对于超大数据还是自己先分解好再发，然后需要客户端主动进行每段确认的，否则会造成网络层的缓冲太大)
	CNL2_MAX_INOUTQUEUESIZE		= 0x8000,							// 每个channel
	CNL2_DFT_PING				= 200,								// 默认的起始网络延时
	CNL2_MAX_PING				= 2000,								// 最大的网络延时(如果计算结果超过了这个，说明网络非常不好了，所以让最大延时等于这个)
	CNL2_MIN_PING				= 4,								// 最小的网络延时(如果延时为0的话会有问题)
	CNL2_MAX_SLOTEXTDATA_SIZE	= 1024,								// 最大的附加参数长度
};
enum
{
	CNL2_CHANNELTYPE_NOTHING	= 0,								// 未定义的channel
	CNL2_CHANNELTYPE_RTDATA		= 1,								// 不可靠的，实时数据，不保证顺序
	CNL2_CHANNELTYPE_SAFE		= 2,								// 可靠的，先入先出的
	CNL2_CHANNELTYPE_LASTSAFE	= 3,								// 半可靠的，保证顺序，但是中间可以丢失数据的
																	// 这种channel的In/Out队列都各只有一个元素，初始化参数中表示队列长度的的相应参数会被忽略
};

// 因为可能做成dll给别人调用，所以需要限对齐方式（不过因为结构里面的最大成员长度为4，所以相当于4字节对齐）
#pragma pack(8)
struct	CNL2CHANNELINFO_T
{
	enum
	{
		PROP_CANSEND			= 0x01,								// 可发送数据
		PROP_CANRECV			= 0x02,								// 可接收数据
		PROP_FASTACK			= 0x04,								// 希望快速确认(比如消息通道就不需要快速确认)
	};
	int		nType;													// CNL2_CHANNELTYPE_XXX
	int		nInNum;													// 传入队列的长度
	int		nOutNum;												// 传出队列的长度
	unsigned char	nProp;											// 属性
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
		STATUS_CONNECTING	= 1,									// 尝试连接
		STATUS_ACCEPTING	= 2,									// 尝试接受
		STATUS_WORKING		= 3,									// 连接成功，正常工作
		STATUS_CLOSING		= 4,									// 正在关闭中
	};
	int		nStatus;												// 目前状态
	int		nSlot;													// Slot的ID
	short	nPeerSlotIdx;											// 对方的Slot的序号，注意不是ID
	struct	sockaddr_in		peerAddr;								// 对方地址
	int		nRTT;													// 网络延时（来回的）
	int		nUpBPS;													// 上行传输率（发送的）
	int		nDownBPS;												// 下行传输率（接收的）
	short	nLostRate;												// 丢包率（百分率）
	unsigned int	nSpecialSeed;									// 用于特殊事件的轮转种子

	whtick_t		nLastSend;										// 上次发送数据时刻
	whtick_t		nLastRecv;										// 上次接收数据时刻
	void	clear()
	{
		memset(this, 0, sizeof(*this));
		nLastSend	= wh_gettickcount();
		nLastRecv	= nLastSend;
		nRTT		= CNL2_DFT_PING;
	}
	const char *	infostr(char *pszStr)										// 打印到字串
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
	// 结构
	struct	INFO_T
	{
		char				szBindAddr[WHNET_MAXADDRSTRLEN];		// 绑定的本地地址(全空的字串代表绑定所有网卡界面的某个可用端口)
		unsigned char		ver;									// 用于校验是否是同类连接
		bool				bUseTickPack;							// 对同一帧发出的数据尽量整合为一个包发出
		bool				bUseCompression;						// 是否使用对包的压缩（注意：不pack，那么压缩就是没有意义的）
		bool				bCanSwitchAddr;							// 是否可以在认为断线的时候改变自己的地址
		bool				bLogNetStat;							// 定期输出网络统计到LOG中
		bool				bLogSendRecv;							// 输出发送和接收的包到cmn日志中
		int					nMaxConnection;							// 最多的连接数目(包括连入连出)
		int					nSockSndBufSize;						// socket的发送缓冲
		int					nSockRcvBufSize;						// socket的接收缓冲
																	// 注：nSockSndBufSize和nSockRcvBufSize在windows下可以设的很高，但是在普通配置的linux默认只能设到131072，参见/proc/sys/net/core中rmem_max和wmem_max。如果为0表示不用重设。
		int					nMaxSinglePacketSize;					// 逻辑上单独一个包的最大尺寸
																	// 这个不应该超过CNL2_MAX_DATAFRAMESIZE*channel的OutNum的最小值，否则这样的大指令分割完肯定导致输出队列溢出。
		// 超时设置
		int					nConnectTimeout;						// 接受一个连接的超时或者连接别人的超时
		int					nConnectRSDiv;							// 连接重发时间为nConnectTimeout/nConnectRSDiv
		int					nCloseTimeout;							// 关闭时的等待
		int					nDropTimeout;							// 断连超时
		int					nNoopDivDrop;							// 如果一段时间没有发送Send就发送Noop
		int					nMaxNoopTime;							// 最长的Noop时间（如果nDropTimeout设置的很大的话，那么可能会导致Noop的时间很长，这样如果对方没有进行相应的设置的话，就会导致对方超时掉线）
		int					nMinNoopTime;							// 最短的Noop时间（因为一般超时不会设置的太小）
																	// Noop的超时应该是max(min(nDropTimeout/nNoopDivDrop, nMaxNoopTime), nMinNoopTime)
		// 对于连接的允许和禁止的地址文件列表
		char				szConnectAllowDeny[WH_MAX_PATH];		// 文件名，如果为空表示不使用
		int					nConnectAllowDenyRefreshInterval;		// 刷新改文件内容的间隔（毫秒）
		// 时间队列
		int					nTQChunkSize;							// 对应whtimequeue::INFO_T中的nChunkSize
																	// 对于服务器，可以把这个定的比较高，比如10000，客户端可以低一些，比如100。
		// 用于通知上层的指令队列长度
		int					nControlOutQueueSize;
		// 加密类型(在一定范围内随机)
		int					nCryptorType1, nCryptorType2;
		// 通道信息
		CNL2CHANNELINFO_T	channelinfo[CNL2_CHANNELNUM_MAX];
		// chunk分配器信息（主要是用来做大量的发送数据包分配的，管理器内部用的是whchunkallocationobj）
		int					nAllocChunkSize;
		int					nAlloc_MAXSIZENUM;
		int					nAlloc_HASHSTEP1;
		int					nAlloc_HASHSTEP2;

		int					nStatInterval;							// 记录统计的间隔

		//
		char				szRSAPriKeyFile[WH_MAX_PATH];			// RSA私钥文件
		char				szRSAPass[64];							// RSA密码
		char				szAuthFile[WH_MAX_PATH];				// 授权文件（加密后）

		INFO_T()
		: ver(CNL2_VER)
		, bUseTickPack(true)
		, bUseCompression(false)
		, bCanSwitchAddr(false)
		, bLogNetStat(false)
		, bLogSendRecv(false)
		, nMaxConnection(2)
		, nSockSndBufSize(512*1024)		// 2007-08-14 把默认buffer设置为512k
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
	// 这个为了可以通过配置文件自动填充
	WHDATAINI_STRUCT_DECLARE(DATA_INI_INFO_T, INFO_T)
	// 函数
	enum
	{
		TYPE_UDP			= 0,
		TYPE_TCP			= 1,
	};
	// 记住这个默认值永远不要改了，因为很多地方都用了
	// 这个Create出来的对象需要调用SelfDestroy删除（推荐使用WHSafeSelfDestroy）
	static	CNL2SlotMan *	Create(int nType=TYPE_UDP);
protected:
	virtual	~CNL2SlotMan()	{};
public:
	// 自我销毁
	virtual	void	SelfDestroy()									= 0;
	// 一帧中最多Send的数据长度(可以发送更多，不过可能会导致内部分片)
	virtual	int		GetMaxSingleDataSize()							= 0;
	// 初始化，返回本地绑定地址
	virtual	int		Init(INFO_T *pInfo, struct sockaddr_in *pAddr)	= 0;
	virtual	int		Init_Reload(INFO_T *pInfo)						= 0;
	// 重新设置channel属性(这个主要用于若干次连接对Channel的要求不同，如：连CAAFS的时候只有一个Channel，连CLS的时候需要4个)
	// 不过要切记如果更换设置的时候必须保证前面的所有slot都已经关闭了
	virtual	int		ReConfigChannel(INFO_T *pInfo)					= 0;
	// 终结
	virtual	int		Release()										= 0;
	// 获得可被外界做select的SOCKET，append到vector的尾部(所以上层最开始使用前需要vect.clear一下)
	// 这个主要是给外界统一select用(因为实际上就一个socket)
	virtual	int		GetSockets(whvector<SOCKET> &vect)				= 0;
	// 这个用于简便使用
	virtual	SOCKET	GetSocket() const								= 0;
	// 内部select
	virtual	int		DoSelect(int nMS)								= 0;
	// 处理接收的数据
	virtual	int		TickRecv()										= 0;
	// 处理内部逻辑和发送
	virtual	int		TickLogicAndSend()								= 0;
	// 设置是否listen（默认listen是false，即不接受连接）
	virtual	int		Listen(bool bListen)							= 0;
	// 连接目标地址，如果返回>0则表示slotid
	// 附加数据的长度不能超过255
	virtual	int	Connect(struct sockaddr_in *addr, void *pExtData=NULL, int nExtSize=0)
																	= 0;
	virtual	int	Connect(const char *cszAddr, void *pExtData=NULL, int nExtSize=0)
																	= 0;
	// 关闭连接
	virtual	int		Close(int nSlot, int nExtData=0)				= 0;
	// 关闭所有连接
	virtual	int		CloseAll()										= 0;
	// 发送数据
	virtual	int		Send(int nSlot, const void *pData, size_t nSize, int nChannel)
																	= 0;
	// *ppData指向的是内部的一个vector缓冲
	// 返回0表示有数据
	// 注意：必须要在每个循环内调用TickRecv之后，调用Recv把所有数据都接收干净
	// 否则如果长时间不调用Recv，而在In队列满了之后，发送方就不能再发送任何数据
	// 而接收方因为接收不到任何数据，就不能主动发送确认，从而导致发送方永远不能再发送数据了
	virtual	int		Recv(int *pnSlot, int *pnChannel, void **ppData, size_t *pnSize)
																	= 0;
	// 指令交互
	enum
	{
		CONTROL_OUT_SLOT_ACCEPTED		= 1,						// Slot完成连入
																	// data[0]为slot号
		CONTROL_OUT_SLOT_CONNECTED		= 2,						// Slot完成连出
																	// data[0]为slot号
		CONTROL_OUT_SLOT_CLOSED			= 3,						// Slot关闭(可能是超时或正常关闭)
																	// CONTROL_OUT_SLOT_CLOSE_REASON_T
		CONTROL_OUT_SLOT_CONNECTREFUSED	= 4,						// Slot连接被拒绝
																	// CONTROL_OUT_SLOT_CLOSE_REASON_T
		CONTROL_OUT_SHOULD_STOP			= 5,						// 通知上层关闭
																	// 暂无参数
	};
	enum
	{
		// 关联的定义CLS_GMS_CLIENT_DROP_T::REMOVEREASON_XXX
		// 注意这个如果修改了也需要在关联的地方修改
		SLOTCLOSE_REASON_NOTHING		= 0x00,						// 未知理由
		SLOTCLOSE_REASON_INITIATIVE		= 0x01,						// 主动关闭
		SLOTCLOSE_REASON_PASSIVE		= 0x02,						// 被动关闭
		SLOTCLOSE_REASON_CLOSETimeout	= 0x03,						// 关闭时超时
		SLOTCLOSE_REASON_DROP			= 0x04,						// 超时关闭
		SLOTCLOSE_REASON_CONNECTTimeout	= 0x05,						// 连接过程中超时关闭
		SLOTCLOSE_REASON_ACCEPTTimeout	= 0x06,						// 接受过程中超时关闭
		SLOTCLOSE_REASON_KEYNOTAGREE	= 0x07,						// KEY交换错误
	};
	enum
	{
		CONNECTREFUSE_REASON_NOTHING		= 0x00,					// 没有原因，就是看你不顺眼
		CONNECTREFUSE_REASON_BADVER			= 0x01,					// 版本不对
		CONNECTREFUSE_REASON_SLOTFULL		= 0x02,					// Slot已经分配到最大值
		CONNECTREFUSE_REASON_BADDATA		= 0x03,					// 数据错误
		CONNECTREFUSE_REASON_INTERNALERROR	= 0xFF,					// 内部错误(比如内存分配出错)
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
		int			nSlot;											// 如果有就是对应Slot的ID
		struct	sockaddr_in		peerAddr;							// 对方地址
		unsigned char	nReason;									// SLOTCLOSE_REASON_XXX
																	// CONNECTREFUSE_REASON_XXX
		int				nExtData;									// 关闭的附加数据
		long			nSlotExt;									// Slot附加数据(因为如果是close则可能要删除相关的上层逻辑对象)
	};
	#pragma pack()
	// 获得内部通知（比如某个slot已经断线）
	// 返回0表示有数据，-1表示没数据
	virtual	int		ControlOut(CONTROL_T **ppCmd, size_t *pnSize)	= 0;
	// 获得SLOT信息(并可以修改其部分内容)
	virtual	CNL2SLOTINFO_T *	GetSlotInfo(int nSlot)				= 0;
	// 判断slot是否存在
	virtual	bool	IsSlotExist(int nSlot) const					= 0;
	// 获得地址允许对象
	virtual cmn_addr_allowdeny &	GetConnectAllowDeny()			= 0;
	// 获得连接附加数据
	virtual	whvector<char> *	GetConnectExtData(int nSlot)		= 0;
	// 设置关联数据
	virtual	int		SetExtLong(int nSlot, long nExt)				= 0;
	// 获取关联数据
	virtual	int		GetExtLong(int nSlot, long *pnExt)				= 0;
	// 设置加密工厂(上层注意保证在SlotMan::Release之后才能把pICryptFactory终结掉)
	virtual	void	SetICryptFactory(ICryptFactory *pICryptFactory)
																	= 0;
	// 获得当前的slot数量
	virtual	int		GetSlotNum() const								= 0;

	// 设置使用auth文件
	virtual	void	SetUseAuth()									= 0;
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETCNL2_H__
