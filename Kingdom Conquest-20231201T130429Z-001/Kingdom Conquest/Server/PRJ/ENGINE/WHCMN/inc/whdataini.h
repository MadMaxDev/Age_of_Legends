// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataini.h
// Creator      : Wei Hua (κ��)
// Comment      : ����whdataprop��ini�ļ�����
//              : �����ļ��ĵ��г��Ȳ��ܳ���1024�ֽ�(�ο�MAXLINELEN�Ķ���)
// CreationDate : 2003-11-08
// ChangeLOG    : 2005-08-03 ������һ��using n_whcmn::xxx�Ķ��壬�����Ϳ����ð���whdataini.h���ļ����ʹ��whdataini::obj�����������
//              : 2006-05-31 �����Ŷ^O^�������������ļ��ĺ궨�幦�ܡ�
//              : 2006-06-20 �����������ļ���include���ܡ���ǰЩ���ӻ������������ļ��ĺ궨���Լ�˳���������㣨�����ͨ����ǿwhstring�е�������������ʵ�ֵģ����ܣ����������������������ﲹ�ϰɣ���
//              : 2006-06-22 �����˵����ڵ�ע�͹��ܡ�������һ������ʱ��ע�ͻ�������̵�ע�;�û�����ˡ�
//              : 2007-01-18 ������#ifdef/#ifndef/#else/#endif����
//              : 2007-06-27 ������m_nIgnoreBadObjLevel�Ա��������Կ��ƺ��Բ���ʶ�����Ƕ�׼���

#ifndef	__WHDATAINI_H__
#define	__WHDATAINI_H__

#ifdef	WIN32
#pragma warning(disable: 4786)
#endif
#include "whdataprop.h"
#include "whvector.h"
#include "whfile.h"
#include "whfile_fcd.h"
#include "whstring.h"
#include "whhash.h"
#include <map>
#include <string>

namespace n_whcmn
{

class	whdataini
{
public:
	// �����г���
	enum
	{
		MAXLINELEN						= 4096,
	};
	// �����
	enum
	{
		ERRNO_SUCCESS					= 0,				// û�д���
		ERRNO_CANNOTOPENFILE			= -1,				// ���ܴ��ļ�
		ERRNO_DUPINCLUDE				= -2,				// �ļ��ظ�include
		ERRNO_BADOBJNAME				= -10,				// ����Ķ�����
		ERRNO_BADLINE					= -11,				// �������
	};
	int		m_nIgnoreBadObjLevel;							// С�ڵ����������Ĵ�����������ȥ(������ֱ���������ֵ��Ĭ��Ϊ1)
	bool	m_bSkip;										// �Ƿ�������ǰ��
	int		m_nSkipLevel;									// �����Ĳ��
	int		m_nLevel;										// ��ǰ�Ĳ��
	int		m_errno;										// �����
	int		m_errline;										// ������к�
	whstrstack	m_ssFNames;									// ����include���ļ���ջ
	whhashset<whstrptr4hash, whcmnallocationobj, _whstr_hashfunc>	m_setFNames;
															// ������ڼ�¼����include�����Ѿ���include���ļ���������ͬһ�ļ����ظ�include
															// ��m_ssFNames���ʹ��
	whmultifilechangedetector			m_mfcd;				// ���ڼ���Ƿ��κη��ʹ����ļ��иı�
	// �������ݷ��ʵĶ���
	class	obj	: public whdataprop_container
	{
	public:
		// ����ʼ�ͽ���ʱӦ�õ��õĺ���
		virtual void	begin()	{}
		virtual void	end()	{}
	};
	// ��ͬ�ֶ���õ����б�
	template<class _Ty>										// _TyӦ���Ǵ�obj�̳еĿ��Խ����ļ��ķ����Ķ���
	struct	objlist	: public obj
	{
		_Ty				*m_pCurObj;
		whvector<_Ty *>	m_vectObjPtr;							// �ϲ���Ը�����Ҫ����reserve

