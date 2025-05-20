// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2007-9-28

#include "stdarg.h"
#include <vector>
#include <algorithm>
#include <functional>
#include "../inc/variant.h"

#include <WHCMN/inc/whstring.h>
using namespace n_whcmn;

namespace
{

#ifdef _WIN32
extern "C"
{
    long  __cdecl _InterlockedIncrement(int volatile *Addend);
    #pragma intrinsic (_InterlockedIncrement)

    long  __cdecl _InterlockedDecrement(int volatile *Addend);
    #pragma intrinsic (_InterlockedDecrement)
}
#else
#include <ext/atomicity.h>
using namespace __gnu_cxx;

long _InterlockedIncrement(volatile int* p)
{
    return __exchange_and_add(p, 1) + 1;
}

long _InterlockedDecrement(volatile int* p)
{
    return __exchange_and_add(p, -1) - 1;
}
#endif

string string_format(const char *fmt,...) 
{ 
	string strResult="";
	if (NULL != fmt)
	{
		va_list marker = NULL;            
		va_start(marker, fmt);										//初始化变量参数 
		size_t nLength = p_vsnprintf(NULL,0,fmt, marker) + 1;		//获取格式化字符串长度
		std::vector<char> vBuffer(nLength, '\0');					//创建用于存储格式化字符串的字符数组
		int nWritten = p_vsnprintf(&vBuffer[0], nLength, fmt, marker);
		if (nWritten>0)
		{
			strResult = &vBuffer[0];
		}
		va_end(marker);												//重置变量参数
	}
	return strResult; 
}

string string_format(const char *fmt,va_list va)
{
	string strResult="";
	if (NULL != fmt)
	{
		size_t nLength = p_vsnprintf(NULL,0,fmt, va) + 1;			//获取格式化字符串长度
		std::vector<char> vBuffer(nLength, '\0');					//创建用于存储格式化字符串的字符数组
		int nWritten = p_vsnprintf(&vBuffer[0], nLength, fmt, va);
		if (nWritten>0)
		{
			strResult = &vBuffer[0];
		}
	}
	return strResult; 
}

void string_trim(string& str)
{
   string::size_type pos = str.find_last_not_of(' ');
   if(pos != string::npos) 
   {
      str.erase(pos + 1);
      pos = str.find_first_not_of(' ');
      if(pos != string::npos) str.erase(0, pos);
   }
   else
   {
	   str.erase(str.begin(), str.end());
   }
}

typedef variant::data_info data_info;
typedef variant::data_cell data_cell;

void v_deallocate(void* p)
{
    if(p)
    {
        free(p);
    }
}

#define v_allocate(c)  malloc(c)

// data_info s_null_info = { 0, 0, 0, 0 };

data_info* d_allocate_info()
{
    data_info* p  = (data_info*)v_allocate(sizeof(data_info));
    p->m_ref      = 1;
    p->m_size     = 0;
    p->m_capacity = 0;
    p->m_vars     = 0;
    return p;
}

data_info* d_add_ref(data_info* d)
{
    _InterlockedIncrement(&d->m_ref);
    return d;
}

void d_destruct(variant* p, size_t c)
{
    variant* e = p + c;
    for(; p != e; ++ p)
    {
        destruct(p);
    }
}

void d_deallocate_array(data_info* d)
{
    d_destruct(d->m_vars, d->m_size);
    v_deallocate(d->m_vars);
    v_deallocate(d);
}

void d_release_array(data_info* d)
{
    if(0 == _InterlockedDecrement(&d->m_ref))
    {
        d_deallocate_array(d);
    }
}

void d_clear_array(data_info* d)
{
    if(d->m_size)
    {
        d_destruct(d->m_vars, d->m_size);
        d->m_size = 0;
    }
}

void d_deallocate_buffer(data_info* d)
{
    v_deallocate(d->m_data);
    v_deallocate(d);
}

void d_release_buffer(data_info* d)
{
    if(0 == _InterlockedDecrement(&d->m_ref))
    {
        d_deallocate_buffer(d);
    }
}

void d_clear_buffer(data_info* d)
{
    if(d->m_size)
    {
        d->m_size = 0;
    }
}

void d_reserve_buffer(data_info* d, size_t c)
{
    if(c <= d->m_capacity)
    {
        return;
    }
    void* t = d->m_data;
    d->m_data = v_allocate(c);
    d->m_capacity = c;
    memcpy(d->m_data, t, d->m_size);
    v_deallocate(t);
}

void d_destruct(data_cell** p, size_t c)
{
    data_cell** e = p + c;
    for(; p != e; ++ p)
    {
        destruct(*p);
        v_deallocate(*p);
    }
}

void d_deallocate_table(data_info* d)
{
    d_destruct(d->m_cell, d->m_size);
    v_deallocate(d->m_cell);
    v_deallocate(d);
}

void d_release_table(data_info* d)
{
    if(0 == _InterlockedDecrement(&d->m_ref))
    {
        d_deallocate_table(d);
    }
}

void d_clear_table(data_info* d)
{
    if(d->m_size)
    {
        d_destruct(d->m_cell, d->m_size);
        d->m_size = 0;
    }
}

void d_reserve_array(data_info* d, size_t c)
{
    if(c <= d->m_capacity)
    {
        return;
    }
    variant* t = d->m_vars;
    d->m_vars = (variant*)v_allocate(c*sizeof(variant));
    d->m_capacity = c;
    memcpy(d->m_vars, t, d->m_size*sizeof(variant));
    v_deallocate(t);
}

void d_erase_array(data_info* d, size_t i)
{
    if(i >= d->m_size)
    {
        return;
    }
    destruct(d->m_vars+i);
    memmove(d->m_vars+i, d->m_vars+i+1, (d->m_size-i-1)*sizeof(variant));
    -- d->m_size;
}

void d_reserve_table(data_info* d, size_t c)
{
    if(c <= d->m_capacity)
    {
        return;
    }
    data_cell** t = d->m_cell;
    d->m_cell = (data_cell**)v_allocate(c*sizeof(data_cell*));
    d->m_capacity = c;
    memcpy(d->m_cell, t, d->m_size*sizeof(data_cell*));
    v_deallocate(t);
}

data_cell** d_prepare_table(data_info* d, data_cell** p)
{
    if(d->m_size < d->m_capacity)
    {
        memmove(p+1, p, (d->m_cell+d->m_size-p)*sizeof(data_cell*));
    }
    else
    {
        data_cell** t = d->m_cell;
        size_t c = d->m_capacity*2;
        if(c < 8)
        {
            c = 8;
        }
        d->m_cell = (data_cell**)v_allocate(c*sizeof(data_cell*));
        d->m_capacity = c;
        size_t off = p-t;
        memcpy(d->m_cell, t, off*sizeof(data_cell*));
        memcpy(d->m_cell+off+1, p, (t+d->m_size-p)*sizeof(data_cell*));
        v_deallocate(t);
        p = d->m_cell+off;
    }
    ++ d->m_size;
    return p;
}

data_cell* d_allocate_cell(const variant& n, const variant& v)
{
    data_cell* p = (data_cell*)v_allocate(sizeof(data_cell));
    construct(p, n, v);
    return p;
}

template<class T>
int v_compare_ret(T x, T y)
{
    if(x < y)
    {
        return -1;
    }
    else if(x > y)
    {
        return 1;
    }
    return 0;
}

int v_compare_ret(size_t x, size_t y)
{
    if(x < y)
    {
        return -1;
    }
    else if(x > y)
    {
        return 1;
    }
    return 0;
}

struct v_op_add
{
    template<class T>
    void operator () (variant& v, T x, T y) const
    {
        v = x + y;
    }
};
struct v_op_sub
{
    template<class T>
    void operator () (variant& v, T x, T y) const
    {
        v = x - y;
    }
};
struct v_op_mul
{
    template<class T>
    void operator () (variant& v, T x, T y) const
    {
        v = x * y;
    }
};
struct v_op_div
{
    template<class T>
    void operator () (variant& v, T x, T y) const
    {
        v = x / y;
    }
};
struct v_op_mod
{
    template<class T>
    void operator () (variant& v, T x, T y) const
    {
        v = x % y;
    }
    void operator () (variant& v, float , float ) const
    {
        v = 0.0f;
    }
    void operator () (variant& v, double , double ) const
    {
        v = 0.0;
    }
};

template<class T>
void v_operator(variant& v, const variant& x, const variant& y, T op)
{
    if(x.m_type >= vt_string || y.m_type >= vt_string)
    {
        return;
    }

    switch(x.m_type)
    {
    case vt_int:
        switch(y.m_type)
        {
        case vt_int:
            return op(v, x.m_int, y.m_int);
        case vt_uint:
            return op(v, (uint_t)x.m_int, y.m_uint);
        case vt_int64:
            return op(v, (int64_t)x.m_int, y.m_int64);
        case vt_uint64:
            return op(v, (uint64_t)x.m_int, y.m_uint64);
        case vt_float:
            return op(v, (float)x.m_int, y.m_float);
        case vt_double:
            return op(v, (double)x.m_int, y.m_double);
        }
        break;
    case vt_uint:
        switch(y.m_type)
        {
        case vt_int:
            return op(v, x.m_uint, (uint_t)y.m_int);
        case vt_uint:
            return op(v, x.m_uint, y.m_uint);
        case vt_int64:
            return op(v, (int64_t)x.m_uint, y.m_int64);
        case vt_uint64:
            return op(v, (uint64_t)x.m_uint, y.m_uint64);
        case vt_float:
            return op(v, (float)x.m_uint, y.m_float);
        case vt_double:
            return op(v, (double)x.m_uint, y.m_double);
        }
        break;
    case vt_int64:
        switch(y.m_type)
        {
        case vt_int:
            return op(v, x.m_int64, (int64_t)y.m_int);
        case vt_uint:
            return op(v, x.m_int64, (int64_t)y.m_uint);
        case vt_int64:
            return op(v, x.m_int64, y.m_int64);
        case vt_uint64:
            return op(v, (uint64_t)x.m_int64, y.m_uint64);
        case vt_float:
            return op(v, (float)x.m_int64, y.m_float);
        case vt_double:
            return op(v, (double)x.m_int64, y.m_double);
        }
        break;
    case vt_uint64:
        switch(y.m_type)
        {
        case vt_int:
            return op(v, x.m_uint64, (uint64_t)y.m_int);
        case vt_uint:
            return op(v, x.m_uint64, (uint64_t)y.m_uint);
        case vt_int64:
            return op(v, x.m_uint64, (uint64_t)y.m_int64);
        case vt_uint64:
            return op(v, x.m_uint64, y.m_uint64);
        case vt_float:
            return op(v, (float)x.m_uint64, y.m_float);
        case vt_double:
            return op(v, (double)x.m_uint64, y.m_double);
        }
        break;
    case vt_float:
        switch(y.m_type)
        {
        case vt_int:
            return op(v, x.m_float, (float)y.m_int);
        case vt_uint:
            return op(v, x.m_float, (float)y.m_uint);
        case vt_int64:
            return op(v, x.m_float, (float)y.m_int64);
        case vt_uint64:
            return op(v, x.m_float, (float)y.m_uint64);
        case vt_float:
            return op(v, x.m_float, y.m_float);
        case vt_double:
            return op(v, (double)x.m_float, y.m_double);
        }
        break;
    case vt_double:
        switch(y.m_type)
        {
        case vt_int:
            return op(v, x.m_double, (double)y.m_int);
        case vt_uint:
            return op(v, x.m_double, (double)y.m_uint);
        case vt_int64:
            return op(v, x.m_double, (double)y.m_int64);
        case vt_uint64:
            return op(v, x.m_double, (double)y.m_uint64);
        case vt_float:
            return op(v, x.m_double, (double)y.m_float);
        case vt_double:
            return op(v, x.m_double, y.m_double);
        }
        break;
    }

    if(x.m_type == vt_none)
    {
        v = y;
    }
    else
    {
        v = x;
    }
}

int v_compare(const variant& x, const variant& y)
{
    switch(x.m_type)
    {
    case vt_int:
        switch(y.m_type)
        {
        case vt_int:
            return v_compare_ret(x.m_int, y.m_int);
        case vt_uint:
            return v_compare_ret((uint_t)x.m_int, y.m_uint);
        case vt_int64:
            return v_compare_ret((int64_t)x.m_int, y.m_int64);
        case vt_uint64:
            return v_compare_ret((uint64_t)x.m_int, y.m_uint64);
        case vt_pointer:
            return v_compare_ret((void*)(ptrdiff_t)x.m_int, y.m_pointer);
        case vt_float:
            return v_compare_ret((float)x.m_int, y.m_float);
        case vt_double:
            return v_compare_ret((double)x.m_int, y.m_double);
        }
        break;
    case vt_uint:
        switch(y.m_type)
        {
        case vt_int:
            return v_compare_ret(x.m_uint, (uint_t)y.m_int);
        case vt_uint:
            return v_compare_ret(x.m_uint, y.m_uint);
        case vt_int64:
            return v_compare_ret((int64_t)x.m_uint, y.m_int64);
        case vt_uint64:
            return v_compare_ret((uint64_t)x.m_uint, y.m_uint64);
        case vt_pointer:
            return v_compare_ret((void*)(ptrdiff_t)x.m_uint, y.m_pointer);
        case vt_float:
            return v_compare_ret((float)x.m_uint, y.m_float);
        case vt_double:
            return v_compare_ret((double)x.m_uint, y.m_double);
        }
        break;
    case vt_int64:
        switch(y.m_type)
        {
        case vt_int:
            return v_compare_ret(x.m_int64, (int64_t)y.m_int);
        case vt_uint:
            return v_compare_ret(x.m_int64, (int64_t)y.m_uint);
        case vt_int64:
            return v_compare_ret(x.m_int64, y.m_int64);
        case vt_uint64:
            return v_compare_ret((uint64_t)x.m_int64, y.m_uint64);
        case vt_pointer:
            return v_compare_ret(x.m_int64, (int64_t)y.m_pointer);
        case vt_float:
            return v_compare_ret((float)x.m_int64, y.m_float);
        case vt_double:
            return v_compare_ret((double)x.m_int64, y.m_double);
        }
        break;
    case vt_uint64:
        switch(y.m_type)
        {
        case vt_int:
            return v_compare_ret(x.m_uint64, (uint64_t)y.m_int);
        case vt_uint:
            return v_compare_ret(x.m_uint64, (uint64_t)y.m_uint);
        case vt_int64:
            return v_compare_ret(x.m_uint64, (uint64_t)y.m_int64);
        case vt_uint64:
            return v_compare_ret(x.m_uint64, y.m_uint64);
        case vt_pointer:
            return v_compare_ret(x.m_uint64, (uint64_t)y.m_pointer);
        case vt_float:
            return v_compare_ret((float)x.m_uint64, y.m_float);
        case vt_double:
            return v_compare_ret((double)x.m_uint64, y.m_double);
        }
        break;
    case vt_pointer:
        switch(y.m_type)
        {
        case vt_int:
            return v_compare_ret(x.m_pointer, (void*)(ptrdiff_t)y.m_int);
        case vt_uint:
            return v_compare_ret((size_t)x.m_pointer, (size_t)y.m_uint);
        case vt_int64:
            return v_compare_ret((int64_t)x.m_pointer, y.m_int64);
        case vt_uint64:
            return v_compare_ret((uint64_t)x.m_pointer, y.m_uint64);
        case vt_pointer:
            return v_compare_ret(x.m_pointer, y.m_pointer);
        }
        break;
    case vt_float:
        switch(y.m_type)
        {
        case vt_int:
            return v_compare_ret(x.m_float, (float)y.m_int);
        case vt_uint:
            return v_compare_ret(x.m_float, (float)y.m_uint);
        case vt_int64:
            return v_compare_ret(x.m_float, (float)y.m_int64);
        case vt_uint64:
            return v_compare_ret(x.m_float, (float)y.m_uint64);
        case vt_float:
            return v_compare_ret(x.m_float, y.m_float);
        case vt_double:
            return v_compare_ret((double)x.m_float, y.m_double);
        }
        break;
    case vt_double:
        switch(y.m_type)
        {
        case vt_int:
            return v_compare_ret(x.m_double, (double)y.m_int);
        case vt_uint:
            return v_compare_ret(x.m_double, (double)y.m_uint);
        case vt_int64:
            return v_compare_ret(x.m_double, (double)y.m_int64);
        case vt_uint64:
            return v_compare_ret(x.m_double, (double)y.m_uint64);
        case vt_float:
            return v_compare_ret(x.m_double, (double)y.m_float);
        case vt_double:
            return v_compare_ret(x.m_double, y.m_double);
        }
        break;
    case vt_string:
        if(y.m_type == vt_string)
        {
            return strcmp(x.m_string->c_str(), y.m_string->c_str());
        }
        break;
    case vt_buffer:
        if(y.m_type == vt_buffer)
        {
            // 不做实现了，两个缓冲比较貌似没有实用价值
        }
        break;
    case vt_array:
        if(y.m_type == vt_array)
        {
            data_info* xd = static_cast<data_info*>(x.m_info);
            data_info* yd = static_cast<data_info*>(y.m_info);
            size_t c = xd->m_size < yd->m_size ? xd->m_size : yd->m_size;
            variant* xp = xd->m_vars;
            variant* yp = yd->m_vars;
            for(size_t i = 0; i < c; ++ i)
            {
                int t = v_compare(xp[i], yp[i]);
                if(t != 0)
                {
                    return t;
                }
            }
            return v_compare_ret(xd->m_size, yd->m_size);
        }
        break;
    case vt_table:
        if(y.m_type == vt_table)
        {
            data_info* xd = static_cast<data_info*>(x.m_info);
            data_info* yd = static_cast<data_info*>(y.m_info);
            size_t c = xd->m_size < yd->m_size ? xd->m_size : yd->m_size;
            data_cell** xp = xd->m_cell;
            data_cell** yp = yd->m_cell;
            for(size_t i = 0; i < c; ++ i)
            {
                data_cell* xc = xp[i];
                data_cell* yc = yp[i];
                int t = v_compare(xc->m_name, yc->m_name);
                if(t != 0)
                {
                    return t;
                }
                t = v_compare(xc->m_value, yc->m_value);
                if(t != 0)
                {
                    return t;
                }
            }
            return v_compare_ret(xd->m_size, yd->m_size);
        }
        break;
    }

    // vt_none is less than all other types
    return v_compare_ret(x.m_type, y.m_type);
}

bool d_search(data_info* d, data_cell**& r, const variant& v)
{
    data_cell** p = d->m_cell;
    size_t c = d->m_size;
	while(c > 0)
	{
        size_t half = c/2;
        data_cell** mid = p+half;

        int i = v_compare((*mid)->m_name, v);
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
            r = mid;
            return true;
        }
	}
    r = p;
    return false;
}

