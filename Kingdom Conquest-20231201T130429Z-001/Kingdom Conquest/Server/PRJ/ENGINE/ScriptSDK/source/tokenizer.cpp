#include "tokenizer.h"
#include <assert.h>
#include <string.h>
#include <memory.h>

//-------------------------------------------------------------------//
// 全局函数
//-------------------------------------------------------------------//
inline psUINT psHashFunc( const psCHAR* s )
{
	const unsigned int shift    = 6;
	const unsigned int bitsize  = 8 * sizeof(psUINT);
	const psUINT maskCode	    = (~0) << (bitsize - shift);

	psUINT len = (psUINT)ps_strlen(s);
	psUINT r = 0;
	for (psUINT i = 0; i < len; ++i)
	{
		r = (r & maskCode) | (r << shift) | s[i];
	}
	return r;
}
inline bool psIsAlpha( psCHAR chr )
{
	if ( (chr >= PS_T('a') && chr <= PS_T('z')) ||
		(chr >=  PS_T('A') && chr <= PS_T('Z')) ||
		( chr == PS_T('_') ) )
	{
		return true;
	}else
	{
		return false;
	}
}

inline bool psIsDigit( psCHAR chr )
{
	if ( chr >= PS_T('0') && chr <= PS_T('9') )
		return true;
	else
		return false;
}

//----------------------------------------------------------------------
// 关键字结构
struct psSTokenWord
{
	const psCHAR *word;
	psETokenType  tokenType;
};

//----------------------------------------------------------------------
// 保留字结构
struct psSReserveWord
{
	psSReserveWord( const psCHAR* _str, psETokenType _type )
	{
		word	  = _str;
		tokenType = _type;
		hashValue = psHashFunc( word );
	}
	const psCHAR* word;
	psETokenType  tokenType;
	psUINT		  hashValue;
};

//----------------------------------------------------------------------
// 所有的关键字
psSTokenWord g_psTokenWords[] =
{
	/**/
	{PS_T("#define") , ttNumberDefine},
	{PS_T("#undef")  , ttNumberUndef},
	{PS_T("#ifdef")  , ttNumberIfdef},
	{PS_T("#ifndef") , ttNumberIfndef},
	{PS_T("#else")   , ttNumberElse},
	{PS_T("#endif")  , ttNumberEndif},
	{PS_T("#include"), ttNumberInclude},
};

psSTokenWord g_psOperators[] =
{
	{PS_T("+")       , ttPlus},
	{PS_T("-")       , ttMinus},
	{PS_T("*")       , ttStar},
	{PS_T("/")       , ttSlash},
	{PS_T("%")       , ttPercent},
	{PS_T("=")       , ttAssignment},
	{PS_T(".")       , ttDot},

	{PS_T("+=")      , ttAddAssign},
	{PS_T("-=")      , ttSubAssign},
	{PS_T("*=")      , ttMulAssign},
	{PS_T("/=")      , ttDivAssign},
	{PS_T("%=")      , ttModAssign},
	{PS_T("|=")      , ttOrAssign},
	{PS_T("&=")      , ttAndAssign},
	{PS_T("^=")      , ttXorAssign},
	{PS_T("<<=")     , ttShiftLeftAssign},
	{PS_T(">>=")     , ttShiftRightAssign},

	{PS_T("|")       , ttBitOr },
	{PS_T("~")       , ttBitNot},
	{PS_T("^")       , ttBitXor},
	{PS_T("<<")      , ttBitShiftLeft},
	{PS_T(">>")      , ttBitShiftRight},
	{PS_T(";")       , ttEndStatement},
	{PS_T(",")       , ttListSeparator},
	{PS_T("{")       , ttStartStatementBlock},
	{PS_T("}")       , ttEndStatementBlock},
	{PS_T("(")       , ttOpenParenthesis},
	{PS_T(")")       , ttCloseParenthesis},
	{PS_T("[")       , ttOpenBracket},
	{PS_T("]")       , ttCloseBracket},
	{PS_T("?")       , ttQuestion},
	{PS_T(":")      , ttColon},
	{PS_T("::")		, ttField},
	{PS_T("==")      , ttEqual},
	{PS_T("!=")      , ttNotEqual},
	{PS_T("<")       , ttLessThan},
	{PS_T(">")       , ttGreaterThan},
	{PS_T("<=")      , ttLessThanOrEqual},
	{PS_T(">=")      , ttGreaterThanOrEqual},
	{PS_T("++")      , ttInc},
	{PS_T("--")      , ttDec},
	{PS_T("&")       , ttAmp},
	{PS_T("!")       , ttNot},
	{PS_T("||")      , ttOr},
	{PS_T("&&")      , ttAnd},
	{PS_T("^^")      , ttXor},
	{PS_T("...")	 , ttEllipsis},
	{PS_T("->")		 , ttArrow},
};

