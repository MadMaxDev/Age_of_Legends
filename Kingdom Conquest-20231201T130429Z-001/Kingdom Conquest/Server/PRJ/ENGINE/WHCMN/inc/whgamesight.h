// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whgamesight.h
// Creator      : Wei Hua (魏华)
// Comment      : 和游戏中视野相关的功能。比如同屏表。
// CreationDate : 2006-06-06
// ChangeLOG    : 2006-06-06 制定接口
//              : 2006-06-07 吕宝虹接手实现
//				  2006-06-30 增加获取GetUCIOutID的接口,该接口用于获取UCI的outerID的列表,列表中不包含NPC
//				  2006-07-02 修改GetUCI_In和GetUCI_Out返回的结构,和相应的内部数据.
//				  2006-07-19 增加了一个AddNpcFirst接口,用于服务器初始化时,在没有玩家的情况下调用.
//								内部结构重写, 原用set<id,id,d>存储同屏列表,现在使用map<id, ralation>结构存储.ralation使用了蒋黎的fast_allocator
//				  2006-10-25 增加TakeSpace()函数，UnTakeSpace()，IsTakeSpace()，用来设置占地块。修改SearchFreePoint()和SearchFreePointNext()的返回值类型，由bool变为int，
//								返回找到的一个可以植入dummy的节点的已有的人数。并且内部改为内部小于4个dummy即视为空闲节点。
//				  2006-12-18 增加带回调的SearchBox和SearchRay，增加EndGetUCIList()
//				  2007-03-06 增加GetUCIUserData()函数,该接口用于获取UCI的nUserData的列表,列表中不包含NPC
//				  2007-03-09 修改，使没有dummy的四叉树的leaf都delete掉。增加1次移动计算内，同一dummy多次调用dummyGO和回到原来点的问题。
//				  2007-03-22 增加GetDummyUnit函数，返回DummyUnit指针
//				  2007-06-05 为跟踪“村长消失”问题增加变量m_idFocused及相关代码
//				  2007-08-08 修改uciinset_t和ucioutset_t的排序方式，以nU为排序主键，这样这两个set里给同一个玩家的信息就集中在一起了
//				  2007-08-08 修改GetUCI_In和GetUCI_Out为每批次调用只返回同一个人物的视野变化列表的前MAX_UCICOUNT个，剩余的推迟到下批再返回
//				  2007-09-05 为whsightlist2D::SearchBox增加一个重载，用于把结果保存在set中（用于需要把ID排序的场合）
//				  2007-12-20 设置玩家对玩家的视野表上限，可通过whsightlist2D::INFO_T::nMaxPlayerSeen修改，缺省为一个玩家最多能看见周围的100个玩家，如果本来应该能看见的两个玩家其中一方的数目满了，则双方的视野表都不会发生改变
//				  2008-06-06 取消uci_table的唯一性检查（可能会引发问题，需要在以后注意观察）
//				  2008-07-22 注释掉一些已经废弃的代码

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
//	蒋黎的fast_allocator
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
		return 0;	// 临时修改一下，免得在gcc4下编译不通过
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
	short           m_x;        // 这个节点保存的是左上角的坐标。
    short           m_y;
	unsigned char   m_level;    // Level为0表示是叶子
	unsigned char   m_index;    // 表明自己是父节点中的哪个(0~3)，这个的作用是可以在删除时快速的清除父节点中的索引
    unsigned char   m_mask;     // 表明各个孩子是否存在
};

struct QTreeTrunk : QTreeNode
{
    QTreeNode*      m_child[4];
};

struct QTreeLeaf : QTreeNode
{
    // 这里没有数据，需要使用QTreeNodeMgr来扩展这个节点
};

// 不同的qtree可以使用同一个内存管理器，这样可以复用节点数据。
struct QTreeNodeMgr
{
	virtual ~QTreeNodeMgr() {}
    virtual QTreeTrunk* AllocateTrunk() = 0;
    virtual void DeallocateTrunk(QTreeTrunk*) = 0;

    virtual QTreeLeaf* AllocateLeaf() = 0;
    virtual void DeallocateLeaf(QTreeLeaf*) = 0;
};


