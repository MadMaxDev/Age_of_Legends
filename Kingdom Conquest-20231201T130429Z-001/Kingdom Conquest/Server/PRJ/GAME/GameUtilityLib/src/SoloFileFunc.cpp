/*****************************************************************************************
														
	ģ������SOLOFILEFUNC								
														
	  Ŀ�ģ��ļ�����								
														
	  ԭ���ߣ�soloman	
	  
	  �޸��ߣ�swolf(����)
			  Erica Gao
														
	��������Microsoft Visual C++ 6.0
														
	  ��ʷ��2000-6-23 9:27:13 ���� 
			2005-6-20 �޸�����
			2006-2-28 �޸�
			2006-3-14 ����д�뺯�������Ա༭������ֵ���
			2006-3-16 ��Array�����ݼӽ�Variable���ͣ���Ϊstring����Cell
					  GetArrayʱ��ת��
					  ��չGetStr������get���κ��������ݵ�string��ʽ
			2006-3-17 ȱʡ��������Ϊ�����һ�������˵��������ͣ�û�ж���ʱΪVariable

					  Load��ʹ���ļ�ȫ��(����·��)������
					  ȥ����ͨ��"�ļ���.txt"���Tableָ��ĺ���

					  �޸�SetCell������ֱ���޸ķ�Load���ɵ�Line
					  ����һ��SetLine��ʽ�����Խ���Load���ɵ�Line��ӽ�Table��

					  ��Line�������˶�ParentTableָ��ļ�⣬
						��Ϊ��Load���ɵ�Line�޷���֤ParentTable��Ϊ��

					  ��Table�������˶�Lineָ��ļ�⣬
					    ��Ϊ�ڱ༭�б�ɾ����Line��ָ���Ϊ��
														
			2007-7-3  By Jiangli��Ϊ��Լ�ڴ棬����һЩ�Ż��������ռ���ڴ�Լ22M����
                      1 �޸�CExcelTable::m_pLinesΪshort���顣��Ҫע�⣬�п�
                        ���޸ı����ݵ�ĳЩ���������������4M�ࣩ
                      2 ׷��Cellʱ��reserve��������1M����Loadʱ����1110����1031��
                      3 ΪExcel_Cell�ṩnew��delete��������ʹ�ø���Ч�ͽ�Լ��
                        ���䷽ʽ��������6M�ࣩ��Loadʱ����1031����760��
                      4 ��ͬ�Ķ��ַ������й����������ٽ�0.3M��
                      5 ΪSExcelLine�ṩnew��delete��������ʹ�ø���Ч�ͽ�Լ��
                        ���䷽ʽ��������0.2M�ࣩ��Loadʱ����760����720��
                      6 �Ż��˱��������롣��Loadʱ����720����281��
                      7 �����˶����ŵ�֧�֣���Excel��ʽ���ݡ�
			2008-6-22 By Jiangli��Ϊ��Լ�ڴ棬����һЩ�Ż���
                      1 ExcelBuild֮ǰ��16076K
                      2 ExcelBuild֮��
                        ԭʼ���룺81320K
                        �Ż�����1��75896K��ʹ�����鱣�浥Ԫ�񣬶�����ָ�����飩
                        �Ż�����2��66948K����������ֵ�����ݣ��յ�Ԫ��ռ���ڴ��ˡ���
                        �Ż�����3��64996K������������ֵ��ָ�����ⵥԪ�������Ԫ���ǹ���ģ�
                        �Ż�����4��62760K��ʹ���ڴ�ط����������ݣ���ʹ�༭���ڴ治������������Ϸ��Ӱ�죬�ɹرգ�
                        �Ż�����5��28480K���Բ��ֱ������ʹ�ö��ַ����������������顣����34280K��
                        �ڴ��ܼ��٣�52840K
                      3 ע�⣬MAXCOLUMNS����Ϊ200��
                      4 ʹ��SetCell��AddCell�����ĵط���Ҫ��һ�����ԣ���Ҫָ�༭������

	  ��ע���������ɶ��뵥���Ϊ����index_list.txt������������ȫ��Ҫʹ�õı�
			ʹ�÷�����
					CExcelTableMng m_ExcelTableMng;
					m_ExcelTableMng.LoadTables(szDir,iType);//iTpyeΪʹ������,szDirΪ��Ŀ¼

					ʹ���� m_ExcelTableMng.Release();
			����ʹ�÷�����
					SExcelLine *pLine = NULL;
					pLine = m_ExcelTableMng.m_pTables[Itemlist]->GetUnit(iExcelID);
					//Itemlist��Ҫ����˳���Լ�define��iExcelIDΪ����ֵ
					//û�������еı�Ҫ��GetLine(iLine)��������ʱiLineΪ�к�
			��Ԫֵ��÷�����
					int re = pLine->GetValue(iCol,&value);
					//reΪ���������valueΪ��õĵ�Ԫֵ����������������������Ĭ��ֵ��
					//int����Ĭ��ֵΪ0��float����Ĭ��ֵΪ0��string����Ĭ��ֵΪ""
														
*****************************************************************************************/
#include <stdio.h>
#include "../inc/SoloFileFunc.h"
#include "../inc/LogMngInf.h"
#include "../inc/MemUtil.h"
#include "../inc/misc.h"
#include "../inc/EmString.h"

#include <WHCMN/inc/whtime.h>

#define MyDeleteArray(ptr){ if (ptr) delete [] ptr; ptr = NULL; }
#define MAX(x,y) ( ((x)>(y))? (x):(y) )
#define MyDeletePoint(p) { if(p) {delete p; p = NULL;} }
#define ISIN_CC(a,b,c)( (((b)>=(a))&&((b)<=(c))) ? TRUE : FALSE )
#define ISIN_CO(a,b,c)( (((b)>=(a))&&((b)<(c))) ? TRUE : FALSE )
#define ISIN_OO(a,b,c)( (((b)>(a))&&((b)<(c))) ? TRUE : FALSE )

#define ISEMPTY( a ) ( a == NULL || strcmp(a,"") == 0 )

#define FOPEN	WHCMN_FILEMAN_GET()->Open

string g_szDir;
CExcelTableMng g_ExcelTableMng;

extern void* Excel_LineData_Alloc(int c);
extern void Excel_LineData_Free(void*);

//struct mem_track
//{
//    int m_count;
//    int m_size;
//    const char* m_name;
//    mem_track(const char* n);
//    ~mem_track();
//    void add(size_t x);
//};
//
//mem_track g_index_track("index");

//************************************//
//              Excel��������         //
//************************************//

CExcelTableMng::CExcelTableMng()
{
	m_pIndexTable	= NULL;
	m_pTables = NULL;
	m_nCount = 0;
	srand((unsigned int)wh_time());
}

CExcelTableMng::~CExcelTableMng()
{
	Release();
}

//��Ҫ���Ƶ����б�����ڴ档
//iType�ǿ��Ƶ����,0��ͨ�ã�1�Ƿ�����ʹ�ã�2�ǿͻ���ʹ��
bool CExcelTableMng::LoadTables( const char * szDir,int iType )
{
	// ���ṩ��������
	iType	= 0;
	if ( szDir == NULL )
	{
		return false;
	}

	g_szDir = szDir;

	m_pIndexTable	= new CExcelTable();
	CExcelTable& Indextable	= *m_pIndexTable;
	//ģ������������
	Indextable.m_vDefs.push_back(DEF_STR);//��ע
	Indextable.m_vDefs.push_back(DEF_INT);//����
	Indextable.m_vDefs.push_back(DEF_STRARRAY);//�ļ���
	Indextable.m_vDefs.push_back(DEF_INT);//������
	Indextable.m_vDefs.push_back(DEF_INT);//��ʼ��
	Indextable.m_vDefs.push_back(DEF_INT);//ʹ������
	Indextable.m_vDefs.push_back(DEF_INTARRAY);//��������
	Indextable.m_vDefs.push_back(DEF_STR);//���ݿ��Ӧ���

	string szTemp;
	szTemp = g_szDir;
	szTemp += "index_list.txt";

	//����������
	if ( Indextable.Load(szTemp.c_str(),1,1) == false )
	{
		char szInfo[10240];
		sprintf( szInfo, GLGR_STD_HDR(47,ERR_EXCEL)"LoadTables,,,%s,%d,%s"
			, szDir, iType , szTemp.c_str() );
		OutputErrorLog( szInfo );
		return false;
	}

	//����������б������,m_pTables[0]Ϊ��,����ֱ����ID������
	m_nCount = Indextable.GetMaxExcelID() + 1;
	m_pTables = new CExcelTable*[m_nCount];
	memset(m_pTables,0,sizeof(CExcelTable*)*m_nCount);
	
	SExcelLine* pIndexLine;
	int type;

	for ( int i=1; i<m_nCount; i++ )
	{
		pIndexLine = Indextable.GetLine(i);
		if ( pIndexLine )
		{
			if ( pIndexLine->GetValue(5,type) == RE_OK )
			{
				if ( iType == 0 || type == 0 || iType == type )
				{// �����������еļ�¼������ʹ�����Ͳ�Load
					CExcelTable * pTable = new CExcelTable;

					if ( pTable && pTable->Load( pIndexLine, iType ) )
					{
						m_pTables[i] = pTable;
					}
					else
					{
						char szInfo[10240];
						sprintf( szInfo, GLGR_STD_HDR(47,ERR_EXCEL)"LoadTables,,,%s,%d,%s"
							, szDir, iType, pIndexLine->FastGetStr(2,false) );
						OutputErrorLog( szInfo );

						MyDeletePoint(pTable);
						Release();
						return false;
					}
				}
			}
		}
	}
	return true;
}

