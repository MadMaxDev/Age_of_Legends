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
			2007-1-11 ��ǿ���, �ո���Ϊ�Ƿ�
														
	  ��ע���������ɶ��뵥���Ϊ����Indexlist.txt������������ȫ��Ҫʹ�õı�
			ʹ�÷�����
					CExcelTableMng m_ExcelTableMng;
					m_ExcelTableMng.LoadTables(szDir,iType);//iTpyeΪʹ������,szDirΪ��Ŀ¼
			����ʹ�÷�����
					SExcelLine * pLine = NULL;
					pLine = m_ExcelTableMng.m_pTables[Itemlist]->GetLine(iExcelID);
					//Itemlist��Ҫ����˳���Լ�define��iExcelIDΪ����ֵ
					//û�������еı�Ҫ��GetLineLoaded(iLine)������iLineΪ�к�
			��Ԫֵ��÷�����
					int re = pLine->GetValue(iCol,&value);
					//reΪ���������valueΪ��õĵ�Ԫֵ����������������������Ĭ��ֵ��
					//int����Ĭ��ֵΪ0��float����Ĭ��ֵΪ0��string����Ĭ��ֵΪ""
														
*****************************************************************************************/

#ifndef __SOLOFILE_FUNC_H__
#define __SOLOFILE_FUNC_H__

#include <time.h>
#include "Excel_Cell.h"
#include "IDTable.h"
#include "IDCompressTable.h"
#include <vector>
#include <list>
#include <string>
#include <WHCMN/inc/whfile_base.h>
#include <WHCMN/inc/whfile_util.h>	// ��������GetDirFileList

using namespace std;

typedef unsigned char Byte;

// �Ƿ������µı�����㷨�����㷨��ҪΪ�ڴ��С���Ż���
#define EXCEL_USE_NEW_LINE 1

#define DEF_INT				0	//������Ϊ����
#define DEF_STR				1	//������Ϊ�Ӵ�
#define DEF_FLOAT			2	//������Ϊ������
#define DEF_VARIABLE		3	//������Ϊ���ͣ��ִ��򸡵�,����Ϊ����
#define DEF_INTARRAY		4	//����Ϊ����
#define DEF_STRARRAY		5	//����Ϊ�Ӵ�
#define DEF_FLOATARRAY		6	//����Ϊ������
#define DEF_VARIABLEARRAY	7	//����Ϊ���ͣ��Ӵ��򸡵�

#define DEF_ISARRAY(a) ( a >= DEF_INTARRAY && a <= DEF_VARIABLEARRAY )

#define RE_OK			100 //�����ɹ�
#define RE_NOVALUE		101 //��Cell
#define RE_NOCOLUM		102 //û�и���
#define RE_NOLINE		103 //û�и���
#define RE_NOTABLE		104 //û�иñ�
#define RE_DEFERROR		105 //���ʹ���
#define RE_DEFVAR		106 //��Ҫ����ת��
#define RE_CELLERROR	107 //Cell�е����ݴ���
#define RE_PRAMERROR	108 //��������

#if EXCEL_USE_NEW_LINE
#   define MAXCOLUMNS		200 // �����������������������ֵ��
#   define SP_COLUMN_BASE   (MAXCOLUMNS)  // ������������ʵֵ��
#   define SP_COLUMN_EMPTY  255             // ������
#   define SP_COLUMN_COUNT  (SP_COLUMN_EMPTY-SP_COLUMN_BASE)    // ���������ĸ���
#else
#   define MAXCOLUMNS		255
#endif
//#define MAXLINEBYTE		65536//һ������ֽ���
#define MAXSUBBYTE		1024 //һ����Ԫֵ����ֽ���
//#define MAXSUBNUM		10	 //���ϵ�Ԫ��������
#define MAXREADFILEBYTE 8192 //ÿ�δ��ļ������ֽ���
#define MAXNUMBER		32   //һ������/������ת����String�������ֽ���


enum FINDLOGIC
{
	Greater,// >
	Less,// <
	Equal,// =
	Greater_Equal,// >=
	Less_Equal,// <= 
	Not_Equal// != 
};

//************************************//
//              Excel��������         //
//************************************//
class CExcelTable;
struct SExcelLine;

class CExcelTableMng
{
public:
	CExcelTableMng();
	~CExcelTableMng();

