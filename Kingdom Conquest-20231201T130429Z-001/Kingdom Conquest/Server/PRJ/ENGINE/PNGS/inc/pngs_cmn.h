// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_pngs
// File         : pngs_cmn.h
// Creator      : Wei Hua (κ��)
// Comment      : PNGS���������ͨ�ù���ģ��(GMS��GZS���������������ܣ����߿������Ϊ���е��߼�����������������������)
//                PNGS��Pixel Network Game Structure����д
//                ����Logic��Interface��Type��Ҫ�ھ���Ӧ���о��嶨�壬���һ��Ҫ���壬�ſ��Ա�֤���ظ���
//                framecontrol�ڸ������һ������
//                ILogicģ�������DLL��ʵ�֣����������ѭ�����׼��(����XXX����ģ�������)��
//                1. ���������DLLӦ�ý�XXX.so��XXX_d.so
//                2. ��������ĺ���Ӧ�ý�XXX_Create
//                3. m_szLogicType�����淶:
//                3.1. ����ֻ����Сд��ĸ��ͷ���м��������ĸ�����֡�����(��ЩҪ����Ҫ��Ϊ���ܷ���غ��ļ��ܹ�һ�£���Ȼ��һ��Ҳû��ϵ)
//                3.2. ���ֵĵ�һ��Сд��ĸ��ʾ�����Logic������
//                3.2.1. 'u'��ʾ������ֲ����ظ�
//                3.2.2. 'm'��ʾ������ֿ����ظ�
//                Ŀǰ�Ѿ�ʹ�õ������У����μ�tty_common_def.h��pngs_def.h����ĺ꣩
//                uGMS_MAINSTRUCTURE	GMS�����
//                uGMS_GAMEPLAY			GMS��Ϸ�������
//                uGS_GMTOOLSINTERFACE	��������GMTOOLS�ӿ�
//                uGS_DB				���ݿ�
//                uGZS_MAINSTRUCTURE	GZS�����
//                uGZS_GAMEPLAY			GZS��Ϸ�������
//                uGS_CHAT				������صķ�����ģ��ӿ�
//                uGC_CHAT				������صĿͻ���ģ��ӿ�
//                4. ���ڼ��ݶ��̵߳�Logicģ���ָ���
//                ԭ���ǣ�������Ƕ��߳̾�ֱ�Ӵ�������ָ�롣����Ƕ��߳̾ͷ���ָ����С�
//                ���������ָ����Ҫ�����������ݵģ�����������ָ��ĸ�ʽһ�£��������ݶ���ָ�����á�����Ƕ��߳̾Ͱ����ݸ����ں��档
//                ��������ָ�������涼��int nSize; void *pData; // ע�ⲻҪ��const void *����Ϊ����Ҫ�������ݷ���
//                �������Ҫ������еľ��ں��渽�����ݵĲ��֣�pDataȻ��ָ��������֡�
/*
����Logicģ�����бȽϴ���ָ��ݡ�һ���÷��������ġ�
if( IsSameThread(m_pLogicGamePlay) )
{
	GMSPACKET_2GP_SVRCMD_T	Cmd2GP;
	...
	Cmd2GP.nDSize			= nDSize;
	Cmd2GP.pData			= pGameCmd->data;
	m_pLogicGamePlay->CmdIn_NMT(this, GMSPACKET_2GP_SVRCMD, &Cmd2GP, sizeof(Cmd2GP));
}
else
{
	// �ӶԷ������ڴ�
	GMSPACKET_2GP_SVRCMD_T	*pCmd2GP;
	if( m_pLogicGamePlay->CmdIn_MT_lock(this, GMSPACKET_2GP_SVRCMD, (void **)&pCmd2GP, nSize + sizeof(*pCmd2GP))<0 )
	{
		return;
	}
	...
	pCmd2GP->nDSize			= nDSize;
	pCmd2GP->pData			= pCmd2GP + 1;
	if( nDSize>0 )
	{
		memcpy(pCmd2GP->pData, pGameCmd->data, nDSize);
	}
	m_pLogicGamePlay->CmdIn_MT_unlock();
}
��������д��Cmd2GP֮�����
	CMN_LOGIC_CMDIN_AUTO(this, m_pLogicGamePlay, GMSPACKET_2GP_SVRCMD, &Cmd2GP, sizeof(Cmd2GP));
���ڽ�С��ָ��ݣ�����ֱ������ʹ��
	CmdOutToLogic_AUTO(m_pLogicGamePlay, GMSPACKET_2GP_GZSOFFLINE, &GZSOL, sizeof(GZSOL));
*/
// ע�⣺����Logic����Ҫͨ��GLOGGER2_WRITEFMT�������д��־����Ϊ���Logic��DLL����GLOGGER2_WRITEFMT��û�г�ʼ����
// CreationDate : 2005-07-25
// Change LOG   : 2005-09-07 ��CMNת��Ϊ�ӿڣ���ΪILogic�ڲ���ʹ��CMN�ķ��������CMN���ǽӿڣ���ILogicʵ�ֵĲ��־���Ҫ����PNGS���̣��������á�
//                2005-09-09 ��Logic���͸�Ϊ�ִ����ͣ������Ͳ���Ҫ�ڹ�����ͷ�ļ��н��ж����ˣ�֮��Ҫ�д��Լ�����ִ����ļ�����
//                2005-10-12 ��CmdIn����д�ڻ����ͬʱ���Ƕ��̺߳ͷǶ����ƵĴ��ݷ�ʽ
//                2006-03-20 ������ILogic������鷽��Detach()�������ڶ���������ǰ���á�
//                2006-08-15 ������ILogic::Detach��bQuick�����������ڳ�ʼ�������ʱ�������������Ͳ������tick�����¿��ܵĶ������ô����ˡ�
//                2006-08-22 �������ļ����������úͻ�ȡ���������ͨ��GetFileMan()����ļ���������Ȼ������ļ���صĲ�����
//                2006-01-23 ��TickLogic��ΪAllTick��AllSureSend��Ϊ��CMNBody����Ҳ���Ա����ڱ��ģ���ڵ�ʹ�ã����ڸ�ģ�������Ӳ�����ܣ�
//                2006-12-01 ��m_szLogicType��CMN::ILogic�Ƶ�ILogicBase���ˡ���ΪILogicBase��������Ҫ��ȡ���ֵġ�

