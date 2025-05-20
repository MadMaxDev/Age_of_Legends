#ifndef __MonoHelper_H__
#define __MonoHelper_H__

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

#include "WHCMN/inc/whvector.h"
using namespace n_whcmn;

class CMonoHelper
{
public:
	CMonoHelper();
	~CMonoHelper();

	int		Init(const char* cszFile);
	int		Release();

	const char*		DoOneQuery(int nRand, const char* cszAttackData, const char* cszDefenseData, float fCounterRate);
private:
	MonoDomain*		m_pDomain;
	MonoAssembly*	m_pAssembly;
	MonoImage*		m_pImage;
	MonoClass*		m_pClass;
	MonoObject*		m_pObject;
	MonoMethod*		m_pMethodInit;
	MonoMethod*		m_pMethodDo;
	uint32_t		m_hObject;

	whvector<char>	m_vectResultBuf;
};

#endif
