// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdataobj.h
// Creator      : Wei Hua (κ��)
// Comment      : ����Ϊ�߼����ṩ�򵥵����Ի�ȡ�����÷�����ͬʱ���صײ�����ݴ�ȡϸ��
//               ��������δ�����Դ��ʼ����������ڹ����ڴ��������������ڷ���崻��¼���ӹ����ڴ��лָ����ݲ����´洢����ν��������洢�ȣ���
//                �뷨�����ܻ�����Զ����ɴ��롣
//                ע�����ȷ������Ҫʹ��list��Ա����whdataobjbase_t��صĻ������ݶ������ֱ��new��������ʹ�þֲ��������μ�tty_common_PlayerData.h�е�˵����
// CreationDate : 2006-02-14
// ChangeLOG    : 2006-02-22 ��������������
//              : 2006-02-27 list����ԭ��WriteValueToStream��ʱ���ǰ���hash������˳�������ᵼ��ReadValueFromStream�±��е�Ԫ��˳����ܺ��������˳��ͬ�����Ը�Ϊ����m_pList�����˳��
//              : 2006-03-08 �����˶���ν�βָ��WHDATAOBJ_CMD_ENDOFOBJ������������һ�����д洢��β�ͬ�������ݡ�������ϵ�Э���Ǽ��ݵġ���Э��������ֻ��һ���������û�н�βָ���ȡ������û�����ݵ�ʱ�������
//              : 2006-03-16 ������ReadValueFromStream�����ڲ�����������Ϊmodified�Ĺ��ܡ�����ͨ�������µĸ���Ҳ���ñ���¼���������������Ϳ���ͨ�������û����ݽ��������޸ġ�
//              :            ����Ҫ��ס���û�����������ʼ������ʼ������Ҫ����ClearAllModified��������и��ı�־��
//              : 2006-03-21 ������whdataobjbase_t��IsModified�������ɱ��ϲ������ж����������Ƿ����˸ı䡣
//              : 2006-04-18 ������ԭ��whdataobj_listtype_prop_t��һЩ�ط�֮ɾ����hash���е���Ŀû��ɾ��list�е���Ŀ�Ĵ���
//              :            ������whdataobj_listtype_prop_t::ClearModifiedWithLockû�аѱ��Ϊɾ������Ŀ����ɾ���Ĵ���
//              :            ���list��Ŀ����ӣ���ʱ����ʧ�ܣ���ɾ������ʱ����ʧ�ܣ�������ӣ���ʱ��Ŀ��״̬���Ǳ��޸ģ������Ǳ���ӡ�
//              : 2006-04-24 ������list��end����
//              : 2006-06-02 ������writeallʱ��DEL������Ҳ�������ݿ�Ĵ���
//              : 2006-06-26 ��writeallʱ�����޸ĵ�Ԫ��״̬������ֱ�ӷ����д�뵥Ԫ���������ᵼ��Ԫ�ص��޸�״̬���ı䣨���������й����н�������dump��û������*^_^*��
//              : 2007-04-05 ������whdataobj_charptrtype_utf8based_prop_t�ڴӹ����ڴ��лָ�����ʱû�������ظ�UTF8���ݣ����´������ݳ�������⡣
//              : 2007-05-28 �����˴�stream�ж�ȡ����ʱ����ѡ�������޸ı�ǣ�������ͱ��ص��Ѿ��޸ĵķ�����ͻ�򱾵ظ���ʧЧ�Ļ��ơ�
//              : 2007-06-21 ������whdataobj_listtype_prop_t��DelAll����������һ������������б�
//              : 2007-08-22 ȡ����hash��m_mapNameID2PropPtr����Ϊֱ��������ͺ��ˡ��������������ڰ�auto����Ҳ�ŵ������ڴ���ȥ��
//              : 2007-08-24 ȡ���������Զ������е�ʵ���ڴ���䡢�ͷŲ������ù���������Ӧ����ȡ����֮
//              : 2007-09-29 ������list��adjust��û�е���hash������ֵ�ڵ�ָ�������
//              : 2007-11-07 ȡ���˸�����ȡ�����е�assert����Ϊ���ڿ������������ݻ�ӿͻ��˴�������
//              : 2007-11-21 ������marker���ͣ����ڸ��������Ǹ�������
//              : 2007-11-22 ������IsToWrite()������������������marker����������Զ���Ǳ��޸�״̬��bug
//              : 2007-12-11 �����е��������Ͷ�����ֻ��ѡ��
//              : 2008-01-15 ������list���͵��߼��ߴ磬�����ڱ��ɾ������û��ɾ��ʱͳ�Ƶĸ�������
//              : 2008-03-04 �����˱���whdataobj_arraytype_prop_t�ڶ�ȡ����ʱ�±�Խ���assert��������Դ���������ݳ��ȣ���ʹ�÷�û������ʱ�Ϳ��ܷ���Խ��������
//              :            ��whdataobj_arraytype_prop_t�������Զ�����Ԫ��ȡ��modified��״̬�Ĺ���SetZeroNotModified
//              : 2008-03-19 ������whdataobj_listtype_prop_t��AddModify��Ԫ��Ϊ�ո����������ڴ��̹����У�֮�����ɾ����������Ԫ��ɾ�������û�и���DB�����⡣
//              :            �������ɸ�ޱ�����������Ϸ���:)

#ifndef	__WHDATAOBJ_H__
#define	__WHDATAOBJ_H__

#include "whfile.h"
#include "whallocmgr2.h"
#include "whdataobj_def.h"
#include "whbits.h"

