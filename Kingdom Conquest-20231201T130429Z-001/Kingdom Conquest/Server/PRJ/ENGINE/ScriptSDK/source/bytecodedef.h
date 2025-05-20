#ifndef __PS_BYTECODEDEF_H__
#define __PS_BYTECODEDEF_H__

#include "string.h"
#include "operand.h"

//---------------------------------------------
// Byte code instructions
enum PS_BCI
{
	//	instruction				|	operand1		|	operand2		|	operand3	|							
	//---------------- memory access ----------------------------------------------------
	BC_NOP = 0, // ��ָ��		|					|					|				|
	BC_SUSPEND, // ����ָ��		|					|					|				|
	BC_POP,		// ��ջָ��-arg	|	arg				|					|				|
	BC_PUSH,	// ��ջָ��+arg	|	arg				|					|				|
	BC_PUSH4,	// 4�ֽ�������ջ|   Դ				|
	BC_PUSH8,	// 8�ֽ�������ջ|	Դ
	BC_PUSHREF,  // ������ջ	|   Դ

	BC_MOV1,	// �ڴ��ƶ�		|	Ŀ��			|	Դ				|				|
	BC_MOV2,	// �ڴ��ƶ�		|	Ŀ��			|	Դ				|				|	
	BC_MOV4,	// �ڴ��ƶ�		|	Ŀ��			|	Դ				|				|
	BC_MOV8,	// �ڴ��ƶ�		|	Ŀ��			|	Դ				|				|

	BC_DEFREF,	 // ������		|   Ŀ��			|   Դ				|				|
	BC_RDREF1,   // ��ָ��		|	Ŀ��			|	Դ				|				| 
	BC_WTREF1,   // д����	    | 	Ŀ��			|	Դ				|				|
	BC_RDREF2,   // ��ָ��		|	Ŀ��			|	Դ				|				| 
	BC_WTREF2,   // д����	    | 	Ŀ��			|	Դ				|				|
	BC_RDREF4,   // ��ָ��		|	Ŀ��			|	Դ				|				| 
	BC_WTREF4,   // д����	    | 	Ŀ��			|	Դ				|				|
	BC_RDREF8,   // ��ָ��		|	Ŀ��			|	Դ				|				| 
	BC_WTREF8,   // д����	    |	Ŀ��			|	Դ				|				| 

	BC_RRET1,	 // ������ֵ	|	Դ				|					|				|
	BC_SRET1,	 // д����ֵ	|	Դ				|					|				|
	BC_RRET2,	 // ������ֵ	|	Դ				|					|				|
	BC_SRET2,	 // д����ֵ	|	Դ				|					|				|
	BC_RRET4,	 // ������ֵ	|	Դ				|					|				|
	BC_SRET4,	 // д����ֵ	|	Դ				|					|				|
	BC_RRET8,	 // ������ֵ	|	Դ				|					|				|
	BC_SRET8,	 // д����ֵ	|	Դ				|					|				|

	BC_PGA,		 //				|	Ŀ				|	Դ				|				|
	BC_PSP,		 //	������ջָ��|   Ŀ��			|
	BC_MEMCPY,	 //				|   Ŀ��			|	Դ				|	size		|
	//---------------- path control ----------------------------------------------------
	BC_CALL,	// ���ص���		|	�������		|					|				|
	BC_RET,		// ���غ�������	|   ��ջָ��+arg	|					|				|
	BC_CALLSYS, // ϵͳ����		|   �������		|	������С		|				|
	BC_JMP,		// ��������ת	|	ָ��ƫ��		|					|				|
	BC_JMPP,	// ���ָ����ת	|	ָ������		|	���ָ������	|				|
	BC_JNZ,		// ��ֵ��ת		|   �������		|	ָ��ƫ��		|				|
	BC_JZ,		// ��ֵ��ת		|   �������		|	ָ��ƫ��		|				|
	// -------------- Integer operations -----------------------------------------------
	BC_ADDi,	// +			|	Ŀ��			|	Դ				|	Դ			|
	BC_SUBi,	// -			|	Ŀ��			|	Դ				|	Դ			|
	BC_MULi,	// *			|	Ŀ��			|	Դ				|	Դ			|
	BC_DIVi,	// /			|	Ŀ��			|	Դ				|	Դ			|
	BC_MODi,	// %			|	Ŀ��			|	Դ				|	Դ			|
	BC_INCi,	// ++			|	Ŀ��			|	Դ				|	Դ			|
	BC_DECi,	// --			|	Ŀ��, Դ		|					|				|
	BC_NEGi,	// -			|   Ŀ��			|	Դ				|				|

