// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whquadtree.h
// Creator      : Wei Hua (魏华)
// Comment      : 四叉树相关应用
//              : 描述区域简化为坐标只能是非负数，左上角为(0,0)点
//              : 坐标系为右下为正方向
//              : 四格的序号描述
//              : 0 1
//              : 2 3
//              : 区域范围描述中，中心点是区域中心偏右下的地方。如下图：
//              : ++++++++
//              : ++++++++
//              : ++++o+++
//              : ++++++++
//              : 这样的区域表示o周围X距离4，Y距离2的范围
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

// 四叉树节点级别
typedef	unsigned char		quadtree_level_t;
// 坐标单位（必须为signed类型的，因为坐标需要相减，而且一些处理需要有负数，如：把边界扩大一下）
// 但是使用坐标必须用非负数
typedef	signed short		quadtree_coord_t;
// 这个无符号的用来做逻辑移位操作
typedef	unsigned short		quadtree_ucoord_t;
// 这个主要用来进行移位
typedef	unsigned int		quadtree_ucoord2_t;
// 上面坐标类型对应的bit数（上面类型改变了，下面这个数值也要改变）
enum
{
	QUADTREE_COORD_BITS		= 16,
};

// 计算一维距离
inline quadtree_coord_t	quadtree_1_dist(quadtree_coord_t nX1, quadtree_coord_t nX2)
{
	quadtree_coord_t	nDX	= nX1 - nX2;
	if( nDX<0 )			nDX	= -nDX;
	return				nDX;
}
// 计算两个坐标间的二维盒子距离（即两个坐标差值的最大值）
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

// 节点的公共结构
struct	QUADTREENODE_T;
struct	QUADTREENODE_CMN_T
{
	QUADTREENODE_T			*pParent;								// 父节点指针
	quadtree_coord_t		nX, nY;									// 中心点坐标
	quadtree_level_t		nLevel;									// Level为0表示是叶子
	unsigned char			nPosInParent;							// 表明自己是父节点中的哪个(0~3)，这个的作用是可以在删除时快速的清除父节点中的索引
};
// 非叶子节点
struct	QUADTREENODE_T		: public QUADTREENODE_CMN_T
{
	unsigned char			nChildMask;								// 表明各个孩子是否存在
	QUADTREENODE_T			*apChild[4];							// 叶子节点需要把QUADTREENODE_T *转换为QUADTREENODE_LEAF_T *
};
// 叶子节点
struct	QUADTREENODE_LEAF_T	: public QUADTREENODE_CMN_T 
{
	// 附加数据部分
	// 如果有相同坐标的需求，则应该在pData中解释
	// pData的数据应该是上层申请、使用、释放的
	void					*pData;
};

// 下面的函数提供对节点和叶子的处理。比如Release时外界对pData数据的释放。
typedef	int	(*QUADTREE_PROCESS_FUNC_NODE_T)(QUADTREENODE_T *pNode, void *pExtInfo);
typedef	int	(*QUADTREE_PROCESS_FUNC_LEAF_T)(QUADTREENODE_LEAF_T *pLeaf, void *pExtInfo);

// 这个函数提供SearchBox的结果节点的过滤和保存的功能（如：内部按照距离排序，比如某种类型的节点不加入最终结果）
// pLeaf为叶子节点结构
// nDistX,Y为叶子到指定点的二维距离，这个主要是为了避免在回调中多余的计算
// pExtInfo为传回给回调的参数（如相关对象的指针）
// 返回非零值表示SearchBox停止搜索，把返回值值返回给调用者（比如返回正数表示可以停止搜索）
typedef	int	(*QUADTREE_SEARCHBOX_JUDGE_FUNC_T)(QUADTREENODE_LEAF_T *pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY, void *pExtInfo);

// 这个函数提供SearchRay的结果节点的过滤和保存的功能
// pLeaf为叶子节点结构
// pExtInfo为传回给回调的参数（如相关对象的指针）
// 返回非零值表示SearchRay停止搜索，把返回值值返回给调用者（比如返回正数表示可以停止搜索）
typedef	int	(*QUADTREE_SEARCHRAY_JUDGE_FUNC_T)(QUADTREENODE_LEAF_T *pLeaf, void *pExtInfo);

