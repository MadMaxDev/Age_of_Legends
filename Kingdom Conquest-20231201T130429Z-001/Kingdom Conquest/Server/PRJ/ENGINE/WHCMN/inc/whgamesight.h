// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whgamesight.h
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
//				  2006-12-18 ���Ӵ��ص���SearchBox��SearchRay������EndGetUCIList()
//				  2007-03-06 ����GetUCIUserData()����,�ýӿ����ڻ�ȡUCI��nUserData���б�,�б��в�����NPC
//				  2007-03-09 �޸ģ�ʹû��dummy���Ĳ�����leaf��delete��������1���ƶ������ڣ�ͬһdummy��ε���dummyGO�ͻص�ԭ��������⡣
//				  2007-03-22 ����GetDummyUnit����������DummyUnitָ��
//				  2007-06-05 Ϊ���١��峤��ʧ���������ӱ���m_idFocused����ش���
//				  2007-08-08 �޸�uciinset_t��ucioutset_t������ʽ����nUΪ��������������������set���ͬһ����ҵ���Ϣ�ͼ�����һ����
//				  2007-08-08 �޸�GetUCI_In��GetUCI_OutΪÿ���ε���ֻ����ͬһ���������Ұ�仯�б��ǰMAX_UCICOUNT����ʣ����Ƴٵ������ٷ���
//				  2007-09-05 Ϊwhsightlist2D::SearchBox����һ�����أ����ڰѽ��������set�У�������Ҫ��ID����ĳ��ϣ�
//				  2007-12-20 ������Ҷ���ҵ���Ұ�����ޣ���ͨ��whsightlist2D::INFO_T::nMaxPlayerSeen�޸ģ�ȱʡΪһ���������ܿ�����Χ��100����ң��������Ӧ���ܿ����������������һ������Ŀ���ˣ���˫������Ұ�����ᷢ���ı�
//				  2008-06-06 ȡ��uci_table��Ψһ�Լ�飨���ܻ��������⣬��Ҫ���Ժ�ע��۲죩
//				  2008-07-22 ע�͵�һЩ�Ѿ������Ĵ���

#ifndef	__WHGAMESIGHT_H__
#define	__WHGAMESIGHT_H__

#include "./whvector.h"
#include "./whhash.h"
#include "./whquadtree.h"
#include "./whallocator2.h"
#include "./whunitallocator2.h"
#include "./whtimequeue.h"
#include "./whtime.h"

#include <utility>
#include <map>
#include <set>
#include <vector>
#include <iomanip>
#include <iostream>



typedef				int (* SearchBox_Fun_t)( int id );
typedef				int (* SearchRay_Fun_t)( int id );

//typedef				int (* Search_Fun_t)( int x, int y );


//struct fast_plex;
//template<class T>
//struct fast_allocator;
//////////////////////////////////////////////////////////////////////////
//
//	�����fast_allocator
//
//////////////////////////////////////////////////////////////////////////
//template< class T = char >
struct fast_plex
{
	class item
	{
	public:
		item* m_next;
	};
	class page
	{
	public:
		page*   m_next;
		size_t  m_reserved;
	};

	size_t  m_page_size;
	size_t  m_item_size;
	page*   m_pages;
	item*	m_items;

	inline void allocate_page()
	{
		// allocate page
		size_t is = m_item_size;
		size_t ps = sizeof(page)+is*m_page_size;

		page* pg = (page*)malloc(ps);
		pg->m_next = m_pages;
		m_pages = pg;

		char* b = (char*)(pg+1);
		char* e = (char*)pg+ps-is;
		for(; b <= e; b += is)
		{
			item* a = (item*)b;
			a->m_next = m_items;
			m_items = a;
		}
		assert(m_items != NULL);
	};
public:
	inline fast_plex(size_t itemSize, size_t pageSize)
	{
		m_page_size = pageSize;
		m_item_size = itemSize;
		m_pages     = 0;
		m_items     = 0;
		if(m_item_size < sizeof(void*))
		{
			m_item_size = sizeof(void*);
		}	
	};
	
	inline ~fast_plex()
	{
		while(m_pages)
		{
			page* p = m_pages;
			m_pages = p->m_next;
			free(p);
		}
	};

	inline void* allocate()
	{
		if(m_items == 0)
		{
			allocate_page();
		}
		item* p = m_items;
		m_items = m_items->m_next;
		return p;
	}
	inline void deallocate(void* pItem)
	{
		item* p = (item*)pItem;
		p->m_next = m_items;
		m_items = p;
	}

	//inline T * New()
	//{
	//	T * p = (T *)allocate();
	//	new(p) T();
	//	return	p;
	//};