#ifndef	__PNGS_CMN_H__
#define	__PNGS_CMN_H__

#include <WHCMN/inc/whvector.h>
#include <WHCMN/inc/whqueue.h>
#include <WHCMN/inc/whfile_base.h>
#include <WHCMN/inc/whallocmgr2.h>
#include <WHCMN/inc/whthread.h>
#include <WHNET/inc/whnetcmn.h>
#include <WHNET/inc/whnetcnl2.h>								// �����pngs_config_CLSClient_Slot_Channel��Ҫ
#include "pngs_def.h"

namespace n_pngs
{

class	ILogicBase
{
public:
	typedef	int							cmd_t;						// �ڴ�䴫�ݣ�cmd_t���Դ�һЩ
	typedef	n_whcmn::whallocationobjwithonemem<>	AO_T;			// ���һ����ڴ�ķ�����
	typedef	n_whcmn::whallocmgr2<AO_T>				AM_T;
	// ������ڲ������д洢ָ��ͷ��Ϣ��ʱ����
	struct	CMDHDR_T
	{
		ILogicBase	*pRstAccepter;
		cmd_t		nCmd;
	};
private:
	// �̱߳�ʶ(�����ʶһ����˵������Logic��һ���߳���)
	// ����ϲ㲻����������Logic����Ĭ����0
	int						m_nThreadTag;
	// ����ָ�����
	n_whcmn::whsmpqueueWL	m_queueCmdIn;
	// �ļ�������ָ��
	n_whcmn::whfileman		*m_pFM;
protected:
	char					m_szLogicType[PNGS_LOGICNAME_LEN];		// ����ڼ̳�����Ӧ����д��(���Ӧ����Լ����Щ�����ظ���Щ�����ظ��ģ�������Ĭ�Ͽ����ظ�)
	int						m_nVer;									// ���Ҳ�ڼ̳�������ã�������-1���ʾ���ü��汾����Ĭ��д0���ʾ����̳����в���д��ô���Ҫ����Σ�
	// ����Type�������淶��ο�Comment�е�3
	// ����Ƿ�ֹͣtick
	bool					m_bStopTick;
protected:
	// �������������Init��Release�е���
	int		QueueInit(int nQueueSize);
	int		QueueRelease();
	// ����������Tick�е���
	int		DealCmdIn();
public:
	ILogicBase()
		: m_nThreadTag(0)
		, m_pFM(NULL)
		, m_nVer(0)													// �����д-1��ʾ����У��VER��Ĭ��д0���ʾ����̳����в���д��ô���Ҫ����Σ�
		, m_bStopTick(false)
	{
		m_szLogicType[0]	= 0;									// ��ʾĬ��ģ�鲻��Ҫ����
	}
	// ����߼�������
	inline	const char *	GetLogicType() const
	{
		return	m_szLogicType;
	}
	// ��ð汾
	inline int	GetVer() const
	{
		return	m_nVer;
	}
	virtual	~ILogicBase()	{};
	// ����̱߳�ʶ
	inline int	GetThreadTag() const
	{
		return	m_nThreadTag;
	}
	// ����ϲ������MT�����ǿ�ư����е�Tag����һ�µ�
	inline void	SetThreadTag(int nTag)
	{
		m_nThreadTag	= nTag;
	}
	// �ж��Լ��Ƿ�ͶԷ���һ���߳�
	// ���Ϊ����˵������MT��
	inline bool	IsSameThread(ILogicBase *pOther) const
	{
		return	m_nThreadTag == pOther->GetThreadTag();
	}
	// �����ļ�������
	inline void	SetFileMan(n_whcmn::whfileman *pFM)
	{
		m_pFM	= pFM;
	}
	// ����ļ�������
	inline n_whcmn::whfileman *	GetFileMan()
	{
		return	m_pFM;
	}
	// stoptick���
	void	SetStopTick(bool bSet);
	// ����ָ�����з�����pRstAccepter�ǿ�����Է��ظ�pRstAccepter
	// ILogicBase�����з���������CmdIn����Ӧ������ͬһ���߳��ڵ��õģ�����һ��������ILogicBase��������һ���̣߳�����һ��LogicӦ��ֻ����һ���̣߳�
	// ��Ҫ����ִ�е�ָ��(�������MT��NMT)
	virtual int		CmdIn_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
	{
		return	DealCmdIn_One_Instant(pRstAccepter, nCmd, pData, nDSize);
	}
	// (���÷���Ӧ���Ѿ��ж��������Ƿ���һ���߳�����)
	// �Ƕ��̵߳��õ�ָ��
	inline int		CmdIn_NMT(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
	{
		return	DealCmdIn_One(pRstAccepter, nCmd, pData, nDSize);
	}
	// ���̵߳��õ�ָ��
	int		CmdIn_MT(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	// ���벢����һ���ڴ�
	int		CmdIn_MT_lock(ILogicBase *pRstAccepter, cmd_t nCmd, void **ppData, int nDSize);
	// �����ո�lock���ڴ�
	int		CmdIn_MT_unlock();

	// ��Է��߼����ͼ򵥵�ָ��Զ��ж��Ƿ���ҪMT��ʽ����
	inline int	CmdOutToLogic_AUTO(ILogicBase *pDstLogic, cmd_t nCmd, const void *pData, int nDSize)
	{
		if( pDstLogic )
		{
			if( IsSameThread(pDstLogic) )
			{
				return	pDstLogic->CmdIn_NMT(this, nCmd, pData, nDSize);
			}
			else
			{
				return	pDstLogic->CmdIn_MT(this, nCmd, pData, nDSize);
			}
		}
		else
		{
			return	-1;
		}
	}
private:
	// ��������ִ�еĵ���ָ���CmdIn_Instant�е��õģ�
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
																	= 0;
	// ������ָ��
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize)
																	= 0;
};

class	CMN			: public ILogicBase
{
public:
	// ��Ϸ�߼�ģ��
	class	ILogic	: public ILogicBase
	{
	public:
		enum	MOOD_T
		{
			MOOD_WORKING		= 0,								// ����������ģʽ�����һ���ǿ�ʼ���Զ��ɹ�����ģʽ�ģ�����������ã�
			MOOD_STOPPING		= 1,								// ���ڽ��������У�������ã�
			MOOD_STOPPED		= 2,								// ���ڽ�����ϵ�״̬���ڲ���������˾��Լ����ý�����
		};
	protected:
		friend class CMNBody;
		// �⼸���������Ǹ�CMN�õ�
		MOOD_T	m_nMood;
		int		m_nPos4CMN;											// CMN���Logic��CMN�е������±���д������
		CMN		*m_pCMN;											// CMN����Լ���thisָ�������
		bool	m_bIsInitOver;										// �����ϲ�Init�Ƿ�����ˣ�����GSDB�ʼ��clean���̣�����������������������������ģ�鿪ʼ��ʼ����
	protected:
		// ���漸��protected����Ҳֻ��CMNʹ��
		// �߼��ڲ���֯(�����CMN�������߼������ӣ����������logic����attach֮g����CMNͳһ����)
		// ���Ҳ���������о��߼���detach�������µ��߼���attach֮����е�
		// ����ÿ���߼�Ӧ���ڲ��Լ���¼���Լ��Ƿ��ǵ�һ�α�Organize��
		// ��������Logic����������ҲӦ��������֯һ��(�����Ҫ��Ϊ�˶�̬�β�Logic������)���ϲ㻹��Ҫ��֤��û������Organize֮ǰ���ܵ����κ��߼�tick(���������¼���ϵ�Logicָ����ܻᵼ���ڴ����)
		// ���԰�Organize�Ĺ���ֻ�޶��ڻ�ȡ����ģ���ָ��
		virtual	int		Organize()									= 0;
		// Detach��������ILogic��������֮ǰ������õģ���Ϊ��Щ���ٵĲ�����Ҫ���������������Ա�����ǰ����
		// bQuick����˼�ǲ��õȴ�����Ϊ������detach���ܻ�ȴ�Ȼ��������tick�Ա����߼��������꣩
		virtual	int		Detach(bool bQuick)							{return 0;}
	protected:
		ILogic()
			: m_nMood(MOOD_WORKING)
			, m_nPos4CMN(-1), m_pCMN(NULL)
			, m_bIsInitOver(true)
		{
		}
	public:
		// ��������
		virtual	void	SelfDestroy()								= 0;
		// ��ʼ��
		virtual	int		Init(const char *cszCFGName)				= 0;
		// ������������
		virtual	int		Init_CFG_Reload(const char *cszCFGName)		= 0;
		// �����ϲ�Init�Ƿ�����������
		inline bool		IsInitOver() const							{return m_bIsInitOver;}
		// �ս�
		virtual	int		Release()									= 0;
		// ��ù���ģʽ
		inline MOOD_T	GetWorkMood() const							{return m_nMood;}

