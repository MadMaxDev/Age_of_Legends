#include "scriptfunction.h"
#include "objecttype.h"
#include "threadmanager.h"
#include "scriptnode.h"
#include "bytecode.h"

//////////////////////////////////////////////////////////////////////////
// class psCScriptFunction
psCScriptFunction::psCScriptFunction(void)
{
	m_ID			 = -1;
	m_StackNeeded	 = 0;
	m_funcFlag		 = 0;

	m_NumLines		 = 0;
	m_BinaryCodeSize = 0;	
	m_NumDebugSymbols = 0;
	m_IsIncluded	  = 0;

	m_LineNumbers	 = NULL;
	m_BinaryCode	 = NULL;
	m_ObjectType	 = NULL;
	m_DebugSymbols   = NULL;
}

psCScriptFunction::~psCScriptFunction(void)
{
	delete []m_BinaryCode;
	delete []m_LineNumbers;
	delete []m_DebugSymbols;
}

void psCScriptFunction::Clear()
{
	m_ParamTypes.clear();
	m_BreakPoints.clear();
	delete []m_BinaryCode;
	delete []m_LineNumbers;
	delete []m_DebugSymbols;

	m_BinaryCode = NULL;
	m_LineNumbers = NULL;
	m_DebugSymbols = NULL;

	m_NumLines = 0;
	m_BinaryCodeSize = 0;
	m_NumDebugSymbols = 0;
}

void psCScriptFunction::SetCode(psCByteCode& code)
{	
	// 输出二进制码
	if ( m_BinaryCode )
	{
		delete []m_BinaryCode;
		m_BinaryCode = NULL;
	}
	m_BinaryCodeSize = (int)code.GetSize();
	if ( m_BinaryCodeSize > 0 )
	{
		m_BinaryCode = new psBYTE[m_BinaryCodeSize];
		code.Output( m_BinaryCode );
	}

	// 设置行号和堆栈大小
	if ( m_LineNumbers )
	{
		delete []m_LineNumbers;
		m_LineNumbers = NULL;
	}
	m_NumLines = (psDWORD)code.m_LineNumbers.size();
	if ( m_NumLines > 0 )
	{
		m_LineNumbers = new int[m_NumLines];
		memcpy( m_LineNumbers, &code.m_LineNumbers[0], m_NumLines * sizeof(int) );
	}
	m_StackNeeded = code.m_LargestStackUsed;
}

void psCScriptFunction::SetDebugSymbols( const std::vector<psSDebugSymbol>& debugSymbols )
{
	if ( m_DebugSymbols )
	{
		delete []m_DebugSymbols;
		m_DebugSymbols = NULL;
	}

	m_NumDebugSymbols = (int)debugSymbols.size();
	if ( m_NumDebugSymbols > 0 )
	{
		m_DebugSymbols = new psSDebugSymbol[ m_NumDebugSymbols ];
		for (int i = 0; i < m_NumDebugSymbols; ++i )
		{
			int lineno;
			m_DebugSymbols[i].name   = debugSymbols[i].name;
			m_DebugSymbols[i].type   = debugSymbols[i].type;
			m_DebugSymbols[i].offset = debugSymbols[i].offset;
			m_DebugSymbols[i].start  = GetPosFromLineNumber(debugSymbols[i].start, lineno);
			m_DebugSymbols[i].end    = GetPosFromLineNumber(debugSymbols[i].end, lineno );
		}
	}
}
int psCScriptFunction::GetSpaceNeededForArguments()
{
	// 得到参数的大小
	int s = 0;
	for( size_t n = 0; n < m_ParamTypes.size(); n++ )
		s += m_ParamTypes[n].GetSizeOnStackDWords();

	return s;
}

int psCScriptFunction::GetArgumentsSpaceOnStack()
{
	int s = 0;
	for( size_t n = 0; n < m_ParamTypes.size(); n++ )
		s += m_ParamTypes[n].GetSizeOnStackDWords();

	if ( m_ReturnType.IsObject() && !m_ReturnType.IsReference() )
		++s;

	if ( m_ObjectType )
		++s;

	return s;
}