void d_set_table(data_info* d, const variant& n, const variant& v)
{
    data_cell** p = 0;
    if(d_search(d, p, n))
    {
        (*p)->m_value = v;
    }
    else
    {
        p = d_prepare_table(d, p);
        *p = d_allocate_cell(n, v);
    }
}

void d_erase_table(data_info* d, const variant& n)
{
    data_cell** p = 0;
    if(!d_search(d, p, n))
    {
        return;
    }
    data_cell* t = *p;
    destruct(t);
    v_deallocate(t);

    memmove(p, p+1, (d->m_cell+d->m_size-p-1)*sizeof(data_cell*));
    -- d->m_size;
}

// sort

struct data_info_cmp_i_lt
{
    variant m_name;
    data_info_cmp_i_lt(const variant& n) : m_name(n) {}
    bool operator () (const variant& x, const variant& y) const
    {
        return x.get(m_name).compare(y.get(m_name)) < 0;
    }
};

struct data_info_cmp_i_gt
{
    variant m_name;
    data_info_cmp_i_gt(const variant& n) : m_name(n) {}
    bool operator () (const variant& x, const variant& y) const
    {
        return x.get(m_name).compare(y.get(m_name)) > 0;
    }
};

void variant_common_sort(variant* v, const variant& n)
{
    if(v->m_type != vt_array)
    {
        return;
    }
    if(v->m_info->m_size == 0)
    {
        return;
    }
    data_info_cmp_i_lt cmp(n);
    std::sort(v->m_info->m_vars, v->m_info->m_vars+v->m_info->m_size, cmp);
}

