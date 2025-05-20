#include "../inc/Excel_Cell.h"
#include <list>
#include <vector>
#include <assert.h>

enum
{
	POOL_SIZE = 1024 * 1024 * 2
};
//#include <windows.h>
//struct mem_track
//{
//    int m_count;
//    int m_size;
//    const char* m_name;
//    mem_track(const char* n) : m_name(n)
//    {
//        m_count = 0;
//        m_size  = 0;
//    }
//    ~mem_track()
//    {
//        char buf[128];
//        sprintf(buf, "%s : count %d size %d.\n", m_name, m_count, m_size);
//        OutputDebugString(buf);
//    }
//    void add(size_t x)
//    {
//        ++ m_count;
//        m_size += (int)x;
//    }
//};
//
//mem_track g_pool_track("pool");
//mem_track g_str_track("str");
//mem_track g_int_track("int");
//mem_track g_flt_track("flt");
//mem_track g_v_str_track("v_str");
//mem_track g_v_int_track("v_int");
//mem_track g_v_flt_track("v_flt");

struct Excel_Pools
{
	struct Excel_Pool
	{
		unsigned long	Alloced;
		unsigned char*	pBuffer;

		void Release()
		{
			delete[]  pBuffer;
		}

		void Create()
		{
			Alloced = 0;
			pBuffer = new unsigned char[POOL_SIZE];
		}
	};

    struct string_data
    {
        int     m_size;
        char    m_data[1];
    };
    struct cell_atom
    {
        cell_atom*  m_next;
    };
	
    typedef std::vector<const char*> string_vec;


	std::list< Excel_Pool >	m_PoolList;
    string_vec  m_strings;


    Excel_Pools()
    {
        m_strings.reserve(4096);
    }

	~Excel_Pools()
	{
		std::list< Excel_Pool >::iterator itor = m_PoolList.begin();
		for( ; itor != m_PoolList.end() ; ++itor )
		{
			Excel_Pool& Pool = *itor;
			Pool.Release();
		}
	}

    // By Jiangli 共享字符串
    // 这部分代码可节约1M内存
    const char* AllocStr(const char* s)
    {
#if 1
        int len = (int)strlen(s);
        if(len >= 32)
        {
            char* d = (char*)Alloc(len+1);
            //g_str_track.add(len+1);
            memcpy(d, s, len+1);
            return d;
        }

		if (m_strings.size() == 0)
		{
			// 分配新的字符串
			char* d = (char*)Alloc(len+1);
			memcpy(d, s, len+1);
			m_strings.push_back(d);
			return d;
		}

        // 使用折半查找。
        const char** b = &m_strings[0];
        const char** p = b;
        size_t c = m_strings.size();
	    while(c > 0)
	    {
            size_t half = c/2;
            const char* mid = *(p+half);

            int i = strcmp(mid, s);
            if(i == 0)
            {
                return mid;
            }
            if(i < 0)
            {
                p = p+half+1;
                c -= half+1;
            }
            else
            {
                c = half;
            }
	    }
        // 分配新的字符串
        char* d = (char*)Alloc(len+1);
        //g_str_track.add(len+1);
        memcpy(d, s, len+1);
        m_strings.insert(m_strings.begin()+(p-b), d);
        return d;
#else
        int len = (int)strlen(s);
        char* d = (char*)Alloc(len+1);
        memcpy(d, s, len+1);
        return d;
#endif
    }
	
	unsigned char* Alloc( unsigned long _Size )
	{
		if( _Size >= POOL_SIZE )
		{
			assert( false );
			return NULL;
		}
        //g_pool_track.add(_Size);
		std::list< Excel_Pool >::iterator itor = m_PoolList.begin();
		for( ; itor != m_PoolList.end() ; ++itor )
		{
			Excel_Pool& Pool = *itor;
			if( POOL_SIZE - Pool.Alloced > _Size )
			{
				unsigned char* pB = Pool.pBuffer + Pool.Alloced;
				Pool.Alloced += _Size;
				return pB;
			}
		}
#if 1
        // 放到前端有利于可用空间被尽早搜索到。
		m_PoolList.push_front( Excel_Pool() );
		m_PoolList.front().Create();
		m_PoolList.front().Alloced = _Size;
		return m_PoolList.front().pBuffer;
#else
		m_PoolList.push_back( Excel_Pool() );
		m_PoolList.back().Create();
		m_PoolList.back().Alloced = _Size;
		return m_PoolList.back().pBuffer;
#endif
	}
};

#ifdef _MSC_VER
#pragma warning(disable : 4073)
#pragma init_seg(lib)
static Excel_Pools g_Pools;
#else
Excel_Pools g_Pools __attribute__((init_priority(2000)));
#endif

static inline unsigned char* Excel_Alloc( unsigned long _Size )
{
	return g_Pools.Alloc( _Size );
}

static inline void Excel_Write(   unsigned char*&	_pBuffer 
						 , const void*		_pVal  
						 , unsigned long	_Size )
{
	memcpy( _pBuffer , _pVal , _Size );
	_pBuffer += _Size;
}

#if 1
void* Excel_LineData_Alloc(int c)
{
    return g_Pools.Alloc(c);
}
void Excel_LineData_Free(void*)
{
}
#else
void* Excel_LineData_Alloc(int c)
{
    return ::malloc(c);
}
void Excel_LineData_Free(void* p)
{
    ::free(p);
}
#endif

Excel_Cell::Excel_Cell( const char* _sz )
{
    theStr = g_Pools.AllocStr(_sz);
}

Excel_Cell::Excel_Cell( float _fvec[] , unsigned long _cnt )
{
	unsigned long buf_size = sizeof( unsigned long ) + _cnt * sizeof( float );

	unsigned char* pBuf = Excel_Alloc( buf_size );
    //g_v_flt_track.add(buf_size);
	
	Excel_Write( pBuf , &_cnt , sizeof( unsigned long ) );

	theFloatVec = ( float* )pBuf;

	Excel_Write( pBuf , _fvec , _cnt * sizeof( float ) );
}

Excel_Cell::Excel_Cell( long _lvec[] , unsigned long _cnt )
{
	unsigned long buf_size = sizeof( unsigned long ) + _cnt * sizeof( long );

	unsigned char* pBuf = Excel_Alloc( buf_size );
    //g_v_int_track.add(buf_size);

	Excel_Write( pBuf , &_cnt , sizeof( unsigned long ) );

	theIntVec = ( long* )pBuf;

	Excel_Write( pBuf , _lvec , _cnt * sizeof( long ) );
}

Excel_Cell::Excel_Cell( const char* _sz_vec[] , unsigned long _cnt )
{
	unsigned long buf_size = sizeof( unsigned long ) + _cnt * sizeof( const char* );

	unsigned char* pBuf = Excel_Alloc( buf_size );
    //g_v_str_track.add(buf_size);

	Excel_Write( pBuf , &_cnt , sizeof( unsigned long ) );

	theStrVec = ( const char** )pBuf;
	
	for( unsigned long i = 0 ; i < _cnt ; ++i )
	{
        theStrVec[i] = g_Pools.AllocStr(_sz_vec[i]);
	}
}
