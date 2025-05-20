#include "compiler.hpp"

//-------------------------------------------------------------------
// ����: DefaultConstructor
// ˵��: 
//-------------------------------------------------------------------
bool psCCompiler::DefaultConstructor(const psSValue& value, psCByteCode *bc)
{
	psSTypeBehaviour *beh = value.type.GetBehaviour();
	assert(beh != 0 );

	bool scriptObject = value.type.ObjectType()->m_bScriptObject;
	// ����ǽӿ������,����FALSE
	if ( beh->isInterface )
		return false;

	// ���������Ĺ��캯��, ����еĻ�
	if ( bc && beh->hasConstructor )
	{	
		int funcId = beh->construct;

		int num = value.type.GetNumArrayElements();
		if ( num > 4 )
		{
			psSValue thisPointer( psCDataType(ttInt, false, false) );
			AllocateTemporaryVariable( thisPointer );

			psSValue loop_i(psCDataType(ttInt, false, false) );
			AllocateTemporaryVariable( loop_i );

			psSValue loop_r(psCDataType(ttInt, false, false) );
			AllocateTemporaryVariable( loop_r );

			psSOperand loop_num( MakeIntValueOp(num) );
			psSOperand obj_size( MakeIntValueOp(value.type.GetSizeInMemoryBytes()) );
			int beforeLabel   = m_NextLabel++;

			bc->Instr2(BC_DEFREF, thisPointer.operand, value.operand );
			bc->Instr2(BC_MOV4, loop_i.operand, MakeIntValueOp(0) );

			bc->Label( beforeLabel );

			// Push this pointer
			bc->Push4( thisPointer.operand );

			// Call constructor
			if ( scriptObject )
				bc->Call( funcId, 0, true );
			else
				bc->CallSys( funcId, 0, true );

			bc->Instr1(BC_INCi, loop_i.operand );
			bc->Instr3(BC_TLi,  loop_r.operand, loop_i.operand, loop_num );
			bc->Instr3(BC_ADDi, thisPointer.operand, thisPointer.operand, obj_size );
			bc->Instr2(BC_JNZ,  loop_r.operand, beforeLabel );

			ReleaseTemporaryVariable( loop_i );
			ReleaseTemporaryVariable( loop_r );
			ReleaseTemporaryVariable( thisPointer );
		}else
		{
			psSOperand thisPointer = value.operand;
			for (int i = 0; i < num; ++i )
			{
				// Push this pointer
				bc->PushRef( thisPointer );

				// Call constructor
				if ( scriptObject )
					bc->Call( funcId, 0, true );
				else
					bc->CallSys( funcId, 0, true );

				thisPointer.offset += value.type.GetSizeInMemoryBytes();
			}
		}
	}

	return true;
}

//-------------------------------------------------------------------
// ����: CompileConstructor
// ˵��: 
//-------------------------------------------------------------------
bool psCCompiler::CompileConstructor(const psSValue& value, psCByteCode *bc)
{
	// Call constructor for the data type
	if( value.type.IsObject() && !value.type.IsReference())
	{
		return DefaultConstructor(value, bc);
	}
	return true;
}

//-------------------------------------------------------------------
// ����: CompileDestructor
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::CompileDestructor(const psSValue& value, psCByteCode *bc)
{
	if( !value.type.IsReference() )
	{
		if( value.type.IsObject())
		{
			psCObjectType* objType = value.type.ObjectType();
			if ( objType->m_Beh.hasDestructor )
			{		
				psSTypeBehaviour *beh = value.type.GetBehaviour();

				int funcId = beh->destruct;
				bool scriptObject = value.type.ObjectType()->m_bScriptObject;

				int num = value.type.GetNumArrayElements();
				if ( num > 4 )
				{
					psSValue thisPointer( psCDataType(ttInt, false, false) );
					AllocateTemporaryVariable( thisPointer );

					psSValue loop_i(psCDataType(ttInt, false, false) );
					AllocateTemporaryVariable( loop_i );

					psSValue loop_r(psCDataType(ttInt, false, false) );
					AllocateTemporaryVariable( loop_r );

					psSOperand loop_num( MakeIntValueOp(num) );
					psSOperand obj_size( MakeIntValueOp(value.type.GetSizeInMemoryBytes()) );

					int beforeLabel   = m_NextLabel++;

					bc->Instr2(BC_DEFREF, thisPointer.operand, value.operand );
					bc->Instr2(BC_MOV4,   loop_i.operand,   MakeIntValueOp(0) );

					bc->Label( beforeLabel );

					// Push this pointer
					bc->Push4( thisPointer.operand );

					// Call constructor
					if ( scriptObject )
						bc->Call( funcId, 0, true );
					else
						bc->CallSys( funcId, 0, true );

					bc->Instr1(BC_INCi, loop_i.operand );
					bc->Instr3(BC_TLi,  loop_r.operand, loop_i.operand, loop_num );
					bc->Instr3(BC_ADDi, thisPointer.operand, thisPointer.operand, obj_size);
					bc->Instr2(BC_JNZ,  loop_r.operand, beforeLabel );

					ReleaseTemporaryVariable( loop_i );
					ReleaseTemporaryVariable( loop_r );
					ReleaseTemporaryVariable( thisPointer );
				}else
				{
					psSOperand thisPointer = value.operand;
					for (int i = 0; i < num; ++i )
					{
						// Push this pointer
						bc->PushRef( thisPointer );

						// Call constructor
						if ( scriptObject )
							bc->Call( funcId, 0, true );
						else
							bc->CallSys( funcId, 0, true );

						thisPointer.offset += value.type.GetSizeInMemoryBytes();
					}
				}
			}
		}
	}
}
//-------------------------------------------------------------------
// ����: CompileClassDecl
// ˵��: ����������
//-------------------------------------------------------------------
int psCCompiler::CompileClassDecl(psCBuilder* builder, psCScriptCode* script, psCScriptNode* node, std::vector<psCScriptNode*>& methods, psCObjectType* objectType)
{
	assert( objectType );

	// ���ñ�����
	Reset( builder, script, objectType );

	int byteoffset = 0;

	psCScriptNode* n = node->GetFirstChild();

	// �õ�����
	psCHAR name[256];
	GetNodeSource( name, 255, n );

	// ������Ƴ�ͻ
	if ( m_pBuilder->CheckNameConflictObject(name, node, script) < 0 )
	{
		return -1;
	}

	// ���ö��������
	objectType->m_Name = name;
	objectType->m_DataType.TokenType() = ttIdentifier;

	methods.clear();
	n = n->GetNextNode();
	if ( n && n->GetNodeType() == snIdentifier )
	{
		// ������м̳�
		psCHAR parentName[256];
		GetNodeSource( parentName, 255, n );

		psCObjectType* parentObjType = m_pBuilder->GetObjectType(parentName);
		if ( parentObjType == NULL )
		{
			psCString msg;
			msg.Format( TXT_IDENTIFIER_s_NOT_DATA_TYPE, parentName );
			Error( msg.AddressOf(), node );
		}else
		{
			if ( !parentObjType->m_Beh.hasConstructor)
			{
				psCString msg;
				msg.Format( TXT_OBJECT_s_IS_NOT_INERITABLE, parentName );
				Error( msg.AddressOf(), node );
			}else
			{
				objectType->m_pParent = parentObjType;
				byteoffset += parentObjType->m_Size;
			}
		}
		n = n->GetNextNode();
	}

	psCByteCode bc;
	// �������еĶ�������
	while (n)
	{
		if ( n->GetNodeType() == snPropertyDecl )
		{
			// �������������Խ��
			CompilePropertyDecl(n, objectType, byteoffset, &bc);
		}else if ( n->GetNodeType() == snEnum )
		{
			// �������ö�ٳ���
			CompileEnum( n, objectType, objectType->m_Name.AddressOf() );
		}else
		{
			// �������ĳ�Ա����,����������ؽ�㱣������
			methods.push_back(n);
		}
		n = n->GetNextNode();
	}

	// ���ö���Ĵ�С
	if ( byteoffset == 0 )
		objectType->m_Size = sizeof(psDWORD);
	else
		objectType->m_Size = byteoffset;

	// ������Ϊ�ű������
	objectType->m_bScriptObject = true;

	// ��Ӷ����ƴ���
	m_ByteCode.AddCode( &bc );

	return 0;
}

