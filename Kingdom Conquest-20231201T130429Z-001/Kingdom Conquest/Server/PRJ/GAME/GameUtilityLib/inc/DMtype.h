/////////////////////////////////////////////////
// DMType.h		:	V0010
// Writen by	:	Liu Gang
// V0010		:	Mar.16.2000
// ChangeLOG    :   2006-03-09 和刘岩商量把QWORD改为long long，这样就可以判断如果id<0为非法ID了。
/////////////////////////////////////////////////
// 通用数据类型的定义

#ifndef __DMTYPE_H__
#define __DMTYPE_H__
/////////////////////////////////////////////////

#include <string.h>
/////////////////////////////////////////////////
#ifndef	FALSE
#define	FALSE	0
#endif

#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef NULL
#define NULL    0
#endif
 
#define	OFF	0
#define	ON	1
#define	NO	0
#define	YES	1

typedef	int						BOOL;
typedef	unsigned char			BYTE;
typedef	unsigned short			WORD;
typedef	unsigned long			DWORD;
typedef	unsigned int			UINT;
typedef	char *					LPSTR;
///typedef	const char *			LPCTSTR;
typedef const char *			LPCSTR;
typedef	unsigned long long		QWORD;
typedef	signed long long		SQWORD;

typedef	long long				int64_t;
typedef	unsigned long long		uint64_t;

typedef unsigned int			uint_t;

/////////////////////////////////////////////////
const char g_szNULL[1] = "";

/////////////////////////////////////////////////
#endif	// __DMTYPE_H__