bool CExcelTableMng::ReloadTable( int iTableID )
{
	if ( m_pTables && ISIN_OO(0,iTableID,m_nCount) &&
		 m_pTables[iTableID] )
	{
		if ( m_pTables[iTableID]->Reload() )
		{
			return true;
		}
		else
		{
			char szInfo[10240];//��ȡʧ��
			sprintf( szInfo, GLGR_STD_HDR(48,ERR_EXCEL)"ReloadTable Failed,,,%d"
				, iTableID );
			OutputErrorLog( szInfo );
		}
	}
	else
	{//û����ر��
		char szInfo[10240];
		sprintf( szInfo, GLGR_STD_HDR(48,ERR_EXCEL)"ReloadTable NULL,,,%d"
			, iTableID );
		OutputErrorLog( szInfo );
	}
	return false;
}

//���ݱ��˳��ŷ���һ�ű�
CExcelTable* CExcelTableMng::SafeGetTable( int iTableID )
{
	if ( m_pTables && ISIN_OO(0,iTableID,m_nCount) && 
		 m_pTables[iTableID] )
	{
		return m_pTables[iTableID];
	}
	else
	{
		std::string buf;
		G_MiscGetStackTrace(buf);
		char szInfo[10240];
		sprintf( szInfo,GLGR_STD_HDR(45,ERR_EXCEL)"Excel_Table,,,%d,%s", iTableID,buf.c_str() );
		OutputErrorLog( szInfo );
	}
	return NULL;
}

CExcelTable* CExcelTableMng::GetTable( int iTableID )
{
	if ( m_pTables && ISIN_OO(0,iTableID,m_nCount) )
	{
		return m_pTables[iTableID];
	}
	return NULL;
}

SExcelLine* CExcelTableMng::SafeGetLine( int iTableID,int iExcelID )
{
	CExcelTable * pTable = SafeGetTable(iTableID);
	if ( pTable )
	{
		return pTable->SafeGetLine(iExcelID);
	}
	return NULL;
}

SExcelLine* CExcelTableMng::GetLine( int iTableID,int iExcelID )
{
	CExcelTable * pTable = GetTable(iTableID);
	if ( pTable )
	{
		return pTable->GetLine(iExcelID);
	}
	return NULL;
}

int CExcelTableMng::GetCount()
{
	return m_nCount;
}
//�ͷſռ�
void CExcelTableMng::Release()
{
	if ( m_pTables )
	{
		for (int i=0;i<m_nCount; i++ )
		{
			MyDeletePoint(m_pTables[i]);
		}
		MyDeleteArray(m_pTables);
	}

	MyDeletePoint(m_pIndexTable);
}

//************************************//
//		�����ֶ�ȡ��Excel��������	  //
//************************************//

CExcelTableNameMap::CExcelTableNameMap()
{

}

CExcelTableNameMap::~CExcelTableNameMap()
{
	Release();
}

void CExcelTableNameMap::Release()
{
	map<string,CExcelTable *>::iterator it = m_TableNameMap.begin();
	while ( it != m_TableNameMap.end() )
	{
		delete (*it).second;
		++it;
	}

	m_TableNameMap.clear();
}

bool CExcelTableNameMap::LoadTables( const char * szDir, vector<Byte> &vDefs )
{
	vector<string> vStrs;
	bool booResult = GetDirFileList( GetFileMan(), szDir, &vStrs, ".txt" );
	if( booResult == false )
	{
		char szInfo[10240];
		sprintf( szInfo, GLGR_STD_HDR(47,ERR_EXCEL)"LoadTables,,,%s,%d"
			, szDir, 0 );
		OutputErrorLog( szInfo );
		return false;
	}

	char nakefilename[1024];
	char noextname[256];

	for( int i=0; i<(int)(vStrs.size()); i++ )
	{
		CExcelTable * pTable = new CExcelTable;
		pTable->m_vDefs = vDefs;

		if ( pTable->Load( vStrs[i].c_str(), 0, 1 ) )
		{
			MakeNakedNameIndex( vStrs[i].c_str(), nakefilename, 1024, 0 );
			GetFileNameNoExt( nakefilename, noextname, 256 );

			m_TableNameMap.insert( make_pair( noextname, pTable ) );
		}
		else
		{
			char szInfo[10240];
			sprintf( szInfo, GLGR_STD_HDR(47,ERR_EXCEL)"LoadTables,,,%s,%d"
				, szDir, 0 );
			OutputErrorLog( szInfo );

			MyDeletePoint(pTable);
			Release();
			return false;
		}
	}
	return true;
}

bool CExcelTableNameMap::ReloadTable( const char * szName )
{
	CExcelTable * pTable = GetTable( szName );
	if ( pTable == NULL )
	{
		return false;
	}

	return ( pTable->Reload() );
}

CExcelTable * CExcelTableNameMap::GetTable( const char * szName )
{
	map<string,CExcelTable *>::iterator it = m_TableNameMap.find( szName );
	if ( it != m_TableNameMap.end() )
	{
		return (*it).second;
	}
	return NULL;
}

//************************************//
//              Find Term �ṹ        //
//************************************//
SFindTerm::SFindTerm( int iCol, const char * szData, FINDLOGIC logic
					, bool bIncNull )
{
	if ( ISEMPTY(szData) )
	{
		delete this;
		return;
	}

	m_iPos     = iCol;
	m_szData   = szData;
	m_logic    = logic;
	m_bIncNull = bIncNull;
	m_bDef     = DEF_INT;
}

bool SFindTerm::Satisfy( SExcelLine * pLine )
{
	if ( pLine == NULL )
	{
		return false;
	}
#if EXCEL_USE_NEW_LINE
    Excel_Cell* pCell = pLine->GetCell(m_iPos);
#else
    if ( m_iPos >= pLine->GetCount())
	{// ��Cell
		return m_bIncNull;
	}
    Excel_Cell* pCell = pLine->m_vCells[m_iPos];
#endif

    if(pCell == NULL)
	{// ��Cell
		return m_bIncNull;
	}

	switch ( m_bDef )
	{
	case DEF_INT:
		{
			int iData = atol(m_szData.c_str());
            int val = pCell->theInt;
			switch (m_logic)
			{
			case Greater:
				return (val > iData);
			case Less:
				return (val < iData);
			case Equal:
				return (val == iData);
			case Greater_Equal:
				return (val >= iData);
			case Less_Equal:
				return (val <= iData);
			case Not_Equal:
				return (val != iData);
			default:
				break;
			}
		}
		break;
	case DEF_FLOAT:
		{
			float fData = (float)atof(m_szData.c_str());
            float val = pCell->theFloat;
			switch (m_logic)
			{
			case Greater:
				return (val > fData);
			case Less:
				return (val < fData);
			case Equal:
				return (val == fData);
			case Greater_Equal:
				return (val >= fData);
			case Less_Equal:
				return (val <= fData);
			case Not_Equal:
				return (val != fData);
			default:
				break;
			}
		}
		break;
	case DEF_STR:
	case DEF_VARIABLE:
		{
            const char* val = pCell->theStr;
			switch (m_logic)
			{
			case Equal:
				return ( strcmp(val,m_szData.c_str()) == 0 );
			case Greater_Equal: // ����
				{
					string szTemp = val;
					int iResult = szTemp.find(m_szData.c_str());
					return ( iResult > -1 );
				}
				break;
			case Not_Equal:
				return ( strcmp(val,m_szData.c_str()) == 0 );
			default:
				break;
			}
		}
		break;
	case DEF_INTARRAY:
		{
			int iData = atol(m_szData.c_str());
			int iNum = pCell->GetVecCnt();

			switch (m_logic)
			{
			case Equal:
			case Greater_Equal: // ����
				{
					for ( int i=0; i<iNum; i++ )
					{
						if (pCell->theIntVec[i] == iData)
						{
							return true;
						}
					}
				}
				break;
			case Not_Equal:
				{
					for ( int i=0; i<iNum; i++ )
					{
						if (pCell->theIntVec[i] == iData)
						{
							return false;
						}
					}
					return true;
				}
				break;
			default:
				break;
			}
		}
		break;
	case DEF_FLOATARRAY:
		{
			float fData = (float)atof(m_szData.c_str());
			int iNum = pCell->GetVecCnt();

			switch (m_logic)
			{
			case Equal:
			case Greater_Equal: // ����
				{
					for ( int i=0; i<iNum; i++ )
					{
						if (pCell->theFloatVec[i] == fData)
						{
							return true;
						}
					}
				}
				break;
			case Not_Equal:
				{
					for ( int i=0; i<iNum; i++ )
					{
						if (pCell->theFloatVec[i] == fData)
						{
							return false;
						}
					}
					return true;
				}
				break;
			default:
				break;
			}
		}
		break;
	case DEF_STRARRAY:
	case DEF_VARIABLEARRAY:
		{
			int iNum = pCell->GetVecCnt();

			switch (m_logic)
			{
			case Equal:
			case Greater_Equal: // ����
				{
					for ( int i=0; i<iNum; i++ )
					{
						if (strcmp(pCell->theStrVec[i],m_szData.c_str()) == 0)
						{
							return true;
						}
					}
				}
				break;
			case Not_Equal:
				{
					for ( int i=0; i<iNum; i++ )
					{
						if (strcmp(pCell->theStrVec[i],m_szData.c_str()) == 0)
						{
							return false;
						}
					}
					return true;
				}
				break;
			default:
				break;
			}
		}
		break;
	default:
		break;
	}
	return false;
}

