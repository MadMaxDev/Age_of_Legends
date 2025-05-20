#include "compiler.hpp"
//-------------------------------------------------------------------
// ����: CompileIFStatement
// ˵��: 
//------------------------------------------------------------------
void psCCompiler::CompileIFStatement(psCScriptNode* inode, bool* hasReturn, psCByteCode* bc)
{
	// ���һ����ָ��
	InstrLINE(bc, inode->GetTokenPos());

	// ��IF�������ǽ�ʹ��һ����ǩ,Ҳ�п���Ϊ'ELSE'��ʹ��һ��
	int afterLabel = m_NextLabel++;

	// �����������ʽ
	psCByteCode exprBC;
	psSValue	  exprValue;
	psCScriptNode* node = inode->GetFirstChild();
	CompileAssignment(node, &exprBC, &exprValue);

	// �����ʽ������
	if ( !exprValue.type.IsBooleanType() ||
		  exprValue.type.IsPointer()  )
	{
		Error(TXT_EXPR_MUST_BE_BOOL, node);
		return;
	}

	psSOperand exprValueOp;
	// ȷ���������ֽڵ����ͳ���
	GetIntOperand( &exprBC, exprValue, exprValueOp );

	// �ͷ���ʱ����
	ReleaseTemporaryVariable(exprValue);

	// �������ʽ�е���ʱ���������ڵ���
	ClearTemporaryVariables( &exprBC );

	if ( exprValueOp.base != PSBASE_CONST )
	{
		// ������ǳ������ʽ, ����������ʽ�Ĵ���
		bc->AddCode(&exprBC);

		bc->Instr2(BC_JZ, exprValueOp, psSOperand(afterLabel));
	}else
	{
		// �������ʽ��
		if (GetConstantInt(exprValueOp) == 0)
		{
			// Jump to the else case
			bc->Instr1(BC_JMP, psSOperand(afterLabel));

			Warning( TXT_IF_STATEMENT_ALWAYS_BE_FALSE, inode);
		}else
		{
			Warning( TXT_IF_STATEMENT_ALWAYS_BE_TRUE, inode);
		}
	}
	if ( exprValue.operand != exprValueOp )
		ReleaseTemporaryOperand( exprValueOp );

	// ���������ʽ�����һ����ָ��
	InstrLINE(bc, inode->GetTokenPos());

	// ����'if'����
	bool hasReturn1;
	psCByteCode ifBC;
	CompileStatement(node->GetNextNode(), &hasReturn1, &ifBC);

	//  ���'if'����Ĵ���
	bc->AddCode(&ifBC);

	// �Ƿ���'else'����
	if (node->GetNextNode() != inode->GetLastChild())
	{
		int afterElse = 0;
		if ( !hasReturn1 )
		{
			afterElse = m_NextLabel++;

			//������'else'��,ִ��'if'����(����Ϊ��ʱ)
			bc->Instr1(BC_JMP, psSOperand(afterElse));
		}

		// Ϊ'if'���Ϊ��ʱ���������һ����ǩ
		bc->Label(afterLabel);

		// ����'else'����
		bool hasReturn2;
		psCByteCode elseBC;
		CompileStatement(inode->GetLastChild(), &hasReturn2, &elseBC);

		// ���'else'���ֵĴ���
		bc->AddCode(&elseBC);

		if( !hasReturn1 )
		{
			// �����'if'���û�з��أ����'if'���Ϊ��ı�ǩ
			bc->Label(afterElse);
		}

		// ���÷��ر�־
		*hasReturn = hasReturn1 && hasReturn2;
	}else
	{
		// Ϊ'if'����Ľ��������һ����ǩ
		bc->Label(afterLabel);
		*hasReturn = false;
	}
}

