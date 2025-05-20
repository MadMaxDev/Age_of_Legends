#include "datatype.h"
#include "objecttype.h"

psCDataType::psCDataType(void)
{
	m_TokenType	   = ttUnrecognizedToken;
	m_bIsReadOnly  = false;
	m_bIsPointer = false;
	m_bIsReference = false;
	m_pObjectType  = NULL;
	m_ArrayDim	   = 0;
}

psCDataType::psCDataType(psETokenType type, bool bConst, bool bRef)
{
	m_TokenType	    = type;
	m_bIsReadOnly   = bConst;
	m_bIsPointer  = bRef;
	m_bIsReference = bRef;
	m_pObjectType   = NULL;
	m_ArrayDim	    = 0;
}

bool psCDataType::operator < (const psCDataType &dt) const
{
	if ( IsSameBaseType(dt) )
		return false;

	if (m_TokenType == ttFloat && dt.m_TokenType == ttDouble)
	{
		return true;
	}
	else if (IsIntegerType())
	{
		if (dt.IsFloatType())
			return true;

		return GetSizeInMemoryBytes() < dt.GetSizeInMemoryBytes();
	}

	return false;
}

bool psCDataType::operator > (const psCDataType &dt) const
{
	if (*this < dt) 
		return false;

	if (IsSameBaseType(dt))
		return false;
	else 
		return true;
}

bool psCDataType::operator !=(const psCDataType &dt) const
{
	return !(*this == dt);
}

bool psCDataType::operator == (const psCDataType &dt) const
{
	if ( m_TokenType    != dt.m_TokenType )    return false;
	if ( m_pObjectType  != dt.m_pObjectType )  return false;
	if ( m_bIsPointer   != dt.m_bIsPointer )   return false;
	if ( m_bIsReference	!= dt.m_bIsReference ) return false;
	if ( m_bIsReadOnly  != dt.m_bIsReadOnly )  return false;
	if ( !IsSameArrayDim(dt) )				   return false;

	return true;
}

bool psCDataType::IsEqualExceptRef(const psCDataType &dt) const
{
	if( m_TokenType   != dt.m_TokenType ) return false;
	if( m_pObjectType != dt.m_pObjectType ) return false;
	if( m_bIsReadOnly != dt.m_bIsReadOnly ) return false;
	if( !IsSameArrayDim(dt) ) return false;

	return true;
}

bool psCDataType::IsEqualExceptRefAndConst(const psCDataType &dt) const
{
	if( m_TokenType   != dt.m_TokenType ) return false;
	if( m_pObjectType != dt.m_pObjectType ) return false;
	if( !IsSameArrayDim(dt) ) return false;

	return true;
}

bool psCDataType::IsEqualExceptConst(const psCDataType &dt) const
{	
	if( m_TokenType    != dt.m_TokenType )	   return false;
	if( m_pObjectType  != dt.m_pObjectType )   return false;
	if( m_bIsPointer   != dt.m_bIsPointer )	   return false;
	if( !IsSameArrayDim(dt) ) return false;

	return true;
}

bool psCDataType::IsPrimitive() const
{
	// A registered object is never a primitive neither is a pointer, nor an array
	if( m_TokenType == ttIdentifier || m_pObjectType || IsArrayType() )
	{
		return false;
	}

	return true;
}

bool psCDataType::IsSameBaseType(const psCDataType &dt) const
{
	if (m_pObjectType != dt.m_pObjectType) return false;
	if ( !IsSameArrayDim(dt) ) return false;

	if( IsIntegerType() && dt.IsIntegerType() ) return true;
	if( IsFloatType() && dt.IsFloatType() ) return true;
	if( IsDoubleType() && dt.IsDoubleType() ) return true;
	if (m_TokenType == dt.m_TokenType ) return true;
	
	return false;
}

bool psCDataType::IsSameTypeExceptSign(const psCDataType& dt) const
{
	return  IsSameBaseType(dt);
}


bool psCDataType::IsSameArrayDim(const psCDataType& rhs) const
{
	if ( m_ArrayDim != rhs.m_ArrayDim )
		return false;

	for ( int i = 0; i < m_ArrayDim; ++i )
	{
		if ( m_ArraySizes[i] != rhs.m_ArraySizes[i] )
		{
			return false;
		}
	}
	return true;
}