		// ��ÿɱ������select��SOCKET��append��vector��β��
		virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect)	{return 0;}
		// ��֤�÷��͵������ܹ�����������(ע�⣺Tick�����кܿ���û��Send����)
		virtual	int		SureSend()									{return 0;}
		// ����
		// ����һ��(�����ٵ��������Tick_BeforeDealCmdIn()��Tick_AfterDealCmdIn())
		int		Tick();
	private:
		// ������Ҫʵ�ֵ�
		virtual int		Tick_BeforeDealCmdIn()						= 0;
		virtual int		Tick_AfterDealCmdIn()						= 0;
	};
protected:
	CMN();
public:
	// ����־����ָ�룩Ϊ������Logic�ṩд��־�Ĺ���
	int		(*m_fn_GLogger_WriteFmt)(int nID, const char *szFmt, ...);

	// CMN���ܴ����cmd�����Ҳ��cmd_t���͵�
	// �����͵�
	enum
	{
		CMD2CMN_INSTANT_REGISTERCMDDEAL		= 1,					// ��ĿǰӦ���Ǿ�������ָ��ּ����ܵ�����ܻ��յ��������PNGSClient����ע��һ��pngs_cmd_tָ���Ӧ��ָ����߼�ģ��
																	// pRstAccepter���ǿ��Դ���ָ����߼�ģ��
	};
	// ��ͨ��
	enum
	{
		CMD2CMN_SHOULDSTOP					= 1,					// �κ�һ��ģ�鶼����������ܷ��������������������ֹͣ���У����һ������ģ�鷢�����ش����ʱ��
																	// һ�㷢�ͷ�ʽΪ CmdOutToLogic_AUTO(m_pCMN, CMN::CMD2CMN_SHOULDSTOP, NULL, 0);
		CMD2CMN_STOPNOW						= 2,					// �κ�һ��ģ�鶼����������ܷ������������������������ֹͣ���У���ȫ�������κ��ͷŹ���
																	// һ�㷢�ͷ�ʽΪ CmdOutToLogic_AUTO(m_pCMN, CMN::CMD2CMN_STOPNOW, NULL, 0);
	};
	// ������Ҫ��ָ��ṹ
	#pragma pack(push, old_pack_num, 1)
	struct	CMD2CMN_INSTANT_REGISTERCMDDEAL_T
	{
		pngs_cmd_t		nCmdReg;
	};
	#pragma pack(pop, old_pack_num)