	//inline void Free( void * p )
	//{
	//	((T *)p)->~T();
	//	deallocate(p);
	//};
};



template<class T>
struct fast_allocator
{
	typedef T value_type;
	typedef T* pointer;
	typedef T& reference;
	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	static fast_plex m_plex;

	template<class Other>
	struct rebind
	{
		typedef fast_allocator<Other> other;
	};

	pointer address(reference val) const
	{
		return (&val);
	}
	const_pointer address(const_reference val) const
	{
		return (&val);
	}
	template<class Other>
		fast_allocator(const fast_allocator<Other>& )
	{
	}
	fast_allocator()
	{
	}
	fast_allocator(const fast_allocator& )
	{
	}
	void deallocate(pointer p, size_type )
	{
		m_plex.deallocate(p);
	}
	pointer allocate(size_type )
	{
		return (pointer)m_plex.allocate();
	}
	pointer allocate(size_type s, const void* )
	{
		return 0;	// ��ʱ�޸�һ�£������gcc4�±��벻ͨ��
	}
	void construct(pointer p, const_reference val)
	{
		new(p) T(val);
	}
	void destroy(pointer ptr)
	{
		NOTUSE(ptr);
		ptr->~T();
	}
	size_t max_size() const
	{
		size_t c = (size_t)(-1) / sizeof (value_type);
		return (0 < c ? c : 1);
	}
	bool operator == (const fast_allocator& )
	{
		return true;
	}
	bool operator != (const fast_allocator& )
	{
		return false;;
	}
};

//////////////////////////////////////////////////////////////////////////


namespace n_whcmn
{

struct QTreeTrunk;

struct QTreeNode
{
    QTreeTrunk*     m_parent;
	short           m_x;        // ����ڵ㱣��������Ͻǵ����ꡣ
    short           m_y;
	unsigned char   m_level;    // LevelΪ0��ʾ��Ҷ��
	unsigned char   m_index;    // �����Լ��Ǹ��ڵ��е��ĸ�(0~3)������������ǿ�����ɾ��ʱ���ٵ�������ڵ��е�����
    unsigned char   m_mask;     // �������������Ƿ����
};

struct QTreeTrunk : QTreeNode
{
    QTreeNode*      m_child[4];
};

struct QTreeLeaf : QTreeNode
{
    // ����û�����ݣ���Ҫʹ��QTreeNodeMgr����չ����ڵ�
};

// ��ͬ��qtree����ʹ��ͬһ���ڴ���������������Ը��ýڵ����ݡ�
struct QTreeNodeMgr
{
	virtual ~QTreeNodeMgr() {}
    virtual QTreeTrunk* AllocateTrunk() = 0;
    virtual void DeallocateTrunk(QTreeTrunk*) = 0;

    virtual QTreeLeaf* AllocateLeaf() = 0;
    virtual void DeallocateLeaf(QTreeLeaf*) = 0;
};


// ����ĺ����ṩ�Խڵ��Ҷ�ӵĴ�������Releaseʱ����pData���ݵ��ͷš�
typedef	int	(*QTreeTrunkFunc)(QTreeTrunk* p, void* ctx);
typedef	int	(*QTreeLeafFunc)(QTreeLeaf* p, void* ctx);

// ��������ṩSearchBox�Ľ���ڵ�Ĺ��˺ͱ���Ĺ��ܣ��磺�ڲ����վ������򣬱���ĳ�����͵Ľڵ㲻�������ս����
// pΪҶ�ӽڵ�ṹ
// dx, dyΪҶ�ӵ�ָ����Ķ�ά���룬�����Ҫ��Ϊ�˱����ڻص��ж���ļ���
// ctxΪ���ظ��ص��Ĳ���������ض����ָ�룩
// ���ط���ֵ��ʾSearchBoxֹͣ�������ѷ���ֵֵ���ظ������ߣ����緵��������ʾ����ֹͣ������
typedef	int	(*QTreeSearchBoxFunc)(QTreeLeaf* p, short dx, short dy, void* ctx);

// ��������ṩSearchRay�Ľ���ڵ�Ĺ��˺ͱ���Ĺ���
// pΪҶ�ӽڵ�ṹ
// ctxΪ���ظ��ص��Ĳ���������ض����ָ�룩
// ���ط���ֵ��ʾSearchRayֹͣ�������ѷ���ֵֵ���ظ������ߣ����緵��������ʾ����ֹͣ������
typedef	int	(*QTreeSearchRayFunc)(QTreeLeaf* p, void *ctx);

struct QTree
{
    // ���ݾ�Ϊ��չ����ʹ�ã���Ҫֱ�ӷ��ʡ�
    typedef QTreeTrunk* QTreeTrunkPtr;
    QTreeNodeMgr*   m_mgr;
    QTreeTrunk*     m_root;     // ���ڵ�ʼ�մ��ڣ�����ɾ�������������һЩ������
    QTreeTrunkPtr** m_patch;    // �ֿ�Ķ�ά����
    short           m_size;     // ����Ĵ�С

