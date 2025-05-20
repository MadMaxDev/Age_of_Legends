#ifndef __PS_STRING_H__
#define __PS_STRING_H__

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <memory.h>
#include <stdarg.h>		// va_list, va_start(), etc
#include <stdlib.h>     // strtod(), strtol()
#include <assert.h>     // assert()
#include "config.h"

// string traits
struct psStringTraitsA
{
	static size_t strlen( const char* str ) 
	{
		return ::strlen( str );
	}
	static int strcmp( const char* s0, const char* s1) 
	{
		return ::strcmp( s0, s1 );
	}
	static int vsnprintf( char* buf, size_t n, const char* fmt, va_list arglist ) 
	{
#ifdef _MSC_VER
		return ::_vsnprintf( buf, n, fmt, arglist );
#else
		return ::vsnprintf( buf, n, fmt, arglist );
#endif
	}
	static const char* nullstring() 
	{
		return "";
	}
};

struct psStringTraitsW
{
	static size_t strlen( const wchar_t* str ) 
	{
		return ::wcslen( str );
	}
	static int strcmp( const wchar_t* s0, const wchar_t* s1) 
	{
		return ::wcscmp( s0, s1 );
	}
	static int vsnprintf( wchar_t* buf, size_t n, const wchar_t* fmt, va_list arglist ) 
	{
		return	ps_vsnwprintf( buf, n, fmt, arglist );
	}
	static const wchar_t* nullstring() 
	{
		return L"";
	}
};

// string template
template <class xchar, class stringtraits >
class psCStringT
{	
public:
	psCStringT()
	{
		m_Length	 = 0;
		m_BufferSize = 1;
		m_pBuffer    = MAlloc<xchar>(1);
		m_pBuffer[0] = 0;
	}
	~psCStringT()
	{
		if( m_pBuffer )
		{
			::free( m_pBuffer );
		}
	}

	psCStringT(const psCStringT &str)
	{
		m_Length	 = str.GetLength();
		m_BufferSize = m_Length + 1;
		m_pBuffer    = MAlloc<xchar>( m_BufferSize );
		memcpy( m_pBuffer, str.m_pBuffer, m_BufferSize*sizeof(xchar) );
	}
	explicit psCStringT(const xchar * str)
	{
		assert( str );
		m_Length	 = stringtraits::strlen(str);
		m_BufferSize = m_Length + 1;
		m_pBuffer    = MAlloc<xchar>( m_BufferSize );

		memcpy( m_pBuffer, str, m_BufferSize * sizeof(xchar) );
	}
	explicit psCStringT(xchar ch)
	{
		m_Length	 = 1;
		m_BufferSize = 2;
		m_pBuffer    = MAlloc<xchar>( m_BufferSize );

		m_pBuffer[0] = ch;
		m_pBuffer[1] = 0;
	}

	void	Allocate(size_t  len, bool keepData)
	{
		xchar *buf = MAlloc<xchar>( len + 1);
		m_BufferSize = len+1;

		if( m_pBuffer )
		{
			if( keepData )
			{
				if( len < m_Length )
					m_Length = len;
				memcpy(buf, m_pBuffer, m_Length * sizeof(xchar) );
			}

			::free( m_pBuffer );
		}

		m_pBuffer = buf;

		// If no earlier data, set to empty string
		if( !keepData )
			m_Length = 0;

		// Make sure the m_pBuffer is null terminated
		m_pBuffer[m_Length] = 0;
	}
	void    SetLength(size_t len)
	{
		if( len >= m_BufferSize )
			Allocate(len, true);

		m_Length	   = len;
		m_pBuffer[len] = 0;
	}
	size_t	GetLength() const { return m_Length; }

	int		Replace(size_t pos, size_t len, const xchar* text)
	{
		assert(text);
		assert (m_pBuffer);

		if (pos + len > m_Length)
			len = m_Length - pos;

		int	   txtLen   = int(stringtraits::strlen(text));
		int	   deltaLen = int(txtLen - len);
		int    newLen	= int(deltaLen + m_Length);
		assert(newLen >= 0);

		xchar* newbuf = NULL;
		if ( newLen > int(m_BufferSize - 1) )
		{
			newbuf = MAlloc<xchar>(newLen+1);

			memcpy( newbuf,       m_pBuffer, pos * sizeof(xchar) );
			memcpy( newbuf + pos, text,	     txtLen * sizeof(xchar) );
			memcpy( newbuf + pos + txtLen,   m_pBuffer + pos + len, (m_Length - pos - len) * sizeof(xchar) );

			// free buffer
			::free( m_pBuffer );

			m_pBuffer    = newbuf;
			m_BufferSize = newLen + 1;
		}
		else
		{
			if ( deltaLen > 0 )
			{
				for ( size_t i = m_Length - 1; i >= pos + len; --i )
				{
					m_pBuffer[i+deltaLen] = m_pBuffer[i];
				}
			}else if ( deltaLen < 0 )
			{
				for ( size_t i = pos + len; i  < m_Length; ++i )
				{
					m_pBuffer[i+deltaLen] = m_pBuffer[i];
				}
			}
			memcpy( m_pBuffer + pos, text, txtLen * sizeof(xchar) );
		}

		m_Length = newLen;
		m_pBuffer[m_Length] = 0;
		return deltaLen;
	}

	void	Fill(size_t pos, size_t len, xchar chr)
	{
		assert ( m_pBuffer );

		size_t endPos = pos + len + 1;

		if (endPos > m_Length)
			endPos = m_Length;

		for (size_t i = pos; i < endPos; ++i)
			m_pBuffer[i] = chr;	
	}

