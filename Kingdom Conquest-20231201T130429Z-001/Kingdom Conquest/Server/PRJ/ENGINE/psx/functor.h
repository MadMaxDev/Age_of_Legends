// Copyright (C) Pixel Studio
// All rights reserved.
// 
// Author : Jiang Li
// Date   : 2005-9-1

#ifndef __FUNCTOR_H_G_SRC_SOURCE_PG_PSX__
#define __FUNCTOR_H_G_SRC_SOURCE_PG_PSX__

#pragma once

#include <psx/config.h>
#include <psx/s_engine.h>

__PG_PSX_SPACE_BEGIN__

class functor_policy;

template<typename T, typename Policy = functor_policy> class functor;

class pg_psx_public functor_base
{
protected:
    s_engine* m_engine;
    int m_module;
    int m_function;
    ref_ptr<ref_obj>    m_refRes;

    functor_base()
    {
        m_engine = NULL;
        m_module = psNO_MODULE;
        m_function = psNO_FUNCTION;
    }

    bool empty() const
    {
        return (m_module == psNO_MODULE) || (m_function == psNO_FUNCTION);
    }
    void clear()
    {
        m_engine = NULL;
        m_module = psNO_MODULE;
        m_function = psNO_FUNCTION;
        m_refRes.reset();
    }


    bool load_d(s_engine& ng, const wchar_t* mod, const wchar_t* decl);
    bool load_d(s_engine& ng, psIModule* mod, const wchar_t* decl);

    void prepare(psIScriptContext*) const;
    void execute(psIScriptContext*) const;
public:
    void ref_res(ref_obj* res)
    {
        m_refRes = res;
    }
    void load_id(s_engine& ng, int modId, int fnId)
    {
        m_engine = &ng;
        m_module = modId;
        m_function = fnId;
        m_refRes.reset();
    }
};

class pg_psx_public functor_policy
{
public:
    typedef decl_text::builder builder_t;

    void throw_error(int r);
    void check(int r)
    {
        if(r < 0)
        {
            throw_error(r);
        }
    }
public:
    psIScriptContext*   m_context;
    s_engine*   m_engine;

    functor_policy(s_engine& ng)
    {
        m_engine = &ng;
        m_context = ng.get_ctx();
    }
    ~functor_policy()
    {
        if(m_context)
        {
            m_engine->put_ctx(m_context);
        }
    }

    template<typename T>
    void set_arg(uint_t idx, T* t)
    {
        check(m_context->SetArgPointer(idx, t));
    }
    template<typename T>
    void set_arg(uint_t idx, const T* t)
    {
        check(m_context->SetArgPointer(idx, (void*)t));
    }
    template<typename T>
    void set_arg(uint_t idx, T& t)
    {
        check(m_context->SetArgPointer(idx, (void*)&t));
    }