		objlist()
			: m_pCurObj(NULL)
		{
		}
		virtual ~objlist()
		{
			clear();
		}
		void	clear()										// clear�������ǿ������¼�������
		{
			for(size_t i=0;i<m_vectObjPtr.size();i++)
			{
				delete	m_vectObjPtr[i];
			}
			m_vectObjPtr.clear();
		}
		virtual void	begin()
		{
			// ���ɶ���
			m_pCurObj	= new _Ty;
			m_vectObjPtr.push_back(m_pCurObj);
		}
		virtual void	end()
		{
			m_pCurObj	= NULL;
		}
		// Ϊwhdataprop_containerʵ�ֵ�
		virtual int		setvalue(const char *cszKey, const char *cszValue, unsigned int nReloadMask)
		{
			assert(m_pCurObj);
			return	m_pCurObj->setvalue(cszKey, cszValue, nReloadMask);
		}
		virtual	void *	findvalueptr(const char *cszKey, int *pnType, int *pnExt, whdataprop_setvalfunc_t *pSetValFunc, int *pnSize, const whdataprop_entry_t **ppEntry)
		{
			assert(m_pCurObj);
			return	m_pCurObj->findvalueptr(cszKey, pnType, pnExt, pSetValFunc, pnSize, ppEntry);
		}
		virtual	void *	findvalueptr(const char *cszKey, int nType, int *pnExt, whdataprop_setvalfunc_t *pSetValFunc, int *pnSize, const whdataprop_entry_t **ppEntry)
		{
			assert(m_pCurObj);
			return	m_pCurObj->findvalueptr(cszKey, nType, pnExt, pSetValFunc, pnSize, ppEntry);
		}
	};
	// �����ַ����ṩ����ָ��Ľӿ�
	class	objgetter
	{
		// ��linux�±����class����
		friend class	whdataini;
	protected:
		typedef	std::map<std::string, obj *>	MAP_T;
		// �ִ�����Ӧָ���ӳ��
		MAP_T							m_mapobj;
	public:
		objgetter();
		~objgetter();
		// �������ֺ�pObj��ӳ��
		int		addobj(const char *cszName, obj *pObj);
		// ͨ�����ֻ�ö�Ӧ�Ķ���ָ��
		obj *	getobj(const char *cszName);
	};
	// �з��������������е�����
	class	lineanalyzer
	{
	public:
		enum
		{
			TYPE_BADLINE		= -1,						// ��д���д���
			TYPE_NOTHING		= 0,						// û���������
			TYPE_COMMENT		= 1,						// ע����
			TYPE_MACRODEFINE	= 2,						// �궨��
			TYPE_INCLUDE		= 3,						// include�ļ���m_szParam[0]������ǰ����ļ���·��������Ŀǰֻ���������·���������EXE�Ķ���������������ļ��ģ�
			TYPE_IFDEF			= 4,						// #ifdef��m_szParam[0]��ifdef����Ĳ�����
			TYPE_IFNDEF			= 5,						// #ifndef��m_szParam[0]��ifndef����Ĳ�����
			TYPE_ELSE			= 6,						// #else
			TYPE_ENDIF			= 7,						// #endif
			TYPE_OBJENDBEGIN	= 10,						// ����ʼ��ͬʱ��ζ���ϸ��������
			TYPE_OBJBEGIN		= 11,						// ����ʼ
			TYPE_OBJEND			= 12,						// �������
			TYPE_VAL			= 21,						// ��ֵ���
			TYPE_IGNORE			= 31,						// �����Ե���䣨��������ifdef/ifndef/else/end��ɵĺ��ԣ�
		};
		char	m_szParam[2][MAXLINELEN];
		char	m_szLineTmp[MAXLINELEN];					// ��ʱ����ĳ�е����ݣ�������һ�У�
	public:
		virtual ~lineanalyzer()								{}
		// �ڲ����㣬���¿�ʼ
		virtual void	reset()								{}
		// ����һ�У������е�����
		virtual int		analyze(const char *cszLine)		= 0;
	};
	// Ĭ�ϵļ��з�����(ini���͵�)
	class	dftlineanalyzer	: public lineanalyzer
	{
	public:
		int		analyze(const char *cszLine);
	};
	// Ƕ�׽ṹ���з�����
	class	cpplineanalyzer	: public lineanalyzer
	{
	public:
		typedef	std::map<std::string, std::string>	MAP_T;
		MAP_T	m_mapMACRO;
		enum
		{
			IF_IFDEF_NO						= 0,
			IF_IFDEF_YES					= 1,
			IF_IFDEF_IGNORE					= 2,			// ���ϲ��ifdef���º��Ե�ifdef
		};
		whvector<int>						m_vectIF;		// IF�ṹջ������ifdef/ifndef/else/endif��
		cpplineanalyzer();
	public:
		void	reset();
		int		analyze(const char *cszLine);
	};
protected:
	// ������ӳ����ָ��
	objgetter		*m_getter;
	// �з�����
	lineanalyzer	*m_analyzer;
	// ��ǰ��Ķ���ջ
	whstack<obj *>	m_ObjStack;
	// ��ǰ�������У�analyzefile�õģ�
	char			m_szCurLine[MAXLINELEN];
	// Ĭ���Դ��Ĺ�����
	whfileman		*m_pFileMan_dft;
	// ʵ�ʵ��ļ�������
	whfileman		*m_pFileMan;
	// �Ƿ����setvalue�Ĵ���
	bool			m_bIgnoreSetValueErr;
public:
	whdataini();
	~whdataini();
	int	setgetter(objgetter *getter);
	int	setlineanalyzer(lineanalyzer *analyzer);
	// ����һ��
	int	analyzeline(const char *cszLine, unsigned int nReloadMask);
	// �����ļ�
	// �����ļ������ļ��ģ����cszFileΪ�ջ��߿��ִ����÷�����
	// bResetΪtrue��ʾ���һЩ�ڲ�������һ����˵����include���ļ��ڷ���ʱbReset��Ҫ����Ϊfalse��
	// nReloadMask�����ĳ���������0�������reload������
	int	analyzefile(const char *cszFile, bool bReset=true, unsigned int nReloadMask=0);
	// ֱ��ʹ���ļ�����ģ������������ͨ�����紫�����������ļ���
	int	analyzefile(whfile *file, bool bReset=true, unsigned int nReloadMask=0);
	// ��ӡ�������̣���Ҫ���ڲ��
	int	printincludestack(FILE *fpout);
	const char *	printincludestack(char *pStrOut, int &nMaxSize);
	inline const char *	printincludestack()
	{
		int		nMaxSize	= 0;
		return	printincludestack(NULL, nMaxSize);
	}
	// ��ӡ�����ִ�����Ҫ���ڲ��
	int	printerrreport(FILE *fpout);
	const char *	printerrreport(char *pStrOut, int &nMaxSize);
	inline const char *	printerrreport()
	{
		int		nMaxSize	= 0;
		return	printerrreport(NULL, nMaxSize);
	}
	// ��õ�ǰ�����ݣ���Ҫ���ڲ��
	inline const char *	GetCurLine() const
	{
		return	m_szCurLine;
	}
	// �����ļ������������������Ҫ�����ͷţ�
	inline void	SetFileMan(whfileman *pFileMan)
	{
		if( pFileMan )
		{
			m_pFileMan	= pFileMan;
		}
		else
		{
			m_pFileMan	= m_pFileMan_dft;
		}
	}
	// ���ö���ռ
	inline void	ResetObjStack()
	{
		m_ObjStack.clear();
	}
	// �����Ƿ����setvalueʱ���Ĵ�һ�㶼���Ҳ���key��
	inline void	SetIgnoreSetValueErr(bool bSet)
	{
		m_bIgnoreSetValueErr	= bSet;
	}
};

template<class _Getter, class _Analyzer>
class	WHDATAINI	: public whdataini
{
private:
	_Getter		getter;
	_Analyzer	analyzer;
public:
	WHDATAINI()
	{
		setgetter(&getter);
		setlineanalyzer(&analyzer);
	}
	// �������ֺ�pObj��ӳ��
	inline int		addobj(const char *cszName, obj *pObj)
	{
		return	getter.addobj(cszName, pObj);
	}
};

typedef	WHDATAINI<whdataini::objgetter, whdataini::cpplineanalyzer>	WHDATAINI_CMN;

}		// EOF namespace n_whcmn

