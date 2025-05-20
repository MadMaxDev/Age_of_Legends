#include "bytecode.h"

// ------------ psCByteInstruction --------------------------
psCByteInstruction::psCByteInstruction()
{
	m_StackInc = 0;
	m_OP = 0xFF;
	m_Size = 0;
	m_StackSize = 0;
	m_bMarked = false;
	m_pPrev = 0;
	m_pNext = 0;

}

int psCByteInstruction::GetNumOperands() const
{
	if (m_Size == SIZEOFBC(0))
		return 0;
	else if (m_Size == SIZEOFBC(1))
		return 1;
	else if (m_Size == SIZEOFBC(2))
		return 2;
	else if (m_Size == SIZEOFBC(3))
		return 3;
	else 
		return 0;

}

void psCByteInstruction::Label(int label)
{
	m_OP = BC_LABEL;
	m_Size = BCS_LABEL;
	m_StackInc = 0;
	m_Operands[0].base = PSBASE_CONST;
	m_Operands[0].offset = label;
}

void psCByteInstruction::Line(int line)
{
	m_OP = BC_LINE;
	m_Size = BCS_LINE;
	m_StackInc = 0;
	m_Operands[0].base = PSBASE_CONST;
	m_Operands[0].offset = line;
}

void psCByteInstruction::Instr0(psBYTE op)
{
	m_OP = op;
	m_Size = g_psBCSize[m_OP];
	m_StackInc = 0;
	assert(m_Size == SIZEOFBC(0));
}

void psCByteInstruction::Instr1(psBYTE op, const psSOperand& op1)
{
	m_OP = op;
	m_Size = g_psBCSize[m_OP];
	m_Operands[0] = op1;

	assert(m_Size == SIZEOFBC(1));
}

void psCByteInstruction::Instr2(psBYTE op, const psSOperand& op1, const psSOperand& op2)
{
	m_OP = op;
	m_Size = g_psBCSize[m_OP];
	m_StackInc = 0;

	m_Operands[0] = op1;
	m_Operands[1] = op2;

	assert(m_Size == SIZEOFBC(2));
}

void psCByteInstruction::Instr3(psBYTE op, const psSOperand& op1, const psSOperand& op2, const psSOperand& op3)
{
	m_OP = op;
	m_Size = g_psBCSize[m_OP];
	m_StackInc = 0;
	m_Operands[0] = op1;
	m_Operands[1] = op2;
	m_Operands[2] = op3;

	assert(m_Size == SIZEOFBC(3));
}

void psCByteInstruction::AddAfter(psCByteInstruction* nextCode)
{
	assert(nextCode);

	if (m_pNext)
		m_pNext->m_pPrev = nextCode;
	
	nextCode->m_pNext = m_pNext;
	nextCode->m_pPrev = this; 
	m_pNext = nextCode;
}

void psCByteInstruction::AddBefore(psCByteInstruction* prevCode)
{
	assert(prevCode);
	if (m_pPrev)
		m_pPrev->m_pNext = prevCode;

	prevCode->m_pPrev = m_pPrev;
	prevCode->m_pNext = this;

	m_pPrev = prevCode;
}

void psCByteInstruction::Remove()
{
	if (m_pPrev) m_pPrev->m_pNext = m_pNext;
	if (m_pNext) m_pNext->m_pPrev = m_pPrev;

	m_pNext = 0;
	m_pPrev = 0;
}

// ----------- psCByteCode -----------------------------
psCByteCode::psCByteCode(void)
{
	m_pLast = 0;
	m_pFirst = 0;
	m_LargestStackUsed = 0;
}

psCByteCode::~psCByteCode(void)
{
	ClearAll();
}

void psCByteCode::ClearAll()
{
#ifndef BI_IN_TEMP_MEMORY
	while( m_pFirst ) 
	{	
		psCByteInstruction *del = m_pFirst;
		m_pFirst = m_pFirst->m_pNext;
		delete del;
	} 
#endif

	m_pFirst = 0;
	m_pLast  = 0;

	m_LineNumbers.clear();
	m_LargestStackUsed = -1;
}

void psCByteCode::AddInstruction()
{
	psCByteInstruction* instr = new psCByteInstruction();
	if (m_pFirst == 0)
		m_pFirst = m_pLast = instr;
	else
	{
		m_pLast->AddAfter(instr);
		m_pLast = instr;
	}
}

