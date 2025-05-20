// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : 
// File         : pixelscript.h
// Creator      : ����
// Comment      : ���Ҫ����ϵͳ�ļ��Ļ����Ͱ������
// CreationDate : 2006-04-21
// ChangeLOG    : 2006-09-22 �޸�psAutoStringPtrΪ�������ü����Ϳ������캯���İ汾��

#ifndef __PIXELSCRIPT_H__
#define __PIXELSCRIPT_H__

#include <assert.h>
#include "types.h"

// �ű�����İ汾��Ϣ
#define PIXELSCRIPT_VERSION        20000
#define PIXELSCRIPT_VERSION_MAJOR  2
#define PIXELSCRIPT_VERSION_MINOR  0
#define PIXELSCRIPT_VERSION_BUILD  0
#define PIXELSCRIPT_VERSION_STRING "2.0.0"

// ����ָ������
typedef void (* psFUNCTION_t)();
typedef void (PS_STDCALL* psSTDCALL_FUNCTION_t)();

// ��ĳ�Ա����ָ������
class psUnknownClass;
typedef void (psUnknownClass::*psMETHOD_t)();

// ���Ϻ���ָ�����ͺ���ĳ�Ա����ָ������
union psUPtr
{
	psMETHOD_t   mthd;
	psFUNCTION_t func; 
};

//-------------------------------------------------------------------------------------------------------------------//
// ����������ֵַ�ĺ궨��
// 'f''��ʾ�����������ز�������(i.e. operator = )
// 'p' ��ʾ�����Ĳ���˵��(i.e. int , int)
// 'r' ��ʾ�����ķ���ֵ����(i.e. int )
// 'c' ��ʾ����
// 'm' ��ʾ��Ա�����������ز�������
// ����: ��C++����ȫ�ֺ���: void func();					���ֵַΪ: psFUNCTION(func)
//						    int func(int a, int b);			���ֵַΪ: psFUNCTIONPR(func, (int a, int b), int)
//							int operator + (int a, int b);  ���ֵַΪ: psFUNCITONPR(operator +, (int a, int b), int)
//							
//		��C++������: class CTest
//					 {
//						void func();						���ֵַΪ: psMETHOD(CTest, func)
//						int  func1(int a);					���ֵַΪ: psMETHODPR(CTest, func1,(int a), int)
//						int operator += (int a);			���ֵַΪ: psMETHODPR(CTest, operator+=, (int a), int)
//					 };
//-------------------------------------------------------------------------------------------------------------------//
#define psFUNCTION(f)		  psFunctionPtr((void (*)())(f))
#define psFUNCTIONPR(f,p,r)	  psFunctionPtr((void (*)())((r (*)p)(f)))
#define psMETHOD(c,m)		  psSMethodPtr<sizeof(void (c::*)())>::Convert((void (c::*)())(&c::m))
#define psMETHODPR(c,m,p,r)	  psSMethodPtr<sizeof(void (c::*)())>::Convert((r (c::*)p)(&c::m))
#define psBASEOFFSET(c, base) ( (int)( (base*)((c*)1) ) - 1 )  

#ifdef _DEBUG
	#define PS_VERIFY(e) assert(e >= 0)
#else
	#define PS_VERIFY(e) (e)
#endif

// ���Ҫ�ѽű��������Ϊ��̬���ӿ�Ҫ�����: PIXELSCRIPT_EXPORT
// �������ʾ���ӽű������,Ҫ�����: PIXELSCRIPT_DLL_LIBRARY_IMPORT 
// ����Ǿ�̬����,PS_API������Ϊ��
#ifdef WIN32
  #ifdef PIXELSCRIPT_STATIC
	#define PS_API
  #else
	#ifdef PIXELSCRIPT_EXPORTS
		#define PS_API __declspec(dllexport)
	#else
		#define PS_API __declspec(dllimport)
	#endif
  #endif
#else
  #define PS_API
#endif

// ���֧�ֱ�β�������
const int PS_MAX_VAR_ARG_SIZE = 32;		

//---------------------------------------------------------------------//
// ��ַ��ʽ
//----------------------------------------------------------------------//
const int PSBASE_NONE	  = 0;	// ������ֱ����Ϊֵ��ʹ��
const int PSBASE_CONST	  = 1;	// ��������¼�����ڳ������е�����
const int PSBASE_STACK	  = 2;	// ����������ڶ�ջ������
const int PSBASE_GLOBAL	  = 3;	// �����������ȫ�ֶ��������
const int PSBASE_HEAP	  = 4;  // ���������������������
const int PSBASE_UNKNOWN  = 5;  // 
//---------------------------------------------------------------

//----------------------------------------------------------------------//
// ���������Ż�ѡ��
//----------------------------------------------------------------------//
struct psSOptimizeOptions
{
	psSOptimizeOptions()
	{
		bDelDebugInstr		= 1;
		bCombinedInstr	    = 1;
		bDelUnnessnaryInstr = 1;
		bDelUnusedInstr	    = 0;
		bCreateSymbolTable  = 0;
		bUnused				= 0;
	}
	psDWORD bCreateSymbolTable  : 1;		// �Ƿ����ɷ��ű�(DEBUG��Ӧ��Ϊ��)
	psDWORD bDelDebugInstr	    : 1;		// �Ƿ�ɾ��������ص�ָ��(RELEASE��Ӧ��Ϊ��)
	psDWORD bCombinedInstr	    : 1;		// �Ƿ�ϲ�ָ��(RELEASE��Ӧ��Ϊ��)
	psDWORD bDelUnnessnaryInstr : 1;		// �Ƿ�ɾ������Ҫ��ָ��(RELEASE��Ӧ��Ϊ��)
	psDWORD bDelUnusedInstr		: 1;		// �Ƿ�ɾ��û���õ�ָ��(RELEASE��Ӧ��Ϊ��)
	psDWORD bUnused				: 27;
};

class psIDataType;
class psIScriptEngine;
class psIScriptContext;
class psIDebugger;

//-------------------------------------------------------------//
// ����: psIBuffer
// ˵��: �ڴ����ݽӿ�,�����ڲ�ͬ��ģ��䴫��
//-------------------------------------------------------------//
class psIBuffer
{
public:
	virtual psBYTE* GetBufferPointer()  = 0;
	virtual void	Release()			= 0;
	virtual void	AddRef()			= 0;
protected:
	virtual ~psIBuffer() {}
};

//----------------------------------------------------------------------//
// ȫ�ֺ���
//----------------------------------------------------------------------//
extern "C"
{
	// �����ű�����ӿ�
	psIScriptEngine * psCreateScriptEngine(psDWORD version);

	// �õ��ű�����汾���ַ���˵��
	const char* psGetLibraryVersion();

	// �����������ڴ��ȫ������(����VC++���ڴ���©)
	void psCleanup();

	// �õ���ǰ��Ľű�������
	psIScriptContext * psGetActiveContext();

	// �õ���������ķ�������
	// ����: 'bc'[in]: �����ƴ���
	//		 'size'[in] : �����ƴ���Ĵ�С
	//		 'outLineNumber'[out]: ����к���Ϣ, ���з�ʽ����:[�к�,λ��,�к�,λ��...]
	//		'outNumLines':����������������
	// ����ֵ: ��������
	psIBuffer* psGetDisassembleCode( const psBYTE* bc, size_t size, psIBuffer** outLineNumbers, int* outNumLines );

	// �õ���Ƿ����ַ�������
	const psCHAR* psGetTokenDefinition(int tokenType);
}

