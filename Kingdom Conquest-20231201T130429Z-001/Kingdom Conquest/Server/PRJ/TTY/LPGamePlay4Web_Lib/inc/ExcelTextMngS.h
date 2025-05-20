#ifndef __ExcelTextMng_H__
#define __ExcelTextMng_H__

#include "../../Common/inc/tty_common_excel_def.h"

#include "WHCMN/inc/whvector.h"

using namespace n_whcmn;

namespace n_pngs
{
class CExcelTextMngS
{
public:
	inline ExcelText*	GetText(unsigned int nExcelID)
	{
		if (nExcelID==0 || nExcelID>m_vectExcelTexts.size() || m_vectExcelTexts[nExcelID-1].nExcelID==0)
		{
			return NULL;
		}
		return &m_vectExcelTexts[nExcelID-1];
	}
	inline void Load(size_t nSize, int nNum, ExcelText* pExcel)
	{
		m_vectExcelTexts.clear();
		m_vectExcelTexts.resize(nSize);
		for (int i=0; i<m_vectExcelTexts.size(); i++)
		{
			m_vectExcelTexts[i].nExcelID	= 0;
		}
		for (int i=0; i<nNum; i++)
		{
			m_vectExcelTexts[pExcel[i].nExcelID-1]	= pExcel[i];
		}
	}
private:
	whvector<ExcelText>		m_vectExcelTexts;
};
}

#endif