//----------------------------------------------------------------------
// 所有的保留字
psSReserveWord g_psReservedWords[] = 
{
	psSReserveWord(PS_T("const")    , ttConst),
	psSReserveWord(PS_T("NULL")     , ttNull),
	psSReserveWord(PS_T("true")     , ttTrue),
	psSReserveWord(PS_T("false")    , ttFalse),

	psSReserveWord(PS_T("void")     , ttVoid),
	psSReserveWord(PS_T("char")		, ttInt8),
	psSReserveWord(PS_T("wchar_t")  , ttWChar),	
	psSReserveWord(PS_T("int")      , ttInt),
	psSReserveWord(PS_T("short")    , ttInt16),
	psSReserveWord(PS_T("uint8")    , ttUInt8),
	psSReserveWord(PS_T("uint16")   , ttUInt16),
	psSReserveWord(PS_T("bool")		, ttBool),
	psSReserveWord(PS_T("double")   , ttDouble),
	psSReserveWord(PS_T("float")    , ttFloat),

	psSReserveWord(PS_T("do")       , ttDo),
	psSReserveWord(PS_T("while")    , ttWhile),
	psSReserveWord(PS_T("for")		, ttFor),
	psSReserveWord(PS_T("break")    , ttBreak),
	psSReserveWord(PS_T("continue") , ttContinue),
	psSReserveWord(PS_T("switch")   , ttSwitch),
	psSReserveWord(PS_T("case")     , ttCase), 
	psSReserveWord(PS_T("default")  , ttDefault),
	psSReserveWord(PS_T("if")       , ttIf),	
	psSReserveWord(PS_T("else")     , ttElse),
	psSReserveWord(PS_T("return")   , ttReturn),

	psSReserveWord(PS_T("class")    , ttClass),
	psSReserveWord(PS_T("operator") , ttOperator),
	psSReserveWord(PS_T("public")	, ttPublic ),
	psSReserveWord(PS_T("this")		, ttThis),
	psSReserveWord(PS_T("sizeof")	, ttSizeof),
	psSReserveWord(PS_T("enum")		, ttEnum),
};

const int PS_NUM_TOKEN_WORDS	   = sizeof(g_psTokenWords)/sizeof(g_psTokenWords[0]);
const int PS_NUM_RESERVED_WORDS	   = sizeof(g_psReservedWords)/sizeof(g_psReservedWords[0]);
const int PS_NUM_OPERATORS		   = sizeof(g_psOperators)/sizeof(g_psOperators[0]);
const int PS_RESERVED_WORD_MAX_LEN = 8;

//-------------------------------------------------------------------
// 名称: psGetTokenDefinition
// 说明: 得到标识符的字符串定义
//-------------------------------------------------------------------
extern "C" const psCHAR* psGetTokenDefinition(int tokenType)
{
	if( tokenType == ttUnrecognizedToken			) return PS_T("<unrecognized token>");
	if( tokenType == ttEnd							) return PS_T("<end of file>");
	if( tokenType == ttWhiteSpace					) return PS_T("<white space>");
	if( tokenType == ttOnelineComment				) return PS_T("<one line comment>");
	if( tokenType == ttMultilineComment				) return PS_T("<multiple lines comment>");
	if( tokenType == ttIdentifier					) return PS_T("<identifier>");
	if( tokenType == ttIntConstant					) return PS_T("<integer constant>");
	if( tokenType == ttFloatConstant				) return PS_T("<float constant>");
	if( tokenType == ttDoubleConstant				) return PS_T("<double constant>");
	if( tokenType == ttStringConstant				) return PS_T("<string constant>");
	if( tokenType == ttNonTerminatedStringConstant	) return PS_T("<unterminated string constant>");
	if( tokenType == ttBitsConstant					) return PS_T("<bits constant>");

	// 在关键字各保留字中寻找
	int n;
	for ( n = 0; n < PS_NUM_OPERATORS; ++n )
	{
		if ( g_psOperators[n].tokenType == tokenType )
			return g_psOperators[n].word;
	}
	for(  n = 0; n < PS_NUM_TOKEN_WORDS; n++ )
	{
		if( g_psTokenWords[n].tokenType == tokenType )
			return g_psTokenWords[n].word;
	}
	for(  n = 0; n < PS_NUM_RESERVED_WORDS; n++ )
	{
		if( g_psReservedWords[n].tokenType == tokenType )
			return g_psReservedWords[n].word;
	}
	return PS_T("<unrecognized token>");
}
//-------------------------------------------------------------------
// 类名: psCTokenizer
// 说明: 
//-------------------------------------------------------------------
psCTokenizer::psCTokenizer(void)
{

}

psCTokenizer::~psCTokenizer(void)
{
}


psETokenType psCTokenizer::GetToken(const psCHAR* source, int sourceLength, int *tokenLength)
{
	m_pSource = source;
	m_SourceLength = sourceLength;

	m_NumLines = 0;
	ParaseToken();

	// Copy the output to the token
	*tokenLength = m_TokenLength;

	return m_TokenType;
}