	//��Ҫ���Ƶ����б�����ڴ档
	//iType�ǿ��Ƶ����,0��ͨ�ã�1�Ƿ�����ʹ�ã�2�ǿͻ���ʹ��
	bool LoadTables( const char * szDir,int iType );
	//����һ�ű�
	bool ReloadTable( int iTableID );
	//�ͷſռ�
	void Release();
	//����һ�ű�
	CExcelTable* SafeGetTable( int iTableID );
	//����һ�ű�
	CExcelTable* GetTable( int iTableID );
	//����һ�ű��е�һ��
	SExcelLine* SafeGetLine( int iTableID,int iExcelID );
	//����һ�ű��е�һ��
	SExcelLine* GetLine( int iTableID,int iExcelID );
	//���ر���
	int GetCount();
	//����������
	CExcelTable*	GetIndexTable()
	{
		return m_pIndexTable;
	}
protected:
	//�������,m_pTables[0]��ԶΪ��,����ָ��,��ֱ����ID������
	CExcelTable** m_pTables;
	//���� = �������ֵ + 1
	int m_nCount;
	CExcelTable*	m_pIndexTable;
};

extern  CExcelTableMng g_ExcelTableMng;

class CExcelTableNameMap
{
public:
	CExcelTableNameMap();
	~CExcelTableNameMap();

	//��Ҫ���Ƶ����б�����ڴ档
	bool LoadTables( const char * szDir, vector<Byte> &vDefs );
	//����һ�ű�
	bool ReloadTable( const char * szName );
	//�ͷſռ�
	void Release();
	//����һ�ű�
	CExcelTable * GetTable( const char * szName );

	// �����ļ�������
	inline void	SetFileMan( n_whcmn::whfileman * pFM )
	{
		m_pFM = pFM;
	}
	// ����ļ�������
	inline n_whcmn::whfileman *	GetFileMan()
	{
		return m_pFM;
	}

protected:
	map<string,CExcelTable *> m_TableNameMap;
	n_whcmn::whfileman * m_pFM;// �ļ�������ָ��
};

struct SFindTerm
{
	int       m_iPos;
	string    m_szData;
	FINDLOGIC m_logic;
	bool      m_bIncNull;
	Byte      m_bDef;

	SFindTerm( int iCol, const char * szData, FINDLOGIC logic
			 , bool bIncNull=false );

	bool Satisfy( SExcelLine * pLine );
};

// ������
struct CExcelIndex
{
    enum
    {
        t_array,    // ������������unsigned short���顣
        t_binary,   // ���ַ����ң���һ��<int,int>���顣
        t_table,    // hash����ʱ��ʵ��
    };

    void*   m_data;
    int     m_size;
    int     m_type;

    CExcelIndex() : m_data(0), m_size(0), m_type(t_array) {}
    ~CExcelIndex();

    void Create(int type, int maxId, int* ids, int cnt, const string& name);
    bool Insert(int id, int index);
    void Remove(int id);
    int Find(int id);
};

//************************************//
//              ֻ�����ݱ���          //
//************************************//
class CExcelTable
{
public:
	CExcelTable(); 
	~CExcelTable(); 

	//����IndexList��һ�еĶ���Loadһ��Table
	bool	Load( SExcelLine * pIndexLine, int iType );
	//����һ�ű�,�׸�ExcelID���ڱ��ĵ�iExcelIDCol��,��iExcelIDLine��
	bool	Load( const char * szName,int iExcelIDCol = 1,int iExcelIDLine = 0 );
	//����
	bool	Reload();

	//���ر������
	int		GetCount(){ return (int)m_vLines.size(); }
	//������������ֵ
	int		GetMaxExcelID();
	//���ر���
	const char * GetName(){ return m_vFileNames[0].c_str(); }

	//�������Ų���һ��
	//���ڷ��ص��Ǳ���һ�еĵ�ַ���Բ�����Է���ֵ�����޸Ĳ���
	SExcelLine * SafeGetLine( int iExcelID,bool booCanBeNull=false );
	SExcelLine * GetLine( int iExcelID );
	//���кŲ���һ��
	SExcelLine * GetLineFromList( int iLineID );

	int		GetColPos( int iCol );
	Byte	GetPosDef( int iPos );
	int		CheckPosDef( int iPos, Byte bDef );

