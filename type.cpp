/* picoc data type module. This manages a tree of data types and has facilities
 * for parsing data types. */

#include "interpreter.h"

/* some basic types */
static int PointerAlignBytes;
static int IntAlignBytes;


/* add a new type to the set of types we know about */
struct ValueType *TypeAdd(Picoc *pc, struct ParseState *Parser, struct ValueType *ParentType, enum BaseType Base, int ArraySize, const char *Identifier, int Sizeof, int AlignBytes)
{
	struct ValueType *NewType = (ValueType*)VariableAlloc(pc, Parser, sizeof(struct ValueType), true);
	NewType->Base = Base;
	NewType->ArraySize = ArraySize;
	NewType->Sizeof = Sizeof;
	NewType->AlignBytes = AlignBytes;
	NewType->Identifier = Identifier;
	NewType->Members = nullptr;
	NewType->FromType = ParentType;
	NewType->DerivedTypeList = nullptr;
	NewType->OnHeap = true;
	NewType->Next = ParentType->DerivedTypeList;
	ParentType->DerivedTypeList = NewType;

	return NewType;
}

/* given a parent type, get a matching derived type and make one if necessary.
 * Identifier should be registered with the shared string table. */
struct ValueType *TypeGetMatching(Picoc *pc, struct ParseState *Parser, struct ValueType *ParentType, enum BaseType Base, int ArraySize, const char *Identifier, int AllowDuplicates)
{
    int Sizeof;
    int AlignBytes;
    struct ValueType *ThisType = ParentType->DerivedTypeList;
    while (ThisType != nullptr && (ThisType->Base != Base || ThisType->ArraySize != ArraySize || ThisType->Identifier != Identifier))
        ThisType = ThisType->Next;

    if (ThisType != nullptr)
    {
        if (AllowDuplicates)
            return ThisType;
        else
            ProgramFail(Parser, "data type '%s' is already defined", Identifier);
    }

    switch (Base)
    {
        case TypePointer:   Sizeof = sizeof(void *); AlignBytes = PointerAlignBytes; break;
        case TypeArray:     Sizeof = ArraySize * ParentType->Sizeof; AlignBytes = ParentType->AlignBytes; break;
        case TypeEnum:      Sizeof = sizeof(int); AlignBytes = IntAlignBytes; break;
        default:            Sizeof = 0; AlignBytes = 0; break;      /* structs and unions will get bigger when we add members to them */
    }

    return TypeAdd(pc, Parser, ParentType, Base, ArraySize, Identifier, Sizeof, AlignBytes);
}

/* stack space used by a value */
int TypeStackSizeValue(struct Value *Val)
{
    if (Val != nullptr && Val->ValOnStack)
        return TypeSizeValue(Val, false);
    else
        return 0;
}

/* memory used by a value */
int TypeSizeValue(struct Value *Val, int Compact)
{
    if (IS_INTEGER_NUMERIC(Val) && !Compact)
        return sizeof(ALIGN_TYPE);     /* allow some extra room for type extension */
    else if (Val->Typ->Base != TypeArray)
        return Val->Typ->Sizeof;
    else
        return Val->Typ->FromType->Sizeof * Val->Typ->ArraySize;
}

/* memory used by a variable given its type and array size */
int TypeSize(struct ValueType *Typ, int ArraySize, int Compact)
{
    if (IS_INTEGER_NUMERIC_TYPE(Typ) && !Compact)
        return sizeof(ALIGN_TYPE);     /* allow some extra room for type extension */
    else if (Typ->Base != TypeArray)
        return Typ->Sizeof;
    else
        return Typ->FromType->Sizeof * ArraySize;
}

/* add a base type */
void TypeAddBaseType(Picoc *pc, struct ValueType *TypeNode, enum BaseType Base, int Sizeof, int AlignBytes)
{
    TypeNode->Base = Base;
    TypeNode->ArraySize = 0;
    TypeNode->Sizeof = Sizeof;
    TypeNode->AlignBytes = AlignBytes;
    TypeNode->Identifier = pc->StrEmpty;
    TypeNode->Members = nullptr;
    TypeNode->FromType = nullptr;
    TypeNode->DerivedTypeList = nullptr;
    TypeNode->OnHeap = false;
    TypeNode->Next = pc->UberType.DerivedTypeList;
    pc->UberType.DerivedTypeList = TypeNode;
}

