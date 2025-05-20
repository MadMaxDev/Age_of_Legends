#include "parser.h"
#include "builder.h"
#include "scriptnode.h"
#include "scriptcode.h"
#include "texts.h"
#include <assert.h>

psCParser::psCParser(psCBuilder* builder)
{
	m_pBuilder = builder;
	m_pScriptNode = 0;
	m_pScriptCode = 0;
	m_IncludeLevel = 0;
	m_Lineno	   = 1;
	m_bErrorWhileParsing = false;
	m_bIsSyntaxError = false;
}

psCParser::~psCParser(void)
{
	Reset();
}

void psCParser::Reset()
{
	m_bErrorWhileParsing = false;
	m_bIsSyntaxError = false;
	m_SourcePos = 0;

	if (m_pScriptNode) 
	{
		delete m_pScriptNode;
	}
	m_TokenList.clear();
	m_pScriptNode  = 0;
	m_pScriptCode  = 0;
	m_IncludeLevel = 0;
	m_CurToken	   = 0;
	m_Lineno	   = 1;
}

//------------------------------------------------------------------
// Prepost
//------------------------------------------------------------------
void psCParser::DoPrepost()
{
	// 根据第一个token决定子结点的类型
	psSToken t;
	do
	{
		m_Lineno += ParseToken(&t);
		if (t.type == ttNumberDefine)
		{
			RewindToSrc(&t);
			ParseMacroDefine();
			if (m_bIsSyntaxError) return;

			// 把预处理指今删除,但不要改变源码的大小和行数
			m_pScriptCode->m_Code.FillExcept(t.pos, m_SourcePos - t.pos - 1, PS_T(' '), PS_T('\n'));
		}else if(t.type == ttNumberUndef)
		{
			RewindToSrc(&t);
			ParseMacroUndef();
			if (m_bIsSyntaxError) return;

			// 把预处理指今删除,但不要改变源码的大小和行数
			m_pScriptCode->m_Code.FillExcept(t.pos, m_SourcePos - t.pos - 1, PS_T(' '), PS_T('\n'));
		}else if (t.type == ttNumberIfdef || t.type == ttNumberIfndef)
		{
			RewindToSrc(&t);
			ParseIfdef();
			if (m_bIsSyntaxError) return;

		}else if (t.type == ttNumberElse)
		{
			Error(TXT_UNEXPECTED_NUMBERELSE, &t);
			return;
		}else if (t.type == ttNumberEndif)
		{
			Error(TXT_UNEXPECTED_NUMBERENDIF, &t);	
			return;
		}else if (t.type == ttNumberInclude)
		{
			RewindToSrc(&t);
			ParseInclude();
			if (m_bIsSyntaxError) return;

		}else if (t.type == ttIdentifier)
		{
			psCString name;
			name.Copy( &m_pScriptCode->m_Code[t.pos], t.length );

			MacroListMap_t::iterator it = m_MacroList.find( name );
			if ( it != m_MacroList.end() )
			{
				RewindToSrc(&t);

				// 进行宏替换
				if (!DoMacroReplace( it->second ))
				{
					AddToken(t);
				}
			}else if ( name[0] == PS_T('_') && name[1] == PS_T('_') )
			{
				if ( name == PS_T("__FILE__") )
				{
					psSMacro macro;
					#ifdef PS_UNICODE
						macro.text = PS_T("L\"");
					#else
						macro.text = PS_T("\"");
					#endif
					psCString filename( m_pScriptCode->m_FileName );
					for (size_t i = 0; i < filename.GetLength(); ++i )
					{
						// 把反斜杆变为正斜杆
						// 即可解决转义符代来的问题，又可以把unix和windows的文件路径统一起来.
						if ( filename[i] == PS_T('\\') )
						{
							filename[i] = PS_T('/');
						}
					}

					macro.text += filename;
					macro.text += PS_T("\"");

					RewindToSrc(&t);

					// 进行宏替换
					DoMacroReplace( macro );
				}else if ( name == PS_T("__LINE__") )
				{
					RewindToSrc(&t);

					psSMacro macro;
					macro.text.Format(PS_T("%d"), m_Lineno);

					// 进行宏替换
					DoMacroReplace( macro );
				}else
				{
					AddToken( t );
				}
			}else
			{
				AddToken( t);
			}
		}else
		{
			AddToken( t );
		}
	}while (t.type != ttEnd);

	AddToken( t );
}	


void psCParser::ParseInclude()
{
	psSToken t;
	ParseToken(&t);
	if (t.type != ttNumberInclude)
	{
		Error(ExpectedToken(PS_T("#include")).AddressOf(), &t);
		return;
	}

	int startPos = t.pos;

	ParseToken(&t);
	if (t.type != ttStringConstant)
	{
		Error(TXT_EXPECTED_STRING, &t);
		return;
	}else
	{
		if (m_pScriptCode->CountLines(startPos, t.pos) > 0)
		{
			t.pos = startPos;
			Error(TXT_INCLUDE_EXPECT_FILENAME, &t);
			return;
		}
	}

	// 
	psCString filename;

	psCScriptCode* subcode = new psCScriptCode();
	filename.Copy(&m_pScriptCode->m_Code[t.pos+1], t.length - 2);

	if ( filename[0] != PS_T('/') )
	{	
		m_pScriptCode->GetAbosolutePath( filename );
		subcode->m_FileName = filename;
	}else
	{
		subcode->m_FileName = filename.SubString(1, filename.GetLength() - 1 );
	}

	// 计算行号偏移
	subcode->m_LineOffset = m_Lineno;

	assert(m_pBuilder);
	if ( !m_pBuilder->ReadFile( subcode->m_FileName, subcode->m_Code ) )
	{
		psCString msg;
		msg.Format( TXT_CANT_READ_FILE_s, subcode->m_Name.AddressOf() );
		Error(msg.AddressOf(), &t );
	}else
	{
		m_IncludeLevel++;

		if ( m_IncludeLevel > 512 )
		{
			psCString msg;
			msg.Format( TXT_INCLUDE_FILE_TOO_MANY_MAX_d, 512 );
			Error( msg.AddressOf(), &t );
		}else
		{
			psCScriptCode* oldCode = m_pScriptCode;
			int oldLineNo = m_Lineno;

			// 给包含代码段作预处理
			m_pScriptCode = subcode;
			m_SourcePos   = 0;
			m_Lineno	  = 1;
			DoPrepost();

			// 解析出子代码段的行号
			subcode->ExtractLineNumbers();

			// 删除子代码段所有的行号字符
			// 和单行注释
			psCHAR* pCode = subcode->m_Code.AddressOf();
			size_t length = subcode->m_Code.GetLength();
			for (size_t i = 0; i < length; ++i )
			{
				if ( pCode[i] == PS_T('/') && pCode[i+1] == PS_T('/') )
				{
					size_t k;
					for ( k = i; k < length; ++k )
					{
						psCHAR chr = pCode[k];
						pCode[k] = PS_T(' ');
						if ( chr == PS_T('\n') )
							break;
					}
					i = k;
				}else if ( pCode[i] == PS_T('\n') )
				{
					pCode[i] = PS_T(' ');
				}
			}

			// 恢复原来的代码段位置,继续处理
			m_SourcePos   = startPos;
			m_pScriptCode = oldCode;
			m_Lineno	  = oldLineNo;

			m_IncludeLevel--;
		}
	}
	// 替换'#include' 
	m_pScriptCode->m_Code.Replace( startPos, t.pos + t.length - startPos, subcode->m_Code.AddressOf() );

	// 把子代码段插入
	m_pScriptCode->m_SubCodes.push_back( subcode );
}

