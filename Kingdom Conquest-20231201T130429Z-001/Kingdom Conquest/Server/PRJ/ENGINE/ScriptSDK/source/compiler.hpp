#include "compiler.h"
#include "builder.h"
#include "variablescope.h"
#include "scriptnode.h"
#include "scriptcode.h"
#include "scriptfunction.h"
#include "texts.h"

//-------------------------------------------------------------------
// 类名: 内联函数
// 说明: 
//-------------------------------------------------------------------
inline void psCCompiler::InstrMOV(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type)
{
	int s = type.GetSizeInMemoryBytes();
	if ( s == 1 )
	{
		bc->Instr2( BC_MOV1, op1, op2);
	}else if ( s == 2) 
	{
		bc->Instr2( BC_MOV2, op1, op2);
	}else if ( s == 4 )
	{
		bc->Instr2( BC_MOV4, op1, op2 );
	}else if ( s == 8 )
	{
		bc->Instr2( BC_MOV8, op1, op2 );
	}else
	{
		assert(m_bHasCompileError);
	}
}

inline void psCCompiler::InstrRDREF(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type)
{
	int s = type.GetSizeInMemoryBytes();
	if ( s == 1 )
	{
		bc->Instr2( BC_RDREF1, op1, op2);
	}else if ( s == 2) 
	{
		bc->Instr2( BC_RDREF2, op1, op2);
	}else if ( s == 4 )
	{
		bc->Instr2( BC_RDREF4, op1, op2 );
	}else if ( s == 8 )
	{
		bc->Instr2( BC_RDREF8, op1, op2 );
	}else
	{
		assert(m_bHasCompileError);
	}
}

inline void psCCompiler::InstrWTREF(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type)
{
	int s = type.GetSizeInMemoryBytes();

	if ( s == 1 )
	{
		bc->Instr2( BC_WTREF1, op1, op2);
	}else if ( s == 2) 
	{
		bc->Instr2( BC_WTREF2, op1, op2);
	}else if ( s == 4 )
	{
		bc->Instr2( BC_WTREF4, op1, op2 );
	}else if ( s == 8 )
	{
		bc->Instr2( BC_WTREF8, op1, op2 );
	}else
	{
		assert(m_bHasCompileError);
	}
}

inline void psCCompiler::InstrRRET(psCByteCode* bc, const psSOperand& op, const psCDataType& type)
{
	int s = 0;
	if ( type.IsObject() || type.IsReference() )
	{
		s = 4;
	}else
	{
		s = type.GetSizeInMemoryBytes();
	}
	if ( s == 1 )
	{
		bc->Instr1(BC_RRET1, op);
	}else if ( s == 2) 
	{
		bc->Instr1(BC_RRET2, op);
	}else if ( s == 4 )
	{
		bc->Instr1(BC_RRET4, op);
	}else if ( s == 8 )
	{
		bc->Instr1(BC_RRET8, op);
	}else
	{
		assert(m_bHasCompileError);
	}
}

inline void psCCompiler::InstrSRET(psCByteCode* bc, const psSOperand& op, const psCDataType& type)
{
	int s = 0;
	if ( type.IsObject() || type.IsReference()  )
	{
		s = 4;
	}else
	{
		s = type.GetSizeInMemoryBytes();
	}
	if ( s == 1 )
	{
		bc->Instr1(BC_SRET1, op);
	}else if ( s == 2) 
	{
		bc->Instr1(BC_SRET2, op);
	}else if ( s == 4 )
	{
		bc->Instr1(BC_SRET4, op);
	}else if ( s == 8 )
	{
		bc->Instr1(BC_SRET8, op);
	}else
	{
		assert(m_bHasCompileError);
	}
}

inline int psCCompiler::InstrLINE(psCByteCode* bc, int pos)
{
	assert(m_pScript);

	int r;
	m_pScript->ConvertPosToRowCol(pos, &r, 0);	
	bc->Line(r);	
	return r;
}

inline psSOperand psCCompiler::MakeIntValueOp(int value)
{
	psSOperand operand;
	operand.BaseOffset(PSBASE_CONST, m_pBuilder->RegisterConstantInt(value));
	return operand;
}

inline psSOperand psCCompiler::MakeFloatValueOp(float value)
{
	psSOperand operand;
	operand.BaseOffset(PSBASE_CONST, m_pBuilder->RegisterConstantFloat(value));
	return operand;
}

inline int psCCompiler::GetConstantInt(const psSOperand& op)
{
	assert (op.base == PSBASE_CONST || m_bHasCompileError );
	return m_pBuilder->GetConstantInt(op.offset);
}

inline float psCCompiler::GetConstantFloat(const psSOperand& op)
{
	assert (op.base == PSBASE_CONST || m_bHasCompileError );
	return m_pBuilder->GetConstantFloat(op.offset);
}

inline void psCCompiler::CheckDataType( psCDataType& dt, psCScriptNode* node )
{
	// 检查类型是否正确
	if ( dt.GetSizeOnStackDWords() == 0)
	{
		psCString msg;
		msg.Format(TXT_DATA_TYPE_CANT_BE_s, dt.GetFormat().AddressOf() );

		Error( msg.AddressOf(), node );
	}
}

inline void psCCompiler::AllocateTemporaryVariable(psSValue& value)
{
	value.operand.BaseOffset(PSBASE_STACK, AllocateVariable(value.type, true));
	value.isTemporary = true;
}