int psCTokenizer::ParaseToken()
{
	if ( m_SourceLength > 0 )
	{
		psCHAR chr = m_pSource[0];

#if 1
		switch( chr )
		{
		case PS_T('"'):		InStringConstant();return 0;
		case PS_T('\''):	InCharConstant();return 0;
		case PS_T('L'):
            if(m_SourceLength > 1)
            {
			    psCHAR chr1 = m_pSource[1];
			    switch( chr1 )
			    {
			    case PS_T('"'):		InStringConstant();return 0;
			    case PS_T('\''):	InCharConstant();return 0;
			    }
            }
            // By Jiangli: 这里不要break
			// 注意这里没有"L"，别复制错了
		case PS_T('A'): case PS_T('B'): case PS_T('C'): case PS_T('D'): case PS_T('E'): case PS_T('F'): case PS_T('G'): case PS_T('H'): case PS_T('I'): case PS_T('J'): case PS_T('K'): case PS_T('M'): case PS_T('N'): case PS_T('O'): case PS_T('P'): case PS_T('Q'): case PS_T('R'): case PS_T('S'): case PS_T('T'): case PS_T('U'): case PS_T('V'): case PS_T('W'): case PS_T('X'): case PS_T('Y'): case PS_T('Z'):
		case PS_T('a'): case PS_T('b'): case PS_T('c'): case PS_T('d'): case PS_T('e'): case PS_T('f'): case PS_T('g'): case PS_T('h'): case PS_T('i'): case PS_T('j'): case PS_T('k'): case PS_T('l'): case PS_T('m'): case PS_T('n'): case PS_T('o'): case PS_T('p'): case PS_T('q'): case PS_T('r'): case PS_T('s'): case PS_T('t'): case PS_T('u'): case PS_T('v'): case PS_T('w'): case PS_T('x'): case PS_T('y'): case PS_T('z'):
		case PS_T('_'):		
			{
				InIdentifier();	
			}return 0;
		case PS_T('0'): case PS_T('1'): case PS_T('2'): case PS_T('3'): case PS_T('4'): case PS_T('5'): case PS_T('6'): case PS_T('7'): case PS_T('8'): case PS_T('9'):
			{
				InNumberConstant();
			}return 0;
		case PS_T('/'):
			{
				if( m_SourceLength > 1 )
				{
					psCHAR chr1 = m_pSource[1];
					switch( chr1 )
					{
					case PS_T('/'):
						{
							// One-line comment
							// Find the length
							int n;
							for( n = 2; n < m_SourceLength; n++ )
							{
								if( m_pSource[n] == PS_T('\n'))
									break;
							}

							m_TokenType = ttOnelineComment;
							m_TokenLength = n+1;

							++m_NumLines;
						}return 0;
					case PS_T('*'):
						{
							// Multi-line comment
							// Find the length
							int n;
							for( n = 2; n < m_SourceLength-1; )
							{
								if ( m_pSource[n] == PS_T('\n') )
									++m_NumLines;

								if ( m_pSource[n++] == PS_T('*') && m_pSource[n] == PS_T('/') )
									break;
							}
							if ( m_pSource[n+1] == PS_T('\n') )
								++m_NumLines;

							m_TokenType = ttMultilineComment;
							m_TokenLength = n+1;
						}return 0;
					case PS_T('='):
						{
							// IsOperator
							m_TokenLength = 2;
							m_TokenType   = ttDivAssign;
						}return 0;
					}
				}

				// IsOperator
				m_TokenLength = 1;
				m_TokenType	  = ttSlash;
			}return 0;
		case PS_T('+'):
			{
				if( m_SourceLength > 1 )
				{
					psCHAR chr1 = m_pSource[1];
					switch( chr1 )
					{
					case PS_T('='):
						{
							m_TokenLength = 2;
							m_TokenType   = ttAddAssign;
						}return 0;
					case PS_T('+'):
						{
							m_TokenLength = 2;
							m_TokenType   = ttInc;
						}return 0;
					}
				}

				m_TokenLength = 1;
				m_TokenType   = ttPlus;
			}return 0;
		case PS_T('-'):
			{
				if( m_SourceLength > 1 )
				{
					psCHAR chr1 = m_pSource[1];
					switch( chr1 )
					{
					case PS_T('='):
						{
							m_TokenLength = 2;
							m_TokenType   = ttSubAssign;
						}return 0;
					case PS_T('-'):
						{
							m_TokenLength = 2;
							m_TokenType   = ttDec;
						}return 0;
					case PS_T('>'):
						{
							m_TokenLength = 2;
							m_TokenType   = ttArrow;
						}return 0;
					}
				}

				m_TokenLength = 1;
				m_TokenType   = ttMinus;
			}return 0;
		case PS_T('*'):
			{
				if ( m_SourceLength > 1 && m_pSource[1] == PS_T('=') )
				{
					m_TokenLength = 2;
					m_TokenType   = ttMulAssign;
				}else
				{
					m_TokenLength = 1;
					m_TokenType   = ttStar;
				}
			}return 0;
		case PS_T('%'):
			{
				if ( m_SourceLength > 1 && m_pSource[1] == PS_T('=') )
				{
					m_TokenLength = 2;
					m_TokenType	  = ttModAssign;
				}else
				{
					m_TokenLength = 1;
					m_TokenType   = ttPercent;
				}
			}return 0;
		case PS_T('='):
			{
				if ( m_SourceLength > 1 && m_pSource[1] == PS_T('=') )
				{
					m_TokenLength = 2;
					m_TokenType   = ttEqual;
				}else
				{
					m_TokenLength = 1;
					m_TokenType	  = ttAssignment;
				}
			}return 0;
		case PS_T('.'): 
			{
				if ( m_SourceLength > 2 && m_pSource[1] == PS_T('.') && m_pSource[2] == PS_T('.') )
				{
					m_TokenLength = 3;
					m_TokenType   = ttEllipsis;
				}else
				{
					m_TokenLength = 1;
					m_TokenType   = ttDot;
				}
			}return 0;
		case PS_T('{'):
			{
				m_TokenLength = 1;
				m_TokenType   = ttStartStatementBlock;
			}return 0;
		case PS_T('}'):
			{
				m_TokenLength = 1;
				m_TokenType   = ttEndStatementBlock;
			}return 0;
		case PS_T('['):
			{
				m_TokenLength = 1;
				m_TokenType   = ttOpenBracket;
			}return 0;
		case PS_T(']'):
			{
				m_TokenLength = 1;
				m_TokenType   = ttCloseBracket;
			}return 0;
		case PS_T('('):
			{
				m_TokenLength = 1;
				m_TokenType   = ttOpenParenthesis;
			}return 0;
		case PS_T(')'):
			{
				m_TokenLength = 1;
				m_TokenType   = ttCloseParenthesis;
			}return 0;
		case PS_T(':'):
			{
				if ( m_SourceLength > 1 && m_pSource[1] == PS_T(':') )
				{
					m_TokenLength = 2;
					m_TokenType = ttField;
				}else
				{
					m_TokenLength = 1;
					m_TokenType   = ttColon;
				}
			}return 0;
		case PS_T(';'):
			{
				m_TokenLength = 1;
				m_TokenType   = ttEndStatement;
			}return 0;
		case PS_T(','):
			{
				m_TokenLength = 1;
				m_TokenType   = ttListSeparator;
			}return 0;
		case PS_T('?'):
			{
				m_TokenLength = 1;
				m_TokenType   = ttQuestion;
			}return 0;
		case PS_T('~'):
			{
				m_TokenLength = 1;
				m_TokenType   = ttBitNot;
			}return 0;
		case PS_T('|'):
			{
				if( m_SourceLength > 1 )
				{
					psCHAR chr1 = m_pSource[1];
					switch( chr1 )
					{
					case PS_T('|'):
						{
							m_TokenLength = 2;
							m_TokenType   = ttOr;
						}return 0;
					case PS_T('='):
						{
							m_TokenLength = 2;
							m_TokenType   = ttOrAssign;
						}return 0;
					}
				}

				m_TokenLength = 1;
				m_TokenType   = ttBitOr;
			}return 0;
		case PS_T('!'):
			{
				if (m_SourceLength > 1 && m_pSource[1] == PS_T('=') )
				{
					m_TokenLength = 2;
					m_TokenType   = ttNotEqual;
				}else
				{
					m_TokenLength = 1;
					m_TokenType   = ttNot;
				}
			}return 0;
		case PS_T('&'):
			{
				if( m_SourceLength > 1 )
				{
					psCHAR chr1 = m_pSource[1];
					switch( chr1 )
					{
					case PS_T('&'):
						{
							m_TokenLength = 2;
							m_TokenType   = ttAnd;
						}return 0;
					case PS_T('='):
						{
							m_TokenLength = 2;
							m_TokenType   = ttAndAssign;
						}return 0;
					}
				}

				m_TokenLength = 1;
				m_TokenType   = ttAmp;
			}return 0;
		case PS_T('^'):
			{
				if( m_SourceLength > 1 )
				{
					psCHAR chr1 = m_pSource[1];
					switch( chr1 )
					{
					case PS_T('='):
						{
							m_TokenLength = 2;
							m_TokenType   = ttXorAssign;
						}return 0;
					case PS_T('^'):
						{
							m_TokenLength = 2;
							m_TokenType   = ttXor;
						}return 0;
					}
				}

				m_TokenLength = 1;
				m_TokenType   = ttBitXor;
			}return 0;
		case PS_T('<'):
			{
				if( m_SourceLength > 1 )
				{
					psCHAR chr1 = m_pSource[1];
					switch( chr1 )
					{
					case PS_T('='):
						{
							m_TokenLength = 2;
							m_TokenType   = ttLessThanOrEqual;
						}return 0;
					case PS_T('<'):
						{
							if ( m_SourceLength > 2 && m_pSource[2] == PS_T('=') )
							{
								m_TokenLength = 3;
								m_TokenType   = ttShiftLeftAssign;
							}else
							{
								m_TokenLength = 2;
								m_TokenType   = ttBitShiftLeft;
							}
						}return 0;
					}
				}

				m_TokenLength = 1;
				m_TokenType   = ttLessThan;
			}return 0;
		case PS_T('>'):
			{
				if( m_SourceLength > 1 )
				{
					psCHAR chr1 = m_pSource[1];
					switch( chr1 )
					{
					case PS_T('='):
						{
							m_TokenLength = 2;
							m_TokenType   = ttGreaterThanOrEqual;
						}return 0;
					case PS_T('>'):
						{
							if ( m_SourceLength > 2 && m_pSource[2] == PS_T('=') )
							{
								m_TokenLength = 3;
								m_TokenType   = ttShiftRightAssign;
							}else
							{
								m_TokenLength = 2;
								m_TokenType   = ttBitShiftRight;
							}
						}return 0;
					}
				}

				m_TokenLength = 1;
				m_TokenType   = ttGreaterThan;
			}return 0;
		case PS_T(' '): case PS_T('\t'): case PS_T('\r'): case PS_T('\n'): case PS_T('\0'): 
			{
				// 2011-9-4 曾经输入脚本在结束符\0之后还有一串\0，所以处理的时候注意一下
				int n;
				for (n = 0; n < m_SourceLength; ++n)
				{
					switch( m_pSource[n] )
					{
					case PS_T(' '): case PS_T('\t'): case PS_T('\r'):case PS_T('\0'):
						break;
					case PS_T('\n'):
						++m_NumLines;
						break;
					default:
						goto skip_whitespace_loop;
					}
				}
skip_whitespace_loop:
				if (n>0)
				{
					m_TokenType = ttWhiteSpace;
					m_TokenLength = n;
				}
				return 0;
			}break;
		case PS_T('#'):
			{
				if ( IsKeyword() )	  return 0;
			}break;
		default:
			{
				// 检查是否为注释
				// NOTE:必需在IsOperator之前,
				// 因为有可能注释符'/'会被当作操作符
				//if ( IsComment() )	  return 0;
				
				// 栓查是否为操作符
				//if ( IsOperator() )  return 0;

				// 检查是否为空白符
				//if ( IsWhiteSpace() ) return 0;
		
				// 检查是否为其它关键字
				//if ( IsKeyword() )	  return 0;
			}
		}
#else
		if ( chr == PS_T('L') && m_SourceLength > 1 ) 
		{
			if (m_pSource[1] == PS_T('"') )
			{
				InStringConstant();
				return 0;
			}else if ( m_pSource[1] == PS_T('\'') )
			{
				InCharConstant();
				return 0;
			}else 
			{
				InIdentifier();	
				return 0;
			}
		}else if ( chr == PS_T('"') )
		{
			InStringConstant();
			return 0;
		}else if ( chr == PS_T('\'') )
		{
			InCharConstant();
			return 0;
		}else if ( psIsAlpha(chr) )
		{
			InIdentifier();	
			return 0;
		}else if ( psIsDigit(chr) )
		{
			InNumberConstant();
			return 0;
		}else
		{		
			// 检查是否为注释
			// NOTE:必需在IsOperator之前,
			// 因为有可能注释符'/'会被当作操作符
			if ( IsComment() )	  return 0;
			
			// 栓查是否为操作符
			if ( IsOperator() )  return 0;

			// 检查是否为空白符
			if ( IsWhiteSpace() ) return 0;
	
			// 检查是否为其它关键字
			if ( IsKeyword() )	  return 0;
		}
#endif
	}

	// If none of the above this is an unrecognized token
	// We can find the length of the token by advancing
	// one step and trying to identify a token there
	m_TokenType  = ttUnrecognizedToken;
	m_TokenLength = 1;
	return -1;
}