//************************************//
//              ֻ�����ݱ���          //
//************************************//
CExcelTable::CExcelTable()
{
	//m_pLines = NULL;//����˳���
	m_iMaxExcelID = 0;//�������ֵ
	m_iExcelIDCol = -1;//����ֵ�к�
	m_iMaxColID = 0;//�����Ч�к�
	m_iColCount = 0;//����, <= m_iMaxColID, >= DefList.size()
	m_iBeginLine = 0;
	//m_pCompressTables = NULL;//ѹ����
}

CExcelTable::~CExcelTable()
{
	//ReleaseCompressTable();
	RemoveAll();
	m_vDefs.clear();
	m_vCols.clear();
	m_vFileNames.clear();
}

void CExcelTable::RemoveAll()
{
	//MyDeleteArray( m_vpCompressTableUnit );
	//MyDeleteArray(m_pLines);

	for ( int i=0; i<(int)m_vLines.size(); i++ )
	{
		MyDeletePoint(m_vLines[i]);
	}
	m_vLines.clear();
}

//����һ�ű�
bool CExcelTable::Load( SExcelLine * pIndexLine, int iType )
{
	// ͨ��
	pIndexLine->GetArray(6,m_vDefs);
	m_iColCount = MAXCOLUMNS;
	m_vCols.resize(m_iColCount);
	m_iMaxColID = m_iColCount - 1;
		
	if ( iType != 0 )
	{// ��ȡ��ҪLoad��ColumnID,ע�����ݲ�һ���������
		vector<int> vTemp;
		pIndexLine->SafeGetArray(6+iType,vTemp,true);

		if ( (int)vTemp.size() > 0 )
		{// ȥ������� Column λ��,��ҪLoad��Col����1,�����0
			m_iColCount = (int)vTemp.size();

			int i;
			for ( i=0; i<=m_iMaxColID; i++ )
			{
				m_vCols[i] = MAXCOLUMNS;
			}

			int iColID;
	
			// ��ҪѰ�������ЧColID
			m_iMaxColID = 0;
			for ( i=0; i<(int)vTemp.size(); i++ )
			{
				iColID = vTemp[i];
				if ( iColID > m_iMaxColID )
				{
					m_iMaxColID = iColID;
				}
				m_vCols[iColID] = i;
			}
			// ȥ�������Columnλ��,ֻ���ܷŽ������ЧColID
			m_vCols.resize(m_iMaxColID+1);
	
			// ��������m_vDefs
			iColID = 0;
			for ( i=0; i<(int)m_vDefs.size(); i++ )
			{
				if ( m_vCols[i] != MAXCOLUMNS )
				{
					if ( i > iColID )
					{// ����Ҫ��Def��ǰŲ
						m_vDefs[iColID] = m_vDefs[i];
					}
					iColID++;
				}
			}
			m_vDefs.resize(iColID);
		}
		else
		{
			for ( int i=0; i<=m_iMaxColID; i++ )
			{
				m_vCols[i] = i;
			}
		}
	}
	else
	{
		for ( int i=0; i<=m_iMaxColID; i++ )
		{
			m_vCols[i] = i;
		}
	}

	int iExcelIDCol;
	int iExcelIDLine;

	pIndexLine->GetValue(3,iExcelIDCol);
	pIndexLine->GetValue(4,iExcelIDLine);

	int iFileNum = pIndexLine->GetArraySize(2);
	for(int i = 0; i<iFileNum; i++)
	{
		string szFilename;
		pIndexLine->GetValue(2,i,szFilename);

		string szTemp;
		szTemp = g_szDir;
		szTemp += szFilename;
		szTemp += ".txt";

		m_vFileNames.push_back(szTemp);
	}

	return Load(m_vFileNames[0].c_str(),iExcelIDCol,iExcelIDLine);
}

//����һ�ű�
bool CExcelTable::Load( const char * szName,int iExcelIDCol,int iExcelIDLine )
{
	if ( ISEMPTY(szName) )
	{
		return false;
	}
	if(m_vFileNames.size() < 1)
	{
		m_vFileNames.push_back(szName);
	}
	//m_szName = szName;
	m_iExcelIDCol = iExcelIDCol;
	m_iBeginLine = iExcelIDLine;

	return Reload();
}

namespace
{

struct excel_parser
{
    struct text_data
    {
        text_data*  m_next;
        char        m_text[1];
    };
    typedef qs_list<text_data> text_list;

    struct line_data
    {
        line_data*  m_next;
        text_list   m_texts;
    };
    typedef qs_list<line_data> line_list;

	char        m_cache[MAXREADFILEBYTE*2+32];
    nr_heap     m_heap;
    line_list   m_lines;
    line_data*  m_line;
    char*       m_pos;
    char*       m_end;

    excel_parser();
    ~excel_parser();

    bool load(whfile* fp);

    void allocate_text(char* p, char* e);
    void allocate_line();
};

excel_parser::excel_parser()
{
    m_line  = 0;
    m_pos   = 0;
    m_end   = 0;
}

excel_parser::~excel_parser()
{
}

void excel_parser::allocate_text(char* p, char* e)
{
    size_t c = e-p;
    text_data* d = (text_data*)m_heap.allocate(sizeof(text_data)+c);
    memcpy(d->m_text, p, c);
    d->m_next    = 0;
    d->m_text[c] = 0;
    m_line->m_texts.push_back(d);
}

void excel_parser::allocate_line()
{
    if(m_line)
    {
        m_lines.push_back(m_line);
    }
    m_line = (line_data*)m_heap.allocate(sizeof(line_data));
    memset(m_line, 0, sizeof(line_data));
}

bool excel_parser::load(whfile* fp)
{
    allocate_line();

    int readed = 0;
    m_pos   = m_cache;
    m_end   = m_cache;
	while((readed = (int)fp->Read(m_end,MAXREADFILEBYTE)) > 0 )
    {
        m_end += readed;
        *m_end = 0;

        // parse begin

        char* p = m_pos;
        while(p < m_end)
        {
            switch(*p)
            {
            case '\r':
                if(p+1 == m_end)
                {
                    goto lb_parse_next;
                }
                allocate_text(m_pos, p);
                allocate_line();
                if(*(p+1) == '\n')
                {
                    p += 2;
                }
                else
                {
                    ++ p;
                }
                m_pos = p;
                break;
            case '\n':
                allocate_text(m_pos, p);
                allocate_line();
                ++ p;
                m_pos = p;
                break;
            case '\t':
                {
                    allocate_text(m_pos, p);
                    ++ p;
                    m_pos = p;
                }
                break;
            case '"':
                {
                    char* d = ++p;
                    m_pos = d;
                    for(; ; ++ p, ++ d)
                    {
                        if(p >= m_end)
                        {
                            -- m_pos;
                            goto lb_parse_next;
                        }
                        if(*p == '"')
                        {
                            ++ p;
                            if(*p != '"')
                            {
                                break;
                            }
                        }
                    }
                    d = p = m_pos;
                    for(; ; ++ p, ++ d)
                    {
                        if(p >= m_end)
                        {
                            -- m_pos;
                            goto lb_parse_next;
                        }
                        if(*p == '"')
                        {
                            ++ p;
                            if(*p != '"')
                            {
                                break;
                            }
                        }
                        *d = *p;
                    }
                    for(; ; ++ p, ++d)
                    {
                        if(p >= m_end)
                        {
                            -- m_pos;
                            goto lb_parse_next;
                        }
                        switch(*p)
                        {
                        case '\r':
                            if(p+1 == m_end)
                            {
                                goto lb_parse_next;
                            }
                            allocate_text(m_pos, d);
                            allocate_line();
                            if(*(p+1) == '\n')
                            {
                                p += 2;
                            }
                            else
                            {
                                ++ p;
                            }
                            m_pos = p;
                            goto lb_parse_cell_exit;
                        case '\n':
                            allocate_text(m_pos, d);
                            allocate_line();
                            ++ p;
                            m_pos = p;
                            goto lb_parse_cell_exit;
                        case '\t':
                            {
                                allocate_text(m_pos, d);
                                ++ p;
                                m_pos = p;
                            }
                            goto lb_parse_cell_exit;
                        default:
                            *d = *p;
                        }
                    }
                }
                break;
            default:
                {
                    ++ p;
                    for(; p < m_end; ++ p)
                    {
                        switch(*p)
                        {
                        case '\t': case '\n': case '\r':
                            goto lb_parse_cell_exit;
                        }
                    }
                }
                break;
            }
lb_parse_cell_exit:;
        }

lb_parse_next:

        // parse end
        if(m_pos < m_end)
        {
            size_t dis = m_end-m_pos;
            memmove(m_cache, m_pos, dis);
            m_pos = m_cache;
            if(dis > MAXREADFILEBYTE) dis = MAXREADFILEBYTE;
            m_end = m_pos+dis;
        }
        else
        {
            m_pos = m_cache;
            m_end = m_cache;
        }
    }

    if(m_pos < m_end)
    {
        allocate_text(m_pos, m_end);
        m_lines.push_back(m_line);
    }

    return true;
}

long q_atol(const char* p)
{
    bool neg = false;
    for(;;++p)
    {
        switch(*p)
        {
        // skip space
        case '\t': case '\r': case '\n': case ' ':
            continue;
        case '-':
            neg = true;
            ++ p;
            break;
        case '+':
            ++ p;
            break;
        case 0:
            return 0;
        }
        break;
    }
    unsigned long c = 0;
    for(;;++p)
    {
        unsigned long d = (unsigned char)(*p-'0');
        if(d >= 10)
        {
            break;
        }
        c = c*10+d;
    }
    if(neg)
    {
        return -(long)c;
    }
    return (long)c;
}

#if EXCEL_USE_NEW_LINE
int ConstructCell(Excel_Cell* pCell, Byte bDef, const char * szValue);
#endif

} // namespace