void psCByteCode::AddInstructionFirst()
{
	psCByteInstruction* instr = new psCByteInstruction();
	if (m_pFirst == 0)
		m_pFirst = m_pLast = instr;
	else
	{
		m_pFirst->AddBefore(instr);
		m_pFirst = instr;
	}
}

psCByteInstruction* psCByteCode::DeleteInstruction(psCByteInstruction* instr)
{
	if( instr == 0 ) return 0;

	psCByteInstruction *ret = instr->m_pPrev ? instr->m_pPrev : instr->m_pNext;

	RemoveInstruction(instr);

	delete instr;

	return ret;
}

void psCByteCode::RemoveInstruction(psCByteInstruction* instr)
{
	if( instr == m_pFirst ) m_pFirst = m_pFirst->m_pNext;
	if( instr == m_pLast )  m_pLast =  m_pLast->m_pPrev;

	if( instr->m_pPrev ) instr->m_pPrev->m_pNext = instr->m_pNext;
	if( instr->m_pNext ) instr->m_pNext->m_pPrev = instr->m_pPrev;

	instr->m_pNext = 0;
	instr->m_pPrev = 0;
}

int psCByteCode::DeleletLastInstr()
{
	if (m_pLast == 0)
		return -1;
	if( m_pFirst== m_pLast )
	{
		delete m_pLast;
		m_pFirst = 0;
		m_pLast = 0;
	}
	else
	{
		psCByteInstruction *instr = m_pLast;
		m_pLast = instr->m_pPrev;
	
		if ( m_pLast )
		{
			m_pLast->m_pNext = 0;
		}
		delete instr;
	}
	return 0;
}

void psCByteCode::AddCode(psCByteCode* bc)
{
	if (bc->m_pFirst)
	{
		if (m_pFirst == 0)
		{
			m_pFirst = bc->m_pFirst;
			m_pLast = bc->m_pLast;
		}else
		{
			m_pLast->m_pNext = bc->m_pFirst;
			bc->m_pFirst->m_pPrev = m_pLast;
			m_pLast = bc->m_pLast;
		}

		bc->m_pFirst = 0;
		bc->m_pLast = 0;
	}
}

void psCByteCode::Finalize(const psSOptimizeOptions* options)
{
	// verify the bytecode
	PostProcess();

	if (options)
	{
		// Optimize the code (optionally)
		Optimize(options);
	}

	// Resolve jumps
	int r = ResolveJumpAddresses();
	assert(r == 0);

	// Build line numbers buffer
	ExtractLineNumbers();
}

//----------------------------------------------------------------------//
// Name : PostProcess
// Desc : 这个函数的功能如下: 
//	`	  1. 检查是否有不可能到达的代码,并删除它
//		  2. 计算每条指针的堆栈大小, 并统计最大的堆栈大小
//----------------------------------------------------------------------//
void psCByteCode::PostProcess()
{
	if( m_pFirst == NULL )
		return;

	m_LargestStackUsed = 0;

	// 设置每条指今标识
	// 'bMarked' 表示这条指今是否被处理过
	// 'stackSize'表示动行到这条指针时的堆栈大小
	psCByteInstruction* instr = m_pFirst;
	while (instr)
	{
		instr->m_bMarked = false;
		instr->m_StackSize = -1;
		instr = instr->m_pNext;
	}

	// 把第一条指令添加到当前路径中
	std::vector< psCByteInstruction* > paths;
	AddPath(paths, m_pFirst, 0);

	// 检查每条指今路径
	for (unsigned int p = 0; p < paths.size(); ++p)
	{
		instr = paths[p];
		int stackSize = instr->m_StackSize;

		while (instr )
		{
			instr->m_bMarked = true;
			instr->m_StackSize = stackSize;
			stackSize += instr->m_StackInc;
			if (stackSize > m_LargestStackUsed)
				m_LargestStackUsed = stackSize;

			if (instr->GetOP() == BC_JMP)
			{
				// 得到我们要跳转的标号
				int label = instr->GetOperand(0).offset;

				// 找到跳转要执行的语句
				psCByteInstruction* dest = 0;
				int r = FindLabel(label, instr, &dest, 0);
				assert(r == 0);

				// 将跳转要执行的语句添加到当前路径
				AddPath(paths, dest, stackSize);
				break;
			}else if (instr->GetOP() == BC_JZ || instr->GetOP() == BC_JNZ)
			{
				// 得到我们要跳转的标号
				int label = instr->GetOperand(1).offset;

				// 找到跳转要执行的语句
				psCByteInstruction* dest = 0;
				int r = FindLabel(label, instr, &dest, 0);
				assert( r == 0 );

				// 将可能跳转到的语句添加到当前路径
				AddPath(paths, dest, stackSize);
				AddPath(paths, instr->m_pNext, stackSize);

				break;
			}else if (instr->GetOP() == BC_JMPP)
			{
				// 得到最大的跳转相对指今条数
				psDWORD max = instr->GetOperand(1).offset;

				// 将可能跳转到的语句添加到当前路径
				psCByteInstruction *dest = instr->m_pNext;
				for( psDWORD n = 0; n <= max && dest != NULL; ++n )
				{
					AddPath(paths, dest, stackSize);
					dest = dest->m_pNext;	
				}				
				break;
			}else
			{
				// 移到下一条指今
				instr = instr->m_pNext;

				// 如果为空或者是已经走到过的指今, 跳出!
				if( instr == NULL || instr->m_bMarked )
					break;
			}
		}
	}

	// 删除根本不可能到达的指今语句
	instr = m_pFirst;
	while( instr )
	{
		if( instr->m_bMarked == false )
		{
			// Remove it
			psCByteInstruction *curr = instr;
			instr = instr->m_pNext;

			DeleteInstruction(curr);
		}
		else
		{
			instr = instr->m_pNext;
		}
	}	
}	