	void	FillExcept(size_t pos, size_t len, xchar chr, xchar filter)
	{
		assert ( m_pBuffer );

		size_t endPos = pos + len + 1;

		if (endPos > m_Length)
			endPos = m_Length;

		for (size_t i = pos; i < endPos; ++i)
		{
			if (m_pBuffer[i] != filter )
				m_pBuffer[i] = chr;	
		}
	}
	void    ReplaceChar(size_t pos, size_t length, xchar lhs, xchar rhs)
	{
		size_t endPos = pos + length;
		if ( endPos > m_Length ) 
			endPos = m_Length;

		for (size_t i = pos; i < endPos; ++i )
		{
			if ( m_pBuffer[i] == lhs )
				m_pBuffer[i] = rhs;
		}
	}

	void	Concatenate(const xchar *str, size_t len)
	{
		assert( str );

		// Allocate memory for the string
		if( m_BufferSize < m_Length + len + 1 )
			Allocate(m_Length + len, true);

		memcpy(&m_pBuffer[m_Length], str, len*sizeof(xchar) );
		m_Length = m_Length + len;
		m_pBuffer[m_Length] = 0;
	}

	size_t	RecalculateLength()
	{	
		if( m_pBuffer == NULL )
		{ 
			Copy(stringtraits::nullstring(), 0);
		}

		m_Length = stringtraits::strlen(m_pBuffer);

		assert(m_Length < m_BufferSize);
		return m_Length;
	}

	void	Format(const xchar *format, ...)
	{
		va_list args;
		va_start(args, format);

		xchar tmp[256];
		int r = stringtraits::vsnprintf(tmp, 255, format, args);

		if( r > 0 )
		{
			Copy(tmp, r);
		}
		else
		{
			size_t cMaxLen = 1024 * 1024 * 100;

			size_t n = 512;
			psCStringT str;					// Use temporary string in case the current m_pBuffer is a parameter
			str.Allocate(n, false);

			while( (r = stringtraits::vsnprintf(str.m_pBuffer, n, format, args)) < 0 && n < cMaxLen )
			{
				n *= 2;
				str.Allocate(n, false);
			}

			Copy(str.m_pBuffer, r);
		}

		va_end(args);
	}

	void	Copy(const xchar *str, size_t len)
	{
		// Allocate memory for the string
		if( m_BufferSize < len + 1 )
			Allocate(len, false);

		// Copy the string
		m_Length = len;
		memcpy(m_pBuffer, str, m_Length*sizeof(xchar) );
		m_pBuffer[m_Length] = 0;
	}

	int		Compare(const xchar *str) const
	{	
		assert( str );
		if( m_pBuffer == NULL )
		{
			return stringtraits::strcmp(stringtraits::nullstring(), str); 
		}

		return stringtraits::strcmp(m_pBuffer, str);
	}

	psCStringT SubString(size_t start, size_t length) const
	{
		if( start >= GetLength() || length == 0 )
		{
			return psCStringT( stringtraits::nullstring() );
		}

		psCStringT tmp;
		tmp.Copy(m_pBuffer + start, length);
		return tmp;
	}

	xchar* RemoveBuffer()
	{
		xchar* buf = m_pBuffer;

		m_BufferSize = 1;
		m_Length     = 0;
		m_pBuffer    = MAlloc<xchar>( m_BufferSize );
		m_pBuffer[0] = 0;
		return buf;
	}

	inline xchar &operator [](size_t n)			{ assert( n < m_Length); return m_pBuffer[n]; }
	inline xchar  operator [](size_t n) const	{ assert( n < m_Length); return m_pBuffer[n]; }

	inline xchar* AddressOf() const				{ return m_pBuffer; }

	inline psCStringT &operator +=(const psCStringT &str)
	{
		Concatenate(str.m_pBuffer, str.m_Length);
		return *this;

	}
	inline psCStringT &operator += (const xchar * str)
	{
		assert( str );
		Concatenate(str, stringtraits::strlen(str));
		return *this;
	}

	inline psCStringT &operator += (xchar ch)
	{
		Concatenate(&ch, 1);

		return *this;
	}

	inline psCStringT &operator = (const psCStringT &str)
	{
		Copy(str.m_pBuffer, str.m_Length);

		return *this;
	}
	inline psCStringT &operator = (const xchar *str )
	{
		if ( str )
		{
			Copy( str, stringtraits::strlen(str) );
		}
		return *this;
	}
	inline psCStringT &operator = (xchar ch)
	{
		Copy(&ch, 1);

		return *this;
	}

	inline bool operator == ( const xchar* str ) const
	{
		return Compare(str) == 0;
	}
	inline bool operator == ( const psCStringT& rhs ) const
	{
		return Compare(rhs.AddressOf()) == 0;
	}

	inline bool operator != ( const xchar* str) const
	{
		return Compare( str ) != 0;
	}
	inline bool operator != ( const psCStringT& rhs) const
	{
		return Compare(rhs.AddressOf()) != 0;
	}

	inline bool operator < ( const xchar* str ) const
	{
		return Compare( str ) < 0;
	}
	inline bool operator < ( const psCStringT& rhs) const
	{
		return Compare( rhs.AddressOf() ) < 0;
	}

	inline psCStringT operator + ( const xchar* str ) const
	{
		psCStringT res( *this );
		res += str;
		return res;
	}
	inline psCStringT operator + (const psCStringT& rhs ) const
	{
		psCStringT res( *this );
		res += rhs;
		return res;
	}
protected:
	size_t	 m_Length;
	size_t	 m_BufferSize;
	xchar	*m_pBuffer;
};

// Type define
typedef psCStringT< char,	 psStringTraitsA > psCStringA;
typedef psCStringT< wchar_t, psStringTraitsW > psCStringW;

#ifdef PS_UNICODE
	typedef psCStringW psCString;
#else
	typedef psCStringA psCString;
#endif

#endif // __PS_STRING_H__
