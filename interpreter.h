#pragma once

/* picoc main header file - this has all the main data structures and
 * function prototypes. If you're just calling picoc you should look at the
 * external interface instead, in picoc.h */

#include "platform.h"


#ifndef min
#define min(x,y) (((x)<(y))?(x):(y))
#endif

#define MEM_ALIGN(x) (((x) + sizeof(ALIGN_TYPE) - 1) & ~(sizeof(ALIGN_TYPE)-1))

#define GETS_BUF_MAX 256

/* for debugging */
#define PRINT_SOURCE_POS ({ PrintSourceTextErrorLine(Parser->pc->CStdOut, Parser->FileName, Parser->SourceText, Parser->Line, Parser->CharacterPos); PlatformPrintf(Parser->pc->CStdOut, "\n"); })
#define PRINT_TYPE(typ) PlatformPrintf(Parser->pc->CStdOut, "%t\n", typ);

/* small processors use a simplified FILE * for stdio, otherwise use the system FILE * */
//typedef struct OutputStream IOFILE;
typedef FILE IOFILE;

/* coercion of numeric types to other numeric types */
#define IS_FP(v) ((v)->Typ->Base == TypeFP)
#define FP_VAL(v) ((v)->Val->FP)

#define IS_POINTER_COERCIBLE(v, ap) ((ap) ? ((v)->Typ->Base == TypePointer) : 0)
#define POINTER_COERCE(v) ((int)(v)->Val->Pointer)

#define IS_INTEGER_NUMERIC_TYPE(t) ((t)->Base >= TypeInt && (t)->Base <= TypeUnsignedLong)
#define IS_INTEGER_NUMERIC(v) IS_INTEGER_NUMERIC_TYPE((v)->Typ)
#define IS_NUMERIC_COERCIBLE(v) (IS_INTEGER_NUMERIC(v) || IS_FP(v))
#define IS_NUMERIC_COERCIBLE_PLUS_POINTERS(v,ap) (IS_NUMERIC_COERCIBLE(v) || IS_POINTER_COERCIBLE(v,ap))


struct Table;
struct Picoc_Struct;

typedef struct Picoc_Struct Picoc;

/* lexical tokens */
enum LexToken
{
	/* 0x00 */ TokenNone,
	/* 0x01 */ TokenComma,
	/* 0x02 */ TokenAssign, TokenAddAssign, TokenSubtractAssign, TokenMultiplyAssign, TokenDivideAssign, TokenModulusAssign,
	/* 0x08 */ TokenShiftLeftAssign, TokenShiftRightAssign, TokenArithmeticAndAssign, TokenArithmeticOrAssign, TokenArithmeticExorAssign,
	/* 0x0d */ TokenQuestionMark, TokenColon,
	/* 0x0f */ TokenLogicalOr,
	/* 0x10 */ TokenLogicalAnd,
	/* 0x11 */ TokenArithmeticOr,
	/* 0x12 */ TokenArithmeticExor,
	/* 0x13 */ TokenAmpersand,
	/* 0x14 */ TokenEqual, TokenNotEqual,
	/* 0x16 */ TokenLessThan, TokenGreaterThan, TokenLessEqual, TokenGreaterEqual,
	/* 0x1a */ TokenShiftLeft, TokenShiftRight,
	/* 0x1c */ TokenPlus, TokenMinus,
	/* 0x1e */ TokenAsterisk, TokenSlash, TokenModulus,
	/* 0x21 */ TokenIncrement, TokenDecrement, TokenUnaryNot, TokenUnaryExor, TokenSizeof, TokenCast,
	/* 0x27 */ TokenLeftSquareBracket, TokenRightSquareBracket, TokenDot, TokenArrow,
	/* 0x2b */ TokenOpenBracket, TokenCloseBracket,
	/* 0x2d */ TokenIdentifier, TokenIntegerConstant, TokenFPConstant, TokenStringConstant, TokenCharacterConstant,
	/* 0x32 */ TokenSemicolon, TokenEllipsis,
	/* 0x34 */ TokenLeftBrace, TokenRightBrace,
	/* 0x36 */ TokenIntType, TokenCharType, TokenFloatType, TokenDoubleType, TokenVoidType, TokenEnumType,
	/* 0x3c */ TokenLongType, TokenSignedType, TokenShortType, TokenStaticType, TokenAutoType, TokenRegisterType, TokenExternType, TokenStructType, TokenUnionType, TokenUnsignedType, TokenTypedef,
	/* 0x46 */ TokenContinue, TokenDo, TokenElse, TokenFor, TokenGoto, TokenIf, TokenWhile, TokenBreak, TokenSwitch, TokenCase, TokenDefault, TokenReturn,
	/* 0x52 */ TokenHashDefine, TokenHashInclude, TokenHashIf, TokenHashIfdef, TokenHashIfndef, TokenHashElse, TokenHashEndif,
	/* 0x59 */ TokenNew, TokenDelete,
	/* 0x5b */ TokenOpenMacroBracket,
	/* 0x5c */ TokenEOF, TokenEndOfLine, TokenEndOfFunction
};

