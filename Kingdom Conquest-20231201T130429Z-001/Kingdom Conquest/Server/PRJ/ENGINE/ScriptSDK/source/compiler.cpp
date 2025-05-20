#include "compiler.hpp"

//-------------------------------------------------------------------
// 类名: psCCompiler
// 说明: 
//-------------------------------------------------------------------
psCCompiler::psCCompiler(void)
{
	m_pBuilder		   = NULL;
	m_pScript		   = NULL;
	m_Variables		   = NULL; 
	m_ObjectType	   = NULL;
	m_StackPos		   = 0;
}

psCCompiler::~psCCompiler(void)
{
	while (m_Variables)
	{
		psCVariableScope* var = m_Variables;
		m_Variables = m_Variables->GetParent();

		delete var;
	}
}	

//----------------------------------------------------------------------//
// 名称: Reset
// 说明: 重置编译器
//----------------------------------------------------------------------//
void psCCompiler::Reset(psCBuilder* builder, psCScriptCode* script, psCObjectType* objectType)
{
	assert(builder&&script);

	m_ObjectType = objectType;
	m_pBuilder = builder;
	m_pScript = script;

	m_bIsMatchArgument    = false;
	m_bHasCompileError    = false;
	m_NextLabel			  = 0;
	m_StackPos			  = 0;

	m_ContinueLabels.clear();
	m_BreakLabels.clear();
	
	m_ByteCode.ClearAll();
	m_DebugSymbols.clear();
	m_FreeVariables.clear();
	m_TempVariables.clear();	
	m_TempValues.clear();
	m_VariableAllocations.clear();
}

//-------------------------------------------------------------------
// 名称: CompileGlobalVariable
// 说明: 编译全局变量
//-------------------------------------------------------------------
int psCCompiler::CompileGlobalVariable(psCBuilder* builder, psCScriptCode* script, psCScriptNode* node, psSGlobalVariableDesc* gvar)
{
	// 重置编译器
	Reset( builder, script, NULL );

	// 添加一个变量域
	AddVariableScope();

	// 编译变量声明
	CompileDeclaration(node, &m_ByteCode, gvar);

	// 移除变量域
	// NOTE: 初始行号为-1表示为全局有效(用于调试器)
	RemoveVariableScope();

	return  (m_bHasCompileError) ? -1 : 0;
}

//-------------------------------------------------------------------
// 名称: CompileFunction
// 说明: 编译函数
//-------------------------------------------------------------------
int psCCompiler::CompileFunction(psCBuilder* builder, psCScriptCode* script, psCScriptNode* func, psCObjectType* objType)
{
	// 重置编译器
	Reset(builder, script, objType);

	// 为函数返回预留一个标签,
	m_NextLabel++;

	// 添加一个变量域,函数的参数,及函数体内最外面的的变量都存在这里
	AddVariableScope();

	int funcType = func->GetNodeType();
	// 得到返回值类型
	if ( funcType == snFunction || funcType == snOpOverload )
	{
		m_ReturnType = m_pBuilder->CreateDataTypeFromNode(func->GetFirstChild(), m_pScript);
		m_pBuilder->ModifyDataTypeFromNode(m_ReturnType, func->GetFirstChild()->GetNextNode(), m_pScript );
	}else
	{	
		m_ReturnType.TokenType() = ttVoid;
		m_ReturnType.SetObjectType( NULL );

		// 函数还有可能为构造/构构函数
		assert( funcType == snConstructor || funcType == snDestructor );
	}

	// 查找参数列表结点, 其TokenType保存了此函数是否为变参函数的属性
	psCScriptNode* node = func->GetFirstChild();
	while (node && node->GetNodeType() != snParameterList)
	{
		node = node->GetNextNode();
	}

	// 脚本本身不支持变参
	if (node->GetTokenType() == ttEllipsis)
	{
		Error(TXT_SCRIPT_SEFL_DONT_SUPPORT_ARGCHANGED, func);
		return -1;
	}

	// 声明函数参数,先以以从左到右的顺序声明
	psCByteCode paramBC;

	int funcArgSize = 0;
	if ( m_ObjectType )
	{
		// 如果是类成员函数,则第一个参数为类对象指针
		funcArgSize++;
	}

	if ( m_ReturnType.IsObject() && !m_ReturnType.IsReference() )
	{
		// 如果返回值为类对象,并且不是引用那么
		// 其中第一个参数为返回类对象的指针
		funcArgSize++;
	}

	if (node)
	{
		// 参数列表结点有可能为空
		node = node->GetFirstChild();
	}

	while (node)
	{
		// 得到参数类型
		psCDataType type = m_pBuilder->CreateDataTypeFromNode(node, m_pScript);
		node = node->GetNextNode();

		// 得到参数类型修饰符
		m_pBuilder->ModifyDataTypeFromNode(type, node, m_pScript);
		node = node->GetNextNode();

		// 检查数据类型
		CheckDataType( type, func );

		assert( node );
		psCScriptNode* arrayDimNode = node;
		if ( arrayDimNode->GetNodeType() == snIdentifier )
		{
			arrayDimNode = arrayDimNode->GetNextNode();
		}
		assert( arrayDimNode && arrayDimNode->GetNodeType() == snArrayDim );
		m_pBuilder->ModifyDataTypeFromNode(type, arrayDimNode, m_pScript );
		
		if ( type.IsArrayType() )
		{
			// 如果是数组类型其一定是引用类型传递
			type.IsPointer() = true;
			type.IsReference() = true;
		}

		// 如果此参数有名称, 声明此变量
		if ( node->GetNodeType() == snIdentifier )
		{
			psCString name;
			name.Copy(&m_pScript->m_Code[node->GetTokenPos()], node->GetTokenLength());

			psSValue value;	
			value.StackValue(type, funcArgSize * sizeof(psDWORD) );

			// 堆栈的偏移是以字节为单位的
			funcArgSize += type.GetSizeOnStackDWords();

			if (m_Variables->DeclareVariable(name.AddressOf(), value) == 0)
				Error(TXT_PARAMETER_ALREADY_DECLARED, func);
		}else
		{
			// 此参数为匿名变量,
			// 匿名变量并不生成名称结点, 固不要调用: node = node->GetNextNode()
			psSValue value;
			value.StackValue(type, funcArgSize * sizeof(psDWORD) );

			m_Variables->DeclareVariable(PS_T(""), value);

			// 堆栈的偏移是以字节为单位的
			funcArgSize += type.GetSizeOnStackDWords();
		}

		// 移到下一个结点
		node = arrayDimNode->GetNextNode();
	}

	psCByteCode constructBC;
	// 如果是类的构造函数, 添加类成员的构造函数代码
	// 要在参数声明后,函数语句块生成前生成
	if (funcType == snConstructor && m_ObjectType)
	{
		DefaultObjectConstructor(m_ObjectType, &constructBC);
	}

	// 编译函数体, 这一个函数体本身并没有变量域
	// 它里面的变量,使用的是与参数变量相同的变量域
	bool hasReturn;
	psCByteCode blockbc;
	CompileStatementBlock(func->GetLastChild(), false, &hasReturn, &blockbc);

	// 如是返回值类型不是空,确认每一个路径都有返回值
	if ( m_ReturnType.TokenType() != ttVoid )
	{
		if( hasReturn == false )
			Error(TXT_NOT_ALL_PATHS_RETURN, func->GetLastChild());
	}

	// 析构所有的局部变量
	psCVariableScope::VariableMap_t::const_reverse_iterator it = m_Variables->GetVariableMap().rbegin();
	for (; it != m_Variables->GetVariableMap().rend(); ++it)
	{
		const psSVariable* var = it->second;

		if ( var->value.operand.base == PSBASE_STACK &&
			 var->value.operand.offset < 0 )
		{
			CompileDestructor(var->value, &blockbc);	

			// 释放临时变量空间
			DeallocateVariable(var->value.operand.offset);
		}
	}	

	psCByteCode destructBC;
	psCByteCode exitBC;
	// 如果是类的析构函数, 添加析构成员的析构代码
	if (funcType == snDestructor && m_ObjectType)
	{
		DefaultObjectDestructor(m_ObjectType, &destructBC);
	}

	// 析构所有的函数参数对象
	it = m_Variables->GetVariableMap().rbegin();
	for (; it != m_Variables->GetVariableMap().rend(); ++it)
	{
		const psSVariable* var = it->second;

		if ( var->value.operand.base == PSBASE_STACK && 
			 var->value.operand.offset >= 0 )
		{
			CompileDestructor(var->value, &destructBC );
		}
	}

	if ( !destructBC.IsEmpty() )
	{
		// NOTE: 之所以要用一个临时变量,保存返回值是由于
		// 析构类成员或类对象参数可能会重写返回值
		psCDataType rtype;
		if ( m_ReturnType.IsObject() || m_ReturnType.IsReference() ||
			 m_ReturnType.GetSizeInMemoryBytes() <= 4.)
		{
			rtype.TokenType() = ttInt;
		}else
		{
			rtype.TokenType() = ttDouble;
		}

		// 保存返回值
		psSOperand retOp( PSBASE_STACK, AllocateVariable( rtype, false ) );
		InstrRRET( &exitBC, retOp, rtype );

		// 添加析构函数的代码
		exitBC.AddCode( &destructBC );

		// 恢复返回值
		InstrSRET( &exitBC, retOp, rtype );

		DeallocateVariable( retOp.offset );
	}

	// 如果有编译错,不必生成最终的代码
	if( m_bHasCompileError ) return -1;

	// 现在应该没有分配的变量
	size_t s1 = m_VariableAllocations.size() ;
	size_t s2 = m_FreeVariables.size();
	assert(s1 == s2);

	// 计算函数起始/结束的位置
	int lastLinePos =  func->GetLastChild()->GetTokenPos() + func->GetLastChild()->GetTokenLength();

	// 移除变量域
	int r;
	m_pScript->ConvertPosToRowCol(lastLinePos, &r, 0);
	CreateDebugSymbols(m_Variables, r);
	RemoveVariableScope();

	//----------------------------------------------------------------------//
	// 连接所有的二进制代码
	//----------------------------------------------------------------------//
	psCByteCode* bc = &m_ByteCode;

	int largestStackUsed = -m_StackPos;

	// 计算总共需要的堆栈大小
	// NOE: 因为我们使用的是向上生长的堆栈,
	// 固入栈操作要把堆栈指减小
	bc->Push(largestStackUsed);

	// 添加类成员构造代码
	bc->AddCode(&constructBC);

	// 添加函数参数声明的代码
	bc->AddCode(&paramBC);

	// 添加语句块的代码
	bc->AddCode(&blockbc);

	// Label 0 是'return'的跳出点
	bc->Label(0);

	// 在函数的未尾加一条行指今
	InstrLINE(bc, lastLinePos);

	// 添回析构类成员和参数的代码
	bc->AddCode(&exitBC);

	// 弹出堆栈
	bc->Pop(largestStackUsed);

	// 函数返回
	bc->Ret(funcArgSize);

	// 最后生成二进制代码
	bc->Finalize(&m_pBuilder->GetOptimizeOptions());

	return 0;
}

//-------------------------------------------------------------------
// 名称: CompileStatementBlock
// 说明: 编译语句块
//-------------------------------------------------------------------
void psCCompiler::CompileStatementBlock(psCScriptNode *block, bool ownVariableScope, bool *hasReturn, psCByteCode *bc)
{
	assert(hasReturn && bc);

	// 设置返回值标志
	*hasReturn = false;

	bool isFinished = false;
	bool hasWarned = false;

	// 在语句块前,添回一条行指今
	InstrLINE(bc, block->GetTokenPos());

	if (ownVariableScope)
	{
		AddVariableScope();
	}

	psCScriptNode* node = block->GetFirstChild();
	while (node)
	{	
		if( !hasWarned && (*hasReturn || isFinished) )
		{
			hasWarned = true;
			Warning(TXT_UNREACHABLE_CODE, node);
		}
		
		if (node->GetNodeType() == snBreak || 
			node->GetNodeType() == snContinue)
		{
			isFinished = true;
		}

		bool ret = false;	
		if (node->GetNodeType() == snDeclaration)
		{
			CompileDeclaration(node, bc, NULL);
		}
		else
		{
			CompileStatement(node, &ret, bc);
		}

		if (ret)
		{
			*hasReturn = true;
		}

		node = node->GetNextNode();
	}

	// 在语句块结束处,添回一条行指今
	int lastLinePos = block->GetTokenPos() + block->GetTokenLength();
	int r;
	m_pScript->ConvertPosToRowCol(lastLinePos, &r, 0);
	InstrLINE( bc, lastLinePos );
		
	// 移除变量域
	if (ownVariableScope)
	{	
		// 以相反的顺序,释放所有的局部变量
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

				// 如果函数没有返回, 并且没有不是由'break',或'continue'跳出变量域,
				// 则需要析构局部变量
				if( !isFinished && !(*hasReturn) )
				{
					CompileDestructor(var->value, bc);	
				}

				// 释放堆栈空间
				DeallocateVariable(var->value.operand.offset);
			}
		} 

		// 移除变量域
		int r;
		m_pScript->ConvertPosToRowCol(block->GetTokenPos() + block->GetTokenLength(), &r, 0);
		CreateDebugSymbols(m_Variables, r);
		RemoveVariableScope();
	}

}

