// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whzlib
// File: whzlib_pck.h
// Creator: Wei Hua (魏华)
// Comment: 和具体打包相关的功能
// CreationDate: 2003-09-04

#ifndef	__WHZLIB_PCK_H__
#define __WHZLIB_PCK_H__

#include "whzlib.h"

namespace n_whzlib
{

////////////////////////////////////////////////////////////////////
// 文件打包编辑相关
////////////////////////////////////////////////////////////////////
// 用所模式
enum
{
	COMPRESSMODE_NOTHING	= 0,
	COMPRESSMODE_ZIP		= 1,
};

// 一个包修改对象
class	whzlib_pck_modifier
{
public:
	struct	CREATEINFO_T
	{
		const char	*szPckFile;					// 文件名指针(这个文件名是不带后缀的，它对应那三个文件)
		int			nUnitSize;					// 单个数据块压缩前的最大长度。
												// 这个数不能太小，否则不利于压缩。
		CREATEINFO_T()
		: szPckFile(NULL)
		, nUnitSize(WHZLIB_MAX_RAWDATAUNIT_SIZE)
		{
		}
	};
	struct	OPENINFO_T
	{
		const char	*szPckFile;					// 文件名指针(这个文件名是不带后缀的，它对应那三个文件)
		OPENINFO_T()
		: szPckFile(NULL)
		{
		}
	};
	// 创建新文件
	// 返回和文件关联的对象指针
	static whzlib_pck_modifier *	Create(CREATEINFO_T *pInfo);
	// 创建一个空包并关闭
	static int						CreateEmptyPck(CREATEINFO_T *pInfo);
	// 以修改方式打开文件
	// 返回和文件关联的对象指针
	static whzlib_pck_modifier *	Open(OPENINFO_T *pInfo);
	// 销毁对象(关闭所有相关文件)
	virtual ~whzlib_pck_modifier()	{}
public:
	// 注意：这4个不要和下面3个AppendXXX函数混用，否则出现结果不可预料
	// 注意：下面的szFileInPck文件名都是带全相对路径，即相对于包的根的路径
	// 添加一个目录(如果父目录不存就自动创建各级父目录)
	// 如果目录已经存在则报错
	virtual int	AddDir(const char *szDirInPck)
												= 0;
	// 添加一个文件(nCompressMode表示如何压缩，取值为COMPRESSMODE_XXX)
	// 如果文件已经存在就报错
	// 如果文件所在目录不存在则自动创建各级父目录
	virtual int	AddFile(const char *szFileInPck, whzlib_file *file, int nCompressMode)
												= 0;
	// 替换文件
	// 如果文件不存在就出错
	virtual int	ReplaceFile(const char *szFileInPck, whzlib_file *file, int nCompressMode)
												= 0;
	// 删除一个目录(包括旗下的所有文件及子目录)或者文件
	virtual int	DelPath(const char *szPathInPck)
												= 0;

	// 注意：这3个不要和上面4个函数混用，否则出现结果不可预料
	// 外界知道目录的访问顺序，直接添加
	// 这里面的szDir和szFile都是相对于父目录的文件名
	// !!!!注意：只有Create出来的文件才能这样操作!!!!
	virtual int	AppendDir(const char *szDir)
												= 0;
	virtual int	AppendFile(const char *szFile, whzlib_file *file, int nCompressMode)
												= 0;
	virtual int	AppendDirEnd()					= 0;

