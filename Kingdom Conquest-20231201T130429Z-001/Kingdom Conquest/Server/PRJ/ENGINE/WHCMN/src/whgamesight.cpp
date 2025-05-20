// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whgamesight.cpp
// Creator      : Wei Hua (κ��)
// Comment      : ����Ϸ����Ұ��صĹ��ܡ�����ͬ����
// CreationDate : 2006-06-06
// ChangeLOG    : 2006-06-06 �ƶ��ӿ�
//              : 2006-06-07 ���������ʵ��
//				  2006-06-30 ���ӻ�ȡGetUCIOutID�Ľӿ�,�ýӿ����ڻ�ȡUCI��outerID���б�,�б��в�����NPC
//				  2006-07-02 �޸�GetUCI_In��GetUCI_Out���صĽṹ,����Ӧ���ڲ�����.
//				  2006-07-19 ������һ��AddNpcFirst�ӿ�,���ڷ�������ʼ��ʱ,��û����ҵ�����µ���.
//								�ڲ��ṹ��д, ԭ��set<id,id,d>�洢ͬ���б�,����ʹ��map<id, ralation>�ṹ�洢.ralationʹ���˽����fast_allocator
//				  2006-10-25 ����TakeSpace()������UnTakeSpace()��IsTakeSpace()����������ռ�ؿ顣�޸�SearchFreePoint()��SearchFreePointNext()�ķ���ֵ���ͣ���bool��Ϊint��
//								�����ҵ���һ������ֲ��dummy�Ľڵ�����е������������ڲ���Ϊ�ڲ�С��4��dummy����Ϊ���нڵ㡣
//				  2007-03-09 �޸ģ�ʹû��dummy���Ĳ�����leaf��delete��������1���ƶ������ڣ�ͬһdummy��ε���dummyGO�ͻص�ԭ��������⡣
//				  2007-06-05 Ϊ���١��峤��ʧ���������ӱ���m_idFocused����ش���
//				  2007-06-13 Init()�����m_nMaxX������Ǵ���ģ���������by ���Ľ�
//				  2007-09-05 Ϊwhsightlist2D::SearchBox����һ�����أ����ڰѽ��������set�У�������Ҫ��ID����ĳ��ϣ�
//				  2007-12-20 ������Ҷ���ҵ���Ұ�����ޣ���ͨ��whsightlist2D::INFO_T::nMaxPlayerSeen�޸ģ�ȱʡΪһ���������ܿ�����Χ��100����ң��������Ӧ���ܿ����������������һ������Ŀ���ˣ���˫������Ұ�����ᷢ���ı�
//				  2007-12-20 ��������Ұ����Ϊ����Ұ����+2
//				  2008-04-17 ΪЧ�ʼ��������ش���
//				  2008-07-22 ����AddDummy��һ��û�м�鷵��ֵ�����Ĵ���
//				  2008-07-22 ע�͵�һЩ�Ѿ������Ĵ���
//				  2008-09-26 ��AddDummy��DummyGo���ж��������Ƿ�Խ�磬Խ���򷵻�-3

#include <math.h>
#include "../inc/whgamesight.h"
#include "../../../GAME/GameUtilityLib/inc/SpeedTest.h"

#define MAX_UCI_TABLE 100
//
template<typename T>
fast_plex fast_allocator<T>::m_plex(sizeof(T), 128);


//////////////////////////////////////////////////////////////////////////
//static	fast_plex<n_whcmn::whsightlist2D::idset_t>	gamesight_idset_allot(sizeof(n_whcmn::whsightlist2D::idset_t), 1024);	// set �ķ�����	


using namespace n_whcmn;


namespace
{

enum
{
    c_patch_size    = 32,               // �ֿ���32*32�ģ�����̫С��
    c_patch_level   = 5,                // �ֿ�Ľڵ�ȼ����ͷֿ��С�Ƕ�Ӧ�ġ�
    c_patch_shift   = c_patch_level,    // ����ͨ����������ֵ��������ֿ��������
    c_patch_need    = c_patch_size*4,   // �����÷ֿ�̫�٣����ٱ�֤4*4�����QTree�ĳߴ�С����������Ͳ��ֿ��ˡ�
    c_patch_stack   = 32,               // �ǵݹ��㷨��Ҫ��ջ��С��������(5+1)*3+1=19���趨Ϊ32��
};

struct SearchBoxCtx
{
    QTreeSearchBoxFunc m_func;
    QTree*  m_qtree;
    void*   m_ctx;
    short   m_x1, m_y1;
    short   m_x2, m_y2;
    short   m_x, m_y;

    int Execute(QTreeNode* pNode);
};

int SearchBoxCtx::Execute(QTreeNode* pNode)
{
    if(pNode == 0) return 0;

    // ջ�Ĵ�СӦ����level*3+1��
    // ����patchʱ����5*3+1��������6*3+1��
    QTreeNode* stk[c_patch_stack];
    QTreeNode** pos = stk;
    *pos++ = pNode;

    while(pos > stk)
    {
        pNode = *--pos;

        // ����½�
        if(m_x2 <= pNode->m_x) continue;
        if(m_y2 <= pNode->m_y) continue;

        // �ڵ���
        short s = (short)(1 << pNode->m_level);

        // ����Ͻ�
        if(m_x1 >= pNode->m_x+s) continue;
        if(m_y1 >= pNode->m_y+s) continue;

        if(pNode->m_level == 0)
        {
            // �����Ҷ�ӽڵ���ִ�лص�����
            short dx = m_x > pNode->m_x ? m_x-pNode->m_x : pNode->m_x-m_x;
            short dy = m_y > pNode->m_y ? m_y-pNode->m_y : pNode->m_y-m_y;
            int rst = m_func(static_cast<QTreeLeaf*>(pNode), dx, dy, m_ctx);
            if(rst) return rst;
            continue;
        }

        // ���ַ�ʽһ�����ã�ȥ����ѭ������������ġ�
        switch(pNode->m_mask)
        {
        case 0: default:
            break;
        case 1:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[0];
            break;
        case 2:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[1];
            break;
        case 3:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[0];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[1];
            break;
        case 4:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[2];
            break;
        case 5:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[0];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[2];
            break;
        case 6:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[1];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[2];
            break;
        case 7:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[0];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[1];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[2];
            break;

        case 8:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[3];
            break;
        case 9:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[0];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[3];
            break;
        case 10:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[1];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[3];
            break;
        case 11:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[0];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[1];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[3];
            break;
        case 12:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[2];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[3];
            break;
        case 13:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[0];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[2];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[3];
            break;
        case 14:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[1];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[2];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[3];
            break;
        case 15:
            *pos++ = ((QTreeTrunk*)pNode)->m_child[0];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[1];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[2];
            *pos++ = ((QTreeTrunk*)pNode)->m_child[3];
            break;
        }

	    // ���û���ӽڵ�Ͳ�������
        //if(pNode->m_mask == 0) continue;

        //for(int i = 0; i < 4; ++ i)
        //{
        //    QTreeNode* n = ((QTreeTrunk*)pNode)->m_child[i];
        //    if(n)
        //    {
        //        *pos++ = n;
        //    }
        //}
    }

    return 0;
}

struct SearchRayCtx
{
    QTreeSearchRayFunc m_func;
    QTree*  m_qtree;
    void*   m_ctx;
    short   m_x1, m_y1;
    short   m_x2, m_y2;
    short   m_range;