    // ���ú���

	QTree();
	~QTree();

	bool Create(QTreeNodeMgr* mgr, int max_level);
	void Destroy();

	// Goover�����ӵ�����˳��ı���
	int	Goover(QTreeTrunkFunc func_trunk, QTreeLeafFunc func_leaf, void* ctx);

	// ����Ҷ�ӽڵ㣬���ԭ���оͷ���ԭ����
	QTreeLeaf* Insert(short x, short y);

	// �����������Ҷ�ӽڵ�
	QTreeLeaf* FindLeaf(short x, short y);

	// �������������ײ�ڵ㣨������Ҷ��Ҳ�����Ƿ�Ҷ�ӣ�
	QTreeNode* FindNode(short x, short y);

	// ��������ɾ��Ҷ�ӽڵ�
	void Delete(short x, short y);

	// ����Ҷ�ӽڵ�ָ��ɾ�������һ��Ҫ��֤�ڵ�ָ����Ч�������Ǵ����ڹ������еģ������ϴ��Լ����ڵ㸽�����ݵ�release������
	void Delete(QTreeLeaf *p);

	// ���Ҵ�ĳ�㿪ʼ�����η�Χ�ڵ�Ҷ�ӽڵ�
	// x,y��ʾ���ĵ����꣬rx, ry��ʾx,y�����ϸ��Եķ�Χ��
	// ��ʵ���Ϊr*2+1�������ĵ����߸��ӳ�r�ľ��룩��
	// ��������򷵻�0�����򷵻�pJudgeFunc�ķ��㷵��ֵ
	int	SearchBox(short x, short y, short rx, short ry, QTreeSearchBoxFunc func, void* ctx);

	// ���Ҵ�ĳ�㿪ʼ��ָ�����߶���������Ҷ�ӽڵ�
	// x1,y1Ϊ��ʼ�����ꡣx2,y2��ʾ�����߶ε��յ�
	// range��ʾ��ײ����չ��Χ����Ϊ����ɢ������߶�����󲿷�������������ϵģ����˶˵���м����㣩��������Ҫ��Ҫ��ײ��ķ�Χ����
	// rangeһ��ȡֵΪ1
	// ��������򷵻�0�����򷵻�pJudgeFunc�ķ��㷵��ֵ
	int	SearchRay(short x1, short y1, short x2, short y2, short range, QTreeSearchRayFunc func, void* ctx);
};

class	whsightlist2D
{
public:
	//	��Ұ��� �������ݽṹ�Ͳ���
	struct	INFO_T
	{
		int	nMaxDummy;			// ����Dummy����
		int	nChunkSize;			// ���������Ĳ�����ÿ���������Ĵ�С
		int	nMaxSightX;			// X��������Ұ
		int	nMaxSightY;			// Y��������Ұ
		int	nGoCheckDelay;		// �ƶ��жϵ���ʱ�����룩
		int	nLodLevel;
		size_t nMaxPlayerSeen;		// ����ܼ��������
		INFO_T()
			: nMaxDummy(500),	nChunkSize(1024*1024),	nMaxSightX(16),		nMaxSightY(16),		nGoCheckDelay(1000),	nLodLevel(12),	nMaxPlayerSeen(0)
		{	};
	};
private:
	int nMaxLeaveSightX, nMaxLeaveSightY;	// ����Ұ�������룬����Ϊ����Ұ�����120%

public:
	// dummy����Ϣ
	struct	DUMMY_INFO_T
	{
		enum
		{
			TYPE_NOTHING		= 0,	// δ�������
			TYPE_PLAYER			= 1,	// ��Ҷ���
			TYPE_NPC			= 2,	// ���з���Ҷ���
			TYPE_GOD			= 3,	// ���ֶ�������Ұ�����޵ģ��ܿ������еĶ�����Ҳ����˵�����������ж����ı��ɼ�����
			TYPE_BOOTH			= 4,	// ��̯����ң��ܱ������˿�����������Ұ�ڿɼ�����������޵����ƣ�
			TYPE_PET			= 5,	// ��ҵĳ����������Ұ��ı仯����
		};
		short	nType;			// �������ͣ���ǰ���TYPE_XXX��
		int		nUserData;		// �ϲ��Լ���������ݣ���ҪΪ�����ϲ��Լ�����һЩ����
		int		nOuterID;		// �ϲ��õĶ���ID
	};


