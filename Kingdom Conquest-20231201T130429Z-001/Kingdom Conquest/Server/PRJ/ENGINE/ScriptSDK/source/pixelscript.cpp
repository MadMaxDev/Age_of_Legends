#include "scriptengine.h"
#include "context.h"
#include "bytecodedef.h"
#include "memmanager.h"
#include <assert.h>

#if defined(_MSC_VER)
	// Disable warning C4312(convert 'psUINT' to 'char *')
	#pragma warning(disable : 4312)
#endif

static const psCHAR* g_psBCName[] = 
{
	PS_T("NOP"),
		PS_T("SUSPEND"),

		PS_T("POP"),
		PS_T("PUSH"),
		PS_T("PUSH4"),
		PS_T("PUSH8"),
		PS_T("PUSHREF"),

		PS_T("MOV1"),
		PS_T("MOV2"),
		PS_T("MOV4"),
		PS_T("MOV8"),

		PS_T("DEFREF"),
		PS_T("RDREF1"),
		PS_T("WTREF1"),
		PS_T("RDREF2"),
		PS_T("WTREF2"),
		PS_T("RDREF4"),
		PS_T("WTREF4"),
		PS_T("RDREF8"),
		PS_T("WTREF8"),

		PS_T("RRET1"),
		PS_T("SRET1"),
		PS_T("RRET2"),
		PS_T("SRET2"),
		PS_T("RRET4"),
		PS_T("SRET4"),
		PS_T("RRET8"),
		PS_T("SRET8"),

		PS_T("PGA"),
		PS_T("PSP"),
		PS_T("MEMCPY"),

		PS_T("CALL"),
		PS_T("RET"),
		PS_T("CALLSYS"),
		PS_T("JMP"),
		PS_T("JMPP"),	
		PS_T("JNZ"),
		PS_T("JZ"),

		PS_T("ADDi"),
		PS_T("SUBi"),
		PS_T("MULi"),
		PS_T("DIVi"),
		PS_T("MODi"),
		PS_T("INCi"),
		PS_T("DECi"),
		PS_T("NEGi"),

		PS_T("ADDf"),
		PS_T("SUBf"),
		PS_T("MULf"),
		PS_T("DIVf"),
		PS_T("MODf"),
		PS_T("INCf"),
		PS_T("DECf"),
		PS_T("NEGf"),

		PS_T("ADDd"),
		PS_T("SUBd"),
		PS_T("MULd"),
		PS_T("DIVd"),
		PS_T("MODd"),
		PS_T("INCd"),
		PS_T("DECd"),
		PS_T("NEGd"),

		PS_T("NOT"),
		PS_T("AND"),
		PS_T("OR"),
		PS_T("XOR"),
		PS_T("SLL"),
		PS_T("SRL"),
		PS_T("SLA"),
		PS_T("SRA"),

		PS_T("TEi"),
		PS_T("TNEi"),
		PS_T("TLi"),
		PS_T("TGi"),
		PS_T("TLEi"),
		PS_T("TGEi"),

		PS_T("TZf"),
		PS_T("TNZf"),
		PS_T("TLf"),
		PS_T("TGf"),
		PS_T("TLEf"),
		PS_T("TGEf"),

		PS_T("TZd"),
		PS_T("TNZd"),
		PS_T("TLd"),
		PS_T("TGd"),
		PS_T("TLEd"),
		PS_T("TGEd"),

		PS_T("I2F"),
		PS_T("F2I"),
		PS_T("D2I"),
		PS_T("I2D"),
		PS_T("D2F"),
		PS_T("F2D"),

		PS_T("DW2B"),
		PS_T("B2DW"),
		PS_T("DW2W"),
		PS_T("W2DW"),
		PS_T("W2B"),
		PS_T("B2W"),

		PS_T("LINE"),
		PS_T("LABLE"),
};
//-------------------------------------------------------------------
// 名称: psGetLibraryVersion
// 说明: 得到脚本引擎库的版本说明
//-------------------------------------------------------------------

const char* psGetLibraryVersion()
{
	#ifdef _DEBUG
		return PIXELSCRIPT_VERSION_STRING " DEBUG";
	#else
		return PIXELSCRIPT_VERSION_STRING;
	#endif
}

