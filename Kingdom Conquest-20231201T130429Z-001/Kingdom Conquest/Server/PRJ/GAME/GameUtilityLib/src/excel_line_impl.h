// 表格的行实现代码

#include <new>

#include "../inc/misc.h"

namespace
{

struct CExcelIndexBinaryCell
{
    int m_id;
    int m_index;
};

int CExcelIndexBinaryCell_Compare(const void* x_, const void* y_)
{
    CExcelIndexBinaryCell* x = (CExcelIndexBinaryCell*)x_;
    CExcelIndexBinaryCell* y = (CExcelIndexBinaryCell*)y_;
    if(x->m_id < y->m_id) return -1;
    else if(x->m_id > y->m_id) return 1;
    return 0;
}
} // namespace

CExcelIndex::~CExcelIndex()
{
    if(m_data)
    {
        ::free(m_data);
    }
}

void CExcelIndex::Create(int type, int maxId, int* ids, int cnt, const string& name)
{
    const int limit_id = 65530;

	// 2009-8-18 liuyi reload时会有内存泄露，先清除一下
	if(m_data)
    {
        ::free(m_data);
		m_data = NULL;
    }

    switch(type)
    {
    case t_array:
        if(maxId >= limit_id)
        {
            type = t_binary;
        }
        break;
    case t_binary:
        break;
    default:
        // 缺省情况下，自动选择索引方式。
        if(maxId >= limit_id)
        {
            type = t_binary;
        }
        else
        {
            type = t_array;
        }
        break;
    }

    m_type = type;

    switch(type)
    {
    case t_array:
        {
            m_size = maxId+1;
		    unsigned short* v = (unsigned short*)malloc(sizeof(unsigned short)*m_size);
		    for(int i = 0; i <= maxId; ++ i)
		    {
			    v[i] = 0xFFFF;
		    }
		    for(int i = 0; i < cnt; ++ i)
		    {
			    int iID = ids[i];
			    if(ISIN_CC(0,iID,maxId))
			    {
				    v[iID] = (unsigned short)i;
			    }
			    else
			    {
				    char szInfo[10240];
				    sprintf(szInfo,GLGR_STD_HDR(49,ERR_EXCEL)",,,%s,%d,%d", name.c_str(), i, iID);
				    OutputErrorLog( szInfo );
			    }
		    }
            m_data = v;
        }
        break;
    case t_binary:
        {
            m_size = cnt;
		    CExcelIndexBinaryCell* v = (CExcelIndexBinaryCell*)malloc(sizeof(CExcelIndexBinaryCell)*m_size);
            for(int i = 0; i < cnt; ++ i)
            {
                v[i].m_id = ids[i];
                v[i].m_index = i;
            }
            ::qsort(v, m_size, sizeof(CExcelIndexBinaryCell), &CExcelIndexBinaryCell_Compare);
            m_data = v;
        }
        break;
    }
}

int CExcelIndex::Find(int id)
{
    switch(m_type)
    {
    case t_array:
        if(unsigned(id) < (unsigned)m_size)
        {
            int idx = ((unsigned short*)m_data)[id];
            if(idx < 0xFFFF) return idx;
        }
        break;
    case t_binary:
        {   // 使用折半查找。
            CExcelIndexBinaryCell* p = (CExcelIndexBinaryCell*)m_data;
            int c = m_size;
	        while(c > 0)
	        {
                int half = c/2;
                CExcelIndexBinaryCell* mid = p+half;

                if(mid->m_id == id)
                {
                    return mid->m_index;
                }
                if(mid->m_id < id)
                {
                    p = p+half+1;
                    c -= half+1;
                }
                else
                {
                    c = half;
                }
	        }
        }
        break;
    }
    return -1;
}

