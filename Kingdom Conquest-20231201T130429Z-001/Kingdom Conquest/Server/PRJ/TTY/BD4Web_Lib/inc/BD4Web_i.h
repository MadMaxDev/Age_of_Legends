//////////////////////////////////////////////////////////////////////////
// BD->Bussiness Dealer,ҵ������ģ��
// �����ṩweb�ӿڸ���Ӫ��,��Ϣ��ʹ�õĶ��������ֽ���
//////////////////////////////////////////////////////////////////////////
#ifndef __BD4Web_i_H__
#define __BD4Web_i_H__

#include "WHCMN/inc/whdataini.h"
#include "WHCMN/inc/whcmd.h"
#include "WHCMN/inc/whtime.h"

#include "WHNET/inc/whnetudpGLogger.h"
#include "WHNET/inc/whconnecter.h"
#include "WHNET/inc/whnettcp.h"

#include "PNGS/inc/pngs_cmn.h"
#include "PNGS/inc/pngs_packet_logic.h"

#include "../../Common/inc/tty_common_BD4Web.h"
#include "../../Common/inc/tty_def_gs4web.h"
#include "../../Common/inc/pngs_packet_web_extension.h"

using namespace n_whcmn;
using namespace n_whnet;

namespace n_pngs
{
class BD4Web_i : public CMN::ILogic
{
// ILogic�ӿ�
protected:
	virtual	int		Organize();
	virtual	int		Detach(bool bQuick);
public:
	virtual	void	SelfDestroy()	{delete this;}
	virtual	int		Init(const char *cszCFGName);
	virtual	int		Init_CFG_Reload(const char *cszCFGName);
	virtual	int		Release();
	virtual	int		GetSockets(n_whcmn::whvector<SOCKET> &vect);
	virtual	int		SureSend();
private:
	virtual	int		DealCmdIn_One_Instant(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		DealCmdIn_One(ILogicBase *pRstAccepter, cmd_t nCmd, const void *pData, int nDSize);
	virtual	int		Tick_BeforeDealCmdIn();
	virtual	int		Tick_AfterDealCmdIn();
// ���Ƶ�tcp���ӷ�����
public:
	class MyCntrSvr : public TCPConnecterServer
	{
	public:
		BD4Web_i*			m_pHost;
	public:
		class MyCntr : public TCPConnecterMsger<tcpmsgerbase>
		{
		public:
			BD4Web_i*		m_pHost;
			sockaddr_in		m_addr;
			typedef int		(BD4Web_i::*Cntr_DealMsg_T)(MyCntr* pCntr, const BD_CMD_BASE_T* pData, size_t nSize);
			Cntr_DealMsg_T	m_p_Cntr_DealMsg;
		public:
			MyCntr(BD4Web_i* pHost)
			: m_pHost(pHost)
			, m_p_Cntr_DealMsg(&BD4Web_i::Cntr_DealMsg)
			{
			}
			void	clear()
			{
				m_pHost		= NULL;
			}
			//////////////////////////////////////////////////////////////////////////
			// TCPConnecterMsger
			//////////////////////////////////////////////////////////////////////////
			// ���ں�accepter����
			virtual	void*	QueryInterface(const char* cszIName)
			{
				return this;
			}
			// �Ƿ��пմ�����Ϣ
			virtual	bool	CanDealMsg() const
			{
				return m_pHost->Cntr_CanDealMsg();
			}
			// ��Ϣ����
			virtual	int		DealMsg(const void *pData, size_t nSize)
			{
				return (m_pHost->*m_p_Cntr_DealMsg)(this, (const BD_CMD_BASE_T*)pData, nSize);
			}
			//////////////////////////////////////////////////////////////////////////
			// tcpmsgerbase
			//////////////////////////////////////////////////////////////////////////
			virtual	int		CheckUnit(const void *pBegin, size_t nSize, size_t *pnUnitLen)
			{
				if (nSize < MIN_BD_CMD_SIZE)
				{
					return CHECKUNIT_RST_NA;
				}
				*pnUnitLen		= 0;
				BD_CMD_BASE_T*	pBaseCmd	= (BD_CMD_BASE_T*)pBegin;
				bd_size_t		nPacketSize	= 0;
// 				if (m_pHost->m_cfginfo.bNBO)
// 				{
// 					nPacketSize	= ntohl(pBaseCmd->nSize);
// 				}
// 				else
				{
					nPacketSize	= pBaseCmd->nSize;
				}
				if (nPacketSize > MAX_BD_CMD_SIZE)
				{
					return CHECKUNIT_RST_ERR;
				}
				if (nPacketSize <= nSize)
				{
					*pnUnitLen	= nPacketSize;
					return CHECKUNIT_RST_OK;
				}
				return CHECKUNIT_RST_NA;
			}
			virtual size_t	GetPackUnitSize(size_t nDSize)
			{
				return nDSize;
			}
			virtual	int		PackUnit(const void *pRawData, size_t nDSize, void *pDstBuf)
			{
				memcpy(pDstBuf, pRawData, nDSize);
				return 0;
			}
			virtual	const void*	GetDataPtrInPackUnit(const void *pPackedData, size_t nTotalSize, size_t *pnDSize)
			{
				*pnDSize	= nTotalSize;
				return pPackedData;
			}
		};
	private:
		// ΪTCPConnecterServerʵ�ֵ�
		virtual	Connecter*	NewConnecter(SOCKET sock, sockaddr_in* pAddr)
		{
			return m_pHost->NewConnecter(sock, pAddr);
		}
		virtual	void		BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T* pCntrInfo)
		{
			m_pHost->BeforeDeleteConnecter(nCntrID, pCntrInfo);
		}
		virtual	void		AfterAddConnecter(Connecter* pCntr)
		{
			m_pHost->AfterAddConnecter(pCntr);
		}
	public:
		MyCntrSvr()
		: m_pHost(NULL)
		{

		}
	};
public:
	struct CFGINFO_T : public whdataini::obj 
	{
		bool		bNBO;				// net byte order,�Ƿ�ʹ�������ֽ���
		MyCntrSvr::DATA_INI_INFO_T	CNTRSVR;
		bool		bBDSupport;		// ����GMָ��֧��(�������Ͳ�����GMָ��)