//-------------------------------------------------------------------
// ����: CompilePropertyDecl
// ˵��: �������Ա��������
//-------------------------------------------------------------------
int psCCompiler::CompilePropertyDecl(psCScriptNode* decl, psCObjectType* objectType, int& byteoffset, psCByteCode* bc)
{
	// �õ��������ͽ��
	psCScriptNode* node = decl->GetFirstChild();

	// �õ���������
	psCDataType raw_type = m_pBuilder->CreateDataTypeFromNode(node, m_pScript);

	// ������͵ĺϷ���
	if( raw_type.GetSizeOnStackDWords() == 0 )
	{
		psCString str;
		str.Format(TXT_DATA_TYPE_CANT_BE_s, raw_type.GetFormat().AddressOf() );
		Error( str.AddressOf(), decl );
	}

	// 
	node = node->GetNextNode();

	psCDataType type;
	psCString name;
	// �������е�����,�����ж��(����: int a, b, c;)
	while (node)
	{
		type = raw_type;

		// ������������(�Ƿ�Ϊ����)
		m_pBuilder->ModifyDataTypeFromNode( type, node, m_pScript );

		// �õ����ƽ��
		node = node->GetNextNode();

		// �����������
		CompileVariableDim(node, bc, name, type);

		if ( name == objectType->m_Name )
		{
			psCString msg;
			msg.Format(TXT_NAME_CONFLICT_s_EXTENDED_TYPE, name.AddressOf() );
			Error( msg.AddressOf(), decl );
		}

		if ( type.IsReference() || type.IsPointer() )
		{
			Error(TXT_PROPERTY_CANNOT_BE_REF, decl);
		}

		// ����ѳ�Ա����Ϊ������Ҳ������ô���,������Ƿ��ܱ�ʵ����
		if ( ( type.IsObject() )  )
		{
			if ( type.ObjectType()->m_Beh.isInterface )
			{
				psCString str;
				str.Format(TXT_OBJECT_s_CANNOT_BE_INSTANCE, type.ObjectType()->m_Name.AddressOf() );
				Error(str.AddressOf(), decl);
			}
		}

		// ��������Ƿ��Ѿ�����
		bool nameIsConflict = false;
		size_t n;
		for (n = 0; n < objectType->m_Properties.size(); ++n)
		{
			if (objectType->m_Properties[n]->name == name)
			{
				nameIsConflict = true;
				break;
			}
		}
		// ����Ƿ���ö�ٳ�����ͻ
		int tmpValue;
		if (m_pBuilder->GetConstantEnum(objectType->m_Name.AddressOf(), name.AddressOf(), tmpValue) )
		{
			nameIsConflict = true;
		}

		if ( nameIsConflict )
		{	
			// ����Ϊ'name'�������Ѿ�����
			psCString msg;
			msg.Format(TXT_PROPERTY_s_ALREADY_EXIST, name.AddressOf());

			Error(msg.AddressOf(), decl);
		}else
		{
			// ����һ���µ����Գ�Ա
			psSProperty* prop = new psSProperty();
			prop->byteOffset = byteoffset;
			prop->name = name;
			prop->type = type;
			prop->base = PSBASE_HEAP;
			objectType->m_Properties.push_back(prop);
		}

		// �����ֽڱ���
		if ( type.IsReference() )
			byteoffset += sizeof(psDWORD);
		else
			byteoffset += type.GetSizeInMemoryBytes();

		// �Ƶ���һ�����
		node = node->GetNextNode();
	}

	return 0;
}

//-------------------------------------------------------------------
// ����: DefaultClassConstructor
// ˵��: �������Ա���Ե�Ĭ�Ϲ��캯��
//-------------------------------------------------------------------
void psCCompiler::DefaultObjectConstructor(psCObjectType* objectType, psCByteCode* bc)
{	
	// ���ø���Ĺ��캯��
	if ( objectType->m_pParent )
	{
		psSValue thisPointer;
		thisPointer.operand.BaseOffset(PSBASE_STACK, 0 );
		thisPointer.type.SetObjectType( objectType->m_pParent );
		thisPointer.type.IsReference() = true;

		assert( objectType->m_pParent->m_Beh.hasConstructor );
		CallObjectMethod( thisPointer, objectType->m_pParent->m_Beh.construct, 0, bc );
	}

	// ���ó�Ա�Ĺ��캯��
	for (size_t i = 0; i < objectType->m_Properties.size(); ++i)
	{
		psSProperty* prop = objectType->m_Properties[i];
		psCObjectType* propObjType = prop->type.ObjectType();
		if (propObjType)
		{
			// �õ���Ա����
			psSValue dst;
			dst.type = prop->type;
			dst.operand.BaseOffset( PSBASE_HEAP, prop->byteOffset );

			// ��������ó�ʼ��Ϊ0
			if ( prop->type.IsReference() )
			{
				bc->Instr2( BC_MOV4, dst.operand, MakeIntValueOp(0) );
			}else if ( propObjType->m_Beh.hasConstructor )
			{
				// Call constructor
				CallObjectMethod( dst, propObjType->m_Beh.construct, 0, bc  );
			}
		}
	}
}

