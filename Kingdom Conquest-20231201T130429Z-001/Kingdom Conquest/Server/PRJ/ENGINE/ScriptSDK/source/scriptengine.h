#ifndef __PS_SCRIPTENGINE_H__
#define __PS_SCRIPTENGINE_H__

#include "config.h"
#include "objecttype.h"
#include "callfunc.h"
#include "context.h"
#include "macro.h"

class psCScriptEngine: public psIScriptEngine
{
	//----------------------------------------------------------------------//
	// ʵ�ֻ���ķ���
	//----------------------------------------------------------------------//
public:
	// �ڴ����
	virtual long AddRef();
	virtual long Release();

	// ע��һ��C++��('name'Ϊ���ڽű���ʹ�õ�����, 'byteSize'Ϊ���С, 'flags'Ϊ�������Ϊ��־) 
	virtual int RegisterObjectType(const psCHAR* name, int byteSize, psDWORD flags);

	// ע��C++��ĳ�Ա����('objname'Ϊ���ڽű���ʹ�õ�����, 'declaration'Ϊ���Ա����������, 'byteOffset'Ϊ����C++���е�ƫ��)
	virtual int RegisterObjectProperty(const psCHAR* objname, const psCHAR* declaration, int byteOffset);

	// ע��C++��ĳ�Ա����('objname'Ϊ���ڽű���ʹ�õ�����, 'declaration'Ϊ���Ա����������, funcPointerΪ�亯����ֵַ, 'callConv'��ʾ����÷�ʽ)
	virtual int RegisterObjectMethod(const psCHAR* objname, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv);

	// ע��C++�����Ϊ('objname'Ϊ���ڽű���ʹ�õ�����, 'behavior'Ϊ����Ϊ����, 'decaration'Ϊ�亯������, funcPointerΪ�亯����ֵַ, 'callConv'��ʾ����÷�ʽ)
	virtual int RegisterObjectBehaviour(const psCHAR* objname, psDWORD behaviour, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv);

	// ע��һ��ȫ�ֱ���('declaration'Ϊ������, 'pointer'Ϊ���ֵַ
	virtual int RegisterGlobalProperty(const psCHAR* declaration, void* pointer);

	// ע��һ�����ͳ���
	virtual int RegisterIntConstant(const psCHAR* name, int v);

	// ע��һ�����㳣��
	virtual int RegisterFloatConstant(const psCHAR* name, float v);

	// ע��һ��ȫ�ֺ���('declaration'Ϊ������, 'funcPointer'Ϊ�亯����ֵַ, 'callConv'��ʾ����÷�ʽ)
	virtual int RegisterGlobalFunction(const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv);

	//  ע��һ��ȫ�ֲ���������('behavior'Ϊ������������, 'decl'Ϊ������, 'funcPointer'Ϊ�亯����ֵַ, 'callConv'��ʾ����÷�ʽ)
	virtual int RegisterGlobalBehaviour(psDWORD behaviour, const psCHAR* decl, psUPtr funcPointer, psDWORD callConv);

	// ������ļ̳й�ϵ
	// ����: 'name'����
	//		 'parent'������
	//		 'baseOffset'C++�����ൽ�����ƫ��
	virtual int DeclareObjectHierarchy(const psCHAR* name, const psCHAR* parent, int baseOffset );
		
	// ע��һ��C++��
	virtual int UnregisterObjectType( const psCHAR* name );

	// ע��һ��ȫ�ֱ���
	virtual int UnregisterGlobalProperty(void* ptr);

	// ע��һ��ȫ�ֺ���
	virtual int UnregisterGlobalFunction( psUPtr funcPtr );

	// ע��һ��ȫ�ֲ���������
	virtual int UnregisterGlobalBehaviour( psUPtr funcPtr );

	// ��ȡ��������͵ĸ���
	virtual unsigned int GetObjectTypeCount() const;

	// ��ȡ���е����������
	virtual void GetAllObjectTypes(psIObjectType* objectTypes[]) const;

	// �������Ƶõ�ע���C++������ͽӿ�, ����������ܻ�ʧ��
	virtual const psIObjectType* FindObjectType(const psCHAR* name) const;

	// ���ݺ���ID,�õ�ϵͳ�����ӿ�
	virtual const psIScriptFunction* GetSystemFunction(int nID) const;

	// �õ�ϵͳȫ�ֺ����ĸ���
	virtual unsigned int GetGlobalFunctionCount() const { return (unsigned int)m_GlobalFunctions.size(); }

	// ���ݺ�������,�õ�ϵͳȫ�ֺ����ӿ�
	virtual const psIScriptFunction* GetGlobalFunction(unsigned int index) const;

