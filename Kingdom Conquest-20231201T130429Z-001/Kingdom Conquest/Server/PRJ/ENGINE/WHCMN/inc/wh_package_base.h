// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace   : n_whcmn
// File        : wh_package_base.h
// Creator     : Wei Hua (κ��)
// Comment     : �����İ��ṹ
// CreationDate: 2006-07-18
// ChangeLOG   : 2006-07-20 ȡ����BLOCK_HDR_T�е�nBlocknumOrSize�����ļ��ĵ�һ���������˸������ݣ��������Լ��ٺ�������˷ѵ����ݡ�
//             : 2006-07-21 ȡ����m_nLastBlockDataMaxSize��m_nLastBlockDataSize��m_nCurOffsetInBlock����Щ����ֱ�Ӽ������������Ϻܶ�ʱ�䡣
//             : 2006-07-31 ������rawfile���ļ�ʱ�书��
//             : 2007-03-27 ȡ����m_setOpenedFileStartΪ���ܹ���һ���ļ��򿪶�Σ���ȷ����������
//			   : 2008-06-06	����һ���ӿڣ�����ֱ�Ӷ�ȡԭʼ��ѹ�����ݣ�by ���Ľܣ�

#ifndef	__WH_PACKAGE_BASE_H__
#define __WH_PACKAGE_BASE_H__

#include "whcmn_def.h"
#include "whfile_base.h"
#include "wh_compress_interface.h"
#include "wh_crypt_interface.h"
#include "whstring.h"
#include "whhash.h"
#include "wh_package_singlefile.h"
#include "wh_package_def.h"

#define FILEINPACKAGE_NOCASE

namespace n_whcmn
{

// �򵥵Ŀ��ļ�
// 
class	wh_package_base
{
public:
	typedef	int			blockoffset_t;								// ��ƫ�ƣ������ļ��ĵ�ƫ���������ǿ�����
																	// ���ﵥ���������ֻ��Ϊ���½����ĳɸ�����ļ�������Ҫ��Ҳ�Ƚ����ѣ���Ϊ�ܶ�������������int������seek�Ĳ���

#ifdef	FILEINPACKAGE_NOCASE										// �԰��е��ļ������ִ�Сд
	typedef	whstrinvector4hashNocase			STRINVECT;
	typedef	_whstr_hashfuncNocase				STRHASHFUNC;
#else																// �԰��е��ļ����ִ�Сд
	typedef	whstrinvector4hash					STRINVECT;
	typedef	_whstr_hashfunc						STRHASHFUNC;
#endif
	typedef	whhashset<STRINVECT, whcmnallocationobj, STRHASHFUNC>	STRSET_T;

	struct	INFO_T
	{
		whfileman::OP_MODE_T	nOPMode;							// �򿪰��ķ�ʽ����ʱ��Ҳ����Ҫֻ���򿪵ģ�������Ϸ�е�ʱ��
		whfile					*pBaseFile;							// ����ײ���ļ���Ҳ������һ����ڴ��ɵ��ڴ��ļ���
		WHCompress				*pWHCompress;						// ѹ����
		ICryptFactory::ICryptor	*pEncryptor;						// ������
		ICryptFactory::ICryptor	*pDecryptor;						// ��������ע�⣺�������ͽ������ɲ���һ����
		int						nBlockSize;							// �����ļ���Ĵ�С��Խ�����С�ļ��˷�Խ��ԽС���ڴ��ļ��˷�Խ��
		char					szPass[128];						// ���루���û���������ĳ���Ϊ0��
		int						nPassLen;							// ���볤��
		char					szDir[WH_MAX_PATH];					// �Լ������Ŀ¼��
		bool					bAutoCloseBaseFile;					// �Ƿ��Զ��رյײ�����ļ�
		bool					bCheckFileBlockNumOnOpen;			// �Ƿ��ڴ��ļ���ʱ������ļ������Ƿ���������Ϊ�ڳ����쳣��ʱ��ܿ��ܵ����ļ�û��д�룬�����ļ����Ѿ��в��ָ����ˣ�����Ϊ���ܹ������ܴ��ļ��Ͳ��ü���ˣ�
		bool					bNoErrOnDelFile;					// ɾ���ļ��������ͺ��ԣ����ܻ��˷�һЩ�ռ䣩