bool CExcelTable::LoadOneExcel(const char* szFileName, vector<int> &vExcelIDs)
{
//	if(szFileName == "")
	if(!strcmp(szFileName,""))
	{
		return false;
	}

	//���ļ������ڴ�
	whfile *fp;
	fp	= FOPEN(szFileName, whfileman::OP_MODE_BIN_READONLY);
	if ( !fp )
	{
		return false;
	}

	if ( (int)m_vCols.size() == 0 )
	{
		m_iColCount = MAXCOLUMNS;
		m_vCols.resize(m_iColCount);
		m_iMaxColID = m_iColCount - 1;

		for ( int i=0; i<=m_iMaxColID; i++ )
		{
			m_vCols[i] = i;
		}
	}

	int iExcelID = 0;

    excel_parser ep;
    ep.load(fp);
    fp->Rewind();
    int line_cnt = (int)ep.m_lines.m_size;
   
	int iLineSize = m_vLines.size();
	vExcelIDs.reserve(iLineSize + line_cnt);
	m_vLines.reserve(iLineSize + line_cnt);

    excel_parser::line_data* line_data = ep.m_lines.m_head;
    int row_idx = 0;

	std::vector<unsigned char> idxs;
    std::vector<Excel_Cell> cells;
    idxs.reserve(256);
    cells.reserve(256);

    while(line_data)
    {
        if(row_idx < m_iBeginLine)
        {
            ++ row_idx;
            line_data = line_data->m_next;
            continue;
        }

        excel_parser::text_data* text_data = line_data->m_texts.m_head;

		//line_data->m_texts.m_size <= m_iExcelIDCol �������п�ʼû�����ݵ���Ϊ����,û����index_list��ע��ı��û��������m_iExcelIDCol == -1,����log
        if( (text_data == 0 || line_data->m_texts.m_size <= m_iExcelIDCol) && m_iExcelIDCol != -1)
        {
			char szInfo[1024];
			sprintf( szInfo,GLGR_STD_HDR(49,ERR_EXCEL)"Empty Line,,,%s,%d",
				szFileName,row_idx);
			OutputErrorLog( szInfo );
        }
        else
        {
            SExcelLine* pLine = new SExcelLine();
            idxs.resize(0);
            cells.resize(0);

            // ������Ҫ�������͵�Ԫ�����
            Excel_Cell vCell;
            int col_idx = 0;
            int idx_cnt = 0;
            //excel_parser::text_data* text_data = line_data->m_texts.m_head;
            while(text_data)
            {
                if(col_idx >= MAXCOLUMNS)
                {
                    break;
                }

                if(col_idx == m_iExcelIDCol)
                {
                    iExcelID = atoi(text_data->m_text);
				    if ( iExcelID > m_iMaxExcelID )
				    {
					    m_iMaxExcelID = iExcelID;
				    }
                    vExcelIDs.push_back(iExcelID);
                }

                int idx = ConstructCell(&vCell, GetPosDef(m_vCols[col_idx]), text_data->m_text);
                if(idx >= SP_COLUMN_BASE)
                {
                    if(idx != SP_COLUMN_EMPTY) idx_cnt = col_idx+1;
                    idxs.push_back((unsigned char)idx);
                }
                else
                {
                    idx_cnt = col_idx+1;
                    idxs.push_back((unsigned char)cells.size());
                    cells.push_back(vCell);
                }
                ++ col_idx;
                text_data = text_data->m_next;
            }
            if(idx_cnt)
            {   // ��ʼ�����ݡ�
                int cell_cnt = (int)cells.size();
                int idx_mem = (idx_cnt+3)&(~3);
                int mem     = idx_mem+cell_cnt*sizeof(Excel_Cell);
                unsigned char* pCellIndex = (unsigned char*)::Excel_LineData_Alloc(mem) + idx_mem;
				if (idx_cnt > 0)
				{
					 memcpy(pCellIndex-idx_cnt, &idxs[0], idx_cnt);
				}
				if (cell_cnt > 0)
				{
					memcpy(pCellIndex, &cells[0], cell_cnt*sizeof(Excel_Cell));
				}
                pLine->m_pCellIndex = pCellIndex;
                pLine->m_nSize = (unsigned char)idx_cnt;
                pLine->m_nUsed = (unsigned char)cell_cnt;
            }
    							
		    //Ϊÿ�����ø���
		    pLine->SetParentTable(this);
		    //����ÿ�е��к�
		    //��Ҫȥ����ʼ��  2006-2-16 solo����
		    pLine->SetLineID((unsigned short)row_idx - m_iBeginLine + iLineSize );
		    m_vLines.push_back(pLine);
        }
        ++ row_idx;
        line_data = line_data->m_next;
    }
			

	WHSafeSelfDestroy(fp);
	return true;
}
bool CExcelTable::Reload()
{
	RemoveAll();

	vector<int> vExcelIDs;

	int iFileNum = m_vFileNames.size();

	for(int i = 0; i<iFileNum; i++)
	{			
		LoadOneExcel(m_vFileNames[i].c_str(), vExcelIDs);
	}

	//==============================================================
	if( m_iExcelIDCol != -1 && vExcelIDs.size() > 0)
	{
        m_index.Create(-1, m_iMaxExcelID, &vExcelIDs[0], (int)vExcelIDs.size(), m_vFileNames[0]);
	}

	
	return true;
}

//�������Ų���һ��
SExcelLine * CExcelTable::SafeGetLine( int iExcelID, bool booCanBeNull )
{
	SExcelLine * pLine = GetLine(iExcelID);
	if( pLine == NULL && !booCanBeNull )
	{
		std::string buf;
		G_MiscGetStackTrace(buf);
		char szInfo[10240];
		sprintf( szInfo,GLGR_STD_HDR(46,ERR_EXCEL)"Excel_Line,,,%s,%d,%s", m_vFileNames[0].c_str(),iExcelID,buf.c_str() );
		OutputErrorLog( szInfo );
	}
	return pLine;
}

SExcelLine * CExcelTable::GetLine( int iExcelID )
{
    int idx = m_index.Find(iExcelID);
    if(idx >= 0)
    {
        return m_vLines[idx];
    }
	return NULL;
}

/*
//�����кŻ��ѹ�����е�һ��
CIDCompressTable	* CExcelTable::GetCompressTableUnitList( int i )
{ 
	if( ((i-m_iBeginLine) >=0) && ((i-m_iBeginLine) < ((int)m_vTables.size()+1)) && m_vpCompressTableUnit )
	{
		return &(m_vpCompressTableUnit[i-m_iBeginLine]); 
	}
	return NULL;
}
//����ѹ����iBegin����ʼ��
void CExcelTable::BuildCompressTable( int iBegin )
{
	m_vpCompressTableUnit = new CIDCompressTable[m_vTables.size() + 1];
	for( int i = 0;i<(int)m_vTables.size();i++ )
	{
		m_vpCompressTableUnit[i].Build( m_vTables[i].m_vCells.size() );
		for( int j=iBegin;j<(int)m_vTables[i].m_vCells.size();j++ )
		{
			char *pstr;
			int iint;
			string sz;
			//��������ѹ������
			if(j>(int)m_vDefs.size()-1 || m_vDefs[j] == DEF_INT)
			{
				sz = "";
				char SS[32];
				sprintf(SS,"%d",m_vTables[i].m_vCells[j].theInt);
				sz.append(SS);
				pstr = (char*)sz.c_str();
				iint = m_vTables[i].m_vCells[j].theInt;
			}
			else if (m_vDefs[j] == DEF_STR)
			{
				pstr = (char*)m_vTables[i].m_vCells[j].theStr;
				iint = 0;
			}
			else if (m_vDefs[j] == DEF_FLOAT)
			{
				sz = "";
				char SS[32];
				sprintf(SS,"%f",m_vTables[i].m_vCells[j].theFloat);
				sz.append(SS);
				pstr = (char*)sz.c_str();
				iint = 0;
			}
			else if (m_vDefs[j] == DEF_INTARRAY)
			{
				sz = "";
				char SS[32];
				int x = m_vTables[i].m_vCells[j].GetVecCnt();
				for(int n=0;n<(x-1);n++)
				{
					memset(SS,0,32);
					sprintf(SS,"%d",m_vTables[i].m_vCells[j].theIntVec[n]);
					sz.append(SS);
					sz.append("*");
				}
				if(n != 0)
				{
					memset(SS,0,32);
					sprintf(SS,"%d",m_vTables[i].m_vCells[j].theIntVec[n]);
					sz.append(SS);
				}
				else
					sz = "";
				pstr = (char*)sz.c_str();
						
				iint = 0;
			}
			else if (m_vDefs[j] == DEF_STRARRAY)
			{
				sz = "";
				int x = m_vTables[i].m_vCells[j].GetVecCnt();
				for(int n=0;n<(x-1);n++)
				{
					sz.append(m_vTables[i].m_vCells[j].theStrVec[n]);
					sz.append("*");
				}
				if(n != 0)
				{
					sz.append(m_vTables[i].m_vCells[j].theStrVec[n]);
				}
				else
					sz = "";
				pstr = (char*)sz.c_str();

				iint = 0;
			}
			else if (m_vDefs[j] == DEF_FLOATARRAY)
			{
				sz = "";
				char SS[32];
				int x = m_vTables[i].m_vCells[j].GetVecCnt();
				for(int n=0;n<(x-1);n++)
				{
					memset(SS,0,32);
					sprintf(SS,"%f",m_vTables[i].m_vCells[j].theFloatVec[n]);
					sz.append(SS);
					sz.append("*");
				}
				if(n != 0)
				{
					memset(SS,0,32);
					sprintf(SS,"%f",m_vTables[i].m_vCells[j].theFloatVec[n]);
					sz.append(SS);
				}
				else
					sz = "";
				pstr = (char*)sz.c_str();
				
				iint = 0;
			}
			m_vpCompressTableUnit[i].AddInto( pstr,iint );
		}
	}
}

//�ͷ�ѹ����
void   CExcelTable::ReleaseCompressTable()
{
	MyDeleteArray(m_vpCompressTableUnit);
}
*/
//���кŲ���һ��
SExcelLine * CExcelTable::GetLineFromList( int iLineID )//i�Ǵ�0��ʼ��
{
	if ( ISIN_CO(0,iLineID,(int)m_vLines.size()) )
	{
		return m_vLines[iLineID];
	}
	return NULL;
}