		CFGINFO_T()
		: bNBO(true)
		, bBDSupport(false)
		{
		}

		WHDATAPROP_DECLARE_MAP(CFGINFO_T);
	};
public:
	BD4Web_i();
	virtual	~BD4Web_i();
	int				Init_CFG(const char* cszCFGName);
private:
	friend class	MyCntrSvr;
	friend class	MyCntrSvr::MyCntr;
	//////////////////////////////////////////////////////////////////////////
	// ΪMyCntrSvr\MyCntrʵ�ֵķ���
	//////////////////////////////////////////////////////////////////////////
	// Cntr�Ƿ��ܹ���
	bool	Cntr_CanDealMsg() const;
	// ���һ��Cntr��һ�ι���
	int		Cntr_DealMsg(MyCntrSvr::MyCntr* pCntr, const BD_CMD_BASE_T*	pBaseCmd, size_t nSize);
	//
	Connecter*	NewConnecter(SOCKET sock, sockaddr_in* pAddr);
	void	AfterAddConnecter(Connecter* pCntr);
	void	BeforeDeleteConnecter(int nCntrID, ConnecterMan::CONNECTOR_INFO_T* pCntrInfo);
	// �Ƴ�һ��Connecter
	int		RemoveMyCntr(int nCntrID);
	int		RemoveMyCntr(MyCntrSvr::MyCntr* pCntr);
	// ������Ϣ
	int		SendCmdToConnecter(int nCntrID, const void* pData, size_t nSize);
	int		SendCmdToConnecter(MyCntrSvr::MyCntr* pCntr, const void* pData, size_t nSize);
private:
	int		DealCmdIn_One_BD4WEB_CMD_RPL(PNGSPACKET_BD4WEB_CMD_RPL_T* pCmd, size_t nSize);
private:
	typedef	CMN::ILogic			FATHERCLASS;

	CFGINFO_T		m_cfginfo;
	MyCntrSvr		m_CntrSvr;
	whtick_t		m_tickNow;
	tcpmsgerbase::DATA_INI_INFO_T	m_MSGER_INFO;

	// ȫ�ַ���ָ��
	ILogicBase*		m_pLogicGS4Web;

	whvector<char>	m_vectrawbuf;
};
}

#endif
