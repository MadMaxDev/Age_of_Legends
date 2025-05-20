#ifndef __PS_COMPILER_H__
#define __PS_COMPILER_H__

#include "config.h"
#include "datatype.h"
#include "string.h"
#include "operand.h"
#include "debugsymbol.h"
#include "bytecode.h"

class psCCompiler
{
public:
	psCCompiler(void);
	~psCCompiler(void);

	// 编译类声明
	int CompileClassDecl(psCBuilder* builder, psCScriptCode* code, psCScriptNode* node, std::vector<psCScriptNode*>& methods, psCObjectType* objectType);

	// 编译函数
	int CompileFunction(psCBuilder* builder, psCScriptCode* script, psCScriptNode* func, psCObjectType* objType);

	// 编译全局变量
	int CompileGlobalVariable(psCBuilder* builder, psCScriptCode* script, psCScriptNode* expr, psSGlobalVariableDesc* gvar);

	// 编译枚举定义
	void CompileEnum( psCScriptNode* node, psCObjectType* objType, const psCHAR* field );

	// 生成类成员属性的默认构造函数
	void DefaultObjectConstructor(psCObjectType* objectType, psCByteCode* bc);

	// 生成类成员属性的默认拷贝构造函数
	void DefaultObjectCopyConstructor(psCObjectType* objectType, psCByteCode* bc);

	// 生成类成员属性的默认构构函数
	void DefaultObjectDestructor(psCObjectType* objectType, psCByteCode* bc);

	// 得到生成的二进制代码
	psCByteCode& GetByteCode()		 { return m_ByteCode; }

	// 得到最大的堆栈使用大小
	int GetLargestStackUsed() const  { return -m_StackPos; }

	// 重置编译器
	void Reset(psCBuilder* builder, psCScriptCode* script, psCObjectType* objectType);

	// 得到所有调试符号
	const std::vector<psSDebugSymbol>&  GetDebugSymbols() const { return m_DebugSymbols;  }

protected:
	// 编译类成员属性声明
	int CompilePropertyDecl(psCScriptNode* node, psCObjectType* objectType, int& byteoffset, psCByteCode* bc);

	// 生成默认构造函数
	bool DefaultConstructor(const psSValue& value, psCByteCode *bc);

	// 编译生成默认构造函数
	bool CompileConstructor(const psSValue& value, psCByteCode *bc);

	// 编译生成默认构构函数
	void CompileDestructor(const psSValue& value, psCByteCode *bc );

	// 编译语句块
	void CompileStatementBlock(psCScriptNode *block, bool ownVariableScope, bool *hasReturn, psCByteCode *bc);

	// 编译变量声明
	void CompileDeclaration(psCScriptNode* decl, psCByteCode* bc, psSGlobalVariableDesc* gvar);

	// 编译变更初始化表达式
	void CompileVariableInitExpr(psCScriptNode* node, psCDataType& type, const psCString& name, psSValue& value, psCByteCode* bc, psSGlobalVariableDesc* gvar);

	void CompileObjectConstructor(psCScriptNode* node, psCDataType& type, const psCString& name, psSValue& value, psCByteCode* bc, psSGlobalVariableDesc* gvar);

	// 编译变量定义
	void CompileVariableDim(psCScriptNode* varDim, psCByteCode* bc, psCString& name, psCDataType& dataType);

	// 编译变量初始化表达式
	void CompileInitExpression(psCScriptNode* initEpxr, psCByteCode* bc, const psSValue& value, psSValue& argValue);

	// 执行赋值操作(基本类型)
	void PerformAssignment(const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node);

	// 做隐式的类型转换
	bool ImplicitConversion( const psSValue& from,  const psCDataType& type, psSValue& to, psCByteCode* bc, psCScriptNode* node, bool isExplicit);

	// 对常量进行做隐式的类型转换
	bool ImplicitConversionConstant( const psSValue& from, const psCDataType& type, psSValue& to, psCByteCode* bc, psCScriptNode* node, bool isExplicit);

	// 编译赋值表达式
	void CompileAssignment(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);

	// 编译条件表达式
	void CompileCondition(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);

	// 编译算数表达式
	void CompileExpression(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);

	// 编译逆波兰表达式
	void CompilePostFixExpression(std::vector<psCScriptNode*>& postfix, psCByteCode* bc, psSValue* exprValue);

	// 编译表达式的原子量
	void CompileExpressionTerm(psCScriptNode* term, psCByteCode* bc, psSValue* exprValue );

	// 编译后置操作符表达式
	void CompileExpressionPostOp(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);
	void CompileObjectPostOp( int op, psCScriptNode* node, psCByteCode* bc, psSValue* exprValue );
	void CompilePrimitivePostOp( int op , psCScriptNode* node, psCByteCode* bc, psSValue* exprValue );
	void CompilePostOpDot( psCScriptNode* node, psCByteCode* bc, psSValue* exprValue );
	void CompilePostOpOpenBracket(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue);