//-------------------------------------------------------------------
// ����: CompileSwitchStamement
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileSwitchStamement(psCScriptNode* snode, bool* hasReturn, psCByteCode* bc)
{
	// Ϊ'break'����������һ����ǩ
	int breakLabel = m_NextLabel++;

	// ��Ϊ'break'�����������ı�ǩ����������
	m_BreakLabels.push_back(breakLabel);

	// ���һ��������
	AddVariableScope(true, false);

	// ����'switch'���������ʽ
	psSValue exprValue;
	psCByteCode exprBC;

	// �ڱ��ʽ֮ǰ���һ����ָ��
	InstrLINE(&exprBC, snode->GetTokenPos());

	psCScriptNode* enode = snode->GetFirstChild();
	CompileAssignment(enode, &exprBC, &exprValue);

	// ��'case'���֮ǰ���һ����ָ��
	InstrLINE(&exprBC, snode->GetTokenPos());

	// �������ʽ�����ͱ�����'int'��'unsigned int'��
	if( !exprValue.type.IsIntegerType() ||
		exprValue.type.IsPointer() )
	{
		Error(TXT_SWITCH_MUST_BE_INTEGRAL, snode);
		return;
	}

	// ȷ���������ֽڵ����ͳ���
	psSOperand exprValueOp;
	GetIntOperand( &exprBC, exprValue, exprValueOp );

	// �������ʽ�����ɵ���ʱ���������ڵ���
	ClearTemporaryVariables(&exprBC);

	// ����һ����ʱ����,���������м���
	psSValue tmp(psCDataType(ttInt, false, false));
	AllocateTemporaryVariable(tmp);

	// ����'case'��ֵ���ǩ��
	// Ԥ��һ����ǩ��,�������ǿ����� CompileCase()���ҵ���ȷ�ı�ǩ��
	int firstCaseLabel = m_NextLabel;
	int defaultLabel = 0;

	std::vector< int >		caseValues;
	std::vector< psSOperand>  caseOps;
	std::vector< int >		caseLabels;

	// �������е�'case'��䣬��ʹ�����ܹ���ת����ȷ�ı�ǩ
	psCScriptNode *cnode = enode->GetNextNode();
	while( cnode )
	{
		// ÿһ��'case'��䣬�����ǳ���
		if (cnode->GetFirstChild() && cnode->GetFirstChild()->GetNodeType() == snExpression)
		{
			// ������ʽ��ֵ
			psSValue cvalue;
			psCByteCode cbc;
			CompileExpression(cnode->GetFirstChild(), &cbc, &cvalue);

			// �����ʽ��ֵ
			if (!cvalue.IsConstant())
				Error(TXT_SWITCH_CASE_MUST_BE_CONSTANT, cnode);

			// �����ʽ��ֵ������
			if (!cvalue.type.IsIntegerType())
				Error(TXT_SWITCH_MUST_BE_INTEGRAL, cnode);

			// ���case����ֵ�����ظ�
			int caseIntValue = GetConstantInt(cvalue.operand);
			for (size_t n = 0; n < caseValues.size(); ++n)
			{
				if (caseValues[n] == caseIntValue)
				{
					psCString msg;
					msg.Format(TXT_CASE_VALUE_d_ALREADY_USED, caseValues[n]);
					Error(msg.AddressOf(), cnode);
					break;
				}
			}

			// ����'case'����ֵ,Ϊ�Ժ�ʹ��
			caseValues.push_back(caseIntValue);
			caseOps.push_back(cvalue.operand);

			// Ϊ���'case'��䱣��һ����ǩ
			caseLabels.push_back(m_NextLabel++);
		}else
		{
			//  'default'�����������е�'case'���֮��
			if (cnode->GetNextNode())
			{
				Error(TXT_DEFAULT_MUST_BE_LAST, cnode);
			}

			// Ϊ'default'��䱣��һ����ǩ
			defaultLabel = m_NextLabel++;
		}
		cnode = cnode->GetNextNode();
	}

	if( defaultLabel == 0 )
	{
		defaultLabel = breakLabel;
	}

	// ����Ż��ĵıȽ��������
	// ���'case'����ֵ��������������ֵС��5�Ļ�
	// ���������ֵ̫��Ļ�������һ����ת��Ĵ���ͻ�ܴ�
	// ������һ����ת������ֻ��Ҫ��һ�αȽϾͿɴﵽĿ�ĵ�

	// ��'case'���ĵ�ֵ����С�����˳������ͬʱҲҪ�Ա�ǩ���Ͳ�������������
	// ʹ������򵥵�ð������
	for (int fwd = 1; fwd < (int)caseValues.size(); ++fwd)
	{
		for (int bck = fwd - 1; bck >= 0; --bck)
		{
			size_t bckp = bck + 1;
			if( caseValues[bck] > caseValues[bckp] )
			{
				std::swap(caseValues[bck], caseValues[bckp]);
				std::swap(caseOps[bck], caseOps[bckp]);
				std::swap(caseLabels[bck], caseLabels[bckp]);
			}else
			{
				break;
			}
		}
	}

		if (caseOps.size() > 0)
		{	
			// ��'case'����ֵ�ֶΣ����������ֵ���С��5�Ļ����ͷ���ͬһ��
			std::vector<int> rangs;
			rangs.push_back(0);
			int n;
			for (n = 1; n < int(caseValues.size()); ++n)
			{
				if (caseValues[n] >= caseValues[n-1] + 5)
					rangs.push_back(n);
			}

			// ����������ʽ��ֵ������'case'����ֵ����ֱ������'default'���
			exprBC.Instr3(BC_TGi, tmp.operand, exprValueOp, caseOps[caseOps.size() - 1]);
			exprBC.Instr2(BC_JNZ, tmp.operand, psSOperand(defaultLabel));

			// ��ÿһ����Χ���д���
			size_t rang;
			for (rang = 0; rang < rangs.size(); ++rang)
			{
				// �ҳ��˷�Χ�е����ֵ
				int min_index = rangs[rang];
				int index	  = min_index;
				int maxRange = caseValues[index];
				for (; index < int(caseValues.size()) && (caseValues[index] < maxRange + 5); ++index)
				{
					maxRange = caseValues[index];
				}

				// ���˵��һ����Χ����ֻ�������Ƚϣ���ô��ֱ�ӽ��бȽ�����
				if (index - min_index < 3)
				{
					for (n = min_index; n < index; ++n)
					{
						// �򵥵ıȽ�ÿ��ֵ��������������Ŀ�ĵ�
						exprBC.Instr3(BC_TEi, tmp.operand, exprValueOp, caseOps[n]);
						exprBC.Instr2(BC_JNZ, tmp.operand, psSOperand(caseLabels[n]));
					}
				}else
				{
					// ������ʽ�ıȴ˷�Χ��С��ֵ��С������'default'���
					exprBC.Instr3(BC_TLi, tmp.operand, exprValueOp, caseOps[min_index] );
					exprBC.Instr2(BC_JNZ, tmp.operand, psSOperand(defaultLabel));

					// ��������Χ�������һ�Σ�û�б����������ֵ�ıȽ�
					int nextRangLabel = m_NextLabel++;
					if (rang < rangs.size() - 1)
					{	
						int offset = m_pBuilder->RegisterConstantInt(maxRange);

						// ������ʽ�ıȴ˷�Χ��С��ֵ����������һ��
						exprBC.Instr3(BC_TGi, tmp.operand, exprValueOp, psSOperand(PSBASE_CONST, offset));
						exprBC.Instr2(BC_JNZ, tmp.operand, psSOperand(nextRangLabel));
					}

					// ���ݱ��ʽ��ֵ����ָ����ת
					exprBC.Instr3(BC_SUBi, tmp.operand, exprValueOp, caseOps[min_index]);
					exprBC.Instr2(BC_JMPP, tmp.operand, maxRange - caseValues[min_index]);	

					//�������˷�Χ����ת������ǿյ�ֱ������'default'���
					index = rangs[rang];
					for (int n = caseValues[index]; n <= maxRange; ++n)
					{
						if (caseValues[index] == n)
							exprBC.Instr1(BC_JMP, caseLabels[index++]);
						else
							exprBC.Instr1(BC_JMP, defaultLabel);
					}

					// �����һ�ο�ʼ
					exprBC.Label(nextRangLabel);
				}
			}	
		}

		// ��������û�����е����
		exprBC.Instr1(BC_JMP, defaultLabel);

		// �ͷ���ʱ������
		ReleaseTemporaryOperand(tmp.operand);
		if( exprValueOp != exprValue.operand )
			ReleaseTemporaryOperand(exprValueOp);
		ReleaseTemporaryVariable(exprValue);

		// ������е�'case'���
		cnode = enode->GetNextNode();
		while (cnode)
		{
			if (cnode->GetFirstChild() && cnode->GetFirstChild()->GetNodeType() == snExpression)
			{
				exprBC.Label(firstCaseLabel++);
				CompileCase(cnode->GetFirstChild()->GetNextNode(), &exprBC);
			}else
			{
				exprBC.Label(defaultLabel);
				// Is default the last case?
				if( cnode->GetNextNode() )
				{
					// We've already reported this error
					break;
				}

				CompileCase(cnode->GetFirstChild(), &exprBC);
			}
			cnode = cnode->GetNextNode();
		}

		// �������еĶ����ƴ���
		bc->AddCode(&exprBC);

		// ���'break'��ת��ǩ
		bc->Label(breakLabel);

		// ���������'break'��ǩ
		m_BreakLabels.pop_back();

		// �Ƴ�������
		int r;
		m_pScript->ConvertPosToRowCol(snode->GetTokenPos() + snode->GetTokenLength(), &r, 0);
		CreateDebugSymbols(m_Variables, r);
		RemoveVariableScope();

}

//-------------------------------------------------------------------
// ����: CompileCase
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileCase(psCScriptNode* node, psCByteCode* bc)
{
	bool isFinished = false;
	bool hasReturn = false;
	while (node)
	{
		if (isFinished || hasReturn)
		{
			Warning(TXT_UNREACHABLE_CODE, node);
			break;
		}
		if (node->GetNodeType() == snBreak ||
			node->GetNodeType() == snContinue)
			isFinished = true;

		CompileStatement(node, &hasReturn, bc);

		node = node->GetNextNode();
	}
}