/* initialise the type system */
void TypeInit(Picoc *pc)
{
    struct IntAlign { char x; int y; } ia;
    struct ShortAlign { char x; short y; } sa;
    struct CharAlign { char x; char y; } ca;
    struct LongAlign { char x; long y; } la;
#ifndef NO_FP
    struct DoubleAlign { char x; double y; } da;
#endif
    struct PointerAlign { char x; void *y; } pa;

    IntAlignBytes = (char *)&ia.y - &ia.x;
    PointerAlignBytes = (char *)&pa.y - &pa.x;

    pc->UberType.DerivedTypeList = nullptr;
    TypeAddBaseType(pc, &pc->IntType, TypeInt, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, &pc->ShortType, TypeShort, sizeof(short), (char *)&sa.y - &sa.x);
    TypeAddBaseType(pc, &pc->CharType, TypeChar, sizeof(char), (char *)&ca.y - &ca.x);
    TypeAddBaseType(pc, &pc->LongType, TypeLong, sizeof(long), (char *)&la.y - &la.x);
    TypeAddBaseType(pc, &pc->UnsignedIntType, TypeUnsignedInt, sizeof(unsigned int), IntAlignBytes);
    TypeAddBaseType(pc, &pc->UnsignedShortType, TypeUnsignedShort, sizeof(unsigned short), (char *)&sa.y - &sa.x);
    TypeAddBaseType(pc, &pc->UnsignedLongType, TypeUnsignedLong, sizeof(unsigned long), (char *)&la.y - &la.x);
    TypeAddBaseType(pc, &pc->UnsignedCharType, TypeUnsignedChar, sizeof(unsigned char), (char *)&ca.y - &ca.x);
    TypeAddBaseType(pc, &pc->VoidType, TypeVoid, 0, 1);
    TypeAddBaseType(pc, &pc->FunctionType, TypeFunction, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, &pc->MacroType, TypeMacro, sizeof(int), IntAlignBytes);
    TypeAddBaseType(pc, &pc->GotoLabelType, TypeGotoLabel, 0, 1);
#ifndef NO_FP
    TypeAddBaseType(pc, &pc->FPType, TypeFP, sizeof(double), (char *)&da.y - &da.x);
    TypeAddBaseType(pc, &pc->TypeType, Type_Type, sizeof(double), (char *)&da.y - &da.x);  /* must be large enough to cast to a double */
#else
    TypeAddBaseType(pc, &pc->TypeType, Type_Type, sizeof(struct ValueType *), PointerAlignBytes);
#endif
    pc->CharArrayType = TypeAdd(pc, nullptr, &pc->CharType, TypeArray, 0, pc->StrEmpty, sizeof(char), (char *)&ca.y - &ca.x);
    pc->CharPtrType = TypeAdd(pc, nullptr, &pc->CharType, TypePointer, 0, pc->StrEmpty, sizeof(void *), PointerAlignBytes);
    pc->CharPtrPtrType = TypeAdd(pc, nullptr, pc->CharPtrType, TypePointer, 0, pc->StrEmpty, sizeof(void *), PointerAlignBytes);
    pc->VoidPtrType = TypeAdd(pc, nullptr, &pc->VoidType, TypePointer, 0, pc->StrEmpty, sizeof(void *), PointerAlignBytes);
}

/* deallocate heap-allocated types */
void TypeCleanupNode(Picoc *pc, struct ValueType *Typ)
{
    struct ValueType *SubType;
    struct ValueType *NextSubType;

    /* clean up and free all the sub-nodes */
    for (SubType = Typ->DerivedTypeList; SubType != nullptr; SubType = NextSubType)
    {
        NextSubType = SubType->Next;
        TypeCleanupNode(pc, SubType);
        if (SubType->OnHeap)
        {
            /* if it's a struct or union deallocate all the member values */
            if (SubType->Members != nullptr)
            {
                VariableTableCleanup(pc, SubType->Members);
                HeapFreeMem(pc, SubType->Members);
            }

            /* free this node */
            HeapFreeMem(pc, SubType);
        }
    }
}

void TypeCleanup(Picoc *pc)
{
    TypeCleanupNode(pc, &pc->UberType);
}

