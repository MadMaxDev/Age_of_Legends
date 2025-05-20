#ifndef __PS_THREADMANAGER_H__
#define __PS_THREADMANAGER_H__

#include "config.h"
#include "string.h"

//=======================================================================
class psCThreadLocalData
{
public:
	std::vector<psIScriptContext *> m_ActiveContexts;
	psCString m_String;

protected:
	friend class psCThreadManager;

	psCThreadLocalData()  { }
	~psCThreadLocalData() { }
};

#ifdef USE_THREADS
	//======================================================================
	extern "C"
	{
		long  _InterlockedIncrement(long volatile *Addend);
		#pragma intrinsic (_InterlockedIncrement)

		long  _InterlockedDecrement(long volatile *Addend);
		#pragma intrinsic (_InterlockedDecrement)	  

		long _InterlockedExchange(long volatile *Target, long Value);
		#pragma intrinsic (_InterlockedExchange)
	}

	class psCAtomicCount
	{
	protected:
		volatile long m_value;
	public:
		psCAtomicCount() : m_value(0)
		{
		}

		psCAtomicCount& operator = ( long v)
		{
			_InterlockedExchange(&m_value, v); 
			return *this;
		}

		long operator++()
		{
			return _InterlockedIncrement(&m_value);
		}

		long operator--()
		{
			return _InterlockedDecrement(&m_value);
		}

		operator long() const
		{
			return m_value;
		}
	};

	//========================================================================
	#define DECLARECRITICALSECTION(x) psCThreadCriticalSection x
	#define ENTERCRITICALSECTION(x) x.Enter()
	#define LEAVECRITICALSECTION(x) x.Leave()

	// From windows.h
	struct CRITICAL_SECTION 
	{
		int reserved[6];
	};

	class psCThreadCriticalSection
	{
	public:
		psCThreadCriticalSection();
		~psCThreadCriticalSection();

		void Enter();
		void Leave();

	protected:
		CRITICAL_SECTION criticalSection;
	};

	class psCThreadManager
	{
	public:
		psCThreadManager();
		~psCThreadManager();

		psCThreadLocalData *GetLocalData();
		int CleanupLocalData();
	protected:
		psCThreadLocalData *GetLocalData(psDWORD threadId);
		void SetLocalData(psDWORD threadId, psCThreadLocalData *tld);

		typedef std::map<unsigned int, psCThreadLocalData* > TldMap_t;
		TldMap_t m_TldMap;

		DECLARECRITICALSECTION(criticalSection);
	};
#else
	typedef long psCAtomicCount;

	#define DECLARECRITICALSECTION(x)
	#define ENTERCRITICALSECTION(x)
	#define LEAVECRITICALSECTION(x)

    class psCThreadManager
	{
	public:
		psCThreadManager(): m_Tld(0) {}
		~psCThreadManager()
		{
			if ( m_Tld )
				delete m_Tld;
		}

		psCThreadLocalData *GetLocalData() 
		{
			if ( m_Tld == 0 )
				m_Tld = new psCThreadLocalData();
			return m_Tld;
		}
	protected:
		psCThreadLocalData* m_Tld;
	};
#endif

psCThreadManager& GetThreadManager();
void			  DeleteThreadManager();


#endif // __PS_THREADMANAGER_H__