namespace n_whcmn
{

// ���ݶ�������Ի���(���ֶ�����Ϊ���麯�������ԾͲ��òŹ����ڴ��������ˡ�������Ҳ�����ڻָ���ʱ��������)
template<class _AllocationObj>
class	whdataobj_prop_t
{
public:
	typedef	whallocmgr2<_AllocationObj>			AM_T;
	// ���������Ի���Ҫ�����ģ���Ϊ�����������ʣ�Ҳ���д
	WHDATAOBJ_NAMEID_T	m_nNameID;									// �����ݿ�ͨѶ���������ĸ�����
	unsigned char		m_nProp;									// ��������
	enum
	{
		PROP_RDONLY		= 0x01,										// ��ʾ������Ϊֻ���������κ�����¶����ܴ���
	};
	// ����ֻ��
	void	SetRDOnly(bool bSet)
	{
		if( bSet )
		{
			whcmnbit_set(m_nProp, (unsigned char)PROP_RDONLY);
		}
		else
		{
			whcmnbit_clr(m_nProp, (unsigned char)PROP_RDONLY);
		}
	}
public:
	whdataobj_prop_t()
		: m_nNameID(0)
		, m_nProp(0)
	{
	}
	whdataobj_prop_t(const NULLCONSTRUCT_T &nc)
	{
		NOTUSE(nc);
	}
	virtual	~whdataobj_prop_t()										{};
	// ���ڰ��ڲ���������
	virtual void	NewInner(void *pMem)							= 0;
	// ���ظ�Ԫ���Ƿ��޸Ĺ���
	virtual bool	IsModified() const								= 0;
	// ��ʾ��Ԫ���Ƿ�Ӧ�ñ�д�루һ����˵�ǵȼ���IsModified��
	virtual bool	IsToWrite() const
	{
		if( whcmnbit_chk(m_nProp, (unsigned char)PROP_RDONLY) )	// �����ֻ����ͱ�ʾ����д
		{
			return	false;
		}
		return	IsModified();
	}
	// ������е��޸�״̬
	virtual void	ClearModified()									= 0;
	// ���ñ��������޸�״̬
	virtual void	SetModifiedWithLock()							= 0;
	// ������������޸�״̬
	virtual void	ClearModifiedWithLock()							= 0;
	// �������ֵ��ͬʱҲ����޸ı�־�����ڻ��һ�����ɾ����Ķ���ע�⣬�������DelAll������
	virtual void	ClearValue()									= 0;
	// ���Լ�������д������
	virtual int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
																	= 0;
	// �����ж������������Լ�
	// ���bSetModifyΪ��������Modify��־�������������ݶ���ᱻ��Ϊ��û�иı���ģ�������������������Ѿ���������modify�򲻽����޸�
	virtual int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
																	= 0;
	// �����ڲ�ָ��
	virtual void	AdjustInnerPtr(AM_T *pMgr, long nOffset)		= 0;
	// ���ֵ�Ļ��������ϲ�Ӧ��֪����ʲô���͵ģ�
	virtual void *	GetValueBuf()									= 0;
};

// ��Ϸ�е����ݶ��󶼴�whdataobjbase_t����
template<class _AllocationObj>
struct	whdataobjbase_t
{
protected:
	WHDATAOBJ_NAMEID_T				m_ver;							// �Լ��İ汾
	WHDATAOBJ_NAMEID_T				m_vermin;						// ���ݵ���Ͱ汾����������������շ�����ȷ���Ƿ���Զ�ȡ��
	bool							m_bWriteMarker;					// �Ƿ���WriteModifiedValueToStream�м�¼marker�����Ĭ���ǲ��򿪵ģ�
public:
	// ����Ǳ��붨��ģ����������ط�Ҫ�ã���д��public���ˣ�
	typedef	whallocmgr2<_AllocationObj>			AM_T;
	typename AM_T::template vector<whdataobj_prop_t<_AllocationObj> *>		m_vectNameID2PropPtr;
																	// m_vectNameID2PropPtr������ŷ����������ָ��
public:
	whdataobjbase_t()
		: m_ver(0), m_vermin(0)
		, m_bWriteMarker(false)
	{
	}
	inline void	SetWriteMarker(bool bSet)
	{
		m_bWriteMarker	= bSet;
	}
	inline bool	GetWriteMarker() const
	{
		return	m_bWriteMarker;
	}
	inline WHDATAOBJ_NAMEID_T	GetVer() const
	{
		return	m_ver;
	}
	inline void	SetVer(WHDATAOBJ_NAMEID_T ver, WHDATAOBJ_NAMEID_T vermin)
	{
		m_ver		= ver;
		m_vermin	= vermin;
	}
	// ������󲢵��ûص�
	int	BrowseData(void (*pFuncNotify)(whdataobj_prop_t<_AllocationObj> *))
	{
		for(size_t i=0; i<m_vectNameID2PropPtr.size(); ++i)
		{
			(*pFuncNotify)(m_vectNameID2PropPtr[i]);
		}
		return	0;
	}
	// д������ͷ����Ϣ
	void	_WriteToStream_Head(char *&pBufStream, int &nSizeLeft)
	{
		// ����д��汾
		*(WHDATAOBJ_CMD_T*)pBufStream	= WHDATAOBJ_CMD_VERSION;
		pBufStream	+= sizeof(WHDATAOBJ_CMD_T);
		nSizeLeft	-= sizeof(WHDATAOBJ_CMD_T);
		*(WHDATAOBJ_NAMEID_T*)pBufStream= m_vermin;
		pBufStream	+= sizeof(WHDATAOBJ_NAMEID_T);
		nSizeLeft	-= sizeof(WHDATAOBJ_NAMEID_T);
		*(WHDATAOBJ_NAMEID_T*)pBufStream= m_ver;
		pBufStream	+= sizeof(WHDATAOBJ_NAMEID_T);
		nSizeLeft	-= sizeof(WHDATAOBJ_NAMEID_T);
	}
	// �������������Ĺ���д�뻺��
	// �ɹ�����0��������<0
	int	WriteModifiedValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll=false)
	{
		_WriteToStream_Head(pBufStream, nSizeLeft);
		// Ȼ��д������
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			if( pProp->IsToWrite() || bWriteAll )
			{
				if( pProp->WriteValueToStream(pBufStream, nSizeLeft, bWriteAll)<0 )
				{
					return	-pProp->m_nNameID;
				}
			}
		}
		return	0;
	}
	// �Ѹ����޸Ĺ��Ĳ��ֵ�ͳ����Ϣ�����
	// Ŀǰֻ����stream��ռ�õĿռ���
	int	StatModified(char *pszDispBuf, int nMaxDispSize, bool bWriteAll)
	{
		char	szBufStream[32768];
		char	*pBufStream	= szBufStream;
		int		nSizeLeft	= sizeof(szBufStream);
		_WriteToStream_Head(pBufStream, nSizeLeft);
		int		nDispRst	= snprintf(pszDispBuf, nMaxDispSize, "hdr %d%s", sizeof(szBufStream)-nSizeLeft, WHLINEEND);
		pszDispBuf			+= nDispRst;
		nMaxDispSize		-= nDispRst;
		// Ȼ��д������
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			if( pProp->IsModified() || bWriteAll )
			{
				pBufStream		= szBufStream;
				nSizeLeft		= sizeof(szBufStream);
				if( pProp->WriteValueToStream(pBufStream, nSizeLeft, bWriteAll)<0 )
				{
					return	-pProp->m_nNameID;
				}
				nDispRst		= snprintf(pszDispBuf, nMaxDispSize, "%d %d%s", pProp->m_nNameID, sizeof(szBufStream)-nSizeLeft, WHLINEEND);
				pszDispBuf		+= nDispRst;
				nMaxDispSize	-= nDispRst;
			}
		}
		return	0;
	}
	// �����������е����ݶԱ������ݽ����޸�
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify=true)
	{
		WHDATAOBJ_CMD_T		cmd;
		WHDATAOBJ_NAMEID_T	nNameID;
		// ���ȼ��汾
		cmd			= *(WHDATAOBJ_CMD_T *)pBufStream;
		pBufStream	+= sizeof(cmd);
		nSizeLeft	-= sizeof(cmd);
		if( cmd != WHDATAOBJ_CMD_VERSION )
		{
			return	-1;
		}
		if( nSizeLeft<=0 )
		{
			return	-2;
		}
		nNameID		= *(WHDATAOBJ_NAMEID_T *)pBufStream;
		pBufStream	+= sizeof(nNameID);
		nSizeLeft	-= sizeof(nNameID);
		if( nNameID > m_ver )
		{
			// ˵���Է��������Լ��İ汾
			return	-3;
		}
		if( nSizeLeft<=0 )
		{
			return	-4;
		}
		nNameID		= *(WHDATAOBJ_NAMEID_T *)pBufStream;
		pBufStream	+= sizeof(nNameID);
		nSizeLeft	-= sizeof(nNameID);
		if( nNameID < m_vermin )
		{
			// ˵���Լ������ݶԷ��İ汾
			return	-5;
		}
		if( nSizeLeft<0 )
		{
			return	-6;
		}
		if( nSizeLeft==0 )
		{
			// û�����ݣ�������Ǻ���ֵ����飩
			return	0;
		}
		// 
		while( nSizeLeft>0 )
		{
			cmd			= *(WHDATAOBJ_CMD_T *)pBufStream;
			pBufStream	+= sizeof(cmd);
			nSizeLeft	-= sizeof(cmd);
			if( cmd == WHDATAOBJ_CMD_ENDOFOBJ )
			{
				// �˶ζ������
				break;
			}
			nNameID		= *(WHDATAOBJ_NAMEID_T *)pBufStream;
			pBufStream	+= sizeof(nNameID);
			nSizeLeft	-= sizeof(nNameID);
			if(nSizeLeft<=0)
			{
				switch( cmd )
				{
				case	WHDATAOBJ_CMD_MARKER:
					{
						// ������Լ���
					}
					break;
				default:
					{
						// assert(0);	��Ϊ�������Ҳ�����Ǵӿͻ��˷����ģ����Բ�Ҫassert��
						return	-11;
					}
					break;
				}
			}
			// �ж�nNamdID�Ƿ�Ϸ�����Ϊ�ͻ��˿��ܷ����Ƿ������ݣ�
			if( !m_vectNameID2PropPtr.IsPosValid(nNameID) )
			{
				return	-20;
			}
			// ����nNameID�ҵ���Ӧ��prop
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[nNameID];
			if( pProp )	// ����Ҳ����Ͳ��ü����ˣ�˵�������������ӵĶ���������Ҳ��Ӧ�ó���
			{
				switch( cmd )
				{
				case	WHDATAOBJ_CMD_MARKER:
				case	WHDATAOBJ_CMD_VAR_MODIFY:
				case	WHDATAOBJ_CMD_LIST_MODIFY_BEGIN:
				case	WHDATAOBJ_CMD_ARR_MODIFY_BEGIN:
					{
						if( pProp->ReadValueFromStream(pBufStream, nSizeLeft, bSetModify)<0 )
						{
							return	-21;
						}
					}
					break;
				default:
					{
						// ����ǲ�Ӧ���е�
						// assert(0);	��Ϊ�������Ҳ�����Ǵӿͻ��˷����ģ����Բ�Ҫassert��
						return	-22;
					}
					break;
				}
			}
			else
			{
				return	-23;
			}
		}
		return	0;
	}
	// �������ݶ����Ƿ��Ǳ��޸Ĺ���
	bool	IsModified()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			if( pProp->IsModified() )
			{
				return	true;
			}
		}
		return	false;
	}
	// ������и��ı�־(���Ӧ�����ڴ��ʼ��ʼ��ReadValueFromStream֮����õģ��Ա�֤֮���µ��޸ĲŻᱻ���)
	// �������ȡ��ʱ��������bSetModifyΪ�٣�����Բ������������ˣ�
	void	ClearAllModified()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			pProp->ClearModified();
		}
	}
	// �������޸�״̬�ĳ�Ա����LOCK״̬(���Ӧ�����ڷ�������ָ��֮�����õ�)
	void	SetAllModifiedWithLock()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			pProp->SetModifiedWithLock();
		}
	}
	// ������������ĸ��ı�־(���Ӧ�����ڳɹ�����֮����õ�)
	void	ClearAllModifiedWithLock()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			pProp->ClearModifiedWithLock();
		}
	}
	// �������ֵ
	void	ClearAllValue()
	{
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*pProp	= m_vectNameID2PropPtr[i];
			pProp->ClearValue();
		}
	}
	// pGoodBuddy��һ�������Ķ���
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset, whdataobjbase_t *pGoodBuddy)
	{
		// ��vector�ڲ��������һ��
		m_vectNameID2PropPtr.AdjustInnerPtr(pMgr, nOffset);
		// ��ÿ��ָ�붼����һ��
		for(size_t i=0;i<m_vectNameID2PropPtr.size(); ++i)
		{
			whdataobj_prop_t<_AllocationObj>	*&pObj	= m_vectNameID2PropPtr[i];
			wh_adjustaddrbyoffset(pObj, nOffset);
			pGoodBuddy->m_vectNameID2PropPtr[i]->NewInner(pObj);
			pObj->AdjustInnerPtr(pMgr, nOffset);
		}
	}
	void *	GetValueBuf(WHDATAOBJ_NAMEID_T nNameID)
	{
		if( nNameID>=(int)m_vectNameID2PropPtr.size() )
		{
			return	NULL;
		}
		return	m_vectNameID2PropPtr[nNameID]->GetValueBuf();
	}
	template<typename _Ty>
	void	GetValue(int nNameID, _Ty &val)
	{
		void	*ptr	= GetValueBuf(nNameID);
		if( ptr )
		{
			val	= *(_Ty *)ptr;
		}
	}
};