	// ����һ���ٵ�ILogic
	static ILogic *		CreateDummyLogic(const char *cszLogicName);

	// ������Ҫ���õĹ��ܣ�ע�⣺��Ҫȡ���ˣ�ԭ�������ͱ�������! �������ȶ���
	virtual	ILogic *	GetLogic(const char *cszType, int nVer)		= 0;
	virtual	int			GetMultiLogic(const char *cszType, int nVer, n_whcmn::whvector<ILogic *> &vect)
																	= 0;

	// ���Դ���ILogic�����ͨ�ú���ָ������
	typedef	n_pngs::CMN::ILogic *	(*fn_ILOGIC_CREATE_T)();
};

class	CMNBody	: public CMN
{
private:
	bool	m_bThreadOn;											// �Ƿ������߳�ģʽ
	bool	m_bShouldStop;											// �յ�ģ�鷢����CMD2CMN_SHOULDSTOPʱ�ᱻ����Ϊtrue���������ѭ����⵽�����־�ͻ�������������ͣ���ˡ�
	n_whcmn::whvector<ILogic *>		m_vectLogic;					// Ĭ��reserve 16��λ��
	n_whcmn::whvector<SOCKET>		m_vectSOCKET;					// Ĭ��reserve PNGS_MAX_SOCKETTOSELECT��λ�ã����Ҳ�resize��
	struct	MY_THEADINFO_T
	{
		n_whcmn::whtid_t		tid;
		n_whcmn::whlock			*pLock;
	};
	n_whcmn::whvector<MY_THEADINFO_T>	m_vectThread;				// Ĭ��reserve 16��λ��
public:
	CMNBody();
	~CMNBody();
	// �����߼�ģ��(ע�⣺��Ҫ��AttachLogic��Ȼ���ٵ���Logic��Init��������ΪInit��Ҳ����Ҫ�õ�д��־�������Ҫ����CMN�ĳ�Ա��ʵ��)
	int		AttachLogic(ILogic *pLogic);
	// �߼�֮����໥����(����������������߼���attach���֮����)
	// ����0��ʾ�ɹ�������-i��ʾ��i��logic��������
	int		OrganizeLogic();
	// ��һ������SOCKET��select����
	// ����cmn_select���ܷ��صĽ��
	int		DoSelect(int nMS, int nThreadTag=0);
	// ����ϲ�ϣ����ͳһ��select����ô�ϲ�͵�������ĺ������seocket���Ͳ��õ���DoSelect��
	// ����vect�ĳߴ磨�����õ�socket��append��vectβ���ģ��������ʼʹ��֮ǰ����Ҫ�Ȱ�vect��գ�
	size_t	GetSockets(n_whcmn::whvector<SOCKET> &vect, int nThreadTag=0);
	// �����߼�����һ��(CMN�Լ��Ĺ���CMN����д������Ҳ��Ϊһ��logic)
	// ������ø������߼�ģ���Tick��SureSend
	int		AllTick(int nThreadTag=0);
	// ������ø������߼�ģ���SureSend
	int		AllSureSend(int nThreadTag=0);
	// ����߼�ģ��(���nType����AttachLogicʱ�Ĳ���nType)
	ILogic *	GetLogic(const char *cszType, int nVer);
	int			GetMultiLogic(const char *cszType, int nVer, n_whcmn::whvector<ILogic *> &vect);

