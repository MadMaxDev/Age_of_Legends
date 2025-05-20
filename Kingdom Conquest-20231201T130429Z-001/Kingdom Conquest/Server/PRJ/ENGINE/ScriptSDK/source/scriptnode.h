#ifndef __PS_SCRIPTNODE_H__
#define __PS_SCRIPTNODE_H__

#include "tokendef.h"
#include "memmanager.h"

enum psENodeType
{
	snUndefined,
	snScript,
	snFunction,
	snConstant,
	snDataType,
	snIdentifier,
	snParameterList,
	snArrayDim,
	snStatementBlock,
	snDeclaration,
	snVariable,
	snVariableDim,
	snInitExpr,
	snExpressionStatement,
	snIf,
	snFor,
	snWhile,
	snReturn,
	snExpression,
	snExprTerm,
	snFunctionCall,
	snArgList,
	snExprPreOp,
	snExprPostOp,
	snExprOperator,
	snSizeof,
	snTypeCast,
	snRefAssign,
	snExprValue,
	snBreak,
	snContinue,
	snDoWhile,
	snAssignment,
	snCondition,
	snGlobalVar,
	snSwitch,
	snCase,
	snInclude,
	snClassDecl,
	snPropertyDecl,
	snConstructor,
	snDestructor,
	snOpOverload,
	snEnum,
	snEnumConstant,
	snIdentifierValue,
};


class psCScriptNode
{ 
public:
	DECL_TEMP_MEM_OP()

public:
	psCScriptNode(psENodeType type);
	~psCScriptNode(void);

	void AddChildLast(psCScriptNode* node);
	void DisconnectParent();
	void SetToken(psSToken* token);

	void SetNodeType(psENodeType type)	  { m_NodeType = type; }
	psENodeType    GetNodeType() const	  { return m_NodeType;     }

	psETokenType   GetTokenType() const	  { return m_TokenType;    }
	int			   GetTokenPos() const	  { return m_TokenPos;	   }
	int			   GetTokenLength() const { return m_TokenLength;  }

	psCScriptNode* GetParent() const	  { return m_pParent;	   }
	psCScriptNode* GetFirstChild() const  { return m_pFirstChild;  }
	psCScriptNode* GetLastChild() const	  { return m_pLastChild;   }
	psCScriptNode* GetPrevNode() const	  { return m_pPrev;		   }
	psCScriptNode* GetNextNode() const	  { return m_pNext;		   }

	void UpdateSourcePos(int pos, int length);
protected:

	psENodeType   m_NodeType;
	psETokenType  m_TokenType;
	int			  m_TokenPos;
	int			  m_TokenLength;

	psCScriptNode* m_pParent;
	psCScriptNode* m_pNext;
	psCScriptNode* m_pPrev;
	psCScriptNode* m_pFirstChild;
	psCScriptNode* m_pLastChild;
};

#define NODE_IN_TEMP_MEMORY

#endif // __PS_SCRIPTNODE_H__
