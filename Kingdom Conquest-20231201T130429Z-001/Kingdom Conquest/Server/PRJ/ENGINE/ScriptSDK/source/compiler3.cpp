#include "compiler.hpp"
//-------------------------------------------------------------------
// 名称: CompileIFStatement
// 说明: 
//------------------------------------------------------------------
void psCCompiler::CompileIFStatement(psCScriptNode* inode, bool* hasReturn, psCByteCode* bc)
{
	// 添加一个行指今
	InstrLINE(bc, inode->GetTokenPos());

	// 再IF语句里，我们将使用一个标签,也有可能为'ELSE'多使用一个
	int afterLabel = m_NextLabel++;

	// 编译条件表达式
	psCByteCode exprBC;
	psSValue	  exprValue;
	psCScriptNode* node = inode->GetFirstChild();
	CompileAssignment(node, &exprBC, &exprValue);

	// 检查表达式的类型
	if ( !exprValue.type.IsBooleanType() ||
		  exprValue.type.IsPointer()  )
	{
		Error(TXT_EXPR_MUST_BE_BOOL, node);
		return;
	}

	psSOperand exprValueOp;
	// 确认它是四字节的整型常量
	GetIntOperand( &exprBC, exprValue, exprValueOp );

	// 释放临时变量
	ReleaseTemporaryVariable(exprValue);

	// 条件表达式中的临时变量生命期到了
	ClearTemporaryVariables( &exprBC );

	if ( exprValueOp.base != PSBASE_CONST )
	{
		// 如果不是常量表达式, 添回条件表达式的代码
		bc->AddCode(&exprBC);

		bc->Instr2(BC_JZ, exprValueOp, psSOperand(afterLabel));
	}else
	{
		// 常量表达式的
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

	// 在条件表达式后添加一条行指今
	InstrLINE(bc, inode->GetTokenPos());

	// 编译'if'语句块
	bool hasReturn1;
	psCByteCode ifBC;
	CompileStatement(node->GetNextNode(), &hasReturn1, &ifBC);

	//  添回'if'语句块的代码
	bc->AddCode(&ifBC);

	// 是否有'else'部分
	if (node->GetNextNode() != inode->GetLastChild())
	{
		int afterElse = 0;
		if ( !hasReturn1 )
		{
			afterElse = m_NextLabel++;

			//　跳到'else'后,执行'if'部分(条件为真时)
			bc->Instr1(BC_JMP, psSOperand(afterElse));
		}

		// 为'if'语句为否时的跳出添加一个标签
		bc->Label(afterLabel);

		// 编译'else'语句块
		bool hasReturn2;
		psCByteCode elseBC;
		CompileStatement(inode->GetLastChild(), &hasReturn2, &elseBC);

		// 添加'else'部分的代码
		bc->AddCode(&elseBC);

		if( !hasReturn1 )
		{
			// 如果在'if'语句没有返回，添加'if'语句为真的标签
			bc->Label(afterElse);
		}

		// 设置返回标志
		*hasReturn = hasReturn1 && hasReturn2;
	}else
	{
		// 为'if'语句块的结束，填加一个标签
		bc->Label(afterLabel);
		*hasReturn = false;
	}
}

//-------------------------------------------------------------------
// 名称: CompileSwitchStamement
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileSwitchStamement(psCScriptNode* snode, bool* hasReturn, psCByteCode* bc)
{
	// 为'break'的跳出保留一个标签
	int breakLabel = m_NextLabel++;

	// 把为'break'的跳出保留的标签，保存起来
	m_BreakLabels.push_back(breakLabel);

	// 添加一个变量域
	AddVariableScope(true, false);

	// 编译'switch'的条件表达式
	psSValue exprValue;
	psCByteCode exprBC;

	// 在表达式之前添加一条行指今
	InstrLINE(&exprBC, snode->GetTokenPos());

	psCScriptNode* enode = snode->GetFirstChild();
	CompileAssignment(enode, &exprBC, &exprValue);

	// 在'case'语句之前添加一条行指今
	InstrLINE(&exprBC, snode->GetTokenPos());

	// 条件表达式的类型必需是'int'或'unsigned int'型
	if( !exprValue.type.IsIntegerType() ||
		exprValue.type.IsPointer() )
	{
		Error(TXT_SWITCH_MUST_BE_INTEGRAL, snode);
		return;
	}

	// 确认它是四字节的整型常量
	psSOperand exprValueOp;
	GetIntOperand( &exprBC, exprValue, exprValueOp );

	// 条件表达式中生成的临时变量生命期到了
	ClearTemporaryVariables(&exprBC);

	// 分配一个临时变量,用来保存中间结果
	psSValue tmp(psCDataType(ttInt, false, false));
	AllocateTemporaryVariable(tmp);

	// 决定'case'的值与标签号
	// 预留一个标签号,那样我们可以在 CompileCase()能找到正确的标签号
	int firstCaseLabel = m_NextLabel;
	int defaultLabel = 0;

	std::vector< int >		caseValues;
	std::vector< psSOperand>  caseOps;
	std::vector< int >		caseLabels;

	// 编译所有的'case'语句，并使它们能够跳转到正确的标签
	psCScriptNode *cnode = enode->GetNextNode();
	while( cnode )
	{
		// 每一个'case'语句，必需是常数
		if (cnode->GetFirstChild() && cnode->GetFirstChild()->GetNodeType() == snExpression)
		{
			// 编译表达式的值
			psSValue cvalue;
			psCByteCode cbc;
			CompileExpression(cnode->GetFirstChild(), &cbc, &cvalue);

			// 检查表达式的值
			if (!cvalue.IsConstant())
				Error(TXT_SWITCH_CASE_MUST_BE_CONSTANT, cnode);

			// 检查表达式的值的类型
			if (!cvalue.type.IsIntegerType())
				Error(TXT_SWITCH_MUST_BE_INTEGRAL, cnode);

			// 检测case语句的值不能重复
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

			// 保存'case'语句的值,为以后使用
			caseValues.push_back(caseIntValue);
			caseOps.push_back(cvalue.operand);

			// 为这个'case'语句保留一个标签
			caseLabels.push_back(m_NextLabel++);
		}else
		{
			//  'default'语句必需在所有的'case'语句之后
			if (cnode->GetNextNode())
			{
				Error(TXT_DEFAULT_MUST_BE_LAST, cnode);
			}

			// 为'default'语句保留一个标签
			defaultLabel = m_NextLabel++;
		}
		cnode = cnode->GetNextNode();
	}

	if( defaultLabel == 0 )
	{
		defaultLabel = breakLabel;
	}

	// 输出优化的的比较运算代码
	// 如果'case'语句的值连续，并且相差的值小于5的话
	// 但如果相差的值太大的话，生成一个跳转表的代码就会很大
	// 会生成一个跳转表，那样只需要做一次比较就可达到目的地

	// 把'case'语句的的值按从小到大的顺序排序，同时也要对标签，和操作数进行排序
	// 使用了最简单的冒泡排序
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
			// 把'case'语句的值分段，如果相连的值相差小于5的话，就放在同一段
			std::vector<int> rangs;
			rangs.push_back(0);
			int n;
			for (n = 1; n < int(caseValues.size()); ++n)
			{
				if (caseValues[n] >= caseValues[n-1] + 5)
					rangs.push_back(n);
			}

			// 如果条件表达式的值比最大的'case'语句的值还大，直接跳到'default'语句
			exprBC.Instr3(BC_TGi, tmp.operand, exprValueOp, caseOps[caseOps.size() - 1]);
			exprBC.Instr2(BC_JNZ, tmp.operand, psSOperand(defaultLabel));

			// 对每一个范围进行处理
			size_t rang;
			for (rang = 0; rang < rangs.size(); ++rang)
			{
				// 找出此范围中的最大值
				int min_index = rangs[rang];
				int index	  = min_index;
				int maxRange = caseValues[index];
				for (; index < int(caseValues.size()) && (caseValues[index] < maxRange + 5); ++index)
				{
					maxRange = caseValues[index];
				}

				// 如果说这一个范围域内只有两个比较，那么就直接进行比较运算
				if (index - min_index < 3)
				{
					for (n = min_index; n < index; ++n)
					{
						// 简单的比较每个值，如果相等则跳到目的地
						exprBC.Instr3(BC_TEi, tmp.operand, exprValueOp, caseOps[n]);
						exprBC.Instr2(BC_JNZ, tmp.operand, psSOperand(caseLabels[n]));
					}
				}else
				{
					// 如果表达式的比此范围最小的值还小，跳到'default'语句
					exprBC.Instr3(BC_TLi, tmp.operand, exprValueOp, caseOps[min_index] );
					exprBC.Instr2(BC_JNZ, tmp.operand, psSOperand(defaultLabel));

					// 如果这个范围域是最后一段，没有必需做与最大值的比较
					int nextRangLabel = m_NextLabel++;
					if (rang < rangs.size() - 1)
					{	
						int offset = m_pBuilder->RegisterConstantInt(maxRange);

						// 如果表达式的比此范围最小的值还大，跳到下一段
						exprBC.Instr3(BC_TGi, tmp.operand, exprValueOp, psSOperand(PSBASE_CONST, offset));
						exprBC.Instr2(BC_JNZ, tmp.operand, psSOperand(nextRangLabel));
					}

					// 根据表达式的值进行指今跳转
					exprBC.Instr3(BC_SUBi, tmp.operand, exprValueOp, caseOps[min_index]);
					exprBC.Instr2(BC_JMPP, tmp.operand, maxRange - caseValues[min_index]);	

					//　构筑此范围的跳转表，如果是空的直接跳到'default'语句
					index = rangs[rang];
					for (int n = caseValues[index]; n <= maxRange; ++n)
					{
						if (caseValues[index] == n)
							exprBC.Instr1(BC_JMP, caseLabels[index++]);
						else
							exprBC.Instr1(BC_JMP, defaultLabel);
					}

					// 标记下一段开始
					exprBC.Label(nextRangLabel);
				}
			}	
		}

		// 捕获所有没能命中的情况
		exprBC.Instr1(BC_JMP, defaultLabel);

		// 释放临时操作数
		ReleaseTemporaryOperand(tmp.operand);
		if( exprValueOp != exprValue.operand )
			ReleaseTemporaryOperand(exprValueOp);
		ReleaseTemporaryVariable(exprValue);

		// 输出所有的'case'语句
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

		// 连接所有的二进制代码
		bc->AddCode(&exprBC);

		// 标记'break'跳转标签
		bc->Label(breakLabel);

		// 弹出保存的'break'标签
		m_BreakLabels.pop_back();

		// 移除变量域
		int r;
		m_pScript->ConvertPosToRowCol(snode->GetTokenPos() + snode->GetTokenLength(), &r, 0);
		CreateDebugSymbols(m_Variables, r);
		RemoveVariableScope();

}