void psCCompiler::DefaultObjectCopyConstructor(psCObjectType* objectType, psCByteCode* bc)
{
	psCByteCode copyBC;

	bool bNeedConstructor = false;
	bool bNeedCopy		  = false;

	// ���ó�Ա�Ĺ��캯��
	for (size_t i = 0; i < objectType->m_Properties.size(); ++i)
	{
		psSProperty*   prop = objectType->m_Properties[i];
		psCObjectType* propObjType = prop->type.ObjectType();

		if (propObjType && propObjType->m_Beh.hasCopyConstructor && !prop->type.IsReference() )
		{
			psSValue src, dst;
			dst.type = prop->type;
			dst.operand.BaseOffset( PSBASE_HEAP, prop->byteOffset );

			// �����Ա���Եĵ�ֵַ
			src.type = prop->type;
			src.type.IsReference() = true;
			AllocateTemporaryVariable(src);

			psSOperand offsetOp = MakeIntValueOp(prop->byteOffset);
			copyBC.Instr3(BC_ADDi, src.operand, psSOperand(PSBASE_STACK, 4), offsetOp );

			// Push argument
			copyBC.Push4( src.operand );

			// Call copy constructor
			CallObjectMethod( dst, propObjType->m_Beh.copy, 1, &copyBC );

			// �ͷ���ʱ����
			ReleaseTemporaryVariable( src );

			bNeedConstructor = true;	
		}else
		{
			bNeedCopy = true;
		}
	}

	if ( bNeedCopy )
	{
		bc->Instr3( BC_MEMCPY, psSOperand(PSBASE_STACK, 0), psSOperand(PSBASE_STACK, 4), objectType->GetSizeInMemoryBytes() );
	}
	if ( objectType->m_pParent && objectType->m_pParent->m_Beh.hasCopyConstructor )
	{
		// ���ø���Ĺ��캯��
		psSValue thisPointer;
		thisPointer.operand.BaseOffset(PSBASE_STACK, 0 );
		thisPointer.type.SetObjectType( objectType->m_pParent );
		thisPointer.type.IsReference() = true;

		// push argument
		bc->Push4( psSOperand( PSBASE_STACK, 4) );

		// call copy constructor
		CallObjectMethod( thisPointer, objectType->m_pParent->m_Beh.copy, 1, bc );
	}
	if ( bNeedConstructor )
	{
		bc->AddCode( &copyBC );
	}
}

//-------------------------------------------------------------------
// ����: DefaultObjectDestructor
// ˵��: �������Ա���Ե�Ĭ����������
//-------------------------------------------------------------------
// �������Ա���Ե�Ĭ�Ϲ�������
void psCCompiler::DefaultObjectDestructor(psCObjectType* objectType, psCByteCode* bc)
{
	// ���ø������������
	if ( objectType->m_pParent && objectType->m_Beh.hasDestructor )
	{
		psSValue thisPointer;
		thisPointer.operand.BaseOffset(PSBASE_STACK, 0 );
		thisPointer.type.SetObjectType( objectType->m_pParent );
		thisPointer.type.IsReference() = true;

		CallObjectMethod( thisPointer, objectType->m_pParent->m_Beh.construct, 0, bc );
	}

	// ���ó�Ա����������
	for (size_t i = 0; i < objectType->m_Properties.size(); ++i)
	{
		psSProperty* prop = objectType->m_Properties[i];
		psCObjectType* propObjType = prop->type.ObjectType();
		if (propObjType && propObjType->m_Beh.hasDestructor && !prop->type.IsReference() )
		{
			// �õ���Ա����
			psSValue dst;
			dst.type = prop->type;
			dst.operand.BaseOffset( PSBASE_HEAP, prop->byteOffset );

			// Call destructor
			CallObjectMethod( dst, propObjType->m_Beh.destruct, 0, bc );
		}
	}	
}

void psCCompiler::CompileObjectConstructor(psCScriptNode* node, psCDataType& type, const psCString& name, psSValue& value, psCByteCode* bc, psSGlobalVariableDesc* gvar)
{
	assert( node->GetNodeType() == snArgList);
	// ������ʼ�����ʽ
	if( !type.IsObject() )
	{
		Error(TXT_MUST_BE_OBJECT, node );
	}
	else
	{
		// �ҳ����еĹ��캯��
		psSTypeBehaviour *beh = type.GetBehaviour();
		assert(beh);

		std::vector<psSValue> argValues;
		std::vector<int> matches;
		psCByteCode argBC;
		CompileArgumentList( node, &argBC, argValues );

		// ƥ�乹�캯��
		MatchFunctionCall( beh->constructors, matches, argValues, false );

		if( matches.size() != 1)
		{		
			// ƥ�䲻�ɹ�
			psCString msg;
			psCString strType = type.GetFormat() + BuildArgListString(argValues);
			if (matches.size() > 1)
			{	
				msg.Format(TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s, strType.AddressOf() );
			}else
			{
				msg.Format(TXT_NO_MATCHING_SIGNATURES_TO_s, strType.AddressOf() );
			}
			Error( msg.AddressOf(), node );
		}else
		{		
			// ƥ��ɹ�
			psCScriptFunction *desc = m_pBuilder->GetFunctionDescription(matches[0]);
			assert(desc);

			// ��������б�
			bc->AddCode( &argBC );

			int argSize = PushArgumentList( node, desc, bc, argValues );

			// ���ù��캯��
			CallObjectMethod( value, matches[0], argSize, bc );
		}
	}	
}