//������������ֵ
int	CExcelTable::GetMaxExcelID()
{
	return m_iMaxExcelID;
}

int CExcelTable::GetColPos( int iCol )
{
	if ( ISIN_CO(0,iCol,m_iMaxColID) )
	{
		return m_vCols[iCol];
	}
	return MAXCOLUMNS;
}

Byte CExcelTable::GetPosDef( int iPos )
{
	int iSize = (int)m_vDefs.size();

	if ( iSize > iPos )
	{
		return m_vDefs[iPos];
	}
	else if ( iSize > 0 )
	{
		return m_vDefs[iSize-1];
	}
	else
	{
		return DEF_VARIABLE;
	}
}

int CExcelTable::CheckPosDef( int iPos, Byte bDef )
{
	Byte bColDef = GetPosDef( iPos );

	if ( bDef == DEF_VARIABLE )
	{// ��Ч
		return RE_DEFERROR;
	}
	else if ( bDef == DEF_VARIABLEARRAY )
	{//Ҫ����һ��ARRAY
		if ( DEF_ISARRAY(bColDef) )
		{
			return RE_OK;
		}
		else
		{
			return RE_DEFERROR;
		}
	}
	else if ( bColDef == bDef )
	{
		return RE_OK;
	}
	else if ( bColDef == DEF_VARIABLE && !DEF_ISARRAY(bDef) )
	{// ��Ҫ����ת��
		return RE_DEFVAR;
	}
	else if ( bColDef == DEF_VARIABLEARRAY && DEF_ISARRAY(bDef) )
	{// ��Ҫ����ת��
		return RE_DEFVAR;
	}
	return RE_DEFERROR;
}

int CExcelTable::SafeGetInt( int iExcelID,int iCol,bool booCanBeNull )
{
	SExcelLine * pLine = SafeGetLine(iExcelID,booCanBeNull); // �� log
	if ( pLine )
	{
		return pLine->SafeGetInt(iCol,booCanBeNull); // �� log
	}
	return 0;
}

const char * CExcelTable::FastGetStr( int iExcelID,int iCol,bool booCanBeNull )
{
	SExcelLine * pLine = SafeGetLine(iExcelID,booCanBeNull);
	if( pLine )
	{
		return pLine->FastGetStr(iCol,booCanBeNull);
	}
	return g_szNULL;
}

void CExcelTable::SafeGetStr( int iExcelID,int iCol,string &Output,bool booCanBeNull )
{
	SExcelLine * pLine = SafeGetLine(iExcelID,booCanBeNull); // �� log
	if( pLine )
	{
		pLine->SafeGetStr(iCol,Output,booCanBeNull); // �� log
	}
	else
	{
		Output = "";
	}
}

float CExcelTable::SafeGetFloat( int iExcelID,int iCol,bool booCanBeNull )
{
	SExcelLine * pLine = SafeGetLine(iExcelID,booCanBeNull); // �� log
	if ( pLine )
	{
		return pLine->SafeGetFloat(iCol,booCanBeNull); // �� log
	}
	return 0.0;
}

void CExcelTable::SafeGetArray( int iExcelID,int iCol,vector<int> &Output,bool booCanBeNull )
{
	SExcelLine * pLine = SafeGetLine(iExcelID,booCanBeNull); // �� log
	if ( pLine )
	{
		pLine->SafeGetArray(iCol,Output,booCanBeNull); // �� log
	}
}

void CExcelTable::SafeGetArray( int iExcelID,int iCol,vector<const char *> &Output,bool booCanBeNull )
{
	SExcelLine * pLine = SafeGetLine(iExcelID,booCanBeNull); // �� log
	if ( pLine )
	{
		pLine->SafeGetArray(iCol,Output,booCanBeNull); // �� log
	}
}

void CExcelTable::SafeGetArray( int iExcelID,int iCol,vector<float> &Output,bool booCanBeNull )
{
	SExcelLine * pLine = SafeGetLine(iExcelID,booCanBeNull); // �� log
	if ( pLine )
	{
		pLine->SafeGetArray(iCol,Output,booCanBeNull); // �� log
	}
}

SExcelLine * CExcelTable::FindLine( int iCol,int iValue )
{
	int iPos = GetColPos(iCol);

	if ( iPos != MAXCOLUMNS )
	{
		int iDefRe = CheckPosDef(iPos,DEF_INT);

		switch ( iDefRe )
		{
		case RE_DEFVAR:
			{
				int iData;
				for ( int i=0; i<(int)m_vLines.size(); i++ )
				{
					if ( m_vLines[i] && 
						 m_vLines[i]->DirectGetVariable(iPos,iData) == RE_OK &&
						 iData == iValue )
					{
						return m_vLines[i];
					}
				}
			}
			break;
		case RE_OK:
			{
				for ( int i=0; i<(int)m_vLines.size(); i++ )
				{
					if ( m_vLines[i] && 
						 m_vLines[i]->DirectIsValue(iPos,iValue) )
					{
						return m_vLines[i];
					}
				}
			}
			break;
		default:
			break;
		}
	}
	return NULL;
}

SExcelLine * CExcelTable::FindLine( int iCol,const char * szValue )
{
	if ( ISEMPTY(szValue) )
	{
		return NULL;
	}

	int iPos = GetColPos(iCol);

	if ( iPos != MAXCOLUMNS && 
		 CheckPosDef(iPos,DEF_STR) != RE_DEFERROR )
	{
		for ( int i=0; i<(int)m_vLines.size(); i++ )
		{
			if ( m_vLines[i] && 
				 m_vLines[i]->DirectIsValue(iPos,szValue) )
			{
				return m_vLines[i];
			}
		}
	}
	return NULL;
}

SExcelLine * CExcelTable::FindLine( int iCol,float fValue )
{
	int iPos = GetColPos(iCol);

	if ( iPos != MAXCOLUMNS )
	{
		int iDefRe = CheckPosDef(iPos,DEF_FLOAT);

		switch ( iDefRe )
		{
		case RE_DEFVAR:
			{
				float fData;
				for ( int i=0; i<(int)m_vLines.size(); i++ )
				{
					if ( m_vLines[i] && 
						 m_vLines[i]->DirectGetVariable(iPos,fData) == RE_OK &&
						 fData == fValue )
					{
						return m_vLines[i];
					}
				}
			}
			break;
		case RE_OK:
			{
				for ( int i=0; i<(int)m_vLines.size(); i++ )
				{
					if ( m_vLines[i] &&
						 m_vLines[i]->DirectIsValue(iPos,fValue) )
					{
						return m_vLines[i];
					}
				}
			}
			break;
		default:
			break;
		}
	}
	return NULL;
}

bool CExcelTable::TestFindTerm( vector<SFindTerm *> &Terms )
{
	for ( int i=0; i<(int)Terms.size(); i++ )
	{
		if ( Terms[i] == NULL )
		{
			return false;
		}

		int iPos = GetColPos(Terms[i]->m_iPos);
		if ( iPos == MAXCOLUMNS )
		{
			return false;
		}
		else
		{
			Byte bDef = GetPosDef(iPos);
			if ( bDef == DEF_INT || 
				 bDef == DEF_FLOAT ||
				( ( bDef == DEF_STR || 
				    bDef > DEF_FLOAT ) &&
				  ( Terms[i]->m_logic == Equal || 
				    Terms[i]->m_logic == Greater_Equal || 
					Terms[i]->m_logic == Not_Equal ) ) )
			{
				Terms[i]->m_iPos = iPos;
				Terms[i]->m_bDef = bDef;
			}
			else
			{//�޷��Ƚϵ�������
				return false;
			}
		}
	}
	return true;
}

void CExcelTable::FindLineGroup( vector<SFindTerm *> &Terms,list<SExcelLine *> &Output )
{
	Output.clear();

	// ��������Ƿ񶼺Ϸ�
	if ( TestFindTerm( Terms ) )
	{// �ڱ����������������������
		for ( int i=0; i<(int)m_vLines.size(); i++ )
		{
			int t;
			for ( t=0; t<(int)Terms.size(); t++ )
			{
				if ( Terms[t]->Satisfy(m_vLines[i]) == false )
				{// ��һ�������㣬�����Ը��еļ��
					break;
				}
			}
			if ( t == int(Terms.size()) )
			{// ����ȫ������
				Output.push_back(m_vLines[i]);
			}
		}
	}

	// �������
	for ( int i=0; i<(int)Terms.size(); i++ )
	{
		if ( Terms[i] )
		{
			delete( Terms[i] );
		}
	}
	Terms.clear();
}

