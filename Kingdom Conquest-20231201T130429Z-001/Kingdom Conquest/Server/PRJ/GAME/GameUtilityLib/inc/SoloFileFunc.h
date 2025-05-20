/*****************************************************************************************
														
	模块名：SOLOFILEFUNC								
														
	  目的：文件函数								
														
	  原作者：soloman	
	  
	  修改者：swolf(王威)
			  Erica Gao
														
	编译器：Microsoft Visual C++ 6.0
														
	  历史：2000-6-23 9:27:13 创建 
			2005-6-20 修改完善
			2006-2-28 修改
			2007-1-11 加强检测, 空格视为非法
														
	  备注：主功能由读入单表改为根据Indexlist.txt（索引表）读入全部要使用的表
			使用方法：
					CExcelTableMng m_ExcelTableMng;
					m_ExcelTableMng.LoadTables(szDir,iType);//iTpye为使用类型,szDir为表目录
			单表使用方法：
					SExcelLine * pLine = NULL;
					pLine = m_ExcelTableMng.m_pTables[Itemlist]->GetLine(iExcelID);
					//Itemlist需要根据顺序自己define，iExcelID为索引值
					//没有索引列的表要用GetLineLoaded(iLine)函数，iLine为行号
			单元值获得方法：
					int re = pLine->GetValue(iCol,&value);
					//re为操作结果。value为获得的单元值。操作错误等特殊情况返回默认值，
					//int类型默认值为0，float类型默认值为0，string类型默认值为""
														
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
#include <WHCMN/inc/whfile_util.h>	// 这里面有GetDirFileList

using namespace std;

typedef unsigned char Byte;

// 是否启用新的表格行算法，新算法主要为内存大小做优化。
#define EXCEL_USE_NEW_LINE 1

#define DEF_INT				0	//数剧列为整型
#define DEF_STR				1	//数剧列为子串
#define DEF_FLOAT			2	//数剧列为浮点数
#define DEF_VARIABLE		3	//数据列为整型，字串或浮点,不能为数组
#define DEF_INTARRAY		4	//数组为整型
#define DEF_STRARRAY		5	//数组为子串
#define DEF_FLOATARRAY		6	//数组为浮点数
#define DEF_VARIABLEARRAY	7	//数组为整型，子串或浮点

#define DEF_ISARRAY(a) ( a >= DEF_INTARRAY && a <= DEF_VARIABLEARRAY )

#define RE_OK			100 //操作成功
#define RE_NOVALUE		101 //空Cell
#define RE_NOCOLUM		102 //没有该列
#define RE_NOLINE		103 //没有该行
#define RE_NOTABLE		104 //没有该表
#define RE_DEFERROR		105 //类型错误
#define RE_DEFVAR		106 //需要类型转换
#define RE_CELLERROR	107 //Cell中的内容错误
#define RE_PRAMERROR	108 //参数错误

#if EXCEL_USE_NEW_LINE
#   define MAXCOLUMNS		200 // 更大的索引用来引用特殊数值。
#   define SP_COLUMN_BASE   (MAXCOLUMNS)  // 特殊索引的其实值。
#   define SP_COLUMN_EMPTY  255             // 空索引
#   define SP_COLUMN_COUNT  (SP_COLUMN_EMPTY-SP_COLUMN_BASE)    // 特殊索引的个数
#else
#   define MAXCOLUMNS		255
#endif
//#define MAXLINEBYTE		65536//一行最大字节数
#define MAXSUBBYTE		1024 //一个单元值最大字节数
//#define MAXSUBNUM		10	 //复合单元最大组成数
#define MAXREADFILEBYTE 8192 //每次从文件读入字节数
#define MAXNUMBER		32   //一个整数/浮点数转化成String后的最大字节数


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
//              Excel管理者类         //
//************************************//
class CExcelTable;
struct SExcelLine;

class CExcelTableMng
{
public:
	CExcelTableMng();
	~CExcelTableMng();