	// ����������������
	int		ReloadAllCFG(const char *cszCFG);

	// �ж��Ƿ�Ӧ��ֹͣ�������ڲ�ԭ����Ҫֹͣ��
	bool	ShouldStop() const;
	// ��������Logic������ֹͣ״̬
	void	SetAllLogicToStopMood();
	// �ж��Ƿ�����Logic���Ѿ������Stop��������������ֹͣ��
	bool	AreAllLogicsReallyStopped();
	// �������е�Logic��ɾ�����е�Logic(���bDestroyΪ��Ļ�)
	// �����bQuick�Ļ��Ͳ����ڽ�����ʱ�����tick��
	int		DetachAllLogic(bool bDestroy, bool bQuick=false);
	// ���û��ֹͣ��logic�ı���ִ�
	const char *	PrintLogicNotStopped();

	/////////////////////
	// �߳����
	/////////////////////
	int		StartThreads();
	int		StopThreads();
	// ��ͣ����thread
	int		LockThreads();
	// �ָ�����thread
	int		UnlockThreads();

private:
	virtual int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
};

// �汾�Ƚϣ����Ƿ�����
enum
{
	PNGS_VERCMP_MODE_EQUAL			= 0,							// ������ȫ��Ȳ���ƥ��
	PNGS_VERCMP_MODE_GE				= 1,							// ������ڵ��ڻ�׼����ƥ��
};
bool	pngs_isverallowed(int nMode, int nBaseVerMain, int nBaseVerSub, int nInputVerMain, int nInputVerSub);
bool	pngs_isverallowed(int nMode, const char * szBaseVer, const char * szInputVer);

// ����CLS��Client��slot�Ĺ̶�����
int		pngs_config_CLSClient_Slot_Channel(n_whnet::CNL2SlotMan::INFO_T *pInfo, int nInOutNum, bool bIsClient);

// ����ILogicģ���DLL��������һ��ָ��
int		pngs_load_logic_dll_and_send_cmd(const char *cszDLLFile, const char *cszCreateFuncName, CMN::cmd_t nCmd, const void *pData, size_t nDSize);
// ����ILogicģ���DLL���������汾��ʷ��ѯ��ָ��ָ��
int		pngs_load_logic_dll_and_query_history(const char *cszDLLFile, const char *cszCreateFuncName, const char *cszVer);

// ��Logic�ڲ�ʹ�ã���Ŀ��Logic����ָ��ڲ��Զ������Ƿ���ͬһ�̣߳��Ա���ò�ͬ�ķ�ʽ���������ݣ�
// _Ty�ṹ����������
//	int				nDSize;
//	void			*pData;	// �� char *pDataҲ�У���Ҫ���ڴ����ִ���������ǰ��constҲ�С�
//	���ڼ�¼�������ݵĳ��Ⱥ���ʱָ��
template<typename _Ty>
inline int	CMN_LOGIC_CMDIN_AUTO(ILogicBase *pLThis, ILogicBase *pLDst, CMN::cmd_t nCmd, _Ty &data)
{
	if( pLDst )
	{
		if( pLThis->IsSameThread(pLDst) || data.nDSize==0 )	// �����ͬһ���߳��ڻ���û�и�������
		{
			pLDst->CmdIn_NMT(pLThis, nCmd, &data, sizeof(data));
		}
		else
		{
			// �ӶԷ������ڴ�
			_Ty	*pCmd2GP;
			if( pLDst->CmdIn_MT_lock(pLThis, nCmd, (void **)&pCmd2GP, data.nDSize + sizeof(data))<0 )
			{
				return	-1;
			}
			// ����ȫ�����ݹ�ȥ
			memcpy(pCmd2GP, &data, sizeof(data));
			// ��������pDataָ��ĵ�ַ
			wh_settypedptr(pCmd2GP->pData, pCmd2GP + 1);
			// �������ݲ���
			memcpy((void *)pCmd2GP->pData, data.pData, data.nDSize);
			pLDst->CmdIn_MT_unlock();
		}
		return	0;
	}
	else
	{
		return	-1;
	}
}

}		// EOF namespace n_pngs

#endif	// EOF __PNGS_CMN_H__
