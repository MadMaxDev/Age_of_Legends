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

	// ����������
	int CompileClassDecl(psCBuilder* builder, psCScriptCode* code, psCScriptNode* node, std::vector<psCScriptNode*>& methods, psCObjectType* objectType);

	// ���뺯��
	int CompileFunction(psCBuilder* builder, psCScriptCode* script, psCScriptNode* func, psCObjectType* objType);

	// ����ȫ�ֱ���
	int CompileGlobalVariable(psCBuilder* builder, psCScriptCode* script, psCScriptNode* expr, psSGlobalVariableDesc* gvar);

	// ����ö�ٶ���
	void CompileEnum( psCScriptNode* node, psCObjectType* objType, const psCHAR* field );

	// �������Ա���Ե�Ĭ�Ϲ��캯��
	void DefaultObjectConstructor(psCObjectType* objectType, psCByteCode* bc);

	// �������Ա���Ե�Ĭ�Ͽ������캯��
	void DefaultObjectCopyConstructor(psCObjectType* objectType, psCByteCode* bc);

	// �������Ա���Ե�Ĭ�Ϲ�������
	void DefaultObjectDestructor(psCObjectType* objectType, psCByteCode* bc);

	// �õ����ɵĶ����ƴ���
	psCByteCode& GetByteCode()		 { return m_ByteCode; }

	// �õ����Ķ�ջʹ�ô�С
	int GetLargestStackUsed() const  { return -m_StackPos; }

	// ���ñ�����
	void Reset(psCBuilder* builder, psCScriptCode* script, psCObjectType* objectType);

	// �õ����е��Է���
	const std::vector<psSDebugSymbol>&  GetDebugSymbols() const { return m_DebugSymbols;  }