template<class _AllocationObj>
class	whdataobj_marker_prop_t	: public whdataobj_prop_t<_AllocationObj>
{
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
	typedef	whallocmgr2<_AllocationObj>			AM_T;
protected:
	whdataobjbase_t<_AllocationObj>				*m_pHost;		// ����ĸ��
public:
	whdataobj_marker_prop_t()
		: m_pHost(NULL)
	{
	}
	whdataobj_marker_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	// ���ڰ��ڲ���������
	virtual void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_marker_prop_t(NULLCONSTRUECTOR);
	}
	// ���ظ�Ԫ���Ƿ��޸Ĺ��ˣ������Զ��Ӧ��Ӱ���Ƿ�Modified���жϣ�
	virtual bool	IsModified() const
	{
		return	false;
	}
	// ��ʾ��Ԫ���Ƿ�Ӧ�ñ�д�루һ����˵�ǵȼ���IsModified�������������������Զд�룬ֻҪ�������޸ģ������Ҫд�룩
	virtual bool	IsToWrite() const
	{
		return	true;
	}
	// ������е��޸�״̬
	virtual void	ClearModified()
	{
	}
	// ���ñ��������޸�״̬
	virtual void	SetModifiedWithLock()
	{
	}
	// ������������޸�״̬
	virtual void	ClearModifiedWithLock()
	{
	}
	// �������ֵ��ͬʱҲ����޸ı�־�����ڻ��һ�����ɾ����Ķ���ע�⣬�������DelAll������
	virtual void	ClearValue()
	{
	}
	// ���Լ�������д������
	virtual int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		if( !m_pHost->GetWriteMarker() )
		{
			// ����ϲ㲻��дmarker������Ͳ���д��
			return	0;
		}
		// һ����˵����ṩ��buffer���ǹ�����
		whfile_i_mem	f(pBufStream, nSizeLeft);
		// ָ���������
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_MARKER;
		f.Write(&cmd);
		// д��NameID
		f.Write(&this->m_nNameID);
		// ���ĺ����д��ָ��pBufStream����󳤶�
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	// �����ж������������Լ�
	// ���bSetModifyΪ��������Modify��־�������������ݶ���ᱻ��Ϊ��û�иı���ģ�������������������Ѿ���������modify�򲻽����޸�
	virtual int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		return	0;
	}
	// �����ڲ�ָ��
	virtual void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// ����������ָ��
		wh_adjustaddrbyoffset(m_pHost, nOffset);
	}
	// ���ֵ�Ļ��������ϲ�Ӧ��֪����ʲô���͵ģ�
	virtual void *	GetValueBuf()
	{
		return	NULL;
	}
public:
	// �Լ��õ�
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, WHDATAOBJ_NAMEID_T nNameID)
	{
		m_pHost					= pHost;
		this->m_nNameID			= nNameID;
		// ����ӳ���
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
	}
};

// �̶��������ͣ�����һ�����ͺͽṹ�ȣ�
template<typename _Ty, class _AllocationObj>
class	whdataobj_cmntype_prop_t	: public whdataobj_prop_t<_AllocationObj>
{
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
	typedef	whallocmgr2<_AllocationObj>			AM_T;
protected:
	enum
	{
		MDSTAT_LOCKED	= 0x80,		// ��ʾ������������
		MDSTAT_MODIFIED	= 0x01,		// ��ʾ���޸���
	};
	unsigned char	m_nMDStat;		// ��ʾ�Ƿ��޸�
	whdataobjbase_t<_AllocationObj>	*m_pHost;		// ����ĸ��
	_Ty		*m_pVar;
public:
	whdataobj_cmntype_prop_t()
		: m_nMDStat(0)
		, m_pHost(NULL)
		, m_pVar(NULL)
	{
	}
	whdataobj_cmntype_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, _Ty *pVar, WHDATAOBJ_NAMEID_T nNameID)
	{
		m_pHost					= pHost;
		m_pVar					= pVar;
		this->m_nNameID			= nNameID;
		// ����ӳ���
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
	}
	inline const _Ty &	get() const
	{
		return	*m_pVar;
	}
	inline void	set(const _Ty & val)
	{
		*m_pVar		= val;
		// ���ԭ����lock������������lock״̬�����
		m_nMDStat	= MDSTAT_MODIFIED;
	}
	inline _Ty *	setbyptr()
	{
		// ���ԭ����lock������������lock״̬�����
		m_nMDStat	= MDSTAT_MODIFIED;
		return		m_pVar;
	}
	bool	IsModified() const
	{
		return	(m_nMDStat & MDSTAT_MODIFIED) != 0;
	}
	void	ClearModified()
	{
		m_nMDStat	= 0;
	}
	void	SetModifiedWithLock()
	{
		if( m_nMDStat != 0  )
		{
			m_nMDStat	|= MDSTAT_LOCKED;
		}
	}
	void	ClearModifiedWithLock()
	{
		if( (m_nMDStat & MDSTAT_LOCKED) != 0 )
		{
			// ֻ����LOCK������û�б������޸Ĺ���������״̬
			m_nMDStat	= 0;
		}
	}
	void	ClearValue()
	{
		memset(m_pVar, 0, sizeof(*m_pVar));
		ClearModified();
	}
	int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		// һ����˵����ṩ��buffer���ǹ�����
		whfile_i_mem	f(pBufStream, nSizeLeft);
		// ָ���������
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_VAR_MODIFY;
		f.Write(&cmd);
		// д��NameID
		f.Write(&this->m_nNameID);
		// д������(��Ϊ�����ǹ̶���)
		f.Write(m_pVar);
		// ���ĺ����д��ָ��pBufStream����󳤶�
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		// �ϲ�Ӧ���Ѿ�������ָ���NameID����
		memcpy(m_pVar, pBufStream, sizeof(*m_pVar));
		pBufStream	+= sizeof(*m_pVar);
		nSizeLeft	-= sizeof(*m_pVar);
		if( nSizeLeft>=0 )
		{
			if( bSetModify )
			{
				m_nMDStat	= MDSTAT_MODIFIED;	// ��������ֱ�ӾͿ��԰�locked��״̬���
			}
			else
			{
				ClearModified();
			}
			return	0;
		}
		return	-1;
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// ����������ָ��
		wh_adjustaddrbyoffset(m_pHost, nOffset);
		// ��������ָ��
		wh_adjustaddrbyoffset(m_pVar, nOffset);
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_cmntype_prop_t(NULLCONSTRUECTOR);
	}
	void *	GetValueBuf()
	{
		return	m_pVar;
	}
};

