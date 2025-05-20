// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpLister.h
// Creator      : Wei Hua (魏华)
// Comment      : 利用UDP的进行列表和访问
// CreationDate : 2004-03-04

#ifndef	__WHNETUDPLISTER_H__
#define	__WHNETUDPLISTER_H__

#include <WHCMN/inc/whvector.h>
#include <WHCMN/inc/whhash.h>
#include <WHCMN/inc/whdataprop.h>
#include "whnetcmn.h"

using namespace n_whcmn;

namespace n_whnet
{

enum
{
	WHNETUDPLISTER_MAX_BUF_SIZE		= 2048,		// 也就是一个记录最长的尺寸
};
enum
{
	WHNETUDPLISTER_STATUS_DEAD		= 0,		// 长时间没有收到Update，或者根本就从来没有收到过Update。
	WHNETUDPLISTER_STATUS_WORKING	= 1,		// 正常状态。在规定时间内收到了update包。
};

#pragma pack(1)

typedef	unsigned short	WHNETUDPLISTER_IDX_T;
typedef	unsigned char	WHNETUDPLISTER_NUM_T;
typedef	signed char		WHNETUDPLISTER_STATUS_T;

// 基本包
struct	WHNETUDPLISTER_CMN_CMD_T
{
	typedef	unsigned char			cmd_t;
	typedef	unsigned char			crc_t;
	cmd_t			nCmd;						// 指令(类型包含在里面了)
	crc_t			nCrc;						// 一字节的CRC校验
	unsigned int	nParam;						// 基本数据，如：服务器ID
	char			Data[1];					// 附加数据部分
};

// REQ_QUERY的Data部分的数据组织
// num_t			后面的层数(如果为0，则表示在根下)
// idx_t[n]			层数个索引，表示要检索的基本层
// num_t			后面的序号的个数
// idx_t[n]			要检索的在该层的序号数组

// RPL_QUERY的Data部分的数据组织
struct	WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T
{
	enum
	{
		RST_OK				= 0,				// 成功获取
		RST_ERR_UNKNOWN		= -1,				// 未知错误
		RST_ERR_IDXTOOBIG	= -2,				// 指定的下标太大
	};
	WHNETUDPLISTER_IDX_T	nIdx;				// 节点的序号
	signed char		nRst;						// 是否成功获取
	signed char		nType;						// 如果成功的话，这个是whnetudpListerSvr::NODE_T::TYPE_XXX的内容
	char			Data[1];					// Node数据部分，根据nType的不同解释不同
												// 比如是叶节点就填写状态
};

// TYPE_GRP对应的Data的解释
// bstr					名称
// idx_t				Child的数量

// TYPE_UNIT对应的Data的解释
// bstr					名称
// byte					该unit状态，可能是whnetudpListerSvr::NODEUNIT_T::STATUS_XXXX
// char[]				附加数据(接收到的Update数据)，如果没有收到数据或者timeout就不发送这部分数据
//						附加数据的解释由查询者去做

#pragma pack()

// 指令定义
// 请求
enum
{
	WHNETUDPLISTER_CMD_REQ_UPDATE		= 0x01,	// 更新请求
	WHNETUDPLISTER_CMD_REQ_QUERY		= 0x81,	// 查询获取指定范围的节点信息的请求
};
// 返回
enum
{
	WHNETUDPLISTER_CMD_RPL_QUERY		= WHNETUDPLISTER_CMD_REQ_QUERY,
												// 注意：这个返回一次有一个节点的结果
};

void	whnetudplister_make_crc(WHNETUDPLISTER_CMN_CMD_T *pCmd, size_t nSize);
bool	whnetudplister_check_crc(WHNETUDPLISTER_CMN_CMD_T *pCmd, size_t nSize);

// 列表服务器
class	whnetudpListerSvr
{
public:
	typedef	unsigned int	svrid_t;			// 服务器ID
	struct	CFGINFO_T	: public whdataprop_container
	{
		int		nMaxData;						// 可接收的最大Update长度
		int		nUpdatePort;					// 更新端口
		int		nQueryPort;						// 查询端口
		int		nSelectInterval;				// select的间隔
		int		nUpdateTimeOut;					// 更新超时(毫秒。超过这个时间没有收到Update就认为死掉了)
		CFGINFO_T()
		: nMaxData(512)
		, nUpdatePort(3400)
		, nQueryPort(3401)
		, nSelectInterval(100)
		, nUpdateTimeOut(60000)
		{
		}
		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
	// 配置信息结构
	CFGINFO_T	m_cfginfo;