//-------------------------------------------------------------------
// ����: CompileForStatement
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileForStatement(psCScriptNode* fnode, psCByteCode* bc)
{
	// ���һ��������
	AddVariableScope(true, true);

	// ������ѭ����ʹ��������ǩ
	int beforeLabel = m_NextLabel++;
	int afterLabel = m_NextLabel++;
	int continueLabel = m_NextLabel++;

	// ����break/continue��ǩ
	m_ContinueLabels.push_back(continueLabel);
	m_BreakLabels.push_back(afterLabel);

	// ���һ����ָ��
	InstrLINE(bc, fnode->GetTokenPos());

	// �����ʼ�����ʽ
	psCScriptNode* first = fnode->GetFirstChild();
	psCByteCode initBC;
	if (first->GetNodeType() == snDeclaration)
		CompileDeclaration(first, &initBC, NULL);
	else
		CompileExpressionStatement(first, &initBC);

	// ��ճ�ʼ�����ʽ�����ɵ���ʱ����
	ClearTemporaryVariables( &initBC );

	// �����������ʽ
	psCScriptNode* second = first->GetNextNode();
	psCByteCode exprBC;
	psSValue exprValue;
	if (second->GetFirstChild())
	{	
		CompileAssignment(second->GetFirstChild(), &exprBC, &exprValue);

		// ����������ʽ������
		if ( !exprValue.type.IsBooleanType() ||
			  exprValue.type.IsPointer() )
		{
			Error(TXT_EXPR_MUST_BE_BOOL, second);
			return;
		}

		// ȷ���������ֽڵ����ͳ���
		psSOperand exprValueOp;
		GetIntOperand( &exprBC, exprValue, exprValueOp );

		// �ͷ���ʱ������
		ReleaseTemporaryVariable(exprValue);

		// ����������ʽ�����ɵ���ʱ����
		ClearTemporaryVariables( &exprBC );

		// ������ʽ��ֵΪ������ѭ��
		exprBC.Instr2(BC_JZ, exprValueOp, psSOperand(afterLabel));
		if ( exprValueOp != exprValue.operand )
			ReleaseTemporaryOperand( exprValueOp );
	}

	//�����������ʽ
	psCByteCode nextBC;	
	psSValue nextValue;
	psCScriptNode* third = second->GetNextNode();
	if (third->GetNodeType() == snAssignment)
	{
		// 
		CompileAssignment(third, &nextBC, &nextValue);

		// �ͷ���ʱ������
		ReleaseTemporaryVariable(nextValue);

		// ����������ʽ�����ɵ���ʱ����
		ClearTemporaryVariables( &nextBC );
	}

	// ����ѭ�������
	psCScriptNode* statement = fnode->GetLastChild();
	bool hasReturn;
	psCByteCode loopBC;
	CompileStatement(statement, &hasReturn, &loopBC);

	//////////////////////////////////////////////////////////////////////////
	// �������еĶ����ƴ����

	// ��ӳ�ʼ������
	bc->AddCode(&initBC);
	bc->Label(beforeLabel);

	// ����������ʽ����
	bc->AddCode(&exprBC);

	// ���ѭ�����ֵĴ���
	bc->AddCode(&loopBC);
	bc->Label(continueLabel);

	// ��������εĴ���
	bc->AddCode(&nextBC);

	// ��һ����ָ��,�������
	//InstrLINE(bc, statement->GetTokenPos() + statement->GetTokenLength());

	bc->Instr1(BC_JMP, psSOperand(beforeLabel));
	bc->Label(afterLabel);

	// ���������break/continue��ǩ
	m_BreakLabels.pop_back();
	m_ContinueLabels.pop_back();

	// NOTE: ��Ϊ��FOR��...���ڿ��ܲ�����ʱ�������
	// ��Ҫ�������еľֲ�����
	// ����������ѭ�����ȴ�����ܲ�����ʱ�������
	psCVariableScope::VariableMap_t::const_reverse_iterator it = m_Variables->GetVariableMap().rbegin();
	for (; it != m_Variables->GetVariableMap().rend(); ++it)
	{
		const psSVariable* var = it->second;

		// �ͷű����ռ�
		if( var->value.operand.base == PSBASE_STACK &&
			var->value.operand.offset < 0 )
		{
			// �������еľֲ�����(��Ҫ�������ñ���)
			// NOTE:�����'break','continue'��'return'�����������ǰ�Ѿ���������

			CompileDestructor(var->value, bc);	
			// �ͷŶ�ջ�ռ�
			DeallocateVariable(var->value.operand.offset);
		}
	}

	// �Ƴ�������
	int r;
	m_pScript->ConvertPosToRowCol(statement->GetTokenPos() + statement->GetTokenLength(), &r, 0);
	CreateDebugSymbols(m_Variables, r);
	RemoveVariableScope();
}

void psCCompiler::CompileWhileStatement(psCScriptNode* wnode, psCByteCode* bc)
{
	// ���һ��������
	AddVariableScope(true, true);

	// ������ѭ����ʹ��������ǩ
	int beforeLabel = m_NextLabel++;
	int afterLabel = m_NextLabel++;

	// ����break/continue��ǩ
	m_ContinueLabels.push_back(beforeLabel);
	m_BreakLabels.push_back(afterLabel);

	// ��ʶ�ڱ��ʽ֮ǰ
	bc->Label(beforeLabel);

	// ���һ����ָ��
	InstrLINE(bc, wnode->GetTokenPos());

	// �����������ʽ��ֵ
	psCByteCode exprBC;
	psSValue exprValue;
	psCScriptNode* first = wnode->GetFirstChild();
	CompileAssignment(first, &exprBC, &exprValue);

	// ����������ʽ������
	if ( !exprValue.type.IsBooleanType() ||
		  exprValue.type.IsPointer() )
	{
		Error(TXT_EXPR_MUST_BE_BOOL, wnode);
		return;
	}

	// ȷ���������ֽڵ����ͳ���
	psSOperand exprValueOp;
	GetIntOperand( &exprBC, exprValue, exprValueOp );

	// �ͷ���ʱ����
	ReleaseTemporaryVariable(exprValue);

	// ����������ʽ�����ɵ���ʱ����
	ClearTemporaryVariables(&exprBC);

	// ����������ʽ�ĵĶ����ƴ���
	bc->AddCode(&exprBC);

	// ����������ʽ��ֵΪ������
	bc->Instr2(BC_JZ, exprValueOp, psSOperand(afterLabel));

	if ( exprValueOp != exprValue.operand )
		ReleaseTemporaryOperand(exprValueOp);

	// ����ѭ������
	bool hasReturn;
	psCByteCode whileBC;
	psCScriptNode* block = first->GetNextNode();
	CompileStatement(block, &hasReturn, &whileBC);
	// ���ѭ������Ϊ��,���һ����ָ��,�Ա����
	if ( whileBC.IsEmpty() )
		whileBC.Instr0(BC_NOP);

	// ���ѭ������Ĵ���
	bc->AddCode(&whileBC);

	// ��ת���������ʽǰ������ִ��
	bc->Instr1(BC_JMP, psSOperand(beforeLabel));

	// ��ʶѭ������
	bc->Label(afterLabel);

	// ���������break/continue��ǩ
	m_ContinueLabels.pop_back();
	m_BreakLabels.pop_back();

	// �Ƴ�������
	int r;
	m_pScript->ConvertPosToRowCol(block->GetTokenPos() + block->GetTokenLength(), &r, 0);
	CreateDebugSymbols(m_Variables, r);
	RemoveVariableScope();
}

void psCCompiler::CompileDoWhileStatement(psCScriptNode* wnode, psCByteCode* bc)
{
	// ���һ��������
	AddVariableScope(true, true);

	// ������ѭ����ʹ��������ǩ
	int beforeLabel = m_NextLabel++;
	int beforeTest =  m_NextLabel++;
	int afterLabel =  m_NextLabel++;

	// ����break/continue��ǩ
	m_ContinueLabels.push_back(beforeTest);
	m_BreakLabels.push_back(afterLabel);

	// ���һ����ָ��
	bc->Label(beforeLabel);

	// ����ѭ������
	psCByteCode whileBC;
	bool hasReturn;
	psCScriptNode* first = wnode->GetFirstChild();
	CompileStatement(first, &hasReturn, &whileBC);

	// ���ѭ������Ϊ��,���һ����ָ��,�Ա����
	if (whileBC.IsEmpty())
		whileBC.Instr0(BC_NOP);

	// ���ѭ������Ĵ���
	bc->AddCode(&whileBC);

	// ��ǿ�ʼ�����ж�
	bc->Label(beforeTest);

	// ���һ����ָ��
	InstrLINE(bc, wnode->GetLastChild()->GetTokenPos());

	//�������������ʽ��ֵ
	psCByteCode exprBC;
	psSValue exprValue;
	CompileAssignment(wnode->GetLastChild(), &exprBC, &exprValue);

	// ����������ʽ������
	if (!exprValue.type.IsBooleanType() ||
		 exprValue.type.IsPointer() )
	{
		Error(TXT_EXPR_MUST_BE_BOOL, wnode->GetLastChild());
		return;
	}

	// ȷ���������ֽڵ����ͳ���
	psSOperand exprValueOp;
	GetIntOperand( &exprBC, exprValue , exprValueOp);

	// �ͷ���ʱ������
	ReleaseTemporaryVariable(exprValue);

	// ����������ʽ�����ɵ���ʱ����
	ClearTemporaryVariables(&exprBC);

	// ����������ʽ�ĵĶ����ƴ���
	bc->AddCode(&exprBC);

	// ������ʽ��ֵΪ�棬���ص��ж����
	bc->Instr2(BC_JNZ, exprValueOp, psSOperand(beforeLabel));
	if ( exprValueOp != exprValue.operand )
		ReleaseTemporaryOperand( exprValueOp );

	// ���ѭ������
	bc->Label(afterLabel);

	// ���������break/continue��ǩ
	m_ContinueLabels.pop_back();
	m_BreakLabels.pop_back();

	// �Ƴ�������
	int r;
	m_pScript->ConvertPosToRowCol(wnode->GetTokenPos() + wnode->GetTokenLength(), &r, 0);
	CreateDebugSymbols(m_Variables, r);
	RemoveVariableScope();
}