//-------------------------------------------------------------//
// ����: psAutoStringPtr
// ˵��: ���Զ�ɾ���ڴ���ַ���ָ��
//-------------------------------------------------------------//
class psAutoStringPtr
{
public:
	psAutoStringPtr( psIBuffer* p ) : m_pStringBuf(p)
	{
	}
	~psAutoStringPtr()
	{
		if ( m_pStringBuf )
		{
			m_pStringBuf->Release();
		}
	}
	psAutoStringPtr( const psAutoStringPtr& rhs )
	{
		m_pStringBuf = rhs.m_pStringBuf;
		m_pStringBuf->AddRef();
	}
	psAutoStringPtr& operator = (const psAutoStringPtr& rhs)
	{
		if ( this == &rhs )
			return *this;
		m_pStringBuf = rhs.m_pStringBuf;
		m_pStringBuf->AddRef();
		return *this;
	}
	operator const psCHAR* () 
	{
		return c_str();
	}
	const psCHAR* c_str() 
	{
		if ( m_pStringBuf )
			return (psCHAR*)(m_pStringBuf->GetBufferPointer());
		else
			return PS_T("");
	}
private:
	psIBuffer* m_pStringBuf;
};

//-------------------------------------------------------------------
// ����: psIOutputStream
// ˵��: ������Ľӿڶ���(�����������ʹ�����Ϣ)
//-------------------------------------------------------------------
class psIOutputStream
{
public:
	virtual void Write(const psCHAR* text) = 0;
	virtual void WriteWarning(const psCHAR* text, const psCHAR* filename, int row, int col) = 0;
	virtual void WriteError(const psCHAR* text, const psCHAR* filename, int row, int col) = 0;
protected:
	virtual ~psIOutputStream() {}
};

//-------------------------------------------------------------------
// ����: psITextStream
// ˵��: �ı����������
//-------------------------------------------------------------------
class psITextStream
{
public:
	virtual		bool	Open( const psCHAR* filename, const psCHAR* mode ) = 0;

	virtual		void    Close() = 0;

	virtual		long	GetLength() = 0;

	virtual		long    Read( psCHAR* buf, long length ) = 0;

	virtual		long    Write( psCHAR* buf, long length ) = 0;
protected:
	virtual ~psITextStream() {}
};

//-------------------------------------------------------------------
// ����: psIBinaryStream
// ˵��: ���������Ľӿڶ���(���ڱ�������)
//-------------------------------------------------------------------
class psIBinaryStream
{
public:
	virtual void Read(void* ptr, unsigned int size) = 0;
	virtual void Write(const void* ptr, unsigned int size) = 0;
protected:
	virtual ~psIBinaryStream() {}
};


//-------------------------------------------------------------------
// ����: psIProperty
// ˵��: ���Խӿ�(�п�����ȫ������,Ҳ�п���Ϊ���Ա������
//-------------------------------------------------------------------
class psIProperty
{
public:
	// �õ�������
	virtual const psCHAR* GetName() const = 0;

	// �õ�����ƫ��
	virtual int GetOffset() const = 0;

	// �õ����Ե���������
	virtual const psIDataType* GetDataType() const = 0;
protected:
	virtual ~psIProperty() {}
};

//-------------------------------------------------------------------
// ����: psITypeBehaviour
// ˵��: �����Ϊ�����ӿ�
//-------------------------------------------------------------------
class psITypeBehaviour
{
public:
	// �õ����캯���ĸ���
	virtual unsigned int GetNumConstructFuncs() const = 0;

	// �õ����캯����ID
	virtual int GetConstructFuncId(unsigned int index) const = 0;

	// �õ�Ĭ�ϵĹ��캯��ID		
	virtual bool HasConstructor() const	= 0;
	virtual int GetConstructFuncId() const = 0;

	// �õ�Ĭ�ϵĿ������캯��ID	
	virtual bool HasCopyConstructor() const	   = 0;
	virtual int GetCopyConstructFuncId() const = 0;

	// �õ�����������ID	
	virtual bool HasDestructor() const	  = 0;
	virtual int GetDestructFuncId() const = 0;

	// �õ������صĲ������ĸ���
	virtual unsigned int GetNumOperators() const = 0;

	// �õ����صĲ������ı�Ƿ�
	virtual int GetOperatorToken(unsigned int index) const = 0;

	// �õ����ز������ĺ���ID
	virtual int GetOperatorFuncId(unsigned int index) const = 0;
protected:
	virtual ~psITypeBehaviour() {}
};

//-------------------------------------------------------------------
// ����: psIObjectType
// ˵��: ��������ͽӿ� 
//-------------------------------------------------------------------
class psIObjectType
{
public:
	// �õ�����
	virtual const psCHAR* GetName() const = 0;

	// �õ���������ڴ��еĴ�С
	virtual int GetSizeInMemoryBytes() const = 0;

	// �õ��෽���ĸ���
	virtual unsigned int GetNumMethods() const = 0;

	// �õ��෽����ID
	virtual int GetMethodFuncId(unsigned int index) const = 0;

	// �õ����Ա���Եĸ���
	virtual unsigned int GetNumProps() const = 0;

	// �õ����Ա����
	virtual const psIProperty* GetProperty(unsigned int index) const = 0;

	// �õ������Ϊ�����ӿ�
	virtual const psITypeBehaviour* GetTypeBehaviour() const = 0;

	// �Ƿ�Ϊ�ű��������
	virtual bool IsScriptObject() const = 0;

	// �õ���������
	virtual psIObjectType* GetParent() const = 0;

	// ���ൽ�����ƫ��
	virtual int GetBaseOffset() const = 0;

	// �õ���������
	virtual const psIDataType* GetDataType() const = 0;
protected:
	virtual ~psIObjectType() {}
};

//-------------------------------------------------------------------
// ����: psIDataType
// ˵��: �������ͽӿ�
//-------------------------------------------------------------------
class psIDataType
{
public:
	// �õ�����������͵���������ͽӿ�
	virtual const psIObjectType* GetObjectType() const = 0;
	
	// �õ�������͵��ַ���˵��
	virtual psAutoStringPtr GetFormatString(const psCHAR* varname) const = 0;

	// �Ƿ�Ϊָ������
	virtual bool IsPointer()	 const = 0;

	// �Ƿ���Ҫ������
	virtual bool IsReference() const = 0;

	// �Ƿ�Ϊֻ����
	virtual bool IsReadOnly() const = 0;

	// �Ƿ�Ϊ������������
	virtual bool IsPrimitive() const = 0;

	// �Ƿ�Ϊ���������
	virtual bool IsObject() const = 0;