void psCTokenizer::InIdentifier()
{
	// Starting with letter or underscore
	assert( psIsAlpha(m_pSource[0]) );

	m_TokenType = ttIdentifier;
	m_TokenLength = 1;


#if 1
	for( int n = 1; n < m_SourceLength; n++ )
	{
		switch(m_pSource[n])
		{
		// 非常险恶：ParaseToken()的字符表里面，没有字母'L'。这里有L。直接粘过来就完蛋了。
		case PS_T('A'): case PS_T('B'): case PS_T('C'): case PS_T('D'): case PS_T('E'): case PS_T('F'): case PS_T('G'): case PS_T('H'): case PS_T('I'): case PS_T('J'): case PS_T('K'): case PS_T('L'): case PS_T('M'): case PS_T('N'): case PS_T('O'): case PS_T('P'): case PS_T('Q'): case PS_T('R'): case PS_T('S'): case PS_T('T'): case PS_T('U'): case PS_T('V'): case PS_T('W'): case PS_T('X'): case PS_T('Y'): case PS_T('Z'):
		case PS_T('a'): case PS_T('b'): case PS_T('c'): case PS_T('d'): case PS_T('e'): case PS_T('f'): case PS_T('g'): case PS_T('h'): case PS_T('i'): case PS_T('j'): case PS_T('k'): case PS_T('l'): case PS_T('m'): case PS_T('n'): case PS_T('o'): case PS_T('p'): case PS_T('q'): case PS_T('r'): case PS_T('s'): case PS_T('t'): case PS_T('u'): case PS_T('v'): case PS_T('w'): case PS_T('x'): case PS_T('y'): case PS_T('z'):
		case PS_T('_'):	
			//no break
		case PS_T('0'): case PS_T('1'): case PS_T('2'): case PS_T('3'): case PS_T('4'): case PS_T('5'): case PS_T('6'): case PS_T('7'): case PS_T('8'): case PS_T('9'):
			m_TokenLength++;
			break;
		default:
			goto skip_InIdentifier;
		}
	}
#else
	for( int n = 1; n < m_SourceLength; n++ )
	{
		psCHAR chr = m_pSource[n];
		if ( psIsAlpha( chr ) || psIsDigit( chr ) )
		{
			m_TokenLength++;
		}else
		{
			break;
		}
	}
#endif

skip_InIdentifier:
	// Make sure the identifier isn't a reserved keyword
	if ( m_TokenLength > PS_RESERVED_WORD_MAX_LEN )
		return;

	psCHAR test[ PS_RESERVED_WORD_MAX_LEN + 1];
	memcpy( test, m_pSource, m_TokenLength * sizeof(psCHAR) );
	test[ m_TokenLength ] = 0;

	psUINT hashValue = psHashFunc( test );
	// 用switch-case展开，用于优化。
	/* 准备工作：将所有关键字用Excel整理成“关键字\t数字UL\tTokenType”这种形式，例如：
	const	1673460980UL	ttConst
	NULL	20796236UL	ttNull
	TRUE	30883173UL	ttTru
	然后用vim命令，自动生成case代码
	:%s/\(\w*\)\t\(\d*UL\)\t\(\w*\)/case \2:\r\tif (ps_strcmp(PS_T("\1"),test)!=0) return;\r\tm_TokenType=\3;\r\treturn;/g
	*/
#if 1
	switch( hashValue )
	{
	case 1673460980UL:
		if (ps_strcmp(PS_T("const"),test)!=0) return;
		m_TokenType=ttConst;
		return;
	case 20796236UL:
		if (ps_strcmp(PS_T("NULL"),test)!=0) return;
		m_TokenType=ttNull;
		return;
	case 30883173UL:
		if (ps_strcmp(PS_T("true"),test)!=0) return;
		m_TokenType=ttTrue;
		return;
	case 1736891621UL:
		if (ps_strcmp(PS_T("false"),test)!=0) return;
		m_TokenType=ttFalse;
		return;
	case 31390308UL:
		if (ps_strcmp(PS_T("void"),test)!=0) return;
		m_TokenType=ttVoid;
		return;
	case 26122354UL:
		if (ps_strcmp(PS_T("char"),test)!=0) return;
		m_TokenType=ttInt8;
		return;
	case 4253497332UL:
		if (ps_strcmp(PS_T("wchar_t"),test)!=0) return;
		m_TokenType=ttWChar;
		return;
	case 433140UL:
		if (ps_strcmp(PS_T("int"),test)!=0) return;
		m_TokenType=ttInt;
		return;
	case 1940323572UL:
		if (ps_strcmp(PS_T("short"),test)!=0) return;
		m_TokenType=ttInt16;
		return;
	case 1973878072UL:
		if (ps_strcmp(PS_T("uint8"),test)!=0) return;
		m_TokenType=ttUInt8;
		return;
	case 2109688950UL:
		if (ps_strcmp(PS_T("uint16"),test)!=0) return;
		m_TokenType=ttUInt16;
		return;
	case 26147820UL:
		if (ps_strcmp(PS_T("bool"),test)!=0) return;
		m_TokenType=ttBool;
		return;
	case 1876310885UL:
		if (ps_strcmp(PS_T("double"),test)!=0) return;
		m_TokenType=ttDouble;
		return;
	case 1740044404UL:
		if (ps_strcmp(PS_T("float"),test)!=0) return;
		m_TokenType=ttFloat;
		return;
	case 6511UL:
		if (ps_strcmp(PS_T("do"),test)!=0) return;
		m_TokenType=ttDo;
		return;
	case 2007407461UL:
		if (ps_strcmp(PS_T("while"),test)!=0) return;
		m_TokenType=ttWhile;
		return;
	case 424946UL:
		if (ps_strcmp(PS_T("for"),test)!=0) return;
		m_TokenType=ttFor;
		return;
	case 1674467435UL:
		if (ps_strcmp(PS_T("break"),test)!=0) return;
		m_TokenType=ttBreak;
		return;
	case 4255579493UL:
		if (ps_strcmp(PS_T("continue"),test)!=0) return;
		m_TokenType=ttContinue;
		return;
	case 4154939624UL:
		if (ps_strcmp(PS_T("switch"),test)!=0) return;
		m_TokenType=ttSwitch;
		return;
	case 26090725UL:
		if (ps_strcmp(PS_T("case"),test)!=0) return;
		m_TokenType=ttCase;
		return;
	case 1736924020UL:
		if (ps_strcmp(PS_T("default"),test)!=0) return;
		m_TokenType=ttDefault;
		return;
	case 6758UL:
		if (ps_strcmp(PS_T("if"),test)!=0) return;
		m_TokenType=ttIf;
		return;
	case 26664165UL:
		if (ps_strcmp(PS_T("else"),test)!=0) return;
		m_TokenType=ttElse;
		return;
	case 4124531950UL:
		if (ps_strcmp(PS_T("return"),test)!=0) return;
		m_TokenType=ttReturn;
		return;
	case 1672879347UL:
		if (ps_strcmp(PS_T("class"),test)!=0) return;
		m_TokenType=ttClass;
		return;
	case 4287060978UL:
		if (ps_strcmp(PS_T("operator"),test)!=0) return;
		m_TokenType=ttOperator;
		return;
	case 1972296291UL:
		if (ps_strcmp(PS_T("public"),test)!=0) return;
		m_TokenType=ttPublic;
		return;
	case 30841459UL:
		if (ps_strcmp(PS_T("this"),test)!=0) return;
		m_TokenType=ttThis;
		return;
	case 4193147878UL:
		if (ps_strcmp(PS_T("sizeof"),test)!=0) return;
		m_TokenType=ttSizeof;
		return;
	case 26672493UL:
		if (ps_strcmp(PS_T("enum"),test)!=0) return;
		m_TokenType=ttEnum;
		return;
	}
#else
	for ( int i = 0; i < PS_NUM_RESERVED_WORDS; ++i )
	{
		if ( g_psReservedWords[i].hashValue == hashValue )
		{
			if ( ps_strcmp( test, g_psReservedWords[i].word ) == 0 )
			{
				// 这是一个保留字
				m_TokenType  = g_psReservedWords[i].tokenType;
				return;
			}
		}
	}
#endif
}