void variant_common_rsort(variant* v, const variant& n)
{
    if(v->m_type != vt_array)
    {
        return;
    }
    if(v->m_info->m_size == 0)
    {
        return;
    }
    data_info_cmp_i_gt cmp(n);
    std::sort(v->m_info->m_vars, v->m_info->m_vars+v->m_info->m_size, cmp);
}

} // namespace

variant::variant()
{
    m_type = vt_none;
    m_data = 0;
}

variant::variant(int v)
{
    m_data = 0;
    m_type  = vt_int;
    *(int*)&m_data = v;
}

variant::variant(uint_t v)
{
    m_data = 0;
    m_type  = vt_uint;
    *(uint_t*)&m_data = v;
}

variant::variant(float v)
{
    m_data = 0;
    m_type  = vt_float;
    *(float*)&m_data = v;
}

variant::variant(double v)
{
    m_type  = vt_double;
    *(double*)&m_data = v;
}

variant::variant(int64_t v)
{
    m_type  = vt_int64;
    *(int64_t*)&m_data = v;
}

variant::variant(uint64_t v)
{
    m_type  = vt_uint64;
    *(uint64_t*)&m_data = v;
}

variant::variant(void* v)
{
    m_data = 0;
    m_type  = vt_pointer;
    *(void**)&m_data = v;
}

