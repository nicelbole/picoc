#pragma once

/* all platform-specific includes and defines go in this file */

/* configurable options */
/* select your host type (or do it in the Makefile):
 * #define  UNIX_HOST
 * #define  FLYINGFOX_HOST
 * #define  SURVEYOR_HOST
 * #define  SRV1_UNIX_HOST
 * #define  UMON_HOST
 * #define  WIN32  (predefined on MSVC)
 */

#define LARGE_INT_POWER_OF_TEN 1000000000   /* the largest power of ten which fits in an int on this architecture */
#define ALIGN_TYPE void *                   /* the default data type to use for alignment */

constexpr int GLOBAL_TABLE_SIZE = 97;				/* global variable table */
constexpr int STRING_TABLE_SIZE = 97;				/* shared string table size */
constexpr int STRING_LITERAL_TABLE_SIZE = 97;		/* string literal table size */
constexpr int RESERVED_WORD_TABLE_SIZE = 97;		/* reserved word table size */
constexpr int PARAMETER_MAX = 16;					/* maximum number of parameters to a function */
constexpr int LINEBUFFER_MAX = 256;					/* maximum number of characters on a line */
constexpr int LOCAL_TABLE_SIZE = 11;				/* size of local variable table (can expand) */
constexpr int STRUCT_TABLE_SIZE = 11;				/* size of struct/union member table (can expand) */

#define INTERACTIVE_PROMPT_START "starting picoc " PICOC_VERSION "\n"
constexpr const char* INTERACTIVE_PROMPT_STATEMENT = "picoc> ";
constexpr const char* INTERACTIVE_PROMPT_LINE = "     > ";

/* host platform includes */
#define USE_MALLOC_STACK					/* stack is allocated using malloc() */
#define USE_MALLOC_HEAP						/* heap is allocated using malloc() */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdarg.h>
#include <setjmp.h>
#include <math.h>
#define PICOC_MATH_LIBRARY
#define USE_READLINE
#undef BIG_ENDIAN

extern jmp_buf ExitBuf;
