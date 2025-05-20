// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_util.h
// Creator     : Wei Hua (κ��)
// Comment     : ������ز���
// CreationDate: 2006-08-14
//             : 2007-01-24 ������CP_INFO_T�е��ļ���׺��
//             : �����˶�������Ŀ¼�ĵ��ļ�ѹ���ͽ�ѹ���ܡ�
//             : 2007-02-01 ������cb_MSGOUT�ص�����
//             : 2007-08-08 ������apszSkipFile�������������ض����ļ�
//             : 2007-08-09 ������pszBadFileFilter��������������ͨ���ƥ����ļ�
//			   : 2008-08-22	Ϊ�ֶ����°����Ӱ汾��֤���ƣ�by ���Ľ�

#ifndef	__WH_PACKAGE_UTIL_H__
#define	__WH_PACKAGE_UTIL_H__

#include "whfile_base.h"
#include "whcmn_def.h"
#include "wh_compress_interface.h"
#include "wh_crypt_interface.h"
#include "wh_package_def.h"

namespace n_whcmn
{

// ���߼��ϣ��ӿڣ�
struct	wh_package_util
{
	// һЩ���ö���
	enum
	{
		PROP_SKIPSYSTEMFILE		= 0x01,								// ����ϵͳ�ļ�����
		PROP_SKIPRDONLYFILE		= 0x02,								// ����ֻ���ļ�����
		PROP_KEEPFILETIME		= 0x04,								// �������ļ�ʱ��
	};
	enum
	{
		GETSRCDIRSTATMODE_NO	= 0x00,								// ����ȡĿ¼��Ϣ
		GETSRCDIRSTATMODE_QUICK	= 0x01,								// ���ٻ�ȡ
		GETSRCDIRSTATMODE_SLOW	= 0x02,								// ���ٻ�ȡ����Ҫ��Ĵ��ļ���
	};
	// notify�����������ͽ����֪ͨ����ʾ������
	struct	notify
	{
	public:
		virtual	~notify()		{}
		virtual	void	SelfDestroy()								= 0;
		// ������������ķ���
		enum
		{
			RST_OK_CONTINUE				= 0,						// �ɹ������Լ������������
			RST_ERR_ABORT				= -1,						// �д���ֹͣ����Ĺ���
		};
		virtual	int		cb_DIR_BEGIN(const char *cszDirName)		= 0;
		virtual	int		cb_DIR_END(const char *cszDirName)			= 0;
		virtual	int		cb_FILE_BEGIN(const char *cszFrom, const char *cszTo, int nFileSize)
																	= 0;
		virtual	int		cb_FILE_END()								= 0;
		virtual	int		cb_FILE_PROCESS(int nCurPos)				= 0;
																	// nCurPos�ǵ�ǰ�������ֽ���
		virtual	int		cb_FILE_SKIP(const char *cszFName)			= 0;
																	// ����ĳ���ļ�
		enum
		{
			ERRCODE_OK					= 0,						// û��:)
			ERRCODE_F_OPEN				= -1,						// ���ļ�����
			ERRCODE_F_CREATE			= -2,						// �����ļ�����
			ERRCODE_F_READ				= -3,						// ��ȡ�ļ�����
			ERRCODE_F_WRITE				= -4,						// д���ļ�����
			ERRCODE_COUNTTOTAL_ERR		= -5,						// ͳ���ܳߴ���̳���
			ERRCODE_NO_DISKSPACE		= -100,						// д���ļ�����
			ERRCODE_OTHER				= -1000,					// ����δ֪���󣬾���������ִ�
		};
		virtual	int		cb_ERROR(int nErrCode, const char *cszErrString)
																	= 0;
																	// �������
		virtual int		cb_MSGOUT(int nMsgCode, const char *cszMsg)	= 0;
																	// �����Ϣ
		virtual int		cb_SRCDIRSTAT(const DIR_STAT_T *pInfo)		{return 0;}
																	// ��ȡԴĿ¼��ͳ����Ϣ
	public:
		static notify	*CreateCmnConsole();						// ����һ����ͨ�Ļ���Console��֪ͨ����ʹ����Ҳ������������notify��
	};
	// �����������������Ĳ����ṹ
	struct	CP_INFO_T
	{
		whfileman		*pFM;										// �ײ���ļ������������ļ��Ĳ�����Ҫͨ�������У�
		const char		*cszSrc;									// ԴĿ¼
		const char		*cszDst;									// Ŀ��Ŀ¼
		int				nCutFileTail;								// �����0���Ŀ���ļ���βɾ����ô���ֽ�
		const char		*cszAppendExt;								// ����ǿ�����Ŀ���ļ�������������׺
		const char		*szPass;
		int				nPassLen;
		int				nBlockSize;									// �����ļ���Ĵ�С��Խ�����С�ļ��˷�Խ��ԽС���ڴ��ļ��˷�Խ��
		// ����ɱ���Ҫ��д�ģ�����û���������ļ�
		WHCompress					*pWHCompress;					// ѹ����
		ICryptFactory::ICryptor		*pEncryptor;					// ������
		ICryptFactory::ICryptor		*pDecryptor;					// ��������ע�⣺�������ͽ������ɲ���һ����
		notify			*pNotify;									// ���ڶ�����ʾ�Ķ���
		DIR_STAT_T::notify			*pStatNotify;					// ͳ��Ŀ¼ʱ�Ķ�����ʾ����
		int				nNotifyStepBytes;							// ÿ������ô���ֽھ�֪ͨһ��
		unsigned char	nProp;										// ��dircp::INFO_T�е�PROP_XXX����һ��
		unsigned char	nGetSrcDirStatMode;							// �Ƿ���Ҫ���ԴĿ¼��ͳ����Ϣ
		bool			*pbShouldStop;								// �������������������Ϊ���򿽱�������Ҫֹͣ
		const char		**apszSkipFile;								// �����ļ����б�
		const char		*pszBadFileFilter;							// ��Ҫ�����ļ���ͨ�������".h|.cpp"�����ĸ�ʽ
		const char		*pValidityStr;								// ��֤�ִ�

		CP_INFO_T()
		{
			WHMEMSET0THIS();
			nNotifyStepBytes	= 100000;
			nBlockSize			= WHPACKAGE_DFT_BLOCKSIZE;
			nGetSrcDirStatMode	= GETSRCDIRSTATMODE_QUICK;
		}
	};
	// Ŀ¼���
	struct	dir2package
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~dir2package()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// �ɻ�
																	// ����Ŀ¼�������
	
		static dir2package *	Create();							// ����ʵ��
	};
	// �����Ŀ¼
	struct	package2dir
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~package2dir()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// �ɻ�
																	// ����Ŀ¼�������

		static package2dir *	Create();							// ����ʵ��
	};
	// Ŀ¼�ĵ��ļ�ѹ��
	struct	sf_compress
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~sf_compress()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// �ɻ�
		// ����Ŀ¼�������

		static sf_compress *	Create();							// ����ʵ��
	};
	// Ŀ¼�ĵ��ļ���ѹ
	struct	sf_decompress
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~sf_decompress()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// �ɻ�
		// ����Ŀ¼�������

		static sf_decompress *	Create();							// ����ʵ��
	};
	// ��һ��Ŀ¼��������һ��Ŀ¼�����֮���ԺͿ�����ͬ����Ϊ������һЩ�ű�Ҫִ�У�
	struct	update
	{
		typedef	wh_package_util::CP_INFO_T		INFO_T;
		virtual	~update()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// �ɻ�
		// ����Ŀ¼�������

		static update *			Create();							// ����ʵ��
	};
	// �������ļ�ϵͳ�俽��Ŀ¼���ļ�������������漸��Ӧ�õĵײ�ʵ�֣�
	struct	dircp
	{
		struct	INFO_T
		{
			whfileman		*pFMSrc;								// �ļ��������ϵͳ
			whfileman		*pFMDst;								// �ļ����������ϵͳ
			const char		*cszSrcDir;								// ԴĿ¼
			const char		*cszDstDir;								// Ŀ��Ŀ¼
			int				nCutFileTail;							// �����0���Ŀ���ļ���βɾ����ô���ֽ�
			const char		*cszAppendExt;							// ����ǿ�����Ŀ���ļ�������������׺
			notify			*pNotify;								// ���ϲ��֪ͨ��
			DIR_STAT_T::notify		*pStatNotify;					// ͳ��Ŀ¼ʱ�Ķ�����ʾ����
			int				nNotifyStepBytes;						// ÿ������ô���ֽھ�֪ͨһ��
			unsigned char	nProp;									// һЩ����
			unsigned char	nGetSrcDirStatMode;						// �Ƿ���Ҫ���ԴĿ¼��ͳ����Ϣ
			bool			*pbShouldStop;
			const char		**apszSkipFile;							// �����ļ����б�
			const char		*pszBadFileFilter;						// ��Ҫ�����ļ���ͨ�������".h|.cpp"�����ĸ�ʽ
			INFO_T()
			{
				WHMEMSET0THIS();
				nNotifyStepBytes	= 100000;
				nGetSrcDirStatMode	= GETSRCDIRSTATMODE_QUICK;
			}
			void	getcmninfofrom(CP_INFO_T *pInfo)
			{
				cszDstDir			= pInfo->cszDst;
				nCutFileTail		= pInfo->nCutFileTail;
				cszAppendExt		= pInfo->cszAppendExt;
				pNotify				= pInfo->pNotify;
				pStatNotify			= pInfo->pStatNotify;
				nNotifyStepBytes	= pInfo->nNotifyStepBytes;
				nProp				= pInfo->nProp;
				nGetSrcDirStatMode	= pInfo->nGetSrcDirStatMode;
				pbShouldStop		= pInfo->pbShouldStop;
				apszSkipFile		= pInfo->apszSkipFile;
				pszBadFileFilter	= pInfo->pszBadFileFilter;
			}
		};
		virtual	~dircp()	{}
		virtual	void		SelfDestroy()		= 0;
		virtual	int			DoIt(INFO_T *pInfo)	= 0;				// �ɻ�
		static dircp *		Create();								// ����ʵ��
	};
};

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_UTIL_H__
