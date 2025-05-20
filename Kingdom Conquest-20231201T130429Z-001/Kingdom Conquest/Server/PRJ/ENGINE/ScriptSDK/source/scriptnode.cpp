#include "scriptnode.h"

psCScriptNode::psCScriptNode(psENodeType type)
{
	m_pParent = 0;
	m_pPrev = 0;
	m_pNext = 0;
	m_pFirstChild = 0;
	m_pLastChild = 0;

	m_NodeType    = type;
	m_TokenType   = ttUnrecognizedToken;
	m_TokenPos    = 0;
	m_TokenLength = 0;
}

psCScriptNode::~psCScriptNode(void)
{
#ifndef NODE_IN_TEMP_MEMORY
	psCScriptNode* node = m_pFirstChild;
	psCScriptNode* next;
	while (node)
	{
		next = node->m_pNext;
		delete node;
		node = next;
	}
#endif
}

void psCScriptNode::AddChildLast(psCScriptNode* node)
{
	if (m_pLastChild)
	{	
		node->m_pParent = this;
		node->m_pPrev = m_pLastChild;
		node->m_pNext = 0;

		m_pLastChild->m_pNext = node;
		m_pLastChild = node;
	}else
	{	
		node->m_pParent = this;
		node->m_pPrev = 0;
		node->m_pNext = 0;

		m_pFirstChild = node;
		m_pLastChild = node;
	}
}

void psCScriptNode::DisconnectParent()
{
	if (m_pParent)
	{
		if (m_pParent->m_pFirstChild == this)
			m_pParent->m_pFirstChild = m_pNext;
		else if( m_pParent->m_pLastChild == this)
			m_pParent->m_pLastChild = m_pPrev;
	}

	if (m_pPrev)
		m_pPrev->m_pNext = m_pNext;

	if (m_pNext)
		m_pNext->m_pPrev = m_pPrev;

	m_pParent = 0;
	m_pNext = 0;
	m_pNext = 0;
}

void psCScriptNode::UpdateSourcePos(int pos, int length)
{
	if (pos == 0 && length == 0) return;

	if (m_TokenPos == 0 && m_TokenLength == 0)
	{
		m_TokenPos = pos;
		m_TokenLength = length;
	}else
	{
		if (m_TokenPos > pos)
		{
			m_TokenLength = m_TokenPos + m_TokenLength - pos;
			m_TokenPos = pos;
		}

		if (pos + length > m_TokenPos + m_TokenLength)
		{
			m_TokenLength = pos + length - m_TokenPos;
		}
	}
}

void psCScriptNode::SetToken(psSToken* token)
{
	m_TokenType = token->type;
}
