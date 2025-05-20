#include "compiler.hpp"

//-------------------------------------------------------------------
// 名称: DefaultConstructor
// 说明: 
//-------------------------------------------------------------------
bool psCCompiler::DefaultConstructor(const psSValue& value, psCByteCode *bc)
{
	psSTypeBehaviour *beh = value.type.GetBehaviour();
	assert(beh != 0 );

	bool scriptObject = value.type.ObjectType()->m_bScriptObject;
	// 如果是接口类对象,返回FALSE
	if ( beh->isInterface )
		return false;

	// 调用类对象的构造函数, 如果有的话
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
// 名称: CompileConstructor
// 说明: 
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
// 名称: CompileDestructor
// 说明: 
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
// 名称: CompileClassDecl
// 说明: 编译类声明
//-------------------------------------------------------------------
int psCCompiler::CompileClassDecl(psCBuilder* builder, psCScriptCode* script, psCScriptNode* node, std::vector<psCScriptNode*>& methods, psCObjectType* objectType)
{
	assert( objectType );

	// 重置编译器
	Reset( builder, script, objectType );

	int byteoffset = 0;

	psCScriptNode* n = node->GetFirstChild();

	// 得到类名
	psCHAR name[256];
	GetNodeSource( name, 255, n );

	// 检查名称冲突
	if ( m_pBuilder->CheckNameConflictObject(name, node, script) < 0 )
	{
		return -1;
	}

	// 设置对象的名称
	objectType->m_Name = name;
	objectType->m_DataType.TokenType() = ttIdentifier;

	methods.clear();
	n = n->GetNextNode();
	if ( n && n->GetNodeType() == snIdentifier )
	{
		// 这个类有继承
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
	// 编译所有的对象属性
	while (n)
	{
		if ( n->GetNodeType() == snPropertyDecl )
		{
			// 编译类对象的属性结点
			CompilePropertyDecl(n, objectType, byteoffset, &bc);
		}else if ( n->GetNodeType() == snEnum )
		{
			// 编译类的枚举常量
			CompileEnum( n, objectType, objectType->m_Name.AddressOf() );
		}else
		{
			// 将类对象的成员函数,或操作符重载结点保存起来
			methods.push_back(n);
		}
		n = n->GetNextNode();
	}

	// 设置对象的大小
	if ( byteoffset == 0 )
		objectType->m_Size = sizeof(psDWORD);
	else
		objectType->m_Size = byteoffset;

	// 此类象为脚本类对象
	objectType->m_bScriptObject = true;

	// 添加二进制代码
	m_ByteCode.AddCode( &bc );

	return 0;
}

//-------------------------------------------------------------------
// 名称: CompilePropertyDecl
// 说明: 编译类成员属性声明
//-------------------------------------------------------------------
int psCCompiler::CompilePropertyDecl(psCScriptNode* decl, psCObjectType* objectType, int& byteoffset, psCByteCode* bc)
{
	// 得到数据类型结点
	psCScriptNode* node = decl->GetFirstChild();

	// 得到数据类型
	psCDataType raw_type = m_pBuilder->CreateDataTypeFromNode(node, m_pScript);

	// 检查类型的合法性
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
	// 声明所有的属性,可能有多个(例如: int a, b, c;)
	while (node)
	{
		type = raw_type;

		// 修饰数据类型(是否为引用)
		m_pBuilder->ModifyDataTypeFromNode( type, node, m_pScript );

		// 得到名称结点
		node = node->GetNextNode();

		// 编译变量定义
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

		// 如果友成员属性为类对象并且不是引用传递,检查类是否能被实例化
		if ( ( type.IsObject() )  )
		{
			if ( type.ObjectType()->m_Beh.isInterface )
			{
				psCString str;
				str.Format(TXT_OBJECT_s_CANNOT_BE_INSTANCE, type.ObjectType()->m_Name.AddressOf() );
				Error(str.AddressOf(), decl);
			}
		}

		// 检查属性是否已经存在
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
		// 检查是否与枚举常量冲突
		int tmpValue;
		if (m_pBuilder->GetConstantEnum(objectType->m_Name.AddressOf(), name.AddressOf(), tmpValue) )
		{
			nameIsConflict = true;
		}

		if ( nameIsConflict )
		{	
			// 名称为'name'的属性已经存在
			psCString msg;
			msg.Format(TXT_PROPERTY_s_ALREADY_EXIST, name.AddressOf());

			Error(msg.AddressOf(), decl);
		}else
		{
			// 这是一个新的属性成员
			psSProperty* prop = new psSProperty();
			prop->byteOffset = byteoffset;
			prop->name = name;
			prop->type = type;
			prop->base = PSBASE_HEAP;
			objectType->m_Properties.push_back(prop);
		}

		// 更新字节编移
		if ( type.IsReference() )
			byteoffset += sizeof(psDWORD);
		else
			byteoffset += type.GetSizeInMemoryBytes();

		// 移到下一个结点
		node = node->GetNextNode();
	}

	return 0;
}

//-------------------------------------------------------------------
// 名称: DefaultClassConstructor
// 说明: 生成类成员属性的默认构造函数
//-------------------------------------------------------------------
void psCCompiler::DefaultObjectConstructor(psCObjectType* objectType, psCByteCode* bc)
{	
	// 调用父类的构造函数
	if ( objectType->m_pParent )
	{
		psSValue thisPointer;
		thisPointer.operand.BaseOffset(PSBASE_STACK, 0 );
		thisPointer.type.SetObjectType( objectType->m_pParent );
		thisPointer.type.IsReference() = true;

		assert( objectType->m_pParent->m_Beh.hasConstructor );
		CallObjectMethod( thisPointer, objectType->m_pParent->m_Beh.construct, 0, bc );
	}

	// 调用成员的构造函数
	for (size_t i = 0; i < objectType->m_Properties.size(); ++i)
	{
		psSProperty* prop = objectType->m_Properties[i];
		psCObjectType* propObjType = prop->type.ObjectType();
		if (propObjType)
		{
			// 得到成员变量
			psSValue dst;
			dst.type = prop->type;
			dst.operand.BaseOffset( PSBASE_HEAP, prop->byteOffset );

			// 如果是引用初始化为0
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

	// 调用成员的构造函数
	for (size_t i = 0; i < objectType->m_Properties.size(); ++i)
	{
		psSProperty*   prop = objectType->m_Properties[i];
		psCObjectType* propObjType = prop->type.ObjectType();

		if (propObjType && propObjType->m_Beh.hasCopyConstructor && !prop->type.IsReference() )
		{
			psSValue src, dst;
			dst.type = prop->type;
			dst.operand.BaseOffset( PSBASE_HEAP, prop->byteOffset );

			// 计算成员属性的地址值
			src.type = prop->type;
			src.type.IsReference() = true;
			AllocateTemporaryVariable(src);

			psSOperand offsetOp = MakeIntValueOp(prop->byteOffset);
			copyBC.Instr3(BC_ADDi, src.operand, psSOperand(PSBASE_STACK, 4), offsetOp );

			// Push argument
			copyBC.Push4( src.operand );

			// Call copy constructor
			CallObjectMethod( dst, propObjType->m_Beh.copy, 1, &copyBC );

			// 释放临时变量
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
		// 调用父类的构造函数
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
// 名称: DefaultObjectDestructor
// 说明: 生成类成员属性的默认析构函数
//-------------------------------------------------------------------
// 生成类成员属性的默认构构函数
void psCCompiler::DefaultObjectDestructor(psCObjectType* objectType, psCByteCode* bc)
{
	// 调用父类的析构函数
	if ( objectType->m_pParent && objectType->m_Beh.hasDestructor )
	{
		psSValue thisPointer;
		thisPointer.operand.BaseOffset(PSBASE_STACK, 0 );
		thisPointer.type.SetObjectType( objectType->m_pParent );
		thisPointer.type.IsReference() = true;

		CallObjectMethod( thisPointer, objectType->m_pParent->m_Beh.construct, 0, bc );
	}

	// 调用成员的析构函数
	for (size_t i = 0; i < objectType->m_Properties.size(); ++i)
	{
		psSProperty* prop = objectType->m_Properties[i];
		psCObjectType* propObjType = prop->type.ObjectType();
		if (propObjType && propObjType->m_Beh.hasDestructor && !prop->type.IsReference() )
		{
			// 得到成员变量
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
	// 类对象初始化表达式
	if( !type.IsObject() )
	{
		Error(TXT_MUST_BE_OBJECT, node );
	}
	else
	{
		// 找出所有的构造函数
		psSTypeBehaviour *beh = type.GetBehaviour();
		assert(beh);

		std::vector<psSValue> argValues;
		std::vector<int> matches;
		psCByteCode argBC;
		CompileArgumentList( node, &argBC, argValues );

		// 匹配构造函数
		MatchFunctionCall( beh->constructors, matches, argValues, false );

		if( matches.size() != 1)
		{		
			// 匹配不成功
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
			// 匹配成功
			psCScriptFunction *desc = m_pBuilder->GetFunctionDescription(matches[0]);
			assert(desc);

			// 编译参数列表
			bc->AddCode( &argBC );

			int argSize = PushArgumentList( node, desc, bc, argValues );

			// 调用构造函数
			CallObjectMethod( value, matches[0], argSize, bc );
		}
	}	
}

//-------------------------------------------------------------------
// 名称: CompileOverloadedOperator
// 说明: 返回为真表示找到了一个或多个匹配的操作符,为否表示没有找到
//-------------------------------------------------------------------
bool psCCompiler::CompileOverloadedOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 )
{
	bool isObject1 = v1->type.IsObject() && !v1->type.IsPointer() && !v1->type.IsArrayType();
	bool isObject2 = v2->type.IsObject() && !v2->type.IsPointer() && !v2->type.IsArrayType();
	if ( !isObject1 && !isObject2 )
	{
		// 只支持类的操作符重载!!!
		return false;
	}

	// 得到操作符的类型
	int token = node->GetTokenType();

	// 查找全局的操作符重载
	// NOTE: 脚本不支持对类的二元操作符重载，支持类的重载操作符有：'-' 和 赋值操作符 ie. '+=', '*=', ....
	std::vector<int> ops;
	m_pBuilder->FindGlobalOperators(token, ops);

	// 进行参数匹配
	std::vector<int> ops1;
	std::vector<int> ops2;
	MatchArgument(0, *v1, ops, ops1, false);
	MatchArgument(1, *v2, ops, ops2, false);

	// 查找同符合两个参数的操作符
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
		// 找到了一个合适的操作符
		psCScriptFunction *desc = m_pBuilder->GetFunctionDescription(ops[0]);
		assert(desc);
		psSValue tmp1, tmp2;

		// 如果果全局操作符,则函数调用方式为(CCALL)
		// 先PUSH右表达式,后PUSH左表达式

		// PUSH右表达式, 如果参数为类对象,且不是引用传递,要生一个临时对象
		bool bOk = ImplicitConversion(*v2, desc->m_ParamTypes[1], tmp2, rbc, node, false );
		assert(bOk);

		PushArgument(tmp2, desc->m_ParamTypes[1], rbc, node);
		bc->AddCode(rbc);	

		// PUSH左表达式,如果参数为类对象,且不是引用传递,要生一个临时对象
		bOk = ImplicitConversion(*v1, desc->m_ParamTypes[0], tmp1, lbc, node, false );
		assert(bOk);

		PushArgument(tmp1, desc->m_ParamTypes[0], lbc, node);
		bc->AddCode(lbc);

		// 执行函数调用
		PerformFunctionCall( desc->m_ID, desc->GetSpaceNeededForArguments(), v0, bc, node, NULL );

		// 释放临时变量
		if ( tmp1.operand != v1->operand )
			ReleaseTemporaryVariable( tmp1 );

		if ( tmp2.operand != v2->operand )
			ReleaseTemporaryVariable( tmp2 );
	}
	else if( ops.size() > 1 )
	{
		// 找到了多个匹配的操作符
		Error(TXT_MORE_THAN_ONE_MATCHING_OP, node);	
	}else
	{
		// 没有找到匹配的操作符
		psCString msg;
		msg.Format( TXT_NO_MATCHING_OP_s_FOUND_FOR_TYPE_s, psGetTokenDefinition( token), v1->type.GetFormat().AddressOf() );
		Error( msg.AddressOf(), node );
	}

	// 没有找到合适的操作
	return true;
}


void psCCompiler::CompileObjectPostOp( int op, psCScriptNode* node, psCByteCode* bc, psSValue* exprValue )
{
	assert( !exprValue->type.IsPointer() );
	assert( exprValue->type.IsObject() );

	// 查找与之匹配的操作符
	psSTypeBehaviour *beh = exprValue->type.GetBehaviour();
	assert(beh);

	// 查找'++/--'操作符
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

		// 常数入栈(0)
		bc->Push4( MakeIntValueOp(0) );

		// 执行函数调用
		PerformFunctionCall(desc->m_ID, desc->GetSpaceNeededForArguments(), &retValue, bc, node, exprValue );

		// 释放临时变量
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

	// 查找与之匹配的操作符
	psSTypeBehaviour *beh = exprValue->type.GetBehaviour();

	// 查找操作符
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
		// 执行函数调用
		PerformFunctionCall( desc->m_ID, desc->GetSpaceNeededForArguments(), &retValue, bc, node, exprValue );

		// 释放临时变量
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

	// 只有类对象才对此操作有效
	if( first->GetNodeType() == snIdentifier )
	{
		// 得到属性名
		psCString name;
		name.Copy(&m_pScript->m_Code[first->GetTokenPos()], first->GetTokenLength());

		// 查找类成员属性
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

					// 计算其属性偏移
					if ( offset != 0 )
					{
						bc->Instr3(BC_ADDi, tmp.operand, exprValue->operand, MakeIntValueOp(offset) );
					}else
					{
						bc->Instr2(BC_MOV4, tmp.operand, exprValue->operand );
					}

					// 释放临时变量
					ReleaseTemporaryVariable( *exprValue );

					// 设置操作数
					exprValue->operand			  = tmp.operand;
					exprValue->isTemporary		  = true;
				}else
				{
					// 释放临时变量
					ReleaseTemporaryVariable( *exprValue );

					// 如果不是引用类对象,其成员属性的地址可以在编译时就知道
					assert( exprValue->operand.base == PSBASE_STACK ||
						exprValue->operand.base == PSBASE_GLOBAL );
					exprValue->operand.offset += offset;

					// 如果类属性是引用类型, 那么其值也应该是引用类型
					if ( prop->type.IsReference() )
						exprValue->type.IsReference() = true;
				}

				// 设置新的数据类型，保持其只读属性
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
		// 类成员函数调用
		if( !exprValue->type.IsObject() )
		{	
			// 不是类对象
			psCString str;
			str.Format(TXT_ILLEGAL_OPERATION_ON_s, exprValue->type.GetFormat().AddressOf() );
			Error(str.AddressOf(), node); 
		}
		else
		{
			psSValue value;

			// 编译函数调用
			CompileFunctionCall(first, bc, &value, exprValue );

			// 释放临时变量
			ReleaseTemporaryVariable( *exprValue );

			// 重新设置其值
			*exprValue = value;
		}
	}
}

void psCCompiler::CompilePostOpOpenBracket(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue)
{
	psCScriptNode* first = node->GetFirstChild();
	if ( exprValue->type.IsArrayType() || exprValue->type.IsPointer() )
	{	
		// 数组对象访问

		// 编译索引值
		psSValue argIndex;
		CompileAssignment(first, bc, &argIndex);

		// 数组索引值必需是整型
		if (!argIndex.type.IsIntegerType() || argIndex.type.IsPointer() )
		{
			Error(TXT_ARRAY_INDEX_MUSTBE_INTEGER, node);
			return;
		}	
		
		psSValue idxValue( g_IntDataType );
		if ( argIndex.type.IsReference() )
		{
			// 解析出引用的值
			AllocateTemporaryVariable( idxValue );
			bc->Instr2( BC_RDREF4, idxValue.operand, argIndex.operand );
			ReleaseTemporaryVariable( argIndex );
		}else
		{
			idxValue = argIndex;
		}

		// 弹出一级数组大小
		exprValue->type.PopArraySize();

		// NOTE: 数据存储是以是行存储的
		int subsize = exprValue->type.GetSizeInMemoryBytes() * exprValue->type.GetNumArrayElements();

		if ( !idxValue.IsConstant() || exprValue->type.IsReference() )
		{
			// 分配临时变量 
			psSValue tmp( psCDataType(ttInt, false, false) ) ;
			AllocateTemporaryVariable( tmp );

			// 计算地址
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

			// 释放临时变量
			ReleaseTemporaryVariable( *exprValue );
			
			// 设置值的属性
			exprValue->operand			   = tmp.operand;
			exprValue->isTemporary		   = true;
			exprValue->type.IsReference()  = true;	
			exprValue->type.IsPointer()	   = false;	 
		}else
		{
			// 如果不是引用地址, 并且索引值是常量, 其偏移可以在编译期就知道
			assert( exprValue->operand.base == PSBASE_STACK ||
				exprValue->operand.base == PSBASE_GLOBAL );

			exprValue->isTemporary		  = false;
			exprValue->operand.offset    += subsize * GetConstantInt( idxValue.operand );
		}

		// 释放临时变量
		ReleaseTemporaryVariable( idxValue );
	}else if ( exprValue->type.IsObject() )
	{
		// 类对象操作符重载

		// 编译索引值
		psSValue idxValue;
		CompileAssignment(first, bc, &idxValue);

		// 查找与之匹配的操作符
		psSTypeBehaviour *beh = exprValue->type.GetBehaviour();
		assert(beh);

		// 查找'[]'操作符
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

		// 匹配函数调用
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
			// 将参数入栈
			PushArgument( idxValue, func->m_ParamTypes[0], bc, node);

			psSValue retValue;

			// 执行函数调用
			PerformFunctionCall(func->m_ID, func->GetSpaceNeededForArguments(), &retValue, bc, node, exprValue );

			// 释放临时对象
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
// 名称: CopyConstructObject
// 说明: 
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

	// 如果是接口类对象,返回FALSE
	if ( objType->m_Beh.isInterface )
	{
		psCString msg;
		msg.Format(TXT_OBJECT_s_CANNOT_BE_INSTANCE, lvalue.type.GetFormat().AddressOf() );
		Error(msg.AddressOf(), node);
		return false;
	}

	// 调用类的拷贝构造函数, 如果有的话.
	// 如果没有则简单的进行Memcpy
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
		// 简单的内存拷贝
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
// 名称: ObjectAssignmentEx
// 说明: 
//-------------------------------------------------------------------
void psCCompiler::ObjectAssignmentEx(int assignOp, psSValue* retValue, const psSValue& lvalue, const psSValue& rvalue,  psCByteCode* bc, psCScriptNode* node )
{
	// 检查左右表达式的类型
	if (!IsLValue(lvalue))
	{
		Error( TXT_ASSIGN_LEFT_MUSTBE_LVALUE, node );	
		return;
	}
	assert( !lvalue.type.IsPointer() );

	// 如果是左值表达式是一个类，检查操作符是否已经被重载
	// 如果没有那就是一个简单的赋值运算
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
		// 精确匹配参数
		MatchArgument(0, rvalue, ops, exactmatches, true);

		if ( exactmatches.size() != 0 || rvalue.type.ObjectType() == objType )
			break;

		// 完全匹配参数
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

		// 参数入栈前, 先做一次隐式转换
		// 这个转换肯定是成功的，因为匹配参数已经通过了
		psSValue tmp;
		bool bOk = ImplicitConversion(rvalue, desc->m_ParamTypes[0], tmp, bc, node, false );
		assert( bOk );

		// 将参数入栈
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
		// 执行函数调用
		PerformFunctionCall( funcId, desc->GetSpaceNeededForArguments(), retValue, bc, node, &obj );

		// 释放临时变量
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
		// 拷贝构造
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

			// 释放临时变量
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

	// 得到数据类型
	psCDataType type = m_pBuilder->CreateDataTypeFromNode( typeNode, m_pScript );
	m_pBuilder->ModifyDataTypeFromNode( type, typeNode->GetNextNode(), m_pScript );

	// Compile assignment 
	psSValue argValue;
	CompileAssignment( argNode, bc, &argValue );

	// 检查变量是否已经初始化
	IsVariableInitialized(argValue, node);

	// 初始化
	exprValue->type = type;

	bool bOk = false;
	if ( type.IsPointer() )
	{
		if ( type.IsEqualExceptConst(argValue.type) )
		{
			// 检查只读属性
			if ( type.IsReadOnly() || !argValue.type.IsReadOnly() )
			{
				if ( !argValue.type.IsReference() )
				{	
					// 解析地址值
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
					// 是否为父类到子类的转换?
					while ( objType )
					{
						if ( objType == argObjType )
							break;
						baseOffset -= objType->m_BaseOffset;
						objType	    = objType->m_pParent;
					}
					if ( objType == NULL )
					{
						// 是否为子类到父类的转换?
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
						// 解析地址值
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
		// 尝试显式转换
		if ( ImplicitConversion( argValue, type, tmp, bc, node, true ) )
		{
			if ( !tmp.type.IsReference() && type.IsReference() )
			{	
				// 解析地址值
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
		// 释放临时变量
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