void CExcelTable::FindLineGroupContinue( vector<SFindTerm *> &Terms,list<SExcelLine *> &Output )
{
	// ��������Ƿ񶼺Ϸ�
	if ( TestFindTerm( Terms ) )
	{// ���ϴεĽ����������������������
		list<SExcelLine*>::iterator itBeg = Output.begin();
		list<SExcelLine*>::iterator itTmp;

		for ( ; itBeg!=Output.end(); )
		{
			int t;
			for ( t=0; t<(int)Terms.size(); t++ )
			{
				if ( Terms[t]->Satisfy( (*itBeg) ) == false )
				{// ��һ�������㣬�����Ը��еļ��
					break;
				}
			}
			if ( t == int(Terms.size()) )
			{// ����ȫ������
				itBeg++;
			}
			else
			{
				itTmp = itBeg++;
				Output.erase( itTmp );
			}
		}
	}

	// �������
	for ( int i=0; i<(int)Terms.size(); i++ )
	{
		if ( Terms[i] )
		{
			delete( Terms[i] );
		}
	}
	Terms.clear();
}

bool CExcelTable::AddLine( int iExcelID )
{
    if(m_iExcelIDCol == -1)
    {
        // �������У����ء�
        return false;
    }

    // �ҵ����ʵĲ���λ��
	int iLineID  = 0;
    int iLineCnt = (int)m_vLines.size();
	for (; iLineID < iLineCnt; ++ iLineID)
	{
        SExcelLine* pLine = m_vLines[iLineID];
        int nID = pLine->SafeGetInt(m_iExcelIDCol, true);

        if(nID >= iExcelID)
        {
            // �Ѿ�ʹ���˵�����ֵ
            if(nID == iExcelID) return false;

            // nID > iExcelID����ǰiLineID���ǲ����λ��
            break;
        }
	}

    // �����µ���
    SExcelLine * pLine = new SExcelLine;
	if(pLine == NULL) return false;

	//Ϊÿ�����ø���
	pLine->SetParentTable(this);

    // ��ʼ����ֵ
	char szValue[64];
	sprintf(szValue, "%d", iExcelID);
	if(!pLine->SetCell(m_iExcelIDCol, szValue))
	{
		delete pLine;
        return false;
	}

    if(!m_index.Insert(iExcelID, iLineID))
    {
        delete pLine;
        return false;
    }

    m_vLines.insert(m_vLines.begin()+iLineID, pLine);
    iLineCnt = (int)m_vLines.size();
    for(; iLineID < iLineCnt; ++ iLineID)
    {
		m_vLines[iLineID]->SetLineID(iLineID);
	}
	return true;
}

bool CExcelTable::SetLine(int iExcelID, SExcelLine* pLine)
{
    int iLineID = m_index.Find(iExcelID);
    if(iLineID < 0) return false;

    SExcelLine* pOld = m_vLines[iLineID];
    if(pOld)
    {   // ����㷨�Ǵ���ǰ�Ĵ����޸����ġ�
        // �о���ʹpOld��NULL��ҲӦ��ִ������Ĵ���Ŷԡ�
	    pLine->SetParentTable(this);
	    pLine->SetLineID(iLineID);
	    m_vLines[iLineID] = pLine;
	    MyDeletePoint(pOld);
        return false;
    }

    return false;
}

bool CExcelTable::SetLine( int iExcelID,int iCol,const char * szValue )
{
	SExcelLine * pLine = GetLine(iExcelID);

	// ��ϵ������˳�����˲�����ı�����ֵ
	// ֻ�� AddLine ������������ֵ
	return ( pLine && 
			 iCol != m_iExcelIDCol &&
		     pLine->SetCell( iCol, szValue ) );
}

void CExcelTable::RemLine(int iExcelID)
{
	SExcelLine * pLine = GetLine(iExcelID);
    if(pLine == NULL) return;

    m_index.Remove(iExcelID);

    // ��Ϊû�д�m_vLines��ɾ��Ԫ�أ����Բ���Ҫ��С������
	int iLineID = pLine->GetLineID();
	if ( ISIN_CO(0,iLineID,(int)m_vLines.size()) )
	{
		m_vLines[iLineID] = NULL;
	}
	MyDeletePoint(pLine);
}

void CExcelTable::GetColNames( vector<string> &vStrs )
{
	vStrs.clear();

	if ( m_iBeginLine < 1 )
	{
		return;
	}

	whfile * fp = FOPEN(m_vFileNames[0].c_str(), whfileman::OP_MODE_BIN_READONLY);
	if ( !fp )
	{
		return;
	}

	char szPool[MAXREADFILEBYTE*2];
	char szTry[MAXREADFILEBYTE];

	int iRemainLen = 0;
	int iTryLen = 0;

	int iStrBeg = 0;
	char SS[MAXSUBBYTE];

	//ÿ�δ��ļ�����8192���ֽ����ۺ϶�������
	while ( (iTryLen = (int)fp->Read(szTry, MAXREADFILEBYTE)) > 0 )
	{
		if ( iRemainLen > 0 )
		{//�����ζ���ļ����ϴζ��뵫ûʹ�����
			memcpy(szPool+iRemainLen,szTry,iTryLen);
			iTryLen += iRemainLen;
		}
		else
		{
			memcpy(szPool,szTry,iTryLen);
		}

		for ( int iCurCha=0; iCurCha<iTryLen; iCurCha++ )
		{
			if ( szPool[iCurCha] == '\n' || szPool[iCurCha] == '\r' )
			{//ÿ�еĽ�β�п�����'\n'���ֿ�����'\r''\n'
				//����һ������
				memcpy(SS,szPool+iStrBeg,iCurCha-iStrBeg);
				SS[iCurCha-iStrBeg]	= 0;
				vStrs.push_back(SS);

				WHSafeSelfDestroy(fp);
				return;
			}
			else
			{// ��ת�з���
				if ( szPool[iCurCha] == '\t' )
				{
					//����һ������
					memcpy(SS,szPool+iStrBeg,iCurCha-iStrBeg);
					SS[iCurCha-iStrBeg]	= 0;
					vStrs.push_back(SS);
					iStrBeg = iCurCha+1;
				}
			}
		}
		iRemainLen = iTryLen-iStrBeg;
		if ( iRemainLen > 0 )
		{//����������ݰ��н�ȡ�������ʣ��,��ʣ�ಿ����ǰŲ
			memcpy(szPool,szPool+iStrBeg,iRemainLen);
		}
		else
		{
			iRemainLen = 0;
		}
		iStrBeg = 0;
	}

	WHSafeSelfDestroy(fp);
}

void CExcelTable::GetBeginLines( vector<string> &vStrs )
{
	vStrs.clear();

	if ( m_iBeginLine > 0 )
	{
		whfile	*fp;
		fp		= FOPEN(m_vFileNames[0].c_str(), whfileman::OP_MODE_BIN_READONLY);
		if ( !fp )
		{
			return;
		}

		int iCurLine = 0;

		char szPool[MAXREADFILEBYTE*2];
		char szTry[MAXREADFILEBYTE];

		int iRemainLen = 0;
		int iTryLen = 0;

		bool bEOL = false;

		int iStrBeg = 0;
		char SS[MAXSUBBYTE];

		//ÿ�δ��ļ�����8192���ֽ����ۺ϶�������
		while ( (iTryLen = (int)fp->Read(szTry, MAXREADFILEBYTE)) > 0 )
		{
			if ( iRemainLen > 0 )
			{//�����ζ���ļ����ϴζ��뵫ûʹ�����
				memcpy(szPool+iRemainLen,szTry,iTryLen);
				iTryLen += iRemainLen;
			}
			else
			{
				memcpy(szPool,szTry,iTryLen);
			}

			for ( int iCurCha=0; iCurCha<iTryLen; iCurCha++ )
			{
				if ( szPool[iCurCha] == '\n' || szPool[iCurCha] == '\r' )
				{//ÿ�еĽ�β�п�����'\n'���ֿ�����'\r''\n'
					if ( !bEOL )
					{//ֻ�����һ��ת�оͿ�����
						if ( iCurLine < m_iBeginLine )
						{
							memcpy(SS,szPool+iStrBeg,iCurCha-iStrBeg);
							SS[iCurCha-iStrBeg] = 0;
							vStrs.push_back(SS);
							bEOL = true;
							iCurLine++; // ����һ��
						}
						else
						{
							WHSafeSelfDestroy(fp);
							return;
						}
					}// End of �Ƿ��ǵ�һ�δ���ת��
					iStrBeg = iCurCha+1;
				}
				else
				{// ��ת�з���
					if ( bEOL )
					{//��һ�п�ʼ��
						bEOL = false;
					}
				}
			}
			iRemainLen = iTryLen-iStrBeg;
			if ( iRemainLen > 0 )
			{//����������ݰ��н�ȡ�������ʣ��,��ʣ�ಿ����ǰŲ
				memcpy(szPool,szPool+iStrBeg,iRemainLen);
			}
			else
			{
				iRemainLen = 0;
			}
			iStrBeg = 0;
		}

		WHSafeSelfDestroy(fp);
	}
}