void psCTokenizer::InNumberConstant()
{
	assert( psIsDigit(m_pSource[0]) );

	int n;
	// Is it a hexadecimal number?
	if ( m_SourceLength > 1 && (m_pSource[1] == PS_T('x') || m_pSource[1] == PS_T('X')))
	{
		for( n = 2; n < m_SourceLength; n++ )
		{
			psCHAR chr = m_pSource[n];
			if( !(chr >= PS_T('0') && chr <= PS_T('9') ) &&
				!(chr >= PS_T('a') && chr <= PS_T('f'))  &&
				!(chr >= PS_T('A') && chr <= PS_T('F')) )
			{
				break;
			}
		}
		m_TokenType = ttBitsConstant;
		m_TokenLength = n;
		return;
	}

	for( n = 1; n < m_SourceLength; n++ )
	{
		if( !psIsDigit( m_pSource[n] ) )
			break;
	}

	if ( n < m_SourceLength && m_pSource[n] == PS_T('.') )
	{
		// float number
		n++;
		for( ; n < m_SourceLength; n++ )
		{
			if( !psIsDigit(m_pSource[n]) )
				break;
		}
		if (n < m_SourceLength && (m_pSource[n] == PS_T('e') || m_pSource[n] == PS_T('E')))
		{
			// IEEE float number
			n++;
			if( n < m_SourceLength && (m_pSource[n] == PS_T('-') || m_pSource[n] == PS_T('+')) )
				n++;

			for( ; n < m_SourceLength; n++ )
			{
				if( !psIsDigit(m_pSource[n]) )
					break;
			}
		}

		if( n < m_SourceLength && (m_pSource[n] == PS_T('f') || m_pSource[n] == PS_T('F')) )
		{
			m_TokenType = ttFloatConstant;
			m_TokenLength = n + 1;
		}else
		{
#ifdef PS_USE_DOUBLE_AS_FLOAT
			m_TokenType = ttFloatConstant;
#else
			m_TokenType = ttDoubleConstant;
#endif
			m_TokenLength = n;
		}
		return;
	}
	m_TokenType = ttIntConstant;
	m_TokenLength = n;
}