//-------------------------------------------------------------------
// 名称: CompileDeclaration
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileDeclaration(psCScriptNode* decl, psCByteCode* bc, psSGlobalVariableDesc* gvar)
{
	// 添回一条行指今
	int r = InstrLINE(bc, decl->GetTokenPos());

	psCScriptNode* node = decl->GetFirstChild();

	// 得到变量数据类型
	psCDataType type = m_pBuilder->CreateDataTypeFromNode(node, m_pScript);
	node = node->GetNextNode();

	// 声明所有的变量
	while (node)
	{
		// 得到数据类型
		m_pBuilder->ModifyDataTypeFromNode(type, node, m_pScript);
		node = node->GetNextNode();

		// 检查数据类型
		CheckDataType( type, node );

		// 得到变量名称
		psCString name;
		CompileVariableDim(node, bc, name, type);
		node = node->GetNextNode();

		// 确认这个名称不是一个类名
		if( m_pBuilder->GetObjectType(name.AddressOf()) != NULL )
		{
			psCString str;
			str.Format(TXT_ILLEGAL_VARIABLE_NAME_s, name.AddressOf());
			Error(str.AddressOf(), decl);
		}
		psSValue variable;

		if ( type.IsReference() )
		{
			// 如果引用类型,引有类型不能是数组
			if (type.IsArrayType())
			{
				Error(TXT_REF_TYPE_CANNOT_BE_ARRAY, decl);
				return;
			}

			if ( node == NULL || node->GetNodeType() != snInitExpr)
			{
				// 引用类型,必需有初始值
				psCString str;
				str.Format(TXT_REF_s_MUST_BE_INITIALIZED, name.AddressOf() );
				Error(str.AddressOf() , decl);

				// 保证继续编译
				variable.isInitialized = true;
				variable.type = type;
				if (NULL == gvar)
				{
					// 如果不是全局变量,声明它
					m_Variables->DeclareVariable(name.AddressOf(), variable);
				}
				return;
			}else
			{	
				psCScriptNode* first = node->GetFirstChild();
				if (first && first->GetNodeType() != snAssignment)
				{		
					Error(TXT_REF_s_INITEXPR_TYPE_ERROR, decl);
				}else
				{
					psSValue exprValue;
					// 编译初始化表达式
					CompileAssignment(first, bc, &exprValue);

					bool bOk = false;
					if ( !type.IsPointer() )
					{
						// 引用不能为临时的
						if ( IsTemporaryVariable(exprValue) )
						{
							psCString str;
							str.Format(TXT_REF_s_CAN_NOT_BE_TEMPOARY, name.AddressOf());
							Error(str.AddressOf(), decl);
						}
					}else
					{
						if ( exprValue.isNullPointer )
							bOk = true;
					}
					
					bool isReference = exprValue.type.IsReference();
					if ( !bOk )
					{
						if ( !exprValue.type.IsReadOnly() || type.IsReadOnly() )
						{	
							if ( type.IsPointer() && exprValue.type.IsArrayType() )
							{
								psCDataType exprValueType( exprValue.type );
								exprValueType.PopArraySize();
								exprValueType.IsPointer()   = true;
								exprValueType.IsReference() = true;
								bOk = exprValueType.IsEqualExceptConst(type) ;
								isReference = true;
							}else 
							{
								bOk = exprValue.type.IsEqualExceptConst(type);
							}
						}
					}
					if ( !bOk )
					{
						// 检查引用的类型是否正确,需要考虑只读属性
						psCString str;
						str.Format( TXT_CANT_CONVERT_s_TO_s, exprValue.type.GetFormat().AddressOf(), type.GetFormat().AddressOf() );
						Error(str.AddressOf(), decl);
					}
	
					// 设置值的属性
					// 生成一个临时对象指针, 因为其指向一个空地址
					variable.type				= type;
					variable.isInitialized      = true;
					variable.startline		    = r;	
					variable.isTemporary		= true;
					// 预先分配一个变量
					if (gvar)
					{
						gvar->base = PSBASE_GLOBAL;
						variable.operand.BaseOffset(PSBASE_GLOBAL, m_pBuilder->AllocateGlobalVar(type));
					}
					else
					{
						variable.operand.BaseOffset(PSBASE_STACK, AllocateVariable(type ) );
					}
	
					if ( exprValue.type.IsReference() )
						bc->Instr2( BC_MOV4, variable.operand, exprValue.operand );
					else
						bc->Instr2( BC_DEFREF, variable.operand, exprValue.operand );
					ReleaseTemporaryVariable( exprValue );
				}
				node = node->GetNextNode();
			}
		}else
		{			
			variable.type = type;
			variable.isInitialized  = (type.IsArrayType());
			variable.isTemporary	= false;
			variable.startline		= r;
	        // 保存变量类型的只读属性, 然后把其只读属性设为否,
			// 否在编译初始化表达式时会报错.
			// 注: 不要忘了恢复
			bool bReadOnly = type.IsReadOnly();
			variable.type.IsReadOnly() = false;

			bool bInitExpr = false;
			if ( node && node->GetNodeType() == snInitExpr )
				bInitExpr = true;

			// 预先分配一个变量
			if (gvar)
			{
				gvar->base = PSBASE_GLOBAL;
				variable.operand.BaseOffset( PSBASE_GLOBAL, m_pBuilder->AllocateGlobalVar(type));
			}
			else
			{
				variable.operand.BaseOffset(PSBASE_STACK, AllocateVariable(type ) );
			}
			
			bool bInit = false;
			if ( node )
			{
				// 编译变更初始化表达式/类构造
				if ( node->GetNodeType() == snInitExpr )
				{
					CompileVariableInitExpr( node, type, name, variable, bc, gvar );

					node = node->GetNextNode();
					bInit = true;
				}else if ( node->GetNodeType() == snArgList )
				{
					CompileObjectConstructor( node, type, name, variable, bc, gvar );

					node = node->GetNextNode();
					bInit = true;
				}
			}
			if ( !bInit )
			{
				// 直接调用构造函数
				if (!CompileConstructor(variable, bc))
				{
					psCString msg;
					msg.Format(TXT_OBJECT_s_CANNOT_BE_INSTANCE, type.GetFormat().AddressOf() );
					Error(msg.AddressOf(), decl);
				}
			}

			// 恢复变量类型的只读属性
			variable.type.IsReadOnly() = bReadOnly;
		}

		// 声明变量
		if (NULL == gvar)
		{
			if (m_Variables->DeclareVariable(name.AddressOf(), variable) == NULL)
			{
				psCString str;
				str.Format(TXT_s_ALREADY_DECLARED, name.AddressOf());
				Error(str.AddressOf(), decl);
			}
		}
		else
		{
			gvar->index		= variable.operand.offset;
			gvar->datatype  = variable.type;
			gvar->name		= name;
		}	
	}

	// 清除临时类变量
	ClearTemporaryVariables( bc );
}

//-------------------------------------------------------------------
// 名称: CompileVariableInitExpr
// 说明: 编译变更初始化表达式
//-------------------------------------------------------------------
void psCCompiler::CompileVariableInitExpr(psCScriptNode* node, psCDataType& type, const psCString& name, psSValue& value, psCByteCode* bc, psSGlobalVariableDesc* gvar)
{
	assert(node->GetNodeType() == snInitExpr);

	psCByteCode exprBC;
	psSValue argValue;

	if ( !CompileConstructor( value, bc ) )
	{
		psCString msg;
		msg.Format(TXT_OBJECT_s_CANNOT_BE_INSTANCE, value.type.GetFormat().AddressOf() );
		Error(msg.AddressOf(), node);
	}

	// 编译初始化表达式
	CompileInitExpression(node, &exprBC, value, argValue);
	value.isInitialized = true;

	if (type.IsPrimitive() && type.IsReadOnly() && argValue.IsConstant())
	{
		// 如果是静态常量,不要加表达式的代码
		// 也不需要变量空间
		// NOTE:在转换的同时就已经给'value'的操作符重新赋值了
		if (!ImplicitConversionConstant(argValue, type, value, NULL, node, false))
		{
			psCString msg;
			msg.Format(TXT_CANT_CONVERT_s_TO_s, argValue.type.GetFormat().AddressOf(), type.GetFormat().AddressOf() );
			Error(msg.AddressOf(), node);
		}

		if (gvar)
		{
			gvar->base = PSBASE_CONST;
			m_pBuilder->DeallocGlobalVar(type);
		}
		else
		{
			DeallocateVariable(value.operand.offset);
		}

	}else
	{
		// 添加表达式的代码
		bc->AddCode(&exprBC);
	}
}

//-------------------------------------------------------------------
// 名称: CompileVariableDim
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileVariableDim(psCScriptNode* varDim, psCByteCode* bc, psCString& name, psCDataType& dataType)
{
	assert(varDim->GetNodeType() == snVariableDim);

	psCScriptNode* node = varDim->GetFirstChild();

	// 得到变量的名称
	name.Copy(&m_pScript->m_Code[node->GetTokenPos()], node->GetTokenLength());
	node = node->GetNextNode();

	psCByteCode arrayBC;
	psSValue constValue;
	while (node)
	{
		// 这是一个数组定义 
		CompileExpression(node, &arrayBC, &constValue);

		int arraySize = m_pBuilder->GetConstantInt( constValue.operand.offset );
		if (!constValue.IsConstant() || arraySize <= 1)
		{
			Error(TXT_ARRAY_SIZE_ERROR, varDim->GetParent());
			return;
		}else if ( arraySize > 0xFFFF )
		{
			Error(TXT_ARRAY_SIZE_TOO_LARGE, varDim->GetParent());
			return;
		}
		if ( !dataType.PushArraySize(arraySize) )
		{
			Error(TXT_ARRAY_DIM_TOO_LARGE, varDim->GetParent() );
			return;
		}
		node = node->GetNextNode();
	}
}

//-------------------------------------------------------------------
// 名称: CompileInitExpression
// 说明: 编译变量初始化表达式
//		 输出参数argValue是为了给静态常量做优化而设,调用者不必再去释放它
//		 其只是做值的参考,这种情况只会出现在普通初始化表达式中
//-------------------------------------------------------------------
void psCCompiler::CompileInitExpression(psCScriptNode* initExpr, psCByteCode* bc, const psSValue& value, psSValue& argValue)
{
	// NOTE: initExpr->GetParent()得到的是declaration结点
	// 因为输出错误信息,需要token的位置信息, 只有declaration结点长保存了这些信息
	assert(initExpr && initExpr->GetNodeType() == snInitExpr);

	psSValue resultValue = value;
	psCDataType& type = resultValue.type;

	int arraySize = 0;

	// 拨出一级数组, 这样一级一级的拨一下,
	// 会最终成为基本赋值运算
	int typeArraySize = resultValue.type.PopArraySize();

	// 得到第一个赋值结点
	psCScriptNode* node = initExpr->GetFirstChild();
	while (node)
	{	
		if (node->GetNodeType() == snAssignment)
		{
			// 这是一个叶结点,执行最基本赋值运算
			CompileAssignment(node, bc, &argValue);

			if ( type.IsObject() && !type.IsPointer() )
			{
				// 类对象赋值
				ObjectAssignment( value, argValue, bc, initExpr );
			}else
			{
				// 普通类型赋值
				PerformAssignment(value, argValue, bc, initExpr->GetParent() );
			}

			// 释放临时对象
			ReleaseTemporaryVariable( argValue );
		}else
		{
			// 这是一个初始化数组的结点,

			// 进行递归运算,可能是多维数组
			CompileInitExpression(node, bc, resultValue, argValue);

			if (node->GetNextNode() != NULL)
			{
				// 数组元素大小不止一个
				resultValue.operand.offset += type.GetSizeInMemoryBytes() * type.GetNumArrayElements();
			}	
		}	

		// 更新数组的大小
		++arraySize;

		// 移到一个结点
		node = node->GetNextNode();
	}

	// 检查数组大小与类型是否相符
	if (arraySize != typeArraySize)
	{
		if (typeArraySize == 1)
		{
			Error( TXT_VARIABLE_INITEXPR_TYPE_ERROR, initExpr->GetParent() );
		}
		else
		{
			psCString msg;
			msg.Format(TXT_INIT_ARRAY_SIZE_d_DONOT_MATCH_d, arraySize, typeArraySize );
			Error( msg.AddressOf(), initExpr->GetParent() );
		}
	}
}


//-------------------------------------------------------------------
// 名称: PerformAssignment
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::PerformAssignment(const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node)
{
	// 检查变量是否为左值
	if (!IsLValue(lvalue))
	{
		Error(TXT_ASSIGN_LEFT_MUSTBE_LVALUE, node);
		return;
	}

	// 检查变量是否已经初始化
	IsVariableInitialized(rvalue, node);

	bool hasError = false;
	if ( lvalue.type.IsPointer() )
	{
		// 指针赋值
		if ( lvalue.type.IsEqualExceptConst(rvalue.type) || rvalue.isNullPointer )
		{
			bc->Instr2( BC_MOV4, lvalue.operand, rvalue.operand );
		}else if ( lvalue.type.IsObject() && rvalue.type.IsObject() && rvalue.type.IsPointer() )
		{
			// 类对象指针赋值
			int baseOffset = rvalue.type.ObjectType()->GetBaseOffset( lvalue.type.ObjectType() );
			if ( baseOffset < 0 )
				hasError = true;

			if ( baseOffset > 0 )
			{
				bc->Instr3( BC_ADDi, lvalue.operand, rvalue.operand, MakeIntValueOp(baseOffset) );
			}else
			{
				bc->Instr2( BC_MOV4, lvalue.operand, rvalue.operand );
			}
		}else if ( rvalue.type.IsArrayType() && rvalue.type.GetArrayDim() == 1 &&
				   rvalue.type.TokenType()  == lvalue.type.TokenType() &&
				   rvalue.type.ObjectType() == lvalue.type.ObjectType() )
		{
			// 数组->指针
			if ( rvalue.type.IsReference() )
				bc->Instr2( BC_MOV4, lvalue.operand, rvalue.operand );
			else
				bc->Instr2( BC_DEFREF, lvalue.operand, rvalue.operand );
		}else
		{
			hasError = true;
		}
	}else 
	{
		if ( rvalue.type.IsPointer() )
		{
			hasError = true;
		}
		else
		{	
			// 类对象赋值
			if ( lvalue.type.IsObject() )
			{
				ObjectAssignment( lvalue, rvalue, bc, node );
				return;
			}

			// 值赋值
			// 检查左/右值的类型一致性	
			psSValue tmp2;
			psCDataType type = lvalue.type;
			type.IsReference() = false;
			type.IsPointer()   = false;

			if ( ImplicitConversion(rvalue, type, tmp2, bc, node, false) )
			{
				// 如果右表达式是引用,解析出其值
				if ( tmp2.type.IsReference() )
				{
					// 重新设置右表达式的类型,分配一个临时变量
					tmp2.type.IsPointer()   = false;
					tmp2.type.IsReference() = false;
					if ( tmp2.operand == rvalue.operand )
						AllocateTemporaryVariable(tmp2);
					InstrRDREF( bc, tmp2.operand, rvalue.operand, tmp2.type );
				}

				if ( lvalue.type.IsReference() )
					InstrWTREF( bc, lvalue.operand, tmp2.operand, lvalue.type );
				else
					InstrMOV(bc, lvalue.operand, tmp2.operand, lvalue.type);
			}else
			{
				hasError = true;
			}

			// 如果是局部变量,设置初始化标志
			if (lvalue.operand.base == PSBASE_STACK && lvalue.operand.offset < 0 && 
				(!lvalue.isTemporary && !lvalue.type.IsReference() ))
			{
				// 查找局部变量
				psSVariable *v = m_Variables->GetVariableByOffset(lvalue.operand.offset);
				if (v)
				{
					// 设置初始化标志
					v->value.isInitialized = true;
				}
			}

			// 如果产生了新的临时变量,释放它
			if ( tmp2.operand != rvalue.operand )
			{
				ReleaseTemporaryVariable(tmp2);
			}
		}
	}

	if ( hasError )
	{
		psCString str;
		str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, rvalue.type.GetFormat().AddressOf(), lvalue.type.GetFormat().AddressOf());
		Error(str.AddressOf(), node);
	}	
}