    int Execute(QTreeNode* pNode);
};

int SearchRayCtx::Execute(QTreeNode* pNode)
{
	if(pNode == NULL) return 0;

	if(pNode->m_level == 0)
	{
		// ��Ҷ�ӣ���ô���Ľǣ���չ��ģ����
		short nL = pNode->m_x - m_range;
		short nT = pNode->m_y - m_range;
		short nR = pNode->m_x + m_range;
		short nB = pNode->m_y + m_range;

		// �жϺ��Լ��Ƿ��ཻ��������ཻ�Ͳ�������
		if(!whbox_check_intersection_with_line(nL, nT, nR, nB, m_x1, m_y1, m_x2, m_y2))
		{
			return	0;
		}

		// ����һ�£�Ȼ�󷵻�
		return	m_func(static_cast<QTreeLeaf*>(pNode), m_ctx);
	}

	// û���ӽڵ�ͷ���
	if(pNode->m_mask == 0) return 0;

	// ��õ�ǰ�ڵ���Ľǣ���չ��ģ�
    short ns = (1 << pNode->m_level) + m_range;
	short nL = pNode->m_x - m_range;
	short nT = pNode->m_y - m_range;
	short nR = pNode->m_x + ns;
	short nB = pNode->m_y + ns;

	// �жϺ��Լ��Ƿ��ཻ��������ཻ�Ͳ�������
	if( !whbox_check_intersection_with_line(nL, nT, nR, nB, m_x1, m_y1, m_x2, m_y2) )
	{
		return	0;
	}

	// ������ʼ������Լ����ĵ�λ��ȷ�������ӽڵ��˳��
	#define	QTREE_SEARCHRAY(i)		\
	{							\
		int	rst	= Execute(((QTreeTrunk*)pNode)->m_child[i]);	\
		if( rst!=0 )			\
		{						\
			return	rst;		\
		}						\
	}							\
	// EOF QTREE_SEARCHRAY

	if( m_x1<pNode->m_x )
	{
		if( m_y1<pNode->m_y )
		{
			// LT	0 1 2 3
			QTREE_SEARCHRAY(0);
			QTREE_SEARCHRAY(1);
			QTREE_SEARCHRAY(2);
			QTREE_SEARCHRAY(3);
		}
		else
		{
			// LB	2 0 3 1
			QTREE_SEARCHRAY(2);
			QTREE_SEARCHRAY(0);
			QTREE_SEARCHRAY(3);
			QTREE_SEARCHRAY(1);
		}
	}
	else
	{
		if( m_y1<pNode->m_y )
		{
			// RT	1 3 0 2
			QTREE_SEARCHRAY(1);
			QTREE_SEARCHRAY(3);
			QTREE_SEARCHRAY(0);
			QTREE_SEARCHRAY(2);
		}
		else
		{
			// RB	3 2 1 0
			QTREE_SEARCHRAY(3);
			QTREE_SEARCHRAY(2);
			QTREE_SEARCHRAY(1);
			QTREE_SEARCHRAY(0);
		}
	}
	return	0;
}

// �������꣬���طֿ�ڵ㣬����ֿ鲻���ڣ��򷵻ظ��ڵ㡣
QTreeTrunk* QTree_PrepareTrunk(QTree* t, short x, short y)
{
    // ʹ���޷����������Իر�С��0���ж�
    if((unsigned short)x >= t->m_size || (unsigned short)y >= t->m_size) return 0;

    if(t->m_patch)
    {
        QTreeTrunk* p = t->m_patch[x >> c_patch_shift][y >> c_patch_shift];
        if(p)
        {
            return p;
        }
    }

    return t->m_root;
}

void QTree_DestroyNode(QTree* t, QTreeNode* p)
{
    if(p == 0) return;

    if(p->m_level == 0)
    {
        t->m_mgr->DeallocateLeaf(static_cast<QTreeLeaf*>(p));
        return;
    }

    for(int i = 0; i < 4; ++ i)
    {
        QTree_DestroyNode(t, static_cast<QTreeTrunk*>(p)->m_child[i]);
    }

    t->m_mgr->DeallocateTrunk(static_cast<QTreeTrunk*>(p));
}

} // namespace

QTree::QTree()
{
    m_mgr   = 0;
    m_root  = 0;
    m_patch = 0;
    m_size  = 0;
}

QTree::~QTree()
{
    Destroy();
}

bool QTree::Create(QTreeNodeMgr* mgr, int max_level)
{
    // ò�ƴ���������ֵ�����⣬���ˡ���ʱ���ܣ�����΢�˷ѵ��ڴ档
    // -- max_level;

    m_mgr   = mgr;
    m_size  = short(1 << max_level); // ����Ĵ�С

    // ���ڵ�ʼ�մ��ڣ�����ɾ��
    m_root  = mgr->AllocateTrunk();
    m_root->m_parent    = 0;
	m_root->m_x         = 0;
    m_root->m_y         = 0;
	m_root->m_level     = (unsigned char)max_level;
	m_root->m_index     = 0;
    m_root->m_mask      = 0;
    m_root->m_child[0]  = 0;
    m_root->m_child[1]  = 0;
    m_root->m_child[2]  = 0;
    m_root->m_child[3]  = 0;

    m_patch = 0;
    if(m_size < c_patch_need) return true;

    // �ֿ�����Ĵ�С
    int cnt = m_size >> c_patch_shift;
    // �������ַ�ʽ������ֻ����һ���ڴ档
    size_t sz = sizeof(void*)*(cnt+cnt*cnt);
    m_patch = (QTreeTrunkPtr**)::malloc(sz);
    memset(m_patch, 0, sz);

    QTreeTrunkPtr* p = (QTreeTrunkPtr*)(m_patch+cnt);
    for(int i = 0; i < cnt; ++ i)
    {
        m_patch[i] = p;
        p += cnt;
    }

    return true;
};

void QTree::Destroy()
{
    if(m_mgr == 0) return;

    QTree_DestroyNode(this, m_root);
    if(m_patch)
    {
        ::free(m_patch);
        m_patch = 0;
    }
    m_mgr   = 0;
    m_size  = 0;
    m_root  = 0;
}

//int	QTree::Goover(QTreeTrunkFunc func_trunk, QTreeLeafFunc func_leaf, void* ctx)
//{
//    GooverCtx gctx;
//    gctx.m_func_trunk   = func_trunk;
//    gctx.m_func_leaf    = func_leaf;
//    gctx.m_qtree        = this;
//    gctx.m_ctx          = ctx;
//    return gctx.Execute(m_root);
//}

QTreeLeaf* QTree::Insert(short x, short y)
{
    QTreeNode* p = QTree_PrepareTrunk(this, x, y);
    if(p == 0) return 0;

    int level = p->m_level;
    while(level)
    {
        -- level;
        if(level == 0)
        {
            int i = ((y << 1)&2) + (x&1);
            QTreeLeaf* f = static_cast<QTreeLeaf*>(static_cast<QTreeTrunk*>(p)->m_child[i]);
            if(f) return f;

            f = m_mgr->AllocateLeaf();
            f->m_parent = static_cast<QTreeTrunk*>(p);
            f->m_index  = (unsigned char)i;
            f->m_x      = x;
            f->m_y      = y;
            f->m_level  = 0;
            f->m_mask   = 0;
            p->m_mask   |= (1 << i);
            static_cast<QTreeTrunk*>(p)->m_child[i] = f;
            return f;
        }

        int i = ((y >> (level-1))&2) + ((x >> level)&1);
        QTreeTrunk* k = static_cast<QTreeTrunk*>(static_cast<QTreeTrunk*>(p)->m_child[i]);
        if(k)
        {
            p = k;
            continue;
        }

        k = m_mgr->AllocateTrunk();
        k->m_parent     = static_cast<QTreeTrunk*>(p);
        k->m_index      = (unsigned char)i;
        k->m_x          = short((x >> level) << level);
        k->m_y          = short((y >> level) << level);
        k->m_level      = (unsigned char)level;
        k->m_mask       = 0;
        k->m_child[0]   = 0;
        k->m_child[1]   = 0;
        k->m_child[2]   = 0;
        k->m_child[3]   = 0;
        p->m_mask   |= (1 << i);
        static_cast<QTreeTrunk*>(p)->m_child[i] = k;
        if(level == c_patch_level && m_patch)
        {
            // ��ӵ�����ֿ��
            m_patch[x >> c_patch_shift][y >> c_patch_shift] = k;
        }
        p = k;
    }

	return static_cast<QTreeLeaf*>(p);
}

QTreeLeaf* QTree::FindLeaf(short x, short y)
{
    QTreeNode* p = QTree_PrepareTrunk(this, x, y);
    if(p == 0) return 0;

    int level = p->m_level;
    while(level)
    {
        -- level;
        int i = (((y << 1) >> level)&2) + ((x >> level)&1);
        p = static_cast<QTreeTrunk*>(p)->m_child[i];
        if(p == 0)
        {
            break;
        }
    }

    return static_cast<QTreeLeaf*>(p);
}

//QTreeNode* QTree::FindNode(short x, short y)
//{
//    QTreeTrunk* p = QTree_PrepareTrunk(this, x, y);
//    if(p == 0) return 0;
//
//    int level = p->m_level;
//    while(level)
//    {
//        -- level;
//        int i = (((y << 1) >> level)&2) + ((x >> level)&1);
//        QTreeNode* t = p->m_child[i];
//        if(t == 0)
//        {
//            break;
//        }
//        p = static_cast<QTreeTrunk*>(t);
//    }
//
//    return p;
//}

void QTree::Delete(short x, short y)
{
	// �Ȳ��ҵ�Ҷ�ӽڵ�
	QTreeLeaf* p = FindLeaf(x, y);
    if(p == 0) return;

	Delete(p);
}

void QTree::Delete(QTreeLeaf *p)
{
    if(p == 0) return;

    QTreeTrunk* parent = p->m_parent;
    assert(parent);

    parent->m_child[p->m_index] = 0;
    parent->m_mask &= ~(1 << p->m_index);

	// ɾ��Ҷ��
    m_mgr->DeallocateLeaf(p);

	// ѭ��ɾ���յĸ��ڵ�
	while(parent->m_mask == 0)
	{
        if(parent == m_root)
        {
            // ��ɾ�����ڵ㡣
			break;
        }

        QTreeTrunk* t = parent;
		parent = parent->m_parent;

        // ����Լ��ڸ��ڵ��е������Ϣ
		parent->m_child[t->m_index] = 0;
        parent->m_mask &= ~(1 << t->m_index);

        if(t->m_level == c_patch_level && m_patch)
        {
            // �ӻ���ķֿ���ȥ����
            m_patch[t->m_x >> c_patch_shift][t->m_y >> c_patch_shift] = 0;
        }
        // ɾ���Լ�
		m_mgr->DeallocateTrunk(t);
	};
}