int psCScriptFunction::GetSpaceNeededForReturnValue()
{
	return m_ReturnType.GetSizeOnStackDWords();
}

const psIObjectType* psCScriptFunction::GetObjectType() const
{
	return m_ObjectType;
}

psCString psCScriptFunction::GetDeclaration(bool bAddClassName) const
{
	psCString str(PS_T(""));

	if( (m_funcFlag & (CONSTRUCTOR|DESTRUCTOR) ) == 0 )
		str = m_ReturnType.GetFormat() + PS_T(" ");

	if (bAddClassName && m_ObjectType)
		str += m_ObjectType->m_Name + PS_T("::");

	str += m_Name + PS_T("(");

	if( m_ParamTypes.size() > 0 )
	{
		size_t n;
		for( n = 0; n < m_ParamTypes.size() - 1; n++ )
			str += m_ParamTypes[n].GetFormat() + PS_T(", ");

		str += m_ParamTypes[n].GetFormat();
	}

	str += PS_T(")");

	return str;
}

psAutoStringPtr psCScriptFunction::GetDeclarationString() const
{
	psCString strDecl = GetDeclaration();

	//-------------------------------------
	psBuffer* buf = new psBuffer;
	buf->Assign( (psBYTE*)strDecl.RemoveBuffer(), true );

	psAutoStringPtr str(buf);
	return str;
}

int psCScriptFunction::GetLineNumber(int pos) const
{
	if( m_NumLines < 2 ) return -1;

	// 做二叉搜索
	int max = (int)m_NumLines/2 - 1;
	int min = 0;
	int i = max/2;

	if (m_LineNumbers[0] >= pos) return m_LineNumbers[1];
	if (m_LineNumbers[m_NumLines - 2] <= pos) return m_LineNumbers[m_NumLines - 1];

	for(;;)
	{
		if( m_LineNumbers[i*2] < pos )
		{
			// Have we found the largest number < programPosition?
			if( min == i ) return m_LineNumbers[i*2+1];

			min = i;
			i = (max + min)/2;
		}
		else if( m_LineNumbers[i*2] > pos )
		{
			// Have we found the smallest number > programPoisition?
			if( max == i ) return m_LineNumbers[i*2+1];

			max = i;
			i = (max + min)/2;
		}
		else
		{
			// We found the exact position
			return m_LineNumbers[i*2+1];
		}
	}
}
int psCScriptFunction::GetNextLineNumber(int lineno, int *pPos) const
{
	if (pPos) *pPos = -1;
	
	if( m_NumLines < 2 ) return -1;

	int nextline = lineno+1;
	int minLine = m_LineNumbers[1];
	int maxLine = m_LineNumbers[m_NumLines - 1];

	if (nextline < minLine)
	{
		if (pPos) *pPos = m_LineNumbers[0];
		return minLine;
	}else if ( nextline > maxLine )
	{
		return maxLine + 1;
	}
	
	for (int index = 1; index < m_NumLines; index += 2)
	{
		if (m_LineNumbers[index] > lineno)
		{
			if (pPos) *pPos = m_LineNumbers[index - 1];
			return m_LineNumbers[index];
		}
	}
		
	if (pPos) *pPos = m_LineNumbers[m_NumLines - 2];
	return maxLine;
}	


int psCScriptFunction::GetPosFromLineNumber(int lineno, int &realLineno) const
{
	realLineno = -1;

	if( m_NumLines < 2 ) return -1;

	// 做二叉搜索
	int max = (int)m_NumLines/2 - 1;
	int min = 0;
	int i = max/2;

	// 此行号是否在此函数内
	int minLine = m_LineNumbers[1];
	int maxLine = m_LineNumbers[m_NumLines - 1];

	if (minLine > lineno) return -1;
	if (maxLine < lineno) return -1;

	if (minLine == lineno) 
	{
		realLineno = minLine;
		return m_LineNumbers[0];
	}
	if (maxLine == lineno) 
	{
		realLineno = maxLine;
		return m_LineNumbers[m_NumLines - 2];
	}

	for(;;)
	{
		int index = i*2;
		if( m_LineNumbers[index+1] < lineno )
		{
			// Have we found the largest number < programPosition?
			if( min == i )	
			{
				realLineno = m_LineNumbers[index+1];
				return m_LineNumbers[index];
			}
	
			min = i;
			i = (max + min)/2;
		}
		else if( m_LineNumbers[index+1] > lineno )
		{
			// Have we found the smallest number > programPoisition?
			if( max == i ) 
			{
				realLineno = m_LineNumbers[index+1];
				return m_LineNumbers[index];
			}

			max = i;
			i = (max + min)/2;
		}
		else
		{
			// We found the exact position	
			realLineno = m_LineNumbers[index+1];
			return m_LineNumbers[index];
		}
	}
}