// ����whdataini::objgetter::get�У�ͨ����������cszName���ɶ���
#define	WHDATAINI_NEW_OBJ0(mType, className)			\
	if( strcmp(cszName, mType)==0 )						\
	{													\
		return	new className;							\
	}													\
	// EOF WHDATAINI_NEW_OBJ0
#define	WHDATAINI_NEW_OBJ(className)					\
	WHDATAINI_NEW_OBJ0(#className, className)			\
	// EOF WHDATAINI_NEW_OBJ0

////////////////////////////////////////////////////////////////////
// �����Ҫ��Ϊ����Щ����û��prop���ܵ��౻�̳к���prop����
////////////////////////////////////////////////////////////////////
#define	WHDATAINI_SORC_DECLARE0(sorc, STRUCTNAME, BASENAME)			\
sorc	STRUCTNAME	: public whdataini::obj, public BASENAME		\
{																	\
	inline BASENAME *	GetBase() const								\
	{																\
		return	(BASENAME *)this;									\
	}																\
	inline BASENAME &	GetBaseRef()								\
	{																\
	return	*GetBase();												\
	}																\
	WHDATAPROP_DECLARE_MAP(STRUCTNAME)								\
};																	\
// EOF WHDATAINI_SORC_DECLARE0
#define	WHDATAINI_STRUCT_DECLARE(STRUCTNAME, BASENAME)				\
	WHDATAINI_SORC_DECLARE0(struct, STRUCTNAME, BASENAME)			\
// EOF WHDATAINI_STRUCT_DECLARE
#define	WHDATAINI_CLASS_DECLARE(STRUCTNAME, BASENAME)				\
	WHDATAINI_SORC_DECLARE0(class, STRUCTNAME, BASENAME)			\
// EOF WHDATAINI_STRUCT_DECLARE


#define	WHDATAINI_SORC_DECLARE_BEGIN0(sorc, STRUCTNAME, BASENAME)	\
sorc	STRUCTNAME	: public whdataini::obj, public BASENAME		\
{																	\
	inline BASENAME *	GetBase() const								\
	{																\
		return	(BASENAME *)this;									\
	}																\
	// EOF WHDATAINI_SORC_DECLARE_BEGIN0
#define	WHDATAINI_SORC_DECLARE_END0(STRUCTNAME)						\
	WHDATAPROP_DECLARE_MAP(STRUCTNAME)								\
};																	\
	// EOF WHDATAINI_SORC_DECLARE_END0
#define	WHDATAINI_STRUCT_DECLARE_BEGIN(STRUCTNAME, BASENAME)		\
	WHDATAINI_SORC_DECLARE_BEGIN0(struct, STRUCTNAME, BASENAME)		\
	// EOF WHDATAINI_STRUCT_DECLARE_BEGIN
#define	WHDATAINI_STRUCT_DECLARE_END(STRUCTNAME)					\
	WHDATAINI_SORC_DECLARE_END0(STRUCTNAME)							\
	// EOF WHDATAINI_SORC_DECLARE_END

// д����Щ�Ϳ����ð���whdataini.h���ļ����ʹ��whdataini::obj�����������
using n_whcmn::whdataini;
using n_whcmn::whdataprop_entry_t;
using n_whcmn::whdataprop_map_t;

#endif	// EOF __WHDATAINI_H__
