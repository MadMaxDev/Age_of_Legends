#include "../inc/MonoHelper.h"
#include <mono/metadata/environment.h>

static const char*		s_cszNamespace			= "ServerBattle";
static const char*		s_cszClassName			= "BattleCal";
static const char*		s_cszMethodInitName		= "Init";
static const int		s_nMethodInitParamCount	= 4;
static const char*		s_cszMethodDoName		= "GetResult";
static const int		s_nMethodDoParamCount	= 0;

CMonoHelper::CMonoHelper()
	: m_pDomain(NULL)
	, m_pAssembly(NULL)
	, m_pImage(NULL)
	, m_pClass(NULL)
	, m_pObject(NULL)
	, m_pMethodInit(NULL)
	, m_pMethodDo(NULL)
	, m_hObject(0)
{

}
CMonoHelper::~CMonoHelper()
{
	Release();
}

int		CMonoHelper::Init(const char* cszFile)
{
	m_pDomain	= mono_jit_init(cszFile);
	if (m_pDomain == NULL)
	{
		return -1;
	}
	m_pAssembly	= mono_domain_assembly_open(m_pDomain, cszFile);
	if (m_pAssembly == NULL)
	{
		return -2;
	}
	m_pImage	= mono_assembly_get_image(m_pAssembly);
	if (m_pImage == NULL)
	{
		return -3;
	}
	m_pClass	= mono_class_from_name(m_pImage, s_cszNamespace, s_cszClassName);
	if (m_pClass == NULL)
	{
		return -4;
	}
	/* mono_object_new () only allocates the storage: 
	 * it doesn't run any constructor. Tell the runtime to run
	 * the default argumentless constructor.
	 */
	m_pObject	= mono_object_new(m_pDomain, m_pClass);
	if (m_pObject == NULL)
	{
		return -5;
	}
	mono_runtime_object_init(m_pObject);

	/* GC handles support 
	*
	* A handle can be created to refer to a managed object and either prevent it
	* from being garbage collected or moved or to be able to know if it has been 
	* collected or not (weak references).
	* mono_gchandle_new () is used to prevent an object from being garbage collected
	* until mono_gchandle_free() is called. Use a [TRUE] value for the pinned argument to
	* prevent the object from being moved (this should be avoided as much as possible 
	* and this should be used only for shorts periods of time or performance will suffer).
	* To create a weakref use mono_gchandle_new_weakref (): track_resurrection should
	* usually be false (see the GC docs for more details).
	* mono_gchandle_get_target () can be used to get the object referenced by both kinds
	* of handle: for a weakref handle, if an object has been collected, it will return NULL.
	*/
	m_hObject	= mono_gchandle_new(m_pObject, true);

	m_pMethodInit		= mono_class_get_method_from_name(m_pClass, s_cszMethodInitName, s_nMethodInitParamCount);
	if (m_pMethodInit == NULL)
	{
		return -6;
	}
	m_pMethodDo			= mono_class_get_method_from_name(m_pClass, s_cszMethodDoName, s_nMethodDoParamCount);
	if (m_pMethodDo == NULL)
	{
		return -7;
	}
	return 0;
}

int		CMonoHelper::Release()
{
	if (m_hObject != 0)
	{
		mono_gchandle_free(m_hObject);
		m_hObject		= 0;
	}

	m_pMethodInit		= NULL;
	m_pMethodDo			= NULL;
	m_pClass			= NULL;
	m_pImage			= NULL;
	m_pAssembly			= NULL;

	if (m_pDomain != NULL)
	{
		mono_jit_cleanup(m_pDomain);
		m_pDomain		= NULL;
	}
	
	return 0;
}

const char*		CMonoHelper::DoOneQuery(int nRand, const char* cszAttackData, const char* cszDefenseData, float fCounterRate)
{
	// 初始化参数
	void*	args[s_nMethodInitParamCount];
	args[0] = &nRand;
	args[1]	= &fCounterRate;
	args[2] = mono_string_new(m_pDomain, cszAttackData);
	args[3] = mono_string_new(m_pDomain, cszDefenseData);
	mono_runtime_invoke(m_pMethodInit, m_pObject, args, NULL);
	// 战斗计算
	MonoString*	pResult		= (MonoString*)mono_runtime_invoke(m_pMethodDo, m_pObject, NULL, NULL);
	char*		szResult	= mono_string_to_utf8(pResult);
	unsigned int	nLength	= strlen(szResult)+1;
	m_vectResultBuf.resize(nLength);
	memcpy(m_vectResultBuf.getbuf(), szResult, nLength);
	// 释放内存
	mono_free(szResult);
	return m_vectResultBuf.getbuf();
}
