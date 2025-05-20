// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace: n_whzlib
// File: whzlib_pck.h
// Creator: Wei Hua (κ��)
// Comment: �;�������صĹ���
// CreationDate: 2003-09-04

#ifndef	__WHZLIB_PCK_H__
#define __WHZLIB_PCK_H__

#include "whzlib.h"

namespace n_whzlib
{

////////////////////////////////////////////////////////////////////
// �ļ�����༭���
////////////////////////////////////////////////////////////////////
// ����ģʽ
enum
{
	COMPRESSMODE_NOTHING	= 0,
	COMPRESSMODE_ZIP		= 1,
};

// һ�����޸Ķ���
class	whzlib_pck_modifier
{
public:
	struct	CREATEINFO_T
	{
		const char	*szPckFile;					// �ļ���ָ��(����ļ����ǲ�����׺�ģ�����Ӧ�������ļ�)
		int			nUnitSize;					// �������ݿ�ѹ��ǰ����󳤶ȡ�
												// ���������̫С����������ѹ����
		CREATEINFO_T()
		: szPckFile(NULL)
		, nUnitSize(WHZLIB_MAX_RAWDATAUNIT_SIZE)
		{
		}
	};
	struct	OPENINFO_T
	{
		const char	*szPckFile;					// �ļ���ָ��(����ļ����ǲ�����׺�ģ�����Ӧ�������ļ�)
		OPENINFO_T()
		: szPckFile(NULL)
		{
		}
	};
	// �������ļ�
	// ���غ��ļ������Ķ���ָ��
	static whzlib_pck_modifier *	Create(CREATEINFO_T *pInfo);
	// ����һ���հ����ر�
	static int						CreateEmptyPck(CREATEINFO_T *pInfo);
	// ���޸ķ�ʽ���ļ�
	// ���غ��ļ������Ķ���ָ��
	static whzlib_pck_modifier *	Open(OPENINFO_T *pInfo);
	// ���ٶ���(�ر���������ļ�)
	virtual ~whzlib_pck_modifier()	{}
public:
	// ע�⣺��4����Ҫ������3��AppendXXX�������ã�������ֽ������Ԥ��
	// ע�⣺�����szFileInPck�ļ������Ǵ�ȫ���·����������ڰ��ĸ���·��
	// ���һ��Ŀ¼(�����Ŀ¼������Զ�����������Ŀ¼)
	// ���Ŀ¼�Ѿ������򱨴�
	virtual int	AddDir(const char *szDirInPck)
												= 0;
	// ���һ���ļ�(nCompressMode��ʾ���ѹ����ȡֵΪCOMPRESSMODE_XXX)
	// ����ļ��Ѿ����ھͱ���
	// ����ļ�����Ŀ¼���������Զ�����������Ŀ¼
	virtual int	AddFile(const char *szFileInPck, whzlib_file *file, int nCompressMode)
												= 0;
	// �滻�ļ�
	// ����ļ������ھͳ���
	virtual int	ReplaceFile(const char *szFileInPck, whzlib_file *file, int nCompressMode)
												= 0;
	// ɾ��һ��Ŀ¼(�������µ������ļ�����Ŀ¼)�����ļ�
	virtual int	DelPath(const char *szPathInPck)
												= 0;

	// ע�⣺��3����Ҫ������4���������ã�������ֽ������Ԥ��
	// ���֪��Ŀ¼�ķ���˳��ֱ�����
	// �������szDir��szFile��������ڸ�Ŀ¼���ļ���
	// !!!!ע�⣺ֻ��Create�������ļ�������������!!!!
	virtual int	AppendDir(const char *szDir)
												= 0;
	virtual int	AppendFile(const char *szFile, whzlib_file *file, int nCompressMode)
												= 0;
	virtual int	AppendDirEnd()					= 0;