	struct	NODE_T
	{
		enum
		{
			TYPE_GRP	= 1,					// 树杈，下面含子对象
			TYPE_UNIT	= 2,					// 树叶
		};
		int					nType;				// 类型
		NODE_T				*pFather;			// 父节点指针
		whvector<char>		vectName;			// 名字(变长的)
		whvector<NODE_T *>	Child;				// 子节点指针列表(如果是叶子则里面应该为空)
		NODE_T()
		: nType(TYPE_GRP)
		, pFather(NULL)
		{
		}
		void	DelAllChildren()
		{
			// 删除所有子对象
			for(size_t i=0;i<Child.size();i++)
			{
				delete	Child[i];
			}
			Child.clear();
		}
		virtual ~NODE_T()						// 用虚函数是为了可以直接删除指针而不用转换
		{
			DelAllChildren();
		}
	};
	struct	NODEUNIT_T	: public NODE_T
	{
		whtick_t		nLastUpdate;			// 上次更新的时间(毫秒)
		WHNETUDPLISTER_STATUS_T	nStatus;		// 状态(这个类型今后不要改，因为别的地方用，如：叶子节点的查询结果中会有这个status)
		svrid_t			nID;					// 标识ID
		whvector<char>	Data;					// 变长数据
		NODEUNIT_T()
		: nLastUpdate(0)
		, nStatus(WHNETUDPLISTER_STATUS_DEAD)
		, nID(0)
		{
		}
	};
protected:
	int					m_nCurCFGLine;			// 当前读到的CFG行
	int					m_nMaxLevel;			// 初始化完毕后的最大层数
	int					m_nMaxChildren;			// 初始化完毕后的最大子节点数
	SOCKET				m_sock4Update;			// 用于监听update包
	SOCKET				m_sock4Query;			// 用于监听query访问包
	SOCKET				m_sockArr[2];			// 存上面两个
	NODE_T				m_root;					// 根节点
	whhashset<whnet4byte_t, whcmnallocationobj, _whnet_addr_hashfunc>		m_setTrustedIP;		// 信任的IP列表
	whhash<svrid_t, NODEUNIT_T*, whcmnallocationobj, _whnet_addr_hashfunc>	m_mapID2NodeUnit;	// ID到节点的映射
	whvector<NODEUNIT_T*>								m_vectNodeUnit;		// 节点数组
	whvector<char>		m_recvbuf;				// 接收缓冲
	WHNETUDPLISTER_CMN_CMD_T	*m_pRecvedCmd;	// 刚刚收到的指令缓冲，初始化时指向m_recvbuf.getbuf()
	int					m_nRecvedDataSize;		// m_recvbuf.size()-sizeof(*m_pRecvedCmd)+1;
	struct sockaddr_in	m_curaddr;				// 当前处理的数据的来源地址
	char				m_szQueryRplBuf[WHNETUDPLISTER_MAX_BUF_SIZE];
												// 用于生成QueryReplay返回结果
public:
	whnetudpListerSvr();
	~whnetudpListerSvr();
	enum
	{
		INITRST_OK						= 0,	// 成功
		INITRST_ERR_CFG_FILE			= -1,	// 文件错误
		INITRST_ERR_CFG_DUPTRUSTEDIP	= -2,	// 有IP重复
		INITRST_ERR_CFG_DUPID			= -3,	// 有ID重复
		INITRST_ERR_CFG_NOID			= -4,	// 没有设置ID
		INITRST_ERR_CFG_BADID			= -5,	// 错误的ID值
		INITRST_ERR_CFG_SYNTAX			= -10,	// 语法错误
		INITRST_ERR_CREATESOCKET		= -11,	// 无法创建socket
	};
	int		Init(const char *cszCFG);
	int		Release();
	int		Tick();								// 一次工作
public:
	inline int	GetCurCFGLine() const
	{
		return	m_nCurCFGLine;
	}
private:
	int		Init_CFG(const char *cszCFG);		// 从配置文件中初始化
	int		Init_Net();							// 初始化网络相关内容
	int		Tick_Update();						// 处理Update
	int		Tick_Update_KeepAlive();			// 看是否有长时间没有收到Update的UNIT，改变他们的状态
	int		Tick_Update_WHNETUDPLISTER_CMD_REQ_UPDATE();
	int		Tick_Query();						// 处理Query
	int		Tick_Query_WHNETUDPLISTER_CMD_REQ_QUERY();

	NODE_T *	GetLevelNode(WHNETUDPLISTER_IDX_T *pLevel, int nLevel);

	// 填充grp的数据到pBuf，返回数据总长度
	int		FillGrpData(void *pBuf, NODE_T *pNode);
	// 填充unit的数据到pBuf，返回数据总长度
	int		FillUnitData(void *pBuf, NODE_T *pNode);

	// 向当前m_curaddr发送查询结果数据
	int		SendQueryRstToCurAddr(WHNETUDPLISTER_CMN_CMD_T *pCmd, int nSize);
};

// 说白了就是ListerServer的客户端（update或query）
class	whnetudpListerCmdMaker
{
protected:
	SOCKET	m_sock;
	char	m_szCMDBuf[WHNETUDPLISTER_MAX_BUF_SIZE];
	whnetudpListerSvr::svrid_t	m_nSvrID;		// 如果自己是Svr，需要发出update，则要设置这个
public:
	whnetudpListerCmdMaker()
	: m_sock(INVALID_SOCKET)
	, m_nSvrID(0)
	{
	}
	inline void	SetSocket(SOCKET sock)
	{
		m_sock		= sock;
	}
	inline void	SetSvrID(whnetudpListerSvr::svrid_t nID)
	{
		m_nSvrID	= nID;
	}
	inline whnetudpListerSvr::svrid_t	GetSvrID() const
	{
		return		m_nSvrID;
	}
	// 向指定地址发送update信息
	// 返回和sendto返回一致(0成功)
	int		SendReqUpdate(const struct sockaddr_in *pAddr, const void *pData, int nSize);
	// 向指定地址发送query指令
	// 返回和sendto返回一致(>0成功)
	// 查询规则：
	// nLevelNum为0表示从根目录查，否则表示从pLevel表示的根开始查
	// 如果pIdx的[0]和[1]相同，则表示查询从序号[0]开始
	// 如果[2]存在，则表示要查询的个数，否则表示查询到结尾
	int		SendReqQuery(const struct sockaddr_in *pAddr
			, WHNETUDPLISTER_IDX_T *pLevel, WHNETUDPLISTER_NUM_T nLevelNum
			, WHNETUDPLISTER_IDX_T *pIdx, WHNETUDPLISTER_NUM_T nIdxNum
			);
private:
	int		CalcCRCAndSend(const struct sockaddr_in *pAddr, WHNETUDPLISTER_CMN_CMD_T *pCmd, int nSize);
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETUDPLISTER_H__
