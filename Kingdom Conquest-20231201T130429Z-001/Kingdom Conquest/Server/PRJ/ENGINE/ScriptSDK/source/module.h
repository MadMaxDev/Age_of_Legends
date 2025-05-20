#ifndef __PS_MODULE_H__
#define __PS_MODULE_H__

#if defined(_MSC_VER)
	#pragma warning(disable : 4786)
#endif

#include "config.h"
#include "string.h"
#include "datatype.h"
#include "objecttype.h"
#include "threadmanager.h"
#include "scriptfunction.h"

const int psFUNC_INIT		 = -1;
const int psFUNC_EXIT		 = -2;
const int psFUNC_STRING		 = -4;

class psCModule: public psIModule
{
public:
	psCModule(const psCHAR* name, int id, psCScriptEngine* engine);
	virtual ~psCModule(void);
	
public:
	//----------------------------------------------------------------------//
	// ʵ�ֻ���ӿڵĺ���
	//----------------------------------------------------------------------//
	// �õ�ģ����
	virtual const psCHAR* GetName() const { return m_Name.AddressOf(); }

	// �õ��ļ���
	virtual const psCHAR* GetFileName() const { return m_FileName.AddressOf(); }

	// �õ�ģ��ID
	virtual int GetID() const			{ return m_ModuleId;		  }

	// ���һ���ű������
	virtual int SetCode(const psCHAR* code, int length);

	// ��ȡԴ����
	virtual const psCHAR* GetSourceCode() const;

	// �õ�ģ���еĺ�������
	virtual unsigned int GetFunctionCount() const { return (unsigned int)m_ScriptFunctions.size(); }

	// ���ݺ���ID. �õ��ű������ӿ�
	virtual const psIScriptFunction* GetFunctionByID(int nID) const;

	virtual const psIScriptFunction* GetInitFunction() const { return m_pInitFunction; }
	virtual const psIScriptFunction* GetExitFunction() const { return m_pExitFunction; }

	// ���ݺ������ƣ� �õ��ű�����ID
	virtual int GetFunctionIDByName(const psCHAR* name) const;

	// ���ݺ�������. �õ��ű�������ID
	virtual int GetFunctionIDByDecl(const psCHAR* decl) const;

	// �õ������ĸ���
	virtual unsigned int GetObjectTypeCount() const { return (unsigned int)m_ObjectTypes.size(); }

	// ��������������õ������
	virtual const psIObjectType* GetObjectType(unsigned int i) const {assert(i<m_ObjectTypes.size()); return m_ObjectTypes[i]; }

	// ������������Ƶõ������
	virtual const psIObjectType* GetObjectType(const psCHAR* name) const { return FindObjectType(name); }

	// �õ�ȫ�ֱ���������
	virtual unsigned int GetGlobalVarCount() const;

	// �õ�ȫ�ֱ�������
	virtual const psIProperty* GetGlobalVar(int index) const;

	// �����������
	virtual int SaveByteCode(psIBinaryStream* out) const;

	// װ�ض�������
	virtual int LoadByteCode(psIBinaryStream* in);

	// �л��ϵ�
	virtual bool ToggleBreakPoint( int lineno, bool& isBreakPoint );

	// �����û��ϵ�
	virtual bool InsertBreakPoint( int lineno);

	// �Ƴ��û��ϵ�
	virtual bool RemoveBreakPoint( int lineno);

	// �Ƴ����е��û��ϵ�
	virtual void RemoveAllBreakPoint();

	// �õ����еĶϵ���Ϣ
	virtual psIBuffer* GetAllBreakPoint( int* numBreakPoints );
public:
	//----------------------------------------------------------------------//
	// �ڲ�ʹ�õĺ���
	//----------------------------------------------------------------------//
	void Reset();
	void Destroy();
	int  Build(psITextStream* pTextStream, psIOutputStream* pOutStream);

	// �ڴ����
	int AddContextRef()		 { return ++m_ContexRefCount;		}
	int ReleaseContextRef()  { return --m_ContexRefCount;		}
	bool CanReset() const     { return (m_ContexRefCount == 0 ); }

	// ��ʼ��/����ȫ�ֱ���
	bool CallInit();
	void CallExit();

	// ��ȡ���������Ϣ
	int GetNextFunctionId()			const	{ return int(m_ScriptFunctions.size());						}

	psCScriptFunction* GetScriptFunction(unsigned int idx) const;
	psCScriptFunction* GetSpecialFunction(int funcID);