//-------------------------------------------------------------------
// 名称: ImplicitConversion
// 说明: 
//-------------------------------------------------------------------
bool psCCompiler::ImplicitConversion( const psSValue& input_from, const psCDataType& type, psSValue& to, psCByteCode* bc, psCScriptNode* node, bool isExplicit)
{		
	struct auto_release_tmp_var
	{
		auto_release_tmp_var( psCCompiler* _c, const psSValue* _src, const psSValue* _dst )
		: compiler( _c ), src (_src), dst( _dst )
		{
		}
		~auto_release_tmp_var()
		{
			if ( compiler && src->operand != dst->operand )
			{
				compiler->ReleaseTemporaryVariable( *src );
			}
		}
		psCCompiler*    compiler;
		const psSValue* src;
		const psSValue* dst;
	};	

	// 设置一个默认值
	// 把类型原始信息保存起来(NOTE: 'from' 和 'to'可能是同一个实例)
	psSValue from( input_from );
	auto_release_tmp_var _auto_release( (&input_from == &to) ? this : NULL, &from, &to );

	to.type				  = type;
	to.type.IsPointer()   = from.type.IsPointer();
	to.type.IsReadOnly()  = from.type.IsReadOnly(); 
	to.type.IsReference() = from.type.IsReference();
	to.AssignExceptType( from );

	// null -> pointer
	if ( type.IsPointer() && from.isNullPointer )
	{
		to.type.IsReference() = true;
		to.type.IsPointer()   = true;
		return true;
	}

	// 检查只读属性,  
	if ( type.IsReference() )
	{
		if ( !type.IsReadOnly() && from.type.IsReadOnly() )
			return false;
	}

	// 数组->指针
	if ( type.IsPointer() && from.type.GetArrayDim() == 1 )
	{
		if ( type.m_TokenType == from.type.m_TokenType &&
			 type.ObjectType() == from.type.ObjectType() )
		{
			to.type.IsPointer()   = true;
			to.type.IsReference() = true;
			if ( bc )
			{
				AllocateTemporaryVariable(to);
				if ( from.type.IsReference() )
				{
					bc->Instr2( BC_MOV4, to.operand, from.operand );
				}else
				{
					bc->Instr2( BC_DEFREF, to.operand, from.operand );
				}
			}
			return true;
		}
		return false;
	}

	// 比较数组的大小
	if ( !from.type.IsSameArrayDim(type) )
			return false;

	// 如果是类对象,直接判断其对象类型是否一致
	if ( type.IsObject() || from.type.IsObject() )
	{
		int baseOffset = from.type.ObjectType()->GetBaseOffset( type.ObjectType() );
		if ( baseOffset < 0 )
			return false;

		to.type.IsPointer()	 = type.IsPointer();
		if ( baseOffset != 0 )
		{
			// 重新计算类对象指针	
			to.type.IsReference() = true;
			AllocateTemporaryVariable(to);	
			if ( from.type.IsReference() )
			{
				bc->Instr2( BC_MOV4, to.operand, from.operand );
			}else
			{
				bc->Instr2( BC_DEFREF, to.operand, from.operand );
			}
			bc->Instr3( BC_ADDi, to.operand, to.operand, MakeIntValueOp(baseOffset) );
		}
		return true;
	}

	if ( type.IsPointer() )
	{		
		if ( from.type.IsEqualExceptConst(type) )
		{
			to.type.IsPointer() = true;
			return true;
		}
		return false;
	}else
	{
		if ( from.type.IsPointer() )
			return false;
	}

	// 如果是常量,直接调用常量转换
	if ( from.IsConstant() )
	{
		return ImplicitConversionConstant( from, type, to, bc, node, false );
	}

	if ( from.type.IsEqualExceptRefAndConst(type) )
	{	
		return true;
	}
	
	psSValue tmpFrom( from );
	if ( from.type.IsReference() )
	{
		// 如果源是一个引用,先解析出来其值
		AllocateTemporaryVariable(tmpFrom);

		if (bc) InstrRDREF(bc, tmpFrom.operand, from.operand, from.type);
	}

	// 预先分配一个目的临时变量
	to.type.IsPointer()   = false;
	to.type.IsReference() = false;
	AllocateTemporaryVariable(to);

	bool bSuccess	 = false;	
	bool bHasWarning = false;

	if ( type.IsIntegerType() || type.IsBooleanType() )
	{		
		if ( from.type.IsIntegerType() || type.IsBooleanType() )
		{	
			int s0 = type.GetSizeInMemoryBytes();
			int s1 = from.type.GetSizeInMemoryBytes();
			if ( s0 < s1 )
			{
				if (s0 == 1)
				{
					if (bc)
					{
						if (s1 == 2)
						{	
							// int16/uint16 -> int -> int8
							bc->Instr2(from.type.IsUnsignedInteger() ? BC_W2I : BC_S2I, to.operand, tmpFrom.operand);
							bc->Instr2(BC_I2C, to.operand, to.operand);
						}else
						{
							// int->int8
							bc->Instr2(BC_I2C, to.operand, tmpFrom.operand);
						}
					}
				}else if ( s0 == 2 ) 
				{
					// int -> int16/uint16
					if (bc)
					{
						bc->Instr2(type.IsUnsignedInteger() ? BC_I2W : BC_I2S, to.operand, tmpFrom.operand);
					}
				}
				bHasWarning = true;
			}else if ( s0 > s1 )
			{
				if ( s1 == 1 )
				{
					if (bc)
					{		
						// int8/uint8 - > int
						bc->Instr2(from.type.IsUnsignedInteger() ? BC_B2I : BC_C2I, to.operand, tmpFrom.operand);

						if (s0 == 2)
						{
							// int -> int16/uint16
							bc->Instr2(type.IsUnsignedInteger() ? BC_I2W : BC_I2S, to.operand, to.operand);
						}
					}
				}else if ( s1 == 2 )
				{
					if (bc)
					{	
						// uint16/int16 -> int
						bc->Instr2(from.type.IsUnsignedInteger() ? BC_W2I : BC_S2I, to.operand, tmpFrom.operand);
					}
				}
			}else
			{
				// 'int' -> 'int'	
				ReleaseTemporaryVariable(to);
				to.AssignExceptType( tmpFrom );

				bHasWarning = !type.IsEqualExceptRefAndConst(from.type);
			}
			bSuccess = true;
		}else if ( from.type.IsFloatType() )
		{
			// 'float' -> 'int'
			if (bc) 
			{
				int s0 = type.GetSizeInMemoryBytes();

				bc->Instr2( BC_F2I, to.operand, tmpFrom.operand);
				// 'int' -> 'int8'/'int16'
				if ( s0 == 1 )
					bc->Instr2( type.IsUnsignedInteger() ? BC_I2B : BC_I2C, to.operand, to.operand );
				else if ( s0 == 2 )
					bc->Instr2( type.IsUnsignedInteger() ? BC_I2W : BC_I2S, to.operand, to.operand );
			}
			bSuccess = true;	
			bHasWarning = true;
		}else if (from.type.IsDoubleType())
		{
			// 'double' -> 'int'
			if (bc)
			{
				int s0 = type.GetSizeInMemoryBytes();

				bc->Instr2( BC_D2I, to.operand, tmpFrom.operand);

				// 'int' -> 'int8'/'int16'
				if ( s0 == 1 )
					bc->Instr2( type.IsUnsignedInteger() ? BC_I2B : BC_I2C, to.operand, to.operand );
				else if ( s0 == 2 )
					bc->Instr2( type.IsUnsignedInteger() ? BC_I2W : BC_I2S, to.operand, to.operand );
			}
			bSuccess = true;
			bHasWarning = true;
		} 
	}else if (type.IsFloatType())
	{
		if (from.type.IsIntegerType() || from.type.IsBooleanType() )
		{
			// 'int' -> 'float'
			if (bc) 
			{	
				int s1 = from.type.GetSizeInMemoryBytes();
				if ( s1 == 1 )
				{
					bc->Instr2( from.type.IsUnsignedInteger() ? BC_B2I : BC_C2I, to.operand, tmpFrom.operand );
					bc->Instr2( BC_I2F, to.operand, to.operand);
				}
				else if ( s1 == 2 )
				{
					bc->Instr2( from.type.IsUnsignedInteger() ? BC_W2I : BC_S2I, to.operand, tmpFrom.operand );	
					bc->Instr2( BC_I2F, to.operand, to.operand);
				}else
				{
					bc->Instr2( BC_I2F, to.operand, tmpFrom.operand);
				}
			}
			bSuccess = true;
			bHasWarning = true;
		}if (from.type.IsDoubleType())
		{
			// 'double' -> 'float'	
			if (bc) bc->Instr2(BC_D2F, to.operand, tmpFrom.operand);
			bSuccess = true;
			bHasWarning = true;
		}
	}else if (type.IsDoubleType())
	{
		if (from.type.IsIntegerType() || from.type.IsBooleanType() )
		{	
			// 'int' -> 'double'
			if (bc)
			{	
				int s1 = from.type.GetSizeInMemoryBytes();

				if ( s1 == 1 )
				{
					bc->Instr2( from.type.IsUnsignedInteger() ? BC_B2I : BC_C2I, to.operand, tmpFrom.operand );
					bc->Instr2( BC_I2D, to.operand, to.operand);
				}
				else if ( s1 == 2 )
				{
					bc->Instr2( from.type.IsUnsignedInteger() ? BC_W2I : BC_S2I, to.operand, tmpFrom.operand );	
					bc->Instr2( BC_I2D, to.operand, to.operand);
				}else
				{
					bc->Instr2( BC_I2D, to.operand, tmpFrom.operand);
				}
			}
			bSuccess    = true;
		}else if (from.type.IsFloatType())
		{
			// 'float' -> 'double'
			if (bc) bc->Instr2(BC_F2D, to.operand, tmpFrom.operand);
			bSuccess = true;
		}
	}
	if ( !isExplicit && bHasWarning )
	{
		// 输出警告
		psCString msg;
		msg.Format( TXT_CONVERT_s_TO_s_MAY_LOST_DATA, from.type.GetFormat().AddressOf(), type.GetFormat().AddressOf() );
		Warning( msg.AddressOf(), node );
	}

	// 释放临时变量		
	if (tmpFrom.operand != from.operand)
	{
		ReleaseTemporaryVariable(tmpFrom);
	}
	return bSuccess;
}