//-------------------------------------------------------------------
// 名称: CompileCase
// 说明: 
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
// 名称: CompileForStatement
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileForStatement(psCScriptNode* fnode, psCByteCode* bc)
{
	// 添加一个变量域
	AddVariableScope(true, true);

	// 我们在循环内使用两个标签
	int beforeLabel = m_NextLabel++;
	int afterLabel = m_NextLabel++;
	int continueLabel = m_NextLabel++;

	// 保存break/continue标签
	m_ContinueLabels.push_back(continueLabel);
	m_BreakLabels.push_back(afterLabel);

	// 添加一个行指今
	InstrLINE(bc, fnode->GetTokenPos());

	// 编译初始化表达式
	psCScriptNode* first = fnode->GetFirstChild();
	psCByteCode initBC;
	if (first->GetNodeType() == snDeclaration)
		CompileDeclaration(first, &initBC, NULL);
	else
		CompileExpressionStatement(first, &initBC);

	// 清空初始化表达式中生成的临时变量
	ClearTemporaryVariables( &initBC );

	// 编译条件表达式
	psCScriptNode* second = first->GetNextNode();
	psCByteCode exprBC;
	psSValue exprValue;
	if (second->GetFirstChild())
	{	
		CompileAssignment(second->GetFirstChild(), &exprBC, &exprValue);

		// 检查条件表达式的类型
		if ( !exprValue.type.IsBooleanType() ||
			  exprValue.type.IsPointer() )
		{
			Error(TXT_EXPR_MUST_BE_BOOL, second);
			return;
		}

		// 确认它是四字节的整型常量
		psSOperand exprValueOp;
		GetIntOperand( &exprBC, exprValue, exprValueOp );

		// 释放临时变量　
		ReleaseTemporaryVariable(exprValue);

		// 清空条件表达式中生成的临时变量
		ClearTemporaryVariables( &exprBC );

		// 如果表达式的值为否，跳出循环
		exprBC.Instr2(BC_JZ, exprValueOp, psSOperand(afterLabel));
		if ( exprValueOp != exprValue.operand )
			ReleaseTemporaryOperand( exprValueOp );
	}

	//编译自增表达式
	psCByteCode nextBC;	
	psSValue nextValue;
	psCScriptNode* third = second->GetNextNode();
	if (third->GetNodeType() == snAssignment)
	{
		// 
		CompileAssignment(third, &nextBC, &nextValue);

		// 释放临时变量　
		ReleaseTemporaryVariable(nextValue);

		// 清空自增表达式中生成的临时变量
		ClearTemporaryVariables( &nextBC );
	}

	// 编译循环代码段
	psCScriptNode* statement = fnode->GetLastChild();
	bool hasReturn;
	psCByteCode loopBC;
	CompileStatement(statement, &hasReturn, &loopBC);

	//////////////////////////////////////////////////////////////////////////
	// 连接所有的二进制代码段

	// 添加初始化代码
	bc->AddCode(&initBC);
	bc->Label(beforeLabel);

	// 添加条件表达式代码
	bc->AddCode(&exprBC);

	// 添回循环部分的代码
	bc->AddCode(&loopBC);
	bc->Label(continueLabel);

	// 添加自增段的代码
	bc->AddCode(&nextBC);

	// 加一条行指今,方便调试
	//InstrLINE(bc, statement->GetTokenPos() + statement->GetTokenLength());

	bc->Instr1(BC_JMP, psSOperand(beforeLabel));
	bc->Label(afterLabel);

	// 弹出保存的break/continue标签
	m_BreakLabels.pop_back();
	m_ContinueLabels.pop_back();

	// NOTE: 因为在FOR（...）内可能产生临时的类对象
	// 固要析构所有的局部变量
	// 而其它几种循环语句却不可能产生临时的类对象
	psCVariableScope::VariableMap_t::const_reverse_iterator it = m_Variables->GetVariableMap().rbegin();
	for (; it != m_Variables->GetVariableMap().rend(); ++it)
	{
		const psSVariable* var = it->second;

		// 释放变量空间
		if( var->value.operand.base == PSBASE_STACK &&
			var->value.operand.offset < 0 )
		{
			// 析构所有的局部变量(不要析构引用变量)
			// NOTE:如果有'break','continue'或'return'其变量在跳出前已经被析构了

			CompileDestructor(var->value, bc);	
			// 释放堆栈空间
			DeallocateVariable(var->value.operand.offset);
		}
	}

	// 移除变量域
	int r;
	m_pScript->ConvertPosToRowCol(statement->GetTokenPos() + statement->GetTokenLength(), &r, 0);
	CreateDebugSymbols(m_Variables, r);
	RemoveVariableScope();
}

