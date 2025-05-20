#include "restore.h"

#include "config.h"
#include "bytecodedef.h"
#include "module.h"
#include "scriptfunction.h"
#include <assert.h>

#define MAKE_FOURCC(c1, c2, c3, c4) ((psDWORD)c1 << 24 | (psDWORD)c2 << 16 | (psDWORD)c3 << 8 | (psDWORD)c4)

const int PBC_MAGIC = MAKE_FOURCC('P', 'B', 'C', '\0');
const int PBC_VERSION = 100;

const int PEC_MAGIC = MAKE_FOURCC('P', 'E', 'C', '\0');
const int PEC_VERSION = 100;

namespace pse
{



	inline psCRestore& operator << (psCRestore& stream, psBYTE value)
	{
		stream.Write(&value, sizeof(value));
		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psBYTE& value)
	{
		stream.Read(&value, sizeof(value));
		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, bool value)
	{
		stream.Write(&value, sizeof(value));
		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, bool& value)
	{
		stream.Read(&value, sizeof(value));
		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, int value)
	{
		stream.Write(&value, sizeof(value));
		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, int& value)
	{
		stream.Read(&value, sizeof(value));
		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, psUINT value)
	{
		stream.Write(&value, sizeof(value));
		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psUINT& value)
	{
		stream.Read(&value, sizeof(value));
		return stream;
	}

	template <class T>
		inline psCRestore& operator << (psCRestore& stream, const std::vector<T>& v)
	{
		psUINT size = (psUINT)v.size();
		stream << size;
		for (psUINT i = 0; i < size; ++i)
			stream << v[i];
		return stream;
	}

	template <class T>
		inline psCRestore& operator >> (psCRestore& stream, std::vector<T>& v)
	{
		psUINT size;
		stream >> size;
		v.resize(size);
		for (psUINT i = 0; i < size; ++i)
			stream >> v[i];
		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, const psCString& str)
	{
		psUINT len = (psUINT)str.GetLength();
		stream << len;
		stream.Write( str.AddressOf(), len * sizeof(psCHAR) );
		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psCString& str)
	{
		psUINT len;
		stream >> len;
		str.SetLength( len );
		stream.Read( str.AddressOf(), len * sizeof(psCHAR) );

		str.RecalculateLength();
		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, const psCDataType& dt)
	{
		int tokenType = (int)dt.TokenType();
		stream << tokenType;
		stream << dt.m_ArrayDim;
		stream.Write( dt.m_ArraySizes, sizeof(int) * dt.m_ArrayDim );

		stream << dt.m_bIsPointer;
		stream << dt.m_bIsReference;
		stream << dt.m_bIsReadOnly;

		stream.WriteObjectTypeIdx( dt.m_pObjectType );
		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psCDataType& dt)
	{
		int tokenType;
		stream >> tokenType;
		dt.TokenType() = (psETokenType)tokenType;
		stream >> dt.m_ArrayDim;
		stream.Read(dt.m_ArraySizes, sizeof(int)*dt.m_ArrayDim );

		stream >> dt.m_bIsPointer;	
		stream >> dt.m_bIsReference;
		stream >> dt.m_bIsReadOnly;

		dt.m_pObjectType = stream.ReadObjectTypeIdx();
		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, const psSProperty& prop)
	{
		stream << prop.name;
		stream << prop.type;
		stream << prop.index;
		stream << prop.base;
		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psSProperty& prop)
	{
		stream >> prop.name;
		stream >> prop.type;
		stream >> prop.index;	
		stream >> prop.base;
		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, const psSTypeBehaviour& beh)
	{
		stream << beh.construct;
		stream << beh.destruct;
		stream << beh.copy;

		stream << beh.constructors;
		stream << beh.operators;
		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psSTypeBehaviour& beh)
	{
		stream >> beh.construct;
		stream >> beh.destruct;
		stream >> beh.copy;

		stream >> beh.constructors;
		stream >> beh.operators;
		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, const psCObjectType& objType )
	{
		// write object type flag
		stream << objType.m_bScriptObject;

		stream << objType.m_Flags;

		// write object type name
		stream << objType.m_Name;

		// write object type size
		stream << objType.m_Size;

		if ( objType.m_bScriptObject )
		{
			// write object type properties
			psUINT count = (psUINT)objType.m_Properties.size();
			stream << count;
			for (psUINT i = 0; i < count; ++i)
				stream << *(objType.m_Properties[i]);

			// write object methods
			stream << objType.m_Methods;

			// write object type behaviour
			stream << objType.m_Beh;
		}

		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psCObjectType& objType )
	{
		// read object type flag
		stream >> objType.m_bScriptObject;

		stream >> objType.m_Flags;

		// read object type name
		stream >> objType.m_Name;

		// read object type size
		stream >> objType.m_Size;

		if (objType.m_bScriptObject)
		{
			// read object type properties
			psUINT count = 0;
			stream >> count;

			objType.m_Properties.resize( count );
			for (psUINT i = 0; i < count; ++i)
			{
				objType.m_Properties[i] = new psSProperty();
				stream >> *(objType.m_Properties[i]);
			}

			// read object methods
			stream >> objType.m_Methods;

			// read object type behaviour
			stream >> objType.m_Beh;

		}

		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, const psSDebugSymbol& symbol)
	{
		stream << symbol.name;
		stream << symbol.type;
		stream << symbol.offset;
		stream << symbol.start;
		stream << symbol.end;

		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psSDebugSymbol& symbol)
	{
		stream >> symbol.name;
		stream >> symbol.type;
		stream >> symbol.offset;
		stream >> symbol.start;
		stream >> symbol.end;

		return stream;
	}