	// �Ƿ�Ϊ����
	virtual bool IsIntegerType() const = 0;

	// �Ƿ�Ϊ������
	virtual bool IsFloatType() const = 0;

	// �Ƿ�Ϊ������(˫����)
	virtual bool IsDoubleType() const = 0;

	// �Ƿ�Ϊ'bool'��
	virtual bool IsBooleanType() const = 0;

	// �Ƿ�Ϊ��������
	virtual bool IsArrayType() const = 0;

	// �õ����ݵ�ά��
	virtual int  GetArrayDim() const = 0;

	// �õ�ĳһά����Ĵ�С
	virtual int  GetArraySize(int dim) const = 0;

	// �õ����ڶ�ջ����ռ�õ�˫�ֽ���
	virtual int  GetSizeOnStackDWords()  const = 0;

	// �õ������ڴ���ռ�õ��ֽ���
	virtual int  GetSizeInMemoryBytes()  const = 0;
protected:
	virtual ~psIDataType() {}
};

//-------------------------------------------------------------------
// ����: psIScriptFunction
// ˵��: �ű������ӿ�
//-------------------------------------------------------------------
class psIScriptFunction
{
public:
	// �õ�������
	virtual const psCHAR* GetName() const = 0;

	// �õ�Դ�ļ�
	virtual const psCHAR* GetSourceFile() const = 0;

	// �õ������ƴ���
	virtual psIBuffer* GetByteCode(int& outsize) const = 0;

	// �õ�����ID(NOTE: ����ID��һ������������ֵ)
	virtual const int GetID() const = 0;

	// �õ��������ַ���˵��
	virtual	psAutoStringPtr GetDeclarationString() const = 0;

	// �õ�����ֵ������
	virtual const psIDataType* GetReturnType() const = 0;

	// �õ�������������������(�����ȫ�ֺ����䷵��ֵΪ��)
	virtual const psIObjectType* GetObjectType() const = 0;

	// �õ���������Ĳ�������
	virtual const unsigned int GetNumParams() const = 0;

	// �õ���������Ĳ�������
	virtual const psIDataType* GetParamType(int index) const = 0;

	// ���кŵõ����������λ��
	virtual int GetPosFromLineNumber(int lineno, int& realLineno) const = 0;

	// �Ӷ��������λ�õõ�Դ������к�
	virtual	int GetLineNumber(int pos) const = 0;

	// �õ���С���к�
	virtual int GetMinLineNumber() const = 0;

	// �õ������к�
	virtual int GetMaxLineNumber() const = 0;
protected:
	virtual ~psIScriptFunction() {}
};

//-------------------------------------------------------------------
// ����: psIModule
// ˵��: �ű�ģ��ӿ�
//-------------------------------------------------------------------
class psIModule
{
public:
	// ���ýű�ģ��
	// �൱�ڰ����ģ�������������
	virtual void Reset() = 0;

	// �Ƿ��������
	virtual bool CanReset() const  = 0;

	// �õ�ģ����
	virtual const psCHAR* GetName() const = 0;

	// �õ��ļ���
	virtual const psCHAR* GetFileName() const = 0;

	// �õ�ģ��ID
	virtual int GetID() const = 0;

	// ���һ���ű������
	// ����: 'code'[in]: Դ����
	//		 'lenght'[in]: Դ�볤��
	// ����ֵ: psSUCCESS: �ɹ�
	//		   psERROR: ʧ��
	virtual int SetCode(const psCHAR* code, int length) = 0;

	// ��ȡԴ����
	virtual const psCHAR* GetSourceCode() const = 0;

	// �õ�ģ���еĺ�������
	virtual unsigned int GetFunctionCount() const = 0;

	// ���ݺ���ID. �õ��ű������ӿ�
	// ����: 'nID'[in]:���ű���������ID
	// ����ֵ: �ű������ӿ�ָ��, ���ֵ����ΪNULL
	virtual const psIScriptFunction* GetFunctionByID(int nID) const = 0;

	// �õ���ʼ�����˳�����
	virtual const psIScriptFunction* GetInitFunction() const = 0;
	virtual const psIScriptFunction* GetExitFunction() const = 0;

	// ���ݺ������ƣ� �õ��ű�������ID
	// ����: 'name'[in]:���ű���������������
	// ����ֵ: �ű�������ID(���ڵ���0�ɹ�)
	//		   psMULTIPLE_FUNCTIONS: �ҵ��˶��ƥ��ĺ���
	//		   psNO_FUNCTION: û���ҵ�ƥ��ĺ���
	//		   psERROR: ��������ű�ģ���ʱ��ʧ��
	virtual int GetFunctionIDByName(const psCHAR* name) const = 0;

	// ���ݺ�������. �õ��ű�������ID
	// ����: 'decl'[in]:���ű���������������
	// ����ֵ: �ű�������ID(���ڵ���0�ɹ�)
	//		   psINVALID_DECLARATION: ��Ч������
	//		   psMULTIPLE_FUNCTIONS: �ҵ��˶��ƥ��ĺ���
	//		   psNO_FUNCTION: û���ҵ�ƥ��ĺ���
	//		   psERROR: ��������ű�ģ���ʱ��ʧ��
	virtual int GetFunctionIDByDecl(const psCHAR* decl) const = 0;

	// �õ������ĸ���
	virtual unsigned int GetObjectTypeCount() const = 0;

	// ��������������õ������
	// ����: 'index': ����������ڽű�ģ���е�����
	// ����ֵ: ��������ͽӿ�(������Ϊ��)
	virtual const psIObjectType* GetObjectType(unsigned int index) const = 0;

	// ������������Ƶõ������
	// ����: 'name': ����������
	// ����ֵ: '��������ͽӿ�(��ֵ�п���Ϊ��)
	virtual const psIObjectType* GetObjectType(const psCHAR* name) const = 0;

	// �õ�ȫ�ֱ���������
	virtual unsigned int GetGlobalVarCount() const = 0;

	// �õ�ȫ�ֱ�������
	// ����: 'index': ȫ�������ڽű�ģ���е�����
	// ����ֵ: ȫ�ֱ������Խӿ�(������Ϊ��)
	virtual const psIProperty* GetGlobalVar(int index) const = 0;

	// �����������
	// ����: 'out': �����������Ķ�������
	// ����ֵ: psSUCESS��  sdf
	virtual int SaveByteCode(psIBinaryStream* out) const = 0;

	// װ�ض�������
	// ����: 'in': �����������Ķ�������
	virtual int LoadByteCode(psIBinaryStream* in) = 0;

	// �л��ϵ�
	// ����: 'lineno':�ϵ���к�
	//		 'hasBreakPoint'��ʱ�����Ƿ��жϵ�
	// ����ֵ: true: �ɹ� false: ʧ��
	virtual bool ToggleBreakPoint( int lineno, bool& hasBreakPoint) = 0;

	// �����û��ϵ�
	// ����: 'lineno':�ϵ���к�
	// ����ֵ: true: �ɹ� false: ʧ��
	virtual bool InsertBreakPoint( int lineno) = 0;

