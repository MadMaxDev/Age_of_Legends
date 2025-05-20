// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whDList.h
// Creator      : Wei Hua (κ��)
// Comment      : D��ʾDistributed�����ڷ�ɢ���������ģ��
// CreationDate : 2005-12-28
// ChangeLog    :
//                2005-05-10 ������whDListXXX
//                2005-05-13 ԭ����whDListNode::leave�ж�ָ�������������ŵ����������ˣ��������next->prev�����NULL
//                2005-08-12 ��whDList�����˳�Ա����ͳ�ƣ�whDListNode������һ��ģ�����
//                2005-08-18 ��whalist::InheritEnlarge�������˲���nNewMaxNum��nNewDataUnitSize������alist���ܳ��Ⱥ͵�Ԫ���ȶ�������չ��
//                2005-12-28 ��whDList��whlist.h�зֳ�����
//                2006-03-02 ������GetNodePtrByDataPtr����
//                2007-09-27 ������insertafter(begin())�ᵼ�·Ƿ�����������

#ifndef	__WHDLIST_H__
#define	__WHDLIST_H__

#include <assert.h>
#include "./whcmn_def.h"

namespace n_whcmn
{

// D��ʾDistributed
// ��node��ɵ�list�������ڷ����(������node���������·��������)
// ���ǰ��������list�ĸ��������еģ�������������node����
template<typename _Ty>
struct	whDList
{
	struct	node
	{
	public:
		node	*prev;
		node	*next;
		whDList	*pHost;
		_Ty		data;	// data������ȫ�û��Լ�����list������ά��
		// ������������Ҫ�ǵø�����ֵ�Ҳ�Ҫ���ʹ��memset��������еĳ�Ա!!!!
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
			// �������list�оͲ�Ҫ�뿪(��ΪΪ�˱��տ��ܻ��ε���leave)
			if( !isinlist() )
			{
				return;
			}
			// �����list���ڲ�ͷβ��Ҫ���κβ���
			if( this == &pHost->head || this == &pHost->tail )
			{
				return;
			}
			// ��Ϊ������Ա�֤����head��tail�����Բ����ж�ָ���Ƿ�Ϊ��
			assert( prev );
			prev->next	= next;
			assert( next );
			next->prev	= prev;
			// ���гߴ�--
			pHost->nSize	--;
			// ��ո���ָ��
			prev			= NULL;
			next			= NULL;
			pHost			= NULL;
		}
		inline void	quit()	// Ϊ�˼򵥣���ҿ�����һ���ķ�������ʱ���¼���id��
		{
			leave();
		}
		// ������DListҲ��ֱ��reset�������ʹ�õ�
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
	int		nSize;			// �����ÿ��NODE��insert��leaveʱ����ı�
	// ע�⣺����ϲ�ǧ��Ҫ�ֶ��ı�nSize�������������!!!!
public:

	whDList()
	{
		reset();
	}
	// Ϊ�˰�ȫ���ǲ������������
	//~whDList()
	//{
	//	clear();
	//}
	void	reset()
	{
		// ����head��tail��ֻ����β����Ҫ�����ķ�ʽ���ӵ�
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
	// ע�⣺�����rxxx��������ѭ����Ҫ��prevָ���������nextָ��
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
		return	(node *)&tail;	// ����ֻ��������
	}
	// ע�⣺�����rxxx��������ѭ����Ҫ��prevָ���������nextָ��
	inline node *	rbegin() const
	{
		return	tail.prev;
	}
	inline node *	rend() const
	{
		return	(node *)&head;	// ����ֻ��������
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
	// ������Ԫ�ض����
	void	clear()
	{
		node	*pNode	= NULL;
		while( (pNode=begin()) != end() )
		{
			pNode->leave();
		}
	}
	// ͨ������ָ�뷴�Ƴ��ڵ�ָ��
	inline node *	GetNodePtrByDataPtr(_Ty *pData)
	{
		return	(node *)wh_getoffsetaddr(pData, -((int)wh_offsetof(node, data)));
	}
	// �ڲ�ָ��У��
	void	AdjustInnerPtr(int nOffset)
	{
		// �����ر����ͷβ�ˣ���Ϊwh_adjustaddrbyoffset�������NULLָ��
		head.pHost	= this;
		tail.pHost	= this;
		// Ȼ�����ÿ��Ԫ��
		node	*pNode	= &head;
		while( pNode != NULL )
		{
			// ����prev��next
			if( pNode->prev!=&head )
			{
				wh_adjustaddrbyoffset(pNode->prev, nOffset);
			}
			if( pNode->next!=&tail )
			{
				wh_adjustaddrbyoffset(pNode->next, nOffset);
			}
			// �����������б�ָ�룬��Ȼ���û��̫���Ҫ
			pNode->pHost	= this;
			// ��һ������
			pNode	= pNode->next;
		}
	}
};

}		// EOF namespace n_whcmn

#endif	// EOF __WHDLIST_H__