	inline psCRestore& operator << (psCRestore& stream, const psCScriptFunction& func)
	{
		// Write function name
		stream << func.m_Name;

		// Write function return type
		stream << func.m_ReturnType;

		// Write function parameter types
		stream << func.m_ParamTypes;

		// Write function ID
		stream << func.m_ID;

		// Write unction flag
		stream << func.m_funcFlag;

		// Write stack needed
		stream << func.m_StackNeeded;

		// Write object type index
		stream.WriteObjectTypeIdx(func.m_ObjectType);

		// Write bytecode
		stream << func.m_BinaryCodeSize;
		if ( func.m_BinaryCodeSize > 0 )
		{
			stream.Write( func.m_BinaryCode, func.m_BinaryCodeSize );
		}

		// Write line numbers
		stream << func.m_NumLines;
		if ( func.m_NumLines > 0 )
		{
			stream.Write( func.m_LineNumbers, sizeof(int) * func.m_NumLines );
		}

		// Write debug symbols
		stream << func.m_NumDebugSymbols;
		if ( func.m_NumDebugSymbols > 0 )
		{
			for (int i = 0; i < func.m_NumDebugSymbols; ++i)
				stream << func.m_DebugSymbols[i];
		}

		return stream;
	}

	inline psCRestore& operator >> (psCRestore& stream, psCScriptFunction& func)
	{
		// Read function name
		stream >> func.m_Name;

		// Read function return type
		stream >> func.m_ReturnType;

		// Read function parameter types
		stream >> func.m_ParamTypes;

		// Read function ID
		stream >> func.m_ID;

		// Read function flag
		stream >> func.m_funcFlag;

		// Read stack needed
		stream >> func.m_StackNeeded;

		// Read object type index
		func.m_ObjectType = stream.ReadObjectTypeIdx();

		// Read bytecode
		stream >> func.m_BinaryCodeSize;
		if ( func.m_BinaryCodeSize > 0 )
		{
			func.m_BinaryCode = new psBYTE[func.m_BinaryCodeSize];
			stream.Read( func.m_BinaryCode, func.m_BinaryCodeSize );
		}

		// Read line numbers
		stream >> func.m_NumLines;
		if ( func.m_NumLines > 0 )
		{
			func.m_LineNumbers = new int[func.m_NumLines];
			stream.Read( func.m_LineNumbers, func.m_NumLines*sizeof(int) );
		}

		// Read debug symbols
		stream >> func.m_NumDebugSymbols;
		if ( func.m_NumDebugSymbols > 0 )
		{
			func.m_DebugSymbols = new psSDebugSymbol[ func.m_NumDebugSymbols ];
			for (int i = 0; i < func.m_NumDebugSymbols; ++i)
				stream >> func.m_DebugSymbols[i];
		}

		return stream;
	}
}
using namespace pse;