	// �Ƴ��û��ϵ�
	// ����: 'lineno':�ϵ���к�
	// ����ֵ: true: �ɹ� false: ʧ��
	virtual bool RemoveBreakPoint( int lineno) = 0;

	// �Ƴ����е��û��ϵ�
	// ����: 'lineno':�ϵ���к�
	// ����ֵ: true: �ɹ� false: ʧ��
	virtual void RemoveAllBreakPoint() = 0;

	// �õ����еĶϵ��к���Ϣ
	virtual psIBuffer* GetAllBreakPoint( int* numBreakPoints ) = 0;
protected:
	virtual ~psIModule() {}
};

//-------------------------------------------------------------------
// �ű����ú���
//-------------------------------------------------------------------
typedef bool (PS_STDCALL * PS_CONFIG_ENGINE_FUNC)(psIScriptEngine* engine);
typedef bool (PS_STDCALL * PS_ARGTYPE_CHECK_FUNC)(const psIScriptFunction* func, const psIDataType** argTypes, const char** constArgs, size_t numArg);

typedef psAutoStringPtr (PS_STDCALL * PS_GET_STRING_VALUE_FUNC)(void* obj);
typedef void (PS_STDCALL * PS_SET_STRING_VALUE_FUNC)(void* obj, const psCHAR* value);

#define PS_REGISTER_ENGINE_FUNC_NAME    "psRegisterEngine"
#define PS_UNREGISTER_ENGINE_FUNC_NAME  "psUnregisterEngine"

//-------------------------------------------------------------------
// ����: psIScriptEngine
// ˵��: �ű�����ӿ�
//-------------------------------------------------------------------
class psIScriptEngine
{
public:
	// �ڴ����
	virtual long AddRef() = 0;
	virtual long Release() = 0;

	// ע��һ��C++��
	// ����: 'name':	 �ڽű���ʹ�õ�����
	//		 'byteSize': ��C++�е���ʵ��С
	//		 'flags'   : �����Ϊ��־.
	// ����ֵ: psINVALID_ARG: ����˺����Ĳ�����Ч.
	//		   psINVALID_NAME: 'name'��ֵΪ��, ������Ч
	//		   psCLASS_EXIST:  �������Ѿ���ע���
	//		   psNAME_TAKEN:  ���������������Ƴ�ͻ
	//		   psSUCCESS:	  �ɹ�
	virtual int RegisterObjectType(const psCHAR* name, int byteSize, psDWORD flags) = 0;

	// ע��C++��ĳ�Ա����
	// ����: 'objname': �ڽű���ʹ�õ�����
	//		 'declaration': ��������
	//		 'byteoffset' : ������C++���ƫ��
	// ����ֵ: psINVALID_OBJECT: 'objname'�������Ч
	//		   psSUCCESS: �ɹ� 
	//		   ����: 'declaration' ��Ч		
	virtual int RegisterObjectProperty(const psCHAR* objname, const psCHAR* declaration, int byteOffset) = 0;
 
	// ע��C++��ĳ�Ա����
	// ����: 'objname': �ڽű���ʹ�õ�����
	//		 'declaration': ���󷽷�����
	//		 'funcPointer': ������ֵַ
	//		 'callConv':    ���÷�ʽ
	// ����ֵ: psINVALID_OBJECT: 'objname'�������Ч
	//		   psNOT_SUPPORTED:  'callConv'ָ���ĵ��÷�ʽ����֧��
	//		   psAPP_CANT_INTERFACE_DEFAULT_ARRAY: 'declaration'�еĲ����ͷ���ֵ����Ϊ��������
	//		   psNAME_TAKEN:    ���������������Ƴ�ͻ
	//		   psINVALID_DECLARATION: 'declaration' ��Ч
	//		   psSUCCESS: �ɹ� 
	virtual int RegisterObjectMethod(const psCHAR* objname, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv) = 0;

	// ע��C++�����Ϊ
	// ����: 'objname': �ڽű���ʹ�õ�����
	//		 'behaviour': ��������Ϊ���Ե�����(e.g: psBEHAVE_CONSTRUCT)
	//		 'declaration': ���󷽷�����
	//		 'funcPointer': ������ֵַ
	//		 'callConv':    ���÷�ʽ
	// ����ֵ: psINVALID_OBJECT: 'objname'�������Ч
	//		   psNOT_SUPPORTED:  'callConv'ָ���ĵ��÷�ʽ����֧��
    //		   psAPP_CANT_INTERFACE_DEFAULT_ARRAY: 'declaration'�еĲ����ͷ���ֵ����Ϊ��������
	//		   psINVALID_DECLARATION: 'declaration' ��Ч
	//		   psALREADY_REGISTERED: �������Ϊ�����Ѿ���ע���
	//		   psSUCCESS: �ɹ�
	virtual int RegisterObjectBehaviour(const psCHAR* objname, psDWORD behaviour, const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv) = 0;

	// ע��һ��ȫ�ֱ���
	// ����: 'declaration' : ȫ�ֱ���������
	//		 'pointer' :	 Ϊ���ֵַ
	// ����ֵ: psINVALID_TYPE: ��Ч����������
	//		   psSUCCESS:      �ɹ�
	//		   ����:		   ��Ч������	
	virtual int RegisterGlobalProperty(const psCHAR* declaration, void* pointer) = 0;

	// ע��һ�����ͳ���
	virtual int RegisterIntConstant(const psCHAR* name, int v) = 0;

	// ע��һ�����㳣��
	virtual int RegisterFloatConstant(const psCHAR* name, float v) = 0;

	// ע��һ��ȫ�ֺ���
	// ����: 'declaration': ��������
	//		 'funcPointer': ������ֵַ
	//		 'callConv': ���÷�ʽ
	// ����ֵ: psINVALID_ARG: ����˺����Ĳ�����Ч.
	//		   psNOT_SUPPORTED: 'callConv'ָ���ĵ��÷�ʽ����֧��
	//		   psINVALID_DECLARATION: 'declaration' ��Ч
	//		   psAPP_CANT_INTERFACE_DEFAULT_ARRAY: 'declaration'�еĲ����ͷ���ֵ����Ϊ��������
	//		   psNAME_TAKEN:    ���������������Ƴ�ͻ
	//		   psSUCCESS: �ɹ�
	virtual int RegisterGlobalFunction(const psCHAR* declaration, psUPtr funcPointer, psDWORD callConv) = 0;

	// ע��һ��ȫ�ֲ���������('behavior'Ϊ������������, 'decl'Ϊ������, 'funcPointer'Ϊ�亯����ֵַ, 'callConv'��ʾ����÷�ʽ)
	// ����: 'behaviour': ȫ����Ϊ��������(e.g:psBEHAVE_ADD)
	//		 'decl': ȫ�ֺ���������
	//		 'funcPointer': ������ֵַ
	//		 'callConv': ���÷�ʽ
	// ����ֵ: psNOT_SUPPORTED: 'callConv'ָ���ĵ��÷�ʽ����֧��
	//		   psINVALID_DECLARATION: 'declaration' ��Ч
	//		   psAPP_CANT_INTERFACE_DEFAULT_ARRAY: 'declaration'�еĲ����ͷ���ֵ����Ϊ��������
	//		   psNAME_TAKEN:    ���������������Ƴ�ͻ
	virtual int RegisterGlobalBehaviour(psDWORD behaviour, const psCHAR* decl, psUPtr funcPointer, psDWORD callConv) = 0;

