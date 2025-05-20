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

	// 得到属性名
	virtual const psCHAR* GetName() const { return name.AddressOf(); }

	// 得到属性偏移
	virtual int GetOffset() const 		{ return byteOffset;}

	// 得到属性的数据类型
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

	// 得到构造函数的个数
	virtual unsigned int GetNumConstructFuncs() const { return (unsigned int)constructors.size(); }

	// 得到构造函数的ID
	virtual int GetConstructFuncId(unsigned int index) const { return constructors[index]; }

	// 得到默认的构造函数ID
	virtual bool HasConstructor() const				 { return hasConstructor;    }
	virtual int  GetConstructFuncId() const			 { return construct;		 }

	//
	virtual bool HasCopyConstructor() const			 { return hasCopyConstructor; }
	virtual int  GetCopyConstructFuncId() const		 { return copy;				  } 

	// 得到析构函数的ID
	virtual bool HasDestructor() const				 { return hasDestructor;	}
	virtual int  GetDestructFuncId() const			 {  return destruct;		}

	// 得到初重载的操作符的个数
	virtual unsigned int GetNumOperators() const		     { return (unsigned int)operators.size()/2; }

	// 得到重载的操作符的标记符
	virtual int GetOperatorToken(unsigned int index) const   { return operators[index*2]; }

	// 得到重载操作符的函数ID
	virtual int GetOperatorFuncId(unsigned int index) const  { return operators[index*2+1]; }
};

class psCObjectType: public psIObjectType
{
public:
	DECL_MEM_OP(psCObjectType)

	//----------------------------------------------------------------------//
	// 实现基类的函数
	//----------------------------------------------------------------------//
	// 得到类名
	virtual const psCHAR* GetName() const  { return m_Name.AddressOf();}

	// 得到这个类在内存中的大小
	virtual int GetSizeInMemoryBytes() const { return m_Size; }

	// 得到类方法的个数
	virtual unsigned int GetNumMethods() const { return (unsigned int)m_Methods.size(); }

	// 得到类方法的ID
	virtual int GetMethodFuncId(unsigned int index) const { return m_Methods[index]; }

	// 得到类成员属性的个数
	virtual unsigned int GetNumProps() const  { return (unsigned int)m_Properties.size(); }

	// 得到类成员属性
	virtual const psIProperty* GetProperty(unsigned int index) const { return m_Properties[index]; }

	// 得到类的行为描述接口
	virtual const psITypeBehaviour* GetTypeBehaviour() const { return &m_Beh;	}

	// 是否为脚本的类对象
	virtual bool IsScriptObject() const { return m_bScriptObject; }
	// 得到父类类型
	virtual psIObjectType* GetParent() const  { return m_pParent; }

	// 父类到子类的偏移
	virtual int GetBaseOffset() const		  { return m_BaseOffset; }

	// 得到数据类型
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
	// 成员变量 
	psCString					 m_Name;					// 类名	
	psETokenType				 m_TokenType;				// 标记类型
	int							 m_Size;					// 类的大小
	int							 m_Flags;					// 类的标识
	bool						 m_bScriptObject;			// 是否为脚本类
	bool						 m_unused[3];				// 
	psCObjectType*				 m_pParent;					// 父类
	int							 m_BaseOffset;				// 与基类的偏移
	psCDataType					 m_DataType;				// 数据类型	
	std::vector< psSProperty* >  m_Properties;				// 类成员属性
	std::vector< int >			 m_Methods;					// 类的方法
	psSTypeBehaviour			 m_Beh;						// 类的行为(操作重载,构造/析构等)
};

#endif // __PS_OBJECTTYPE_H__