bool CExcelIndex::Insert(int id, int index)
{
    switch(m_type)
    {
    case t_array:
        if(unsigned(id) >= (unsigned)m_size)
        {
            if(id < 0)
            {
                return false;
            }
            unsigned short* v = (unsigned short*)::malloc(sizeof(unsigned short)*(id+1));

            for(int i = m_size; i < id; ++ i)
            {
                v[i] = 0xFFFF;
            }
            if(m_data)
            {
                memcpy(v, m_data, sizeof(unsigned short)*m_size);
                ::free(m_data);
            }
            v[id] = (unsigned short)index;
            m_data = v;
            m_size = id+1;
        }
        else
        {
            unsigned short* v = (unsigned short*)m_data;
            for(int i = id+1; i < m_size; ++ i)
            {
                if(v[i] != 0xFFFF)
                {
                    ++ v[i];
                }
            }
            v[id] = (unsigned short)index;
        }
        break;
    case t_binary:
        {
            // 使用折半查找，获得插入的位置。
            CExcelIndexBinaryCell* d = (CExcelIndexBinaryCell*)m_data;
            CExcelIndexBinaryCell* p = d;
            int c = m_size;
	        while(c > 0)
	        {
                int half = c/2;
                CExcelIndexBinaryCell* mid = p+half;
                if(mid->m_id < id)
                {
                    p = p+half+1;
                    c -= half+1;
                }
                else
                {
                    c = half;
                }
	        }
		    CExcelIndexBinaryCell* v = (CExcelIndexBinaryCell*)malloc(sizeof(CExcelIndexBinaryCell)*(m_size+1));
            int c1 = (int)(p-d);
            if(d)
            {
                int c2 = m_size-c1;
                // 索引递增
                for(int i = 0; i < c2; ++ i)
                {
                    ++ p[i].m_index;
                }
                memcpy(v, d, sizeof(CExcelIndexBinaryCell)*c1);
                memcpy(v+c1+1, p, sizeof(CExcelIndexBinaryCell)*c2);
                ::free(d);
            }
            v[c1].m_id = id;
            v[c1].m_index = index;
            m_data = v;
            ++ m_size;
        }
        break;
    }

    return true;
}

void CExcelIndex::Remove(int id)
{
    switch(m_type)
    {
    case t_array:
        if(unsigned(id) < (unsigned)m_size)
        {
            ((unsigned short*)m_data)[id] = 0xFFFF;
        }
        break;
    case t_binary:
        {   // 使用折半查找。
            CExcelIndexBinaryCell* p = (CExcelIndexBinaryCell*)m_data;
            int c = m_size;
	        while(c > 0)
	        {
                int half = c/2;
                CExcelIndexBinaryCell* mid = p+half;

                if(mid->m_id == id)
                {
                    ::memmove(mid, mid+1, ((CExcelIndexBinaryCell*)m_data+m_size-mid-1)*sizeof(CExcelIndexBinaryCell));
                    -- m_size;
                    break;
                }
                if(mid->m_id < id)
                {
                    p = p+half+1;
                    c -= half+1;
                }
                else
                {
                    c = half;
                }
	        }
        }
        break;
    }
}

