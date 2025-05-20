#ifndef __PS_TOKEN_DEF_H__
#define __PS_TOKEN_DEF_H__

#include "../include/types.h"

enum psETokenType
{	
	ttUnrecognizedToken = 0,

	ttEnd,				   // End of file

	// White space and comments
	ttWhiteSpace,          // ' ', '\t', '\r' '\n'
	ttOnelineComment,      // // \n
	ttMultilineComment,    // /* */

	// Atoms
	ttIdentifier,          // abc123
	ttIntConstant,         // 1234
	ttCharConstant,		   // 'abcd'
	ttWCharConstant,	   // L'a'
	ttFloatConstant,       // 12.34e56f
	ttDoubleConstant,	   // 12.34e56
	ttStringConstant,      // "123"
	ttWStringConstant,	   // L'123"
	ttNonTerminatedStringConstant, // "123
	ttNonTerminatedCharConstant, // '123
	ttBitsConstant,        // 0xFFFF

	// Math operators
	ttPlus,                // +
	ttMinus,               // -
	ttStar,                // *
	ttSlash,               // /
	ttPercent,             // %

	ttHandle,              // #

	ttAddAssign,           // +=
	ttSubAssign,           // -=
	ttMulAssign,           // *=
	ttDivAssign,           // /=
	ttModAssign,           // %=

	ttOrAssign,            // |=
	ttAndAssign,           // &=
	ttXorAssign,           // ^=
	ttShiftLeftAssign,     // <<=
	ttShiftRightAssign,    // >>=

	ttInc,                 // ++
	ttDec,                 // --

	ttDot,                 // .

	// Statement tokens
	ttAssignment,          // =
	ttEndStatement,        // ;
	ttListSeparator,       // ,
	ttStartStatementBlock, // {
	ttEndStatementBlock,   // }
	ttOpenParenthesis,     // (
	ttCloseParenthesis,    // )
	ttOpenBracket,         // [
	ttCloseBracket,        // ]
	ttAmp,                 // &

	// Bitwise operators
	ttBitOr,               // |
	ttBitNot,              // ~
	ttBitXor,              // ^
	ttBitShiftLeft,        // <<
	ttBitShiftRight,       // >>

	// Compare operators
	ttEqual,               // ==
	ttNotEqual,            // !=
	ttLessThan,            // <
	ttGreaterThan,         // >
	ttLessThanOrEqual,     // <=
	ttGreaterThanOrEqual,  // >=

	ttQuestion,            // ?
	ttColon,               // :

	ttNot,                 // not
	ttAnd,				   // and
	ttOr,				   // or
	ttXor,                 // xor

	// Reserved keywords
	ttIf,                  // if
	ttElse,                // else
	ttFor,				   // for
	ttWhile,               // while
	ttBool,                // bool
	ttInt,                 // int
	ttInt8,                // int8
	ttInt16,               // int16
	ttUInt8,			   // uint8
	ttUInt16,			   // uint16
	ttFloat,               // float
	ttVoid,                // void
	ttTrue,                // true
	ttFalse,               // false
	ttReturn,              // return
	ttBreak,               // break
	ttContinue,            // continue
	ttConst,			   // const
	ttDo,                  // do
	ttDouble,              // double
	ttSwitch,              // switch
	ttCase,                // case
	ttDefault,             // default
	ttNull,                // null
	ttNumberDefine,		   // #define
	ttNumberUndef,		   // #undef
	ttNumberIfdef,		   // #ifdef 
	ttNumberIfndef,		   // #ifndef 
	ttNumberElse,		   // #else
	ttNumberEndif,		   // #endif
	ttNumberInclude,	   // #include
	ttClass,			   // class
	ttOperator,			   // operator
	ttThis,				   // this pointer
	ttSizeof,			   // sizeof
	ttEllipsis,	   // ...
	ttArrow,			   // ->
	ttPublic,			   // public
	ttField,			   // ::
	ttEnum,				   // enum
};

#define ttChar ttInt8

#ifdef _MSC_VER	
	#define ttWChar ttInt16
#else
	#define ttWChar ttInt
#endif
	
struct psSToken 
{
	psETokenType	type;	
	int				length;
	int				pos;
	int				index;
};

#endif // __PS_TOKEN_DEF_H__