// �ִ�����
template<class _AllocationObj>
class	whdataobj_charptrtype_prop_t	: public whdataobj_prop_t<_AllocationObj>
{
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
	typedef	whallocmgr2<_AllocationObj>			AM_T;
protected:
	enum
	{
		MDSTAT_LOCKED	= 0x80,		// ��ʾ������������
		MDSTAT_MODIFIED	= 0x01,		// ��ʾ���޸���
	};
	unsigned char	m_nMDStat;		// ��ʾ�Ƿ��޸�
	whdataobjbase_t<_AllocationObj>	*m_pHost;	// ����ĸ��
	char	*m_pVar;				// �ִ�ָ��
	size_t	m_nMaxSize;				// �ִ�������󳤶�
public:
	whdataobj_charptrtype_prop_t()
		: m_nMDStat(0)
		, m_pHost(NULL)
		, m_pVar(NULL)
		, m_nMaxSize(0)
	{
	}
	whdataobj_charptrtype_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, char *pVar, size_t nMaxSize, WHDATAOBJ_NAMEID_T nNameID)
	{
		m_pHost					= pHost;
		m_pVar					= pVar;
		m_nMaxSize				= nMaxSize;
		this->m_nNameID			= nNameID;
		// ����ӳ���
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
	}
	inline const char *	get() const
	{
		return	m_pVar;
	}
	inline void	set(const char *pStr)
	{
		strncpy(m_pVar, pStr, m_nMaxSize-1);
		m_pVar[m_nMaxSize-1]	= 0;
		m_nMDStat	= MDSTAT_MODIFIED;
	}
	inline size_t	getmaxsize() const
	{
		return	m_nMaxSize;
	}
	inline char *	setbyptr()	// ע�⣺����͵��ⲿ�Լ��ܺó����ˡ����ܳ���Ŷ��!!!!
	{
		m_nMDStat	= MDSTAT_MODIFIED;
		return		m_pVar;
	}
	bool	IsModified() const
	{
		return	(m_nMDStat & MDSTAT_MODIFIED) != 0;
	}
	void	ClearModified()
	{
		m_nMDStat	= 0;
	}
	void	SetModifiedWithLock()
	{
		if( m_nMDStat != 0  )
		{
			m_nMDStat	|= MDSTAT_LOCKED;
		}
	}
	void	ClearModifiedWithLock()
	{
		if( (m_nMDStat & MDSTAT_LOCKED) != 0 )
		{
			// ֻ����LOCK������û�б������޸Ĺ���������״̬
			m_nMDStat	= 0;
		}
	}
	void	ClearValue()
	{
		m_pVar[0]	= 0;
		ClearModified();
	}
	int		_WriteValueToStream(const char *pszStr, char *&pBufStream, int &nSizeLeft, bool bWriteAll, int nMaxSize)
	{
		// һ����˵����ṩ��buffer���ǹ�����
		whfile_i_mem	f(pBufStream, nSizeLeft);
		// ָ���������
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_VAR_MODIFY;
		f.Write(&cmd);
		// д��NameID
		f.Write(&this->m_nNameID);
		// ��д��䳤�ִ���nMaxSize������֤pszStr������Ϊ��������¹�����
		if( f.WriteVStr(pszStr, nMaxSize)<0 )
		{
			return	-1;
		}
		// ���ĺ����д��ָ��pBufStream����󳤶�
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		return	_WriteValueToStream(m_pVar, pBufStream, nSizeLeft, bWriteAll, m_nMaxSize);
	}
	int		_ReadValueFromStream(char *pszStr, int nMaxSize, char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		// �ϲ�Ӧ���Ѿ�������ָ���NameID����
		// �������������ִ�����
		whfile_i_mem	f(pBufStream, nSizeLeft);
		if( f.ReadVStr(pszStr, nMaxSize)<=0 )
		{
			return	-1;
		}
		// ���ĺ����д��ָ��pBufStream����󳤶�
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		if( nSizeLeft>=0 )
		{
			if( bSetModify )
			{
				m_nMDStat		= MDSTAT_MODIFIED;	// ��������ֱ�ӾͿ��԰�locked��״̬���
			}
			else
			{
				ClearModified();
			}
			return	0;
		}
		return	-1;
	}
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		return	_ReadValueFromStream(m_pVar, m_nMaxSize, pBufStream, nSizeLeft, bSetModify);
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// ����������ָ��
		wh_adjustaddrbyoffset(m_pHost, nOffset);
		// �����ִ�ָ��
		wh_adjustaddrbyoffset(m_pVar, nOffset);
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_charptrtype_prop_t(NULLCONSTRUECTOR);
	}
	void *	GetValueBuf()
	{
		return	m_pVar;
	}
};
// ʹ��ʱ������ͨ�ִ��������ں���������ʱ����Ҫ���UTF8��
template<class _AllocationObj>
class	whdataobj_charptrtype_utf8based_prop_t	: public whdataobj_charptrtype_prop_t<_AllocationObj>
{
	typedef	whdataobj_charptrtype_prop_t<_AllocationObj>	FATHERCLASS;
protected:
	typedef	whallocmgr2<_AllocationObj>			AM_T;
	typename AM_T::template vector<char>					m_vectUTF8;
public:
	whdataobj_charptrtype_utf8based_prop_t()
		: FATHERCLASS()
	{
	}
	whdataobj_charptrtype_utf8based_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
		, m_vectUTF8(nc)
	{
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, char *pVar, size_t nMaxSize, WHDATAOBJ_NAMEID_T nNameID)
	{
		FATHERCLASS::SetParam(pHost, pVar, nMaxSize, nNameID);
		m_vectUTF8.SetMgr(pHost->m_vectNameID2PropPtr.GetMgr());
		m_vectUTF8.reserve(nMaxSize*2);	// ��Ϊ���ֽ���Զ������4�ֽڵģ�ֻ��˫�ֽڵ��ַ��ſ��ܱ��4�ֽڵ�
		Refresh2UTF8();
	}
	void	Refresh2UTF8()
	{
		wh_char2utf8(whdataobj_charptrtype_prop_t<_AllocationObj>::m_pVar, strlen(whdataobj_charptrtype_prop_t<_AllocationObj>::m_pVar), m_vectUTF8.getbuf(), m_vectUTF8.capacity()-1);
		m_vectUTF8.resize(strlen(m_vectUTF8.getbuf()));
	}
	void	Refresh2Char()
	{
		wh_utf82char(m_vectUTF8.getbuf(), m_vectUTF8.size(), whdataobj_charptrtype_prop_t<_AllocationObj>::m_pVar, whdataobj_charptrtype_prop_t<_AllocationObj>::m_nMaxSize-1);
	}
	inline const char *	getUTF8()
	{
		return	m_vectUTF8.getbuf();
	}
	inline void	set(const char *pStr)
	{
		FATHERCLASS::set(pStr);
		// ����UTF8���ִ���ʹ���ڲ���Ĭ���ַ���
		Refresh2UTF8();
	}
	inline void	setUTF8(const char *pStr)
	{
		size_t	nLen	= strlen(pStr);
		if( nLen>=m_vectUTF8.capacity() )
		{
			assert(0);
			return;
		}
		memcpy(m_vectUTF8.getbuf(), pStr, nLen+1);
		m_vectUTF8.resize(nLen);
		FATHERCLASS::set(pStr);
		whdataobj_charptrtype_prop_t<_AllocationObj>::m_nMDStat	= whdataobj_charptrtype_prop_t<_AllocationObj>::MDSTAT_MODIFIED;
		// ������ͨchar����
		Refresh2Char();
	}
	// ����ǰ���UTF8��д��
	int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		return	FATHERCLASS::_WriteValueToStream(m_vectUTF8.getbuf(), pBufStream, nSizeLeft, bWriteAll, m_vectUTF8.capacity()-1);
	}
	// Ȼ����UTF8����
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		int	rst	= FATHERCLASS::_ReadValueFromStream(m_vectUTF8.getbuf(), m_vectUTF8.capacity()-1, pBufStream, nSizeLeft, bSetModify);
		if( rst<0 )
		{
			return	rst;
		}
		m_vectUTF8.resize(strlen(m_vectUTF8.getbuf()));
		// ����һ����ͨChar�ĵĲ���
		Refresh2Char();
		return	0;
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// ��������ģ�����Ҳ�����m_pHostָ��ģ�
		FATHERCLASS::AdjustInnerPtr(pMgr, nOffset);
		// �����Լ���
		m_vectUTF8.AdjustInnerPtr(pMgr, nOffset);
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_charptrtype_utf8based_prop_t(NULLCONSTRUECTOR);
	}
};