		INFO_T()
			: nOPMode(whfileman::OP_MODE_BIN_READONLY)
			, pBaseFile(NULL)
			, pWHCompress(NULL)
			, pEncryptor(NULL), pDecryptor(NULL)
			, nBlockSize(WHPACKAGE_DFT_BLOCKSIZE)
			, nPassLen(0)
			, bAutoCloseBaseFile(false)
			, bCheckFileBlockNumOnOpen(false)
			, bNoErrOnDelFile(true)
		{
			memset(szDir, 0, sizeof(szDir));
		}
		// ����������ƽṹ�л�ȡ�Լ���Ҫ����Ϣ
		template<typename _TyOther>
		void	ReadFromOther(const _TyOther &other)
		{
			// pBaseFile����ͨ�����ݣ����ܱ��˲��ṩ�����ԾͲ�Ҫд��
			nOPMode				= other.nOPMode;
			pWHCompress			= other.pWHCompress;
			pEncryptor			= other.pEncryptor;
			pDecryptor			= other.pDecryptor;
			nBlockSize			= other.nBlockSize;
			memcpy(szPass, other.szPass, other.nPassLen);
			nPassLen			= other.nPassLen;
		}
	};
	#pragma pack(1)
	struct	PACKAGE_HDR_T 
	{
		enum
		{
			VER			= 2
		};
		char			szMagic[16];								// ħ���������ڱ�ʶ�ļ�����
		int				nVer;										// �汾
		int				nBlockSize;									// �����ļ���Ĵ�С��Խ�����С�ļ��˷�Խ��ԽС���ڴ��ļ��˷�Խ��
		blockoffset_t	nAvailIdx;									// ���ÿ������ͷ
		blockoffset_t	nMaxBlockIdx;								// ���һ�����ÿ����+1��Ҳ���ǿ����·���Ŀ��ÿ���ţ�
																	// ����ڴ��ļ���ʱ����ܸ����ļ����������������ļ����Ȳ��ǿ鳤�������������߼�����Ŀ�������������ϣ�������ļ����ˡ�
		unsigned char	PassMD5[16];								// ���������MD5У���
		char			junk[16+64];								// ����ǰ��չ�128�ֽ��������Ժ�����
		enum
		{
			RF_IDX_GOD		= 0,									// ���λ���ȱ�����û�����ʲô��
			RF_IDX_FINFO	= 1,									// �ļ���Ϣ��
			RF_IDX_FNAME	= 2,									// �ļ�����
			RF_IDX_TOTAL	= 8										// ����
		};
		blockoffset_t	anRawfile[RF_IDX_TOTAL];					// ���ڴ洢��Ҫ��RAW�����ļ�����Ϣ

		static const char	*CSZMAGIC;