	//将要控制的所有表读入内存。
	//iType是控制的类别,0是通用，1是服务器使用，2是客户端使用
	bool LoadTables( const char * szDir,int iType );
	//重载一张表
	bool ReloadTable( int iTableID );
	//释放空间
	void Release();
	//返回一张表
	CExcelTable* SafeGetTable( int iTableID );
	//返回一张表
	CExcelTable* GetTable( int iTableID );
	//返回一张表中的一行
	SExcelLine* SafeGetLine( int iTableID,int iExcelID );
	//返回一张表中的一行
	SExcelLine* GetLine( int iTableID,int iExcelID );
	//返回表数
	int GetCount();
	//返回索引表
	CExcelTable*	GetIndexTable()
	{
		return m_pIndexTable;
	}
protected:
	//表的数组,m_pTables[0]永远为空,含空指针,可直接用ID做索引
	CExcelTable** m_pTables;
	//表数 = 最大索引值 + 1
	int m_nCount;
	CExcelTable*	m_pIndexTable;
};

extern  CExcelTableMng g_ExcelTableMng;

class CExcelTableNameMap
{
public:
	CExcelTableNameMap();
	~CExcelTableNameMap();

	//将要控制的所有表读入内存。
	bool LoadTables( const char * szDir, vector<Byte> &vDefs );
	//重载一张表
	bool ReloadTable( const char * szName );
	//释放空间
	void Release();
	//返回一张表
	CExcelTable * GetTable( const char * szName );

	// 设置文件管理器
	inline void	SetFileMan( n_whcmn::whfileman * pFM )
	{
		m_pFM = pFM;
	}
	// 获得文件管理器
	inline n_whcmn::whfileman *	GetFileMan()
	{
		return m_pFM;
	}

protected:
	map<string,CExcelTable *> m_TableNameMap;
	n_whcmn::whfileman * m_pFM;// 文件管理器指针
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

// 索引类
struct CExcelIndex
{
    enum
    {
        t_array,    // 数组索引：是unsigned short数组。
        t_binary,   // 二分法查找：是一个<int,int>数组。
        t_table,    // hash表：暂时不实现
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
//              只读数据表类          //
//************************************//
class CExcelTable
{
public:
	CExcelTable(); 
	~CExcelTable(); 

	//根据IndexList中一行的定义Load一个Table
	bool	Load( SExcelLine * pIndexLine, int iType );
	//读入一张表,首个ExcelID号在表格的第iExcelIDCol列,第iExcelIDLine行
	bool	Load( const char * szName,int iExcelIDCol = 1,int iExcelIDLine = 0 );
	//重载
	bool	Reload();

	//返回表的行数
	int		GetCount(){ return (int)m_vLines.size(); }
	//返回最大的索引值
	int		GetMaxExcelID();
	//返回表名
	const char * GetName(){ return m_vFileNames[0].c_str(); }

	//按索引号查找一行
	//由于返回的是表中一行的地址所以不允许对返回值进行修改操作
	SExcelLine * SafeGetLine( int iExcelID,bool booCanBeNull=false );
	SExcelLine * GetLine( int iExcelID );
	//按行号查找一行
	SExcelLine * GetLineFromList( int iLineID );

	int		GetColPos( int iCol );
	Byte	GetPosDef( int iPos );
	int		CheckPosDef( int iPos, Byte bDef );

	////////////////////////////////////////////////////////////////
	//如果出现问题,自动记录log,然后返回0/""/0.0
	//方便获得iExcelID行iCol列的值,idx为复合单元中的序号
	//
	//取合法的空Cell值属正常操作,不记log,会获得0/""/0.0
	//因此需要自解释空Cell值时请用 GetLine + GetValue
	//
	int		SafeGetInt( int iExcelID,int iCol,bool booCanBeNull=false );
	const char * FastGetStr( int iExcelID,int iCol,bool booCanBeNull=false );
	void	SafeGetStr( int iExcelID,int iCol,string &Output,bool booCanBeNull=false );
	float	SafeGetFloat( int iExcelID,int iCol,bool booCanBeNull=false );
	void	SafeGetArray( int iExcelID,int iCol,vector<int> &Output,bool booCanBeNull=false );
	void	SafeGetArray( int iExcelID,int iCol,vector<const char *> &Output,bool booCanBeNull=false );
	void	SafeGetArray( int iExcelID,int iCol,vector<float> &Output,bool booCanBeNull=false );

