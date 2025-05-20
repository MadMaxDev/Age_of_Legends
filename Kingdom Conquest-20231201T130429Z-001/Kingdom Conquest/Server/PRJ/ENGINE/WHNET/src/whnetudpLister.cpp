// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpLister.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ����UDP�Ľ����б�ͷ���
// CreationDate : 2004-03-04

#include <WHNET/inc/whnetudp.h>
#include "../inc/whnetudpLister.h"
#include <WHCMN/inc/whstring.h>
#include <WHCMN/inc/whcrc.h>
#include <stdio.h>

using namespace n_whcmn;

namespace n_whnet
{

void	whnetudplister_make_crc(WHNETUDPLISTER_CMN_CMD_T *pCmd, size_t nSize)
{
	pCmd->nCrc	= 0;
	pCmd->nCrc	= crc8((char *)pCmd, nSize);
	pCmd->nCrc	^= 0x44;
	pCmd->nCrc	= crc8((char *)pCmd, nSize);
}
bool	whnetudplister_check_crc(WHNETUDPLISTER_CMN_CMD_T *pCmd, size_t nSize)
{
	unsigned char	mycrc = pCmd->nCrc;
	bool		bval;
	whnetudplister_make_crc(pCmd, nSize);
	bval		= (mycrc == pCmd->nCrc);
	pCmd->nCrc	= mycrc;
	return		bval;
}

}		// EOF namespace n_whnet

using namespace n_whnet;
using namespace n_whcmn;

////////////////////////////////////////////////////////////////////
// whnetudpListerSvr
////////////////////////////////////////////////////////////////////

WHDATAPROP_MAP_BEGIN_AT_ROOT(whnetudpListerSvr::CFGINFO_T)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nMaxData, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nUpdatePort, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nQueryPort, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nSelectInterval, 0)
	WHDATAPROP_ON_SETVALUE_smp_reload1(int, nUpdateTimeOut, 0)
WHDATAPROP_MAP_END()

