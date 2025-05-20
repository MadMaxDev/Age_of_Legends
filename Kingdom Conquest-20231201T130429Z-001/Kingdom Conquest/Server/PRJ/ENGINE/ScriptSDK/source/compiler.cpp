#include "compiler.hpp"

//-------------------------------------------------------------------
// ����: psCCompiler
// ˵��: 
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
// ����: Reset
// ˵��: ���ñ�����
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
// ����: CompileGlobalVariable
// ˵��: ����ȫ�ֱ���
//-------------------------------------------------------------------
int psCCompiler::CompileGlobalVariable(psCBuilder* builder, psCScriptCode* script, psCScriptNode* node, psSGlobalVariableDesc* gvar)
{
	// ���ñ�����
	Reset( builder, script, NULL );

	// ���һ��������
	AddVariableScope();

	// �����������
	CompileDeclaration(node, &m_ByteCode, gvar);

	// �Ƴ�������
	// NOTE: ��ʼ�к�Ϊ-1��ʾΪȫ����Ч(���ڵ�����)
	RemoveVariableScope();

	return  (m_bHasCompileError) ? -1 : 0;
}

//-------------------------------------------------------------------
// ����: CompileFunction
// ˵��: ���뺯��
//-------------------------------------------------------------------
int psCCompiler::CompileFunction(psCBuilder* builder, psCScriptCode* script, psCScriptNode* func, psCObjectType* objType)
{
	// ���ñ�����
	Reset(builder, script, objType);

	// Ϊ��������Ԥ��һ����ǩ,
	m_NextLabel++;

	// ���һ��������,�����Ĳ���,����������������ĵı�������������
	AddVariableScope();

	int funcType = func->GetNodeType();
	// �õ�����ֵ����
	if ( funcType == snFunction || funcType == snOpOverload )
	{
		m_ReturnType = m_pBuilder->CreateDataTypeFromNode(func->GetFirstChild(), m_pScript);
		m_pBuilder->ModifyDataTypeFromNode(m_ReturnType, func->GetFirstChild()->GetNextNode(), m_pScript );
	}else
	{	
		m_ReturnType.TokenType() = ttVoid;
		m_ReturnType.SetObjectType( NULL );

		// �������п���Ϊ����/��������
		assert( funcType == snConstructor || funcType == snDestructor );
	}

	// ���Ҳ����б���, ��TokenType�����˴˺����Ƿ�Ϊ��κ���������
	psCScriptNode* node = func->GetFirstChild();
	while (node && node->GetNodeType() != snParameterList)
	{
		node = node->GetNextNode();
	}

	// �ű�����֧�ֱ��
	if (node->GetTokenType() == ttEllipsis)
	{
		Error(TXT_SCRIPT_SEFL_DONT_SUPPORT_ARGCHANGED, func);
		return -1;
	}

	// ������������,�����Դ����ҵ�˳������
	psCByteCode paramBC;

	int funcArgSize = 0;
	if ( m_ObjectType )
	{
		// ��������Ա����,���һ������Ϊ�����ָ��
		funcArgSize++;
	}

	if ( m_ReturnType.IsObject() && !m_ReturnType.IsReference() )
	{
		// �������ֵΪ�����,���Ҳ���������ô
		// ���е�һ������Ϊ����������ָ��
		funcArgSize++;
	}

	if (node)
	{
		// �����б����п���Ϊ��
		node = node->GetFirstChild();
	}

	while (node)
	{
		// �õ���������
		psCDataType type = m_pBuilder->CreateDataTypeFromNode(node, m_pScript);
		node = node->GetNextNode();

		// �õ������������η�
		m_pBuilder->ModifyDataTypeFromNode(type, node, m_pScript);
		node = node->GetNextNode();

		// �����������
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
			// ���������������һ�����������ʹ���
			type.IsPointer() = true;
			type.IsReference() = true;
		}

		// ����˲���������, �����˱���
		if ( node->GetNodeType() == snIdentifier )
		{
			psCString name;
			name.Copy(&m_pScript->m_Code[node->GetTokenPos()], node->GetTokenLength());

			psSValue value;	
			value.StackValue(type, funcArgSize * sizeof(psDWORD) );

			// ��ջ��ƫ�������ֽ�Ϊ��λ��
			funcArgSize += type.GetSizeOnStackDWords();

			if (m_Variables->DeclareVariable(name.AddressOf(), value) == 0)
				Error(TXT_PARAMETER_ALREADY_DECLARED, func);
		}else
		{
			// �˲���Ϊ��������,
			// �������������������ƽ��, �̲�Ҫ����: node = node->GetNextNode()
			psSValue value;
			value.StackValue(type, funcArgSize * sizeof(psDWORD) );

			m_Variables->DeclareVariable(PS_T(""), value);

			// ��ջ��ƫ�������ֽ�Ϊ��λ��
			funcArgSize += type.GetSizeOnStackDWords();
		}

		// �Ƶ���һ�����
		node = arrayDimNode->GetNextNode();
	}

	psCByteCode constructBC;
	// �������Ĺ��캯��, ������Ա�Ĺ��캯������
	// Ҫ�ڲ���������,������������ǰ����
	if (funcType == snConstructor && m_ObjectType)
	{
		DefaultObjectConstructor(m_ObjectType, &constructBC);
	}

	// ���뺯����, ��һ�������屾��û�б�����
	// ������ı���,ʹ�õ��������������ͬ�ı�����
	bool hasReturn;
	psCByteCode blockbc;
	CompileStatementBlock(func->GetLastChild(), false, &hasReturn, &blockbc);

	// ���Ƿ���ֵ���Ͳ��ǿ�,ȷ��ÿһ��·�����з���ֵ
	if ( m_ReturnType.TokenType() != ttVoid )
	{
		if( hasReturn == false )
			Error(TXT_NOT_ALL_PATHS_RETURN, func->GetLastChild());
	}

	// �������еľֲ�����
	psCVariableScope::VariableMap_t::const_reverse_iterator it = m_Variables->GetVariableMap().rbegin();
	for (; it != m_Variables->GetVariableMap().rend(); ++it)
	{
		const psSVariable* var = it->second;

		if ( var->value.operand.base == PSBASE_STACK &&
			 var->value.operand.offset < 0 )
		{
			CompileDestructor(var->value, &blockbc);	

			// �ͷ���ʱ�����ռ�
			DeallocateVariable(var->value.operand.offset);
		}
	}	

	psCByteCode destructBC;
	psCByteCode exitBC;
	// ����������������, ���������Ա����������
	if (funcType == snDestructor && m_ObjectType)
	{
		DefaultObjectDestructor(m_ObjectType, &destructBC);
	}

	// �������еĺ�����������
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
		// NOTE: ֮����Ҫ��һ����ʱ����,���淵��ֵ������
		// �������Ա�������������ܻ���д����ֵ
		psCDataType rtype;
		if ( m_ReturnType.IsObject() || m_ReturnType.IsReference() ||
			 m_ReturnType.GetSizeInMemoryBytes() <= 4.)
		{
			rtype.TokenType() = ttInt;
		}else
		{
			rtype.TokenType() = ttDouble;
		}

		// ���淵��ֵ
		psSOperand retOp( PSBASE_STACK, AllocateVariable( rtype, false ) );
		InstrRRET( &exitBC, retOp, rtype );

		// ������������Ĵ���
		exitBC.AddCode( &destructBC );

		// �ָ�����ֵ
		InstrSRET( &exitBC, retOp, rtype );

		DeallocateVariable( retOp.offset );
	}

	// ����б����,�����������յĴ���
	if( m_bHasCompileError ) return -1;

	// ����Ӧ��û�з���ı���
	size_t s1 = m_VariableAllocations.size() ;
	size_t s2 = m_FreeVariables.size();
	assert(s1 == s2);

	// ���㺯����ʼ/������λ��
	int lastLinePos =  func->GetLastChild()->GetTokenPos() + func->GetLastChild()->GetTokenLength();

	// �Ƴ�������
	int r;
	m_pScript->ConvertPosToRowCol(lastLinePos, &r, 0);
	CreateDebugSymbols(m_Variables, r);
	RemoveVariableScope();

	//----------------------------------------------------------------------//
	// �������еĶ����ƴ���
	//----------------------------------------------------------------------//
	psCByteCode* bc = &m_ByteCode;

	int largestStackUsed = -m_StackPos;

	// �����ܹ���Ҫ�Ķ�ջ��С
	// NOE: ��Ϊ����ʹ�õ������������Ķ�ջ,
	// ����ջ����Ҫ�Ѷ�ջָ��С
	bc->Push(largestStackUsed);

	// ������Ա�������
	bc->AddCode(&constructBC);

	// ��Ӻ������������Ĵ���
	bc->AddCode(&paramBC);

	// �������Ĵ���
	bc->AddCode(&blockbc);

	// Label 0 ��'return'��������
	bc->Label(0);

	// �ں�����δβ��һ����ָ��
	InstrLINE(bc, lastLinePos);

	// ����������Ա�Ͳ����Ĵ���
	bc->AddCode(&exitBC);

	// ������ջ
	bc->Pop(largestStackUsed);

	// ��������
	bc->Ret(funcArgSize);

	// ������ɶ����ƴ���
	bc->Finalize(&m_pBuilder->GetOptimizeOptions());

	return 0;
}