void CExcelTable::Save( const char * szName, bool booHeader )
{
	// ����̧ͷ����
    vector<string> vStrs;
	
	if ( booHeader )
	{
		GetBeginLines(vStrs);
	}

	whfile	*fp;

	if ( ISEMPTY(szName) )
	{
		fp = FOPEN(m_vFileNames[0].c_str(), whfileman::OP_MODE_BIN_CREATE);
	}
	else
	{
		fp = FOPEN(szName, whfileman::OP_MODE_BIN_CREATE);
	}
	
	if ( !fp )
	{
		return;
	}

	int i;
	for ( i=0; i<(int)vStrs.size(); i++ )
	{// ��д̧ͷ����
		fp->Write( vStrs[i].c_str(), vStrs[i].size() );
		fp->Write("\r\n", 2);
	}
	
	for ( ; i<m_iBeginLine; i++ )
	{// ����հ���
		fp->Write("\r\n", 2);
	}

	int iPos;
	int iSize;

	string szValue;

	for ( i=0; i<(int)m_vLines.size(); i++ )
	{
		if ( m_vLines[i] )
		{// ���������ݣ���������ɾ���У�����
			iSize = m_vLines[i]->GetCount();
	
			if ( iSize > 0 )
			{
				int j;
				for ( j=0; j<(int)m_vCols.size(); j++ )
				{// Ϊû�б��������в���հ�
					if ( m_vCols[j] == 0 )
					{
						break;
					}
					fp->Write("\t", 1);
				}
                    
				for ( ; j<(int)m_vCols.size(); j++ )
				{
					iPos = m_vCols[j];

					if ( iPos != MAXCOLUMNS )
					{
						m_vLines[i]->DirectGetString( iPos,GetPosDef(iPos),szValue );
						fp->Write(szValue.c_str(), szValue.size());

						if ( iPos == (iSize-1) )
						{// ���һ�����治�ü� \t ��
							break;
						}
					}
					fp->Write("\t", 1);
				}
			}
			fp->Write("\r\n", 2);
		}
	}

	WHSafeSelfDestroy(fp);
}

//************************************//
//              Excel���нṹ         //
//************************************//
#if EXCEL_USE_NEW_LINE
#   include "excel_line_impl.h"
#else
SExcelLine::SExcelLine()
{
}

SExcelLine::~SExcelLine()
{
	for ( int i=0; i<(int)m_vCells.size(); i++ )
	{
		MyDeletePoint(m_vCells[i]);
	}
	m_vCells.clear();
}

Excel_Cell * SExcelLine::CreateCell( Byte bDef,const char * szValue )
{
	if ( ISEMPTY(szValue) )
	{//�� Cell
		return NULL;
	}

	switch ( bDef )
	{
	case DEF_INT:
		{
			long lValue = atol(szValue);
			return ( new Excel_Cell(lValue) );
		}
		break;
	case DEF_STR:
	case DEF_VARIABLE:
		{
			return ( new Excel_Cell(szValue) );
		}	
		break;
	case DEF_FLOAT:
		{
			float fValue = (float)atof(szValue);
			return ( new Excel_Cell(fValue) );
		}
		break;
	case DEF_INTARRAY:
		{
			vector<long> v;
			GetFromString(szValue,v);
			return ( new Excel_Cell(&(v[0]),(int)v.size()) );
		}
		break;
	case DEF_STRARRAY://@@
	case DEF_VARIABLEARRAY:
		{
			vector<const char *> v;
			GetFromString(szValue,v);
			return ( new Excel_Cell(&(v[0]),(int)v.size()) );
		}
		break;
	case DEF_FLOATARRAY:
		{
			vector<float> v;
			GetFromString(szValue,v);
			return ( new Excel_Cell(&(v[0]),(int)v.size()) );
		}	
		break;
	default:
		break;
	}
	return NULL;
}


//��Ԫ��ֵ����
void SExcelLine::AddCell( Byte bDef, const char * szValue )
{
	Excel_Cell * pCell = CreateCell( bDef, szValue );

	m_vCells.push_back(pCell);
}

bool SExcelLine::SetCell( int iCol, const char * szValue )
{
	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return false;
	}

	if ( iPos < (int)m_vCells.size() )
	{// ɾ��ԭCell
		MyDeletePoint(m_vCells[iPos]);
	}
	else
	{
		int iMaxPos = (int)m_vCells.size() - 1;
		while ( iPos > iMaxPos )
		{//�����Cell
			m_vCells.push_back(NULL);
			iMaxPos++;
		}
	}

	m_vCells[iPos] = CreateCell( m_ParentTable->GetPosDef(iPos), szValue );

	return true;
}

bool SExcelLine::DirectIsValue( int iPos,int iValue )
{
	if ( ISIN_CO(0,iPos,(int)m_vCells.size()) && 
		 m_vCells[iPos] )
	{
		return ( m_vCells[iPos]->theInt == iValue );
	}
	return false;
}

bool SExcelLine::DirectIsValue( int iPos,const char * szValue )
{
	if ( !ISEMPTY(szValue) &&
		 ISIN_CO(0,iPos,(int)m_vCells.size()) && 
		 m_vCells[iPos] )
	{
		return ( strcmp(m_vCells[iPos]->theStr,szValue) == 0 );
	}
	return false;
}

bool SExcelLine::DirectIsValue( int iPos,float fValue )
{
	if ( ISIN_CO(0,iPos,(int)m_vCells.size()) && 
		 m_vCells[iPos] )
	{
		return ( m_vCells[iPos]->theFloat == fValue );
	}
	return false;
}