//-------------------------------------------------------------------
// ����: CompileOverloadedOperator
// ˵��: ����Ϊ���ʾ�ҵ���һ������ƥ��Ĳ�����,Ϊ���ʾû���ҵ�
//-------------------------------------------------------------------
bool psCCompiler::CompileOverloadedOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	bool isObject1 = v1->type.IsObject() && !v1->type.IsPointer() && !v1->type.IsArrayType();
	bool isObject2 = v2->type.IsObject() && !v2->type.IsPointer() && !v2->type.IsArrayType();
	if ( !isObject1 && !isObject2 )
	{
		// ֻ֧����Ĳ���������!!!
		return false;
	}

	// �õ�������������
	int token = node->GetTokenType();

	// ����ȫ�ֵĲ���������
	// NOTE: �ű���֧�ֶ���Ķ�Ԫ���������أ�֧��������ز������У�'-' �� ��ֵ������ ie. '+=', '*=', ....
	std::vector<int> ops;
	m_pBuilder->FindGlobalOperators(token, ops);

	// ���в���ƥ��
	std::vector<int> ops1;
	std::vector<int> ops2;
	MatchArgument(0, *v1, ops, ops1, false);
	MatchArgument(1, *v2, ops, ops2, false);

	// ����ͬ�������������Ĳ�����
	ops.resize(0);
	for(size_t n = 0; n < ops1.size(); n++ )
	{
		for( size_t m = 0; m < ops2.size(); m++ )
		{
			if( ops1[n] == ops2[m] )
			{
				ops.push_back(ops1[n]);
				break;
			}
		}
	}

	if( ops.size() == 1 )
	{
		// �ҵ���һ�����ʵĲ�����
		psCScriptFunction *desc = m_pBuilder->GetFunctionDescription(ops[0]);
		assert(desc);
		psSValue tmp1, tmp2;

		// �����ȫ�ֲ�����,�������÷�ʽΪ(CCALL)
		// ��PUSH�ұ��ʽ,��PUSH����ʽ

		// PUSH�ұ��ʽ, �������Ϊ�����,�Ҳ������ô���,Ҫ��һ����ʱ����
		bool bOk = ImplicitConversion(*v2, desc->m_ParamTypes[1], tmp2, rbc, node, false );
		assert(bOk);

		PushArgument(tmp2, desc->m_ParamTypes[1], rbc, node);
		bc->AddCode(rbc);	

		// PUSH����ʽ,�������Ϊ�����,�Ҳ������ô���,Ҫ��һ����ʱ����
		bOk = ImplicitConversion(*v1, desc->m_ParamTypes[0], tmp1, lbc, node, false );
		assert(bOk);

		PushArgument(tmp1, desc->m_ParamTypes[0], lbc, node);
		bc->AddCode(lbc);

		// ִ�к�������
		PerformFunctionCall( desc->m_ID, desc->GetSpaceNeededForArguments(), v0, bc, node, NULL );

		// �ͷ���ʱ����
		if ( tmp1.operand != v1->operand )
			ReleaseTemporaryVariable( tmp1 );

		if ( tmp2.operand != v2->operand )
			ReleaseTemporaryVariable( tmp2 );
	}
	else if( ops.size() > 1 )
	{
		// �ҵ��˶��ƥ��Ĳ�����
		Error(TXT_MORE_THAN_ONE_MATCHING_OP, node);	
	}else
	{
		// û���ҵ�ƥ��Ĳ�����
		psCString msg;
		msg.Format( TXT_NO_MATCHING_OP_s_FOUND_FOR_TYPE_s, psGetTokenDefinition( token), v1->type.GetFormat().AddressOf() );
		Error( msg.AddressOf(), node );
	}

	// û���ҵ����ʵĲ���
	return true;
}


void psCCompiler::CompileObjectPostOp( int op, psCScriptNode* node, psCByteCode* bc, psSValue* exprValue )
{
	assert( !exprValue->type.IsPointer() );
	assert( exprValue->type.IsObject() );

	// ������֮ƥ��Ĳ�����
	psSTypeBehaviour *beh = exprValue->type.GetBehaviour();
	assert(beh);

	// ����'++/--'������
	psCScriptFunction* desc = NULL;
	size_t n;
	for( n = 0; n < beh->operators.size(); n+= 2 )
	{
		if( op == beh->operators[n] )
		{ 
			psCScriptFunction* func = m_pBuilder->GetFunctionDescription(beh->operators[n+1]);
			if (func && 
				func->m_ParamTypes.size() == 1 &&
				func->m_ParamTypes[0].IsEqualExceptConst( psCDataType(ttInt, false, false) ) )
			{
				desc = func;
				break;
			}
		}
	}

	if( desc )
	{
		psSValue retValue;

		// ������ջ(0)
		bc->Push4( MakeIntValueOp(0) );

		// ִ�к�������
		PerformFunctionCall(desc->m_ID, desc->GetSpaceNeededForArguments(), &retValue, bc, node, exprValue );

		// �ͷ���ʱ����
		ReleaseTemporaryVariable( *exprValue );

		*exprValue = retValue;
	}
	else
	{
		psCString str;
		str.Format(TXT_OBJECT_DOESNT_SUPPORT_POST_INC_OP);
		Error(str.AddressOf(), node);
	}
}


void psCCompiler::CompileObjectPrevOp( int op, psCScriptNode* node, psCByteCode* bc, psSValue* exprValue )
{
	assert( !exprValue->type.IsPointer() );

	// ������֮ƥ��Ĳ�����
	psSTypeBehaviour *beh = exprValue->type.GetBehaviour();

	// ���Ҳ�����
	psCScriptFunction* desc = NULL;
	size_t n;
	for( n = 0; n < beh->operators.size(); n+= 2 )
	{
		if( op == beh->operators[n] )
		{ 
			psCScriptFunction* func = m_pBuilder->GetFunctionDescription(beh->operators[n+1]);
			if ( func && func->m_ParamTypes.size() == 0 )
			{
				desc = func;
				break;
			}
		}
	}

	if( desc )
	{
		psSValue retValue;
		// ִ�к�������
		PerformFunctionCall( desc->m_ID, desc->GetSpaceNeededForArguments(), &retValue, bc, node, exprValue );

		// �ͷ���ʱ����
		ReleaseTemporaryVariable( *exprValue );

		*exprValue = retValue;
	}
	else
	{
		psCString str;
		str.Format( TXT_OBJECT_DOESNT_SUPPORT_PRE_OP_s, psGetTokenDefinition(op) );
		Error(str.AddressOf(), node);
	}
}


