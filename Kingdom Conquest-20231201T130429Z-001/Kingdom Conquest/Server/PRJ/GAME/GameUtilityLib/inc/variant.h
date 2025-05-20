// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2007-9-28

#ifndef __VARIANT_H_PG_BASE_VESSEL__
#define __VARIANT_H_PG_BASE_VESSEL__

#include "DMtype.h"
#include "misc.h"

#include <string>
using namespace std;

enum tty_variant_t
{
    vt_none,

    // int type
    vt_int,
    vt_uint,
    vt_int64,
    vt_uint64,
    vt_pointer,

    // float value
    vt_float,
    vt_double,

    // object type
    vt_string, // string must be the first object type.
    vt_buffer,
    vt_array,
    vt_table,
};

struct variant
{
    //typedef function<bool(const variant&, const variant&), memory::pool_allocator<byte_t> > compare_fn_t;

    struct data_cell;
    struct data_info
    {
        volatile int  m_ref;
        size_t      m_size;
        size_t      m_capacity;
        union
        {
            variant*    m_vars;
            data_cell** m_cell;
            void*       m_data;
        };
    };

    uint_t  m_type;
    union
    {
        uint64_t    m_data;
        int         m_int;
        uint_t      m_uint;
        int64_t     m_int64;
        uint64_t    m_uint64;
        float       m_float;
        double      m_double;
        void*       m_pointer;
        string*		m_string;
        data_info*  m_info;
    };

    variant();
    variant(int v);
    variant(size_t v);
    variant(float v);
    variant(double v);
    variant(int64_t v);
    variant(uint64_t v);
    variant(void* v);
    variant(const char* v);
    variant(const string& v);
    variant(const variant& v);
    ~variant();

    // type
    uint_t type() const;
    void type(uint_t t);
    bool valid() const;

    // convertion
    int      to_int() const;
    uint_t   to_uint() const;
    float    to_float() const;
    double   to_double() const;
    int64_t  to_int64() const;
    uint64_t to_uint64() const;
    void*    to_pointer() const;
    string	 to_string() const;

    // container property
    size_t size() const; // sizeof string, array, table.
    bool empty() const;
    void clear();

    // container traverse
    variant fetch(size_t i) const;
    bool fetch(size_t i, variant& v) const;
    bool fetch(size_t i, variant& n, variant& v) const;

    // buffer
    void bset(void*,size_t);
    void* bget();
    void bset(const variant&,void*,size_t);
    void* bget(const variant&);

    // container operation
    void reserve(size_t c);
    void push_front(const variant& v);  // array
    void push_back(const variant& v);   // array
    void insert(size_t i, const variant& v); // array
    void erase(const variant& n);
    bool has(const variant& n) const;
    void set(const variant& n, const variant& v);
    variant get(const variant& n) const;
    variant& ref(const variant& n);
    size_t index(const variant& n) const;
    size_t lower_bound(const variant& n) const;
    size_t upper_bound(const variant& n) const;
    variant find(const variant& field, const variant& value);
    variant copy() const;


    // collect table field to array. push_back(src.fetch(...).get(n))
    void collect(const variant& src, size_t field);
    void collect(const variant& src, const variant& field);

    // split
    variant split_to_int_array(char sep);
    variant split_to_array(char sep);
    variant split_to_array(size_t cnt, char sep);
    variant split_to_table(char sep_pair, char sep_nv);

    // sort
    //void sort(const compare_fn_t& cmp);
    void sort();
    void sort(size_t i);
    void sort(const char* n);
    void sort(const string& n);
    void sort(const variant& n);
    void rsort();
    void rsort(size_t i);
    void rsort(const char* n);
    void rsort(const string& n);
    void rsort(const variant& n);

    // string
    void trim();
    variant& format(const char* fmt, ...);
    variant& append_format(const char* fmt, ...);

    // assignment
    variant& operator = (int v);
    variant& operator = (uint_t v);
    variant& operator = (float v);
    variant& operator = (double v);
    variant& operator = (int64_t v);
    variant& operator = (uint64_t v);
    variant& operator = (void* v);
    variant& operator = (const char* v);
    variant& operator = (const string& v);
    variant& operator = (const variant& v);

    // compare
    int compare(const variant& v) const;

    friend bool operator == (const variant& x, const variant& y) { return x.compare(y) == 0; }
    friend bool operator != (const variant& x, const variant& y) { return x.compare(y) != 0; }
    friend bool operator <= (const variant& x, const variant& y) { return x.compare(y) <= 0; }
    friend bool operator >= (const variant& x, const variant& y) { return x.compare(y) >= 0; }
    friend bool operator < (const variant& x, const variant& y) { return x.compare(y) < 0; }
    friend bool operator > (const variant& x, const variant& y) { return x.compare(y) > 0; }

    // math operator
    friend variant operator + (const variant& x, const variant& y);
    friend variant operator - (const variant& x, const variant& y);
    friend variant operator * (const variant& x, const variant& y);
    friend variant operator / (const variant& x, const variant& y);
    friend variant operator % (const variant& x, const variant& y);
};

struct variant::data_cell
{
    variant m_name;
    variant m_value;

    data_cell(const variant& n, const variant& v) : m_name(n), m_value(v) {}
};

template<typename T>
void destruct(T* p)
{
    (void)p;
    p->~T();
}

template<typename T>
void construct(T* p)
{
    ::new(p) T();
}

template<typename T,typename P1>
void construct(T* p,P1 p1)
{
    ::new(p) T(p1);
}

template<typename T,typename P1,typename P2>
void construct(T* p,P1 p1,P2 p2)
{
    ::new(p) T(p1,p2);
}

#endif  // __VARIANT_H_PG_BASE_VESSEL__
