/********************************************************
	Copyright 1996-2005, Pixel Software Corporation, 
	All rights reserved.

	Ŀ��: �ڴ��Ż�

    ����: Jiangli

    ��ʷ: 2007-7-4 ����

*********************************************************/
#ifndef __MEM_UTIL_H__
#define __MEM_UTIL_H__

#include <malloc.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#define PLEX_OBJ_ENABLE 1

namespace pg
{
template<bool v> struct my_static_assert{};
template<> struct my_static_assert<true> {};
template<int x> struct static_assert_test{};
}

#define PG_STATIC_ASSERT(b) typedef pg::static_assert_test<sizeof(pg::my_static_assert<(bool)(b)>)> static_assert_type_ //PG_PP_CAT(static_assert_type_, __LINE__)

// no release heap
class nr_heap
{
protected:
    struct page;

    page*   m_pages;
    char*   m_pos;
    char*   m_end;
    size_t  m_page_size;
    size_t  m_huge_size;
public:
    nr_heap(size_t page_size = 8100);
    ~nr_heap();

    void* allocate(size_t s);
    void clear();
};

// ���ٵĵȴ�С�ڴ������
class quick_plex
{
protected:
    struct page
    {
        union
        {
            page*   m_next;
            char    m_align_data[8];
        };
    };
    struct item
    {
        item*   m_next;
    };

    page*   m_pages;
    item*   m_items;
    size_t  m_page_size;
    size_t  m_item_size;

    void allocate_page();

public:
    quick_plex(size_t item_size, size_t page_size = 128);
    ~quick_plex();

    void* allocate()
	{
		if(m_items == 0)
		{
			allocate_page();
		}
		item* p = m_items;
		m_items = m_items->m_next;
		return p;
	}
    void deallocate(void* p)
    {
		((item*)p)->m_next = m_items;
		m_items = (item*)p;
    }

    static quick_plex*  m_plex[16];

    template<class T>
    static T* allocate_t()
    {
        PG_STATIC_ASSERT(sizeof(T) > 0 && sizeof(T) <= 64);
        return (T*)m_plex[(sizeof(T)-1)>>2]->allocate();
    }
    template<class T>
    static void deallocate_t(void* p)
    {
        PG_STATIC_ASSERT(sizeof(T) > 0 && sizeof(T) <= 64);
        m_plex[(sizeof(T)-1)>>2]->deallocate(p);
    }
};

template<class D>
struct quick_plex_obj
{
#if PLEX_OBJ_ENABLE
#ifdef _MSC_VER
#pragma push_macro("new")
#undef new
#endif
    void* operator new (size_t s)
    {
        assert(s == sizeof(D));
        return quick_plex::allocate_t<D>();
    }
    void* operator new (size_t s, void* p)
    {
        return p;
    }
    void operator delete (void* p, void*)
    {
        quick_plex::deallocate_t<D>(p);
    }
    void operator delete (void* p)
    {
        quick_plex::deallocate_t<D>(p);
    }
    void* operator new (size_t s, int, const char* , int )
    {
        assert(s == sizeof(D));
        return quick_plex::allocate_t<D>();
    }
    void operator delete (void* p, int, const char* , int )
    {
        quick_plex::deallocate_t<D>(p);
    }
    void* operator new (size_t s, const char* , int )
    {
        assert(s == sizeof(D));
        return quick_plex::allocate_t<D>();
    }
    void operator delete (void* p, const char* , int )
    {
        quick_plex::deallocate_t<D>(p);
    }
#ifdef _MSC_VER
#pragma pop_macro("new")
#endif
#endif // PLEX_OBJ_ENABLE

};

// quick single list
template<class T>
struct qs_list
{
    T*      m_head;
    T*      m_tail;
    size_t  m_size;

    qs_list()
    {
        m_head  = 0;
        m_tail  = 0;
        m_size  = 0;
    }
    void push_back(T* p)
    {
        if(m_head == 0)
        {
            m_head = p;
        }
        else
        {
            m_tail->m_next = p;
        }
        p->m_next = 0;
        m_tail = p;
        ++ m_size;
    }
};

// �������Ҫ��Ϊ���Ż����ǡ�
// ȱ�㣺���ʺ���Ƶ�������ɾ����Ӧ�á�
// �ŵ㣺1 �����ȶ��ı�����Ч��Ҫ����map��
//       2 ����ʱ�����ط����µ��ڴ档map<string, ...>�Ĳ���ʵ����Ҫ����һ����ʱstring����ġ�
struct string_table
{
    struct  item
    {
        void*   m_value;
        char    m_name[1];
    };
    item**  m_data;
    size_t  m_size;
    size_t  m_capacity;

    string_table();
    ~string_table() { clear(); }

    // û�ҵ�����NULL
    item* find(const char* n);

    // ���򷵻����ݣ�û���򷵻��½�������
    item* get(const char* n);
    item* get(const char* n, bool& has);

    // �����µĻ��߸��Ǿɵ�
    item* insert(const char* n, void* v);

    void erase(const char* n);
    void clear();
};

#endif // __MEM_UTIL_H__
