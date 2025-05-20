#ifndef __PS_STDVECTOR_H__
#define __PS_STDVECTOR_H__

#include "pixelscript.h"
#include <vector>
#include <string>

template <typename T>
class psVectorRegisterHelper
{
public:
	static void PS_CDECL Construct(std::vector<T>* in)
	{
		::new (in) std::vector<T>();
	}

	static void PS_CDECL Destruct(std::vector<T>* in)
	{
		using namespace std;
		in->~vector<T>();
	}

	static void PS_CDECL CopyConstruct(std::vector<T>* in, const std::vector<T>& rhs)
	{
		::new (in) std::vector<T>(rhs);
	}

	static void PS_CDECL NumConstruct( std::vector<T>* in, int size )
	{
		::new (in) std::vector<T>(size);
	}

	static std::vector<T>& PS_CDECL Assign( std::vector<T>* lhs, const std::vector<T>& rhs )
	{
		*lhs = rhs;
		return *lhs;
	}

	static T* PS_CDECL Index(std::vector<T>* lhs, int i)
	{
		return &(*lhs)[i];
	}

	static int PS_CDECL Size(std::vector<T>* lhs)
	{
		return (int)lhs->size();
	}

	static void PS_CDECL Resize( std::vector<T>* lhs, int size )
	{
		lhs->resize(size);
	}

	static void PS_CDECL Clear( std::vector<T>* lhs )
	{
		lhs->clear();
	}

	static void PS_CDECL PushBack( std::vector<T> *lhs, const T& in )
	{
		lhs->push_back(in);
	}

	static void PS_CDECL PopBack(std::vector<T>* lhs)
	{
		lhs->pop_back();
	}
};

template <typename T>
void psRegisterStdVector(psIScriptEngine* engine, const psCHAR* vecName, const psCHAR* typeName)
{
	typedef std::basic_string< psCHAR, std::char_traits<psCHAR>, std::allocator<psCHAR> > string_t;

	PS_VERIFY( engine->RegisterObjectType( vecName, sizeof(std::vector<T>), psOBJ_CLASS_CDA ) );

	PS_VERIFY( engine->RegisterObjectBehaviour(  vecName, 
												 psBEHAVE_CONSTRUCT, 
												 PS_T("void f()"), 
												 psFUNCTION(psVectorRegisterHelper<T>::Construct), 
												 psCALL_CDECL_OBJFIRST) );

	PS_VERIFY( engine->RegisterObjectBehaviour( vecName,
												psBEHAVE_DESTRUCT,
												PS_T("void f()"),
												psFUNCTION(psVectorRegisterHelper<T>::Destruct),
												psCALL_CDECL_OBJFIRST) );

	string_t decl;
	decl = PS_T("void f(const ");
	decl += vecName;
	decl += PS_T(" &in)" );
	PS_VERIFY( engine->RegisterObjectBehaviour(   vecName,
												  psBEHAVE_CONSTRUCT,
												 decl.c_str(),
												 psFUNCTION(psVectorRegisterHelper<T>::CopyConstruct),
												 psCALL_CDECL_OBJFIRST) );

	PS_VERIFY( engine->RegisterObjectBehaviour( vecName,
												psBEHAVE_CONSTRUCT,
												PS_T("void f(int)"),
												psFUNCTION(psVectorRegisterHelper<T>::NumConstruct),
												psCALL_CDECL_OBJFIRST) );

	decl = typeName;
	decl += PS_T("& f(int)");
	PS_VERIFY( engine->RegisterObjectBehaviour(   vecName,
												  psBEHAVE_INDEX,
												  decl.c_str(),
												  psFUNCTION(psVectorRegisterHelper<T>::Index),
												  psCALL_CDECL_OBJFIRST) );

	decl = vecName;
	decl += PS_T("& f(const ");
	decl += vecName;
	decl += PS_T(" &in)" );
	PS_VERIFY( engine->RegisterObjectBehaviour(   vecName,
												  psBEHAVE_ASSIGNMENT,
												  decl.c_str(),
												  psFUNCTION(psVectorRegisterHelper<T>::Assign),
												  psCALL_CDECL_OBJFIRST ) );

	PS_VERIFY( engine->RegisterObjectMethod(   vecName,
											   PS_T("int size()"),
											   psFUNCTION(psVectorRegisterHelper<T>::Size),
											   psCALL_CDECL_OBJFIRST) );

	PS_VERIFY( engine->RegisterObjectMethod(    vecName ,
												PS_T("void resize(int)"),
												psFUNCTION(psVectorRegisterHelper<T>::Resize),
												psCALL_CDECL_OBJFIRST) );

	PS_VERIFY( engine->RegisterObjectMethod(  vecName, 
											  PS_T("void clear()"), 
											  psFUNCTION(psVectorRegisterHelper<T>::Clear),
											  psCALL_CDECL_OBJFIRST) );

	decl  = PS_T("void push_back(const ");
	decl += typeName;
	decl += PS_T(" &in)" );
	PS_VERIFY( engine->RegisterObjectMethod(  vecName,
											  decl.c_str(),
											  psFUNCTION(psVectorRegisterHelper<T>::PushBack),
											  psCALL_CDECL_OBJFIRST ) );

	PS_VERIFY( engine->RegisterObjectMethod( vecName,
											   PS_T("void pop_back()"),
											   psFUNCTION(psVectorRegisterHelper<T>::PopBack),
											   psCALL_CDECL_OBJFIRST ) );
}

#endif // __PS_STDVECTOR_H__