void psCByteCode::ExtractLineNumbers()
{
	m_LineNumbers.clear();
	int lastLinePos = -1;
	int pos = 0;
	psCByteInstruction *instr = m_pFirst;
	while( instr )
	{
		psCByteInstruction *curr = instr;
		instr = instr->m_pNext;
		pos += curr->GetSize();

		if( curr->GetOP() == BC_LINE )
		{
			int lineno = curr->GetOperand(0).offset;

			if( lastLinePos == pos )
			{
				// 如果上一条行指令的位置和当前的相等,
				// 需要更新指今的行号
				m_LineNumbers[m_LineNumbers.size() - 1] = lineno;
			}else
			{
				m_LineNumbers.push_back(pos);
				m_LineNumbers.push_back(lineno);
			}
			lastLinePos = pos;	

			// 删除这条行指令
			DeleteInstruction( curr );
		}
	}
}

int psCByteCode::FindLabel(int label, psCByteInstruction* from, psCByteInstruction** dest, int *positiveDelta )
{
	// 向前查找标号
	int labelPos = -from->GetSize();
	psCByteInstruction* labelInstr = from;
	while (labelInstr)
	{
		labelPos += labelInstr->GetSize();
		labelInstr = labelInstr->m_pNext;

		if ( labelInstr && (labelInstr->GetOP() == BC_LABEL) && 
			(labelInstr->GetOperand(0).offset == label))
		{
			break;
		}	
	}

	// 如果前面没有找到, 那么要向后查找
	if (labelInstr == 0)
	{
		labelPos = -from->GetSize();
		labelInstr = from;
		while (labelInstr)
		{	
			labelInstr = labelInstr->m_pPrev;
			if (labelInstr)
			{
				labelPos -= labelInstr->GetSize();
		
				if ((labelInstr->GetOP() == BC_LABEL) &&
					(labelInstr->GetOperand(0).offset == label))
						break;
			}
		}
	}

	if (labelInstr != 0)
	{
		// 找到了这个标号
		if (dest) *dest = labelInstr;
		if (positiveDelta) *positiveDelta = labelPos;
		return 0;
	}

	return -1;
}