bool psCDataType::IsIntegerType() const
{
	return (m_TokenType == ttInt   ||
			m_TokenType == ttInt8  ||
			m_TokenType == ttInt16 ||
			m_TokenType == ttUInt8 ||
			m_TokenType == ttUInt16) ? true : false;
}

bool psCDataType::IsBooleanType() const
{
	return (m_TokenType == ttBool) ? true : false;
}

bool psCDataType::IsFloatType() const
{
	return (m_TokenType == ttFloat) ? true : false;

}

bool psCDataType::IsDoubleType() const
{
	return (m_TokenType == ttDouble) ? true : false;
}

bool psCDataType::PushArraySize( int size )
{
	if ( m_ArrayDim < MAX_ARRAY_DIM )
	{
		m_ArraySizes[m_ArrayDim++] = size;	 
		return true;
	}
	else
	{
		return false;
	}
}

int psCDataType::PopArraySize()
{
	if ( m_ArrayDim > 0)
	{
		int s = m_ArraySizes[0];

		for (int i = 1; i < m_ArrayDim; ++i )
		{
			m_ArraySizes[i - 1] = m_ArraySizes[i];
		}
		--m_ArrayDim;
		return s;
	}else
	{
		return 1;
	}
}

int psCDataType::GetNumArrayElements() const
{
	int c = 1;
	for (int i = 0; i < m_ArrayDim; ++i)
		c *= m_ArraySizes[i];

	return c;
}

const psIObjectType* psCDataType::GetObjectType() const
{
	return m_pObjectType;
}

psSTypeBehaviour* psCDataType::GetBehaviour() const
{
	return (m_pObjectType) ? &(m_pObjectType->m_Beh) : NULL; 
}

int psCDataType::GetSizeInMemoryBytes() const
{
	if( m_pObjectType != 0 )
		return m_pObjectType->m_Size;

	if( m_TokenType == ttVoid )
		return 0;

	if( m_TokenType == ttInt8 ||
		m_TokenType == ttUInt8)
		return sizeof(psBYTE);

	if( m_TokenType == ttInt16 ||
		m_TokenType == ttUInt16)
		return sizeof(psWORD);

	if( m_TokenType == ttDouble)
		return sizeof(double);

	return sizeof(int);
}

int	 psCDataType::GetSizeInMemoryDWords() const
{
	int memsize = GetSizeInMemoryBytes();
	int s		= memsize/sizeof(psDWORD);
	if ( s * (int)sizeof(psDWORD) < memsize)
		++s;
	return s;
}

int psCDataType::GetSizeOnStackDWords() const
{
	if ( m_bIsPointer || m_bIsReference )
		return 1;
	else
		return GetSizeInMemoryDWords();
}

int  psCDataType::GetTotalSizeOnStackDWords()  const
{
	if ( m_bIsPointer || m_bIsReference )
	{
		return 1;
	}
	else
	{
		int memsize = GetSizeInMemoryBytes() * GetNumArrayElements();
		int s		= memsize/sizeof(psDWORD);
		if ( s * (int)sizeof(psDWORD) < memsize)
			++s;
		return s;
	}
}

psCString psCDataType::GetFormat(const psCHAR* varname) const
{
	psCString str;

	if( m_bIsReadOnly )
		str = PS_T("const ");

	if( m_TokenType != ttIdentifier )
	{
		const psCHAR *t = psGetTokenDefinition(m_TokenType);
		if (t)
			str += t;
	}
	else
	{
		if( m_pObjectType == 0 )
			str += PS_T("<unknown>");
		else
			str += m_pObjectType->m_Name;
	}
	if( m_bIsReference)
	{
		if ( m_bIsPointer )
			str += PS_T("*");
		else
			str += PS_T("&");
	}

	if (varname)
	{
		str += PS_T(" ");
		str += varname;
	}

	psCHAR buf[32];
	for (int i = 0; i < m_ArrayDim; ++i)
	{
		ps_snprintf(buf, 32, PS_T("[%d]"), m_ArraySizes[i]);
		str += buf;
	}
	return str;
}

psCString psCDataType::GetFormat() const
{
	return GetFormat(NULL);
}

psAutoStringPtr psCDataType::GetFormatString(const psCHAR* varname) const
{
	psCString strDesc = GetFormat(varname);

	psBuffer* buf = new psBuffer;
	buf->Assign( (psBYTE*)strDesc.RemoveBuffer(), true );
	psAutoStringPtr str(buf);
	return str;
}

