
#include "../inc/MemUtil.h"
#include <string.h>

// nr_heap

struct nr_heap::page
{
    page*   m_next;
};

nr_heap::nr_heap(size_t page_size)
{
    m_pages     = 0;
    m_pos       = 0;
    m_end       = 0;
    m_page_size = page_size;

    if(m_page_size < 1000)
    {
        m_page_size = 1000;
    }
    else if(m_page_size > 1048500)
    {
        m_page_size = 1048500;
    }

    m_huge_size = m_page_size >> 1;
}

nr_heap::~nr_heap()
{
    clear();
}

void nr_heap::clear()
{
    while(m_pages)
    {
        void* p = m_pages;
        m_pages = m_pages->m_next;
        ::free(p);
    }
	m_pos = 0;
	m_end = 0;
}

void* nr_heap::allocate(size_t s)
{
    s = (s+3)&(~3);
    size_t r = m_end-m_pos;
    if(r >= s)
    {
        char* p = m_pos;
        m_pos += s;
        return p;
    }
    if(s >= m_huge_size)
    {
        page* pg = (page*)::malloc(sizeof(page)+s);
        pg->m_next = m_pages;
        m_pages = pg;
        return pg+1;
    }
    page* pg = (page*)::malloc(sizeof(page)+m_page_size);
    pg->m_next = m_pages;
    m_pages = pg;
    char* p = (char*)(pg+1);
    m_pos = p+s;
    m_end = p+m_page_size;
    return p;
}

// quick_plex
#ifdef _MSC_VER
#pragma warning(disable : 4073)
#pragma init_seg(lib)
#define INIT_ATTRIBUTE
#else
#define INIT_ATTRIBUTE __attribute__((init_priority(2000)))
#endif

namespace
{
template<int S> struct quick_plex_n : quick_plex
{
    quick_plex_n() : quick_plex((S+1)*4) {}
};
quick_plex_n<0> s_plex0 INIT_ATTRIBUTE;
quick_plex_n<1> s_plex1 INIT_ATTRIBUTE;
quick_plex_n<2> s_plex2 INIT_ATTRIBUTE;
quick_plex_n<3> s_plex3 INIT_ATTRIBUTE;
quick_plex_n<4> s_plex4 INIT_ATTRIBUTE;
quick_plex_n<5> s_plex5 INIT_ATTRIBUTE;
quick_plex_n<6> s_plex6 INIT_ATTRIBUTE;
quick_plex_n<7> s_plex7 INIT_ATTRIBUTE;
quick_plex_n<8> s_plex8 INIT_ATTRIBUTE;
quick_plex_n<9> s_plex9 INIT_ATTRIBUTE;
quick_plex_n<10> s_plex10 INIT_ATTRIBUTE;
quick_plex_n<11> s_plex11 INIT_ATTRIBUTE;
quick_plex_n<12> s_plex12 INIT_ATTRIBUTE;
quick_plex_n<13> s_plex13 INIT_ATTRIBUTE;
quick_plex_n<14> s_plex14 INIT_ATTRIBUTE;
quick_plex_n<15> s_plex15 INIT_ATTRIBUTE;
} // namespace

quick_plex* quick_plex::m_plex[16] =
{
    &s_plex0,  &s_plex1,  &s_plex2,  &s_plex3,
    &s_plex4,  &s_plex5,  &s_plex6,  &s_plex7,
    &s_plex8,  &s_plex9,  &s_plex10, &s_plex11,
    &s_plex12, &s_plex13, &s_plex14, &s_plex15,
};

quick_plex::quick_plex(size_t item_size, size_t page_size)
{
	m_page_size = page_size;
	m_item_size = item_size;
	m_pages     = 0;
	m_items     = 0;
	if(m_item_size < sizeof(void*))
	{
		m_item_size = sizeof(void*);
	}	
}

quick_plex::~quick_plex()
{
	while(m_pages)
	{
		page* p = m_pages;
		m_pages = p->m_next;
		free(p);
	}
}

void quick_plex::allocate_page()
{
	size_t is = m_item_size;
	size_t ps = sizeof(page)+is*m_page_size;

    page* pg = (page*)::malloc(ps);
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
}

static string_table::item** string_table_search(string_table* tab, const char* n, bool& has)
{
    string_table::item** p = tab->m_data;
    size_t c = tab->m_size;
	while(c > 0)
	{
        size_t half = c/2;
        string_table::item** mid = p+half;

        int i = strcmp((*mid)->m_name, n);
        if(i < 0)
        {
            p = ++ mid;
            c -= half+1;
        }
        else if(i > 0)
        {
            c = half;
        }
        else
        {
            has = true;
            return mid;
        }
	}
    has = false;
    return p;
}

static string_table::item* string_table_allocate(const char* n, void* v)
{
    string_table::item* p = (string_table::item*)::malloc(sizeof(string_table::item)+strlen(n));
    strcpy(p->m_name, n);
    p->m_value = v;
    return p;
}

static string_table::item** string_table_insert(string_table* tab, string_table::item** p, const char* n, void* v)
{
    string_table::item** d = tab->m_data;
    size_t c = tab->m_size;
    size_t s = p-d;
    ++tab->m_size;
    if(c < tab->m_capacity)
    {
        memmove(p+1, p, (c-s)*sizeof(*p));
    }
    else
    {
        tab->m_capacity = tab->m_size*2;
        tab->m_data = (string_table::item**)::malloc(sizeof(*p)*tab->m_capacity);

        if(c)
        {
            memcpy(tab->m_data, d, s*sizeof(*p));
            memcpy(tab->m_data+s+1, p, (c-s)*sizeof(*p));
        }
        p = tab->m_data+s;
        ::free(d);
    }

    *p = string_table_allocate(n, v);
    return p;
}

string_table::string_table()
{
    m_data  = 0;
    m_size  = 0;
    m_capacity = 0;
}

string_table::item* string_table::find(const char* n)
{
    bool has;
    item** p = string_table_search(this, n, has);
    if(has) return *p;
    return 0;
}

string_table::item* string_table::get(const char* n)
{
    bool has;
    item** p = string_table_search(this, n, has);
    if(has) return *p;

    p = string_table_insert(this, p, n, 0);
    return *p;
}

string_table::item* string_table::get(const char* n, bool& has)
{
    item** p = string_table_search(this, n, has);
    if(has) return *p;

    p = string_table_insert(this, p, n, 0);
    return *p;
}

string_table::item* string_table::insert(const char* n, void* v)
{
    item* p = get(n);
    p->m_value = v;
    return p;
}

void string_table::erase(const char* n)
{
    bool has;
    item** p = string_table_search(this, n, has);
    if(!has) return;

    ::free(*p);
    -- m_size;
    ::memmove(p, p+1, (m_size-(p-m_data))*sizeof(*p));
}

void string_table::clear()
{
    if(m_data == 0) return;

    for(size_t i = 0; i < m_size; ++ i)
    {
        ::free(m_data[i]);
    }

    ::free(m_data);
    m_size = 0;
    m_capacity = 0;
	m_data = 0;
}