	////////////////////////////////////////////////////////////////
	//�����������,�Զ���¼log,Ȼ�󷵻�0/""/0.0
	//������iExcelID��iCol�е�ֵ,idxΪ���ϵ�Ԫ�е����
	//
	//ȡ�Ϸ��Ŀ�Cellֵ����������,����log,����0/""/0.0
	//�����Ҫ�Խ��Ϳ�Cellֵʱ���� GetLine + GetValue
	//
	int		SafeGetInt( int iExcelID,int iCol,bool booCanBeNull=false );
	const char * FastGetStr( int iExcelID,int iCol,bool booCanBeNull=false );
	void	SafeGetStr( int iExcelID,int iCol,string &Output,bool booCanBeNull=false );
	float	SafeGetFloat( int iExcelID,int iCol,bool booCanBeNull=false );
	void	SafeGetArray( int iExcelID,int iCol,vector<int> &Output,bool booCanBeNull=false );
	void	SafeGetArray( int iExcelID,int iCol,vector<const char *> &Output,bool booCanBeNull=false );
	void	SafeGetArray( int iExcelID,int iCol,vector<float> &Output,bool booCanBeNull=false );

	/////////////////////////////////////////////////////////////////////////////
	//�����ݲ���
	//���ص�Col��ֵΪValue��һ��
	SExcelLine * FindLine( int iCol,int iValue );
	SExcelLine * FindLine( int iCol,const char * szValue );
	SExcelLine * FindLine( int iCol,float fValue );

	//  ����ʹ���⼸������
	bool	TestFindTerm( vector<SFindTerm *> &Terms );
	void	FindLineGroup( vector<SFindTerm *> &Terms,list<SExcelLine *> &Output );
	void	FindLineGroupContinue( vector<SFindTerm *> &Terms,list<SExcelLine *> &Output );

	void	RemoveAll();

	//*************************************************************/
	bool	AddLine( int iExcelID );
	bool	SetLine( int iExcelID,SExcelLine * pLine );
	bool	SetLine( int iExcelID,int iCol,const char * szValue );
	void	RemLine( int iExcelID );

	void	Save( const char * szName=NULL,bool booHeader=true );

	bool	LoadOneExcel(const char* szFileName, vector<int> &vExcelIDs);
/*
	//����ѹ����iBegin����ʼ��
	void			  BuildCompressTable( int iBegin );
	//�ͷ�ѹ����
	void			  ReleaseCompressTable();
	//���������Ż��ѹ�����е�һ��
	CIDCompressTable* GetCompressLine( int iLine );
*/
	int		GetExcelIDCol(){ return  m_iExcelIDCol; }//����ֵ�к�

	void	GetColNames( vector<string> &vStrs );
	int		GetMaxColID(){ return m_iMaxColID; }

public:
	vector<Byte> m_vDefs;//ԭExcel����������
	vector<Byte> m_vCols;//ԭExcel�������ڱ�Table�е�iCol

protected:
	void	GetBeginLines( vector<string> &vStrs );

protected:

    CExcelIndex m_index;
	//SExcelLine** m_pLines;//����˳���
    //unsigned short* m_pLines;   // ����˳���
	vector<SExcelLine *> m_vLines;//��˳���
	//CIDCompressTable * m_pCompressTables;//ѹ����
	vector<string> m_vFileNames; //�ļ���(ȫ������·���ͺ�׺)

	//string m_szName;//����(ȫ������·���ͺ�׺)

	int m_iMaxExcelID;//�������ֵ
	int m_iExcelIDCol;//����ֵ�к�
	int m_iMaxColID;//�����Ч�к�
	int m_iColCount;//����, <= m_iMaxColID, >= DefList.size()

	int m_iBeginLine;//��ʼ��
};
//************************************//
//           Excel���нṹ            //
//************************************//
#if EXCEL_USE_NEW_LINE
struct SExcelLine : quick_plex_obj<SExcelLine>
{
    friend class CExcelTable;

	SExcelLine();
	~SExcelLine();

	int			GetCount(){ return (int)m_nSize; }

    //// ÿ��Cell������һ��Bit��־�Ƿ���д�������û����д�����򷵻�NULL
    //Excel_Cell* GetMaskCell(int iCol)
    //{
    //    if((unsigned)iCol >= m_nSize) return 0;
    //    unsigned mask = *((unsigned*)(m_pCells+m_nCapacity)+iCol/32);
    //    if(mask&(1<<iCol%32)) return m_pCells + iCol;
    //    return 0;
    //}
    Excel_Cell* GetCell(int iPos);

	////////////////////////////////////////////////////////////////
	//��Ԫ��ֵ����
	void		AddCell( Byte bDef,const char * szValue );
	bool		SetCell( int iCol,const char * szValue );
	