		// �ڲ�������õ������˵���Ϣ
	struct	DummyUnit
	{
		//// ��Ұ
		//int				nSightX;
		//int				nSightY;
		// �ϴ�����
		int				nX;
		int				nY;
		// ��������
		int				nNewX;
		int				nNewY;
		// �����д����Ϣ
		DUMMY_INFO_T	info;

		// �����Ϣ
		void	clear()
		{
			WHMEMSET0THIS();
		}
	};


	whsightlist2D();

	~whsightlist2D();

	// ��ʼ���Ͳ�������
	int	Init( INFO_T * pInfo );

	// �ս�
	int	Release();

	// �߼����У�������ʱ���ƶ��жϣ�
	int	Tick();

	// ������(��NPC)������ID
	// int nX, int nY�ǳ�ʼ��λ��
	// Ĭ�ϵ������Ұ�������Ұ
	int	AddDummy( whsightlist2D::DUMMY_INFO_T *pDummyInfo, int nX, int nY );

	// �������ڷ����������ʱ�����NPC,��������ڲ���������Ĳ���.
	int	AddNpcFirst( whsightlist2D::DUMMY_INFO_T *pDummyInfo, int nX, int nY );

	// ����������
	const whsightlist2D::DUMMY_INFO_T *	GetDummy( int nID );

	// ɾ�����
	int	DelDummy( int nID );

	//// ���������Ұ��nSightX��nSightY��ʾX��Y�����ϵ���Ұ�����Ŀɼ���ΧӦ����2X*2Y�ľ��Ρ�ע�⣺���ܳ������Ұ��
	//int	AdjustDummySight( int	nID, int nSightX, int nSightY );

	// ����˶�
	int	DummyGo( int nID, int nX, int nY );

	//////////////////////////////////////////////////////////////////////////
	// ����һ����Χ�ڵ�����dummy ��id,���ص�������.
	void	SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, whvector<int> & vID );

	// ����һ����Χ�ڵ�����dummy ��id,���ص�set��.
	void	SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, std::set<int> & vID );

	// ����һ�������ڵ�����dummy ��id,
	void	SearchRay(int nStartX, int nStartY, int nEndX, int nEndY, int nExtendRange, whvector<int> & vID );

	// ����һ����Χ�ڵ�����dummy ��id,���ص�������.���ص���
	void	SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, SearchBox_Fun_t pCallBack );

	// ����һ�������ڵ�����dummy ��id,.���ص���
	void	SearchRay(int nStartX, int nStartY, int nEndX, int nEndY, int nExtendRange, SearchRay_Fun_t pCallBack );



	// ����һ���������dummy�����뵽���ص�������.
	void	GetDummyOnPoint( int nX, int nY, whvector<int> & vID );

	// ����һ���㸽�����еĵ�.nPointX��nPointY��Ҫ���ҵķ�Χ�����ĵ�.������Ϊ�������.
	// ����в�����ռ�ؿ飬�ɹ����صط���ռ������0-3��ʧ�ܷ���-1
	int   SearchFreePoint( int & nPointX, int & nPointY, int iRadius );

	// �����ϴβ��ҵĽ����������.nPointX��nPointYֻ��Ϊ�������.������Ϊ���ĵ�����,
	//	���ҵ����ĵ���SearchFreePoint�������Ѿ�����������.
	// ����в�����ռ�ؿ飬�ɹ����صط���ռ������0-3��ʧ�ܷ���-1
	int   SearchFreePointNext( int & nPointX, int & nPointY );

	// ����ĳ������һ��ռ�ؿ�
	bool	  TakeSpace( int nPointX, int nPointY );

	// ȡ������ĳ������һ��ռ�ؿ�
	bool	  UnTakeSpace( int nPointX, int nPointY );

	// ���һ�����Ƿ���ռ�ؿ�
	bool	  IsTakeSpace( int nPointX, int nPointY );
	
	// ��ȡһ��dummy
	DummyUnit * GetDummyUnit( int userID );

