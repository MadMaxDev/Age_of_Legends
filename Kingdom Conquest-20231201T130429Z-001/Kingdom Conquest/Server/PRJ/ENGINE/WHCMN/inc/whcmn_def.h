// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whcmn_def.h
// Creator      : Wei Hua (κ��)
// Comment      : whcmn���õĶ���
// CreationDate : 2003-05-15
// CHANGELOG    :
//              : 2005-09-15 whmisc.h������ȫ��ת�Ƶ�whcmn_def.h�С�
//              : 2006-01-23 WHSafeSelfDestroy������Ϊ���ã�ԭ��һֱд���ˡ�
//              : 2006-02-05 WHRANGE_T��������������ʵ����һ�¡�
//              : 2006-02-06 WHRANGE_T���ǲ����˼򵥵ļ̳еķ�������ΪVC6��֧��ģ����ƫ�ػ���gcc��֧���ڽṹ�����ģ������ػ�(��ʲô�ػ�����֧��Ŷ)��
//              : 2006-02-23 ����whsetbool_true/false����
//              : 2006-03-09 ������whbufferwrite/read���������ڼ򵥵Ļ���ƫ�ƶ�ȡ
//              : 2007-11-15 ������whsafeptr��detach����

#ifndef __WHCMN_DEF_H__
#define	__WHCMN_DEF_H__

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>						// û�����RAND��û������
#include <string.h>

#include <malloc.h>

// ������������к�ID������ص�ID��������ѭ������
// ���Ա����¾�ID�滻ʱ�Ĵ���
// ���ֻ������ʽʹ��ʱ���ã�ƽʱ����Ϊ�˷���ID���ܼ�ʱ�ͷŵĴ���
// #define	WHCMN_USECYCLE_ID

