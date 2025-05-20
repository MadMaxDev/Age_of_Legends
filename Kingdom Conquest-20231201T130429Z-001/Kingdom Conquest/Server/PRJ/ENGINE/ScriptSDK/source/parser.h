/*
SCRIPT        = (FUNCTION | DECLARATION | INCLUDE | CLASSDECL | OPOVERLOAD | ENUM )*
ENUM		  = 'enum' IDENTIFIER? {'  (ENUMCONST ',')* ENUMCONST? '}' ';'
ENUMCONST	  = IDENTIFIER ( = CONSTANT ) ?
TYPE          = 'const'? DATATYPE
TYPEMOD       = ('&'|'*')?
FUNCTION      = TYPE TYPEMOD IDENTIFIER PARAMLIST BLOCK
INCLUDE		  = '#include' STRING;
CLASSDECL	  = 'class' IDENTIFY (':'  'public' IDENTIFY )? {' (CONSTRUCTOR|DESTRUCTOR|PROPERYDECL|FUNCTION|ENUM)* '}' ';'
PROPERTYDECL  = TYPE TYPEMOD IDENTIFY ';'
CONSTRUCTOR	  = IDENTIFER PARAMLIST BLOCK
DESTRUCTOR	  = IDENTIFIER PARAMLIST BLOCK
OPOVERLOAD	  = TYPE TYPEMOD 'operator' OP PARAMLIST BLOCK

DATATYPE      = REALTYPE | IDENTIFIER
REALTYPE      = 'void' | 'bool' | 'float' | 'int' | 'uint' | 'int8' | 'int16' | 'uint8' | 'uint16' 
PARAMLIST     = '(' (TYPE TYPEMOD IDENTIFIER? (',' TYPE TYPEMOD IDENTIFIER?)*)? '...'? ')'
BLOCK         = '{' (DECLARATION | STATEMENT)* '}'
DECLARATION   = TYPE TYPEMOD VARIABLEDIM ('=' INITEXPR )?|(PARAMLIST)? (',' TYPEMOD VARIABLEDIM ('=' INITEXPR )?|(PARAMLIST)? )* ';'
VARIABLEDIM	  = IDENTIFIER ('[' EXPRESSION ']')*
INITEXPR	  = ASSIGNMENT | '{' INITEXPTR (',' INITEXPTR)* '}'

STATEMENT     = BLOCK | IF | SWITCH | WHILE | DOWHILE | RETURN | EXPRSTATEMENT | BREAK | CONTINUE | ENUM
BREAK         = 'break' ';'
CONTINUE      = 'continue' ';'
EXPRSTATEMENT = ASSIGNMENT? ';'
FOR           = 'for' '(' (DECLARATION | EXPRSTATEMENT) EXPRSTATEMENT ASSIGNMENT? ')' STATEMENT
IF            = 'if' '(' ASSIGNMENT ')' STATEMENT ('else' STATEMENT)?
SWITCH		  = 'switch' '(' ASSIGNMENT ')' '{' (CASE)* }'
CASE		  = 'case'|'defulat' (SATEMENT)*
WHILE         = 'while' '(' ASSIGNMENT ')' STATEMENT
DOWHILE       = 'do' STATEMENT 'while' '(' ASSIGNMENT ')' ';'
RETURN        = 'return' ASSIGNMENT? ';'
ASSIGNMENT    = CONDITION (ASSIGNOP ASSIGNMENT)?
CONDITION     = EXPRESSION ('?' ASSIGNMENT ':' ASSIGNMENT)?
EXPRESSION    = TERM (OP TERM)*
TERM          = PRE* VALUE POST*
VALUE         = '(' ASSIGNMENT ')' | CONSTANT | IDENTIFIER | FUNCTIONCALL | CONVERSION | SIZEOF | TYPE_CAST
IDENTIFIER_V  = (IDENTIFIER)? (::)? IDNETIFIER
SIZEOF		  = 'sizeof' '(' ASSIGNMENT|REALTYPE|IDENTIFIER ')' 
TYPE_CAST	  = '<' TYPE TYPEMOD  '>' '(' ASSIGNMENT ')' 
PRE           = '-' | '+' | 'not' | '++' | '--' | '~' | '&'
POST          = '++' | '--' | ('.' | '->') (IDENTIFIER | FUNCTIONCALL) | '[' ASSIGNMENT ']'
FUNCTIONCALL  = (IDENTIFIER '::')? IDENTIFIER ARGLIST
ARGLIST       = '(' (ASSIGNMENT (',' ASSIGNMENT)*)? ')'
CONSTANT      = "abc" | 123 | 123.1 | 'true' | 'false' | 'this' | 0xFFFF
OP            = 'and' | 'or' |
'==' | '!=' | '<' | '<=' | '>=' | '>' |
'+' | '-' | '*' | '/' | '%' | '|' | '&' | '^' | '<<' | '>>' | '>>>'
ASSIGNOP      = '=' | '+=' | '-=' | '*=' | '/=' | '%=' | '|=' | '&=' | '^=' | '<<=' | '>>=' | '>>>='
MACRODEFINE   = '#define' MACRO TEXT
MACRO		  = TEXT ( '(' TEXT (',' TEXT)* ')'
*/