// �б�����
// ע�⣺_Ty�б�����һ��_IDTy���͵�ID������Ҫ��GetID��SetID������
// ��������GetStatus��GetStatusLow��SetStatus���������õ�status�������涨���WHDATAOBJ_LISTTYPE_STATUS_XXX
// ���Գɲο�TTY_LISTUNIT_BASE_T�Ķ��壨tty_common_Base.h��
template<typename _Ty, typename _IDTy, class _AllocationObj>
class	whdataobj_listtype_prop_t	: public whdataobj_prop_t<_AllocationObj>
{
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
public:
	typedef	whallocmgr2<_AllocationObj>								AM_T;
	typedef	typename AM_T::template list<_Ty>						LIST_T;
	typedef	typename LIST_T::iterator								LISTITERATOR_T;
	typedef	typename LIST_T::const_iterator							LISTCONSTITERATOR_T;
	typedef	whhash<_IDTy, LISTITERATOR_T,_AllocationObj>			HASH_T;
	typedef	typename HASH_T::kv_iterator							HASHIT;
protected:
	bool		m_bModified;										// ��ʾ�Ƿ��޸�
	whdataobjbase_t<_AllocationObj>	*m_pHost;						// ����ĸ��
	LIST_T		*m_pList;
	LIST_T		**m_ppList;
	int			m_nLogicSize;										// �߼��ϵĳߴ�
private:
	// ��ƷID����ʵ�б�Ԫ��iterator�Ķ�Ӧ�б�
	HASH_T		*m_pmapReal;
private:
	inline void	EraseItem(HASHIT &it)
	{
		m_pList->erase(it.getvalue());
		m_pmapReal->erase(it);
	}
public:
	whdataobj_listtype_prop_t()
		: m_bModified(false)
		, m_pHost(NULL)
		, m_pList(NULL), m_ppList(NULL)
		, m_nLogicSize(0)
		, m_pmapReal(NULL)
	{
	}
	whdataobj_listtype_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	virtual ~whdataobj_listtype_prop_t()
	{
		//modified by zhanghaoyang 2011/11/29 �����ûset�����Ԫ�أ����׳��쳣�ˣ������ʧ�ܣ����Լ�������жϣ�Ӧ�ò������κ�Ӱ��
		if(m_pHost!=NULL)
			GetAM()->Delete(m_pmapReal);
	}
	inline AM_T *	GetAM()
	{
		return	m_pHost->m_vectNameID2PropPtr.GetMgr();
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, LIST_T *&pList, WHDATAOBJ_NAMEID_T nNameID)
	{
		m_pHost					= pHost;
		m_pList					= pList;
		m_ppList				= &pList;
		this->m_nNameID			= nNameID;
		// ����hash��
		GetAM()->AllocByType(m_pmapReal);
		new (m_pmapReal) HASH_T(0,GetAM()->GetAO());
		// ����ӳ���
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
	}
	bool	IsModified() const
	{
		return	m_bModified;
	}
	// ����б�����Խ����б����(���Ի��begin��end)
	inline const LIST_T &	GetList() const
	{
		return	*m_pList;
	}
	inline int	GetLogicSize() const
	{
		return	m_nLogicSize;
	}
	// ����(ֻ��)
	LISTITERATOR_T	Find(const _IDTy id)
	{
		// ��Real���в���
		HASHIT	kvit	= m_pmapReal->find(id);
		if( kvit == m_pmapReal->end() )
		{
			return	m_pList->end();
		}
		return	kvit.getvalue();
	}
	// �ṩend���ڱȽ�Find�Ľ�������ø�begin�ˡ�
	inline LISTITERATOR_T	end()
	{
		return	m_pList->end();
	}
	// ���(���ص���������ȡ��Ӧ�ṹ���ý����޸ģ�ע�������ID���ֱ���������ṩ��idһ�£���粻�����޸���)
	// ���Ҫ�޸�Ҳ��Ҫ���������
	LISTITERATOR_T	AddModify(const _IDTy id, bool bSetModify=true)
	{
		// ��������־Ϊ���Ĺ�����Ϊ�������return�������Ȱ����������ã�
		if( bSetModify )
		{
			whsetbool_true(m_bModified);
		}

		// ���ҿ���ID�Ƿ��Ѿ�����
		HASHIT	it	= m_pmapReal->find(id);;
		if( it != m_pmapReal->end() )
		{
			LISTITERATOR_T	&listit	= it.getvalue();
			_Ty	&unit	= (*listit);
			if( bSetModify )
			{
				// ����ԭ����״̬
				switch( unit.GetStatusLow() )
				{
					case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN:
						// ��������Ϊ���޸�
						unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED);
					break;
					case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED:
						// ���˵�����Ѿ��޸Ĺ��ģ���Ϊԭ�����ܱ�����ΪLOCK״̬�ˣ�������Ҫ�����ԭ����LOCK״̬��
						unit.ClearStatusBit(WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK);
					case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD:
						if( (unit.GetStatus()&WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK) != 0 )
						{
							// ���˵�����ϴθ��½��֮ǰ����ģ����������޸��ˣ�������Ҫ�����ԭ����LOCK״̬����ֱ�Ӹ�Ϊmodified��
							unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED);
						}
						else
						{
							// ���˵�����ӹ���û�й����̣��Ǿͼ�������ADD״̬
						}
					break;
					case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL:
						// �ոձ�ɾ������������ӣ�������Ҫ������Ϊ�޸�
						// ��������Ϊ���޸�
						// ע�⣺���һ����������ӣ����̷���ʧ�ܣ���ɾ�������̷���ʧ�ܣ������������֮����״̬����Modified������Add�������ڴ����ж�ȡModified��Addָ��ʱ����ʽӦ��һ�£��������û�о���ӣ�����о��޸ģ�
						unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED);
						// �߼��ߴ�����
						m_nLogicSize	++;
					break;
					default:
						// ������˵��Ӧ�ó�����������ֵ�ģ������˵��״̬λ�������޸��ˣ�
#ifdef	_DEBUG
						if( WHCMN_GET_DBGLVL()>=2 )
						{
							assert(0);
						}
						else
						{
							// �����Ϊ�˱�֤���Ե�ʱ�������Ҳ������������
							unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED);
						}
#endif
					break;
				}
			}
			else
			{
				// ����ԭ�����κ�״̬��������޸Ĺ���״̬
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN);
			}
			return	listit;
		}
		else
		{
			// ��Real���д���
			it	= m_pmapReal->put(id);
			LISTITERATOR_T	&listit	= it.getvalue();
			// Ҳ�������б��д���
			m_pList->push_back(&listit);
			_Ty	&unit	= (*listit);
			if( bSetModify )
			{
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD);
			}
			else
			{
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN);
			}
			// ����ID
			unit.SetID(id);
			// �߼��ߴ�����
			m_nLogicSize	++;
			return	listit;
		}
	}
	// �������list�п�������
	int		CopyFrom(whdataobj_listtype_prop_t *pOther)
	{
		for(LISTITERATOR_T it=pOther->m_pList->begin(); it!=pOther->m_pList->end(); ++it)
		{
			_Ty	&unit	= (*it);
			if( unit.GetStatusLow() != WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL )
			{
				LISTITERATOR_T	nit	= AddModify(unit.GetID(), true);
				(*nit).CopyFrom(unit);
			}
		}
		return	0;
	}
	// ɾ��(�����Ƿ����)
	bool	Del(const _IDTy id, bool bSetModify=true)
	{
		HASHIT	it	= m_pmapReal->find(id);
		if( m_pmapReal->end() == it )
		{
			// ������
			return	false;
		}
		return	Del(it, bSetModify);
	}
	// ���ݵ�����ɾ��
	bool	Del(HASHIT	it, bool bSetModify=true)
	{
		_Ty	&unit	= (*it.getvalue());
		switch( unit.GetStatusLow() )
		{
		case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN:
		case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED:
			{
				// ����Ϊɾ��״̬
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL);
			}
			break;
		case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD:
			{
				if( (unit.GetStatus()&WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK) != 0 )
				{
					// ˵���ϸ���ӵ�ָ������Ѿ��������ˣ�ֻ��û���յ����أ�����Ϊ�˱��ջ���Ӧ�÷���ɾ����ָ��
					unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL);
				}
				else
				{
					// ���ϴε���ӵ����ˣ�ֱ��ɾ��(�����ط���Ҫɾ)
					EraseItem(it);
				}
			}
			break;
		case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL:
			{
				// ���������Ӧ���ǲ������ģ��ϲ㲻��ɾ���Ѿ�ɾ����һ�εĶ��������Է��ش���
				return	false;
			}
			break;
		}
		// �߼��ߴ����
		m_nLogicSize	--;
		// ����Ϊ���Ĺ�
		if( bSetModify )
		{
			whsetbool_true(m_bModified);
		}
		else
		{
			// �����ø��ľ�ֱ��ɾ����
			EraseItem(it);
		}
		return	true;
	}
	void	DelAll()
	{
		HASHIT it	= m_pmapReal->begin(); 
		while( it != m_pmapReal->end() )
		{
			HASHIT tmpit	= it++;
			Del(tmpit, true);
		}
	}
	// ��������޸ı�־
	void	ClearModified()
	{
		// ������е�Ԫ�ĸ��ı�־����ɾ���Ѿ�ɾ����
		HASHIT	it	= m_pmapReal->begin();
		while( it!=m_pmapReal->end() )
		{
			HASHIT	itcur(it++);
			_Ty	&unit		= (*itcur.getvalue());
			if( unit.GetStatusLow() == WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL )
			{
				// ����ɾ��֮
				EraseItem(itcur);
			}
			else
			{
				// �޸�״̬Ϊ��ͨ
				unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN);
			}
		}
		// �������־
		whsetbool_false(m_bModified);
	}
	void	SetModifiedWithLock()
	{
		for(LISTITERATOR_T it=m_pList->begin(); it!=m_pList->end(); ++it)
		{
			_Ty	&unit	= (*it);
			if( unit.GetStatus() != 0 )
			{
				unit.SetStatusBit(WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK);
			}
		}
	}
	void	ClearModifiedWithLock()
	{
		whsetbool_false(m_bModified);
		HASHIT	it	= m_pmapReal->begin();
		while( it!=m_pmapReal->end() )
		{
			HASHIT	itcur(it++);
			_Ty	&unit		= (*itcur.getvalue());
			if( unit.GetStatus() != 0 )
			{
				if( (unit.GetStatus()&WHDATAOBJ_LISTTYPE_UNIT_STATUS_BIT_LOCK) != 0 )
				{
					if( unit.GetStatusLow() == WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL )
					{
						// �Ǳ�ɾ���ģ�����ɾ��֮
						EraseItem(itcur);
					}
					else
					{
						// ���޸Ļ���ӵģ�����ֱ�����״̬
						unit.SetStatus(WHDATAOBJ_LISTTYPE_UNIT_STATUS_CMN);
					}
				}
				else
				{
					// ˵����lock�������������ӵ��޸ģ�˵���������޸���
					whsetbool_true(m_bModified);
				}
			}
		}
	}
	void	ClearValue()
	{
		// ����б�
		m_pmapReal->clear();
		m_pList->clear();
		// �������־
		whsetbool_false(m_bModified);
	}
	int	WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		// һ����˵����ṩ��buffer���ǹ�����
		whfile_i_mem	f(pBufStream, nSizeLeft);
		// ��ʼ
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_LIST_MODIFY_BEGIN;
		f.Write(&cmd);
		f.Write(&this->m_nNameID);
		// ������иĶ��ġ���ӵġ�ɾ����
		for(LISTITERATOR_T it=m_pList->begin(); it!=m_pList->end(); ++it)
		{
			_Ty	&unit	= (*it);
			if( bWriteAll )
			{
				// ��Ϊ�Ǳ߽���WriteAll������ʱ���������������
				// ������ע�ⲻ�ܰ�ɾ��������Ҳ����ȥ��
				if( unit.GetStatusLow() == WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL )
				{
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_DEL;
					f.Write(&cmd);
					// д����ƷID
					_IDTy	nID	= unit.GetID();
					f.Write(&nID);
				}
				else
				{
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_ADD;
					f.Write(&cmd);
					// д������(��Ϊ�����ǹ̶���)
					f.Write(&unit);
				}
			}
			else
			{
				// ����֮����ҲҪȡ��λ����Ϊ������LOCK�е�Ԫ�ز�û���յ����̳ɹ��ķ��أ��ٴδ洢��ʱ����Щ������Ȼ��Ҫ�洢�Ρ�
				switch( unit.GetStatusLow() )
				{
				case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_MODIFIED:
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_MODIFY;
					f.Write(&cmd);
					// д������(��Ϊ�����ǹ̶���)
					f.Write(&unit);
					break;
				case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_ADD:
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_ADD;
					f.Write(&cmd);
					// д������(��Ϊ�����ǹ̶���)
					f.Write(&unit);
					break;
				case	WHDATAOBJ_LISTTYPE_UNIT_STATUS_DEL:
					cmd		= WHDATAOBJ_CMD_LIST_ITEM_DEL;
					f.Write(&cmd);
					// д����ƷID
					_IDTy	nID	= unit.GetID();
					f.Write(&nID);
					break;
				}
			}
		}
		// ����
		cmd	= WHDATAOBJ_CMD_LIST_MODIFY_END;
		f.Write(&cmd);
		// ���ĺ����д��ָ��pBufStream����󳤶�
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		// �ϲ�Ӧ���Ѿ�������ָ���NameID����
		whfile_i_mem	f(pBufStream, nSizeLeft);
		bool	bStop	= false;
		while( !bStop && f.GetSizeLeft()>0 )
		{
			WHDATAOBJ_CMD_T	cmd;
			f.Read(&cmd);
			switch(cmd)
			{
				case	WHDATAOBJ_CMD_LIST_MODIFY_END:
					bStop	= true;
				break;
				case	WHDATAOBJ_CMD_LIST_ITEM_MODIFY:
				case	WHDATAOBJ_CMD_LIST_ITEM_ADD:
				{
					// ������������
					_Ty	data;
					f.Read(&data);
					// �������
					LISTITERATOR_T	it	= AddModify(data.GetID(), bSetModify);
					if( it != m_pList->end() )
					{
						(*it).CopyFrom(data);	// �б�����ݵ�Ԫ�ṹ����ʵ��CopyFrom����
					}
				}
				break;
				case	WHDATAOBJ_CMD_LIST_ITEM_DEL:
				{
					// ����ID
					_IDTy	nID;
					f.Read(&nID);
					// ɾ��
					Del(nID, bSetModify);
				}
				break;
			}
		}
		pBufStream	= f.GetCurBufPtr();
		nSizeLeft	= f.GetSizeLeft();
		if( nSizeLeft<0 )
		{
			return	-1;
		}

		return	0;
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// ����������ָ��
		wh_adjustaddrbyoffset(m_pHost, nOffset);
		// ����list������
		pMgr->AdjustPtrAndInner(m_pList, nOffset);
		// Ȼ���ԭ������Ҳ�ı�ֵ
		wh_adjustaddrbyoffset(m_ppList, nOffset);
		*m_ppList	= m_pList;
		// hash��Ҳ��Ҫ����
		wh_adjustaddrbyoffset(m_pmapReal, nOffset);
		m_pmapReal->AdjustInnerPtr(nOffset);
		// ÿ��hashֵҲҪ��������Ϊ�������ڴ�ָ��
		for(HASHIT it = m_pmapReal->begin(); it != m_pmapReal->end(); ++it)
		{
			wh_adjustaddrbyoffset(it.getvalue().pNode, nOffset);
		}
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_listtype_prop_t(NULLCONSTRUECTOR);
	}
	void *	GetValueBuf()
	{
		// list�������������
		return	NULL;
	}
};

