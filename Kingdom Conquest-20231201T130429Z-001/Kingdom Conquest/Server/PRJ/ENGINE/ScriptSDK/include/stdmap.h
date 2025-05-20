#ifndef __PS_STDMAP_H__
#define __PS_STDMAP_H__

#include <map>
#include <string>
#include "pixelscript.h"

template <typename KEY, typename VALUE>
class psMapRegisterHelper
{
public:
	typedef typename std::map<KEY, VALUE> map_t;
	typedef typename std::map<KEY, VALUE>::iterator map_iterator;

	static void PS_CDECL Construct(map_t* in)
	{
		::new (in) map_t();
	}

	static void PS_CDECL Destruct(map_t* in)
	{
		in->~map_t();
	}

	static void PS_CDECL CopyConstruct(map_t* in, const map_t& rhs)
	{
		::new (in) map_t(rhs);
	}

	static map_t& PS_CDECL Assign( map_t* lhs, const map_t& rhs )
	{
		*lhs = rhs;
		return *lhs;
	}

	static VALUE& PS_CDECL Index(map_t& lhs, const KEY& key)
	{
		return lhs[key];
	}

	static int PS_CDECL Size(map_t* lhs)
	{
		return (int)lhs->size();
	}

	static void PS_CDECL Clear( map_t* lhs )
	{
		lhs->clear();
	}

	static map_iterator PS_CDECL Find( map_t* lhs, const KEY& key )
	{
		return lhs->find( key );
	}

	static map_iterator PS_CDECL Begin( map_t* lhs )
	{
		return lhs->begin();
	}

	static map_iterator PS_CDECL End( map_t* lhs )
	{
		return lhs->end();
	}

	//---------------------------------------------------------------------
	static void PS_CDECL ConstructIt(map_iterator* in )
	{
		::new (in) map_iterator();
	}

	static void PS_CDECL DestructIt(map_iterator* in)
	{
		in->~map_iterator();
	}

	static void PS_CDECL CopyConstructIt(map_iterator* in, const map_iterator& rhs)
	{
		::new (in) map_iterator(rhs);
	}

	static map_iterator& PS_CDECL AssignIt( map_iterator* lhs, const map_iterator& rhs )
	{
		*lhs = rhs;
		return *lhs;
	}

	static const KEY& PS_CDECL GetItKey( map_iterator* in )
	{
		return (*in)->first;
	}

	static VALUE& PS_CDECL GetItValue( map_iterator* in )
	{
		return (*in)->second;
	}

	static bool PS_CDECL EqualIt( map_iterator& lhs, map_iterator& rhs )
	{
		return lhs == rhs;
	}
	static bool PS_CDECL NotEqualIt( map_iterator& lhs, map_iterator& rhs )
	{
		return lhs != rhs;
	}
	static map_iterator& PS_CDECL ItInc(map_iterator& lhs)
	{
		return ++lhs;
	}
	static map_iterator& PS_CDECL ItDec(map_iterator& lhs)
	{
		return --lhs;
	}
};