int	QTree::SearchBox(short x, short y, short rx, short ry, QTreeSearchBoxFunc func, void* ctx)
{
    if(rx == 0 && ry == 0)
    {
        // ����һ���㣬���Ż��㷨
        if((unsigned short)x >= m_size || (unsigned short)y >= m_size) return 0;
        QTreeNode* p = m_patch ? m_patch[x >> c_patch_shift][y >> c_patch_shift] : m_root;

        while(p)
        {
            if(p->m_level == 0)
            {
                // �����Ҷ�ӽڵ���ִ�лص�����
                short dx = x > p->m_x ? x-p->m_x : p->m_x-x;
                short dy = y > p->m_y ? y-p->m_y : p->m_y-y;
                return func(static_cast<QTreeLeaf*>(p), dx, dy, ctx);
            }

            int s = p->m_level-1;
            p = static_cast<QTreeTrunk*>(p)->m_child[(((y << 1) >> s)&2) + ((x >> s)&1)];
        }
        return 0;
    }

    // ����һ������
    SearchBoxCtx sctx;
    sctx.m_func     = func;
    sctx.m_qtree    = this;
    sctx.m_ctx      = ctx;

    // �߽����ͼ��
    sctx.m_x1       = x-rx;
    if(sctx.m_x1 >= m_size) return 0;
    if(sctx.m_x1 < 0) sctx.m_x1 = 0;
    sctx.m_y1       = y-ry;
    if(sctx.m_y1 >= m_size) return 0;
    if(sctx.m_y1 < 0) sctx.m_y1 = 0;
    // �߽���Ҫ��1���Ǹ�����ҿ������䡣
    sctx.m_x2       = x+rx+1;
    if(sctx.m_x2 <= 0) return 0;
    if(sctx.m_x2 > m_size) sctx.m_x2 = m_size;
    sctx.m_y2       = y+ry+1;
    if(sctx.m_y2 <= 0) return 0;
    if(sctx.m_y2 > m_size) sctx.m_y2 = m_size;
    sctx.m_x        = x;
    sctx.m_y        = y;

    if(m_patch == 0)
    {
        // �����֧˵��������С������Ҫ�ֿ��ˡ�
        return sctx.Execute(m_root);
    }

    int ix1 = int(sctx.m_x1) >> c_patch_shift;
    int iy1 = int(sctx.m_y1) >> c_patch_shift;
    int ix2 = int(sctx.m_x2+c_patch_size-1) >> c_patch_shift;
    int iy2 = int(sctx.m_y2+c_patch_size-1) >> c_patch_shift;

    for(int ix = ix1; ix < ix2; ++ ix)
    {
        for(int iy = iy1; iy < iy2; ++ iy)
        {
            int r = sctx.Execute(m_patch[ix][iy]);
            if(r != 0)
            {
                return r;
            }
        }
    }

    return 0;
}

int	QTree::SearchRay(short x1, short y1, short x2, short y2, short range, QTreeSearchRayFunc func, void* ctx)
{
    SearchRayCtx sctx;

    sctx.m_func     = func;
    sctx.m_qtree    = this;
    sctx.m_ctx      = ctx;

    // �߽����ͼ��
    sctx.m_x1       = x1;
    sctx.m_y1       = y1;
    sctx.m_x2       = x2;
    sctx.m_y2       = y2;
    sctx.m_range    = range;

    return sctx.Execute(m_root);
}

struct QTreeSight : QTreeLeaf
{
    // ÿ��Ҷ�Ӷ�Ҫ����idset_t��û�б�Ҫ�ġ����������Ը���Ч������Լ��
    whsightlist2D::idset_t  m_data;
};

struct SightQTreeNodeMgr : QTreeNodeMgr
{
    struct page
    {
        page*   m_next;
    };

    page*   m_page;
    size_t  m_page_size;
    char*   m_pos;
    char*   m_end;

    QTreeSight* m_sight_free;
    size_t      m_sight_used;
    QTreeTrunk* m_trunk_free;
    size_t      m_trunk_used;

    SightQTreeNodeMgr();
    ~SightQTreeNodeMgr();

    void* Allocate(size_t c);

    virtual QTreeTrunk* AllocateTrunk();
    virtual void DeallocateTrunk(QTreeTrunk*);

    virtual QTreeLeaf* AllocateLeaf();
    virtual void DeallocateLeaf(QTreeLeaf*);

    template<class T>
	static void construct(T* p)
	{
		new(p) T;
	}
    template<class T>
	static void destruct(T* p)
	{
		NOTUSE(p);
		p->~T();
	}
};

static SightQTreeNodeMgr g_SightQTreeNodeMgr;

SightQTreeNodeMgr::SightQTreeNodeMgr()
{
    // ȱʡ��512K
    m_page_size = 512000;

    m_page  = 0;
    m_pos   = 0;
    m_end   = 0;

    m_sight_free    = 0;
    m_sight_used    = 0;
    m_trunk_free    = 0;
    m_trunk_used    = 0;
}

SightQTreeNodeMgr::~SightQTreeNodeMgr()
{
    // ����ȫ���ͷţ�������ζ���ڴ�й¶
    assert(m_sight_used == 0 && m_trunk_used == 0);

    while(m_page)
    {
        page* t = m_page;
        m_page = m_page->m_next;
        ::free(t);
    }
}

void* SightQTreeNodeMgr::Allocate(size_t c)
{
    size_t r = m_end-m_pos;
    if(r >= c)
    {
        void* p = m_pos;
        m_pos += c;
        return p;
    }

    page* pg = (page*)::malloc(sizeof(page)+m_page_size);
    pg->m_next = m_page;
    m_page = pg;
    m_pos = (char*)(pg+1);
    m_end = m_pos+m_page_size;

    void* p = m_pos;
    m_pos += c;
    return p;
}

QTreeTrunk* SightQTreeNodeMgr::AllocateTrunk()
{
    ++m_trunk_used;
    if(m_trunk_free)
    {
        QTreeTrunk* t = m_trunk_free;
        m_trunk_free  = t->m_parent;
        return t;
    }
    return (QTreeTrunk*)Allocate(sizeof(QTreeTrunk));
}

void SightQTreeNodeMgr::DeallocateTrunk(QTreeTrunk* t)
{
    --m_trunk_used;
    t->m_parent  = m_trunk_free;
    m_trunk_free = t;
}

QTreeLeaf* SightQTreeNodeMgr::AllocateLeaf()
{
    ++m_sight_used;
    if(m_sight_free)
    {
        QTreeSight* t = m_sight_free;
        m_sight_free  = (QTreeSight*)t->m_parent;
        construct(&t->m_data);
        return t;
    }
    QTreeSight* t = (QTreeSight*)Allocate(sizeof(QTreeSight));
    construct(&t->m_data);
    return t;
}