protected:
	// �������Ա��������
	int CompilePropertyDecl(psCScriptNode* node, psCObjectType* objectType, int& byteoffset, psCByteCode* bc);

	// ����Ĭ�Ϲ��캯��
	bool DefaultConstructor(const psSValue& value, psCByteCode *bc);

	// ��������Ĭ�Ϲ��캯��
	bool CompileConstructor(const psSValue& value, psCByteCode *bc);

	// ��������Ĭ�Ϲ�������
	void CompileDestructor(const psSValue& value, psCByteCode *bc );

	// ��������
	void CompileStatementBlock(psCScriptNode *block, bool ownVariableScope, bool *hasReturn, psCByteCode *bc);

	// �����������
	void CompileDeclaration(psCScriptNode* decl, psCByteCode* bc, psSGlobalVariableDesc* gvar);

	// ��������ʼ�����ʽ
	void CompileVariableInitExpr(psCScriptNode* node, psCDataType& type, const psCString& name, psSValue& value, psCByteCode* bc, psSGlobalVariableDesc* gvar);

	void CompileObjectConstructor(psCScriptNode* node, psCDataType& type, const psCString& name, psSValue& value, psCByteCode* bc, psSGlobalVariableDesc* gvar);

	// �����������
	void CompileVariableDim(psCScriptNode* varDim, psCByteCode* bc, psCString& name, psCDataType& dataType);

	// ���������ʼ�����ʽ
	void CompileInitExpression(psCScriptNode* initEpxr, psCByteCode* bc, const psSValue& value, psSValue& argValue);

	// ִ�и�ֵ����(��������)
	void PerformAssignment(const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node);

	// ����ʽ������ת��
	bool ImplicitConversion( const psSValue& from,  const psCDataType& type, psSValue& to, psCByteCode* bc, psCScriptNode* node, bool isExplicit);

	// �Գ�����������ʽ������ת��
	bool ImplicitConversionConstant( const psSValue& from, const psCDataType& type, psSValue& to, psCByteCode* bc, psCScriptNode* node, bool isExplicit);

	// ���븳ֵ���ʽ
	void CompileAssignment(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);

	// �����������ʽ
	void CompileCondition(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);

	// �����������ʽ
	void CompileExpression(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);

	// �����沨�����ʽ
	void CompilePostFixExpression(std::vector<psCScriptNode*>& postfix, psCByteCode* bc, psSValue* exprValue);

	// ������ʽ��ԭ����
	void CompileExpressionTerm(psCScriptNode* term, psCByteCode* bc, psSValue* exprValue );

	// ������ò��������ʽ
	void CompileExpressionPostOp(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);
	void CompileObjectPostOp( int op, psCScriptNode* node, psCByteCode* bc, psSValue* exprValue );
	void CompilePrimitivePostOp( int op , psCScriptNode* node, psCByteCode* bc, psSValue* exprValue );
	void CompilePostOpDot( psCScriptNode* node, psCByteCode* bc, psSValue* exprValue );
	void CompilePostOpOpenBracket(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue);

	// ����ǰ�ò��������ʽ
	void CompileExpressionPrevOp(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);
	void CompileObjectPrevOp( int op, psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue  );
	void CompilePrimitivePrevOp( int op, psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue );

	// ������ʽ��ֵ
	void CompileExpressionValue(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue);

	void CompileIdentifierValue( psCScriptNode* vnode, psCByteCode* bc, psSValue* exprValue );
	bool GetGlobalProperty( const psCHAR* name, psCByteCode* bc, psSValue* exprValue);
	bool GetObjectProperty( psCObjectType* objType, const psCHAR* name, psSValue* exprValue );

	// ����sizeof
	void CompileSizeof(psCScriptNode* node, psCByteCode* bc,  psSValue* exprValue );

	// ����type_cast
	void CompileTypeCast(psCScriptNode* node, psCByteCode* bc, psSValue* exprValue );

	// ����ö�ٳ���
	int CompileEnumConstant( psCScriptNode* node, psCHAR* name, int hint );

	// ���볣���ַ������
	void CompileStringConstant(psCScriptNode* expr, psCByteCode* bc, psSValue* exprValue, psCString* pOutString);

	// �������ز�����
	bool CompileOverloadedOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 );

	// ���������
	bool CompileOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 );

	void CompilePointerOperator( psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 );

	// ������ѧ������
	void CompileMathOperator(psCScriptNode* node, psCByteCode* bc, bool& bAssign, psSValue* v0, psSValue* v1, psSValue* v2);

	// ��������Ʋ�����
	void CompileBitwiseOperator(psCScriptNode* node, psCByteCode* bc, bool& bAssign, psSValue* v0, psSValue* v1, psSValue* v2 );

	// ����Ƚϲ�����
	void CompileComparisionOperator(psCScriptNode* node, psCByteCode* bc, psSValue* v0, psSValue* v1, psSValue* v2 );

	// �����߼�������
	void CompileBooleanOperator(psCScriptNode* node, psCByteCode* bc, psCByteCode* lbc, psCByteCode* rbc, psSValue* v0, psSValue* v1, psSValue* v2 );

	// ����һ�����
	void CompileStatement(psCScriptNode* node, bool* hasReturn, psCByteCode* bc);

	// ����һ�����ʽ���
	void CompileExpressionStatement(psCScriptNode *enode, psCByteCode *bc);

	// ����'if'���
	void CompileIFStatement(psCScriptNode* inode, bool* hasReturn, psCByteCode* bc);

	// ����'switch'���
	void CompileSwitchStamement(psCScriptNode* snode, bool* hasReturn, psCByteCode* bc);

	// ����'case'���
	void CompileCase(psCScriptNode* node, psCByteCode* bc);

	// ����'break'���
	void CompileBreakStatement(psCScriptNode* bnode, psCByteCode* bc);

	// ����'continue'���
	void CompileContinueStatement(psCScriptNode* cnode, psCByteCode* bc);

	// ����'for'���
	void CompileForStatement(psCScriptNode* fnode,  psCByteCode* bc);

	// ����'while'���
	void CompileWhileStatement(psCScriptNode* wnode, psCByteCode* bc);

	// ����'Do-while'���
	void CompileDoWhileStatement(psCScriptNode* wnode, psCByteCode* bc);

	// ����'return'���
	void CompileReturnStatement(psCScriptNode *node, psCByteCode *bc);

	// ���뺯������
	void CompileFunctionCall(psCScriptNode* node, psCByteCode* bc, psSValue* retValue, const psSValue* object);

	// ׼����������
	void MatchFunctionCall(const std::vector<int>& allfuncs, std::vector<int>& matches, const std::vector<psSValue>& argValues, bool isExactMatch );

	// ��������б�
	void CompileArgumentList( psCScriptNode* argListNode, psCByteCode* bc, std::vector<psSValue>& argValues );
	
	//
	int PushArgumentList( psCScriptNode* argListNode, psCScriptFunction* func, psCByteCode* bc, const std::vector<psSValue>& argValues );

	// ���������б��ַ���
	psCString BuildArgListString(const std::vector<psSValue>& argTypes);

	// ƥ�����
	void MatchArgument(int argIndex, const psSValue& argValue, const std::vector<int>& funcs, std::vector<int>& matches, bool isExactMatch);

	// ������ջ
	void PushArgument( const psSValue& argValue, const psCDataType& to, psCByteCode* bc, psCScriptNode* node);

	// ִ�к�������
	void PerformFunctionCall(int func, int argSize, psSValue* retValue, psCByteCode* bc, psCScriptNode*node, const psSValue* object );

	// �жϱ����Ƿ񾭳�ʼ��
	bool IsVariableInitialized(const psSValue& var, psCScriptNode* node,  bool reportWarning = true );

	// ����������Ľ����,���ҽ�ϱ�Ϊ����
	void SwapPostFixOperands( std::stack<psCScriptNode*>& postfix, std::vector< psCScriptNode* >& target );

	// ��������������ȼ�
	int  GetPrecedence(psCScriptNode* op);

	// ���һ��������
	void AddVariableScope(bool bIsBreakScope = false, bool bIsContinueScope = false);

	// �������ű�
	void CreateDebugSymbols( psCVariableScope* varscope, int curline);

	// ɾ����ǰ����ջ���ı�����
	void RemoveVariableScope();

	// ����һ������
	int  AllocateVariable(const psCDataType &type, bool tmp = false);	

	int  GetVariableSlot(int offset);

	// �ͷ�һ������
	inline void DeallocateVariable(int offset) 
	{
		m_FreeVariables.push_back( offset );
	}

	// ������ʱ����
	inline void AllocateTemporaryVariable(psSValue& value);

	// �ͷ�һ����ʱ����
	void ReleaseTemporaryVariable(const psSValue& v);

	void ReleaseTemporaryOperand( const psSOperand& v );

	// ��ղ��ͷ����е���ʱ����
	void ClearTemporaryVariables(psCByteCode* bc);

	// ����һ����ʱ�����
	bool CopyConstructObject(const psSValue& dst, const psSValue& src, psCByteCode* bc, psCScriptNode* node);

	// �����������и�ֵ
	void ObjectAssignmentEx(int assignOp, psSValue* retValue, const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node);
	inline	void ObjectAssignment(const psSValue& lvalue, const psSValue& rvalue, psCByteCode* bc, psCScriptNode* node )
	{
		ObjectAssignmentEx( ttAssignment, NULL, lvalue, rvalue, bc, node);
	}
	
	// ���������ĳ�Ա����
	void CallObjectMethod(const psSValue& value, const psSOperand& funcId, int argSize, psCByteCode* bc);

	// �ж��Ƿ�Ϊ��ֵ
	bool IsLValue(const psSValue& value);

	// �ж��Ƿ�Ϊ��ʱ����
	bool IsTemporaryVariable(const psSValue& value);

	// ����һ�����ͳ���������
	inline psSOperand MakeIntValueOp(int value);

	// ����һ�������ͳ���������
	inline psSOperand MakeFloatValueOp(float value);

	// �õ����ͳ�����������ֵ
	inline int GetConstantInt(const psSOperand& op);

	// �õ������ͳ�����������ֵ
	inline float GetConstantFloat(const psSOperand& op);

	//����һ����ָ��
	inline int InstrLINE(psCByteCode* bc, int pos);

	// ����һ��MOVָ��
	inline void InstrMOV(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type);

	// ����һ��RDRERָ��
	inline void InstrRDREF(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type);

	// ����һ��WTREFָ��
	inline void InstrWTREF(psCByteCode* bc, const psSOperand& op1, const psSOperand& op2, const psCDataType& type);
	
	inline void InstrRRET(psCByteCode* bc, const psSOperand& op, const psCDataType& type);

	inline void InstrSRET(psCByteCode* bc, const psSOperand& op, const psCDataType& type);

	inline void CheckDataType( psCDataType& dt, psCScriptNode* node  );

	void GetNodeSource( psCHAR* name, int max_len, psCScriptNode* node );

	void GetIntOperand( psCByteCode* bc, const psSValue& value, psSOperand& intOperand );

	// ���ɴ�����Ϣ
	void Error(const psCHAR *msg, psCScriptNode *node);

	// ���ɾ�����Ϣ
	void Warning(const psCHAR *msg, psCScriptNode *node);