public:	
	// UCI�б�
	struct	UCI_T
	{
		int			nU;
		int			nI;
		mutable int	nD;
	};
	typedef std::vector<UCI_T> UCI_Inc_Table;
	// ��ȡ��ҿɼ��б�仯
	// ÿ��tick����һ�Σ�����ÿ����ҵĿɼ��б��ȫ������Ұ��Ҽ����ֻ�ȫ������Ұ��ң��ܺͲ�����MAX_UCICOUNT��
	bool GetUCIincTable(UCI_Inc_Table &incTable);

	//// ��ȡ��ұ��ɼ��б�仯
	//// ÿ��tick����һ���Σ���������ֱ������ֵ<0Ϊֹ
	//// ���ӵ���ң���;�������µ��˿������Լ���Ӧ�ð��Լ��ĳ�ʼ�����ݷ��͸��Է��Ŀͻ��ˣ�
	//int	GetUCI_In(UCI_T *pUCI);

	//// ���ٵ���ң���;����ԭ�������Լ������ڿ������Լ��ˣ�Ӧ��֪ͨ�Է��ͻ���ɾ����Ӧ�����ݣ�
	//int	GetUCI_Out(UCI_T *pUCI);

	//// ��ʼ��ȡUCI�б�,������������������null, nRetID����0;
	//inline	DUMMY_INFO_T * 	BeginGetUCIList( int nID, int & nRetID );

	//// ������ȡUCI�б�,������������������null, nRetID����0;������BeginGetUCIList()�������
	//inline	DUMMY_INFO_T * GetNextFromUCI( int & nRetID   );

	//// ֹͣ��ǰ�Ļ�ȡUCI����Ϊ���ڵ����µ�beginǰ������������end�����򲻻�get�ɹ�
	//inline	void EndGetUCIList();


	//// ��ȡUCI�б��ѽ������out�У�out�д�ŵ��ǿ��Կ���nID��outerID������б��в�������NPC
	//inline void	GetUCIOutID( int nID, whvector<int> & out );

	// ��ȡUCI�б��ѽ������out�У�out�д�ŵ��ǿ��Կ���nID��nUserData������б��в�������NPC
	inline void GetUCIUserData( int nID, whvector<int> & out );




public:	



	// �Ĳ�������
	int	_OnAddDum( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY );

	// tick����
	int	_OnDumGo( int nID );

	// �Ĳ�������
	int	_OnProDumGo( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY );
	

	// ���ص��� �Ĳ�������
	int _CallOutCallbackFun_Ray( QTreeLeaf * pLeaf, SearchRay_Fun_t fun );
	
	int _CallOutCallbackFun_Box( QTreeLeaf * pLeaf, SearchBox_Fun_t fun );
	
	typedef		std::set<int, std::less<int>, fast_allocator<int> >		idset_t;		// ID Set
	typedef		idset_t::iterator										idsetitor_t;	// ID Set::itetator




private:

	//typedef		QuadtreeMan<whcmnallocationobj, whchunkallocationobjonesize<whcmnallocationobj> >		qtman_t;	// �Ĳ���




	INFO_T													m_info;
	whcmnallocationobj										m_cao;
	whunitallocatorFixed2<DummyUnit, whcmnallocationobj>	m_Dummys;
    QTree*                                                  m_qtree;
	//qtman_t													* m_pQuadTreeMan;
	idset_t													m_godset;			// GOD SET


	// �����б��ɼ��б�����Ӧ���ǶԵȵģ��ڴ������ҳ����ɼ��б�ʹ�ã�....
	struct		beSee 
	{
		idset_t		npc;		// һ��dummy��NPC����û������,����ֻ��Ϊ��ɾ��NPC�ı��ɼ��б���.
		idset_t		player;		// һ��dummy���Ա���ҿ�������Щ��ҵ��б�.
		idset_t		booth;		// һ��dummy�ɼ���̯λ�б�
	};


	typedef	std::map<int, beSee>							durlmap_t;	
	typedef	durlmap_t::iterator								durlapitor_t;		// Map durlmap_t::iterator


	//	��¼��ÿ��dummy id��Ӧ�� dummyRelation_t
	durlmap_t												m_dummyRelaMap;

	//	���и��µ���Ҫ�����dummy id
	idset_t													m_updateSet_Move;

	// ��¼�´ν��д����tick
	whtick_t 												m_tNextTick;


	int														m_nMaxX;
	int														m_nMaxY;

	//////////////////////////////////////////////////////////////////////////
	//		��ʱ����,���ڼ�����Ӻ��ƶ�dummy�Ĺ���	

	// ��ǰ����id��Ӧ��map��
	durlapitor_t											m_CurDumItor;			// ��ǰ�����durlmap_tԪ��
	// ��ǰ�����DummyUnit *
	DummyUnit												* m_pCurDummy;

	//////////////////////////////////////////////////////////////////////////
	//		��ʱ����������BeginGetUCIList������GetNextFromUCI����
	durlapitor_t											m_CurDum_GetUCIList_Itor;		
	idsetitor_t												m_CurID_GetUCIList_Itor;
	bool													m_bGetting;


	// UCI update ���
private:
	//======================
	//
	//	UCI��
	//
	//======================

	//struct uci_t: public UCI_T
	//{
	//	int nD;
	//};
	template<class _Ty>
		struct uci_less_byID
		{
			bool operator()( const _Ty &a, const _Ty &b) const
			{
				return (a.nU != b.nU ) ? (a.nU < b.nU) : (a.nI < b.nI);
			}
		};
	template<class _Ty>
		struct uciiter_less_bynD
		{
			bool operator()( const _Ty &a, const _Ty &b) const
			{
				return (a->nD != b->nD ) ? (a->nD < b->nD) : (a->nI < b->nI);
			}
		};
	typedef std::multiset<UCI_T, uci_less_byID<UCI_T> > ucitable;
	ucitable uci_table;
	bool add_uci_item(UCI_T &uci_item);

private:

	////======================
	////
	////	���� GetUCI_In
	////
	////======================

	//struct	uciin_t 
	//{
	//	int			nI;
	//	int			nU;
	//	int			nD;
	//};

	//template<class _Ty>
	//struct uciin_less
	//{
	//	inline	bool	operator() ( const _Ty & a, const _Ty & b ) const
	//	{
	//		if ( a.nU != b.nU ) {
 //               return a.nU < b.nU;
	//		}

	//		return	( a.nD != b.nD ) ? ( a.nD < b.nD ) : ( a.nI < b.nI );
	//	}
	//};


	////======================
	////
	////	���� GetUCI_Out
	////
	////======================
	//struct	 uciout_t
	//{
	//	int	nI;
	//	int	nU;
	//};

	//template<class _Ty>
	//struct uciout_less
	//{	
	//	inline	bool	operator() ( const _Ty & a, const _Ty & b ) const
	//	{
	//		return ( a.nU != b.nU ) ? ( a.nU < b.nU ) : ( a.nI < b.nI );
	//	}
	//};

	////////////////////////////////////////////////////////////////////////////
	//
	//typedef		std::set<uciin_t, uciin_less<uciin_t>, fast_allocator<uciin_t> >			uciinset_t;
	//typedef		uciinset_t::iterator														uciinsetitor_t;		// uciinset_t::iterator

	//typedef		std::set<uciout_t, uciout_less<uciout_t>, fast_allocator<uciout_t> >		ucioutset_t;
	//typedef		ucioutset_t::iterator														ucioutsetitor_t;	// ucioutset_t::iterator


	//uciinset_t					m_uciIn;
	//ucioutset_t					m_uciOut;
	//uciinsetitor_t				m_uciInItor;
	//ucioutsetitor_t				m_uciOutItor;
	//int							m_uciLastIn, m_uciLastOut;
	//int							m_uciCountIn, m_uciCountOut;
	enum
	{
		MAX_UCICOUNT = 20
	};
	


// �����Ĳ����ص�
private:
	// �����ȸ����Ĳ����ڵ�
	void _UpdateQuadtree( );	


	// ���нڵ����

	//enum	
	//{
	//	TAKE_SPACE_NODE_ID = -1024,
	//};

	// �����꣬���ڴ洢���нڵ��ռλ�ڵ��
	struct		node_point_t
	{
		int	nX;
		int	nY;		
		size_t uDummyNum;

		node_point_t( int x, int y )	: nX(x), nY(y)
		{
		};

		node_point_t()
		{
		};
	};
	typedef		std::vector< node_point_t >		node_point_vec_t;
	typedef		node_point_vec_t::iterator										node_point_vec_itor_t;		

	
	node_point_vec_t														m_vFreePoint;
	size_t																	m_vShowPos;	


	struct node_point_less
	{	
		inline	bool	operator() ( const node_point_t & a, const node_point_t & b ) const
		{
			return ( a.nX != b.nX ) ? ( a.nX < b.nX ) : ( a.nY < b.nY );
		}
	};

	typedef		std::set<node_point_t, node_point_less /*,  fast_allocator<node_point_t>*/ >			node_point_set;
	typedef		node_point_set::iterator														node_point_set_itor;

	node_point_set															m_TakeSpaceNodePointSet;	// ռλ��ļ���

    void AddFreePoint(int x, int y);