// 下面的函数提供对节点和叶子的处理。比如Release时外界对pData数据的释放。
typedef	int	(*QTreeTrunkFunc)(QTreeTrunk* p, void* ctx);
typedef	int	(*QTreeLeafFunc)(QTreeLeaf* p, void* ctx);

// 这个函数提供SearchBox的结果节点的过滤和保存的功能（如：内部按照距离排序，比如某种类型的节点不加入最终结果）
// p为叶子节点结构
// dx, dy为叶子到指定点的二维距离，这个主要是为了避免在回调中多余的计算
// ctx为传回给回调的参数（如相关对象的指针）
// 返回非零值表示SearchBox停止搜索，把返回值值返回给调用者（比如返回正数表示可以停止搜索）
typedef	int	(*QTreeSearchBoxFunc)(QTreeLeaf* p, short dx, short dy, void* ctx);

// 这个函数提供SearchRay的结果节点的过滤和保存的功能
// p为叶子节点结构
// ctx为传回给回调的参数（如相关对象的指针）
// 返回非零值表示SearchRay停止搜索，把返回值值返回给调用者（比如返回正数表示可以停止搜索）
typedef	int	(*QTreeSearchRayFunc)(QTreeLeaf* p, void *ctx);

struct QTree
{
    // 数据均为扩展函数使用，不要直接访问。
    typedef QTreeTrunk* QTreeTrunkPtr;
    QTreeNodeMgr*   m_mgr;
    QTreeTrunk*     m_root;     // 根节点始终存在，不会删除，这样会带来一些便利。
    QTreeTrunkPtr** m_patch;    // 分块的二维数组
    short           m_size;     // 区域的大小

    // 公用函数

	QTree();
	~QTree();

	bool Create(QTreeNodeMgr* mgr, int max_level);
	void Destroy();

	// Goover是由子到父的顺序的遍历
	int	Goover(QTreeTrunkFunc func_trunk, QTreeLeafFunc func_leaf, void* ctx);

	// 插入叶子节点，如果原来有就返回原来的
	QTreeLeaf* Insert(short x, short y);

	// 根据坐标查找叶子节点
	QTreeLeaf* FindLeaf(short x, short y);

	// 根据坐标查找最底层节点（可以是叶子也可能是非叶子）
	QTreeNode* FindNode(short x, short y);

	// 根据坐标删除叶子节点
	void Delete(short x, short y);

	// 根据叶子节点指针删除（这个一定要保证节点指针有效，并且是存在于管理器中的，并且上次自己做节点附加数据的release工作）
	void Delete(QTreeLeaf *p);

	// 查找从某点开始矩形形范围内的叶子节点
	// x,y表示中心点坐标，rx, ry表示x,y方向上各自的范围。
	// 真实跨度为r*2+1（即中心点两边各延长r的距离）。
	// 如果正常则返回0，否则返回pJudgeFunc的非零返回值
	int	SearchBox(short x, short y, short rx, short ry, QTreeSearchBoxFunc func, void* ctx);

	// 查找从某点开始的指定长线段内碰到的叶子节点
	// x1,y1为起始点坐标。x2,y2表示了射线段的终点
	// range表示碰撞的扩展范围（因为在离散情况下线段碰点大部分情况都是碰不上的（除了端点和中间整点），所以需要把要碰撞点的范围扩大）
	// range一般取值为1
	// 如果正常则返回0，否则返回pJudgeFunc的非零返回值
	int	SearchRay(short x1, short y1, short x2, short y2, short range, QTreeSearchRayFunc func, void* ctx);
};