#ifndef __PS_PARSER_H__
#define __PS_PARSER_H__

#include "tokenizer.h"
#include "string.h"
#include "macro.h"
#include "scriptcode.h"

class psCBuilder;
class psCScriptNode;

class psCParser
{
public:
	psCParser(psCBuilder* pBuilder);
	~psCParser(void);

	int ParseScript(psCScriptCode* script);
	int ParseFunctionDeclaration(psCScriptCode* script);
	int ParseOpOverloadDeclaration( psCScriptCode* script );

	int ParseDataType(psCScriptCode* script);
	int ParsePropertyDeclaration(psCScriptCode* script);

	psCScriptNode* GetScriptNode() { return m_pScriptNode; }

	MacroListMap_t& GetMacroList() { return m_MacroList; }

protected:
	void Reset();
	void ParseTokenList();
	void AddToken( const psSToken& t )
	{
		if ( m_IncludeLevel == 0 )
		{
			// 如果是被包含的代码, 不要添加到token列表
			m_TokenList.push_back( t );
			m_TokenList.back().index = (int)m_TokenList.size() - 1;
		}
	}
	int  GetToken(psCString& text, int pos, psSToken* token, int& numLine);
	inline int  ParseToken(psSToken* token)
	{ 
		int numLine = 0;
		m_SourcePos = GetToken(m_pScriptCode->m_Code, m_SourcePos, token, numLine );
		return numLine;
	}
	inline void RewindToSrc( const psSToken* token )
	{
		m_SourcePos = token->pos;
	}

	inline void GetToken( psSToken* token )
	{
		assert( m_CurToken < (int)m_TokenList.size() );
		*token = ( m_TokenList[m_CurToken++] );
	}

	inline void RewindTo(const psSToken* token)	
	{	
		assert ( token->index >= 0 && token->index < (int)m_TokenList.size()  );
		m_CurToken = token->index;
	}

	void Error(const psCHAR* text, const psSToken* token);

	void DoPrepost();
	bool DoMacroReplace(psSMacro& macro);
	bool DoMacroReplace(psSMacro& macro, psCString& text);
	void DoMacroParamReplace(psSMacro& macro, psCString& text, std::vector<psCString>& params);

	psCScriptNode* ParseScript();

	void ParseInclude();
	void ParseIfdef();
	void ParseMacroDefine(bool bAddToMacroList = true);
	void ParseMacroUndef(bool bRemove = true);
	void ParseMacro(psCString& name, psSMacro& macro);
	void ParseMacroText(psCString& text);
	bool IsMacroDefined(psCString& name);