    void set_arg(uint_t idx, bool a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
    void set_arg(uint_t idx, char a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
    void set_arg(uint_t idx, byte_t a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
    void set_arg(uint_t idx, short a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
    void set_arg(uint_t idx, ushort_t a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
    void set_arg(uint_t idx, int a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
    void set_arg(uint_t idx, uint_t a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
    void set_arg(uint_t idx, long a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
    void set_arg(uint_t idx, ulong_t a)
    {
        check(m_context->SetArgDWord(idx, a));
    }
//    void set_arg(uint_t idx, int64_t a)
//    {
//        check(m_context->SetArgQWord(idx, a));
//    }
//    void set_arg(uint_t idx, uint64_t a)
//    {
//        check(m_context->SetArgQWord(idx, a));
//    }
    void set_arg(uint_t idx, float a)
    {
        check(m_context->SetArgFloat(idx, a));
    }
    void set_arg(uint_t idx, double a)
    {
        check(m_context->SetArgDouble(idx, a));
    }
    void set_arg(uint_t idx, wchar_t* a)
    {
        check(m_context->SetArgPointer(idx, a));
    }
    void set_arg(uint_t idx, const wchar_t* a)
    {
        check(m_context->SetArgPointer(idx, (void*)a));
    }

    template<typename T>
    struct get_ret_i
    {
        static T get(psIScriptContext* ctx)
        {
            return *(T*)ctx->GetReturnPointer();
        }
    };
    template<typename T>
    struct get_ret_i<T&>
    {
        static T& get(psIScriptContext* ctx)
        {
            return *(T*)ctx->GetReturnPointer();
        }
    };
    template<typename T>
    struct get_ret_i<T*>
    {
        static T* get(psIScriptContext* ctx)
        {
            return (T*)ctx->GetReturnPointer();
        }
    };
    template<> struct get_ret_i<char>
    {
        static char get(psIScriptContext* ctx)
        {
            return (char)ctx->GetReturnDWord();
        }
    };
    template<> struct get_ret_i<byte_t>
    {
        static byte_t get(psIScriptContext* ctx)
        {
            return (byte_t)ctx->GetReturnDWord();
        }
    };
    template<> struct get_ret_i<short>
    {
        static short get(psIScriptContext* ctx)
        {
            return (short)ctx->GetReturnDWord();
        }
    };
    template<> struct get_ret_i<ushort_t>
    {
        static ushort_t get(psIScriptContext* ctx)
        {
            return (ushort_t)ctx->GetReturnDWord();
        }
    };
    template<> struct get_ret_i<int>
    {
        static int get(psIScriptContext* ctx)
        {
            return (int)ctx->GetReturnDWord();
        }
    };
    template<> struct get_ret_i<uint_t>
    {
        static uint_t get(psIScriptContext* ctx)
        {
            return (uint_t)ctx->GetReturnDWord();
        }
    };
    template<> struct get_ret_i<long>
    {
        static long get(psIScriptContext* ctx)
        {
            return (long)ctx->GetReturnDWord();
        }
    };
    template<> struct get_ret_i<ulong_t>
    {
        static ulong_t get(psIScriptContext* ctx)
        {
            return (ulong_t)ctx->GetReturnDWord();
        }
    };
//    template<> struct get_ret_i<int64_t>
//    {
//        static int64_t get(psIScriptContext* ctx)
//        {
//            return (int64_t)ctx->GetReturnQWord();
//        }
//    };
//    template<> struct get_ret_i<uint64_t>
//    {
//        static uint64_t get(psIScriptContext* ctx)
//        {
//            return (uint64_t)ctx->GetReturnQWord();
//        }
//    };
    template<> struct get_ret_i<float>
    {
        static float get(psIScriptContext* ctx)
        {
            return ctx->GetReturnFloat();
        }
    };
    template<> struct get_ret_i<double>
    {
        static double get(psIScriptContext* ctx)
        {
            return ctx->GetReturnDouble();
        }
    };
    template<> struct get_ret_i<const wchar_t*>
    {
        static const wchar_t* get(psIScriptContext* ctx)
        {
            return (wchar_t*)ctx->GetReturnPointer();
        }
    };
    template<> struct get_ret_i<wchar_t*>
    {
        __declspec(deprecated) static wchar_t* get(psIScriptContext* ctx)
        {
            return (wchar_t*)ctx->GetReturnPointer();
        }
    };
    template<> struct get_ret_i<void>
    {
        static void get(psIScriptContext* )
        {
        }
    };
    template<> struct get_ret_i<bool>
    {
        static bool get(psIScriptContext* ctx)
        {
            return ctx->GetReturnDWord() != 0;
        }
    };

    template<typename T>
    T get_ret()
    {
        return get_ret_i<T>::get(m_context);
    }
};

#define BOOST_PP_ITERATION_PARAMS_1 (3, (0, PG_FUNCTIONS_PARAM_MAX, "detail/functor_i.h"))
#include BOOST_PP_ITERATE()


__PG_PSX_SPACE_END__

#endif  // __FUNCTOR_H_G_SRC_SOURCE_PG_PSX__