//-------------------------------------------------------------------
// 名称: ImplicitConversionConstant
// 说明: 
//-------------------------------------------------------------------
bool psCCompiler::ImplicitConversionConstant( const psSValue& from, const psCDataType& type, psSValue& to, psCByteCode* bc, psCScriptNode* node, bool isExplicit)
{	
	assert(from.IsConstant());

	//　如果基类相同，直接赋值后返回
	if( type.IsEqualExceptRefAndConst(from.type) )
	{
		to = from;
		to.type.IsReadOnly() = true;
		return true;
	}

	if (type.IsIntegerType())
	{	
		if( from.type.IsFloatType() )
		{
			// 'float' -> 'int'
			float fc = GetConstantFloat(from.operand);
			int ic = (int)fc;
			if( float(ic) != fc )
			{
				psCString str;
				str.Format(TXT_NOT_EXACT_g_d_g, fc, ic, float(ic));
				if( !isExplicit && node ) Warning(str.AddressOf(), node);
			}
			to.type = psCDataType(ttInt, true, false);
			to.operand = MakeIntValueOp(ic);

			// 再试一次，可能会转化到'int8', 'int16'等更小的整型
			return ImplicitConversionConstant( to, type, to, bc, node,  isExplicit);	
		}else if (from.type.IsDoubleType())
		{
			// 'double' -> 'int'
			double fc = m_pBuilder->GetConstantDouble(from.operand.offset);
			int ic = int(fc);

			if( double(ic) != fc )
			{
				psCString str;
				str.Format(TXT_NOT_EXACT_g_d_g, fc, ic, double(ic));
				if( !isExplicit && node) Warning(str.AddressOf(), node);
			}
			to.type = psCDataType(ttInt, true, false);
			to.operand = MakeIntValueOp(ic);

			// 再试一次，可能会转化到'int8', 'int16'等更小的整型
			return ImplicitConversionConstant( to, type, to, bc, node,  isExplicit);
		}else if (from.type.IsIntegerType() && from.type.GetSizeInMemoryBytes() < type.GetSizeInMemoryBytes())
		{
			// 如果是从小整型转到大整型，　直接赋值就可
			to.type = psCDataType(type.TokenType(), true, false);
			to.operand = from.operand;
			return true;
		}else if (from.type.IsIntegerType() && from.type.GetSizeInMemoryBytes() > type.GetSizeInMemoryBytes() )
		{
			//  如果是从大整型转到小整型
			if( type.GetSizeInMemoryBytes() == 1 )
			{
				// 'int' -> 'int8'
				int value;
				if (type.IsUnsignedInteger())
					value = (unsigned char)(GetConstantInt(from.operand));
				else
					value = (char)(GetConstantInt(from.operand));

				if( value != GetConstantInt(from.operand) )
					if( !isExplicit && node ) Warning(TXT_VALUE_TOO_LARGE_FOR_TYPE, node);
				
				to.operand = MakeIntValueOp(value);
			}else if ( type.GetSizeInMemoryBytes() == 2 )
			{
				// 'int' -> 'int16'
				int value;
				if (type.IsUnsignedInteger())
					value = (unsigned short)(GetConstantInt(from.operand));
				else
					value = (short)(GetConstantInt(from.operand));
				
				if( value != GetConstantInt(from.operand) )
					if( !isExplicit && node ) Warning(TXT_VALUE_TOO_LARGE_FOR_TYPE, node);

				to.operand = MakeIntValueOp(value);
			}else
			{
				// 'int' -> 'int'
				to.operand = from.operand;
			}

			to.type.TokenType() = type.TokenType(); 
			return true;
		}
	}else if (type.IsFloatType())
	{
		if (from.type.IsDoubleType())
		{
			// 'double' -> 'float'
			double ic = m_pBuilder->GetConstantDouble(from.operand.offset);
			float fc = float(ic);

			if( double(fc) != ic )
			{
				psCString str;
				str.Format(TXT_POSSIBLE_LOSS_OF_PRECISION);
				if( !isExplicit && node) Warning(str.AddressOf(), node);
			}

			to.type = psCDataType(ttFloat, true, false);
			to.operand = MakeFloatValueOp(fc);
			return true;
		}else if (from.type.IsIntegerType())
		{
			// 'int' -> 'float'
			int ic = GetConstantInt(from.operand);
			float fc = float(ic);

			if( int(fc) != ic )
			{
				psCString str;
				str.Format(TXT_NOT_EXACT_d_g_d, ic, fc, int(fc));
				if( !isExplicit && node) Warning(str.AddressOf(), node);
			}

			to.type = psCDataType(ttFloat, true, false);
			to.operand = MakeFloatValueOp(fc);	
			return true;
		}
	}else if (type.IsDoubleType())
	{
		if (from.type.IsIntegerType())
		{
			// 'int' -> 'double'
			int ic = GetConstantInt(from.operand);
			double fc = double(ic);

			if( int(fc) != ic )
			{
				psCString str;
				str.Format(TXT_NOT_EXACT_d_g_d, ic, fc, int(fc));
				if( !isExplicit && node) Warning(str.AddressOf(), node);
			}

			int offset = m_pBuilder->RegisterConstantDouble(fc);

			to.type = psCDataType(ttDouble, true, false);
			to.operand.BaseOffset(PSBASE_CONST, offset);
			return true;
		}else if (from.type.IsFloatType())
		{	
			// 'float' -> 'double'
			int offset = m_pBuilder->RegisterConstantDouble(GetConstantFloat(from.operand));
			to.type = psCDataType(ttDouble, true, false);
			to.operand.BaseOffset(PSBASE_CONST, offset);
			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------
// 名称: CompileAssignment
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileAssignment(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue)
{
	psCScriptNode *lexpr = expr->GetFirstChild();
	if( lexpr->GetNextNode() )
	{
		// 这是一个赋值表达式
		psCByteCode rbc, lbc;
		psSValue rvalue, lvalue;
		psCScriptNode* opNode = lexpr->GetNextNode();

		// 编译赋值表达式的左/右原子
		CompileAssignment(opNode->GetNextNode(), &rbc, &rvalue);
		CompileCondition(lexpr, &lbc, &lvalue);
		
		int op = opNode->GetTokenType();
		// 如果是左值表达式是一个类，检查操作符是否已经被重载
		// 如果没有那就是一个简单的赋值运算
		if( lvalue.type.IsObject() && !lvalue.type.IsPointer() )
		{	
			// 先加右表达式代码，再加左表达式代码
			bc->AddCode(&rbc);
			bc->AddCode(&lbc);
			
			// 对类对象进行赋值
			ObjectAssignmentEx( op, exprValue, lvalue, rvalue, bc, expr );

			// 释放临时对象
			ReleaseTemporaryVariable( rvalue );
			ReleaseTemporaryVariable( lvalue );
			return;
		}else
		{
			// 对简单数据类型赋值
			if (op == ttAssignment)
			{
				// 是一个简单的赋值操作
				// 先加右表达式代码，再加左表达式代码
				bc->AddCode(&rbc);
				bc->AddCode(&lbc);

				PerformAssignment(lvalue, rvalue, bc, expr);
			}
			else
			{		
				// 是一个带运算操作的赋值操作
				psSValue tmp;
				CompileOperator(opNode, bc, &lbc, &rbc, &tmp, &lvalue, &rvalue);
				
				PerformAssignment(lvalue, tmp, bc, expr );

				ReleaseTemporaryVariable( tmp );
			}
			// 释放临时变量
			ReleaseTemporaryVariable( rvalue );

			// 把表达式的值设为左表达式的值
			// 固左表达式就不要释放了
			*exprValue = lvalue;
		}	
	}else
	{
		// 简单表达式
		CompileCondition(lexpr, bc, exprValue);
	}
}

//-------------------------------------------------------------------
// 名称: CompileCondition
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileCondition(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue)
{
	psCScriptNode* cexpr = expr->GetFirstChild();
	if (cexpr->GetNextNode())
	{
		// 保存原来的临时变量
		std::vector< psSValue > oldTempValues = m_TempValues;
		m_TempValues.clear();

		// 条件表达式
		psCByteCode ebc;
		psSValue cvalue;
		CompileExpression(cexpr, &ebc, &cvalue);

		// 检查表达式的类型是否为'bool'类型
		// 'int'和'unsigned int'也可以当做'bool'型
		if ( !cvalue.type.IsBooleanType() &&
			 !cvalue.type.IsIntegerType() ||
			  cvalue.type.IsPointer() )
		{
			Error(TXT_EXPR_MUST_BE_BOOL, expr);
		}

		psSOperand cvalueOp;
		// 确认其是四字节整型常量
		GetIntOperand( &ebc, cvalue, cvalueOp );

		psCByteCode lebc, rebc;
		psSValue lvalue, rvalue;

		// 编译左表达式
		CompileAssignment(cexpr->GetNextNode(), &lebc, &lvalue);
		
		// 分配一个临时变量	
		// NOTE: 暂时把其临时属性设为否
		// 因为临时变量是不能被赋值的
		// 在赋值完毕后,要把其临时属性设为真
		exprValue->type = lvalue.type;
		exprValue->type.IsPointer() = false;
		exprValue->type.IsReadOnly()      = false;
		AllocateTemporaryVariable(*exprValue);

		exprValue->isTemporary = false;

		// 输出二进制代码
		int elseLabel = m_NextLabel++;
		int afterLabel = m_NextLabel++;

		// 先添加条件表达式代码
		bc->AddCode(&ebc);

		// 如果条件表达式的值为'false'，则跳到右表达式代码
		bc->Instr2(BC_JZ, cvalueOp, psSOperand(elseLabel));

		// 添加左表达式代码
		bc->AddCode(&lebc);

		// 给表达式的值赋值
		if ( lvalue.type.IsObject() )
		{
			// 拷贝构造这个临时类对象
			CopyConstructObject( *exprValue, lvalue, bc, expr );
		}else
		{
			PerformAssignment(*exprValue, lvalue, bc, expr);
		}

		// 释放临时类对象
		ReleaseTemporaryVariable( lvalue );

		// 左表达式中的临时变量的生命期到了
		// NOTE: 注意必需在编译右表达式之前释放 
		//       左表达式生成的临时变量, 因为如果条件
		//       表达式的值为'false'左表达式运行时没有生成临时变量,
		//       而在编译时却生成了, 故一定要在此释放
		ClearTemporaryVariables(bc);

		// 编译右表达式
		CompileAssignment(cexpr->GetNextNode()->GetNextNode(), &rebc, &rvalue);
		psSValue tmp;
		// 检查右表达式类型能否转换到左表达式
		if (!ImplicitConversion(rvalue, lvalue.type, tmp, &rebc, expr, false))
		{
			psCString str;
			str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, rvalue.type.GetFormat().AddressOf(), lvalue.type.GetFormat().AddressOf());
			Error(str.AddressOf(), expr);
			return;
		}		

		// 跳出
		bc->Instr1(BC_JMP, psSOperand(afterLabel));

		// 添加右表达式代码
		bc->Label(elseLabel);
		bc->AddCode(&rebc);
		
		// 给表达式的值赋值
		if (tmp.type.IsObject())
		{
			// 拷贝构造这个临时类对象
			CopyConstructObject( *exprValue, tmp, bc, expr  );
		}else
		{
			// 赋值
			PerformAssignment(*exprValue, tmp, bc, expr);
		}	
		// 释放临时变量
		ReleaseTemporaryVariable( rvalue );
		if ( tmp.operand != rvalue.operand )
			ReleaseTemporaryVariable( tmp );

		// 右表达式中的临时变量的生命期到了
		ClearTemporaryVariables(bc);

	    bc->Label(afterLabel);

		// 释放临时变量
		ReleaseTemporaryVariable(cvalue);
		if ( cvalueOp != cvalue.operand )
			ReleaseTemporaryOperand( cvalueOp );

		// 条件表达式中的临时变量的生命期到了
		ClearTemporaryVariables(bc);

		// 设置为临时变量
		exprValue->isTemporary = true;

		// 恢复原来的临时变量
		m_TempValues = oldTempValues;
	}else 
	{
		// 数学表达式
		CompileExpression(cexpr, bc, exprValue);
	}
}

//-------------------------------------------------------------------
// 名称: CompileExpression
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileExpression(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue)
{	
	assert(expr->GetNodeType() == snExpression);

	// 转换到逆波兰表达式. i.e: a+b => ab+
	std::stack< psCScriptNode* > S1, S2;

	psCScriptNode *node = expr->GetFirstChild();
	while (node)
	{
		int precedence = GetPrecedence(node);
		while( !S1.empty() && 
			precedence <= GetPrecedence(S1.top()))
		{
			S2.push(S1.top());
			S1.pop();
		}
		S1.push(node);

		node = node->GetNextNode();
	}

	while (!S1.empty())
	{
		S2.push(S1.top());
		S1.pop();
	}

	std::vector< psCScriptNode* > postfix;
	postfix.reserve(S2.size());

	// 转换结合性为左结合
	SwapPostFixOperands(S2, postfix);

	// 算出所有表达式的的值
	psCByteCode exprBC;
	CompilePostFixExpression(postfix, &exprBC, exprValue);

	// 添加表达式代码
	bc->AddCode(&exprBC);

	postfix.clear();
}

void psCCompiler::SwapPostFixOperands(std::stack<psCScriptNode*>& postfix, std::vector< psCScriptNode* >& target)
{
	if (postfix.size() == 0) return;

	psCScriptNode* node = postfix.top();
	postfix.pop();

	if (node->GetNodeType() == snExprTerm)
	{
		target.push_back(node);
		return;
	}
	SwapPostFixOperands(postfix, target);
	SwapPostFixOperands(postfix, target);

	target.push_back(node);
}

int psCCompiler::GetPrecedence(psCScriptNode* op)
{
	// x*y, x/y, x%y
	// x+y, x-y
	// x<=y, x<y, x>=y, x>y
	// x==y, x!=y
	// x and y
	// (x xor y)
	// x or y

	// The following are not used in this function,
	// but should have lower precedence than the above
	// x ? y : z
	// x = y

	// The expression term have the highest precedence
	if( op->GetNodeType() == snExprTerm )
		return 1;

	// Evaluate operators by token
	int tokenType = op->GetTokenType();
	if( tokenType == ttStar || tokenType == ttSlash || tokenType == ttPercent )
		return 0;

	if( tokenType == ttPlus || tokenType == ttMinus )
		return -1;

	if( tokenType == ttBitShiftLeft ||
		tokenType == ttBitShiftRight )
		return -2;

	if( tokenType == ttAmp )
		return -3;

	if( tokenType == ttBitXor )
		return -4;

	if( tokenType == ttBitOr )
		return -5;

	if( tokenType == ttLessThanOrEqual ||
		tokenType == ttLessThan ||
		tokenType == ttGreaterThanOrEqual ||
		tokenType == ttGreaterThan ||
		tokenType == ttShiftLeftAssign ||
		tokenType == ttShiftRightAssign )
		return -6;

	if( tokenType == ttEqual || tokenType == ttNotEqual || tokenType == ttXor )
		return -7;

	if( tokenType == ttAnd )
		return -8;

	if( tokenType == ttOr )
		return -9;

	// Unknown operator
	assert(false);

	return 0;
}

//-------------------------------------------------------------------
// 名称: CompilePostFixExpression
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompilePostFixExpression(std::vector< psCScriptNode* >& postfix, psCByteCode* bc, psSValue* exprValue)
{	
	// 弹出最后一个结点
	psCScriptNode* node = postfix[postfix.size() - 1];
	postfix.pop_back();

	if (node->GetNodeType() == snExprTerm)
	{
		CompileExpressionTerm(node, bc, exprValue);
		return;
	}else
	{
		// 编译两个表达式原子
		psCByteCode rbc, lbc;
		psSValue rvalue, lvalue;

		// NOTE!!!
		// 如果是bool运算符却需要立即清除临时对像
		// 因为内里有跳转语句，不能等到整个表达式结束时一起清除
		int boolOp = (node->GetTokenType() == ttOr ||
					  node->GetTokenType() == ttAnd );

		// 编译左表达式
		CompilePostFixExpression(postfix, &lbc, &lvalue);
		if (boolOp)
		{
			ClearTemporaryVariables(&lbc);
		}
		// 编译右表达式
		CompilePostFixExpression(postfix, &rbc, &rvalue);
		if (boolOp)
		{
			ClearTemporaryVariables(&rbc);
		}

		// 编译操作符
		CompileOperator(node, bc, &lbc, &rbc, exprValue, &lvalue, &rvalue);

		// 释放临时变量
		ReleaseTemporaryVariable( rvalue );	
		ReleaseTemporaryVariable( lvalue );
	}
}


void psCCompiler::CompileExpressionTerm(psCScriptNode* term, psCByteCode* bc, psSValue* exprValue)
{
	psCScriptNode* vnode = term->GetFirstChild();
	while (vnode && vnode->GetNodeType() != snExprValue)
		vnode = vnode->GetNextNode();

	// 编译表达式值结点
	psCByteCode vbc;
	CompileExpressionValue(vnode, &vbc, exprValue );
	
	// 编译后置操作符
	psCScriptNode *pnode = vnode->GetNextNode();
	while( pnode )
	{
		CompileExpressionPostOp(pnode, &vbc, exprValue);
		pnode = pnode->GetNextNode();

	}

	// 编译前置操作符
	pnode = vnode->GetPrevNode();
	while ( pnode )
	{		
		CompileExpressionPrevOp(pnode, &vbc, exprValue);
		pnode = pnode->GetPrevNode();	
	}

	// 添加二进制代码
	bc->AddCode(&vbc);
}


//-------------------------------------------------------------------
// 名称: CompileExpressionPostOp
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileExpressionPostOp(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue)
{
	// 得到操作符
	int op = node->GetTokenType();

	// 检查变量是否已经初始化
	IsVariableInitialized(*exprValue, node);

	if ( op == ttInc || op == ttDec )
	{
		if ( exprValue->type.IsPointer() )
		{
			if ( !IsLValue(*exprValue) )
			{
				Error(TXT_ASSIGN_LEFT_MUSTBE_LVALUE, node);
				return;
			}
			
			int size = exprValue->type.GetSizeInMemoryBytes();
			if (size == 0)
			{
				Error(TXT_ILLEGAL_OPERATION, node);
				return;
			}
			psSValue tmp( exprValue->type );
			AllocateTemporaryVariable( tmp );

			psSOperand offset = MakeIntValueOp(size);
			bc->Instr2( BC_MOV4, tmp.operand, exprValue->operand );
			bc->Instr3( (op == ttInc) ? BC_ADDi : BC_SUBi, exprValue->operand, exprValue->operand, offset );

			// 释放临时变量
			ReleaseTemporaryVariable( *exprValue );
			exprValue->AssignExceptType( tmp );
		}else
		{
			if ( exprValue->type.IsObject() )
				CompileObjectPostOp( op, node, bc, exprValue );
			else
				CompilePrimitivePostOp( op, node, bc, exprValue );
		}
	}else if ( op == ttDot || op == ttArrow )
	{	
		if ( !exprValue->type.IsObject() ||
			 (exprValue->type.IsPointer() && op == ttDot ) ||
			 (!exprValue->type.IsPointer() && op == ttArrow ) )
		{
			Error( TXT_ILLEGAL_OPERATION, node );
			return;
		}
		CompilePostOpDot( node, bc, exprValue );

	}else if ( op == ttOpenBracket )
	{
		CompilePostOpOpenBracket( node, bc, exprValue );
	}else
	{
		Error(TXT_ILLEGAL_OPERATION, node);
	}
}

void psCCompiler::CompilePrimitivePostOp( int op, psCScriptNode* node, psCByteCode* bc, psSValue* exprValue )
{
	assert( !exprValue->type.IsPointer() );
	assert( exprValue->type.IsPrimitive() );

	if (op == ttInc || op == ttDec )
	{
		// 产生一个临时变量
		psSValue ret(exprValue->type);
		AllocateTemporaryVariable(ret);

		// 拷贝数据到返回值中
		if ( exprValue->type.IsReference() )
			InstrRDREF( bc, ret.operand, exprValue->operand, exprValue->type );
		else
			InstrMOV( bc, ret.operand, exprValue->operand, exprValue->type );

		psSValue tmp = *exprValue;
		if ( exprValue->type.IsReference() )
		{
			AllocateTemporaryVariable( tmp );
			InstrMOV( bc, tmp.operand, ret.operand, tmp.type );
		}

		if (exprValue->type.IsIntegerType() )
		{
			if (op == ttDec)
				bc->Instr1(BC_DECi, tmp.operand);
			else
				bc->Instr1(BC_INCi, tmp.operand);
		}else if (tmp.type.IsFloatType())
		{
			if (op == ttDec)
				bc->Instr1(BC_DECf, tmp.operand);
			else
				bc->Instr1(BC_INCf, tmp.operand);
		}else if (tmp.type.IsDoubleType())
		{
			if (op == ttDec)
				bc->Instr1(BC_DECd, tmp.operand);
			else
				bc->Instr1(BC_INCd, tmp.operand);
		}else
		{
			Error(TXT_ILLEGAL_OPERATION, node);
		}	

		// 释放临时变量
		ReleaseTemporaryVariable( *exprValue );
		// 回写指针数据
		if ( exprValue->type.IsReference() )
		{
			InstrWTREF( bc, exprValue->operand, tmp.operand, exprValue->type );
			ReleaseTemporaryVariable( tmp );
		}

		// 设置表达式的值
		*exprValue = ret;
		exprValue->type.IsReference() = false;
	}else
	{
		Error( TXT_ILLEGAL_OPERATION, node );
	}

}

//-------------------------------------------------------------------
// 名称: CompileExpressionPrevOp
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileExpressionPrevOp(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue)
{
	// 得到操作符
	int op = node->GetTokenType();

	// 变量是否已经初始化
	IsVariableInitialized(*exprValue, node);
	
	if ( exprValue->type.IsPointer() )
	{
		if ( op == ttStar )
		{
			if (exprValue->type.GetSizeInMemoryBytes() == 0)
			{
				Error(TXT_ILLEGAL_OPERATION, node);
				return;
			}
			exprValue->type.IsPointer()   = false;
			exprValue->type.IsReference() = true;
		}else if ( op == ttInc || op == ttDec )
		{
			if ( !IsLValue(*exprValue) )
			{
				Error(TXT_ASSIGN_LEFT_MUSTBE_LVALUE, node);
				return;
			}
			int size = exprValue->type.GetSizeInMemoryBytes();
			if (size == 0)
			{
				Error(TXT_ILLEGAL_OPERATION, node);
				return;
			}
			psSOperand offset = MakeIntValueOp(size);
			bc->Instr3( (op == ttInc) ? BC_ADDi : BC_SUBi, exprValue->operand, exprValue->operand, offset );
		}else
		{
			Error( TXT_ILLEGAL_OPERATION, node );
		}
	}else
	{
		if (op == ttAmp)
		{
			// 解析地址值
			// NOTE: 不能解析临时变量的地址值
			//		 也不能解析数组的地址值
			if ( exprValue->type.IsArrayType()	 || 
				 exprValue->type.IsPointer()   )
			{
				psCString msg;
				msg.Format( TXT_INAVLID_GET_s_POINTER, exprValue->type.GetFormat().AddressOf() );
				Error( msg.AddressOf(), node );
			}else
			{	
				if ( !exprValue->type.IsReference() )
				{
					// 产生一临时变量
					psSValue tmp(exprValue->type);
					tmp.type.IsPointer()   = true;	
					tmp.type.IsReference() = true;
 					AllocateTemporaryVariable(tmp);

					bc->Instr2(BC_DEFREF, tmp.operand, exprValue->operand);

					// 释放临时变量
					ReleaseTemporaryVariable( *exprValue );

					// 重新设置表达式的值
					*exprValue = tmp;
				}else
				{
					exprValue->type.IsPointer() = true;
				}
			}
		}else
		{
			if ( exprValue->type.IsObject() )
			{
				if ( op == ttMinus || op == ttPlus ||
				 	 op == ttInc   || op == ttDec  || 
					 op == ttNot   || op == ttBitNot )
				{
					CompileObjectPrevOp( op, node, bc, exprValue );
				}else
				{
					Error( TXT_ILLEGAL_OPERATION, node );
				}
			}else
			{
				CompilePrimitivePrevOp( op, node, bc, exprValue );
			}
		}
	}
}

void psCCompiler::CompilePrimitivePrevOp( int op, psCScriptNode* node, psCByteCode* bc, psSValue* exprValue )
{
	assert( !exprValue->type.IsPointer() );

	if (op == ttMinus )
	{	
		if (exprValue->IsConstant())
		{
			// 如果是常数求反
			if (exprValue->type.IsFloatType())
			{
				exprValue->operand = MakeFloatValueOp(-GetConstantFloat(exprValue->operand));
			}
			else if (exprValue->type.IsIntegerType())
			{
				exprValue->operand = MakeIntValueOp(-GetConstantInt(exprValue->operand));
			}
			else if (exprValue->type.IsDoubleType())
			{
				double d = m_pBuilder->GetConstantDouble(exprValue->operand.offset);
				int offset = m_pBuilder->RegisterConstantDouble(-d);
				exprValue->operand.offset = offset;
			}else
			{
				Error(TXT_ILLEGAL_OPERATION, node);
			}
			return;
		}

		// 产生一个临时变量
		psSValue tmp(exprValue->type);
		AllocateTemporaryVariable( tmp );

		// 如果是指针，先解析其值
		psSValue tmpExpr = *exprValue;
		if ( exprValue->type.IsReference() )
		{
			AllocateTemporaryVariable( tmpExpr );
			InstrRDREF( bc, tmpExpr.operand, exprValue->operand, tmpExpr.type );
		}

		if (tmp.type.IsIntegerType())
		{
			bc->Instr2(BC_NEGi, tmp.operand, tmpExpr.operand);
		}else if (tmp.type.IsFloatType())
		{
			bc->Instr2(BC_NEGf, tmp.operand, tmpExpr.operand);
		}else if (tmp.type.IsDoubleType())
		{
			bc->Instr2(BC_NEGd, tmp.operand, tmpExpr.operand);
		}else
		{
			Error(TXT_ILLEGAL_OPERATION, node);
		}

		// 释放临时变量
		ReleaseTemporaryVariable( *exprValue );
		if ( exprValue->type.IsReference() )
		{
			ReleaseTemporaryVariable( tmpExpr );
		}

		// 重新设置属性值(重新设置引用属性)
		*exprValue = tmp;	
		exprValue->type.IsReference() = false;
	}else if (op == ttBitNot)
	{
		if ( (exprValue->type.IsIntegerType())  )
		{
			if (exprValue->IsConstant())
			{
				exprValue->operand = MakeIntValueOp( ~GetConstantInt(exprValue->operand) );
			}else
			{
				// 产生一临时变量
				psSValue tmp(exprValue->type);
				AllocateTemporaryVariable(tmp);

				// 如果是指针，先解析其值
				psSValue tmpExpr = *exprValue;
				if ( exprValue->type.IsReference() )
				{
					AllocateTemporaryVariable( tmpExpr );
					InstrRDREF( bc, tmpExpr.operand, exprValue->operand, tmpExpr.type );
				}

				bc->Instr2(BC_NOT, tmp.operand, tmpExpr.operand);

				// 释放临时变量
				ReleaseTemporaryVariable( *exprValue );
				if ( exprValue->type.IsReference() )
				{
					ReleaseTemporaryVariable( tmpExpr );
				}

				// 重新设置属性值
				*exprValue = tmp;		
				exprValue->type.IsReference() = false;
			}
		}else
		{
			Error(TXT_ILLEGAL_OPERATION, node);
		}
	}else if (op == ttDec || op == ttInc)
	{
		if ( !IsLValue(*exprValue) )
		{
			Error(TXT_ASSIGN_LEFT_MUSTBE_LVALUE, node);
		}else
		{		
			// 如果是指针，先解析其值
			psSValue tmpExpr = *exprValue;
			if ( exprValue->type.IsReference() )
			{
				AllocateTemporaryVariable( tmpExpr );
				InstrRDREF( bc, tmpExpr.operand, exprValue->operand, tmpExpr.type );
			}

			if (exprValue->type.IsIntegerType() )
			{
				if (op == ttDec)
					bc->Instr1(BC_DECi, tmpExpr.operand);
				else
					bc->Instr1(BC_INCi, tmpExpr.operand);
			}else if (exprValue->type.IsFloatType())
			{
				if (op == ttDec)
					bc->Instr1(BC_DECf, tmpExpr.operand);
				else
					bc->Instr1(BC_INCf, tmpExpr.operand);
			}else if (exprValue->type.IsDoubleType())
			{
				if (op == ttDec)
					bc->Instr1(BC_DECd, tmpExpr.operand);
				else
					bc->Instr1(BC_INCd, tmpExpr.operand);
			}else
			{
				Error(TXT_ILLEGAL_OPERATION, node);
			}
			// 回写
			if ( exprValue->type.IsReference() )
			{
				InstrWTREF( bc, exprValue->operand, tmpExpr.operand, exprValue->type );
				ReleaseTemporaryVariable( tmpExpr );
			}
		}
	}else if (op == ttNot)
	{
		// 只有'bool,'int','unsigned int'型，对此操作符才有效
		if( exprValue->type.IsBooleanType() ||
			exprValue->type.IsIntegerType() )
		{		
			// 产生一临时变量
			psSValue tmp(g_BoolDataType);
			AllocateTemporaryVariable( tmp );

			// 如果是指针，先解析其值
			psSValue tmpExpr = *exprValue;
			if ( exprValue->type.IsReference() )
			{
				AllocateTemporaryVariable( tmpExpr );
				InstrRDREF( bc, tmpExpr.operand, exprValue->operand, tmpExpr.type );
			}

			bc->Instr3(BC_TEi, tmp.operand, tmpExpr.operand, MakeIntValueOp(0) );

			// 释放临时变量
			ReleaseTemporaryVariable( *exprValue );
			if ( exprValue->type.IsReference() )
			{
				ReleaseTemporaryVariable( tmpExpr );
			}

			// 重新设置表达式的值
			*exprValue = tmp;	
			exprValue->type.IsReference() = false;
		}else
		{
			psCString str;
			str.Format(TXT_NO_CONVERSION_s_TO_s, exprValue->type.GetFormat().AddressOf(), "bool/int");
			Error(str.AddressOf(), node);
		}
	}else if (op == ttPlus )
	{
		// Do nothing!
		return;
	}else
	{
		Error( TXT_ILLEGAL_OPERATION, node );
	}
}

//-------------------------------------------------------------------
// 名称: CompileExpressionValue
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileExpressionValue(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue)
{

	psCScriptNode *vnode = expr->GetFirstChild();
	if( vnode->GetNodeType() == snIdentifierValue )
	{
		CompileIdentifierValue( vnode, bc, exprValue );
	}else if (vnode->GetNodeType() == snConstant)
	{
		// 得到常量字符串
		psCHAR buf[256];
		GetNodeSource(buf, 255, vnode );

		exprValue->isTemporary			    = false;
		exprValue->isInitialized		    = true;
		exprValue->type.IsReference() = false;
		exprValue->type.IsPointer()   = false;
		exprValue->type.IsReadOnly()        = true;

		exprValue->type.SetObjectType( NULL );

		psCHAR* end = NULL;
		if (vnode->GetTokenType() == ttIntConstant)
		{
			// 整型常量
			int offset = m_pBuilder->RegisterConstantInt( ps_strtol(buf, &end, 0) );
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
			exprValue->type.TokenType() = ttInt;
	
		}else if ( vnode->GetTokenType() == ttCharConstant ||
				   vnode->GetTokenType() == ttWCharConstant )
		{
			int len =  vnode->GetTokenLength();
			if ( len > 8 )
			{
				len = 8;
			}
			psCHAR str[ 8 ];
			memcpy( str, buf, (len)*sizeof(psCHAR) );
			
			int offset  = 0;
			int std_len = 0;
			if ( vnode->GetTokenType() == ttCharConstant )
			{
				exprValue->type.TokenType() = ttInt8;
				offset = m_pBuilder->RegisterConstantCharA( str, len );
				std_len = 3;
			}else
			{
				exprValue->type.TokenType() = ttWChar; 
				offset = m_pBuilder->RegisterConstantCharW( str, len );
				std_len = 4;
			}
			if (len < std_len )	
			{
				Error( TXT_CHAR_INIT_EXPR_TOO_FEW, vnode );
			}else if ( len > std_len )
			{
				Error( TXT_CHAR_INIT_EXPR_TOO_MANY,vnode );
			}
			// 字符常量
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
		}else if (vnode->GetTokenType() == ttBitsConstant)
		{
			// 二进制整型常量
			int val = 0;
			ps_sscanf(buf, PS_T("%x"), &val);

			int offset = m_pBuilder->RegisterConstantInt(val);
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
			exprValue->type.TokenType() = ttInt;

		}else if (vnode->GetTokenType() == ttFloatConstant)
		{
			// 浮点型号常量
			int offset = m_pBuilder->RegisterConstantFloat( (float)ps_strtod(buf, &end) );
			exprValue->type.TokenType() = ttFloat;
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);

		}else if (vnode->GetTokenType() == ttTrue ||
				  vnode->GetTokenType() == ttFalse)
		{
			// 逻辑型常量
			int offset = m_pBuilder->RegisterConstantInt((vnode->GetTokenType() == ttTrue) ? 1 :0);
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
			exprValue->type.TokenType() = ttBool;

		}else if (vnode->GetTokenType() == ttNull)
		{
			// 空指针常量
			int offset = m_pBuilder->RegisterConstantInt(0);
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
			exprValue->type.TokenType()   = ttInt;
			exprValue->type.IsReference() = true;
			exprValue->type.IsPointer()	  = true;
			exprValue->isNullPointer	  = true;

		}else if (vnode->GetTokenType() == ttDoubleConstant)
		{
			// 'double'型常量
			int offset = m_pBuilder->RegisterConstantDouble(ps_strtod(buf, &end));
			exprValue->operand = psSOperand(PSBASE_CONST, offset);
			exprValue->type.TokenType() = ttDouble;
		}else if (vnode->GetTokenType() == ttStringConstant ||
				  vnode->GetTokenType() == ttWStringConstant )
		{
			// 字符串常量
			psCString str, cat;

			int string_start = 1;
			if ( vnode->GetTokenType() == ttWStringConstant )
			{		
				++string_start;
			}

			// 处理多行字符串 i.e. "good"
			//					   "hello"
			psCScriptNode *snode = vnode->GetFirstChild();
			while( snode )
			{
				cat.Copy(&m_pScript->m_Code[snode->GetTokenPos()+string_start], (snode->GetTokenLength()-(1+string_start) ) );

				str += cat;

				snode = snode->GetNextNode();
			}

			// 注删字符串常量
			int offset = 0;
			if ( vnode->GetTokenType() == ttWStringConstant )
			{			
				exprValue->type.TokenType()   = ttWChar;
				offset = m_pBuilder->RegisterConstantStrW( str );
			}else
			{		
				exprValue->type.TokenType()   = ttChar;
				offset = m_pBuilder->RegisterConstantStrA( str );
			}
			exprValue->type.IsPointer()   = false;
			exprValue->type.IsReference() = false;
			exprValue->type.m_ArrayDim	  = 0;
			exprValue->type.PushArraySize( (int)str.GetLength() + 1 );

			exprValue->operand.BaseOffset( PSBASE_CONST, offset );
		}else if (vnode->GetTokenType() == ttThis)
		{
			// this指针值
			if ( m_ObjectType == NULL )
			{
				// 如果不是类成员函数，this指针无效
				Error(TXT_INVALID_THIS_POINTER, vnode);

				// 保证继续编译
				exprValue->type = psCDataType(ttInt, false, false);
				AllocateTemporaryVariable( *exprValue );
			}else
			{
				// 设置属性值
				exprValue->type				  = m_ObjectType->m_DataType;
				exprValue->type.IsReadOnly()  = false;
				exprValue->type.IsPointer()   = true;
				exprValue->type.IsReference() = true;

				// 分配一临时操作数
				AllocateTemporaryVariable(*exprValue);

				// 得到this指针的地址值
				bc->Instr2(BC_MOV4, exprValue->operand, psSOperand(PSBASE_STACK, 0) );
			}
		}else
		{
			assert(false);
		}
	}else if(vnode->GetNodeType() == snAssignment)
	{
		CompileAssignment(vnode, bc, exprValue);
	}else if (vnode->GetNodeType() == snFunctionCall)
	{
		CompileFunctionCall(vnode, bc, exprValue, NULL );
	}
	else if (vnode->GetNodeType() == snSizeof )
	{
		CompileSizeof( vnode, bc, exprValue );
	}else if (vnode->GetNodeType() == snTypeCast )
	{
		CompileTypeCast( vnode, bc, exprValue );
	}else
	{
		assert(false);
	}
}


void psCCompiler::CompileIdentifierValue( psCScriptNode* vnode, psCByteCode* bc, psSValue* exprValue )
{
	assert( vnode->GetNodeType() == snIdentifierValue );

	// 域名
	psCHAR field[256];
	bool   hasField = false;

	psCScriptNode* n = vnode->GetFirstChild();
	if ( n->GetNextNode() )
	{
		GetNodeSource( field, 255, n );
		hasField = true;
		n = n->GetNextNode();
	}else
	{
		if ( vnode->GetTokenType() == ttField )
		{
			// ::
			field[0] = 0;
			hasField = true;
		}
	}

	// 得到变量名
	psCHAR name[256];
	GetNodeSource( name, 255, n );

	//bool hasError = false;

	if ( hasField )
	{
		if (field[0] == 0 )
		{
			// 全局属性
			if ( !GetGlobalProperty( name, bc, exprValue ) )
			{
				// Error
				psCString str;
				str.Format(TXT_s_NOT_DECLARED, name );
				Error(str.AddressOf(), vnode);
			}
		}else
		{
			// 是否为类域的枚举常量
			int value;
			if ( m_pBuilder->GetConstantEnum( field, name, value) )
			{	
				exprValue->type			 = g_IntDataType;
				exprValue->isTemporary   = false;
				exprValue->isInitialized = true;
				exprValue->operand		 = MakeIntValueOp( value );
			}else
			{
				psCString str;
				str.Format(TXT_s_NOT_DECLARED, name );
				Error(str.AddressOf(), vnode);
			}
		}
	}else
	{
		psSVariable* var = m_Variables->GetVariable(name);
		if (var == NULL)
		{
			// 如果不是局部变量,,看是否为类的成员属性
			if ( m_ObjectType && GetObjectProperty( m_ObjectType, name, exprValue ) )
				return;

			// 是否为全局变量
			if ( GetGlobalProperty( name, bc, exprValue ) )
				return;

			// Error
			psCString str;
			str.Format(TXT_s_NOT_DECLARED, name );
			Error(str.AddressOf(), vnode);
		}else
		{	
			// 局部变量/数组
			*exprValue = var->value; 

			// NOTE: 重新设置临时属性
			// 因为有可能一个引用类型号的变量是临时的, 最后要被释放
			// 这时isTempoary的值为真, 如果此时不设置临时属性,
			// 这个临时变量就会被释放多次
			exprValue->isTemporary   = false;		
			exprValue->isInitialized = true;
		}
	}
}

bool psCCompiler::GetGlobalProperty( const psCHAR* name, psCByteCode* bc, psSValue* exprValue)
{
	// 在全局变量中查找
	psSProperty* prop = m_pBuilder->GetGlobalProperty(name);
	if (prop)
	{
		// 设置值的类型　
		exprValue->type = prop->type;

		// 设置属性值
		exprValue->isTemporary   = false;
		exprValue->isInitialized = true;

		// 如果属性的基址方式为立即寻址,则其为整型常量
		if ( prop->base == PSBASE_NONE )
		{
			// 整型常量
			if ( exprValue->type.IsIntegerType() )
			{
				exprValue->operand = MakeIntValueOp( prop->index );
			}
			else if (exprValue->type.IsFloatType() )
			{
				float* p = (float*)(&(prop->index));
				exprValue->operand = MakeFloatValueOp(*p);
			}
		}else
		{
			// 如果全局属性的索引小于0,则是系统的全局对象,否则为模块的全局变量
			if (prop->index < 0)
			{
				// 表达式的引用属性为真
				exprValue->type.IsReference() = true;

				// 分配一个临时操作数
				AllocateTemporaryVariable(*exprValue);

				// 得到属性的地址值(因为类型是引用)
				bc->Instr2(BC_PGA, exprValue->operand, prop->index);
			}else
			{
				// 得到全局属性操作数
				exprValue->operand.BaseOffset(prop->base, prop->index);
			}
		}
		return true;
	}

	// 是否为枚举常量
	int value;
	if ( m_pBuilder->GetConstantEnum(PS_T(""), name, value) )
	{	
		exprValue->type			 = g_IntDataType;
		exprValue->isTemporary   = false;
		exprValue->isInitialized = true;
		exprValue->operand		 = MakeIntValueOp( value );
		return true;
	}
	return false;
}

bool psCCompiler::GetObjectProperty( psCObjectType* objType, const psCHAR* name, psSValue* exprValue )
{
	// 此函数为类的成员函数, 此变量有可能为类的成员
	int baseOffset = 0;
	psSProperty *prop = m_pBuilder->GetObjectProperty( objType, name, baseOffset );
	if( prop )
	{	
		// 设置属性值
		exprValue->type					 = prop->type;
		exprValue->isInitialized		 = true;
		exprValue->operand.BaseOffset( PSBASE_HEAP, prop->byteOffset + baseOffset );
		return true;
	}

	// 是否为类域的枚举常量
	int value;
	if ( m_pBuilder->GetConstantEnum( objType->m_Name.AddressOf(), name, value) )
	{	
		exprValue->type			 = g_IntDataType;
		exprValue->isTemporary   = false;
		exprValue->isInitialized = true;
		exprValue->operand		 = MakeIntValueOp( value );
		return true;
	}
	return false;
}

//-------------------------------------------------------------------
// 名称: CompileOperator
// 说明: 
//-------------------------------------------------------------------
bool psCCompiler::CompileOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	// 检查变量是否已经初始化
	IsVariableInitialized(*v1, node);
	IsVariableInitialized(*v2, node);

	// 首先，看量否有对此操作的重载
	if( CompileOverloadedOperator(node,  bc, lbc, rbc, v0, v1, v2) )
		return true;

	if ( v1->type.IsPointer() || v2->type.IsPointer() )
	{
		CompilePointerOperator( node, bc, lbc, rbc, v0, v1, v2 );
		return false;
	}

	//  两个表达式中只能为基本类型
	if( !v1->type.IsPrimitive() || !v2->type.IsPrimitive() ||
	 	 v1->type.IsArrayType() ||  v2->type.IsArrayType() )
	{	
		psCString str;
		str.Format( TXT_NO_MATCHING_OP_s_FOUND_FOR_TYPE_s, psGetTokenDefinition( node->GetTokenType() ), v1->type.GetFormat().AddressOf() );
		Error(str.AddressOf(), node);

		v0->SetDefaultValue( v1->type );
	}else
	{
		psSValue tmp1 = *v1;
		psSValue tmp2 = *v2;
		if ( tmp1.type.IsReference() )
		{	
			tmp1.type.IsReference() = false;
			AllocateTemporaryVariable( tmp1 );
		}
		if ( tmp2.type.IsReference() )
		{	
			tmp2.type.IsReference() = false;
			AllocateTemporaryVariable( tmp2 );
		}

		int op = node->GetTokenType();
		if( op == ttAnd || op == ttOr )
		{
			// NOTE: 注意解引用时代码位置, 左表达式的应放到左表达式的代码中
			//		 右表达式应放到右表达式的代码中
			//		 否则解引用的代码次序不对.
			if ( v1->type.IsReference() )
			{
				InstrRDREF( lbc, tmp1.operand, v1->operand, tmp1.type );
			}
			if ( v2->type.IsReference() )
			{
				InstrRDREF( rbc, tmp2.operand, v2->operand, tmp2.type );
			}
			// Boolean operators
			// && || ^^
			CompileBooleanOperator(node, bc, lbc, rbc, v0, &tmp1, &tmp2);
		}else
		{		
			psCByteCode opBC;
			bool bAssign = false;

			if( op == ttPlus    || op == ttAddAssign ||
				op == ttMinus   || op == ttSubAssign ||
				op == ttStar    || op == ttMulAssign ||
				op == ttSlash   || op == ttDivAssign ||
				op == ttPercent || op == ttModAssign )
			{
				// Math operators
				// + - * / % += -= *= /= %=
				CompileMathOperator(node, &opBC, bAssign, v0, &tmp1, &tmp2);

			}else if( op == ttAmp          || op == ttAndAssign         ||
				op == ttBitOr              || op == ttOrAssign          ||
				op == ttBitXor             || op == ttXorAssign         ||
				op == ttBitShiftLeft       || op == ttShiftLeftAssign   ||
				op == ttBitShiftRight      || op == ttShiftRightAssign )
			{	
				// Bitwise operators
				// << >> >>> & | ^ <<= >>= >>>= &= |= ^=
				CompileBitwiseOperator(node, &opBC, bAssign, v0, &tmp1, &tmp2);

			}else if (op == ttEqual	  || op == ttNotEqual		 ||
				op == ttLessThan	  || op == ttLessThanOrEqual ||
				op == ttGreaterThan   || op == ttGreaterThanOrEqual )
			{
				// Comparison operators
				// == != < > <= >=
				CompileComparisionOperator(node, &opBC, v0, &tmp1, &tmp2);
			}

			// 添加左右表达式的代码
			// 如果是赋值表达式，则结合性是从右到左。
			if ( bAssign )
			{
				bc->AddCode( rbc );
				bc->AddCode( lbc );
			}else
			{
				bc->AddCode( lbc );
				bc->AddCode( rbc );
			}

			// 添加解引用的表达式代码
			if ( v1->type.IsReference() )
			{
				InstrRDREF( bc, tmp1.operand, v1->operand, tmp1.type );
			}
			if ( v2->type.IsReference() )
			{	
				InstrRDREF( bc, tmp2.operand, v2->operand, tmp2.type );
			}

			// 添加操作符运算的代码
			bc->AddCode( &opBC );

			// 对于非布尔运算，要注意代码生成的顺序，要和临时变量生成的顺序一至
			// 否则可能为出错。
		}

        if ( tmp1.operand != v1->operand )
			ReleaseTemporaryVariable( tmp1 );
		if ( tmp2.operand != v2->operand )
			ReleaseTemporaryVariable( tmp2 );
	}

	return false;
}

