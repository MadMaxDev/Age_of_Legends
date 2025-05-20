#pragma once

class BinStream
{
public:

	FILE* m_pFile;
	
	BinStream( const wchar_t* _Path , const wchar_t* _Mode );
	
	~BinStream();
		
	bool IsOK() { return m_pFile != NULL; }
	int Size();
	void Read( void* _pBuffer , int _Size );
	void Skip( int _Size , int _Mode  = SEEK_CUR );
	void Write( const void* _pBuffer , int _Size );
};

template< typename T > inline void operator << ( BinStream& _Stm , const T& _Val )
{
	_Stm.Write( &_Val , sizeof( T ) );
}

template< typename T > inline void operator >> ( BinStream& _Stm , T& _Val )
{
	_Stm.Read( &_Val , sizeof( T ) );
}

inline void operator << ( BinStream& _Stm , const std::wstring& _v )
{	
	unsigned long len = unsigned long( _v.size() );
	_Stm << len;
	_Stm.Write( _v.c_str() , len * sizeof( wchar_t ) );
}

inline void operator >> ( BinStream& _Stm , std::wstring& _v )
{	
	unsigned long len;
	_Stm >> len;
	wchar_t* str = (wchar_t* )_alloca( (len+1 )* sizeof( wchar_t ));
	str[len] = L'\0';
	_Stm.Read( str , len * sizeof( wchar_t ) );
	_v = str;
}

inline void operator << ( BinStream& _Stm , const std::string& _v )
{	
	unsigned long len = unsigned long( _v.size() );
	_Stm << len;
	_Stm.Write( _v.c_str() , len );
}

inline void operator >> ( BinStream& _Stm , std::string& _v )
{	
	unsigned long len;
	_Stm >> len;
	char* str = (char* )_alloca(len+1);
	str[len] = '\0';
	_Stm.Read( str , len );
	_v = str;
}

template< typename T > 
inline void operator >> ( BinStream& _Stm , std::vector<T>& _v )
{
	unsigned long len;
	_Stm >> len;
	_v.reserve(len);
	_v.resize(len);
	for(DWORD t = 0; t < len; ++t)
	{
		_Stm >> _v[t];
	}
}

template< typename T > 
inline void operator << ( BinStream& _Stm , const std::vector<T>& _v )
{
	unsigned long len = (unsigned long)_v.size();
	_Stm<<len;
	for(unsigned long t = 0;t<len;++t)
	{
		_Stm<<_v[t];
	}
}