	// -------------- floating point operations ----------------------------------------	
	BC_ADDf,	// +			|	Ŀ��			|	Դ				|	Դ			|
	BC_SUBf,	// -			|	Ŀ��			|	Դ				|	Դ			|
	BC_MULf,	// *			|	Ŀ��			|	Դ				|	Դ			|
	BC_DIVf,	// /			|	Ŀ��			|	Դ				|	Դ			|
	BC_MODf,	// %			|	Ŀ��			|	Դ				|	Դ			|
	BC_INCf,	// ++			|	Ŀ��			|	Դ				|	Դ			|
	BC_DECf,	// --			|	Ŀ��, Դ		|					|				|
	BC_NEGf,	// -(int)		|   Ŀ��			|	Դ				|				|

	// -------------- double operations ----------------------------------------------	
	BC_ADDd,	// +			|	Ŀ��			|	Դ				|	Դ			|
	BC_SUBd,	// -			|	Ŀ��			|	Դ				|	Դ			|
	BC_MULd,	// *			|	Ŀ��			|	Դ				|	Դ			|
	BC_DIVd,	// /			|	Ŀ��			|	Դ				|	Դ			|
	BC_MODd,	// %			|	Ŀ��			|	Դ				|	Դ			|
	BC_INCd,	// ++			|	Ŀ��			|	Դ				|	Դ			|
	BC_DECd,	// --			|	Ŀ��, Դ		|					|	 			|
	BC_NEGd,	// -(int)		|   Ŀ��			|	Դ				|				|

	// -------------- Bitwise operations ---------------------------------------------
	BC_NOT,	// ~(not)		|	Ŀ��			|	Դ				|				| 
	BC_AND,	// &(and)		|	Ŀ��			|	Դ				|	Դ			| 
	BC_OR,		// |(or)	|	Ŀ��			|	Դ				|	Դ			| 
	BC_XOR,	// ^(xor)		|	Ŀ��			|	Դ				|	Դ			| 
	BC_SLL,	// <<(logical left shift )			|	Ŀ��			|	Դ			|	Դ			| 	 
	BC_SRL,	// >>(logical right shift )			|	Ŀ��			|	Դ			|	Դ			| 
	BC_SLA,	// <<<(arithmetic left shift)		|	Ŀ��			|	Դ			|	Դ			| 
	BC_SRA,	// >>>(arithmetic right shift)		|	Ŀ��			|	Դ			|	Դ			| 

	// --------------- Comparison operations ----------------------------------------------
	BC_TEi, // ==		|	Ŀ��			|	Դ				|	Դ			|  
	BC_TNEi,// !=		|	Ŀ��			|	Դ				|	Դ			|
	BC_TLi,	// <		|	Ŀ��			|	Դ				|	Դ			|
	BC_TGi, // >		|	Ŀ��			|	Դ				|	Դ			|
	BC_TLEi,// <=		|	Ŀ��			|	Դ				|	Դ			|
	BC_TGEi,// >=		|	Ŀ��			|	Դ				|	Դ			|

	BC_TEf, // ==		|	Ŀ��			|	Դ				|	Դ			|  
	BC_TNEf,// !=		|	Ŀ��			|	Դ				|	Դ			|
	BC_TLf,	// <		|	Ŀ��			|	Դ				|	Դ			|
	BC_TGf, // >		|	Ŀ��			|	Դ				|	Դ			|
	BC_TLEf,// <=		|	Ŀ��			|	Դ				|	Դ			|
	BC_TGEf,// >=		|	Ŀ��			|	Դ				|	Դ			|

	BC_TEd, // ==		|	Ŀ��			|	Դ				|	Դ			|  
	BC_TNEd,// !=		|	Ŀ��			|	Դ				|	Դ			|
	BC_TLd,	// <		|	Ŀ��			|	Դ				|	Դ			|
	BC_TGd, // >		|	Ŀ��			|	Դ				|	Դ			|
	BC_TLEd,// <=		|	Ŀ��			|	Դ				|	Դ			|
	BC_TGEd,// >=		|	Ŀ��			|	Դ				|	Դ			|