void psCByteCode::Optimize(const psSOptimizeOptions* options)
{
	assert(options);
	
	psCByteInstruction *instr;
	psCByteInstruction *curr;

	if (options->bDelDebugInstr)
	{
		// 删除DEBUG相关的指今
		instr = m_pFirst;
		while( instr )
		{	
			curr = instr;
			instr = instr->m_pNext;

			if (curr->GetOP() == BC_NOP)
			{
				DeleteInstruction(curr);
			}
		}
	}

	if (options->bDelUnnessnaryInstr)
	{
		instr = m_pFirst;
		while( instr )
		{	
			curr = instr;
			instr = instr->m_pNext;

			if (curr->GetOP() == BC_PUSH || curr->GetOP() == BC_POP)
			{
				// PUSH 0, POP 0 => NONE
				if (curr->GetOperand(0).offset == 0)
				{
					// Remove it
					DeleteInstruction(curr);
					continue;
				}
			}

			int posDelta = 0;
			if (curr->GetOP() == BC_JMP)
			{
				int r = FindLabel(curr->m_Operands[0].offset, curr, 0, &posDelta);
				assert(r == 0);
				if (posDelta == 0)
				{
					// JMP 0 => NONE
					DeleteInstruction(curr);
				}
			}else if (curr->GetOP() == BC_JZ || curr->GetOP() == BC_JNZ)
			{
				int r = FindLabel(curr->m_Operands[1].offset, curr, 0, &posDelta);
				assert(r == 0);
				if (posDelta == 0)
				{
					// JNZ 0 => NONE
					// JZ 0 => NONE
					DeleteInstruction(curr);
				}
			}
		}
	}

	// 合并指今
	if (options->bCombinedInstr)
	{	
		instr = m_pFirst;
		while( instr )
		{	
			curr = instr;
			instr = instr->m_pNext;

#if 1
			if( !instr ) break;

			switch( instr->m_OP )
			{
			case BC_MOV4:
				{
					switch( curr->m_OP )
					{
					case BC_ADDi: case BC_SUBi: case BC_DIVi: case BC_MULi: case BC_MODi: case BC_NEGi:
					case BC_ADDf: case BC_SUBf: case BC_DIVf: case BC_MULf: case BC_MODf: case BC_NEGf:
					case BC_AND: case BC_OR: case BC_XOR: case BC_NOT: case BC_SLL: case BC_SRL: case BC_SLA: case BC_SRA:
					case BC_F2I: case BC_I2F: case BC_D2I: case BC_D2F:
					case BC_I2D: case BC_F2D:
						{
							goto com1;
						}
						break;
					case BC_MOV4:
						{
							goto com2;
						}
						break;
					case BC_DEFREF:
						{
							goto com4;
						}
						break;
					case BC_RDREF4:
						{
							goto com5;
						}
						break;
					default:
						{
							continue;
						}
					}
				}
				break;
			case BC_MOV8:
				{
					switch( curr->m_OP )
					{
					case BC_ADDd: case BC_SUBd: case BC_DIVd: case BC_MULd: case BC_MODd: case BC_NEGd:
						{
							goto com1;
						}
						break;
					case BC_MOV8:
						{
							goto com2;
						}
						break;
					case BC_DEFREF:
						{
							goto com4;
						}
						break;
					case BC_RDREF8:
						{
							goto com5;
						}
						break;
					default:
						{
							continue;
						}
					}
				}
				break;
			case BC_PUSH4:
				{
					switch( curr->m_OP )
					{
					case BC_DEFREF: goto com3;
					default: continue;
					}
				}
				break;
			default:
				{
					continue;
				}
			}

com1:
			{
				// ADDi x, y, z, MOV4 a, x => Addi a, y, z
				// NEGi x, y, MOV4 a, x => NEGi a, y
				// F2I x, y, MOV4 a, x => F2I a, y
				if (curr->m_Operands[0] == instr->m_Operands[1])
				{		
					if ( !IsOperandUsed(instr->m_pNext, curr->m_Operands[0]) )
					{
						if ( curr->GetNumOperands() == 3 )
						{
							instr->Instr3( curr->GetOP(), instr->GetOperand(0), curr->GetOperand(1), curr->GetOperand(2) );
						}else
						{
							instr->Instr2( curr->GetOP(), instr->GetOperand(0), curr->GetOperand(1) );
						}
						DeleteInstruction(curr);
					}
				}
			}
			goto cend;
com2:
			{
				// MOV4 x, y, MOV4 y, x => NONE
				if (curr->m_Operands[0] == instr->m_Operands[1] &&
					curr->m_Operands[1] == instr->m_Operands[0])
				{
					psCByteInstruction* next = instr;
					instr = instr->m_pNext;
					
					DeleteInstruction(curr);
					DeleteInstruction(next);
					continue;
				}else if (curr->m_Operands[0] == instr->m_Operands[1])
				{
					if ( !IsOperandUsed( instr->m_pNext, curr->m_Operands[0] ) )
					{
						// MOV4 x, y, MOV4 z, x => MOV4 z, y
						instr->Instr2(BC_MOV4, instr->GetOperand(0), curr->GetOperand(1) );

						// 删除这条指今
						DeleteInstruction( curr );
					}
				}
			}
			goto cend;
com3:
			{
				// DEFREF x, y, PUSH4 x => PUSHREF y
				if (curr->GetOperand(0) == instr->GetOperand(0))
				{
					if ( !IsOperandUsed( instr->m_pNext, curr->GetOperand(0) ) )
					{
						instr->Instr1(BC_PUSHREF, curr->GetOperand(1) );
						instr->m_StackInc = 1;

						// 删除这条指今
						DeleteInstruction( curr );
					}
				}
			}	
			goto cend;
com4:
			{
				// DEFREF x, y, MOV z, x => DEFREF z, y
				if (curr->GetOperand(0) == instr->GetOperand(1))
				{
					if ( !IsOperandUsed(instr->m_pNext, curr->GetOperand(0) ) )
					{
						instr->Instr2( curr->GetOP(), instr->GetOperand(0), curr->GetOperand(1) );
						DeleteInstruction(curr);
					}
				}
			}
			goto cend;
com5:
			{
				// RDREF x, y, MOV z, x => RDREF z, y
				if (curr->GetOperand(0) == instr->GetOperand(1))
				{
					if ( !IsOperandUsed(instr->m_pNext, curr->GetOperand(0) ))
					{
						instr->Instr2( curr->GetOP(), instr->GetOperand(0), curr->GetOperand(1) );
						DeleteInstruction( curr );
					}
				}
			}
cend:
			;// 空指令，为了编译通过
#else
			if (IsCombination(curr, BC_ADDi, BC_MOV4) ||
				IsCombination(curr, BC_SUBi, BC_MOV4) ||
				IsCombination(curr, BC_DIVi, BC_MOV4) ||
				IsCombination(curr, BC_MULi, BC_MOV4) ||
				IsCombination(curr, BC_MODi, BC_MOV4) ||
				IsCombination(curr, BC_NEGi, BC_MOV4) ||
				
				IsCombination(curr, BC_ADDf, BC_MOV4) ||
				IsCombination(curr, BC_SUBf, BC_MOV4) ||
				IsCombination(curr, BC_DIVf, BC_MOV4) ||
				IsCombination(curr, BC_MULf, BC_MOV4) ||
				IsCombination(curr, BC_MODf, BC_MOV4) ||
				IsCombination(curr, BC_NEGf, BC_MOV4) ||
				
				IsCombination(curr, BC_AND,  BC_MOV4) ||
				IsCombination(curr, BC_OR,   BC_MOV4) ||
				IsCombination(curr, BC_XOR,  BC_MOV4) ||
				IsCombination(curr, BC_NOT,  BC_MOV4) ||
				IsCombination(curr, BC_SLL,  BC_MOV4) ||
				IsCombination(curr, BC_SRL,  BC_MOV4) ||
				IsCombination(curr, BC_SLA,  BC_MOV4) ||
				IsCombination(curr, BC_SRA,  BC_MOV4) ||
				
				IsCombination(curr, BC_F2I,  BC_MOV4) ||
				IsCombination(curr, BC_I2F,  BC_MOV4) ||
				IsCombination(curr, BC_D2I,  BC_MOV4)  ||
				IsCombination(curr, BC_D2F,  BC_MOV4)  ||
				
				IsCombination(curr, BC_ADDd, BC_MOV8) ||
				IsCombination(curr, BC_SUBd, BC_MOV8) ||
				IsCombination(curr, BC_DIVd, BC_MOV8) ||
				IsCombination(curr, BC_MULd, BC_MOV8) ||
				IsCombination(curr, BC_MODd, BC_MOV8) ||
				IsCombination(curr, BC_NEGd, BC_MOV8) ||
				
				IsCombination(curr, BC_I2D,  BC_MOV4)  ||
				IsCombination(curr, BC_F2D,  BC_MOV4) )
			{
				// ADDi x, y, z, MOV4 a, x => Addi a, y, z
				// NEGi x, y, MOV4 a, x => NEGi a, y
				// F2I x, y, MOV4 a, x => F2I a, y
				if (curr->m_Operands[0] == instr->m_Operands[1])
				{		
					if ( !IsOperandUsed(instr->m_pNext, curr->m_Operands[0]) )
					{
						if ( curr->GetNumOperands() == 3 )
						{
							instr->Instr3( curr->GetOP(), instr->GetOperand(0), curr->GetOperand(1), curr->GetOperand(2) );
						}else
						{
							instr->Instr2( curr->GetOP(), instr->GetOperand(0), curr->GetOperand(1) );
						}
						DeleteInstruction(curr);
					}
				}
			}else if (IsCombination(curr, BC_MOV4, BC_MOV4)||
					  IsCombination(curr, BC_MOV8, BC_MOV8))
			{
				// MOV4 x, y, MOV4 y, x => NONE
				if (curr->m_Operands[0] == instr->m_Operands[1] &&
					curr->m_Operands[1] == instr->m_Operands[0])
				{
					psCByteInstruction* next = instr;
					instr = instr->m_pNext;
					
					DeleteInstruction(curr);
					DeleteInstruction(next);
					continue;
				}else if (curr->m_Operands[0] == instr->m_Operands[1])
				{
					if ( !IsOperandUsed( instr->m_pNext, curr->m_Operands[0] ) )
					{
						// MOV4 x, y, MOV4 z, x => MOV4 z, y
						instr->Instr2(BC_MOV4, instr->GetOperand(0), curr->GetOperand(1) );

						// 删除这条指今
						DeleteInstruction( curr );
					}
				}
			}else if (IsCombination(curr, BC_DEFREF, BC_PUSH4))
			{
				// DEFREF x, y, PUSH4 x => PUSHREF y
				if (curr->GetOperand(0) == instr->GetOperand(0))
				{
					if ( !IsOperandUsed( instr->m_pNext, curr->GetOperand(0) ) )
					{
						instr->Instr1(BC_PUSHREF, curr->GetOperand(1) );
						instr->m_StackInc = 1;

						// 删除这条指今
						DeleteInstruction( curr );
					}
				}
			}else if (IsCombination(curr, BC_DEFREF, BC_MOV4) ||
					  IsCombination(curr, BC_DEFREF, BC_MOV8))
			{
				// DEFREF x, y, MOV z, x => DEFREF z, y
				if (curr->GetOperand(0) == instr->GetOperand(1))
				{
					if ( !IsOperandUsed(instr->m_pNext, curr->GetOperand(0) ) )
					{
						instr->Instr2( curr->GetOP(), instr->GetOperand(0), curr->GetOperand(1) );
						DeleteInstruction(curr);
					}
				}
			}else if (IsCombination(curr, BC_RDREF4, BC_MOV4) ||
				 	  IsCombination(curr, BC_RDREF8, BC_MOV8))
			{
				// RDREF x, y, MOV z, x => RDREF z, y
				if (curr->GetOperand(0) == instr->GetOperand(1))
				{
					if ( !IsOperandUsed(instr->m_pNext, curr->GetOperand(0) ))
					{
						instr->Instr2( curr->GetOP(), instr->GetOperand(0), curr->GetOperand(1) );
						DeleteInstruction( curr );
					}
				}
			}
#endif
		}
	}

	// 删除没有用的指令, 这个过程比较慢 
	if (options->bDelUnusedInstr)
	{
		bool r = false;
		do 
		{
			r = DeleteUnusedInstr();
		}while (r);
	}

}