int psCRestore::SaveEngineConfig(psCScriptEngine* engine)
{	
	assert( m_pStream && engine );

	m_pEngine = engine;
	psCRestore& stream = *this;

	psUINT i, count;

	// write magic number and version
	stream << (PEC_MAGIC);
	stream << (PEC_VERSION);
	
	// write all object types
	count = (psUINT)m_pEngine->m_ObjectTypes.size();
	stream << (count);
	{
		psCScriptEngine::ObjectTypeMap_t::iterator ite = m_pEngine->m_ObjectTypes.begin();
		psCScriptEngine::ObjectTypeMap_t::iterator end = m_pEngine->m_ObjectTypes.end();
		for ( ; ite != end; ++ite )
		{
			stream << *(ite->second);
		}
	}
	
	// write all system functions
	count = (psUINT)m_pEngine->m_SystemFunctions.size();
	stream << (count);
	for ( i = 0; i < count; ++i )
	{
		stream << *(m_pEngine->m_SystemFunctions[i]);
	}

	//// write global behaviours
	//WriteTypeBehaviour(m_pEngine->m_GlobalBehavious);

	//// write global properties
	//count = m_pEngine->m_GlobalProps.size();
	//WRITE_NUM(count);
	//for (i = 0; i < count; ++i)
	//	WriteProperty(m_pEngine->m_GlobalProps[i]);

	return 0;
}

int psCRestore::RestoreEngineConfig(psCScriptEngine* engine)
{
	assert( m_pStream && engine );

	m_pEngine = engine;
	psCRestore& stream = *this;

	psUINT i, count;
	int magic;
	int version;

	// read magic number and version
	stream >> (magic);
	stream >> (version);

	if (magic != PEC_MAGIC || version != PEC_VERSION)
		return psERROR;
	
	// read all object types
	stream >> (count);
	for ( i = 0; i < count; ++i )
	{
		psCObjectType* ot = new psCObjectType();
		stream >> *ot;
		m_pEngine->m_ObjectTypes[ot->m_Name.AddressOf()] = ot;
	}
	
	// read all system functions
	stream >> (count);
	m_pEngine->m_SystemFunctions.resize(count);
	m_pEngine->m_SystemFunctionInterfaces.resize(count);
	for ( i = 0; i < count; ++i )
	{
		psCScriptFunction* func = new psCScriptFunction();
		m_pEngine->m_SystemFunctions[i] = func;

		// read function
		stream >> *(func);

		// create a system function interface
		psSSystemFunctionInterface *newInterface = new psSSystemFunctionInterface;
		memset(newInterface, 0, sizeof(psSSystemFunctionInterface));
		m_pEngine->m_SystemFunctionInterfaces[i] = newInterface;
	}

	// read global behaviours
	//ReadTypeBehaviour(m_pEngine->m_GlobalBehavious);

	//// read global properties
	//READ_NUM(count);
	//m_pEngine->m_GlobalProps.resize(count);
	//for (i = 0; i < count; ++i)
	//{
	//	psSProperty* prop = new psSProperty();
	//	m_pEngine->m_GlobalProps[i] = prop;

	//	ReadProperty(prop);
	//}
	return 0;
}

int psCRestore::SaveModule(psCModule* module) 
{	
	assert( module && m_pStream );

	m_pModule = module;
	psCRestore& stream = *this;

	unsigned int i, count;

	// write magic number and version
	stream << PBC_MAGIC;
	stream << PBC_VERSION;

	// initFunction
	if (m_pModule->m_pInitFunction )
	{
		stream << (int)1;
		stream << *m_pModule->m_pInitFunction;
	}else
	{
		stream << (int)0;
	}

	// exit function
	if ( m_pModule->m_pExitFunction )
	{
		stream << (int)1;
		stream << *m_pModule->m_pExitFunction;
	}else
	{
		stream << (int)0;
	}

	// script object types
	count = (psUINT) m_pModule->m_ObjectTypes.size();
	stream << count;
	for (i = 0; i < count; ++i)
		stream << *(m_pModule->m_ObjectTypes[i]);

	// global operator overload
	stream << m_pModule->m_GlobalOperators;
	
	// scriptFunctions[]
	count = (psUINT)m_pModule->m_ScriptFunctions.size();
	stream << count;
	for( i = 0; i < count; ++i )
		stream << *(m_pModule->m_ScriptFunctions[i]);

	// scriptGlobals[]
	count = (psUINT)m_pModule->m_ScriptGlobals.size();
	stream << count;
	for( i = 0; i < count; ++i ) 
		stream << *(m_pModule->m_ScriptGlobals[i]);

	// constantMem
	stream << m_pModule->m_ConstantMemSize;
	if ( m_pModule->m_ConstantMemSize > 0 )
		stream.Write( m_pModule->m_ConstantMem, m_pModule->m_ConstantMemSize );

	// globalMem size
	stream << m_pModule->m_GlobalMemSize;

	return psSUCCESS;
}


