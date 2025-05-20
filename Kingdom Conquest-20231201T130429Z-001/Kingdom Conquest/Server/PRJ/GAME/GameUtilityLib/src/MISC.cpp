//#include	"stdafx.h"
#include	<stdio.h>
#include	<assert.h>
#include	<string.h>
#include	<stdarg.h>
#include	"../inc/misc.h"
#include	"../inc/Mat.h"
#include	"../inc/LogMngInf.h"


int		GetRank( int i,int j )//求阶程
{
	int iData = 0;
	int iCount = 0;
	for( int x = i;x> 0;x-- )
	{
		iData += x;
		iCount++;
		if( iCount >= j )
			break;
	}

	return iData;
}


char g_szByteStr[2048];

LPCSTR	DataToByteStr( BYTE * pData,int iSize )
{
	if( iSize > 512 )
	{
		char	szStr[4096];
		sprintf( szStr,"DataToByteStr %d\n",iSize);
		OutputErrorLog( szStr );
		iSize = 512;
	}

	char	sz[4096];
	for( int i=0;i<iSize;i++ )
	{
		int iData = pData[i];
		sprintf( sz,"%2X",iData );

		if( i == 0 )
		{
			strcpy( g_szByteStr,sz );
		}
		else
		{
			strcat( g_szByteStr," " );
			strcat( g_szByteStr,sz );
		}
	}
	strcat( g_szByteStr,"\0" );

	return g_szByteStr;
}


int	RoundFunc( float f )
{
	if( f - floor( f ) >= 0.5f )
		return	(int)ceil(f);
	return (int)floor( f );
}



int	GetCurveUnit( int & iCurHigh,const int iCurStep,const BOOL booIs )
{
	float fStep;
	if( iCurStep > 0 )
	{
		if( booIs )
			fStep = (float)iCurHigh*2/(float)iCurStep;
		else fStep = (float)iCurHigh/(float)iCurStep/2;
	}
	else
	{
		if( booIs )
			fStep = (float)iCurHigh;
		else fStep = (float)iCurHigh;
	}

	int iStep;
	iStep = RoundFunc( fStep );
	iCurHigh -= iStep;

	return  iStep;
}

//#define DEBUG_RAND_OLD	// 测试82版以前分布不均的随机算法

int	 GetMaxRand( int iMax )//可以取出超过30000小于10000000的随机数
{
	if( iMax <= 100 )
	{
		if( iMax <= 1 )
		{
			return 0;
		}
		return rand()%iMax;
	}
	assert( iMax <= 10000000 ); // 只能随机一千万以内
#ifdef DEBUG_RAND_OLD
 	int iRand = ( rand()%100 ) * 1000000 + 
 		( rand()%100 ) * 10000 + 
 		( rand()%100 ) * 100 + 
 		rand()%100;
	return iRand%iMax;
#else
	return RandF()%iMax;
#endif
}

int RandIntCC( int iMin, int iMax )
{
	if ( iMax > iMin )
	{
		return ( iMin + GetMaxRand( iMax - iMin + 1 ) );
	}
	else
	{
		return iMin;
	}
}

float RandFloatCC( float fMin,float fMax )
{
	int iStep = (int)( ( fMax - fMin ) * 1000 );
	if ( iStep > 0 )
	{
		int iRand = rand() % ( iStep + 1 );
		return  ( fMin + (float)iRand / 1000.0f );
	}
	else if ( iStep < 0 )
	{
		char szStr[4096];
		sprintf( szStr,"GetFloatRand Min %f	Max%f,in reverse\n",fMin,fMax );
		OutputErrorLog( szStr );
		return fMax;
	}
	return fMin;
}


float	GetSphereHigh( float fDist,float fRadius )
{
	if( fDist >= fRadius )
	{
		return 0.0f;
	}

	return (float)sqrt( (double)(fRadius*fRadius - fDist*fDist) );
}