	// �򿪲����Ŀ¼����(ʹ�����ֱ��ɾ������)
	virtual n_whcmn::WHDirBase *	OpenDir(int nDirID)
												= 0;
	virtual n_whcmn::WHDirBase *	OpenDir(const char *szDir)
												= 0;
	// ���ý�ѹ����(���������Creat��Open֮�����)
	virtual void	SetPassword(const char *szPassword)
												= 0;
	// ���ڴ�����flush��Ӳ����
	virtual int		Flush()						= 0;
};

// ��Щnotify��Ҫ�Ǳ���Ŀ¼�����г��ֵ�(��ӭ���)
enum
{
	WHZLIB_PCK_NOTIFY_ERR			= 0,		// ͨ�����
												// ����ṹWHZLIB_PCK_NOTIFY_ERR_T
	WHZLIB_PCK_NOTIFY_FILE			= 1,		// ���ڴ�����ļ�
												// ����ṹWHZLIB_PCK_NOTIFY_STR_T
	WHZLIB_PCK_NOTIFY_FILEDONE		= 2,		// �ļ��������
												// ����ṹWHZLIB_PCK_NOTIFY_STR_T
	WHZLIB_PCK_NOTIFY_ENTERDIR		= 11,		// ����Ŀ¼
												// ����ṹWHZLIB_PCK_NOTIFY_STR_T
	WHZLIB_PCK_NOTIFY_LEAVEDIR		= 12,		// �뿪Ŀ¼
												// ����ṹWHZLIB_PCK_NOTIFY_STR_T
	WHZLIB_PCK_NOTIFY_TOTALNUM		= 20,		// �����ܹ����ļ���
												// ����ṹWHZLIB_PCK_NOTIFY_INT_T
};
enum
{
	WHZLIB_PCK_NOTIFY_ERR_ATTRIBWARNING	= -2,	// �ļ����Ծ���
	WHZLIB_PCK_NOTIFY_ERR_CANNOTOPEN	= -1,	// �޷����ļ�
	WHZLIB_PCK_NOTIFY_ERR_KNOWN			= 0,	// δ֪����
	WHZLIB_PCK_NOTIFY_NOERR_WORKDONE	= 1,	// һ�ι�������
};
#pragma pack(1)
struct	WHZLIB_PCK_NOTIFY_STR_T
{
	const char	*szStr;							// һ����ָ���ļ���Ŀ¼��ȫ����ָ��(����ڰ��ļ���)
};
struct	WHZLIB_PCK_NOTIFY_INT_T
{
	int			nInt;
};
struct	WHZLIB_PCK_NOTIFY_ERR_T
{
	int			nErr;							// �������
	const char	*szErr;							// �����ִ�
};
#pragma pack()
// �����������ѹ��ʱ�������֮���(��Ҫ����Ŀ¼���������)
class	whzlib_pck_notify
{
public:
	virtual ~whzlib_pck_notify()				{}
	// ͨ�浱ǰ�������
	// nCmd����������WHZLIB_PCK_NOTIFY_XXXX
	virtual void	Notify(int nCmd, void *pData, int nSize)
												= 0;
	virtual void	NotifyStr(int nCmd, const char *szStr)
												= 0;
	virtual void	NotifyInt(int nCmd, int nInt)
												= 0;
	virtual void	NotifyErr(int nCmd, int nErr, const char *szErr)
												= 0;
	// ����̳��������ѡ���ڲ����������˳�
	virtual bool	ShouldQuit() const			{return false;}
};
class	whzlib_pck_notify_console	: public whzlib_pck_notify
{
public:
	struct	STATINFO_T
	{
		int		nTotalFile;			// ���ļ���
		int		nTotalDir;			// ��Ŀ¼��
		void	clear()
		{
			memset(this, 0, sizeof(*this));
		}
		STATINFO_T()
		: nTotalFile(0), nTotalDir(0)
		{ }
	};
private:
	int		m_nDirIndent;			// ������ͬ��Ŀ¼������
	int		m_nIndentStep;			// ÿ����������
	STATINFO_T	m_statinfo;			// ͳ����Ϣ
private:
	int		myindentprintf(const char *szFmt, ...);
public:
	whzlib_pck_notify_console();
	// ͨ�浱ǰ�������
	void	Notify(int nCmd, void *pData, int nSize);
	void	NotifyStr(int nCmd, const char *szStr);
	void	NotifyInt(int nCmd, int nInt);
	void	NotifyErr(int nCmd, int nErr, const char *szErr);
	// ����ͳ����Ϣ
	void	ResetStat()
	{
		m_statinfo.clear();
	}
	// ������Ϣͳ��
	inline const STATINFO_T *	GetTotalStatInfo() const
	{
		return	&m_statinfo;
	}
};

// �Դ���ļ���������
int	whzlib_pck_tidy(const char *szOldPckFile, const char *szNewPckFile, whzlib_pck_notify *pNotify = NULL);

// ѹ��һ��Ŀ¼��һ���ļ�
int	whzlib_pck_make(const char *szDir, const char *szPckFile, const char *szPassword = NULL, whzlib_pck_notify *pNotify = NULL);
// ��ѹ�ļ���һ��Ŀ¼
int	whzlib_pck_extract(const char *szPckFile, const char *szDir, const char *szPassword = NULL, whzlib_pck_notify *pNotify = NULL);

// ��һ����patchһ��Ŀ¼
// aszPckList����һ����NULL��β�ĳ����ִ����飬��ʾ��ЩĿ¼һ���ǰ�
int	whzlib_pck_patch(const char *szPckFile, const char **aszPckList, const char *szDstDir, int nManProperty = whzlib_filemodifyman::PROPERTY_AUTOCREATEPCK, const char *szPassword = NULL, whzlib_pck_notify *pNotify = NULL);

int	whzlib_pck_checkver(const char *szPckFile,const char *szDstDir,const char *szPassword,whzlib_pck_notify *pNotify);

}		// EOF namespace n_whzlib

#endif	// EOF __WHZLIB_PCK_H__