void SightQTreeNodeMgr::DeallocateLeaf(QTreeLeaf* t)
{
    --m_sight_used;
    destruct(&static_cast<QTreeSight*>(t)->m_data);
    t->m_parent  = (QTreeTrunk*)m_sight_free;
    m_sight_free = static_cast<QTreeSight*>(t);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

int	quadtree_searchBox_addDummy( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY, void * pExtInfo )
{
	return	( (whsightlist2D *)pExtInfo )->_OnAddDum( pLeaf, nDistX, nDistY );
}

int	quadtree_searchBox_dummyGo( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY, void * pExtInfo )
{
	return	( (whsightlist2D *)pExtInfo )->_OnProDumGo( pLeaf, nDistX, nDistY );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

whsightlist2D::whsightlist2D() //: idset_allot( sizeof(idset_t), 1024 )
//:m_uciCountIn(0)
//,m_uciCountOut(0)
//,m_isFocused(false)
{
	m_qtree = NULL;
	m_bGetting		= false;
}

whsightlist2D::~whsightlist2D()
{
	Release();
}

// ��ʼ���Ͳ�������
int	whsightlist2D::Init( INFO_T * pInfo )
{
	memcpy( & m_info, pInfo, sizeof(m_info) );

	// ��ʼ��������
	if( m_Dummys.Init( & m_cao, m_info.nMaxDummy, m_info.nChunkSize ) < 0 )	{
		return	-1;
	}

	// ��ʼ���Ĳ���
	m_qtree = new QTree();

	// û�ռ���
	if ( ! m_qtree ) {
		return	-2;	
	}
	m_qtree->Create(&g_SightQTreeNodeMgr, m_info.nLodLevel);

	m_tNextTick = wh_gettickcount() + m_info.nGoCheckDelay;

	m_nMaxX = (1 << m_info.nLodLevel) - 1;
	m_nMaxY = m_nMaxX;

	nMaxLeaveSightX = pInfo->nMaxSightX + 2;
	nMaxLeaveSightY = pInfo->nMaxSightY + 2;

	m_info.nMaxPlayerSeen = (pInfo->nMaxPlayerSeen > 0) ? pInfo->nMaxPlayerSeen : MAX_UCI_TABLE;

	return	0;
}

// �ս�
int	whsightlist2D::Release()
{
	if ( m_qtree ) {
		m_qtree->Destroy();
		delete	m_qtree;
		m_qtree = NULL;
	}

	m_Dummys.Release();
	return	0;
}

// �߼����У�������ʱ���ƶ��жϣ�
int	whsightlist2D::Tick()
{
	whtick_t	tickNow			= wh_gettickcount();

	// ʱ�䵽��
	if ( tickNow >= m_tNextTick  ) {
		{
			CPU_GUARD(GameSight_UpdateQuadtree);
			_UpdateQuadtree();
		}
		// ����ʱ��������Ƿ��ж���
		for ( idsetitor_t it= m_updateSet_Move.begin(); it != m_updateSet_Move.end(); ++it ) {
			
			_OnDumGo( *it );
		}

		m_updateSet_Move.clear();
		m_tNextTick					= wh_gettickcount() + m_info.nGoCheckDelay;
		return 0;
	}
	return	1;
}

// ������(��NPC)������ID
// int nX, int nY�ǳ�ʼ��λ��
// Ĭ�ϵ������Ұ�������Ұ
int	whsightlist2D::AddDummy( whsightlist2D::DUMMY_INFO_T *pDummyInfo, int nX, int nY )
{
	if (nX > m_nMaxX || nX < 0 || nY > m_nMaxY || nY < 0)
	{
		return -3;
	}

	// ���������nothing��ʱ��,�޶���,ֱ�ӷ��ش���.
	if ( pDummyInfo->nType == DUMMY_INFO_T::TYPE_NOTHING ) {
		return	-2;
	}

	
	// ����������,npc,�ϵ۵�ʱ��,�����ռ�.
	int		nID;
	DummyUnit	*pDummy	= m_Dummys.Alloc( & nID );

	//if(m_idFocused == -1)
		//m_idFocused = nID;

	// û�пռ���
	if( !pDummy )	{
		return	-1;
	}
	// �ȳ�ʼ��һ������
	pDummy->clear();
	// ���������Ϣ
	memcpy( &pDummy->info, pDummyInfo, sizeof(pDummy->info) );


	
	// ���������god��ʱ��, ��ֱ�����god�ɼ���������.�������˲��ɼ�god.god ��Ұ����Ϊ����(-1).������.
	//		�򲻻�����Ĳ�����صĺ���.
	if ( pDummyInfo->nType == DUMMY_INFO_T::TYPE_GOD ) {
		m_godset.insert(nID) ;
		return	nID;
	}

	//---------------------------
	// �������֮��Ŀɼ����.

	// ��������
	pDummy->nX		= pDummy->nNewX		= nX;
	pDummy->nY		= pDummy->nNewY		= nY;


	QTreeLeaf	*pLeaf = m_qtree->Insert( nX, nY );
	if (pLeaf == NULL)
	{
		return -3;
	}

	//static char	str[1024];
	//sprintf( str, "add ���   type= %d, id= %d, X= %d, Y= %d, newX= %d, newY= %d\n", pDummy->info.nType, nID, pDummy->nX, pDummy->nY, pDummy->nNewX, pDummy->nNewY ); 
	//OutputDebugString( str );

	static_cast<QTreeSight*>(pLeaf)->m_data.insert(nID);

	// ���dummy�Ĺ�ϵmap��
	static beSee			dummyRela;
	m_CurDumItor	= m_dummyRelaMap.insert( std::make_pair( nID, dummyRela ) ).first;
	m_pCurDummy		= pDummy;

	// ��quadtree_searchBox_judeg_fun_t�е���������Ա�ĺ���_addToSet, �������Ұ����,���ѵ��Ľ���ﻥ��Ƚϡ�
	m_qtree->SearchBox( pDummy->nX, pDummy->nY, m_info.nMaxSightX, m_info.nMaxSightY, quadtree_searchBox_addDummy, this );
	
	m_pCurDummy = NULL;
	m_CurDumItor = m_dummyRelaMap.end();

	return	nID;
}

// ����������
const whsightlist2D::DUMMY_INFO_T *	whsightlist2D::GetDummy( int nID )
{
	DummyUnit	*pDummy	= m_Dummys.GetByID(nID);
	if( !pDummy )	{
		return	NULL;
	}	
	return		& pDummy->info;
}

// ɾ�����
int	whsightlist2D::DelDummy( int nID )
{
	DummyUnit	*pDummy	= m_Dummys.GetByID(nID);
	if( !pDummy )	{
		return	-1;
	}	

	// ɾ���ϵ�
	if ( pDummy->info.nType == DUMMY_INFO_T::TYPE_GOD ) {
		m_godset.erase( nID );
		return	m_Dummys.Free(nID);
	}


	durlapitor_t itMap;
	if ( ( itMap  = m_dummyRelaMap.find(nID) )    == m_dummyRelaMap.end()  ) {
		return	-2;
	}

	//uciout_t	uciOut;
	UCI_T	uciOut;
	uciOut.nD = -1;


	// ɾ��player
	if ( pDummy->info.nType == DUMMY_INFO_T::TYPE_PLAYER ) {

		idsetitor_t	itBeSeeID = itMap->second.player.begin();

		for ( ; itBeSeeID != itMap->second.player.end(); ++ itBeSeeID ) {
			m_dummyRelaMap[*itBeSeeID].player.erase(nID);

			uciOut.nI = nID;
			uciOut.nU = *itBeSeeID;
			//m_uciOut.insert(uciOut);
			add_uci_item(uciOut);
		}

		 itBeSeeID= itMap->second.npc.begin();

		for ( ; itBeSeeID != itMap->second.npc.end(); ++itBeSeeID ) {
			m_dummyRelaMap[*itBeSeeID].player.erase(nID);
			m_dummyRelaMap[*itBeSeeID].npc.erase(nID);
		}
	}
	

	// ɾ��npc
	else {
		idsetitor_t	itBeSeeID = itMap->second.player.begin();

		for ( ; itBeSeeID != itMap->second.player.end(); ++ itBeSeeID ) {
			m_dummyRelaMap[*itBeSeeID].npc.erase(nID);

			uciOut.nI = nID;
			uciOut.nU = *itBeSeeID;
			//m_uciOut.insert(uciOut);
			add_uci_item(uciOut);
		}
	}

	// ��map��ɾ��
	m_dummyRelaMap.erase( itMap );

	//	��ȡ��id�ĵ�.���Ĳ�����ɾ��	
	QTreeLeaf	*pLeaf = m_qtree->FindLeaf( pDummy->nX, pDummy->nY );
	
	//----------------------------------------------------------------------------------------------
	// Ҷ�Ӵ���
	if ( pLeaf )
    {
        idset_t& ids = static_cast<QTreeSight*>(pLeaf)->m_data;
        ids.erase(nID);
        if(ids.empty())
        {
			m_qtree->Delete( pLeaf );
			// static char	str[1024];
			// sprintf( str, "��leaf NULL  type= %d, id= %d, X= %d, Y= %d, newX= %d, newY= %d\n", pDummy->info.nType, nID, pDummy->nX, pDummy->nY, pDummy->nNewX, pDummy->nNewY ); 
        }
	}
	//----------------------------------------------------------------------------------------------

	//if ( pLeaf && pLeaf->pData ) {
	//	( (idset_t *) (pLeaf->pData) )->erase(nID);
	//}
	
	return	m_Dummys.Free(nID);
}

int	whsightlist2D::_OnAddDum( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY )
{
	if ( ! m_pCurDummy ) {
		return	-1;
	}

	int	id = m_CurDumItor->first;
	int	dist = nDistX + nDistY;

	//uciin_t		uciIn;
	UCI_T		uciIn;
	uciIn.nD	= dist;

	DummyUnit	* pDummyOther = NULL;
	int		idOther = -1;	


	// ��ȡ���ڵ��Ĳ�����Ҷ�������е�id.

    idset_t* pIdset = &static_cast<QTreeSight*>(pLeaf)->m_data;
	idsetitor_t it = pIdset->begin();

	// ����²������NPC
	switch ( (m_pCurDummy->info).nType )
	{
	case DUMMY_INFO_T::TYPE_NPC:
		{

			// ������Χ�������
			for ( ; it != pIdset->end(); ++ it  )
			{
				// �Ƿ����Լ�
				idOther  = * it;
				if ( id != idOther )
				{
					pDummyOther = m_Dummys.GetByID( idOther );
					assert(pDummyOther);

					switch((pDummyOther->info).nType)
					{
					// �²�������NPC����Χ������NPC����������pDummyOther��NPC��m_pCurDummy��NPC��
					case DUMMY_INFO_T::TYPE_NPC:
						continue;
						break;
					// �²�������NPC����Χ���������,�������Ҳ��뵽NPC��beSee�ֶΡ���pDummyOther����ң�m_pCurDummy��NPC��
					case DUMMY_INFO_T::TYPE_PLAYER:
					case DUMMY_INFO_T::TYPE_BOOTH:
						{
							(m_CurDumItor->second).player.insert(idOther);		// ���⣺ m_pCurDummy �� pDummyOther����
							m_dummyRelaMap[idOther].npc.insert(id);

							uciIn.nI = id;
							uciIn.nU = idOther;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);
						}
						break;
					default:
						;
					}
				}
			}
		}
		break;

	//////////////////////////////////////////////////////////////////////////

	// ����²���������
	case DUMMY_INFO_T::TYPE_PLAYER:
		{
			// ������Χ�������
			for ( ; it != pIdset->end(); ++ it  )
			{
				// �Ƿ����Լ�
				idOther  = * it;
				if ( id != idOther )
				{
					pDummyOther = m_Dummys.GetByID( idOther );
					assert(pDummyOther);

					// �²���������ҵ���Χ������NPC, �Ǿ�ֻ��������NPC���뵽����ֶΡ���pDummyOther��NPC��m_pCurDummy����ң�
					switch ( (pDummyOther->info).nType )
					{
					case DUMMY_INFO_T::TYPE_NPC:
						{
							m_dummyRelaMap[idOther].player.insert(id);	
							(m_CurDumItor->second).npc.insert(idOther);						// ���⣺ m_pCurDumm y�� pDummyOther����

							uciIn.nI = idOther;
							uciIn.nU = id;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);
						}
						break;

					// �²���������ҵ���Χ��������ң�������룬��pDummyOther����ң�m_pCurDummy����ң�
					case DUMMY_INFO_T::TYPE_PLAYER:
						if ((m_CurDumItor->second).player.size() < m_info.nMaxPlayerSeen && m_dummyRelaMap[idOther].player.size() < m_info.nMaxPlayerSeen)
						{		
							(m_CurDumItor->second).player.insert( idOther );		// ���⣺  pDummyOther �� m_pCurDummy ����
							m_dummyRelaMap[idOther].player.insert( id );	

							uciIn.nI = id;
							uciIn.nU = idOther;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);

							uciIn.nI = idOther;
							uciIn.nU = id;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);
						}
						break;
					// �²���������ҵ���Χ������̯λ��������룬��pDummyOther��̯λ��m_pCurDummy����ң�
					case DUMMY_INFO_T::TYPE_BOOTH:
						{		
							(m_CurDumItor->second).booth.insert( idOther );		// ���⣺  pDummyOther �� m_pCurDummy ����
							m_dummyRelaMap[idOther].player.insert( id );	

							uciIn.nI = id;
							uciIn.nU = idOther;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);

							uciIn.nI = idOther;
							uciIn.nU = id;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);
						}
						break;
					default:
						;
					}
				}
			}
		}
		break;
	// ����²��������ҵ�̯λ
	case DUMMY_INFO_T::TYPE_BOOTH:
		{
			// ������Χ�������
			for ( ; it != pIdset->end(); ++ it  )
			{
				// �Ƿ����Լ�
				idOther  = * it;
				if ( id != idOther )
				{
					pDummyOther = m_Dummys.GetByID( idOther );
					assert(pDummyOther);

					// �²�������̯λ����Χ������NPC, �Ǿ�ֻ��������NPC���뵽����ֶΡ���pDummyOther��NPC��m_pCurDummy����ң�
					switch ( (pDummyOther->info).nType )
					{
					case DUMMY_INFO_T::TYPE_NPC:
						{
							m_dummyRelaMap[idOther].player.insert(id);	
							(m_CurDumItor->second).npc.insert(idOther);						// ���⣺ m_pCurDumm y�� pDummyOther����

							uciIn.nI = idOther;
							uciIn.nU = id;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);
						}
						break;

					// �²�������̯λ����Χ��������ң�������룬��pDummyOther����ң�m_pCurDummy��̯λ��
					case DUMMY_INFO_T::TYPE_PLAYER:
						if ((m_CurDumItor->second).player.size() < m_info.nMaxPlayerSeen && m_dummyRelaMap[idOther].player.size() < m_info.nMaxPlayerSeen)
						{		
							(m_CurDumItor->second).player.insert( idOther );		// ���⣺  pDummyOther �� m_pCurDummy ����
							m_dummyRelaMap[idOther].booth.insert( id );	

							uciIn.nI = id;
							uciIn.nU = idOther;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);

							uciIn.nI = idOther;
							uciIn.nU = id;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);
						}
						break;
					// �²�������̯λ����Χ������̯λ��������룬��pDummyOther��̯λ��m_pCurDummy��̯λ��
					case DUMMY_INFO_T::TYPE_BOOTH:
						{		
							(m_CurDumItor->second).booth.insert( idOther );		// ���⣺  pDummyOther �� m_pCurDummy ����
							m_dummyRelaMap[idOther].booth.insert( id );	

							uciIn.nI = id;
							uciIn.nU = idOther;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);

							uciIn.nI = idOther;
							uciIn.nU = id;

							//m_uciIn.insert( uciIn );
							add_uci_item(uciIn);
						}
						break;
					default:
						;
					}
				}
			}
		}
		break;
	default:
		;
	}

	return	0;
}