	////////////////////////////////////////////////////////////////
	//��CExcelTable::FindLine����,��ؼ��������
	//ֱ����Pos������,�ڴ˲����ظ����
	bool		DirectIsValue( int iPos,int iValue );
	bool		DirectIsValue( int iPos,const char * szValue );
	bool		DirectIsValue( int iPos,float fValue );
	
	int			DirectGetVariable( int iPos,int &iValue );
	int			DirectGetVariable( int iPos,float &fValue );
	
	void		DirectGetString( int iPos,Byte bDef,string &szValue );
	////////////////////////////////////////////////////////////////
	//ȡ��Ԫֵ��iColΪ�к�
	int 		GetValue( int iCol,int &iValue );
	int 		GetValue( int iCol,string &szValue );
	int 		GetValue( int iCol,float &fValue );
	int 		GetArray( int iCol,vector<Byte> &Output );
	int 		GetArray( int iCol,vector<int> &Output );
	int 		GetArray( int iCol,vector<const char *> &Output );
	int 		GetArray( int iCol,vector<float> &Output );
	//ȡ��Ԫֵ�е�һ����,iColΪ�к�,idxΪ������š�
	//���ຯ��ֻ������AA*BB*CC��Ԫֵ��Ч
	int 		GetValue( int iCol,int idx,int &iValue );
	int 		GetValue( int iCol,int idx,string &szValue );
	int 		GetValue( int iCol,int idx,float &fValue );
	
	////////////////////////////////////////////////////////////////
	//ȡ��Ԫֵ��iColΪ�к�,�����������,�Զ���¼log,Ȼ�󷵻�0/""/0.0
	//
	//ȡ�Ϸ��Ŀ�Cellֵ����������,����log,����0/""/0.0
	//�����Ҫ�Խ��Ϳ�Cellֵʱ���� GetValue
	//

