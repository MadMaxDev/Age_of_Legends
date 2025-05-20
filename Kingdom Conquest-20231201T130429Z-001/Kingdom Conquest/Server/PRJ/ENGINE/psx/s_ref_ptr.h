// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-8-31

#ifndef __PS_REF_PTR_H_G_SRC_SOURCE_PG_PSX__
#define __PS_REF_PTR_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/config.h>

__PG_PSX_SPACE_BEGIN__

template<typename T>
class no_ref_psobj : public T
{
private:
    virtual int AddRef() = 0;
    virtual int Release() = 0;
public:
};

template<typename T>
class s_ref_ptr
{
public:
    typedef T   value_type;
    typedef T&  reference;
    typedef T*  pointer;
    typedef no_ref_psobj<T>* no_ref_ptr;

protected:
    pointer m_data;
public:
    ~s_ref_ptr()
    {
        if(m_data != NULL)
        {
            m_data->Release();
        }
    }
    s_ref_ptr():
        m_data(NULL)
    {
    }
    s_ref_ptr(pointer p)
    {
        m_data = p;
        if(p != NULL)
        {
            p->AddRef();
        }
    }
    s_ref_ptr(const s_ref_ptr& p)
    {
        m_data = p.m_data;
        if(p.m_data != NULL)
        {
            p.m_data->AddRef();
        }
    }
    bool empty() const
    {
        return (m_data == NULL);
    }

    operator pointer () const
    {
        return m_data;
    }
    reference operator * () const
    {
        return *m_data;
    }
    pointer* operator & ()
    {
        return &m_data;
    }
    no_ref_ptr operator -> () const
    {
        return static_cast<no_ref_ptr>(m_data);
    }
    bool operator ! () const
    {
        return (m_data == NULL);
    }
    bool operator < (pointer p) const
    {
        return m_data < p;
    }
    s_ref_ptr& operator = (pointer p)
    {
        if(p != NULL)
        {
            p->AddRef();
        }
        if(m_data != NULL)
        {
            m_data->Release();
        }
        m_data = p;
        return *this;
    }
    s_ref_ptr& operator = (const s_ref_ptr& ptr)
    {
        *this = ptr.m_data;
        return *this;
    }
    void reset()
    {
        pointer p = m_data;
        if (p)
        {
            m_data = NULL;
            p->Release();
        }
    }

    void attach(pointer p)
    {
        if(m_data == p)
        {
            return;
        }
        if(m_data != NULL)
        {
            m_data->Release();
        }
        m_data = p;
    }
    pointer detach()
    {
        pointer p = m_data;
        m_data = NULL;
        return p;
    }


}; // class s_ref_ptr


__PG_PSX_SPACE_END__

#endif  // __PS_REF_PTR_H_G_SRC_SOURCE_PG_PSX__