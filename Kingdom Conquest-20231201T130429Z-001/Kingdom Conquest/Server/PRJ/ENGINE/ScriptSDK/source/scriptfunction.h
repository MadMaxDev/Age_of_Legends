#ifndef __PS_SCRIPTFUNCTION_H__
#define __PS_SCRIPTFUNCTION_H__

#include "config.h"
#include "debugsymbol.h"
#include "datatype.h"
#include "string.h"
#include "memmanager.h"

//-------------------------------------------------------------//
class psCScriptFunction: public psIScriptFunction
{

public:	
	DECL_MEM_OP(psCScriptFunction)

	psCScriptFunction(void);
	virtual ~psCScriptFunction(void);

public:
	// 得到函数名
	virtual const psCHAR* GetName() const			{ return m_Name.AddressOf(); };
	
	// 得到源文件
	virtual const psCHAR* GetSourceFile() const		{ return m_SourceFileName.AddressOf(); }

	// 得到函数索引
	virtual const int GetID() const					{ return  m_ID;				  }

	virtual psIBuffer* GetByteCode(int& outsize) const;

	// 得到函数的字符串说明(NOTE: 这个返回值要用psDeleteString释放掉)
	virtual	psAutoStringPtr GetDeclarationString() const;

	// 得到返回值的类型
	virtual const psIDataType* GetReturnType() const  { return &(m_ReturnType); }

	// 得到这个函数的类对象类型(如果是全局函数其返回值为空)
	virtual const psIObjectType* GetObjectType() const;

	// 得到这个函数的参数个数
	virtual const unsigned int GetNumParams() const { return (unsigned int)m_ParamTypes.size(); }

	// 得到这个函数的参数类型
	virtual const psIDataType* GetParamType(int index) const { return &(m_ParamTypes[index]); }

	// 从行号得到二进制码的位置
	virtual int GetPosFromLineNumber(int lineno, int& realLineNo) const;

	// 从二进制码的位置得到源代码的行号
	virtual	int GetLineNumber(int pos) const;

	// 得到下一有效代码的行号(pos中返回其在二进制码中的位置)
	virtual int GetNextLineNumber(int lineno, int *pos) const;

	int GetMinLineNumber() const
	{
		if (m_NumLines < 2) return -1;
		return m_LineNumbers[1];
	}
	int GetMinPos()
	{
		if (m_NumLines < 2) return -1;
		return m_LineNumbers[0];
	}

	int GetMaxLineNumber() const
	{
		if (m_NumLines < 2) return -1;
		return m_LineNumbers[m_NumLines - 1];
	}

	int GetMaxLinePos() const 
	{
		if (m_NumLines < 2) return -1;
		return m_LineNumbers[m_NumLines - 2];
	}

	// 插入一个断点
	bool InsertBreakPoint(int lineno);
	bool InsertBreakPointbyPos( int pos ); 

	// 移除一个断点
	bool RemoveBreakPoint(int lineno);
	bool RemoveBreakPointByPos(int pos);

	// 切换断点
	bool ToggleBreakPoint(int lineno, bool& hasBreakPoint );

	// 移除所有的断点
	void RemoveAllBreakPoints();
public:
	enum {CONSTRUCTOR = 1, DESTRUCTOR = 2, HAS_SOURCE = 4, IS_ARG_VARIATIONAL = 8};

	bool IsArgVariational() const { return (m_funcFlag & IS_ARG_VARIATIONAL) ? true : false ; }
	void SetArgVariational(bool bArgVariational) 
	{ 
		if ( bArgVariational)
			m_funcFlag |= IS_ARG_VARIATIONAL;
		else
			m_funcFlag &= ~IS_ARG_VARIATIONAL;
	}
	void SetSourceFile( const psCString& filename )
	{
		m_SourceFileName = filename;
	}
	void SetHasSource( bool bHas ) 
	{
		if ( bHas )
			m_funcFlag |= HAS_SOURCE; 
		else
			m_funcFlag &= ~HAS_SOURCE;
	}
	bool HasSource() const		   { return (m_funcFlag & HAS_SOURCE) ? true : false; }

	void SetConstructor(bool bIs ) 
	{
		if ( bIs )
			m_funcFlag |= CONSTRUCTOR;
		else
			m_funcFlag &= ~CONSTRUCTOR;
	}
	void SetDestructor(bool bIs)
	{
		if ( bIs )
			m_funcFlag |= DESTRUCTOR;
		else
			m_funcFlag &= ~DESTRUCTOR;
	}
	int GetArgumentsSpaceOnStack();
	int GetSpaceNeededForArguments();
	int GetSpaceNeededForReturnValue();

	psCString GetDeclaration(bool bAddClassName = false) const;

	void SetCode( psCByteCode& code);
	void SetDebugSymbols( const std::vector<psSDebugSymbol>& debugSymbols );

	void Clear();

	int							 m_ID;					 // 函数的ID
	psCString					 m_Name;				 // 函数名称
	psCString					 m_SourceFileName;		 // 函数的源文件
	psCDataType					 m_ReturnType;			 // 返回值类型
	std::vector<psCDataType>     m_ParamTypes;			 // 参数类型

	psBYTE*						 m_BinaryCode;			 // 二进制代码
	int							 m_BinaryCodeSize;

	int*						 m_LineNumbers;			 // 行号	
	int							 m_NumLines;

	psSDebugSymbol*				 m_DebugSymbols;
	int							 m_NumDebugSymbols;

	psCObjectType *				 m_ObjectType;			 // 对象类型
	int							 m_funcFlag;			 // 函数属性标志
	int							 m_StackNeeded;			 // 需要的堆栈大小
	int							 m_IsIncluded;			 // 这个函数是否在头文件内
	struct BREAK_POINT
	{
		int	 pos;
		int  bc;
	};
	std::vector<BREAK_POINT>	 m_BreakPoints;

};

#endif // __PS_SCRIPTFUNCTION_H__