void psCCompiler::CompileWhileStatement(psCScriptNode* wnode, psCByteCode* bc)
{
	// 添加一个变量域
	AddVariableScope(true, true);

	// 我们在循环内使用两个标签
	int beforeLabel = m_NextLabel++;
	int afterLabel = m_NextLabel++;

	// 保存break/continue标签
	m_ContinueLabels.push_back(beforeLabel);
	m_BreakLabels.push_back(afterLabel);

	// 标识在表达式之前
	bc->Label(beforeLabel);

	// 添加一条行指今
	InstrLINE(bc, wnode->GetTokenPos());

	// 编译条件表达式的值
	psCByteCode exprBC;
	psSValue exprValue;
	psCScriptNode* first = wnode->GetFirstChild();
	CompileAssignment(first, &exprBC, &exprValue);

	// 检查条件表达式的类型
	if ( !exprValue.type.IsBooleanType() ||
		  exprValue.type.IsPointer() )
	{
		Error(TXT_EXPR_MUST_BE_BOOL, wnode);
		return;
	}

	// 确认它是四字节的整型常量
	psSOperand exprValueOp;
	GetIntOperand( &exprBC, exprValue, exprValueOp );

	// 释放临时变量
	ReleaseTemporaryVariable(exprValue);

	// 清空条件表达式中生成的临时变量
	ClearTemporaryVariables(&exprBC);

	// 添加条件表达式的的二进制代码
	bc->AddCode(&exprBC);

	// 如果条件表达式的值为否，跳出
	bc->Instr2(BC_JZ, exprValueOp, psSOperand(afterLabel));

	if ( exprValueOp != exprValue.operand )
		ReleaseTemporaryOperand(exprValueOp);

	// 编译循环语句块
	bool hasReturn;
	psCByteCode whileBC;
	psCScriptNode* block = first->GetNextNode();
	CompileStatement(block, &hasReturn, &whileBC);
	// 如果循环部分为空,添加一条空指今,以便调试
	if ( whileBC.IsEmpty() )
		whileBC.Instr0(BC_NOP);

	// 添加循环语句块的代码
	bc->AddCode(&whileBC);

	// 跳转对条件表达式前，继续执行
	bc->Instr1(BC_JMP, psSOperand(beforeLabel));

	// 标识循环结束
	bc->Label(afterLabel);

	// 弹出保存的break/continue标签
	m_ContinueLabels.pop_back();
	m_BreakLabels.pop_back();

	// 移除变量域
	int r;
	m_pScript->ConvertPosToRowCol(block->GetTokenPos() + block->GetTokenLength(), &r, 0);
	CreateDebugSymbols(m_Variables, r);
	RemoveVariableScope();
}