void psCCompiler::CompilePostOpDot( psCScriptNode* node, psCByteCode* bc, psSValue* exprValue )
{
	assert( exprValue->type.IsObject() );

	psCScriptNode* first = node->GetFirstChild();

	// ֻ�������ŶԴ˲�����Ч
	if( first->GetNodeType() == snIdentifier )
	{
		// �õ�������
		psCString name;
		name.Copy(&m_pScript->m_Code[first->GetTokenPos()], first->GetTokenLength());

		// �������Ա����
		if( exprValue->type.IsObject()  )
		{
			int baseOffset = 0;
			psSProperty *prop = m_pBuilder->GetObjectProperty( exprValue->type.ObjectType(), name.AddressOf(), baseOffset );
			if( prop )
			{
				int offset = prop->byteOffset + baseOffset;
				if ( exprValue->type.IsReference()  )
				{		
					psSValue tmp(psCDataType(ttInt, false, false));
					AllocateTemporaryVariable(tmp);

					// ����������ƫ��
					if ( offset != 0 )
					{
						bc->Instr3(BC_ADDi, tmp.operand, exprValue->operand, MakeIntValueOp(offset) );
					}else
					{
						bc->Instr2(BC_MOV4, tmp.operand, exprValue->operand );
					}

					// �ͷ���ʱ����
					ReleaseTemporaryVariable( *exprValue );

					// ���ò�����
					exprValue->operand			  = tmp.operand;
					exprValue->isTemporary		  = true;
				}else
				{
					// �ͷ���ʱ����
					ReleaseTemporaryVariable( *exprValue );

					// ����������������,���Ա���Եĵ�ַ�����ڱ���ʱ��֪��
					assert( exprValue->operand.base == PSBASE_STACK ||
						exprValue->operand.base == PSBASE_GLOBAL );
					exprValue->operand.offset += offset;

					// �������������������, ��ô��ֵҲӦ������������
					if ( prop->type.IsReference() )
						exprValue->type.IsReference() = true;
				}

				// �����µ��������ͣ�������ֻ������
				bool isReadOnly				  = exprValue->type.IsReadOnly();
				bool isReference			  = exprValue->type.IsReference();

				exprValue->type				  = prop->type;
				exprValue->type.IsReadOnly()  = isReadOnly;
				exprValue->type.IsReference() = isReference;
				exprValue->isInitialized = true;
			}
			else
			{
				psCString str;
				str.Format(TXT_s_NOT_MEMBER_OF_s, name.AddressOf(), exprValue->type.GetFormat().AddressOf() );
				Error(str.AddressOf(), node);
			}
		}
		else
		{
			psCString str;
			str.Format(TXT_s_NOT_MEMBER_OF_s, name.AddressOf(), exprValue->type.GetFormat().AddressOf() );
			Error(str.AddressOf(), node);
		}
	}else
	{
		// ���Ա��������
		if( !exprValue->type.IsObject() )
		{	
			// ���������
			psCString str;
			str.Format(TXT_ILLEGAL_OPERATION_ON_s, exprValue->type.GetFormat().AddressOf() );
			Error(str.AddressOf(), node); 
		}
		else
		{
			psSValue value;

			// ���뺯������
			CompileFunctionCall(first, bc, &value, exprValue );

			// �ͷ���ʱ����
			ReleaseTemporaryVariable( *exprValue );

			// ����������ֵ
			*exprValue = value;
		}
	}
}

void psCCompiler::CompilePostOpOpenBracket(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue)
{
	psCScriptNode* first = node->GetFirstChild();
	if ( exprValue->type.IsArrayType() || exprValue->type.IsPointer() )
	{	
		// ����������

		// ��������ֵ
		psSValue argIndex;
		CompileAssignment(first, bc, &argIndex);

		// ��������ֵ����������
		if (!argIndex.type.IsIntegerType() || argIndex.type.IsPointer() )
		{
			Error(TXT_ARRAY_INDEX_MUSTBE_INTEGER, node);
			return;
		}	
		
		psSValue idxValue( g_IntDataType );
		if ( argIndex.type.IsReference() )
		{
			// ���������õ�ֵ
			AllocateTemporaryVariable( idxValue );
			bc->Instr2( BC_RDREF4, idxValue.operand, argIndex.operand );
			ReleaseTemporaryVariable( argIndex );
		}else
		{
			idxValue = argIndex;
		}

		// ����һ�������С
		exprValue->type.PopArraySize();

		// NOTE: ���ݴ洢�������д洢��
		int subsize = exprValue->type.GetSizeInMemoryBytes() * exprValue->type.GetNumArrayElements();

		if ( !idxValue.IsConstant() || exprValue->type.IsReference() )
		{
			// ������ʱ���� 
			psSValue tmp( psCDataType(ttInt, false, false) ) ;
			AllocateTemporaryVariable( tmp );

			// �����ַ
			psSOperand subsizeOp(PSBASE_CONST, m_pBuilder->RegisterConstantInt( subsize ) );

			bc->Instr3(BC_MULi, tmp.operand, idxValue.operand, subsizeOp );
			if ( !exprValue->type.IsReference() )
			{
				psSValue tmpBase( psCDataType(ttInt, false, false) ) ;
				AllocateTemporaryVariable( tmpBase );

				bc->Instr2(BC_DEFREF, tmpBase.operand, exprValue->operand );			
				bc->Instr3(BC_ADDi,   tmp.operand, tmpBase.operand, tmp.operand);

				ReleaseTemporaryVariable( tmpBase );
			}else
			{
				bc->Instr3(BC_ADDi, tmp.operand, exprValue->operand, tmp.operand);
			}

			// �ͷ���ʱ����
			ReleaseTemporaryVariable( *exprValue );
			
			// ����ֵ������
			exprValue->operand			   = tmp.operand;
			exprValue->isTemporary		   = true;
			exprValue->type.IsReference()  = true;	
			exprValue->type.IsPointer()	   = false;	 
		}else
		{
			// ����������õ�ַ, ��������ֵ�ǳ���, ��ƫ�ƿ����ڱ����ھ�֪��
			assert( exprValue->operand.base == PSBASE_STACK ||
				exprValue->operand.base == PSBASE_GLOBAL );

			exprValue->isTemporary		  = false;
			exprValue->operand.offset    += subsize * GetConstantInt( idxValue.operand );
		}

		// �ͷ���ʱ����
		ReleaseTemporaryVariable( idxValue );
	}else if ( exprValue->type.IsObject() )
	{
		// ��������������

		// ��������ֵ
		psSValue idxValue;
		CompileAssignment(first, bc, &idxValue);

		// ������֮ƥ��Ĳ�����
		psSTypeBehaviour *beh = exprValue->type.GetBehaviour();
		assert(beh);

		// ����'[]'������
		std::vector< psCScriptFunction*  > funcs;
		size_t n;
		for( n = 0; n < beh->operators.size(); n+= 2 )
		{
			if( ttOpenBracket == beh->operators[n] )
			{ 
				psCScriptFunction* func = m_pBuilder->GetFunctionDescription(beh->operators[n+1]);
				if (func && func->m_ParamTypes.size() == 1)
				{
					funcs.push_back( func );
				}
			}
		}

		// ƥ�亯������
		int matchedFuncCount = 0;
		psCScriptFunction* func = NULL;
		for (n = 0; n < funcs.size(); ++n )
		{
			psCScriptFunction* desc = funcs[n];
			bool bOk = ImplicitConversion( idxValue, desc->m_ParamTypes[0], idxValue, bc, node, false);
			if ( bOk )
			{
				func = desc;
				++matchedFuncCount;
			}
		}

		if( matchedFuncCount == 1 )
		{
			assert(func);
			// ��������ջ
			PushArgument( idxValue, func->m_ParamTypes[0], bc, node);

			psSValue retValue;

			// ִ�к�������
			PerformFunctionCall(func->m_ID, func->GetSpaceNeededForArguments(), &retValue, bc, node, exprValue );

			// �ͷ���ʱ����
			ReleaseTemporaryVariable(idxValue);
			ReleaseTemporaryVariable( *exprValue );

			*exprValue = retValue;
		}
		else if ( matchedFuncCount == 0 )
		{
			psCString str;
			str.Format(TXT_OBJECT_DOESNT_SUPPORT_INDEX_OP);
			Error(str.AddressOf(), node);
		}else
		{
			assert(func);
			psCString str;
			str.Format(TXT_MULTIPLE_MATCHING_SIGNATURES_TO_s, func->GetDeclaration().AddressOf() );
			Error(str.AddressOf(), node );
		}
	}else
	{
		Error(TXT_ILLEGAL_OPERATION, node);
	}
}