private:
	psCByteCode m_ByteCode;									// ���ɵĶ��ƴ���
	bool m_bIsMatchArgument;								// NOTE: ������ֵΪ��,���÷�����ʱ������

	bool m_bHasCompileError;								// �Ƿ��б����
	int	 m_NextLabel;										// ��һ��Ҫ����ı�ǩ
	int	 m_StackPos;										// ��ǰ�����ջ��λ��

	psCDataType		  m_ReturnType;						
	psCVariableScope* m_Variables;							// ��ʱ�ı���(�ֲ�,ȫ��,��ʱ)
	psCBuilder*		  m_pBuilder;							// Builder
	psCScriptCode*	  m_pScript;							// ��ǰ�����Դ��
	psCObjectType*	  m_ObjectType;							// ��ǰ�����Ķ�������(���Ϊ��,��Ϊȫ�ֺ�����ȫ�ֲ���������)

	std::vector< int >		 m_BreakLabels;					// Ϊbreak��תԤ���ı�ǩ����ʽ
	std::vector< int >		 m_ContinueLabels;				// Ϊcontinue��תԤ���ı�ǩ����ʽ

	std::vector< int >				  m_FreeVariables;		// ���е���ʱ�����ռ�
	std::vector< int >				  m_TempVariables;		// ��ǰ�����˵���ʱ����
	psCByteCode						  m_TmpVarDesBC;		// ��ʱ�������������ƴ���

	std::vector< psSValue >			  m_TempValues;

	std::vector< psSDebugSymbol >	  m_DebugSymbols;		// ���Է���(���ڵ�����)
	int								  m_LargestStackUsed;	// ����ʹ�õ�����ջ��С
	
	std::vector< std::pair< psCDataType, int > >		  m_VariableAllocations;
};

//-------------------------------------------------------------------
static const psCDataType g_IntDataType(ttInt, false, false);
static const psCDataType g_BoolDataType( ttBool, false, false );
//------------------------------------------------------------------ 

#endif // __PS_COMPILER_H__