	// ��ȡȫ�ֱ�����Ϣ
	int  GetGlobalVarIDByName(const psCHAR *name);
	int  GetGlobalVarIDByDecl(const psCHAR *decl);

	// ��ӽű�����
	psCScriptFunction* AddScriptFunction(int id, const psCHAR* name, psCDataType returnType, std::vector<psCDataType>& paramTypes);
	int	AddScriptFunction(psCScriptFunction* func) { m_ScriptFunctions.push_back( func ); return (int)m_ScriptFunctions.size() - 1; }

	// ��ӳ���
	int AddConstantDouble( double value);
	int AddConstantInt( int value);
	int AddConstantFloat(float value);
	int AddConstantStrA( const char* str );
	int AddConstantStrW( const wchar_t* str );
	
	// �õ������غ�ȫ�����ݳ�
	psBYTE* GetGlobalMemPtr() const   { return m_GlobalMem; }
	psBYTE* GetConstantMemPtr() const { return m_ConstantMem; }
	int GetConstantMemSize() const    { return m_ConstantMemSize;}
	int GetGloablMemSize() const	  { return m_GlobalMemSize; }

	int  AddConstMemSize( int s );

	// ����ȫ�ֿռ�
	int	 AllocGlobalMemory(const psCDataType& type);
	void DeallocGlobalMemory(const psCDataType& type);

	void AddObjectType(psCObjectType* pObjType)		 { m_ObjectTypes.push_back( pObjType );  }
	void AddGlobalVar(psSProperty* prop)			 { m_ScriptGlobals.push_back(prop);		 }

	bool IsGlobalVarInitialized() const				 { return m_IsGlobalVarInitialized;		 }

	const psCString& Name() const					 { return m_Name;			   }
	void SetFileName( const psCHAR* filename )		 { m_FileName = filename;	   }
	psSProperty* GlobalVar(size_t id) const			 { return m_ScriptGlobals[id]; }

	// �õ��ű��Ķ�������
	psCObjectType* FindObjectType(const psCHAR* name) const;
	int GetObjectTypeIdx(const psCObjectType* objType);

	// ����ȫ�ֲ���������
	void FindGlobalOperators(int op, std::vector<int>& funcs);
	void AddGlobalOperator(int op, int func);

	// �õ��ű�Դ��
	psCScriptCode* GetCode()						 { return m_pCode; }

	// ��Restore��ֱ������ByteCode�ļ������ܵĽӿ�
	void SetModuleRestoreSuccess()						{m_IsBuildWithoutErrors=true;}
private:	
    std::vector<psCScriptFunction*>			 m_ScriptFunctions;			// �ű�����
	std::vector<psSProperty*>				 m_ScriptGlobals;			// �ű�ȫ�ֲ���

	psBYTE*									 m_GlobalMem;				// ȫ�ֱ����ռ�
	psBYTE*									 m_ConstantMem;				// ������	
	int										 m_ConstantMemSize;
	int										 m_ConstantMemCapacity;
	int										 m_GlobalMemSize;
	std::map< int, int >					 m_IntConstants;			// �����˵����ͳ�����	
	std::map< float, int >					 m_FloatConstants;			// �����˵ĵ����ȸ����ͳ�����
	std::map< double, int >					 m_DoubleConstants;			// �����˵�˫���ȸ����ͳ�����
	std::map< psCStringA, int >				 m_StrAConstants;			// Ansi���ֳ����б� 
	std::map< psCStringW, int >				 m_StrWConstants;			// Unicode���ֳ����б� 

	std::vector<psCObjectType*>				 m_ObjectTypes;				// �ű��Ķ�������
	std::vector<int>						 m_GlobalOperators;			// �ű����ȫ�ֲ���������

	psCScriptFunction						*m_pInitFunction;			// ��ʼ��ȫ�ֲ����ĺ���
	psCScriptFunction						*m_pExitFunction;			// 
	psCScriptFunction						*m_pStringFunction;			// �ַ������к���

	psCString								 m_Name;					// ģ������
	psCString								 m_FileName;				// �ļ���
	psCScriptCode*							 m_pCode;					// �ű�Դ����
	int										 m_ModuleId;				// ģ��ID
	bool									 m_IsBuildWithoutErrors;	// �Ƿ񹹽��ɹ�
	bool									 m_IsGlobalVarInitialized;  // ȫ�ֱ����Ƿ��Ѿ���ʼ��
	psCScriptEngine*						 m_pEngine;					// Engine

	psCAtomicCount							 m_ContexRefCount;			// ���ü���

	friend class psCRestore;
};

#endif // __PS_MODULE_H__