//-------------------------------------------------------------------
// 名称: psCreateScriptEngine
// 说明: 创建脚本引擎
//-------------------------------------------------------------------
psIScriptEngine* psCreateScriptEngine(psDWORD version)
{
	if( (version/10000) != PIXELSCRIPT_VERSION_MAJOR )
		return 0;

	if( (version/100)%100 != PIXELSCRIPT_VERSION_MINOR )
		return 0;

	if( (version%100) > PIXELSCRIPT_VERSION_BUILD )
		return 0;

	return new psCScriptEngine();
}

//-------------------------------------------------------------------
// 名称: psCleanup
// 说明: 清除被管理的内存和全局数据(避免VC++报内存遗漏)
//-------------------------------------------------------------------
void psCleanup()
{
	psTempMemManager::instance().destroy();
	psIMemManager::destroyAll();
	DeleteThreadManager();
}

//-------------------------------------------------------------------
// 名称: psGetDisassembleCode
// 说明: 对二进制代码进行反汇编
//-------------------------------------------------------------------
psIBuffer* psGetDisassembleCode(const psBYTE* bc, size_t size, psIBuffer** outLineNumbers, int* outNumLines )
{
	assert( bc );

	psCString code(PS_T(""));
	psCString str, op1, op2, op3;
	const psCHAR* bases[] =
	{
		PS_T(""), // NONE
		PS_T("const"),  // constant
		PS_T("stack"),  // Stack
		PS_T("global"), // global
		PS_T("heap"),   // Heap
		PS_T("unknwon"), // error
	};
	const int cMaxBaseIndex = sizeof(bases)/sizeof(psCHAR*);

	int numLines        = 0;
	const int strides[] = {1, 1, 4, 4, 1};

	const psBYTE* s_bc = bc;
	while ( bc < s_bc + size )
	{
		psBYTE base1 = BC_BASE1(bc);
		psBYTE base2 = BC_BASE2(bc);
		psBYTE base3 = BC_BASE3(bc);
		int offset1 = BC_OFFSET1(bc);
		int offset2 = BC_OFFSET2(bc);
		int offset3 = BC_OFFSET3(bc);

		#define FORMAT_OP1_STR(str)  str.Format(PS_T("%s[%d]"), bases[base1], offset1/strides[base1]);
		#define FORMAT_OP2_STR(str)  str.Format(PS_T("%s[%d]"), bases[base2], offset2/strides[base2])
		#define FORMAT_OP3_STR(str)  str.Format(PS_T("%s[%d]"), bases[base3], offset3/strides[base3])

		if (base1 > cMaxBaseIndex-1) base1 = cMaxBaseIndex;
		if (base2 > cMaxBaseIndex-1) base2 = cMaxBaseIndex;
		if (base3 > cMaxBaseIndex-1) base3 = cMaxBaseIndex;
		switch(*bc)
		{
		case BC_NOP:
		case BC_SUSPEND:
			str.Format(PS_T("%5d   %-8s\n"), bc - s_bc, g_psBCName[*bc]);
			bc += SIZEOFBC(0);
			++numLines;
			break;

		case BC_POP:
		case BC_PUSH:
		case BC_PUSH4:
		case BC_PUSH8:
		case BC_PUSHREF:	
		case BC_PSP:
		case BC_CALL:
		case BC_RRET1:
		case BC_SRET1:
		case BC_RRET2:
		case BC_SRET2:
		case BC_RRET4:
		case BC_SRET4:
		case BC_RRET8:
		case BC_SRET8:

		case BC_RET:
		case BC_JMP:	

		case BC_DECi:
		case BC_INCi:	
		case BC_DECf:
		case BC_INCf:
		case BC_DECd:
		case BC_INCd:

			FORMAT_OP1_STR(op1);

			str.Format(PS_T("%5d   %-8s %-15s\n"), bc - s_bc, g_psBCName[*bc], op1.AddressOf());
			bc += SIZEOFBC(1);	
			++numLines;
			break;

		case BC_CALLSYS:
		case BC_MOV1:
		case BC_MOV2:
		case BC_MOV4:
		case BC_MOV8:
		case BC_DEFREF:
		case BC_RDREF1:
		case BC_WTREF1:
		case BC_RDREF2:
		case BC_WTREF2:
		case BC_RDREF4:
		case BC_WTREF4:
		case BC_RDREF8:
		case BC_WTREF8:
		case BC_PGA:	

		case BC_NOT:
		case BC_NEGi:
		case BC_NEGf:
		case BC_NEGd:

		case BC_JZ:
		case BC_JNZ:
		case BC_JMPP:

		case BC_I2F:
		case BC_I2D:
		case BC_F2I:	
		case BC_F2D:	
		case BC_D2I:
		case BC_D2F:
		case BC_I2B:
		case BC_B2I:
		case BC_I2C:
		case BC_C2I:
		case BC_W2I:
		case BC_I2W:
		case BC_S2I:
		case BC_I2S:

			FORMAT_OP1_STR(op1);
			FORMAT_OP2_STR(op2);
			str.Format(PS_T("%5d   %-8s %-15s %-15s\n"), bc - s_bc, g_psBCName[*bc],
				op1.AddressOf(), op2.AddressOf());

			bc += SIZEOFBC(2);	
			++numLines;
			break;

		case BC_MEMCPY:
		case BC_ADDi:
		case BC_SUBi:
		case BC_MULi:
		case BC_DIVi:
		case BC_MODi:

		case BC_ADDf:
		case BC_SUBf:
		case BC_MULf:
		case BC_DIVf:
		case BC_MODf:

		case BC_ADDd:
		case BC_SUBd:
		case BC_MULd:
		case BC_DIVd:
		case BC_MODd:

		case BC_TEi:
		case BC_TNEi:
		case BC_TLi:
		case BC_TLEi:
		case BC_TGi:
		case BC_TGEi:

		case BC_TEf:
		case BC_TNEf:
		case BC_TLf:
		case BC_TLEf:
		case BC_TGf:
		case BC_TGEf:

		case BC_TEd:
		case BC_TNEd:
		case BC_TLd:
		case BC_TLEd:
		case BC_TGd:
		case BC_TGEd:

		case BC_XOR:
		case BC_OR:
		case BC_AND:
		case BC_SLL:
		case BC_SRL:
		case BC_SLA:
		case BC_SRA:
			FORMAT_OP1_STR(op1);
			FORMAT_OP2_STR(op2);
			FORMAT_OP3_STR(op3);
			str.Format(PS_T("%5d   %-8s %-15s %-15s %-15s\n"), bc - s_bc, g_psBCName[*bc],
				op1.AddressOf(), op2.AddressOf(), op3.AddressOf()); 

			bc += SIZEOFBC(3);	
			++numLines;
			break;
		default: 
			code += PS_T("Unknow instruction\n");
			goto EXIT_DISASSEMBLE;
		}
		code += str;
	}
EXIT_DISASSEMBLE:
	if ( outNumLines )
	{
		*outNumLines = numLines;
	}
	if ( outLineNumbers )
	{
		psBuffer* outLineBuf = new psBuffer;
		outLineBuf->Alloc( numLines * 2 * sizeof(int) );

		int* lineNumbers = (int*)outLineBuf->GetBufferPointer();
		int  pos		 = 0;
		for ( int i = 0; i < numLines; ++i )
		{
			lineNumbers[i*2 + 0] = i;
			lineNumbers[i*2 + 1] = pos;

			int bcs = g_psBCSize[ *(s_bc + pos) ];
			assert( bcs > 0 );
			pos += bcs;
		}
		*outLineNumbers = outLineBuf;
	}

	psBuffer* codeBuf = new psBuffer;
	codeBuf->Assign( (psBYTE*)( code.RemoveBuffer() ), true );
	return codeBuf;
}