variant::variant(const char* v)
{
    m_data = 0;
    m_type = vt_string;
	m_string = new string(v);
}

variant::variant(const string& v)
{
    m_data = 0;
    m_type = vt_string;
    m_string = new string(v);
}

variant::variant(const variant& v)
{
    if(v.m_type < vt_string)
    {
        m_type = v.m_type;
        m_data = v.m_data;
        return;
    }
    
    m_data = 0;
    switch(v.m_type)
    {
    case vt_string:
        m_string = new string(*v.m_string);
        break;
    case vt_buffer:
    case vt_table:
    case vt_array:
        m_info = d_add_ref(v.m_info);
        break;
    default:
        // invalid type
        m_type = vt_none;
        return;
    }
    m_type  = v.m_type;
}

variant::~variant()
{
    if(m_type < vt_string)
    {
        return;
    }

    switch(m_type)
    {
    case vt_string:
		if(m_string)
		{
			delete m_string;
			m_string = 0;
		}
        break;
    case vt_buffer:
        d_release_buffer(m_info);
        break;
    case vt_array:
        d_release_array(m_info);
        break;
    case vt_table:
        d_release_table(m_info);
        break;
    }
}

uint_t variant::type() const
{
    return m_type;
}

void variant::type(uint_t t)
{
    if(m_type == t)
    {
        return;
    }
    if(uint_t(t) > vt_table)
    {
        // invalid type;
        return;
    }

    destruct(this);

    m_type  = t;
    m_data  = 0;

    switch(t)
    {
    case vt_string:
        m_string = new string;
        break;
    case vt_buffer:
    case vt_table:
    case vt_array:
        m_info = d_allocate_info();
        break;
    }
}

int variant::to_int() const
{
    switch(m_type)
    {
    case vt_int:
    case vt_uint:
    case vt_int64:
    case vt_uint64:
    case vt_pointer:
        return m_int;
    case vt_float:
        return (int)m_float;
    case vt_double:
        return (int)m_double;
    case vt_string:
        {
            int r = 0;
            sscanf(m_string->c_str(), "%d", &r);
            return r;
        }
    }
    return 0;
}