// ��������
template<typename _Ty, class _AllocationObj>
class	whdataobj_arraytype_prop_t	:	public whdataobj_prop_t<_AllocationObj>
{
protected:
	typedef	whdataobj_prop_t<_AllocationObj>	FATHERCLASS;
	bool	m_bModified;			// ��ʾ�Ƿ��޸�
	whdataobjbase_t<_AllocationObj>	*m_pHost;		// ����ĸ��
	_Ty		*m_pArray;				// ����ͷָ��
	int		m_nArraySize;			// ���鳤��
	enum
	{
		MDSTAT_LOCKED	= 0x80,		// ��ʾ������������
		MDSTAT_MODIFIED	= 0x01,		// ��ʾ���޸���
	};
	unsigned char	*m_pnStatus;	// ��ӦArrayÿ��Ԫ�ص�״̬��־
	// ����ĳ��λ�ñ��޸�
	inline void	SetArrayUnitModified(int nIdx)
	{
		m_pnStatus[nIdx]		= MDSTAT_MODIFIED;
	}
	// ��������λ�ñ��޸�
	inline void	SetArrayUnitModified(int nIdx, int nSpan)
	{
		for(int i=0;i<nSpan;i++)
		{
			m_pnStatus[nIdx+i]	= MDSTAT_MODIFIED;
		}
	}
	// ���ĳ��λ�õ��޸ı��
	inline void	ClrArrayUnitModified(int nIdx)
	{
		m_pnStatus[nIdx]		= 0;
	}
	// �������ĳ��λ�õ��޸ı��
	inline void	ClrArrayUnitModified(int nIdx, int nSpan)
	{
		for(int i=0;i<nSpan;i++)
		{
			m_pnStatus[nIdx+i]	= 0;
		}
	}
public:
	typedef	whallocmgr2<_AllocationObj>			AM_T;
	whdataobj_arraytype_prop_t()
		: m_bModified(false)
		, m_pHost(NULL)
		, m_pArray(NULL)
		, m_nArraySize(0)
		, m_pnStatus(NULL)
	{
	}
	whdataobj_arraytype_prop_t(const NULLCONSTRUCT_T &nc)
		: FATHERCLASS(nc)
	{
	}
	~whdataobj_arraytype_prop_t()
	{
		if( m_pnStatus )
		{
			m_pHost->m_vectNameID2PropPtr.GetMgr()->Free(m_pnStatus);
			m_pnStatus			= NULL;
		}
	}
	void	SetParam(whdataobjbase_t<_AllocationObj> *pHost, _Ty *pArray, int nArraySize, WHDATAOBJ_NAMEID_T nNameID)
	{
		assert(nArraySize<0x10000);
		m_pHost					= pHost;
		m_pArray				= pArray;
		m_nArraySize			= nArraySize;
		this->m_nNameID				= nNameID;
		// ����ӳ���
		m_pHost->m_vectNameID2PropPtr[nNameID]	= this;
		// �����Ӧ�ı��λ
		if( m_pnStatus )
		{
			// ������˵�ǲ�Ӧ���Ѿ��еģ����ǲ��ų�ĳЩʱ����һ�������ظ����ù����������ݵ��������
			m_pHost->m_vectNameID2PropPtr.GetMgr()->Free(m_pnStatus);
		}
		m_pnStatus				= (unsigned char *)m_pHost->m_vectNameID2PropPtr.GetMgr()->Alloc(sizeof(unsigned char)*nArraySize);
		assert(m_pnStatus);
		memset(m_pnStatus, 0, nArraySize);
	}
	inline const _Ty *	getarray() const		// �������ָ�뿪ͷ�����Ҫ������ȡ�������������
	{
		return	m_pArray;
	}
	inline const _Ty &	get(int nIdx) const		// ��þ��������Ա(��������ϲ㸺��)
	{
		return	m_pArray[nIdx];
	}
	inline int	size() const					// ������鳤��
	{
		return	m_nArraySize;
	}
	inline void	set(int nIdx, const _Ty &val)
	{
		m_pArray[nIdx]		= val;
		SetArrayUnitModified(nIdx);
		whsetbool_true(m_bModified);
	}
	// nIdx����ʼ�±�
	// nSpan������Ԫ�صĸ���
	_Ty *	setbyptr(int nIdx, int nSpan)
	{
#ifdef	_DEBUG
		assert(nIdx>=0 && nIdx+nSpan<=m_nArraySize);
#endif
		SetArrayUnitModified(nIdx, nSpan);
		whsetbool_true(m_bModified);
		return	m_pArray + nIdx;
	}
	// ����һ��Ԫ��
	_Ty *	setbyptr(int nIdx)
	{
		SetArrayUnitModified(nIdx);
		whsetbool_true(m_bModified);
		return	m_pArray + nIdx;
	}
	// �Զ���0�Ĳ�����Ϊ�Ǹ�д��״̬
	void	SetZeroNotModified()
	{
		bool	bModified	= false;
		for(int i=0;i<m_nArraySize;i++)
		{
			if( m_pnStatus[i] )
			{
				if( m_pArray[i]==0 )
				{
					m_pnStatus[i]	= 0;
				}
				else
				{
					bModified	= true;
				}
			}
		}
		if( bModified )
		{
			whsetbool_true(m_bModified);
		}
		else
		{
			whsetbool_false(m_bModified);
		}
	}
	// �������list�п�������
	int		CopyFrom(whdataobj_arraytype_prop_t *pOther)
	{
		for(int i=0;i<pOther->m_nArraySize;i++)
		{
			if( pOther->get(i) != 0 )
			{
				set(i, pOther->get(i));
			}
		}
		return	0;
	}
	bool	IsModified() const
	{
		return	m_bModified;
	}
	void	ClearModified()
	{
		whsetbool_false(m_bModified);
		memset(m_pnStatus, 0, WHSIZEOFARRAY(m_pnStatus,m_nArraySize));
	}
	void	SetModifiedWithLock()
	{
		// �������޸Ĺ�������LOCK��־
		for(int i=0;i<m_nArraySize;i++)
		{
			if( m_pnStatus[i] != 0 )
			{
				m_pnStatus[i]	|= MDSTAT_LOCKED;
			}
		}
	}
	void	ClearModifiedWithLock()
	{
		whsetbool_false(m_bModified);
		// ��������LOCK��־��Modified״̬���
		for(int i=0;i<m_nArraySize;i++)
		{
			if( m_pnStatus[i] != 0 )
			{
				if( (m_pnStatus[i]&MDSTAT_LOCKED) != 0 )
				{
					// �����״̬
					m_pnStatus[i]	= 0;
				}
				else
				{
					// ˵�������޸ĵ�
					whsetbool_true(m_bModified);
				}
			}
		}
	}
	void	ClearValue()
	{
		memset(m_pArray, 0, WHSIZEOFARRAY(m_pArray,m_nArraySize));
		ClearModified();
	}
	int		WriteValueToStream(char *&pBufStream, int &nSizeLeft, bool bWriteAll)
	{
		// һ����˵����ṩ��buffer���ǹ�����
		whfile_i_mem	f(pBufStream, nSizeLeft);
		WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_ARR_MODIFY_BEGIN;
		f.Write(&cmd);
		f.Write(&this->m_nNameID);
		if( bWriteAll )
		{
			// ��0��ʼ����m_nArraySize�������
			WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_ARR_ITEM_MODIFYN;
			f.Write(&cmd);
			// ��ʼƫ��(�����-1λ�õ�)
			f.WriteVSize(1);
			// ���鳤��
			f.WriteVSize(m_nArraySize);
			// ����
			f.Write(m_pArray, WHSIZEOFARRAY(m_pArray, m_nArraySize));
		}
		else
		{
			// ��ʼ
			// ������б����ĵ�
			int		nLastIdx	= -1;
			int		i			= 0;
			bool	bSearchModi	= false;
			while(i<(int)m_nArraySize)
			{
				int	nCount	= 0;
				if( bSearchModi )
				{
					// ���������ı��޸ĵĿ�
					{
						int	j	= i;
						while( j<m_nArraySize && m_pnStatus[j]!=0 )
						{
							++nCount;
							++j;
						}
					}
					if( nCount==0 )
					{
						// ����i<m_nArraySize�������²����ܳ��������������
						assert(0);
						break;
					}
					int	nDiff	= i - nLastIdx;
					if( nCount == 1 )
					{
						WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_ARR_ITEM_MODIFY1;
						f.Write(&cmd);
						f.WriteVSize(nDiff);
						f.Write(&m_pArray[i]);
					}
					else
					{
						WHDATAOBJ_CMD_T	cmd	= WHDATAOBJ_CMD_ARR_ITEM_MODIFYN;
						f.Write(&cmd);
						f.WriteVSize(nDiff);
						f.WriteVSize(nCount);
						f.Write(&m_pArray[i], WHSIZEOFARRAY(m_pArray, nCount));
					}
					i			+= nCount;
					nLastIdx	= i;
					bSearchModi	= false;
				}
				else
				{
					// ����������û���޸ĵĿ�
					{
						int	j	= i;
						while( j<m_nArraySize && m_pnStatus[j]==0 )
						{
							++nCount;
							++j;
						}
					}
					// ����
					i			+= nCount;
					bSearchModi	= true;
				}
			}
		}
		// ����
		cmd	= WHDATAOBJ_CMD_ARR_MODIFY_END;
		f.Write(&cmd);
		// ���ĺ����д��ָ��pBufStream����󳤶�
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		assert(nSizeLeft>0);
		return	0;
	}
	int		ReadValueFromStream(char *&pBufStream, int &nSizeLeft, bool bSetModify)
	{
		// �ϲ�Ӧ���Ѿ�������ָ���NameID����
		whfile_i_mem	f(pBufStream, nSizeLeft);
		bool	bStop	= false;
		int		nIdx	= -1;
		while( !bStop && f.GetSizeLeft()>0 )
		{
			WHDATAOBJ_CMD_T	cmd;
			f.Read(&cmd);
			switch(cmd)
			{
				case	WHDATAOBJ_CMD_ARR_MODIFY_END:
					bStop	= true;
				break;
				case	WHDATAOBJ_CMD_ARR_ITEM_MODIFY1:
				{
					// ����ƫ��
					int	nDiff	= f.ReadVSize();
					if( nDiff<=0 )
					{
						return	-1;
					}
					nIdx	+= nDiff;
					if( nIdx>=m_nArraySize )
					{
						return	-2;
					}
					// ��������
					f.Read(&m_pArray[nIdx]);
					// �����޸�
					if( bSetModify )
					{
						SetArrayUnitModified(nIdx);
					}
					else
					{
						ClrArrayUnitModified(nIdx);
					}
					// ��һ����ʼλ��
					++nIdx;
				}
				break;
				case	WHDATAOBJ_CMD_ARR_ITEM_MODIFYN:
				{
					// ����ƫ��
					int	nDiff	= f.ReadVSize();
					if( nDiff<=0 )
					{
						return	-11;
					}
					nIdx	+= nDiff;
					if( nIdx>=m_nArraySize )
					{
						return	-12;
					}
					// ��������
					int	nCount	= f.ReadVSize();
					if( nCount<=0 )
					{
						return	-13;
					}
					// ��������
					assert(nIdx+nCount<=m_nArraySize);
					f.Read(&m_pArray[nIdx], WHSIZEOFARRAY(m_pArray, nCount));
					// �����޸�
					if( bSetModify )
					{
						SetArrayUnitModified(nIdx, nCount);
					}
					else
					{
						ClrArrayUnitModified(nIdx, nCount);
					}
					// ��һ����ʼλ��
					nIdx	+= nCount;
				}
				break;
			}
		}
		// ���ĺ����д��ָ��pBufStream����󳤶�
		pBufStream		= f.GetCurBufPtr();
		nSizeLeft		= f.GetSizeLeft();
		if( nSizeLeft>=0 )
		{
			if( bSetModify )
			{
				whsetbool_true(m_bModified);
			}
			return	0;
		}
		return	-1;
	}
	void	AdjustInnerPtr(AM_T *pMgr, long nOffset)
	{
		// ����������ָ��
		wh_adjustaddrbyoffset(m_pHost, nOffset);
		// ����ָ��
		wh_adjustaddrbyoffset(m_pArray, nOffset);
		wh_adjustaddrbyoffset(m_pnStatus, nOffset);
	}
	void	NewInner(void *pMem)
	{
		new (pMem) whdataobj_arraytype_prop_t(NULLCONSTRUECTOR);
	}
	void *	GetValueBuf()
	{
		return	m_pArray;
	}
};

}		// EOF namespace n_whcmn