template <typename KEY, typename VALUE>
void psRegisterStdMap(psIScriptEngine* engine, const psCHAR* mapName, const psCHAR* mapItName, const psCHAR* keyTypeName, const psCHAR* valueTypeName)
{
	typedef psMapRegisterHelper<KEY, VALUE> RegisterHelper_t;
	typedef std::basic_string< psCHAR, std::char_traits<psCHAR>, std::allocator<psCHAR> > string_t;

	string_t decl;

	//------------------------------------------------------------------------------------------------
	PS_VERIFY( engine->RegisterObjectType( mapItName, sizeof(typename RegisterHelper_t::map_iterator), psOBJ_CLASS_CDA) );

	PS_VERIFY( engine->RegisterObjectBehaviour( mapItName, 
												psBEHAVE_CONSTRUCT, 
												PS_T("void f()"), 
												psFUNCTION(RegisterHelper_t::ConstructIt), 
												psCALL_CDECL_OBJFIRST) );

	PS_VERIFY( engine->RegisterObjectBehaviour( mapItName,
												psBEHAVE_DESTRUCT,
												PS_T("void f()"),
												psFUNCTION(RegisterHelper_t::DestructIt),
												psCALL_CDECL_OBJFIRST) );

	decl = PS_T("void f(const ");
	decl += mapItName;
	decl += PS_T(" &in)");
	PS_VERIFY( engine->RegisterObjectBehaviour( mapItName,
												psBEHAVE_CONSTRUCT,
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::CopyConstructIt),
												psCALL_CDECL_OBJFIRST) );

	decl = mapItName;
	decl += PS_T("& f(const ");
	decl += mapItName;
	decl += PS_T(" &in)");
	PS_VERIFY( engine->RegisterObjectBehaviour( mapItName,
												psBEHAVE_ASSIGNMENT,
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::AssignIt),
												psCALL_CDECL_OBJFIRST ) );

	decl  = mapItName;
	decl += PS_T("& f()");
	PS_VERIFY( engine->RegisterObjectBehaviour( mapItName, 
											    psBEHAVE_INC, 
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::ItInc),
												psCALL_CDECL_OBJFIRST ) );
	PS_VERIFY( engine->RegisterObjectBehaviour( mapItName, 
												psBEHAVE_DEC, 
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::ItDec),
												psCALL_CDECL_OBJFIRST ) );

	decl = string_t(PS_T("bool f(const ")) + mapItName + PS_T(" &lhs, const ") + mapItName + PS_T(" &rhs)");
	PS_VERIFY( engine->RegisterGlobalBehaviour( psBEHAVE_EQUAL,
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::EqualIt),
												psCALL_CDECL ) );
	PS_VERIFY( engine->RegisterGlobalBehaviour( psBEHAVE_NOTEQUAL,
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::NotEqualIt),
												psCALL_CDECL ) );

	decl = PS_T("const ");
	decl += keyTypeName;
	decl += PS_T("& key()");
	PS_VERIFY( engine->RegisterObjectMethod(   mapItName, 
											   decl.c_str(),
											   psFUNCTION(RegisterHelper_t::GetItKey),
											   psCALL_CDECL_OBJFIRST ) );


	decl =  valueTypeName;
	decl += PS_T(" & value()");
	PS_VERIFY( engine->RegisterObjectMethod(   mapItName, 
											   decl.c_str(),
											   psFUNCTION(RegisterHelper_t::GetItValue),
											   psCALL_CDECL_OBJFIRST ) );

	//------------------------------------------------------------------------------------------------
	PS_VERIFY( engine->RegisterObjectType( mapName, sizeof(typename RegisterHelper_t::map_t), psOBJ_CLASS_CDA ) );

	PS_VERIFY( engine->RegisterObjectBehaviour(  mapName, 
												 psBEHAVE_CONSTRUCT, 
												 PS_T("void f()"), 
												 psFUNCTION(RegisterHelper_t::Construct), 
												 psCALL_CDECL_OBJFIRST) );

	PS_VERIFY( engine->RegisterObjectBehaviour( mapName,
												psBEHAVE_DESTRUCT,
												PS_T("void f()"),
												psFUNCTION(RegisterHelper_t::Destruct),
												psCALL_CDECL_OBJFIRST) );

	decl = PS_T("void f(const ");
	decl += mapName;
	decl += PS_T(" &in)" );
	PS_VERIFY( engine->RegisterObjectBehaviour( mapName,
												psBEHAVE_CONSTRUCT,
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::CopyConstruct),
												psCALL_CDECL_OBJFIRST) );

	decl = valueTypeName;
	decl += PS_T(" & f(const ");
	decl += keyTypeName;
	decl += PS_T(" &in)");
	PS_VERIFY( engine->RegisterObjectBehaviour( mapName,
												psBEHAVE_INDEX,
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::Index),
												psCALL_CDECL_OBJFIRST) );

	decl  = mapName;
	decl += PS_T( "& f(const ");
	decl += mapName;
	decl += PS_T( " &in)" );
	PS_VERIFY( engine->RegisterObjectBehaviour( mapName,
												psBEHAVE_ASSIGNMENT,
												decl.c_str(),
												psFUNCTION(RegisterHelper_t::Assign),
												psCALL_CDECL_OBJFIRST ) );

	PS_VERIFY( engine->RegisterObjectMethod(mapName,
											PS_T("int size()"),
											psFUNCTION(RegisterHelper_t::Size),
											psCALL_CDECL_OBJFIRST) );

	PS_VERIFY( engine->RegisterObjectMethod(mapName, 
											PS_T("void clear()"), 
											psFUNCTION(RegisterHelper_t::Clear),
											psCALL_CDECL_OBJFIRST) );

	decl = mapItName;
	decl += PS_T(" find(const ");
	decl += keyTypeName;
	decl += PS_T(" &in)");
	PS_VERIFY( engine->RegisterObjectMethod(mapName, 
											decl.c_str(), 
											psFUNCTION(RegisterHelper_t::Find),
											psCALL_CDECL_OBJFIRST) );

	decl  = mapItName;
	decl += PS_T(" begin()" );
	PS_VERIFY( engine->RegisterObjectMethod(mapName, 
											decl.c_str() ,
											psFUNCTION(RegisterHelper_t::Begin),
											psCALL_CDECL_OBJFIRST) );
	decl  = mapItName;
	decl += PS_T(" end()" );
	PS_VERIFY( engine->RegisterObjectMethod(mapName, 
											decl.c_str() ,
											psFUNCTION(RegisterHelper_t::End),
											psCALL_CDECL_OBJFIRST) );

}

#endif // __PS_STDMAP_H__
