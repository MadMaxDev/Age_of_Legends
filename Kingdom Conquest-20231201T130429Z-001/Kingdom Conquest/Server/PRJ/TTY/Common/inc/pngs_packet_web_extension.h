#ifndef __PNGS_PACKET_WEB_EXTENSION_H__
#define __PNGS_PACKET_WEB_EXTENSION_H__

#include "PNGS/inc/pngs_def.h"
#include "WHNET/inc/whnetcmn.h"
#include "WHNET/inc/whnetepoll.h"

using namespace n_whnet;

namespace n_pngs
{
// �����������ĵ�ǰ�汾(�汾��ƥ���ʱ���ܻ���)
enum
{
	LPMainStructure_VER			= 1,
	LPGamePlaye_VER				= 1,
	CAAFS4Web_VER				= 1,
	CLS4Web_VER					= 1,
	Postman4Web_VER				= 1,
	GS4Web_VER					= 1,
	BD4Web_VER					= 1,
};

// �ն�����
enum
{
	TERM_TYPE_NONE				= 0,			// ɶҲ����
	TERM_TYPE_IOS				= 1,			// ƻ���ֻ�
	TERM_TYPE_ANDROID			= 2,			// ��׿
	TERM_TYPE_WP				= 3,			// windows phone
	TERM_TYPE_WEB				= 4,			// web��
	TERM_TYPE_IPAD				= 5,			// iPad
	TERM_TYPE_NUM,								// ���ֵ
};

//////////////////////////////////////////////////////////////////////////
// ָ���
//////////////////////////////////////////////////////////////////////////
enum
{
	//////////////////////////////////////////////////////////////////////////
	// CAAFS4Web->LP
	//////////////////////////////////////////////////////////////////////////
	// CAAFS4Web����LP������Լ�����Ϣ
	CAAFS4Web_LP_HI							= 1,
	// ֪ͨLP������뿪ʼ����CLS4Web��(��ҵ�һЩ��Ϣ����֪ͨ��CLS4Web)
	CAAFS4Web_LP_CLIENTWANTCLS4Web			= 3,
	// CAAFS4Web������һЩԤ������(��:�û�����CAAFS4Web��ʱ��,ÿ���˿�����һ��prelogin,һ������ֻ�ܷ���һ��)
	CAAFS4Web_LP_CLIENT_CMD					= 4,

	//////////////////////////////////////////////////////////////////////////
	// LP->CAAFS4Web
	//////////////////////////////////////////////////////////////////////////
	// LP����CAAFS4Web���Թ�����
	LP_CAAFS4Web_HI							= 11,
	// LP��CAAFS4Web��һЩ���ƹ���
	LP_CAAFS4Web_CTRL						= 12,
	// LP����CAAFS4Web��LP����Ϣ�仯��(��:���������仯�ȵ�)
	LP_CAAFS4Web_LPINFO						= 13,
	// LP����CAAFS4Web�������������CLS4Web��
	// ֻҪ��Ӧ��CLS4Web������CAAFS4Webһ�����յ�������ص�(���CLS4Web����,LP������CLS4Web�û���ʱ��Ҳ����CAAFS4Web�����û����ӽ������Ϣ)
	LP_CAAFS4Web_CLIENTCANGOTOCLS4Web		= 14,

	//////////////////////////////////////////////////////////////////////////
	//CLS4Web->LP
	//////////////////////////////////////////////////////////////////////////
	// CLS4Web����LP������Լ��ĳ�ʼ��Ϣ
	CLS4Web_LP_HI							= 21,
	CLS4Web_LP_HI1							= 22,
	// CLS4Web֪ͨLP�ͻ��˿��������Լ���
	CLS4Web_LP_CLIENTCANGOTOCLS4Web			= 24,
	// CLS4Web֪ͨLP�ͻ������ӽ��
	CLS4Web_LP_CLIENTTOCLS4Web_RST			= 25,
	// CLS4Web֪ͨLP�ͻ��˶���(GMS�յ������Ϣ��ʱ���û�Ӧ���Ѿ���CLSɾ����)
	CLS4Web_LP_CLIENT_DROP					= 26,
	// һЩ���ƹ���(��֪ͨ�Լ�����ͣ����)
	CLS4Web_LP_CTRL							= 27,