void psCCompiler::CompileReturnStatement(psCScriptNode* rnode, psCByteCode* bc)
{
	// NOTE: ����֮����һ��retBC���洢����ֵ�Ķ����ƴ�������Ϊ:
	//       �����ֲ�����ʱ���Ի���д����ֵ
	psCByteCode retBC;

	// ���һ����ָ��
	InstrLINE(bc, rnode->GetTokenPos());

	// �õ�����ֵ����
	if ( m_ReturnType.GetSizeOnStackDWords() == 0 )
	{
		if (rnode->GetFirstChild())
			Error(TXT_CANT_RETURN_VALUE, rnode);
	}else
	{
		// ����ֵ�Ǳ��ʽֵ
		if( rnode->GetFirstChild() )
		{	
			// ������ʽ��ֵ
			psSValue expr;
			CompileAssignment(rnode->GetFirstChild(), bc, &expr);

			// ���Ա����Ƿ��Ѿ���ʼ��
			IsVariableInitialized(expr, rnode);

			if (!ImplicitConversion(expr, m_ReturnType, expr, bc, rnode, false))
			{
				psCString str;
				str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, expr.type.GetFormat().AddressOf(), m_ReturnType.GetFormat().AddressOf());
				Error(str.AddressOf(), rnode);
			}

			// ���������,��ֵ��������ʱ����
			if ( m_ReturnType.IsReference() )
			{
				if (IsTemporaryVariable(expr))
				{
					Error(TXT_REF_CANNOT_BE_TEMPORARY, rnode);
				}
			}

			if ( m_ReturnType.IsObject() )
			{
				if ( m_ReturnType.IsReference() )
				{
					if (expr.type.IsReference())
					{
						// ֱ�ӰѶ���ָ�븳������ֵ
						retBC.Instr1(BC_SRET4, expr.operand );
					}else
					{
						// ��Ҫ���������ַ
						psSValue tmp(g_IntDataType);
						AllocateTemporaryVariable(tmp);
						retBC.Instr2(BC_DEFREF, tmp.operand, expr.operand);
						retBC.Instr1(BC_SRET4, tmp.operand);
						ReleaseTemporaryVariable(tmp);
					}
				}else
				{
					// �������ֵ��������Ҳ�������
					// ��ô��һ������Ϊ�䷵��������ָ��
					psSValue retObj( m_ReturnType );
					retObj.type.IsPointer()  = true;
					retObj.type.IsReference() = true;

					if ( m_ObjectType )
					{
						retObj.operand.BaseOffset( PSBASE_STACK, 4 );
					}else
					{
						retObj.operand.BaseOffset( PSBASE_STACK, 0 );
					}
					// ����������������
					CopyConstructObject(retObj, expr, bc, rnode);

					// д����ֵ
					retBC.Instr1(BC_SRET4, retObj.operand );
				}
			}else
			{
				// ��д����ֵ
				if ( expr.type.IsReference() && !m_ReturnType.IsReference() )
				{
					// ����һ��ʱ����, ����������ֵ
					psSValue retValue(m_ReturnType);
					AllocateTemporaryVariable( retValue );

					InstrRDREF( &retBC,  retValue.operand, expr.operand, m_ReturnType );
					InstrSRET(  &retBC,  retValue.operand, m_ReturnType );

					// �ͷ���ʱ����
					ReleaseTemporaryVariable( retValue );
				}else if ( !expr.type.IsReference() && m_ReturnType.IsReference() )
				{
					// ����һ��ʱ����, ��������ֵַ
					psSValue retValue(expr.type);
					retValue.type.IsPointer() = true;

					AllocateTemporaryVariable(retValue);

					retBC.Instr2(BC_DEFREF, retValue.operand, expr.operand );
					retBC.Instr1(BC_SRET4,  retValue.operand );

					// �ͷ���ʱ����
					ReleaseTemporaryVariable( retValue );
				}else
				{
					InstrSRET( &retBC, expr.operand, m_ReturnType);
				}
			}			

			// �ͷ���ʱ����
			ReleaseTemporaryVariable(expr);

			// �����ʱ����
			// NOTE: Ҫ��д����ֵ֮����ǰ,��Ϊ�п��ܻ������������,
			//	     ��ʱ�пɰ�return�Ĵ�����ֵ������.
			ClearTemporaryVariables(bc);
		}else
		{
			Error(TXT_MUST_RETURN_VALUE, rnode);
		}
	}

	// �������еľֲ�����,��������
	psCVariableScope *vs = m_Variables;
	while( vs ) 
	{
		psCVariableScope::VariableMap_t::const_reverse_iterator iter = vs->GetVariableMap().rbegin();
		for (; iter != vs->GetVariableMap().rend(); ++iter)
		{
			const psSVariable* var = iter->second;	

			// �������еľֲ������(��Ҫ�������ñ���,����ֵ����)
			if (var->value.operand.offset < 0 &&
				var->value.operand.base == PSBASE_STACK  )
			{
				CompileDestructor(var->value, bc);
			}
		}
		vs = vs->GetParent();
	} 

	// ������÷���ֵ�Ĵ���
	bc->AddCode(&retBC);

	// ��������
	bc->Instr1(BC_JMP, psSOperand(0));
}

void psCCompiler::CompileBreakStatement(psCScriptNode* node, psCByteCode* bc)
{
	// ���һ����ָ��
	InstrLINE(bc, node->GetTokenPos());

	// ���'break��ǩû�����ݣ���ô����һ����Ч��'break'
	if (m_BreakLabels.size() == 0)
	{
		Error(TXT_INVALID_BREAK, node);
		return;
	}

	// �������еľֲ�����
	psCVariableScope *vs = m_Variables;
	while( vs && !vs->IsBreakScope() ) 
	{
		psCVariableScope::VariableMap_t::const_reverse_iterator iter = vs->GetVariableMap().rbegin();
		for (; iter != vs->GetVariableMap().rend(); ++iter)
		{
			const psSVariable* var = iter->second;		
			// �������еľֲ������(��Ҫ�������ñ���)
			if (var->value.operand.offset < 0 &&
				var->value.operand.base == PSBASE_STACK  )
			{
				CompileDestructor(var->value, bc);
			}
		}
		vs = vs->GetParent();
	} 

	bc->Instr1(BC_JMP, psSOperand(m_BreakLabels[m_BreakLabels.size() - 1]));
}

void psCCompiler::CompileContinueStatement(psCScriptNode* node, psCByteCode* bc)
{
	// ���һ����ָ��
	InstrLINE(bc, node->GetTokenPos());

	// ���'continue'��ǩû�����ݣ���ô����һ����Ч��'continue'
	if (m_ContinueLabels.size() == 0)
	{
		Error(TXT_INVALID_CONTINUE, node);
		return;
	}

	// �������еľֲ�����
	psCVariableScope *vs = m_Variables;
	while( vs && !vs->IsContinueScope() ) 
	{
		psCVariableScope::VariableMap_t::const_reverse_iterator iter = vs->GetVariableMap().rbegin();
		for (; iter != vs->GetVariableMap().rend(); ++iter)
		{
			const psSVariable* var = iter->second;			
			// �������еľֲ������(��Ҫ�������ñ���)
			if (var->value.operand.offset < 0 &&
				var->value.operand.base == PSBASE_STACK  )
			{
				CompileDestructor(var->value, bc);
			}
		}
		vs = vs->GetParent();
	} 

	bc->Instr1(BC_JMP, psSOperand(m_ContinueLabels[m_ContinueLabels.size()-1]));
}