/* used in dynamic memory allocation */
struct AllocNode
{
	unsigned int Size;
	struct AllocNode *NextFree;
};

/* whether we're running or skipping code */
enum RunMode
{
	RunModeRun,                 /* we're running code as we parse it */
	RunModeSkip,                /* skipping code, not running */
	RunModeReturn,              /* returning from a function */
	RunModeCaseSearch,          /* searching for a case label */
	RunModeBreak,               /* breaking out of a switch/while/do */
	RunModeContinue,            /* as above but repeat the loop */
	RunModeGoto                 /* searching for a goto label */
};

/* parser state - has all this detail so we can parse nested files */
struct ParseState
{
	Picoc *pc;							/* the picoc instance this parser is a part of */
	const unsigned char *Pos;			/* the character position in the source text */
	char *FileName;						/* what file we're executing (registered string) */
	short int Line;						/* line number we're executing */
	short int CharacterPos;				/* character/column in the line we're executing */
	enum RunMode Mode;					/* whether to skip or run code */
	int SearchLabel;					/* what case label we're searching for */
	const char *SearchGotoLabel;		/* what goto label we're searching for */
	const char *SourceText;				/* the entire source text */
	short int HashIfLevel;				/*how many "if"s we're nested down */
	short int HashIfEvaluateToLevel;	/* if we're not evaluating an if branch, what the last evaluated level was */
	char DebugMode;						/* debugging mode */
	int ScopeID;						/* for keeping track of local variables (free them after they go out of scope) */
};

/* values */
enum BaseType
{
	TypeVoid,					/* no type */
	TypeInt,					/* integer */
	TypeShort,					/* short integer */
	TypeChar,					/* a single character (signed) */
	TypeLong,					/* long integer */
	TypeUnsignedInt,			/* unsigned integer */
	TypeUnsignedShort,			/* unsigned short integer */
	TypeUnsignedChar,			/* unsigned 8-bit number */ /* must be before unsigned long */
	TypeUnsignedLong,			/* unsigned long integer */
	TypeFP,						/* floating point */
	TypeFunction,				/* a function */
	TypeMacro,					/* a macro */
	TypePointer,				/* a pointer */
	TypeArray,					/* an array of a sub-type */
	TypeStruct,					/* aggregate type */
	TypeUnion,					/* merged type */
	TypeEnum,					/* enumerated integer type */
	TypeGotoLabel,				/* a label we can "goto" */
	Type_Type					/* a type for storing types */
};

/* data type */
struct ValueType
{
	enum BaseType Base;					/* what kind of type this is */
	int ArraySize;						/* the size of an array type */
	int Sizeof;							/* the storage required */
	int AlignBytes;						/* the alignment boundary of this type */
	const char *Identifier;				/* the name of a struct or union */
	struct ValueType *FromType;			/* the type we're derived from (or nullptr) */
	struct ValueType *DerivedTypeList;	/* first in a list of types derived from this one */
	struct ValueType *Next;				/* next item in the derived type list */
	struct Table *Members;				/* members of a struct or union */
	int OnHeap;							/* true if allocated on the heap */
	int StaticQualifier;				/* true if it's a static */
};