bool psCByteCode::DeleteUnusedInstr()
{
	bool r = false;

	psCByteInstruction* instr = m_pFirst;
	psCByteInstruction* curr;

	while( instr )
	{	
		curr = instr;
		instr = instr->m_pNext;

		int op  = curr->GetOP();
		int writeIdx = g_psBCReadWriteIndex[op].writeOp;

		if ( writeIdx != -1 )
		{
			const psSOperand& writeOp = curr->GetOperand(writeIdx);
			if ( writeOp.base == PSBASE_STACK && writeOp.offset < 0 )
			{
				// NOTE: 只有对堆栈的写操作可以删除
				if ( !IsOperandUsed( instr, writeOp ) )
				{
					// 如果读出来的临时操作数没有用到,可以删除这条指今
					DeleteInstruction( curr );
					r = true;
				}
			}
		}
	}

	return r;
}

bool psCByteCode::IsOperandUsed( psCByteInstruction* startInstr, const psSOperand& operand )
{
	psCByteInstruction* instr = m_pFirst;
	while (instr)
	{
		instr->m_bMarked = false;
		instr = instr->m_pNext;
	}

	std::vector< psCByteInstruction* > paths;
	paths.push_back( startInstr );

	for (size_t i = 0; i < paths.size(); ++i)
	{
		psCByteInstruction* pathInstr = paths[i];
		while (pathInstr)
		{
			pathInstr->m_bMarked = true;

			if (IsInstrReadOp(pathInstr, operand))
				return true;

			if (IsInstrWriteOp(pathInstr, operand))
				return false;

			int op = pathInstr->GetOP();
			if (op == BC_JMP)
			{
				// Find the label that we should jump to
				short label = pathInstr->GetOperand(0).offset;
				psCByteInstruction* dest = 0;
				int r = FindLabel(label, pathInstr, &dest, 0);
				assert(r == 0);

				paths.push_back( dest );
				break;
			}else if (op == BC_JZ || op == BC_JNZ)
			{
				short label = pathInstr->GetOperand(1).offset;

				psCByteInstruction* dest = 0;
				int r = FindLabel(label, pathInstr, &dest, 0);
				assert( r == 0 );

				// Add both paths to the code paths
				paths.push_back( dest );
				paths.push_back( pathInstr->m_pNext );
				break;
			}else if (op == BC_JMPP)
			{
				// We need to know the largest value 
				psDWORD max = pathInstr->GetOperand(1).offset;

				// Add all destinations to the code paths
				psCByteInstruction *dest = pathInstr->m_pNext;
				for( psDWORD n = 0; n <= max && dest != 0; ++n )
				{
					paths.push_back( dest );
					dest = dest->m_pNext;	
				}				
				break;
			}else
			{
				pathInstr = pathInstr->m_pNext;
				if( pathInstr == NULL || pathInstr->m_bMarked )
					break;
			}
		}
	}
	return false;
}