namespace
{
//#ifdef _MSC_VER
#ifdef WIN32
#pragma push_macro("new")
#undef new
#endif
    template<class T>
	static void construct(T* p)
	{
		new(p) T;
	}
    template<class T, class P0>
	static void construct(T* p, P0 p0)
	{
		new(p) T(p0);
	}
    template<class T, class P0, class P1>
	static void construct(T* p, P0 p0, P1 p1)
	{
		new(p) T(p0, p1);
	}
    template<class T>
	static void destruct(T* p)
	{
		(void)p;
		p->~T();
	}
#ifdef _MSC_VER
#pragma pop_macro("new")
#endif
Excel_Cell  s_sp_cell[SP_COLUMN_COUNT <= 64 ? 64 : 0] =
{
    Excel_Cell(0L),   Excel_Cell(1L),   Excel_Cell(2L),    Excel_Cell(3L),
    Excel_Cell(4L),   Excel_Cell(5L),   Excel_Cell(6L),    Excel_Cell(7L),
    Excel_Cell(8L),   Excel_Cell(9L),   Excel_Cell(10L),   Excel_Cell(11L),
    Excel_Cell(12L),  Excel_Cell(13L),  Excel_Cell(14L),   Excel_Cell(15L),

    Excel_Cell(20L),  Excel_Cell(25L),  Excel_Cell(30L),   Excel_Cell(35L),
    Excel_Cell(40L),  Excel_Cell(45L),  Excel_Cell(50L),   Excel_Cell(55L),
    Excel_Cell(60L),  Excel_Cell(65L),  Excel_Cell(70L),   Excel_Cell(75L),
    Excel_Cell(80L),  Excel_Cell(85L),  Excel_Cell(90L),   Excel_Cell(95L),

    Excel_Cell(100L), Excel_Cell(110L), Excel_Cell(120L),  Excel_Cell(130L),
    Excel_Cell(140L), Excel_Cell(150L), Excel_Cell(160L),  Excel_Cell(170L),
    Excel_Cell(180L), Excel_Cell(190L), Excel_Cell(200L),  Excel_Cell(210L),
    Excel_Cell(220L), Excel_Cell(230L), Excel_Cell(240L),  Excel_Cell(250L),

    Excel_Cell(300L), Excel_Cell(350L), Excel_Cell(400L),  Excel_Cell(450L),
    Excel_Cell(500L), Excel_Cell(550L), Excel_Cell(600L),  Excel_Cell(650L),
    Excel_Cell(700L), Excel_Cell(750L), Excel_Cell(800L),  Excel_Cell(850L),
    Excel_Cell(900L), Excel_Cell(950L), Excel_Cell(1000L), Excel_Cell(1100L),
};

int ConstructCell(Excel_Cell* pCell, Byte bDef, const char * szValue)
{
	if(ISEMPTY(szValue))
	{//空 Cell
		return SP_COLUMN_EMPTY;
	}
	switch(bDef)
	{
	case DEF_INT:
		{
			long lValue = atol(szValue);
            // 使用折半查找。
            Excel_Cell* b = s_sp_cell;
            Excel_Cell* p = b;
            int c = SP_COLUMN_COUNT;
	        while(c > 0)
	        {
                int half = c/2;
                Excel_Cell* mid = p+half;

                if(mid->theInt == lValue)
                {
                    return SP_COLUMN_BASE+(int)size_t(mid-b);
                }
                if(mid->theInt < lValue)
                {
                    p = p+half+1;
                    c -= half+1;
                }
                else
                {
                    c = half;
                }
	        }
			construct(pCell, lValue);
		}
		break;
	case DEF_STR:
	case DEF_VARIABLE:
		{
			construct(pCell, szValue);
		}	
		break;
	case DEF_FLOAT:
		{
			float fValue = (float)atof(szValue);
			construct(pCell, fValue);
		}
		break;
	case DEF_INTARRAY:
		{
			vector<long> v;
			GetFromString(szValue,v);
			construct(pCell, &(v[0]),(int)v.size());
		}
		break;
	case DEF_STRARRAY://@@
	case DEF_VARIABLEARRAY:
		{
			vector<const char *> v;
			GetFromString(szValue,v);
			construct(pCell, &(v[0]),(int)v.size());
		}
		break;
	case DEF_FLOATARRAY:
		{
			vector<float> v;
			GetFromString(szValue,v);
			construct(pCell, &(v[0]),(int)v.size());
		}	
		break;
	default:
        pCell->theStr = 0;
		return SP_COLUMN_EMPTY;
	}
    return 0;
}

} // namespace

SExcelLine::SExcelLine()
{
	m_ParentTable   = 0;
    m_pCellData     = 0;
    m_nSize         = 0;
    m_nUsed         = 0;
	m_iLineID       = 0;
}

SExcelLine::~SExcelLine()
{
    if(m_pCellData)
    {
	    for(int i = 0; i < (int)m_nUsed; ++ i)
	    {
            destruct(m_pCellData + i);
	    }
        int idx_mem = ((int)m_nSize+3)&(~3);
        ::Excel_LineData_Free(m_pCellIndex-idx_mem);
    }
}