uint_t variant::to_uint() const
{
    switch(m_type)
    {
    case vt_int:
    case vt_uint:
    case vt_int64:
    case vt_uint64:
    case vt_pointer:
        return m_uint;
    case vt_float:
        return (uint_t)m_float;
    case vt_double:
        return (uint_t)m_double;
    case vt_string:
        {
            uint_t r = 0;
            sscanf(m_string->c_str(), "%u", &r);
            return r;
        }
    }
    return 0;
}

float variant::to_float() const
{
    switch(m_type)
    {
    case vt_int:
        return (float)m_int;
    case vt_uint:
        return (float)m_uint;
    case vt_int64:
        return (float)m_int64;
    case vt_uint64:
        return (float)m_uint64;
    case vt_float:
        return (float)m_float;
    case vt_double:
        return (float)m_double;
    case vt_string:
        {
            float r = 0;
            sscanf(m_string->c_str(), "%f", &r);
            return r;
        }
    }
    return 0;
}

double variant::to_double() const
{
    switch(m_type)
    {
    case vt_int:
        return (double)m_int;
    case vt_uint:
        return (double)m_uint;
    case vt_int64:
        return (double)m_int64;
    case vt_uint64:
        return (double)m_uint64;
    case vt_float:
        return (double)m_float;
    case vt_double:
        return (double)m_double;
    case vt_string:
        {
            float r = 0;
            sscanf(m_string->c_str(), "%f", &r);
            return r;
        }
    }
    return 0;
}

int64_t variant::to_int64() const
{
    switch(m_type)
    {
    case vt_int:
        return m_int;
    case vt_uint:
        return m_uint;
    case vt_int64:
    case vt_uint64:
        return m_int64;
    case vt_pointer:
        return int64_t(m_pointer);
    case vt_float:
        return (int64_t)m_float;
    case vt_double:
        return (int64_t)m_double;
    case vt_string:
        {
            int64_t r = 0;
            sscanf(m_string->c_str(), "%"WHINT64PRFX"d", &r);
            return r;
        }
    }
    return 0;
}

uint64_t variant::to_uint64() const
{
    switch(m_type)
    {
    case vt_int:
        return m_int;
    case vt_uint:
        return m_uint;
    case vt_int64:
    case vt_uint64:
        return m_uint64;
    case vt_pointer:
        return uint64_t(m_pointer);
    case vt_float:
        return (uint64_t)m_float;
    case vt_double:
        return (uint64_t)m_double;
    case vt_string:
        {
            uint64_t r = 0;
            sscanf(m_string->c_str(), "%"WHINT64PRFX"u", &r);
            return r;
        }
    }
    return 0;
}

void* variant::to_pointer() const
{
    if(m_type == vt_pointer)
    {
        return m_pointer;
    }
    return 0;
}

string variant::to_string() const
{
    string str;
    switch(m_type)
    {
    case vt_int:
        str = string_format("%d", m_int);
        break;
    case vt_uint:
        str = string_format("%u", m_uint);
        break;
    case vt_int64:
        str = string_format("%"WHINT64PRFX"d", m_int64);
        break;
    case vt_uint64:
        str = string_format("%"WHINT64PRFX"u", m_uint64);
        break;
    case vt_pointer:
        str = string_format("%IX", m_pointer);
        break;
    case vt_float:
        str = string_format("%g", m_float);
        break;
    case vt_double:
        str = string_format("%g", m_double);
        break;
    case vt_string:
        return *m_string;
    case vt_array:
        {
            data_info* inf = static_cast<data_info*>(m_info);
            variant* p = inf->m_vars;
            variant* e = p + inf->m_size;
            if(p == e)
            {
                break;
            }
            str.append("{");
            str.append(p->to_string());
            ++ p;
            for(; p < e; ++ p)
            {
                str.append(",");
                str.append(p->to_string());
            }
            str.append("}");
        }
        break;
    case vt_table:
        {
            data_cell** p = m_info->m_cell;
            data_cell** e = p + m_info->m_size;
            if(p == e)
            {
                break;
            }
            for(; p < e; ++ p)
            {
                str.append("[");
                str.append((*p)->m_name.to_string());
                str.append("|");
                str.append((*p)->m_value.to_string());
                str.append("]");
            }
        }
        break;
    }
    return str;
}

size_t variant::size() const
{
    size_t c = 0;
    switch(m_type)
    {
    case vt_string:
        c = m_string->size();
        break;
    case vt_buffer:
    case vt_array:
    case vt_table:
        c = m_info->m_size;
        break;
    }
    return c;
}

bool variant::empty() const
{
    size_t c = size();
    return c == 0;
}

bool variant::valid() const
{
    return m_type != vt_none;
}

void variant::clear()
{
    if(m_type < vt_string)
    {
        m_data = 0;
        return;
    }

    switch(m_type)
    {
    case vt_string:
        m_string->clear();
        break;
    case vt_buffer:
        d_clear_buffer(m_info);
        break;
    case vt_array:
        d_clear_array(m_info);
        break;
    case vt_table:
        d_clear_table(m_info);
        break;
    }
}

void variant::reserve(size_t c)
{
    switch(m_type)
    {
    case vt_none:
        type(vt_array);
        // do not break here.
    case vt_array:
        d_reserve_array(m_info, c);
        break;
    case vt_table:
        d_reserve_table(m_info, c);
        break;
    case vt_buffer:
        d_reserve_buffer(m_info, c);
        break;
    }
}

bool variant::fetch(size_t i, variant& v) const
{
    switch(m_type)
    {
    case vt_array:
        if(i < m_info->m_size)
        {
            v = m_info->m_vars[i];
            return true;
        }
        break;
    case vt_table:
        if(i < m_info->m_size)
        {
            data_cell* p = m_info->m_cell[i];
            v = p->m_value;
            return true;
        }
        break;
    }
    return false;
}