		void	reset()
		{
			WHMEMSET0THIS();
			nVer			= VER;
			strcpy(szMagic, CSZMAGIC);
			nMaxBlockIdx	= 1;
		}
		bool	IsOK() const										// �жϴ��ļ��ж�����ͷ�Ƿ���ȷ
		{
			return	nVer == VER && strcmp(szMagic, CSZMAGIC)==0;
		}
		void	print(FILE *fp) const
		{
			fprintf(fp, "%s,%d,%d,%d,%d,%s", szMagic, nVer, nBlockSize, nAvailIdx, nMaxBlockIdx, wh_hex2str(PassMD5,sizeof(PassMD5)));
		}
	};
	#pragma pack()
	// ����ļ��Ƿ��ǰ��ļ�
	enum	enumFILEISWHAT
	{
		FILEISWHAT_ERROR	= -1,		// �ļ��򿪴���
		FILEISWHAT_COMMON	= 0,		// һ���ļ�
		FILEISWHAT_PCK		= 1,		// һ����ļ�
		FILEISWHAT_TAILPCK	= 2,		// tail���ļ�
	};
	static enumFILEISWHAT	FileIsWhat(whfile *fp);
	static enumFILEISWHAT	FileIsWhat(whfileman *pFM, const char *cszFName);
public:
	#pragma pack(1)
	// ÿ�����ͷ
	struct	BLOCK_HDR_T
	{
		blockoffset_t			nPrev;								// ǰһ����ţ�0��ʾû�У�
		blockoffset_t			nNext;								// ��һ����ţ�0��ʾû�У�
	};
	// ��һ��ͷ���ĸ�������
	struct	FIRST_BLOCK_EXT_T
	{
		blockoffset_t			nEnd;								// ����һ�����
		int						nFileSize;							// �ļ�����
		time_t					nFileTime;							// �ļ�����޸�ʱ��
		char					junk[32-12-8];						// ��������õģ����ܵ����ó�32�ֽڣ�����ǰ���BLOCK_HDR_T����
								// 12��FIRST_BLOCK_EXT_T��ǰ�����ݵĴ�С
								// 8��BLOCK_HDR_T�����ݵĴ�С
	};
	#pragma pack()
	// �ڲ�RAW�ļ�
	class	rawfile	: public whfile
	{
		// ����ļ����ȱ��ˣ�����صı���Ҳ��仯�������ǣ�
		// m_nEnd����m_FirstBlockExt.nFileSize
		// ����ļ�ָ��仯�ˣ�����صı���Ҳ��仯�������ǣ�
		// m_nCurOffset��m_nCurBlock��m_nCurOffsetInBlock
	public:
		rawfile(wh_package_base	*pPB);
		~rawfile();													// ������������Close
		int		Open(whfileman::OP_MODE_T nOPMode, blockoffset_t &nStart);
																	// ����modeѡ���Ƿ񴴽��ļ�������Ǵ�����nStart�����ԣ������������ļ�����ʼ���
																	// ����Ǵ��ļ���nStart�����ļ���ʼ��ţ��ڲ�������ļ��Ľ����鲢����������
																	// (ע�⣺��������򿪺���ͨ�ļ���w�򿪲�ͬ������ɾ�����е��ļ���)
		int		Close();											// �о���ûɶ�����ģ���Ϊ����д�������Ǽ�ʹ��Ч��
		int		Delete();											// ɾ���ļ����ݣ�֮������ļ����������ݾͲ������ˣ����Ҳ��ܼ������κζ�д����!!!��
																	// ͬ���ļ�����ָ��Ҳ��ɾ�����벻Ҫ��������!!!!
		int		Extend(int nSize);									// �����ļ����ȣ�����ɹ������µ��ļ����ȣ�
																	// ע�⣺����ɹ���Extend���ı䵱ǰ��Offset�����ʧ�ܣ���Offset�Ͳ�֪����ʲô�ط���
		void	SetFileTime(time_t t);
		inline blockoffset_t	GetStart() const
		{
			return	m_nStart;
		}
		inline blockoffset_t	GetEnd() const
		{
			return	m_FirstBlockExt.nEnd;
		}
		inline blockoffset_t	GetBlockDataMaxSize() const
		{
			return	m_pPB->GetBlockDataMaxSize();
		}
		inline blockoffset_t	GetFirstBlockDataMaxSize() const
		{
			return	m_pPB->GetFirstBlockDataMaxSize();
		}
		inline bool	IsCurBlockLastOne() const						// �жϵ�ǰ��block�Ƿ������һ��
		{
			return	m_nCurBlock == GetEnd();
		}
		inline bool	IsReadOnly() const								// �ж��ļ��Ƿ���ֻ����
		{
			return	whfileman::IsReadOnlyMode(m_nOPMode);
		}
		// �����һ�����ڵ�ƫ�ƣ�����ǵ�һ�����ϸ������ݵĳ��ȣ�
		// *pnBlockNum���ܿ�����һ��*pnOffsetInBlock���ڵ�ǰ���ڵı��ˣ�*pnOffsetInBlockֻ�����һ��Ż����ָ���β��
		inline void	CalcBlocknumAndOffsetinblock(int nOffset, int *pnBlockNum, int *pnOffsetInBlock) const
		{
			int	nRawOffset		= nOffset + sizeof(FIRST_BLOCK_EXT_T);
			*pnBlockNum			= nRawOffset/GetBlockDataMaxSize();
			*pnOffsetInBlock	= nRawOffset%GetBlockDataMaxSize();
			if( (*pnOffsetInBlock)==0 )
			{
				if( nOffset>=FileSize() )
				{
					(*pnBlockNum)		--;
					(*pnOffsetInBlock)	= GetBlockDataMaxSize();
				}
			}
		}
		// �����ļ�����
		inline void	AddFileSize(int nAdd)
		{
			m_FirstBlockExt.nFileSize	+= nAdd;
			m_bShouldSaveFirstBlockExt	= true;
		}
		///////////////////////////////////
		// Ϊwhfile�ӿ�ʵ�ֵķ���
		///////////////////////////////////
		void	SelfDestroy()
		{
			delete	this;
		}
		int		FileSize() const;									// �ڴ��ļ�ʱ����ֱ�Ӵӵ�һ���ж����ļ��ߴ�����
		// RAW�ļ��Ͳ���Ҫʱ�����������
		time_t	FileTime() const;
		int		Read(void *pBuf, int nSize);
		int		Seek(int nOffset, int nOrigin);
		bool	IsEOF();
		int		Tell();
		int		Write(const void *pBuf, int nSize);
		int		Flush();
	protected:
		// ��������
		// ��ָ��
		wh_package_base	*m_pPB;
		// ��ģʽ
		whfileman::OP_MODE_T	m_nOPMode;
		// ��ǰ���ͷ��Ϣ
		BLOCK_HDR_T		m_CurBlockHdr;
		// ��һ��ĸ�����Ϣ
		FIRST_BLOCK_EXT_T		m_FirstBlockExt;
		// ��Ҫ���渽����Ϣ
		bool			m_bShouldSaveFirstBlockExt;
		// �ļ���ʼ��ţ����������m_FirstBlockExt���д洢��
		blockoffset_t	m_nStart;
		// λ��ָ�루������ǰ��Ϳ��ڵ�ƫ�ƣ�����ƫ����ֻ��ȥͷ��BLOCK_HDR_T��֮���ƫ�ƣ���
		blockoffset_t	m_nCurBlock;
		// �ļ�ȫ�ֵ�Offset
		int				m_nCurOffset;
	protected:
		// ���÷���
		// ��ָ���Ƶ�ָ����������ͷ���������κ�����
		int	SeekToBlockHdr(blockoffset_t nBlock);
		// ��ָ���Ƶ�ָ���飬������ͷ��m_CurBlockHdr
		int	SeekToBlockAndReadHdr(blockoffset_t nBlock);
		// ����m_pPB���ļ�ָ���ƶ������ļ���ǰע�ӵ�λ�ã���Ϊ���������ڿ��ڵ�ƫ�ƣ����԰����ƫ�Ʒ��ظ��ϲ�
		int	SeekToMyCurPos(int *pnOffsetInBlock);
	};
	// ��rawfile��ʹ��wh_package_base�еķ���
	friend class wh_package_base::rawfile;
	// ��չ�����ո����ĵ����ļ�
	class	file4out	: public wh_package_singlefile
	{
	private:
		wh_package_base	*m_pPB;
		wh_package_base::blockoffset_t	m_nStartBlock;
	public:
		file4out(wh_package_base *pPB)
			: m_pPB(pPB)
			, m_nStartBlock(0)
		{
		}
		~file4out()
		{
		}
		void	SelfDestroy()
		{
			delete	this;
		}
	};
	friend class wh_package_base::file4out;
protected:
	// �Ƿ񱻳�ʼ����
	bool						m_bInited;
	bool						m_bShouldSavePackageHdr;			// m_package_hdr�ı��ˣ���Ҫ����
	bool						m_bShouldSaveFileList;				// �ļ���Ϣ��ı��ˣ���Ҫ����
	bool						m_bShouldSaveVectorFileName;		// �ļ�����ı��ˣ���Ҫ����
	// ��������
	INFO_T						m_info;								// ��¼һ�³�ʼ��������
	PACKAGE_HDR_T				m_package_hdr;						// �ļ�ͷ���򿪰���ʱ����룬��������䶯�ڹرհ���ʱ�򱣴�
	// �ļ��д洢���ļ��б�Ԫ
	// �������б��ļ��еĵ�Ԫ����(unsigned char)
	enum
	{
		TYPE_FILE				= 1,								// �ļ�
		TYPE_DIRBEGIN			= 2,								// Ŀ¼��ʼ
		TYPE_DIREND				= 3,								// Ŀ¼����
	};
	// Ȼ�������ֵĿ�ʼƫ��(int����Ϊwhstrinvector4hash�����offset�������������Ծ���ô�ð�)
	// �������ļ���Ŀ¼����(�ļ�����Ҫ�ļ�����ʼ���blockoffset_t nStartBlock�������ļ��Ƿ�ѹ������֮���ֱ�����ļ������п�����Ŀ¼���ں���û�����ʲô����...)
	// �ڴ��е�Ŀ¼�ṹ��map
	struct	FILENODE_T 
	{
	public:
		typedef	whhash<STRINVECT, FILENODE_T, whcmnallocationobj, STRHASHFUNC>			STR2NODE_T;
	private:
		bool	bIsDir;												// ��������Ƿ���Ŀ¼
	public:
		union	_U_T
		{
			struct	_D_T
			{
				STR2NODE_T		*pContent;							// Ŀ¼���ݣ�һ�㶼��Ϊ�գ�����Ŀ¼��û���ļ�Ҳֻ��mapΪ�գ�
			}d;
			struct	_F_T
			{
				blockoffset_t	nStartBlock;						// �ļ���ʼ���
			}f;
		}u;
		FILENODE_T()
		{
			WHMEMSET0THIS();
		}
		inline bool IsDir() const
		{
			return	bIsDir;
		}
		inline void	SetIsDir(bool bSet)
		{
			bIsDir	= bSet;
		}
	};
	typedef	FILENODE_T::STR2NODE_T	STR2NODE_T;
	FILENODE_T					m_RootNode;							// ��Ӧ���ڵ��node
	whvector<char>				m_vectFNAME;						// ����ļ����Ĵ󻺳�
	STRSET_T					m_setFNAME;							// �Ѿ����ڵ��ļ�����
	// ������õ�Dir����
	class	dir4out		: public WHDirBase
	{
	private:
		STR2NODE_T				*m_pDir;							// ��Ӧ��Ŀ¼����ӳ�����
		STR2NODE_T::kv_iterator	m_it;								// ��������еĵ�����
		ENTRYINFO_T				m_EntryInfo;
		char					m_szBaseDir[WH_MAX_PATH];			// ��Ӧ��Ŀ¼��
	public:
		dir4out(STR2NODE_T *pDir, const char *cszBaseDir);
		virtual ~dir4out();
		virtual void			SelfDestroy()	{delete this;}
		virtual const char *	GetDir() const	{return m_szBaseDir;}
		virtual int				Close();
		virtual ENTRYINFO_T *	Read();
		virtual void			Rewind();
	};
protected:
	// ע�⣺������Щ�������ķ������ܱ����ʹ�ã����ʹ��Ҫ���Ƕ��̵߳�ʱ��ͱȽ��鷳����Ϊ��Щ�������ܻᱻ�ļ����������ֱ���á�
	// ������������̶߳�������AllocBlockȻ������д���ͷ����ô�Ϳ��ܳ������ˡ�

	// ��ȡһ�����õ��¿�ƫ�ƣ�����0��ʾ�޷���ȡ��һ��Ӧ�ò��������������ģ�����Ӳ�����˻����ļ��ˣ�
	// Ӧ�����ȴӿ��ñ����ң����û�����·���
	// ����ɹ���ͬʱ�ڲ���whfileָ��Ҳ���ƶ�����Ӧ���block��ͷλ�ã�����û�ж�ȡ��ͷ���ݣ������ϲ��������д���µĿ�ͷ���ݣ�
	blockoffset_t	AllocBlock();
	// ����һ��rawblock
	int				MakeRawBlock(blockoffset_t nBlock);
	// ���ڲ��ļ�ָ���ƶ���ָ����Ŀ�ͷ�����濪ͷ����AllocBlock�ɹ����λ��ʹһ���ģ�
	int				SeekToBlockHdr(blockoffset_t nBlock);
	// ���ڲ��ļ�ָ���ƶ���ָ�����ָ��λ�ã�����nOffset��ָBLOCK_HDR_T֮���ƫ��
	int				SeekToBlockAndOffset(blockoffset_t nBlock, blockoffset_t nOffset);
	// ��ĳ���鵽ĳ����֮���һ��������Ȼ�ϲ���Ҫ��֤���������Ч�ġ���
	int				FreeBlock(blockoffset_t nStart, blockoffset_t nEnd);
	// ���ļ�f�ж���Ŀ¼���pRootΪ���ڵ㴴��Ŀ¼�����ɵݹ飩
	int				ReadDirTreeFromFile(STR2NODE_T *pRoot, whfile *f);
	// ��Ŀ¼�����ݴ����ļ����ɵݹ飩
	int				SaveDirTreeToFile(STR2NODE_T *pRoot, whfile *f);
	// �ͷ�Ŀ¼�����ɵݹ飩
	int				FreeDirTree(STR2NODE_T *&pRoot);
	// ɾ��Ŀ¼������������Ŀ¼���ļ������ɵݹ飩
	int				DelDirTree(STR2NODE_T *&pRoot);
	// �������ļ��Ĵ���
	// ͨ���ļ������Ҷ�Ӧ�Ľڵ㣨���Ҳ������Ŀ¼��
	// cszPath��������xxx/yyy/zzz�����ģ�����/Ҳ������\��������Ӹ�Ŀ¼��ʼ�����·��
	struct	FINDNODEBYPATHNAME_RST_T
	{
		// ����ҵ��ļ���Ŀ¼
		FILENODE_T			*pFN;									// ���������յĽڵ�
		