//-------------------------------------------------------------------
// ����: CopyConstructObject
// ˵��: 
//-------------------------------------------------------------------
bool psCCompiler::CopyConstructObject(const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node)
{
	if ( lvalue.type.ObjectType() == NULL )
	{
		assert(false);
		return false;
	}

	psCDataType type( lvalue.type );
	type.IsPointer() = true;
	type.IsReadOnly()  = true;
	if ( !rvalue.type.IsEqualExceptRefAndConst(type) )
	{
		psCString str;
		str.Format(TXT_CANT_IMPLICITLY_CONVERT_s_TO_s,  rvalue.type.GetFormat().AddressOf(), type.GetFormat().AddressOf() );
		Error(str.AddressOf(), node);
		return false;
	}	

	psCObjectType* objType = lvalue.type.ObjectType();

	// ����ǽӿ������,����FALSE
	if ( objType->m_Beh.isInterface )
	{
		psCString msg;
		msg.Format(TXT_OBJECT_s_CANNOT_BE_INSTANCE, lvalue.type.GetFormat().AddressOf() );
		Error(msg.AddressOf(), node);
		return false;
	}

	// ������Ŀ������캯��, ����еĻ�.
	// ���û����򵥵Ľ���Memcpy
	if ( objType->m_Beh.hasCopyConstructor  )
	{		
		// push argument
		if ( rvalue.type.IsReference())
			bc->Push4( rvalue.operand );
		else
			bc->PushRef( rvalue.operand );

		// Call copy constructor
		CallObjectMethod( lvalue, objType->m_Beh.copy, 1, bc );

	}else
	{
		// �򵥵��ڴ濽��
		psSOperand lvalueOp = lvalue.operand;
		psSOperand rvalueOp = rvalue.operand;

		if ( !lvalue.type.IsReference() )
		{
			lvalueOp.BaseOffset( PSBASE_STACK, AllocateVariable(g_IntDataType, true) );
			bc->Instr2(BC_DEFREF, lvalueOp, lvalue.operand );
		}
		if ( !rvalue.type.IsReference() )
		{
			rvalueOp.BaseOffset( PSBASE_STACK, AllocateVariable(g_IntDataType, true) );
			bc->Instr2(BC_DEFREF, rvalueOp, rvalue.operand );
		}

		// Memcpy
		bc->Instr3( BC_MEMCPY, lvalueOp, rvalueOp, objType->GetSizeInMemoryBytes() );

		if ( lvalueOp != lvalue.operand )
		{
			ReleaseTemporaryOperand( lvalueOp );
		}
		if ( rvalueOp != rvalue.operand )
		{
			ReleaseTemporaryOperand( rvalueOp );
		}
	}

	return true;
}

