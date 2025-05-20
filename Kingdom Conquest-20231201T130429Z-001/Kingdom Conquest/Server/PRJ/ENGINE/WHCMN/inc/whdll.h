// Copyright (C) PixelGame, http://www.pixelgame.com
// All rights reserved.
// namespace    : n_whcmn
// File         : whdll.h
// Creator      : Wei Hua (κ��)
// Comment      : ��̬���ӿ��ʹ��
//              : ��linux�����ɶ�̬���ӿ���Ҫ�����ӵ�ʱ��ָ��"-shared"����
//              : ��windows�����ɶ�̬���ӿ���Ҫ����DLL���̣����Ҵ���DllMain����
//              : WHDLL_Loaderֻ�ܶ�Ӧ����һ��DLL�ļ�������Release������ӦLoad��Դ���ͷ�
// CreationDate : 2004-11-25
// ChangeLog    :

#ifndef	__WHDLL_H__
#define	__WHDLL_H__

namespace n_whcmn
{

// ע��ṹ����й��캯���Ͳ���ʹ������ķ�ʽ���г�ʼ����
struct	WHDLL_LOAD_UNIT_T
{
	const char	*pcszFuncName;					// ������
	void		**ppFunc;						// ָ����ָ���ָ��
	void		*pDftFunc;						// Ĭ�Ϻ���
};

class	WHDLL_Loader
{
public:
	char	m_szLastError[256];					// �������������龿����ʲô����
	int		m_nErrUnit;							// ��������������������Ǹ�func��ʱ����Ĵ�
public:
	static WHDLL_Loader *	Create();
	// ����ÿ����Ԫָ��ĺ���ָ��ΪĬ�ϵĺ���
	static void				SetDftFunc(WHDLL_LOAD_UNIT_T *aUnit)
	{
		while( aUnit->pcszFuncName )
		{
			*aUnit->ppFunc	= aUnit->pDftFunc;
			aUnit	++;
		}
	}
	WHDLL_Loader()
	: m_nErrUnit(0)
	{
		m_szLastError[0]	= 0;
	}
	virtual ~WHDLL_Loader()	{}
public:
	// ���������˵�������溯�����ܷ�������ֵ
	enum
	{
		RST_OK							= 0,	// �ɹ�
		RST_ERR_ALREADYLOAD				= -1,	// dll�Ѿ�������
		RST_ERR_CANNOTOPEN				= -2,	// �޷���dll
		RST_ERR_CANNOTCLOSE				= -3,	// �޷��ر�dll
		RST_ERR_CANNOTLOADFUNC			= -4,	// �޷�����ĳ������()
	};
	// aUnit�����е����һ��һ��ȫ��NULL��ʾ����
	// ���������pnErrUnit�м�¼�����Ǹ��޷�����
	virtual int		Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit)	= 0;
	virtual void *	GetFunc(const char *cszFuncName)						= 0;
	virtual int		Release()			= 0;
};

// _dftfuncptr�����ǡ�&Func������ʽ��������g++�»ᱨwarning
#define	WHDLL_LOAD_UNIT_DECLARE0(_funcname, _funcvar, _dftfuncptr)	\
	{_funcname, (void **)&_funcvar, (void *)_dftfuncptr},
	// EOF WHDLL_LOAD_UNIT_DECLARE0
#define	WHDLL_LOAD_UNIT_DECLARE(_funcvar, _dftfuncptr)				\
	WHDLL_LOAD_UNIT_DECLARE0(#_funcvar, _funcvar, _dftfuncptr)
	// EOF WHDLL_LOAD_UNIT_DECLARE
#define	WHDLL_LOAD_UNIT_DECLARE_LAST()								\
	{NULL, NULL, NULL},
	// EOF WHDLL_LOAD_UNIT_DECLARE_LAST

// Ϊ�˱���ͬһ��DLL�������Σ�������������� 
class	WHDLL_Loader_Man
{
public:
	static WHDLL_Loader_Man *	Create();
	virtual ~WHDLL_Loader_Man()	{}
public:
	// Load���DLL�������Release�������Ϳ�����
	virtual WHDLL_Loader *		Load(const char *cszDLLName, WHDLL_LOAD_UNIT_T *aUnit)
																	= 0;
};

}		// EOF namespace n_whcmn


#ifdef	WIN32
#define WH_DLL_API	__declspec(dllexport)
#else
#define	WH_DLL_API
#endif

#endif	// EOF __WHDLL_H__