////////////////////////////////////////////////////////////////////////////

// ����˶�
int	whsightlist2D::DummyGo( int nID, int nX, int nY )
{
	//if( nID == m_idFocused )
		//printf("�峤�ƶ���������(%d, %d)\n", nX, nY); 

	if (nX > m_nMaxX || nX < 0 || nY > m_nMaxY || nY < 0)
	{
		return -3;
	}

	DummyUnit	*pDummy	= m_Dummys.GetByID(nID);
	if ( ! pDummy ) {
		return	-1;
	}

	//	 GOD�ƶ�������Լ�������.�������ɼ�,���Ŀɼ��б���䶯.����ֻ�����Լ�������.
	if ( DUMMY_INFO_T::TYPE_GOD == (pDummy->info).nType ) {
		pDummy->nX = pDummy->nNewX = nX;
		pDummy->nY = pDummy->nNewY = nY;
		return	0;
	}

	// GOD�����dummy,û�����ƶ������
	if( (nX ==  pDummy->nNewX) && (nY == pDummy->nNewY) ) {	
		return	0;
	}

	// �����dummy�ƶ���һ����λ��,��û�м���,Ȼ�����ƶ���һ��λ��,�����ø�dummyΪû���ƶ���,���ƶ�������ɾ��
	if ( (pDummy->nX == nX) && (pDummy->nY == nY) ) { 
		m_updateSet_Move.erase( nID );
		return	0;
	}

	// ��¼������
	pDummy->nNewX   = nX;
	pDummy->nNewY	= nY;


	// ���ƶ����Ĳ��뵽��Ҫ���µļ�����
	m_updateSet_Move.insert( nID );
	return	0;
}

////////////////////////////////////////////////////////////////////////////

void whsightlist2D::_UpdateQuadtree()
{
	DummyUnit * pDummy = NULL;
	idsetitor_t it = m_updateSet_Move.begin();
	for ( ; it != m_updateSet_Move.end(); ++ it ) {
		pDummy = m_Dummys.GetByID( *it );
		if ( ! pDummy ) {
			continue;
		}
		// ��ԭ�����Ĳ���Ҷ����ɾ��,����ӵ��µ��Ĳ���Ҷ����.
        QTreeLeaf	*pLeafOld = m_qtree->FindLeaf( pDummy->nX, pDummy->nY );
        QTreeLeaf	*pLeafNew = m_qtree->Insert( pDummy->nNewX, pDummy->nNewY );

		assert( pLeafNew );

		//static char	str[1024];
		//sprintf( str, "��Ӹ�Ҷ��  type= %d, id= %d, X= %d, Y= %d, newX= %d, newY= %d\n", pDummy->info.nType, *it, pDummy->nX, pDummy->nY, pDummy->nNewX, pDummy->nNewY ); 
		//OutputDebugString( str );

        //if ( pLeafOld ) {


        if( pLeafOld )
		{
			//----------------------------------------------------------------------------------------------
			idset_t& oldIds = static_cast<QTreeSight*>(pLeafOld)->m_data;
			oldIds.erase(*it);	// ɾ��id
			// ���Ҷ�����Ѿ�û���û���,��ɾ����Ҷ��
			if (oldIds.empty())
			{
					m_qtree->Delete( pLeafOld );
				//static char	str[1024];
				//sprintf( str, "��Ҷ��ɾ��  type= %d, id= %d, X= %d, Y= %d, newX= %d, newY= %d\n", pDummy->info.nType, *it, pDummy->nX, pDummy->nY, pDummy->nNewX, pDummy->nNewY ); 
				//OutputDebugString( str );
			}		
			//}
	//     else {
				//static char	str[1024];
				//sprintf( str, "�ҿ�!��Ȼû�ҵ�  type= %d, id= %d, X= %d, Y= %d, newX= %d, newY= %d\n", pDummy->info.nType, *it, pDummy->nX, pDummy->nY, pDummy->nNewX, pDummy->nNewY ); 
				//OutputDebugString( str );
	//     }
		}
		else
		{
			WHCMN_LOG_WRITEFMT(WHCMN_LOG_ID_FATAL, "whsightlist2D::_UpdateQuadtree: OuterID=%d, Type=%hd, UserData = %d, (%d, %d) to (%d, %d)",
				pDummy->info.nOuterID, pDummy->info.nType, pDummy->info.nUserData, pDummy->nX, pDummy->nY, pDummy->nNewX, pDummy->nNewY);
		}

		//----------------------------------------------------------------------------------------------

        static_cast<QTreeSight*>(pLeafNew)->m_data.insert(*it);
	}
}

//////////////////////////////////////////////////////////////////////////