variant variant::fetch(size_t i) const
{
    switch(m_type)
    {
    case vt_array:
        if(i < m_info->m_size)
        {
            return m_info->m_vars[i];
        }
        break;
    case vt_table:
        if(i < m_info->m_size)
        {
            data_cell* p = m_info->m_cell[i];
            return p->m_value;
        }
        break;
    }
    return variant();
}

bool variant::fetch(size_t i, variant& n, variant& v) const
{
    switch(m_type)
    {
    case vt_array:
        if(i < m_info->m_size)
        {
            v = m_info->m_vars[i];
            return true;
        }
        break;
    case vt_table:
        if(i < m_info->m_size)
        {
            data_cell* p = m_info->m_cell[i];
            n = p->m_name;
            v = p->m_value;
            return true;
        }
        break;
    }
    return false;
}

void variant::bset(void* p,size_t c)
{
    type(vt_buffer);
    reserve(c);
    ::memcpy(m_info->m_data, p, c);
    m_info->m_size = c;
}

void* variant::bget()
{
    if(m_type != vt_buffer) return 0;
    return m_info->m_data;
}

void variant::bset(const variant& k, void* p, size_t c)
{
    variant v;
    v.bset(p, c);
    set(k, v);
}

void* variant::bget(const variant& k)
{
    return get(k).bget();
}

variant variant::copy() const
{
    switch(m_type)
    {
    case vt_buffer:
        {
            variant v;
            v.type(vt_buffer);
            v.reserve(m_info->m_size);
            ::memcpy(v.m_info->m_data, m_info->m_data, m_info->m_size);
            v.m_info->m_size = m_info->m_size;
            return v;
        }
        break;
    case vt_array:
        {
            variant v;
            v.type(vt_array);
            v.reserve(m_info->m_size);
            variant* p = m_info->m_vars;
            variant* e = p + m_info->m_size;
            variant* d = v.m_info->m_vars;
            for(; p < e; ++ p, ++ d)
            {
                construct<variant, const variant>(d, p->copy());
            }
            v.m_info->m_size = m_info->m_size;
            return v;
        }
        break;
    case vt_table:
        {
            variant v;
            v.type(vt_table);
            v.reserve(m_info->m_size);
            data_cell** p = m_info->m_cell;
            data_cell** e = p + m_info->m_size;
            data_cell** d = v.m_info->m_cell;
            for(; p < e; ++ p, ++ d)
            {
                *d = d_allocate_cell((*p)->m_name.copy(), (*p)->m_value.copy());
            }
            v.m_info->m_size = m_info->m_size;
            return v;
        }
        break;
    default:
        return *this;
    }
}

void variant::push_front(const variant& v)
{
    if(m_type != vt_array)
    {
        if(m_type == vt_none)
        {
            type(vt_array);
        }
        else
        {
            return;
        }
    }
    if(m_info->m_size < m_info->m_capacity)
    {
        memmove(m_info->m_vars+1, m_info->m_vars, m_info->m_size*sizeof(variant));
    }
    else
    {
        size_t c = m_info->m_capacity*2;
        if(c < 8)
        {
            c = 8;
        }
        variant* t = m_info->m_vars;
        m_info->m_vars = (variant*)v_allocate(c*sizeof(variant));
        m_info->m_capacity = c;
        memcpy(m_info->m_vars+1, t, m_info->m_size*sizeof(variant));
        v_deallocate(t);
    }
    construct(m_info->m_vars, v);
    ++ m_info->m_size;
}

void variant::push_back(const variant& v)
{
    if(m_type != vt_array)
    {
        if(m_type == vt_none)
        {
            type(vt_array);
        }
        else
        {
            return;
        }
    }
    if(m_info->m_size >= m_info->m_capacity)
    {
        size_t c = m_info->m_capacity*2;
        if(c < 8)
        {
            c = 8;
        }
        variant* t = m_info->m_vars;
        m_info->m_vars = (variant*)v_allocate(c*sizeof(variant));
        m_info->m_capacity = c;
        memcpy(m_info->m_vars, t, m_info->m_size*sizeof(variant));
        v_deallocate(t);
    }
    construct(m_info->m_vars+m_info->m_size, v);
    ++ m_info->m_size;
}

void variant::insert(size_t i, const variant& v)
{
    if(m_type != vt_array)
    {
        if(m_type == vt_none)
        {
            type(vt_array);
        }
        else
        {
            return;
        }
    }
    if(i > m_info->m_size)
    {
        return;
    }
    if(m_info->m_size < m_info->m_capacity)
    {
        memmove(m_info->m_vars+i+1, m_info->m_vars+i, (m_info->m_size-i)*sizeof(variant));
    }
    else
    {
        size_t c = m_info->m_capacity*2;
        if(c < 8)
        {
            c = 8;
        }
        variant* t = m_info->m_vars;
        m_info->m_vars = (variant*)v_allocate(c*sizeof(variant));
        m_info->m_capacity = c;
        memcpy(m_info->m_vars, t, i*sizeof(variant));
        memcpy(m_info->m_vars+i+1, t+i, (m_info->m_size-i)*sizeof(variant));
        v_deallocate(t);
    }
    construct(m_info->m_vars+i, v);
    ++ m_info->m_size;
}

void variant::erase(const variant& n)
{
    switch(m_type)
    {
    case vt_array:
        {
            size_t i = 0;
            switch(n.m_type)
            {
            case vt_int:
            case vt_uint:
                i = n.m_uint;
                break;
            case vt_int64:
            case vt_uint64:
                i = (size_t)n.m_uint64;
                break;
            default:
                return;
            }
            d_erase_array(m_info, i);
        }
        break;
    case vt_table:
        d_erase_table(m_info, n);
        break;
    }
}

