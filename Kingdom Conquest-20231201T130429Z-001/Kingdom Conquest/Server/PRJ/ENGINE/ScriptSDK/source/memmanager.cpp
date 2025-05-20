#include "memmanager.h"

//psTempMemManager g_tempmemmgr(4096);
psIMemManager* psIMemManager::s_first = NULL;

void psIMemManager::destroyAll()
{
	psIMemManager* p = s_first;
	while ( p )
	{
		p->destroy();
		p = p->next;
	}
}

#define PAGE_SIZE 130400    //随便取了一个2^17，还留了些空余
psTempMemManager::psTempMemManager(unsigned int PageSize) : m_PageSize(PageSize)
														  , m_PageIndex(0)
														  , m_PageOffset(0)
{
	m_Pages.push_back( (char*)malloc( PAGE_SIZE ) );
}

psTempMemManager::~psTempMemManager()
{
	destroy();
}

void psTempMemManager::destroy()
{
	for (size_t i = 0; i < m_Pages.size(); ++i )
	{
		free( m_Pages[i] );
	}
	m_Pages.clear();
	m_PageIndex = 0;
	m_PageOffset = 0;
}

void* psTempMemManager::alloc(size_t s)
{
	if ( m_PageOffset + s <= m_PageSize )
	{
		char* ptr  = m_Pages[m_PageIndex] + m_PageOffset;
		m_PageOffset += (unsigned int)s;
		return (void*)ptr;
	}
	else
	{
		m_PageOffset = 0;
		m_PageIndex++;
		if ( m_PageIndex >= m_Pages.size() )
		{
			// allocate a new page
			m_Pages.push_back( (char*)malloc( m_PageSize ) );
		}
		m_PageOffset += (unsigned int)s;
		return (void*)m_Pages[m_PageIndex];
	}
}

void psTempMemManager::clear()
{
	m_PageOffset = 0;
	m_PageIndex  = 0;
}

psTempMemManager& psTempMemManager::instance()
{
	static psTempMemManager s_instance( PAGE_SIZE );
	return s_instance;
}