	/////////////////////////////////////////////////////////////////////////////
	//按数据查找
	//返回第Col列值为Value的一行
	SExcelLine * FindLine( int iCol,int iValue );
	SExcelLine * FindLine( int iCol,const char * szValue );
	SExcelLine * FindLine( int iCol,float fValue );

	//  谨慎使用这几个函数
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
	//创建压缩表，iBegin是起始列
	void			  BuildCompressTable( int iBegin );
	//释放压缩表
	void			  ReleaseCompressTable();
	//根据索引号获得压缩表中的一行
	CIDCompressTable* GetCompressLine( int iLine );
*/
	int		GetExcelIDCol(){ return  m_iExcelIDCol; }//索引值列号

	void	GetColNames( vector<string> &vStrs );
	int		GetMaxColID(){ return m_iMaxColID; }

public:
	vector<Byte> m_vDefs;//原Excel表格各列类型
	vector<Byte> m_vCols;//原Excel表格各列在本Table中的iCol

protected:
	void	GetBeginLines( vector<string> &vStrs );

protected:

    CExcelIndex m_index;
	//SExcelLine** m_pLines;//索引顺序表
    //unsigned short* m_pLines;   // 索引顺序表
	vector<SExcelLine *> m_vLines;//行顺序表
	//CIDCompressTable * m_pCompressTables;//压缩表
	vector<string> m_vFileNames; //文件名(全名包括路径和后缀)

	//string m_szName;//表名(全名包括路径和后缀)

	int m_iMaxExcelID;//最大索引值
	int m_iExcelIDCol;//索引值列号
	int m_iMaxColID;//最大有效列号
	int m_iColCount;//列数, <= m_iMaxColID, >= DefList.size()

	int m_iBeginLine;//起始行
};
//************************************//
//           Excel表行结构            //
//************************************//
#if EXCEL_USE_NEW_LINE
struct SExcelLine : quick_plex_obj<SExcelLine>
{
    friend class CExcelTable;

	SExcelLine();
	~SExcelLine();

	int			GetCount(){ return (int)m_nSize; }

    //// 每个Cell都会有一个Bit标志是否填写过，如果没有填写过，则返回NULL
    //Excel_Cell* GetMaskCell(int iCol)
    //{
    //    if((unsigned)iCol >= m_nSize) return 0;
    //    unsigned mask = *((unsigned*)(m_pCells+m_nCapacity)+iCol/32);
    //    if(mask&(1<<iCol%32)) return m_pCells + iCol;
    //    return 0;
    //}
    Excel_Cell* GetCell(int iPos);

	////////////////////////////////////////////////////////////////
	//单元赋值函数
	void		AddCell( Byte bDef,const char * szValue );
	bool		SetCell( int iCol,const char * szValue );
	
	////////////////////////////////////////////////////////////////
	//由CExcelTable::FindLine调用,相关检测已做过
	//直接用Pos做索引,在此不做重复检查
	bool		DirectIsValue( int iPos,int iValue );
	bool		DirectIsValue( int iPos,const char * szValue );
	bool		DirectIsValue( int iPos,float fValue );
	
	int			DirectGetVariable( int iPos,int &iValue );
	int			DirectGetVariable( int iPos,float &fValue );
	