//-------------------------------------------------------------------
// ����: ObjectAssignmentEx
// ˵��: 
//-------------------------------------------------------------------
void psCCompiler::ObjectAssignmentEx(int assignOp, psSValue* retValue, const psSValue& lvalue, const psSValue& rvalue,  psCByteCode* bc, psCScriptNode* node )
{
	// ������ұ��ʽ������
	if (!IsLValue(lvalue))
	{
		Error( TXT_ASSIGN_LEFT_MUSTBE_LVALUE, node );	
		return;
	}
	assert( !lvalue.type.IsPointer() );

	// �������ֵ���ʽ��һ���࣬���������Ƿ��Ѿ�������
	// ���û���Ǿ���һ���򵥵ĸ�ֵ����
	std::vector<int> matches;
	std::vector<int> exactmatches;
	std::vector<int> ops;
	int baseOffset = 0;

	psCObjectType *objType = lvalue.type.ObjectType();
	while ( objType ) 
	{
		psSTypeBehaviour& beh = objType->m_Beh;

		size_t n;
		for( n = 0; n < beh.operators.size(); n += 2 )
		{
			if( assignOp == beh.operators[n] )
			{
				ops.push_back(beh.operators[n+1]);
			}
		}
		// ��ȷƥ�����
		MatchArgument(0, rvalue, ops, exactmatches, true);

		if ( exactmatches.size() != 0 || rvalue.type.ObjectType() == objType )
			break;

		// ��ȫƥ�����
		if ( matches.size() == 0 )
		{
			MatchArgument( 0, rvalue, ops, matches, false );
		}

		baseOffset = objType->m_BaseOffset;
		objType = objType->m_pParent;
		ops.clear();
	}

	int funcId = -1;
	bool bFind = false;
	if ( exactmatches.size() == 1 )
	{
		funcId = exactmatches[0];
		bFind  = true;
	}else if ( exactmatches.size() == 0 && matches.size() == 1 )
	{
		funcId = matches[0];
		bFind  = true;
	}
	if( bFind )
	{
		psCScriptFunction *desc = m_pBuilder->GetFunctionDescription(funcId);
		assert(desc); 

		// ������ջǰ, ����һ����ʽת��
		// ���ת���϶��ǳɹ��ģ���Ϊƥ������Ѿ�ͨ����
		psSValue tmp;
		bool bOk = ImplicitConversion(rvalue, desc->m_ParamTypes[0], tmp, bc, node, false );
		assert( bOk );

		// ��������ջ
		PushArgument(tmp, desc->m_ParamTypes[0], bc, node);

		psSValue obj;
		obj.type.SetObjectType( desc->m_ObjectType );
		obj.type.IsReference() = lvalue.type.IsReference();
		obj.type.IsPointer()   = lvalue.type.IsPointer();
		obj.operand = lvalue.operand;
		if ( baseOffset != 0 )
		{
			obj.type.IsReference() = true;
			obj.type.IsPointer()   = true;
			AllocateTemporaryVariable( obj );
			if ( lvalue.type.IsReference() )
				bc->Instr2( BC_MOV4, obj.operand, lvalue.operand );
			else
				bc->Instr2( BC_DEFREF, obj.operand, lvalue.operand );
			bc->Instr2( BC_ADDi, obj.operand, MakeIntValueOp(baseOffset) );
		}
		// ִ�к�������
		PerformFunctionCall( funcId, desc->GetSpaceNeededForArguments(), retValue, bc, node, &obj );

		// �ͷ���ʱ����
		if (tmp.operand != rvalue.operand)
		{
			ReleaseTemporaryVariable( tmp );
		}
		if ( baseOffset != 0 )
		{
			ReleaseTemporaryVariable( obj );
		}
		return;
	}else if ( matches.size() > 1 )
	{	
		Error(TXT_MORE_THAN_ONE_MATCHING_OP, node);		
		return;
	}

	if ( rvalue.type.IsPointer() || 
		rvalue.type.ObjectType()  != lvalue.type.ObjectType() )
	{
		psCString msg;
		msg.Format( TXT_CANT_CONVERT_s_TO_s, rvalue.type.GetFormat().AddressOf(), lvalue.type.GetFormat().AddressOf() );
		Error( msg.AddressOf(), node );
	}else
	{	
		// ��������
		CopyConstructObject( lvalue, rvalue, bc, node );
	}
}

void psCCompiler::CallObjectMethod(const psSValue& value, const psSOperand& funcId, int argSize, psCByteCode* bc)
{
	psCObjectType* objType = value.type.ObjectType();
	assert(objType && bc );

	// push this pointer
	if ( value.type.IsReference() )
	{
		bc->Push4( value.operand );
	}else
	{
		bc->PushRef( value.operand );
	}

	if ( objType->m_bScriptObject )
	{
		bc->Call( funcId, argSize, true );
	}else
	{
		bc->CallSys( funcId, argSize, true );
	}
}

void psCCompiler::CompileSizeof(psCScriptNode* node, psCByteCode* bc,  psSValue* exprValue )
{
	psCScriptNode* argNode = node->GetFirstChild();
	assert( argNode );

	psCDataType type;
	if ( argNode->GetNodeType() == snDataType )
	{
		type = m_pBuilder->CreateDataTypeFromNode( argNode, m_pScript );
		if ( argNode->GetNextNode() )
		{
			m_pBuilder->ModifyDataTypeFromNode( type, argNode->GetNextNode(), m_pScript );

			// 
			CheckDataType( type, node );
		}
	}else if ( argNode->GetNodeType() == snAssignment )
	{
		psCString name;
		name.Copy(&m_pScript->m_Code[argNode->GetTokenPos()], argNode->GetTokenLength() );

		type.SetObjectType( m_pBuilder->GetObjectType(name.AddressOf()) );
		if ( type.GetObjectType() == NULL )
		{
			psSValue argValue;
			CompileAssignment( argNode, bc, &argValue );

			// �ͷ���ʱ����
			ReleaseTemporaryVariable( argValue );

			type = argValue.type;
		}
	}

	int s = sizeof(psDWORD);
	if ( type.IsReference() && !type.IsArrayType() )
		s = sizeof(psDWORD);
	else
		s = type.GetSizeInMemoryBytes() * type.GetNumArrayElements();

	exprValue->type    = psCDataType(ttInt, true, false );
	exprValue->operand = MakeIntValueOp( s );
}

