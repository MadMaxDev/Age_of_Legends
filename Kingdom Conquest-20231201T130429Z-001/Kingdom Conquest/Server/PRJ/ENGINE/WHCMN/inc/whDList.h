// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whDList.h
// Creator      : Wei Hua (魏华)
// Comment      : D表示Distributed，基于分散对象的链表模板
// CreationDate : 2005-12-28
// ChangeLog    :
//                2005-05-10 增加了whDListXXX
//                2005-05-13 原来把whDListNode::leave中对指针清零这两个放到括号里面了，结果导致next->prev变成了NULL
//                2005-08-12 给whDList增加了成员个数统计，whDListNode增加了一个模板参数
//                2005-08-18 给whalist::InheritEnlarge增加了了参数nNewMaxNum、nNewDataUnitSize，这样alist的总长度和单元长度都可以扩展了
//                2005-12-28 把whDList从whlist.h中分出来。
//                2006-03-02 增加了GetNodePtrByDataPtr方法
//                2007-09-27 修正了insertafter(begin())会导致非法操作的问题

#ifndef	__WHDLIST_H__
#define	__WHDLIST_H__

#include <assert.h>
#include "./whcmn_def.h"

namespace n_whcmn
{

// D表示Distributed
// 由node组成的list，不基于分配的(即各个node对象不是重新分配出来的)
// 而是包含在组成list的各个对象中的，即各个对象都有node性质
template<typename _Ty>
struct	whDList
{
	struct	node
	{
	public:
		node	*prev;
		node	*next;
		whDList	*pHost;
		_Ty		data;	// data部分完全用户自己打理，list不负责维护
		// 所以他的宿主要记得给它赋值且不要随便使用memset来清空所有的成员!!!!
		node()
			: prev(NULL), next(NULL)
			, pHost(NULL)
		{
		}
		~node()
		{
			leave();
		}
		inline bool	isinlist() const
		{
			return	pHost!=NULL;
		}
		inline bool	isinlist(whDList *pList) const
		{
			return	pHost==pList;
		}
		inline void	insertafter(node *pnode)
		{
			if( !pnode->next )
			{
				insertbefore(pnode);
				return;
			}
			assert(!isinlist());
			prev		= pnode;
			next		= pnode->next;
			prev->next	= this;
			next->prev	= this;
			pHost		= pnode->pHost;
			pHost->nSize	++;
		}
		inline void	insertbefore(node *pnode)
		{
			assert(!isinlist());
			next		= pnode;
			prev		= pnode->prev;
			prev->next	= this;
			next->prev	= this;
			pHost		= pnode->pHost;
			pHost->nSize	++;
		}
		inline void	leave()
		{
			// 如果不在list中就不要离开(因为为了保险可能会多次调用leave)
			if( !isinlist() )
			{
				return;
			}
			// 如果是list的内部头尾则不要做任何操作
			if( this == &pHost->head || this == &pHost->tail )
			{
				return;
			}
			// 因为上面可以保证不是head和tail，所以不用判断指针是否为空
			assert( prev );
			prev->next	= next;
			assert( next );
			next->prev	= prev;
			// 队列尺寸--
			pHost->nSize	--;
			// 清空各个指针
			prev			= NULL;
			next			= NULL;
			pHost			= NULL;
		}
		inline void	quit()	// 为了简单，大家可以用一样的方法（如时间事件的id）
		{
			leave();
		}
		// 这是在DList也想直接reset的情况下使用的
		inline void	reset()
		{
			prev			= NULL;
			next			= NULL;
			pHost			= NULL;
		}
	};

	node	head;
	node	tail;

private:
	friend	struct node;
	int		nSize;			// 这个在每个NODE被insert和leave时都会改变
	// 注意：这个上层千万不要手动改变nSize，否则就乱套了!!!!
public:

	whDList()
	{
		reset();
	}
	// 为了安全还是不用这个析构了
	//~whDList()
	//{
	//	clear();
	//}
	void	reset()
	{
		// 连接head和tail，只有首尾是需要这样的方式连接的
		head.next		= &tail;
		tail.prev		= &head;
		head.pHost		= this;
		tail.pHost		= this;
		nSize			= 0;
	}
	inline node *	begin()
	{
		return	head.next;
	}
	inline node *	end()
	{
		return	&tail;
	}
	// 注意：如果用rxxx函数，则循环需要用prev指针而不能用next指针
	inline node *	rbegin()
	{
		return	tail.prev;
	}
	inline node *	rend()
	{
		return	&head;
	}
	inline node *	begin() const
	{
		return	head.next;
	}
	inline node *	end() const
	{
		return	(node *)&tail;	// 无奈只能这样了
	}
	// 注意：如果用rxxx函数，则循环需要用prev指针而不能用next指针
	inline node *	rbegin() const
	{
		return	tail.prev;
	}
	inline node *	rend() const
	{
		return	(node *)&head;	// 无奈只能这样了
	}
	inline void	AddToHead(node *pNode)
	{
		pNode->insertafter(&head);
	}
	inline void	AddToTail(node *pNode)
	{
		pNode->insertbefore(&tail);
	}
	inline int	size() const
	{
		return	nSize;
	}
	// 令所有元素都离队
	void	clear()
	{
		node	*pNode	= NULL;
		while( (pNode=begin()) != end() )
		{
			pNode->leave();
		}
	}
	// 通过数据指针反推出节点指针
	inline node *	GetNodePtrByDataPtr(_Ty *pData)
	{
		return	(node *)wh_getoffsetaddr(pData, -((int)wh_offsetof(node, data)));
	}
	// 内部指针校正
	void	AdjustInnerPtr(int nOffset)
	{
		// 不用特别纠正头尾了，因为wh_adjustaddrbyoffset不会纠正NULL指针
		head.pHost	= this;
		tail.pHost	= this;
		// 然后纠正每个元素
		node	*pNode	= &head;
		while( pNode != NULL )
		{
			// 调整prev和next
			if( pNode->prev!=&head )
			{
				wh_adjustaddrbyoffset(pNode->prev, nOffset);
			}
			if( pNode->next!=&tail )
			{
				wh_adjustaddrbyoffset(pNode->next, nOffset);
			}
			// 调整所属的列表指针，虽然这个没有太大必要
			pNode->pHost	= this;
			// 下一个对象
			pNode	= pNode->next;
		}
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHDLIST_H__