	// -------------- type conversion ----------------------------------------------
	BC_I2F,	    // int to float					    |	Ŀ��			|	Դ			|	
	BC_F2I,	    // float to int					    |	Ŀ��			|	Դ			|	
	BC_D2I,	    // double to int					|	Ŀ��			|	Դ			|
	BC_I2D,	    // int to double					|	Ŀ��			|	Դ			|
	BC_D2F,	    // double to float					|	Ŀ��			|	Դ			|
	BC_F2D,  	// float to double					|	Ŀ��			|	Դ			|

	BC_I2B,     // int  to uint8					|	Ŀ��			|   Դ			|
	BC_B2I,		// uint8 to int					    |	Ŀ��			|	Դ			|
	BC_I2W,     // int to uint16				    |	Ŀ��			|	Դ			|
	BC_W2I,		// uint16 to int					|	Ŀ��			|	Դ			|		
	BC_I2C,		// int to int8
	BC_C2I,		// int8 to int
	BC_I2S,		// int to int16
	BC_S2I,		// int16 to int
	//--------------- Temporary tokens, can't be output to the final program ---------------
	BC_LINE, //				|  LineNum				|					
	BC_LABEL,//				|  Label				|
};

#define SIZEOFBC(n) (sizeof(int) + n*sizeof(int))

const int BCS_NOP  = SIZEOFBC(0);
const int BCS_SUSPEND = SIZEOFBC(0);
const int BCS_POP  = SIZEOFBC(1);
const int BCS_PUSH = SIZEOFBC(1);
const int BCS_PUSH4 = SIZEOFBC(1);
const int BCS_PUSH8 = SIZEOFBC(1);
const int BCS_PUSHREF = SIZEOFBC(1);

const int BCS_MOV1  = SIZEOFBC(2);
const int BCS_MOV2  = SIZEOFBC(2);
const int BCS_MOV4  = SIZEOFBC(2);
const int BCS_MOV8  = SIZEOFBC(2);

const int BCS_DEFREF = SIZEOFBC(2);
const int BCS_RDREF1 = SIZEOFBC(2);
const int BCS_WTREF1 = SIZEOFBC(2);
const int BCS_RDREF2 = SIZEOFBC(2);
const int BCS_WTREF2 = SIZEOFBC(2);
const int BCS_RDREF4 = SIZEOFBC(2);
const int BCS_WTREF4 = SIZEOFBC(2);
const int BCS_RDREF8 = SIZEOFBC(2);
const int BCS_WTREF8 = SIZEOFBC(2);

const int BCS_RRET1 = SIZEOFBC(1);
const int BCS_SRET1 = SIZEOFBC(1);
const int BCS_RRET2 = SIZEOFBC(1);
const int BCS_SRET2 = SIZEOFBC(1);
const int BCS_RRET4 = SIZEOFBC(1);
const int BCS_SRET4 = SIZEOFBC(1);
const int BCS_RRET8 = SIZEOFBC(1);
const int BCS_SRET8 = SIZEOFBC(1);

const int BCS_PGA = SIZEOFBC(2);
const int BCS_PSP = SIZEOFBC(1);
const int BCS_MEMCPY = SIZEOFBC(3);

const int BCS_CALL = SIZEOFBC(1);
const int BCS_RET  = SIZEOFBC(1);
const int BCS_CALLSYS = SIZEOFBC(2);
const int BCS_JMP  = SIZEOFBC(1);
const int BCS_JMPP = SIZEOFBC(2);
const int BCS_JZ   = SIZEOFBC(2);
const int BCS_JNZ  = SIZEOFBC(2);

const int BCS_ADDi = SIZEOFBC(3);
const int BCS_SUBi = SIZEOFBC(3);
const int BCS_MULi = SIZEOFBC(3);
const int BCS_DIVi = SIZEOFBC(3);
const int BCS_MODi = SIZEOFBC(3);
const int BCS_INCi = SIZEOFBC(1);
const int BCS_DECi = SIZEOFBC(1);
const int BCS_NEGi = SIZEOFBC(2);

const int BCS_ADDf = SIZEOFBC(3);
const int BCS_SUBf = SIZEOFBC(3);
const int BCS_MULf = SIZEOFBC(3);
const int BCS_DIVf = SIZEOFBC(3);
const int BCS_MODf = SIZEOFBC(3);
const int BCS_INCf = SIZEOFBC(1);
const int BCS_DECf = SIZEOFBC(1);
const int BCS_NEGf = SIZEOFBC(2);