void psCTokenizer::InCharConstant()
{	
	// Char constant
	int n = 1;
	psETokenType tokenType = ttCharConstant;
	if ( m_pSource[0] == PS_T('L') )
	{
		++n;
		tokenType = ttWCharConstant;
	}	
	bool evenSlashes = true;
	for( ; n < m_SourceLength; n++ )
	{
		psCHAR chr = m_pSource[n];
		if ( chr == PS_T('\n') ) 
			break;

		if( chr== PS_T('\'') && evenSlashes )
		{
			m_TokenType = tokenType;
			m_TokenLength = n + 1;
			return;
		}	
		if( chr == PS_T('\\') ) 
			evenSlashes = !evenSlashes; 
		else 
			evenSlashes = true;
	}
	m_TokenType = ttNonTerminatedCharConstant;
	m_TokenLength = n;
}

void psCTokenizer::InStringConstant()
{
	// String constant	
	bool evenSlashes = true;
	int n = 1;
	psETokenType tokenType = ttStringConstant;
	if ( m_pSource[0] == PS_T('L') )
	{
		++n;
		tokenType = ttWStringConstant;
	}
	for( ; n < m_SourceLength; n++ )
	{
		psCHAR chr = m_pSource[n];
		if( chr == PS_T('\n') ) break;
		if( chr == PS_T('"') && evenSlashes )
		{
			m_TokenType   = tokenType;
			m_TokenLength = n+1;
			return;
		}
		if( chr == PS_T('\\') ) 
			evenSlashes = !evenSlashes; 
		else 
			evenSlashes = true;
	}
	m_TokenType = ttNonTerminatedStringConstant;
	m_TokenLength = n;
}