public: 
	// ��һ���Ĳ���Ҷ���ϵ����е�dummy ��id���ص�������.
	int		AddDummyToVecOnLeaf( QTreeLeaf * pLeaf, whvector<int> * v );
	// ��һ���Ĳ���Ҷ���ϵ����е�dummy ��id���ص�set��.
	int		AddDummyToSetOnLeaf( QTreeLeaf * pLeaf, std::set<int> * v );


	// debug
	//void _ptMap()
	//{
	//	std::cout << std::endl << "----------map---------- " << std::endl;
	//	for ( durlapitor_t it = m_dummyRelaMap.begin(); it != m_dummyRelaMap.end(); ++ it ) {

	//		std::cout << "[ "<< it->first << " ]" << std::endl; 
	//		if ( ! it->second.npc.empty() ) {
	//			std::cout << "npc:" << std::endl;
	//			for ( idsetitor_t itid = it->second.npc.begin(); itid != it->second.npc.end(); ++ itid  ) {
	//				std::cout << "\t" << *itid ;
	//			}
	//			std::cout << std::endl;
	//		}

	//		if ( ! it->second.player.empty() ) {
	//			std::cout << "player:" << std::endl; 
	//			for ( idsetitor_t itid = it->second.player.begin(); itid != it->second.player.end(); ++ itid  ) {
	//				std::cout << "\t" << *itid ;
	//			}
	//			std::cout << std::endl << std::endl; 
	//		}
	//	}
	//	std::cout << "==========end=========== " << std::endl << std::endl ;
	//}

// ���ٴ峤����仯����ĺ���
//private:
//	bool m_isFocused;
//public:
//	void setFocus(void)	{ m_isFocused = true; }
//	int m_idFocused;
};


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
inline bool whsightlist2D::add_uci_item(UCI_T &uci_item)
{
	try
	{
		//ucitable::iterator iter = uci_table.find(uci_item);
		//if( iter == uci_table.end() )
			uci_table.insert(uci_item);
		//else
		//{
		//	if( uci_item.nD < 0 )
		//		uci_table.erase(iter);
		//	else
		//		iter->nD = uci_item.nD;
		//}
	}
	catch(...)
	{
		assert(0 && "����uci�����\n");
		return false;
	}
	return true;
}

//// ��ȡ��ұ��ɼ��б�仯
//// ÿ��tick����һ���Σ���������ֱ������ֵ<0Ϊֹ
//// ���ӵ���ң���;�������µ��˿������Լ���Ӧ�ð��Լ��ĳ�ʼ�����ݷ��͸��Է��Ŀͻ��ˣ�
//inline	int	whsightlist2D::GetUCI_In( UCI_T * pUCI )
//{
//	if( m_uciCountIn == 0 )
//		m_uciInItor = m_uciIn.begin();
//
//	while( m_uciInItor != m_uciIn.end() )
//	{
//		(m_uciInItor->nU == m_uciLastIn) ?  ++m_uciCountIn : (m_uciLastIn = m_uciInItor->nU, m_uciCountIn = 1);
//		if( m_uciCountIn <= MAX_UCICOUNT )
//		{
//			pUCI->nI = m_uciInItor->nI;
//			pUCI->nU = m_uciInItor->nU;
//			uciinsetitor_t iterbak = m_uciInItor;
//			++m_uciInItor;
//			m_uciIn.erase(iterbak);
//
//			return	0;
//		}
//		++m_uciInItor;
//	}
//
//	if( !m_uciIn.empty() )
//		printf("In: %d\n", m_uciIn.size());
//	m_uciCountIn = 0;
//	pUCI = NULL;
//	return -1;
//
//}
//
//// ���ٵ���ң���;����ԭ�������Լ������ڿ������Լ��ˣ�Ӧ��֪ͨ�Է��ͻ���ɾ����Ӧ�����ݣ�
//inline	int	whsightlist2D::GetUCI_Out( UCI_T * pUCI )
//{
//	if( m_uciCountOut == 0 )
//		m_uciOutItor = m_uciOut.begin();
//
//	while( m_uciOutItor != m_uciOut.end() )
//	{
//		(m_uciOutItor->nU == m_uciLastOut) ? ++m_uciCountOut : (m_uciLastOut = m_uciOutItor->nU, m_uciCountOut = 1);
//		if( m_uciCountOut <= MAX_UCICOUNT )
//		{
//			( (pUCI->nI = m_uciOutItor->nI) == m_uciLastOut ) ? ++m_uciCountOut : m_uciCountOut = 1;
//			pUCI->nU = m_uciOutItor->nU;
//			ucioutsetitor_t iterbak = m_uciOutItor;
//			++m_uciOutItor;
//			m_uciOut.erase(iterbak);
//
//			return	0;
//		}
//		++m_uciOutItor;
//	}
//
//	if( !m_uciOut.empty() )
//		printf("Out: %d\n", m_uciOut.size());
//	m_uciCountOut = 0;
//	pUCI = NULL;
//	return -1;
//}
//
//
//// ��ʼ��ȡUCI�б�,������������������null, nRetID����0;
//inline	whsightlist2D::DUMMY_INFO_T * 	whsightlist2D::BeginGetUCIList( int nID, int & nRetID )
//{
//	// �Ѿ���ʼ�ˣ��򷵻ؿ�
//	if ( m_bGetting ) {
//		return	NULL;
//	}
//	// ��Ϊ�ѿ�ʼ
//	m_bGetting	= true;
//
//	if ( (m_CurDum_GetUCIList_Itor = m_dummyRelaMap.find( nID )) == m_dummyRelaMap.end() ) {
//		nRetID = -1;
//		return	NULL;
//	}
//	m_CurID_GetUCIList_Itor = (m_CurDum_GetUCIList_Itor->second).player.begin();
//
//	if ( m_CurID_GetUCIList_Itor != (m_CurDum_GetUCIList_Itor->second).player.end() ) {
//		nRetID = * m_CurID_GetUCIList_Itor;
//		return	& ( (m_Dummys.GetByID(nRetID))->info );
//	}
//
//	nRetID = -1;	
//	return	NULL;
//}
//
//// ������ȡUCI�б�,������������������null, nRetID����0;������BeginGetUCIList()�������
//inline	whsightlist2D::DUMMY_INFO_T * whsightlist2D::GetNextFromUCI( int & nRetID  )
//{
//	// δ��ʼ�򷵻ؿ�
//	if ( ! m_bGetting ) {
//		return	NULL;
//	}
//
//	++ m_CurID_GetUCIList_Itor ;
//	if ( m_CurID_GetUCIList_Itor != (m_CurDum_GetUCIList_Itor->second).player.end() ) {
//		nRetID = * m_CurID_GetUCIList_Itor;
//		return	& ( (m_Dummys.GetByID(nRetID))->info );
//	}
//
//	nRetID = -1;	
//	return	NULL;
//}
//
//// ֹͣ��ȡ
//inline	void whsightlist2D::EndGetUCIList()
//{
//	// �ֶ�����ֹͣ
//	m_bGetting	= false;
//};
//
//
//
// ��ȡUCI�б��ѽ������out�У�out�д�ŵ��ǿ��Կ���nID��outerID������б��в�������NPC
//inline void	whsightlist2D::GetUCIOutID( int nID, whvector<int> & out )
//{
//	whsightlist2D::DummyUnit * pDummy = NULL;
//	out.clear();
//
//	durlapitor_t		MapItorTmp;
//	idsetitor_t			idItorTmp;
//
//	if ( (MapItorTmp = m_dummyRelaMap.find( nID )) == m_dummyRelaMap.end() ) {
//		return;
//	}
//
//	idItorTmp = (MapItorTmp->second).player.begin();
//
//	for ( ; idItorTmp != (MapItorTmp->second).player.end(); ++ idItorTmp ) {
//		pDummy = m_Dummys.GetByID( * idItorTmp );
//		if ( pDummy == NULL ) {
//			continue;
//		}
//		out.push_back( pDummy->info.nOuterID );
//	}
//}