int SExcelLine::DirectGetVariable( int iPos,int &iValue )
{
	if ( ISIN_CO(0,iPos,(int)m_vCells.size()) && 
		 m_vCells[iPos] )
	{
		iValue = atoi(m_vCells[iPos]->theStr);
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
	if ( ISIN_CO(0,iPos,(int)m_vCells.size()) && 
		 m_vCells[iPos] )
	{
		fValue = (float)atof(m_vCells[iPos]->theStr);
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
	if ( ISIN_CO(0,iPos,(int)m_vCells.size()) )
	{
		GetCellStr( m_vCells[iPos], bDef, szValue );
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		if ( iDefRe == RE_OK )
		{
			iValue = m_vCells[iPos]->theInt;
		}
		else
		{
			iValue = atoi(m_vCells[iPos]->theStr);
		}
        return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
}

int SExcelLine::GetValue( int iCol,string &szValue )
{
	szValue = "";

	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return RE_NOCOLUM;
	}

	// �κ����͵����ݶ����� get �� string
	DirectGetString( iPos, m_ParentTable->GetPosDef(iPos), szValue );

	if ( szValue == "" )
	{
		return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		if ( iDefRe == RE_OK )
		{
			fValue = m_vCells[iPos]->theFloat;
		}
		else
		{
			fValue = (float)atof(m_vCells[iPos]->theStr);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();

		if ( iDefRe == RE_OK )
		{
			long * IntVec = m_vCells[iPos]->theIntVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( (Byte)IntVec[i] );
			}
		}
		else
		{
			const char** StrVec = m_vCells[iPos]->theStrVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( (Byte)atoi(StrVec[i]) );
			}
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();

		if ( iDefRe == RE_OK )
		{
			long * IntVec = m_vCells[iPos]->theIntVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( IntVec[i] );
			}
		}
		else
		{
			const char** StrVec = m_vCells[iPos]->theStrVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( atoi(StrVec[i]) );
			}
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();
		const char** StrVec = m_vCells[iPos]->theStrVec;

		for ( int i=0; i<iLen; i++ )
		{
			Output.push_back(StrVec[i]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();

		if ( iDefRe == RE_OK )
		{
			float* FloatVec = m_vCells[iPos]->theFloatVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( FloatVec[i] );
			}
		}
		else
		{
			const char** StrVec = m_vCells[iPos]->theStrVec;
			for ( int i=0; i<iLen; i++ )
			{
				Output.push_back( (float)atof(StrVec[i]) );
			}
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] &&
		 ISIN_CO(0,idx,(int)m_vCells[iPos]->GetVecCnt()) )
	{
		if ( iDefRe == RE_OK )
		{
			iValue = m_vCells[iPos]->theIntVec[idx];
		}
		else
		{
			iValue = atoi(m_vCells[iPos]->theStrVec[idx]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] &&
		 ISIN_CO(0,idx,(int)m_vCells[iPos]->GetVecCnt()) )
	{
		szValue = m_vCells[iPos]->theStrVec[idx];
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] &&
		 ISIN_CO(0,idx,(int)m_vCells[iPos]->GetVecCnt()) )
	{
		if ( iDefRe == RE_OK )
		{
			fValue = m_vCells[iPos]->theFloatVec[idx];
		}
		else
		{
			fValue = (float)atof(m_vCells[iPos]->theStrVec[idx]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
}

void SExcelLine::ErrorLogLineCol( const char * szError,int iCol,int iRE,int idx )
{
	char szInfo[10240];

	int iExcelID = -1;
	if( m_ParentTable->GetExcelIDCol() != -1
		&& m_ParentTable->GetExcelIDCol() != iCol )
	{
		iExcelID = SafeGetInt( m_ParentTable->GetExcelIDCol() );
	}


	sprintf( szInfo,GLGR_STD_HDR(44,ERR_EXCEL)"%s,%d,,%s,%d,%d,%d,%d",
		szError,
		iRE,
		m_ParentTable->GetName(), 
		m_iLineID
		, iCol,idx,iExcelID  );


	OutputErrorLog( szInfo );
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		return m_vCells[iPos]->theStr;
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] &&
		 ISIN_CO(0,idx,(int)m_vCells[iPos]->GetVecCnt()) )
	{
		return m_vCells[iPos]->theStrVec[idx];
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
	
	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
    {
		return m_vCells[iPos]->GetVecCnt();
	}
	return 0; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();
		if ( iLen % 2 != 0 )
		{// ��������ż����˵����ʽ���󣬲�����Ҫ��
			ErrorLogLineCol( "Excel_GetRandx", iCol, RE_CELLERROR );
			return RE_CELLERROR;
		}

		int iRand = GetMaxRand(iRate);
		int iCount = 0;

		for ( int idx=0; idx<iLen; idx+=2 )
		{
			if ( iDefRe == RE_OK )
			{
				iCount += m_vCells[iPos]->theIntVec[idx];
			}
			else
			{
				iCount += atoi(m_vCells[iPos]->theStrVec[idx]);
			}

			if ( iCount >= iRand )
			{
				if ( iDefRe == RE_OK )
				{
					iData = m_vCells[iPos]->theIntVec[idx+1];
				}
				else
				{
					iData = atoi(m_vCells[iPos]->theStrVec[idx+1]);
				}
				iDataIdx = idx/2;
				return RE_OK;
			}
		}
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();
		if ( iLen % 2 != 0 )
		{// ��������ż����˵����ʽ���󣬲�����Ҫ��
			ErrorLogLineCol( "Excel_GetRandx", iCol, RE_CELLERROR );
			return RE_CELLERROR;
		}

		int iRand = GetMaxRand(iRate);
		int iCount = 0;

		for ( int idx=0; idx<iLen; idx+=2 )
		{
			iCount += atoi(m_vCells[iPos]->theStrVec[idx]);

			if ( iCount >= iRand )
			{		
				szData = m_vCells[iPos]->theStrVec[idx+1];
				iDataIdx = idx/2;
				return RE_OK;
			}
		}
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();
		int idx = rand() % iLen;

		if ( iDefRe == RE_OK )
		{
			iData = m_vCells[iPos]->theIntVec[idx];
		}
		else
		{
			iData = atoi(m_vCells[iPos]->theStrVec[idx]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();
		int idx = rand() % iLen;
		szData = m_vCells[iPos]->theStrVec[idx];
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iLen = m_vCells[iPos]->GetVecCnt();
		int idx = rand() % iLen;

		if ( iDefRe == RE_OK )
		{
			fData = m_vCells[iPos]->theFloatVec[idx];
		}
		else
		{
			fData = (float)atof(m_vCells[iPos]->theStrVec[idx]);
		}
		return RE_OK;
	}
	return RE_NOVALUE; // �� Cell
}

//�жϸ��ϵ�Ԫ���Ƿ���iData
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

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		int iNum = m_vCells[iPos]->GetVecCnt();
	
		if ( iDefRe == RE_OK )
		{
			for ( int i=0; i<iNum; i++ )
			{
				if ( iData == m_vCells[iPos]->theIntVec[i] )
				{
					return true;
				}
			}
		}
		else
		{
			for ( int i=0; i<iNum; i++ )
			{
				if ( iData == atoi(m_vCells[iPos]->theStrVec[i]) )
				{
					return true;
				}
			}
		}
	}
	return false; // �� Cell
}

bool SExcelLine::IsNull( int iCol )
{
	int iPos = m_ParentTable->GetColPos(iCol);
	if ( iPos == MAXCOLUMNS )
	{
		return true;
	}

	if ( iPos < (int)m_vCells.size() && m_vCells[iPos] )
	{
		return false;
	}
	return true; // �� Cell
}

//���游��
void SExcelLine::SetParentTable( CExcelTable* ParentTable )
{
	m_ParentTable = ParentTable;
}

//�����к�
void SExcelLine::SetLineID( unsigned short iLineID )
{
	m_iLineID = iLineID;
}

//��ȡ�к�
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
#endif
//************************************//
//              ���ú���              //
//************************************//

void GetFromString( const char * srcString, vector<Byte> &Output, char szSeparator )
{
	Output.clear();

	if ( ISEMPTY(srcString) )
	{
		return;
	}

	size_t iLength	= strlen(srcString);
	char* pszScr	= const_cast< char* >( srcString );
	char* pszToken	= pszScr;

	Byte bNum;

	for ( size_t i=0; i<iLength; i++ )
	{
		if( pszScr[i] == szSeparator )
		{// �Ƿָ���
			pszScr[i] = '\0';
			bNum = (Byte)atoi(pszToken);
			Output.push_back(bNum);
			pszToken = pszScr + i + 1;
		}
	}
	bNum = (Byte)atoi(pszToken);
	Output.push_back(bNum);
}

void GetFromString( const char * srcString, vector<int> &Output, char szSeparator )
{
	Output.clear();

	if ( ISEMPTY(srcString) )
	{
		return;
	}

	size_t iLength	= strlen(srcString);
	char* pszScr	= const_cast< char* >( srcString );
	char* pszToken	= pszScr;

	int iNum;

	for ( size_t i=0; i<iLength; i++ )
	{
		if( pszScr[i] == szSeparator )
		{// �Ƿָ���
			pszScr[i] = '\0';
			iNum = atoi(pszToken);
			Output.push_back(iNum);
			pszToken = pszScr + i + 1;
		}
	}
	iNum = atoi(pszToken);
	Output.push_back(iNum);
}

void GetFromString( const char * srcString, vector<long> &Output, char szSeparator )
{
	Output.clear();

	if ( ISEMPTY(srcString) )
	{
		return;
	}

	size_t iLength	= strlen(srcString);
	char* pszScr	= const_cast< char* >( srcString );
	char* pszToken	= pszScr;

	long lNum;

	for ( size_t i=0; i<iLength; i++ )
	{
		if( pszScr[i] == szSeparator )
		{// �Ƿָ���
			pszScr[i] = '\0';
			lNum = atol(pszToken);
			Output.push_back(lNum);
			pszToken = pszScr + i + 1;
		}
	}
	lNum = atol(pszToken);
	Output.push_back(lNum);
}

void GetFromString( const char * srcString, vector<float> &Output, char szSeparator )
{
	Output.clear();

	if ( ISEMPTY(srcString) )
	{
		return;
	}

	size_t iLength	= strlen(srcString);
	char* pszScr	= const_cast< char* >( srcString );
	char* pszToken	= pszScr;

	float fNum;

	for ( size_t i=0; i<iLength; i++ )
	{
		if( pszScr[i] == szSeparator )
		{// �Ƿָ���
			pszScr[i] = '\0';
			fNum = (float)atof(pszToken);
			Output.push_back(fNum);
			pszToken = pszScr + i + 1;
		}
	}
	fNum = (float)atof(pszToken);
	Output.push_back(fNum);
}

void GetFromString( const char * srcString , vector< const char * > &Output , char szSeparator )
{
	Output.clear();

	if ( ISEMPTY(srcString) )
	{
		return;
	}

	size_t iLength	= strlen(srcString);
	char* pszScr	= const_cast< char* >( srcString );
	char* pszToken	= pszScr;

	for ( size_t i=0; i<iLength; i++ )
	{
		if( pszScr[i] == szSeparator )
		{// �Ƿָ���
			pszScr[i] = '\0';
			if ( strcmp(pszScr,"") == 0 )
			{// �ÿո����
				Output.push_back( " " );
			}
			else
			{
				Output.push_back( pszToken );
			}
			pszToken = pszScr + i + 1;
		}
	}
	Output.push_back( pszToken );
}

bool FindInString( string &srcString, const char * szString, char szSeparator )
{
	if ( srcString == "" || ISEMPTY(szString) )
	{
		return false;
	}

	char SS[MAXSUBBYTE];
	int nCharCount = 0;

	size_t iLength = srcString.size();

	for ( size_t i=0; i<iLength; i++ )
	{
		// �Ƿָ���
		if( srcString[i] == szSeparator )
		{
			if ( nCharCount > 0 )
			{
				SS[nCharCount] = '\0';
				nCharCount = 0;

				if ( strcmp(SS,szString) == 0 )
				{
					return true;
				}
			}
		}
		else
		{
			SS[nCharCount] = srcString[i];
			nCharCount++;
		}
	}
	// ���һ��
	if( nCharCount > 0 )
	{
		SS[nCharCount] = '\0';

		return ( strcmp(SS,szString) == 0 );
	}
	// û���ҵ�
	return false;
}

void RemFromString( string &srcString, const char * szString, char szSeparator )
{
	if ( srcString == "" || ISEMPTY(szString) )
	{
		return;
	}

	char SS[MAXSUBBYTE];
	int nCharCount = 0;

	size_t iLength = srcString.size();
	size_t iRemLength = strlen(szString);

	for ( size_t i=0; i<iLength; i++ )
	{
		// �Ƿָ���
		if( srcString[i] == szSeparator )
		{
			if ( nCharCount > 0 )
			{
				SS[nCharCount] = '\0';
				nCharCount = 0;

				if ( strcmp(SS,szString) == 0 )
				{// ��� szString*
					srcString.erase(i-iRemLength,iRemLength+1);
					return;
				}
			}
		}
		else
		{
			SS[nCharCount] = srcString[i];
			nCharCount++;
		}
	}
	// ���һ��
	if( nCharCount > 0 )
	{
		SS[nCharCount] = '\0';

		if ( strcmp(SS,szString) == 0 )
		{
			if ( iLength > iRemLength )
			{// ��� *szString
				srcString.erase(iLength-iRemLength-1,iRemLength+1);
			}
			else
			{// ��� szString
				srcString.erase(0,iLength);
			}
		}
	}
}

void AddToString( string &srcString, const char * szString, char szSeparator )
{
	if ( ISEMPTY(szString) )
	{
		return;
	}

	if ( srcString == "" )
	{
		srcString = szString;
	}
	else if ( FindInString( srcString, szString, szSeparator ) == false )
	{
		srcString += szSeparator;
		srcString += szString;
	}
}