	// ������ļ̳й�ϵ
	// ����: 'name'����
	//		 'parent'������
	//		 'baseOffset'C++�����ൽ�����ƫ��
	virtual int DeclareObjectHierarchy(const psCHAR* name, const psCHAR* parent, int baseOffset ) = 0 ;

	// ע��һ��C++��
	// ����: 'name': ����
	// ����ֵ: psSUCCESS: �ɹ�
	//		   psERROR: ʧ��
	virtual int UnregisterObjectType( const psCHAR* name ) = 0;

	// ע��һ��ȫ�ֱ���
	// ����: 'ptr': ȫ�ֱ�����ָ��
	// ����ֵ: psSUCCESS: �ɹ�
	//		   psERROR: ʧ��
	virtual int UnregisterGlobalProperty(void* ptr)	= 0;

	// ע��һ��ȫ�ֺ���
	// ����: 'funcPtr': ȫ�ֺ�����ָ��
	// ����ֵ: psSUCCESS: �ɹ�
	//		   psERROR: ʧ��
	virtual int UnregisterGlobalFunction( psUPtr funcPtr ) = 0;

	// ע��һ��ȫ�ֲ���������
	// ����: 'ptr': ȫ�ֺ�����ָ��
	// ����ֵ: psSUCCESS: �ɹ�
	//		   psERROR: ʧ��
	virtual int UnregisterGlobalBehaviour( psUPtr funcPtr ) = 0;

	// ��ȡ��������͵ĸ���
	virtual unsigned int GetObjectTypeCount() const = 0;

	// ��ȡ���е����������
	virtual void GetAllObjectTypes(psIObjectType* objectTypes[]) const = 0;

	// �������Ƶõ�ע���C++������ͽӿ�, ����������ܻ�ʧ��
	// ����: 'name': ����������
	// ����ֵ: �����ӿ�, ��ֵ����Ϊ��
	virtual const psIObjectType* FindObjectType(const psCHAR* name) const = 0;

	// ���ݺ���ID,�õ�ϵͳ�����ӿ�(NOTE:ϵͳ������ID��С��0)
	// ����: 'nID': ����ID
	// ����ֵ: �ű������ӿ�, ��ֵ����Ϊ��
	virtual const psIScriptFunction* GetSystemFunction(int nID) const = 0;

	// �õ�ϵͳȫ�ֺ����ĸ���
	virtual unsigned int GetGlobalFunctionCount() const = 0; 

	// ���ݺ�������,�õ�ϵͳȫ�ֺ����ӿ�
	// ����: 'index': ȫ�ֺ����ڽű������е�����
	// ����ֵ: �ű������ӿ�(��ֵ������Ϊ��)
	virtual const psIScriptFunction* GetGlobalFunction(unsigned int index) const = 0;

	// �õ�ȫ�����Եĸ���
	virtual unsigned int GetGlobalPropCount() const = 0;

	// ��ȡ���е�ȫ������
	virtual void GetAllGlobalProps( psIProperty* pAllGlobalProps[] ) const = 0;

	//  �������Ƶõ�ȫ�ֱ������Խӿ�
	virtual const psIProperty* FindGlobalProp(const psCHAR* name) const = 0;

	// �õ��ű�ģ�������
	virtual unsigned int GetScriptModuleCount() const = 0; 

	// ���������õ��ű�ģ��ӿ�
	// ����: 'index' : �ű�ģ���ڽű������е�����
	// ����ֵ: �ű�ģ��ӿ�(�䲻����Ϊ��)
	virtual psIModule* GetModule(unsigned int index) const = 0;

	// ����ģ�����Ƶõ��ű�ģ��ӿ�
	// ����: 'name' : �ű�ģ�������
	// ����ֵ: �ű�ģ��ӿ�, ��ֵ����Ϊ��
	virtual psIModule* GetModule(const psCHAR* name) const = 0;

	// ����һ���ű�ģ��ָ��
	// ����: 'name'[in] �ű�ģ�������
	//		 'filename'[in]: Դ�ļ�·��
	//		 'mod'[out]���ڷ��ؽű�ģ���ָ���ַ.
	// ����ֵ: psSUCCESS: �ɹ�
	//		   psERROR: ʧ��
	// 		   psMODULE_EXIST : ģ���Ѿ�����, ����ʱ'ppMod'��������ģ���ָ��.
	virtual int CreateModule( const psCHAR* name, const psCHAR* filename, psIModule** mod ) = 0;

	// �������еĽű�ģ��
	virtual void Reset() = 0;

	// ����һ���ű�ģ��
	virtual int Discard(const psCHAR* module) = 0;

	// ���ô����Ż�ѡ��
	virtual void SetOptimizeOptions( psSOptimizeOptions options ) = 0;

	// �õ������Ż�ѡ��
	virtual psSOptimizeOptions GetOptimizeOptions() const = 0;

	// ����һ���ű�ģ��
	// ����: 'mod': �ű�ģ��ӿ�.
	//		 'pTextStream': ���ڷ����ı��ļ��Ľӿ�
	//		 'pOutStream': �������������Ϣ�Ľӿ�
	// ����ֵ: sSUCCESS: �ɹ�
	//		   psINVALID_CONFIGURATION: ����������Ч
	//		   psMODULE_CANT_BE_RESET: ģ�鲻�ܱ�����: 
	virtual int Compile( psIModule* mod, psITextStream* pTextStream, psIOutputStream* pOutStream ) = 0;

	// ִ��һ���ַ���
	// ����: 'ctx': ����ִ�нű��Ķ��������Ľӿ�, �����Ϊ��,��ô�ű�ʹ�ýű������Ĭ�Ͻű������Ľӿ�
	//		  Ҫע���������Ƕ��̵߳�, Ҫ��֤ÿ���߳��ò�ͬ�Ľű������Ľӿ�.
	//		 'script': �ַ���Դ��
	//		 'pOutStream':  �������������Ϣ�Ľӿ�
	virtual int ExecuteString(psIScriptContext* ctx, const psCHAR *script, psIOutputStream* pOutStream ) = 0;

	// ����Ĭ�ϵĶ�ջ��С
	// ����: 'initial': ��ʼ��ջ��С
	//		 'maximum': ���Ķ�ջ��С, ���Ϊ0��ʾû������
	// ����ֵ: ��
	virtual void SetDefaultContextStackSize(psUINT initial, psUINT maximum) = 0;

	// �����ű����������Ľӿ�
	// ����: 'context':ָ��ű����������Ľӿڵ�ָ���ַ
	// ����ֵ: psSUCCESS: �����ɹ�
	//		   psERROR : ����ʧ��			
	virtual int CreateContext(psIScriptContext **contex) = 0;