/* parse a struct or union declaration */
void TypeParseStruct(struct ParseState *Parser, struct ValueType **Typ, int IsStruct)
{
    struct Value *LexValue;
    struct ValueType *MemberType;
    char *MemberIdentifier;
    char *StructIdentifier;
    struct Value *MemberValue;
    enum LexToken Token;
    int AlignBoundary;
    Picoc *pc = Parser->pc;

    Token = LexGetToken(Parser, &LexValue, false);
    if (Token == TokenIdentifier)
    {
        LexGetToken(Parser, &LexValue, true);
        StructIdentifier = LexValue->Val->Identifier;
        Token = LexGetToken(Parser, nullptr, false);
    }
    else
    {
        static char TempNameBuf[7] = "^s0000";
        StructIdentifier = PlatformMakeTempName(pc, TempNameBuf);
    }

    *Typ = TypeGetMatching(pc, Parser, &Parser->pc->UberType, IsStruct ? TypeStruct : TypeUnion, 0, StructIdentifier, true);
    if (Token == TokenLeftBrace && (*Typ)->Members != nullptr)
        ProgramFail(Parser, "data type '%t' is already defined", *Typ);

    Token = LexGetToken(Parser, nullptr, false);
    if (Token != TokenLeftBrace)
    {
        /* use the already defined structure */
#if 0
        if ((*Typ)->Members == nullptr)
            ProgramFail(Parser, "structure '%s' isn't defined", LexValue->Val->Identifier);
#endif
        return;
    }

    if (pc->TopStackFrame != nullptr)
        ProgramFail(Parser, "struct/union definitions can only be globals");

    LexGetToken(Parser, nullptr, true);
    (*Typ)->Members = (Table*)VariableAlloc(pc, Parser, sizeof(struct Table) + STRUCT_TABLE_SIZE * sizeof(struct TableEntry), true);
    (*Typ)->Members->HashTable = (struct TableEntry **)((char *)(*Typ)->Members + sizeof(struct Table));
    TableInitTable((*Typ)->Members, (struct TableEntry **)((char *)(*Typ)->Members + sizeof(struct Table)), STRUCT_TABLE_SIZE, true);

    do {
        TypeParse(Parser, &MemberType, &MemberIdentifier, nullptr);
        if (MemberType == nullptr || MemberIdentifier == nullptr)
            ProgramFail(Parser, "invalid type in struct");

        MemberValue = VariableAllocValueAndData(pc, Parser, sizeof(int), false, nullptr, true);
        MemberValue->Typ = MemberType;
        if (IsStruct)
        {
            /* allocate this member's location in the struct */
            AlignBoundary = MemberValue->Typ->AlignBytes;
            if (((*Typ)->Sizeof & (AlignBoundary-1)) != 0)
                (*Typ)->Sizeof += AlignBoundary - ((*Typ)->Sizeof & (AlignBoundary-1));

            MemberValue->Val->Integer = (*Typ)->Sizeof;
            (*Typ)->Sizeof += TypeSizeValue(MemberValue, true);
        }
        else
        {
            /* union members always start at 0, make sure it's big enough to hold the largest member */
            MemberValue->Val->Integer = 0;
            if (MemberValue->Typ->Sizeof > (*Typ)->Sizeof)
                (*Typ)->Sizeof = TypeSizeValue(MemberValue, true);
        }

        /* make sure to align to the size of the largest member's alignment */
        if ((*Typ)->AlignBytes < MemberValue->Typ->AlignBytes)
            (*Typ)->AlignBytes = MemberValue->Typ->AlignBytes;

        /* define it */
        if (!TableSet(pc, (*Typ)->Members, MemberIdentifier, MemberValue, Parser->FileName, Parser->Line, Parser->CharacterPos))
            ProgramFail(Parser, "member '%s' already defined", &MemberIdentifier);

        if (LexGetToken(Parser, nullptr, true) != TokenSemicolon)
            ProgramFail(Parser, "semicolon expected");

    } while (LexGetToken(Parser, nullptr, false) != TokenRightBrace);

    /* now align the structure to the size of its largest member's alignment */
    AlignBoundary = (*Typ)->AlignBytes;
    if (((*Typ)->Sizeof & (AlignBoundary-1)) != 0)
        (*Typ)->Sizeof += AlignBoundary - ((*Typ)->Sizeof & (AlignBoundary-1));

    LexGetToken(Parser, nullptr, true);
}

/* create a system struct which has no user-visible members */
struct ValueType *TypeCreateOpaqueStruct(Picoc *pc, struct ParseState *Parser, const char *StructName, int Size)
{
    struct ValueType *Typ = TypeGetMatching(pc, Parser, &pc->UberType, TypeStruct, 0, StructName, false);

    /* create the (empty) table */
    Typ->Members = (Table*)VariableAlloc(pc, Parser, sizeof(struct Table) + STRUCT_TABLE_SIZE * sizeof(struct TableEntry), true);
    Typ->Members->HashTable = (struct TableEntry **)((char *)Typ->Members + sizeof(struct Table));
    TableInitTable(Typ->Members, (struct TableEntry **)((char *)Typ->Members + sizeof(struct Table)), STRUCT_TABLE_SIZE, true);
    Typ->Sizeof = Size;

    return Typ;
}