//-------------------------------------------------------------------
// ����: CompileStatementBlock
// ˵��: ��������
//-------------------------------------------------------------------
void psCCompiler::CompileStatementBlock(psCScriptNode *block, bool ownVariableScope, bool *hasReturn, psCByteCode *bc)
{
	assert(hasReturn && bc);

	// ���÷���ֵ��־
	*hasReturn = false;

	bool isFinished = false;
	bool hasWarned = false;

	// ������ǰ,���һ����ָ��
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

	// �����������,���һ����ָ��
	int lastLinePos = block->GetTokenPos() + block->GetTokenLength();
	int r;
	m_pScript->ConvertPosToRowCol(lastLinePos, &r, 0);
	InstrLINE( bc, lastLinePos );
		
	// �Ƴ�������
	if (ownVariableScope)
	{	
		// ���෴��˳��,�ͷ����еľֲ�����
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

				// �������û�з���, ����û�в�����'break',��'continue'����������,
				// ����Ҫ�����ֲ�����
				if( !isFinished && !(*hasReturn) )
				{
					CompileDestructor(var->value, bc);	
				}

				// �ͷŶ�ջ�ռ�
				DeallocateVariable(var->value.operand.offset);
			}
		} 

		// �Ƴ�������
		int r;
		m_pScript->ConvertPosToRowCol(block->GetTokenPos() + block->GetTokenLength(), &r, 0);
		CreateDebugSymbols(m_Variables, r);
		RemoveVariableScope();
	}

}

