#ifndef __PS_OPERAND_H__
#define __PS_OPERAND_H__

#include "datatype.h"

//-------------------------------------------------------------------
// �ṹ��: psSOperand
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
	int	   base;			// ��ַ��ʽ
	int	   offset;			// ���ƫ��
};

//-------------------------------------------------------------------
// �ṹ��: psSValue
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
	psCDataType  type;						 // ֵ������
	psSOperand   operand;					 // ֵ�Ĳ�����
	bool         isInitialized		;		 // �����Ƿ��Ѿ���ʼ��
	bool		 isTemporary		;		 // �����Ƿ�Ϊһ��ʱ����
	bool		 isNullPointer		;		 // �Ƿ�Ϊһ����ָ��
	bool		 unused				;		 // �Ƿ�Ϊ��ֵ
	int			 startline			;		 // �����Ŀ�ʼ���к�(���ڵ�����)
};

#endif // __PS_OPERAND_H__