whnetudpListerSvr::whnetudpListerSvr()
: m_nCurCFGLine(0)
, m_nMaxLevel(0), m_nMaxChildren(0)
, m_sock4Update(INVALID_SOCKET)
, m_sock4Query(INVALID_SOCKET)
, m_pRecvedCmd(NULL)
, m_nRecvedDataSize(0)
{
}
whnetudpListerSvr::~whnetudpListerSvr()
{
}
int		whnetudpListerSvr::Init(const char *cszCFG)
{
	int	rst;
	// ��������
	rst	= Init_CFG(cszCFG);
	if( rst<0 ) 
	{
		return	rst;
	}

	// ��ʼ�����粿��
	rst	= Init_Net();
	if( rst<0 ) 
	{
		return	rst;
	}

	// ����
	m_recvbuf.resize(m_cfginfo.nMaxData);
	m_pRecvedCmd	= (WHNETUDPLISTER_CMN_CMD_T *)m_recvbuf.getbuf();

	return		INITRST_OK;
}
int		whnetudpListerSvr::Init_CFG(const char *cszCFG)
{
	FILE		*fp;
	fp			= fopen(cszCFG, "rt");
	if( !fp )
	{
		return	INITRST_ERR_CFG_FILE;
	}

	// ����һ��һ�з���
	char		buf[1024];
	char		key[1024], value[1024];
	char		prevname[1024] = "";
	int			rst;
	int			initrst			= INITRST_OK;
	bool		bIsTrustedIP	= false;

	m_setTrustedIP.clear();
	m_mapID2NodeUnit.clear();
	m_vectNodeUnit.clear();

	// ��ǰջ
	whvector<NODE_T *>	nodestack;
	nodestack.reserve(10);

	// ��ǰ�Ľڵ�ָ��
	NODE_T		*pFatherNode	= NULL;
	NODE_T		*pCurNode		= &m_root;
	NODEUNIT_T	*pNodeUnit		= NULL;
	// �Ѹ��Ž�ȥ
	nodestack.push_back(pFatherNode);
	nodestack.push_back(pCurNode);

	m_nCurCFGLine	= 0;
	m_nMaxLevel		= 0;
	m_nMaxChildren	= 0;

	while( !feof(fp) )
	{
		if( !fgets(buf, sizeof(buf)-1, fp) )
		{
			// �������
			break;
		}
		m_nCurCFGLine	++;
		// ������
 		rst		= wh_strsplit("sa", buf, "=", key, value);
		// �ж��Ƿ���ע�ͻ����
		if( key[0]==0
		||  (key[0]=='/' && key[1]=='/')
		)
		{
			continue;
		}
		if( rst==1 )
		{
			switch( key[0] )
			{
				case	'{':
				{
					// �����Ƿ���TRUSTEDIP
					if( stricmp("TRUSTEDIP", prevname)==0 )
					{
						bIsTrustedIP		= true;
					}
					else
					{
						// ���浱ǰָ��
						pFatherNode	= pCurNode;
						// �࿪ͷ
						// �������ƵĿ�ͷ�ж�����
						if( '*' == prevname[0] )
						{
							// ˵����grp
							// ����֮
							pCurNode			= new NODE_T;
							pCurNode->nType		= NODE_T::TYPE_GRP;
							wh_copystr2vector(pCurNode->vectName, prevname+1);
							// ����Ҷ��
							pNodeUnit			= NULL;
						}
						else
						{
							// ˵����Ҷ��unit
							// ����֮
							pNodeUnit			= new NODEUNIT_T;
							pCurNode			= pNodeUnit;
							//
							pNodeUnit->nType	= NODE_T::TYPE_UNIT;
							pNodeUnit->nStatus	= WHNETUDPLISTER_STATUS_DEAD;
							pNodeUnit->nID		= 0;
							wh_copystr2vector(pNodeUnit->vectName, prevname);
							pNodeUnit->Data.reserve(m_cfginfo.nMaxData);
						}
						// ����ջ��
						nodestack.push_back(pCurNode);
						// ����father���б���
						pFatherNode->Child.push_back(pCurNode);
						pCurNode->pFather		= pFatherNode;
						// �ж��Ƿ������Level
						if( (size_t)m_nMaxLevel<nodestack.size() )
						{
							m_nMaxLevel		= nodestack.size();
						}
						// �ж��Ƿ�������ӽڵ���
						if( (size_t)m_nMaxChildren<pFatherNode->Child.size() )
						{
							m_nMaxChildren	= pFatherNode->Child.size();
						}
					}
				}
				break;
				case	'}':
				{
					// ���β
					if( bIsTrustedIP )
					{
						bIsTrustedIP	= false;
					}
					else
					{
						// ��ջ�е����Լ�
						if( nodestack.size()<3 )
						{
							// �﷨���󣬿϶�������д����
							initrst	= INITRST_ERR_CFG_SYNTAX;
							goto	End;
						}
						// ��鵱ǰ�����Ƿ�����
						if( pCurNode->nType == NODE_T::TYPE_UNIT )
						{
							if( pNodeUnit->nID == 0 )
							{
								initrst	= INITRST_ERR_CFG_NOID;
								goto	End;
							}
						}
						//
						nodestack.pop_back();
						// ���ϸ��ڵ��¼Ϊ��ǰָ��
						pCurNode	= nodestack.getfromtail(0);
						if( pCurNode->nType == NODE_T::TYPE_UNIT )
						{
							pNodeUnit	= (NODEUNIT_T *)pCurNode;
						}
						else
						{
							pNodeUnit	= NULL;
						}
						pFatherNode	= nodestack.getfromtail(1);
					}
				}
				break;
				default:
					// �������������һ��������
					strcpy(prevname, key);
				break;
			}
		}
		else if( rst==2 )
		{
			// ��trustedip
			if( bIsTrustedIP )
			{
				if( stricmp("IP", key)==0 )
				{
					// �ýڵ�IP
					whnet4byte_t	ipbytes;
					cmn_IPtoBin(value, ipbytes.c);
					if( !m_setTrustedIP.put(ipbytes) )
					{
						initrst	= INITRST_ERR_CFG_DUPTRUSTEDIP;
						goto	End;
					}
				}
			}
			// ˵����key = valueģʽ
			else if( pNodeUnit )
			{
				// �����Ҷ�Ӿ��ж��Ƿ�����Щ
				if( stricmp("ID", key)==0 )
				{
					pNodeUnit->nID	= atoi(value);
					if( pNodeUnit->nID==0 )
					{
						initrst	= INITRST_ERR_CFG_BADID;
						goto	End;
					}
					// ����hash��
					if( !m_mapID2NodeUnit.put(pNodeUnit->nID, pNodeUnit) )
					{
						initrst	= INITRST_ERR_CFG_DUPID;
						goto	End;
					}
					m_vectNodeUnit.push_back(pNodeUnit);
				}
			}
			else if( pFatherNode==NULL )
			{
				m_cfginfo.setvalue(key, value);
			}
		}
	}

End:
	fclose(fp);
	return		initrst;
}
int		whnetudpListerSvr::Init_Net()
{
	// ��ʼ������socket
	m_sock4Update	= udp_create_socket(m_cfginfo.nUpdatePort);
	if( INVALID_SOCKET == m_sock4Update )
	{
		return	INITRST_ERR_CREATESOCKET;
	}
	m_sock4Query	= udp_create_socket(m_cfginfo.nQueryPort);
	if( INVALID_SOCKET == m_sock4Query )
	{
		return	INITRST_ERR_CREATESOCKET;
	}
	m_sockArr[0]	= m_sock4Update;
	m_sockArr[1]	= m_sock4Query;
	return		INITRST_OK;
}