//-------------------------------------------------------------------
// 名称: psGetActiveContext
// 说明: 得到当前活动的脚本上下文
//-------------------------------------------------------------------
psIScriptContext *psGetActiveContext()
{
	psCThreadLocalData *tld = GetThreadManager().GetLocalData();
	if( tld->m_ActiveContexts.size() == 0 )
		return 0;
	return tld->m_ActiveContexts[tld->m_ActiveContexts.size()-1];
}

//-------------------------------------------------------------------
// 名称: psPushActiveContext
// 说明: 把当前活动的脚本上下文入栈
//-------------------------------------------------------------------
void psPushActiveContext(psIScriptContext *ctx)
{
	psCThreadLocalData *tld = GetThreadManager().GetLocalData();
	tld->m_ActiveContexts.push_back(ctx);
}

//-------------------------------------------------------------------
// 名称: psPopActiveContext
// 说明: 把当前活动的脚本上下文出栈
//-------------------------------------------------------------------
void psPopActiveContext(psIScriptContext *ctx)
{
	psCThreadLocalData *tld = GetThreadManager().GetLocalData();

	assert(tld->m_ActiveContexts.size() > 0);
	assert(tld->m_ActiveContexts[tld->m_ActiveContexts.size()-1] == ctx);

	tld->m_ActiveContexts.pop_back();
}
