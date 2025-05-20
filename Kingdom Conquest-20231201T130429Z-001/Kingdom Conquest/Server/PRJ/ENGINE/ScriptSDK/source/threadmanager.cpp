#include "threadmanager.h"

//////////////////////////////////////////////////////////////////////////
// Singleton
struct ThreadManagerWrapper
{
   ThreadManagerWrapper()
   {
	  m_pThreadManager = new psCThreadManager;
   }
   ~ThreadManagerWrapper()
   {
	   delete m_pThreadManager;
   }
   psCThreadManager& GetThreadManager()
   {
	   return *m_pThreadManager;
   }
   void DeleteThreadManager()
   {
	   delete m_pThreadManager;
	   m_pThreadManager = NULL;
   }	

   psCThreadManager* m_pThreadManager;
}g_ThreadManager;

psCThreadManager& GetThreadManager()
{
   return g_ThreadManager.GetThreadManager();
}

void DeleteThreadManager()
{
	g_ThreadManager.DeleteThreadManager();
}


//======================================================================
#ifdef USE_THREADS

// From windows.h
extern "C"
{
	void PS_STDCALL InitializeCriticalSection(CRITICAL_SECTION *);
	void PS_STDCALL DeleteCriticalSection(CRITICAL_SECTION *);
	void PS_STDCALL EnterCriticalSection(CRITICAL_SECTION *);
	void PS_STDCALL LeaveCriticalSection(CRITICAL_SECTION *);
	unsigned long PS_STDCALL GetCurrentThreadId();
}

psCThreadManager::psCThreadManager()
{
}

psCThreadManager::~psCThreadManager()
{
	ENTERCRITICALSECTION(criticalSection);

	// Delete all thread local datas
	TldMap_t::iterator it = m_TldMap.begin();
	while ( it != m_TldMap.end() )
	{
		if (it->second)
			delete it->second;
		++it;
	}
	LEAVECRITICALSECTION(criticalSection);
}

int psCThreadManager::CleanupLocalData()
{
	psDWORD id = GetCurrentThreadId();
	int r = 0;

	ENTERCRITICALSECTION(criticalSection);
	
	TldMap_t::iterator it = m_TldMap.find(id);
	if (it != m_TldMap.end())
	{
		psCThreadLocalData *tld = (psCThreadLocalData*)it->second;

		// Can we really remove it at this time?
		if( tld->m_ActiveContexts.size() == 0 )
		{
			delete tld;
			m_TldMap.erase(it);
			r = 0;
		}
		else
			r = psCONTEXT_ACTIVE;
	}
	LEAVECRITICALSECTION(criticalSection);

	return r;
}

psCThreadLocalData *psCThreadManager::GetLocalData()
{
	psDWORD id = GetCurrentThreadId();

	psCThreadLocalData *tld = GetLocalData(id);
	if( tld == 0 )
	{
		// Create a new tld
		tld = new psCThreadLocalData();
		SetLocalData(id, tld);
	}

	return tld;
}

psCThreadLocalData *psCThreadManager::GetLocalData(psDWORD threadId)
{
	psCThreadLocalData *tld = 0;

	ENTERCRITICALSECTION(criticalSection);

	TldMap_t::iterator it = m_TldMap.find(threadId);
	if (it != m_TldMap.end())
		tld = it->second;

	LEAVECRITICALSECTION(criticalSection);

	return tld;
}

void psCThreadManager::SetLocalData(psDWORD threadId, psCThreadLocalData *tld)
{
	ENTERCRITICALSECTION(criticalSection);

	m_TldMap[threadId] = tld;

	LEAVECRITICALSECTION(criticalSection);
}

//=========================================================================
psCThreadCriticalSection::psCThreadCriticalSection()
{
	InitializeCriticalSection(&criticalSection);
}

psCThreadCriticalSection::~psCThreadCriticalSection()
{
	DeleteCriticalSection(&criticalSection);
}

void psCThreadCriticalSection::Enter()
{
	EnterCriticalSection(&criticalSection);
}

void psCThreadCriticalSection::Leave()
{
	LeaveCriticalSection(&criticalSection);
}

#endif
         