bool psCTokenizer::IsWhiteSpace()
{
	//这里实际上包含了最后一个字符'\0'
	const psCHAR Whitespaces[] = PS_T(" \t\r\n");
	int n;
	for (n = 0; n < m_SourceLength; ++n)
	{
		bool isWhiteSpace = false;
		for (int w = 0; w < int (sizeof(Whitespaces)/sizeof(psCHAR)); ++w )
		{
			if (m_pSource[n] == Whitespaces[w])
			{
				isWhiteSpace = true;
				if ( Whitespaces[w] == PS_T('\n') )
				{
					++m_NumLines;
				}
				break;
			}
		}

		if (!isWhiteSpace)
			break;
	}

	if (n > 0)
	{
		m_TokenType = ttWhiteSpace;
		m_TokenLength = n;
		return true;
	}else
	{
		return false;
	}
}

bool psCTokenizer::IsComment()
{
	if (m_SourceLength < 2)
		return false;

	if (m_pSource[0] != PS_T('/'))
		return false;

	if (m_pSource[1] == PS_T('/'))
	{
		// One-line comment
		// Find the length
		int n;
		for( n = 2; n < m_SourceLength; n++ )
		{
			if( m_pSource[n] == PS_T('\n'))
				break;
		}

		m_TokenType = ttOnelineComment;
		m_TokenLength = n+1;

		++m_NumLines;
		return true;
	}else
	{
		if (m_pSource[1] == PS_T('*'))
		{
			// Multi-line comment
			// Find the length
			int n;
			for( n = 2; n < m_SourceLength-1; )
			{
				if ( m_pSource[n] == PS_T('\n') )
					++m_NumLines;

				if ( m_pSource[n++] == PS_T('*') && m_pSource[n] == PS_T('/') )
					break;
			}
			if ( m_pSource[n+1] == PS_T('\n') )
				++m_NumLines;

			m_TokenType = ttMultilineComment;
			m_TokenLength = n+1;
			return true;
		}else
			return false;
	}
}

