/* picoc external interface. This should be the only header you need to use if
 * you're using picoc as a library. Internal details are in interpreter.h */
#pragma once

/* picoc version number */
#define PICOC_VERSION "v1.0"

#include "interpreter.h"
#include <setjmp.h>

/* this has to be a macro, otherwise errors will occur due to the stack being corrupt */
#define PicocPlatformSetExitPoint(pc) setjmp((pc)->PicocExitBuf)

#ifdef SURVEYOR_HOST
/* mark where to end the program for platforms which require this */
extern int PicocExitBuf[];

#define PicocPlatformSetExitPoint(pc) setjmp((pc)->PicocExitBuf)
#endif

/* parse.c */
void PicocParse(Picoc *, const char *, const char *, int, int, int, int, int);
void PicocParseInteractive(Picoc *);

/* platform.c */
void PicocCallMain(Picoc *, int, char **);
void PicocInitialise(Picoc *, int);
void PicocCleanup(Picoc *);
void PicocPlatformScanFile(Picoc *, const char *);

/* include.c */
void PicocIncludeAllSystemHeaders(Picoc *);