//-------------------------------------------------------------------
// ����: CompileDeclaration
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileDeclaration(psCScriptNode* decl, psCByteCode* bc, psSGlobalVariableDesc* gvar)
{
	// ���һ����ָ��
	int r = InstrLINE(bc, decl->GetTokenPos());

	psCScriptNode* node = decl->GetFirstChild();

	// �õ�������������
	psCDataType type = m_pBuilder->CreateDataTypeFromNode(node, m_pScript);
	node = node->GetNextNode();

	// �������еı���
	while (node)
	{
		// �õ���������
		m_pBuilder->ModifyDataTypeFromNode(type, node, m_pScript);
		node = node->GetNextNode();

		// �����������
		CheckDataType( type, node );

		// �õ���������
		psCString name;
		CompileVariableDim(node, bc, name, type);
		node = node->GetNextNode();

		// ȷ��������Ʋ���һ������
		if( m_pBuilder->GetObjectType(name.AddressOf()) != NULL )
		{
			psCString str;
			str.Format(TXT_ILLEGAL_VARIABLE_NAME_s, name.AddressOf());
			Error(str.AddressOf(), decl);
		}
		psSValue variable;

		if ( type.IsReference() )
		{
			// �����������,�������Ͳ���������
			if (type.IsArrayType())
			{
				Error(TXT_REF_TYPE_CANNOT_BE_ARRAY, decl);
				return;
			}

			if ( node == NULL || node->GetNodeType() != snInitExpr)
			{
				// ��������,�����г�ʼֵ
				psCString str;
				str.Format(TXT_REF_s_MUST_BE_INITIALIZED, name.AddressOf() );
				Error(str.AddressOf() , decl);

				// ��֤��������
				variable.isInitialized = true;
				variable.type = type;
				if (NULL == gvar)
				{
					// �������ȫ�ֱ���,������
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
					// �����ʼ�����ʽ
					CompileAssignment(first, bc, &exprValue);

					bool bOk = false;
					if ( !type.IsPointer() )
					{
						// ���ò���Ϊ��ʱ��
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
						// ������õ������Ƿ���ȷ,��Ҫ����ֻ������
						psCString str;
						str.Format( TXT_CANT_CONVERT_s_TO_s, exprValue.type.GetFormat().AddressOf(), type.GetFormat().AddressOf() );
						Error(str.AddressOf(), decl);
					}
	
					// ����ֵ������
					// ����һ����ʱ����ָ��, ��Ϊ��ָ��һ���յ�ַ
					variable.type				= type;
					variable.isInitialized      = true;
					variable.startline		    = r;	
					variable.isTemporary		= true;
					// Ԥ�ȷ���һ������
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
	        // ����������͵�ֻ������, Ȼ�����ֻ��������Ϊ��,
			// ���ڱ����ʼ�����ʽʱ�ᱨ��.
			// ע: ��Ҫ���˻ָ�
			bool bReadOnly = type.IsReadOnly();
			variable.type.IsReadOnly() = false;

			bool bInitExpr = false;
			if ( node && node->GetNodeType() == snInitExpr )
				bInitExpr = true;

			// Ԥ�ȷ���һ������
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
				// ��������ʼ�����ʽ/�๹��
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
				// ֱ�ӵ��ù��캯��
				if (!CompileConstructor(variable, bc))
				{
					psCString msg;
					msg.Format(TXT_OBJECT_s_CANNOT_BE_INSTANCE, type.GetFormat().AddressOf() );
					Error(msg.AddressOf(), decl);
				}
			}

			// �ָ��������͵�ֻ������
			variable.type.IsReadOnly() = bReadOnly;
		}

		// ��������
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

	// �����ʱ�����
	ClearTemporaryVariables( bc );
}

//-------------------------------------------------------------------
// ����: CompileVariableInitExpr
// ˵��: ��������ʼ�����ʽ
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

	// �����ʼ�����ʽ
	CompileInitExpression(node, &exprBC, value, argValue);
	value.isInitialized = true;

	if (type.IsPrimitive() && type.IsReadOnly() && argValue.IsConstant())
	{
		// ����Ǿ�̬����,��Ҫ�ӱ��ʽ�Ĵ���
		// Ҳ����Ҫ�����ռ�
		// NOTE:��ת����ͬʱ���Ѿ���'value'�Ĳ��������¸�ֵ��
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
		// ��ӱ��ʽ�Ĵ���
		bc->AddCode(&exprBC);
	}
}

//-------------------------------------------------------------------
// ����: CompileVariableDim
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileVariableDim(psCScriptNode* varDim, psCByteCode* bc, psCString& name, psCDataType& dataType)
{
	assert(varDim->GetNodeType() == snVariableDim);

	psCScriptNode* node = varDim->GetFirstChild();

	// �õ�����������
	name.Copy(&m_pScript->m_Code[node->GetTokenPos()], node->GetTokenLength());
	node = node->GetNextNode();

	psCByteCode arrayBC;
	psSValue constValue;
	while (node)
	{
		// ����һ�����鶨�� 
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
// ����: CompileInitExpression
// ˵��: ���������ʼ�����ʽ
//		 �������argValue��Ϊ�˸���̬�������Ż�����,�����߲�����ȥ�ͷ���
//		 ��ֻ����ֵ�Ĳο�,�������ֻ���������ͨ��ʼ�����ʽ��
//-------------------------------------------------------------------
void psCCompiler::CompileInitExpression(psCScriptNode* initExpr, psCByteCode* bc, const psSValue& value, psSValue& argValue)
{
	// NOTE: initExpr->GetParent()�õ�����declaration���
	// ��Ϊ���������Ϣ,��Ҫtoken��λ����Ϣ, ֻ��declaration��㳤��������Щ��Ϣ
	assert(initExpr && initExpr->GetNodeType() == snInitExpr);

	psSValue resultValue = value;
	psCDataType& type = resultValue.type;

	int arraySize = 0;

	// ����һ������, ����һ��һ���Ĳ�һ��,
	// �����ճ�Ϊ������ֵ����
	int typeArraySize = resultValue.type.PopArraySize();

	// �õ���һ����ֵ���
	psCScriptNode* node = initExpr->GetFirstChild();
	while (node)
	{	
		if (node->GetNodeType() == snAssignment)
		{
			// ����һ��Ҷ���,ִ���������ֵ����
			CompileAssignment(node, bc, &argValue);

			if ( type.IsObject() && !type.IsPointer() )
			{
				// �����ֵ
				ObjectAssignment( value, argValue, bc, initExpr );
			}else
			{
				// ��ͨ���͸�ֵ
				PerformAssignment(value, argValue, bc, initExpr->GetParent() );
			}

			// �ͷ���ʱ����
			ReleaseTemporaryVariable( argValue );
		}else
		{
			// ����һ����ʼ������Ľ��,

			// ���еݹ�����,�����Ƕ�ά����
			CompileInitExpression(node, bc, resultValue, argValue);

			if (node->GetNextNode() != NULL)
			{
				// ����Ԫ�ش�С��ֹһ��
				resultValue.operand.offset += type.GetSizeInMemoryBytes() * type.GetNumArrayElements();
			}	
		}	

		// ��������Ĵ�С
		++arraySize;

		// �Ƶ�һ�����
		node = node->GetNextNode();
	}

	// ��������С�������Ƿ����
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
// ����: PerformAssignment
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::PerformAssignment(const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node)
{
	// �������Ƿ�Ϊ��ֵ
	if (!IsLValue(lvalue))
	{
		Error(TXT_ASSIGN_LEFT_MUSTBE_LVALUE, node);
		return;
	}

	// �������Ƿ��Ѿ���ʼ��
	IsVariableInitialized(rvalue, node);

	bool hasError = false;
	if ( lvalue.type.IsPointer() )
	{
		// ָ�븳ֵ
		if ( lvalue.type.IsEqualExceptConst(rvalue.type) || rvalue.isNullPointer )
		{
			bc->Instr2( BC_MOV4, lvalue.operand, rvalue.operand );
		}else if ( lvalue.type.IsObject() && rvalue.type.IsObject() && rvalue.type.IsPointer() )
		{
			// �����ָ�븳ֵ
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
			// ����->ָ��
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
			// �����ֵ
			if ( lvalue.type.IsObject() )
			{
				ObjectAssignment( lvalue, rvalue, bc, node );
				return;
			}

			// ֵ��ֵ
			// �����/��ֵ������һ����	
			psSValue tmp2;
			psCDataType type = lvalue.type;
			type.IsReference() = false;
			type.IsPointer()   = false;

			if ( ImplicitConversion(rvalue, type, tmp2, bc, node, false) )
			{
				// ����ұ��ʽ������,��������ֵ
				if ( tmp2.type.IsReference() )
				{
					// ���������ұ��ʽ������,����һ����ʱ����
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

			// ����Ǿֲ�����,���ó�ʼ����־
			if (lvalue.operand.base == PSBASE_STACK && lvalue.operand.offset < 0 && 
				(!lvalue.isTemporary && !lvalue.type.IsReference() ))
			{
				// ���Ҿֲ�����
				psSVariable *v = m_Variables->GetVariableByOffset(lvalue.operand.offset);
				if (v)
				{
					// ���ó�ʼ����־
					v->value.isInitialized = true;
				}
			}

			// ����������µ���ʱ����,�ͷ���
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
// ����: ImplicitConversion
// ˵��: 
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

	// ����һ��Ĭ��ֵ
	// ������ԭʼ��Ϣ��������(NOTE: 'from' �� 'to'������ͬһ��ʵ��)
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

	// ���ֻ������,  
	if ( type.IsReference() )
	{
		if ( !type.IsReadOnly() && from.type.IsReadOnly() )
			return false;
	}

	// ����->ָ��
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

	// �Ƚ�����Ĵ�С
	if ( !from.type.IsSameArrayDim(type) )
			return false;

	// ����������,ֱ���ж�����������Ƿ�һ��
	if ( type.IsObject() || from.type.IsObject() )
	{
		int baseOffset = from.type.ObjectType()->GetBaseOffset( type.ObjectType() );
		if ( baseOffset < 0 )
			return false;

		to.type.IsPointer()	 = type.IsPointer();
		if ( baseOffset != 0 )
		{
			// ���¼��������ָ��	
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

	// ����ǳ���,ֱ�ӵ��ó���ת��
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
		// ���Դ��һ������,�Ƚ���������ֵ
		AllocateTemporaryVariable(tmpFrom);

		if (bc) InstrRDREF(bc, tmpFrom.operand, from.operand, from.type);
	}

	// Ԥ�ȷ���һ��Ŀ����ʱ����
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
		// �������
		psCString msg;
		msg.Format( TXT_CONVERT_s_TO_s_MAY_LOST_DATA, from.type.GetFormat().AddressOf(), type.GetFormat().AddressOf() );
		Warning( msg.AddressOf(), node );
	}

	// �ͷ���ʱ����		
	if (tmpFrom.operand != from.operand)
	{
		ReleaseTemporaryVariable(tmpFrom);
	}
	return bSuccess;
}

//-------------------------------------------------------------------
// ����: ImplicitConversionConstant
// ˵��: 
//-------------------------------------------------------------------
bool psCCompiler::ImplicitConversionConstant( const psSValue& from, const psCDataType& type, psSValue& to, psCByteCode* bc, psCScriptNode* node, bool isExplicit)
{	
	assert(from.IsConstant());

	//�����������ͬ��ֱ�Ӹ�ֵ�󷵻�
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

			// ����һ�Σ����ܻ�ת����'int8', 'int16'�ȸ�С������
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

			// ����һ�Σ����ܻ�ת����'int8', 'int16'�ȸ�С������
			return ImplicitConversionConstant( to, type, to, bc, node,  isExplicit);
		}else if (from.type.IsIntegerType() && from.type.GetSizeInMemoryBytes() < type.GetSizeInMemoryBytes())
		{
			// ����Ǵ�С����ת�������ͣ���ֱ�Ӹ�ֵ�Ϳ�
			to.type = psCDataType(type.TokenType(), true, false);
			to.operand = from.operand;
			return true;
		}else if (from.type.IsIntegerType() && from.type.GetSizeInMemoryBytes() > type.GetSizeInMemoryBytes() )
		{
			//  ����ǴӴ�����ת��С����
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
// ����: CompileAssignment
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileAssignment(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue)
{
	psCScriptNode *lexpr = expr->GetFirstChild();
	if( lexpr->GetNextNode() )
	{
		// ����һ����ֵ���ʽ
		psCByteCode rbc, lbc;
		psSValue rvalue, lvalue;
		psCScriptNode* opNode = lexpr->GetNextNode();

		// ���븳ֵ���ʽ����/��ԭ��
		CompileAssignment(opNode->GetNextNode(), &rbc, &rvalue);
		CompileCondition(lexpr, &lbc, &lvalue);
		
		int op = opNode->GetTokenType();
		// �������ֵ���ʽ��һ���࣬���������Ƿ��Ѿ�������
		// ���û���Ǿ���һ���򵥵ĸ�ֵ����
		if( lvalue.type.IsObject() && !lvalue.type.IsPointer() )
		{	
			// �ȼ��ұ��ʽ���룬�ټ�����ʽ����
			bc->AddCode(&rbc);
			bc->AddCode(&lbc);
			
			// ���������и�ֵ
			ObjectAssignmentEx( op, exprValue, lvalue, rvalue, bc, expr );

			// �ͷ���ʱ����
			ReleaseTemporaryVariable( rvalue );
			ReleaseTemporaryVariable( lvalue );
			return;
		}else
		{
			// �Լ��������͸�ֵ
			if (op == ttAssignment)
			{
				// ��һ���򵥵ĸ�ֵ����
				// �ȼ��ұ��ʽ���룬�ټ�����ʽ����
				bc->AddCode(&rbc);
				bc->AddCode(&lbc);

				PerformAssignment(lvalue, rvalue, bc, expr);
			}
			else
			{		
				// ��һ������������ĸ�ֵ����
				psSValue tmp;
				CompileOperator(opNode, bc, &lbc, &rbc, &tmp, &lvalue, &rvalue);
				
				PerformAssignment(lvalue, tmp, bc, expr );

				ReleaseTemporaryVariable( tmp );
			}
			// �ͷ���ʱ����
			ReleaseTemporaryVariable( rvalue );

			// �ѱ��ʽ��ֵ��Ϊ����ʽ��ֵ
			// ������ʽ�Ͳ�Ҫ�ͷ���
			*exprValue = lvalue;
		}	
	}else
	{
		// �򵥱��ʽ
		CompileCondition(lexpr, bc, exprValue);
	}
}

//-------------------------------------------------------------------
// ����: CompileCondition
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileCondition(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue)
{
	psCScriptNode* cexpr = expr->GetFirstChild();
	if (cexpr->GetNextNode())
	{
		// ����ԭ������ʱ����
		std::vector< psSValue > oldTempValues = m_TempValues;
		m_TempValues.clear();

		// �������ʽ
		psCByteCode ebc;
		psSValue cvalue;
		CompileExpression(cexpr, &ebc, &cvalue);

		// �����ʽ�������Ƿ�Ϊ'bool'����
		// 'int'��'unsigned int'Ҳ���Ե���'bool'��
		if ( !cvalue.type.IsBooleanType() &&
			 !cvalue.type.IsIntegerType() ||
			  cvalue.type.IsPointer() )
		{
			Error(TXT_EXPR_MUST_BE_BOOL, expr);
		}

		psSOperand cvalueOp;
		// ȷ���������ֽ����ͳ���
		GetIntOperand( &ebc, cvalue, cvalueOp );

		psCByteCode lebc, rebc;
		psSValue lvalue, rvalue;

		// ��������ʽ
		CompileAssignment(cexpr->GetNextNode(), &lebc, &lvalue);
		
		// ����һ����ʱ����	
		// NOTE: ��ʱ������ʱ������Ϊ��
		// ��Ϊ��ʱ�����ǲ��ܱ���ֵ��
		// �ڸ�ֵ��Ϻ�,Ҫ������ʱ������Ϊ��
		exprValue->type = lvalue.type;
		exprValue->type.IsPointer() = false;
		exprValue->type.IsReadOnly()      = false;
		AllocateTemporaryVariable(*exprValue);

		exprValue->isTemporary = false;

		// ��������ƴ���
		int elseLabel = m_NextLabel++;
		int afterLabel = m_NextLabel++;

		// ������������ʽ����
		bc->AddCode(&ebc);

		// ����������ʽ��ֵΪ'false'���������ұ��ʽ����
		bc->Instr2(BC_JZ, cvalueOp, psSOperand(elseLabel));

		// �������ʽ����
		bc->AddCode(&lebc);

		// �����ʽ��ֵ��ֵ
		if ( lvalue.type.IsObject() )
		{
			// �������������ʱ�����
			CopyConstructObject( *exprValue, lvalue, bc, expr );
		}else
		{
			PerformAssignment(*exprValue, lvalue, bc, expr);
		}

		// �ͷ���ʱ�����
		ReleaseTemporaryVariable( lvalue );

		// ����ʽ�е���ʱ�����������ڵ���
		// NOTE: ע������ڱ����ұ��ʽ֮ǰ�ͷ� 
		//       ����ʽ���ɵ���ʱ����, ��Ϊ�������
		//       ���ʽ��ֵΪ'false'����ʽ����ʱû��������ʱ����,
		//       ���ڱ���ʱȴ������, ��һ��Ҫ�ڴ��ͷ�
		ClearTemporaryVariables(bc);

		// �����ұ��ʽ
		CompileAssignment(cexpr->GetNextNode()->GetNextNode(), &rebc, &rvalue);
		psSValue tmp;
		// ����ұ��ʽ�����ܷ�ת��������ʽ
		if (!ImplicitConversion(rvalue, lvalue.type, tmp, &rebc, expr, false))
		{
			psCString str;
			str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s, rvalue.type.GetFormat().AddressOf(), lvalue.type.GetFormat().AddressOf());
			Error(str.AddressOf(), expr);
			return;
		}		

		// ����
		bc->Instr1(BC_JMP, psSOperand(afterLabel));

		// ����ұ��ʽ����
		bc->Label(elseLabel);
		bc->AddCode(&rebc);
		
		// �����ʽ��ֵ��ֵ
		if (tmp.type.IsObject())
		{
			// �������������ʱ�����
			CopyConstructObject( *exprValue, tmp, bc, expr  );
		}else
		{
			// ��ֵ
			PerformAssignment(*exprValue, tmp, bc, expr);
		}	
		// �ͷ���ʱ����
		ReleaseTemporaryVariable( rvalue );
		if ( tmp.operand != rvalue.operand )
			ReleaseTemporaryVariable( tmp );

		// �ұ��ʽ�е���ʱ�����������ڵ���
		ClearTemporaryVariables(bc);

	    bc->Label(afterLabel);

		// �ͷ���ʱ����
		ReleaseTemporaryVariable(cvalue);
		if ( cvalueOp != cvalue.operand )
			ReleaseTemporaryOperand( cvalueOp );

		// �������ʽ�е���ʱ�����������ڵ���
		ClearTemporaryVariables(bc);

		// ����Ϊ��ʱ����
		exprValue->isTemporary = true;

		// �ָ�ԭ������ʱ����
		m_TempValues = oldTempValues;
	}else 
	{
		// ��ѧ���ʽ
		CompileExpression(cexpr, bc, exprValue);
	}
}

//-------------------------------------------------------------------
// ����: CompileExpression
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileExpression(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue)
{	
	assert(expr->GetNodeType() == snExpression);

	// ת�����沨�����ʽ. i.e: a+b => ab+
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

	// ת�������Ϊ����
	SwapPostFixOperands(S2, postfix);

	// ������б��ʽ�ĵ�ֵ
	psCByteCode exprBC;
	CompilePostFixExpression(postfix, &exprBC, exprValue);

	// ��ӱ��ʽ����
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
// ����: CompilePostFixExpression
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompilePostFixExpression(std::vector< psCScriptNode* >& postfix, psCByteCode* bc, psSValue* exprValue)
{	
	// �������һ�����
	psCScriptNode* node = postfix[postfix.size() - 1];
	postfix.pop_back();

	if (node->GetNodeType() == snExprTerm)
	{
		CompileExpressionTerm(node, bc, exprValue);
		return;
	}else
	{
		// �����������ʽԭ��
		psCByteCode rbc, lbc;
		psSValue rvalue, lvalue;

		// NOTE!!!
		// �����bool�����ȴ��Ҫ���������ʱ����
		// ��Ϊ��������ת��䣬���ܵȵ��������ʽ����ʱһ�����
		int boolOp = (node->GetTokenType() == ttOr ||
					  node->GetTokenType() == ttAnd );

		// ��������ʽ
		CompilePostFixExpression(postfix, &lbc, &lvalue);
		if (boolOp)
		{
			ClearTemporaryVariables(&lbc);
		}
		// �����ұ��ʽ
		CompilePostFixExpression(postfix, &rbc, &rvalue);
		if (boolOp)
		{
			ClearTemporaryVariables(&rbc);
		}

		// ���������
		CompileOperator(node, bc, &lbc, &rbc, exprValue, &lvalue, &rvalue);

		// �ͷ���ʱ����
		ReleaseTemporaryVariable( rvalue );	
		ReleaseTemporaryVariable( lvalue );
	}
}


void psCCompiler::CompileExpressionTerm(psCScriptNode* term, psCByteCode* bc, psSValue* exprValue)
{
	psCScriptNode* vnode = term->GetFirstChild();
	while (vnode && vnode->GetNodeType() != snExprValue)
		vnode = vnode->GetNextNode();

	// ������ʽֵ���
	psCByteCode vbc;
	CompileExpressionValue(vnode, &vbc, exprValue );
	
	// ������ò�����
	psCScriptNode *pnode = vnode->GetNextNode();
	while( pnode )
	{
		CompileExpressionPostOp(pnode, &vbc, exprValue);
		pnode = pnode->GetNextNode();

	}

	// ����ǰ�ò�����
	pnode = vnode->GetPrevNode();
	while ( pnode )
	{		
		CompileExpressionPrevOp(pnode, &vbc, exprValue);
		pnode = pnode->GetPrevNode();	
	}

	// ��Ӷ����ƴ���
	bc->AddCode(&vbc);
}


//-------------------------------------------------------------------
// ����: CompileExpressionPostOp
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileExpressionPostOp(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue)
{
	// �õ�������
	int op = node->GetTokenType();

	// �������Ƿ��Ѿ���ʼ��
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

			// �ͷ���ʱ����
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
		// ����һ����ʱ����
		psSValue ret(exprValue->type);
		AllocateTemporaryVariable(ret);

		// �������ݵ�����ֵ��
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

		// �ͷ���ʱ����
		ReleaseTemporaryVariable( *exprValue );
		// ��дָ������
		if ( exprValue->type.IsReference() )
		{
			InstrWTREF( bc, exprValue->operand, tmp.operand, exprValue->type );
			ReleaseTemporaryVariable( tmp );
		}

		// ���ñ��ʽ��ֵ
		*exprValue = ret;
		exprValue->type.IsReference() = false;
	}else
	{
		Error( TXT_ILLEGAL_OPERATION, node );
	}

}

//-------------------------------------------------------------------
// ����: CompileExpressionPrevOp
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileExpressionPrevOp(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue)
{
	// �õ�������
	int op = node->GetTokenType();

	// �����Ƿ��Ѿ���ʼ��
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
			// ������ֵַ
			// NOTE: ���ܽ�����ʱ�����ĵ�ֵַ
			//		 Ҳ���ܽ�������ĵ�ֵַ
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
					// ����һ��ʱ����
					psSValue tmp(exprValue->type);
					tmp.type.IsPointer()   = true;	
					tmp.type.IsReference() = true;
 					AllocateTemporaryVariable(tmp);

					bc->Instr2(BC_DEFREF, tmp.operand, exprValue->operand);

					// �ͷ���ʱ����
					ReleaseTemporaryVariable( *exprValue );

					// �������ñ��ʽ��ֵ
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
			// ����ǳ�����
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

		// ����һ����ʱ����
		psSValue tmp(exprValue->type);
		AllocateTemporaryVariable( tmp );

		// �����ָ�룬�Ƚ�����ֵ
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

		// �ͷ���ʱ����
		ReleaseTemporaryVariable( *exprValue );
		if ( exprValue->type.IsReference() )
		{
			ReleaseTemporaryVariable( tmpExpr );
		}

		// ������������ֵ(����������������)
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
				// ����һ��ʱ����
				psSValue tmp(exprValue->type);
				AllocateTemporaryVariable(tmp);

				// �����ָ�룬�Ƚ�����ֵ
				psSValue tmpExpr = *exprValue;
				if ( exprValue->type.IsReference() )
				{
					AllocateTemporaryVariable( tmpExpr );
					InstrRDREF( bc, tmpExpr.operand, exprValue->operand, tmpExpr.type );
				}

				bc->Instr2(BC_NOT, tmp.operand, tmpExpr.operand);

				// �ͷ���ʱ����
				ReleaseTemporaryVariable( *exprValue );
				if ( exprValue->type.IsReference() )
				{
					ReleaseTemporaryVariable( tmpExpr );
				}

				// ������������ֵ
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
			// �����ָ�룬�Ƚ�����ֵ
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
			// ��д
			if ( exprValue->type.IsReference() )
			{
				InstrWTREF( bc, exprValue->operand, tmpExpr.operand, exprValue->type );
				ReleaseTemporaryVariable( tmpExpr );
			}
		}
	}else if (op == ttNot)
	{
		// ֻ��'bool,'int','unsigned int'�ͣ��Դ˲���������Ч
		if( exprValue->type.IsBooleanType() ||
			exprValue->type.IsIntegerType() )
		{		
			// ����һ��ʱ����
			psSValue tmp(g_BoolDataType);
			AllocateTemporaryVariable( tmp );

			// �����ָ�룬�Ƚ�����ֵ
			psSValue tmpExpr = *exprValue;
			if ( exprValue->type.IsReference() )
			{
				AllocateTemporaryVariable( tmpExpr );
				InstrRDREF( bc, tmpExpr.operand, exprValue->operand, tmpExpr.type );
			}

			bc->Instr3(BC_TEi, tmp.operand, tmpExpr.operand, MakeIntValueOp(0) );

			// �ͷ���ʱ����
			ReleaseTemporaryVariable( *exprValue );
			if ( exprValue->type.IsReference() )
			{
				ReleaseTemporaryVariable( tmpExpr );
			}

			// �������ñ��ʽ��ֵ
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
// ����: CompileExpressionValue
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileExpressionValue(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue)
{

	psCScriptNode *vnode = expr->GetFirstChild();
	if( vnode->GetNodeType() == snIdentifierValue )
	{
		CompileIdentifierValue( vnode, bc, exprValue );
	}else if (vnode->GetNodeType() == snConstant)
	{
		// �õ������ַ���
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
			// ���ͳ���
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
			// �ַ�����
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
		}else if (vnode->GetTokenType() == ttBitsConstant)
		{
			// ���������ͳ���
			int val = 0;
			ps_sscanf(buf, PS_T("%x"), &val);

			int offset = m_pBuilder->RegisterConstantInt(val);
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
			exprValue->type.TokenType() = ttInt;

		}else if (vnode->GetTokenType() == ttFloatConstant)
		{
			// �����ͺų���
			int offset = m_pBuilder->RegisterConstantFloat( (float)ps_strtod(buf, &end) );
			exprValue->type.TokenType() = ttFloat;
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);

		}else if (vnode->GetTokenType() == ttTrue ||
				  vnode->GetTokenType() == ttFalse)
		{
			// �߼��ͳ���
			int offset = m_pBuilder->RegisterConstantInt((vnode->GetTokenType() == ttTrue) ? 1 :0);
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
			exprValue->type.TokenType() = ttBool;

		}else if (vnode->GetTokenType() == ttNull)
		{
			// ��ָ�볣��
			int offset = m_pBuilder->RegisterConstantInt(0);
			exprValue->operand.BaseOffset(PSBASE_CONST, offset);
			exprValue->type.TokenType()   = ttInt;
			exprValue->type.IsReference() = true;
			exprValue->type.IsPointer()	  = true;
			exprValue->isNullPointer	  = true;

		}else if (vnode->GetTokenType() == ttDoubleConstant)
		{
			// 'double'�ͳ���
			int offset = m_pBuilder->RegisterConstantDouble(ps_strtod(buf, &end));
			exprValue->operand = psSOperand(PSBASE_CONST, offset);
			exprValue->type.TokenType() = ttDouble;
		}else if (vnode->GetTokenType() == ttStringConstant ||
				  vnode->GetTokenType() == ttWStringConstant )
		{
			// �ַ�������
			psCString str, cat;

			int string_start = 1;
			if ( vnode->GetTokenType() == ttWStringConstant )
			{		
				++string_start;
			}

			// ��������ַ��� i.e. "good"
			//					   "hello"
			psCScriptNode *snode = vnode->GetFirstChild();
			while( snode )
			{
				cat.Copy(&m_pScript->m_Code[snode->GetTokenPos()+string_start], (snode->GetTokenLength()-(1+string_start) ) );

				str += cat;

				snode = snode->GetNextNode();
			}

			// עɾ�ַ�������
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
			// thisָ��ֵ
			if ( m_ObjectType == NULL )
			{
				// ����������Ա������thisָ����Ч
				Error(TXT_INVALID_THIS_POINTER, vnode);

				// ��֤��������
				exprValue->type = psCDataType(ttInt, false, false);
				AllocateTemporaryVariable( *exprValue );
			}else
			{
				// ��������ֵ
				exprValue->type				  = m_ObjectType->m_DataType;
				exprValue->type.IsReadOnly()  = false;
				exprValue->type.IsPointer()   = true;
				exprValue->type.IsReference() = true;

				// ����һ��ʱ������
				AllocateTemporaryVariable(*exprValue);

				// �õ�thisָ��ĵ�ֵַ
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

	// ����
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

	// �õ�������
	psCHAR name[256];
	GetNodeSource( name, 255, n );

	//bool hasError = false;

	if ( hasField )
	{
		if (field[0] == 0 )
		{
			// ȫ������
			if ( !GetGlobalProperty( name, bc, exprValue ) )
			{
				// Error
				psCString str;
				str.Format(TXT_s_NOT_DECLARED, name );
				Error(str.AddressOf(), vnode);
			}
		}else
		{
			// �Ƿ�Ϊ�����ö�ٳ���
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
			// ������Ǿֲ�����,,���Ƿ�Ϊ��ĳ�Ա����
			if ( m_ObjectType && GetObjectProperty( m_ObjectType, name, exprValue ) )
				return;

			// �Ƿ�Ϊȫ�ֱ���
			if ( GetGlobalProperty( name, bc, exprValue ) )
				return;

			// Error
			psCString str;
			str.Format(TXT_s_NOT_DECLARED, name );
			Error(str.AddressOf(), vnode);
		}else
		{	
			// �ֲ�����/����
			*exprValue = var->value; 

			// NOTE: ����������ʱ����
			// ��Ϊ�п���һ���������ͺŵı�������ʱ��, ���Ҫ���ͷ�
			// ��ʱisTempoary��ֵΪ��, �����ʱ��������ʱ����,
			// �����ʱ�����ͻᱻ�ͷŶ��
			exprValue->isTemporary   = false;		
			exprValue->isInitialized = true;
		}
	}
}

bool psCCompiler::GetGlobalProperty( const psCHAR* name, psCByteCode* bc, psSValue* exprValue)
{
	// ��ȫ�ֱ����в���
	psSProperty* prop = m_pBuilder->GetGlobalProperty(name);
	if (prop)
	{
		// ����ֵ�����͡�
		exprValue->type = prop->type;

		// ��������ֵ
		exprValue->isTemporary   = false;
		exprValue->isInitialized = true;

		// ������ԵĻ�ַ��ʽΪ����Ѱַ,����Ϊ���ͳ���
		if ( prop->base == PSBASE_NONE )
		{
			// ���ͳ���
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
			// ���ȫ�����Ե�����С��0,����ϵͳ��ȫ�ֶ���,����Ϊģ���ȫ�ֱ���
			if (prop->index < 0)
			{
				// ���ʽ����������Ϊ��
				exprValue->type.IsReference() = true;

				// ����һ����ʱ������
				AllocateTemporaryVariable(*exprValue);

				// �õ����Եĵ�ֵַ(��Ϊ����������)
				bc->Instr2(BC_PGA, exprValue->operand, prop->index);
			}else
			{
				// �õ�ȫ�����Բ�����
				exprValue->operand.BaseOffset(prop->base, prop->index);
			}
		}
		return true;
	}

	// �Ƿ�Ϊö�ٳ���
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
	// �˺���Ϊ��ĳ�Ա����, �˱����п���Ϊ��ĳ�Ա
	int baseOffset = 0;
	psSProperty *prop = m_pBuilder->GetObjectProperty( objType, name, baseOffset );
	if( prop )
	{	
		// ��������ֵ
		exprValue->type					 = prop->type;
		exprValue->isInitialized		 = true;
		exprValue->operand.BaseOffset( PSBASE_HEAP, prop->byteOffset + baseOffset );
		return true;
	}

	// �Ƿ�Ϊ�����ö�ٳ���
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
// ����: CompileOperator
// ˵��: 
//-------------------------------------------------------------------
bool psCCompiler::CompileOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	// �������Ƿ��Ѿ���ʼ��
	IsVariableInitialized(*v1, node);
	IsVariableInitialized(*v2, node);

	// ���ȣ��������жԴ˲���������
	if( CompileOverloadedOperator(node,  bc, lbc, rbc, v0, v1, v2) )
		return true;

	if ( v1->type.IsPointer() || v2->type.IsPointer() )
	{
		CompilePointerOperator( node, bc, lbc, rbc, v0, v1, v2 );
		return false;
	}

	//  �������ʽ��ֻ��Ϊ��������
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
			// NOTE: ע�������ʱ����λ��, ����ʽ��Ӧ�ŵ�����ʽ�Ĵ�����
			//		 �ұ��ʽӦ�ŵ��ұ��ʽ�Ĵ�����
			//		 ��������õĴ�����򲻶�.
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

			// ������ұ��ʽ�Ĵ���
			// ����Ǹ�ֵ���ʽ���������Ǵ��ҵ���
			if ( bAssign )
			{
				bc->AddCode( rbc );
				bc->AddCode( lbc );
			}else
			{
				bc->AddCode( lbc );
				bc->AddCode( rbc );
			}

			// ��ӽ����õı��ʽ����
			if ( v1->type.IsReference() )
			{
				InstrRDREF( bc, tmp1.operand, v1->operand, tmp1.type );
			}
			if ( v2->type.IsReference() )
			{	
				InstrRDREF( bc, tmp2.operand, v2->operand, tmp2.type );
			}

			// ��Ӳ���������Ĵ���
			bc->AddCode( &opBC );

			// ���ڷǲ������㣬Ҫע��������ɵ�˳��Ҫ����ʱ�������ɵ�˳��һ��
			// �������Ϊ����
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
		// ָ��Ӽ�(+=, +, -, -=)
		if ( v1->type.IsPointer() && v2->type.IsIntegerType() && !v2->type.IsPointer() )
		{
			v0->type = v1->type;	

			// ����һ����ʱ����
			AllocateTemporaryVariable(*v0 );

			if ( op == ttAddAssign || op == ttSubAssign )
			{
				// ����Դ��ҵ���
				bc->AddCode( rbc );
				bc->AddCode( lbc );
			}else
			{
				// ����Դ�����
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
			//  ����һ����ʱ����
			v0->type = g_BoolDataType;
			AllocateTemporaryVariable(*v0);

			// ����Դ�����
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
// ����: CompileMathOperator
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileMathOperator(psCScriptNode* node, psCByteCode* bc, bool& bAssign, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	psSValue tmp1;	
	psSValue tmp2;
	
	bAssign = false;

	// �ѱ��ʽ��ֵ��ת����ͳһ���������͡�
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

	// ������ת��
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

	// ����ֵ������
	v0->type = psCDataType(to.TokenType(), true, false);

	// �õ�������
	int op = node->GetTokenType();

	bool isConstant = tmp1.IsConstant() && tmp2.IsConstant();
	if (!isConstant)
	{
		// ����һ����ʱ����	
		AllocateTemporaryVariable( *v0 );

		if( op == ttAddAssign || op == ttSubAssign ||
			op == ttMulAssign || op == ttDivAssign ||
			op == ttModAssign )
		{
			// ����Ǹ�ֵ���㣬�����Ӧ���Ǵ��ҵ���
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
				// ������'0'�쳣
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
				// ����ģ'0'�쳣
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
		// ��������
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

	//  �ͷ���ʱ����(����еĻ�)
	if (tmp1.operand != v1->operand)
		ReleaseTemporaryVariable(tmp1);

	if (tmp2.operand != v2->operand)
		ReleaseTemporaryVariable(tmp2);
}

//-------------------------------------------------------------------
// ����: CompileBitwiseOperator
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileBitwiseOperator(psCScriptNode* node, psCByteCode* bc,  bool& bAssign, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	// �õ�������
	int op = node->GetTokenType();

	// ���ʽ�����Ͷ�����Ϊ'int'����
	psSValue tmp1, tmp2;

	// ������ת��	
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

	// ���ñ��ʽ������
	v0->type = tmp1.type;

	if (!isConstant)
	{
		// ����һ����ʱ����	
		AllocateTemporaryVariable( *v0 );

		if (op == ttAndAssign ||
			op == ttOrAssign  ||
			op == ttXorAssign ||
			op == ttShiftLeftAssign ||
			op == ttShiftRightAssign )
		{	
			// ����Ǹ�ֵ���㣬�����Ӧ���Ǵ��ҵ���
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
		// ��Ч�Ĳ�����
		assert(false);
	}

	// �ͷ���ʱ����(����еĻ�)
	if (tmp1.operand != v1->operand)
		ReleaseTemporaryVariable(tmp1);
	if (tmp2.operand != v2->operand)
		ReleaseTemporaryVariable(tmp2);
}

//-------------------------------------------------------------------
// ����: CompileComparisionOperator
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileComparisionOperator(psCScriptNode* node, psCByteCode* bc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	// ����������ߵĲ�����ת����ͬһ������
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
	// ������ת��
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

	//�����ñ��ʽ������
	v0->type = g_BoolDataType;

	if (!isConstant)
	{
		// ����һ����ʱ����	
		AllocateTemporaryVariable( *v0 );
	}
	
	// �õ�������
	int op = node->GetTokenType();

	if (type.IsBooleanType())
	{
		// 'bool'����
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

	// �ͷ���ʱ����(����еĻ�)
	if (tmp1.operand != v1->operand)
		ReleaseTemporaryVariable(tmp1);
	if (tmp2.operand != v2->operand)
		ReleaseTemporaryVariable(tmp2);
}

//-------------------------------------------------------------------
// ����: CompileBooleanOperator
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileBooleanOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	// ���ʽ�����Ͷ�����Ϊ'boolean'����
	psSValue tmp1, tmp2;

	// ������ת��	
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

	// ����ֵ������
	v0->type = g_BoolDataType;

	if (!isConstant)
	{
		// ����һ����ʱ����	
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
			// �������ʽ�Ĵ���
			bc->AddCode(lbc);

			int label1 = m_NextLabel++;
			int label2 = m_NextLabel++;
			
			if (op == ttAnd)
			{
				// ʵ�ֶ�·����
				bc->Instr2(BC_JNZ, tmp1.operand, label1);
					
				// ���ñ��ʽ��ֵ
				bc->Instr2(BC_MOV4, v0->operand, psSOperand(PSBASE_CONST, offset0));

				// ������β
				bc->Instr1(BC_JMP, label2);
			}else 
			{
				// ʵ�ֶ�·����
				bc->Instr2(BC_JZ, tmp1.operand, label1);

				// ���ñ��ʽ��ֵ
				bc->Instr2(BC_MOV4, v0->operand, psSOperand(PSBASE_CONST, offset1));

				// ������β
				bc->Instr1(BC_JMP, label2);
			}

			bc->Label(label1);

			// ����ұ��ʽ�Ĵ���
			bc->AddCode(rbc);

			// ���ñ��ʽ��ֵ
			bc->Instr3( BC_TNEi, v0->operand, tmp2.operand, psSOperand(PSBASE_CONST, offset0) );

			bc->Label(label2);
		}

		// �ͷ���ʱ����(����еĻ�)
		if (tmp1.operand != v1->operand)
			ReleaseTemporaryVariable(tmp1);
		if (tmp2.operand != v2->operand)
			ReleaseTemporaryVariable(tmp2);
	}
}

//-------------------------------------------------------------------
// ����: CompileStatement
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileStatement(psCScriptNode* node, bool* hasReturn, psCByteCode* bc)
{
	// ��ʱӦ��û���κ���ʱ����
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

	// ��ղ�����������ʱ����
	ClearTemporaryVariables(bc);
}


void psCCompiler::CompileExpressionStatement(psCScriptNode *enode, psCByteCode *bc)
{
	// ���һ����ָ��
	InstrLINE(bc, enode->GetTokenPos());

	if( enode->GetFirstChild() )
	{
		// ������ʽ	
		psSValue exprValue;
		CompileAssignment(enode->GetFirstChild(), bc, &exprValue);

		// �ͷ���ʱ����
		ReleaseTemporaryVariable( exprValue );
	}
}