void psCCompiler::CompileDoWhileStatement(psCScriptNode* wnode, psCByteCode* bc)
{
	// 添加一个变量域
	AddVariableScope(true, true);

	// 我们在循环内使用三个标签
	int beforeLabel = m_NextLabel++;
	int beforeTest =  m_NextLabel++;
	int afterLabel =  m_NextLabel++;

	// 保存break/continue标签
	m_ContinueLabels.push_back(beforeTest);
	m_BreakLabels.push_back(afterLabel);

	// 添加一条行指今
	bc->Label(beforeLabel);

	// 编译循环语句块
	psCByteCode whileBC;
	bool hasReturn;
	psCScriptNode* first = wnode->GetFirstChild();
	CompileStatement(first, &hasReturn, &whileBC);

	// 如果循环部分为空,添加一条空指今,以便调试
	if (whileBC.IsEmpty())
		whileBC.Instr0(BC_NOP);

	// 添加循环语句块的代码
	bc->AddCode(&whileBC);

	// 标记开始进行判断
	bc->Label(beforeTest);

	// 添加一条行指今
	InstrLINE(bc, wnode->GetLastChild()->GetTokenPos());

	//　编译条件表达式的值
	psCByteCode exprBC;
	psSValue exprValue;
	CompileAssignment(wnode->GetLastChild(), &exprBC, &exprValue);

	// 检查条件表达式的类型
	if (!exprValue.type.IsBooleanType() ||
		 exprValue.type.IsPointer() )
	{
		Error(TXT_EXPR_MUST_BE_BOOL, wnode->GetLastChild());
		return;
	}

	// 确认它是四字节的整型常量
	psSOperand exprValueOp;
	GetIntOperand( &exprBC, exprValue , exprValueOp);

	// 释放临时操作数
	ReleaseTemporaryVariable(exprValue);

	// 清空条件表达式中生成的临时变量
	ClearTemporaryVariables(&exprBC);

	// 添加条件表达式的的二进制代码
	bc->AddCode(&exprBC);

	// 如果表达式的值为真，跳回到判断语句
	bc->Instr2(BC_JNZ, exprValueOp, psSOperand(beforeLabel));
	if ( exprValueOp != exprValue.operand )
		ReleaseTemporaryOperand( exprValueOp );

	// 标记循环结束
	bc->Label(afterLabel);

	// 弹出保存的break/continue标签
	m_ContinueLabels.pop_back();
	m_BreakLabels.pop_back();

	// 移除变量域
	int r;
	m_pScript->ConvertPosToRowCol(wnode->GetTokenPos() + wnode->GetTokenLength(), &r, 0);
	CreateDebugSymbols(m_Variables, r);
	RemoveVariableScope();
}