void psCCompiler::CompileFunctionCall(psCScriptNode* node, psCByteCode* bc, psSValue* retValue, const psSValue* object)
{
	psCObjectType* objectType = NULL;
	if ( object )
	{
		objectType = object->type.ObjectType();
	}

	// ��һ������Ǻ���������������,������
	psCHAR name[256];
	psCHAR field[256];
	bool isFunction	   = false;
	bool isConstructor = false;	

	psCObjectType* objField = (node->GetTokenType() == ttField) ? NULL : m_ObjectType;

	// �õ����ƽ��
	psCScriptNode* nm = node->GetFirstChild();
	if ( nm->GetNextNode() && nm->GetNextNode()->GetNodeType() == snIdentifier )
	{
		// �õ�����
		GetNodeSource( field, 255, nm );
		objField = m_pBuilder->GetObjectType( field );
		if ( objField == NULL )
		{
			psCString msg;
			msg.Format( TXT_IDENTIFIER_s_NOT_DATA_TYPE, field );
			Error( msg.AddressOf() , node);
		}
		nm = nm->GetNextNode();
	}

	// �õ�������
	GetNodeSource( name, 255, nm );

	// ��������Ʋ������������Ǻ�����
	if ( m_pBuilder->GetObjectType( name ) == NULL )
	{
		// ������ຯ�����ã��õ���ĳ�Ա����
		// ����õ�ȫ�ֺ���
		isFunction = true;
	}

	psCScriptNode* argList = node->GetLastChild();	
	// ׼����������	
	std::vector< int >	   funcs;
	std::vector<int>	   matches;
	std::vector<int>	   exactmatches;
	std::vector<psSValue>  argValues;
	psCByteCode			   argBC;
	CompileArgumentList( argList, &argBC, argValues );

	int baseOffset = 0;
	if ( isFunction )
	{
		// ��ȡ��ȷ�ĺ�����
		psCObjectType* curObjType = objectType;
		psSOperand     objPointer;
		if ( curObjType == NULL )
		{
			curObjType = m_ObjectType;
			while( curObjType )
			{
				if ( curObjType == objField )
					break;
				curObjType = curObjType->m_pParent;
			}
		}
		// ����ƥ��ĺ���
		while ( curObjType )
		{
			// �෽��
			m_pBuilder->GetObjectMethodDescrptions( name, curObjType, funcs );

			// ��ȷƥ��
			MatchFunctionCall( funcs, exactmatches, argValues, true );
			if ( exactmatches.size() != 0 )
				break;

			// ��ȫƥ��
			if ( matches.size() == 0 )
			{
				MatchFunctionCall( funcs, matches, argValues, false );
			}

			baseOffset += curObjType->m_BaseOffset;
			curObjType  = curObjType->m_pParent;
			funcs.clear();
		}

		if ( objectType == NULL && matches.size() == 0 ) 
		{
			// ƥ��ȫ�ֺ���
			m_pBuilder->GetFunctionDescriptions( name, funcs );
			MatchFunctionCall( funcs, matches, argValues, false );
		}
	}else
	{
		// ������Ǻ�������,���п���������ת��
		psCObjectType* ot = m_pBuilder->GetObjectType( name );
		if ( ot == NULL )
		{
			Error(TXT_ILLEGAL_CALL, node);
			return;
		}

		// ����һ�����캯��
		funcs = ot->m_Beh.constructors;

		// ���캯�����ܱ���ʾ���෽������ʽ�ĵ���
		if( objectType )
		{
			Error(TXT_ILLEGAL_CALL, node);
		}
		else
		{
			isConstructor = true;

			// ���ú������ص�����
			retValue->type.SetObjectType( ot );
			retValue->type.m_TokenType = ttIdentifier;
			retValue->type.IsReference() = false;
			retValue->type.IsPointer()   = false;

			if ( ot->m_Beh.isInterface )
			{
				psCString msg;
				msg.Format( TXT_OBJECT_s_CANNOT_BE_INSTANCE, ot->m_Name.AddressOf() );
				Error( msg.AddressOf(), node );
			}else
			{		
				// ����һ����ʱ����
				AllocateTemporaryVariable(*retValue);

				if ( !ot->m_Beh.hasConstructor && argList->GetFirstChild() == NULL )
				{
					// ���һ�������û�в���Ϊ�յĹ��캯��, �����һ����ʱ����������,
					// ���õ����乹�캯��
					return;
				}
			}
		}
		MatchFunctionCall( funcs, matches, argValues, false );
	}
	psCScriptFunction *func = NULL;
	if ( exactmatches.size() == 1 )
	{
		func = m_pBuilder->GetFunctionDescription(exactmatches[0]);
	}else if ( exactmatches.size() == 0 && matches.size() == 1)
	{
		func = m_pBuilder->GetFunctionDescription(matches[0]);
	}

	if ( func && func->IsArgVariational()  )
	{	
		// ��������˱�κ�������
		// ���ñ�κ����������������ͼ��
		PS_ARGTYPE_CHECK_FUNC pTypeCheckFunc = m_pBuilder->GetArgTypeCheckFunc();
		if ( pTypeCheckFunc && (int)argValues.size() < PS_MAX_VAR_ARG_SIZE )
		{
			const psIDataType* argTypes[PS_MAX_VAR_ARG_SIZE];
			const char*		   argConst[PS_MAX_VAR_ARG_SIZE];
			psSValue		   tmpArgValue[PS_MAX_VAR_ARG_SIZE];
			size_t i = 0;
			for ( ; i < func->m_ParamTypes.size(); ++i )
			{
				bool bOk = ImplicitConversion( argValues[i], func->m_ParamTypes[i], tmpArgValue[i], NULL, argList, false );
				assert( bOk );
			}
			for ( ; i < argValues.size(); ++i )
			{
				tmpArgValue[i] = argValues[i];
			}
			for ( i = 0; i < argValues.size(); ++i )
			{
				psCDataType& type = tmpArgValue[i].type;
				argTypes[i] = (psIDataType*)(&type);
				if ( argValues[i].IsConstant() )
				{
					//int offset = argValues[i].operand.offset;
					argConst[i] = (char*)m_pBuilder->GetConstantPtr( tmpArgValue[i].operand.offset );
				}else
				{
					argConst[i] = NULL;
				}
			}
			if ( !(*pTypeCheckFunc)( func, argTypes, argConst, argValues.size() ) )
			{
				func = NULL;
				matches.clear();
			}
		}
	}

	if ( func )
	{
		// ��������
		bc->AddCode( &argBC );
		int argSize = PushArgumentList( argList, func, bc, argValues );

		if ( isConstructor )
		{
			CallObjectMethod( *retValue, func->m_ID, argSize, bc );
		}else
		{
			if ( func->m_ObjectType != NULL )
			{
				psSValue objValue;
				objValue.type.SetObjectType( func->m_ObjectType );
				objValue.type.IsPointer()   = true;
				if ( object )
				{
					if ( baseOffset == 0 )
					{
						objValue.AssignExceptType( *object );
						objValue.type.IsReference() = object->type.IsReference();
					}else
					{
						objValue.type.IsReference() = true;
						AllocateTemporaryVariable( objValue );
						bc->Instr3(BC_ADDi, objValue.operand, object->operand, MakeIntValueOp(baseOffset) );
					}
				}else
				{
					objValue.type.IsReference() = true;
					objValue.type.IsPointer()   = true;
					objValue.operand.BaseOffset( PSBASE_STACK, 0 );
					if ( baseOffset != 0 )
					{
						AllocateTemporaryVariable( objValue );
						bc->Instr3(BC_ADDi, objValue.operand, objValue.operand, MakeIntValueOp(baseOffset) );
					}
				}
				PerformFunctionCall( func->m_ID, argSize, retValue, bc, node, &objValue );

				// �ͷ���ʱ����
				if ( object == NULL || object->operand != objValue.operand )
					ReleaseTemporaryVariable( objValue );
			}else
			{
				PerformFunctionCall( func->m_ID, argSize, retValue, bc, node, NULL );
			}
		}
	}else
	{	
		psCString strType(name);
		strType   += BuildArgListString(argValues);

		psCString msg;
		if ( matches.size() > 0 )
			msg.Format(TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s, strType.AddressOf() );
		else
			msg.Format(TXT_NO_MATCHING_SIGNATURES_TO_s, strType.AddressOf() );

		// Dummy value	
		retValue->type = psCDataType(ttInt, true, false);
		retValue->operand = MakeIntValueOp(0);

		Error( msg.AddressOf(), node );
	}
}