bool psCByteCode::IsInstrWriteOp(psCByteInstruction* instr, const psSOperand& operand)
{
	int op = instr->GetOP();

	int i0 = g_psBCReadWriteIndex[op].writeOp;
	if ( i0 != -1 )
	{
		if ( instr->GetOperand(i0) == operand )
			return true;
	}

	return false;
}

bool psCByteCode::IsInstrReadOp(psCByteInstruction* instr, const psSOperand& operand)
{
	int op = instr->GetOP();

	int i0 = g_psBCReadWriteIndex[op].readOp0;
	int i1 = g_psBCReadWriteIndex[op].readOp1;
	if ( i0 != -1 )
	{
		if ( instr->GetOperand(i0) == operand )
			return true;
	}
	if ( i1 != -1)
	{
		if ( instr->GetOperand(i1) == operand)
			return true;
	}
	return false;
}

bool psCByteCode::IsCombination(psCByteInstruction *curr, psBYTE bc1, psBYTE bc2)
{
	if( curr->m_OP == bc1 && curr->m_pNext && curr->m_pNext->m_OP == bc2 )
		return true;

	return false;
}


int psCByteCode::ResolveJumpAddresses()
{
	int pos = 0;
	
	psCByteInstruction *instr = m_pFirst;
	while( instr )
	{
		// 回填所有的跳转偏移
		pos += instr->GetSize();

		if( instr->GetOP() == BC_JMP)
		{
			int label = instr->m_Operands[0].offset;
			int labelPosOffset;
			int r = FindLabel(label, instr, 0, &labelPosOffset);
			if( r == 0 )
				instr->m_Operands[0].offset = labelPosOffset;
			else
				return -1;

		}else if (instr->GetOP() == BC_JZ  || instr->GetOP() == BC_JNZ )
		{
			int label = instr->m_Operands[1].offset;
			int labelPosOffset;
			int r = FindLabel(label, instr, 0, &labelPosOffset);
			if( r == 0 )
				instr->m_Operands[1].offset = labelPosOffset;
			else
				return -1;
		}
		instr = instr->m_pNext;
	}

	return 0;
}