BOOL	Angle1NearToAngle1(  float * A1,const float * A2,float fSpeed )
{
	float fStep = (float)fabs(*A1 - *A2);
	if( fStep < fSpeed )
	{
		*A1 = *A2;
		return true;
	}
	else if( fStep > PI )
	{
		if( *A1 < *A2 )
		{
			*A1 = *A1 + PI2;
			//			*A1 -= fSpeed;
		}
		else
		{
			*A1 = *A1 - PI2;//((int)fStep/PI2)*
			//			*A1 += fSpeed;
		}
		/*		if( *A1 < -PI )
		*A1 += PI2;
		else if( *A1 >= PI )
		*A1 -= PI2;*/
	}
	//	else
	{
		F1NearToF2(  A1,A2,fSpeed );
	}

	return *A1 == *A2;
}

float GetCurveHigh( float & fCur,float fStep,float fMaxHigh )
{
	float fBak;
	fBak = fCur;
	fCur =(float)( sqrt( fMaxHigh*fMaxHigh*( 1 - fabs(fStep - 0.5f)*2 ) ));
	return fCur - fBak;
}



float	GetRandAngle()
{
	float fAngle;
	int iRand = rand()%360;
	fAngle = 2*PI*iRand/360;
	return  fAngle;
}


void	F1NearToF2Ex1(  float * f1,const float * f2,float fSpeed )//速度逐渐减慢
{
	float fStep = (float)fabs(*f1-*f2) / fSpeed / 100;
	if( fStep > 1.0f )
	{
		fSpeed *= fStep;
	}
	F1NearToF2(  f1,f2,fSpeed );
}


void OutputDebugStringF( const char* format, ... )
{
	va_list vl;
	va_start(vl,format);

	char buf[2048];
	p_vsnprintf(buf,sizeof(buf) - 1,format,vl);
	va_end(vl);

#ifdef WIN32
	OutputDebugString(buf);
#else
	printf(buf);
#endif
}

// 2009-8-18 liuyi StackTrace 独立出来以便其他地方使用

bool g_excel_sym_enabled = false;
int g_excel_sym_depth = 16;

#ifdef WIN32

#include <windows.h>
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

static void AppendTextFormatV(std::string& buf, const char* fmt, va_list args)
{
    int n = p_vsnprintf(NULL,0,fmt, args);
    if(n > 0)
    {
        size_t s = buf.size();
        buf.resize(s+n);
        p_vsnprintf(&buf[0]+s, n, fmt, args);
    }
}

static void AppendTextFormat(std::string& buf, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    AppendTextFormatV(buf, fmt, args);
    va_end(args);
}

struct LocalBuffer
{
    char*   m_pos;
    char*   m_end;
    char*   m_fmt;
    char*   m_mod;
    IMAGEHLP_SYMBOL*    m_sym;

    LocalBuffer(char* p, char* e) : m_pos(p), m_end(e)
    {
        m_mod = (char*)::HeapAlloc(::GetProcessHeap(), 0, MAX_PATH);
        m_fmt = (char*)::HeapAlloc(::GetProcessHeap(), 0, 1000);
        m_sym = (IMAGEHLP_SYMBOL*)::HeapAlloc(::GetProcessHeap(), 0, 1000);
        m_sym->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
        m_sym->Address = 0;
        m_sym->Size = 0;
        m_sym->MaxNameLength = 1000-sizeof(IMAGEHLP_SYMBOL);
    }
    ~LocalBuffer()
    {
        if(m_mod)
        {
            ::HeapFree(::GetProcessHeap(), 0, m_mod);
        }
        if(m_fmt)
        {
            ::HeapFree(::GetProcessHeap(), 0, m_fmt);
        }
        if(m_sym)
        {
            ::HeapFree(::GetProcessHeap(), 0, m_sym);
        }
    }

    void Append(char* p)
    {
        size_t c = strlen(p);
        if(c > size_t(m_end-m_pos))
        {
            c = m_end-m_pos;
        }
        memcpy(m_pos, p, c);
        m_pos += c;
    }
    
