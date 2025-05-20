#include "scriptcode.h"

psCScriptCode::psCScriptCode() : m_LineOffset(0)
{
}

psCScriptCode::~psCScriptCode(void)
{
	Destroy();
}

void psCScriptCode::Destroy()
{
	for (size_t i = 0; i < m_SubCodes.size(); ++i )
	{
		delete m_SubCodes[i];
	}
	m_SubCodes.clear();
	m_LinePositions.clear();
}

void psCScriptCode::GetAbosolutePath(psCString& path)
{
	// 解析出件名
	int pos = (int)m_FileName.GetLength() - 1;
	for (; pos >= 0; --pos)
	{
		if (m_FileName[pos] == PS_T('\\') ||
			m_FileName[pos] == PS_T('/') )
			break;
	}
	if (pos >= 0)
	{	
		psCString relPath;
		relPath.Copy(m_FileName.AddressOf(), pos + 1);
		path = relPath + path;
	}
}

void psCScriptCode::ExtractLineNumbers()
{
	// Find the positions of each line
	m_LinePositions.clear();
	
	m_LinePositions.push_back(0);
	for (int n = 0; n < (int)m_Code.GetLength(); ++n)
		if ( m_Code[n] == PS_T('\n'))
		{
			m_LinePositions.push_back(n+1);
		}
	m_LinePositions.push_back( (int)m_Code.GetLength() );
}

int psCScriptCode::CountLines(int startPos, int endPos)
{
	if (startPos < 0) startPos = 0;
	if (endPos > (int)m_Code.GetLength() - 1)
		endPos = (int)m_Code.GetLength() - 1;

	int count = 0;
	for (int i = startPos; i <= endPos; ++i)
		if (m_Code[i] == PS_T('\n'))
		{
			count++;
		}
	return count;

}
psCScriptCode* psCScriptCode::ConvertPosToRowCol(int pos, int *row, int *col)
{
	if (m_LinePositions.size() == 0)
	{
		if (row) *row = 0;
		if (col) *col = 1;
		return this;
	}
	int max = int(m_LinePositions.size() - 1);
	int min = 0;
	int i = max/2;

	// 做二叉搜索
	for (;;)
	{
		if ( m_LinePositions[i] < pos)
		{
			// Have we found the largest number < programPosition?
			if ( min == i ) break;

			min = i;
			i = (min + max)/2;
		}else if (m_LinePositions[i] > pos)
		{
			// Have we found the smallest number > programPoisition?
			if( max == i ) break;
			
			max = i;
			i = (min + max)/2;
		}else
		{
			// We found the exact position
			break;
		}
	}

	int r = i + 1;
	int c = pos - m_LinePositions[i] + 1;
	// 如果其有子代码段的话,有可能是子代码段
	for ( size_t k = 0; k < m_SubCodes.size(); ++k )
	{
		if ( m_SubCodes[k]->m_LineOffset == r )
		{
			m_SubCodes[k]->ConvertPosToRowCol(c, row, col);
			return m_SubCodes[k];
		}
	}
	if (row) *row = r;
	if (col) *col = c;

	return this;
}