	// �õ�ȫ�����Եĸ���
	virtual unsigned int GetGlobalPropCount() const { return (unsigned int)m_GlobalProps.size();	}

	// ��ȡ���е�ȫ������
	virtual void GetAllGlobalProps( psIProperty* pAllGlobalProps[] ) const;

	//  �������Ƶõ�ȫ�ֱ������Խӿ�
	virtual const psIProperty* FindGlobalProp(const psCHAR* name) const;

	// �õ��ű�ģ�������
	virtual unsigned int GetScriptModuleCount() const { return (unsigned int)m_ScriptModules.size(); }

	// ���������õ��ű�ģ��ӿ�
	virtual psIModule* GetModule(unsigned int index) const;

	// ����ģ�����Ƶõ��ű�ģ��ӿ�
	virtual psIModule* GetModule(const psCHAR* name) const;

	virtual int CreateModule(const psCHAR* name, const psCHAR* filename, psIModule** mod);

	// �������еĽű�ģ��
	virtual void Reset();

	// ����һ���ű�ģ��
	virtual int Discard(const psCHAR* module);

	// ���ô����Ż�ѡ��
	virtual void SetOptimizeOptions( psSOptimizeOptions options ) { m_OptimizeOptions = options; }

	// �õ������Ż�ѡ��
	virtual psSOptimizeOptions GetOptimizeOptions() const		  { return m_OptimizeOptions; }

	// ����һ���ű�ģ��
	virtual int Compile(  psIModule* mod, psITextStream* pTextStream, psIOutputStream* pOutStream );

	// ִ��һ���ַ���
	virtual int ExecuteString( psIScriptContext* ctx, const psCHAR *script, psIOutputStream* pOutStream );

	// ����Ĭ�ϵĶ�ջ��С
	virtual void SetDefaultContextStackSize(psUINT initial, psUINT maximum);

	// �����ű����������Ľӿ�
	virtual int CreateContext(psIScriptContext **contex);

	virtual void SetDebugger( psIDebugger* debugger )  { m_pDebugger = debugger; }

	// ע��һ���ַ���������('dataType'Ϊ����,'getFunc'Ϊ��δ�һ�������õ��ַ���ֵ�ĺ���ָ��, 'setFunc'Ϊ��δ�һ���ַ���ָ�븳ֵ )
	virtual int  RegisterStringFactory(const psCHAR* dataType, PS_GET_STRING_VALUE_FUNC getFunc, PS_SET_STRING_VALUE_FUNC setFunc);

	// �ж�һ�����������Ƿ�Ϊ�ַ�������
	virtual bool IsStringType(const psIObjectType* objType) const;

	// ����һ���ַ���������ֵ
	virtual bool  SetStringValue(const psIObjectType* objType, void* obj, const psCHAR* value);

	// �õ�һ���ַ���������ֵ
	virtual psAutoStringPtr GetStringValue(const psIObjectType* objType, void* obj) const;

	// ����һ�������
	virtual void* AllocObject( int moduleId, const psIObjectType* objType );

	// �ͷ�һ�������
	virtual bool FreeObject( void* obj, int moduleId, const psIObjectType* objType  );

	// ���ú궨���б��ַ���
	virtual bool SetMacroList(const psCHAR* macros, psCHAR sperator);

	// �õ��궨���б��ַ���
	virtual const psCHAR* GetMacroList();

	// ���ñ�κ������ͼ��ص�����
	virtual void SetArgTypeCheckFunc( PS_ARGTYPE_CHECK_FUNC pFunc ) { m_pArgTypeCheckFunc = pFunc; }

	// �õ���κ������ͼ��ص�����
	virtual PS_ARGTYPE_CHECK_FUNC GetArgTypeCheckFunc()				{ return m_pArgTypeCheckFunc; }
public:	
	//----------------------------------------------------------------------//
	// �ڲ�����
	//----------------------------------------------------------------------//
	psCScriptEngine(void);
	virtual ~psCScriptEngine(void);

	psCObjectType* _findObjectType( const psCHAR* name ) const;

	void AddSystemFunction(psCScriptFunction* &func, psSSystemFunctionInterface* internal);
	void DeleteSystemFunctionById( int funcId );

	const std::vector<int>& GetSystemFuncIDByName( const psCString& name ) const;

	bool IsFunctionEqual(psCScriptFunction* f0, psCScriptFunction* f1);

	psSSystemFunctionInterface* GetSystemFunctionInterface(int index) { return m_SystemFunctionInterfaces[index]; }

	unsigned int GetSystemFunctionCount() const			  { return (unsigned int)m_SystemFunctions.size();  }
	inline psCScriptFunction* GetSystemFunctionByIndex(int index) 
	{ 
		assert( index < (int)m_SystemFunctions.size() && index >= 0 );	
		return m_SystemFunctions[index];
	}
	inline psCScriptFunction* GetSystemFunctionByID( int nID )
	{
		return GetSystemFunctionByIndex( -1 - nID );
	}
	
