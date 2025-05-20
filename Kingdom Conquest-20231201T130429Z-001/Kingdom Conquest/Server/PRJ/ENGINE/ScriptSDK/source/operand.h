#ifndef __PS_OPERAND_H__
#define __PS_OPERAND_H__

#include "datatype.h"

//-------------------------------------------------------------------
// 结构名: psSOperand
//-------------------------------------------------------------------
struct psSOperand
{
	psSOperand()
	{
		base = PSBASE_UNKNOWN;
		offset = 0;
	}
	psSOperand(int _offset)
	{
		base = PSBASE_NONE;
		offset = _offset;
	}
	void BaseOffset(int _base, int _offset)
	{
		base = _base;
		offset = _offset;
	}
	psSOperand(int _base, int _offset)
	{
		base = _base;
		offset = _offset;
	}
	psSOperand(const psSOperand& v)
	{
		base = v.base;
		offset = v.offset;
	}
	bool operator == (const psSOperand& v) const
	{
		if (v.base == base && v.offset == offset)
			return true;
		else
			return false;
	}
	bool operator != (const psSOperand& v) const
	{
		return !(*this == v);
	}
	int	   base;			// 基址方式
	int	   offset;			// 相对偏移
};

//-------------------------------------------------------------------
// 结构名: psSValue
//-------------------------------------------------------------------
class psSValue
{
public:
	psSValue()
	{
		isInitialized = false;
		isTemporary   = false;
		isNullPointer = false;
		startline	  = -1;
	}
	psSValue(const psCDataType& _type): type(_type)
	{
		isInitialized = false;
		isTemporary   = false;
		isNullPointer = false;
		startline	  = -1;
	}
	psSValue(const psSValue& rhs)
	{
		type		  = rhs.type;
		operand		  = rhs.operand;
		isInitialized = rhs.isInitialized;
		isTemporary   = rhs.isTemporary;
		isNullPointer = rhs.isNullPointer;
		startline	  = rhs.startline;
	}
	void SetDefaultValue(const psCDataType& _type)
	{
		type			   = _type;
		isInitialized	   = false;
		isTemporary		   = false;
		isNullPointer	   = false;
		startline		   = -1;
	}
	void AssignExceptType( const psSValue& rhs )
	{
		isInitialized	   = rhs.isInitialized;
		isTemporary		   = rhs.isTemporary;
		isNullPointer	   = rhs.isNullPointer;
		startline		   = rhs.startline;
		operand			   = rhs.operand;
	}	

	void StackValue(const psCDataType& _type, int offset)
	{
		type = _type;
		if (offset >= 0)
			isInitialized = true;
		else
			isInitialized = false;
		operand.base = PSBASE_STACK;
		operand.offset = offset;
	}
	bool IsConstant() const
	{
		return (operand.base == PSBASE_CONST);
	}
	bool IsStackVariable() const
	{
		return (operand.base == PSBASE_STACK);
	}
	psCDataType  type;						 // 值的类型
	psSOperand   operand;					 // 值的操作数
	bool         isInitialized		;		 // 变量是否已经初始化
	bool		 isTemporary		;		 // 变量是否为一临时变量
	bool		 isNullPointer		;		 // 是否为一个空指针
	bool		 unused				;		 // 是否为左值
	int			 startline			;		 // 变量的开始的行号(用于调试器)
};

#endif // __PS_OPERAND_H__