void psCCompiler::CompilePointerOperator( psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	bool hasError = false;
	int op = node->GetTokenType();
	if (  op == ttAddAssign || op == ttPlus || 
		  op == ttSubAssign || op == ttMinus  )
	{	
		// 指针加减(+=, +, -, -=)
		if ( v1->type.IsPointer() && v2->type.IsIntegerType() && !v2->type.IsPointer() )
		{
			v0->type = v1->type;	

			// 分配一个临时变量
			AllocateTemporaryVariable(*v0 );

			if ( op == ttAddAssign || op == ttSubAssign )
			{
				// 结合性从右到左
				bc->AddCode( rbc );
				bc->AddCode( lbc );
			}else
			{
				// 结合性从左到右
				bc->AddCode( lbc ); 
				bc->AddCode( rbc );
			}
			if ( v2->type.IsReference() )
			{
				bc->Instr2( BC_RDREF4, v0->operand, v2->operand );	
				bc->Instr3( (op == ttAddAssign || op == ttPlus) ? BC_ADDi : BC_SUBi, v0->operand, v1->operand, v0->operand );
			}else
			{		
				bc->Instr3( (op == ttAddAssign || op == ttPlus) ? BC_ADDi : BC_SUBi, v0->operand, v1->operand, v2->operand );
			}
		}else
		{
			hasError = true;
		}
	}else if ( op == ttEqual    || 
			   op == ttNotEqual ||
			   op == ttLessThan ||
			   op == ttGreaterThan ||
			   op == ttLessThanOrEqual ||
			   op == ttGreaterThanOrEqual )
	{
		if (  v2->isNullPointer || 
			 (v2->type.IsEqualExceptConst(v1->type)) )
		{
			//  分配一个临时变量
			v0->type = g_BoolDataType;
			AllocateTemporaryVariable(*v0);

			// 结合性从左到右
			bc->AddCode( lbc ); 
			bc->AddCode( rbc );

			if ( op == ttEqual )
				bc->Instr3( BC_TEi, v0->operand, v1->operand, v2->operand );
			else if ( op == ttNotEqual )
				bc->Instr3( BC_TNEi, v0->operand, v1->operand, v2->operand );
			else if ( op == ttLessThan )
				bc->Instr3( BC_TLi, v0->operand, v1->operand, v2->operand );
			else if ( op == ttGreaterThan )
				bc->Instr3( BC_TGi, v0->operand, v1->operand, v2->operand );
			else if ( op == ttLessThanOrEqual )
				bc->Instr3( BC_TLEi, v0->operand, v1->operand, v2->operand );
			else 
				bc->Instr3( BC_TGEi, v0->operand, v1->operand, v2->operand );
		}else
		{
			hasError = true;
		}
	}
	if ( hasError )
	{		
		v0->SetDefaultValue( g_BoolDataType );

		psCString str;
		str.Format( TXT_CANT_CONVERT_s_TO_s, v2->type.GetFormat().AddressOf(), v1->type.GetFormat().AddressOf() );
		Error( str.AddressOf(), node );
	}
}