variant variant::get(const variant& n) const
{
    switch(m_type)
    {
    case vt_string:
    case vt_array:
        {
            size_t i = 0;
            switch(n.m_type)
            {
            case vt_int:
            case vt_uint:
                i = n.m_uint;
                break;
            case vt_int64:
            case vt_uint64:
                i = (size_t)n.m_uint64;
                break;
            default:
                return variant();
            }
            if(m_type == vt_string)
            {
                if(i < m_string->size())
                {
                    return variant((*m_string)[i]);
                }
                break;
            }
            if(i < m_info->m_size)
            {
                return m_info->m_vars[i];
            }
        }
        break;
    case vt_table:
        {
            data_cell** p = 0;
            if(d_search(m_info, p, n))
            {
                return (*p)->m_value;
            }
        }
        break;
    }
    return variant();
}

bool variant::has(const variant& n) const
{
    switch(m_type)
    {
    case vt_array:
        {
            variant* p = m_info->m_vars;
            variant* e = p + m_info->m_size;
            for(; p < e; ++ p)
            {
                if(*p == n)
                {
                    return true;
                }
            }
        }
        break;
    case vt_table:
        {
            data_cell** p = 0;
            if(d_search(m_info, p, n))
            {
                return true;
            }
        }
        break;
    }
    return false;
}

variant variant::find(const variant& field, const variant& value)
{
    switch(m_type)
    {
    case vt_array:
        {
            variant* p = m_info->m_vars;
            variant* e = p + m_info->m_size;
            for(; p < e; ++ p)
            {
                if(p->get(field) == value)
                {
                    return *p;
                }
            }
        }
        break;
    case vt_table:
        {
            data_cell** p = m_info->m_cell;
            data_cell** e = p + m_info->m_size;
            for(; p < e; ++ p)
            {
                variant& v = (*p)->m_value;
                if(v.get(field) == value)
                {
                    return v;
                }
            }
        }
        break;
    }
    return variant();
}

size_t variant::lower_bound(const variant& n) const
{
    switch(m_type)
    {
    case vt_array:
        {
            variant* p = m_info->m_vars;
            variant* e = p + m_info->m_size;
            for(; p < e && *p < n; ++ p);
            return p-m_info->m_vars;
        }
        break;
    case vt_table:
        {
            data_cell** p = 0;
            if(d_search(m_info, p, n))
            {
                return p-m_info->m_cell;
            }
        }
        break;
    }
    return size_t(-1);
}

size_t variant::upper_bound(const variant& n) const
{
    switch(m_type)
    {
    case vt_array:
        {
            variant* p = m_info->m_vars;
            variant* e = p + m_info->m_size;
            for(; p < e && *p <= n; ++ p);
            return p-m_info->m_vars;
        }
        break;
    case vt_table:
        {
            data_cell** p = 0;
            if(d_search(m_info, p, n))
            {
                return p-m_info->m_cell;
            }
        }
        break;
    }
    return size_t(-1);
}

size_t variant::index(const variant& n) const
{
    switch(m_type)
    {
    case vt_array:
        {
            variant* p = m_info->m_vars;
            variant* e = p + m_info->m_size;
            for(; p < e; ++ p)
            {
                if(*p == n)
                {
                    return size_t(p-m_info->m_vars);
                }
            }
        }
        break;
    case vt_table:
        {
            data_cell** p = 0;
            if(d_search(m_info, p, n))
            {
                return p-m_info->m_cell;
            }
        }
        break;
    }
    return size_t(-1);
}

variant& variant::ref(const variant& n)
{
    if(m_type == vt_array)
    {
        size_t i = 0;
        switch(n.m_type)
        {
        case vt_int:
        case vt_uint:
            i = n.m_uint;
            break;
        case vt_int64:
        case vt_uint64:
            i = (size_t)n.m_uint64;
            break;
        default:
            throw "variant ref invalid index type.";
        }
        if(i >= m_info->m_size)
        {
            throw "variant ref overflow.";
        }
        return m_info->m_vars[i];
    }
    if(m_type != vt_table)
    {
        type(vt_table);
    }
    data_cell** p = 0;
    if(!d_search(m_info, p, n))
    {
        p = d_prepare_table(m_info, p);
        variant v;
        v.type(vt_table);
        *p = d_allocate_cell(n, v);
    }
    return (*p)->m_value;
}

void variant::set(const variant& n, const variant& v)
{
    switch(m_type)
    {
    case vt_none:
        type(vt_table);
        // do not break here.
    case vt_table:
        d_set_table(m_info, n, v);
        break;
    case vt_array:
        {
            size_t i = 0;
            switch(n.m_type)
            {
            case vt_int:
            case vt_uint:
                i = n.m_uint;
                break;
            case vt_int64:
            case vt_uint64:
                i = (size_t)n.m_uint64;
                break;
            default:
                return;
            }
            if(i < m_info->m_size)
            {
                m_info->m_vars[i] = v;
            }
        }
        break;
    }
}

void variant::collect(const variant& src, size_t field)
{
    collect(src, variant(field));
}

void variant::collect(const variant& src, const variant& field)
{
    type(vt_array);
    size_t c = src.size();
    reserve(size()+c);
    for(size_t i = 0; i < c; ++ i)
    {
        variant v;
        src.fetch(i, v);
        if(!v.valid())
        {
            continue;
        }
        variant f = v.get(field);
        if(!f.valid())
        {
            continue;
        }
        push_back(f);
    }
}

//void variant::sort(const function<bool(const variant&, const variant&), memory::pool_allocator<byte_t> >& cmp)
//{
//    if(m_type != vt_array)
//    {
//        return;
//    }
//    if(m_info->m_size == 0)
//    {
//        return;
//    }
//    std::sort(m_info->m_vars, m_info->m_vars+m_info->m_size, cmp);
//}

void variant::sort()
{
    if(m_type != vt_array)
    {
        return;
    }
    if(m_info->m_size == 0)
    {
        return;
    }
    std::sort(m_info->m_vars, m_info->m_vars+m_info->m_size);
}

void variant::sort(size_t i)
{
    variant_common_sort(this, i);
}

void variant::sort(const char* n)
{
    variant_common_sort(this, n);
}

void variant::sort(const string& n)
{
    variant_common_sort(this, n);
}

void variant::sort(const variant& n)
{
    variant_common_sort(this, n);
}