class	whsightlist2D
{
public:
	//	视野表的 基础数据结构和操作
	struct	INFO_T
	{
		int	nMaxDummy;			// 最多的Dummy数量
		int	nChunkSize;			// 给分配器的参数：每次整块分配的大小
		int	nMaxSightX;			// X方向的最长视野
		int	nMaxSightY;			// Y方向的最长视野
		int	nGoCheckDelay;		// 移动判断的延时（毫秒）
		int	nLodLevel;
		size_t nMaxPlayerSeen;		// 最多能见的玩家数
		INFO_T()
			: nMaxDummy(500),	nChunkSize(1024*1024),	nMaxSightX(16),		nMaxSightY(16),		nGoCheckDelay(1000),	nLodLevel(12),	nMaxPlayerSeen(0)
		{	};
	};
private:
	int nMaxLeaveSightX, nMaxLeaveSightY;	// 出视野的最大距离，设置为进视野距离的120%

public:
	// dummy的信息
	struct	DUMMY_INFO_T
	{
		enum
		{
			TYPE_NOTHING		= 0,	// 未定义对象
			TYPE_PLAYER			= 1,	// 玩家对象
			TYPE_NPC			= 2,	// 所有非玩家对象
			TYPE_GOD			= 3,	// 这种东西的视野是无限的，能看到所有的东西，也就是说他存在于所有东西的被可见表中
			TYPE_BOOTH			= 4,	// 摆摊的玩家，能被所有人看见（不受视野内可见玩家数量上限的限制）
			TYPE_PET			= 5,	// 玩家的宠物，不参与视野表的变化计算
		};
		short	nType;			// 对象类型（见前面的TYPE_XXX）
		int		nUserData;		// 上层自己定义的数据，主要为了让上层自己区分一些东西
		int		nOuterID;		// 上层用的对象ID
	};


		// 内部保存的用的用于人的信息
	struct	DummyUnit
	{
		//// 视野
		//int				nSightX;
		//int				nSightY;
		// 上次坐标
		int				nX;
		int				nY;
		// 最新坐标
		int				nNewX;
		int				nNewY;
		// 外界填写的信息
		DUMMY_INFO_T	info;

		// 清空信息
		void	clear()
		{
			WHMEMSET0THIS();
		}
	};


	whsightlist2D();

	~whsightlist2D();

	// 初始化和参数设置
	int	Init( INFO_T * pInfo );

	// 终结
	int	Release();

	// 逻辑运行（比如延时的移动判断）
	int	Tick();

	// 添加玩家(或NPC)，返回ID
	// int nX, int nY是初始的位置
	// 默认的玩家视野就是最长视野
	int	AddDummy( whsightlist2D::DUMMY_INFO_T *pDummyInfo, int nX, int nY );

	// 可以用在服务端启动的时候插入NPC,这个函数内部不会检索四叉树.
	int	AddNpcFirst( whsightlist2D::DUMMY_INFO_T *pDummyInfo, int nX, int nY );

	// 获得玩家数据
	const whsightlist2D::DUMMY_INFO_T *	GetDummy( int nID );

	// 删除玩家
	int	DelDummy( int nID );

	//// 调整玩家视野（nSightX和nSightY表示X和Y方向上的视野，他的可见范围应该是2X*2Y的矩形。注意：不能超过最长视野）
	//int	AdjustDummySight( int	nID, int nSightX, int nSightY );

	// 玩家运动
	int	DummyGo( int nID, int nX, int nY );

	//////////////////////////////////////////////////////////////////////////
	// 搜索一个范围内的所有dummy 的id,返回到数组中.
	void	SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, whvector<int> & vID );

	// 搜索一个范围内的所有dummy 的id,返回到set中.
	void	SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, std::set<int> & vID );

	// 搜索一个射线内的所有dummy 的id,
	void	SearchRay(int nStartX, int nStartY, int nEndX, int nEndY, int nExtendRange, whvector<int> & vID );

	// 搜索一个范围内的所有dummy 的id,返回到数组中.带回调的
	void	SearchBox( int nCenterX, int nCenterY, int nRadiusX, int nRadiusY, SearchBox_Fun_t pCallBack );

	// 搜索一个射线内的所有dummy 的id,.带回调的
	void	SearchRay(int nStartX, int nStartY, int nEndX, int nEndY, int nExtendRange, SearchRay_Fun_t pCallBack );



	// 将在一个点的所有dummy都插入到返回的数组中.
	void	GetDummyOnPoint( int nX, int nY, whvector<int> & vID );

	// 查找一个点附近空闲的点.nPointX和nPointY是要查找的范围的中心点.并且作为结果返回.
	// 结果中不包括占地块，成功返回地方所占的人数0-3，失败返回-1
	int   SearchFreePoint( int & nPointX, int & nPointY, int iRadius );

	// 继续上次查找的结果继续查找.nPointX和nPointY只做为结果返回.并不做为中心点输入,
	//	查找的中心点在SearchFreePoint函数中已经保存下来了.
	// 结果中不包括占地块，成功返回地方所占的人数0-3，失败返回-1
	int   SearchFreePointNext( int & nPointX, int & nPointY );

	// 设置某个点是一个占地块
	bool	  TakeSpace( int nPointX, int nPointY );

	// 取消设置某个点是一个占地块
	bool	  UnTakeSpace( int nPointX, int nPointY );

	// 检测一个点是否是占地块
	bool	  IsTakeSpace( int nPointX, int nPointY );
	
	// 获取一个dummy
	DummyUnit * GetDummyUnit( int userID );