namespace n_whcmn
{

#ifdef	__GNUC__
	struct	S_GODDAMNLINUXWARNING
	{
		static void *whoffsetdummpptr;	// ���ֻ��Ϊ�˲���linux��warning
	};
	// ������ݳ�Աƫ��
	#define wh_offsetof(s,m)			(size_t)&(((s *)n_whcmn::S_GODDAMNLINUXWARNING::whoffsetdummpptr)->m)
	// ������Ա�ĳߴ�
	#define	wh_sizeinclass(s,m)			sizeof(((s *)n_whcmn::S_GODDAMNLINUXWARNING::whoffsetdummpptr)->m)
#else
	// ������ݳ�Աƫ��
	#define wh_offsetof(s,m)			(size_t)&(((s *)0)->m)
	// ������Ա�ĳߴ�
	#define	wh_sizeinclass(s,m)			sizeof(((s *)0)->m)
#endif

#define	WHSIZEOFARRAY(pArr, num)		(sizeof(*pArr)*num)
#define	WHNUMOFARRAYUNIT(arr)			(sizeof(arr)/sizeof(arr[0]))

#define	WHMEMSET0THIS()					memset(this, 0, sizeof(*this));
template<typename _Ty>
inline void	WHMEMSET0(_Ty &v)
{
	memset(&v, 0, sizeof(v));
}
template<typename _Ty>
inline void	WHMEMSET0PTR(_Ty *p)
{
	memset(p, 0, sizeof(*p));
}

// ����ƫ�ƻ�õ�ַָ��
// ��������constָ����Ϊ��const�͵����ݲ���ǿ��ת��Ҳ���Ա�ʹ�ã�
inline void *	wh_getoffsetaddr(const void *ptr, int off)
{
	return	(((char *)ptr)+off);
}
// ���ָ���������е����
template<typename _Ty>
inline int		wh_getidxinarray(_Ty *first, _Ty *dummy)
{
	return	(dummy-first);
}
// ���ض�����ָ��ĵ���
template<typename _Ty>
inline void		wh_adjustaddrbyoffset(_Ty *&ptr, int off)
{
	if( !ptr )	return;	// ����ǿ�ָ��Ͳ�У����
	ptr	= (_Ty *)wh_getoffsetaddr(ptr, off);
}
// һ�����������ڲ�����ĵ���
template<typename _Ty>
inline void		wh_adjustarraybyoffset(_Ty *ptr, int num, int off)
{
	for(int i=0;i<num;i++)
	{
		ptr->AdjustInnerPtr(off);
		++ptr;
	}
}

// �Ա�ָ����ƫ��
inline int		wh_calcoffset(void *first, void *next)
{
	return	(int)(((char*)next) - ((char*)first));
}

// �������͸�ֵָ��
template<typename _Ty>
inline void		wh_settypedptr(_Ty *&pDst, const void *pSrc)
{
	pDst	= (_Ty *)pSrc;
}

// ����������������ĵ�ַ
template<typename _Ty>
inline	void *	wh_getptrnexttovar(_Ty &var)
{
	return	(&var) + 1;
}
// ����ָ�����ͻ�ý�����ĵ�ַ
template<typename _Ty>
inline	void *	wh_getptrnexttoptr(_Ty *ptr)
{
	return	(void*)(ptr + 1);
}

// д�벢�޸Ļ���ָ������ͳߴ�
inline char *	whbufferwrite(char *pBuf, int &nSizeLeft, const void *pData, int nDataSize)
{
	assert(nSizeLeft>=nDataSize);
	memcpy(pBuf, pData, nDataSize);
	nSizeLeft	-= nDataSize;
	return	pBuf + nDataSize;
}
template<typename _Ty>
inline char *	whbufferwrite(char *pBuf, int &nSizeLeft, _Ty *pData)
{
	return	whbufferwrite(pBuf, nSizeLeft, pData, sizeof(*pData));
}
// �������޸Ļ���ָ������ͳߴ�
inline char *	whbufferread(char *pBuf, int &nSizeLeft, void *pData, int nDataSize)
{
	assert(nSizeLeft>=nDataSize);
	memcpy(pData, pBuf, nDataSize);
	nSizeLeft	-= nDataSize;
	return	pBuf + nDataSize;
}
template<typename _Ty>
inline char *	whbufferread(char *pBuf, int &nSizeLeft, _Ty *pData)
{
	return	whbufferread(pBuf, nSizeLeft, pData, sizeof(*pData));
}

// ���������Ϊ����ʱ����Ҫ����һ��ʲôҲ�����Ĺ��캯��(����ֻ��Ϊ��ˢ�����)������Ϊ�������������ԣ���Ҫ��һ������
enum	NULLCONSTRUCT_T
{
	NULLCONSTRUECTOR   = 0,
};
#define	NEW_NULLCONSTRUECTOR(pMem,type)	new (pMem) type(NULLCONSTRUECTOR)

// ��Χ�����(ע�⣺���ֵ�ĸ���ֻ����32767��)
template <typename _Ty>
struct	WHRANGE_T_BASE
{
private:
	_Ty		vFrom;
	_Ty		vDiff;
	bool	bSame;
public:
	inline const _Ty 	GetFrom() const
	{
		return	vFrom;
	}
	inline const _Ty 	GetTo() const
	{
		return	vFrom+vDiff;
	}
public:
	WHRANGE_T_BASE()
	: vFrom(0), vDiff(0), bSame(true)
	{
	}
	WHRANGE_T_BASE(_Ty _From, _Ty _To)
	{
		Set(_From, _To);
	}
	void	Set(_Ty _From, _Ty _To)
	{
		// ��֤_From<=_To
		bSame	= false;
		if( _From>_To )
		{
			vFrom	= _To;
			vDiff	= _From - _To;
		}
		else if( _From<_To )
		{
			vFrom	= _From;
			vDiff	= _To - _From;
		}
		else
		{
			vFrom	= _From;
			bSame	= true;
		}
	}
	void	Set(const WHRANGE_T_BASE &other)
	{
		vFrom	= other.vFrom;
		vDiff	= other.vDiff;
		bSame	= other.bSame;
	}
	inline _Ty	GetRand() const
	{
		if( bSame )
		{
			return	vFrom;
		}
		return	vFrom+rand()%vDiff;
	};
	inline _Ty	GetRand_floattype() const
	{
		if( bSame )
		{
			return	vFrom;
		}
		return	vFrom+(vDiff*float(rand())/float(RAND_MAX));
	};
};
template <typename _Ty>
struct	WHRANGE_T	: public WHRANGE_T_BASE<_Ty>
{
	WHRANGE_T()	: WHRANGE_T_BASE<_Ty>()
	{
	}
	WHRANGE_T(_Ty _From, _Ty _To)	: WHRANGE_T_BASE<_Ty>(_From, _To)
	{
	}
};
#define	WHRANGE_T_F(ftype)									\
template <>													\
struct	WHRANGE_T<ftype>	: public WHRANGE_T_BASE<ftype>	\
{															\
	WHRANGE_T()	: WHRANGE_T_BASE<ftype>()	{}				\
	WHRANGE_T(ftype _From, ftype _To)	: WHRANGE_T_BASE<ftype>(_From, _To)	{}	\
	inline ftype	GetRand() const		{return	GetRand_floattype();}			\
};															\
// EOF WHRANGE_T_F
WHRANGE_T_F(float)
WHRANGE_T_F(double)

////////////////////////////////////////////////////////////////////
// ��ָ��İ�ȫɾ��
// ��֧������Ȩת��!!!!
////////////////////////////////////////////////////////////////////
struct	WHDelMethod_Cmn
{
	WHDelMethod_Cmn() {}
	template<typename _Ty>
	inline void	operator()(_Ty *ptr)	const	{delete ptr;}
};
template<typename _Ty, class _DelMethod=WHDelMethod_Cmn>
struct	whsafeptr
{
private:
	_DelMethod	m_dm;
protected:
	_Ty	*ptr;
public:
	whsafeptr()
	: ptr(NULL)
	{
	}
	whsafeptr(_Ty *__ptr)
	: ptr(__ptr)
	{
	}
	~whsafeptr()
	{
		selfdelete();
	}
	// ע��selfdelete����ͨ���麯���ķ�ʽд����Ϊ������ʱ������Ѿ����ƻ���
	void	selfdelete()
	{
		if( ptr )
		{
			m_dm(ptr);
			ptr		= NULL;
		}
	}
	inline void	detach()	// �������ض�����¿��Բ�ɾ��ָ�루���磺safeptrֻ��Ϊ�˳����쳣�����ɾ��ָ��ģ������������Ҫnew������ָ���������ط�����ʹ�ã�
	{
		ptr	= NULL;
	}
	inline void	setptr(_Ty *__ptr)
	{
		ptr	= __ptr;
	}
	inline bool operator ! () const
	{
		return	ptr==NULL;
	}
	inline bool	operator != (const _Ty * __other) const
	{
		return	ptr != __other;
	}
	inline bool	operator == (const _Ty * __other) const
	{
		return	ptr == __other;
	}
	inline _Ty&	operator*() const
	{
		return	*get();
	}
	inline _Ty*	operator->() const
	{
		return	get();
	}
	inline _Ty*	get() const
	{
		return	ptr;
	}
	inline operator _Ty* () const
	{
		return	ptr;
	}
};

struct	whsafeptr_FILE	: public whsafeptr<FILE>
{
	explicit whsafeptr_FILE(FILE *__ptr)
	: whsafeptr<FILE>(__ptr)
	{
	}
	~whsafeptr_FILE()
	{
		selfdelete();
	}
	void	selfdelete()
	{
		if( ptr )
		{
			fclose(ptr);
			ptr	= NULL;
		}
	}
};

struct	whsafeptr_FILE_excludeSTDs	: public whsafeptr<FILE>
{
	explicit whsafeptr_FILE_excludeSTDs(FILE *__ptr)
	: whsafeptr<FILE>(__ptr)
	{
	}
	~whsafeptr_FILE_excludeSTDs()
	{
		selfdelete();
	}
	void	selfdelete()
	{
		if( ptr )
		{
			if( ptr!=stdin && ptr!=stdout && ptr!=stderr )
			{
				fclose(ptr);
			}
			ptr	= NULL;
		}
	}
};

// ��ȫ��delete
template<typename _Ty>
void	WHSafeDelete(_Ty *&pObj)
{
	if( pObj!=NULL )
	{
		delete	pObj;
		pObj	= NULL;
	}
}

// ��ȫ��selfdestroy
template<typename _Ty>
void	WHSafeSelfDestroy(_Ty *&pObj)
{
	if( pObj!=NULL )
	{
		pObj->SelfDestroy();
		pObj	= NULL;
	}
}
// Safe SelfDestroy
struct	WHDelMethod_SD
{
	WHDelMethod_SD() {}
	template<typename _Ty>
	inline void	operator()(_Ty *ptr)	const	{ptr->SelfDestroy();}
};
// SD mean Sefl Destroy
template<typename _Ty>
struct	whsafeSDptr	: public whsafeptr<_Ty, WHDelMethod_SD>
{
	typedef	whsafeptr<_Ty, WHDelMethod_SD>	FATHERCLASS;
	whsafeSDptr()
	{
	}
	whsafeSDptr(_Ty *__ptr)
		: FATHERCLASS(__ptr)
	{
	}
};

// ���ȡ���Ĳ�������
inline int	wh_getintpartnum(int nTotal, int nParSize, int *pnResidue)
{
	return	nTotal/nParSize + ((*pnResidue=nTotal%nParSize)!=0 ? 1:0);
}

inline void	whsetbool_true(bool &bVar)
{
	if( !bVar )
	{
		bVar	= true;
	}
}
inline void	whsetbool_false(bool &bVar)
{
	if( bVar )
	{
		bVar	= false;
	}
}

// ��ȡ��������
const char *	wh_getenv(const char *cszKey);
// ���û�������
void			wh_setenv(const char *cszKey, const char *cszVal);

// ���ֻ�����������
typedef	unsigned char			whbyte;
#ifdef WIN32
#define	WHINT64PRFX		"I64"
typedef __int64					whint64;
typedef unsigned __int64		whuint64;
#endif
#ifdef	__GNUC__
#define	WHINT64PRFX		"ll"
typedef long long				whint64;
typedef unsigned long long		whuint64;
#endif

// ��ʱ��Ҫ���ִ���Ϊ����������
template<int _SIZE>
struct	wh_fixstr_t
{
	char	s[_SIZE];
	wh_fixstr_t()
	{
		s[0]		= 0;
		s[_SIZE-1]	= 0;
	}
	wh_fixstr_t(const char *cszStr)
	{
		operator = (cszStr);
	}
	inline void	clear()
	{
		s[0]	= 0;
	}
	inline void operator = (const char *cszStr)
	{
		strncpy(s, cszStr, sizeof(s)-1);
	}
	inline void operator = (const wh_fixstr_t &other)
	{
		operator = (other.s);
	}
	inline char & operator [] (int nIdx)
	{
		return	s[nIdx];
	}
	inline operator char * ()
	{
		return	s;
	}
	inline operator const char * () const
	{
		return	s;
	}
};

// unsigned int/short/char���ֽ�ת��
union	whlsc_t
{
	unsigned int	l;
	unsigned short	s[2];
	unsigned char	c[4];
	whlsc_t()
		: l(0)
	{
	}
	whlsc_t(unsigned int _l)
		: l(_l)
	{
	}
};
// whuint64/unsigned int/short/char���ֽ�ת��
union	whllsc_t
{
	whuint64		I;
	unsigned int	l[2];
	unsigned short	s[4];
	unsigned char	c[8];
	whllsc_t()
		: I(0)
	{
	}
	whllsc_t(whuint64 _I)
		: I(_I)
	{
	}
};

}		// EOF namespace n_whcmn

