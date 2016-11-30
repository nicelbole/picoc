/* string.h library for large systems - small embedded systems use clibrary.c instead */
#include <time.h>
#include "../interpreter.h"

static int CLOCKS_PER_SECValue = CLOCKS_PER_SEC;

#ifdef CLK_PER_SEC
static int CLK_PER_SECValue = CLK_PER_SEC;
#endif

#ifdef CLK_TCK
static int CLK_TCKValue = CLK_TCK;
#endif

void StdAsctime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Pointer = asctime((const tm*)Param[0]->Val->Pointer);
}

void StdClock(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Integer = clock();
}

void StdCtime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Pointer = ctime((const long int*)Param[0]->Val->Pointer);
}

void StdDifftime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->FP = difftime((time_t)Param[0]->Val->Integer, Param[1]->Val->Integer);
}

void StdGmtime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Pointer = gmtime((const long int*)Param[0]->Val->Pointer);
}

void StdLocaltime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Pointer = localtime((const long int*)Param[0]->Val->Pointer);
}

void StdMktime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Integer = (int)mktime((tm*)Param[0]->Val->Pointer);
}

void StdTime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Integer = (int)time((long int*)Param[0]->Val->Pointer);
}

void StdStrftime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Integer = strftime((char*)Param[0]->Val->Pointer, (unsigned long int)Param[1]->Val->Integer, (const char*)Param[2]->Val->Pointer, (const tm*)Param[3]->Val->Pointer);
}

void StdStrptime(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	extern char *strptime(const char *s, const char *format, struct tm *tm);

	ReturnValue->Val->Pointer = strptime((const char*)Param[0]->Val->Pointer, (const char*)Param[1]->Val->Pointer, (tm*)Param[2]->Val->Pointer);
}

void StdGmtime_r(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Pointer = gmtime_r((const long int*)Param[0]->Val->Pointer, (tm*)Param[1]->Val->Pointer);
}

void StdTimegm(struct ParseState *Parser, struct Value *ReturnValue, struct Value **Param, int NumArgs)
{
	ReturnValue->Val->Integer = timegm((tm*)Param[0]->Val->Pointer);
}

/* handy structure definitions */
const char StdTimeDefs[] = "\
typedef int time_t; \
typedef int clock_t;\
";

/* all string.h functions */
struct LibraryFunction StdTimeFunctions[] =
{
		{ StdAsctime,		"char *asctime(struct tm *);" },
		{ StdClock,			"time_t clock();" },
		{ StdCtime,			"char *ctime(int *);" },
		{ StdDifftime,		"double difftime(int, int);" },
		{ StdGmtime,		"struct tm *gmtime(int *);" },
		{ StdLocaltime,		"struct tm *localtime(int *);" },
		{ StdMktime,		"int mktime(struct tm *ptm);" },
		{ StdTime,			"int time(int *);" },
		{ StdStrftime,		"int strftime(char *, int, char *, struct tm *);" },
		{ StdStrptime,		"char *strptime(char *, char *, struct tm *);" },
		{ StdGmtime_r,		"struct tm *gmtime_r(int *, struct tm *);" },
		{ StdTimegm,		"int timegm(struct tm *);" },
		{ nullptr,			nullptr }
};


/* creates various system-dependent definitions */
void StdTimeSetupFunc(Picoc *pc)
{
	/* make a "struct tm" which is the same size as a native tm structure */
	TypeCreateOpaqueStruct(pc, nullptr, TableStrRegister(pc, "tm"), sizeof(struct tm));

	/* define CLK_PER_SEC etc. */
	VariableDefinePlatformVar(pc, nullptr, "CLOCKS_PER_SEC", &pc->IntType, (union AnyValue *)&CLOCKS_PER_SECValue, false);
#ifdef CLK_PER_SEC
	VariableDefinePlatformVar(pc, nullptr, "CLK_PER_SEC", &pc->IntType, (union AnyValue *)&CLK_PER_SECValue, false);
#endif
#ifdef CLK_TCK
	VariableDefinePlatformVar(pc, nullptr, "CLK_TCK", &pc->IntType, (union AnyValue *)&CLK_TCKValue, false);
#endif
}