		STR2NODE_T			*pDir;									// ���ֻ�ҵ����֣���pDir����Զ��һ��Ŀ¼�Ķ���
																	// ������ҵ�ȫ������pDir��һ��Ŀ¼����
		int					nOffset;								// ������ҵ����֣�����û��ʵ�ֲ��ֵ����ֵĿ�ʼƫ��
																	// ���磺a/b/c/d/e.txt�����Ŀǰֻ����Ŀ¼a/b/c����pDir����a/b/c���Ŀ¼����nOffset=6����d/e.txt��ʼ��λ�ã�
																	// ������ҵ�ȫ������nOffset�������һ�εĿ�ʼƫ��
	};
	enum
	{
		FINDNODEBYPATHNAME_RST_OK					= 0,			// �ɹ��ҵ��ļ�
		FINDNODEBYPATHNAME_RST_PARTIAL				= 1,			// �ҵ�����·��
		FINDNODEBYPATHNAME_RST_ERR_MIDDLEFILEINPATH	= -1,			// ·���м����ļ������ܼ�������
	};
	int				FindNodeByPathName(const char *cszPath, FINDNODEBYPATHNAME_RST_T *pRst);
	// ��pRoot��Կ�ʼ�������༶Ŀ¼����������ļ�����cszPath�����pRoot��·��
	FILENODE_T *	CreateNodeByPathName(const char *cszPath, STR2NODE_T *pRoot, bool bIsFile);
public:
	wh_package_base();
	~wh_package_base();
	void	SelfDestroy()	{delete this;}
	int	Init(INFO_T *pInfo);
	int	Release();
	int	SavePackageHdr();											// �����ͷ�仯�˾ͱ���֮
	int	SaveInfoFiles();											// �������������ļ�
	int	Flush();													// ������ȰѰ�ͷ���̣�Ȼ����flush�����ļ�����
	inline rawfile *	NewRawFile()
	{
		return	new rawfile(this);
	}
	inline blockoffset_t	GetBlockDataMaxSize() const				// ����һ���п���д���ݵĿռ�Ĵ�С
	{
		return	m_package_hdr.nBlockSize - sizeof(BLOCK_HDR_T);
	}
	inline blockoffset_t	GetFirstBlockDataMaxSize() const		// �����һ���п���д���ݵĿռ�Ĵ�С
	{
		return	GetBlockDataMaxSize() - sizeof(FIRST_BLOCK_EXT_T);
	}
	inline whfile *	GetBaseFile()									// ������Ҫ��rawfile���ܻ�ʹ�õ����
	{
		return	m_info.pBaseFile;
	}
	inline bool	IsReadOnly() const									// �ж��ļ��Ƿ���ֻ����
	{
		return	whfileman::IsReadOnlyMode(m_info.nOPMode);
	}
	inline const PACKAGE_HDR_T &	GetPackageHdr() const
	{
		return	m_package_hdr;
	}
	inline const INFO_T &	GetInitInfo() const
	{
		return	m_info;
	}
	// �����ɾ���ļ�
	int			DelFileByBlockIdx(blockoffset_t nStart);
	// �����ļ������ļ��Ĵ�����Щ�ļ�������Ŀ¼�������������������·����
	whfile *	OpenFile(const char *szFile, whfileman::OP_MODE_T nOPMode, blockoffset_t *pnStart=NULL);
	WHDirBase *	OpenDir(const char *szDir);
	int			MakeDir(const char *szDir);
	// ɾ��·�����ļ���Ŀ¼��
	int			DelPath(const char *szPath);
	// ��ȡ�ļ���Ϣ����Ҫ��Ϊ�˲���ѹ�������ݾͻ�û�����Ϣ��
	int			GetPathInfo(const char *cszPath, whfileman::PATHINFO_T *pPathInfo);
	// ������
	// �����������ļ�������б������ı䵱ǰ��block��
	int			GetBlockList(blockoffset_t nStart, whvector<blockoffset_t> &vect);

	// ���ذ����ļ���ԭʼ���ݣ�ѹ��������ݣ������Ľ����
	whfile *	OpenRawFile(const char *szFile, whfileman::OP_MODE_T nOPMode, blockoffset_t *pnStart=NULL);
};

}		// EOF namespace n_whcmn

#endif	// EOF __WH_PACKAGE_BASE_H__
