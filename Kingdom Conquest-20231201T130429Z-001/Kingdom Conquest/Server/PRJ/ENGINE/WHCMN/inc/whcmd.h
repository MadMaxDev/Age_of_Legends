// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whcmd.h
// Creator      : Wei Hua (κ��)
// Comment      : ͨ��������õĽӿڡ��Ҳο���MFC����Ϣӳ���ʵ�֡�
// CreationDate : 2003-12-18
// ע�⣺
// �������Ա������ַʱ����Ҫд�ɡ�&classname::funcname������ʽ����Ϊlinux����Ҫ����
// ChangeLOG    : 2005-10-11 ������whcmdshrink::GetDataNotRead
//              : 2006-02-10 ������whcmn_strcmd_reganddeal��Help����
//              : 2006-05-08 ��whcmdshrink��һЩ�Ƚϳ��ĺ���ȡ����inline
//              : 2007-01-29 �޸���ReadStr��ReadBigStr�ĳɹ�����ֵ��ԭ����0�����ڸ�Ϊ�������ִ��ĳ��ȣ����������һ��0��β����Ȼ���Զ�����0��β��
//              : 2007-03-01 ������whcmdshrink::Reset()����
//              : 2008-06-18 ������whcmdshrink::SetShrinkResize()����

#ifndef	__WHCMD_H__
#define	__WHCMD_H__

#include "whcmn_def.h"
#include "whstring.h"
#include "whqueue.h"
#include "whthread.h"
#include "whhash.h"
#include "whbits.h"
#include <map>
#ifdef	WIN32
#include <winsock2.h>
#endif
#ifdef	__GNUC__
#include <netinet/in.h>
#endif