public:	
	// UCI列表
	struct	UCI_T
	{
		int			nU;
		int			nI;
		mutable int	nD;
	};
	typedef std::vector<UCI_T> UCI_Inc_Table;
	// 获取玩家可见列表变化
	// 每个tick调用一次，包括每个玩家的可见列表的全部出视野玩家及部分或全部入视野玩家（总和不超过MAX_UCICOUNT）
	bool GetUCIincTable(UCI_Inc_Table &incTable);

	//// 获取玩家被可见列表变化
	//// 每个tick调用一批次，持续调用直到返回值<0为止
	//// 增加的玩家（用途：即有新的人看到了自己，应该把自己的初始化数据发送给对方的客户端）
	//int	GetUCI_In(UCI_T *pUCI);

	//// 减少的玩家（用途：即原来看到自己人现在看不到自己了，应该通知对方客户端删除相应的数据）
	//int	GetUCI_Out(UCI_T *pUCI);

	//// 开始获取UCI列表,如果查找完毕则函数返回null, nRetID返回0;
	//inline	DUMMY_INFO_T * 	BeginGetUCIList( int nID, int & nRetID );

	//// 继续获取UCI列表,如果查找完毕则函数返回null, nRetID返回0;必须在BeginGetUCIList()后面调用
	//inline	DUMMY_INFO_T * GetNextFromUCI( int & nRetID   );

	//// 停止当前的获取UCI的行为。在调用新的begin前，必须调用这个end，否则不会get成功
	//inline	void EndGetUCIList();


	//// 获取UCI列表，把结果存在out中，out中存放的是可以看见nID的outerID，这个列表中并不包含NPC
	//inline void	GetUCIOutID( int nID, whvector<int> & out );

	// 获取UCI列表，把结果存在out中，out中存放的是可以看见nID的nUserData，这个列表中并不包含NPC
	inline void GetUCIUserData( int nID, whvector<int> & out );




public:	



	// 四叉树调用
	int	_OnAddDum( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY );

	// tick调用
	int	_OnDumGo( int nID );

	// 四叉树调用
	int	_OnProDumGo( QTreeLeaf * pLeaf, quadtree_coord_t nDistX, quadtree_coord_t nDistY );
	

	// 带回调的 四叉树调用
	int _CallOutCallbackFun_Ray( QTreeLeaf * pLeaf, SearchRay_Fun_t fun );
	
	int _CallOutCallbackFun_Box( QTreeLeaf * pLeaf, SearchBox_Fun_t fun );
	
	typedef		std::set<int, std::less<int>, fast_allocator<int> >		idset_t;		// ID Set
	typedef		idset_t::iterator										idsetitor_t;	// ID Set::itetator