	// ���õ�����
	// ����: 'debugger' �������ӿ�
	// ����ֵ: ��
	virtual void SetDebugger( psIDebugger* debugger ) = 0;

	// ע��һ���ַ���������(  )
	// ����: 'dataType': �ַ���������
	//		 'getFunc'Ϊ��δ�һ�������õ��ַ���ֵ�ĺ���ָ��.
	//		 'setFunc'Ϊ��δ�һ���ַ���ָ�븳ֵ
	// ����ֵ: psSUCCESS: �ɹ�
	//		   psINVALID_TYPE: ��Ч������
	virtual int  RegisterStringFactory(const psCHAR* dataType, PS_GET_STRING_VALUE_FUNC getFunc, PS_SET_STRING_VALUE_FUNC setFunc) = 0 ;

	// �ж�һ�����������Ƿ�Ϊ�ַ�������
	// ����: 'objType': ��������ͽӿ�
	virtual bool IsStringType(const psIObjectType* objType) const = 0;

	// ����һ���ַ���������ֵ
	// ����: 'obj': �ַ��������ָ��.
	//		 'value': �ַ�����ֵ
	// ����ֵ: psSUCCESS: �ɹ�
	//		   psERROR:   ʧ��
	virtual bool  SetStringValue(const psIObjectType* objType, void* obj, const psCHAR* value) = 0;

	// �õ�һ���ַ���������ֵ
	// ����: 'obj': �ַ��������ָ��.
	// ����ֵ: NULL: ʧ��, ����Ϊ�ַ�����ֵ.
	virtual psAutoStringPtr GetStringValue(const psIObjectType* objType, void* obj) const = 0;

	// ����һ�������
	// ����: 'moduleId': �ű�ģ��ID(�����ϵͳ�������ֵΪ-1)
	//		 'objType':  ��������ͽӿ�
	// ����ֵ: NULL: ʧ��, ����Ϊ������ָ��
	virtual void* AllocObject( int moduleId, const psIObjectType* objType ) = 0;

	// �ͷ�һ�������
	// ����: 'obj': ������ָ��
	//		 'moduleId': �ű�ģ��ID(�����ϵͳ�������ֵΪ-1)
	//		 'objType':  ��������ͽӿ�
	// ����ֵ: true: �ɹ�. false: ʧ��
	virtual bool FreeObject( void* obj, int moduleId, const psIObjectType* objType  ) = 0;

	// ���ú궨���б��ַ���
	// ����: 'macros':   ���еĺ궨��(e.g."_DEBGU; UNICODE" )
	//		 'sperator': �ָ��ַ�
	// ����ֵ: true: �ɹ�. false: ʧ��
	virtual bool SetMacroList(const psCHAR* macros, psCHAR sperator) = 0;

	// �õ��궨���б��ַ���
	// ����ֵ: Ϊ�궨���ַ���ָ��(�䲻����Ϊ��)
	virtual const psCHAR* GetMacroList() = 0;

	// ���ñ�κ������ͼ��ص�����
	virtual void SetArgTypeCheckFunc( PS_ARGTYPE_CHECK_FUNC pFunc ) = 0;

	// �õ���κ������ͼ��ص�����
	virtual PS_ARGTYPE_CHECK_FUNC GetArgTypeCheckFunc() = 0;
protected:
	virtual ~psIScriptEngine() {}
};

//-------------------------------------------------------------------
// ����: psIScriptContext
// ˵��: �ű����������Ľӿ�
//-------------------------------------------------------------------
class psIScriptContext
{
public:
	// �ڴ����
	virtual long AddRef() = 0;
	virtual long Release() = 0;

	// �õ�����ָ��
	virtual psIScriptEngine *GetEngine() = 0;

	// �õ���ǰ��������״̬
	// ����ֵ: psEXECUTION_SUSPENDED: ������
	//		   psEXECUTION_ACTIVE :   �
	//         psEXECUTION_EXCEPTION: �쳣
	//		   psEXECUTION_FINISHED:  ���н���
	//		   psEXECUTION_PREPARED:  ׼������
	//		   psEXECUTION_UNINITIALIZED: δ��ʼ��
	//		   psERROR: δ֪����
	virtual int GetState() = 0;

	// ׼��ִ��һ���ű�����
	// ����: 'moduleID'[in] : �ű�ģ��ID(���Ϊ-1,��ʾ�ٴ�׼��������һ�εĺ���)
	//		 'funcID'[in]: �ű�����ID(���Ϊ-1,��ʾ�ٴ�׼��������һ�εĺ���)
	//		 'obj'[in]:   �����ָ��,��������һ����ķ�����ô��Ϊ�����ָ��
	//					  ���Ϊһ��ȫ�ֺ���,��Ӧ��Ϊ��.
	// ����ֵ: psCONTEXT_ACTIVE: ��ǰ�ű����������Ѿ��ǻ��.
	//		   psNO_FUNCTION: ��ȡ����ָ��ʧ��
	//		   psNO_MODULE: ��ȡ�ű�ģ��ʧ��
	//		   psIVALID_OBJECT_POINTER: 'obj'��ֵ��Ч
	//		   psSUCCESS: �ɹ�
	//		   psERROR: ʧ��
	virtual int Prepare(int moduleID, int funcID, void* obj) = 0;

	// ���ò�����ֵ
	// ����: 'arg[in]': ����������(��0��ʼ)
	//		 'value'[in]: ������ֵ
	// ����ֵ: psCONTEXT_NOT_PREPARED: �ű�������û��׼�����к���
	//		   psINVALID_ARG: ��Ч���������
	//		   psINVALID_TYPE: �������Ͳ�ƥ��
	//		   psSUCESS: �ɹ�
	////////////////////////////////////////////////////////////////////
	//		   NOTE:��������������ʧ��,�Ժ����'Execute'����ʱҲ��ʧ��.
	virtual int SetArgDWord(psUINT arg, psDWORD value) = 0;
	virtual int SetArgFloat(psUINT arg, float value) = 0;
	virtual int SetArgDouble(psUINT arg, double value) = 0;
	virtual int SetArgPointer(psUINT arg, void* ptr) = 0;

	// �õ������ķ���ֵ
	virtual psDWORD GetReturnDWord() = 0;
	virtual float   GetReturnFloat() = 0;
	virtual double  GetReturnDouble() = 0;
	virtual void*   GetReturnPointer() = 0;

	// ���к���
	// exeFlag: psEXEC_DEBUGΪ���Զ���(�����õ������Ļص�����)
	//			psEXEC_STEP_INTOΪ��������, 
	//			psEXEC_STEP_OVERΪ����ִ��
	//			0Ϊ����ִ��
	// ����ֵ: Ϊ��ǰ�����ĵ�״̬(psEXECUTION_FINISHED ... )
	virtual int Execute( int exeFlag ) = 0;

	// ���к�����һ�����ָ��
	// ����ֵ: Ϊ��ǰ�����ĵ�״̬(psEXECUTION_FINISHED ... )
	virtual int ExecuteInstr() = 0;