namespace n_whcmn
{

////////////////////////////////////////////////////////////////////
// �������
////////////////////////////////////////////////////////////////////
// ע�⣺�������Read��Write�ڷ��ֳߴ��ʱ���׳��ִ��쳣�����������Ҫ��try/catch(const char *cszErr) !!!!
// WriteʱPos���Զ�����������ͨ��Pos���ʵ��д����ܳ��ȣ����ߵ���WriteEnd�Ա�֤shrinksize����д�볤�ȡ�
class	whcmdshrink
{
protected:
	// ָ����������ָ��
	char	*m_pShrink;
	// �����Ĵ�С(��Write����д��������С)
	int		m_nShrinkSize;
	// ԭʼ��m_nShrinkSize������Reset��
	int		m_nShrinkSizeOld;
	// ��ǰShink����λ��(R��W)
	int		m_nPos;
	// �Ƿ���Ҫ��int��shortת�������ֽ���(NetByteOrder)
	bool	m_bNBO;
public:
	// ���ó������ݵĻ��壨���������������Reset��
	inline	void	SetShrink(char *pShrink, int nShrinkSize)
	{
		m_pShrink		= pShrink;
		m_nShrinkSize	= nShrinkSize;
		m_nShrinkSizeOld= nShrinkSize;
		m_nPos			= 0;
	}
	inline	void	SetShrinkResize(char *pShrink, int nShrinkSize)		// ���һ����д��ʱ���������������ⲿ�Ļ���resizeһ�£����������ⲿ���𣩣�������Ҳ��������һ������ָ��
	{
		m_pShrink		= pShrink;
		m_nShrinkSize	= nShrinkSize;
		m_nShrinkSizeOld= nShrinkSize;
	}
	inline void		Reset()
	{
		m_nShrinkSize	= m_nShrinkSizeOld;
		m_nPos			= 0;
	}
	inline	void	SetNBO(bool bSet)
	{
		m_bNBO			= bSet;
	}
	// ��û���ָ��
	inline	char *	GetShrinkBuf() const
	{
		return	m_pShrink;
	}
	// ��ý�����ĳ���
	inline	int		GetShrinkSize() const
	{
		return	m_nShrinkSize;
	}
	// ��ý���/���ȡ�����л�ʣ�µ��ֽ���
	inline	int		GetSizeLeft() const
	{
		return	m_nShrinkSize - m_nPos;
	}
	inline	char *	GetCurPtr() const
	{
		return	m_pShrink + m_nPos;
	}
	inline int		GetCurPos() const
	{
		return	m_nPos;
	}
public:
	whcmdshrink()
	: m_pShrink(0), m_nShrinkSize(0), m_nShrinkSizeOld(0), m_nPos(0)
	, m_bNBO(false)
	{}
	// ע����ߵ�Read��Write��Ȼ�з��أ����ǳ�������throw����ʽ�����������Բ����д���ķ��ء�
	int	Read(void *pData, int nSize)
	{
		int	nPos = m_nPos + nSize;
		if( nPos>m_nShrinkSize )
		{
			// �޷�����
			throw	"Read size err!";			// �ⲿ��Ҫcatch���
			return	-1;
		}
		memcpy(pData, m_pShrink+m_nPos, nSize);
		m_nPos	= nPos;
		return	nSize;
	}
	// ������β�����ض����ĳ���
	int	ReadTillEnd(void *pData)
	{
		int	nSize	= m_nShrinkSize - m_nPos;
		if( nSize<=0 )
		{
			return	0;
		}
		memcpy(pData, m_pShrink+m_nPos, nSize);
		m_nPos	= m_nShrinkSize;
		return	nSize;
	}
	// ��ָ�����·ŵ���ͷ
	inline void	Rewind()
	{
		m_nPos	= 0;
	}
	// �������һ������
	int	Seek(int nSize)
	{
		m_nPos	+= nSize;
		if( m_nPos>m_nShrinkSize )
		{
			m_nPos	= m_nShrinkSize;
			throw	"Seek size err!";			// �ⲿ��Ҫcatch���
			return	-1;
		}
		return	0;
	}
	// ���ʣ�໹û�ж�������
	// ��Ҫͨ�������ж��Ƿ�������
	void *	GetDataNotRead(int *pnSize)
	{
		*pnSize	= m_nShrinkSize - m_nPos;
		return	m_pShrink+m_nPos;
	}
	template<typename _Ty>
	inline int	Read(_Ty *pData)
	{
		return	Read(pData, sizeof(*pData));
	}
	#define	WHCMDSHRINK_READ_NBO_DECLARE(ty, fn)	\
	inline int	Read(ty *pData)						\
	{												\
		int		rst;								\
		rst		= Read(pData, sizeof(*pData));		\
		if( m_bNBO )								\
		{											\
			*pData	= fn(*pData);					\
		}											\
		return	rst;								\
	}												\
	// EOF WHCMDSHRINK_READ_NBO_DECLARE
	WHCMDSHRINK_READ_NBO_DECLARE(int, ntohl);
	WHCMDSHRINK_READ_NBO_DECLARE(long, ntohl);
	WHCMDSHRINK_READ_NBO_DECLARE(short, ntohs);
	WHCMDSHRINK_READ_NBO_DECLARE(unsigned int, ntohl);
	WHCMDSHRINK_READ_NBO_DECLARE(unsigned long, ntohl);
	WHCMDSHRINK_READ_NBO_DECLARE(unsigned short, ntohs);
	// nMaxSize�ǰ������'\0'��β����󳤶�
	// ���ض����Ĳ�����0��β���ִ�����
	int	ReadStr(char *pszData, int nMaxSize)
	{
		char	*pShrink = m_pShrink+m_nPos;
		int		nLen = *(unsigned char*)pShrink;
		int		nPos = m_nPos + nLen+1;
		if( nPos>m_nShrinkSize || nLen>=nMaxSize )
		{
			// �޷�����
			throw	"ReadStr size err!";			// �ⲿ��Ҫcatch���
			return	-1;
		}
		memcpy(pszData, pShrink+1, nLen);
		pszData[nLen]	= 0;						// ������һ��0
		m_nPos	= nPos;
		return	nLen;
	}
	int	ReadBigStr(char *pszData, int nMaxSize)
	{
		char	*pShrink	= m_pShrink+m_nPos;
		int		nLen = *(unsigned short*)pShrink;
		int		nPos = m_nPos + nLen+sizeof(unsigned short);
		if( nPos>m_nShrinkSize || nLen>=nMaxSize )
		{
			// �޷�����
			throw	"ReadStr size err!";			// �ⲿ��Ҫcatch���
			return	-1;
		}
		memcpy(pszData, pShrink+sizeof(unsigned short), nLen);
		pszData[nLen]	= 0;						// ������һ��0
		m_nPos	= nPos;
		return	nLen;
	}
	// �ɱ䳤���ȣ��������ֱ����Writeд����Ӧ���ȵ�����
	int	ReadVSize(int *pnSize)
	{
		char	*pShrink	= m_pShrink+m_nPos;
		int		nSSize		= whbit_vn2_get(pShrink, GetSizeLeft(), (unsigned int *)pnSize);
		if( nSSize<=0 )
		{
			throw	"ReadVSize size err!";
			return	-1;
		}
		m_nPos	+= nSSize;
		return	nSSize;
	}
	// �ɱ䳤���ݣ��������ݳ��ȣ�
	int	ReadVData(void *pData, int nMaxSize)
	{
		// �ȶ��볤��
		int		nSize;
		int		nSSize		= ReadVSize(&nSize);
		if( nSSize<=0 )
		{
			throw	"ReadVData size err!";
			return	-1;
		}
		if( nSize>nMaxSize )
		{
			// ����������
			// ��Ҫ��ƫ��ָ���ƶ���ȥ
			m_nPos	-= nSSize;
			throw	"ReadVData buffer not enough!";
			return	-2;
		}
		// �ٶ�������
		int		nDSize		= Read(pData, nSize);
		if( nDSize<=0 )
		{
			m_nPos	-= nSSize;
			throw	"ReadVData Read err!";
			return	-3;
		}
		return	nDSize;
	}

