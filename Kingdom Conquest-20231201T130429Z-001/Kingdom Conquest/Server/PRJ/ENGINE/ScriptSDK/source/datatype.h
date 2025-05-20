#ifndef __PS_DATATYPE_H__
#define __PS_DATATYPE_H__

#include "config.h"
#include "tokenizer.h"
#include "string.h"

class psCDataType: public psIDataType
{
	//----------------------------------------------------------------------//
	// ʵ�ֻ���Ľӿ�
	//----------------------------------------------------------------------//
public:
	// �õ�����������͵���������ͽӿ�
	virtual const psIObjectType* GetObjectType() const;

	// �õ�������͵��ַ���˵��
	virtual psAutoStringPtr GetFormatString(const psCHAR* varname) const;

	// �Ƿ�Ϊ��������
	virtual bool IsPointer() const					 { return m_bIsPointer; }

	// �Ƿ���Ҫ������
	virtual bool IsReference() const				 { return m_bIsReference; }

	// �Ƿ�Ϊֻ����
	virtual bool IsReadOnly() const { return m_bIsReadOnly; }

	// �Ƿ�Ϊ������������
	virtual bool IsPrimitive() const;

	// �Ƿ�Ϊ���������
	virtual bool IsObject() const { return (m_pObjectType != NULL); }

	// �Ƿ�Ϊ����
	virtual bool IsIntegerType() const;

	// �Ƿ�Ϊ������
	virtual bool IsFloatType() const;

	// �Ƿ�Ϊ������(˫����)
	virtual bool IsDoubleType() const;

	// �Ƿ�Ϊ'bool'��
	virtual bool IsBooleanType() const;

	// �Ƿ�Ϊ��������
	virtual bool IsArrayType() const		 {	return (m_ArrayDim > 0);	 }

	// �õ����ݵ�ά��
	virtual int  GetArrayDim() const		 {  return m_ArrayDim;			 }

	// �õ�ĳһά����Ĵ�С
	virtual int  GetArraySize(int dim) const { return m_ArraySizes[dim];	 }

	// �õ������ڴ���ռ�õ��ֽ���
	virtual int  GetSizeInMemoryBytes()  const;

	virtual int	 GetSizeInMemoryDWords() const;

	int GetSizeOnStackDWords() const;

	int GetTotalSizeOnStackDWords() const;

	
public:
	//----------------------------------------------------------------------//
	// �ڲ�����
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
