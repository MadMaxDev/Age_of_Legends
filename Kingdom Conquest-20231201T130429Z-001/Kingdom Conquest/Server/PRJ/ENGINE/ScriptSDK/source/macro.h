#ifndef __PS_MACRO_H__
#define __PS_MACRO_H__

#include "config.h"

//-------------------------------------------------------------------
// �ṹ��: psSMacro
//------------------------------------------------------------------
struct psSMacro
{
	void add_param(const psCString& rhs )
	{
		params.push_back( rhs );
	}
	size_t param_size() const 
	{ 
		return params.size(); 
	}
	const psCString& get_param(size_t i) const
	{
		assert( i < params.size() );
		return (params)[i];
	}
	psCString				text;		// �������
	std::vector<psCString>  params;		// ��Ĳ���
};	

//-------------------------------------------------------------------
// ���Ͷ���
//-------------------------------------------------------------------
typedef std::map<psCString, psSMacro> MacroListMap_t;

#endif // __PS_MACRO_H__