bool psCScriptFunction::InsertBreakPoint(int lineno)
{
	if ( m_BinaryCode == NULL )
		return false;

	int realLineno = -1;
	int pos = GetPosFromLineNumber(lineno, realLineno);
	if (  pos < 0 || realLineno != lineno )
		return false;

	return InsertBreakPointbyPos( pos );
}


bool psCScriptFunction::InsertBreakPointbyPos( int pos )
{
	BREAK_POINT bp = { pos, *(int*)(m_BinaryCode+pos) };

	if ( bp.bc != BC_SUSPEND )
	{
		m_BreakPoints.push_back( bp );
		*(int*)(m_BinaryCode + pos ) = BC_SUSPEND;	
		return true;
	}else
	{
		return false;
	}
}

bool psCScriptFunction::RemoveBreakPoint(int lineno)
{
	if ( m_BinaryCode == NULL )
		return false;

	int realLineno = -1;
	int pos = GetPosFromLineNumber(lineno, realLineno);
	if (  pos < 0  || realLineno != lineno )
		return false;

	RemoveBreakPointByPos( pos );
	return true;
}

bool psCScriptFunction::RemoveBreakPointByPos(int pos)
{
	for (size_t i = 0; i < m_BreakPoints.size(); ++i)
	{
		if ( m_BreakPoints[i].pos == pos )
		{
			BREAK_POINT bp = m_BreakPoints[i];

			m_BreakPoints[i] = m_BreakPoints[m_BreakPoints.size()-1];
			m_BreakPoints.pop_back();

			*(int*)(m_BinaryCode + bp.pos ) = bp.bc;
			return true;
		}
	}
	return false;
}

bool psCScriptFunction::ToggleBreakPoint(int lineno, bool& isBreakPoint)
{
	isBreakPoint = false;
	if ( m_BinaryCode == NULL  )
		return false;

	int realLineno = -1;
	int pos = GetPosFromLineNumber(lineno, realLineno);

	if (  pos < 0 || realLineno != lineno )
		return false;

	if ( !RemoveBreakPointByPos( pos ) )
	{
		isBreakPoint = true;
		return InsertBreakPointbyPos( pos );
	}else
	{
		return true;
	}
}

void psCScriptFunction::RemoveAllBreakPoints()
{
	for (size_t i = 0; i < m_BreakPoints.size(); ++i)
	{
		BREAK_POINT bp = m_BreakPoints[i];

		*(int*)(m_BinaryCode + bp.pos ) = bp.bc;
	}
	m_BreakPoints.clear();
}

psIBuffer* psCScriptFunction::GetByteCode(int& outsize ) const
{
	outsize    = m_BinaryCodeSize;	
	if ( m_BinaryCodeSize > 0 && m_BinaryCode )
	{
		psBuffer* pBuf = new psBuffer;
		if ( m_BreakPoints.size() > 0 )
		{	
			pBuf->Alloc( m_BinaryCodeSize );
			// copy byte code
			memcpy ( pBuf->GetBufferPointer(), m_BinaryCode, m_BinaryCodeSize );

			// remove breakpoints
			for (size_t b = 0; b < m_BreakPoints.size(); ++b )
			{
				*( (int*)( pBuf->GetBufferPointer() + m_BreakPoints[b].pos )) = m_BreakPoints[b].bc;
			}
		}else
		{
			pBuf->Assign( m_BinaryCode, false );
		}
		return pBuf;
	}else
	{
		return NULL;
	}
}