	void		DirectGetString( int iPos,Byte bDef,string &szValue );
	////////////////////////////////////////////////////////////////
	//取单元值，iCol为列号
	int 		GetValue( int iCol,int &iValue );
	int 		GetValue( int iCol,string &szValue );
	int 		GetValue( int iCol,float &fValue );
	int 		GetArray( int iCol,vector<Byte> &Output );
	int 		GetArray( int iCol,vector<int> &Output );
	int 		GetArray( int iCol,vector<const char *> &Output );
	int 		GetArray( int iCol,vector<float> &Output );
	//取单元值中的一部分,iCol为列号,idx为部分序号。
	//此类函数只对类似AA*BB*CC单元值有效
	int 		GetValue( int iCol,int idx,int &iValue );
	int 		GetValue( int iCol,int idx,string &szValue );
	int 		GetValue( int iCol,int idx,float &fValue );
	
	////////////////////////////////////////////////////////////////
	//取单元值，iCol为列号,如果出现问题,自动记录log,然后返回0/""/0.0
	//
	//取合法的空Cell值属正常操作,不记log,会获得0/""/0.0
	//因此需要自解释空Cell值时请用 GetValue
	//

	// 目的:	或取某列的数据，如果失败会纪录错误log
	// 参数:	列号, 空格是否合法
	// 返回值:	对应整数数据
	int 		SafeGetInt( int iCol,bool booCanBeNull=false );

	// 目的:	或取某列的数据，如果失败会纪录错误log
	// 参数:	列号, 空格是否合法
	// 返回值:	字符串数据
	const char * FastGetStr( int iCol,bool booCanBeNull=false );

	// 目的:	或取某列的数据，如果失败会纪录错误log
	// 参数:	列号，字符串数据
	// 返回值:	无
	void		SafeGetStr( int iCol,string &szValue,bool booCanBeNull=false );

	// 目的:	或取某列的数据，如果失败会纪录错误log
	// 参数:	列号
	// 返回值:	对应浮点数据
	float 		SafeGetFloat( int iCol,bool booCanBeNull=false );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号,项号
	// 返回值:	对应整数数据
	int 		SafeGetArrayInt( int iCol,int idx,bool booCanBeNull=false );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号,项号
	// 返回值:	字符串数据
	const char * FastGetArrayStr( int iCol,int idx,bool booCanBeNull=false );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号,项号，字符串数据
	// 返回值:	无
	void		SafeGetArrayStr( int iCol,int idx,string &szValue,bool booCanBeNull=false );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号,项号
	// 返回值:	对应浮点数据	
	float 		SafeGetArrayFloat( int iCol,int idx,bool booCanBeNull=false );

	// 目的:	返回复合单元值的组成部分总数(对应项采用*分割）
	// 参数:	列号
	// 返回值:	项数
	int			GetArraySize( int iCol );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号, 空格是否合法
	// 返回值:	对应整数数据
	int 		SafeGetRandInt( int iCol,bool booCanBeNull=false );


	void		SafeGetArray( int iCol,vector<int> &Output,bool booCanBeNull=false );
	void 		SafeGetArray( int iCol,vector<const char *> &Output,bool booCanBeNull=false );
	void		SafeGetArray( int iCol,vector<float> &Output,bool booCanBeNull=false );

	////////////////////////////////////////////////////////////////
	// 目的:	从复合单元值中随机抽取一部分
	int			RandGetData( int iCol,int &iData );
	int 		RandGetData( int iCol,string &szData );
	int 		RandGetData( int iCol,float &fData );

	// 目的:	在（几率*数据*几率*数据*...）的格式下随机获取一个数据(几率之合最大为100)
	// 参数:	列号，接受的数据，接受是第几个数据
	// 返回值:	RE_
	int			GetRand100( int iCol,int &iData,int &iDataIdx );
	int			GetRand100( int iCol,string &szData,int &iDataIdx );

	int			GetRandx( int iCol,int &iData,int &iDataIdx,int iRate );
	int			GetRandx( int iCol,string &szData,int &iDataIdx,int iRate );