Excel_Cell* SExcelLine::GetCell(int iPos)
{
    if((unsigned)iPos >= m_nSize) return 0;

    int idx = m_pCellIndex[-(int)m_nSize+iPos];
    if(idx >= MAXCOLUMNS)
    {
        if(idx == SP_COLUMN_EMPTY) return 0;
        return s_sp_cell+idx-SP_COLUMN_BASE;
    }

    return m_pCellData + idx;
}

void SExcelLine::CreateCell(int nIdx, Byte bDef, const char * szValue)
{
	if ( ISEMPTY(szValue) )
	{//空 Cell
		return;
	}

    Excel_Cell vCell;
    int CellIdx = ConstructCell(&vCell, bDef, szValue);

    int used    = (int)m_nUsed;
    if(nIdx < m_nSize)
    {   // 索引数据还够用
        unsigned char& refidx = m_pCellIndex[-(int)m_nSize+nIdx];
        if(CellIdx >= SP_COLUMN_BASE)
        {   // 如果是特殊索引，那么就不需要扩展数据了。
            refidx = CellIdx;
            return;
        }
        else if(refidx < m_nUsed)
        {   // 如果是索引到一个已有的数据，那么就只需要替换就可以了。
            m_pCellData[refidx] = vCell;
            return;
        }
        ++ used;
    }
    else
    {
        // 构造失败，不需要扩展数据。
        if(CellIdx == SP_COLUMN_EMPTY) return;
        if(CellIdx < SP_COLUMN_BASE)
        {
            ++ used;
        }
    }

    // 重新调整数据的大小。

    int cnt     = nIdx >= m_nSize ? nIdx+1 : m_nSize;
    int idx_mem = (cnt+3)&(~3);
    int mem     = idx_mem+used*sizeof(Excel_Cell);

    unsigned char* pCellIndex = (unsigned char*)::Excel_LineData_Alloc(mem) + idx_mem;
    if(m_pCellIndex)
    {
        // 复制和初始化索引
        for(int i = 0; i < (int)m_nSize; ++ i)
        {
            pCellIndex[-(int)cnt+i] = m_pCellIndex[-(int)m_nSize+i];
        }
        for(int i = (int)m_nSize; i < cnt; ++ i)
        {
            pCellIndex[-(int)cnt+i] = SP_COLUMN_EMPTY;
        }

        // 复制单元格
        Excel_Cell* pCellData = (Excel_Cell*)pCellIndex;
        memcpy(pCellData, m_pCellData, (int)m_nUsed*sizeof(Excel_Cell));

        // 资源释放
        ::Excel_LineData_Free(m_pCellIndex-(((int)m_nSize+3)&(~3)));
    }
    else
    {
        for(int i = 0; i < cnt; ++ i)
        {
            pCellIndex[-(int)cnt+i] = SP_COLUMN_EMPTY;
        }
    }
    m_pCellIndex = pCellIndex;
    m_nSize      = (unsigned char)cnt;
    m_nUsed      = used;

    if(CellIdx >= SP_COLUMN_BASE)
    {   // 特殊索引
        m_pCellIndex[-(int)m_nSize+nIdx] = CellIdx;
        return;
    }

    m_pCellIndex[-(int)m_nSize+nIdx]  = used-1;
    m_pCellData[used-1] = vCell;
}

//单元赋值函数
void SExcelLine::AddCell( Byte bDef, const char * szValue )
{
    CreateCell(m_nSize-1, bDef, szValue);
}

bool SExcelLine::SetCell( int iCol, const char * szValue )
{
	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return false;
	}

    CreateCell(iPos, m_ParentTable->GetPosDef(iPos), szValue);

	return true;
}

bool SExcelLine::DirectIsValue( int iPos,int iValue )
{
    Excel_Cell* pCell = GetCell(iPos);
	if(pCell)
	{
		return ( pCell->theInt == iValue );
	}
	return false;
}

bool SExcelLine::DirectIsValue( int iPos,const char * szValue )
{
    if(ISEMPTY(szValue)) return false;
    Excel_Cell* pCell = GetCell(iPos);
	if(pCell)
	{
		return ( strcmp(pCell->theStr,szValue) == 0 );
	}
	return false;
}