const int BCS_ADDd = SIZEOFBC(3);
const int BCS_SUBd = SIZEOFBC(3);
const int BCS_MULd = SIZEOFBC(3);
const int BCS_DIVd = SIZEOFBC(3);
const int BCS_MODd = SIZEOFBC(3);
const int BCS_INCd = SIZEOFBC(1);
const int BCS_DECd = SIZEOFBC(1);
const int BCS_NEGd = SIZEOFBC(2);

const int BCS_NOT = SIZEOFBC(2);
const int BCS_AND = SIZEOFBC(3);
const int BCS_OR  = SIZEOFBC(3);
const int BCS_XOR = SIZEOFBC(3);
const int BCS_SLL = SIZEOFBC(3);
const int BCS_SRL = SIZEOFBC(3);
const int BCS_SLA = SIZEOFBC(3);
const int BCS_SRA = SIZEOFBC(3);

const int BCS_TEi   = SIZEOFBC(3);
const int BCS_TNEi  = SIZEOFBC(3);
const int BCS_TLi   = SIZEOFBC(3);
const int BCS_TGi   = SIZEOFBC(3);
const int BCS_TLEi  = SIZEOFBC(3);
const int BCS_TGEi  = SIZEOFBC(3);

const int BCS_TEf   = SIZEOFBC(3);
const int BCS_TNEf  = SIZEOFBC(3);
const int BCS_TLf   = SIZEOFBC(3);
const int BCS_TGf   = SIZEOFBC(3);
const int BCS_TLEf  = SIZEOFBC(3);
const int BCS_TGEf  = SIZEOFBC(3);

const int BCS_TEd   = SIZEOFBC(3);
const int BCS_TNEd  = SIZEOFBC(3);
const int BCS_TLd   = SIZEOFBC(3);
const int BCS_TGd   = SIZEOFBC(3);
const int BCS_TLEd  = SIZEOFBC(3);
const int BCS_TGEd  = SIZEOFBC(3);

const int BCS_I2F  = SIZEOFBC(2);
const int BCS_F2I  = SIZEOFBC(2);
const int BCS_D2I = SIZEOFBC(2);
const int BCS_I2D  = SIZEOFBC(2);
const int BCS_D2F = SIZEOFBC(2);
const int BCS_F2D = SIZEOFBC(2);

const int BCS_I2B = SIZEOFBC(2);
const int BCS_B2I = SIZEOFBC(2);
const int BCS_I2W = SIZEOFBC(2);
const int BCS_W2I = SIZEOFBC(2);
const int BCS_I2C = SIZEOFBC(2);
const int BCS_C2I = SIZEOFBC(2);
const int BCS_I2S = SIZEOFBC(2);
const int BCS_S2I = SIZEOFBC(2);

const int BCS_LINE  = 0;
const int BCS_LABEL = 0;