	void* GetGlobalAddress(size_t idx);

	void FindGlobalOperators(int op, std::vector<int>& ops);

	bool CallObjectMethod(void *obj, int modId, int func);
	bool CallObjectMethod(void *obj, void *param, int modId, int func);
	bool CallArrayObjMethod( void* obj, int modId, int func );

	size_t GetNumMacros() const						{ return m_MacroList.size(); }
	const psCString& GetMacroName(size_t idx) const { return m_MacroList[idx];   }

	int GetInitialContextStackSize() const			{ return m_InitContexStackSize;  }
	int GetMaxContextStackSize()     const			{ return m_MaxContexStackSize;   }

	void PrepareEngine();

	psCObjectType *GetObjectType(const psCHAR *type);

	int AddBehaviourFunction(psCScriptFunction &func, psSSystemFunctionInterface &internal);

	int ConfigError(int err);

	// �жϲ������ܷ���Ա�����
	static bool IsOpAllowOverload(int op, bool object);

	// �ű�ģ��
	psCModule* GetLastModule() const		{ return m_pLastModule;			 }
	psCModule* GetStringModule() const		{ return m_pStringModule;		 }

	void SetLastModule(psCModule* module)   { m_pLastModule = module;}
	void ClearModule(int index)				{ m_ScriptModules[index] = NULL; }

	psIDebugger* GetDebugger() const		{ return m_pDebugger; }

	typedef std::vector<psCString>			 MacroList_t;
private:
	struct STRING_FACTORY
	{
		PS_GET_STRING_VALUE_FUNC	pGetValueFuncPointer;
		PS_SET_STRING_VALUE_FUNC	pSetValueFuncPointer;
	};
	struct stringptr_lesser
	{
		bool operator()(const psCHAR* lhs, const psCHAR* rhs ) const
		{
			return (ps_strcmp(lhs, rhs) < 0);
		}
	};
	typedef std::map< const psCHAR*, int, stringptr_lesser >			 ModuleNameMap_t;
	typedef	std::map< const psIObjectType*, STRING_FACTORY>				 StringFactoryMap_t;	
	typedef std::map< psCString, std::vector< int > >					 FuncNameMap_t;
	typedef std::map< const psCHAR*, psCObjectType*, stringptr_lesser >  ObjectTypeMap_t;
	typedef std::map< const psCHAR*, psSProperty*, stringptr_lesser >	 PropertyMap_t;

	int										    m_InitContexStackSize;					// ��ʼ��ջ�Ĵ�С
	int											m_MaxContexStackSize;					// ���Ķ�ջ��С

	// �ű�ע�����Ϣ
	StringFactoryMap_t							m_StringFactory;						// �ַ�������
	std::vector< int >							m_GlobalOperators;						// ȫ�ֲ���������Ϊ����
	ObjectTypeMap_t								m_ObjectTypes;							// ���е����������
	PropertyMap_t								m_GlobalProps;							// ȫ�ֱ�������
	std::vector< void* >						m_GlobalPropAddresses;					// ȫ�ֱ�����ַ
	std::vector< psCScriptFunction* >			m_SystemFunctions;						// ϵͳ����
	std::vector< psSSystemFunctionInterface* >  m_SystemFunctionInterfaces;				// ϵͳ�����ӿ�
	std::vector< int >							m_FreeFunctionId;						// ����ĺ���ID
	std::vector< int >							m_GlobalFunctions;						// ȫ�ֺ���ID
	FuncNameMap_t								m_FuncNameMap;							 
	psIDebugger								   *m_pDebugger;

	bool										m_bConfigFailed;						// �ű������Ƿ��д�
	bool										m_bIsPrepared;							// �����Ƿ��Ѿ�׼������

	psCAtomicCount								m_RefCount;								// ���ü���
	std::vector<psCModule *>					m_ScriptModules;						// ���еĽű�ģ��
	ModuleNameMap_t								m_ModuleNameMap;						//
	mutable psCModule							*m_pLastModule;							// ������ʵĽű�ģ��
	psCModule									*m_pStringModule;						// �����ַ���ģ��
	psCContext									*m_pDefaultCtx;							// Ĭ�ϵ�����������

	psSOptimizeOptions							 m_OptimizeOptions;

	// �궨��
	MacroList_t m_MacroList;
	psCString m_strMacros;

	PS_ARGTYPE_CHECK_FUNC						 m_pArgTypeCheckFunc;					// ��κ����������ͼ��Ļص�
	friend class psCRestore;
};

#endif // __PS_SCRIPTENGINE_H__