bool SExcelLine::DirectIsValue( int iPos,float fValue )
{
    Excel_Cell* pCell = GetCell(iPos);
	if(pCell)
	{
		return ( pCell->theFloat == fValue );
	}
	return false;
}

int SExcelLine::DirectGetVariable( int iPos,int &iValue )
{
    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		iValue = atoi(pCell->theStr);
		return RE_OK;
	}
	else
	{
		iValue = 0;
		return RE_NOVALUE;
	}
}

int SExcelLine::DirectGetVariable( int iPos,float &fValue )
{
    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		fValue = (float)atof(pCell->theStr);
		return RE_OK;
	}
	else
	{
		fValue = 0.0;
		return RE_NOVALUE;
	}
}

void SExcelLine::DirectGetString( int iPos,Byte bDef,string &szValue )
{
    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		GetCellStr( pCell, bDef, szValue );
	}
	else
	{
		szValue = "";
	}
}

int SExcelLine::GetValue( int iCol,int &iValue )
{
	iValue = 0;

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_INT);
    if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		if ( iDefRe == RE_OK )
		{
			iValue = pCell->theInt;
		}
		else
		{
			iValue = atoi(pCell->theStr);
		}
        return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetValue( int iCol,string &szValue )
{
	szValue = "";

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	// 任何类型的数据都可以 get 到 string
	DirectGetString( iPos, m_ParentTable->GetPosDef(iPos), szValue );

	if ( szValue == "" )
	{
		return RE_NOVALUE; // 空 Cell
	}
	return RE_OK;
}