private:

	//typedef		QuadtreeMan<whcmnallocationobj, whchunkallocationobjonesize<whcmnallocationobj> >		qtman_t;	// 四叉树




	INFO_T													m_info;
	whcmnallocationobj										m_cao;
	whunitallocatorFixed2<DummyUnit, whcmnallocationobj>	m_Dummys;
    QTree*                                                  m_qtree;
	//qtman_t													* m_pQuadTreeMan;
	idset_t													m_godset;			// GOD SET


	// 被见列表（可见列表？两者应该是对等的，在代码里我常作可见列表使用）....
	struct		beSee 
	{
		idset_t		npc;		// 一个dummy被NPC看见没有意义,这里只是为了删除NPC的被可见列表用.
		idset_t		player;		// 一个dummy可以被玩家看见的那些玩家的列表.
		idset_t		booth;		// 一个dummy可见的摊位列表
	};


	typedef	std::map<int, beSee>							durlmap_t;	
	typedef	durlmap_t::iterator								durlapitor_t;		// Map durlmap_t::iterator


	//	记录了每个dummy id对应的 dummyRelation_t
	durlmap_t												m_dummyRelaMap;

	//	所有更新的需要计算的dummy id
	idset_t													m_updateSet_Move;

	// 记录下次进行处理的tick
	whtick_t 												m_tNextTick;


	int														m_nMaxX;
	int														m_nMaxY;

	//////////////////////////////////////////////////////////////////////////
	//		临时变量,用于加速添加和移动dummy的过程	

	// 当前处理id对应的map项
	durlapitor_t											m_CurDumItor;			// 当前处理的durlmap_t元素
	// 当前处理的DummyUnit *
	DummyUnit												* m_pCurDummy;

	//////////////////////////////////////////////////////////////////////////
	//		临时变量，用于BeginGetUCIList（）和GetNextFromUCI（）
	durlapitor_t											m_CurDum_GetUCIList_Itor;		
	idsetitor_t												m_CurID_GetUCIList_Itor;
	bool													m_bGetting;


	// UCI update 相关
private:
	//======================
	//
	//	UCI表
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
	////	用于 GetUCI_In
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
	////	用于 GetUCI_Out
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
	


// 用于四叉树回调
private:
	// 必须先更新四叉树节点
	void _UpdateQuadtree( );	


	// 空闲节点相关

	//enum	
	//{
	//	TAKE_SPACE_NODE_ID = -1024,
	//};

	// 点坐标，用于存储空闲节点和占位节点的
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

	node_point_set															m_TakeSpaceNodePointSet;	// 占位点的集合

    void AddFreePoint(int x, int y);


public: 
	// 将一个四叉树叶子上的所有的dummy 的id返回到数组中.
	int		AddDummyToVecOnLeaf( QTreeLeaf * pLeaf, whvector<int> * v );
	// 将一个四叉树叶子上的所有的dummy 的id返回到set中.
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

// 跟踪村长坐标变化所需的函数
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
		assert(0 && "插入uci表出错\n");
		return false;
	}
	return true;
}

//// 获取玩家被可见列表变化
//// 每个tick调用一批次，持续调用直到返回值<0为止
//// 增加的玩家（用途：即有新的人看到了自己，应该把自己的初始化数据发送给对方的客户端）
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
//// 减少的玩家（用途：即原来看到自己人现在看不到自己了，应该通知对方客户端删除相应的数据）
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
//// 开始获取UCI列表,如果查找完毕则函数返回null, nRetID返回0;
//inline	whsightlist2D::DUMMY_INFO_T * 	whsightlist2D::BeginGetUCIList( int nID, int & nRetID )
//{
//	// 已经开始了，则返回空
//	if ( m_bGetting ) {
//		return	NULL;
//	}
//	// 设为已开始
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
//// 继续获取UCI列表,如果查找完毕则函数返回null, nRetID返回0;必须在BeginGetUCIList()后面调用
//inline	whsightlist2D::DUMMY_INFO_T * whsightlist2D::GetNextFromUCI( int & nRetID  )
//{
//	// 未开始则返回空
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
//// 停止获取
//inline	void whsightlist2D::EndGetUCIList()
//{
//	// 手动调用停止
//	m_bGetting	= false;
//};
//
//
//
// 获取UCI列表，把结果存在out中，out中存放的是可以看见nID的outerID，这个列表中并不包含NPC
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

    // 只改变大小，不释放内存。
	out.resize(0);

	durlapitor_t		MapItorTmp;
	idsetitor_t			idItorTmp;

	if ( (MapItorTmp = m_dummyRelaMap.find( nID )) == m_dummyRelaMap.end() ) {
		return;
	}

	idItorTmp = (MapItorTmp->second).player.begin();

    // 提前分配内存
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

// 检测一个点是不是占地块
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
