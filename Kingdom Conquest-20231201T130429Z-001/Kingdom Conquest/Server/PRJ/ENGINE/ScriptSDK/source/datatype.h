#ifndef __PS_DATATYPE_H__
#define __PS_DATATYPE_H__

#include "config.h"
#include "tokenizer.h"
#include "string.h"

class psCDataType: public psIDataType
{
	//----------------------------------------------------------------------//
	// 实现基类的接口
	//----------------------------------------------------------------------//
public:
	// 得到这个数据类型的类对象类型接口
	virtual const psIObjectType* GetObjectType() const;

	// 得到这个类型的字符串说明
	virtual psAutoStringPtr GetFormatString(const psCHAR* varname) const;

	// 是否为引用类型
	virtual bool IsPointer() const					 { return m_bIsPointer; }

	// 是否需要解引用
	virtual bool IsReference() const				 { return m_bIsReference; }

	// 是否为只读的
	virtual bool IsReadOnly() const { return m_bIsReadOnly; }

	// 是否为基础数据类型
	virtual bool IsPrimitive() const;

	// 是否为类对象类型
	virtual bool IsObject() const { return (m_pObjectType != NULL); }

	// 是否为整型
	virtual bool IsIntegerType() const;

	// 是否为浮点型
	virtual bool IsFloatType() const;

	// 是否为浮点型(双精度)
	virtual bool IsDoubleType() const;

	// 是否为'bool'型
	virtual bool IsBooleanType() const;

	// 是否为数组类型
	virtual bool IsArrayType() const		 {	return (m_ArrayDim > 0);	 }

	// 得到数据的维数
	virtual int  GetArrayDim() const		 {  return m_ArrayDim;			 }

	// 得到某一维数组的大小
	virtual int  GetArraySize(int dim) const { return m_ArraySizes[dim];	 }

	// 得到其在内存中占用的字节数
	virtual int  GetSizeInMemoryBytes()  const;

	virtual int	 GetSizeInMemoryDWords() const;

	int GetSizeOnStackDWords() const;

	int GetTotalSizeOnStackDWords() const;

	
public:
	//----------------------------------------------------------------------//
	// 内部函数
	//----------------------------------------------------------------------//
	psCDataType(void);
	psCDataType(psETokenType type, bool bConst, bool bRef);
	virtual ~psCDataType() {}

	bool& IsReadOnly()				{ return m_bIsReadOnly;  }
	bool& IsPointer()				{ return m_bIsPointer; }
	bool& IsReference()				{ return m_bIsReference; }

	bool IsUnsignedInteger() const	{ return m_TokenType == ttUInt8 || m_TokenType == ttUInt16; }

	psETokenType& TokenType()		{ return m_TokenType; }
	psETokenType  TokenType() const { return m_TokenType; }

	bool IsSameBaseType(const psCDataType &dt) const;
	bool IsEqualExceptRef(const psCDataType &) const;
	bool IsEqualExceptRefAndConst(const psCDataType &) const;
	bool IsEqualExceptConst(const psCDataType &) const;
	bool IsSameTypeExceptSign(const psCDataType& dt) const;
	bool IsSameArrayDim(const std::vector<int>& arraySizes) const;
	bool IsSameArrayDim(const psCDataType& rhs) const;
	int  GetNumArrayElements()	const;

	psSTypeBehaviour* GetBehaviour() const;
	psCObjectType*	  ObjectType()   const				 { return m_pObjectType; }
	void			  SetObjectType(psCObjectType* type) { m_pObjectType = type; }
	bool			  PushArraySize(int size);
	int				  PopArraySize();

	psCString GetFormat() const;
	psCString GetFormat(const psCHAR* varname) const;

	bool operator ==(const psCDataType &) const;
	bool operator !=(const psCDataType &) const;
	bool operator < (const psCDataType &) const;
	bool operator > (const psCDataType &) const;

	psETokenType     m_TokenType;
	psCObjectType*   m_pObjectType;
	int				 m_ArraySizes[MAX_ARRAY_DIM];
	int				 m_ArrayDim;
	bool		     m_bIsReadOnly;
	bool		     m_bIsPointer;
	bool			 m_bIsReference;
};

#endif // __PS_DATATYPE_H__
