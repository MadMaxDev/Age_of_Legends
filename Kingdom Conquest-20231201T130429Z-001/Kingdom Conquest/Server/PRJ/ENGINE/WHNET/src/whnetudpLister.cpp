// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whnet
// File         : whnetudpLister.cpp
// Creator      : Wei Hua (魏华)
// Comment      : 利用UDP的进行列表和访问
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
	// 读入配置
	rst	= Init_CFG(cszCFG);
	if( rst<0 ) 
	{
		return	rst;
	}

	// 初始化网络部分
	rst	= Init_Net();
	if( rst<0 ) 
	{
		return	rst;
	}

	// 其他
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

	// 读入一行一行分析
	char		buf[1024];
	char		key[1024], value[1024];
	char		prevname[1024] = "";
	int			rst;
	int			initrst			= INITRST_OK;
	bool		bIsTrustedIP	= false;

	m_setTrustedIP.clear();
	m_mapID2NodeUnit.clear();
	m_vectNodeUnit.clear();

	// 当前栈
	whvector<NODE_T *>	nodestack;
	nodestack.reserve(10);

	// 当前的节点指针
	NODE_T		*pFatherNode	= NULL;
	NODE_T		*pCurNode		= &m_root;
	NODEUNIT_T	*pNodeUnit		= NULL;
	// 把根放进去
	nodestack.push_back(pFatherNode);
	nodestack.push_back(pCurNode);

	m_nCurCFGLine	= 0;
	m_nMaxLevel		= 0;
	m_nMaxChildren	= 0;

	while( !feof(fp) )
	{
		if( !fgets(buf, sizeof(buf)-1, fp) )
		{
			// 输入错误
			break;
		}
		m_nCurCFGLine	++;
		// 分析行
 		rst		= wh_strsplit("sa", buf, "=", key, value);
		// 判断是否是注释或空行
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
					// 看看是否是TRUSTEDIP
					if( stricmp("TRUSTEDIP", prevname)==0 )
					{
						bIsTrustedIP		= true;
					}
					else
					{
						// 保存当前指针
						pFatherNode	= pCurNode;
						// 类开头
						// 根据名称的开头判断类型
						if( '*' == prevname[0] )
						{
							// 说明是grp
							// 生成之
							pCurNode			= new NODE_T;
							pCurNode->nType		= NODE_T::TYPE_GRP;
							wh_copystr2vector(pCurNode->vectName, prevname+1);
							// 不是叶子
							pNodeUnit			= NULL;
						}
						else
						{
							// 说明是叶子unit
							// 生成之
							pNodeUnit			= new NODEUNIT_T;
							pCurNode			= pNodeUnit;
							//
							pNodeUnit->nType	= NODE_T::TYPE_UNIT;
							pNodeUnit->nStatus	= WHNETUDPLISTER_STATUS_DEAD;
							pNodeUnit->nID		= 0;
							wh_copystr2vector(pNodeUnit->vectName, prevname);
							pNodeUnit->Data.reserve(m_cfginfo.nMaxData);
						}
						// 放入栈中
						nodestack.push_back(pCurNode);
						// 加入father的列表中
						pFatherNode->Child.push_back(pCurNode);
						pCurNode->pFather		= pFatherNode;
						// 判断是否是最大Level
						if( (size_t)m_nMaxLevel<nodestack.size() )
						{
							m_nMaxLevel		= nodestack.size();
						}
						// 判断是否是最多子节点数
						if( (size_t)m_nMaxChildren<pFatherNode->Child.size() )
						{
							m_nMaxChildren	= pFatherNode->Child.size();
						}
					}
				}
				break;
				case	'}':
				{
					// 类结尾
					if( bIsTrustedIP )
					{
						bIsTrustedIP	= false;
					}
					else
					{
						// 从栈中弹出自己
						if( nodestack.size()<3 )
						{
							// 语法错误，肯定是括号写多了
							initrst	= INITRST_ERR_CFG_SYNTAX;
							goto	End;
						}
						// 检查当前数据是否完整
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
						// 把上个节点记录为当前指针
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
					// 如果不是括号则一定是名称
					strcpy(prevname, key);
				break;
			}
		}
		else if( rst==2 )
		{
			// 是trustedip
			if( bIsTrustedIP )
			{
				if( stricmp("IP", key)==0 )
				{
					// 该节点IP
					whnet4byte_t	ipbytes;
					cmn_IPtoBin(value, ipbytes.c);
					if( !m_setTrustedIP.put(ipbytes) )
					{
						initrst	= INITRST_ERR_CFG_DUPTRUSTEDIP;
						goto	End;
					}
				}
			}
			// 说明是key = value模式
			else if( pNodeUnit )
			{
				// 如果是叶子就判断是否是这些
				if( stricmp("ID", key)==0 )
				{
					pNodeUnit->nID	= atoi(value);
					if( pNodeUnit->nID==0 )
					{
						initrst	= INITRST_ERR_CFG_BADID;
						goto	End;
					}
					// 放入hash表
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
	// 初始化两个socket
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

	// 关闭两个socket，如果他们是好的话
	cmn_safeclosesocket(m_sock4Update);
	cmn_safeclosesocket(m_sock4Query);

	return		0;
}

int		whnetudpListerSvr::Tick()
{
	// 选择两个
	cmn_select_rd_array(m_sockArr, 2, m_cfginfo.nSelectInterval);

	// 处理Update
	Tick_Update();

	// 处理Query
	Tick_Query();

	return		0;
}
int		whnetudpListerSvr::Tick_Update()
{
	int			rst = 0;

	// 看有没有数据
	while( cmn_select_rd(m_sock4Update, 0)>0 )
	{
		// 收取并获得地址
		socklen_t			len = sizeof(m_curaddr);
		rst		= ::recvfrom(
				m_sock4Update
				, m_recvbuf.getbuf(), m_recvbuf.capacity()
				, 0
				, (struct sockaddr *)&m_curaddr, &len
				);
		if( rst<=0 )
		{
			// 一般可能是发送对方没有监听
			continue;
		}
		// 获得数据部的尺寸
		m_recvbuf.resize(rst);
		m_nRecvedDataSize	= m_recvbuf.size()-sizeof(*m_pRecvedCmd)+1;

		// 进行crc校验
		if( !whnetudplister_check_crc(m_pRecvedCmd, m_recvbuf.size()) )
		{
			// 不处理
			continue;
		}

		// 处理该指令
		switch( m_pRecvedCmd->nCmd )
		{
			case	WHNETUDPLISTER_CMD_REQ_UPDATE:
			{
				Tick_Update_WHNETUDPLISTER_CMD_REQ_UPDATE();
			}
			break;
		}
	}

	// 数据处理完了，看是否有长时间没有收到Update的UNIT，改变他们的状态
	Tick_Update_KeepAlive();

	return		0;
}
int		whnetudpListerSvr::Tick_Update_KeepAlive()
{
	whtick_t	nNow = wh_gettickcount();
	// 检查所有Unit的上次的更新时间
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
			// 说明被及时更新了
			pNodeUnit->nStatus	= WHNETUDPLISTER_STATUS_WORKING;
		}
	}
	return		0;
}
int		whnetudpListerSvr::Tick_Update_WHNETUDPLISTER_CMD_REQ_UPDATE()
{
	if( !m_setTrustedIP.has(m_curaddr.sin_addr.s_addr) )
	{
		// 非被信任的地址
		return	0;
	}
	// 查询该地址是否合法
	NODEUNIT_T	*pNodeUnit;
	if( !m_mapID2NodeUnit.get((svrid_t)m_pRecvedCmd->nParam, pNodeUnit) )
	{
		// 非法的ID
		return	0;
	}
	// 更新时间
	pNodeUnit->nLastUpdate	= wh_gettickcount();
	// 获取数据部分，存放在pNodeUnit中
	pNodeUnit->Data.resize( m_nRecvedDataSize );
	memcpy(pNodeUnit->Data.getbuf(), m_pRecvedCmd->Data, pNodeUnit->Data.size());

	return		0;
}
int		whnetudpListerSvr::Tick_Query()
{
	int			rst = 0;
	// 看有没有数据
	while( cmn_select_rd(m_sock4Query, 0)>0 )
	{
		// 收取并获得地址
		socklen_t			len = sizeof(m_curaddr);
		rst		= ::recvfrom(
				m_sock4Query
				, m_recvbuf.getbuf(), m_recvbuf.capacity()
				, 0
				, (struct sockaddr *)&m_curaddr, &len
				);
		if( rst<=0 )
		{
			// 一般可能是发送对方没有监听
			continue;
		}
		// 获得数据部的尺寸
		m_recvbuf.resize(rst);
		m_nRecvedDataSize	= m_recvbuf.size()-sizeof(*m_pRecvedCmd)+1;

		// 进行crc校验
		if( !whnetudplister_check_crc(m_pRecvedCmd, m_recvbuf.size()) )
		{
			// 不处理
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
// !!!! 随后这个函数内应该记录访问日志，这样便于将来查攻击记录，和封IP !!!!
int		whnetudpListerSvr::Tick_Query_WHNETUDPLISTER_CMD_REQ_QUERY()
{
	// 数据结构说明见头文件：whnetudpLister.h
	whstrstream	wss;
	wss.setbin(m_pRecvedCmd->Data, m_nRecvedDataSize);
	WHNETUDPLISTER_NUM_T	nLevelNum, nIdxNum;
	WHNETUDPLISTER_IDX_T	*pLevel = NULL, *pIdx = NULL;
	// 先读取出层数
	if( wss.read(&nLevelNum)<=0 )
	{
		return	-1;
	}
	// 判断是否超过了层数限制
	if( nLevelNum>m_nMaxLevel )
	{
		// 可能是破坏包
		return	-1;
	}
	// Level数组指针
	pLevel		= (WHNETUDPLISTER_IDX_T *)wss.getcurptr();
	// 让stream的指针后移
	wss.readarray((WHNETUDPLISTER_IDX_T*)NULL, nLevelNum);	// 就算nLevelNum是0也没关系，里面就不会读
	// 读入索引的个数
	if( wss.read(&nIdxNum)<=0 )
	{
		return	-1;
	}
	if( nIdxNum>3 && nIdxNum>m_nMaxChildren )
	{
		// 可能是破坏包
		return	-1;
	}
	// Idx数组指针
	pIdx		= (WHNETUDPLISTER_IDX_T *)wss.getcurptr();

	// 找到那一层
	NODE_T	*pLevelNode = GetLevelNode(pLevel, nLevelNum);
	if( !pLevelNode
	||  pLevelNode->nType!=NODE_T::TYPE_GRP
	)
	{
		// 没有找到相应的层
		return	-1;
	}

	// 判断是否是范围(生成范围内的索引)
	whvector<WHNETUDPLISTER_IDX_T>	vectTmpIdx;
	if( nIdxNum>=2 )
	{
		if( pIdx[0]==pIdx[1] )
		{
			int	nMax = pLevelNode->Child.size() - pIdx[0];
			if( nIdxNum>=3 )
			{
				// 则pIdx[2]是从pIdx[0]开始的个数
				nIdxNum	= nMax>pIdx[2] ? pIdx[2] : nMax;
			}
			else
			{
				// 到最大可能的值
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

	// 将所有请求的数据发给用户
	int	i;
	WHNETUDPLISTER_CMN_CMD_T	*pCmd = (WHNETUDPLISTER_CMN_CMD_T *)m_szQueryRplBuf;
	pCmd->nCmd	= WHNETUDPLISTER_CMD_RPL_QUERY;
	pCmd->nCrc	= 0;			// 发给客户端，这个不用校验
	WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T	*pData = (WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T *)pCmd->Data;
	int	nDSize	= 0;

	for(i=0;i<nIdxNum;i++)
	{
		pData->nIdx			= pIdx[i];
		// 这个nDSize是最小可能的返回尺寸
		nDSize				= sizeof(*pData)-1;
		if( pData->nIdx >= pLevelNode->Child.size() )
		{
			// 下标超界，返回错误
			// 对于错误的节点也要返回，免得对于正常的程序频繁发来re请求
			pData->nRst		= WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T::RST_ERR_IDXTOOBIG;
			// 这样nDSize稍微浪费一点啦，不过没关系啦
		}
		else
		{
			// 获得NODE指针(这个是一定可以获得的)
			NODE_T	*pCurNode	= pLevelNode->Child[pData->nIdx];
			assert(pCurNode);
			pData->nRst		= WHNETUDPLISTER_CMD_RPL_QUERY_DATA_T::RST_OK;
			pData->nType	= pCurNode->nType;
			// 拷贝数据
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
		// 发送给相应的用户，也就是当前地址对应的用户
		SendQueryRstToCurAddr(pCmd, nDSize + sizeof(*pCmd)-sizeof(pCmd->Data));
	}

	return		0;
}

whnetudpListerSvr::NODE_T *	whnetudpListerSvr::GetLevelNode(WHNETUDPLISTER_IDX_T *pLevel, int nLevel)
{
	// 从root开始
	NODE_T	*pCurNode = &m_root;
	for(int i=0;i<nLevel;i++)
	{
		WHNETUDPLISTER_IDX_T	nIdx = pLevel[i];
		if( nIdx>=pCurNode->Child.size() )
		{
			// 下标超界了
			return	NULL;
		}
		pCurNode	= pCurNode->Child[nIdx];
	}

	// 没有找到
	return	pCurNode;
}
int		whnetudpListerSvr::FillGrpData(void *pBuf, NODE_T *pNode)
{
	assert(pNode->nType == NODE_T::TYPE_GRP);
	
	whstrstream	wss;
	wss.setbin(pBuf, 0);
	
	WHNETUDPLISTER_IDX_T	nNum = pNode->Child.size();
	// 格式参见whnetudpLister.h中：TYPE_GRP对应的Data的解释
	wss.writestrasvstr(pNode->vectName.getbuf());
	wss.write(&nNum);

	return	wss.getlen();
}
int		whnetudpListerSvr::FillUnitData(void *pBuf, NODE_T *pNode)
{
	assert(pNode->nType == NODE_T::TYPE_UNIT);

	NODEUNIT_T	*pNodeUnit = (NODEUNIT_T *)pNode;

	// 格式参见whnetudpLister.h中：TYPE_UNIT对应的Data的解释
	whstrstream	wss;
	wss.setbin(pBuf, 0);
	wss.writestrasvstr(pNodeUnit->vectName.getbuf());
	wss.write(&pNodeUnit->nStatus);
	wss.write(pNodeUnit->Data.getbuf(), pNodeUnit->Data.size());

	return	wss.getlen();
}
int		whnetudpListerSvr::SendQueryRstToCurAddr(WHNETUDPLISTER_CMN_CMD_T *pCmd, int nSize)
{
	// 生成校验核
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
		// m_nSvrID必须被设置
		assert(0);
		return	-1;
	}
	WHNETUDPLISTER_CMN_CMD_T	*pCmd = (WHNETUDPLISTER_CMN_CMD_T *)m_szCMDBuf;
	pCmd->nCmd					= WHNETUDPLISTER_CMD_REQ_UPDATE;
	pCmd->nParam				= m_nSvrID;
	memcpy(pCmd->Data, pData, nSize);
	// 总长
	nSize						+= sizeof(*pCmd)-1;
	// 发送
	return	CalcCRCAndSend(pAddr, pCmd, nSize);
}
int		whnetudpListerCmdMaker::SendReqQuery(const struct sockaddr_in *pAddr
		, WHNETUDPLISTER_IDX_T *pLevel, WHNETUDPLISTER_NUM_T nLevelNum
		, WHNETUDPLISTER_IDX_T *pIdx, WHNETUDPLISTER_NUM_T nIdxNum
		)
{
	// query的数据结构见whnetudpLister.h中：REQ_QUERY的Data部分的数据组织
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

	// 发送
	return	CalcCRCAndSend(pAddr, pCmd, nSize);
}
int		whnetudpListerCmdMaker::CalcCRCAndSend(const struct sockaddr_in *pAddr, WHNETUDPLISTER_CMN_CMD_T *pCmd, int nSize)
{
	// 计算校验核
	whnetudplister_make_crc(pCmd, nSize);
	return	udp_sendto(m_sock, pCmd, nSize, pAddr);
}