	// 编译前置操作符表达式
	void CompileExpressionPrevOp(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);
	void CompileObjectPrevOp( int op, psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue  );
	void CompilePrimitivePrevOp( int op, psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue );

	// 编译表达式的值
	void CompileExpressionValue(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);

	void CompileIdentifierValue( psCScriptNode* vnode, psCByteCode* bc, psSValue* exprValue );
	bool GetGlobalProperty( const psCHAR* name, psCByteCode* bc, psSValue* exprValue);
	bool GetObjectProperty( psCObjectType* objType, const psCHAR* name, psSValue* exprValue );

	// 编译sizeof
	void CompileSizeof(psCScriptNode* node, psCByteCode* bc,  psSValue* exprValue );

	// 编译type_cast
	void CompileTypeCast(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue );

	// 编译枚举常量
	int CompileEnumConstant( psCScriptNode* node, psCHAR* name, int hint );

	// 编译常量字符串结点
	void CompileStringConstant(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue, psCString* pOutString);

	// 编译重载操作符
	bool CompileOverloadedOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 );

	// 编译操作符
	bool CompileOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 );

	void CompilePointerOperator( psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 );

	// 编译数学操作符
	void CompileMathOperator(psCScriptNode* node, psCByteCode* bc, bool& bAssign, psSValue* v0, psSValue* v1, psSValue* v2);

	// 编译二进制操作数
	void CompileBitwiseOperator(psCScriptNode* node, psCByteCode* bc, bool& bAssign, psSValue* v0, psSValue* v1, psSValue* v2 );

	// 编译比较操作符
	void CompileComparisionOperator(psCScriptNode* node, psCByteCode* bc, psSValue* v0, psSValue* v1, psSValue* v2 );

	// 编译逻辑操作符
	void CompileBooleanOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 );

	// 编译一条语句
	void CompileStatement(psCScriptNode* node, bool* hasReturn, psCByteCode* bc);

	// 编译一条表达式语句
	void CompileExpressionStatement(psCScriptNode *enode, psCByteCode *bc);

	// 编译'if'语句
	void CompileIFStatement(psCScriptNode* inode, bool* hasReturn, psCByteCode* bc);

	// 编译'switch'语句
	void CompileSwitchStamement(psCScriptNode* snode, bool* hasReturn, psCByteCode* bc);

	// 编译'case'语句
	void CompileCase(psCScriptNode* node, psCByteCode* bc);

	// 编译'break'语句
	void CompileBreakStatement(psCScriptNode* bnode, psCByteCode* bc);

	// 编译'continue'语句
	void CompileContinueStatement(psCScriptNode* cnode, psCByteCode* bc);

	// 编译'for'语句
	void CompileForStatement(psCScriptNode* fnode,  psCByteCode* bc);

	// 编译'while'语句
	void CompileWhileStatement(psCScriptNode* wnode, psCByteCode* bc);

	// 编译'Do-while'语句
	void CompileDoWhileStatement(psCScriptNode* wnode, psCByteCode* bc);

	// 编译'return'语句
	void CompileReturnStatement(psCScriptNode *node, psCByteCode *bc);

	// 编译函数调用
	void CompileFunctionCall(psCScriptNode* node, psCByteCode* bc, psSValue* retValue, const psSValue* object);

	// 准备函数调用
	void MatchFunctionCall(const std::vector<int>& allfuncs, std::vector<int>& matches, const std::vector<psSValue>& argValues, bool isExactMatch );

	// 编译参数列表
	void CompileArgumentList( psCScriptNode* argListNode, psCByteCode* bc, std::vector<psSValue>& argValues );
	
	//
	int PushArgumentList( psCScriptNode* argListNode, psCScriptFunction* func, psCByteCode* bc, const std::vector<psSValue>& argValues );

	// 构建参数列表字符串
	psCString BuildArgListString(const std::vector<psSValue>& argTypes);

	// 匹配参数
	void MatchArgument(int argIndex, const psSValue& argValue, const std::vector<int>& funcs, std::vector<int>& matches, bool isExactMatch);

	// 参数入栈
	void PushArgument( const psSValue& argValue, const psCDataType& to, psCByteCode* bc, psCScriptNode* node);

	// 执行函数调用
	void PerformFunctionCall(int func, int argSize, psSValue* retValue, psCByteCode* bc, psCScriptNode*node, const psSValue* object );

	// 判断变量是否经初始化
	bool IsVariableInitialized(const psSValue& var, psCScriptNode* node,  bool reportWarning = true );

	// 交换运算符的结合性,由右结合变为左结合
	void SwapPostFixOperands( std::stack<psCScriptNode*>& postfix, std::vector< psCScriptNode* >& target );

	// 得以运算符的优先级
	int  GetPrecedence(psCScriptNode* op);

	// 添加一个变量域
	void AddVariableScope(bool bIsBreakScope = false, bool bIsContinueScope = false);

	// 创建符号表
	void CreateDebugSymbols( psCVariableScope* varscope, int curline);

	// 删除当前处于栈顶的变量域
	void RemoveVariableScope();

	// 分配一个变量
	int  AllocateVariable(const psCDataType &type, bool tmp = false);	

	int  GetVariableSlot(int offset);

	// 释放一个变量
	inline void DeallocateVariable(int offset) 
	{
		m_FreeVariables.push_back( offset );
	}

	// 分配临时变量
	inline void AllocateTemporaryVariable(psSValue& value);

	// 释放一个临时变量
	void ReleaseTemporaryVariable(const psSValue& v);

	void ReleaseTemporaryOperand( const psSOperand& v );

	// 清空并释放所有的临时变量
	void ClearTemporaryVariables(psCByteCode* bc);

	// 生成一个临时类变量
	bool CopyConstructObject(const psSValue& dst, const psSValue& src, psCByteCode* bc, psCScriptNode* node);

	// 对两类对象进行赋值
	void ObjectAssignmentEx(int assignOp, psSValue* retValue, const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node);
	inline	void ObjectAssignment(const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node )
	{
		ObjectAssignmentEx( ttAssignment, NULL, lvalue, rvalue, bc, node);
	}
	
	// 调用类对象的成员函数
	void CallObjectMethod(const psSValue& value, const psSOperand& funcId, int argSize, psCByteCode* bc);

	// 判断是否为左值
	bool IsLValue(const psSValue& value);

	// 判断是否为临时变量
	bool IsTemporaryVariable(const psSValue& value);

	// 产生一个整型常数操作数
	inline psSOperand MakeIntValueOp(int value);

	// 产生一个浮点型常数操作数
	inline psSOperand MakeFloatValueOp(float value);

	// 得到整型常数操作数的值
	inline int GetConstantInt(const psSOperand& op);

	// 得到浮点型常数操作数的值
	inline float GetConstantFloat(const psSOperand& op);

	//生成一个行指今
	inline int InstrLINE(psCByteCode* bc, int pos);

	// 生成一个MOV指令
	inline void InstrMOV(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type);

	// 生成一个RDRER指令
	inline void InstrRDREF(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type);

	// 生成一个WTREF指令
	inline void InstrWTREF(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type);
	
	inline void InstrRRET(psCByteCode* bc, const psSOperand& op, const psCDataType& type);

	inline void InstrSRET(psCByteCode* bc, const psSOperand& op, const psCDataType& type);

	inline void CheckDataType( psCDataType& dt, psCScriptNode* node  );

	void GetNodeSource( psCHAR* name, int max_len, psCScriptNode* node );

	void GetIntOperand( psCByteCode* bc, const psSValue& value, psSOperand& intOperand );

	// 生成错误信息
	void Error(const psCHAR *msg, psCScriptNode *node);

	// 生成警告信息
	void Warning(const psCHAR *msg, psCScriptNode *node);
