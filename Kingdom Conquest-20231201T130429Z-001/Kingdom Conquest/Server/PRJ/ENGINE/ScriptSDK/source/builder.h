#ifndef __PS_BUILDER_H__
#define __PS_BUILDER_H__

#include "config.h"
#include "objecttype.h"
#include "string.h"
#include "macro.h"

//-------------------------------------------------------------------
// �ṹ��: psSGlobalVariableDesc
//-------------------------------------------------------------------
struct psSGlobalVariableDesc
{
	psCString    name;
	psSProperty *property;
	psCDataType  datatype;
	int		     index;
	int			 base;
};

//-------------------------------------------------------------------
// �ṹ��: psSFunctionDesc
//-------------------------------------------------------------------
struct psSFunctionDesc
{
	psCScriptCode *script;
	psCScriptNode *node;
	psCString	   name;
	int			   funcId;
};

//-------------------------------------------------------------------
// ����: psCBuilder
// ˵��: 
//-------------------------------------------------------------------
class psCBuilder
{
public:
	psCBuilder(psCScriptEngine*engine, psCModule* module);
	~psCBuilder();

	void SetOutputStream( psIOutputStream* pOut) { m_pOut = pOut; }
	void SetTextStream( psITextStream* pTextStream) { m_pTextStream = pTextStream; }


	// ������������(������עɾ����ʱʹ��)
	int ParseDataType(const psCHAR* name, psCDataType* dt);

	// ������������(������עɾ����ʱʹ��)
	int ParseFunctionDeclaration(const psCHAR *decl, psCScriptFunction *func, bool* bArgChanged);

	// ������������������
	int ParseOpOverloadDeclaration(const psCHAR *decl, psCScriptFunction *func, psETokenType& tokenType);

	// ������������(������עɾ����ʱʹ��)
	int ParseVariableDeclaration(const psCHAR *decl, psSProperty *var);

	// ���������Ϣ
	void WriteError(const psCHAR *scriptname, const psCHAR* filename, const psCHAR *message, int r, int c);

	// ���������Ϣ
	void WriteWarning(const psCHAR *scriptname, const psCHAR* filename, const psCHAR *message, int r, int c);

	// �����Ϣ
	void WriteInfo(const psCHAR *scriptname, const psCHAR *msg, int r, int c, bool preMessage);

	// ��ʼ����
	int Build( );

	// �����ַ���
	int BuildString(const psCHAR* str);

	// �����ͽ�㴴������������
	psCDataType CreateDataTypeFromNode(psCScriptNode* node, psCScriptCode* script);

	// �޸��������͵�����(ֻ��,���õȵ�)
	void ModifyDataTypeFromNode(psCDataType& type, psCScriptNode* node, psCScriptCode* script );

	// ��������, �õ���������
	psCObjectType* GetObjectType(const psCHAR* name);

	// ע�᳣��
	int RegisterConstantDouble(double value);

	int RegisterConstantStrA( const psCString& str );

	int RegisterConstantStrW( const psCString& str );

	int RegisterConstantCharA( psCHAR* str, int &len);

	int RegisterConstantCharW( psCHAR* str, int &len);

	int RegisterConstantInt(int value);

	int RegisterConstantFloat(float value);

	// �õ�����ֵ
	double GetConstantDouble(int offset);

	int    GetConstantInt(int offset);

	float  GetConstantFloat(int offset);

	const char* GetConstantPtr( int offset );

	// ����ȫ�ֱ���
	int    AllocateGlobalVar( const psCDataType& type);
	void   DeallocGlobalVar(const psCDataType& type);

	// ��������, �õ��������Ա����
	psSProperty* GetObjectProperty(const psCObjectType* objectType, const psCHAR *name, int& baseOffset );

	// ��������, �õ���ĳ�Ա����
	void GetObjectMethodDescrptions(const psCHAR* name, psCObjectType* type, std::vector<int>& funcs);

	// ���ݺ���ID,�õ�����ָ��
	psCScriptFunction *GetFunctionDescription(int funcID);

	// ���ݺ�������,�õ�һ�麯��ָ��
	void GetFunctionDescriptions( const psCHAR* name, std::vector<int> &funcs );

	// �õ�ȫ�ֱ���
	psSProperty* GetGlobalProperty( const psCHAR* name );