int SExcelLine::GetValue( int iCol,float &fValue )
{
	fValue = 0.0;

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_FLOAT);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		if ( iDefRe == RE_OK )
		{
			fValue = pCell->theFloat;
		}
		else
		{
			fValue = (float)atof(pCell->theStr);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetArray( int iCol,vector<Byte> &Output )
{
	Output.clear();

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_INTARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();

		if ( iDefRe == RE_OK )
		{
			long * IntVec = pCell->theIntVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( (Byte)IntVec[i] );
			}
		}
		else
		{
			const char** StrVec = pCell->theStrVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( (Byte)atoi(StrVec[i]) );
			}
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetArray( int iCol,vector<int> &Output )
{
	Output.clear();

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_INTARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();

		if ( iDefRe == RE_OK )
		{
			long * IntVec = pCell->theIntVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( IntVec[i] );
			}
		}
		else
		{
			const char** StrVec = pCell->theStrVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( atoi(StrVec[i]) );
			}
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetArray( int iCol,vector<const char *> &Output )
{
	Output.clear();

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_STRARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();
		const char** StrVec = pCell->theStrVec;

		for ( int i=0; i<iLen; i++ )
		{
			Output.push_back(StrVec[i]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetArray( int iCol,vector<float> &Output )
{
	Output.clear();

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_FLOATARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();

		if ( iDefRe == RE_OK )
		{
			float* FloatVec = pCell->theFloatVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( FloatVec[i] );
			}
		}
		else
		{
			const char** StrVec = pCell->theStrVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( (float)atof(StrVec[i]) );
			}
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetValue( int iCol,int idx,int &iValue )
{
	iValue = 0;

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_INTARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell && ISIN_CO(0,idx,(int)pCell->GetVecCnt()) )
	{
		if ( iDefRe == RE_OK )
		{
			iValue = pCell->theIntVec[idx];
		}
		else
		{
			iValue = atoi(pCell->theStrVec[idx]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetValue( int iCol,int idx,string &szValue )
{
	szValue = "";

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_STRARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell && ISIN_CO(0,idx,(int)pCell->GetVecCnt()) )
	{
		szValue = pCell->theStrVec[idx];
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetValue( int iCol,int idx,float &fValue )
{
	fValue = 0.0f;

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_FLOATARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell && ISIN_CO(0,idx,(int)pCell->GetVecCnt()) )
	{
		if ( iDefRe == RE_OK )
		{
			fValue = pCell->theFloatVec[idx];
		}
		else
		{
			fValue = (float)atof(pCell->theStrVec[idx]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

void SExcelLine::ErrorLogLineCol( const char * szError,int iCol,int iRE,int idx )
{
	int iExcelID = -1;
	if( m_ParentTable->GetExcelIDCol() != -1
		&& m_ParentTable->GetExcelIDCol() != iCol )
	{
		iExcelID = SafeGetInt( m_ParentTable->GetExcelIDCol() );
	}

    std::string buf;
    G_MiscGetStackTrace(buf);

    OutputErrorLogF(
        GLGR_STD_HDR(44,ERR_EXCEL)"%s,%d,,%s,%d,%d,%d,%d,%s",
		szError,
		iRE,
		m_ParentTable->GetName(), 
		m_iLineID
		, iCol,idx,iExcelID,buf.c_str());
}

int SExcelLine::SafeGetInt( int iCol,bool booCanBeNull )
{
	int iValue;
	int iRE = GetValue(iCol,iValue);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_Int",iCol,iRE );
	}
	return iValue;
}

const char * SExcelLine::FastGetStr( int iCol,bool booCanBeNull )
{
	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		ErrorLogLineCol( "Excel_FastStr",iCol,RE_NOCOLUM );
		return g_szNULL;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_STR);
	if ( iDefRe == RE_DEFERROR )
	{
		ErrorLogLineCol( "Excel_FastStr",iCol,RE_DEFERROR );
		return g_szNULL;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		return pCell->theStr;
	}

	if ( !booCanBeNull )
	{
		ErrorLogLineCol( "Excel_FastStr",iCol,RE_NOVALUE );
	}
	return g_szNULL;
}

void SExcelLine::SafeGetStr( int iCol,string &szValue,bool booCanBeNull )
{
	int iRE = GetValue(iCol,szValue);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_Str",iCol,iRE );
	}
}

float SExcelLine::SafeGetFloat( int iCol,bool booCanBeNull )
{
	float fValue;
	int iRE = GetValue(iCol,fValue);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_Float",iCol,iRE );
	}
	return fValue;
}

void SExcelLine::SafeGetArray( int iCol,vector<int> &Output,bool booCanBeNull )
{
	int iRE = GetArray(iCol,Output);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_ArrayInt",iCol,iRE );
	}
}

void SExcelLine::SafeGetArray( int iCol,vector<const char *> &Output,bool booCanBeNull )
{
	int iRE = GetArray(iCol,Output);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_ArrayStr",iCol,iRE );
	}
}

void SExcelLine::SafeGetArray( int iCol,vector<float> &Output,bool booCanBeNull )
{
	int iRE = GetArray(iCol,Output);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_ArrayFloat",iCol,iRE );
	}
}

int SExcelLine::SafeGetArrayInt( int iCol,int idx,bool booCanBeNull )
{
	int iValue;
	int iRE = GetValue(iCol,idx,iValue);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_ArrayIntIdx",iCol,iRE,idx );
	}
	return iValue;
}

int SExcelLine::SafeGetRandInt( int iCol,bool booCanBeNull )
{
	int iValue;
	int iRE = RandGetData(iCol,iValue);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_SafeGetRandInt",iCol,iRE );
	}
	return iValue;
}

const char * SExcelLine::FastGetArrayStr( int iCol, int idx, bool booCanBeNull )
{
	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		ErrorLogLineCol( "Excel_ArrayFastStrIdx",iCol,RE_NOCOLUM,idx );
		return g_szNULL;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_STRARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		ErrorLogLineCol( "Excel_ArrayFastStrIdx",iCol,RE_DEFERROR,idx );
		return g_szNULL;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell && ISIN_CO(0,idx,(int)pCell->GetVecCnt()) )
	{
		return pCell->theStrVec[idx];
	}

	if ( !booCanBeNull )
	{
		ErrorLogLineCol( "Excel_ArrayFastStrIdx",iCol,RE_NOVALUE,idx );
	}
	return g_szNULL;
}

void SExcelLine::SafeGetArrayStr( int iCol,int idx,string &szValue,bool booCanBeNull )
{
	int iRE = GetValue(iCol,idx,szValue);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_ArrayStrIdx",iCol,iRE,idx );
	}
}

