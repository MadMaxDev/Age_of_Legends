#ifndef __PS_VARIABLESCOPE_H__
#define __PS_VARIABLESCOPE_H__

#include "datatype.h"
#include "string.h"
#include "operand.h"
#include <map>

struct psSVariable
{
	psCString name;
	psSValue  value;
};

class psCVariableScope
{
public:
	psCVariableScope(psCVariableScope* parent);
	~psCVariableScope(void);

	psSVariable* DeclareVariable(const psCHAR *name, const psSValue& operand);
	psSVariable* GetVariable(const psCHAR *name) const;
	psSVariable* GetVariableByOffset(int offset) const;

	bool& IsBreakScope()    { return m_bIsBreakScope;	 }
	bool& IsContinueScope() { return m_bIsContinueScope; }

	void SetParent(psCVariableScope* parent) { m_pParent = parent;   }
	psCVariableScope* GetParent()  const	 { return m_pParent;	 }

	struct strcmpor
	{
		bool operator ()( const psCHAR* lhs,  const psCHAR* rhs) const
		{
			return ps_strcmp( lhs, rhs) < 0 ;
		}
	};
	typedef std::map< const psCHAR*, psSVariable*, strcmpor > VariableMap_t;

	const VariableMap_t& GetVariableMap() const { return m_VariableMap; }
private:
	psCVariableScope* m_pParent;

	bool m_bIsContinueScope;
	bool m_bIsBreakScope;

	VariableMap_t			    m_VariableMap;
	std::vector< psSVariable* > m_AnnoymousVariables;
};

#endif // __PS_VARIABLESCOPE_H__
