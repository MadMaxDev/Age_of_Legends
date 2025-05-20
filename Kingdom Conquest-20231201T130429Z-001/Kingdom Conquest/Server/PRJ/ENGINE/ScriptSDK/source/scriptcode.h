#ifndef __PS_SCRIPTCODE_H__
#define __PS_SCRIPTCODE_H__
#if defined(_MSC_VER)
	#pragma warning(disable : 4786)
#endif

#include "config.h"
#include "string.h"

class psCScriptCode
{
public:
	psCScriptCode(void);
	~psCScriptCode(void);
public:
	void Destroy();

	void SetCode(const psCHAR* name, const psCHAR* code)
	{
		SetCode( name, code, ps_strlen(code) );
	}

	void  SetCode(const psCHAR* name, const psCHAR* code, size_t length)
	{
		m_Code.Copy(code, length);
		m_Name = name;
	}

	void SetFileName(const psCHAR* filename) { m_FileName = filename; }

	void GetAbosolutePath(psCString& path);

	int  CountLines(int startPos, int endPos);

	void ExtractLineNumbers();

	psCScriptCode* ConvertPosToRowCol(int pos, int *row, int *col);

	psCString		 m_Name;
	psCString		 m_Code;
	psCString		 m_FileName;
	int				 m_LineOffset;
	std::vector<int> m_LinePositions;
	std::vector< psCScriptCode* > m_SubCodes;
};

#endif // __PS_SCRIPTCODE_H__