	// 打开并获得目录对象(使用完毕直接删除即可)
	virtual n_whcmn::WHDirBase *	OpenDir(int nDirID)
												= 0;
	virtual n_whcmn::WHDirBase *	OpenDir(const char *szDir)
												= 0;
	// 设置解压密码(这个必须在Creat或Open之后调用)
	virtual void	SetPassword(const char *szPassword)
												= 0;
	// 将内存内容flush到硬盘上
	virtual int		Flush()						= 0;
};

// 这些notify主要是遍历目录过程中出现的(欢迎添加)
enum
{
	WHZLIB_PCK_NOTIFY_ERR			= 0,		// 通告错误
												// 命令结构WHZLIB_PCK_NOTIFY_ERR_T
	WHZLIB_PCK_NOTIFY_FILE			= 1,		// 正在处理的文件
												// 命令结构WHZLIB_PCK_NOTIFY_STR_T
	WHZLIB_PCK_NOTIFY_FILEDONE		= 2,		// 文件处理完毕
												// 命令结构WHZLIB_PCK_NOTIFY_STR_T
	WHZLIB_PCK_NOTIFY_ENTERDIR		= 11,		// 进入目录
												// 命令结构WHZLIB_PCK_NOTIFY_STR_T
	WHZLIB_PCK_NOTIFY_LEAVEDIR		= 12,		// 离开目录
												// 命令结构WHZLIB_PCK_NOTIFY_STR_T
	WHZLIB_PCK_NOTIFY_TOTALNUM		= 20,		// 报告总共的文件数
												// 命令结构WHZLIB_PCK_NOTIFY_INT_T
};
enum
{
	WHZLIB_PCK_NOTIFY_ERR_ATTRIBWARNING	= -2,	// 文件属性警告
	WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN	= -1,	// 无法打开文件
	WHZLIB_PCK_NOTIFY_ERR_KNOWN			= 0,	// 未知错误
	WHZLIB_PCK_NOTIFY_NOERR_WORKDONE	= 1,	// 一段工作结束
};
#pragma pack(1)
struct	WHZLIB_PCK_NOTIFY_STR_T
{
	const char	*szStr;							// 一般是指向文件或目录的全名的指针(相对于包文件根)
};
struct	WHZLIB_PCK_NOTIFY_INT_T
{
	int			nInt;
};
struct	WHZLIB_PCK_NOTIFY_ERR_T
{
	int			nErr;							// 错误代码
	const char	*szErr;							// 错误字串
};
#pragma pack()
// 用于在整理或压缩时输出进度之类的(主要处理目录遍历类操作)
class	whzlib_pck_notify
{
public:
	virtual ~whzlib_pck_notify()				{}
	// 通告当前处理情况
	// nCmd定义见上面的WHZLIB_PCK_NOTIFY_XXXX
	virtual void	Notify(int nCmd, void *pData, int nSize)
												= 0;
	virtual void	NotifyStr(int nCmd, const char *szStr)
												= 0;
	virtual void	NotifyInt(int nCmd, int nInt)
												= 0;
	virtual void	NotifyErr(int nCmd, int nErr, const char *szErr)
												= 0;
	// 子类继承这个可以选择在操作过程中退出
	virtual bool	ShouldQuit() const			{return false;}
};
class	whzlib_pck_notify_console	: public whzlib_pck_notify
{
public:
	struct	STATINFO_T
	{
		int		nTotalFile;			// 总文件数
		int		nTotalDir;			// 总目录数
		void	clear()
		{
			memset(this, 0, sizeof(*this));
		}
		STATINFO_T()
		: nTotalFile(0), nTotalDir(0)
		{ }
	};
private:
	int		m_nDirIndent;			// 用于让同级目录更明显
	int		m_nIndentStep;			// 每次缩进距离
	STATINFO_T	m_statinfo;			// 统计信息
private:
	int		myindentprintf(const char *szFmt, ...);
public:
	whzlib_pck_notify_console();
	// 通告当前处理情况
	void	Notify(int nCmd, void *pData, int nSize);
	void	NotifyStr(int nCmd, const char *szStr);
	void	NotifyInt(int nCmd, int nInt);
	void	NotifyErr(int nCmd, int nErr, const char *szErr);
	// 重置统计信息
	void	ResetStat()
	{
		m_statinfo.clear();
	}
	// 处理信息统计
	inline const STATINFO_T *	GetTotalStatInfo() const
	{
		return	&m_statinfo;
	}
};

// 对打包文件重新整理
int	whzlib_pck_tidy(const char *szOldPckFile, const char *szNewPckFile, whzlib_pck_notify *pNotify = NULL);

// 压缩一个目录到一个文件
int	whzlib_pck_make(const char *szDir, const char *szPckFile, const char *szPassword = NULL, whzlib_pck_notify *pNotify = NULL);
// 解压文件到一个目录
int	whzlib_pck_extract(const char *szPckFile, const char *szDir, const char *szPassword = NULL, whzlib_pck_notify *pNotify = NULL);

// 用一个包patch一个目录
// aszPckList中是一个以NULL结尾的常数字串数组，表示那些目录一定是包
int	whzlib_pck_patch(const char *szPckFile, const char **aszPckList, const char *szDstDir, int nManProperty = whzlib_filemodifyman::PROPERTY_AUTOCREATEPCK, const char *szPassword = NULL, whzlib_pck_notify *pNotify = NULL);

int	whzlib_pck_checkver(const char *szPckFile,const char *szDstDir,const char *szPassword,whzlib_pck_notify *pNotify);

}		// EOF namespace n_whzlib

#endif	// EOF __WHZLIB_PCK_H__