void psCByteCode::AddPath(std::vector< psCByteInstruction* > &paths, psCByteInstruction* instr, int stackSize)
{
	if (instr->m_bMarked)
	{
		// 如果这条指今已经被走到过,其堆栈大小与当前堆栈大小一定相等
		// 如果不相等,编译器生成的代码肯定有错!
		assert( instr->m_StackSize == stackSize);
	}else
	{
		// 添加一条新的指今路径
		instr->m_bMarked = true;
		instr->m_StackSize = stackSize;
		paths.push_back(instr);
	}
}


void psCByteCode::Instr0(psBYTE op)
{
	AddInstruction();
	m_pLast->Instr0(op);
}

void psCByteCode::Instr1(psBYTE op, const psSOperand& op1)
{
	AddInstruction();
	m_pLast->Instr1(op, op1);

	// 如果对堆栈位置有影响的指今, 则不能以这种方式生成代码
	assert( op != BC_PUSHREF && op != BC_PUSH &&
			op != BC_PUSH4 && op != BC_PUSH8 &&
			op != BC_POP && op != BC_RET && 
			op != BC_CALL );
}

void psCByteCode::Instr2(psBYTE op, const psSOperand& op1, const psSOperand& op2)
{
	AddInstruction();
	m_pLast->Instr2(op, op1, op2);
	assert(op != BC_CALLSYS);
}