static const int g_psBCSize[] = 
{
	BCS_NOP,
	BCS_SUSPEND,

	BCS_POP,
	BCS_PUSH,
	BCS_PUSH4,
	BCS_PUSH8,
	BCS_PUSHREF,

	BCS_MOV1,
	BCS_MOV2,
	BCS_MOV4,
	BCS_MOV8,

	BCS_DEFREF,
	BCS_RDREF1,
	BCS_WTREF1,
	BCS_RDREF2,
	BCS_WTREF2,
	BCS_RDREF4,
	BCS_WTREF4,
	BCS_RDREF8,
	BCS_WTREF8,

	BCS_RRET1,
	BCS_SRET1,
	BCS_RRET2,
	BCS_SRET2,
	BCS_RRET4,
	BCS_SRET4,
	BCS_RRET8,
	BCS_SRET8,

	BCS_PGA,
	BCS_PSP,
	BCS_MEMCPY,

	BCS_CALL,
	BCS_RET,
	BCS_CALLSYS,
	BCS_JMP,
	BCS_JMPP,
	BCS_JNZ,
	BCS_JZ,

	BCS_ADDi,
	BCS_SUBi,
	BCS_MULi,
	BCS_DIVi,
	BCS_MODi,
	BCS_INCi,
	BCS_DECi,
	BCS_NEGi,

	BCS_ADDf,
	BCS_SUBf,
	BCS_MULf,
	BCS_DIVf,
	BCS_MODf,
	BCS_INCf,
	BCS_DECf,
	BCS_NEGf,

	BCS_ADDd,
	BCS_SUBd,
	BCS_MULd,
	BCS_DIVd,
	BCS_MODd,
	BCS_INCd,
	BCS_DECd,
	BCS_NEGd,

	BCS_NOT,
	BCS_AND,
	BCS_OR,
	BCS_XOR,
	BCS_SLL,
	BCS_SRL,
	BCS_SLA,
	BCS_SRA,

	BCS_TEi,
	BCS_TNEi,
	BCS_TLi,
	BCS_TGi,
	BCS_TLEi,
	BCS_TGEi,

	BCS_TEf,
	BCS_TNEf,
	BCS_TLf,
	BCS_TGf,
	BCS_TLEf,
	BCS_TGEf,

	BCS_TEd,
	BCS_TNEd,
	BCS_TLd,
	BCS_TGd,
	BCS_TLEd,
	BCS_TGEd,

	BCS_I2F,
	BCS_F2I,
	BCS_D2I,
	BCS_I2D,
	BCS_D2F,
	BCS_F2D,

	BCS_I2B,
	BCS_B2I,
	BCS_I2W,
	BCS_W2I,
	BCS_I2C,
	BCS_C2I,
	BCS_I2S,
	BCS_S2I,

	BCS_LINE,
	BCS_LABEL,
};



struct PS_BC_READWRITE_FLAG
{
	int readOp0 : 4;
	int readOp1 : 4;
	int writeOp : 4;
	int dummy   : 4;
};


