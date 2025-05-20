#include "objecttype.h"

//////////////////////////////////////////////////////////////////////////
// class psCObjectType
psCObjectType::psCObjectType(void)
{
	m_bScriptObject = false;
	m_pParent		= NULL;
	m_Size			= 0;
	m_BaseOffset	= 0;
	m_TokenType		= ttIdentifier;

	m_DataType.SetObjectType(this);
	m_DataType.IsReadOnly() = true;
	m_DataType.TokenType()  = ttIdentifier;
}

psCObjectType::~psCObjectType(void)
{
	for( size_t n = 0; n < m_Properties.size(); n++ )
		if( m_Properties[n] ) delete m_Properties[n];
}

psSProperty* psCObjectType::FindProperty(const psCHAR* name) const
{
	for( size_t n = 0; n < m_Properties.size(); n++ )
	{
		if (m_Properties[n]->name == name)
			return m_Properties[n];
	}
	return 0;
}

int psCObjectType::GetBaseOffset( const psCObjectType* objType ) const
{
	int baseOffset = 0;

	const psCObjectType* testType = this;
	while ( testType )
	{
		if ( testType == objType )
		{
			assert( baseOffset >= 0 );				
			return baseOffset;
		}
		baseOffset += testType->m_BaseOffset;
		testType = testType->m_pParent;
	}

	return -1;
}