float SExcelLine::SafeGetArrayFloat( int iCol,int idx,bool booCanBeNull )
{
	float fValue;
	int iRE = GetValue(iCol,idx,fValue);
	if ( iRE > RE_OK && 
		( iRE != RE_NOVALUE || !booCanBeNull ) )
	{
		ErrorLogLineCol( "Excel_ArrayFloatIdx",iCol,iRE,idx );
	}
	return fValue;
}

int SExcelLine::GetArraySize( int iCol )
{
	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return 0;
	}
	
	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_VARIABLEARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return 0;
	}
	
    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
    {
		return pCell->GetVecCnt();
	}
	return 0; // 空 Cell
}

int SExcelLine::GetRandx( int iCol,int &iData,int &iDataIdx,int iRate )
{
	iData = 0;

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		ErrorLogLineCol( "Excel_GetRandx", iCol, RE_NOCOLUM );
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_INTARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		ErrorLogLineCol( "Excel_GetRandx", iCol, RE_DEFERROR );
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();
		if ( iLen % 2 != 0 )
		{// 数量不是偶数，说明格式错误，不符合要求
			ErrorLogLineCol( "Excel_GetRandx", iCol, RE_CELLERROR );
			return RE_CELLERROR;
		}

		int iRand = GetMaxRand(iRate);
		int iCount = 0;

		for ( int idx=0; idx<iLen; idx+=2 )
		{
			if ( iDefRe == RE_OK )
			{
				iCount += pCell->theIntVec[idx];
			}
			else
			{
				iCount += atoi(pCell->theStrVec[idx]);
			}

			if ( iCount >= iRand )
			{
				if ( iDefRe == RE_OK )
				{
					iData = pCell->theIntVec[idx+1];
				}
				else
				{
					iData = atoi(pCell->theStrVec[idx+1]);
				}
				iDataIdx = idx/2;
				return RE_OK;
			}
		}
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetRandx( int iCol,string &szData,int &iDataIdx,int iRate )
{
	szData = "";

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		ErrorLogLineCol( "Excel_GetRandx", iCol, RE_NOCOLUM );
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_STRARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		ErrorLogLineCol( "Excel_GetRandx", iCol, RE_DEFERROR );
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();
		if ( iLen % 2 != 0 )
		{// 数量不是偶数，说明格式错误，不符合要求
			ErrorLogLineCol( "Excel_GetRandx", iCol, RE_CELLERROR );
			return RE_CELLERROR;
		}

		int iRand = GetMaxRand(iRate);
		int iCount = 0;

		for ( int idx=0; idx<iLen; idx+=2 )
		{
			iCount += atoi(pCell->theStrVec[idx]);

			if ( iCount >= iRand )
			{		
				szData = pCell->theStrVec[idx+1];
				iDataIdx = idx/2;
				return RE_OK;
			}
		}
	}
	return RE_NOVALUE; // 空 Cell
}

int SExcelLine::GetRand100( int iCol,int &iData,int &iDataIdx )
{
	return GetRandx(iCol,iData,iDataIdx,100);
}

int SExcelLine::GetRand100( int iCol,string &szData,int &iDataIdx )
{
	return GetRandx(iCol,szData,iDataIdx,100);
}