//-------------------------------------------------------------------
// 名称: CompileMathOperator
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileMathOperator(psCScriptNode* node, psCByteCode* bc, bool& bAssign, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	psSValue tmp1;	
	psSValue tmp2;
	
	bAssign = false;

	// 把表达式的值都转换到统一的数据类型　
	psCDataType to;
	if( v1->type.IsDoubleType() || v2->type.IsDoubleType() )
	{
		to.TokenType() = ttDouble;
	}
	else if( v1->type.IsFloatType() || v2->type.IsFloatType() )
	{
		to.TokenType()= ttFloat;
	}
	else 
	{
		to.TokenType() = ttInt;
	}

	// 做类型转换
	if (!ImplicitConversion(*v1, to, tmp1, bc, node, false))
	{
		psCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_MATH_TYPE, tmp1.type.GetFormat().AddressOf());
		Error(str.AddressOf(), node);
		return;
	}

	if (!ImplicitConversion(*v2, to, tmp2, bc, node, false))
	{
		psCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_MATH_TYPE, tmp2.type.GetFormat().AddressOf());
		Error(str.AddressOf(), node);
		return;
	}

	// 设置值的类型
	v0->type = psCDataType(to.TokenType(), true, false);

	// 得到操作符
	int op = node->GetTokenType();

	bool isConstant = tmp1.IsConstant() && tmp2.IsConstant();
	if (!isConstant)
	{
		// 分配一个临时变量	
		AllocateTemporaryVariable( *v0 );

		if( op == ttAddAssign || op == ttSubAssign ||
			op == ttMulAssign || op == ttDivAssign ||
			op == ttModAssign )
		{
			// 如果是赋值运算，结合性应该是从右到左
			bAssign = true;
		}
	}

	if (to.IsIntegerType())
	{		
		#define INT1  (m_pBuilder->GetConstantInt(tmp1.operand.offset))
		#define INT2  (m_pBuilder->GetConstantInt(tmp2.operand.offset))
		int offset;

		if ( op == ttPlus || op == ttAddAssign )
		{
			// 'int' + 'int'
			if (isConstant)
			{
				if (to.IsIntegerType())
					offset = m_pBuilder->RegisterConstantInt(INT1 + INT2);
				else 
					offset = m_pBuilder->RegisterConstantInt((psDWORD)INT1 + (psDWORD)INT2);

				v0->operand.BaseOffset(PSBASE_CONST, offset);

			}else
			{
				bc->Instr3(BC_ADDi, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if( op == ttSubAssign || op == ttMinus )
		{
			// 'int' - 'int'
			if (isConstant)
			{
				if (to.IsIntegerType())
					offset = m_pBuilder->RegisterConstantInt(INT1 - INT2);
				else 
					offset = m_pBuilder->RegisterConstantInt((psDWORD)INT1 - (psDWORD)INT2);

				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_SUBi, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op == ttStar || op == ttMulAssign)
		{
			// 'int' * 'int'
			if (isConstant)
			{
				if (to.IsIntegerType())
					offset = m_pBuilder->RegisterConstantInt(INT1 * INT2);
				else 
					offset = m_pBuilder->RegisterConstantInt((psDWORD)INT1 * (psDWORD)INT2);

				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_MULi, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op == ttSlash|| op == ttDivAssign)
		{
			// 'int' / 'int'
			if (isConstant)
			{	
				// 常数除'0'异常
				if (INT2 == 0)
				{
					Error(TXT_DIVIDE_BY_ZERO, node);
					return;
				}

				if (to.IsIntegerType())
				{
					offset = m_pBuilder->RegisterConstantInt(INT1 / INT2);
				}
				else 
				{
					offset = m_pBuilder->RegisterConstantInt((psDWORD)INT1 / (psDWORD)INT2);
				}

				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_DIVi, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op  == ttModAssign|| op == ttPercent)
		{
			// 'int' % 'int' 
			if (isConstant)
			{
				// 常数模'0'异常
				if (INT2 == 0)
				{
					Error(TXT_DIVIDE_BY_ZERO, node);
					return;
				}

				if (to.IsIntegerType())
					offset = m_pBuilder->RegisterConstantInt(INT1 % INT2);
				else 
					offset = m_pBuilder->RegisterConstantInt((psDWORD)INT1 % (psDWORD)INT2);

				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_MODi, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
	}else if( to.IsFloatType() )
	{
		// 浮点运算
		#define FLOAT1  (m_pBuilder->GetConstantFloat(tmp1.operand.offset))
		#define FLOAT2  (m_pBuilder->GetConstantFloat(tmp2.operand.offset))
		int offset;

		if ( op == ttPlus || op == ttAddAssign)
		{
			// 'float' + 'float'
			if (isConstant)
			{
				offset = m_pBuilder->RegisterConstantFloat(FLOAT1 + FLOAT2);
				v0->operand.BaseOffset(PSBASE_CONST, offset);

			}else
			{
				bc->Instr3(BC_ADDf, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if( op == ttSubAssign || op == ttMinus)
		{
			// 'float' - 'float'
			if (isConstant)
			{
				offset = m_pBuilder->RegisterConstantFloat(FLOAT1 - FLOAT2);
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_SUBf, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op == ttStar || op == ttMulAssign)
		{
			// 'float' * 'float'
			if (isConstant)
			{
				offset = m_pBuilder->RegisterConstantFloat(FLOAT1 * FLOAT2);
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_MULf, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op == ttSlash|| op == ttDivAssign)
		{
			// 'float' / 'float'
			if (isConstant)
			{
				offset = m_pBuilder->RegisterConstantFloat(FLOAT1 / FLOAT2);
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_DIVf, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op == ttModAssign|| op  == ttPercent)
		{
			// 'float' % 'float'
			if (isConstant)
			{
				offset = m_pBuilder->RegisterConstantFloat((float)fmod(FLOAT1, FLOAT2));
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_MODf, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
	}else if (to.IsDoubleType())
	{		
		#define DOUBLE1  (m_pBuilder->GetConstantDouble(tmp1.operand.offset))
		#define DOUBLE2  (m_pBuilder->GetConstantDouble(tmp2.operand.offset))
		if ( op == ttPlus || op == ttAddAssign)
		{
			// 'dobule' + 'double'
			if (isConstant)
			{
				int offset = m_pBuilder->RegisterConstantDouble(DOUBLE1 + DOUBLE2);
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_ADDd, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if( op == ttSubAssign || op == ttMinus)
		{		
			// 'double' - 'double'
			if (isConstant)
			{
				int offset = m_pBuilder->RegisterConstantDouble(DOUBLE1 - DOUBLE2);
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_SUBd, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op == ttStar || op == ttMulAssign)
		{
			// 'double' * 'double'
			if (isConstant)
			{
				int offset = m_pBuilder->RegisterConstantDouble(DOUBLE1 * DOUBLE2);
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_MULd, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op == ttSlash || op == ttDivAssign)
		{
			// 'dobule' / 'double'
			if (isConstant)
			{
				int offset = m_pBuilder->RegisterConstantDouble(DOUBLE1 / DOUBLE2);
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_DIVd, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
		else if ( op == ttModAssign || op == ttPercent)
		{
			// 'double' % 'double'
			if (isConstant)
			{
				int offset = m_pBuilder->RegisterConstantDouble(fmod(DOUBLE1,DOUBLE2));
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				bc->Instr3(BC_MODd, v0->operand, tmp1.operand, tmp2.operand);
			}
		}
	}

	//  释放临时变量(如果有的话)
	if (tmp1.operand != v1->operand)
		ReleaseTemporaryVariable(tmp1);

	if (tmp2.operand != v2->operand)
		ReleaseTemporaryVariable(tmp2);
}

//-------------------------------------------------------------------
// 名称: CompileBitwiseOperator
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileBitwiseOperator(psCScriptNode* node, psCByteCode* bc,  bool& bAssign, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	// 得到操作符
	int op = node->GetTokenType();

	// 表达式的类型都必需为'int'类型
	psSValue tmp1, tmp2;

	// 做类型转换	
	if (!ImplicitConversion(*v1, g_IntDataType, tmp1, bc, node, false))
	{
		psCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, v1->type.GetFormat().AddressOf(), PS_T("int"));
		Error(str.AddressOf(), node);
		return;
	}

	if (!ImplicitConversion(*v2, g_IntDataType, tmp2, bc, node, false))
	{
		psCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, v2->type.GetFormat().AddressOf(), PS_T("int"));
		Error(str.AddressOf(), node);
		return;
	}

	bAssign = false;

	bool isConstant = (tmp1.IsConstant() && tmp2.IsConstant());

	// 设置表达式的类型
	v0->type = tmp1.type;

	if (!isConstant)
	{
		// 分配一个临时变量	
		AllocateTemporaryVariable( *v0 );

		if (op == ttAndAssign ||
			op == ttOrAssign  ||
			op == ttXorAssign ||
			op == ttShiftLeftAssign ||
			op == ttShiftRightAssign )
		{	
			// 如果是赋值运算，结合性应该是从右到左
			bAssign = true;
		}
	}

	#define V_UINT1  (psUINT)(m_pBuilder->GetConstantInt(tmp1.operand.offset))
	#define V_UINT2  (psUINT)(m_pBuilder->GetConstantInt(tmp2.operand.offset))

	int offset;
	if ( op == ttAmp || op == ttAndAssign)
	{
		// v1 & v2
		if (isConstant)
		{
			offset = m_pBuilder->RegisterConstantInt(V_UINT1 & V_UINT2);
			v0->operand.BaseOffset(PSBASE_CONST, offset);
		}
		else
		{
			bc->Instr3(BC_AND, v0->operand, tmp1.operand, tmp2.operand);
		}
	}else if (op == ttBitOr || op == ttOrAssign)
	{
		// v1 | v2
		if (isConstant)
		{
			offset = m_pBuilder->RegisterConstantInt(V_UINT1 | V_UINT2);
			v0->operand.BaseOffset(PSBASE_CONST, offset);
		}
		else
		{
			bc->Instr3(BC_OR, v0->operand, tmp1.operand, tmp2.operand);
		}
	}else if (op == ttBitXor || op == ttXorAssign)
	{
		// v1 ^ v2
		if (isConstant)
		{
			offset = m_pBuilder->RegisterConstantInt(V_UINT1 ^ V_UINT2);
			v0->operand.BaseOffset(PSBASE_CONST, offset);
		}else
		{
			bc->Instr3(BC_XOR, v0->operand, tmp1.operand, tmp2.operand);
		}
	}else if (op == ttShiftLeftAssign  || op == ttBitShiftLeft || 
			  op == ttShiftRightAssign || op == ttBitShiftRight)
	{
		if (isConstant)
		{
			if  (op == ttShiftLeftAssign || op == ttBitShiftLeft)
			{
				offset = m_pBuilder->RegisterConstantInt( V_UINT1 << V_UINT2 );
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}else
			{
				offset = m_pBuilder->RegisterConstantInt( V_UINT1 >> V_UINT2 );
				v0->operand.BaseOffset(PSBASE_CONST, offset);
			}
		}else
		{
			if  (op == ttShiftLeftAssign || op == ttBitShiftLeft)
				bc->Instr3(BC_SLA, v0->operand, tmp1.operand, tmp2.operand );
			else
				bc->Instr3(BC_SRA, v0->operand, tmp1.operand, tmp2.operand );
		}
	}else
	{
		// 无效的操作符
		assert(false);
	}

	// 释放临时变量(如果有的话)
	if (tmp1.operand != v1->operand)
		ReleaseTemporaryVariable(tmp1);
	if (tmp2.operand != v2->operand)
		ReleaseTemporaryVariable(tmp2);
}

//-------------------------------------------------------------------
// 名称: CompileComparisionOperator
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileComparisionOperator(psCScriptNode* node, psCByteCode* bc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	// 把运算符两边的操作数转换倒同一个类型
	psCDataType type;
	if (v1->type.IsDoubleType() || v2->type.IsDoubleType())
	{
		type.TokenType() = ttDouble;
	}
	else if (v1->type.IsFloatType() || v2->type.IsFloatType())
	{
		type.TokenType() = ttFloat;
	}
	else if (v1->type.IsIntegerType() || v2->type.IsIntegerType())
	{
		type.TokenType() = ttInt;
	}
	else if (v1->type.IsBooleanType() || v2->type.IsBooleanType())
	{
		type.TokenType() = ttBool;
	}else
	{
		Error(TXT_ILLEGAL_OPERATION, node);
		return;
	}

	psSValue tmp1, tmp2;
	// 做类型转换
	if (!ImplicitConversion(*v1, type, tmp1, bc, node, false))
	{
		psCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, v1->type.GetFormat().AddressOf(), type.GetFormat().AddressOf());
		Error(str.AddressOf(), node);
		return;
	}
	if (!ImplicitConversion(*v2, type, tmp2, bc, node, false))
	{
		psCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, v2->type.GetFormat().AddressOf(), type.GetFormat().AddressOf());
		Error(str.AddressOf(), node);
		return;
	}
	
	bool isConstant = (tmp1.IsConstant() && tmp2.IsConstant());

	//　设置表达式的类型
	v0->type = g_BoolDataType;

	if (!isConstant)
	{
		// 分配一个临时变量	
		AllocateTemporaryVariable( *v0 );
	}
	
	// 得到操作符
	int op = node->GetTokenType();

	if (type.IsBooleanType())
	{
		// 'bool'运算
		if (op == ttEqual || op == ttNotEqual)
		{
			if (isConstant)
			{
				int i1 = m_pBuilder->GetConstantInt(tmp1.operand.offset);
				int i2 = m_pBuilder->GetConstantInt(tmp2.operand.offset);

				if (op == ttEqual)
				{
					v0->operand.BaseOffset( PSBASE_CONST, m_pBuilder->RegisterConstantInt( i1 == i2 ) );
				}
				else
				{
					v0->operand.BaseOffset( PSBASE_CONST, m_pBuilder->RegisterConstantInt( i1 != i2 ) );
				}
			}else
			{
				if (op == ttEqual)
					bc->Instr3(BC_TEi, v0->operand, tmp1.operand, tmp2.operand);
				else
					bc->Instr3(BC_TNEi, v0->operand, tmp1.operand, tmp2.operand);
			}
		}else
		{
			// TODO: Use TXT_ILLEGAL_OPERATION_ON
			Error(TXT_ILLEGAL_OPERATION, node);
		}
	}else if (type.IsIntegerType())
	{
		if (isConstant)
		{
			int value = 0;
			int i1 = m_pBuilder->GetConstantInt(tmp1.operand.offset);
			int i2 = m_pBuilder->GetConstantInt(tmp2.operand.offset);

			if (op == ttEqual)
				value = (i1 == i2) ? 1 : 0;
			else if (op == ttNotEqual)
				value = (i1 != i2) ? 1 : 0;
			else if (op == ttLessThan)
				value = (i1 < i2) ? 1 : 0;
			else if (op == ttLessThanOrEqual)
				value = (i1 <= i2) ? 1 : 0;
			else if (op == ttGreaterThan)
				value = (i1 > i2) ? 1 : 0;
			else if (op == ttGreaterThanOrEqual)
				value = (i1 >= i2) ? 1 : 0;
			else
			{
				assert(false);
				Error(TXT_ILLEGAL_OPERATION, node);
			}

			v0->operand.BaseOffset(PSBASE_CONST, m_pBuilder->RegisterConstantInt(value) );
		}else
		{
			if (op == ttEqual)
				bc->Instr3(BC_TEi,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttNotEqual)
				bc->Instr3(BC_TNEi,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttLessThan)
				bc->Instr3(BC_TLi, v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttLessThanOrEqual)
				bc->Instr3(BC_TLEi, v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttGreaterThan)
				bc->Instr3(BC_TGi,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttGreaterThanOrEqual)
				bc->Instr3(BC_TGEi,  v0->operand, tmp1.operand, tmp2.operand);
			else
			{
				assert(false);
				Error(TXT_ILLEGAL_OPERATION, node);
			}
		}
	}else if (type.IsFloatType())
	{
		if (isConstant)
		{
			int value = 0;
			float f1 = GetConstantFloat(tmp1.operand);
			float f2 = GetConstantFloat(tmp2.operand);

			if (op == ttEqual)
				value = (f1 == f2) ? 1 : 0;
			else if (op == ttNotEqual)
				value = (f1 != f2) ? 1 : 0;
			else if (op == ttLessThan)
				value = (f1 < f2) ? 1 : 0;
			else if (op == ttLessThanOrEqual)
				value = (f1 <= f2) ? 1 : 0;
			else if (op == ttGreaterThan)
				value = (f1 > f2) ? 1 : 0;
			else if (op == ttGreaterThanOrEqual)
				value = (f1 >= f2) ? 1 : 0;
			else
			{
				assert(false);
				Error(TXT_ILLEGAL_OPERATION, node);
			}
			v0->operand.BaseOffset(PSBASE_CONST, m_pBuilder->RegisterConstantInt(value) );
		}else
		{
			if (op == ttEqual)
				bc->Instr3(BC_TEf,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttNotEqual)
				bc->Instr3(BC_TNEf, v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttLessThan)
				bc->Instr3(BC_TLf,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttLessThanOrEqual)
				bc->Instr3(BC_TLEf,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttGreaterThan)
				bc->Instr3(BC_TGf, v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttGreaterThanOrEqual)
				bc->Instr3(BC_TGEf,  v0->operand, tmp1.operand, tmp2.operand);
			else
			{
				assert(false);
				Error(TXT_ILLEGAL_OPERATION, node);
			}
		}
	}else if (type.IsDoubleType())
	{
		if (isConstant)
		{
			int value = 0;
			double d1 = m_pBuilder->GetConstantDouble(tmp1.operand.offset);
			double d2 = m_pBuilder->GetConstantDouble(tmp2.operand.offset);

			if (op == ttEqual)
				value = (d1 == d2) ? 1 : 0;
			else if (op == ttNotEqual)
				value = (d1 != d2) ? 1 : 0;
			else if (op == ttLessThan)
				value = (d1 < d2) ? 1 : 0;
			else if (op == ttLessThanOrEqual)
				value = (d1 <= d2) ? 1 : 0;
			else if (op == ttGreaterThan)
				value = (d1 > d2) ? 1 : 0;
			else if (op == ttGreaterThanOrEqual)
				value = (d1 >= d2) ? 1 : 0;
			else
			{
				assert(false);
				Error(TXT_ILLEGAL_OPERATION, node);
			}
			v0->operand.BaseOffset(PSBASE_CONST, m_pBuilder->RegisterConstantInt(value) );
		}else
		{
			if (op == ttEqual)
				bc->Instr3(BC_TEd,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttNotEqual)
				bc->Instr3(BC_TNEf,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttLessThan)
				bc->Instr3(BC_TLd,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttLessThanOrEqual)
				bc->Instr3(BC_TLEd,  v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttGreaterThan)
				bc->Instr3(BC_TGd, v0->operand, tmp1.operand, tmp2.operand);
			else if (op == ttGreaterThanOrEqual)
				bc->Instr3(BC_TGEd,  v0->operand, tmp1.operand, tmp2.operand);
			else
			{
				assert(false);
				Error(TXT_ILLEGAL_OPERATION, node);
			}
		}
	}

	// 释放临时变量(如果有的话)
	if (tmp1.operand != v1->operand)
		ReleaseTemporaryVariable(tmp1);
	if (tmp2.operand != v2->operand)
		ReleaseTemporaryVariable(tmp2);
}

//-------------------------------------------------------------------
// 名称: CompileBooleanOperator
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileBooleanOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	// 表达式的类型都必需为'boolean'类型
	psSValue tmp1, tmp2;

	// 做类型转换	
	if (!ImplicitConversion(*v1, g_BoolDataType, tmp1, lbc, node, false))
	{
		psCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, v1->type.GetFormat().AddressOf(), PS_T("bool"));
		Error(str.AddressOf(), node);
		return;
	}

	if (!ImplicitConversion(*v2, g_BoolDataType, tmp2, rbc, node, false))
	{
		psCString str;
		str.Format(TXT_NO_CONVERSION_s_TO_s, v2->type.GetFormat().AddressOf(), PS_T("bool"));
		Error(str.AddressOf(), node);
		return;
	}

	bool isConstant = tmp1.IsConstant() && tmp2.IsConstant();

	// 设置值的类型
	v0->type = g_BoolDataType;

	if (!isConstant)
	{
		// 分配一个临时变量	
		AllocateTemporaryVariable( *v0 );
	}

	int offset1 = m_pBuilder->RegisterConstantInt(1);
	int offset0 = m_pBuilder->RegisterConstantInt(0);

	int op = node->GetTokenType();
	if (op == ttAnd || op == ttOr)
	{
		if (isConstant)
		{
			if (op == ttAnd)
			{
				if (GetConstantInt(tmp1.operand) && GetConstantInt(tmp2.operand))
					v0->operand.BaseOffset(PSBASE_CONST, offset1);
				else
					v0->operand.BaseOffset(PSBASE_CONST, offset0);
			}
			else
			{
				if (GetConstantInt(tmp1.operand) || GetConstantInt(tmp2.operand))
					v0->operand.BaseOffset(PSBASE_CONST, offset1);
				else
					v0->operand.BaseOffset(PSBASE_CONST, offset0);
			}
		}else
		{
			// 添加左表达式的代码
			bc->AddCode(lbc);

			int label1 = m_NextLabel++;
			int label2 = m_NextLabel++;
			
			if (op == ttAnd)
			{
				// 实现短路操作
				bc->Instr2(BC_JNZ, tmp1.operand, label1);
					
				// 设置表达式的值
				bc->Instr2(BC_MOV4, v0->operand, psSOperand(PSBASE_CONST, offset0));

				// 跳到结尾
				bc->Instr1(BC_JMP, label2);
			}else 
			{
				// 实现断路操作
				bc->Instr2(BC_JZ, tmp1.operand, label1);

				// 设置表达式的值
				bc->Instr2(BC_MOV4, v0->operand, psSOperand(PSBASE_CONST, offset1));

				// 跳到结尾
				bc->Instr1(BC_JMP, label2);
			}

			bc->Label(label1);

			// 添加右表达式的代码
			bc->AddCode(rbc);

			// 设置表达式的值
			bc->Instr3( BC_TNEi, v0->operand, tmp2.operand, psSOperand(PSBASE_CONST, offset0) );

			bc->Label(label2);
		}

		// 释放临时变量(如果有的话)
		if (tmp1.operand != v1->operand)
			ReleaseTemporaryVariable(tmp1);
		if (tmp2.operand != v2->operand)
			ReleaseTemporaryVariable(tmp2);
	}
}

//-------------------------------------------------------------------
// 名称: CompileStatement
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::CompileStatement(psCScriptNode* node, bool* hasReturn, psCByteCode* bc)
{
	// 此时应该没有任何临时变量
	assert( m_TempValues.size() == 0 );

	if (hasReturn)
	{
		*hasReturn = false;
	}
	int type = node->GetNodeType();

	if (type == snStatementBlock)
		CompileStatementBlock(node, true, hasReturn, bc);
	else if( type == snExpressionStatement )
		CompileExpressionStatement(node, bc);
	else if (type == snIf)
		CompileIFStatement(node, hasReturn, bc);
	else if (type == snSwitch)
		CompileSwitchStamement(node, hasReturn, bc);
	else if (type == snCase)
		CompileCase(node, bc);
	else if (type == snBreak)
		CompileBreakStatement(node, bc);
	else if (type == snContinue)
		CompileContinueStatement(node, bc);
	else if (type == snFor)
		CompileForStatement(node, bc);
	else if (type == snWhile)
		CompileWhileStatement(node, bc);
	else if (type == snDoWhile)
		CompileDoWhileStatement(node, bc);
	else if( type == snEnum )
		CompileEnum( node, NULL, NULL );
	else if (type == snReturn)
	{
		CompileReturnStatement(node, bc);
		if (hasReturn)
			*hasReturn = true;
	} 

	// 清空并析构所有临时变量
	ClearTemporaryVariables(bc);
}


void psCCompiler::CompileExpressionStatement(psCScriptNode *enode, psCByteCode *bc)
{
	// 添加一条行指今
	InstrLINE(bc, enode->GetTokenPos());

	if( enode->GetFirstChild() )
	{
		// 编译表达式	
		psSValue exprValue;
		CompileAssignment(enode->GetFirstChild(), bc, &exprValue);

		// 释放临时变量
		ReleaseTemporaryVariable( exprValue );
	}
}