static const PS_BC_READWRITE_FLAG g_psBCReadWriteIndex[] = 
{
	{-1, -1, -1, -1},							// "NOP"
	{-1, -1, -1, -1},							// "SUSPEND"
	{-1, -1, -1, -1},							// "POP"
	{-1, -1, -1, -1},						    // "PUSH"
	{ 0, -1, -1, -1},							// "PUSH4"
	{ 0, -1, -1, -1},							// "PUSH8"
	{ 0, -1, -1, -1},							// "PUSHREF"

	{ 1, -1,  0, -1},							// "MOV1"
	{ 1, -1,  0, -1},							// "MOV2"
	{ 1, -1,  0, -1},							// "MOV4"
	{ 1, -1,  0, -1},							// "MOV8"

	{-1, -1,  0, -1},							// "DEFREF",
	{ 1, -1,  0, -1},							// "RDREF1",
	{ 0,  1, -1, -1},							// "WTREF1",
	{ 1, -1,  0, -1},							// "RDREF2",
	{ 0,  1, -1, -1},							// "WTREF2",
	{ 1, -1,  0, -1},							// "RDREF4",
	{ 0,  1, -1, -1},							// "WTREF4",
	{ 1, -1,  0, -1},							// "RDREF8",
	{ 0,  1, -1, -1},							// "WTREF8",

	{-1, -1,  0, -1},							// "RRET1",
	{ 0, -1, -1, -1},							// "SRET1",
	{-1, -1,  0, -1},							// "RRET2",
	{ 0, -1, -1, -1},							// "SRET2",
	{-1, -1,  0, -1},							// "RRET4",
	{ 0, -1, -1, -1},							// "SRET4",
	{-1, -1,  0, -1},							// "RRET8",
	{ 0, -1, -1, -1},							// "SRET8",
	
	{ 1, -1,  0, -1},							// "PGA",
	{-1, -1,  0, -1},							// "PSP",
	{ 0,  1, -1, -1},							// "MEMCPY",		

	{-1, -1, -1, -1},							// "CALL",
	{-1, -1, -1, -1},							// "RET",
	{-1, -1, -1, -1},							// "CALLSYS",
	{-1, -1, -1, -1},							// "JMP",
	{-1, -1, -1, -1},							// "JMPP",	
	{ 0, -1, -1, -1},							// "JNZ",
	{ 0, -1, -1, -1},							// "JZ",

	{ 1,  2,  0, -1},							// "ADDi",
	{ 1,  2,  0, -1},							// "SUBi",
	{ 1,  2,  0, -1},							// "MULi",
	{ 1,  2,  0, -1},							// "DIVi",
	{ 1,  2,  0, -1},							// "MODi",
	{ 0, -1, -1, -1},							// "INCi",	NOTE: ��ʱ����д��������-1,����Ϊ��ָ���Ż�ʱ����Ҫ�Ż���
	{ 0, -1, -1, -1},							// "DECi",  NOTE: ��ʱ����д��������-1,����Ϊ��ָ���Ż�ʱ����Ҫ�Ż���
	{ 1, -1,  0, -1},							// "NEGi",

	{ 1,  2,  0, -1},							// "ADDf",
	{ 1,  2,  0, -1},							// "SUBf",
	{ 1,  2,  0, -1},							// "MULf",
	{ 1,  2,  0, -1},							// "DIVf",
	{ 1,  2,  0, -1},							// "MODf",
	{ 0, -1, -1, -1},							// "INCf" ,NOTE: ��ʱ����д��������-1,����Ϊ��ָ���Ż�ʱ����Ҫ�Ż���
	{ 0, -1, -1, -1},							// "DECf", NOTE: ��ʱ����д��������-1,����Ϊ��ָ���Ż�ʱ����Ҫ�Ż���
	{ 1, -1,  0, -1},							// "NEGf",

	{ 1,  2,  0, -1},							// "ADDd",
	{ 1,  2,  0, -1},							// "SUBd",
	{ 1,  2,  0, -1},							// "MULd",
	{ 1,  2,  0, -1},							// "DIVd",
	{ 1,  2,  0, -1},							// "MODd",
	{ 0, -1, -1, -1},							// "INCd", NOTE: ��ʱ����д��������-1,����Ϊ��ָ���Ż�ʱ����Ҫ�Ż���
	{ 0, -1, -1, -1},							// "DECd", NOTE: ��ʱ����д��������-1,����Ϊ��ָ���Ż�ʱ����Ҫ�Ż���
	{ 1, -1,  0, -1},							// "NEGd",

	{ 1, -1,  0, -1},							// "NOT",
	{ 1,  2,  0, -1},							// "AND",
	{ 1,  2,  0, -1},							// "OR",
	{ 1,  2,  0, -1},							// "XOR",
	{ 1,  2,  0, -1},							// "SLL",
	{ 1,  2,  0, -1},							// "SRL",
	{ 1,  2,  0, -1},							// "SLA",
	{ 1,  2,  0, -1},							// "SRA",

	{ 1,  2,  0, -1},							// "TEi",
	{ 1,  2,  0, -1},							// "TNEi",
	{ 1,  2,  0, -1},							// "TLi",
	{ 1,  2,  0, -1},							// "TGi",
	{ 1,  2,  0, -1},							// "TLEi",
	{ 1,  2,  0, -1},							// "TGEi",

	{ 1,  2,  0, -1},							// "TZf",
	{ 1,  2,  0, -1},							// "TNZf",
	{ 1,  2,  0, -1},							// "TLf",
	{ 1,  2,  0, -1},							// "TGf",
	{ 1,  2,  0, -1},							// "TLEf",
	{ 1,  2,  0, -1},							// "TGEf",

	{ 1,  2,  0, -1},							// "TZd",
	{ 1,  2,  0, -1},							// "TNZd",
	{ 1,  2,  0, -1},							// "TLd",
	{ 1,  2,  0, -1},							// "TGd",
	{ 1,  2,  0, -1},							// "TLEd",
	{ 1,  2,  0, -1},							// "TGEd",

	{ 1, -1,  0, -1},							// "I2F",
	{ 1, -1,  0, -1},							// "F2I",
	{ 1, -1,  0, -1},							// "D2I",	
	{ 1, -1,  0, -1},							// "I2D",
	{ 1, -1,  0, -1},							// "D2F",
	{ 1, -1,  0, -1},							// "F2D",
	{ 1, -1,  0, -1},							// "I2B",
	{ 1, -1,  0, -1},							// "B2I",
	{ 1, -1,  0, -1},							// "I2W",
	{ 1, -1,  0, -1},							// "W2I",
	{ 1, -1,  0, -1},							// "I2C",
	{ 1, -1,  0, -1},							// "C2I",
	{ 1, -1,  0, -1},							// "I2S",
	{ 1, -1,  0, -1},							// "S2I"

	{-1, -1, -1, -1},							// "LINE"
	{-1, -1, -1, -1},							// "LABLE"
};

#endif // __PS_BYTECODEDEF_H__