int psCRestore::RestoreModule(psCModule* module) 
{
	assert( module && m_pStream );

	m_pModule = module;
	psCRestore& stream = *this;

	unsigned int i, count;

	int magic, version;
	// read magic number and version
	stream >> magic;
	stream >> version;

	if ( magic != PBC_MAGIC || version != PBC_VERSION )
		return psERROR;

	// initFunction
	int hasInitFunc = 0;
	stream >> hasInitFunc;
	if ( hasInitFunc )
	{
		psCScriptFunction* func = m_pModule->GetSpecialFunction(psFUNC_INIT);
		stream >> *func;
	}

	// exit function
	int hasExitFunc = 0;
	stream >> hasExitFunc;
	if ( hasExitFunc )
	{
		psCScriptFunction* func = m_pModule->GetSpecialFunction(psFUNC_EXIT);
		stream >> *func;
	}

	// script object types
	stream >> count;
	m_pModule->m_ObjectTypes.resize( count );
	for (i = 0; i < count; ++i)
	{
		m_pModule->m_ObjectTypes[i] = new psCObjectType();
		stream >> *(m_pModule->m_ObjectTypes[i]);
	}

	// global operator overload
	stream >> m_pModule->m_GlobalOperators;

	// scriptFunctions[]
	stream >> count;
	m_pModule->m_ScriptFunctions.resize( count );
	for( i = 0; i < count; ++i )
	{
		m_pModule->m_ScriptFunctions[i] = new psCScriptFunction();
		stream >> *(m_pModule->m_ScriptFunctions[i]);
	}

	// scriptGlobals[]
	stream >> count;
	 m_pModule->m_ScriptGlobals.resize( count );
	for( i = 0; i < count; ++i ) 
	{
		m_pModule->m_ScriptGlobals[i] = new psSProperty();
		stream >> *(m_pModule->m_ScriptGlobals[i]);
	}

	// constantMem
	stream >> m_pModule->m_ConstantMemSize;
	if ( m_pModule->m_ConstantMemSize > 0  )
	{
		m_pModule->m_ConstantMem = new psBYTE[m_pModule->m_ConstantMemSize];
		stream.Read( m_pModule->m_ConstantMem, m_pModule->m_ConstantMemSize );
	}

	// globalMem
	stream >> m_pModule->m_GlobalMemSize;

	module->SetModuleRestoreSuccess();

	// Call initialize function
	if ( !m_pModule->CallInit() )
		return psERROR;

	return psSUCCESS;
}


void psCRestore::WriteObjectTypeIdx(const psCObjectType* ot)
{	
	psCRestore& stream = *this;

	int type = -1;
	if( ot )
	{
		type = ot->m_bScriptObject ? 1 : 0;
		stream << type;
		if (ot->m_bScriptObject)
		{
			if (m_pModule)
				stream << m_pModule->GetObjectTypeIdx(ot);
			else
				stream << -1;
		}else
		{
			stream << ot->m_Name;
		}
	}else
	{
		stream << type;
	}
}

psCObjectType* psCRestore::ReadObjectTypeIdx() 
{
	psCRestore& stream = *this;

	int type;
	stream >> type;
	if (type == 1)
	{
		int idx = -1;
		stream >> idx;

		if (idx >= 0 && idx < int(m_pModule->m_ObjectTypes.size()) && m_pModule)
			return m_pModule->m_ObjectTypes[idx];
		else
			return NULL;

	}else if (type == 0)
	{
		// Read the object type name
		psCString typeName;
		stream >> typeName;
		
		// Find the registered object type in the m_pEngine
		if (m_pEngine)
			return m_pEngine->GetObjectType(typeName.AddressOf());
		else
			return NULL;
	}else
	{
		return NULL;
	}
}