	//////////////////////////////////////////////////////////////////////////
	// LP->CLS4Web
	//////////////////////////////////////////////////////////////////////////
	// LP����CLS4Web��������������
	LP_CLS4Web_HI							= 31,
	// LP����CLS4Web��Ҫ�пͻ�����������
	LP_CLS4Web_CLIENTWANTCLS4Web			= 34,

	//////////////////////////////////////////////////////////////////////////
	// CAAFS4Web->CLIENT
	//////////////////////////////////////////////////////////////////////////
	// ���ҽ���,�ͻ����ж�info����������ߵĻ��Ϳ��Լ�������
	CAAFS4Web_CLIENT_CAAFS4WebINFO			= 91,
	// ֪ͨ�û�Ŀǰ�Ŷӵ�״��
	CAAFS4Web_CLIENT_QUEUEINFO				= 92,
	// ֪ͨ�û����Կ�ʼ����CLS4Web��(�����û�CLS4Web�ĵ�ַ,�Լ���������)
	CAAFS4Web_CLIENT_GOTOCLS4Web			= 93,

	//////////////////////////////////////////////////////////////////////////
	// CLIENT->CAAFS4Web
	//////////////////////////////////////////////////////////////////////////
	CLIENT_CAAFS4Web_2LP					= 98,

	//////////////////////////////////////////////////////////////////////////
	// CLIENT->CLS4Web
	//////////////////////////////////////////////////////////////////////////
	// �ͻ��˷��͸�CLS4Web����Ϣ(��������)
	CLIENT_CLS4Web_CONNECT_EXTINFO			= 101,
	// �û�����
	CLIENT_CLS4Web_DATA						= 102,

	//////////////////////////////////////////////////////////////////////////
	// CLS4Web->CLIENT
	//////////////////////////////////////////////////////////////////////////
	// �û�����
	CLS4Web_CLIENT_DATA						= 112,

	//////////////////////////////////////////////////////////////////////////
	// CLS4Web�����������Ĺ���ָ��
	//////////////////////////////////////////////////////////////////////////
	// �ͻ��˷���������
	CLS4Web_SVR_CLIENT_DATA					= 201,