/* parse an enum declaration */
void TypeParseEnum(struct ParseState *Parser, struct ValueType **Typ)
{
    struct Value *LexValue;
    struct Value InitValue;
    enum LexToken Token;
    int EnumValue = 0;
    char *EnumIdentifier;
    Picoc *pc = Parser->pc;

    Token = LexGetToken(Parser, &LexValue, false);
    if (Token == TokenIdentifier)
    {
        LexGetToken(Parser, &LexValue, true);
        EnumIdentifier = LexValue->Val->Identifier;
        Token = LexGetToken(Parser, nullptr, false);
    }
    else
    {
        static char TempNameBuf[7] = "^e0000";
        EnumIdentifier = PlatformMakeTempName(pc, TempNameBuf);
    }

    TypeGetMatching(pc, Parser, &pc->UberType, TypeEnum, 0, EnumIdentifier, Token != TokenLeftBrace);
    *Typ = &pc->IntType;
    if (Token != TokenLeftBrace)
    {
        /* use the already defined enum */
        if ((*Typ)->Members == nullptr)
            ProgramFail(Parser, "enum '%s' isn't defined", EnumIdentifier);

        return;
    }

    if (pc->TopStackFrame != nullptr)
        ProgramFail(Parser, "enum definitions can only be globals");

    LexGetToken(Parser, nullptr, true);
    (*Typ)->Members = &pc->GlobalTable;
    memset((void *)&InitValue, '\0', sizeof(struct Value));
    InitValue.Typ = &pc->IntType;
    InitValue.Val = (union AnyValue *)&EnumValue;
    do {
        if (LexGetToken(Parser, &LexValue, true) != TokenIdentifier)
            ProgramFail(Parser, "identifier expected");

        EnumIdentifier = LexValue->Val->Identifier;
        if (LexGetToken(Parser, nullptr, false) == TokenAssign)
        {
            LexGetToken(Parser, nullptr, true);
            EnumValue = ExpressionParseInt(Parser);
        }

        VariableDefine(pc, Parser, EnumIdentifier, &InitValue, nullptr, false);

        Token = LexGetToken(Parser, nullptr, true);
        if (Token != TokenComma && Token != TokenRightBrace)
            ProgramFail(Parser, "comma expected");

        EnumValue++;

    } while (Token == TokenComma);
}

/* parse a type - just the basic type */
int TypeParseFront(struct ParseState *Parser, struct ValueType **Typ, int *IsStatic)
{
    struct ParseState Before;
    struct Value *LexerValue;
    enum LexToken Token;
    int Unsigned = false;
    struct Value *VarValue;
    int StaticQualifier = false;
    Picoc *pc = Parser->pc;
    *Typ = nullptr;

    /* ignore leading type qualifiers */
    ParserCopy(&Before, Parser);
    Token = LexGetToken(Parser, &LexerValue, true);
    while (Token == TokenStaticType || Token == TokenAutoType || Token == TokenRegisterType || Token == TokenExternType)
    {
        if (Token == TokenStaticType)
            StaticQualifier = true;

        Token = LexGetToken(Parser, &LexerValue, true);
    }

    if (IsStatic != nullptr)
        *IsStatic = StaticQualifier;

    /* handle signed/unsigned with no trailing type */
    if (Token == TokenSignedType || Token == TokenUnsignedType)
    {
        enum LexToken FollowToken = LexGetToken(Parser, &LexerValue, false);
        Unsigned = (Token == TokenUnsignedType);

        if (FollowToken != TokenIntType && FollowToken != TokenLongType && FollowToken != TokenShortType && FollowToken != TokenCharType)
        {
            if (Token == TokenUnsignedType)
                *Typ = &pc->UnsignedIntType;
            else
                *Typ = &pc->IntType;

            return true;
        }

        Token = LexGetToken(Parser, &LexerValue, true);
    }

    switch (Token)
    {
        case TokenIntType: *Typ = Unsigned ? &pc->UnsignedIntType : &pc->IntType; break;
        case TokenShortType: *Typ = Unsigned ? &pc->UnsignedShortType : &pc->ShortType; break;
        case TokenCharType: *Typ = Unsigned ? &pc->UnsignedCharType : &pc->CharType; break;
        case TokenLongType: *Typ = Unsigned ? &pc->UnsignedLongType : &pc->LongType; break;
#ifndef NO_FP
        case TokenFloatType: case TokenDoubleType: *Typ = &pc->FPType; break;
#endif
        case TokenVoidType: *Typ = &pc->VoidType; break;

        case TokenStructType: case TokenUnionType:
            if (*Typ != nullptr)
                ProgramFail(Parser, "bad type declaration");

            TypeParseStruct(Parser, Typ, Token == TokenStructType);
            break;

        case TokenEnumType:
            if (*Typ != nullptr)
                ProgramFail(Parser, "bad type declaration");

            TypeParseEnum(Parser, Typ);
            break;

        case TokenIdentifier:
            /* we already know it's a typedef-defined type because we got here */
            VariableGet(pc, Parser, LexerValue->Val->Identifier, &VarValue);
            *Typ = VarValue->Val->Typ;
            break;

        default: ParserCopy(Parser, &Before); return false;
    }

    return true;
}

