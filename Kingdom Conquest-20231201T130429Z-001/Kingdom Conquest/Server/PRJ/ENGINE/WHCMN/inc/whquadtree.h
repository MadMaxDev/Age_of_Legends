// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whquadtree.h
// Creator      : Wei Hua (κ��)
// Comment      : �Ĳ������Ӧ��
//              : ���������Ϊ����ֻ���ǷǸ��������Ͻ�Ϊ(0,0)��
//              : ����ϵΪ����Ϊ������
//              : �ĸ���������
//              : 0 1
//              : 2 3
//              : ����Χ�����У����ĵ�����������ƫ���µĵط�������ͼ��
//              : ++++++++
//              : ++++++++
//              : ++++o+++
//              : ++++++++
//              : �����������ʾo��ΧX����4��Y����2�ķ�Χ
//              :
// CreationDate : 2005-12-05
// ChangeLOG    : 

#ifndef	__WHQUADTREE_H__
#define	__WHQUADTREE_H__

#include "whallocator2.h"
#include "whbits.h"
#include "whbox.h"

namespace n_whcmn
{

// �Ĳ����ڵ㼶��
typedef	unsigned char		quadtree_level_t;
// ���굥λ������Ϊsigned���͵ģ���Ϊ������Ҫ���������һЩ������Ҫ�и������磺�ѱ߽�����һ�£�
// ����ʹ����������÷Ǹ���
typedef	signed short		quadtree_coord_t;
// ����޷��ŵ��������߼���λ����
typedef	unsigned short		quadtree_ucoord_t;
// �����Ҫ����������λ
typedef	unsigned int		quadtree_ucoord2_t;
// �����������Ͷ�Ӧ��bit�����������͸ı��ˣ����������ֵҲҪ�ı䣩
enum
{
	QUADTREE_COORD_BITS		= 16,
};

// ����һά����
inline quadtree_coord_t	quadtree_1_dist(quadtree_coord_t nX1, quadtree_coord_t nX2)
{
	quadtree_coord_t	nDX	= nX1 - nX2;
	if( nDX<0 )			nDX	= -nDX;
	return				nDX;
}
// �������������Ķ�ά���Ӿ��루�����������ֵ�����ֵ��
inline quadtree_coord_t	quadtree_2_dist(quadtree_coord_t nX1, quadtree_coord_t nY1, quadtree_coord_t nX2, quadtree_coord_t nY2)
{
	quadtree_coord_t	nDX	= quadtree_1_dist(nX1, nX2);
	quadtree_coord_t	nDY	= quadtree_1_dist(nY1, nY2);
	if( nDX>nDY )		return	nDX;
	else				return	nDY;
}


#pragma pack(push, old_pack_num, 1)
union	QUADTREE_COORD_SHIFT_T
{
	quadtree_ucoord2_t		u;
	struct
	{
		quadtree_coord_t	cL;
		quadtree_coord_t	cH;
	};
	QUADTREE_COORD_SHIFT_T()
	: u(0)
	{
	}
	QUADTREE_COORD_SHIFT_T(quadtree_ucoord2_t __u)
	: u(__u)
	{
	}
	QUADTREE_COORD_SHIFT_T(quadtree_coord_t __cL, quadtree_coord_t __cH)
	: cL(__cL), cH(__cH)
	{
	}
	inline void	leftshift()
	{
		u	<<= 1;
	}
};
#pragma pack(pop, old_pack_num)

// �ڵ�Ĺ����ṹ
struct	QUADTREENODE_T;
struct	QUADTREENODE_CMN_T
{
	QUADTREENODE_T			*pParent;								// ���ڵ�ָ��
	quadtree_coord_t		nX, nY;									// ���ĵ�����
	quadtree_level_t		nLevel;									// LevelΪ0��ʾ��Ҷ��
	unsigned char			nPosInParent;							// �����Լ��Ǹ��ڵ��е��ĸ�(0~3)������������ǿ�����ɾ��ʱ���ٵ�������ڵ��е�����
};
// ��Ҷ�ӽڵ�
struct	QUADTREENODE_T		: public QUADTREENODE_CMN_T
{
	unsigned char			nChildMask;								// �������������Ƿ����
	QUADTREENODE_T			*apChild[4];							// Ҷ�ӽڵ���Ҫ��QUADTREENODE_T *ת��ΪQUADTREENODE_LEAF_T *
};
// Ҷ�ӽڵ�
struct	QUADTREENODE_LEAF_T	: public QUADTREENODE_CMN_T 
{
	// �������ݲ���
	// �������ͬ�����������Ӧ����pData�н���
	// pData������Ӧ�����ϲ����롢ʹ�á��ͷŵ�
	void					*pData;
};

// ����ĺ����ṩ�Խڵ��Ҷ�ӵĴ�������Releaseʱ����pData���ݵ��ͷš�
typedef	int	(*QUADTREE_PROCESS_FUNC_NODE_T)(QUADTREENODE_T *pNode, void *pExtInfo);
typedef	int	(*QUADTREE_PROCESS_FUNC_LEAF_T)(QUADTREENODE_LEAF_T *pLeaf, void *pExtInfo);

// ��������ṩSearchBox�Ľ���ڵ�Ĺ��˺ͱ���Ĺ��ܣ��磺�ڲ����վ������򣬱���ĳ�����͵Ľڵ㲻�������ս����
// pLeafΪҶ�ӽڵ�ṹ
// nDistX,YΪҶ�ӵ�ָ����Ķ�ά���룬�����Ҫ��Ϊ�˱����ڻص��ж���ļ���
// pExtInfoΪ���ظ��ص��Ĳ���������ض����ָ�룩
// ���ط���ֵ��ʾSearchBoxֹͣ�������ѷ���ֵֵ���ظ������ߣ����緵��������ʾ����ֹͣ������
typedef	int	(*QUADTREE_SEARCHBOX_JUDGE_FUNC_T)(QUADTREENODE_LEAF_T *pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY, void *pExtInfo);

// ��������ṩSearchRay�Ľ���ڵ�Ĺ��˺ͱ���Ĺ���
// pLeafΪҶ�ӽڵ�ṹ
// pExtInfoΪ���ظ��ص��Ĳ���������ض����ָ�룩
// ���ط���ֵ��ʾSearchRayֹͣ�������ѷ���ֵֵ���ظ������ߣ����緵��������ʾ����ֹͣ������
typedef	int	(*QUADTREE_SEARCHRAY_JUDGE_FUNC_T)(QUADTREENODE_LEAF_T *pLeaf, void *pExtInfo);

template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE=512*1024>
class	QuadtreeMan
{
public:
	QuadtreeMan(_AllocationObj *pAllocationObj);
	~QuadtreeMan();
	int	Init(quadtree_level_t nMaxLevel);
	int	Release(QUADTREE_PROCESS_FUNC_LEAF_T pReleaseFunc, void *pExtInfo);
	// Goover�����ӵ�����˳��ı���
	int	Goover(QUADTREE_PROCESS_FUNC_NODE_T pFuncNode, QUADTREE_PROCESS_FUNC_LEAF_T pFuncLeaf, void *pExtInfo);
	// ����Ҷ�ӽڵ㣬���ԭ���оͷ���ԭ����
	QUADTREENODE_LEAF_T *	Insert(quadtree_coord_t nX, quadtree_coord_t nY);
	// �����������Ҷ�ӽڵ�
	QUADTREENODE_LEAF_T *	FindLeaf(quadtree_coord_t nX, quadtree_coord_t nY);
	// �������������ײ�ڵ㣨������Ҷ��Ҳ�����Ƿ�Ҷ�ӣ�
	QUADTREENODE_CMN_T *	FindNode(quadtree_coord_t nX, quadtree_coord_t nY);
	// ��������ɾ��Ҷ�ӽڵ�
	int	Delete(quadtree_coord_t nX, quadtree_coord_t nY, QUADTREE_PROCESS_FUNC_LEAF_T pReleaseFunc, void *pExtInfo);
	// ����Ҷ�ӽڵ�ָ��ɾ�������һ��Ҫ��֤�ڵ�ָ����Ч�������Ǵ����ڹ������еģ������ϴ��Լ����ڵ㸽�����ݵ�release������
	void	Delete(QUADTREENODE_LEAF_T *pLeaf);
	// ���Ҵ�ĳ�㿪ʼ�����η�Χ�ڵ�Ҷ�ӽڵ�
	// nCenterX, nCenterY��ʾ���ĵ�����
	// nRadiusX, nRadiusY��ʾX��Y�����ϸ��Եķ�Χ����ʵ���ΪnRadius*2+1�������ĵ����߸��ӳ�nRadius�ľ��룩��
	// ��������򷵻�0
	// ���򷵻�pJudgeFunc�ķ��㷵��ֵ
	int	SearchBox(
		quadtree_coord_t nCenterX, quadtree_coord_t nCenterY
		, quadtree_coord_t nRadiusX, quadtree_coord_t nRadiusY
		, QUADTREE_SEARCHBOX_JUDGE_FUNC_T pJudgeFunc, void *pExtInfo
		);
	// ���Ҵ�ĳ�㿪ʼ��ָ�����߶���������Ҷ�ӽڵ�
	// nStartX, nStartYΪ��ʼ������
	// nEndX, nEndY��ʾ�����߶ε��յ�
	// nExtendRange��ʾ��ײ����չ��Χ����Ϊ����ɢ������߶�����󲿷�������������ϵģ����˶˵���м����㣩��������Ҫ��Ҫ��ײ��ķ�Χ����
	// nExtendRangeһ��ȡֵΪ1
	// ��������򷵻�0
	// ���򷵻�pJudgeFunc�ķ��㷵��ֵ
	int	SearchRay(
		quadtree_coord_t nStartX, quadtree_coord_t nStartY
		, quadtree_coord_t nEndX, quadtree_coord_t nEndY
		, quadtree_coord_t nExtendRange
		, QUADTREE_SEARCHRAY_JUDGE_FUNC_T pJudgeFunc, void *pExtInfo
		);
private:
	_AllocationObjOneSize	m_newer_QUADTREENODE_T;
	_AllocationObjOneSize	m_newer_QUADTREENODE_LEAF_T;
	QUADTREENODE_T		*m_pRoot;									// �����
	quadtree_level_t	m_nMaxLevel;								// �Ĳ���������
	quadtree_level_t	m_nDiffLevel;								// ����󼶱�Ĳ��
	quadtree_coord_t	m_nWidth;									// �����ȣ�2��m_nMaxLevel�η���
	// ������Ϊ����boxsearch�ݹ���üӿ�ʹ�õ���ʱ����
	quadtree_coord_t	m_bs_nCenterX, m_bs_nCenterY, m_bs_nRadiusX, m_bs_nRadiusY;
	QUADTREE_SEARCHBOX_JUDGE_FUNC_T	m_bs_pJudgeFunc;
	void				*m_bs_pExtInfo;
	// ������Ϊ����raysearch�ݹ���üӿ�ʹ�õ���ʱ����
	quadtree_coord_t	m_rs_nStartX, m_rs_nStartY, m_rs_nEndX, m_rs_nEndY, m_rs_nExtendRange;
	QUADTREE_SEARCHRAY_JUDGE_FUNC_T m_rs_pJudgeFunc;
	void				*m_rs_pExtInfo;
private:
	int	_Goover(QUADTREENODE_CMN_T *pNode, QUADTREE_PROCESS_FUNC_NODE_T pFuncNode, QUADTREE_PROCESS_FUNC_LEAF_T pFuncLeaf, void *pExtInfo);
	int	_SearchBox(QUADTREENODE_CMN_T *pNode);
	int	_SearchRay(QUADTREENODE_CMN_T *pNode);
	struct	GOOVER_RELEASE_INFO_T
	{
		QuadtreeMan						*pThis;
		void							*pExtInfoOutside;
		QUADTREE_PROCESS_FUNC_LEAF_T	pFuncOutside;
	};
	static int	goover_release_func_leaf(QUADTREENODE_LEAF_T *pLeaf, void *pExtInfo)
	{
		GOOVER_RELEASE_INFO_T	*pReleaseInfo	= (GOOVER_RELEASE_INFO_T *)pExtInfo;
		int	rst	= (*pReleaseInfo->pFuncOutside)(pLeaf, pReleaseInfo->pExtInfoOutside);
		if( rst!=0 )
		{
			return	rst;
		}
		// Ȼ��ڵ�ֱ��ɾ������
		pReleaseInfo->pThis->MyDelete(pLeaf);
		return	0;
	}
	static int	goover_release_func_node(QUADTREENODE_T *pNode, void *pExtInfo)
	{
		// ֱ��ɾ��
		GOOVER_RELEASE_INFO_T	*pReleaseInfo	= (GOOVER_RELEASE_INFO_T *)pExtInfo;
		pReleaseInfo->pThis->MyDelete(pNode);
		return	0;
	}
	// ��ʡʱ��Ͳ����й��캯����
	QUADTREENODE_T *	MyNew_QUADTREENODE_T()
	{
		return	(QUADTREENODE_T *)m_newer_QUADTREENODE_T.Alloc();
	}
	QUADTREENODE_LEAF_T *	MyNew_QUADTREENODE_LEAF_T()
	{
		return	(QUADTREENODE_LEAF_T *)m_newer_QUADTREENODE_LEAF_T.Alloc();
	}
	void	MyDelete(QUADTREENODE_T *ptr)
	{
		return	m_newer_QUADTREENODE_T.Free(ptr);
	}
	void	MyDelete(QUADTREENODE_LEAF_T *ptr)
	{
		return	m_newer_QUADTREENODE_LEAF_T.Free(ptr);
	}
};

template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::QuadtreeMan(_AllocationObj *pAllocationObj)
: m_pRoot(NULL)
, m_nMaxLevel(0)
, m_nDiffLevel(0)
, m_nWidth(0)
{
	m_newer_QUADTREENODE_T.Init(pAllocationObj, sizeof(QUADTREENODE_T), _CHUNKSIZE);
	m_newer_QUADTREENODE_LEAF_T.Init(pAllocationObj, sizeof(QUADTREENODE_LEAF_T), _CHUNKSIZE);
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::~QuadtreeMan()
{
	// ʲôҲ�������ϲ���Ҫ�Լ�����Release
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Init(quadtree_level_t nMaxLevel)
{
	// ����˵��û�г�ʼ��
	assert(m_nMaxLevel == 0);
	// ��֤������ȷ��
	assert(nMaxLevel > 0 && nMaxLevel <= QUADTREE_COORD_BITS);
	m_nMaxLevel	= nMaxLevel;
	m_nDiffLevel= QUADTREE_COORD_BITS - nMaxLevel;
	m_nWidth	= 1;
	m_nWidth	<<= m_nMaxLevel;
	return	0;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Release(QUADTREE_PROCESS_FUNC_LEAF_T pReleaseFunc, void *pExtInfo)
{
	GOOVER_RELEASE_INFO_T	ReleaseInfo;
	ReleaseInfo.pThis			= this;
	ReleaseInfo.pFuncOutside	= pReleaseFunc;
	ReleaseInfo.pExtInfoOutside	= pExtInfo;
	int	rst	= Goover(&QuadtreeMan::goover_release_func_node, &QuadtreeMan::goover_release_func_leaf, &ReleaseInfo);
	if( rst!=0 )
	{
		return	rst;
	}
	// ���Root�ڵ�
	m_pRoot	= NULL;
	return	0;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Goover(QUADTREE_PROCESS_FUNC_NODE_T pFuncNode, QUADTREE_PROCESS_FUNC_LEAF_T pFuncLeaf, void *pExtInfo)
{
	// ��������Ҷ�ӽڵ㣬��pFunc�Խڵ���д���
	return	_Goover(m_pRoot, pFuncNode, pFuncLeaf, pExtInfo);
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::_Goover(QUADTREENODE_CMN_T *pNode, QUADTREE_PROCESS_FUNC_NODE_T pFuncNode, QUADTREE_PROCESS_FUNC_LEAF_T pFuncLeaf, void *pExtInfo)
{
	if( pNode == NULL )
	{
		return	0;
	}
	if( pNode->nLevel > 0 )
	{
		// ˵���Ƿ�Ҷ��
		// �����ӽڵ�
		QUADTREENODE_T	*pRealNode	= (QUADTREENODE_T *)pNode;
		if( pRealNode->nChildMask != 0 )
		{
			#define	_GOOVER(i)	\
			{					\
				int	rst	= _Goover(pRealNode->apChild[i], pFuncNode, pFuncLeaf, pExtInfo);	\
				if( rst!=0 )	return	rst;	\
			}					\
			// EOF _GOOVER
			_GOOVER(0);
			_GOOVER(1);
			_GOOVER(2);
			_GOOVER(3);
		}
		// �����Լ������ڷ�Ҷ�ӽڵ�����ǻ���պ���ָ��ģ�
		if( pFuncNode != NULL )
		{
			return	(*pFuncNode)(pRealNode, pExtInfo);
		}
		return	0;
	}
	else
	{
		// ��Ҷ��
		return	(*pFuncLeaf)((QUADTREENODE_LEAF_T *)pNode, pExtInfo);
	}
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
QUADTREENODE_LEAF_T *	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Insert(quadtree_coord_t nX, quadtree_coord_t nY)
{
	assert(nX>=0 && nY>=0 && nX<m_nWidth && nY<m_nWidth);
	// �Ȱ�nX��nY�ƶ������󣨸�λ�����ϣ��Ա��ں�������λȡ����ȡ��λ�Ĺ���
	nX	= ((quadtree_ucoord_t)nX) << m_nDiffLevel;
	nY	= ((quadtree_ucoord_t)nY) << m_nDiffLevel;
	// �Ӹ���ʼ
	quadtree_coord_t	nLTX		= 0;
	quadtree_coord_t	nLTY		= 0;
	quadtree_coord_t	nWidth		= m_nWidth;
	quadtree_level_t	nLevel		= m_nMaxLevel;
	QUADTREENODE_T		**ppThis	= &m_pRoot;
	QUADTREENODE_T		*pParent	= NULL;
	QUADTREE_COORD_SHIFT_T	sYX;
	while( nLevel>0 )
	{
		// ��ȼ��루���������Ҫ����������/2��>>1Ҫ��һ��
		nWidth				/= 2;
		// �ж��Ƿ���Ҫ�����ڵ�
		QUADTREENODE_T	*&pThis	= *ppThis;
		if( pThis == NULL )
		{
			// �����½ڵ�
			pThis			= MyNew_QUADTREENODE_T();
			// ��������
			pThis->nLevel	= nLevel;
			// �����ʱ��nLTX,Y�����������Ͻǵ����꣬��pThis->nX,Y����������������
			pThis->nX		= nLTX + nWidth;
			pThis->nY		= nLTY + nWidth;
			pThis->pParent	= pParent;
			// �����Լ��Ǹ��ڵ���ĸ�(���ڸ������˵�����ֵ��ʵû������)
			pThis->nPosInParent	= (unsigned char)sYX.cH;
			// �����丸�ڵ��ж�Ӧ�Լ�������
			if( pParent != NULL )
			{
				whbit_uchar_set(&pParent->nChildMask, sYX.cH);
			}
			// ����ӽڵ�����
			pThis->nChildMask	= 0;
			// ����ӽڵ�ָ��
			memset(pThis->apChild, 0, sizeof(pThis->apChild));
		}
		// �ж���������
		sYX.cH				= 0;
		sYX.cL				= nY;
		sYX.leftshift();
		nY					= sYX.cL;
		if( sYX.cH!=0 )
		{
			nLTY	= pThis->nY;
		}	// ����ͻ��Ǹ���Y

		sYX.cL				= nX;
		sYX.leftshift();
		nX					= sYX.cL;
		if( (sYX.cH & 1) != 0 )
		{
			nLTX	= pThis->nX;
		}	// ����ͻ��Ǹ���X

		// ͨ������Ĳ�����nLTX,Y�ͱ�����µ��ӽڵ�����Ͻ�������

		// ������һ��
		nLevel		--;
		ppThis		= &pThis->apChild[sYX.cH];
		pParent		= pThis;
	}
	QUADTREENODE_LEAF_T		*&pLeaf	= (QUADTREENODE_LEAF_T *&)(*ppThis);
	if( pLeaf == NULL )
	{
		pLeaf				= MyNew_QUADTREENODE_LEAF_T();
		pLeaf->nLevel		= 0;
		pLeaf->nX			= nLTX;
		pLeaf->nY			= nLTY;
		pLeaf->pParent		= pParent;
		pLeaf->nPosInParent	= (unsigned char)sYX.cH;
		// ��new������������Ҫ�����һ��ָ��
		pLeaf->pData		= NULL;
		whbit_uchar_set(&pParent->nChildMask, sYX.cH);
	}
	return	pLeaf;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
QUADTREENODE_LEAF_T *	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::FindLeaf(quadtree_coord_t nX, quadtree_coord_t nY)
{
	assert(nX>=0 && nY>=0 && nX<m_nWidth && nY<m_nWidth);
	// �Ȱ�nX��nY�ƶ��������
	nX	= ((quadtree_ucoord_t)nX) << m_nDiffLevel;
	nY	= ((quadtree_ucoord_t)nY) << m_nDiffLevel;
	// �Ӹ���ʼ
	quadtree_level_t	nLevel=m_nMaxLevel;
	QUADTREENODE_T		*pNode=m_pRoot;
	QUADTREE_COORD_SHIFT_T	sYX;
	while( nLevel>0 )
	{
		if( pNode == NULL )
		{
			return	NULL;
		}
		// �ж���������
		sYX.cH				= 0;
		sYX.cL				= nY;
		sYX.leftshift();
		nY					= sYX.cL;
		sYX.cL				= nX;
		sYX.leftshift();
		nX					= sYX.cL;
		// ������һ��
		nLevel				--;
		pNode				= pNode->apChild[sYX.cH];
	}
	return	(QUADTREENODE_LEAF_T *)(pNode);
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
QUADTREENODE_CMN_T *	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::FindNode(quadtree_coord_t nX, quadtree_coord_t nY)
{
	if( m_pRoot == NULL )
	{
		return	NULL;
	}
	assert(nX>=0 && nY>=0 && nX<m_nWidth && nY<m_nWidth);
	// �Ȱ�nX��nY�ƶ��������
	nX	= ((quadtree_ucoord_t)nX) << m_nDiffLevel;
	nY	= ((quadtree_ucoord_t)nY) << m_nDiffLevel;
	// �Ӹ���ʼ
	quadtree_level_t	nLevel=m_nMaxLevel;
	QUADTREENODE_T		*pNode=m_pRoot;
	QUADTREE_COORD_SHIFT_T	sYX;
	while( nLevel>0 )
	{
		// �ж���������
		sYX.cH				= 0;
		sYX.cL				= nY;
		sYX.leftshift();
		nY					= sYX.cL;
		sYX.cL				= nX;
		sYX.leftshift();
		nX					= sYX.cL;
		// �����Ӧ�ķ�֧���������˳�ѭ�������ص�ǰ�ڵ�
		if( !pNode->apChild[sYX.cH] )
		{
			break;
		}
		// ������һ��
		pNode				= pNode->apChild[sYX.cH];
		nLevel				--;
	}
	return	pNode;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Delete(quadtree_coord_t nX, quadtree_coord_t nY, QUADTREE_PROCESS_FUNC_LEAF_T pReleaseFunc, void *pExtInfo)
{
	// �Ȳ��ҵ�Ҷ�ӽڵ�
	QUADTREENODE_LEAF_T	*pLeaf	= FindLeaf(nX, nY);
	if( pLeaf )
	{
		// �Ƚ��������ͷ�
		int	rst	= (*pReleaseFunc)(pLeaf, pExtInfo);
		if( rst!=0 )
		{
			return	rst;
		}
		// ��ɾ���ڵ�
		Delete(pLeaf);
		return	0;
	}
	return	-1;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
void	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Delete(QUADTREENODE_LEAF_T *pLeaf)
{
	assert(pLeaf);
	QUADTREENODE_T	*pParent	= pLeaf->pParent;
	// ��ʱ�ĸ��ڵ�ض���Ϊ�յ�
	assert(pParent);
	pParent->apChild[pLeaf->nPosInParent]	= NULL;
	whbit_uchar_clr(&pParent->nChildMask, pLeaf->nPosInParent);
	// ɾ��Ҷ��
	MyDelete(pLeaf);
	// �ݹ�ɾ���յĸ��ڵ�
	while( 1 )
	{
		// �ж��Լ��Ѿ�û�к�����
		if( pParent->nChildMask != 0 )
		{
			// �к��ӣ����ü�����
			break;
		}
		QUADTREENODE_T	*pThis	= pParent;
		pParent			= pThis->pParent;
		// �ж��Ƿ���root
		if( pParent == NULL )
		{
			m_pRoot		= NULL;
			// ɾ���Լ�
			MyDelete(pThis);
			// �˳�ѭ��
			break;
		}
		// �Լ�û�к����ˣ�Ӧ��ɾ����
		// ����Լ��ڸ��ڵ��е������Ϣ
		pParent->apChild[pThis->nPosInParent]	= NULL;
		whbit_uchar_clr(&pParent->nChildMask, pThis->nPosInParent);
		// ɾ���Լ�
		MyDelete(pThis);
	};
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::SearchBox(
	quadtree_coord_t nCenterX, quadtree_coord_t nCenterY
	, quadtree_coord_t nRadiusX, quadtree_coord_t nRadiusY
	, QUADTREE_SEARCHBOX_JUDGE_FUNC_T pJudgeFunc, void *pExtInfo
	)
{
	// ��¼����ı���
	m_bs_nCenterX	= nCenterX;
	m_bs_nCenterY	= nCenterY;
	m_bs_nRadiusX	= nRadiusX;
	m_bs_nRadiusY	= nRadiusY;
	m_bs_pJudgeFunc	= pJudgeFunc;
	m_bs_pExtInfo	= pExtInfo;
	//printf("start: %d\t%d\n", nCenterX, nCenterY);
	return	_SearchBox(m_pRoot);
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::_SearchBox(QUADTREENODE_CMN_T *pNode)
{
#if 0
	if( pNode == NULL )
	{
		return	0;
	}
	// �ȼ������ľ��루���Ҷ�Ӻͷ�Ҷ�Ӷ�Ҫ�ã�
	quadtree_coord_t	nDistX	= quadtree_1_dist(m_bs_nCenterX, pNode->nX);
	quadtree_coord_t	nDistY	= quadtree_1_dist(m_bs_nCenterY, pNode->nY);
	// �����Ҷ�ӽڵ���ֱ���жϾ��룬Ȼ����Ȼ�󷵻�
	//printf("level: %d\t%d\t%d\n", pNode->nLevel,pNode->nX,pNode->nY);
	if( pNode->nLevel == 0  )
	{
		if( nDistX<=m_bs_nRadiusX && nDistY<=m_bs_nRadiusY )
		{
			int	rst	= (*m_bs_pJudgeFunc)((QUADTREENODE_LEAF_T *)pNode, nDistX, nDistY, m_bs_pExtInfo);
			if( rst!=0 )
			{
				return	rst;
			}
		}
		return	0;
	}
	// ���û���ӽڵ�Ͳ�������
	QUADTREENODE_T	*pRealNode	= (QUADTREENODE_T *)pNode;
	if( pRealNode->nChildMask == 0 )
	{
		return	0;
	}
	// �ж��Լ���������ָ�������н���
	// ע���������жϷ�ʽ�ᵼ����Ч����������ƫ���µĵط�������ͼ��
	// ++++++++
	// ++++++++
	// ++++o+++
	// ++++++++
	// �����������ʾo��ΧX����4��Y����2�ķ�Χ
	quadtree_coord_t	nWidth	= ((quadtree_ucoord_t)1) << (pNode->nLevel-1);
	if( (nWidth+m_bs_nRadiusX) < nDistX
	||  (nWidth+m_bs_nRadiusY) < nDistY
	)
	{
		// û�н��������ټ�����
		return	0;
	}
	// �н�������������ӽڵ�
	#define	_SEARCH_BOX(i)	\
	{						\
		int	rst	= _SearchBox(pRealNode->apChild[i]);	\
		if( rst!=0 )		\
		{					\
			return	rst;	\
		}					\
	}						\
	// EOF _SEARCH_BOX
	_SEARCH_BOX(0);
	_SEARCH_BOX(1);
	_SEARCH_BOX(2);
	_SEARCH_BOX(3);
#else
    // By JiangLi �ǵݹ��㷨��δ��ϸ���Թ�
    if(pNode == 0) return 0;

    QUADTREE_SEARCHBOX_JUDGE_FUNC_T pJudgeFunc = m_bs_pJudgeFunc;
    QUADTREENODE_CMN_T* stk[64];
    QUADTREENODE_CMN_T** pos = stk;
    *pos++ = pNode;

    while(pos > stk)
    {
        pNode = *--pos;

	    // �����Ҷ�ӽڵ���ֱ���жϾ��룬Ȼ����Ȼ�󷵻�
	    if( pNode->nLevel == 0  )
	    {
            // ������㣬������������������������Լ���һЩ����
            quadtree_coord_t nDistX	= m_bs_nCenterX > pNode->nX ? m_bs_nCenterX-pNode->nX : pNode->nX-m_bs_nCenterX;
            if(nDistX > m_bs_nRadiusX) continue;
	        quadtree_coord_t nDistY	= m_bs_nCenterY > pNode->nY ? m_bs_nCenterY-pNode->nY : pNode->nY-m_bs_nCenterY;
            if(nDistY > m_bs_nRadiusY) continue;
			int	rst	= pJudgeFunc((QUADTREENODE_LEAF_T *)pNode, nDistX, nDistY, m_bs_pExtInfo);
			if(rst) return rst;
            continue;
	    }

	    // ���û���ӽڵ�Ͳ�������
	    if(((QUADTREENODE_T*)pNode)->nChildMask == 0) continue;

	    // �ж��Լ���������ָ�������н���
	    // ע���������жϷ�ʽ�ᵼ����Ч����������ƫ���µĵط�������ͼ��
	    // ++++++++
	    // ++++++++
	    // ++++o+++
	    // ++++++++
	    // �����������ʾo��ΧX����4��Y����2�ķ�Χ

		// ���û�н��������ټ����ˡ�
        // ʹ���޷��������ƹ�����ֵ�ļ��㡣
        quadtree_coord_t nWidth	= ((quadtree_ucoord_t)1) << (pNode->nLevel-1);
        quadtree_coord_t rx = nWidth+m_bs_nRadiusX;
        if(quadtree_ucoord_t(rx+rx) < quadtree_ucoord_t(pNode->nX-m_bs_nCenterX+rx)) continue;
        quadtree_coord_t ry = nWidth+m_bs_nRadiusY;
        if(quadtree_ucoord_t(ry+ry) < quadtree_ucoord_t(pNode->nY-m_bs_nCenterY+ry)) continue;

        for(int i = 0; i < 4; ++ i)
        {
            QUADTREENODE_CMN_T* n = ((QUADTREENODE_T*)pNode)->apChild[i];
            if(n)
            {
                *pos++ = n;
            }
        }
    }
#endif
	return	0;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::SearchRay(
	quadtree_coord_t nStartX, quadtree_coord_t nStartY
	, quadtree_coord_t nEndX, quadtree_coord_t nEndY
	, quadtree_coord_t nExtendRange
	, QUADTREE_SEARCHRAY_JUDGE_FUNC_T pJudgeFunc, void *pExtInfo
	)
{
	// ��¼����ı���
	m_rs_nStartX	= nStartX;
	m_rs_nStartY	= nStartY;
	m_rs_nEndX		= nEndX;
	m_rs_nEndY		= nEndY;
	m_rs_nExtendRange	= nExtendRange;
	m_rs_pJudgeFunc	= pJudgeFunc;
	m_rs_pExtInfo	= pExtInfo;
	return	_SearchRay(m_pRoot);
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::_SearchRay(QUADTREENODE_CMN_T *pNode)
{
	if( pNode == NULL )
	{
		return	0;
	}
	if( pNode->nLevel == 0 )
	{
		// ��Ҷ�ӣ���ô���Ľǣ���չ��ģ����
		quadtree_coord_t	nL	= pNode->nX - m_rs_nExtendRange;
		quadtree_coord_t	nT	= pNode->nY - m_rs_nExtendRange;
		quadtree_coord_t	nR	= pNode->nX + m_rs_nExtendRange;
		quadtree_coord_t	nB	= pNode->nY + m_rs_nExtendRange;
		// �жϺ��Լ��Ƿ��ཻ��������ཻ�Ͳ�������
		if( !whbox_check_intersection_with_line(nL, nT, nR, nB, m_rs_nStartX, m_rs_nStartY, m_rs_nEndX, m_rs_nEndY) )
		{
			return	0;
		}
		// ����һ�£�Ȼ�󷵻�
		return	(*m_rs_pJudgeFunc)((QUADTREENODE_LEAF_T *)pNode, m_rs_pExtInfo);
	}
	// ˵����Ҷ��
	// ���û���ӽڵ�Ҳ���ü�����
	QUADTREENODE_T	*pRealNode	= (QUADTREENODE_T *)pNode;
	if( pRealNode->nChildMask == 0 )
	{
		return	0;
	}
	// ��õ�ǰ�ڵ���Ľǣ���չ��ģ�
	quadtree_coord_t	nWidth	= ( ((quadtree_ucoord_t)1) << (pNode->nLevel-1) ) + m_rs_nExtendRange;
	quadtree_coord_t	nL		= pNode->nX - nWidth;
	quadtree_coord_t	nT		= pNode->nY - nWidth;
	quadtree_coord_t	nR		= pNode->nX + nWidth - 1;
	quadtree_coord_t	nB		= pNode->nY + nWidth - 1;
	// �жϺ��Լ��Ƿ��ཻ��������ཻ�Ͳ�������
	if( !whbox_check_intersection_with_line(nL, nT, nR, nB, m_rs_nStartX, m_rs_nStartY, m_rs_nEndX, m_rs_nEndY) )
	{
		return	0;
	}
	// ������ʼ������Լ����ĵ�λ��ȷ�������ӽڵ��˳��
	#define	_SEARCHRAY(i)		\
	{							\
		int	rst	= _SearchRay(pRealNode->apChild[i]);	\
		if( rst!=0 )			\
		{						\
			return	rst;		\
		}						\
	}							\
	// EOF _SEARCHRAY
	if( m_rs_nStartX<pNode->nX )
	{
		if( m_rs_nStartY<pNode->nY )
		{
			// LT	0 1 2 3
			_SEARCHRAY(0);
			_SEARCHRAY(1);
			_SEARCHRAY(2);
			_SEARCHRAY(3);
		}
		else
		{
			// LB	2 0 3 1
			_SEARCHRAY(2);
			_SEARCHRAY(0);
			_SEARCHRAY(3);
			_SEARCHRAY(1);
		}
	}
	else
	{
		if( m_rs_nStartY<pNode->nY )
		{
			// RT	1 3 0 2
			_SEARCHRAY(1);
			_SEARCHRAY(3);
			_SEARCHRAY(0);
			_SEARCHRAY(2);
		}
		else
		{
			// RB	3 2 1 0
			_SEARCHRAY(3);
			_SEARCHRAY(2);
			_SEARCHRAY(1);
			_SEARCHRAY(0);
		}
	}
	return	0;
}

}		// EOF namespace n_whcmn

#endif	// EOF __WHQUADTREE_H__