void psCParser::ParseIfdef()
{
	bool bTrue;

	psSToken ifToken;
	ParseToken(&ifToken);
	if (ifToken.type == ttNumberIfdef)
	{
		bTrue = true;
	}
	else if (ifToken.type == ttNumberIfndef)
	{
		bTrue = false;
	}
	else
	{
		Error(ExpectedTokens(PS_T("#ifdef"), PS_T("#ifndef")).AddressOf(), &ifToken);
		return;
	}

	psSToken t;
	ParseToken(&t);

	psCString name;
	name.Copy(&m_pScriptCode->m_Code[t.pos], t.length);

	if (t.type != ttIdentifier)
	{
		psCString msg;
		msg.Format(TXT_UNEXPECTED_TOKEN_s, name.AddressOf());
		Error(msg.AddressOf(), &t);
		return;
	}else
	{
		if (m_pScriptCode->CountLines(ifToken.pos, t.pos) > 0)
		{
			Error(TXT_IFDEF_EXPECTED_IDENTIFIER, &ifToken);
			return;
		}
	}

	// 更新#ifdef/#ifndef的长度
	ifToken.length = t.pos + t.length - ifToken.pos;

	psSToken elseToken;

	bool bHasElse = false;
	// parse #ifdef - #else ? - #endif
	int level = 1;
	do
	{
		ParseToken(&t);
		if (t.type == ttNumberIfdef)
		{
			level++;
		}else if (t.type == ttNumberIfndef)
		{
			level++;
		}else if (t.type == ttNumberElse)
		{
			if (level == 1)
			{
				if (!bHasElse)
				{
					bHasElse = true;
					elseToken = t;
				}else
				{
					Error(TXT_UNEXPECTED_NUMBERELSE, &t);
					return;
				}
			}
		}else if (t.type == ttNumberEndif)
		{
			level--;

			if (level == 0)
			{	
				// Match over
				bool bMatchIf = IsMacroDefined(name);
				if (!bTrue)
					bMatchIf = !bMatchIf;

				if (bMatchIf)
				{
					// 填充#ifdef/#ifndef
					m_pScriptCode->m_Code.Fill(ifToken.pos, ifToken.length, PS_T(' '));

					if (bHasElse)
					{	
						// 填充#else - #endif
						m_pScriptCode->m_Code.FillExcept(elseToken.pos, t.pos + t.length - elseToken.pos, PS_T(' '), PS_T('\n'));					
					}else
					{
						m_pScriptCode->m_Code.Fill(t.pos, t.length, PS_T(' '));
					}

					// 更新当前源码位置
					m_SourcePos = ifToken.pos;	
					return;
				}else
				{
					if (bHasElse)
					{		
						// 填充#ifdef/#ifndef - #else
						m_pScriptCode->m_Code.FillExcept(ifToken.pos, elseToken.pos + elseToken.length - ifToken.pos, PS_T(' '), PS_T('\n'));

						// 填充#endif
						m_pScriptCode->m_Code.Fill(t.pos, t.length, PS_T(' '));

						// 更新当前源码位置
						m_SourcePos = elseToken.pos;
					}else
					{
						// 填充#ifdef/#ifndef - #endif
						m_pScriptCode->m_Code.FillExcept(ifToken.pos, t.pos + t.length - ifToken.pos, PS_T(' '), PS_T('\n'));

						// 更新当前源码位置
						m_SourcePos = t.pos + t.length;
					}

					return;
				}
			}
		}
	}while (t.type != ttEnd);

	Error(TXT_UNEXPECTED_END_OF_FILE, &t);
}
void psCParser::ParseMacroDefine(bool bAddToMacroList)
{
	psSToken t;
	ParseToken(&t);
	if (t.type != ttNumberDefine)
	{
		Error(ExpectedToken(PS_T("#ifdef")).AddressOf(), &t);
		return;
	}
	int startPos = t.pos;

	psSMacro macro;
	psCString name;
	ParseMacro(name, macro);
	if (m_bIsSyntaxError) return;

	ParseToken(&t);
	if (m_pScriptCode->CountLines(startPos, t.pos) > 0)
	{
		// 空白宏
		macro.text = PS_T("");
		RewindToSrc(&t);
	}else
	{
		RewindToSrc(&t);
		ParseMacroText(macro.text);
		if (m_bIsSyntaxError) return;
	}
	if (!bAddToMacroList) return;

	if (IsMacroDefined(name))
	{
		psCString txt;
		txt.Format(TXT_MACRO_s_REDEFINITION, name.AddressOf());
		Error(txt.AddressOf(), &t);
	}else if ( name == PS_T("__FILE__") || name == PS_T("__LINE__") )
	{
		psCString txt;
		txt.Format(TXT_MACRO_s_IS_RESREVED, name.AddressOf() );
		Error(txt.AddressOf(), &t);
	}else
	{
		m_MacroList[name] = macro;
	}
}

void psCParser::ParseMacroUndef(bool bRemove)
{
	psSToken t;
	ParseToken(&t);
	if (t.type != ttNumberUndef)
	{
		Error(ExpectedToken(PS_T("#undef")).AddressOf(), &t);
		return;
	}
	int startPos = t.pos;

	ParseToken(&t);
	psCString name;
	name.Copy(&(m_pScriptCode->m_Code[t.pos]), t.length);

	if (t.type != ttIdentifier)
	{
		psCString txt;
		txt.Format(TXT_UNEXPECTED_TOKEN_s, name.AddressOf());
		Error(txt.AddressOf(), &t);
		return;
	}else
	{
		if (m_pScriptCode->CountLines(startPos, t.pos) > 0)
		{
			t.pos = startPos;
			Error(TXT_IFDEF_EXPECTED_IDENTIFIER, &t);
			return;
		}
	}
	if (!bRemove) return;

	MacroListMap_t::iterator it = m_MacroList.find(name);
	if (it != m_MacroList.end())
		m_MacroList.erase(it);

}

void psCParser::ParseMacro(psCString& name, psSMacro& macro)
{
	psSToken t;
	ParseToken(&t);
	name.Copy(&(m_pScriptCode->m_Code[t.pos]), t.length);
	if(t.type != ttIdentifier)
	{
		psCString msg;
		msg.Format(TXT_UNEXPECTED_TOKEN_s, name.AddressOf());
		Error(msg.AddressOf(), &t);
		return;
	}

	int endname = t.pos + t.length;

	ParseToken(&t);
	if (t.type == ttOpenParenthesis && t.pos == endname)
	{
		psCString param;
		for (;;)
		{
			ParseToken(&t);

			if (t.type == ttCloseParenthesis)
			{
				return;
			}
			else if (t.type == ttListSeparator)
			{
				continue;
			}else if (t.type == ttIdentifier)
			{	
				param.Copy(&(m_pScriptCode->m_Code[t.pos]), t.length);

				macro.add_param(param);
			}else if (t.type == ttEnd)
			{
				Error(ExpectedToken(PS_T(")")).AddressOf(), &t);
				return;
			}else
			{
				param.Copy(&(m_pScriptCode->m_Code[t.pos]), t.length);
				psCString txt;
				txt.Format(TXT_UNEXPECTED_TOKEN_s, param.AddressOf());
				Error(txt.AddressOf(), &t);
				return;
			}
		}
	}else
	{
		m_SourcePos = endname;
	}

}

void psCParser::ParseMacroText(psCString& text)
{
	int startPos = m_SourcePos;
	int sourceLength = (int)m_pScriptCode->m_Code.GetLength();

	for (; m_SourcePos < sourceLength; m_SourcePos++)
	{
		psCHAR chr = m_pScriptCode->m_Code[m_SourcePos];
		if (chr == PS_T('/'))
		{
			psCHAR chr2 = m_pScriptCode->m_Code[m_SourcePos+1];
			if (chr2 == PS_T('/') || chr2 == PS_T('*'))
			{
				// 如果遇到注释符, 记下结束的位置
				break;
			}
		}else if (chr == PS_T('\n') || chr == PS_T('\r'))
		{
			break;
		}
	}
	int len = m_SourcePos - startPos;
	if (len > 0)
	{
		text.Copy(&(m_pScriptCode->m_Code[startPos]), len);
	}else
	{
		text = PS_T("");
	}
}

bool psCParser::IsMacroDefined(psCString& name)
{
	assert(m_pBuilder);

	MacroListMap_t::iterator it = m_MacroList.find(name);
	if (it != m_MacroList.end())
		return true;

	return false;
}

bool psCParser::DoMacroReplace(psSMacro& macro, psCString& text)
{
	std::vector<psCString> params;	
	psCString paramTxt;
	psSToken t;
	int numLine;

	int pos = 0;	

	// Get name token
	pos = GetToken(text, pos, &t, numLine);
	int startPos = t.pos;

	pos = GetToken(text, pos, &t, numLine);
	if (t.type == ttOpenParenthesis)
	{
		int level = 1;

		bool bOk = false;
		int  paramStart = t.pos + t.length;

		// Parse macro parameters	
		params.clear();
		do
		{	
			pos = GetToken(text, pos, &t, numLine);

			if (t.type == ttOpenParenthesis)
			{
				level++;
			}
			else if (t.type == ttCloseParenthesis)
			{
				level--;
			}else if (t.type == ttListSeparator)
			{
				if ( paramStart >= 0 && level == 1 )
				{
					paramTxt.Copy( &text[paramStart], t.pos - paramStart );
					params.push_back( paramTxt );

					paramStart = t.pos + t.length;
				}
			}		
			if (level == 0)
			{
				bOk = true;
				if ( paramStart > 0 )
				{
					paramTxt.Copy( &text[paramStart], t.pos - paramStart );
					params.push_back(paramTxt);
				}
				break;
			}
		}while (t.type != ttEnd  );

		// 如不能正确匹配括号,则不作宏替换
		if (!bOk)
			return false;
	}

	// 看宏参是否匹配	
	if (macro.param_size() == params.size())
	{
		// 先作宏参替换
		psCString code;		
		DoMacroParamReplace( macro, code, params);

		// 最后替换到源码中
		text.Replace(startPos, pos - startPos, code.AddressOf() );
	}
	return true;
}

void psCParser::DoMacroParamReplace( psSMacro& macro, psCString& text,  std::vector<psCString>& params)
{	
	text = macro.text;
	if (macro.params.empty())
		return;

	int pos = 0, type;	
	int len;
	
	psCString name;
	int sourceLength = (int)text.GetLength();
	do 
	{	
		// Get a token
		do 
		{
			if (pos >= sourceLength )	
			{
				type = ttEnd;
				len = 0;
			}else
			{
				type = m_Tokenizer.GetToken(&text[pos], sourceLength - pos, &len);
			}

			if (type == ttIdentifier)
			{	
				name.Copy(&text[pos], len);

				for (size_t i = 0; i < macro.param_size(); ++i)
				{
					if (macro.get_param(i) == name)
					{
						pos += text.Replace(pos, len, params[i].AddressOf() );

						// 重新得到源码长度
						sourceLength = (int)text.GetLength();
						break;
					}
				}
			}

			// update source
			pos += len;
		} while(type == ttWhiteSpace ||
			type == ttOnelineComment ||
			type == ttMultilineComment);

	}while(type != ttEnd);
}

bool psCParser::DoMacroReplace(psSMacro& macro)
{
	psSToken t;
	ParseToken(&t);

	if (t.type == ttIdentifier)
	{
		int startPos = t.pos;
		int length = t.length;

		if (!macro.params.empty())
		{	
			// 如果是带参宏
			ParseToken(&t);
			if (t.type == ttOpenParenthesis)
			{
				// 更新长度大小
				// 由于Token之间可能会有空格,不能直接相加(length += t.lenght)
				length = t.pos + t.length - startPos;
				int level = 1;
				bool bOk = false;
				do
				{
					ParseToken(&t);
					length = t.pos + t.length - startPos;
					if (t.type == ttOpenParenthesis)
					{
						level++;
					}else if (t.type == ttCloseParenthesis)
					{
						level--;
					}

					if (level == 0)
					{
						bOk = true;
						break;
					}
				} while(t.type != ttEnd);

				// 如果不能成功匹配,则不进行宏替换
				if (!bOk)
					return false;
			}else
			{
				Error(ExpectedToken(PS_T("(")).AddressOf(), &t);
				return false;
			}
		}

		psCString text;
		text.Copy(&m_pScriptCode->m_Code[startPos], length);

		if (text != macro.text)
		{
			// 进行宏替换
			DoMacroReplace(macro, text);

			//替换到源码
			m_pScriptCode->m_Code.Replace(startPos, length, text.AddressOf() );  

			m_SourcePos = startPos;
			return true;
		}
	}
	return false;
}
//----------------------- End Prepost -----------------------------//