/* parse a type - the part at the end after the identifier. eg. array specifications etc. */
struct ValueType *TypeParseBack(struct ParseState *Parser, struct ValueType *FromType)
{
    enum LexToken Token;
    struct ParseState Before;

    ParserCopy(&Before, Parser);
    Token = LexGetToken(Parser, nullptr, true);
    if (Token == TokenLeftSquareBracket)
    {
        /* add another array bound */
        if (LexGetToken(Parser, nullptr, false) == TokenRightSquareBracket)
        {
            /* an unsized array */
            LexGetToken(Parser, nullptr, true);
            return TypeGetMatching(Parser->pc, Parser, TypeParseBack(Parser, FromType), TypeArray, 0, Parser->pc->StrEmpty, true);
        }
        else
        {
            /* get a numeric array size */
            enum RunMode OldMode = Parser->Mode;
            int ArraySize;
            Parser->Mode = RunModeRun;
            ArraySize = ExpressionParseInt(Parser);
            Parser->Mode = OldMode;

            if (LexGetToken(Parser, nullptr, true) != TokenRightSquareBracket)
                ProgramFail(Parser, "']' expected");

            return TypeGetMatching(Parser->pc, Parser, TypeParseBack(Parser, FromType), TypeArray, ArraySize, Parser->pc->StrEmpty, true);
        }
    }
    else
    {
        /* the type specification has finished */
        ParserCopy(Parser, &Before);
        return FromType;
    }
}

/* parse a type - the part which is repeated with each identifier in a declaration list */
void TypeParseIdentPart(struct ParseState *Parser, struct ValueType *BasicTyp, struct ValueType **Typ, char **Identifier)
{
    struct ParseState Before;
    enum LexToken Token;
    struct Value *LexValue;
    int Done = false;
    *Typ = BasicTyp;
    *Identifier = Parser->pc->StrEmpty;

    while (!Done)
    {
        ParserCopy(&Before, Parser);
        Token = LexGetToken(Parser, &LexValue, true);
        switch (Token)
        {
            case TokenOpenBracket:
                if (*Typ != nullptr)
                    ProgramFail(Parser, "bad type declaration");

                TypeParse(Parser, Typ, Identifier, nullptr);
                if (LexGetToken(Parser, nullptr, true) != TokenCloseBracket)
                    ProgramFail(Parser, "')' expected");
                break;

            case TokenAsterisk:
                if (*Typ == nullptr)
                    ProgramFail(Parser, "bad type declaration");

                *Typ = TypeGetMatching(Parser->pc, Parser, *Typ, TypePointer, 0, Parser->pc->StrEmpty, true);
                break;

            case TokenIdentifier:
                if (*Typ == nullptr || *Identifier != Parser->pc->StrEmpty)
                    ProgramFail(Parser, "bad type declaration");

                *Identifier = LexValue->Val->Identifier;
                Done = true;
                break;

            default: ParserCopy(Parser, &Before); Done = true; break;
        }
    }

    if (*Typ == nullptr)
        ProgramFail(Parser, "bad type declaration");

    if (*Identifier != Parser->pc->StrEmpty)
    {
        /* parse stuff after the identifier */
        *Typ = TypeParseBack(Parser, *Typ);
    }
}

/* parse a type - a complete declaration including identifier */
void TypeParse(struct ParseState *Parser, struct ValueType **Typ, char **Identifier, int *IsStatic)
{
    struct ValueType *BasicType;

    TypeParseFront(Parser, &BasicType, IsStatic);
    TypeParseIdentPart(Parser, BasicType, Typ, Identifier);
}

/* check if a type has been fully defined - otherwise it's just a forward declaration */
int TypeIsForwardDeclared(struct ParseState *Parser, struct ValueType *Typ)
{
    if (Typ->Base == TypeArray)
        return TypeIsForwardDeclared(Parser, Typ->FromType);

    if ( (Typ->Base == TypeStruct || Typ->Base == TypeUnion) && Typ->Members == nullptr)
        return true;

    return false;
}