// �����Ҫ��Ϊ��д��ʱ���Դ�ӡ��䣬������ȥ����ʱ�����
#define	WHTMPPRINTF			printf

// �����Ҫ��������һ��������������ӡ�ִ��õ�
#define	WH_SMPINBUFF_PRINT	pszBuf+=rst;nSizeLeft-=rst;assert(nSizeLeft>=0);rst=snprintf

// �����Ҫ������д�ṹ��С�ڲ�����
#define	LESS_THAN_OP(thisvar, othervar)			\
	if( thisvar < othervar )					\
	{											\
		return	true;							\
	}											\
	else if( thisvar > othervar )				\
	{											\
		return	false;							\
	}											\
	// EOF LESSTHAN_OP
// Ĭ�϶Է�����������other
#define	LESS_THAN_OP_SMP(var)	LESS_THAN_OP(var, other.var)

// һ������lessthan������ôд
// bool	xxx::lessthan (const _Ty &other) const
// {
// 	LESS_THAN_OP_SMP(var1)
// 	LESS_THAN_OP_SMP(var2)
// 	LESS_THAN_OP_SMP(var3)
// 	return	false;
// }

// ����
template<typename T>
inline T wh_l_shift(const T & a, int n)
{
	return	a << n;
}
// ����
template<typename T>
inline T wh_r_shift(const T & a, int n)
{
	return	a >> n;
}

