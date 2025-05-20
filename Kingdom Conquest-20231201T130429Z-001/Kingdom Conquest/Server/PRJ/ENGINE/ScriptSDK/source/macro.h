#ifndef __PS_MACRO_H__
#define __PS_MACRO_H__

#include "config.h"

//-------------------------------------------------------------------
// 结构名: psSMacro
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
	psCString				text;		// 宏的名称
	std::vector<psCString>  params;		// 宏的参数
};	

//-------------------------------------------------------------------
// 类型定义
//-------------------------------------------------------------------
typedef std::map<psCString, psSMacro> MacroListMap_t;

#endif // __PS_MACRO_H__
