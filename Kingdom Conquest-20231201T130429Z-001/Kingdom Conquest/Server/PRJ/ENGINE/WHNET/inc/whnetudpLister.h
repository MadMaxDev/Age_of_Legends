// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpLister.h
// Creator      : Wei Hua (κ��)
// Comment      : ����UDP�Ľ����б�ͷ���
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
	WHNETUDPLISTER_MAX_BUF_SIZE		= 2048,		// Ҳ����һ����¼��ĳߴ�
};
enum
{
	WHNETUDPLISTER_STATUS_DEAD		= 0,		// ��ʱ��û���յ�Update�����߸����ʹ���û���յ���Update��
	WHNETUDPLISTER_STATUS_WORKING	= 1,		// ����״̬���ڹ涨ʱ�����յ���update����
};

#pragma pack(1)

typedef	unsigned short	WHNETUDPLISTER_IDX_T;
typedef	unsigned char	WHNETUDPLISTER_NUM_T;
typedef	signed char		WHNETUDPLISTER_STATUS_T;

// ������
struct	WHNETUDPLISTER_CMN_CMD_T
{
	typedef	unsigned char			cmd_t;
	typedef	unsigned char			crc_t;
	cmd_t			nCmd;						// ָ��(���Ͱ�����������)
	crc_t			nCrc;						// һ�ֽڵ�CRCУ��
	unsigned int	nParam;						// �������ݣ��磺������ID
	char			Data[1];					// �������ݲ���
};

// REQ_QUERY��Data���ֵ�������֯
// num_t			����Ĳ���(���Ϊ0�����ʾ�ڸ���)
// idx_t[n]			��������������ʾҪ�����Ļ�����
// num_t			�������ŵĸ���
// idx_t[n]			Ҫ�������ڸò���������

// RPL_QUERY��Data���ֵ�������֯
struct	WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T
{
	enum
	{
		RST_OK				= 0,				// �ɹ���ȡ
		RST_ERR_UNKNOWN		= -1,				// δ֪����
		RST_ERR_IDXTOOBIG	= -2,				// ָ�����±�̫��
	};
	WHNETUDPLISTER_IDX_T	nIdx;				// �ڵ�����
	signed char		nRst;						// �Ƿ�ɹ���ȡ
	signed char		nType;						// ����ɹ��Ļ��������whnetudpListerSvr::NODE_T::TYPE_XXX������
	char			Data[1];					// Node���ݲ��֣�����nType�Ĳ�ͬ���Ͳ�ͬ
												// ������Ҷ�ڵ����д״̬
};

// TYPE_GRP��Ӧ��Data�Ľ���
// bstr					����
// idx_t				Child������

// TYPE_UNIT��Ӧ��Data�Ľ���
// bstr					����
// byte					��unit״̬��������whnetudpListerSvr::NODEUNIT_T::STATUS_XXXX
// char[]				��������(���յ���Update����)�����û���յ����ݻ���timeout�Ͳ������ⲿ������
//						�������ݵĽ����ɲ�ѯ��ȥ��

#pragma pack()

// ָ���
// ����
enum
{
	WHNETUDPLISTER_CMD_REQ_UPDATE		= 0x01,	// ��������
	WHNETUDPLISTER_CMD_REQ_QUERY		= 0x81,	// ��ѯ��ȡָ����Χ�Ľڵ���Ϣ������
};
// ����
enum
{
	WHNETUDPLISTER_CMD_RPL_QUERY		= WHNETUDPLISTER_CMD_REQ_QUERY,
												// ע�⣺�������һ����һ���ڵ�Ľ��
};

void	whnetudplister_make_crc(WHNETUDPLISTER_CMN_CMD_T *pCmd, size_t nSize);
bool	whnetudplister_check_crc(WHNETUDPLISTER_CMN_CMD_T *pCmd, size_t nSize);

// �б������
class	whnetudpListerSvr
{
public:
	typedef	unsigned int	svrid_t;			// ������ID
	struct	CFGINFO_T	: public whdataprop_container
	{
		int		nMaxData;						// �ɽ��յ����Update����
		int		nUpdatePort;					// ���¶˿�
		int		nQueryPort;						// ��ѯ�˿�
		int		nSelectInterval;				// select�ļ��
		int		nUpdateTimeOut;					// ���³�ʱ(���롣�������ʱ��û���յ�Update����Ϊ������)
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
	// ������Ϣ�ṹ
	CFGINFO_T	m_cfginfo;