////////////////////////////////////////////////////////////////////
// ΪDLL֮����Ϣͬ��
////////////////////////////////////////////////////////////////////
#include "whdll.h"
void *	WHCMN_STATIC_INFO_Out();
extern "C" WH_DLL_API void	WHCMN_STATIC_INFO_In(void *pInfo);
#ifdef	_DEBUG
unsigned char	WHCMN_GET_DBGLVL();
void			WHCMN_SET_DBGLVL(unsigned char nLvl);
#endif
enum
{
	WHCMN_NOTIFY_NOTHING	= 0,				// ����ʲôҲû�е����
	WHCMN_NOTIFY_MEMERR		= 1,				// �ڴ��������
	WHCMN_NOTIFY_SHMERR		= 2,				// �����ڴ��������
};
int				WHCMN_GET_NOTIFY();
void			WHCMN_SET_NOTIFY(int nNotify);

// �����õ���LOGID
enum
{
	// һ����԰���־д�����ID 
	WHCMN_LOG_ID_CMN				=	0,
	// ��Ҫ����д������
	WHCMN_LOG_ID_ERROR				=	1,
	// ����������ص�д������(������Ϸ�к��û�������صľͷŵ������Ӧ��ԭ�������е�client��־������������һЩ���Էֵ�player��npc��־��ȥд)
	WHCMN_LOG_ID_STORY				=	2,
	// ��������
	WHCMN_LOG_ID_FATAL				=	4,
};
typedef	int (*LOG_WRITEFMT_FUNC_T)(int nID, const char *szFmt, ...);
void	WHCMN_Set_Log_WriteFmt( LOG_WRITEFMT_FUNC_T pFunc );
LOG_WRITEFMT_FUNC_T	WHCMN_Get_Log_WriteFmt();
#define	WHCMN_LOG_WRITEFMT			WHCMN_Get_Log_WriteFmt()
#define WHCMN_LOG_STD_HDR(code,keyword)		#code","#keyword","

// ��ñ����ַ�����Ĭ����û����д�ģ�
const char *	WHCMN_getcharset();
// ���ñ����ַ���
void	WHCMN_setcharset(const char *cszName);

#endif	// EOF __WHCMN_DEF_H__