int		whnetudpListerSvr::Release()
{
	m_root.DelAllChildren();

	// �ر�����socket����������ǺõĻ�
	cmn_safeclosesocket(m_sock4Update);
	cmn_safeclosesocket(m_sock4Query);

	return		0;
}

int		whnetudpListerSvr::Tick()
{
	// ѡ������
	cmn_select_rd_array(m_sockArr, 2, m_cfginfo.nSelectInterval);

	// ����Update
	Tick_Update();

	// ����Query
	Tick_Query();

	return		0;
}
int		whnetudpListerSvr::Tick_Update()
{
	int			rst = 0;

	// ����û������
	while( cmn_select_rd(m_sock4Update, 0)>0 )
	{
		// ��ȡ����õ�ַ
		socklen_t			len = sizeof(m_curaddr);
		rst		= ::recvfrom(
				m_sock4Update
				, m_recvbuf.getbuf(), m_recvbuf.capacity()
				, 0
				, (struct sockaddr *)&m_curaddr, &len
				);
		if( rst<=0 )
		{
			// һ������Ƿ��ͶԷ�û�м���
			continue;
		}
		// ������ݲ��ĳߴ�
		m_recvbuf.resize(rst);
		m_nRecvedDataSize	= m_recvbuf.size()-sizeof(*m_pRecvedCmd)+1;

		// ����crcУ��
		if( !whnetudplister_check_crc(m_pRecvedCmd, m_recvbuf.size()) )
		{
			// ������
			continue;
		}

		// �����ָ��
		switch( m_pRecvedCmd->nCmd )
		{
			case	WHNETUDPLISTER_CMD_REQ_UPDATE:
			{
				Tick_Update_WHNETUDPLISTER_CMD_REQ_UPDATE();
			}
			break;
		}
	}

	// ���ݴ������ˣ����Ƿ��г�ʱ��û���յ�Update��UNIT���ı����ǵ�״̬
	Tick_Update_KeepAlive();

	return		0;
}
int		whnetudpListerSvr::Tick_Update_KeepAlive()
{
	whtick_t	nNow = wh_gettickcount();
	// �������Unit���ϴεĸ���ʱ��
	for(size_t i=0; i<m_vectNodeUnit.size(); i++)
	{
		NODEUNIT_T	*pNodeUnit = m_vectNodeUnit[i];
		assert(pNodeUnit);
		if( pNodeUnit->nLastUpdate==0
		||  wh_tickcount_diff(nNow, pNodeUnit->nLastUpdate)>=m_cfginfo.nUpdateTimeOut
		)
		{
			pNodeUnit->nStatus	= WHNETUDPLISTER_STATUS_DEAD;
		}
		else
		{
			// ˵������ʱ������
			pNodeUnit->nStatus	= WHNETUDPLISTER_STATUS_WORKING;
		}
	}
	return		0;
}
int		whnetudpListerSvr::Tick_Update_WHNETUDPLISTER_CMD_REQ_UPDATE()
{
	if( !m_setTrustedIP.has(m_curaddr.sin_addr.s_addr) )
	{
		// �Ǳ����εĵ�ַ
		return	0;
	}
	// ��ѯ�õ�ַ�Ƿ�Ϸ�
	NODEUNIT_T	*pNodeUnit;
	if( !m_mapID2NodeUnit.get((svrid_t)m_pRecvedCmd->nParam, pNodeUnit) )
	{
		// �Ƿ���ID
		return	0;
	}
	// ����ʱ��
	pNodeUnit->nLastUpdate	= wh_gettickcount();
	// ��ȡ���ݲ��֣������pNodeUnit��
	pNodeUnit->Data.resize( m_nRecvedDataSize );
	memcpy(pNodeUnit->Data.getbuf(), m_pRecvedCmd->Data, pNodeUnit->Data.size());

	return		0;
}
int		whnetudpListerSvr::Tick_Query()
{
	int			rst = 0;
	// ����û������
	while( cmn_select_rd(m_sock4Query, 0)>0 )
	{
		// ��ȡ����õ�ַ
		socklen_t			len = sizeof(m_curaddr);
		rst		= ::recvfrom(
				m_sock4Query
				, m_recvbuf.getbuf(), m_recvbuf.capacity()
				, 0
				, (struct sockaddr *)&m_curaddr, &len
				);
		if( rst<=0 )
		{
			// һ������Ƿ��ͶԷ�û�м���
			continue;
		}
		// ������ݲ��ĳߴ�
		m_recvbuf.resize(rst);
		m_nRecvedDataSize	= m_recvbuf.size()-sizeof(*m_pRecvedCmd)+1;

		// ����crcУ��
		if( !whnetudplister_check_crc(m_pRecvedCmd, m_recvbuf.size()) )
		{
			// ������
			continue;
		}

		switch( m_pRecvedCmd->nCmd )
		{
			case	WHNETUDPLISTER_CMD_REQ_QUERY:
			{
				Tick_Query_WHNETUDPLISTER_CMD_REQ_QUERY();
			}
			break;
		}
	}

	return		0;
}
// !!!! ������������Ӧ�ü�¼������־���������ڽ����鹥����¼���ͷ�IP !!!!
int		whnetudpListerSvr::Tick_Query_WHNETUDPLISTER_CMD_REQ_QUERY()
{
	// ���ݽṹ˵����ͷ�ļ���whnetudpLister.h
	whstrstream	wss;
	wss.setbin(m_pRecvedCmd->Data, m_nRecvedDataSize);
	WHNETUDPLISTER_NUM_T	nLevelNum, nIdxNum;
	WHNETUDPLISTER_IDX_T	*pLevel = NULL, *pIdx = NULL;
	// �ȶ�ȡ������
	if( wss.read(&nLevelNum)<=0 )
	{
		return	-1;
	}
	// �ж��Ƿ񳬹��˲�������
	if( nLevelNum>m_nMaxLevel )
	{
		// �������ƻ���
		return	-1;
	}
	// Level����ָ��
	pLevel		= (WHNETUDPLISTER_IDX_T *)wss.getcurptr();
	// ��stream��ָ�����
	wss.readarray((WHNETUDPLISTER_IDX_T*)NULL, nLevelNum);	// ����nLevelNum��0Ҳû��ϵ������Ͳ����
	// ���������ĸ���
	if( wss.read(&nIdxNum)<=0 )
	{
		return	-1;
	}
	if( nIdxNum>3 && nIdxNum>m_nMaxChildren )
	{
		// �������ƻ���
		return	-1;
	}
	// Idx����ָ��
	pIdx		= (WHNETUDPLISTER_IDX_T *)wss.getcurptr();

	// �ҵ���һ��
	NODE_T	*pLevelNode = GetLevelNode(pLevel, nLevelNum);
	if( !pLevelNode
	||  pLevelNode->nType!=NODE_T::TYPE_GRP
	)
	{
		// û���ҵ���Ӧ�Ĳ�
		return	-1;
	}

	// �ж��Ƿ��Ƿ�Χ(���ɷ�Χ�ڵ�����)
	whvector<WHNETUDPLISTER_IDX_T>	vectTmpIdx;
	if( nIdxNum>=2 )
	{
		if( pIdx[0]==pIdx[1] )
		{
			int	nMax = pLevelNode->Child.size() - pIdx[0];
			if( nIdxNum>=3 )
			{
				// ��pIdx[2]�Ǵ�pIdx[0]��ʼ�ĸ���
				nIdxNum	= nMax>pIdx[2] ? pIdx[2] : nMax;
			}
			else
			{
				// �������ܵ�ֵ
				nIdxNum	= nMax;
			}
			vectTmpIdx.reserve(nIdxNum);
			for(size_t i=0;i<nIdxNum;i++)
			{
				vectTmpIdx.push_back(pIdx[0]+i);
			}
			pIdx	= vectTmpIdx.getbuf();
		}
	}

	// ��������������ݷ����û�
	int	i;
	WHNETUDPLISTER_CMN_CMD_T	*pCmd = (WHNETUDPLISTER_CMN_CMD_T *)m_szQueryRplBuf;
	pCmd->nCmd	= WHNETUDPLISTER_CMD_RPL_QUERY;
	pCmd->nCrc	= 0;			// �����ͻ��ˣ��������У��
	WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T	*pData = (WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T *)pCmd->Data;
	int	nDSize	= 0;

	for(i=0;i<nIdxNum;i++)
	{
		pData->nIdx			= pIdx[i];
		// ���nDSize����С���ܵķ��سߴ�
		nDSize				= sizeof(*pData)-1;
		if( pData->nIdx >= pLevelNode->Child.size() )
		{
			// �±곬�磬���ش���
			// ���ڴ���Ľڵ�ҲҪ���أ���ö��������ĳ���Ƶ������re����
			pData->nRst		= WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T::RST_ERR_IDXTOOBIG;
			// ����nDSize��΢�˷�һ����������û��ϵ��
		}
		else
		{
			// ���NODEָ��(�����һ�����Ի�õ�)
			NODE_T	*pCurNode	= pLevelNode->Child[pData->nIdx];
			assert(pCurNode);
			pData->nRst		= WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T::RST_OK;
			pData->nType	= pCurNode->nType;
			// ��������
			int	nDSize2		= 0;
			switch( pData->nType )
			{
				case	whnetudpListerSvr::NODE_T::TYPE_GRP:
					nDSize2	= FillGrpData(pData->Data, pCurNode);
				break;
				case	whnetudpListerSvr::NODE_T::TYPE_UNIT:
					nDSize2	= FillUnitData(pData->Data, pCurNode);
				break;
			}
			nDSize			+= nDSize2;
			assert( (size_t)nDSize<sizeof(m_szQueryRplBuf) );
		}
		// ���͸���Ӧ���û���Ҳ���ǵ�ǰ��ַ��Ӧ���û�
		SendQueryRstToCurAddr(pCmd, nDSize + sizeof(*pCmd)-sizeof(pCmd->Data));
	}

	return		0;
}

