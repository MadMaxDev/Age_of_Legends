#ifndef __PS_TOKENIZER_H__
#define __PS_TOKENIZER_H__

#include "tokendef.h"

class psCTokenizer
{
public:
	psCTokenizer(void);
	~psCTokenizer(void);

	psETokenType GetToken(const psCHAR* pSource,  int SourceLength,  int* TokenLength);

	int GetNumLines() { return m_NumLines; }

protected:
	int  ParaseToken();
	bool IsWhiteSpace();
	bool IsComment();
	bool IsKeyword();
	bool IsOperator();
	void InIdentifier();
	void InStringConstant();
	void InCharConstant();
	void InNumberConstant();

	const psCHAR   *m_pSource;	
	psETokenType	 m_TokenType;

	int m_SourceLength;
	int m_TokenLength;
	int m_NumLines;

};

#endif // __PS_TOKENIZER_H__