int psCParser::ParseFunctionDeclaration(psCScriptCode* script)
{
	Reset();
	m_pScriptCode = script; 
	ParseTokenList();

	m_pScriptNode = ParseFunctionDeclaration();

	if (m_bErrorWhileParsing)
		return -1;

	return 0;
}

int psCParser::ParseOpOverloadDeclaration( psCScriptCode* script )
{
	Reset();
	m_pScriptCode = script; 
	ParseTokenList();
	m_pScriptNode = ParseOpOverloadDeclaration();

	if (m_bErrorWhileParsing)
		return -1;

	return 0;
}

int psCParser::ParseDataType(psCScriptCode* script)
{
	Reset();
	m_pScriptCode = script;
	ParseTokenList();
	m_pScriptNode = new psCScriptNode(snDataType);

	m_pScriptNode->AddChildLast(ParseType(false));
	if (m_bIsSyntaxError) return -1;

	if (m_bErrorWhileParsing)
		return -1;
	return 0;

}

int psCParser::ParsePropertyDeclaration(psCScriptCode* script)
{
	Reset();
	m_pScriptCode = script;

	ParseTokenList();

	m_pScriptNode = new psCScriptNode(snDeclaration);

	m_pScriptNode->AddChildLast(ParseType());
	if (m_bIsSyntaxError) return -1;

	m_pScriptNode->AddChildLast(ParseTypeMod());
	if (m_bIsSyntaxError) return -1;

	m_pScriptNode->AddChildLast(ParseIdentifier());
	if (m_bIsSyntaxError) return -1;

	psSToken t;
	GetToken( &t );
	if ( t.type == ttOpenBracket )
	{
		do 
		{
			GetToken(&t);
			if (t.type == ttCloseBracket )
				return 0;

			RewindTo(&t);
			m_pScriptNode->AddChildLast( ParseConstant() );
			if ( m_bIsSyntaxError ) return -1;
			
			// 必需是整型常量
			if ( t.type != ttIntConstant &&
				 t.type != ttBitsConstant ) 
			{
				return -1;
			}
		}while ( t.type != ttEnd );

		// Error
		return -1;
	}
	return 0;
}
//---------------------------------------------------------------
// Parse script
//---------------------------------------------------------------
int psCParser::ParseScript(psCScriptCode* script)
{
	Reset();

	m_pScriptCode = script;

	// 重新定位到源码头
	m_SourcePos = 0;

	// 得到所有的预处理宏
	m_MacroList.clear();
	assert(m_pBuilder);
	m_pBuilder->GetAllMacros(m_MacroList);

	// 对脚本源码进行预处理
	// 比如:(宏替换), 构造token列表
	DoPrepost();
	if (m_bIsSyntaxError) return -1;

	// 重新解析源码的行号
	m_pScriptCode->ExtractLineNumbers();

	// 分析脚本
	m_pScriptNode = ParseScript();

	if (m_bErrorWhileParsing)
		return -1;

	return 0;
}

void psCParser::ParseTokenList()
{
	m_CurToken = 0;
	psSToken t;
	do 
	{
		ParseToken( &t );
		t.index = m_CurToken++;
		m_TokenList.push_back( t );
	}while ( t.type != ttEnd );
	m_CurToken = 0;
}
 
psCScriptNode* psCParser::ParseFunctionDeclaration()
{
	psCScriptNode* node = new psCScriptNode(snFunction);

	node->AddChildLast(ParseType());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseTypeMod());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseIdentifier());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseParameterList());
	if (m_bIsSyntaxError) return node;

	return node;
}

psCScriptNode* psCParser::ParseClassDecl()
{
	psCScriptNode* node = new psCScriptNode(snClassDecl);
	psSToken t;
	GetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	if (t.type != ttClass)
	{
		Error(ExpectedToken(PS_T("class")).AddressOf(), &t);
		return node;
	}

	node->AddChildLast(ParseIdentifier());
	if (m_bIsSyntaxError) return node;

	GetToken(&t);
	if ( t.type == ttColon )
	{
		GetToken(&t);
		if ( t.type != ttPublic )
		{
			Error( ExpectedToken(PS_T("public")).AddressOf(), &t);
			return node;
		}
	
		node->AddChildLast( ParseIdentifier() );
		if (m_bIsSyntaxError) return node;	

		GetToken(&t);
	}

	if (t.type != ttStartStatementBlock)
	{
		Error(ExpectedToken(PS_T("{")).AddressOf(), &t);
		return node;
	}

	do 
	{
		GetToken(&t);
		if (t.type == ttEndStatementBlock)
			break;
		else
		{
			psSToken t2;
			GetToken(&t2);
			RewindTo(&t);
					
			if (t.type == ttEnum)
			{
				node->AddChildLast( ParseEnum() );
			}else if (t.type == ttIdentifier && t2.type == ttOpenParenthesis)
				node->AddChildLast(ParseConstructor());
			else if (t.type == ttBitNot)
			{
				node->AddChildLast(ParseDestructor());
			}else
			{
				if (IsClassProperty())
					node->AddChildLast(ParsePropertyDecl());
				else
				{
					if (IsOperatorOverload())
						node->AddChildLast(ParseOperatorOverload());
					else
						node->AddChildLast(ParseFunction());
				}
			}
			if (m_bIsSyntaxError) return node;
		}

	}while(t.type != ttEnd);

	GetToken(&t);
	if (t.type != ttEndStatement)
	{
		Error(ExpectedToken(PS_T(";")).AddressOf(), &t);
	}
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}

psCScriptNode* psCParser::ParsePropertyDecl()
{
	psCScriptNode* node = new psCScriptNode(snPropertyDecl);

	psSToken t;
	GetToken(&t);
	RewindTo(&t);
	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseType());
	if (m_bIsSyntaxError) return node;

	do 
	{
		node->AddChildLast(ParseTypeMod());
		if ( m_bIsSyntaxError ) return node;

		node->AddChildLast(ParseVariableDim());	
		if (m_bIsSyntaxError) return node;

		GetToken(&t);
		if (t.type == ttListSeparator)
			continue;
		else if (t.type == ttEndStatement)
		{
			node->UpdateSourcePos(t.pos, t.length);
			return node;
		}else
		{
			RewindTo(&t);
		}
	}while (t.type != ttEnd);

	t.pos = node->GetTokenPos();
	Error(ExpectedToken(PS_T(";")).AddressOf(), &t);
	return node;
}

psCScriptNode* psCParser::ParseConstructor()
{
	psCScriptNode* node = new psCScriptNode(snConstructor);
	psSToken t;
	GetToken(&t);
	RewindTo(&t);
	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseIdentifier());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseParameterList());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseStatementBlock());
	return node;
}

psCScriptNode* psCParser::ParseDestructor()
{
	psCScriptNode* node = new psCScriptNode(snDestructor);
	psSToken t;
	GetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	if (t.type != ttBitNot )
	{
		Error(ExpectedToken(PS_T("~")).AddressOf(), &t);
		return node;
	}

	node->AddChildLast(ParseIdentifier());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseParameterList());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseStatementBlock());
	return node;
}

psCScriptNode* psCParser::ParseOpOverloadDeclaration()
{
	psSToken t;
	GetToken(&t);
	RewindTo(&t);
	psCScriptNode* node = new psCScriptNode(snOpOverload);
	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseType());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseTypeMod());
	if (m_bIsSyntaxError) return node;

	GetToken(&t);
	if (t.type != ttOperator)
	{
		Error(ExpectedToken(PS_T("operator")).AddressOf(), &t);
		return node;
	}

	GetToken(&t);
	if ( !IsOpAllowOverload(t.type) )
	{
		psCString msg, name;
		name.Copy(&m_pScriptCode->m_Code[t.pos], t.length);

		msg.Format(TXT_UNEXPECTED_TOKEN_s, name.AddressOf() );
		Error(msg.AddressOf(), &t);
		return node;
	}
	if ( t.type == ttOpenBracket )
	{
		GetToken(&t);
		if ( t.type != ttCloseBracket )
		{	
			ExpectedToken( PS_T("]") );
			return node;
		}
	}
	node->SetToken(&t);

	node->AddChildLast(ParseParameterList());

	return node;
}

psCScriptNode* psCParser::ParseOperatorOverload()
{
	psCScriptNode* node = ParseOpOverloadDeclaration();
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseStatementBlock());
	return node;
}