private:
	psCByteCode m_ByteCode;									// 生成的二制代码
	bool m_bIsMatchArgument;								// NOTE: 如果这个值为真,不用分配临时变量空

	bool m_bHasCompileError;								// 是否有编译错
	int	 m_NextLabel;										// 下一个要分配的标签
	int	 m_StackPos;										// 当前分配的栈顶位置

	psCDataType		  m_ReturnType;						
	psCVariableScope* m_Variables;							// 当时的变量(局部,全局,临时)
	psCBuilder*		  m_pBuilder;							// Builder
	psCScriptCode*	  m_pScript;							// 当前编译的源码
	psCObjectType*	  m_ObjectType;							// 当前函数的对象类型(如果为空,则为全局函数或全局操作符重载)

	std::vector< int >		 m_BreakLabels;					// 为break跳转预留的标签字仪式
	std::vector< int >		 m_ContinueLabels;				// 为continue跳转预留的标签字仪式

	std::vector< int >				  m_FreeVariables;		// 空闲的临时变量空间
	std::vector< int >				  m_TempVariables;		// 当前分配了的临时变量
	psCByteCode						  m_TmpVarDesBC;		// 临时变量的析构二制代码

	std::vector< psSValue >			  m_TempValues;

	std::vector< psSDebugSymbol >	  m_DebugSymbols;		// 调试符号(用于调试器)
	int								  m_LargestStackUsed;	// 函数使用的最大堆栈大小
	
	std::vector< std::pair< psCDataType, int > >		  m_VariableAllocations;
};

//-------------------------------------------------------------------
static const psCDataType g_IntDataType(ttInt, false, false);
static const psCDataType g_BoolDataType( ttBool, false, false );
//------------------------------------------------------------------ 

#endif // __PS_COMPILER_H__