int	whsightlist2D::_OnDumGo( int nID )
{
	//assert( !(m_isFocused && nID == 65536) );    // �峤��ʧ����ļ����룬by ���Ľ�

	m_pCurDummy = m_Dummys.GetByID(nID);
	if ( m_pCurDummy == NULL ) {
		return	-1;
	}

	if (m_pCurDummy->info.nType == DUMMY_INFO_T::TYPE_GOD ) {
		return 0;
	}



	if (	( m_CurDumItor = m_dummyRelaMap.find(nID) )	==	m_dummyRelaMap.end()	) {
		return -2;
	}


	//	��ɾ����idȫ����������.
	DummyUnit * pOtherDummy = NULL;

	// ������»���	
	//uciout_t		uciout;
	UCI_T		uciout;
	uciout.nD = -1;

	// ɾ��player
	{
		CPU_GUARD(GameSight_DummyGo_Delete);
		switch ( m_pCurDummy->info.nType )
		{
		case DUMMY_INFO_T::TYPE_PLAYER:
			{
				idsetitor_t	itBeSeeID = m_CurDumItor->second.player.begin();
				idsetitor_t it_end = m_CurDumItor->second.player.end();
				for ( ; itBeSeeID != it_end; ++ itBeSeeID )
				{
					//// �ж��������...�Ƿ񳬳���Χ....
					//// ��Ȼ����Ұ���򲻱仯..
					pOtherDummy  = m_Dummys.GetByID( *itBeSeeID );
					if ( abs( pOtherDummy->nNewX - m_pCurDummy->nNewX ) <= nMaxLeaveSightX  
						&& abs(pOtherDummy->nNewY - m_pCurDummy->nNewY) <= nMaxLeaveSightY )
					{
						continue;
					}
					else
					{	// ������Ұ�ˡ�
						m_dummyRelaMap[*itBeSeeID].player.erase(nID);

						// ��ɾ����id��¼����,�����ƶ�����player, ��Χ�ڵ���player, ��2��player����ӶԷ�����Ұ����ʧ
						uciout.nI = *itBeSeeID;		// cur player
						uciout.nU = nID;			// other player
						//m_uciOut.insert( uciout );
						add_uci_item(uciout);
						
						m_dummyRelaMap[nID].player.erase(*itBeSeeID);
						uciout.nI = nID;			// cur player	
						uciout.nU = *itBeSeeID;		// other player
						//m_uciOut.insert( uciout );
						add_uci_item(uciout);
					}
				}

				itBeSeeID= m_CurDumItor->second.booth.begin();
				it_end = m_CurDumItor->second.booth.end();
				for ( ; itBeSeeID != it_end; ++ itBeSeeID )
				{
					//// �ж��������...�Ƿ񳬳���Χ....
					//// ��Ȼ����Ұ���򲻱仯..
					pOtherDummy  = m_Dummys.GetByID( *itBeSeeID );
					if ( abs( pOtherDummy->nNewX - m_pCurDummy->nNewX ) <= nMaxLeaveSightX  
						&& abs(pOtherDummy->nNewY - m_pCurDummy->nNewY) <= nMaxLeaveSightY )
					{
						continue;
					}
					else
					{	// ������Ұ�ˡ�
						m_dummyRelaMap[*itBeSeeID].player.erase(nID);

						// ��ɾ����id��¼����,�����ƶ�����player, ��Χ�ڵ���player, ��2��player����ӶԷ�����Ұ����ʧ
						uciout.nI = *itBeSeeID;		// cur player
						uciout.nU = nID;			// other player
						//m_uciOut.insert( uciout );
						add_uci_item(uciout);
						
						m_dummyRelaMap[nID].booth.erase(*itBeSeeID);
						uciout.nI = nID;			// cur player	
						uciout.nU = *itBeSeeID;		// other player
						//m_uciOut.insert( uciout );
						add_uci_item(uciout);
					}
				}

				itBeSeeID= m_CurDumItor->second.npc.begin();
				it_end = m_CurDumItor->second.npc.end();
				for ( ; itBeSeeID != it_end; ++itBeSeeID )
				{
					//// �ж��������...�Ƿ񳬳���Χ....
					//// ��Ȼ����Ұ���򲻱仯..
					pOtherDummy  = m_Dummys.GetByID( *itBeSeeID );
					if ( abs( pOtherDummy->nNewX - m_pCurDummy->nNewX ) <= nMaxLeaveSightX  
						&& abs(pOtherDummy->nNewY - m_pCurDummy->nNewY) <= nMaxLeaveSightY )
					{
						continue;
					}

					else
					{		// ������Ұ�ˡ�
						m_dummyRelaMap[*itBeSeeID].player.erase(nID);
						m_dummyRelaMap[nID].npc.erase(*itBeSeeID);	

						// ��ɾ����id��¼����, �����ƶ�����player, ��Χ�ڵ���npc,�� npc �� player����Ұ����ʧ
						uciout.nI = *itBeSeeID;			// cur npc
						uciout.nU = nID;				// cur player
						//m_uciOut.insert( uciout );
						add_uci_item(uciout);
					}
				}
			}
			break;
		// ɾ��npc
		case DUMMY_INFO_T::TYPE_NPC:
			{
				idsetitor_t	itBeSeeID = m_CurDumItor->second.player.begin();

				for ( ; itBeSeeID != m_CurDumItor->second.player.end(); ++ itBeSeeID )
				{
					//// �ж��������...�Ƿ񳬳���Χ....
					//// ��Ȼ����Ұ���򲻱仯..
					pOtherDummy  = m_Dummys.GetByID( *itBeSeeID );
					if ( abs( pOtherDummy->nNewX - m_pCurDummy->nNewX ) <= nMaxLeaveSightX  && 
						abs(pOtherDummy->nNewY - m_pCurDummy->nNewY) <= nMaxLeaveSightY )
					{
						continue;
					}

					else
					{			// ������Ұ�ˡ�
						m_dummyRelaMap[*itBeSeeID].npc.erase(nID);	
						m_dummyRelaMap[nID].player.erase(*itBeSeeID);

						// ��ɾ����id��¼����,�����ƶ�����npc, ��Χ�ڵ���player, �� npc �� player����Ұ����ʧ
						uciout.nI = nID;				// cur npc
						uciout.nU = *itBeSeeID;			// cur player
						//m_uciOut.insert( uciout );
						add_uci_item(uciout);
					}
				}
			}
			break;
		default:
			;
		}
	}



	m_pCurDummy->nX = m_pCurDummy->nNewX;
	m_pCurDummy->nY = m_pCurDummy->nNewY;

	// �����Ĳ���
	{
		CPU_GUARD(GameSight_DummyGo_SearchBox)
		m_qtree->SearchBox( m_pCurDummy->nNewX, m_pCurDummy->nNewY, m_info.nMaxSightX, m_info.nMaxSightY, quadtree_searchBox_dummyGo, this );
	}

	m_pCurDummy = NULL;
	m_CurDumItor = m_dummyRelaMap.end();

	return	0;
}

////////////////////////////////////////////////////////////////////////////