/* function definition */
struct FuncDef
{
	struct ValueType *ReturnType;	/* the return value type */
	int NumParams;					/* the number of parameters */
	int VarArgs;					/* has a variable number of arguments after the explicitly specified ones */
	struct ValueType **ParamType;	/* array of parameter types */
	char **ParamName;				/* array of parameter names */
	void (*Intrinsic)();			/* intrinsic call address or nullptr */
	struct ParseState Body;			/* lexical tokens of the function body if not intrinsic */
};

/* macro definition */
struct MacroDef
{
	int NumParams;					/* the number of parameters */
	char **ParamName;				/* array of parameter names */
	struct ParseState Body;			/* lexical tokens of the function body if not intrinsic */
};

/* values */
union AnyValue
{
	char Character;
	short ShortInteger;
	int Integer;
	long LongInteger;
	unsigned short UnsignedShortInteger;
	unsigned int UnsignedInteger;
	unsigned long UnsignedLongInteger;
	unsigned char UnsignedCharacter;
	char *Identifier;
	char ArrayMem[2];				/* placeholder for where the data starts, doesn't point to it */
	struct ValueType *Typ;
	struct FuncDef FuncDef;
	struct MacroDef MacroDef;
	double FP;
	void *Pointer;					/* unsafe native pointers */
};

struct Value
{
	struct ValueType *Typ;			/* the type of this value */
	union AnyValue *Val;			/* pointer to the AnyValue which holds the actual content */
	struct Value *LValueFrom;		/* if an LValue, this is a Value our LValue is contained within (or NULL) */
	char ValOnHeap;					/* this Value is on the heap */
	char ValOnStack;				/* the AnyValue is on the stack along with this Value */
	char AnyValOnHeap;				/* the AnyValue is separately allocated from the Value on the heap */
	char IsLValue;					/* is modifiable and is allocated somewhere we can usefully modify it */
	int ScopeID;					/* to know when it goes out of scope */
	char OutOfScope;
};


union TableEntryPayload
{
	struct ValueEntry
	{
		char *Key;				/* points to the shared string table */
		struct Value *Val;		/* the value we're storing */
	} v;						/* used for tables of values */

	char Key[1];				/* dummy size - used for the shared string table */

	struct BreakpointEntry		/* defines a breakpoint */
	{
		const char *FileName;
		short int Line;
		short int CharacterPos;
	} b;

};

/* hash table data structure */
struct TableEntry
{
	struct TableEntry *Next;		/* next item in this hash chain */
	const char *DeclFileName;		/* where the variable was declared */
	unsigned short DeclLine;
	unsigned short DeclColumn;

	union TableEntryPayload p;
};

struct Table
{
	short Size;
	bool OnHeap;
	struct TableEntry **HashTable;
};

/* stack frame for function calls */
struct StackFrame
{
	struct ParseState ReturnParser;			/* how we got here */
	const char *FuncName;					/* the name of the function we're in */
	struct Value *ReturnValue;				/* copy the return value here */
	struct Value **Parameter;				/* array of parameter values */
	int NumParams;											/* the number of parameters */
	struct Table LocalTable;								/* the local variables and parameters */
	struct TableEntry *LocalHashTable[LOCAL_TABLE_SIZE];
	struct StackFrame *PreviousStackFrame;					/* the next lower stack frame */
};

/* lexer state */
enum LexMode
{
	LexModeNormal,
	LexModeHashInclude,
	LexModeHashDefine,
	LexModeHashDefineSpace,
	LexModeHashDefineSpaceIdent
};

struct LexState
{
	const char *Pos;
	const char *End;
	const char *FileName;
	int Line;
	int CharacterPos;
	const char *SourceText;
	enum LexMode Mode;
	int EmitExtraNewlines;
};

/* library function definition */
struct LibraryFunction
{
	void (*Func)(struct ParseState *, struct Value *, struct Value **, int);
	const char *Prototype;
};

struct StringOutputStream
{
	struct ParseState *Parser;
	char *WritePos;
};

/* output stream-type specific state information */
union OutputStreamInfo
{
	struct StringOutputStream Str;
};

/* stream-specific method for writing characters to the console */
typedef void CharWriter(unsigned char, union OutputStreamInfo *);

/* used when writing output to a string - eg. sprintf() */
struct OutputStream
{
	CharWriter *Putch;
	union OutputStreamInfo i;
};

/* possible results of parsing a statement */
enum ParseResult { ParseResultEOF, ParseResultError, ParseResultOk };