	// �˳�����
	virtual int Abort() = 0;

	// �ͷŶԽű�ģ�������
	virtual void ReleaseModuleRef() = 0;

	// ���ó�ʼ����
	virtual int SetInitFunction( int modId, int funcId ) = 0;

	// �õ��쳣�������к�
	virtual int GetExceptionLineNumber() = 0;

	// �õ��쳣�����ĺ���ID
	virtual int GetExceptionFunctionId() = 0;

	// �õ������쳣��ģ��ID
	virtual int GetExceptionModuleId()   = 0;

	// �õ��쳣���ַ���˵��
	virtual const psCHAR* GetExceptionString() = 0;

	// �õ���ǰ���ڵ��к�
	virtual int GetCurrentLineNumber() = 0;

	// �õ���ǰ���ڵĶ��������е�λ��
	virtual int GetCurrentBytePos() = 0;

	// ���ҵ��Է���
	virtual const psIDataType* FindDebugSymbol(const psCHAR* name, psDWORD* addr) const = 0;

	// �õ���ǰ�Ľű�ģ��
	virtual psIModule* GetCurrentModule() = 0;

	// �õ���ǰ�Ľű������ӿ�
	virtual psIScriptFunction* GetCurrentFunction()  = 0;

	// �õ���ʼ�Ľű������ӿ�
	virtual psIScriptFunction* GetInitFunction()  = 0;

	// �õ���ǰ����ջ���ַ���˵��
	virtual psAutoStringPtr GetCallStackString() const = 0;

	// �����쳣
	virtual int SetException(const psCHAR *descr) = 0;

protected:
	virtual ~psIScriptContext() {};
};

//-------------------------------------------------------------------
// ����: psIDebugger
// ˵��: �ű��������ӿ�
//-------------------------------------------------------------------
class psIDebugger
{
public:
	// �������Ļص��¼�
	virtual int OnDebugEvent(psIScriptContext* ctx) = 0;
	
	// �ڴ���һ���ű�ģ��ʱ֪ͨ������
	virtual void OnCreateModule( psIModule* mod ) = 0;

	// ��ɾ��һ���ű�ģ��ʱ֪ͨ������
	virtual void OnDeleteModule( psIModule* mod ) = 0;

	// �ڱ���һ���ű�ģ��ʱ֪ͨ������
	virtual void OnCompileModule( psIModule* mod, int r ) = 0;
protected:
	virtual ~psIDebugger() {};
};

//----------------------------------------------------------------------//
// ��������
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// ���÷�ʽԼ��
//----------------------------------------------------------------------//
const psDWORD psCALL_CDECL            = 0;
const psDWORD psCALL_STDCALL          = 1;
const psDWORD psCALL_THISCALL         = 2;
const psDWORD psCALL_CDECL_OBJFIRST   = 4;

//----------------------------------------------------------------------//
// ���־
// ˵��һ��C++�����Ϊ��־.
// i.e. ���һ����ֻ�й��캯��,��ô���־Ϊ: psOBJ_CLASS_CONSTRUCTOR
// i.e. ���һ�������й��캯��,������������,���и�ֵ����,��ô���־Ϊ: psOBJ_CLASS_CDA
//----------------------------------------------------------------------//
const psDWORD psOBJ_CLASS             = 1;
const psDWORD psOBJ_INTERFACE         = 2;
const psDWORD psOBJ_CLASS_CONSTRUCTOR = 4;
const psDWORD psOBJ_CLASS_DESTRUCTOR  = 8;
const psDWORD psOBJ_CLASS_ASSIGNMENT  = 16;
const psDWORD psOBJ_PRIMITIVE		  = 32;
const psDWORD psOBJ_CLASS_MAX		  = 65;

const psDWORD psOBJ_CLASS_C           = (psOBJ_CLASS + psOBJ_CLASS_CONSTRUCTOR);
const psDWORD psOBJ_CLASS_CD          = (psOBJ_CLASS + psOBJ_CLASS_CONSTRUCTOR + psOBJ_CLASS_DESTRUCTOR);
const psDWORD psOBJ_CLASS_CA          = (psOBJ_CLASS + psOBJ_CLASS_CONSTRUCTOR + psOBJ_CLASS_ASSIGNMENT);
const psDWORD psOBJ_CLASS_CDA         = (psOBJ_CLASS + psOBJ_CLASS_CONSTRUCTOR + psOBJ_CLASS_DESTRUCTOR + psOBJ_CLASS_ASSIGNMENT);
const psDWORD psOBJ_CLASS_D           = (psOBJ_CLASS + psOBJ_CLASS_DESTRUCTOR);
const psDWORD psOBJ_CLASS_A           = (psOBJ_CLASS + psOBJ_CLASS_ASSIGNMENT);
const psDWORD psOBJ_CLASS_DA          = (psOBJ_CLASS + psOBJ_CLASS_DESTRUCTOR + psOBJ_CLASS_ASSIGNMENT);