void psCCompiler::CompileReturnStatement(psCScriptNode* rnode, psCByteCode* bc)
{
	// NOTE: 这里之所用一个retBC来存储返回值的二进制代码是因为:
	//       析构局部变量时可以会重写返回值
	psCByteCode retBC;

	// 添加一条行指今
	InstrLINE(bc, rnode->GetTokenPos());

	// 得到所回值变量
	if ( m_ReturnType.GetSizeOnStackDWords() == 0 )
	{
		if (rnode->GetFirstChild())
			Error(TXT_CANT_RETURN_VALUE, rnode);
	}else
	{
		// 返回值是表达式值
		if( rnode->GetFirstChild() )
		{	
			// 编译表达式的值
			psSValue expr;
			CompileAssignment(rnode->GetFirstChild(), bc, &expr);

			// 测试变量是否已经初始化
			IsVariableInitialized(expr, rnode);

			if (!ImplicitConversion(expr, m_ReturnType, expr, bc, rnode, false))
			{
				psCString str;
				str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, expr.type.GetFormat().AddressOf(), m_ReturnType.GetFormat().AddressOf());
				Error(str.AddressOf(), rnode);
			}

			// 如果是引用,其值不能是临时变量
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
						// 直接把对象指针赋给返回值
						retBC.Instr1(BC_SRET4, expr.operand );
					}else
					{
						// 需要解析对象地址
						psSValue tmp(g_IntDataType);
						AllocateTemporaryVariable(tmp);
						retBC.Instr2(BC_DEFREF, tmp.operand, expr.operand);
						retBC.Instr1(BC_SRET4, tmp.operand);
						ReleaseTemporaryVariable(tmp);
					}
				}else
				{
					// 如果返回值是类对象并且不是引用
					// 那么第一个参数为其返回类对象的指针
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
					// 拷贝构造这个类对象
					CopyConstructObject(retObj, expr, bc, rnode);

					// 写返回值
					retBC.Instr1(BC_SRET4, retObj.operand );
				}
			}else
			{
				// 回写返回值
				if ( expr.type.IsReference() && !m_ReturnType.IsReference() )
				{
					// 分配一临时变量, 解析出引用值
					psSValue retValue(m_ReturnType);
					AllocateTemporaryVariable( retValue );

					InstrRDREF( &retBC,  retValue.operand, expr.operand, m_ReturnType );
					InstrSRET(  &retBC,  retValue.operand, m_ReturnType );

					// 释放临时变量
					ReleaseTemporaryVariable( retValue );
				}else if ( !expr.type.IsReference() && m_ReturnType.IsReference() )
				{
					// 分配一临时变量, 解析出地址值
					psSValue retValue(expr.type);
					retValue.type.IsPointer() = true;

					AllocateTemporaryVariable(retValue);

					retBC.Instr2(BC_DEFREF, retValue.operand, expr.operand );
					retBC.Instr1(BC_SRET4,  retValue.operand );

					// 释放临时变量
					ReleaseTemporaryVariable( retValue );
				}else
				{
					InstrSRET( &retBC, expr.operand, m_ReturnType);
				}
			}			

			// 释放临时变量
			ReleaseTemporaryVariable(expr);

			// 清空临时空量
			// NOTE: 要在写返回值之代码前,因为有可能会调用析构函数,
			//	     这时有可把return寄存器的值给改了.
			ClearTemporaryVariables(bc);
		}else
		{
			Error(TXT_MUST_RETURN_VALUE, rnode);
		}
	}

	// 析构所有的局部变量,参数除外
	psCVariableScope *vs = m_Variables;
	while( vs ) 
	{
		psCVariableScope::VariableMap_t::const_reverse_iterator iter = vs->GetVariableMap().rbegin();
		for (; iter != vs->GetVariableMap().rend(); ++iter)
		{
			const psSVariable* var = iter->second;	

			// 析构所有的局部类变量(不要析构引用变量,返回值变量)
			if (var->value.operand.offset < 0 &&
				var->value.operand.base == PSBASE_STACK  )
			{
				CompileDestructor(var->value, bc);
			}
		}
		vs = vs->GetParent();
	} 

	// 添回设置返回值的代码
	bc->AddCode(&retBC);

	// 跳出程序
	bc->Instr1(BC_JMP, psSOperand(0));
}

