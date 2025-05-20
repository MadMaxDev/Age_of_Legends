#ifndef __PS_MEMMANAGER_H__
#define __PS_MEMMANAGER_H__

#include <stdlib.h>
#include <string.h>
#include <vector>

class psTempMemManager
{
public:
	psTempMemManager(unsigned int PageSize);
	~psTempMemManager();

	void* alloc(size_t s);
	void  clear();
	void  destroy();

	static psTempMemManager& instance();

private:
	struct MEMORY_PAGE 
	{
		char*	 pMemory;
		size_t   Capacity;
	};
	unsigned int			   m_PageSize;
	unsigned int			   m_PageIndex;
	unsigned int			   m_PageOffset;
	std::vector< char* >	   m_Pages;
};

class psIMemManager
{
	static psIMemManager* s_first;
	psIMemManager* next;
public:
	psIMemManager()  
	{
		if ( s_first )
		{
			next = s_first;
		}else
		{
			next = NULL;
		}	
		s_first = this;
	}
	virtual void destroy() = 0;

	static void destroyAll();
protected:
	virtual ~psIMemManager() {}
};

template< class T >
class psMemManager: public psIMemManager
{
public:
	psMemManager( unsigned int PageSize ) : m_PageSize( PageSize )
	{
	}
	~psMemManager()
	{
		destroy();
	}
	void* alloc(size_t s) 
	{
		if ( m_FreeSlot.size() > 0 )
		{
			void* p = m_FreeSlot.back();
			m_FreeSlot.pop_back();
			return p;
		}else
		{
			void* p = ::malloc( s * m_PageSize );
			for (unsigned int i = 1; i < m_PageSize; ++i )
			{
				m_FreeSlot.push_back( (char*)(p) + i * s );
			}
			m_Pages.push_back( p );
			return p;
		}
	}
	void dealloc( void* p )
	{
		m_FreeSlot.push_back( p );
	}
	void destroy()
	{
		for (size_t i = 0; i < m_Pages.size(); ++i )
		{
			::free( m_Pages[i] );
		}
		m_Pages.clear();
		m_FreeSlot.clear();
	}
	static psMemManager<T>& instance()
	{
		static psMemManager<T> s_instance( 128 );
		return s_instance;
	}
protected:
	unsigned int		 m_PageSize;
	std::vector< void* > m_Pages;
	std::vector< void* > m_FreeSlot;
};

struct psManageTempMemory
{
	psManageTempMemory()					// 如果不写一个构造函数会报warning
	{
	}
	~psManageTempMemory()
	{
		psTempMemManager::instance().clear();
	}
};

#define DECL_TEMP_MEM_OP()				    void* operator new (size_t s) \
											{ return psTempMemManager::instance().alloc(s); } \
											void  operator delete (void* p) { }

#define DECL_MEM_OP(classname)				void* operator new (size_t s) \
											{ return psMemManager<classname>::instance().alloc(s); } \
											void  operator delete( void* p) \
											{ psMemManager<classname>::instance().dealloc(p); } 

#define MANAGE_TEMP_MEMORY()				psManageTempMemory __manage_temp_memory;

#endif // __PS_MEMMANAGER_H__