	// Ŀ��:	��ȡĳ�е����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�, �ո��Ƿ�Ϸ�
	// ����ֵ:	��Ӧ��������
	int 		SafeGetInt( int iCol,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ�е����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�, �ո��Ƿ�Ϸ�
	// ����ֵ:	�ַ�������
	const char * FastGetStr( int iCol,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ�е����ݣ����ʧ�ܻ��¼����log
	// ����:	�кţ��ַ�������
	// ����ֵ:	��
	void		SafeGetStr( int iCol,string &szValue,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ�е����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�
	// ����ֵ:	��Ӧ��������
	float 		SafeGetFloat( int iCol,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�,���
	// ����ֵ:	��Ӧ��������
	int 		SafeGetArrayInt( int iCol,int idx,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�,���
	// ����ֵ:	�ַ�������
	const char * FastGetArrayStr( int iCol,int idx,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�,��ţ��ַ�������
	// ����ֵ:	��
	void		SafeGetArrayStr( int iCol,int idx,string &szValue,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�,���
	// ����ֵ:	��Ӧ��������	
	float 		SafeGetArrayFloat( int iCol,int idx,bool booCanBeNull=false );

	// Ŀ��:	���ظ��ϵ�Ԫֵ����ɲ�������(��Ӧ�����*�ָ
	// ����:	�к�
	// ����ֵ:	����
	int			GetArraySize( int iCol );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�, �ո��Ƿ�Ϸ�
	// ����ֵ:	��Ӧ��������
	int 		SafeGetRandInt( int iCol,bool booCanBeNull=false );


	void		SafeGetArray( int iCol,vector<int> &Output,bool booCanBeNull=false );
	void 		SafeGetArray( int iCol,vector<const char *> &Output,bool booCanBeNull=false );
	void		SafeGetArray( int iCol,vector<float> &Output,bool booCanBeNull=false );

	////////////////////////////////////////////////////////////////
	// Ŀ��:	�Ӹ��ϵ�Ԫֵ�������ȡһ����
	int			RandGetData( int iCol,int &iData );
	int 		RandGetData( int iCol,string &szData );
	int 		RandGetData( int iCol,float &fData );

	// Ŀ��:	�ڣ�����*����*����*����*...���ĸ�ʽ�������ȡһ������(����֮�����Ϊ100)
	// ����:	�кţ����ܵ����ݣ������ǵڼ�������
	// ����ֵ:	RE_
	int			GetRand100( int iCol,int &iData,int &iDataIdx );
	int			GetRand100( int iCol,string &szData,int &iDataIdx );

	int			GetRandx( int iCol,int &iData,int &iDataIdx,int iRate );
	int			GetRandx( int iCol,string &szData,int &iDataIdx,int iRate );

	// Ŀ��:	�жϸ��ϵ�Ԫ���Ƿ���iData
	bool		IsData( int iCol,int iData );

	////////////////////////////////////////////////////////////////
	// Ŀ��:	�ж�Cell�Ƿ�Ϊ��, iColΪ�к�
	bool		IsNull( int iCol );

	////////////////////////////////////////////////////////////////
	// Ŀ��:	���游��
	void		SetParentTable(CExcelTable* ParentTable);

	// Ŀ��:	�����к�
	void        SetLineID( unsigned short iLn );
	// Ŀ��:	��ȡ�к�
	int			GetLineID();

protected:
	void		CreateCell(int nIdx, Byte bDef,const char * szValue );

	void		GetCellStr( Excel_Cell * pCell,Byte bDef,string &szValue );

	void		ErrorLogLineCol( const char * szError,int iCol,int iRE,int idx = 0 );
protected:
	CExcelTable *	m_ParentTable;
    union
    {
        Excel_Cell*     m_pCellData;    // ������Excel_Cell���顣
        unsigned char*  m_pCellIndex;   // ������byte���顣
    };
    unsigned char   m_nSize;    // ��Ԫ��ĸ�����С�ڵ���������β���Ŀո��ӻᱻ�õ���
    unsigned char   m_nUsed;    // ʹ�õĸ���������
	unsigned short	m_iLineID;//�к�
};

#else
struct SExcelLine : quick_plex_obj<SExcelLine>
{
	SExcelLine();
	~SExcelLine();

	int			GetCount(){ return (int)m_vCells.size(); }
    Excel_Cell* GetCell(int iCol) { return m_vCells[iCol]; }
    void		Reserve(int cnt) { m_vCells.reserve(cnt); }
	////////////////////////////////////////////////////////////////
	//��Ԫ��ֵ����
	void		AddCell( Byte bDef,const char * szValue );
	bool		SetCell( int iCol,const char * szValue );
	
	////////////////////////////////////////////////////////////////
	//��CExcelTable::FindLine����,��ؼ��������
	//ֱ����Pos������,�ڴ˲����ظ����
	bool		DirectIsValue( int iPos,int iValue );
	bool		DirectIsValue( int iPos,const char * szValue );
	bool		DirectIsValue( int iPos,float fValue );
	
	int			DirectGetVariable( int iPos,int &iValue );
	int			DirectGetVariable( int iPos,float &fValue );
	
	void		DirectGetString( int iPos,Byte bDef,string &szValue );
	////////////////////////////////////////////////////////////////
	//ȡ��Ԫֵ��iColΪ�к�
	int 		GetValue( int iCol,int &iValue );
	int 		GetValue( int iCol,string &szValue );
	int 		GetValue( int iCol,float &fValue );
	int 		GetArray( int iCol,vector<Byte> &Output );
	int 		GetArray( int iCol,vector<int> &Output );
	int 		GetArray( int iCol,vector<const char *> &Output );
	int 		GetArray( int iCol,vector<float> &Output );
	//ȡ��Ԫֵ�е�һ����,iColΪ�к�,idxΪ������š�
	//���ຯ��ֻ������AA*BB*CC��Ԫֵ��Ч
	int 		GetValue( int iCol,int idx,int &iValue );
	int 		GetValue( int iCol,int idx,string &szValue );
	int 		GetValue( int iCol,int idx,float &fValue );
	
	////////////////////////////////////////////////////////////////
	//ȡ��Ԫֵ��iColΪ�к�,�����������,�Զ���¼log,Ȼ�󷵻�0/""/0.0
	//
	//ȡ�Ϸ��Ŀ�Cellֵ����������,����log,����0/""/0.0
	//�����Ҫ�Խ��Ϳ�Cellֵʱ���� GetValue
	//

	// Ŀ��:	��ȡĳ�е����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�, �ո��Ƿ�Ϸ�
	// ����ֵ:	��Ӧ��������
	int 		SafeGetInt( int iCol,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ�е����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�, �ո��Ƿ�Ϸ�
	// ����ֵ:	�ַ�������
	const char * FastGetStr( int iCol,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ�е����ݣ����ʧ�ܻ��¼����log
	// ����:	�кţ��ַ�������
	// ����ֵ:	��
	void		SafeGetStr( int iCol,string &szValue,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ�е����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�
	// ����ֵ:	��Ӧ��������
	float 		SafeGetFloat( int iCol,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�,���
	// ����ֵ:	��Ӧ��������
	int 		SafeGetArrayInt( int iCol,int idx,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�,���
	// ����ֵ:	�ַ�������
	const char * FastGetArrayStr( int iCol,int idx,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�,��ţ��ַ�������
	// ����ֵ:	��
	void		SafeGetArrayStr( int iCol,int idx,string &szValue,bool booCanBeNull=false );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�,���
	// ����ֵ:	��Ӧ��������	
	float 		SafeGetArrayFloat( int iCol,int idx,bool booCanBeNull=false );

	// Ŀ��:	���ظ��ϵ�Ԫֵ����ɲ�������(��Ӧ�����*�ָ
	// ����:	�к�
	// ����ֵ:	����
	int			GetArraySize( int iCol );

	// Ŀ��:	��ȡĳ��(��Ӧ�����*�ָ����ĳ�����ݣ����ʧ�ܻ��¼����log
	// ����:	�к�, �ո��Ƿ�Ϸ�
	// ����ֵ:	��Ӧ��������
	int 		SafeGetRandInt( int iCol,bool booCanBeNull=false );


	void		SafeGetArray( int iCol,vector<int> &Output,bool booCanBeNull=false );
	void 		SafeGetArray( int iCol,vector<const char *> &Output,bool booCanBeNull=false );
	void		SafeGetArray( int iCol,vector<float> &Output,bool booCanBeNull=false );

	////////////////////////////////////////////////////////////////
	// Ŀ��:	�Ӹ��ϵ�Ԫֵ�������ȡһ����
	int			RandGetData( int iCol,int &iData );
	int 		RandGetData( int iCol,string &szData );
	int 		RandGetData( int iCol,float &fData );

	// Ŀ��:	�ڣ�����*����*����*����*...���ĸ�ʽ�������ȡһ������(����֮�����Ϊ100)
	// ����:	�кţ����ܵ����ݣ������ǵڼ�������
	// ����ֵ:	RE_
	int			GetRand100( int iCol,int &iData,int &iDataIdx );
	int			GetRand100( int iCol,string &szData,int &iDataIdx );

	int			GetRandx( int iCol,int &iData,int &iDataIdx,int iRate );
	int			GetRandx( int iCol,string &szData,int &iDataIdx,int iRate );

	// Ŀ��:	�жϸ��ϵ�Ԫ���Ƿ���iData
	bool		IsData( int iCol,int iData );

	////////////////////////////////////////////////////////////////
	// Ŀ��:	�ж�Cell�Ƿ�Ϊ��, iColΪ�к�
	bool		IsNull( int iCol );

	////////////////////////////////////////////////////////////////
	// Ŀ��:	���游��
	void		SetParentTable(CExcelTable* ParentTable);
	// Ŀ��:	�����к�
	void        SetLineID( unsigned short iLn );
	// Ŀ��:	��ȡ�к�
	int			GetLineID();

protected:
	Excel_Cell * CreateCell( Byte bDef,const char * szValue );

	void		GetCellStr( Excel_Cell * pCell,Byte bDef,string &szValue );

	void		ErrorLogLineCol( const char * szError,int iCol,int iRE,int idx = 0 );
public:
	vector<Excel_Cell *> m_vCells;//��Ԫֵ

protected:
	CExcelTable *	m_ParentTable;
	unsigned short	m_iLineID;//�к�
};

#endif

//************************************//
//              ���ú���              //
//************************************//
// ���ַ����Ĳ���
void GetFromString( const char * srcString, vector<Byte> &Output, char szSeparator ='*' );
void GetFromString( const char * srcString, vector<int> &Output, char szSeparator ='*' );
void GetFromString( const char * srcString, vector<long> &Output, char szSeparator ='*' );
void GetFromString( const char * srcString, vector<const char *> &Output, char szSeparator ='*' );
void GetFromString( const char * srcString, vector<float> &Output, char szSeparator ='*' );
bool FindInString( string &srcString, const char * szString, char szSeparator ='*' );
void RemFromString( string &srcString, const char * szString, char szSeparator ='*' );
void AddToString( string &srcString, const char * szString, char szSeparator ='*' );

#endif //__SOLOFILEFUNC_H__