void variant::rsort()
{
    if(m_type != vt_array)
    {
        return;
    }
    if(m_info->m_size == 0)
    {
        return;
    }
    std::sort(m_info->m_vars, m_info->m_vars+m_info->m_size, std::greater<variant>());
}

void variant::rsort(size_t i)
{
    variant_common_rsort(this, i);
}

void variant::rsort(const char* n)
{
    variant_common_rsort(this, n);
}

void variant::rsort(const string& n)
{
    variant_common_rsort(this, n);
}

void variant::rsort(const variant& n)
{
    variant_common_rsort(this, n);
}

variant variant::split_to_array(size_t cnt, char sep)
{
    if(m_type != vt_string)
    {
        return variant();
    }
    const string& str = *m_string;
    variant r;
    r.type(vt_array);
    if(cnt == 0)
    {
        return r;
    }

    const char* p = str.c_str();
    const char* s = p;
    for(; *p; ++ p)
    {
		if(whstr_Check_MultiChar(p))
        {
			// 可能是双字节文字（中文）
            ++p;
			continue;
        }
        if(*p == sep)
        {
            r.push_back(string(s, p));
            if(r.size() == cnt)
            {
                return r;
            }
            s = p+1;
        }
    }
    r.push_back(string(s, p));

    return r;
}

variant variant::split_to_array(char sep)
{
    if(m_type != vt_string)
    {
        return variant();
    }
    const string& str = *m_string;
    variant r;
    r.type(vt_array);

    const char* p = str.c_str();
    const char* s = p;
    for(; *p; ++ p)
    {
		if(whstr_Check_MultiChar(p))
        {
			// 可能是双字节文字（中文）
            ++p;
			continue;
        }
        if(*p == sep)
        {
            r.push_back(string(s, p));
            s = p+1;
        }
    }
    r.push_back(string(s, p));

    return r;
}

variant variant::split_to_int_array(char sep)
{
    if(m_type != vt_string)
    {
        return variant();
    }
    const string& str = *m_string;
    variant r;
    r.type(vt_array);

    const char* p = str.c_str();
    const char* s = p;
    for(; *p; ++ p)
    {
		if(whstr_Check_MultiChar(p))
        {
			// 可能是双字节文字（中文）
            ++p;
			continue;
        }
        if(*p == sep)
        {
            r.push_back(atoi(s));
            s = p+1;
        }
    }
    r.push_back(string(s, p));

    return r;
}

variant variant::split_to_table(char sep_pair, char sep_nv)
{
    if(m_type != vt_string)
    {
        return variant();
    }
    const string& str = *m_string;
    variant r;
    r.type(vt_table);

    const char* p = str.c_str();
    const char* s = p;
    for(; *p; ++ p)
    {
        if(*p != sep_nv)
        {
            continue;
        }
        variant n(string(s, p));
        s = ++ p;
        for(;; ++ p)
        {
            if(*p == sep_pair)
            {
                variant v(string(s, p));
                r.set(n, v);
                s = p+1;
                break;
            }
            else if(*p == 0)
            {
                variant v(string(s, p));
                r.set(n, v);
                return r;
            }
        }
    }

    return r;
}

void variant::trim()
{
    switch(m_type)
    {
    case vt_string:
        {
			string_trim(*m_string);
        }
        break;
    case vt_array:
        {
            size_t c = m_info->m_size;
            for(size_t i = 0; i < c; ++ i)
            {
                m_info->m_vars[i].trim();
            }
        }
        break;
    case vt_table:
        {
            size_t c = m_info->m_size;
            for(size_t i = 0; i < c; ++ i)
            {
                data_cell* p = m_info->m_cell[i];
                p->m_name.trim();
                p->m_value.trim();
            }
        }
        break;
    }
}

variant& variant::format(const char* fmt, ...)
{
    string str;
	va_list args;
    va_start(args, fmt);
    str = string_format(fmt, args);
	va_end(args);
    *this = str;
    return *this;
}

variant& variant::append_format(const char* fmt, ...)
{
    string str = to_string();
	va_list args;
    va_start(args, fmt);
    str += string_format(fmt, args);
	va_end(args);
    *this = str;
    return *this;
}

variant& variant::operator = (int v)
{
    destruct(this);
    construct(this, v);
    return *this;
}

variant& variant::operator = (uint_t v)
{
    destruct(this);
    construct(this, v);
    return *this;
}

variant& variant::operator = (float v)
{
    destruct(this);
    construct(this, v);
    return *this;
}

variant& variant::operator = (double v)
{
    destruct(this);
    construct(this, v);
    return *this;
}

variant& variant::operator = (int64_t v)
{
    destruct(this);
    construct(this, v);
    return *this;
}

variant& variant::operator = (uint64_t v)
{
    destruct(this);
    construct(this, v);
    return *this;
}

variant& variant::operator = (void* v)
{
    destruct(this);
    construct(this, v);
    return *this;
}

variant& variant::operator = (const char* v)
{
    destruct(this);
    construct(this, v);
    return *this;
}

variant& variant::operator = (const string& v)
{
    variant t(v);
    destruct(this);
    construct(this, t);
    return *this;
}

variant& variant::operator = (const variant& v)
{
    if(this == &v)
    {
        return *this;
    }
    variant t(v);
    destruct(this);
    construct(this, t);
    return *this;
}

int variant::compare(const variant& v) const
{
    int i = v_compare(*this, v);
    return i;
}

variant operator + (const variant& x, const variant& y)
{
    variant v;
    v_operator(v, x, y, v_op_add());
    return v;
}

variant operator - (const variant& x, const variant& y)
{
    variant v;
    v_operator(v, x, y, v_op_sub());
    return v;
}

variant operator * (const variant& x, const variant& y)
{
    variant v;
    v_operator(v, x, y, v_op_mul());
    return v;
}

variant operator / (const variant& x, const variant& y)
{
    variant v;
    v_operator(v, x, y, v_op_div());
    return v;
}

variant operator % (const variant& x, const variant& y)
{
    variant v;
    v_operator(v, x, y, v_op_mod());
    return v;
}