	psCScriptNode* ParseClassDecl();
	psCScriptNode* ParsePropertyDecl();
	psCScriptNode* ParseConstructor();
	psCScriptNode* ParseDestructor();

	psCScriptNode* ParseOperatorOverload();
	psCScriptNode* ParseOpOverloadDeclaration();

	psCScriptNode* ParseFunction();
	psCScriptNode* ParseFunctionDeclaration();
	psCScriptNode* ParseGlobaVar();

	psCScriptNode* ParseType(bool allowConst = true);
	psCScriptNode* ParseRealType();
	psCScriptNode* ParseDataType();
	psCScriptNode* ParseTypeMod();

	psCScriptNode* ParseParameterList();
	psCScriptNode* ParseArrayDim();
	psCScriptNode* ParseArgList();

	psCScriptNode* ParseStatementBlock();
	psCScriptNode* ParseDeclaration(bool bGlobal = false);
	psCScriptNode* ParseVariableDim();
	psCScriptNode* ParseInitExpression();

	psCScriptNode* ParseStatement();
	psCScriptNode* ParseEnum();
	psCScriptNode* ParesEnumConstant();
	psCScriptNode* ParseExpressionStatement();

	psCScriptNode* ParseFor();
	psCScriptNode* ParseWhile();
	psCScriptNode* ParseDoWhile();

	psCScriptNode* ParseIf();
	psCScriptNode* ParseSwitch();
	psCScriptNode* ParseCase();

	psCScriptNode* ParseBreak();
	psCScriptNode* ParseContinue();
	psCScriptNode* ParseReturn();

	psCScriptNode* ParseAssignment();
	psCScriptNode* ParseAssignOperator();
	psCScriptNode* ParseCondition();
	psCScriptNode* ParseExpression();
	psCScriptNode* ParseExprTerm();
	psCScriptNode* ParseExprValue();
	psCScriptNode* ParseSizeof();
	psCScriptNode* ParseTypeCast();
	psCScriptNode* ParseExprOperator();
	psCScriptNode* ParseExprPreOp();
	psCScriptNode* ParseExprPostOp();

	psCScriptNode* ParseIdentifier();
	psCScriptNode* ParseIdentifierValue();
	psCScriptNode* ParseConstant();

	psCScriptNode* ParseStringConstant();
	psCScriptNode* ParseFunctionCall();
	psCScriptNode* ParseToken(int token);
	psCScriptNode* ParseOneOf(int* tokens, int num);

	bool IsGlobalVar();
	bool IsDeclaration();
	bool IsFunctionCall();
	bool IsClassProperty();
	bool IsOperatorOverload();

	bool IsDataType(int tokenType);
	bool IsOperator(int tokenType);
	bool IsPreOperator(int tokenType);
	bool IsPostOperator(int tokenType);
	bool IsOpAllowOverload(int tokenType );
	bool IsConstant(int tokenType);
	bool IsRealType(int tokenType);
	bool IsAssignOperator(int tokenType);
    bool IsTypeMod( int tokenType )
	{
		if ( tokenType == ttStar || tokenType == ttAmp )
			return true;
		else
			return false;
	}

	psCString ExpectedToken(const psCHAR *token);
	psCString ExpectedTokens(const psCHAR *token1, const psCHAR *token2);
	psCString ExpectedOneOf(int *tokens, int count);

	psCBuilder*	     m_pBuilder;
	psCScriptCode*   m_pScriptCode;
	psCScriptNode*   m_pScriptNode;
	psCTokenizer     m_Tokenizer;	
	int			     m_SourcePos;
	bool		     m_bIsSyntaxError;
	bool		     m_bErrorWhileParsing;
	int				 m_IncludeLevel;
	int				 m_Lineno;
	std::vector< psSToken > m_TokenList;
	int						m_CurToken;
	MacroListMap_t   m_MacroList;
};

#endif // __PS_PARSER_H__