	//////////////////////////////////////////////////////////////////////////
	// CLS�յ��ķ����������Ĺ���ָ��
	//////////////////////////////////////////////////////////////////////////
	// ��������CLS��һЩ���ƹ���(��:�߳�ĳ���û�������ĳЩ��Ϣ��)
	SVR_CLS4Web_CTRL						= 210,
	// ������ұ�־
	SVR_CLS4Web_SET_TAG_TO_CLIENT			= 211,
	// ������һ��ҵ�����
	SVR_CLS4Web_CLIENT_DATA					= 213,
	// ���������ҵ�����
	SVR_CLS4Web_MULTICLIENT_DATA			= 215,
	// ����ĳ����־�������������
	SVR_CLS4Web_TAGGED_CLIENT_DATA			= 217,
	// �����������
	SVR_CLS4Web_ALL_CLIENT_DATA				= 219,
	// ����64λ��־
	SVR_CLS4Web_SET_TAG64_TO_CLIENT			= 221,
	// ����ĳ��64λ��־�������������
	SVR_CLS4Web_TAGGED64_CLIENT_DATA		= 222,
};

#pragma pack(1)
//////////////////////////////////////////////////////////////////////////
// CAAFS4Web->LP
//////////////////////////////////////////////////////////////////////////
struct CAAFS4Web_LP_HI_T 
{
	pngs_cmd_t			nCmd;
	int					nGroupID;					// CAAFS��ź���Ӧ��CLS��Ӧ
	int					nCAAFS4WebVer;				// CAAFS�汾
	bool				bReconnect;					// ��ʾ������������µ�
};
struct CAAFS4Web_LP_CLIENTWANTCLS4Web_T 
{
	pngs_cmd_t			nCmd;
	int					nClientIDInCAAFS4Web;		// �û���CAAFS4Web�е�ID
	unsigned int		IP;							// �û���IP
	unsigned int		nPassword;					// �û�����CLS4Web������
	int					nTermType;					// �ն�����
};
struct CAAFS4Web_LP_CLIENT_CMD_T 
{
	pngs_cmd_t			nCmd;
	int					nClientIDInCAAFS4Web;		// �û���CAAFS4Web�е�ID
	unsigned int		IP;							// �û�IP
	// ������������ָ�������
};
//////////////////////////////////////////////////////////////////////////
// LP->CAAFS4Web
//////////////////////////////////////////////////////////////////////////
struct LP_CAAFS4Web_HI_T
{
	enum
	{
		RST_OK						= 0,			// ���ӳɹ�
		RST_ERR_BAD_VER				= 1,			// �汾��ƥ��
		RST_ERR_BAD_GROUPID			= 2,			// group idԽ��
		RST_ERR_DUP_GROUPID			= 3,			// group id�ظ�
		RST_ERR_MEMERR				= 4,			// �ڴ�������
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRst;
	int					nLPVer;						// LP�İ汾��
};
struct LP_CAAFS4Web_CTRL_T 
{
	enum
	{
		SUBCMD_EXIT					= 0,			// �˳�
		SUBCMD_STRCMD				= 1,			// �ִ�ָ��
		SUBCMD_STRRST				= 2,			// �ִ�ָ��ķ���
		SUBCMD_CLIENTCMDNUMSUB1		= 3,			// ���û���ָ�����-1,nExt���û���nClientIDInCAAFS4Web
													// ���÷���
		SUBCMD_CLIENTQUEUETRANS		= 4,			// ת���û����Ŷ�,nExt���û���nClientIDInCAAFS4Web,nParam��Ŀ�����
													// ���÷���
	};
	pngs_cmd_t			nCmd;
	pngs_cmd_t			nSubCmd;
	int					nCAAFS4WebIdx;
	int					nExt;
	int					nParam;
};
struct LP_CAAFS4Web_LPINFO_T 
{
	pngs_cmd_t			nCmd;
	unsigned short		nLPAvailPlayer;
};
struct LP_CAAFS4Web_CLIENTCANGOTOCLS4Web_T 
{
	enum
	{
		RST_OK						= 0,			// �ɹ�,��������CLS4Web
		RST_ERR_MEM					= 1,			// ʧ��,�޷������ڴ�
		RST_ERR_CLS4Web				= 2,			// ʧ��,û�п�ȥ��CLS4Web
		RST_ERR_NOCLS4WebCANACCEPT	= 3,			// ʧ��,û�п��Խ����û���CLS��
		RST_ERR_UNKNOWN				= 255,			// ʧ��,����λ�ô���
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRst;						// ���
	port_t				nPort;						// CLS4Web�Ķ˿�
	unsigned int		IP;							// CLS4Web��IP
	int					nClientIDInCAAFS4Web;		// �û���CAAFS4Web�е�ID
	int					nClientID;					// �û���LP�е�ID,������ǽ���������Ϸ�����е�ͳһ����ID
};
//////////////////////////////////////////////////////////////////////////
// CLS4Web->LP
//////////////////////////////////////////////////////////////////////////
struct CLS4Web_LP_HI_T
{
	pngs_cmd_t			nCmd;
	int					nGroupID;					// CLS4Web���,�Ͷ�Ӧ��CAAFS4Web��Ӧ
	int					nCLS4WebVer;				// CLS4Web�汾
	int					nOldID;						// �ɵ�CLS4WebID,�����0��ʾ������
};
struct CLS4Web_LP_HI1_T 
{
	pngs_cmd_t			nCmd;
	port_t				nPort;						// ��Client�Ķ˿�
	unsigned int		IP;							// ��Client��IP
};
struct CLS4Web_LP_CLIENTCANGOTOCLS4Web_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;					// �û���LP/CLS�е�ID
};
struct CLS4Web_LP_CLIENTTOCLS4Web_RST_T 
{
	enum
	{
		RST_OK						= 0,			// ���ӳɹ�
		RST_ERR_NOTCONNECT			= 1,			// �ͻ��˸���û��������
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRst;						// �Ƿ�ɹ�
	int					nClientID;					// �û���LP�е�ID(Ҳ��CLS�е�)
};
struct CLS4Web_LP_CLIENT_DROP_T 
{
	enum
	{
		REMOVEREASON_NOTHING		= 0,
		REMOVEREASON_SENDERR		= 1,			// ��ͻ��˷�������ʱSend���ش���(�����ǻ�������)
		REMOVEREASON_CLIENTHACK		= 2,			// �ͻ��˷�����ָ����hack����
		REMOVEREASON_KICKED			= 3,			// �ͻ��˱���������ָ���߳�
		REMOVEREASON_NOTEXIST		= 4,			// �ͻ��˶�Ӧ��ID������
		REMOVEREASON_END			= 5,			// �������ÿͻ�����������
		REMOVEREASON_AS_DROP		= 6,			// 
		REMOVEREASON_CLS4Web_ERR	= 7,			// ��CLS4Web������Ϣʧ�ܵȵ�
		// ǰ���Ӧ�ò��ᳬ��100,(�������Щ����ΪClose��ɵ�)
		REMOVEREASON_CNTRCLOSE_REASON_UNKNOWN			= 100+epoll_server::close_reason_unknown,
		REMOVEREASON_CNTRCLOSE_REASON_INITIATIVE		= 100+epoll_server::close_reason_initiative,
		REMOVEREASON_CNTRCLOSE_REASON_PASSIVE			= 100+epoll_server::close_reason_passive,
		REMOVEREASON_CNTRCLOSE_REASON_CLOSE_TIMEOUT		= 100+epoll_server::close_reason_close_timeout,
		REMOVEREASON_CNTRCLOSE_REASON_DROP				= 100+epoll_server::close_reason_drop,
		REMOVEREASON_CNTRCLOSE_REASON_CONNECT_TIMEOUT	= 100+epoll_server::close_reason_connect_timeout,
		REMOVEREASON_CNTRCLOSE_REASON_ACCEPT_TIMEOUT	= 100+epoll_server::close_reason_accept_timeout,
		REMOVEREASON_CNTRCLOSE_REASON_KEY_NOT_AGREE		= 100+epoll_server::close_reason_key_not_agree,
		REMOVEREASON_CNTRCLOSE_REASON_CNTRID_NOT_MATCH	= 100+epoll_server::close_reason_cntrid_notmatch,
		REMOVEREASON_CNTRCLOSE_REASON_DATASIZE_ZERO		= 100+epoll_server::close_reason_datasize_zero,
		REMOVEREASON_CNTRCLOSE_REASON_DATASIZE_TOOBIG	= 100+epoll_server::close_reason_datasize_toobig,
		REMOVEREASON_CNTRCLOSE_REASON_DECRYPT			= 100+epoll_server::close_reason_decrypt,
		REMOVEREASON_CNTRCLOSE_REASON_ENCRYPT			= 100+epoll_server::close_reason_encrypt,
		REMOVEREASON_CNTRCLOSE_REASON_RECV				= 100+epoll_server::close_reason_recv,
		REMOVEREASON_CNTRCLOSE_REASON_SEND				= 100+epoll_server::close_reason_send,
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRemoveReason;				// �����RemovePlayer֮ǰ�����������˵���ر�ԭ��
	int					nClientID;					// �û���LP�е�ID(Ҳ��CLS�е�)
};
struct CLS4Web_LP_CTRL_T 
{
	enum
	{
		SUBCMD_EXIT					= 0,			// �˳�(����LP�Լ������˳���)
	};
	pngs_cmd_t			nCmd;
	pngs_cmd_t			nSubCmd;
	int					nParam;
};
//////////////////////////////////////////////////////////////////////////
// LP->CLS4Web
//////////////////////////////////////////////////////////////////////////
struct LP_CLS4Web_HI_T 
{
	enum
	{
		RST_OK						= 0x00,			// ���ӳɹ�
		RST_ERR_BAD_VER				= 0x01,			// �汾����
		RST_ERR_BAD_GROUPID			= 0x02,			// groupid����
		RST_ERR_TOOMANY_CLS4Web		= 0x03,			// ���groupid��CLS4Web̫����
		RST_ERR_MEMERR				= 0x04,			// �ڴ�������
		RST_ERR_UNKNOWN				= 0xFF,			// δ֪����
	};
	pngs_cmd_t			nCmd;
	unsigned char		nRst;
	int					nCLS4WebID;
	int					nLPMaxPlayer;
	int					nCLS4WebMaxConnection;
	int					nLPVer;
};
struct LP_CLS4Web_CLIENTWANTCLS4Web_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;					// �û���LP�е�ID(ʵ����Ҳ������CLS4Web�е�ID)
	unsigned int		nPassword;					// ��������(CAAFS4Web����LP,LP��ת��������)
	int					nTermType;					// �ն�����
};
//////////////////////////////////////////////////////////////////////////
// CAAFS4Web->CLIENT
//////////////////////////////////////////////////////////////////////////
struct CAAFS4Web_CLIENT_CAAFS4WebINFO_T 
{
	enum
	{
		SELF_NOTIFY_REFUSEALL		= 0x00000001,	// �ܾ����е�¼
	};
	pngs_cmd_t			nCmd;
	unsigned char		nVerCmpMode;				// �汾�ȽϷ�ʽ
	char				szVer[PNGS_VER_LEN];		// �汾
	unsigned short		nQueueSize;					// ǰ���Ŷӵ�����(����һ�����Ͳ�Ҫ���û�����)
	unsigned int		nSelfNotify;				// 32bit��ʾ32����˼
	int					nInfoLen;					// info�ĳ���
	char				info[1];					// �����ִ�
};
struct CAAFS4Web_CLIENT_QUEUEINFO_T 
{
	pngs_cmd_t			nCmd;
	unsigned char		nChannel;					// ��Ӧ��ͨ��
	unsigned char		nQueueSize;					// ǰ���Ŷӵ�����
};
struct CAAFS4Web_CLIENT_GOTOCLS4Web_T 
{
	pngs_cmd_t			nCmd;
	port_t				nPort;						// CLS4Web�˿�
	unsigned int		IP;							// CLS4Web��ַIP
	unsigned int		nPassword;					// ��������
	int					nClientID;					// ���ID
};
//////////////////////////////////////////////////////////////////////////
// CLIENT->CAAFS4Web->LP
//////////////////////////////////////////////////////////////////////////
struct CLIENT_CAAFS4Web_2LP_T 
{
	pngs_cmd_t			nCmd;
	char				data[1];					// ��������loginһ�������ݰ�,����������߼�����
};
struct CLIENT_CAAFS4Web_EXTINFO_T 
{
	int					nTermType;					// �ն�����
};
//////////////////////////////////////////////////////////////////////////
// CLIENT->CLS4Web
//////////////////////////////////////////////////////////////////////////
struct CLIENT_CLS4Web_EXTINFO_T 
{
	int					nClientID;					// ���ID
	unsigned int		nPassword;					// ��������CLS4Web������
};
struct CLIENT_CLS4Web_DATA_T 
{
	pngs_cmd_t			nCmd;
	char				data[1];
};
//////////////////////////////////////////////////////////////////////////
// CLS4Web->CLIENT
//////////////////////////////////////////////////////////////////////////
// �û�����
struct CLS4Web_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	char				data[1];
};
//////////////////////////////////////////////////////////////////////////
// CLS4Web����������͵Ĺ���ָ��
//////////////////////////////////////////////////////////////////////////
struct CLS4Web_SVR_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;
	char				data[1];					// ����
};
//////////////////////////////////////////////////////////////////////////
// CLS4Web�յ��ķ����������Ĺ���ָ��
//////////////////////////////////////////////////////////////////////////
struct SVR_CLS4Web_CTRL_T 
{
	enum
	{
		SUBCMD_EXIT						= 0,			// �˳�,nParam������
		SUBCMD_STRCMD					= 1,			// �ִ�ָ��,nParam���׵�ַ������Ϊ�ִ��׵�ַ
		SUBCMD_STRRST					= 2,			// �ִ�ָ��ķ���,nParam���׵�ַ������Ϊ�ִ��׵�ַ
		SUBCMD_KICKPLAYERBYID			= 3,			// ����ID�߳�ĳ���û�,nParamΪPlayerID
		SUBCMD_CHECKPLAYEROFFLINE		= 6,			// ����Ƿ����û��Ѿ�������(���ֻ��LP�ᷢ��,CLS4Web������ID�Ƿ����,��������ھ�������LP�����û�������Ϣ),nParam���û�ID����,�������int����
		SUBCMD_KICKPLAYERBYID_AS_DROP	= 7,			// ��SUBCMD_KICKPLAYERBYIDһ��,��������LP�����û���DROP�Ľ��
		SUBCMD_KICKPLAYERBYID_AS_END	= 8,			// ��SUBCMD_KICKPLAYERBYIDһ��,��������LP�����û��������˳��Ľ��
		SUBCMD_SET_NAME					= 9,			// �����û����ִ�,��Ҫ���ڼ�¼��־,nParamΪ�û�ID,��������ִ�
		SUBCMD_PLAYER_STAT_OFF			= 10,			// �ر�ͳ��,nParamΪClientID
		SUBCMD_PLAYER_STAT_ON			= 11,			// ����ͳ��,nParamΪClientID,���滹��һ��������ʾ���ͼ��
		SUBCMD_KICK_ALL_PLAYER			= 12,			// ���������������
	};
	pngs_cmd_t			nCmd;
	pngs_cmd_t			nSubCmd;
	int					nParam;
};
struct SVR_CLS4Web_SET_TAG_TO_CLIENT_T 
{
	pngs_cmd_t			nCmd;
	unsigned char		nTagIdx;
	short				nTagVal;
	int					nClientID;
};
struct SVR_CLS4Web_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;						// ���ID
	char				data[1];						// ����
};
struct SVR_CLS4Web_MULTICLIENT_DATA_T
{
	pngs_cmd_t			nCmd;
	unsigned short		nClientNum;
	int					anClientID[1];
	// ���֮�������ʵ����
	int		GetDSize(int nTotalSize)
	{
		return nTotalSize - wh_offsetof(SVR_CLS4Web_MULTICLIENT_DATA_T, anClientID) - nClientNum*sizeof(int);
	}
	void*	GetDataPtr()
	{
		return anClientID + nClientNum;
	}
	static int	GetTotalSize(int nClientIDNum, int nDSize)
	{
		return wh_offsetof(SVR_CLS4Web_MULTICLIENT_DATA_T, anClientID) + nClientIDNum*sizeof(int) + nDSize;
	}
};
struct SVR_CLS4Web_TAGGED_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	unsigned char		nTagIdx;
	short				nTagVal;
	char				data[1];
};
struct SVR_CLS4Web_ALL_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	char				data[1];
};
struct SVR_CLS4Web_SET_TAG64_TO_CLIENT_T 
{
	pngs_cmd_t			nCmd;
	int					nClientID;
	whuint64			nTag;
	bool				bDel;
};
struct SVR_CLS4Web_TAGGED64_CLIENT_DATA_T 
{
	pngs_cmd_t			nCmd;
	whuint64			nTag;
	char				data[1];
};
#pragma pack()
}

#endif