whnetudpListerSvr::NODE_T *	whnetudpListerSvr::GetLevelNode(WHNETUDPLISTER_IDX_T *pLevel, int nLevel)
{
	// ��root��ʼ
	NODE_T	*pCurNode = &m_root;
	for(int i=0;i<nLevel;i++)
	{
		WHNETUDPLISTER_IDX_T	nIdx = pLevel[i];
		if( nIdx>=pCurNode->Child.size() )
		{
			// �±곬����
			return	NULL;
		}
		pCurNode	= pCurNode->Child[nIdx];
	}

	// û���ҵ�
	return	pCurNode;
}
int		whnetudpListerSvr::FillGrpData(void *pBuf, NODE_T *pNode)
{
	assert(pNode->nType == NODE_T::TYPE_GRP);
	
	whstrstream	wss;
	wss.setbin(pBuf, 0);
	
	WHNETUDPLISTER_IDX_T	nNum = pNode->Child.size();
	// ��ʽ�μ�whnetudpLister.h�У�TYPE_GRP��Ӧ��Data�Ľ���
	wss.writestrasvstr(pNode->vectName.getbuf());
	wss.write(&nNum);

	return	wss.getlen();
}
int		whnetudpListerSvr::FillUnitData(void *pBuf, NODE_T *pNode)
{
	assert(pNode->nType == NODE_T::TYPE_UNIT);

	NODEUNIT_T	*pNodeUnit = (NODEUNIT_T *)pNode;

	// ��ʽ�μ�whnetudpLister.h�У�TYPE_UNIT��Ӧ��Data�Ľ���
	whstrstream	wss;
	wss.setbin(pBuf, 0);
	wss.writestrasvstr(pNodeUnit->vectName.getbuf());
	wss.write(&pNodeUnit->nStatus);
	wss.write(pNodeUnit->Data.getbuf(), pNodeUnit->Data.size());

	return	wss.getlen();
}
int		whnetudpListerSvr::SendQueryRstToCurAddr(WHNETUDPLISTER_CMN_CMD_T *pCmd, int nSize)
{
	// ����У���
	whnetudplister_make_crc(pCmd, nSize);
	return	udp_sendto(m_sock4Query, pCmd, nSize, &m_curaddr);
}