    char* GetModule(HANDLE hProcess, size_t addr, size_t& rel_addr)
    {
        BYTE* base = (BYTE*)(size_t)SymGetModuleBase(hProcess, (DWORD)addr);
        if(base == 0)
        {
            return 0;
        }
        IMAGE_DOS_HEADER* dosHeader    = (IMAGE_DOS_HEADER*)base;
        if(dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
        {
            return 0;
        }
        if(dosHeader->e_lfanew <= 0)
        {
            return 0;
        }
        IMAGE_NT_HEADERS* headers   = (IMAGE_NT_HEADERS*)((BYTE*)base+dosHeader->e_lfanew);
        rel_addr = addr - (size_t(base)-headers->OptionalHeader.ImageBase);
        GetModuleFileName((HMODULE)base, m_mod, MAX_PATH);

        return m_mod;
    }
};

__declspec(naked) void __stdcall GetContext(DWORD& nEip, DWORD& nEsp, DWORD& nEbp)
{
    nEip, nEsp, nEbp;
    __asm
    {
        mov ecx, [esp]
        mov eax, [esp+4]
        mov [eax], ecx
        mov eax, [esp+8]
        mov [eax], esp
        mov eax, [esp+12]
        mov [eax], ebp
        ret 12
    }
}

struct
{
    IMAGEHLP_SYMBOL sym;
    CHAR buf[1000];
}
s_symBuf = 
{
    {
        sizeof(IMAGEHLP_SYMBOL),
        0, 0, 0, 1000,
    },
};

bool s_sym_init = false;
bool s_sym_succ = false;

__declspec(noinline) void G_MiscGetStackTrace(std::string& buf)
{
    if(!g_excel_sym_enabled) return;

    HANDLE hProcess = GetCurrentProcess();
    if(!s_sym_init)
    {
        s_sym_init = true;
        s_sym_succ = SymInitialize(hProcess, NULL, TRUE) ? true : false;
    }
    if(!s_sym_succ)
    {
        return;
    }

    HANDLE hThread = GetCurrentThread();
    BOOL bResult;
    DWORD_PTR Disp;
    IMAGEHLP_LINE Line;

    IMAGEHLP_SYMBOL* pSym = &s_symBuf.sym;

    CONTEXT ctx;
    memset(&ctx, 0, sizeof(ctx));
    GetContext(ctx.Eip, ctx.Esp, ctx.Ebp);

    STACKFRAME sfm;
    memset(&sfm, 0, sizeof(sfm));
	sfm.AddrPC.Offset = ctx.Eip;
	sfm.AddrStack.Offset = ctx.Esp;
	sfm.AddrFrame.Offset = ctx.Ebp;
	sfm.AddrPC.Mode = AddrModeFlat;
	sfm.AddrStack.Mode = AddrModeFlat;
	sfm.AddrFrame.Mode = AddrModeFlat;

    int idx = 0;
    int idx_s = 1;

    buf.reserve(256);

    for( ; ; )
    {
        bResult = StackWalk(
            IMAGE_FILE_MACHINE_I386,
            hProcess,
            hThread,
            &sfm,
            &ctx,
			NULL,
			SymFunctionTableAccess,
			SymGetModuleBase,
			NULL);
        if(!bResult || sfm.AddrFrame.Offset == 0)
        {
            break;
        }
        ++ idx;
        if(idx <= idx_s) continue;
        if(idx > g_excel_sym_depth) break;

        bResult = SymGetLineFromAddr(hProcess, sfm.AddrPC.Offset, &Disp, &Line);
        if(bResult)
        {
            AppendTextFormat(buf, "%s(%d):", Line.FileName, Line.LineNumber);
        }
        bResult = SymGetSymFromAddr(hProcess, sfm.AddrPC.Offset, &Disp, pSym);
        if(bResult)
        {
            buf.append(pSym->Name);
        }
        buf.push_back('$');
    }
}

#else

#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>

void G_MiscGetStackTrace(std::string& buf)
{
	if(!g_excel_sym_enabled) return;

	void *array[g_excel_sym_depth];
	size_t size;
	char **strings;
	size_t i;
	
	size = backtrace (array, g_excel_sym_depth);
	strings = backtrace_symbols (array, size);
	
	for (i = 0; i < size; i++)
	{
		buf.append(strings[i]);
		buf.push_back('$');
	}
	
	free(strings);
}

#endif // WIN32