inline void whsightlist2D::GetUCIUserData( int nID, whvector<int> & out )
{
	whsightlist2D::DummyUnit * pDummy = NULL;

    // ֻ�ı��С�����ͷ��ڴ档
	out.resize(0);

	durlapitor_t		MapItorTmp;
	idsetitor_t			idItorTmp;

	if ( (MapItorTmp = m_dummyRelaMap.find( nID )) == m_dummyRelaMap.end() ) {
		return;
	}

	idItorTmp = (MapItorTmp->second).player.begin();

    // ��ǰ�����ڴ�
	out.reserve((MapItorTmp->second).player.size());

	for ( ; idItorTmp != (MapItorTmp->second).player.end(); ++ idItorTmp ) {
		pDummy = m_Dummys.GetByID( * idItorTmp );
		if ( pDummy == NULL ) {
			continue;
		}
		out.push_back( pDummy->info.nUserData );
	}
}


//////////////////////////////////////////////////////////////////////////

// ���һ�����ǲ���ռ�ؿ�
inline bool   whsightlist2D::IsTakeSpace( int nPointX, int nPointY )
{
	if ( (nPointX < 0) || (nPointY < 0) || (nPointX > m_nMaxX) || (nPointY > m_nMaxY) ) {
		return	true;
	}
	node_point_t	takePoint( nPointX, nPointY );
	return	( m_TakeSpaceNodePointSet.find( takePoint ) != m_TakeSpaceNodePointSet.end() );
}

//////////////////////////////////////////////////////////////////////////

inline whsightlist2D::DummyUnit * whsightlist2D::GetDummyUnit( int nUserID )
{
	DummyUnit * pDummy = m_Dummys.GetByID( nUserID );
    return	 pDummy;
}

}		// EOF namespace n_whcmn

#endif	// EOF __WHGAMESIGHT_H__
