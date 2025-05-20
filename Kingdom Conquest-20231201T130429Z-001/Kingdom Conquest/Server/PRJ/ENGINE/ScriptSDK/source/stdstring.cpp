#include <assert.h>
#include <sstream>
#include <stdio.h>
#include "../include/stdstring.h"


template <typename xchar, bool bWideChar = (sizeof(xchar) == sizeof(wchar_t) ) >
class psStringRegisterHelper
{
public:
	typedef std::basic_string< xchar, std::char_traits<xchar>,		 std::allocator<xchar> > string_t;
	typedef std::basic_ostringstream<xchar, std::char_traits<xchar>, std::allocator<xchar> > ostringstream_t;

	struct StringBuffer: public psIBuffer
	{
		StringBuffer( size_t s )
		{
			m_pBuf = (psBYTE*)::malloc( s );
			m_NeedDel = true;
		}
		StringBuffer( psBYTE* pBuf ) : m_pBuf( pBuf )
								     , m_NeedDel( false )
									 , m_RefCount( 1 )
		{
		}
		~StringBuffer()
		{
			if ( m_pBuf && m_NeedDel  )
			{
				::free( m_pBuf );
			}
		}
		void Release()
		{
			if ( --m_RefCount == 0 )
			{
				delete this;
			}
		}
		void AddRef()
		{
			++m_RefCount;
		}
		psBYTE* GetBufferPointer()
		{
			return m_pBuf;
		}
		psBYTE* m_pBuf;
		bool    m_NeedDel;
		psDWORD m_RefCount;
	};
	static psAutoStringPtr PS_STDCALL GetStringValue(void* obj)
	{
		assert( obj );

		psIBuffer* pBuf = NULL;	
		if ( bWideChar )
		{	
			std::wstring* str = (std::wstring*)obj;

			// Unicode
			#ifdef PS_UNICODE	
				pBuf = new StringBuffer( (psBYTE*)(str->c_str()) );
			#else
				pBuf = new StringBuffer( (str->length() + 1) * sizeof(wchar_t) );
				wcstombs( (char*)(pBuf->GetBufferPointer()), str->c_str(), (str->length() + 1) * sizeof(wchar_t) );
			#endif
		}else
		{	
			std::string* str = (std::string*)obj;

			// Ansi
			#ifdef PS_UNICODE	
				pBuf = new StringBuffer( (str->length() + 1) * sizeof(wchar_t) );
				mbstowcs( (wchar_t*)(pBuf->GetBufferPointer()), str->c_str(), str->length() + 1 );
			#else
				pBuf = new StringBuffer( (psBYTE*)(str->c_str() ) );
			#endif
		}
		return psAutoStringPtr( pBuf );
	}

	static void PS_STDCALL SetStringValue(void* obj, const psCHAR* value)
	{	
		assert( obj && value );
		if ( bWideChar )
		{	
			std::wstring* str = (std::wstring*)obj;

			// Unicode
			#ifdef PS_UNICODE	
				*str = value;
			#else
				size_t len = ps_strlen( value );
				wchar_t *wbuf = new wchar_t[ len + 1];
				mbstowcs( wbuf, value, len + 1 );
				*str = wbuf;
				delete []wbuf;
			#endif
		}else
		{	
			std::string* str = (std::string*)obj;

			// Ansi
			#ifdef PS_UNICODE
				size_t mblen = ( ps_strlen( value ) + 1 ) * sizeof(wchar_t);
				char *mbuf = new char[ mblen ];
				wcstombs( mbuf, value, mblen );
				*str = mbuf;
				delete []mbuf;
			#else
				*str = value;
			#endif
		}
	}

	static xchar& GetChar(string_t& s, int index) 
	{
		return s[index];
	}
	static int GetLength(string_t& s)
	{
		return (int)s.length();
	}

	static void ConstructString(string_t *thisPointer)
	{
		new(thisPointer) string_t();
	}

	static void ConstructString2(string_t* thisPointer, const xchar* str)
	{
		new(thisPointer) string_t(str);
	}

	static void ConstructString3(string_t* thisPointer, const string_t& rhs )
	{
		new(thisPointer) string_t(rhs);
	}

	static void DestructString(string_t *thisPointer)
	{
		thisPointer->~string_t();
	}

	static string_t AddIntString(string_t &str, int i)
	{
		ostringstream_t stream;
		stream << i;
		return stream.str() + str;
	}

	static string_t &AssignIntToString(string_t &dest, int i)
	{
		ostringstream_t stream;
		stream << i;
		dest = stream.str(); 
		return dest;
	}

	static string_t &AddAssignIntToString(string_t &dest, int i)
	{
		ostringstream_t stream;
		stream << i;
		dest += stream.str(); 
		return dest;
	}

	static string_t AddStringInt(string_t &str, int i)
	{
		ostringstream_t stream;
		stream << i;
		return str + stream.str();
	}

	static string_t &AssignFloatToString(string_t &dest, float f)
	{
		ostringstream_t stream;
		stream << f;
		dest = stream.str(); 
		return dest;
	}