	// ������Ƴ�ͻ(�������)
	int CheckNameConflictObject(const psCHAR *name, psCScriptNode *node, psCScriptCode *code);

	// ������Ƴ�ͻ(���ȫ�ֱ�����)
	int CheckNameConflictGlobal(const psCHAR *name, psCScriptNode *node, psCScriptCode *code);

	// ������Ƴ�ͻ(�����ĳ�Ա��)
	int CheckNameConflictMember(psCDataType &dt, const psCHAR *name, psCScriptNode *node, psCScriptCode *code);

	// ������Ƴ�ͻ(�����ĳ�Ա����)
	int CheckMethodConflict(psCObjectType* objType, psCString& name, psCDataType& retType, std::vector<psCDataType>& paramTypes, int type, int tokenType = 0);

	// ȷ�������Ƿ���ȷ
	int VerifyProperty(psCDataType *dt, const psCHAR *decl, psCString &outName, psCDataType &outType);

	// ��ȡ����ָ��
	psCScriptEngine* GetEngine() const { return m_pEngine; }

	// �õ����еĺ궨��
	void GetAllMacros(MacroListMap_t& macroList);

	// ��ӽű�����
	psCScriptFunction* AddScriptFunction(const psCString& name, psCDataType* returnType, std::vector<psCDataType>* paramTypes);

	// ����ȫ�ֲ���������
	void FindGlobalOperators(int op, std::vector<int>& funcs);

	// �õ��Ż�ѡ��
	const psSOptimizeOptions& GetOptimizeOptions(void) const { return m_OptimizeOptions; }

	bool ReadFile( const psCString& filename, psCString& code );

	bool DeclareConstantEnum( const psCHAR* field, const psCHAR* name, int value );

	bool GetConstantEnum( const psCHAR* field, const psCHAR* name, int& value );

	PS_ARGTYPE_CHECK_FUNC GetArgTypeCheckFunc();
protected:
	// ע��ű�����
	psCScriptFunction* RegisterScriptFunction(int funcID, psCScriptNode *node, psCScriptCode *file, psCObjectType* objType = 0);

	// ע��ͷ�ļ�
	int RegisterIncludeMod(psCScriptNode *node, psCScriptCode *file);

	// �����ű�
	void ParseScripts();

	// �������е�ȫ�ֺ���
	void CompileFunctions();

	// �������е�ȫ�ֱ���
	void CompileGlobalVariable(psCCompiler& compiler, psCScriptNode* node, psCScriptCode* code, psCByteCode& InitBC);

	// ������
	void CompileClass(psCCompiler& compiler, psCScriptNode* node, psCScriptCode* code);

	// �����Ƿ��Ѿ�����
	bool IsFunctionExist( const psCString& name, std::vector<psCDataType>& parameterTypes);

	// ȫ�ֲ�����,�Ƿ��Ѿ�������
	bool IsGlobalOperatorExist(int op, std::vector<psCDataType>& parameterTypes);

	struct stringptr_lesser
	{
		bool operator()(const psCHAR* lhs, const psCHAR* rhs ) const
		{
			return (ps_strcmp(lhs, rhs) < 0);
		}
	};

	typedef std::map< psCString, int > EnumConstantMap_t;
	typedef std::map< const psCHAR*, psSGlobalVariableDesc*, stringptr_lesser > GlobalVariableMap_t;

	psIOutputStream*		m_pOut;								//
	psITextStream*			m_pTextStream;
	int					    m_NumErrors;						// �ж��ٸ�����
	int						m_NumWarnings;						// �ж��ٸ�����
	psCModule*				m_pModule;							// ��ǰ������ģ��
	GlobalVariableMap_t						 m_GlobalVariables; // ���е�ȫ�ֱ���

	std::vector<psSFunctionDesc *>		     m_Functions;		// ��ǰҪ�����ȫ�ֺ���
	std::map< psCString, EnumConstantMap_t > m_EnumConstants;	// ���е�ö�ٳ���
	std::vector< psSGlobalVariableDesc* >	 m_CompileGlobalVariables;// ��ǰҪ�����ȫ�ֱ���

	psCScriptEngine*   m_pEngine;							 // ����ָ��
	psSOptimizeOptions m_OptimizeOptions;					 // �������ɵ��Ż�ѡ��
};

#endif // __PS_BUILDER_H__