int	SExcelLine::RandGetData( int iCol,int &iData )
{
	iData = 0;

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_INTARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();
		int idx = rand() % iLen;

		if ( iDefRe == RE_OK )
		{
			iData = pCell->theIntVec[idx];
		}
		else
		{
			iData = atoi(pCell->theStrVec[idx]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int	SExcelLine::RandGetData( int iCol,string &szData )
{
	szData = "";

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_STRARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();
		int idx = rand() % iLen;
		szData = pCell->theStrVec[idx];
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

int	SExcelLine::RandGetData( int iCol,float &fData )
{
	fData = 0.0;

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_FLOATARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return RE_DEFERROR;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iLen = pCell->GetVecCnt();
		int idx = rand() % iLen;

		if ( iDefRe == RE_OK )
		{
			fData = pCell->theFloatVec[idx];
		}
		else
		{
			fData = (float)atof(pCell->theStrVec[idx]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // 空 Cell
}

//判断复合单元中是否有iData
bool SExcelLine::IsData( int iCol,int iData )
{
	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return false;
	}

	int iDefRe = m_ParentTable->CheckPosDef(iPos,DEF_INTARRAY);
	if ( iDefRe == RE_DEFERROR )
	{
		return false;
	}

    Excel_Cell* pCell = GetCell(iPos);
    if(pCell)
	{
		int iNum = pCell->GetVecCnt();
	
		if ( iDefRe == RE_OK )
		{
			for ( int i=0; i<iNum; i++ )
			{
				if ( iData == pCell->theIntVec[i] )
				{
					return true;
				}
			}
		}
		else
		{
			for ( int i=0; i<iNum; i++ )
			{
				if ( iData == atoi(pCell->theStrVec[i]) )
				{
					return true;
				}
			}
		}
	}
	return false; // 空 Cell
}

bool SExcelLine::IsNull( int iCol )
{
	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return true;
	}

	if ( GetCell(iPos) )
	{
		return false;
	}
	return true; // 空 Cell
}

//保存父表
void SExcelLine::SetParentTable( CExcelTable* ParentTable )
{
	m_ParentTable = ParentTable;
}

//保存行号
void SExcelLine::SetLineID( unsigned short iLineID )
{
	m_iLineID = iLineID;
}

//获取行号
int	SExcelLine::GetLineID()
{
	return (int)m_iLineID;
}

void SExcelLine::GetCellStr( Excel_Cell * pCell,Byte bDef,string &szValue )
{
	szValue = "";

	if ( pCell == NULL )
	{
		return;
	}

	switch ( bDef )
	{
	case DEF_INT:
		{
			char SS[MAXNUMBER];
			sprintf(SS,"%d",int(pCell->theInt));
			szValue = SS;
		}
		break;
	case DEF_STR:
	case DEF_VARIABLE:
		{
			szValue = pCell->theStr;
		}	
		break;
	case DEF_FLOAT:
		{
			char SS[MAXNUMBER];
			sprintf(SS,"%f",pCell->theFloat);
			szValue = SS;
		}
		break;
	case DEF_INTARRAY:
		{
			int iSize = pCell->GetVecCnt();

			if ( iSize > 0 )
			{
				char SS[MAXNUMBER];
				sprintf(SS,"%d",int(pCell->theIntVec[0]));
				szValue = SS;

				for ( int i=1; i<iSize; i++ )
				{
					sprintf(SS,"*%d",int(pCell->theIntVec[i]));
					szValue.append(SS);
				}
			}
		}
		break;
	case DEF_STRARRAY:
	case DEF_VARIABLEARRAY:
		{
			int iSize = pCell->GetVecCnt();

			if ( iSize > 0 )
			{
				char SS[MAXSUBBYTE];
				sprintf(SS,"%s",pCell->theStrVec[0]);
				szValue = SS;

				for ( int i=1; i<iSize; i++ )
				{
					sprintf(SS,"*%s",pCell->theStrVec[i]);
					szValue.append(SS);
				}
			}
		}
		break;
	case DEF_FLOATARRAY:
		{
			int iSize = pCell->GetVecCnt();

			if ( iSize > 0 )
			{
				char SS[MAXNUMBER];
				sprintf(SS,"%f",pCell->theFloatVec[0]);
				szValue = SS;

				for ( int i=1; i<iSize; i++ )
				{
					sprintf(SS,"*%f",pCell->theFloatVec[i]);
					szValue.append(SS);
				}
			}
		}	
		break;
	default:
		break;
	}
}
