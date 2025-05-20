#include "variablescope.h"
#include "bytecodedef.h"
#include <assert.h>

psCVariableScope::psCVariableScope(psCVariableScope* parent)
{
	m_pParent = parent;
	m_bIsContinueScope = false;
	m_bIsBreakScope = false;
}

psCVariableScope::~psCVariableScope(void)
{
	VariableMap_t::iterator iter = m_VariableMap.begin();
	for( ; iter != m_VariableMap.end(); ++iter)
	{
		if (iter->second)
			delete iter->second;
		iter->second = 0;
	}	
	m_VariableMap.clear();

	for (size_t i = 0; i < m_AnnoymousVariables.size(); ++i )
	{
		delete m_AnnoymousVariables[i];
	}
	m_AnnoymousVariables.clear();
}

psSVariable* psCVariableScope::DeclareVariable(const psCHAR *name, const psSValue& value)
{
	assert(name);
	bool bAnnoymouse = (name[0] == PS_T('\0'));
	if ( !bAnnoymouse )
	{
		// See if the variable is already declared
		if (m_VariableMap.find(name) != m_VariableMap.end())
			return 0;
	}

	psSVariable* var    = new psSVariable();
	var->name		    = name;
	var->value		    = value;

	// NOTE: 这里直接把变量名字符串的指针,当作key, 要确保其有效性
	if ( bAnnoymouse )
	{
		m_AnnoymousVariables.push_back( var );
	}else
	{
		const psCHAR* key	= var->name.AddressOf();
		m_VariableMap[key]  = var;
	}
	return var;
}

psSVariable* psCVariableScope::GetVariable(const psCHAR* name) const
{
	VariableMap_t::const_iterator iter = m_VariableMap.find(name);
	if (iter != m_VariableMap.end())
		return iter->second;

	if (m_pParent)
		return m_pParent->GetVariable(name);

	return 0;
}

psSVariable* psCVariableScope::GetVariableByOffset(int offset) const
{
	VariableMap_t::const_iterator iter = m_VariableMap.begin();
	for (; iter != m_VariableMap.end(); ++iter)
	{
		psSVariable* var = iter->second;
		if ( var && var->value.IsStackVariable() && 
			var->value.operand.offset == offset) 
			return iter->second;
	}

	if (m_pParent)
		return m_pParent->GetVariableByOffset(offset);

	return 0;
}