bool psCTokenizer::IsOperator()
{
	switch( m_pSource[0] )
	{
	case PS_T('+'):
		{
			if (  m_pSource[1] == PS_T('=')  && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttAddAssign;
			}
			else if ( m_pSource[1] == PS_T('+')  && m_SourceLength > 1)
			{
				m_TokenLength = 2;
				m_TokenType   = ttInc;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttPlus;
			}
		}break;
	case PS_T('-'):
		{
			if ( m_pSource[1] == PS_T('=') && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttSubAssign;
			}else if ( m_pSource[1] == PS_T('-')  && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttDec;
			}else if ( m_pSource[1] == PS_T('>')  && m_SourceLength > 1)
			{
				m_TokenLength = 2;
				m_TokenType   = ttArrow;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttMinus;
			}
		}break;
	case PS_T('*'):
		{
			if ( m_pSource[1] == PS_T('=')  && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttMulAssign;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttStar;
			}
		}break;
	case PS_T('/'):
		{
			if ( m_pSource[1] == PS_T('=')  && m_SourceLength > 1)
			{
				m_TokenLength = 2;
				m_TokenType   = ttDivAssign;
			}else
			{
				m_TokenLength = 1;
				m_TokenType	  = ttSlash;
			}
		}break;
	case PS_T('%'):
		{
			if ( m_pSource[1] == PS_T('=')  && m_SourceLength > 1)
			{
				m_TokenLength = 2;
				m_TokenType	  = ttModAssign;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttPercent;
			}
		}break;
	case PS_T('='):
		{
			if ( m_pSource[1] == PS_T('=')  && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttEqual;
			}else
			{
				m_TokenLength = 1;
				m_TokenType	  = ttAssignment;
			}
		}break;
	case PS_T('.'): 
		{
			if ( m_pSource[1] == PS_T('.') && m_pSource[2] == PS_T('.')  && m_SourceLength > 2 )
			{
				m_TokenLength = 3;
				m_TokenType   = ttEllipsis;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttDot;
			}
		}break;
	case PS_T('{'):
		{
			m_TokenLength = 1;
			m_TokenType   = ttStartStatementBlock;
		}break;
	case PS_T('}'):
		{
			m_TokenLength = 1;
			m_TokenType   = ttEndStatementBlock;
		}break;
	case PS_T('['):
		{
			m_TokenLength = 1;
			m_TokenType   = ttOpenBracket;
		}break;
	case PS_T(']'):
		{
			m_TokenLength = 1;
			m_TokenType   = ttCloseBracket;
		}break;
	case PS_T('('):
		{
			m_TokenLength = 1;
			m_TokenType   = ttOpenParenthesis;
		}break;
	case PS_T(')'):
		{
			m_TokenLength = 1;
			m_TokenType   = ttCloseParenthesis;
		}break;
	case PS_T(':'):
		{
			if ( m_pSource[1] == PS_T(':') && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType = ttField;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttColon;
			}
		}break;
	case PS_T(';'):
		{
			m_TokenLength = 1;
			m_TokenType   = ttEndStatement;
		}break;
	case PS_T(','):
		{
			m_TokenLength = 1;
			m_TokenType   = ttListSeparator;
		}break;
	case PS_T('?'):
		{
			m_TokenLength = 1;
			m_TokenType   = ttQuestion;
		}break;
	case PS_T('~'):
		{
			m_TokenLength = 1;
			m_TokenType   = ttBitNot;
		}break;
	case PS_T('|'):
		{
			if ( m_pSource[1] == PS_T('|')  && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttOr;
			}else if ( m_pSource[1] == PS_T('=') && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttOrAssign;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttBitOr;
			}
		}break;
	case PS_T('!'):
		{
			if (m_pSource[1] == PS_T('=') && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttNotEqual;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttNot;
			}
		}break;
	case PS_T('&'):
		{
			if ( m_pSource[1] == PS_T('&')  && m_SourceLength > 1)
			{
				m_TokenLength = 2;
				m_TokenType   = ttAnd;
			}else if(  m_pSource[1] == PS_T('=')  && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType  = ttAndAssign;
			}else 
			{
				m_TokenLength = 1;
				m_TokenType   = ttAmp;
			}
		}break;
	case PS_T('^'):
		{
			if ( m_pSource[1] == PS_T('=')  && m_SourceLength > 1)
			{
				m_TokenLength = 2;
				m_TokenType	  = ttXorAssign;
			}else if ( m_pSource[1] == PS_T('^') && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType	  = ttXor;
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttBitXor;
			}
		}break;
	case PS_T('<'):
		{
			if ( m_pSource[1] == PS_T('=')  && m_SourceLength > 1)
			{
				m_TokenLength = 2;
				m_TokenType   = ttLessThanOrEqual;
			}else if ( m_pSource[1] == PS_T('<')  && m_SourceLength > 1)
			{
				if ( m_pSource[2] == PS_T('=')  && m_SourceLength > 2 )
				{
					m_TokenLength = 3;
					m_TokenType   = ttShiftLeftAssign;
				}else
				{
					m_TokenLength = 2;
					m_TokenType   = ttBitShiftLeft;
				}
			}else
			{
				m_TokenLength = 1;
				m_TokenType   = ttLessThan;
			}
		}break;
	case PS_T('>'):
		{
			if ( m_pSource[1] == PS_T('=')  && m_SourceLength > 1 )
			{
				m_TokenLength = 2;
				m_TokenType   = ttGreaterThanOrEqual;
			}else if ( m_pSource[1] == PS_T('>')  && m_SourceLength > 1 )
			{
				if ( m_pSource[2] == PS_T('=') && m_SourceLength > 2 )
				{
					m_TokenLength = 3;
					m_TokenType   = ttShiftRightAssign;
				}else
				{
					m_TokenLength = 2;
					m_TokenType   = ttBitShiftRight;
				}
			}else
			{
				m_TokenLength  = 1;
				m_TokenType    = ttGreaterThan;
			}
		}break;
	default: return false;
	}
	return true;
}

bool psCTokenizer::IsKeyword()
{
	// 填允所有的关键字序号
	int words[PS_NUM_TOKEN_WORDS];
	int n;
	for (n = 0; n < PS_NUM_TOKEN_WORDS; ++n)
		words[n] = n;

	int numWords = PS_NUM_TOKEN_WORDS;
	int lastPossible = -1;

	for (n = 0; n < m_SourceLength && numWords > 0; ++n)
	{
		// 排除所有不符合的关键字
		for ( int i = 0; i < numWords; ++i)
		{
			// 如果遇到当前关键字结束标识
			if ( g_psTokenWords[words[i]].word[n] == PS_T('\0' ))
			{
				if (numWords > 1)
				{	
					// 如是有多个候选关键字, 记下最后此关键字
					lastPossible = words[i];

					// 从关键字列表中删除此关键字
					words[i--] = words[--numWords];
					continue;
				}else
				{
					// 如是果只有一个候选关键字,那么肯定就是最后匹配的关键字
					m_TokenType = g_psTokenWords[words[i]].tokenType;
					m_TokenLength = n;
					return true;
				}
			}

			// 如果当前字符与当前关键字字符不符, 从关键字列表中删除此关键字
			if ( m_pSource[n] != g_psTokenWords[words[i]].word[n] )
			{
				words[i--] = words[--numWords];
			}
		}

	}

	// 如果还有多个候选关键字
	if ( numWords )
	{
		// 如果有一个候选关键字在此位置结束, 则返回此关键字
		for (int i = 0; i < PS_NUM_TOKEN_WORDS; ++i)
			if (g_psTokenWords[i].word[n] == PS_T('\0'))
			{
				m_TokenType = g_psTokenWords[words[i]].tokenType;
				m_TokenLength = n;
				return true;
			}
	}

	// 也有可能是最后一个被排除的关键字
	if ( lastPossible > -1)
	{
		m_TokenType   = g_psTokenWords[lastPossible].tokenType;
		m_TokenLength = (int)ps_strlen(g_psTokenWords[lastPossible].word);
		return true;
	}
	return false;
}