// �����ù�����صģ�ע�⣺����str����ֱ����array�ķ�ʽ��
#define	AUTO_DATA_SetParam( a,b )		a.SetParam(this, &pBase->a, b)
#define	AUTO_DATA_SetParam_smp(a)		AUTO_DATA_SetParam(a,enum_##a)
#define	AUTO_DATA_SetParamPtr( ptr,b )		ptr.SetParam(this, pBase->ptr, b)
#define	AUTO_DATA_SetParamPtr_smp( ptr )	AUTO_DATA_SetParamPtr(ptr, enum_##ptr)
#define	AUTO_DATA_SetParamArray( a,b )	a.SetParam(this, pBase->a,WHNUMOFARRAYUNIT(pBase->a), b)
#define	AUTO_DATA_SetParamArray_smp(a)	AUTO_DATA_SetParamArray(a,enum_##a)
#define	AUTO_DATA_SetMarker( a,b )		a.SetParam(this, b)
#define	AUTO_DATA_SetMarker_smp( a )	AUTO_DATA_SetMarker(a,enum_##a)

// �ʹ�ӡԪ����صģ�֮ǰʹ����Ӧ�ö���rst�������ó�ֵΪ0��
// �����Ҫ��������һ��������������ӡ�ִ��õ�
#define	AUTO_DATA_SMPPRINT	assert(rst>=0);pszBuf+=rst;nSizeLeft-=rst;assert(nSizeLeft>=0);rst=snprintf
#define	AUTO_DATA_DispCmn( t, a )		AUTO_DATA_SMPPRINT( pszBuf, nSizeLeft, "%s:"t"%s",#a,a.get(), WHLINEEND)
#define	AUTO_DATA_DispInt( a )			AUTO_DATA_DispCmn("%d", a)
#define	AUTO_DATA_DispInt64( a )		AUTO_DATA_DispCmn("0x%"WHINT64PRFX"X", a)
#define	AUTO_DATA_DispTime( a )			AUTO_DATA_SMPPRINT( pszBuf, nSizeLeft, "%s:%lu%s",#a,a.get(), WHLINEEND)
#define	AUTO_DATA_DispFloat( a )		AUTO_DATA_SMPPRINT( pszBuf, nSizeLeft, "%s:%.4f%s",#a,a.get(), WHLINEEND)
#define	AUTO_DATA_DispStr( a )			AUTO_DATA_SMPPRINT( pszBuf, nSizeLeft, "%s:%s%s",#a,a.get(), WHLINEEND)
#define	AUTO_DATA_DispArray0( t, a )	AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, "%s ",#a );{for(int i=0;i<a.size();++i) {if( a.get(i)!=0 ) {AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, "[%d]."t" ", i, a.get(i) );}}}AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, WHLINEEND )
#define	AUTO_DATA_DispArray( a )		AUTO_DATA_DispArray0("%d", a)
#define	AUTO_DATA_DispStrArray( a )		AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, "%s ",#a );{for(int i=0;i<a.size();++i) {if( a.get(i)[0]!=0 ) {AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, "[%d].%s%s", i, (const char *)a.get(i), WHLINEEND);}}}AUTO_DATA_SMPPRINT(pszBuf, nSizeLeft, WHLINEEND )

#endif