void psCCompiler::PerformFunctionCall(int funcID, int argSize, psSValue* retValue, psCByteCode* bc, psCScriptNode*node, const psSValue* object )
{
	psCScriptFunction *desc = m_pBuilder->GetFunctionDescription(funcID);
	assert( desc );

	psSValue tmpValue(desc->m_ReturnType);
	if ( desc->m_ReturnType.IsObject() && !desc->m_ReturnType.IsReference() )
	{
		// �������ֵΪ�����,���Ҳ�������ֵ�Ļ�,
		// �����ڶ�ջ�Ϸ��������ʱ����󲢰���������һ����������
		AllocateTemporaryVariable( tmpValue );

		bc->PushRef( tmpValue.operand );

		// NOTE: ��ʱ�����Ĵ�СӦ�üӣ�
		argSize++;
	}

	bool bObject = false;

	// Push this pointer
	if ( (object && object->type.IsObject() ) )
	{
		psCObjectType* objType = object->type.ObjectType();
		assert( objType );
		if ( object->type.IsReference() )
		{	
			bc->Push4( object->operand );
		}else
		{
			bc->PushRef( object->operand );
		}

		bObject = true;
	}

	// �ű�������IDӦ�ô��ڵ���0,���������IDС��0����ϵͳ����
	if (desc->m_ID < 0)
	{
		// ����ϵͳ����
		bc->CallSys( desc->m_ID, argSize, bObject  );
	}else
	{
		// ���ýű�����(����������һ���ű�ģ��)
		bc->Call( desc->m_ID, argSize, bObject  );
	}

	if (retValue)
	{		
		retValue->type = desc->m_ReturnType;

		if (desc->m_ReturnType.GetSizeOnStackDWords() == 0 )
		{
			retValue->operand.BaseOffset( PSBASE_NONE, 0 );
		}else
		{
			if (desc->m_ReturnType.IsObject())
			{
				if ( !desc->m_ReturnType.IsReference() )
				{	
					retValue->AssignExceptType( tmpValue );
				}else
				{		
					// �������ֵ�����������
					// ��ȡ���ֵַ,����������������Ϊ��

					// ����һ����ʱ������
					AllocateTemporaryVariable(*retValue);

					// ��ȡ����ֵ
					bc->Instr1(BC_RRET4, retValue->operand);
				}
			}else
			{		
				// ����һ����ʱ������
				AllocateTemporaryVariable(*retValue);

				// ��ȡ����ֵ
				InstrRRET(bc, retValue->operand, retValue->type);
			}
		}
	}else
	{
		// �ͷ���ʱ����
		ReleaseTemporaryVariable( tmpValue );
	}
}

void psCCompiler::CompileArgumentList( psCScriptNode* argListNode, psCByteCode* bc, std::vector<psSValue>& argValues )
{
	psCScriptNode* arg = argListNode->GetFirstChild();
	size_t argindex = 0;
	while ( arg )
	{
		argindex++;
		arg = arg->GetNextNode();
	}
	argValues.resize( argindex );

	// ���ҵ����������б�
	arg = argListNode->GetLastChild();
	argindex--;

	while ( arg )
	{ 
		// 
		CompileAssignment( arg, bc, &argValues[argindex] );

		// �Ƶ���һ������
		argindex--;
		arg = arg->GetPrevNode();
	}
}

void psCCompiler::MatchFunctionCall(const std::vector<int>& allfuncs, std::vector<int>& matches, const std::vector<psSValue>& argValues, bool isExactMatch )
{
	assert( &allfuncs != &matches );

	// ���������һ�����Ҳ��Ǳ�ε���,�߳�ȥ
	std::vector< int > funcs;
	for( size_t n = 0; n < allfuncs.size(); ++n )
	{
		psCScriptFunction *desc = m_pBuilder->GetFunctionDescription(allfuncs[n]);
		assert(desc);

		if( desc->m_ParamTypes.size() == argValues.size() || desc->IsArgVariational() )
		{
			funcs.push_back( allfuncs[n] );
		}
	}

	matches = funcs;
	//��ƥ�亯������	
	for (size_t i = 0; i < argValues.size(); ++i )
	{
		// ����ƥ��
		MatchArgument( (int)i, argValues[i], funcs, matches, isExactMatch );

		// ���º�ѡ����
		funcs = matches;
	}
}

int psCCompiler::PushArgumentList( psCScriptNode* argListNode, psCScriptFunction* func, psCByteCode* bc, const std::vector<psSValue>& argValues )
{
	assert( func->IsArgVariational() || func->m_ParamTypes.size() == argValues.size() );


	int argSize = 0;

	// ���ҵ���Ѳ�����ջ
	size_t i = argValues.size();

	// ��β��ֵĲ�����ջ
	for ( ; i >  func->m_ParamTypes.size(); --i )
	{
		const psSValue& argValue = argValues[i - 1];

		// �������������Ƿ�Ϊָ������䴫��Ĳ����Ƿ�Ϊ��ֵַ
		psCDataType type( argValue.type );
		if ( type.IsPointer() )
			type.IsReference() = true;
		else
			type.IsReference() = false;

		if ( type.IsFloatType() && !type.IsReference() )
		{
			// ע: ��C++�����еı�κ����еı�ζ�����double��ջ��
			type.TokenType() = ttDouble;

			psSValue tmpValue( type );
			AllocateTemporaryVariable( tmpValue );

			if (argValue.type.IsReference())
			{	
				// �����������������Ҫ�Ƚ�����
				bc->Instr2(BC_RDREF4, tmpValue.operand, argValue.operand);
				bc->Instr2(BC_F2D, tmpValue.operand, tmpValue.operand);
			}else
			{
				bc->Instr2( BC_F2D, tmpValue.operand, argValue.operand );
			}
			PushArgument( tmpValue, type, bc, argListNode );

			ReleaseTemporaryVariable( tmpValue );
		}else
		{
			PushArgument( argValue, type, bc, argListNode );
		}

		// ���²�����С
		argSize += type.GetSizeOnStackDWords();

		ReleaseTemporaryVariable( argValue );
	}

	// ����������ջ
	for ( i = func->m_ParamTypes.size(); i > 0; --i )
	{
		psCDataType &to			 = func->m_ParamTypes[i - 1];
		const psSValue& argValue = argValues[ i - 1];

		bool reportWarning = true;
		if ( to.IsReference() && !to.IsPointer() )
			reportWarning = false;
		IsVariableInitialized( argValue, argListNode, reportWarning);

		psSValue matchedArg;
		// ��һ����ʽת�����϶�Ӧ�óɹ�
		// ��Ϊ���Ѿ�ͨ������ƥ����
		bool bOk = ImplicitConversion( argValue, to, matchedArg, bc, argListNode, false);
		assert( bOk );

		// ���²�����С
		argSize += to.GetSizeOnStackDWords();

		// �Ѳ�����ջ
		PushArgument( matchedArg, to, bc, argListNode );	

		// �ͷ���ʱ����	
		if ( matchedArg.operand != argValue.operand)
		{
			ReleaseTemporaryVariable( matchedArg );
		}
		ReleaseTemporaryVariable( argValue );
	}
	return argSize;
}

psCString psCCompiler::BuildArgListString(const std::vector<psSValue>& argValues)
{
	// ����һ���ɶ��Ĳ����б�
	psCString strType( PS_T("(" ));

	for (int n = 0; n < (int)argValues.size() - 1; ++n)
	{
		strType += argValues[n].type.GetFormat();

		strType += PS_T(",");
	}		

	if (argValues.size() > 0)
	{
		strType += argValues[argValues.size() - 1].type.GetFormat();
	}
	strType += PS_T(")");

	return strType;
}