void psCCompiler::CompileBreakStatement(psCScriptNode* node, psCByteCode* bc)
{
	// 添加一条行指今
	InstrLINE(bc, node->GetTokenPos());

	// 如果'break标签没有数据，那么这是一个无效的'break'
	if (m_BreakLabels.size() == 0)
	{
		Error(TXT_INVALID_BREAK, node);
		return;
	}

	// 析构所有的局部变量
	psCVariableScope *vs = m_Variables;
	while( vs && !vs->IsBreakScope() ) 
	{
		psCVariableScope::VariableMap_t::const_reverse_iterator iter = vs->GetVariableMap().rbegin();
		for (; iter != vs->GetVariableMap().rend(); ++iter)
		{
			const psSVariable* var = iter->second;		
			// 析构所有的局部类变量(不要析构引用变量)
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
	// 添加一条行指今
	InstrLINE(bc, node->GetTokenPos());

	// 如果'continue'标签没有数据，那么这是一个无效的'continue'
	if (m_ContinueLabels.size() == 0)
	{
		Error(TXT_INVALID_CONTINUE, node);
		return;
	}

	// 析构所有的局部变量
	psCVariableScope *vs = m_Variables;
	while( vs && !vs->IsContinueScope() ) 
	{
		psCVariableScope::VariableMap_t::const_reverse_iterator iter = vs->GetVariableMap().rbegin();
		for (; iter != vs->GetVariableMap().rend(); ++iter)
		{
			const psSVariable* var = iter->second;			
			// 析构所有的局部类变量(不要析构引用变量)
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

	// 第一个结点是函数名或者是类名,类型名
	psCHAR name[256];
	psCHAR field[256];
	bool isFunction	   = false;
	bool isConstructor = false;	

	psCObjectType* objField = (node->GetTokenType() == ttField) ? NULL : m_ObjectType;

	// 得到名称结点
	psCScriptNode* nm = node->GetFirstChild();
	if ( nm->GetNextNode() && nm->GetNextNode()->GetNodeType() == snIdentifier )
	{
		// 得到域名
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

	// 得到函数名
	GetNodeSource( name, 255, nm );

	// 如果此名称不是类名，则是函数名
	if ( m_pBuilder->GetObjectType( name ) == NULL )
	{
		// 如果是类函数调用，得到类的成员函数
		// 否则得到全局函数
		isFunction = true;
	}

	psCScriptNode* argList = node->GetLastChild();	
	// 准备函数调用	
	std::vector< int >	   funcs;
	std::vector<int>	   matches;
	std::vector<int>	   exactmatches;
	std::vector<psSValue>  argValues;
	psCByteCode			   argBC;
	CompileArgumentList( argList, &argBC, argValues );

	int baseOffset = 0;
	if ( isFunction )
	{
		// 获取正确的函数域
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
		// 查找匹配的函数
		while ( curObjType )
		{
			// 类方法
			m_pBuilder->GetObjectMethodDescrptions( name, curObjType, funcs );

			// 精确匹配
			MatchFunctionCall( funcs, exactmatches, argValues, true );
			if ( exactmatches.size() != 0 )
				break;

			// 完全匹配
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
			// 匹配全局函数
			m_pBuilder->GetFunctionDescriptions( name, funcs );
			MatchFunctionCall( funcs, matches, argValues, false );
		}
	}else
	{
		// 如果不是函数调用,则有可能是类型转换
		psCObjectType* ot = m_pBuilder->GetObjectType( name );
		if ( ot == NULL )
		{
			Error(TXT_ILLEGAL_CALL, node);
			return;
		}

		// 这是一个构造函数
		funcs = ot->m_Beh.constructors;

		// 构造函数不能被显示用类方法的形式的调用
		if( objectType )
		{
			Error(TXT_ILLEGAL_CALL, node);
		}
		else
		{
			isConstructor = true;

			// 设置函数返回的类型
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
				// 分配一个临时对象
				AllocateTemporaryVariable(*retValue);

				if ( !ot->m_Beh.hasConstructor && argList->GetFirstChild() == NULL )
				{
					// 如果一个类对象没有参数为空的构造函数, 则分配一个临时类对象就行了,
					// 不用调用其构造函数
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
		// 如果区配了变参函数调用
		// 调用变参函数参数编译期类型检查
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
		// 函数调用
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

				// 释放临时变量
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
		// 如果返回值为类对象,并且不是引用值的话,
		// 事先在堆栈上分配这个临时类对象并把它当作第一个参数传进
		AllocateTemporaryVariable( tmpValue );

		bc->PushRef( tmpValue.operand );

		// NOTE: 此时参数的大小应该加１
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

	// 脚本函数的ID应该大于等于0,如果函数的ID小于0则是系统函数
	if (desc->m_ID < 0)
	{
		// 调用系统函数
		bc->CallSys( desc->m_ID, argSize, bObject  );
	}else
	{
		// 调用脚本函数(可以是再另一个脚本模块)
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
					// 如果返回值类型是类对象
					// 读取其地址值,固其类型引用属性为真

					// 分配一个临时变量　
					AllocateTemporaryVariable(*retValue);

					// 读取返回值
					bc->Instr1(BC_RRET4, retValue->operand);
				}
			}else
			{		
				// 分配一个临时变量　
				AllocateTemporaryVariable(*retValue);

				// 读取返回值
				InstrRRET(bc, retValue->operand, retValue->type);
			}
		}
	}else
	{
		// 释放临时变量
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

	// 从右到左编译参数列表
	arg = argListNode->GetLastChild();
	argindex--;

	while ( arg )
	{ 
		// 
		CompileAssignment( arg, bc, &argValues[argindex] );

		// 移到下一个参数
		argindex--;
		arg = arg->GetPrevNode();
	}
}

void psCCompiler::MatchFunctionCall(const std::vector<int>& allfuncs, std::vector<int>& matches, const std::vector<psSValue>& argValues, bool isExactMatch )
{
	assert( &allfuncs != &matches );

	// 如果参数大不一样并且不是变参调用,踢出去
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
	//　匹配函数参数	
	for (size_t i = 0; i < argValues.size(); ++i )
	{
		// 参数匹配
		MatchArgument( (int)i, argValues[i], funcs, matches, isExactMatch );

		// 更新候选函数
		funcs = matches;
	}
}

int psCCompiler::PushArgumentList( psCScriptNode* argListNode, psCScriptFunction* func, psCByteCode* bc, const std::vector<psSValue>& argValues )
{
	assert( func->IsArgVariational() || func->m_ParamTypes.size() == argValues.size() );


	int argSize = 0;

	// 从右到左把参数入栈
	size_t i = argValues.size();

	// 变参部分的参数入栈
	for ( ; i >  func->m_ParamTypes.size(); --i )
	{
		const psSValue& argValue = argValues[i - 1];

		// 根据数据类型是否为指针决定其传入的参数是否为地址值
		psCDataType type( argValue.type );
		if ( type.IsPointer() )
			type.IsReference() = true;
		else
			type.IsReference() = false;

		if ( type.IsFloatType() && !type.IsReference() )
		{
			// 注: 在C++中所有的变参函数中的变参都是以double入栈的
			type.TokenType() = ttDouble;

			psSValue tmpValue( type );
			AllocateTemporaryVariable( tmpValue );

			if (argValue.type.IsReference())
			{	
				// 如果参数是引用类型要先解引用
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

		// 更新参数大小
		argSize += type.GetSizeOnStackDWords();

		ReleaseTemporaryVariable( argValue );
	}

	// 其它参数入栈
	for ( i = func->m_ParamTypes.size(); i > 0; --i )
	{
		psCDataType &to			 = func->m_ParamTypes[i - 1];
		const psSValue& argValue = argValues[ i - 1];

		bool reportWarning = true;
		if ( to.IsReference() && !to.IsPointer() )
			reportWarning = false;
		IsVariableInitialized( argValue, argListNode, reportWarning);

		psSValue matchedArg;
		// 做一次隐式转换，肯定应该成功
		// 因为都已经通过参数匹配了
		bool bOk = ImplicitConversion( argValue, to, matchedArg, bc, argListNode, false);
		assert( bOk );

		// 更新参数大小
		argSize += to.GetSizeOnStackDWords();

		// 把参数入栈
		PushArgument( matchedArg, to, bc, argListNode );	

		// 释放临时变量	
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
	// 构造一个可读的参数列表
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
			// 如果参数索引大于函数的参数个数,那么此函数必为变参函数
			assert(desc->IsArgVariational() );
			matches.push_back(funcs[n]);
			continue;
		}

		psCDataType& to = desc->m_ParamTypes[argIndex];

		psSValue tmp;
		// 尝试一次隐式转换
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
		// 如果是数组类型其一定是以引用方式传递的
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
			// 如果是类引用传递，直接'push'其地址值
			if ( argValue.type.IsReference() )
				bc->Push4(argValue.operand);
			else
				bc->PushRef(argValue.operand);
		}else
		{
			// 生成一个临时的类对象,
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
				// 分配一个临时变量读出其地址值
				tmpArg.type.IsPointer() = false;
				tmpArg.type.IsReference() = false;

				AllocateTemporaryVariable(tmpArg);
				InstrRDREF(bc, tmpArg.operand, argValue.operand, argValue.type);	

				// 释放临时变量
				ReleaseTemporaryVariable(tmpArg);
			} 

			int s = tmpArg.type.GetSizeInMemoryBytes();
			if ( s == 1 )
			{
				psSValue tmp1;
				AllocateTemporaryVariable(tmp1);
				bc->Instr2( tmpArg.type.IsUnsignedInteger() ? BC_B2I : BC_C2I, tmp1.operand, tmpArg.operand );

				bc->Push4( tmp1.operand );

				// 释放临时变量
				ReleaseTemporaryVariable(tmp1);
			}else if ( s == 2 )
			{
				psSValue tmp1;
				AllocateTemporaryVariable(tmp1);

				bc->Instr2( tmpArg.type.IsUnsignedInteger() ? BC_W2I : BC_S2I, tmp1.operand, tmpArg.operand );	
				bc->Push4( tmp1.operand );

				// 释放临时变量
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
				//(如果参数是引用,则其值已经是地址值)
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
// 类名: IsLValue
// 说明: 
//-------------------------------------------------------------------
bool psCCompiler::IsLValue(const psSValue& value)
{
	// 如果是引用只检查只读性
	if ( value.type.IsArrayType()	)	  {	return false;	}
	if ( value.type.IsPointer() ) 
	{
		return !value.isTemporary;
	}else
	{
		if ( value.type.IsReadOnly()  )	  { return false;   }

		// 常量或临时变量都不能是左值
		if ( value.IsConstant()		 ||
			IsTemporaryVariable(value) )
		{
			return false;
		}
		return true;
	}
}

//-------------------------------------------------------------------
// 类名: IsTemporaryVariable
// 说明: 
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
		// 如果是做参数匹配,不用分配空临时空间
		return 0x7FFFFFFF;
	}

	// NOTE:不要让不同类型的数据使用同样的临时空间
	// 也不要让不同引用类型的数据类型使用同样的临时空间
	// i.e.如果临时类对象和临时整型使用同样的编移,
	//	   或者引用类对象和非引用类对象使用同样的编移
	//	   最后异常处理时释放类对象会出错
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

	// NOTE: 数据类型的大小必需大于0,如果不是变量的分配可能会出错
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

		// 如果临时变量的堆栈偏移为0x7FFFFFFF, 则表式其是在作参数匹配时产生的
		// 如果是临时类对象, 等到其生命期结束时在释放它
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

	// 释放临时操作数
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

		// 调用析构函数
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
		// 生成符号表(用于调试器)
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
	// 临是变量,类变量或引用都被认为是已经初始化过的
	if ( var.type.IsReference() ||
		 var.type.IsArrayType() ||
		 var.type.IsObject()    ||
		 IsTemporaryVariable(var) )
	{
		return true;
	}

	if (var.operand.base != PSBASE_STACK)
		return true;

	// 查找局部变量
	psSVariable *v = m_Variables->GetVariableByOffset(var.operand.offset);
	if (v)
	{
		if ( !v->value.type.IsObject() && !v->value.isInitialized && reportWarning )
		{
			psCString str;
			str.Format(TXT_s_NOT_INITIALIZED, v->name.AddressOf() );
			Warning(str.AddressOf(), node);	
		}		

		//重新设置初始化标志,这样就只会警告一次
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
	// NOTE: 如果表达式的值是引用类型,要解析出其值
	// 如果表达式的值的类型是char或者short要转换为int
	// 否则,TNZi/TZi的结果会不确.
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

	// 得到类名
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
		// NOTE:这是一种合法情况
		// 在做参数匹配时,可能会出现隐式转换出错,而这时结点指针为空
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
		// NOTE:这是一种合法情况
		// 在做参数匹配时,可能会出现隐式转换出错,而这时结点指针为空
		return;
	}

	psCString str;

	int r, c;
	psCScriptCode* code =m_pScript->ConvertPosToRowCol(node->GetTokenPos(), &r, &c);

	m_pBuilder->WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), msg, r, c);

	m_bHasCompileError = true;
}