	static string_t &AddAssignFloatToString( string_t &dest, float f )
	{
		ostringstream_t stream;
		stream << f;
		dest += stream.str(); 
		return dest;
	}

	static string_t AddStringFloat(string_t &str, float f)
	{
		ostringstream_t stream;
		stream << f;
		return str + stream.str();
	}

	static string_t AddFloatString(string_t &str, float f)
	{
		ostringstream_t stream;
		stream << f;
		return stream.str() + str;
	}

	static string_t AddTextString(const xchar* text, string_t& str)
	{
		string_t r = text;
		r += str;
		return r;
	}

	static string_t AddStringText(string_t& str, const xchar* text)
	{
		return str + text;
	}

	static string_t& AssignTextToString(string_t& str, const xchar* text)
	{
		str  = text;
		return str;
	}

	static string_t& AddAssignTextToString(string_t& str, const xchar* text )
	{
		str += text;
		return str;
	}
};

void psRegisterStdStringA(psIScriptEngine *engine)
{
	typedef psStringRegisterHelper< char > helper_t;
	typedef helper_t::string_t			   string_t;
	using namespace std;

	// Register the bstr type
	PS_VERIFY( engine->RegisterObjectType(PS_T("string"), sizeof(string_t), psOBJ_CLASS_CDA) );	

	// Register the bstr factory
	PS_VERIFY( engine->RegisterStringFactory(PS_T("string"), &helper_t::GetStringValue, &helper_t::SetStringValue) );	

	// Register the object operator overloads
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_CONSTRUCT,  PS_T("void f()"),               psFUNCTION(helper_t::ConstructString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_CONSTRUCT,  PS_T("void f(const char* s)"), psFUNCTION(helper_t::ConstructString2), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_CONSTRUCT,  PS_T("void f(const string & s)"),      psFUNCTION(helper_t::ConstructString3), psCALL_CDECL_OBJFIRST));

	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_DESTRUCT,   PS_T("void f()"),                    psFUNCTION(helper_t::DestructString),  psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_ASSIGNMENT, PS_T("string &f(const string & s)"), psMETHODPR(string_t, operator =, (const string_t&), string_t&), psCALL_THISCALL));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_ASSIGNMENT, PS_T("string &f(const char* s)"),    psFUNCTION(helper_t::AssignTextToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_INDEX,      PS_T("char& f(int idx)"),            psFUNCTION(helper_t::GetChar), psCALL_CDECL_OBJFIRST));

	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_ADD_ASSIGN, PS_T("string &f(const string &in)"), psMETHODPR(string_t, operator+=, (const string_t&), string_t&), psCALL_THISCALL));

	// Register the global operator overloads
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_EQUAL,       PS_T("bool f(const string &in, const string &in)"), psFUNCTIONPR(operator==, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_NOTEQUAL,    PS_T("bool f(const string &in, const string &in)"), psFUNCTIONPR(operator!=, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_LEQUAL,      PS_T("bool f(const string &in, const string &in)"), psFUNCTIONPR(operator<=, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_GEQUAL,      PS_T("bool f(const string &in, const string &in)"), psFUNCTIONPR(operator>=, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_LESSTHAN,    PS_T("bool f(const string &in, const string &in)"), psFUNCTIONPR(operator <, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_GREATERTHAN, PS_T("bool f(const string &in, const string &in)"), psFUNCTIONPR(operator >, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("string f(const string &in, const string &in)"), psFUNCTIONPR(operator +, (const string_t &, const string_t &), string_t), psCALL_CDECL));

	// Register the object methods
	PS_VERIFY( engine->RegisterObjectMethod(PS_T("string"), PS_T("int length()"),  psFUNCTION(helper_t::GetLength),    psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectMethod(PS_T("string"), PS_T("char* c_str()"), psMETHOD(string_t, c_str), psCALL_THISCALL));

	// Automatic conversion from values
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_ASSIGNMENT, PS_T("string &f(float)"), psFUNCTION(helper_t::AssignFloatToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_ADD_ASSIGN, PS_T("string &f(float)"), psFUNCTION(helper_t::AddAssignFloatToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("string f(const string &in, float)"), psFUNCTION(helper_t::AddStringFloat), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("string f(float, const string &in)"), psFUNCTION(helper_t::AddFloatString), psCALL_CDECL));

	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_ASSIGNMENT, PS_T("string &f(int)"), psFUNCTION(helper_t::AssignIntToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_ADD_ASSIGN, PS_T("string &f(int)"), psFUNCTION(helper_t::AddAssignIntToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("string f(const string &in, int)"), psFUNCTION(helper_t::AddStringInt), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("string f(int, const string &in)"), psFUNCTION(helper_t::AddIntString), psCALL_CDECL));

	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,		  PS_T("string f(const char*, const string & in)"), psFUNCTION(helper_t::AddTextString), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,		  PS_T("string f(const string & in, const char*)"), psFUNCTION(helper_t::AddStringText), psCALL_CDECL));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("string"), psBEHAVE_ADD_ASSIGN, PS_T("string &f(const char*)"), psFUNCTION(helper_t::AddAssignTextToString), psCALL_CDECL_OBJFIRST) );
}