void psCByteCode::Instr3(psBYTE op, const psSOperand& op1, const psSOperand& op2, const psSOperand& op3)
{
	AddInstruction();
	m_pLast->Instr3(op, op1, op2, op3);
}

void psCByteCode::Label(int label)
{
	AddInstruction();
	m_pLast->Label(label);
}

void psCByteCode::Line(int line)
{
	AddInstruction();
	m_pLast->Line(line);
}

void psCByteCode::Call( const psSOperand& funcId, int argSize, bool bObject )
{
	AddInstruction();

	m_pLast->Instr1( BC_CALL, funcId );

	m_pLast->m_StackInc = -argSize - (bObject ? 1 : 0);
}


void psCByteCode::CallSys( const psSOperand& funcId, int argSize, bool bObject )
{
	AddInstruction();

	m_pLast->Instr2(BC_CALLSYS, funcId, argSize);

	m_pLast->m_StackInc = -argSize - (bObject ? 1 : 0);
}


void psCByteCode::Ret(int stackInc)
{
	AddInstruction();
	m_pLast->Instr1(BC_RET, stackInc);
	m_pLast->m_StackInc = -stackInc;
}

void psCByteCode::Push(int stackInc)
{
	AddInstruction();
	m_pLast->Instr1(BC_PUSH, stackInc);
	m_pLast->m_StackInc = stackInc;
}

void psCByteCode::Push4(const psSOperand& op)
{
	AddInstruction();
	m_pLast->Instr1(BC_PUSH4, op);
	m_pLast->m_StackInc = 1;
}

void psCByteCode::Push8(const psSOperand& op)
{
	AddInstruction();
	m_pLast->Instr1(BC_PUSH8, op);
	m_pLast->m_StackInc = 2;
}

void psCByteCode::PushRef(const psSOperand& op)
{
	AddInstruction();
	m_pLast->Instr1(BC_PUSHREF, op);
	m_pLast->m_StackInc = 1;
}

void psCByteCode::Pop(int stackInc)
{
	AddInstruction();
	m_pLast->Instr1(BC_POP, stackInc);
	m_pLast->m_StackInc = -stackInc;
}

int psCByteCode::GetSize() const
{
	int size = 0;
	psCByteInstruction *instr = m_pFirst;
	while( instr )
	{
		size += instr->GetSize();

		instr = instr->m_pNext;
	}

	return size;
}

void psCByteCode::Output(psBYTE* array) const
{
	psBYTE *ap = array;

	int i;
	psCByteInstruction *instr = m_pFirst;
	while( instr )
	{
		if (instr->GetSize() > 0)
		{
			ap[0] = instr->GetOP();
			ap[1] = ap[2] = ap[3] = 0;
			for (i = 0; i < instr->GetNumOperands(); ++i)
			{
				ap[i+1] = instr->GetOperand(i).base;
				*(int*)(ap + (i+1)*sizeof(int)) = instr->GetOperand(i).offset;
			}
			ap += instr->GetSize();
		}

		instr = instr->m_pNext;
	}
}