psCScriptNode* psCParser::ParseScript()
{
	psCScriptNode* node = new psCScriptNode(snScript);

	// 根据第一个token决定子结点的类型
	psSToken t1;

	for (;;)
	{
		while (!m_bIsSyntaxError)
		{
			GetToken(&t1);
			RewindTo(&t1);

			if (t1.type == ttEnd)
			{
				// 到文件尾
				return node;
			}
			if (t1.type == ttClass)
			{
				node->AddChildLast(ParseClassDecl());
			}else if ( t1.type == ttEnum )
			{
				node->AddChildLast(ParseEnum());
			}
			else if (t1.type == ttConst || IsDataType(t1.type))
			{
				if (IsGlobalVar())
					node->AddChildLast(ParseDeclaration(true));
				else
				{
					if (IsOperatorOverload())
						node->AddChildLast(ParseOperatorOverload());
					else
						node->AddChildLast(ParseFunction());
				}
			}else if ( t1.type == ttEndStatement )
			{
				// do nothing
				GetToken(&t1);
			}else
			{
				// 不能识别的token
				psCString str;
				const psCHAR* t = psGetTokenDefinition(t1.type);
				if (t == 0) t = PS_T("<unknown token>");

				str.Format(TXT_UNEXPECTED_TOKEN_s, t);
				Error(str.AddressOf(), &t1);	
			}
		}

		if (m_bIsSyntaxError)
		{
			// 跳过错误的语句, 以便继续分析

			// 查找 (';' or '{' or end) 
			GetToken(&t1);
			while (t1.type != ttEndStatement && 
				t1.type != ttEnd && 
				t1.type != ttStartStatementBlock)
				GetToken(&t1);

			if (t1.type == ttStartStatementBlock)
			{
				// Find the end of block and skip nested blocks
				int level = 1;
				while (level > 0)
				{
					GetToken(&t1);
					if (t1.type == ttStartStatementBlock) ++level;
					else if (t1.type == ttEndStatementBlock) --level;
					else if(t1.type == ttEnd) break;
				}
			}

			// 继续分析
			m_bIsSyntaxError = false;
		}
	}
}


bool psCParser::IsGlobalVar()
{
	// 保存最开始的token位置, 以便最后恢复
	psSToken t1;
	GetToken(&t1);
	RewindTo(&t1);

	psSToken t2;
	GetToken(&t2);
	// A global variable can start with a const
	if (t2.type == ttConst)
		GetToken(&t2);

	// datatype
	if (!IsDataType(t2.type))
	{
		RewindTo(&t1);
		return false;
	}

	// amp (may be)
	GetToken(&t2);
	if ( IsTypeMod( t2.type) )
		GetToken(&t2);
	
	// identifier
	if ( t2.type != ttIdentifier)
	{
		RewindTo(&t1);
		return false;
	}
	
	GetToken(&t2);
	if ( t2.type == ttEndStatement || 
		t2.type == ttAssignment || 
		t2.type == ttListSeparator ||
		t2.type == ttOpenBracket)
	{
		RewindTo(&t1);
		return true;
	}

	if (t2.type == ttOpenParenthesis)
	{
		// 如果标识符后接的圆括后结束后，
		// 是一个语句块或文件尾，则视为函数
		while (t2.type != ttCloseParenthesis && t2.type != ttEnd)
			GetToken(&t2);

		if (t2.type == ttEnd)
			return false;
		else
		{
			GetToken(&t2);
			if (t2.type == ttStartStatementBlock || t2.type == ttEnd)
			{
				RewindTo(&t1);
				return false;
			}
		}
		RewindTo(&t1);
		return true;
	}

	RewindTo(&t1);
	return false;
}

psCScriptNode* psCParser::ParseFunction()
{
	psCScriptNode* node = new psCScriptNode(snFunction);

	psSToken t;
	GetToken(&t);
	RewindTo(&t);
	t.type = ( m_IncludeLevel > 0 ) ? ttNumberInclude : ttUnrecognizedToken;
	node->SetToken( &t );
	node->UpdateSourcePos(t.pos, t.length);

	// FUNCTION = TYPE TYPEMODE IDENTIFIER ARGLIST BLOCK
	node->AddChildLast(ParseType());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseTypeMod());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseIdentifier());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseParameterList());
	if (m_bIsSyntaxError) return node;

	node->AddChildLast(ParseStatementBlock());
	if (m_bIsSyntaxError) return node;

	return node;
}

psCScriptNode* psCParser::ParseGlobaVar()
{
	// GLOBVAR = TYPE IDENTIFIER ('=' ASSIGNMENT)? (',' IDENTIFIER ('=' ASSIGNMENT)?)* ';'
	psCScriptNode* node = new psCScriptNode(snGlobalVar);

	node->AddChildLast(ParseType());
	if (m_bIsSyntaxError) return node;

	psSToken t;
	for (;;)
	{
		// Parse identifier
		node->AddChildLast(ParseIdentifier());
		if (m_bIsSyntaxError) return node;

		GetToken(&t);
		if (t.type == ttAssignment)
		{
			node->AddChildLast(ParseAssignment());
			if (m_bIsSyntaxError) return node;
		}else if (t.type == ttOpenParenthesis)
		{
			// 初始化形参列表(a, b, ...)
			RewindTo(&t);
			node->AddChildLast(ParseArgList());
			if (m_bIsSyntaxError) return node;
		}else
			RewindTo(&t);

		GetToken(&t);
		if (t.type == ttEndStatement)
		{
			// 声明结束
			node->UpdateSourcePos(t.pos, t.length);
			return node;
		}else if (t.type == ttListSeparator)
		{
			// if it is list sperator continue
			continue;
		}else
		{
			// Error
			Error(ExpectedTokens(PS_T(","), PS_T(";")).AddressOf(), &t);
			return node;
		}
	}

	// never could be here
	return node;
}

psCScriptNode* psCParser::ParseTypeMod()
{
	psCScriptNode* node = new psCScriptNode(snDataType);
	
	psSToken t;
	// Parse possibly byref token
	GetToken(&t);
	RewindTo(&t);
	if ( IsTypeMod( t.type ) )
	{
		node->AddChildLast( ParseToken(t.type) );
	}
	return node;
}

psCScriptNode* psCParser::ParseType(bool allowConst)
{
	psCScriptNode* node = new psCScriptNode(snDataType);
	psSToken t;
	if (allowConst)
	{
		GetToken(&t);
		RewindTo(&t);
		if (t.type == ttConst)
		{
			node->AddChildLast(ParseToken(ttConst));
			if (m_bIsSyntaxError) return node;
		}
	}
	
	node->AddChildLast(ParseDataType());

	return node;
}


psCScriptNode* psCParser::ParseToken(int token)
{
	psCScriptNode* node = new psCScriptNode(snUndefined);
	
	psSToken t1;
	GetToken(&t1);
	if (t1.type != token)
	{
		Error(ExpectedToken(psGetTokenDefinition(token)).AddressOf(), &t1);
		return node;
	}
	node->SetToken(&t1);
	node->UpdateSourcePos(t1.pos, t1.length);
	return node;
}

psCScriptNode* psCParser::ParseOneOf(int* tokens, int count)
{
	psCScriptNode* node = new psCScriptNode(snUndefined);

	psSToken t1;
	GetToken(&t1);
	for (int i = 0; i < count; ++i)
		if (t1.type == tokens[i])
		{
			node->SetToken(&t1);
			node->UpdateSourcePos(t1.pos, t1.length);
			return node;
		}

	Error(ExpectedOneOf(tokens, count).AddressOf(), &t1);
	return node;
}

psCScriptNode* psCParser::ParseDataType()
{
	psCScriptNode* node = new psCScriptNode(snDataType);
	psSToken t;
	GetToken(&t);
	if (!IsDataType(t.type))
	{
		Error(TXT_EXPECTED_DATA_TYPE, &t);
		return node;
	}
	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}

psCScriptNode* psCParser::ParseRealType()
{
	psCScriptNode* node = new psCScriptNode(snDataType);

	psSToken t;
	GetToken(&t);
	if (!IsRealType(t.type))
	{
		Error(TXT_EXPECTED_DATA_TYPE, &t);
		return node;
	}
	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}