/* a chunk of heap-allocated tokens we'll cleanup when we're done */
struct CleanupTokenNode
{
	void *Tokens;
	const char *SourceText;
	struct CleanupTokenNode *Next;
};

/* linked list of lexical tokens used in interactive mode */
struct TokenLine
{
	struct TokenLine *Next;
	unsigned char *Tokens;
	int NumBytes;
};


/* a list of libraries we can include */
struct IncludeLibrary
{
	char *IncludeName;
	void (*SetupFunction)(Picoc *pc);
	struct LibraryFunction *FuncList;
	const char *SetupCSource;
	struct IncludeLibrary *NextLib;
};

#define SPLIT_MEM_THRESHOLD 16                      /* don't split memory which is close in size */
#define BREAKPOINT_TABLE_SIZE 21


/* the entire state of the picoc system */
struct Picoc_Struct
{
	/* parser global data */
	struct Table GlobalTable;
	struct CleanupTokenNode *CleanupTokenList;
	struct TableEntry *GlobalHashTable[GLOBAL_TABLE_SIZE];

	/* lexer global data */
	struct TokenLine *InteractiveHead;
	struct TokenLine *InteractiveTail;
	struct TokenLine *InteractiveCurrentLine;
	int LexUseStatementPrompt;
	union AnyValue LexAnyValue;
	struct Value LexValue;
	struct Table ReservedWordTable;
	struct TableEntry *ReservedWordHashTable[RESERVED_WORD_TABLE_SIZE];

	/* the table of string literal values */
	struct Table StringLiteralTable;
	struct TableEntry *StringLiteralHashTable[STRING_LITERAL_TABLE_SIZE];

	/* the stack */
	struct StackFrame *TopStackFrame;

	/* the value passed to exit() */
	int PicocExitValue;

	/* a list of libraries we can include */
	struct IncludeLibrary *IncludeLibList;

	/* heap memory */
	unsigned char *HeapMemory;			/* stack memory since our heap is malloc()ed */
	void *HeapBottom;					/* the bottom of the (downward-growing) heap */
	void *StackFrame;					/* the current stack frame */
	void *HeapStackTop;					/* the top of the stack */

	/* types */
	struct ValueType UberType;
	struct ValueType IntType;
	struct ValueType ShortType;
	struct ValueType CharType;
	struct ValueType LongType;
	struct ValueType UnsignedIntType;
	struct ValueType UnsignedShortType;
	struct ValueType UnsignedLongType;
	struct ValueType UnsignedCharType;
	struct ValueType FPType;
	struct ValueType VoidType;
	struct ValueType TypeType;
	struct ValueType FunctionType;
	struct ValueType MacroType;
	struct ValueType EnumType;
	struct ValueType GotoLabelType;
	struct ValueType *CharPtrType;
	struct ValueType *CharPtrPtrType;
	struct ValueType *CharArrayType;
	struct ValueType *VoidPtrType;

	/* debugger */
	struct Table BreakpointTable;
	struct TableEntry *BreakpointHashTable[BREAKPOINT_TABLE_SIZE];
	int BreakpointCount;
	int DebugManualBreak;

	/* C library */
	int BigEndian;
	int LittleEndian;

	IOFILE *CStdOut;
	IOFILE CStdOutBase;

	/* the picoc version string */
	const char *VersionString;

	/* exit longjump buffer */
	jmp_buf PicocExitBuf;

	/* string table */
	struct Table StringTable;
	struct TableEntry *StringHashTable[STRING_TABLE_SIZE];
	char *StrEmpty;
};

/* table.c */
void TableInit(Picoc *);
char *TableStrRegister(Picoc *, const char *);
char *TableStrRegister2(Picoc *, const char *, int);
void TableInitTable(struct Table *, struct TableEntry **, int, bool);
int TableSet(Picoc *, struct Table *, char *, struct Value *, const char *, int, int);
int TableGet(struct Table *, const char *, struct Value **, const char **, int *, int *);
struct Value *TableDelete(Picoc *pc, struct Table *, const char *);
char *TableSetIdentifier(Picoc *, struct Table *, const char *, int);
void TableStrFree(Picoc *);