//----------------------------------------------------------------------//
// �����Ϊ����
//----------------------------------------------------------------------//
 const psDWORD psBEHAVE_CONSTRUCT     = 0;
 const psDWORD psBEHAVE_DESTRUCT      = 1;

 const psDWORD psBEHAVE_FIRST_ASSIGN  = 2;
 const psDWORD psBEHAVE_ASSIGNMENT    = 2;
 const psDWORD psBEHAVE_ADD_ASSIGN    = 3;
 const psDWORD psBEHAVE_SUB_ASSIGN    = 4;
 const psDWORD psBEHAVE_MUL_ASSIGN    = 5;
 const psDWORD psBEHAVE_DIV_ASSIGN    = 6;
 const psDWORD psBEHAVE_MOD_ASSIGN    = 7;
 const psDWORD psBEHAVE_OR_ASSIGN     = 8;
 const psDWORD psBEHAVE_AND_ASSIGN    = 9;
 const psDWORD psBEHAVE_XOR_ASSIGN    = 10;
 const psDWORD psBEHAVE_SLL_ASSIGN    = 11;
 const psDWORD psBEHAVE_SRL_ASSIGN    = 12;
 const psDWORD psBEHAVE_SRA_ASSIGN    = 13;
 const psDWORD psBEHAVE_LAST_ASSIGN   = 13;
 const psDWORD psBEHAVE_FIRST_DUAL    = 14;
 const psDWORD psBEHAVE_ADD           = 14;
 const psDWORD psBEHAVE_SUBTRACT      = 15;
 const psDWORD psBEHAVE_MULTIPLY      = 16;
 const psDWORD psBEHAVE_DIVIDE        = 17;
 const psDWORD psBEHAVE_MODULO        = 18;
 const psDWORD psBEHAVE_EQUAL         = 19;
 const psDWORD psBEHAVE_NOTEQUAL      = 20;
 const psDWORD psBEHAVE_LESSTHAN      = 21;
 const psDWORD psBEHAVE_GREATERTHAN   = 22;
 const psDWORD psBEHAVE_LEQUAL        = 23;
 const psDWORD psBEHAVE_GEQUAL        = 24;
 const psDWORD psBEHAVE_LOGIC_OR	  = 25;
 const psDWORD psBEHAVE_LOGIC_AND	  = 26;
 const psDWORD psBEHAVE_BIT_OR        = 27;
 const psDWORD psBEHAVE_BIT_AND       = 28;
 const psDWORD psBEHAVE_BIT_XOR       = 29;
 const psDWORD psBEHAVE_BIT_SLL       = 30;
 const psDWORD psBEHAVE_BIT_SRL       = 31;
 const psDWORD psBEHAVE_LAST_DUAL     = 31;

 const psDWORD psBEHAVE_FIRST_UNARY   = 32;
 const psDWORD psBEHAVE_UNARY_MINUS   = 32;
 const psDWORD psBEHAVE_UNARY_PLUS	  = 33;
 const psDWORD psBEHAVE_UNARY_NOT	  = 34;		  
 const psDWORD psBEHAVE_UNARY_BITNOT  = 35;
 const psDWORD psBEHAVE_INC			  = 36;
 const psDWORD psBEHAVE_DEC			  = 37;
 const psDWORD psBEHAVE_LAST_UNARY    = 37;

 const psDWORD psBEHAVE_INDEX         = 38;

 //----------------------------------------------------------------------//
 // ����ֵ
 //----------------------------------------------------------------------//
 const int psSUCCESS                              =  0;
 const int psERROR                                = -1;
 const int psCONTEXT_ACTIVE                       = -2;
 const int psCONTEXT_NOT_FINISHED                 = -3;
 const int psCONTEXT_NOT_PREPARED                 = -4;
 const int psINVALID_ARG                          = -5;
 const int psNO_FUNCTION                          = -6;
 const int psNOT_SUPPORTED                        = -7;
 const int psINVALID_NAME                         = -8;
 const int psNAME_TAKEN                           = -9;
 const int psINVALID_DECLARATION                  = -10;
 const int psINVALID_OBJECT                       = -11;
 const int psINVALID_TYPE                         = -12;
 const int psALREADY_REGISTERED                   = -13;
 const int psMULTIPLE_FUNCTIONS                   = -14;
 const int psNO_MODULE                            = -15; 
 const int psNO_GLOBAL_VAR                        = -16;
 const int psINVALID_CONFIGURATION                = -17;
 const int psINVALID_INTERFACE                    = -18;
 const int psAPP_CANT_INTERFACE_DEFAULT_ARRAY     = -19; 
 const int psMODULE_CANT_BE_RESET				  = -20;
 const int psNO_BYTECODE						  = -21;
 const int psCLASS_EXIST						  = -22;
 const int psMODULE_EXIST						  = -23;
 const int psIVALID_OBJECT_POINTER				  = -24;
 const int psDONT_FIND_CLASS					  = -25;
 const int psCLASS_CICLE_DEPENDENCE				  = -26;
 const int psCANT_INIT_MODULE					  = -27;
 //----------------------------------------------------------------------//
 // �ű�����������״̬
 //----------------------------------------------------------------------//

 const int psEXECUTION_FINISHED      = 0;
 const int psEXECUTION_SUSPENDED     = 1;
 const int psEXECUTION_ABORTED       = 2;
 const int psEXECUTION_EXCEPTION     = 3;
 const int psEXECUTION_PREPARED      = 4;
 const int psEXECUTION_UNINITIALIZED = 5;
 const int psEXECUTION_ACTIVE        = 6;
 const int psEXECUTION_ERROR         = 7;


 //----------------------------------------------------------------------//
 // ����ִ�еı�ʶ
 //----------------------------------------------------------------------//
 const int psEXEC_STEP_INTO = 1;
 const int psEXEC_STEP_OVER = 2;
 const int psEXEC_DEBUG		= 4;

 // Prepare flags
 const int psPREPARE_PREVIOUS = -1;

//----------------------------------------------------------------------//
// ����������ֵַ�ĺ�������
//----------------------------------------------------------------------//
#include <memory.h>

 inline psUPtr psFunctionPtr(psFUNCTION_t func)
 {
	 psUPtr p;
	 memset(&p, 0, sizeof(p));
	 p.func = func;

	 return p;
 }

 // Method pointers

 // Declare a dummy class so that we can determine the size of a simple method pointer
 class psCSimpleDummy {};
 typedef void (psCSimpleDummy::*psSIMPLEMETHOD_t)();
 const int SINGLE_PTR_SIZE = sizeof(psSIMPLEMETHOD_t);


 // Define template
 template <int N>
 struct psSMethodPtr
 {
	 template<class M>
		 static psUPtr Convert(M Mthd)
	 {
		 // This version of the function should never be executed, nor compiled,
		 // as it would mean that the size of the method pointer cannot be determined.
		 // int ERROR_UnsupportedMethodPtr[-1];
		 return 0;
	 }
 };


 // Template specialization
 template <>
 struct psSMethodPtr<SINGLE_PTR_SIZE>
 {
	 template<class M> 
		 static psUPtr Convert(M Mthd)
	 {
		 psUPtr p;
		 memset(&p, 0, sizeof(p));

		 memcpy(&p, &Mthd, SINGLE_PTR_SIZE);

		 return p;
	 }
 };


#if defined(_MSC_VER) && !defined(__MWERKS__)

 // MSVC and Intel uses different sizes for different class method pointers
 template <>
 struct psSMethodPtr<SINGLE_PTR_SIZE+1*sizeof(int)>
 {
	 template <class M>
		 static psUPtr Convert(M Mthd)
	 {
		 psUPtr p;
		 memset(&p, 0, sizeof(p));

		 memcpy(&p, &Mthd, SINGLE_PTR_SIZE+sizeof(int));

		 return p;
	 }
 };

 template <>
 struct psSMethodPtr<SINGLE_PTR_SIZE+2*sizeof(int)>
 {
	 template <class M>
		 static psUPtr Convert(M Mthd)
	 {
		 // This is where a class with virtual inheritance falls

		 // Since method pointers of this type doesn't have all the 
		 // information we need we force a compile failure for this case.
		 int ERROR_VirtualInheritanceIsNotAllowedForMSVC[-1];

		 // The missing information is the location of the vbase table,
		 // which is only known at compile time.

		 // You can get around this by forward declaring the class and 
		 // storing the sizeof its method pointer in a constant. Example:

		 // class ClassWithVirtualInheritance;
		 // const int ClassWithVirtualInheritance_workaround = sizeof(void ClassWithVirtualInheritance::*());

		 // This will force the compiler to use the unknown type 
		 // for the class, which falls under the next case

		 psUPtr p;
		 return p;
	 }
 };

 template <>
 struct psSMethodPtr<SINGLE_PTR_SIZE+3*sizeof(int)>
 {
	 template <class M>
		 static psUPtr Convert(M Mthd)
	 {
		 psUPtr p;
		 memset(&p, 0, sizeof(p));

		 memcpy(&p, &Mthd, SINGLE_PTR_SIZE+3*sizeof(int));

		 return p;
	 }
 };

#endif

#endif // __PIXELSCRIPT_H__