	int	Write(const void *pData, int nSize)
	{
		int	nPos = m_nPos + nSize;
		if( nPos>m_nShrinkSize )
		{
			// �޷�д��
			throw	"Write size err!";			// �ⲿ��Ҫcatch���
			return	-1;
		}
		memcpy(m_pShrink+m_nPos, pData, nSize);
		m_nPos	= nPos;
		return	0;
	}
	template<typename _Ty>
	inline int	Write(const _Ty *pData)
	{
		return	Write(pData, sizeof(*pData));
	}
	#define	WHCMDSHRINK_WRITE_NBO_DECLARE(ty, fn)	\
	inline int	Write(const ty *pData)				\
	{												\
		ty	Data	= *pData;						\
		if( m_bNBO )								\
		{											\
			Data	= fn(*pData);					\
		}											\
		return	Write(&Data, sizeof(Data));			\
	}												\
	// EOF WHCMDSHRINK_WRITE_NBO_DECLARE
	WHCMDSHRINK_WRITE_NBO_DECLARE(int, htonl);
	WHCMDSHRINK_WRITE_NBO_DECLARE(long, htonl);
	WHCMDSHRINK_WRITE_NBO_DECLARE(short, htons);
	WHCMDSHRINK_WRITE_NBO_DECLARE(unsigned int, htonl);
	WHCMDSHRINK_WRITE_NBO_DECLARE(unsigned long, htonl);
	WHCMDSHRINK_WRITE_NBO_DECLARE(unsigned short, htons);
	int	WriteStr(const char *pszData)
	{
		// ע�⣺Write��֮��û�����һ��0�����Գ��Ȳ��ᳬ��ԭ����0��β�ִ��ĳ���
		char	*pShrink = m_pShrink+m_nPos;
		int		nLen = strlen(pszData);
		int		nPos = m_nPos + nLen+1;
		if( nPos>m_nShrinkSize || nLen>255 )
		{
			// �޷�д��
			throw	"WriteStr size err!";			// �ⲿ��Ҫcatch���
			return	-1;
		}
		*(unsigned char *)pShrink	= nLen;
		memcpy(pShrink+1, pszData, nLen);
		m_nPos	= nPos;
		return	0;
	}
	int	WriteBigStr(const char *pszData, unsigned short nLen)
	{
		// ������ԭ���ִ��ĳ��ȶ�һ��short
		char	*pShrink = m_pShrink+m_nPos;
		int		nPos = m_nPos + nLen+sizeof(unsigned short);
		if( nPos>m_nShrinkSize )
		{
			// �޷�д��
			throw	"WriteStr size err!";			// �ⲿ��Ҫcatch���
			return	-1;
		}
		*(unsigned short *)pShrink	= nLen;
		memcpy(pShrink+sizeof(unsigned short), pszData, nLen);
		m_nPos	= nPos;
		return	0;
	}
	int	WriteBigStr(const char *pszData)
	{
		return	WriteBigStr(pszData, strlen(pszData));
	}
	// �ɱ䳤���ȣ��������ֱ����Writeд����Ӧ���ȵ�����
	int	WriteVSize(int nSize)
	{
		if( GetSizeLeft()<4+nSize )
		{
			return	-1;
		}
		char	*pShrink	= m_pShrink+m_nPos;
		int		nSSize		= whbit_vn2_set(pShrink, nSize);
		m_nPos	+= nSSize;
		return	nSSize;
	}
	// �ɱ䳤���ݣ������ܳ��ȣ�
	int	WriteVData(const void *pData, int nSize)
	{
		// ��д�볤��
		int		nSSize		= WriteVSize(nSize);
		if( nSSize<0 )
		{
			return	-1;
		}
		// ��д������
		int		nDSize		= Write(pData, nSize);
		if( nDSize<0 )
		{
			return	-2;
		}
		return	nSSize + nDSize;
	}
	// ���������ڸ������
	inline void	WriteEnd()
	{
		m_nShrinkSize	= m_nPos;
	}
	// ����ת����һ�����ڱ���peek���������ݣ����ȡ���е�ĳ���֣�ת��Ϊ������
	#define	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(ty, fn)	\
	inline ty	Translate(ty val)					\
	{												\
		if( m_bNBO )								\
		{											\
			return	fn(val);						\
		}											\
		else										\
		{											\
			return	val;							\
		}											\
	}												\
	// EOF WHCMDSHRINK_TRANSLATE_NBO_DECLARE
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(int, ntohl);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(long, ntohl);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(short, ntohs);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(unsigned int, ntohl);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(unsigned long, ntohl);
	WHCMDSHRINK_TRANSLATE_NBO_DECLARE(unsigned short, ntohs);

