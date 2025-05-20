#ifndef __PS_OBJECTTYPE_H__
#define __PS_OBJECTTYPE_H__

#if defined(_MSC_VER)
	// disable warning : C4786
	#pragma warning(disable : 4786)
#endif

#include "config.h"
#include "tokendef.h"
#include "string.h"
#include "datatype.h"
#include "memmanager.h"

class psSProperty: public psIProperty
{
public:	
	DECL_MEM_OP(psSProperty)

	// �õ�������
	virtual const psCHAR* GetName() const { return name.AddressOf(); }

	// �õ�����ƫ��
	virtual int GetOffset() const 		{ return byteOffset;}

	// �õ����Ե���������
	virtual const psIDataType* GetDataType() const { return &type; }
public:
	virtual ~psSProperty() {}

	psCString   name;
	psCDataType type;
	union 
	{
		int   byteOffset;
		int   index;
	};
	int		  base;
};

class psSTypeBehaviour: public psITypeBehaviour
{
public:	
	DECL_MEM_OP(psSTypeBehaviour)

	psSTypeBehaviour()
	{
		construct = 0; 
		destruct  = 0;
		copy      = 0; 
		hasConstructor = false; 
		hasDestructor  = false;
		isInterface	   = false; 
		hasCopyConstructor = false;	
	}	
	virtual ~psSTypeBehaviour() {}

	int  construct;
	int  destruct;
	int  copy;

	bool hasConstructor			;
	bool hasDestructor			;
	bool hasCopyConstructor		;
	bool isInterface			;

	std::vector<int> constructors;
	std::vector<int> operators;

	// �õ����캯���ĸ���
	virtual unsigned int GetNumConstructFuncs() const { return (unsigned int)constructors.size(); }

	// �õ����캯����ID
	virtual int GetConstructFuncId(unsigned int index) const { return constructors[index]; }

	// �õ�Ĭ�ϵĹ��캯��ID
	virtual bool HasConstructor() const				 { return hasConstructor;    }
	virtual int  GetConstructFuncId() const			 { return construct;		 }

	//
	virtual bool HasCopyConstructor() const			 { return hasCopyConstructor; }
	virtual int  GetCopyConstructFuncId() const		 { return copy;				  } 

	// �õ�����������ID
	virtual bool HasDestructor() const				 { return hasDestructor;	}
	virtual int  GetDestructFuncId() const			 {  return destruct;		}

	// �õ������صĲ������ĸ���
	virtual unsigned int GetNumOperators() const		     { return (unsigned int)operators.size()/2; }

	// �õ����صĲ������ı�Ƿ�
	virtual int GetOperatorToken(unsigned int index) const   { return operators[index*2]; }

	// �õ����ز������ĺ���ID
	virtual int GetOperatorFuncId(unsigned int index) const  { return operators[index*2+1]; }
};

class psCObjectType: public psIObjectType
{
public:
	DECL_MEM_OP(psCObjectType)

	//----------------------------------------------------------------------//
	// ʵ�ֻ���ĺ���
	//----------------------------------------------------------------------//
	// �õ�����
	virtual const psCHAR* GetName() const  { return m_Name.AddressOf();}

	// �õ���������ڴ��еĴ�С
	virtual int GetSizeInMemoryBytes() const { return m_Size; }

	// �õ��෽���ĸ���
	virtual unsigned int GetNumMethods() const { return (unsigned int)m_Methods.size(); }

	// �õ��෽����ID
	virtual int GetMethodFuncId(unsigned int index) const { return m_Methods[index]; }

	// �õ����Ա���Եĸ���
	virtual unsigned int GetNumProps() const  { return (unsigned int)m_Properties.size(); }

	// �õ����Ա����
	virtual const psIProperty* GetProperty(unsigned int index) const { return m_Properties[index]; }

	// �õ������Ϊ�����ӿ�
	virtual const psITypeBehaviour* GetTypeBehaviour() const { return &m_Beh;	}

	// �Ƿ�Ϊ�ű��������
	virtual bool IsScriptObject() const { return m_bScriptObject; }
	// �õ���������
	virtual psIObjectType* GetParent() const  { return m_pParent; }

	// ���ൽ�����ƫ��
	virtual int GetBaseOffset() const		  { return m_BaseOffset; }

	// �õ���������
	virtual const psIDataType* GetDataType() const { return &m_DataType; }
public:
	psCObjectType(void);
	virtual ~psCObjectType(void);

	psSProperty* FindProperty(const psCHAR* name) const;
	int		     GetBaseOffset( const psCObjectType* objType ) const;

	inline bool  IsSuperOrSelf( const psCObjectType* objType ) const
	{
		return GetBaseOffset( objType ) != -1;
	}

	//----------------------------------------------------------------------//
	// ��Ա���� 
	psCString					 m_Name;					// ����	
	psETokenType				 m_TokenType;				// �������
	int							 m_Size;					// ��Ĵ�С
	int							 m_Flags;					// ��ı�ʶ
	bool						 m_bScriptObject;			// �Ƿ�Ϊ�ű���
	bool						 m_unused[3];				// 
	psCObjectType*				 m_pParent;					// ����
	int							 m_BaseOffset;				// ������ƫ��
	psCDataType					 m_DataType;				// ��������	
	std::vector< psSProperty* >  m_Properties;				// ���Ա����
	std::vector< int >			 m_Methods;					// ��ķ���
	psSTypeBehaviour			 m_Beh;						// �����Ϊ(��������,����/������)
};

#endif // __PS_OBJECTTYPE_H__