void psCCompiler::MatchArgument(int argIndex, const psSValue& argValue, const std::vector<int>& funcs, std::vector<int>& matches, bool isExactMatch)
{	
	bool bIsMatchArgumentOld = m_bIsMatchArgument;
	m_bIsMatchArgument = true;

	bool isMatchExceptConst = false;
	bool isMatchWithBaseType = false;
	//	bool isMatchExceptSign = false;

	matches.clear();
	for (size_t n = 0; n < funcs.size(); ++n)
	{
		psCScriptFunction *desc = m_pBuilder->GetFunctionDescription(funcs[n]);
		assert(desc);

		if ( argIndex >= int(desc->m_ParamTypes.size()) )
		{
			// ��������������ں����Ĳ�������,��ô�˺�����Ϊ��κ���
			assert(desc->IsArgVariational() );
			matches.push_back(funcs[n]);
			continue;
		}

		psCDataType& to = desc->m_ParamTypes[argIndex];

		psSValue tmp;
		// ����һ����ʽת��
		if (!ImplicitConversion(argValue, to, tmp, NULL, NULL, false))
			continue;

		if (to.IsEqualExceptRefAndConst(tmp.type))
		{
			if (tmp.type.IsEqualExceptRef(argValue.type))
			{
				if (!isExactMatch) matches.clear();
				isExactMatch = true;
				matches.push_back(funcs[n]);
				continue;
			}
			if (!isExactMatch)
			{
				// Is it a match except const?
				if (tmp.type.IsEqualExceptRefAndConst(argValue.type))
				{
					if (!isMatchExceptConst) matches.clear();
					isMatchExceptConst = true;
					matches.push_back(funcs[n]);
					continue;
				}
				if( !isMatchExceptConst )
				{
					// Is it a size promotion, e.g. int8 -> int?
					if (to.IsSameBaseType(argValue.type))
					{
						if( !isMatchWithBaseType ) matches.clear();

						isMatchWithBaseType = true;

						matches.push_back(funcs[n]);
						continue;
					}

					if( !isMatchWithBaseType )
					{
						matches.push_back(funcs[n]);
					} // end if( !isMatchWithBaseType )
				}// end if( !isMatchExceptConst )
			} // end if (!isExactMatch)
		}// end if (to.IsEqualExceptRefAndConst(argValue.type))
	}

	m_bIsMatchArgument = bIsMatchArgumentOld;
}

void psCCompiler::PushArgument(const psSValue& argValue, const psCDataType& to, psCByteCode* bc, psCScriptNode* node )
{
	if ( argValue.type.IsArrayType() )
	{
		// ���������������һ���������÷�ʽ���ݵ�
		if ( argValue.type.IsReference() )
		{
			bc->Push4( argValue.operand );
		}else
		{
			bc->PushRef( argValue.operand );
		}
	}else if (argValue.type.IsObject())
	{
		if ( to.IsReference() )
		{
			// ����������ô��ݣ�ֱ��'push'���ֵַ
			if ( argValue.type.IsReference() )
				bc->Push4(argValue.operand);
			else
				bc->PushRef(argValue.operand);
		}else
		{
			// ����һ����ʱ�������,
			psSValue tmpObj( to );
			tmpObj.type.IsPointer()  = true;
			tmpObj.type.IsReference() = true;

			AllocateTemporaryVariable( tmpObj );

			bc->Push( to.GetTotalSizeOnStackDWords() );
			bc->Instr1(BC_PSP, tmpObj.operand );

			CopyConstructObject( tmpObj, argValue, bc, node);

			ReleaseTemporaryVariable( tmpObj );
		}

	}else
	{
		// push argument
		if ( !to.IsReference() )
		{		
			psSValue tmpArg = argValue;
			if ( argValue.type.IsReference() )
			{
				// ����һ����ʱ�����������ֵַ
				tmpArg.type.IsPointer() = false;
				tmpArg.type.IsReference() = false;

				AllocateTemporaryVariable(tmpArg);
				InstrRDREF(bc, tmpArg.operand, argValue.operand, argValue.type);	

				// �ͷ���ʱ����
				ReleaseTemporaryVariable(tmpArg);
			} 

			int s = tmpArg.type.GetSizeInMemoryBytes();
			if ( s == 1 )
			{
				psSValue tmp1;
				AllocateTemporaryVariable(tmp1);
				bc->Instr2( tmpArg.type.IsUnsignedInteger() ? BC_B2I : BC_C2I, tmp1.operand, tmpArg.operand );

				bc->Push4( tmp1.operand );

				// �ͷ���ʱ����
				ReleaseTemporaryVariable(tmp1);
			}else if ( s == 2 )
			{
				psSValue tmp1;
				AllocateTemporaryVariable(tmp1);

				bc->Instr2( tmpArg.type.IsUnsignedInteger() ? BC_W2I : BC_S2I, tmp1.operand, tmpArg.operand );	
				bc->Push4( tmp1.operand );

				// �ͷ���ʱ����
				ReleaseTemporaryVariable(tmp1);
			}else if ( s == 4 )
			{
				bc->Push4( tmpArg.operand );
			}else if ( s == 8 )
			{
				bc->Push8( tmpArg.operand );
			}else
			{
				assert(m_bHasCompileError);
			}
		}else
		{
			if ( argValue.type.IsReference() )
			{
				assert( argValue.type.IsPrimitive() );
				//(�������������,����ֵ�Ѿ��ǵ�ֵַ)
				bc->Push4(argValue.operand);
			}else
			{
				// push variable reference
				bc->PushRef(argValue.operand);
			}
		}
	}
}

//-------------------------------------------------------------------
// ����: IsLValue
// ˵��: 
//-------------------------------------------------------------------
bool psCCompiler::IsLValue(const psSValue& value)
{
	// ���������ֻ���ֻ����
	if ( value.type.IsArrayType()	)	  {	return false;	}
	if ( value.type.IsPointer() ) 
	{
		return !value.isTemporary;
	}else
	{
		if ( value.type.IsReadOnly()  )	  { return false;   }

		// ��������ʱ��������������ֵ
		if ( value.IsConstant()		 ||
			IsTemporaryVariable(value) )
		{
			return false;
		}
		return true;
	}
}

//-------------------------------------------------------------------
// ����: IsTemporaryVariable
// ˵��: 
//-------------------------------------------------------------------
bool psCCompiler::IsTemporaryVariable(const psSValue& value)
{
	if ( (value.operand.base == PSBASE_STACK && value.isTemporary ) )
	{
		if ( ( !value.type.IsPointer() && !value.type.IsReference() ) )
			return true;
		else
			return false;
	}else
	{
		return false;
	}
}

int psCCompiler::AllocateVariable(const psCDataType& type, bool tmp)
{
	if ( m_bIsMatchArgument )
	{
		// �����������ƥ��,���÷������ʱ�ռ�
		return 0x7FFFFFFF;
	}

	// NOTE:��Ҫ�ò�ͬ���͵�����ʹ��ͬ������ʱ�ռ�
	// Ҳ��Ҫ�ò�ͬ�������͵���������ʹ��ͬ������ʱ�ռ�
	// i.e.�����ʱ��������ʱ����ʹ��ͬ���ı���,
	//	   �������������ͷ����������ʹ��ͬ���ı���
	//	   ����쳣����ʱ�ͷ����������
	psCDataType t(type);

	int totalSize = t.GetTotalSizeOnStackDWords();
	if ( t.IsPrimitive() )
	{
		if ( totalSize == 1 ) 
			t.TokenType() = ttInt;
		else if ( totalSize == 2 )
			t.TokenType() = ttDouble;
	}

	for (size_t i = 0; i < m_FreeVariables.size(); ++i)
	{
		int offset =  m_FreeVariables[i];
		int slot = GetVariableSlot(offset);
		if (slot >= 0) 
		{
			psCDataType& test_type = m_VariableAllocations[slot].first;
			if ( test_type.IsEqualExceptConst(t) && 
				test_type.IsReference() == t.IsReference() )
			{
				m_FreeVariables[i] = m_FreeVariables.back();
				m_FreeVariables.pop_back();

				if ( tmp )
				{
					m_TempVariables.push_back(offset);
				}
				return offset;
			}
		}
	}	

	// NOTE: �������͵Ĵ�С�������0,������Ǳ����ķ�����ܻ����
	assert ( totalSize != 0 || m_bHasCompileError );

	m_StackPos -= totalSize;

	int offset = m_StackPos * int( sizeof(psDWORD) );
	m_VariableAllocations.push_back( std::make_pair( t, offset ) );
	if ( tmp )
	{
		m_TempVariables.push_back( offset );
	}
	return offset;
}


