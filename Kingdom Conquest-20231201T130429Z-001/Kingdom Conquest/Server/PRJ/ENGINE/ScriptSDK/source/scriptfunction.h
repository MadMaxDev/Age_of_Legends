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
	// �õ�������
	virtual const psCHAR* GetName() const			{ return m_Name.AddressOf(); };
	
	// �õ�Դ�ļ�
	virtual const psCHAR* GetSourceFile() const		{ return m_SourceFileName.AddressOf(); }

	// �õ���������
	virtual const int GetID() const					{ return  m_ID;				  }

	virtual psIBuffer* GetByteCode(int& outsize) const;

	// �õ��������ַ���˵��(NOTE: �������ֵҪ��psDeleteString�ͷŵ�)
	virtual	psAutoStringPtr GetDeclarationString() const;

	// �õ�����ֵ������
	virtual const psIDataType* GetReturnType() const  { return &(m_ReturnType); }

	// �õ�������������������(�����ȫ�ֺ����䷵��ֵΪ��)
	virtual const psIObjectType* GetObjectType() const;

	// �õ���������Ĳ�������
	virtual const unsigned int GetNumParams() const { return (unsigned int)m_ParamTypes.size(); }

	// �õ���������Ĳ�������
	virtual const psIDataType* GetParamType(int index) const { return &(m_ParamTypes[index]); }

	// ���кŵõ����������λ��
	virtual int GetPosFromLineNumber(int lineno, int& realLineNo) const;

	// �Ӷ��������λ�õõ�Դ������к�
	virtual	int GetLineNumber(int pos) const;

	// �õ���һ��Ч������к�(pos�з������ڶ��������е�λ��)
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

	// ����һ���ϵ�
	bool InsertBreakPoint(int lineno);
	bool InsertBreakPointbyPos( int pos ); 

	// �Ƴ�һ���ϵ�
	bool RemoveBreakPoint(int lineno);
	bool RemoveBreakPointByPos(int pos);

	// �л��ϵ�
	bool ToggleBreakPoint(int lineno, bool& hasBreakPoint );

	// �Ƴ����еĶϵ�
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

	int							 m_ID;					 // ������ID
	psCString					 m_Name;				 // ��������
	psCString					 m_SourceFileName;		 // ������Դ�ļ�
	psCDataType					 m_ReturnType;			 // ����ֵ����
	std::vector<psCDataType>     m_ParamTypes;			 // ��������

	psBYTE*						 m_BinaryCode;			 // �����ƴ���
	int							 m_BinaryCodeSize;

	int*						 m_LineNumbers;			 // �к�	
	int							 m_NumLines;

	psSDebugSymbol*				 m_DebugSymbols;
	int							 m_NumDebugSymbols;

	psCObjectType *				 m_ObjectType;			 // ��������
	int							 m_funcFlag;			 // �������Ա�־
	int							 m_StackNeeded;			 // ��Ҫ�Ķ�ջ��С
	int							 m_IsIncluded;			 // ��������Ƿ���ͷ�ļ���
	struct BREAK_POINT
	{
		int	 pos;
		int  bc;
	};
	std::vector<BREAK_POINT>	 m_BreakPoints;

};

#endif // __PS_SCRIPTFUNCTION_H__
