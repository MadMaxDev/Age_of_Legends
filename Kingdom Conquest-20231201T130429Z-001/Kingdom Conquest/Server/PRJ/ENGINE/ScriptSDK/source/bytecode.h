#ifndef __PS_BYTECODE_H__
#define __PS_BYTECODE_H__

#include "config.h"
#include "bytecodedef.h"
#include "string.h"
#include "memmanager.h"

class psCByteCode;
class psCByteInstruction
{
public:
	DECL_TEMP_MEM_OP()

public:
	psCByteInstruction();

	int GetSize() const { return m_Size;} 
	int GetStackInc() const { return m_StackInc; }

	psBYTE GetOP() const { return m_OP; }
	int    GetNumOperands() const;

	const psSOperand& GetOperand(int index) const {
		assert(index>=0 && index <=2);
		return m_Operands[index]; 
	}

	void Instr0(psBYTE op);
	void Instr1(psBYTE op, const psSOperand& op1);
	void Instr2(psBYTE op, const psSOperand& op1, const psSOperand& op2);
	void Instr3(psBYTE op, const psSOperand& op1, const psSOperand& op2, const psSOperand& op3);

	void Label(int label);
	void Line(int line);
	int  GetLineNum()  { return *((int*)m_Operands); }

	void AddAfter(psCByteInstruction* nextCode);
	void AddBefore(psCByteInstruction* prevCode);
	void Remove();

	friend class psCByteCode;
private:
	bool   m_bMarked;
	psBYTE m_OP;
	int	   m_Size;
	int	   m_StackSize;
	int	   m_StackInc;
	psSOperand m_Operands[3];

	psCByteInstruction* m_pPrev;
	psCByteInstruction* m_pNext;
};

class psCByteCode
{
public:
	psCByteCode(void);
	~psCByteCode(void);

	void ClearAll();
	void Label(int label);
	void Line(int  line);
	void Ret(int stackInc);
	void Push(int stackInc);
	void Push4(const psSOperand& op);
	void Push8(const psSOperand& op);
	void PushRef(const psSOperand& op);

	void Pop(int stackInc);

	void CallSys( const psSOperand& funcId, int argSize, bool bObject );
	void Call( const psSOperand& funcId, int argSize, bool bObject  );

	void SetStr(const psSOperand& op);

	void Instr0(psBYTE op);
	void Instr1(psBYTE op, const psSOperand& op1);
	void Instr2(psBYTE op, const psSOperand& op1, const psSOperand& op2);
	void Instr3(psBYTE op, const psSOperand& op1, const psSOperand& op2, const psSOperand& op3);

	int  GetLastInstrOp()		 { return (m_pLast) ? m_pLast->GetOP() : -1; }
	psSOperand GetLastInstrOperand0() { return (m_pLast) ? m_pLast->GetOperand(0) : psSOperand(); }
	int  DeleletLastInstr();

	void Finalize(const psSOptimizeOptions* options = NULL);
	void AddCode(psCByteCode* bc);
	int  GetSize() const ;
	void Output(psBYTE* array) const;

	bool IsEmpty() const { return (m_pFirst == NULL); }

	std::vector< int >  m_LineNumbers;
	int					m_LargestStackUsed;
protected:
	void AddInstruction();
	void AddInstructionFirst();
	psCByteInstruction* DeleteInstruction(psCByteInstruction* instr);
	void RemoveInstruction(psCByteInstruction* instr);

	void Optimize(const psSOptimizeOptions* options);
	int  ResolveJumpAddresses();
	void PostProcess();
	void ExtractLineNumbers();

	int FindLabel(int label, psCByteInstruction* from, psCByteInstruction** dest, int *positiveDelta);
	void AddPath(std::vector< psCByteInstruction* > &paths, psCByteInstruction *instr, int stackSize);

	bool DeleteUnusedInstr();
	bool IsCombination(psCByteInstruction *curr, psBYTE bc1, psBYTE bc2);
	bool IsInstrWriteOp(psCByteInstruction* instr, const psSOperand& op);
	bool IsInstrReadOp(psCByteInstruction* instr, const psSOperand& op);
	bool IsOperandUsed(psCByteInstruction* instr, const psSOperand& op);

private:
	psCByteInstruction* m_pFirst;
	psCByteInstruction* m_pLast;

};

#define BI_IN_TEMP_MEMORY

#endif // __PS_BYTECODE_H__