int psCCompiler::GetVariableSlot(int offset)
{
	for( int n = 0; n < (int)m_VariableAllocations.size(); n++ )
	{
		if( m_VariableAllocations[n].second == offset  )
		{
			return n;
		}
	}
	return -1;
}

void psCCompiler::ReleaseTemporaryVariable( const psSValue& v )
{	
	if ( v.type.IsObject() && !v.type.IsReference() && v.isTemporary )
	{
		if ( v.operand.base   != PSBASE_STACK || 
			v.operand.offset == 0x7FFFffff )
			return;

		// �����ʱ�����Ķ�ջƫ��Ϊ0x7FFFFFFF, ���ʽ������������ƥ��ʱ������
		// �������ʱ�����, �ȵ��������ڽ���ʱ���ͷ���
		m_TempValues.push_back( v );
	}else
	{
		ReleaseTemporaryOperand( v.operand );
	}
}

void psCCompiler::ReleaseTemporaryOperand( const psSOperand& op )
{
	if ( op.base != PSBASE_STACK || 
		op.offset == 0x7FFFffff )
		return;

	// �ͷ���ʱ������
	for ( size_t i = 0; i < m_TempVariables.size(); ++i )
	{
		if (op.offset == m_TempVariables[i])
		{
			m_FreeVariables.push_back(m_TempVariables[i]);

			m_TempVariables[i] = m_TempVariables.back();
			m_TempVariables.pop_back();
			return;
		}
	}
}

void psCCompiler::ClearTemporaryVariables(psCByteCode* bc)
{
	assert( bc );

	for (size_t i = 0; i < m_TempValues.size(); ++i )
	{
		const psSValue& v = m_TempValues[i];

		// ������������
		CompileDestructor( v, bc );

		ReleaseTemporaryOperand( v.operand );
	}
	m_TempValues.clear();

}

void psCCompiler::AddVariableScope(bool bIsBreakScope, bool bIsContinueScope)
{
	m_Variables = new psCVariableScope(m_Variables);
	m_Variables->IsBreakScope() = bIsBreakScope;
	m_Variables->IsContinueScope() = bIsContinueScope;
}

void psCCompiler::CreateDebugSymbols( psCVariableScope* varscope, int curline )
{
	if (m_pBuilder->GetOptimizeOptions().bCreateSymbolTable)
	{	
		// ���ɷ��ű�(���ڵ�����)
		psSDebugSymbol symbol;
		psCVariableScope::VariableMap_t::const_reverse_iterator it;
		it = varscope->GetVariableMap().rbegin();
		for (; it != varscope->GetVariableMap().rend(); ++it)
		{
			const psSVariable* var = it->second;
			if (var->value.operand.base == PSBASE_STACK)
			{
				symbol.name = var->name;
				symbol.type = var->value.type;
				symbol.offset = var->value.operand.offset;
				symbol.start = var->value.startline;
				symbol.end = curline;

				m_DebugSymbols.push_back(symbol);
			}
		}
	}
}

void psCCompiler::RemoveVariableScope()
{
	if (m_Variables)
	{
		psCVariableScope* varscope = m_Variables;
		m_Variables = m_Variables->GetParent();

		delete varscope;
	}
}


bool psCCompiler::IsVariableInitialized(const psSValue& var, psCScriptNode* node, bool reportWarning )
{
	// ���Ǳ���,����������ö�����Ϊ���Ѿ���ʼ������
	if ( var.type.IsReference() ||
		 var.type.IsArrayType() ||
		 var.type.IsObject()    ||
		 IsTemporaryVariable(var) )
	{
		return true;
	}

	if (var.operand.base != PSBASE_STACK)
		return true;

	// ���Ҿֲ�����
	psSVariable *v = m_Variables->GetVariableByOffset(var.operand.offset);
	if (v)
	{
		if ( !v->value.type.IsObject() && !v->value.isInitialized && reportWarning )
		{
			psCString str;
			str.Format(TXT_s_NOT_INITIALIZED, v->name.AddressOf() );
			Warning(str.AddressOf(), node);	
		}		

		//�������ó�ʼ����־,������ֻ�ᾯ��һ��
		v->value.isInitialized = true;

		return false;
	}else
	{
		return true;
	}
}

void psCCompiler::GetIntOperand( psCByteCode* bc, const psSValue& value, psSOperand& intOperand )
{
	int s = value.type.GetSizeInMemoryBytes();
	if ( !value.type.IsReference() && s == 4 )
	{
		intOperand = value.operand;
		return;
	}

	intOperand.BaseOffset( PSBASE_STACK, AllocateVariable( g_IntDataType, true) );
	// NOTE: ������ʽ��ֵ����������,Ҫ��������ֵ
	// ������ʽ��ֵ��������char����shortҪת��Ϊint
	// ����,TNZi/TZi�Ľ���᲻ȷ.
	if ( value.type.IsReference() )
	{
		InstrRDREF( bc, intOperand, value.operand, value.type );	

		if ( s == 1 )
		{
			bc->Instr2(value.type.IsUnsignedInteger() ? BC_B2I : BC_C2I, intOperand, intOperand );
		}else if ( s == 2)
		{
			bc->Instr2(value.type.IsUnsignedInteger() ? BC_W2I : BC_S2I, intOperand, intOperand );
		}
	}else
	{
		if ( s == 1 )
		{
			bc->Instr2(value.type.IsUnsignedInteger() ? BC_B2I : BC_C2I, intOperand, value.operand );
		}else if ( s == 2)
		{
			bc->Instr2(value.type.IsUnsignedInteger() ? BC_W2I : BC_S2I, intOperand, value.operand );
		}
	}
}

void psCCompiler::GetNodeSource( psCHAR* name, int max_len, psCScriptNode* n )
{
	int length = n->GetTokenLength();

	// �õ�����
	if ( length > max_len )
	{
		length = max_len; 
	}
	memcpy(name, &m_pScript->m_Code[n->GetTokenPos()], length * sizeof(psCHAR) );
	name[length] = 0;

	if ( n->GetTokenLength() > max_len )
	{
		psCString msg;
		msg.Format( TXT_NAME_s_IS_TOO_LONG, name );
		Warning( msg.AddressOf(), n );
	}
}


void psCCompiler::Warning(const psCHAR *msg, psCScriptNode *node)
{
	assert(m_pBuilder && m_pScript);

	if (node == NULL || msg == NULL)
	{
		// NOTE:����һ�ֺϷ����
		// ��������ƥ��ʱ,���ܻ������ʽת������,����ʱ���ָ��Ϊ��
		return;
	}

	psCString str;

	int r, c;
	psCScriptCode* code = m_pScript->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

	m_pBuilder->WriteWarning(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), msg, r, c);
}

void psCCompiler::Error(const psCHAR *msg, psCScriptNode *node)
{	
	assert(m_pBuilder && m_pScript);
	if (node == NULL || msg == NULL)
	{	
		// NOTE:����һ�ֺϷ����
		// ��������ƥ��ʱ,���ܻ������ʽת������,����ʱ���ָ��Ϊ��
		return;
	}

	psCString str;

	int r, c;
	psCScriptCode* code =m_pScript->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

	m_pBuilder->WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), msg, r, c);

	m_bHasCompileError = true;
}