int	whsightlist2D::_OnProDumGo( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY )
{
	CPU_GUARD(GameSight_OnProDumGo);

	if ( ! m_pCurDummy ) {
		return	-1;
	}

	int	id = m_CurDumItor->first;
	int	dist = nDistX + nDistY;

	//uciin_t		uciIn;
	UCI_T		uciIn;
	uciIn.nD	= dist;

	DummyUnit	* pDummyOther = NULL;
	int		idOther = -1;	


	// ��ȡ���ڵ��Ĳ�����Ҷ�������е�id.
	idset_t* pIdset = &static_cast<QTreeSight*>(pLeaf)->m_data;
	idsetitor_t it = pIdset->begin();


	switch ( (m_pCurDummy->info).nType )
	{
	// ��������ƶ�����NPC
	case DUMMY_INFO_T::TYPE_NPC:
		{
			// ������Χ�������
			for ( ; it != pIdset->end(); ++ it  )
			{
				// �Ƿ����Լ�
				idOther  = * it;
				if ( id != idOther )
				{
					pDummyOther = m_Dummys.GetByID( idOther );
					assert( pDummyOther );

					switch ( (pDummyOther->info).nType )
					{
					// �²�������NPC����Χ������NPC����������pDummyOther��NPC��m_pCurDummy��NPC��
					case DUMMY_INFO_T::TYPE_NPC:
						{
							continue;		
						}
						break;
					// �²�������NPC����Χ���������,�������Ҳ��뵽NPC��beSee�ֶΡ���pDummyOther����ң�m_pCurDummy��NPC��
					case DUMMY_INFO_T::TYPE_PLAYER:
					case DUMMY_INFO_T::TYPE_BOOTH:
						{
							// ���⣺ m_pCurDummy �� pDummyOther����
							if ( ( (m_CurDumItor->second).player.insert(idOther) ).second )
							{	
								m_dummyRelaMap[idOther].npc.insert( id );				
							
								uciIn.nI = id;
								uciIn.nU = idOther;
								//m_uciIn.insert( uciIn );
								add_uci_item(uciIn);
							}
						}
						break;
					default:
						;
					}
				}
			}
		}
		break;

	/////////////////////////////////////

	// ��������ƶ��������
	case DUMMY_INFO_T::TYPE_PLAYER:
		{
			// ������Χ�������
			for ( ; it != pIdset->end(); ++ it  )
			{
				// �Ƿ����Լ�
				idOther  = * it;
				if ( id != idOther )
				{
					pDummyOther = m_Dummys.GetByID( idOther );
					assert(pDummyOther);

					// �²���������ҵ���Χ������NPC, �Ǿ�ֻ��������NPC���뵽����ֶΡ���pDummyOther��NPC��m_pCurDummy����ң�
					switch ( (pDummyOther->info).nType )
					{
					case DUMMY_INFO_T::TYPE_NPC:
						{
							// ���⣺pDummyOther  �� m_pCurDummy ����
							if ( (m_dummyRelaMap[idOther].player.insert(id)).second )
							{
								(m_CurDumItor->second).npc.insert(idOther);					

								uciIn.nI = idOther;
								uciIn.nU = id;
								//m_uciIn.insert( uciIn );
								add_uci_item(uciIn);
							}
						}
						break;
					// �²���������ҵ���Χ��������ң�������룬��pDummyOther����ң�m_pCurDummy����ң�
					case DUMMY_INFO_T::TYPE_PLAYER:
						if ((m_CurDumItor->second).player.size() < m_info.nMaxPlayerSeen && m_dummyRelaMap[idOther].player.size() < m_info.nMaxPlayerSeen)
						{	
							// ���⣺ m_pCurDummy ���࿴�� pDummyOther����

							if ( (m_CurDumItor->second).player.insert( idOther ).second ) {	
								m_dummyRelaMap[idOther].player.insert( id );	
						
								uciIn.nI = id;
								uciIn.nU = idOther;
								//m_uciIn.insert( uciIn );
								add_uci_item(uciIn);

								uciIn.nI = idOther;
								uciIn.nU = id;
								//m_uciIn.insert( uciIn );
								add_uci_item(uciIn);
							}
						}
						break;
					case DUMMY_INFO_T::TYPE_BOOTH:
						{	
							// ���⣺ ���࿴��
							if ( (m_CurDumItor->second).booth.insert( idOther ).second ) {	
								m_dummyRelaMap[idOther].player.insert( id );	
						
								uciIn.nI = id;
								uciIn.nU = idOther;
								//m_uciIn.insert( uciIn );
								add_uci_item(uciIn);

								uciIn.nI = idOther;
								uciIn.nU = id;
								//m_uciIn.insert( uciIn );
								add_uci_item(uciIn);
							}
						}
						break;
					default:
						;
					}
				}
			}
		}
		break;
	default:
		;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////
//
//int	whsightlist2D::AddNpcFirst(whsightlist2D::DUMMY_INFO_T *pDummyInfo, int nX, int nY )
//{
//	//////////////////////////////////////////////////////////////////////////
//	// ���������nothing��ʱ��,�޶���,ֱ�ӷ��ش���.
//	if ( pDummyInfo->nType == DUMMY_INFO_T::TYPE_NOTHING ) {
//		return	-2;
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	// ����������,npc,�ϵ۵�ʱ��,�����ռ�.
//	int		nID;
//	DummyUnit	*pDummy	= m_Dummys.Alloc( & nID );
//
//	// û�пռ���
//	if( !pDummy )	{
//		return	-1;
//	}
//	// �ȳ�ʼ��һ������
//	pDummy->clear();
//	// ���������Ϣ
//	memcpy( &pDummy->info, pDummyInfo, sizeof(pDummy->info) );
//
//
//	//////////////////////////////////////////////////////////////////////////
//	// ���������god��ʱ��, ��ֱ�����god�ɼ���������.�������˲��ɼ�god.god ��Ұ����Ϊ����(-1).������.
//	//		�򲻻�����Ĳ�����صĺ���.
//	if ( pDummyInfo->nType == DUMMY_INFO_T::TYPE_GOD ) {
//		m_godset.insert(nID) ;
//		return	nID;
//	}
//
//	//---------------------------
//	// �������֮��Ŀɼ����.
//
//	// ��������
//	pDummy->nX		= pDummy->nNewX		= nX;
//	pDummy->nY		= pDummy->nNewY		= nY;
//
//
//	QTreeLeaf	*pLeaf = m_qtree->Insert( nX, nY );
//	//static char	str[1024];
//	//sprintf( str, "������� npc  type= %d, id= %d, X= %d, Y= %d, newX= %d, newY= %d\n", pDummy->info.nType, nID, pDummy->nX, pDummy->nY, pDummy->nNewX, pDummy->nNewY ); 
//	//OutputDebugString( str );
//
//    static_cast<QTreeSight*>(pLeaf)->m_data.insert(nID);
//
//	// ���dummy�Ĺ�ϵmap��
//	static beSee			dummyRela;
//	m_dummyRelaMap.insert( std::make_pair( nID, dummyRela ) );
//	
//	return	nID;
//}
//
//
//
////////////////////////////////////////////////////////////////////////////
// �ڲ��õĴ��ݱ����õ�

struct	 searchQuad_forOut_t
{
	whvector<int>		* pV;
	whsightlist2D		* pSL2D;
	searchQuad_forOut_t( whsightlist2D * _s, whvector<int> * _v )
	{
		pV = _v;			pSL2D = _s;
	}
};
//struct	 searchQuad_forSetOut_t
//{
//	std::set<int>		* pV;
//	whsightlist2D		* pSL2D;
//	searchQuad_forSetOut_t( whsightlist2D * _s, std::set<int> * _v )
//	{
//		pV = _v;			pSL2D = _s;
//	}
//};
////////////////////////////////////////////////////////////////////////////

inline	int	Lane_quadtree_searchBox_forOut( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY, void * pExtInfo )
{
	return	 ((searchQuad_forOut_t *)pExtInfo)->pSL2D->AddDummyToVecOnLeaf( pLeaf, ((searchQuad_forOut_t *)pExtInfo)->pV );
}

////////////////////////////////////////////////////////////////////////////
//
//inline	int	Lane_quadtree_searchBox_forSetOut( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY, void * pExtInfo )
//{
//	return	 ((searchQuad_forSetOut_t *)pExtInfo)->pSL2D->AddDummyToSetOnLeaf( pLeaf, ((searchQuad_forSetOut_t *)pExtInfo)->pV );
//}
//
////////////////////////////////////////////////////////////////////////////

inline	int	Lane_quadtree_searchRay_forOut( QTreeLeaf * pLeaf, void * pExtInfo )
{
	return	 ((searchQuad_forOut_t *)pExtInfo)->pSL2D->AddDummyToVecOnLeaf( pLeaf, ((searchQuad_forOut_t *)pExtInfo)->pV );
}

////////////////////////////////////////////////////////////////////////////
// ����һ����Χ�ڵ�����dummy ��id,���ص�������.

void	whsightlist2D::SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, whvector<int> & vID )
{
	vID.clear();
	searchQuad_forOut_t		t( this, &vID );
	m_qtree->SearchBox( nCenterX, nCenterY, nRadiusX, nRadiusY, Lane_quadtree_searchBox_forOut, &t );
}

////////////////////////////////////////////////////////////////////////////
//// ����һ����Χ�ڵ�����dummy ��id,���ص�set��.
//
//void	whsightlist2D::SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, std::set<int> & vID )
//{
//	vID.clear();
//	searchQuad_forSetOut_t		t( this, &vID );
//	m_qtree->SearchBox( nCenterX, nCenterY, nRadiusX, nRadiusY, Lane_quadtree_searchBox_forSetOut, &t );
//}
//
////////////////////////////////////////////////////////////////////////////
// ����һ�������ڵ�����dummy ��id,

void	whsightlist2D::SearchRay(int nStartX, int nStartY, int nEndX, int nEndY, int nExtendRange, whvector<int> & vID ) 
{
	vID.clear();
	searchQuad_forOut_t		t(this, &vID);
	m_qtree->SearchRay( nStartX, nStartY, nEndX, nEndY, nExtendRange, Lane_quadtree_searchRay_forOut, &t );
}

////////////////////////////////////////////////////////////////////////////
// �ڲ��õĴ��ݱ����õ�			���ص���

struct	 searchQuad_forOut_callback_box_t
{
	whsightlist2D							* pSL2D;
	SearchBox_Fun_t							pFun;
	searchQuad_forOut_callback_box_t( whsightlist2D * _s, SearchBox_Fun_t _fun )
	{
		pSL2D = _s;			pFun = _fun;
	}
};

////////////////////////////////////////////////////////////////////////////
// �Ĳ����ص�,	���ص���
inline	int	Lane_quadtree_searchBox_forOut_callback( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY, void * pExtInfo )
{
	return	 ((searchQuad_forOut_callback_box_t *)pExtInfo)->pSL2D->_CallOutCallbackFun_Box( pLeaf, ((searchQuad_forOut_callback_box_t *)pExtInfo)->pFun );
}

// ����һ����Χ�ڵ�����dummy ��id,���ص�������.	���ص���
void	whsightlist2D::SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, SearchBox_Fun_t pCallBack )
{
	searchQuad_forOut_callback_box_t		t(this, pCallBack);
	m_qtree->SearchBox( nCenterX, nCenterY, nRadiusX, nRadiusY, Lane_quadtree_searchBox_forOut_callback, &t );
}	