/* lex.c */
void LexInit(Picoc *);
void LexCleanup(Picoc *);
void *LexAnalyse(Picoc *, const char *, const char *, int, int *);
void LexInitParser(struct ParseState *, Picoc *, const char *, void *, char *, int, int);
enum LexToken LexGetToken(struct ParseState *, struct Value **, int);
enum LexToken LexRawPeekToken(struct ParseState *);
void LexToEndOfLine(struct ParseState *);
void *LexCopyTokens(struct ParseState *, struct ParseState *);
void LexInteractiveClear(Picoc *, struct ParseState *);
void LexInteractiveCompleted(Picoc *, struct ParseState *);
void LexInteractiveStatementPrompt(Picoc *);

/* parse.c */
/* the following are defined in picoc.h:
 * void PicocParse(const char *FileName, const char *Source, int SourceLen, int RunIt, int CleanupNow, int CleanupSource);
 * void PicocParseInteractive(); */
void PicocParseInteractiveNoStartPrompt(Picoc *, int);
enum ParseResult ParseStatement(struct ParseState *, int);
struct Value *ParseFunctionDefinition(struct ParseState *, struct ValueType *, char *);
void ParseCleanup(Picoc *pc);
void ParserCopyPos(struct ParseState *, struct ParseState *);
void ParserCopy(struct ParseState *, struct ParseState *);

/* expression.c */
int ExpressionParse(struct ParseState *, struct Value **);
long ExpressionParseInt(struct ParseState *);
void ExpressionAssign(struct ParseState *, struct Value *, struct Value *, int, const char *, int, int);
long ExpressionCoerceInteger(struct Value *);
unsigned long ExpressionCoerceUnsignedInteger(struct Value *);
double ExpressionCoerceFP(struct Value *);

/* type.c */
void TypeInit(Picoc *);
void TypeCleanup(Picoc *);
int TypeSize(struct ValueType *, int, int );
int TypeSizeValue(struct Value *, int );
int TypeStackSizeValue(struct Value *);
int TypeLastAccessibleOffset(Picoc *, struct Value *);
int TypeParseFront(struct ParseState *, struct ValueType **, int *);
void TypeParseIdentPart(struct ParseState *, struct ValueType *, struct ValueType **, char **);
void TypeParse(struct ParseState *, struct ValueType **, char **, int *);
struct ValueType *TypeGetMatching(Picoc *, struct ParseState *, struct ValueType *, enum BaseType, int, const char *, int);
struct ValueType *TypeCreateOpaqueStruct(Picoc *, struct ParseState *, const char *, int);
int TypeIsForwardDeclared(struct ParseState *, struct ValueType *);

/* heap.c */
void HeapInit(Picoc *, int);
void HeapCleanup(Picoc *);
void *HeapAllocStack(Picoc *, int);
bool HeapPopStack(Picoc *, int);
void HeapUnpopStack(Picoc *, int);
void HeapPushStackFrame(Picoc *);
int HeapPopStackFrame(Picoc *);
void *HeapAllocMem(Picoc *, int);
void HeapFreeMem(Picoc *, void *);

/* variable.c */
void VariableInit(Picoc *);
void VariableCleanup(Picoc *);
void VariableFree(Picoc *, struct Value *);
void VariableTableCleanup(Picoc *, struct Table *);
void *VariableAlloc(Picoc *, struct ParseState *, int, int);
void VariableStackPop(struct ParseState *, struct Value *);
struct Value *VariableAllocValueAndData(Picoc *, struct ParseState *, int, int, struct Value *, int);
struct Value *VariableAllocValueAndCopy(Picoc *, struct ParseState *, struct Value *, int);
struct Value *VariableAllocValueFromType(Picoc *, struct ParseState *, struct ValueType *, int, struct Value *, int );
struct Value *VariableAllocValueFromExistingData(struct ParseState *, struct ValueType *, union AnyValue *, int, struct Value *);
struct Value *VariableAllocValueShared(struct ParseState *, struct Value *);
struct Value *VariableDefine(Picoc *pc, struct ParseState *, char *, struct Value *, struct ValueType *, int);
struct Value *VariableDefineButIgnoreIdentical(struct ParseState *, char *Ident, struct ValueType *, int, int *);
int VariableDefined(Picoc *, const char *);
int VariableDefinedAndOutOfScope(Picoc *, const char *);
void VariableRealloc(struct ParseState *, struct Value *, int);
void VariableGet(Picoc *, struct ParseState *, const char *, struct Value **);
void VariableDefinePlatformVar(Picoc *, struct ParseState *, const char *, struct ValueType *, union AnyValue *, int);
void VariableStackFrameAdd(struct ParseState *, const char *, int);
void VariableStackFramePop(struct ParseState *);
struct Value *VariableStringLiteralGet(Picoc *, char *);
void VariableStringLiteralDefine(Picoc *, char *, struct Value *);
void *VariableDereferencePointer(struct ParseState *, struct Value *, struct Value **, int *, struct ValueType **, int *);
int VariableScopeBegin(struct ParseState *, int*);
void VariableScopeEnd(struct ParseState *, int, int);