	#define	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(ty, fn)	\
	inline ty	TranslateHToN(ty val)					\
	{												\
		if( m_bNBO )								\
		{											\
			return	fn(val);						\
		}											\
		else										\
		{											\
			return	val;							\
		}											\
	}												\
	// EOF WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(int, htonl);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(long, htonl);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(short, htons);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(unsigned int, htonl);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(unsigned long, htonl);
	WHCMDSHRINK_TRANSLATEHTON_NBO_DECLARE(unsigned short, htons);
};

////////////////////////////////////////////////////////////////////
// ��������
////////////////////////////////////////////////////////////////////
// ��������
typedef	unsigned int	whcmd_t;

enum
{
	// û�в���
	WHCMDFUNC_PARAMTYPE_v									= 0,
	// ������һ��voidָ��
	WHCMDFUNC_PARAMTYPE_p									= 1,
	// ������һ���ִ�
	WHCMDFUNC_PARAMTYPE_s									= 2,
	// ������һ������
	WHCMDFUNC_PARAMTYPE_i									= 11,
	// ��������������
	WHCMDFUNC_PARAMTYPE_ii									= 12,
	// ��������������
	WHCMDFUNC_PARAMTYPE_iii									= 13,
};

// �����Ҫ���ݽ����Ӧ��ͨ�����ڲ���ȫ�ֿɼ�������
class	whcmddealer;
typedef	void (whcmddealer::*whcmd_fc_t)(void);
typedef	void (whcmddealer::*whcmd_fc_v_t)(void);
typedef	void (whcmddealer::*whcmd_fc_p_t)(const void *, size_t);
typedef	void (whcmddealer::*whcmd_fc_c_t)(const char *);
typedef	void (whcmddealer::*whcmd_fc_i_t)(int);
typedef	void (whcmddealer::*whcmd_fc_ii_t)(int, int);
typedef	void (whcmddealer::*whcmd_fc_iii_t)(int, int, int);

#pragma pack(1)
// ͨ�õ�ָ��ṹ
struct	whcmd_cmd_buf_t
{
	whcmd_t	nCmd;
	char	szData[1];
};
// �����Ǹ��־�������Ľṹ
struct	whcmd_param_i_t
{
	int		nParam;
};
struct	whcmd_param_ii_t
{
	int		nParam1;
	int		nParam2;
};
struct	whcmd_param_iii_t
{
	int		nParam1;
	int		nParam2;
	int		nParam3;
};
#pragma pack()
// (��������������֤��_Ty�ṹ����szData��)
// ͨ�����ݳ��ȵõ��ܳ���
template<typename _Ty>
inline size_t	whcmd_gettotalsize(_Ty *pCmd, size_t nDataSize)
{
	return	sizeof(*pCmd)-sizeof(pCmd->szData) + nDataSize;
}
// ͨ���ܳ��ȵõ����ݳ���
template<typename _Ty>
inline size_t	whcmd_getdatasize(_Ty *pCmd, size_t nTotalSize)
{
	return	nTotalSize - sizeof(*pCmd)+sizeof(pCmd->szData);
}
// ��ָ��Ͳ������ϵ�һ��(�����ܳ���)
inline size_t	whcmd_assemble(char *szBuf, whcmd_t nCmd, const void *pParam, size_t nParamSize)
{
	whcmd_cmd_buf_t	*pBuf = (whcmd_cmd_buf_t *)szBuf;
	pBuf->nCmd	= nCmd;
	memcpy(pBuf->szData, pParam, nParamSize);
	return	whcmd_gettotalsize(pBuf, nParamSize);
}

struct	whcmd_entry_t
{
	const char			*cszKey;							// ������
	whcmd_t				nCmd;								// ��Ӧָ����
	int					nParamType;							// ��������WHCMDFUNC_PARAMTYPE_XXX
															// ��Ҫ������ʶ��������������
	whcmd_fc_t			pFunc;								// ����ָ��
};
struct	whcmd_map_t
{
	const char			*cszClassName;						// ���������
	const whcmd_map_t	*lpBaseMap;							// ָ�����whcmd_map_t�ṹ
	const whcmd_entry_t	*lpEntries;							// ����������������
};

// whcmddealer�����ʼ�������к���ע�ᶼ�Ǿ�̬��ɵ�
class	whcmddealer
{
public:
	virtual ~whcmddealer()	{}
	// ͨ���Ѿ��ҵ�Entry������Ӧ�ĺ���(public����ΪSearchAndDo�е�Job����Ҫ����)
	void	CallEntryDealCmd(const whcmd_entry_t *pEntry, const void *pData, size_t nSize);
	// ����nCmd�ҵ���Ӧ�ĺ�����������֮
	void	DealCmd(whcmd_t nCmd, const void *pData, size_t nSize);
	// ����cszFuncName�ҵ���Ӧ�ĺ�����������֮
	void	DealCmd(const char *cszFuncName, const void *pData, size_t nSize);
protected:
	// (ע����VC�У�ģ�庯��û��д���ඨ�����棬���Ծ�������)
	// ��������Entry��
	// ��pJob->Work����trueʱֹͣ��������������true
	// ���û��ƥ�䵽�򷵻�false
	template<class JOB>
	bool	SearchAndDo(JOB &Job)
	{
		const whcmd_map_t	*pMap		= GetCmdMap();
		const whcmd_entry_t	*pEntry;

		while(pMap)
		{
			pEntry	= pMap->lpEntries;
			// �ҷ��ϵĳ�Ա
			while( pEntry->cszKey )
			{
				if( Job.Work(pEntry) )
				{
					return	true;
				}
				pEntry	++;
			}
			// ���ϼ�
			pMap	= pMap->lpBaseMap;
		}

		// û���ҵ�����Entry
		return	false;
	}
protected:
	virtual const whcmd_map_t * GetCmdMap() const
	{
		// ������಻ʵ�־ͷ��ؿ�ָ��
		return	0;
	}
	virtual void	DealCmd_WhenUnknown(whcmd_t nCmd, const void *pData, size_t nSize)
	{}
	virtual void	DealCmd_WhenUnknown(const char *cszFuncName, const void *pData, size_t nSize)
	{}
};

////////////////////////////////////////////////////////////
// ��������ӳ�����ĸ�����
////////////////////////////////////////////////////////////
// ����Ҫ�к���ӳ�������һ��Ҫд���
// ���ඨ������Ҫ����仰
#define	WHCMD_DECLARE_MAP(_theClass)						\
private:													\
	typedef	_theClass	theClass;							\
	static const n_whcmn::whcmd_entry_t	_cmdEntries[];		\
	static const n_whcmn::whcmd_map_t	_cmdMap;			\
public:														\
	virtual const n_whcmn::whcmd_map_t * GetCmdMap() const;	\
	// **** EOF WHCMD_DECLARE_MAP

// ��ͨ���ӳ�䶨��
#define	WHCMD_MAP_BEGIN0(_theClass, _baseClassCmdMapPtr)	\
	const n_whcmn::whcmd_map_t * _theClass::GetCmdMap() const				\
		{ return &_theClass::_cmdMap; }						\
	const n_whcmn::whcmd_map_t _theClass::_cmdMap	=		\
		{													\
			#_theClass, _baseClassCmdMapPtr, _theClass::_cmdEntries			\
		};													\
	const n_whcmn::whcmd_entry_t _theClass::_cmdEntries[]	=				\
	{														\
	// **** EOF GC_BEGIN_MSG_MAP0
#define	WHCMD_MAP_BEGIN(_theClass, _baseClass)				\
	WHCMD_MAP_BEGIN0(_theClass, &_baseClass::_cmdMap)		\
	// **** EOF GC_BEGIN_MSG_MAP

// ����ӳ�䶨��(����Ҫ������Ա�������)
#define	WHCMD_MAP_BEGIN_AT_ROOT(_theClass)					\
	WHCMD_MAP_BEGIN0(_theClass, NULL)						\
	// **** EOF WHCMD_MAP_BEGIN_AT_ROOT

// ӳ�䶨�����
#define WHCMD_MAP_END()										\
		{ 0, 0, 0, 0 }										\
	};														\
	// **** EOF WHCMD_MAP_END

// �������ӳ�䶨��
#define	WHCMD_MAP_ON0(name, cmd, type, chk_type_t, func)	\
	{ name, cmd, type, (whcmd_fc_t)(chk_type_t)&theClass::func },			\
	// **** EOF WHCMD_MAP_ON0
// �ú��������ַ�����
#define	WHCMD_MAP_ON(cmd, type, chk_type_t, func)			\
	WHCMD_MAP_ON0(#func, cmd, type, chk_type_t, func)		\
	// **** EOF WHCMD_MAP_ON

// �����Ǹ����ض��������ͺ����Ķ��壬��WHCMDFUNC_PARAMTYPE_XXX��Ӧ
#define	WHCMD_MAP_ON_v(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_v, whcmd_fc_v_t, func)			\
	// **** EOF WHCMD_MAP_ON_v
#define	WHCMD_MAP_ON_p(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_p, whcmd_fc_p_t, func)			\
	// **** EOF WHCMD_MAP_ON_p
#define	WHCMD_MAP_ON_c(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_s, whcmd_fc_c_t, func)			\
	// **** EOF WHCMD_MAP_ON_c
#define	WHCMD_MAP_ON_i(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_i, whcmd_fc_i_t, func)			\
	// **** EOF WHCMD_MAP_ON_i
#define	WHCMD_MAP_ON_ii(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_ii, whcmd_fc_ii_t, func)			\
	// **** EOF WHCMD_MAP_ON_ii
#define	WHCMD_MAP_ON_iii(cmd, func)							\
	WHCMD_MAP_ON(cmd, WHCMDFUNC_PARAMTYPE_iii, whcmd_fc_iii_t, func)		\
	// **** EOF WHCMD_MAP_ON_iii

// ͨ��console������̶߳���
class	whcmn_nml_console
{
public:
	whsmpqueueWL	m_cmdqueue;								// ���ͨ�����������л��console������
	whtid_t			m_tid;									// console�����̵߳��߳�id
	char			m_szPrompt[64];							// console������ʾ����Ĭ��"$ "��
	char			m_szBuf[32768];							// һ�����붼���ᳬ�������
public:
	whcmn_nml_console();
	// �����߳�(ɱ���̲߳�����ͨ��m_tid����(����ֻ��ʹ�������)��һ��Ӧ��������"exit"���Զ�������ͬʱ���Ҳ����ͨ��m_cmdqueue�е�exit�������Լ��Ľ���)
	int		StartThread();
	// �ȴ����������tid���߳���Ҫ���Լ��Ľ������ƣ����磺�յ�exit��
	int		WaitThreadEnd(unsigned int nTimeout = 1000);
	// �������̵߳��õĲ���
	int		ThreadFunc();
};

// ͨ��ָ������
// ˵����
// 1. ָ��nCmd��Ҫ̫����ΪԽ���ڲ�����������Խ��(�����1024���£��ڲ�����Ϊ����65535�ͳ���)
// 2. ����������Լ��Ĺ��캯�����Լ�ע�����е�ָ��
template<class _TyDealer>
class	whcmn_cmd_reganddeal
{
protected:
	typedef	int (_TyDealer::*cmd_deal_f)(const void *pData, size_t nDSize);
	struct	CMDUNIT_T
	{
		_TyDealer				*pThis;
		cmd_deal_f				pFunc;
	};
	whvector<CMDUNIT_T>			m_vectCmdDeal;
	_TyDealer					*m_pDftDealer;
	cmd_deal_f					m_pDftFunc;
public:
	int							m_nUnknownCmd;
public:
	whcmn_cmd_reganddeal()
	: m_pDftDealer(NULL)
	, m_pDftFunc(NULL)
	, m_nUnknownCmd(0)
	{
		m_vectCmdDeal.reserve(1024);
	}
	// Ϊm_vectCmdDealԤ���ռ�
	inline void	Reserve(size_t nCap)
	{
		m_vectCmdDeal.reserve(nCap);
	}
	// ע��һ��ָ���Լ���Ӧ�ĺ���
	int		RegCmd(int nCmd, _TyDealer *pThis, cmd_deal_f pFunc)
	{
		if( nCmd<0 )
		{
			// Ĭ�Ϻ���
			m_pDftDealer	= pThis;
			m_pDftFunc		= pFunc;
			return	0;
		}
		if( nCmd>=65536 )
		{
			// ָ�Χ����
			assert(0);
			return	-1;
		}
		if( nCmd+1>(int)m_vectCmdDeal.size() )
		{
			size_t	nOldSize	= m_vectCmdDeal.size();
			m_vectCmdDeal.resize(nCmd+1);
			for(int i=nOldSize;i<=nCmd;i++)
			{
				// �Ȱ�û�õĶ���0
				m_vectCmdDeal[i].pThis	= NULL;
				m_vectCmdDeal[i].pFunc	= NULL;
			}
		}
		if( m_vectCmdDeal[nCmd].pFunc!=NULL )
		{
			// ��ָ��ԭ���Ѿ�ע�����
			assert(0);
			return	-2;
		}
		m_vectCmdDeal[nCmd].pThis		= pThis;
		m_vectCmdDeal[nCmd].pFunc		= pFunc;
		return	0;
	}
	// ����һ��ָ��
	int		CallFunc(int nCmd, const void *pData, size_t nDSize)
	{
		if( nCmd<0 || (size_t)nCmd>=m_vectCmdDeal.size() )
		{
			// ָ�Χ����
			// ��Ϊ�п�������Ϊ�����µģ������������⹥�������������ﲻ��assert
			return	-1000;
		}

		cmd_deal_f	pFunc	= m_vectCmdDeal[nCmd].pFunc;
		// if( !pFunc ) �����ᵼ��VC�������fatal error C1001: INTERNAL COMPILER ERROR��
		if( pFunc == NULL )
		{
			// û��������ָ��
			if( m_pDftDealer!=NULL
			&&  m_pDftFunc!=NULL
			)
			{
				// ����Ĭ��ָ��
				m_nUnknownCmd	= nCmd;
				return	(m_pDftDealer->*m_pDftFunc)(pData, nDSize);
			}
			return	-1001;
		}

		return	(m_vectCmdDeal[nCmd].pThis->*pFunc)(pData, nDSize);
	}
};
// ����������Ϊ�������õ�whcmn_cmd_reganddeal���û����ʹ�õ�
#define	WHCMN_CMD_REGANDDEAL_DECLARE0(classname, vname)		\
	whcmn_cmd_reganddeal<classname>	vname;					\
	typedef	classname	WHThisClass;						\
	// EOF WHCMN_CMD_REGANDDEAL_DECLARE0
#define	WHCMN_CMD_REGANDDEAL_REGCMD0(vname, nCmd, Func)		vname.RegCmd(nCmd, this, &WHThisClass::Func)
	// EOF WHCMN_CMD_REGANDDEAL_REGCMD0

#define	WHCMN_CMD_REGANDDEAL_DECLARE_m_rad(classname)		WHCMN_CMD_REGANDDEAL_DECLARE0(classname, m_rad)
	// EOF WHCMN_CMD_REGANDDEAL_DECLARE_m_rad
#define	WHCMN_CMD_REGANDDEAL_REGCMD(nCmd, Func)				WHCMN_CMD_REGANDDEAL_REGCMD0(m_rad, nCmd, Func)
	// EOF WHCMN_CMD_REGANDDEAL_REGCMD

// ͨ���ִ�ָ������
// ˵����
// 1. Ĭ��ָ����ִ�Сд
// 2. ����������Լ��Ĺ��캯�����Լ�ע�����е�ָ��
// 3. ��Ҫע����ǣ�_TyDealer�����Ǻ������ڲ���ʱ������࣬��������û����Ĵ�
template<class _TyDealer, class _TyStr = whstrptr4hashNocase, class _HashFunc = _whstr_hashfuncNocase>
class	whcmn_strcmd_reganddeal
{
protected:
	typedef	int (_TyDealer::*cmd_deal_f)(const char *pszParam);
	struct	CMDUNIT_T
	{
		_TyDealer				*pThis;
		cmd_deal_f				pFunc;
		const char				*cszHelp;					// 2006-02-10 �����㽨����ӡ���Ϊ�Զ�ָ�������
		unsigned int			nExecMask;					// 2008-07-22 �����㽨����ӡ�ָ���ִ���ж����롣
	};
	typedef	whhash<_TyStr, CMDUNIT_T, whcmnallocationobj, _HashFunc>	MYHASH_T;
	typedef	typename MYHASH_T::kv_iterator			MYHASHIT;
	MYHASH_T					m_mapCmd2Func;
	_TyDealer					*m_pDftDealer;
	cmd_deal_f					m_pDftFunc;
	char						m_szLINEEND[4];
public:
	char						m_szUnknownCmd[1024];		// �������������¼����ʶ���ָ��
	bool						m_bShouldHelp;				// �ڴ�������п������������ʾ��Ҫ�ϲ����help
	bool						m_bSortCmd;					// ��ʾ��Help��ʱ����cmd�������������sort�Ļ���ʽ��˳�����hash���е�˳�������Ƚ��ң�
public:
	whcmn_strcmd_reganddeal()
	: m_pDftDealer(NULL)
	, m_pDftFunc(NULL)
	, m_bShouldHelp(false)
	, m_bSortCmd(true)
	{
		m_szUnknownCmd[0]		= 0;
		strcpy(m_szLINEEND, WHLINEEND);
	}
	inline void	SetLineEnd(const char *cszLineEnd)
	{
		WH_STRNCPY0(m_szLINEEND, cszLineEnd);
	}
	inline void	SetSortCmd(bool bSet)
	{
		m_bSortCmd	= bSet;
	}
	// ע��һ��ָ���Լ���Ӧ�ĺ���
	int		RegCmd(const char *pszCmd, _TyDealer *pThis, cmd_deal_f pFunc, const char *cszHelp=NULL, unsigned int nExecMask=0xFFFFFFFF)
	{
		if( pszCmd==NULL )
		{
			// ����Ĭ�Ϻ���
			m_pDftDealer	= pThis;
			m_pDftFunc		= pFunc;
			return	0;
		}
		CMDUNIT_T	*pUnit;
		if( !m_mapCmd2Func.putalloc(pszCmd, pUnit) )
		{
			return	-1;
		}
		pUnit->pThis		= pThis;
		pUnit->pFunc		= pFunc;
		pUnit->cszHelp		= cszHelp;
		pUnit->nExecMask	= nExecMask;
		return	0;
	}
	// ����һ��ָ��
	int		CallFunc(const char *pszCmd, const char *pszParam, unsigned int nExecMask=0xFFFFFFFF)
	{
		m_bShouldHelp	= false;
		CMDUNIT_T	*pUnit;
		if( !m_mapCmd2Func.get(pszCmd, &pUnit) )
		{
			if( m_pDftDealer!=NULL
			&&  m_pDftFunc!=NULL
			)
			{
				// ����Ĭ��ָ��
				strcpy(m_szUnknownCmd, pszCmd);
				return	(m_pDftDealer->*m_pDftFunc)(pszParam);
			}
			return	-1001;
		}
		if( (nExecMask & pUnit->nExecMask) != 0 )
		{
			return	(pUnit->pThis->*pUnit->pFunc)(pszParam);
		}
		return		-1002;
	}
	// ���ָ�������Buffer�����pszCmdΪ�����������ָ�������
	// ����szBufferָ��
	const char *	Help(char *szBuffer, int nBufLen, const char *pszCmd=NULL)
	{
		char	*pszOldBuffer	= szBuffer;
		int		nOldBufLen		= nBufLen;
		bool	bIsPattern		= false;
		whsafeptr<whstrlike>	pwsl(whstrlike::Create());
		szBuffer[0]				= 0;	// ������ִ�
		if( pszCmd && strchr(pszCmd, '%') )
		{
			bIsPattern			= true;
			pwsl->SetPattern(pszCmd);
		}
		if( pszCmd == NULL || pszCmd[0]==0 || bIsPattern )
		{
			std::map<_TyStr, const char *>	sortList;
			// �г�ȫ��ָ��
			for(MYHASHIT itkv=m_mapCmd2Func.begin();itkv!=m_mapCmd2Func.end();++itkv)
			{
				CMDUNIT_T	&unit	= itkv.getvalue();
				if( unit.cszHelp != NULL )
				{
					if( bIsPattern )
					{
						// �����Ƿ�ģʽƥ��
						if( !pwsl->IsLike(itkv.getkey()) )
						{
							continue;
						}
					}
					if( m_bSortCmd )
					{
						sortList.insert( std::make_pair(itkv.getkey(), unit.cszHelp) );
					}
					else
					{
						int	rst	= snprintf(szBuffer, nBufLen, "%s %s%s", itkv.getkey().GetPtr(), unit.cszHelp, m_szLINEEND);
						szBuffer	+= rst;
						nBufLen		-= rst;
						if( rst<=0 || nBufLen<=0 )
						{
							// û�пռ���
							break;
						}
					}
				}
			}
			if( m_bSortCmd )
			{
				// ���map
				for(typename std::map<_TyStr, const char *>::iterator it=sortList.begin();it!=sortList.end();++it)
				{
					int	rst	= snprintf(szBuffer, nBufLen, "%s %s%s", (*it).first.GetPtr(), (*it).second, m_szLINEEND);
					szBuffer	+= rst;
					nBufLen		-= rst;
					if( rst<=0 || nBufLen<=0 )
					{
						// û�пռ���
						break;
					}
				}
			}
			if( nBufLen == nOldBufLen )
			{
				// ˵��û���ҵ��κ�ָ��
				snprintf(szBuffer, nBufLen, "[NO SUCH CMD]:%s", pszCmd);
			}
		}
		else
		{
			CMDUNIT_T	*pUnit;
			if( m_mapCmd2Func.get(pszCmd, &pUnit) )
			{
				// �г���Ӧ��ָ��
				if( pUnit->cszHelp )
				{
					snprintf(szBuffer, nBufLen, "%s %s", pszCmd, pUnit->cszHelp);
				}
				else
				{
					snprintf(szBuffer, nBufLen, "cmd:%s [NO HELP!]", pszCmd);
				}
			}
			else
			{
				// û��������ָ��
				snprintf(szBuffer, nBufLen, "[NO SUCH CMD]:%s", pszCmd);
			}
		}
		// Ϊ�˱��ո�������һ��0
		pszOldBuffer[nOldBufLen-1]	= 0;
		return	pszOldBuffer;
	}
};
// ����������Ϊ�������õ�whcmn_strcmd_reganddeal���û����ʹ�õ�
#define	whcmn_strcmd_reganddeal_DECLARE0(classname, vname)		\
	whcmn_strcmd_reganddeal<classname>	vname;					\
	typedef	classname	WHThisClass;							\
	// EOF whcmn_strcmd_reganddeal_DECLARE0
#define	whcmn_strcmd_reganddeal_REGCMD0(vname, pszCmd, Func, help, mask)	vname.RegCmd(pszCmd, this, &WHThisClass::Func, help, mask)
	// EOF whcmn_strcmd_reganddeal_REGCMD0

#define	whcmn_strcmd_reganddeal_DECLARE_m_srad(classname)			whcmn_strcmd_reganddeal_DECLARE0(classname, m_srad)
	// EOF whcmn_strcmd_reganddeal_DECLARE_m_srad
#define	whcmn_strcmd_reganddeal_REGCMD_m_srad0(pszCmd, Func, help, mask)	whcmn_strcmd_reganddeal_REGCMD0(m_srad, pszCmd, Func, help, mask)
	// EOF whcmn_strcmd_reganddeal_REGCMD_m_srad0
#define	whcmn_strcmd_reganddeal_REGCMD_m_srad(pszCmd, Func, help)	whcmn_strcmd_reganddeal_REGCMD_m_srad0(pszCmd, Func, help, 0xFFFFFFFF)
	// EOF whcmn_strcmd_reganddeal_REGCMD_m_srad
#define	whcmn_strcmd_reganddeal_REGCMD_m_srad_smp0(cmd, help, mask)	whcmn_strcmd_reganddeal_REGCMD_m_srad0(#cmd, _i_srad_##cmd, help, mask)
	// EOF whcmn_strcmd_reganddeal_REGCMD
#define	whcmn_strcmd_reganddeal_REGCMD_m_srad_smp(cmd, help)		whcmn_strcmd_reganddeal_REGCMD_m_srad(#cmd, _i_srad_##cmd, help)
	// EOF whcmn_strcmd_reganddeal_REGCMD

// �������Դ������ս���ŵ�vect�У�
// <randstr>:size		����size���ȵ�����ִ�
// <randbin>:size		����size���ȵ��������������
// <dupstr>:num,str		����str�ظ�num�ε��ִ�
// <dupbin>:num,hexstr	����hexstr��ʾ�Ķ����������ظ�num�εĶ���������
// <file>:filename		���ļ����ݶ���
int	whcmd_make_tststr(const char *cszStr, whvector<char> *pvect, bool *pbIsStr);

}		// EOF namespace n_whcmn

#endif	// EOF __WHCMD_H__