	struct	NODE_T
	{
		enum
		{
			TYPE_GRP	= 1,					// ��辣����溬�Ӷ���
			TYPE_UNIT	= 2,					// ��Ҷ
		};
		int					nType;				// ����
		NODE_T				*pFather;			// ���ڵ�ָ��
		whvector<char>		vectName;			// ����(�䳤��)
		whvector<NODE_T *>	Child;				// �ӽڵ�ָ���б�(�����Ҷ��������Ӧ��Ϊ��)
		NODE_T()
		: nType(TYPE_GRP)
		, pFather(NULL)
		{
		}
		void	DelAllChildren()
		{
			// ɾ�������Ӷ���
			for(size_t i=0;i<Child.size();i++)
			{
				delete	Child[i];
			}
			Child.clear();
		}
		virtual ~NODE_T()						// ���麯����Ϊ�˿���ֱ��ɾ��ָ�������ת��
		{
			DelAllChildren();
		}
	};
	struct	NODEUNIT_T	: public NODE_T
	{
		whtick_t		nLastUpdate;			// �ϴθ��µ�ʱ��(����)
		WHNETUDPLISTER_STATUS_T	nStatus;		// ״̬(������ͽ��Ҫ�ģ���Ϊ��ĵط��ã��磺Ҷ�ӽڵ�Ĳ�ѯ����л������status)
		svrid_t			nID;					// ��ʶID
		whvector<char>	Data;					// �䳤����
		NODEUNIT_T()
		: nLastUpdate(0)
		, nStatus(WHNETUDPLISTER_STATUS_DEAD)
		, nID(0)
		{
		}
	};
protected:
	int					m_nCurCFGLine;			// ��ǰ������CFG��
	int					m_nMaxLevel;			// ��ʼ����Ϻ��������
	int					m_nMaxChildren;			// ��ʼ����Ϻ������ӽڵ���
	SOCKET				m_sock4Update;			// ���ڼ���update��
	SOCKET				m_sock4Query;			// ���ڼ���query���ʰ�
	SOCKET				m_sockArr[2];			// ����������
	NODE_T				m_root;					// ���ڵ�
	whhashset<whnet4byte_t, whcmnallocationobj, _whnet_addr_hashfunc>		m_setTrustedIP;		// ���ε�IP�б�
	whhash<svrid_t, NODEUNIT_T*, whcmnallocationobj, _whnet_addr_hashfunc>	m_mapID2NodeUnit;	// ID���ڵ��ӳ��
	whvector<NODEUNIT_T*>								m_vectNodeUnit;		// �ڵ�����
	whvector<char>		m_recvbuf;				// ���ջ���
	WHNETUDPLISTER_CMN_CMD_T	*m_pRecvedCmd;	// �ո��յ���ָ��壬��ʼ��ʱָ��m_recvbuf.getbuf()
	int					m_nRecvedDataSize;		// m_recvbuf.size()-sizeof(*m_pRecvedCmd)+1;
	struct sockaddr_in	m_curaddr;				// ��ǰ��������ݵ���Դ��ַ
	char				m_szQueryRplBuf[WHNETUDPLISTER_MAX_BUF_SIZE];
												// ��������QueryReplay���ؽ��
public:
	whnetudpListerSvr();
	~whnetudpListerSvr();
	enum
	{
		INITRST_OK						= 0,	// �ɹ�
		INITRST_ERR_CFG_FILE			= -1,	// �ļ�����
		INITRST_ERR_CFG_DUPTRUSTEDIP	= -2,	// ��IP�ظ�
		INITRST_ERR_CFG_DUPID			= -3,	// ��ID�ظ�
		INITRST_ERR_CFG_NOID			= -4,	// û������ID
		INITRST_ERR_CFG_BADID			= -5,	// �����IDֵ
		INITRST_ERR_CFG_SYNTAX			= -10,	// �﷨����
		INITRST_ERR_CREATESOCKET		= -11,	// �޷�����socket
	};
	int		Init(const char *cszCFG);
	int		Release();
	int		Tick();								// һ�ι���
public:
	inline int	GetCurCFGLine() const
	{
		return	m_nCurCFGLine;
	}
private:
	int		Init_CFG(const char *cszCFG);		// �������ļ��г�ʼ��
	int		Init_Net();							// ��ʼ�������������
	int		Tick_Update();						// ����Update
	int		Tick_Update_KeepAlive();			// ���Ƿ��г�ʱ��û���յ�Update��UNIT���ı����ǵ�״̬
	int		Tick_Update_WHNETUDPLISTER_CMD_REQ_UPDATE();
	int		Tick_Query();						// ����Query
	int		Tick_Query_WHNETUDPLISTER_CMD_REQ_QUERY();

	NODE_T *	GetLevelNode(WHNETUDPLISTER_IDX_T *pLevel, int nLevel);

	// ���grp�����ݵ�pBuf�����������ܳ���
	int		FillGrpData(void *pBuf, NODE_T *pNode);
	// ���unit�����ݵ�pBuf�����������ܳ���
	int		FillUnitData(void *pBuf, NODE_T *pNode);

	// ��ǰm_curaddr���Ͳ�ѯ�������
	int		SendQueryRstToCurAddr(WHNETUDPLISTER_CMN_CMD_T *pCmd, int nSize);
};

// ˵���˾���ListerServer�Ŀͻ��ˣ�update��query��
class	whnetudpListerCmdMaker
{
protected:
	SOCKET	m_sock;
	char	m_szCMDBuf[WHNETUDPLISTER_MAX_BUF_SIZE];
	whnetudpListerSvr::svrid_t	m_nSvrID;		// ����Լ���Svr����Ҫ����update����Ҫ�������
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
	// ��ָ����ַ����update��Ϣ
	// ���غ�sendto����һ��(0�ɹ�)
	int		SendReqUpdate(const struct sockaddr_in *pAddr, const void *pData, int nSize);
	// ��ָ����ַ����queryָ��
	// ���غ�sendto����һ��(>0�ɹ�)
	// ��ѯ����
	// nLevelNumΪ0��ʾ�Ӹ�Ŀ¼�飬�����ʾ��pLevel��ʾ�ĸ���ʼ��
	// ���pIdx��[0]��[1]��ͬ�����ʾ��ѯ�����[0]��ʼ
	// ���[2]���ڣ����ʾҪ��ѯ�ĸ����������ʾ��ѯ����β
	int		SendReqQuery(const struct sockaddr_in *pAddr
			, WHNETUDPLISTER_IDX_T *pLevel, WHNETUDPLISTER_NUM_T nLevelNum
			, WHNETUDPLISTER_IDX_T *pIdx, WHNETUDPLISTER_NUM_T nIdxNum
			);
private:
	int		CalcCRCAndSend(const struct sockaddr_in *pAddr, WHNETUDPLISTER_CMN_CMD_T *pCmd, int nSize);
};

}		// EOF namespace n_whnet

#endif	// EOF __WHNETUDPLISTER_H__