/* clibrary.c */
void BasicIOInit(Picoc *);
void LibraryInit(Picoc *);
void LibraryAdd(Picoc *, struct Table *, const char *, struct LibraryFunction *);
void CLibraryInit(Picoc *);
void PrintCh(char, IOFILE *);
void PrintSimpleInt(long, IOFILE *);
void PrintInt(long Num, int, int, int, IOFILE *);
void PrintStr(const char *, IOFILE *);
void PrintFP(double, IOFILE *);
void PrintType(struct ValueType *, IOFILE *);
void LibPrintf(struct ParseState *, struct Value *, struct Value **, int);

/* platform.c */
/* the following are defined in picoc.h:
 * void PicocCallMain(int argc, char **argv);
 * int PicocPlatformSetExitPoint();
 * void PicocInitialise(int StackSize);
 * void PicocCleanup();
 * void PicocPlatformScanFile(const char *FileName);
 * extern int PicocExitValue; */
void ProgramFail(struct ParseState *, const char *, ...);
void ProgramFailNoParser(Picoc *, const char *, ...);
void AssignFail(struct ParseState *, const char *, struct ValueType *, struct ValueType *, int, int, const char *, int);
void LexFail(Picoc *pc, struct LexState *, const char *, ...);
void PlatformInit(Picoc *);
void PlatformCleanup(Picoc *pc);
char *PlatformGetLine(char *, int, const char *);
int PlatformGetCharacter();
void PlatformPutc(unsigned char, union OutputStreamInfo *);
void PlatformPrintf(IOFILE *, const char *, ...);
void PlatformVPrintf(IOFILE *, const char *, va_list);
void PlatformExit(Picoc *, int);
char *PlatformMakeTempName(Picoc *, char *);
void PlatformLibraryInit(Picoc *);

/* include.c */
void IncludeInit(Picoc *);
void IncludeCleanup(Picoc *);
void IncludeRegister(Picoc *, const char *, void (*)(Picoc *pc), struct LibraryFunction *, const char *);
void IncludeFile(Picoc *, char *);
/* the following is defined in picoc.h:
 * void PicocIncludeAllSystemHeaders(); */

/* debug.c */
void DebugInit(Picoc *);
void DebugCleanup(Picoc *);
void DebugCheckStatement(struct ParseState *);


/* stdio.c */
extern const char StdioDefs[];
extern struct LibraryFunction StdioFunctions[];
void StdioSetupFunc(Picoc *);

/* math.c */
extern struct LibraryFunction MathFunctions[];
void MathSetupFunc(Picoc *);

/* string.c */
extern struct LibraryFunction StringFunctions[];
void StringSetupFunc(Picoc *);

/* stdlib.c */
extern struct LibraryFunction StdlibFunctions[];
void StdlibSetupFunc(Picoc *);

/* time.c */
extern const char StdTimeDefs[];
extern struct LibraryFunction StdTimeFunctions[];
void StdTimeSetupFunc(Picoc *pc);

/* errno.c */
void StdErrnoSetupFunc(Picoc *pc);

/* ctype.c */
extern struct LibraryFunction StdCtypeFunctions[];

/* stdbool.c */
extern const char StdboolDefs[];
void StdboolSetupFunc(Picoc *);

/* unistd.c */
extern const char UnistdDefs[];
extern struct LibraryFunction UnistdFunctions[];
void UnistdSetupFunc(Picoc *);