////////////////////////////////////////////////////////////////////
// whnetudpListerCmdMaker
////////////////////////////////////////////////////////////////////
int		whnetudpListerCmdMaker::SendReqUpdate(const struct sockaddr_in *pAddr, const void *pData, int nSize)
{
	if( m_nSvrID==0 )
	{
		// m_nSvrID���뱻����
		assert(0);
		return	-1;
	}
	WHNETUDPLISTER_CMN_CMD_T	*pCmd = (WHNETUDPLISTER_CMN_CMD_T *)m_szCMDBuf;
	pCmd->nCmd					= WHNETUDPLISTER_CMD_REQ_UPDATE;
	pCmd->nParam				= m_nSvrID;
	memcpy(pCmd->Data, pData, nSize);
	// �ܳ�
	nSize						+= sizeof(*pCmd)-1;
	// ����
	return	CalcCRCAndSend(pAddr, pCmd, nSize);
}
int		whnetudpListerCmdMaker::SendReqQuery(const struct sockaddr_in *pAddr
		, WHNETUDPLISTER_IDX_T *pLevel, WHNETUDPLISTER_NUM_T nLevelNum
		, WHNETUDPLISTER_IDX_T *pIdx, WHNETUDPLISTER_NUM_T nIdxNum
		)
{
	// query�����ݽṹ��whnetudpLister.h�У�REQ_QUERY��Data���ֵ�������֯
	WHNETUDPLISTER_CMN_CMD_T	*pCmd = (WHNETUDPLISTER_CMN_CMD_T *)m_szCMDBuf;
	pCmd->nCmd					= WHNETUDPLISTER_CMD_REQ_QUERY;
	whstrstream	wss;
	wss.setbin(pCmd->Data, 0);
	wss.write(&nLevelNum);
	if( nLevelNum>0 )
	{
		wss.writearray(pLevel, nLevelNum);
	}
	wss.write(&nIdxNum);
	if( nIdxNum>0 )
	{
		wss.writearray(pIdx, nIdxNum);
	}

	int	nSize = sizeof(*pCmd)-1 + wss.getlen();

	// ����
	return	CalcCRCAndSend(pAddr, pCmd, nSize);
}
int		whnetudpListerCmdMaker::CalcCRCAndSend(const struct sockaddr_in *pAddr, WHNETUDPLISTER_CMN_CMD_T *pCmd, int nSize)
{
	// ����У���
	whnetudplister_make_crc(pCmd, nSize);
	return	udp_sendto(m_sock, pCmd, nSize, pAddr);
}