psCScriptNode* psCParser::ParseIdentifier()
{
	psCScriptNode* node = new psCScriptNode(snIdentifier);

	psSToken t;
	GetToken(&t);

	if (t.type != ttIdentifier)
	{
		Error(TXT_EXPECTED_IDENTIFIER, &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}

psCScriptNode* psCParser::ParseIdentifierValue()
{
	psCScriptNode* node = new psCScriptNode( snIdentifierValue );

	psSToken t;
	GetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	if ( t.type != ttField )
	{
		RewindTo(&t);

		node->AddChildLast(ParseIdentifier());	
		if (m_bIsSyntaxError) return node;

		GetToken(&t);
		if ( t.type == ttField )
		{
			node->AddChildLast(ParseIdentifier());	
			if (m_bIsSyntaxError) return node;
		}else
		{	
			RewindTo(&t);
		}
	}else
	{
		node->SetToken( &t );
		node->AddChildLast( ParseIdentifier() );

		if (m_bIsSyntaxError) return node;
	}
	return node;
}

psCScriptNode* psCParser::ParseParameterList()
{
	psCScriptNode* node = new psCScriptNode(snParameterList);
	
	psSToken t;
	GetToken(&t);	
	if ( t.type != ttOpenParenthesis )
	{
		Error(ExpectedToken(PS_T("(")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	psSToken t1;
	GetToken(&t1);
	if ( t1.type == ttCloseParenthesis)
	{
		node->UpdateSourcePos(t1.pos, t1.length);
		// Statement block is finished
		return node;
	}else
	{
		RewindTo(&t1);

		for (;;)
		{

			GetToken(&t1);
			if (t1.type == ttEllipsis)
			{
				// 如果是变参函数
				node->SetToken(&t1);
				
				GetToken(&t1);
				if (t1.type != ttCloseParenthesis)
				{
					Error(ExpectedToken(PS_T(")")).AddressOf(), &t1);
				}	
				return node;
			}else
			{
				RewindTo(&t1);
			}

			// Parse data type
			psCScriptNode* typeNode = ParseType();
			node->AddChildLast(typeNode);
			if ( m_bIsSyntaxError ) return node;
			
			node->AddChildLast( ParseTypeMod() );
			if ( m_bIsSyntaxError ) return node;

			// Parse identifier 
			// 参数可能没有名称
			GetToken(&t1);
			if (t1.type == ttIdentifier)
			{
				RewindTo(&t1);

				node->AddChildLast(ParseIdentifier());
				if ( m_bIsSyntaxError ) return node;
			}else
			{
				RewindTo(&t1);
			}

			node->AddChildLast( ParseArrayDim() );
			if ( m_bIsSyntaxError ) return node;

			GetToken(&t1);
			if (t1.type == ttCloseParenthesis)
			{
				node->UpdateSourcePos(t1.pos, t1.length);
				return node;
			}else if (t1.type == ttListSeparator)
			{
				continue;
			}else
			{
				Error(ExpectedTokens(PS_T(")"), PS_T(",")).AddressOf(), &t1);
				return node;
			}
		}
	}
}


psCScriptNode* psCParser::ParseArrayDim()
{
	psCScriptNode* node = new psCScriptNode(snArrayDim);
	
	psSToken t;
	GetToken(&t);
	node->UpdateSourcePos( t.pos, t.length );

	if ( t.type != ttOpenBracket )
	{	
		RewindTo(&t);
		return node;
	}

	do 
	{
		psCScriptNode* sizeNode = ParseConstant();
		if ( m_bIsSyntaxError ) return node;

		if ( sizeNode->GetTokenType() != ttIntConstant )
		{
			Error( TXT_ARRAY_SIZE_MUST_BE_INT_CONSTANT, &t );
			return node;
		}
		node->AddChildLast( sizeNode );

		GetToken(&t);
		if ( t.type != ttCloseBracket )
		{
			Error( ExpectedToken(PS_T(")")).AddressOf(), &t );
			return node;
		}

		GetToken(&t);
	}while ( t.type == ttOpenBracket );

	RewindTo(&t);
	return node;
}


psCScriptNode* psCParser::ParseExprValue()
{
	psCScriptNode* node = new psCScriptNode(snExprValue);

	psSToken t;
	GetToken(&t);
	RewindTo(&t);

	if (IsConstant(t.type))
	{
		node->AddChildLast(ParseConstant());
	}else if (t.type == ttIdentifier || t.type == ttField || IsRealType(t.type) )
	{
		if ( IsFunctionCall() )
			node->AddChildLast(ParseFunctionCall());
		else
			node->AddChildLast(ParseIdentifierValue());
	}
	else if (t.type == ttOpenParenthesis)
	{
		GetToken(&t);
		node->UpdateSourcePos(t.pos, t.length);

		node->AddChildLast(ParseAssignment());
		if (m_bIsSyntaxError ) return node;

		GetToken(&t);
		if( t.type != ttCloseParenthesis )
		{
			Error(ExpectedToken(PS_T(")")).AddressOf(), &t);
			return node;
		}
		node->UpdateSourcePos(t.pos, t.length);
	}else if ( t.type == ttSizeof )
	{
		node->AddChildLast( ParseSizeof() );
	}else if ( t.type == ttLessThan )
	{
		node->AddChildLast( ParseTypeCast() );
	}else
	{
		Error(TXT_EXPECTED_EXPRESSION_VALUE, &t);
	}
	return node;
}

psCScriptNode* psCParser::ParseSizeof()
{
	psCScriptNode* node = new psCScriptNode(snSizeof);

	psSToken t;
	GetToken(&t);

	node->UpdateSourcePos(t.pos, t.length);

	if ( t.type != ttSizeof )
	{
		Error( ExpectedToken(PS_T("sizeof" )).AddressOf(), &t );
		return node;
	}

	GetToken(&t);
	if ( t.type != ttOpenParenthesis )
	{
		Error( ExpectedToken(PS_T("(")).AddressOf(), &t );
		return node;
	}

	psSToken t1;
	GetToken(&t);
	GetToken(&t1);
	RewindTo(&t);

	if ( IsRealType( t.type ) )
	{
		node->AddChildLast( ParseType() );

		GetToken(&t);
		RewindTo(&t);
		if ( IsTypeMod( t.type ) )
		{
			node->AddChildLast( ParseTypeMod() );
		}
	}else
	{
		node->AddChildLast( ParseAssignment() );
	}
	if ( m_bIsSyntaxError ) return node;

	GetToken(&t);
	if ( t.type != ttCloseParenthesis )
	{
		Error( ExpectedToken( PS_T(")") ).AddressOf(), &t );
		return node;
	}

	return node;
}

psCScriptNode* psCParser::ParseTypeCast()
{
	psCScriptNode* node = new psCScriptNode(snTypeCast);

	psSToken t;
	GetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	if ( t.type != ttLessThan )
	{
		Error( ExpectedToken( PS_T("<" )).AddressOf(), &t );
		return node;
	}
	node->AddChildLast( ParseType() );
	if ( m_bIsSyntaxError ) return node;

	node->AddChildLast( ParseTypeMod() );
	if ( m_bIsSyntaxError ) return node;

	GetToken(&t);
	if ( t.type != ttGreaterThan )
	{
		Error( ExpectedToken( PS_T(">" )).AddressOf(), &t );
		return node;
	}

	GetToken(&t);
	if ( t.type != ttOpenParenthesis )
	{
		Error( ExpectedToken( PS_T("(") ).AddressOf(), &t );
		return node;
	}

	node->AddChildLast( ParseAssignment() );
	if ( m_bIsSyntaxError ) return node;

	GetToken(&t);
	if ( t.type != ttCloseParenthesis )
	{
		Error( ExpectedToken( PS_T(")") ).AddressOf(), &t );
		return node;
	}
	return node;
}

psCScriptNode* psCParser::ParseConstant()
{
	psCScriptNode* node = new psCScriptNode(snConstant);
	
	psSToken t;
	GetToken(&t);
	if (!IsConstant(t.type))
	{
		Error(TXT_EXPECTED_CONSTANT, &t);
		return node;
	}
	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	// We want to gather a list of string constants to concatenate as children
	// 处理多行字符串常量如
	// "abc"
	// "bcd"
	//  ...

	psETokenType tokenType = t.type;
	if ( tokenType == ttStringConstant ||
		 tokenType == ttWStringConstant )
	{
		RewindTo(&t);
	}
		
	while (t.type == ttStringConstant ||
		   t.type == ttWStringConstant )
	{
		node->AddChildLast(ParseStringConstant());
		if ( m_bIsSyntaxError ) return node;
		
		if ( t.type != tokenType )
		{
			Error( TXT_CONCATE_STRING_TYPE_NOT_MATCH, &t );
			return node;
		}
		GetToken(&t);
		RewindTo(&t);
	}
	return node;
}

psCScriptNode* psCParser::ParseStringConstant()
{
	psCScriptNode* node = new psCScriptNode(snConstant);

	psSToken t;
	GetToken(&t);
	if (t.type != ttStringConstant &&
		t.type != ttWStringConstant )
	{
		Error(TXT_EXPECTED_STRING, &t);
		return node;
	}
	
	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}

psCScriptNode* psCParser::ParseFunctionCall()
{
	psCScriptNode* node = new psCScriptNode(snFunctionCall);

	psSToken t;
	GetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);

	if ( t.type != ttField )
	{
		RewindTo(&t);

		node->AddChildLast(ParseIdentifier());	
		if (m_bIsSyntaxError) return node;

		GetToken(&t);
		if ( t.type == ttField )
		{
			node->AddChildLast(ParseIdentifier());	
			if (m_bIsSyntaxError) return node;
		}else
		{	
			RewindTo(&t);
		}
	}else
	{
		node->SetToken( &t );
		node->AddChildLast(ParseIdentifier());

		if (m_bIsSyntaxError) return node;
	}

	node->AddChildLast(ParseArgList());
	return node;
}

psCScriptNode* psCParser::ParseArgList()
{
	psCScriptNode* node = new psCScriptNode(snArgList);
	
	psSToken t;
	GetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	
	if (t.type != ttOpenParenthesis)
	{
		Error(ExpectedToken(PS_T("(")).AddressOf(), &t);
		return node;
	}else 
	{
		GetToken(&t);
		if (t.type == ttCloseParenthesis)
		{
			node->UpdateSourcePos(t.pos, t.length);
			return node;
		}else
		{
			RewindTo(&t);
			for (;;)
			{
				node->AddChildLast(ParseAssignment());
				if (m_bIsSyntaxError) return node;

				GetToken(&t);
				if( t.type == ttCloseParenthesis)
				{
					node->UpdateSourcePos(t.pos, t.length);
					return node;
				}else if (t.type == ttListSeparator)
					continue;
				else
				{
					Error(ExpectedTokens(PS_T(","), PS_T(")")).AddressOf(), &t);
					return node;
				}
			}
		}
	
	}
	// Never could be here
	return node;
}

psCScriptNode* psCParser::ParseStatementBlock()
{
	psCScriptNode* node = new psCScriptNode(snStatementBlock);

	psSToken t1;
	GetToken(&t1);
	if (t1.type != ttStartStatementBlock)
	{
		Error(ExpectedToken(PS_T("{")).AddressOf(), &t1);
		return node;
	}
	node->UpdateSourcePos(t1.pos, t1.length);

	for (;;)
	{
		while (!m_bIsSyntaxError)
		{
			GetToken(&t1);
			if (t1.type == ttEndStatementBlock)
			{
				node->UpdateSourcePos(t1.pos, t1.length);
				return node;			
			}else
			{
				RewindTo(&t1);
				if (IsDeclaration())
					node->AddChildLast(ParseDeclaration());
				else 
					node->AddChildLast(ParseStatement());
			}
		}

		if (m_bIsSyntaxError)
		{
			// 跳过错误的语句, 以便继续分析

			// 查找 (';' or '{' or end) 
			GetToken(&t1);
			while (t1.type != ttEndStatement && 
				t1.type != ttEnd && 
				t1.type != ttStartStatementBlock)
				GetToken(&t1);

			if (t1.type == ttStartStatementBlock)
			{
				// Find the end of block and skip nested blocks
				int level = 1;
				while (level > 0)
				{
					GetToken(&t1);
					if (t1.type == ttStartStatementBlock) ++level;
					else if (t1.type == ttEndStatementBlock) --level;
					else if(t1.type == ttEnd) break;
				}
			}else if (t1.type == ttEndStatementBlock)
			{
				RewindTo(&t1);
			}else if (t1.type == ttEnd)
			{
				Error(TXT_UNEXPECTED_END_OF_FILE, &t1);
				return node;
			}

			// 继续分析
			m_bIsSyntaxError = false;
		}
	}

	// Never could be here
	return node;
}

bool psCParser::IsDeclaration()
{
	psSToken t1, t2;
	GetToken(&t1);

	if (t1.type == ttConst)
	{
		GetToken(&t2);
	}else
		t2 = t1;

	if (!IsDataType(t2.type))
	{
		RewindTo(&t1);
		return false;
	}

	GetToken(&t2);
	if ( IsTypeMod(t2.type) || t2.type == ttIdentifier)
	{
		// 引用变量
		RewindTo(&t1);
		return true;
	}

	RewindTo(&t1);
	return false;
}

bool psCParser::IsFunctionCall()
{
	psSToken t1, t2;
	GetToken(&t1);
	
	if ( t1.type == ttField )
	{
		GetToken(&t2);
	}else
	{
		GetToken(&t2);
		if ( t2.type == ttField )
		{
			if ( t1.type != ttIdentifier )
				return false;
			GetToken(&t2);
		}else
		{
			RewindTo(&t2);
			t2 = t1;
		}
	}

	if (t2.type != ttIdentifier && !IsRealType(t2.type))
	{
		RewindTo(&t1);
		return false;
	}

	GetToken(&t2);
	if (t2.type == ttOpenParenthesis)
	{
		RewindTo(&t1);
		return true;
	}

	RewindTo(&t1);
	return false;
}

bool psCParser::IsClassProperty()
{
	psSToken t1, t2;
	GetToken(&t1);

	if (t1.type == ttConst)
		GetToken(&t2);
	else
		t2 = t1;

	if (!IsDataType(t2.type))
	{
		RewindTo(&t1);
		return false;
	}
	
	GetToken(&t2);
	if ( IsTypeMod( t2.type ) )
	{
		GetToken(&t2);
	}
	if (t2.type != ttIdentifier)
	{
		RewindTo(&t1);
		return false;
	}

	GetToken(&t2);
	if (t2.type == ttEndStatement ||
		t2.type == ttListSeparator ||
		t2.type == ttOpenBracket)
	{
		RewindTo(&t1);
		return true;
	}

	RewindTo(&t1);
	return false;
}

bool psCParser::IsOperatorOverload()
{
	psSToken t1, t2;
	GetToken(&t1);

	if (t1.type == ttConst)
		GetToken(&t2);
	else
		t2 = t1;

	if (!IsDataType(t2.type))
	{
		RewindTo(&t1);
		return false;
	}

	GetToken(&t2);
	if ( IsTypeMod( t2.type ) )
		GetToken(&t2);

	if (t2.type == ttOperator)
	{
		RewindTo(&t1);
		return true;
	}	

	RewindTo(&t1);
	return false;
}

psCScriptNode* psCParser::ParseDeclaration(bool bGlobal)
{
	psCScriptNode* node = new psCScriptNode(bGlobal?snGlobalVar:snDeclaration);

	psSToken t;
	GetToken(&t);
	RewindTo(&t);
	node->UpdateSourcePos(t.pos, t.length);

	// Parse type
	node->AddChildLast(ParseType());
	if (m_bIsSyntaxError) return node;
	
	for (;;)
	{
		node->AddChildLast(ParseTypeMod());
		if (m_bIsSyntaxError) return node;

		node->AddChildLast(ParseVariableDim());
		if (m_bIsSyntaxError) return node;

		GetToken(&t);
		if (t.type == ttAssignment)
		{		
			// If next token is assignment, parse expression
			// 变量初始化要计算赋值表达式
			node->AddChildLast(ParseInitExpression());
			if (m_bIsSyntaxError) return node;

		}else if (t.type == ttOpenParenthesis)
		{
			RewindTo(&t);
			// 初始化形参
			node->AddChildLast(ParseArgList());
			if (m_bIsSyntaxError) return node;
		}else 
			RewindTo(&t);

		GetToken(&t);
		if (t.type == ttListSeparator)
		{
			continue;
		}else if (t.type == ttEndStatement)
		{
			node->UpdateSourcePos(t.pos, t.length);
			return node;
		}else
		{
			Error(ExpectedTokens(PS_T(","), PS_T(";")).AddressOf(), &t);
			return node;
		}
	}
	// Never could be here
	return node;
}

psCScriptNode* psCParser::ParseVariableDim()
{
	psCScriptNode* node = new psCScriptNode(snVariableDim);

	// Parse identifer
	node->AddChildLast(ParseIdentifier());
	if (m_bIsSyntaxError) return node;	

	psSToken t;	
	for (;;)
	{	
		GetToken(&t);
		if (t.type == ttOpenBracket)
		{
			// Parse constant expression
			node->AddChildLast(ParseExpression());
			if (m_bIsSyntaxError) return node;

			GetToken(&t);
			if (t.type == ttCloseBracket)
			{
				continue;
			}else
			{
				Error(ExpectedToken(PS_T("]")).AddressOf(), &t);
				return node;
			}
		}else 
		{
			RewindTo(&t);
			return node;
		}
	}
}

psCScriptNode* psCParser::ParseInitExpression()
{
	psCScriptNode* node = new psCScriptNode(snInitExpr);

	psSToken t;
	GetToken(&t);	
	node->UpdateSourcePos(t.pos, t.length);

	if (t.type == ttStartStatementBlock)
	{
		for (;;)
		{
			node->AddChildLast(ParseInitExpression());
			if (m_bIsSyntaxError) return node;
			
			GetToken(&t);
			if (t.type == ttListSeparator)
			{
				continue;
			}else if (t.type == ttEndStatementBlock)
			{
				node->UpdateSourcePos(t.pos, t.length);
				return node;
			}else
			{
				Error(ExpectedTokens(PS_T(","), PS_T("}")).AddressOf(), &t);
				return node;
			}
		}
	}else 
	{	
		RewindTo(&t);
		node->AddChildLast(ParseAssignment());
		return node;
	}
}

psCScriptNode* psCParser::ParseStatement()
{
	psSToken t1;
	GetToken(&t1);
	RewindTo(&t1);

	if (t1.type == ttIf)
		return ParseIf();
	else if (t1.type == ttSwitch)
		return ParseSwitch();
	else if (t1.type == ttFor)
		return ParseFor();
	else if (t1.type == ttWhile)
		return ParseWhile();
	else if (t1.type == ttDo)
		return ParseDoWhile();
	else if (t1.type == ttWhile)
		return ParseWhile();
	else if (t1.type == ttStartStatementBlock)
		return ParseStatementBlock();
	else if (t1.type == ttReturn)
		return ParseReturn();
	else if (t1.type == ttBreak)
		return ParseBreak();
	else if (t1.type == ttContinue)
		return ParseContinue();
	else if (t1.type == ttEnum )
		return ParseEnum();
	else
		return ParseExpressionStatement();
}

psCScriptNode* psCParser::ParseEnum()
{
	psCScriptNode* node = new psCScriptNode( snEnum );

	psSToken t;
	GetToken(&t);
	if ( t.type != ttEnum )
	{
		Error( ExpectedToken(PS_T("enum")).AddressOf(), &t );
		return node;
	}
	node->UpdateSourcePos( t.pos, t.length );

	GetToken(&t);
	if ( t.type == ttIdentifier )
	{
		RewindTo(&t);
		node->AddChildLast( ParseIdentifier() );
		if ( m_bIsSyntaxError ) return node;

		GetToken(&t);
	}

	if ( t.type != ttStartStatementBlock )
	{
		Error( ExpectedToken(PS_T("{")).AddressOf(), &t );
		return node;
	}

	for ( ;; )
	{
		node->AddChildLast( ParesEnumConstant() );
		if ( m_bIsSyntaxError ) return node;

		GetToken(&t);
		if ( t.type == ttEndStatementBlock )
		{
			break;
		}else if ( t.type != ttListSeparator )
		{
			Error( ExpectedTokens(PS_T(","), PS_T("}")).AddressOf(), &t );
			return node;
		}
	}

	GetToken(&t);
	if ( t.type != ttEndStatement )
	{
		Error( ExpectedToken(PS_T(";")).AddressOf(), &t);
	}
	return node;
}

psCScriptNode* psCParser::ParesEnumConstant()
{
	psCScriptNode* node = new psCScriptNode( snEnumConstant );

	node->AddChildLast( ParseIdentifier() );
	if ( m_bIsSyntaxError ) return node;

	psSToken t;
	GetToken(&t);
	if ( t.type == ttAssignment )
	{
		GetToken(&t);
		if ( t.type == ttMinus || t.type == ttPlus )
		{
			node->SetToken( &t );
		}else
		{
			RewindTo(&t);
		}
		node->AddChildLast( ParseConstant() );
		if ( m_bIsSyntaxError ) return node;
	}else
	{
		RewindTo(&t);
	}
	return node;
}

psCScriptNode* psCParser::ParseExpressionStatement()
{
	psCScriptNode* node = new psCScriptNode(snExpressionStatement);
	
	psSToken t;
	GetToken(&t);
	if (t.type == ttEndStatement)
	{
		node->UpdateSourcePos(t.pos, t.length);
		return node;
	}else
	{
		RewindTo(&t);

		node->AddChildLast(ParseAssignment());
		if (m_bIsSyntaxError) return node;
		
		GetToken(&t);
		if (t.type != ttEndStatement)
		{
			Error(ExpectedToken(PS_T(";")).AddressOf(), &t);
			return node;
		}

		node->UpdateSourcePos(t.pos, t.length);
		return node;
	}
}	


psCScriptNode* psCParser::ParseSwitch()
{
	psCScriptNode* node = new psCScriptNode(snSwitch);

	psSToken t;
	GetToken(&t);
	if (t.type != ttSwitch)
	{
		Error(ExpectedToken(PS_T("switch")).AddressOf(), &t);
		return node;
	}else
	{
		node->UpdateSourcePos(t.pos, t.length);
		GetToken(&t);
		if (t.type != ttOpenParenthesis)
		{
			Error(ExpectedToken(PS_T("(")).AddressOf(), &t);
			return node;
		}
		node->AddChildLast(ParseAssignment());
		if (m_bIsSyntaxError) return node;
		
		GetToken(&t);
		if (t.type != ttCloseParenthesis)
		{
			Error(ExpectedToken(PS_T(")")).AddressOf(), &t);
			return node;
		}

		GetToken(&t);
		if (t.type != ttStartStatementBlock)
		{
			Error(ExpectedToken(PS_T("{")).AddressOf(), &t);
			return node;
		}
		
		while (!m_bIsSyntaxError)
		{
			GetToken(&t);
			if (t.type == ttEndStatementBlock || t.type == ttEnd)
				break;
			
			RewindTo(&t);
		
			node->AddChildLast(ParseCase());
			if (m_bIsSyntaxError) return node;
		}
		if (t.type != ttEndStatementBlock)
		{
			Error(ExpectedToken(PS_T("}")).AddressOf(), &t);
			return node;
		}
		return node;
	}
}

psCScriptNode* psCParser::ParseCase()
{
	psCScriptNode* node = new psCScriptNode(snCase);
	psSToken t;
	GetToken(&t);
	if (t.type != ttCase && t.type != ttDefault)
	{
		Error(ExpectedTokens(PS_T("case"), PS_T("default")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	if (t.type == ttCase)
	{
		node->AddChildLast(ParseExpression());
	}

	GetToken(&t);
	if (t.type != ttColon)
	{
		Error(ExpectedToken(PS_T(":")).AddressOf(), &t);
		return node;
	}

	// Parse statements until we find either of }, case, default, and break
	GetToken(&t);
	RewindTo(&t);
	while (t.type != ttCase && 
		t.type != ttDefault &&
		t.type != ttEndStatementBlock &&
		t.type != ttBreak)
	{
		node->AddChildLast(ParseStatement());
		if (m_bIsSyntaxError) return node;

		GetToken(&t);
		RewindTo(&t);
	}

	// If the case was ended with a break statement, add it to the node
	if( t.type == ttBreak )
		node->AddChildLast(ParseBreak());

	return node;
}

psCScriptNode* psCParser::ParseIf()
{
	psCScriptNode* node = new psCScriptNode(snIf);
	psSToken t;
	GetToken(&t);
	if (t.type != ttIf)
	{
		Error(ExpectedToken(PS_T("if")).AddressOf(), &t);
		return node;
	}
	
	node->UpdateSourcePos(t.pos, t.length);
	GetToken(&t);
	if (t.type != ttOpenParenthesis)
	{
		Error(ExpectedToken(PS_T("(")).AddressOf(), &t);
		return node;
	}

	node->AddChildLast(ParseAssignment());
	if (m_bIsSyntaxError) return node;

	GetToken(&t);
	if (t.type != ttCloseParenthesis)
	{
		Error(ExpectedToken(PS_T(")")).AddressOf(), &t);
		return node;
	}

	node->AddChildLast(ParseStatement());
	if (m_bIsSyntaxError) return node;

	GetToken(&t);
	if (t.type != ttElse)
	{
		// No else statement return already
		RewindTo(&t);
		return node;
	}

	node->AddChildLast(ParseStatement());
	return node;
}

psCScriptNode* psCParser::ParseFor()
{
	psCScriptNode* node = new psCScriptNode(snFor);

	psSToken t;
	GetToken(&t);
	if (t.type != ttFor)
	{
		Error(ExpectedToken(PS_T("For")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if (t.type != ttOpenParenthesis)
	{
		Error(ExpectedToken(PS_T("(")).AddressOf(), &t);
		return node;
	}

	if (IsDeclaration())
		node->AddChildLast(ParseDeclaration());
	else
		node->AddChildLast(ParseExpressionStatement());
	if (m_bIsSyntaxError) return node;
	
	node->AddChildLast(ParseExpressionStatement());
	if (m_bIsSyntaxError) return node;

	GetToken(&t);
	if (t.type != ttCloseParenthesis)
	{
		RewindTo(&t);

		node->AddChildLast(ParseAssignment());
		if (m_bIsSyntaxError) return node;

		GetToken(&t);
		if (t.type != ttCloseParenthesis)
		{
			Error(ExpectedToken(PS_T(")")).AddressOf(), &t);
			return node;
		}
	}

	node->AddChildLast(ParseStatement());
	return node;
}

psCScriptNode* psCParser::ParseWhile()
{
	psCScriptNode* node = new psCScriptNode(snWhile);
	psSToken t;
	GetToken(&t);
	if (t.type != ttWhile)
	{
		Error(ExpectedToken(PS_T("while")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if (t.type != ttOpenParenthesis)
	{
		Error(ExpectedToken(PS_T("(")).AddressOf(), &t);
		return node;
	}
	node->AddChildLast(ParseAssignment());
	if (m_bIsSyntaxError) return node;

	GetToken(&t);
	if (t.type != ttCloseParenthesis)
	{
		Error(ExpectedToken(PS_T(")")).AddressOf(), &t);
		return node;
	}

	node->AddChildLast(ParseStatement());
	return node;
}

psCScriptNode* psCParser::ParseDoWhile()
{
	psCScriptNode* node = new psCScriptNode(snDoWhile);
	psSToken t;
	GetToken(&t);
	if (t.type != ttDo)
	{
		Error(ExpectedToken(PS_T("do")).AddressOf(), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);
	
	node->AddChildLast(ParseStatement());
	if (m_bIsSyntaxError) return node;
	
	GetToken(&t);
	if (t.type != ttWhile)
	{
		Error(ExpectedToken(PS_T("while")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);
	
	GetToken(&t);
	if (t.type != ttOpenParenthesis)
	{
		Error(ExpectedToken(PS_T("(")).AddressOf(), &t);
		return node;
	}

	node->AddChildLast(ParseAssignment());
	if (m_bIsSyntaxError) return node;

	GetToken(&t);
	if (t.type != ttCloseParenthesis)
	{
		Error(ExpectedToken(PS_T(")")).AddressOf(), &t);
		return node;
	}

	GetToken(&t);
	if (t.type != ttEndStatement)
	{
		Error(ExpectedToken(PS_T(";")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}	

psCScriptNode* psCParser::ParseReturn()
{
	psCScriptNode* node = new psCScriptNode(snReturn);
	psSToken t;
	GetToken(&t);
	if (t.type != ttReturn)
	{
		Error(ExpectedToken(PS_T("return")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if( t.type == ttEndStatement )
	{
		node->UpdateSourcePos(t.pos, t.length);
		return node;
	}

	RewindTo(&t);

	node->AddChildLast(ParseAssignment());
	if( m_bIsSyntaxError ) return node;

	GetToken(&t);
	if( t.type != ttEndStatement )
	{
		Error(ExpectedToken(PS_T(";")).AddressOf(), &t);
		return node;
	}

	node->UpdateSourcePos(t.pos, t.length);
	
	return node;
}

psCScriptNode* psCParser::ParseBreak()
{
	psCScriptNode* node = new psCScriptNode(snBreak);

	psSToken t;
	GetToken(&t);
	if (t.type != ttBreak)
	{
		Error(ExpectedToken(PS_T("break")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if(t.type != ttEndStatement)
	{
		Error(ExpectedToken(PS_T(";")).AddressOf(), &t);
		return node;
	}	
	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

psCScriptNode* psCParser::ParseContinue()
{
	psCScriptNode* node = new psCScriptNode(snContinue);

	psSToken t;
	GetToken(&t);
	if (t.type != ttContinue)
	{
		Error(ExpectedToken(PS_T("continue")).AddressOf(), &t);
		return node;
	}
	node->UpdateSourcePos(t.pos, t.length);

	GetToken(&t);
	if(t.type != ttEndStatement)
	{
		Error(ExpectedToken(PS_T(";")).AddressOf(), &t);
		return node;
	}	
	node->UpdateSourcePos(t.pos, t.length);

	return node;
}

psCScriptNode* psCParser::ParseAssignment()
{
	psCScriptNode* node = new psCScriptNode(snAssignment);
	
	psSToken t;
	GetToken(&t);
	RewindTo(&t);
	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseCondition());
	if (m_bIsSyntaxError) return node;

	GetToken(&t);
	RewindTo(&t);
	if (IsAssignOperator(t.type))
	{
		node->AddChildLast(ParseAssignOperator());
		if (m_bIsSyntaxError) return node;

		// 处理列如(a = (b = c = ....))的情况
		node->AddChildLast(ParseAssignment());
		if (m_bIsSyntaxError) return node;
	}
	
	return node;
}

psCScriptNode* psCParser::ParseCondition()
{	
	psSToken t;
	GetToken(&t);
	RewindTo(&t);

	psCScriptNode* node = new psCScriptNode(snCondition);
	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseExpression());
	if (m_bIsSyntaxError) return node;

	// 处理问号表达式 a ? b : c
	GetToken(&t);
	if (t.type == ttQuestion)
	{
		node->AddChildLast(ParseAssignment());
		if (m_bIsSyntaxError) return node;

		GetToken(&t);
		if (t.type != ttColon)
		{
			Error(ExpectedToken(PS_T(":")).AddressOf(), &t);
			return node;
		}

		node->AddChildLast(ParseAssignment());
		if (m_bIsSyntaxError) return node;
	}else
	{
		RewindTo(&t);
	}
	return node;
}


psCScriptNode* psCParser::ParseExpression()
{
	psSToken t;
	GetToken(&t);
	RewindTo(&t);

	psCScriptNode* node = new psCScriptNode(snExpression);
	node->UpdateSourcePos(t.pos, t.length);

	node->AddChildLast(ParseExprTerm());
	if (m_bIsSyntaxError) return node;

	for (;;)
	{
		GetToken(&t);
		RewindTo(&t);

		if (!IsOperator(t.type))
		{
			return node;
		}

		node->AddChildLast(ParseExprOperator());
		if (m_bIsSyntaxError) return node;

		node->AddChildLast(ParseExprTerm());

	}
}

psCScriptNode* psCParser::ParseExprTerm()
{
	psCScriptNode* node = new psCScriptNode(snExprTerm);

	for (;;)
	{
		psSToken t;
		GetToken(&t);
		RewindTo(&t);
		if (!IsPreOperator(t.type))
			break;

		node->AddChildLast(ParseExprPreOp());
		if (m_bIsSyntaxError) return node;
	}

	node->AddChildLast(ParseExprValue());
	if (m_bIsSyntaxError) return node;

	for (;;)
	{
		psSToken t;
		GetToken(&t);
		RewindTo(&t);
		if (!IsPostOperator(t.type))
			return node;

		node->AddChildLast(ParseExprPostOp());
		if (m_bIsSyntaxError) return node;
	}
	// never could be here
	return node;
}

psCScriptNode* psCParser::ParseExprPreOp()
{
	psCScriptNode* node = new psCScriptNode(snExprPreOp);

	psSToken t;
	GetToken(&t);
	if (!IsPreOperator(t.type))
	{
		Error(TXT_EXPECTED_PRE_OPERATOR, &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}

psCScriptNode* psCParser::ParseExprPostOp()
{
	psCScriptNode* node = new psCScriptNode(snExprPostOp);

	psSToken t;
	GetToken(&t);
	if (!IsPostOperator(t.type))
	{
		Error(TXT_EXPECTED_POST_OPERATOR, &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	
	if ( t.type == ttDot || t.type == ttArrow )
	{
		psSToken t1, t2;
		GetToken(&t1);
		GetToken(&t2);
		RewindTo(&t1);
		if (t2.type == ttOpenParenthesis)
			node->AddChildLast(ParseFunctionCall());
		else
			node->AddChildLast(ParseIdentifier());
	}else if (t.type == ttOpenBracket)
	{
		node->AddChildLast(ParseAssignment());

		GetToken(&t);
		if( t.type != ttCloseBracket )
		{
			Error(ExpectedToken(PS_T("]")).AddressOf(), &t);
			return node;
		}
		node->UpdateSourcePos(t.pos, t.length);
	}

	return node;
}

psCScriptNode* psCParser::ParseExprOperator()
{
	psCScriptNode* node = new psCScriptNode(snExprOperator);
	psSToken t;
	GetToken(&t);
	if (!IsOperator(t.type))
	{
		Error(TXT_EXPECTED_OPERATOR, &t);
		return node;
	}
	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}


psCScriptNode* psCParser::ParseAssignOperator()
{
	psCScriptNode* node = new psCScriptNode(snExprOperator);
	psSToken t;
	GetToken(&t);
	if (!IsAssignOperator(t.type))
	{
		Error(TXT_EXPECTED_OPERATOR, &t);
		return node;
	}

	node->SetToken(&t);
	node->UpdateSourcePos(t.pos, t.length);
	return node;
}

int psCParser::GetToken(psCString& text, int pos, psSToken* token, int& numLine)
{
	int sourceLength = (int)text.GetLength();
	numLine = 0;
	do 
	{
		if (pos >= sourceLength )	
		{
			token->type = ttEnd;
			token->length = 0;
		}else
		{
			int tokenLen  = 0;
			token->type	  = m_Tokenizer.GetToken(&text[pos], sourceLength - pos, &tokenLen );
			token->length = tokenLen;

			numLine += m_Tokenizer.GetNumLines();
		}
		token->pos   = pos;
		token->index = -1;
		
		// update source
		pos += token->length;	

	} while(token->type == ttWhiteSpace ||
			token->type == ttOnelineComment ||
			token->type == ttMultilineComment);
	return pos;
}

void psCParser::Error(const psCHAR* text, const psSToken* token)
{
	if ( token->index < 0 )
		RewindToSrc( token );
	else
		RewindTo(token);

	m_bIsSyntaxError = true;
	m_bErrorWhileParsing = true;

	int row, col;
	psCScriptCode* code = m_pScriptCode->ConvertPosToRowCol(token->pos, &row, &col);
	
	if (m_pBuilder)
		m_pBuilder->WriteError(code->m_Name.AddressOf(), code->m_FileName.AddressOf(), text, row, col);
}

bool psCParser::IsRealType(int tokenType)
{
	if( tokenType == ttVoid ||
		tokenType == ttInt ||
		tokenType == ttInt8 ||
		tokenType == ttInt16 ||
		tokenType == ttUInt8 ||
		tokenType == ttUInt16 ||
		tokenType == ttFloat ||
		tokenType == ttBool ||
		tokenType == ttDouble )
		return true;
	else
		return false;
}

bool psCParser::IsDataType(int tokenType)
{
	if( tokenType == ttIdentifier ||
		IsRealType(tokenType) )
		return true;

	return false;
}

bool psCParser::IsOperator(int tokenType)
{
	if( tokenType == ttPlus ||
		tokenType == ttMinus ||
		tokenType == ttStar ||
		tokenType == ttSlash ||
		tokenType == ttPercent ||
		tokenType == ttAnd ||
		tokenType == ttOr ||
		tokenType == ttXor ||
		tokenType == ttEqual ||
		tokenType == ttNotEqual ||
		tokenType == ttLessThan ||
		tokenType == ttLessThanOrEqual ||
		tokenType == ttGreaterThan ||
		tokenType == ttGreaterThanOrEqual ||
		tokenType == ttAmp ||
		tokenType == ttBitOr ||
		tokenType == ttBitXor ||
		tokenType == ttBitShiftLeft ||
		tokenType == ttBitShiftRight)
		return true;

	return false;
}

bool psCParser::IsAssignOperator(int tokenType)
{
	if( tokenType == ttAssignment ||
		tokenType == ttAddAssign ||
		tokenType == ttSubAssign ||
		tokenType == ttMulAssign ||
		tokenType == ttDivAssign ||
		tokenType == ttModAssign ||
		tokenType == ttAndAssign ||
		tokenType == ttOrAssign ||
		tokenType == ttXorAssign ||
		tokenType == ttShiftLeftAssign ||
		tokenType == ttShiftRightAssign )
		return true;

	return false;
}

bool psCParser::IsPreOperator(int tokenType)
{
	if( tokenType == ttMinus ||
		tokenType == ttPlus ||
		tokenType == ttNot ||
		tokenType == ttInc ||
		tokenType == ttDec ||
		tokenType == ttBitNot ||
		tokenType == ttAmp ||
		tokenType == ttStar )
		return true;
	return false;
}

bool psCParser::IsPostOperator(int tokenType)
{
	if( tokenType == ttInc   ||
		tokenType == ttDec   ||
		tokenType == ttDot   ||
		tokenType == ttArrow ||
		tokenType == ttOpenBracket)
		return true;
	return false;
}

bool psCParser::IsOpAllowOverload(int tokenType )
{
	if( tokenType == ttPlus ||
		tokenType == ttMinus ||
		tokenType == ttStar ||
		tokenType == ttSlash ||
		tokenType == ttPercent ||
		tokenType == ttAnd ||
		tokenType == ttOr ||
		tokenType == ttXor ||
		tokenType == ttEqual ||
		tokenType == ttNotEqual ||
		tokenType == ttLessThan ||
		tokenType == ttLessThanOrEqual ||
		tokenType == ttGreaterThan ||
		tokenType == ttGreaterThanOrEqual ||
		tokenType == ttAmp ||
		tokenType == ttBitOr ||
		tokenType == ttBitXor ||
		tokenType == ttBitShiftLeft ||
		tokenType == ttBitShiftRight ||

		tokenType == ttAssignment ||
		tokenType == ttAddAssign ||
		tokenType == ttSubAssign ||
		tokenType == ttMulAssign ||
		tokenType == ttDivAssign ||
		tokenType == ttModAssign ||
		tokenType == ttAndAssign ||
		tokenType == ttOrAssign ||
		tokenType == ttXorAssign ||
		tokenType == ttShiftLeftAssign ||
		tokenType == ttShiftRightAssign ||
		
		tokenType == ttMinus ||
		tokenType == ttPlus  ||
		tokenType == ttInc   ||
		tokenType == ttDec   ||
		tokenType == ttNot   ||
		tokenType == ttBitNot ||
		tokenType == ttOpenBracket )
		return true;
	return false;

}

bool psCParser::IsConstant(int tokenType)
{
	if( tokenType == ttIntConstant ||
		tokenType == ttCharConstant ||
		tokenType == ttWCharConstant ||
		tokenType == ttFloatConstant ||
		tokenType == ttDoubleConstant ||
		tokenType == ttStringConstant ||
		tokenType == ttWStringConstant ||
		tokenType == ttTrue ||
		tokenType == ttFalse ||
		tokenType == ttBitsConstant ||
		tokenType == ttNull ||
		tokenType == ttThis)
		return true;

	return false;
}

psCString psCParser::ExpectedToken(const psCHAR *token)
{
	psCString str;

	str.Format(TXT_EXPECTED_s, token);

	return str;
}


psCString psCParser::ExpectedTokens(const psCHAR *t1, const psCHAR *t2)
{
	psCString str;

	str.Format(TXT_EXPECTED_s_OR_s, t1, t2);

	return str;
}

psCString psCParser::ExpectedOneOf(int *tokens, int count)
{
	psCString str;

	str = TXT_EXPECTED_ONE_OF;
	for( int n = 0; n < count; n++ )
	{
		str += psGetTokenDefinition(tokens[n]);
		if( n < count-1 )
			str += PS_T(", ");
	}
	return str;
}