void psCCompiler::CompileTypeCast( psCScriptNode* node, psCByteCode* bc,  psSValue* exprValue )
{
	psCScriptNode* typeNode = node->GetFirstChild();
	psCScriptNode* argNode  = node->GetLastChild();

	// �õ���������
	psCDataType type = m_pBuilder->CreateDataTypeFromNode( typeNode, m_pScript );
	m_pBuilder->ModifyDataTypeFromNode( type, typeNode->GetNextNode(), m_pScript );

	// Compile assignment 
	psSValue argValue;
	CompileAssignment( argNode, bc, &argValue );

	// �������Ƿ��Ѿ���ʼ��
	IsVariableInitialized(argValue, node);

	// ��ʼ��
	exprValue->type = type;

	bool bOk = false;
	if ( type.IsPointer() )
	{
		if ( type.IsEqualExceptConst(argValue.type) )
		{
			// ���ֻ������
			if ( type.IsReadOnly() || !argValue.type.IsReadOnly() )
			{
				if ( !argValue.type.IsReference() )
				{	
					// ������ֵַ
					AllocateTemporaryVariable( *exprValue );
					bc->Instr2( BC_DEFREF, exprValue->operand, argValue.operand );
				}else
				{
					exprValue->AssignExceptType( argValue );
				}
				bOk = true;
			}
		}
		else 
		{		
			if ( argValue.type.IsArrayType() )
			{	
				AllocateTemporaryVariable( *exprValue );
				if ( argValue.type.IsReference() )
				{
					bc->Instr2( BC_MOV4, exprValue->operand, argValue.operand );
				}else
				{
					bc->Instr2( BC_DEFREF, exprValue->operand, argValue.operand );
				}
				bOk = true;
			}else if ( argValue.type.IsPointer() )
			{
				psCObjectType* objType    = type.ObjectType();
				psCObjectType* argObjType = argValue.type.ObjectType();
				int baseOffset = 0;
				if ( objType && argObjType )
				{
					// �Ƿ�Ϊ���ൽ�����ת��?
					while ( objType )
					{
						if ( objType == argObjType )
							break;
						baseOffset -= objType->m_BaseOffset;
						objType	    = objType->m_pParent;
					}
					if ( objType == NULL )
					{
						// �Ƿ�Ϊ���ൽ�����ת��?
						baseOffset = 0;
						objType    = type.ObjectType();
						while ( argObjType )
						{
							if ( objType == argObjType )
								break;
							baseOffset += argObjType->m_BaseOffset;
							argObjType  = argObjType->m_pParent;
						}
						if ( argObjType == NULL )
							baseOffset = 0;
					}
				}
				if ( baseOffset != 0)
				{
					AllocateTemporaryVariable( *exprValue );
					bc->Instr3( BC_ADDi, exprValue->operand, argValue.operand, MakeIntValueOp(baseOffset) );
				}else
				{	
					exprValue->AssignExceptType( argValue );
				}
				bOk = true;
			}else
			{
				if (argValue.type.IsIntegerType() || argValue.type.IsBooleanType())
				{
					if ( argValue.type.IsReference() )
					{			
						// ������ֵַ
						AllocateTemporaryVariable( *exprValue );
						bc->Instr2( BC_RDREF4, exprValue->operand, argValue.operand );
					}else
					{
						exprValue->AssignExceptType( argValue );
					}
					bOk = true;
				}
			}
		}
	}else
	{
		psSValue tmp;
		// ������ʽת��
		if ( ImplicitConversion( argValue, type, tmp, bc, node, true ) )
		{
			if ( !tmp.type.IsReference() && type.IsReference() )
			{	
				// ������ֵַ
				AllocateTemporaryVariable( *exprValue );
				bc->Instr2( BC_DEFREF, exprValue->operand, tmp.operand );
				ReleaseTemporaryVariable( tmp );	
			}else if ( tmp.type.IsReference() && !type.IsReference() )
			{
				exprValue->type.IsReference() = true;
				exprValue->AssignExceptType( tmp );	
			}else
			{
				exprValue->AssignExceptType( tmp );	
			}
			bOk = true;
		}else
		{
			ReleaseTemporaryVariable(tmp);
			if ( type.IsIntegerType() )
			{
				if ( argValue.type.IsPointer() )
				{
					exprValue->AssignExceptType( argValue );	
					bOk		   = true;
				}else
				{
					if ( argValue.type.IsBooleanType() )
					{
						exprValue->AssignExceptType( argValue );
						bOk = true;
					}
				}
			}else if ( type.IsBooleanType() )
			{
				if ( argValue.type.IsPointer() )
				{
					exprValue->AssignExceptType( argValue );	
					bOk		   = true;
				}else
				{
					if ( argValue.type.IsIntegerType() )
					{
						exprValue->AssignExceptType( argValue );
						bOk = true;
					}
				}
			}
		}
	}

	if ( !bOk )
	{
		psCString msg;
		msg.Format( TXT_CANT_CONVERT_s_TO_s, argValue.type.GetFormat().AddressOf(), type.GetFormat().AddressOf() );
		Error( msg.AddressOf(), node );
	}
	if ( argValue.operand != exprValue->operand )
	{
		// �ͷ���ʱ����
		ReleaseTemporaryVariable( argValue );
	}
}

void psCCompiler::CompileEnum( psCScriptNode* node, psCObjectType* objType, const psCHAR* field )
{
	assert( node->GetNodeType() == snEnum );
	
	psCHAR  name[256];
	psCHAR  fieldName[25];
	psCScriptNode* n = node->GetFirstChild();
	if ( n->GetNodeType() == snIdentifier )
	{
		if ( objType )
		{
			psCString msg;
			msg.Format( TXT_ENUM_ALREADY_HAS_FIELD_s, objType->m_Name.AddressOf() );
			Error( msg.AddressOf(), node );
		}else
		{
			GetNodeSource( fieldName, 255, n );
			field = fieldName;
		}
		n = n->GetNextNode();
	}

	int hint = 0;
	while ( n )
	{
		int intValue = CompileEnumConstant( n, name, hint );

		bool hasError = false; 
		if ( objType || field )
		{
			if ( !m_pBuilder->DeclareConstantEnum( field, name, intValue ) )
			{
				hasError = true;
			}
			else
			{
				if ( objType )
				{
					hasError = objType->FindProperty( name ) != NULL;
				}else
				{	
					hasError = m_pBuilder->GetGlobalProperty( name ) != NULL;
				}
			}
		}else
		{
			assert( m_Variables );
			psSValue value( g_IntDataType );
			value.operand = MakeIntValueOp( intValue );
			if ( m_Variables->DeclareVariable( name, value ) == NULL )
			{
				hasError = true;
			}
		}

		if ( hasError )
		{
			psCString msg;
			msg.Format(TXT_PROPERTY_s_ALREADY_EXIST, name);
			Error( msg.AddressOf(), n->GetFirstChild() );
		}
		n = n->GetNextNode();	
		hint = intValue+1;
	}
}

int psCCompiler::CompileEnumConstant( psCScriptNode* node, psCHAR* name, int hint )
{
	assert( node->GetNodeType() == snEnumConstant );

	psCScriptNode* n = node->GetFirstChild();
	GetNodeSource( name, 255, n );

	n = n->GetNextNode();
	if ( n )
	{
		if(  n->GetTokenType() != ttIntConstant && 
			 n->GetTokenType() != ttBitsConstant )
		{
			Error(PS_T("expected constant expression "), n );
		}else
		{
			psCHAR buf[65];
			GetNodeSource( buf, 64, n );

			int v = ps_strtol(buf, NULL, 0);
			if ( node->GetTokenType() == ttMinus )
				v = -v;
			return v;
		}
	}
	return hint;
}