	// 目的:	判断复合单元中是否有iData
	bool		IsData( int iCol,int iData );

	////////////////////////////////////////////////////////////////
	// 目的:	判断Cell是否为空, iCol为列号
	bool		IsNull( int iCol );

	////////////////////////////////////////////////////////////////
	// 目的:	保存父表
	void		SetParentTable(CExcelTable* ParentTable);

	// 目的:	保存行号
	void        SetLineID( unsigned short iLn );
	// 目的:	获取行号
	int			GetLineID();

protected:
	void		CreateCell(int nIdx, Byte bDef,const char * szValue );

	void		GetCellStr( Excel_Cell * pCell,Byte bDef,string &szValue );

	void		ErrorLogLineCol( const char * szError,int iCol,int iRE,int idx = 0 );
protected:
	CExcelTable *	m_ParentTable;
    union
    {
        Excel_Cell*     m_pCellData;    // 正向是Excel_Cell数组。
        unsigned char*  m_pCellIndex;   // 反向是byte数组。
    };
    unsigned char   m_nSize;    // 单元格的个数，小于等于列数。尾部的空格子会被裁掉。
    unsigned char   m_nUsed;    // 使用的格子数量。
	unsigned short	m_iLineID;//行号
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
	//单元赋值函数
	void		AddCell( Byte bDef,const char * szValue );
	bool		SetCell( int iCol,const char * szValue );
	
	////////////////////////////////////////////////////////////////
	//由CExcelTable::FindLine调用,相关检测已做过
	//直接用Pos做索引,在此不做重复检查
	bool		DirectIsValue( int iPos,int iValue );
	bool		DirectIsValue( int iPos,const char * szValue );
	bool		DirectIsValue( int iPos,float fValue );
	
	int			DirectGetVariable( int iPos,int &iValue );
	int			DirectGetVariable( int iPos,float &fValue );
	
	void		DirectGetString( int iPos,Byte bDef,string &szValue );
	////////////////////////////////////////////////////////////////
	//取单元值，iCol为列号
	int 		GetValue( int iCol,int &iValue );
	int 		GetValue( int iCol,string &szValue );
	int 		GetValue( int iCol,float &fValue );
	int 		GetArray( int iCol,vector<Byte> &Output );
	int 		GetArray( int iCol,vector<int> &Output );
	int 		GetArray( int iCol,vector<const char *> &Output );
	int 		GetArray( int iCol,vector<float> &Output );
	//取单元值中的一部分,iCol为列号,idx为部分序号。
	//此类函数只对类似AA*BB*CC单元值有效
	int 		GetValue( int iCol,int idx,int &iValue );
	int 		GetValue( int iCol,int idx,string &szValue );
	int 		GetValue( int iCol,int idx,float &fValue );
	
	////////////////////////////////////////////////////////////////
	//取单元值，iCol为列号,如果出现问题,自动记录log,然后返回0/""/0.0
	//
	//取合法的空Cell值属正常操作,不记log,会获得0/""/0.0
	//因此需要自解释空Cell值时请用 GetValue
	//

	// 目的:	或取某列的数据，如果失败会纪录错误log
	// 参数:	列号, 空格是否合法
	// 返回值:	对应整数数据
	int 		SafeGetInt( int iCol,bool booCanBeNull=false );

	// 目的:	或取某列的数据，如果失败会纪录错误log
	// 参数:	列号, 空格是否合法
	// 返回值:	字符串数据
	const char * FastGetStr( int iCol,bool booCanBeNull=false );

	// 目的:	或取某列的数据，如果失败会纪录错误log
	// 参数:	列号，字符串数据
	// 返回值:	无
	void		SafeGetStr( int iCol,string &szValue,bool booCanBeNull=false );