// ���ص��� �Ĳ�������
int whsightlist2D::_CallOutCallbackFun_Box( QTreeLeaf * pLeaf, SearchBox_Fun_t fun )
{
    if(pLeaf == 0) return 0;
	idset_t* pIdset = &static_cast<QTreeSight*>(pLeaf)->m_data;
    if(pIdset->empty()) return 0;

	// fun ���ܻ���� DelDummy ����ɵ�����ʧЧ����ѭ����
	// ���Կ���һ���б�����ѭ������ʱ����˴����⣬��Ҫ�۲�᲻��Ӱ�쵽�����ĵط���
	// liuyi 2008-07-16
	idset_t BakSet = *pIdset;
	idsetitor_t it = BakSet.begin();
	for ( ;it != BakSet.end(); ++ it ) {
		int ret = fun( *it );
		if ( ret < 0 ) {
			return	ret;
		}
	}
	return 0;
}

///////////=========================================
// search ray struct with callback
struct	 searchQuad_forOut_callback_ray_t
{
	whsightlist2D							* pSL2D;		// this
	SearchRay_Fun_t							pFun;			// call back fun
	searchQuad_forOut_callback_ray_t( whsightlist2D * _s, SearchRay_Fun_t _fun )
	{
		pSL2D = _s;			pFun = _fun;
	}
};
//////////////////////////////////////////////////////////////////////////
// �Ĳ����ص�,���ص���
inline	int	Lane_quadtree_searchRay_forOut_callback( QTreeLeaf * pLeaf, void * pExtInfo )
{
	return	 ((searchQuad_forOut_callback_ray_t *)pExtInfo)->pSL2D->_CallOutCallbackFun_Ray( 
						pLeaf, 
						((searchQuad_forOut_callback_ray_t *)pExtInfo)->pFun );
}

// ����һ�������ڵ�����dummy ��id,.���ص���
void	whsightlist2D::SearchRay(int nStartX, int nStartY, int nEndX, int nEndY, int nExtendRange, SearchRay_Fun_t pCallBack )
{
	searchQuad_forOut_callback_ray_t		t(this, pCallBack);
	m_qtree->SearchRay( nStartX, nStartY, nEndX, nEndY, nExtendRange, Lane_quadtree_searchRay_forOut_callback, &t );
}	

// ���ص��� �Ĳ�������
int whsightlist2D::_CallOutCallbackFun_Ray( QTreeLeaf * pLeaf, SearchRay_Fun_t fun )
{
    if(pLeaf == 0) return 0;
	idset_t* pIdset = &static_cast<QTreeSight*>(pLeaf)->m_data;
    if(pIdset->empty()) return 0;
	idsetitor_t it = pIdset->begin();
	for ( ;it != pIdset->end(); ++ it ) {
		int ret = fun( *it );
		if ( ret < 0 ) {
			return	ret;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////
//// ����һ���������dummy�����뵽���ص�������.
//
//void	whsightlist2D::GetDummyOnPoint( int nX, int nY, whvector<int> & vID ) 
//{
//	QTreeLeaf		* pQuadleft		= m_qtree->FindLeaf( nX, nY );
//	if ( ! pQuadleft ) {
//		return;
//	}
//
//	AddDummyToVecOnLeaf( pQuadleft, &vID );
//}
//
////////////////////////////////////////////////////////////////////////////

int whsightlist2D::AddDummyToVecOnLeaf( QTreeLeaf * pLeaf, whvector<int> * v )
{
    if(pLeaf == 0) return 0;
	idset_t* pIdset = &static_cast<QTreeSight*>(pLeaf)->m_data;
    if(pIdset->empty()) return 0;
	idsetitor_t it = pIdset->begin();
	for ( ;it != pIdset->end(); ++ it ) {
		v->push_back( *it );
	}
	return 0;
};

////////////////////////////////////////////////////////////////////////////
//
//int whsightlist2D::AddDummyToSetOnLeaf( QTreeLeaf * pLeaf, std::set<int> * v )
//{
//    if(pLeaf == 0) return 0;
//	idset_t* pIdset = &static_cast<QTreeSight*>(pLeaf)->m_data;
//    if(pIdset->empty()) return 0;
//	idsetitor_t it = pIdset->begin();
//	for ( ;it != pIdset->end(); ++ it ) {
//		v->insert( *it );
//	}
//	return 0;
//};
//
////////////////////////////////////////////////////////////////////////////

void whsightlist2D::AddFreePoint(int x, int y)
{
    if(IsTakeSpace(x, y)) return;

	QTreeSight* t = static_cast<QTreeSight*>(m_qtree->FindLeaf(x, y));
    int num = t ? t->m_data.size() : 0;
    if(num >= 4) return;

    node_point_t	pos(x, y);
    pos.uDummyNum = num;
    m_vFreePoint.push_back(pos);
}

int   whsightlist2D::SearchFreePoint( int & nPointX, int & nPointY, int iRadius )
{
	signed short len = 0;
	signed short startX = nPointX, startY = nPointY;

	m_vFreePoint.clear();
	m_vShowPos = 0;

	AddFreePoint(nPointX, nPointY);

	for ( signed short i = 1; i < iRadius; i++ ) {
		startX = nPointX-i, startY = nPointY-i;
		len  +=2;

		for ( signed short j = 0; j < 2*i+1; j++ ) {
			signed short x2 = startX+j , y2 = startY;

			AddFreePoint(x2, y2);
			AddFreePoint(x2, y2+len);
		}

		for ( signed short j = 0; j < 2*i+1-2; j++ ) {
			signed short x2 = startX  , y2 = startY + j + 1;

			AddFreePoint(x2, y2);
			AddFreePoint(x2+len, y2);
		}
	}

	if ( m_vFreePoint.size() == 0 ) {
		return	-1;
	}

	nPointX = m_vFreePoint[0].nX ;
	nPointY = m_vFreePoint[0].nY ;
	m_vShowPos ++;
	return	m_vFreePoint[0].uDummyNum;	
}


//////////////////////////////////////////////////////////////////////////

int   whsightlist2D::SearchFreePointNext( int & nPointX, int & nPointY )
{
	//QTreeLeaf		* pQuadleft = NULL;

	while  ( m_vShowPos < m_vFreePoint.size() ) {

		if ( ! IsTakeSpace( m_vFreePoint[m_vShowPos].nX, m_vFreePoint[m_vShowPos].nY ) ) {

			QTreeSight* pQuadleft = static_cast<QTreeSight*>(m_qtree->FindLeaf(m_vFreePoint[m_vShowPos].nX, m_vFreePoint[m_vShowPos].nY));

            int num = pQuadleft ? pQuadleft->m_data.size() : 0;
            if(num < 4)
            {
				nPointX = m_vFreePoint[m_vShowPos].nX;
				nPointY = m_vFreePoint[m_vShowPos].nY;
				m_vFreePoint[m_vShowPos].uDummyNum	= num;

				m_vShowPos ++ ;
				return	num;
            }
		}

		m_vShowPos ++;
	}

	return	-1;
}

////////////////////////////////////////////////////////////////////////////

bool whsightlist2D::TakeSpace( int nPointX, int nPointY )
{
	node_point_t	takePoint( nPointX, nPointY );	
	return	 m_TakeSpaceNodePointSet.insert( takePoint ).second;
}

////////////////////////////////////////////////////////////////////////////
//
//bool  whsightlist2D::UnTakeSpace( int nPointX, int nPointY )
//{
//	node_point_t	takePoint( nPointX, nPointY );
//	return (m_TakeSpaceNodePointSet.erase( takePoint ) != 0 );
//}
//
////////////////////////////////////////////////////////////////////////////

bool whsightlist2D::GetUCIincTable(UCI_Inc_Table &incTable)
{
	try
	{
		typedef std::multimap<int, ucitable::iterator> ucisortedlist;
		typedef std::pair<int, ucitable::iterator> sortedlistitem;
		ucisortedlist sorted_list;

		//if( !uci_table.empty() )
		//	printf("Total: %u", uci_table.size());

		int lastnU = -1;
		for(ucitable::iterator iter = uci_table.begin(); iter != uci_table.end(); ++iter)
		{
			if( lastnU != iter->nU)
			{
				if( lastnU != -1 )
				{
					//printf("Count: %u\n", sorted_list.size());

					int count = 0;
					for(ucisortedlist::iterator it = sorted_list.begin(); it != sorted_list.end(); ++it, ++count)
					{
						if(it->first >= 0 && count > MAX_UCICOUNT)
							break;
						incTable.push_back(*(it->second));
						uci_table.erase(it->second);
					}
					sorted_list.clear();
				}
				lastnU = iter->nU;
			}
			sorted_list.insert( sortedlistitem(iter->nD, iter) );
		}
		if( lastnU != -1 )
		{
			//printf("Count: %u\n", sorted_list.size());

			int count = 0;
			for(ucisortedlist::iterator it = sorted_list.begin(); it != sorted_list.end(); ++it, ++count)
			{
				if(it->first >= 0 && count > MAX_UCICOUNT)
					break;
				incTable.push_back(*(it->second));
				uci_table.erase(it->second);
			}
		}
		//if( !uci_table.empty() )
		//	printf("Left: %u\n", uci_table.size());
	}
	catch(...)
	{
		assert(0 && "��ȡUCI�����");
		return false;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

#undef MAX_UCI_TABLE