template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE=512*1024>
class	QuadtreeMan
{
public:
	QuadtreeMan(_AllocationObj *pAllocationObj);
	~QuadtreeMan();
	int	Init(quadtree_level_t nMaxLevel);
	int	Release(QUADTREE_PROCESS_FUNC_LEAF_T pReleaseFunc, void *pExtInfo);
	// Goover是由子到父的顺序的遍历
	int	Goover(QUADTREE_PROCESS_FUNC_NODE_T pFuncNode, QUADTREE_PROCESS_FUNC_LEAF_T pFuncLeaf, void *pExtInfo);
	// 插入叶子节点，如果原来有就返回原来的
	QUADTREENODE_LEAF_T *	Insert(quadtree_coord_t nX, quadtree_coord_t nY);
	// 根据坐标查找叶子节点
	QUADTREENODE_LEAF_T *	FindLeaf(quadtree_coord_t nX, quadtree_coord_t nY);
	// 根据坐标查找最底层节点（可以是叶子也可能是非叶子）
	QUADTREENODE_CMN_T *	FindNode(quadtree_coord_t nX, quadtree_coord_t nY);
	// 根据坐标删除叶子节点
	int	Delete(quadtree_coord_t nX, quadtree_coord_t nY, QUADTREE_PROCESS_FUNC_LEAF_T pReleaseFunc, void *pExtInfo);
	// 根据叶子节点指针删除（这个一定要保证节点指针有效，并且是存在于管理器中的，并且上次自己做节点附加数据的release工作）
	void	Delete(QUADTREENODE_LEAF_T *pLeaf);
	// 查找从某点开始矩形形范围内的叶子节点
	// nCenterX, nCenterY表示中心点坐标
	// nRadiusX, nRadiusY表示X、Y方向上各自的范围。真实跨度为nRadius*2+1（即中心点两边各延长nRadius的距离）。
	// 如果正常则返回0
	// 否则返回pJudgeFunc的非零返回值
	int	SearchBox(
		quadtree_coord_t nCenterX, quadtree_coord_t nCenterY
		, quadtree_coord_t nRadiusX, quadtree_coord_t nRadiusY
		, QUADTREE_SEARCHBOX_JUDGE_FUNC_T pJudgeFunc, void *pExtInfo
		);
	// 查找从某点开始的指定长线段内碰到的叶子节点
	// nStartX, nStartY为起始点坐标
	// nEndX, nEndY表示了射线段的终点
	// nExtendRange表示碰撞的扩展范围（因为在离散情况下线段碰点大部分情况都是碰不上的（除了端点和中间整点），所以需要把要碰撞点的范围扩大）
	// nExtendRange一般取值为1
	// 如果正常则返回0
	// 否则返回pJudgeFunc的非零返回值
	int	SearchRay(
		quadtree_coord_t nStartX, quadtree_coord_t nStartY
		, quadtree_coord_t nEndX, quadtree_coord_t nEndY
		, quadtree_coord_t nExtendRange
		, QUADTREE_SEARCHRAY_JUDGE_FUNC_T pJudgeFunc, void *pExtInfo
		);
private:
	_AllocationObjOneSize	m_newer_QUADTREENODE_T;
	_AllocationObjOneSize	m_newer_QUADTREENODE_LEAF_T;
	QUADTREENODE_T		*m_pRoot;									// 根结点
	quadtree_level_t	m_nMaxLevel;								// 四叉树最多层数
	quadtree_level_t	m_nDiffLevel;								// 和最大级别的差别
	quadtree_coord_t	m_nWidth;									// 区域宽度（2的m_nMaxLevel次方）
	// 下面是为了让boxsearch递归调用加快使用的临时变量
	quadtree_coord_t	m_bs_nCenterX, m_bs_nCenterY, m_bs_nRadiusX, m_bs_nRadiusY;
	QUADTREE_SEARCHBOX_JUDGE_FUNC_T	m_bs_pJudgeFunc;
	void				*m_bs_pExtInfo;
	// 下面是为了让raysearch递归调用加快使用的临时变量
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
		// 然后节点直接删除即可
		pReleaseInfo->pThis->MyDelete(pLeaf);
		return	0;
	}
	static int	goover_release_func_node(QUADTREENODE_T *pNode, void *pExtInfo)
	{
		// 直接删除
		GOOVER_RELEASE_INFO_T	*pReleaseInfo	= (GOOVER_RELEASE_INFO_T *)pExtInfo;
		pReleaseInfo->pThis->MyDelete(pNode);
		return	0;
	}
	// 节省时间就不用有构造函数了
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
	// 什么也不做，上层需要自己调用Release
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Init(quadtree_level_t nMaxLevel)
{
	// 这样说明没有初始化
	assert(m_nMaxLevel == 0);
	// 保证数据正确性
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
	// 清空Root节点
	m_pRoot	= NULL;
	return	0;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Goover(QUADTREE_PROCESS_FUNC_NODE_T pFuncNode, QUADTREE_PROCESS_FUNC_LEAF_T pFuncLeaf, void *pExtInfo)
{
	// 遍历所有叶子节点，让pFunc对节点进行处理。
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
		// 说明是非叶子
		// 处理子节点
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
		// 处理自己（对于非叶子节点可以是会给空函数指针的）
		if( pFuncNode != NULL )
		{
			return	(*pFuncNode)(pRealNode, pExtInfo);
		}
		return	0;
	}
	else
	{
		// 是叶子
		return	(*pFuncLeaf)((QUADTREENODE_LEAF_T *)pNode, pExtInfo);
	}
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
QUADTREENODE_LEAF_T *	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Insert(quadtree_coord_t nX, quadtree_coord_t nY)
{
	assert(nX>=0 && nY>=0 && nX<m_nWidth && nY<m_nWidth);
	// 先把nX和nY移动到最左（高位）边上，以便在后面作移位取依次取高位的工作
	nX	= ((quadtree_ucoord_t)nX) << m_nDiffLevel;
	nY	= ((quadtree_ucoord_t)nY) << m_nDiffLevel;
	// 从根开始
	quadtree_coord_t	nLTX		= 0;
	quadtree_coord_t	nLTY		= 0;
	quadtree_coord_t	nWidth		= m_nWidth;
	quadtree_level_t	nLevel		= m_nMaxLevel;
	QUADTREENODE_T		**ppThis	= &m_pRoot;
	QUADTREENODE_T		*pParent	= NULL;
	QUADTREE_COORD_SHIFT_T	sYX;
	while( nLevel>0 )
	{
		// 宽度减半（这个后面需要），好像用/2比>>1要快一点
		nWidth				/= 2;
		// 判断是否需要创建节点
		QUADTREENODE_T	*&pThis	= *ppThis;
		if( pThis == NULL )
		{
			// 创建新节点
			pThis			= MyNew_QUADTREENODE_T();
			// 基本数据
			pThis->nLevel	= nLevel;
			// 在这个时候nLTX,Y就是区域左上角的坐标，而pThis->nX,Y则是区域中心坐标
			pThis->nX		= nLTX + nWidth;
			pThis->nY		= nLTY + nWidth;
			pThis->pParent	= pParent;
			// 标明自己是父节点的哪个(对于根结点来说，这个值其实没有意义)
			pThis->nPosInParent	= (unsigned char)sYX.cH;
			// 设置其父节点中对应自己的掩码
			if( pParent != NULL )
			{
				whbit_uchar_set(&pParent->nChildMask, sYX.cH);
			}
			// 清空子节点掩码
			pThis->nChildMask	= 0;
			// 清空子节点指针
			memset(pThis->apChild, 0, sizeof(pThis->apChild));
		}
		// 判定坐标区间
		sYX.cH				= 0;
		sYX.cL				= nY;
		sYX.leftshift();
		nY					= sYX.cL;
		if( sYX.cH!=0 )
		{
			nLTY	= pThis->nY;
		}	// 否则就还是父的Y

		sYX.cL				= nX;
		sYX.leftshift();
		nX					= sYX.cL;
		if( (sYX.cH & 1) != 0 )
		{
			nLTX	= pThis->nX;
		}	// 否则就还是父的X

		// 通过上面的操作，nLTX,Y就变成了新的子节点的左上角坐标了

		// 进行下一步
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
		// 新new出来的数据需要先清空一下指针
		pLeaf->pData		= NULL;
		whbit_uchar_set(&pParent->nChildMask, sYX.cH);
	}
	return	pLeaf;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
QUADTREENODE_LEAF_T *	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::FindLeaf(quadtree_coord_t nX, quadtree_coord_t nY)
{
	assert(nX>=0 && nY>=0 && nX<m_nWidth && nY<m_nWidth);
	// 先把nX和nY移动到最边上
	nX	= ((quadtree_ucoord_t)nX) << m_nDiffLevel;
	nY	= ((quadtree_ucoord_t)nY) << m_nDiffLevel;
	// 从根开始
	quadtree_level_t	nLevel=m_nMaxLevel;
	QUADTREENODE_T		*pNode=m_pRoot;
	QUADTREE_COORD_SHIFT_T	sYX;
	while( nLevel>0 )
	{
		if( pNode == NULL )
		{
			return	NULL;
		}
		// 判定坐标区间
		sYX.cH				= 0;
		sYX.cL				= nY;
		sYX.leftshift();
		nY					= sYX.cL;
		sYX.cL				= nX;
		sYX.leftshift();
		nX					= sYX.cL;
		// 进行下一步
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
	// 先把nX和nY移动到最边上
	nX	= ((quadtree_ucoord_t)nX) << m_nDiffLevel;
	nY	= ((quadtree_ucoord_t)nY) << m_nDiffLevel;
	// 从根开始
	quadtree_level_t	nLevel=m_nMaxLevel;
	QUADTREENODE_T		*pNode=m_pRoot;
	QUADTREE_COORD_SHIFT_T	sYX;
	while( nLevel>0 )
	{
		// 判定坐标区间
		sYX.cH				= 0;
		sYX.cL				= nY;
		sYX.leftshift();
		nY					= sYX.cL;
		sYX.cL				= nX;
		sYX.leftshift();
		nX					= sYX.cL;
		// 如果相应的分支不存在则退出循环，返回当前节点
		if( !pNode->apChild[sYX.cH] )
		{
			break;
		}
		// 进行下一步
		pNode				= pNode->apChild[sYX.cH];
		nLevel				--;
	}
	return	pNode;
}
template<class _AllocationObj, class _AllocationObjOneSize, int _CHUNKSIZE>
int	QuadtreeMan<_AllocationObj, _AllocationObjOneSize, _CHUNKSIZE>::Delete(quadtree_coord_t nX, quadtree_coord_t nY, QUADTREE_PROCESS_FUNC_LEAF_T pReleaseFunc, void *pExtInfo)
{
	// 先查找到叶子节点
	QUADTREENODE_LEAF_T	*pLeaf	= FindLeaf(nX, nY);
	if( pLeaf )
	{
		// 先进行内容释放
		int	rst	= (*pReleaseFunc)(pLeaf, pExtInfo);
		if( rst!=0 )
		{
			return	rst;
		}
		// 再删除节点
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
	// 此时的父节点必定不为空的
	assert(pParent);
	pParent->apChild[pLeaf->nPosInParent]	= NULL;
	whbit_uchar_clr(&pParent->nChildMask, pLeaf->nPosInParent);
	// 删除叶子
	MyDelete(pLeaf);
	// 递归删除空的父节点
	while( 1 )
	{
		// 判断自己已经没有孩子了
		if( pParent->nChildMask != 0 )
		{
			// 有孩子，不用继续了
			break;
		}
		QUADTREENODE_T	*pThis	= pParent;
		pParent			= pThis->pParent;
		// 判断是否是root
		if( pParent == NULL )
		{
			m_pRoot		= NULL;
			// 删除自己
			MyDelete(pThis);
			// 退出循环
			break;
		}
		// 自己没有孩子了，应该删除了
		// 清空自己在父节点中的相关信息
		pParent->apChild[pThis->nPosInParent]	= NULL;
		whbit_uchar_clr(&pParent->nChildMask, pThis->nPosInParent);
		// 删除自己
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
	// 记录不变的变量
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
	// 先计算中心距离（这个叶子和非叶子都要用）
	quadtree_coord_t	nDistX	= quadtree_1_dist(m_bs_nCenterX, pNode->nX);
	quadtree_coord_t	nDistY	= quadtree_1_dist(m_bs_nCenterY, pNode->nY);
	// 如果是叶子节点则直接判断距离，然后处理，然后返回
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
	// 如果没有子节点就不继续了
	QUADTREENODE_T	*pRealNode	= (QUADTREENODE_T *)pNode;
	if( pRealNode->nChildMask == 0 )
	{
		return	0;
	}
	// 判断自己的区域否和指定区域有交集
	// 注：这样的判断方式会导致有效区域是中心偏右下的地方。如下图：
	// ++++++++
	// ++++++++
	// ++++o+++
	// ++++++++
	// 这样的区域表示o周围X距离4，Y距离2的范围
	quadtree_coord_t	nWidth	= ((quadtree_ucoord_t)1) << (pNode->nLevel-1);
	if( (nWidth+m_bs_nRadiusX) < nDistX
	||  (nWidth+m_bs_nRadiusY) < nDistY
	)
	{
		// 没有交集。不再继续了
		return	0;
	}
	// 有交集，则处理各个子节点
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
    // By JiangLi 非递归算法，未仔细测试过
    if(pNode == 0) return 0;

    QUADTREE_SEARCHBOX_JUDGE_FUNC_T pJudgeFunc = m_bs_pJudgeFunc;
    QUADTREENODE_CMN_T* stk[64];
    QUADTREENODE_CMN_T** pos = stk;
    *pos++ = pNode;

    while(pos > stk)
    {
        pNode = *--pos;

	    // 如果是叶子节点则直接判断距离，然后处理，然后返回
	    if( pNode->nLevel == 0  )
	    {
            // 逐个计算，如果不满足条件就跳出，可以减少一些运算
            quadtree_coord_t nDistX	= m_bs_nCenterX > pNode->nX ? m_bs_nCenterX-pNode->nX : pNode->nX-m_bs_nCenterX;
            if(nDistX > m_bs_nRadiusX) continue;
	        quadtree_coord_t nDistY	= m_bs_nCenterY > pNode->nY ? m_bs_nCenterY-pNode->nY : pNode->nY-m_bs_nCenterY;
            if(nDistY > m_bs_nRadiusY) continue;
			int	rst	= pJudgeFunc((QUADTREENODE_LEAF_T *)pNode, nDistX, nDistY, m_bs_pExtInfo);
			if(rst) return rst;
            continue;
	    }

	    // 如果没有子节点就不继续了
	    if(((QUADTREENODE_T*)pNode)->nChildMask == 0) continue;

	    // 判断自己的区域否和指定区域有交集
	    // 注：这样的判断方式会导致有效区域是中心偏右下的地方。如下图：
	    // ++++++++
	    // ++++++++
	    // ++++o+++
	    // ++++++++
	    // 这样的区域表示o周围X距离4，Y距离2的范围

		// 如果没有交集。不再继续了。
        // 使用无符号数来绕过绝对值的计算。
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
	// 记录不变的变量
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
		// 是叶子，那么求四角（扩展后的）会简单
		quadtree_coord_t	nL	= pNode->nX - m_rs_nExtendRange;
		quadtree_coord_t	nT	= pNode->nY - m_rs_nExtendRange;
		quadtree_coord_t	nR	= pNode->nX + m_rs_nExtendRange;
		quadtree_coord_t	nB	= pNode->nY + m_rs_nExtendRange;
		// 判断和自己是否相交，如果不相交就不继续了
		if( !whbox_check_intersection_with_line(nL, nT, nR, nB, m_rs_nStartX, m_rs_nStartY, m_rs_nEndX, m_rs_nEndY) )
		{
			return	0;
		}
		// 处理一下，然后返回
		return	(*m_rs_pJudgeFunc)((QUADTREENODE_LEAF_T *)pNode, m_rs_pExtInfo);
	}
	// 说明非叶子
	// 如果没有子节点也不用继续了
	QUADTREENODE_T	*pRealNode	= (QUADTREENODE_T *)pNode;
	if( pRealNode->nChildMask == 0 )
	{
		return	0;
	}
	// 获得当前节点的四角（扩展后的）
	quadtree_coord_t	nWidth	= ( ((quadtree_ucoord_t)1) << (pNode->nLevel-1) ) + m_rs_nExtendRange;
	quadtree_coord_t	nL		= pNode->nX - nWidth;
	quadtree_coord_t	nT		= pNode->nY - nWidth;
	quadtree_coord_t	nR		= pNode->nX + nWidth - 1;
	quadtree_coord_t	nB		= pNode->nY + nWidth - 1;
	// 判断和自己是否相交，如果不相交就不继续了
	if( !whbox_check_intersection_with_line(nL, nT, nR, nB, m_rs_nStartX, m_rs_nStartY, m_rs_nEndX, m_rs_nEndY) )
	{
		return	0;
	}
	// 根据起始点相对自己中心的位置确定遍历子节点的顺序
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
