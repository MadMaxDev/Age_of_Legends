#ifndef __PS_BUILDER_H__
#define __PS_BUILDER_H__

#include "config.h"
#include "objecttype.h"
#include "string.h"
#include "macro.h"

//-------------------------------------------------------------------
// 结构名: psSGlobalVariableDesc
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
// 结构名: psSFunctionDesc
//-------------------------------------------------------------------
struct psSFunctionDesc
{
	psCScriptCode *script;
	psCScriptNode *node;
	psCString	   name;
	int			   funcId;
};

//-------------------------------------------------------------------
// 类名: psCBuilder
// 说明: 
//-------------------------------------------------------------------
class psCBuilder
{
public:
	psCBuilder(psCScriptEngine*engine, psCModule* module);
	~psCBuilder();

	void SetOutputStream( psIOutputStream* pOut) { m_pOut = pOut; }
	void SetTextStream( psITextStream* pTextStream) { m_pTextStream = pTextStream; }


	// 分析数据类型(引擎在注删函数时使用)
	int ParseDataType(const psCHAR* name, psCDataType* dt);

	// 分析函数声明(引擎在注删函数时使用)
	int ParseFunctionDeclaration(const psCHAR *decl, psCScriptFunction *func, bool* bArgChanged);

	// 分析操作符重载声明
	int ParseOpOverloadDeclaration(const psCHAR *decl, psCScriptFunction *func, psETokenType& tokenType);

	// 分析变量声明(引擎在注删函数时使用)
	int ParseVariableDeclaration(const psCHAR *decl, psSProperty *var);

	// 输出错误信息
	void WriteError(const psCHAR *scriptname, const psCHAR* filename, const psCHAR *message, int r, int c);

	// 输出警告信息
	void WriteWarning(const psCHAR *scriptname, const psCHAR* filename, const psCHAR *message, int r, int c);

	// 输出信息
	void WriteInfo(const psCHAR *scriptname, const psCHAR *msg, int r, int c, bool preMessage);

	// 开始构建
	int Build( );

	// 编译字符串
	int BuildString(const psCHAR* str);

	// 从类型结点创建了数据类型
	psCDataType CreateDataTypeFromNode(psCScriptNode* node, psCScriptCode* script);

	// 修改数据类型的属性(只读,引用等等)
	void ModifyDataTypeFromNode(psCDataType& type, psCScriptNode* node, psCScriptCode* script );

	// 根据名称, 得到对象类型
	psCObjectType* GetObjectType(const psCHAR* name);

	// 注册常量
	int RegisterConstantDouble(double value);

	int RegisterConstantStrA( const psCString& str );

	int RegisterConstantStrW( const psCString& str );

	int RegisterConstantCharA( psCHAR* str, int &len);

	int RegisterConstantCharW( psCHAR* str, int &len);

	int RegisterConstantInt(int value);

	int RegisterConstantFloat(float value);

	// 得到常量值
	double GetConstantDouble(int offset);

	int    GetConstantInt(int offset);

	float  GetConstantFloat(int offset);

	const char* GetConstantPtr( int offset );

	// 分配全局变量
	int    AllocateGlobalVar( const psCDataType& type);
	void   DeallocGlobalVar(const psCDataType& type);

	// 根据名称, 得到类的属成员属性
	psSProperty* GetObjectProperty(const psCObjectType* objectType, const psCHAR *name, int& baseOffset );

	// 根据名称, 得到类的成员方法
	void GetObjectMethodDescrptions(const psCHAR* name, psCObjectType* type, std::vector<int>& funcs);

	// 根据函数ID,得到函数指针
	psCScriptFunction *GetFunctionDescription(int funcID);

	// 根据函数名称,得到一组函数指针
	void GetFunctionDescriptions( const psCHAR* name, std::vector<int> &funcs );

	// 得到全局变量
	psSProperty* GetGlobalProperty( const psCHAR* name );

	// 检查名称冲突(检查类名)
	int CheckNameConflictObject(const psCHAR *name, psCScriptNode *node, psCScriptCode *code);

	// 检查名称冲突(检查全局变量名)
	int CheckNameConflictGlobal(const psCHAR *name, psCScriptNode *node, psCScriptCode *code);

	// 检查名称冲突(检查类的成员名)
	int CheckNameConflictMember(psCDataType &dt, const psCHAR *name, psCScriptNode *node, psCScriptCode *code);

	// 检查名称冲突(检查类的成员函数)
	int CheckMethodConflict(psCObjectType* objType, psCString& name, psCDataType& retType, std::vector<psCDataType>& paramTypes, int type, int tokenType = 0);

	// 确认属性是否正确
	int VerifyProperty(psCDataType *dt, const psCHAR *decl, psCString &outName, psCDataType &outType);

	// 获取引擎指针
	psCScriptEngine* GetEngine() const { return m_pEngine; }

	// 得到所有的宏定义
	void GetAllMacros(MacroListMap_t& macroList);

	// 添加脚本函数
	psCScriptFunction* AddScriptFunction(const psCString& name, psCDataType* returnType, std::vector<psCDataType>* paramTypes);

	// 查找全局操作符重载
	void FindGlobalOperators(int op, std::vector<int>& funcs);

	// 得到优化选项
	const psSOptimizeOptions& GetOptimizeOptions(void) const { return m_OptimizeOptions; }

	bool ReadFile( const psCString& filename, psCString& code );

	bool DeclareConstantEnum( const psCHAR* field, const psCHAR* name, int value );

	bool GetConstantEnum( const psCHAR* field, const psCHAR* name, int& value );

	PS_ARGTYPE_CHECK_FUNC GetArgTypeCheckFunc();
protected:
	// 注册脚本函数
	psCScriptFunction* RegisterScriptFunction(int funcID, psCScriptNode *node, psCScriptCode *file, psCObjectType* objType = 0);

	// 注册头文件
	int RegisterIncludeMod(psCScriptNode *node, psCScriptCode *file);

	// 分析脚本
	void ParseScripts();

	// 编译所有的全局函数
	void CompileFunctions();

	// 编译所有的全局变量
	void CompileGlobalVariable(psCCompiler& compiler, psCScriptNode* node, psCScriptCode* code, psCByteCode& InitBC);

	// 编译类
	void CompileClass(psCCompiler& compiler, psCScriptNode* node, psCScriptCode* code);

	// 函数是否已经存在
	bool IsFunctionExist( const psCString& name, std::vector<psCDataType>& parameterTypes);

	// 全局操作符,是否已经被重载
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
	int					    m_NumErrors;						// 有多少个错误
	int						m_NumWarnings;						// 有多少个警告
	psCModule*				m_pModule;							// 当前构建的模块
	GlobalVariableMap_t						 m_GlobalVariables; // 所有的全局变量

	std::vector<psSFunctionDesc *>		     m_Functions;		// 当前要编译的全局函数
	std::map< psCString, EnumConstantMap_t > m_EnumConstants;	// 所有的枚举常量
	std::vector< psSGlobalVariableDesc* >	 m_CompileGlobalVariables;// 当前要编译的全局变量

	psCScriptEngine*   m_pEngine;							 // 引擎指针
	psSOptimizeOptions m_OptimizeOptions;					 // 代码生成的优化选项
};

#endif // __PS_BUILDER_H__