void psRegisterStdStringW(psIScriptEngine *engine)
{
	typedef psStringRegisterHelper< wchar_t > helper_t;
	typedef helper_t::string_t				  string_t;
	using namespace std;

	// Register the bstr type
	PS_VERIFY( engine->RegisterObjectType(PS_T("wstring"), sizeof(string_t), psOBJ_CLASS_CDA) );	

	// Register the bstr factory
	PS_VERIFY( engine->RegisterStringFactory(PS_T("wstring"), &helper_t::GetStringValue, &helper_t::SetStringValue) );	

	// Register the object operator overloads
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_CONSTRUCT,  PS_T("void f()"),               psFUNCTION(helper_t::ConstructString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_CONSTRUCT,  PS_T("void f(const wchar_t* s)"), psFUNCTION(helper_t::ConstructString2), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_CONSTRUCT,  PS_T("void f(const wstring & s)"),      psFUNCTION(helper_t::ConstructString3), psCALL_CDECL_OBJFIRST));

	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_DESTRUCT,   PS_T("void f()"),                    psFUNCTION(helper_t::DestructString),  psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_ASSIGNMENT, PS_T("wstring &f(const wstring & s)"), psMETHODPR(string_t, operator =, (const string_t&), string_t&), psCALL_THISCALL));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_ASSIGNMENT, PS_T("wstring &f(const wchar_t* s)"),    psFUNCTION(helper_t::AssignTextToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_INDEX,      PS_T("wchar_t& f(int idx)"),            psFUNCTION(helper_t::GetChar), psCALL_CDECL_OBJFIRST));

	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_ADD_ASSIGN, PS_T("wstring &f(const wstring &in)"), psMETHODPR(string_t, operator+=, (const string_t&), string_t&), psCALL_THISCALL));

	// Register the global operator overloads
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_EQUAL,       PS_T("bool f(const wstring &in, const wstring &in)"), psFUNCTIONPR(operator ==, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_NOTEQUAL,    PS_T("bool f(const wstring &in, const wstring &in)"), psFUNCTIONPR(operator !=, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_LEQUAL,      PS_T("bool f(const wstring &in, const wstring &in)"), psFUNCTIONPR(operator <=, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_GEQUAL,      PS_T("bool f(const wstring &in, const wstring &in)"), psFUNCTIONPR(operator >=, (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_LESSTHAN,    PS_T("bool f(const wstring &in, const wstring &in)"), psFUNCTIONPR(operator <,  (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_GREATERTHAN, PS_T("bool f(const wstring &in, const wstring &in)"), psFUNCTIONPR(operator >,  (const string_t &, const string_t &), bool), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("wstring f(const wstring &in, const wstring &in)"), psFUNCTIONPR(operator +, (const string_t &, const string_t &), string_t), psCALL_CDECL));

	// Register the object methods
	PS_VERIFY( engine->RegisterObjectMethod(PS_T("wstring"), PS_T("int length()"),  psFUNCTION(helper_t::GetLength),    psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectMethod(PS_T("wstring"), PS_T("wchar_t* c_str()"), psMETHOD(string_t, c_str), psCALL_THISCALL));

	// Automatic conversion from values
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_ASSIGNMENT, PS_T("wstring &f(float)"), psFUNCTION(helper_t::AssignFloatToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_ADD_ASSIGN, PS_T("wstring &f(float)"), psFUNCTION(helper_t::AddAssignFloatToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("wstring f(const wstring &in, float)"), psFUNCTION(helper_t::AddStringFloat), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("wstring f(float, const wstring &in)"), psFUNCTION(helper_t::AddFloatString), psCALL_CDECL));

	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_ASSIGNMENT, PS_T("wstring &f(int)"), psFUNCTION(helper_t::AssignIntToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_ADD_ASSIGN, PS_T("wstring &f(int)"), psFUNCTION(helper_t::AddAssignIntToString), psCALL_CDECL_OBJFIRST));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("wstring f(const wstring &in, int)"), psFUNCTION(helper_t::AddStringInt), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,         PS_T("wstring f(int, const wstring &in)"), psFUNCTION(helper_t::AddIntString), psCALL_CDECL));

	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,		  PS_T("wstring f(const wchar_t*, const wstring & in)"), psFUNCTION(helper_t::AddTextString), psCALL_CDECL));
	PS_VERIFY( engine->RegisterGlobalBehaviour(psBEHAVE_ADD,		  PS_T("wstring f(const wstring & in, const wchar_t*)"), psFUNCTION(helper_t::AddStringText), psCALL_CDECL));
	PS_VERIFY( engine->RegisterObjectBehaviour(PS_T("wstring"), psBEHAVE_ADD_ASSIGN, PS_T("wstring &f(const wchar_t*)"), psFUNCTION(helper_t::AddAssignTextToString), psCALL_CDECL_OBJFIRST) );
}