	// 目的:	或取某列的数据，如果失败会纪录错误log
	// 参数:	列号
	// 返回值:	对应浮点数据
	float 		SafeGetFloat( int iCol,bool booCanBeNull=false );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号,项号
	// 返回值:	对应整数数据
	int 		SafeGetArrayInt( int iCol,int idx,bool booCanBeNull=false );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号,项号
	// 返回值:	字符串数据
	const char * FastGetArrayStr( int iCol,int idx,bool booCanBeNull=false );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号,项号，字符串数据
	// 返回值:	无
	void		SafeGetArrayStr( int iCol,int idx,string &szValue,bool booCanBeNull=false );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号,项号
	// 返回值:	对应浮点数据	
	float 		SafeGetArrayFloat( int iCol,int idx,bool booCanBeNull=false );

	// 目的:	返回复合单元值的组成部分总数(对应项采用*分割）
	// 参数:	列号
	// 返回值:	项数
	int			GetArraySize( int iCol );

	// 目的:	或取某列(对应项采用*分割），第某个数据，如果失败会纪录错误log
	// 参数:	列号, 空格是否合法
	// 返回值:	对应整数数据
	int 		SafeGetRandInt( int iCol,bool booCanBeNull=false );


	void		SafeGetArray( int iCol,vector<int> &Output,bool booCanBeNull=false );
	void 		SafeGetArray( int iCol,vector<const char *> &Output,bool booCanBeNull=false );
	void		SafeGetArray( int iCol,vector<float> &Output,bool booCanBeNull=false );

	////////////////////////////////////////////////////////////////
	// 目的:	从复合单元值中随机抽取一部分
	int			RandGetData( int iCol,int &iData );
	int 		RandGetData( int iCol,string &szData );
	int 		RandGetData( int iCol,float &fData );

	// 目的:	在（几率*数据*几率*数据*...）的格式下随机获取一个数据(几率之合最大为100)
	// 参数:	列号，接受的数据，接受是第几个数据
	// 返回值:	RE_
	int			GetRand100( int iCol,int &iData,int &iDataIdx );
	int			GetRand100( int iCol,string &szData,int &iDataIdx );

	int			GetRandx( int iCol,int &iData,int &iDataIdx,int iRate );
	int			GetRandx( int iCol,string &szData,int &iDataIdx,int iRate );

	// 目的:	判断复合单元中是否有iData
	bool		IsData( int iCol,int iData );

	////////////////////////////////////////////////////////////////
	// 目的:	判断Cell是否为空, iCol为列号
	bool		IsNull( int iCol );

	////////////////////////////////////////////////////////////////
	// 目的:	保存父表
	void		SetParentTable(CExcelTable* ParentTable);
	// 目的:	保存行号
	void        SetLineID( unsigned short iLn );
	// 目的:	获取行号
	int			GetLineID();

protected:
	Excel_Cell * CreateCell( Byte bDef,const char * szValue );

	void		GetCellStr( Excel_Cell * pCell,Byte bDef,string &szValue );

	void		ErrorLogLineCol( const char * szError,int iCol,int iRE,int idx = 0 );
public:
	vector<Excel_Cell *> m_vCells;//单元值

protected:
	CExcelTable *	m_ParentTable;
	unsigned short	m_iLineID;//行号
};

#endif

//************************************//
//              公用函数              //
//************************************//
// 对字符串的操作
void GetFromString( const char * srcString, vector<Byte> &Output, char szSeparator ='*' );
void GetFromString( const char * srcString, vector<int> &Output, char szSeparator ='*' );
void GetFromString( const char * srcString, vector<long> &Output, char szSeparator ='*' );
void GetFromString( const char * srcString, vector<const char *> &Output, char szSeparator ='*' );
void GetFromString( const char * srcString, vector<float> &Output, char szSeparator ='*' );
bool FindInString( string &srcString, const char * szString, char szSeparator ='*' );
void RemFromString( string &srcString, const char * szString, char szSeparator ='*' );
void AddToString( string &srcString, const char * szString, char szSeparator ='*' );

#endif //__SOLOFILEFUNC_H__
