#define INSIDE_FASTDB

#include "stdtp.h"
#include <stdio.h>

BEGIN_FASTDB_NAMESPACE

static void CLI_CALLBACK_CC stderrTrace(char* msg)
{
    fputs(msg, stderr);
}

dbTraceFunctionPtr dbTraceFunction = stderrTrace;
bool dbTraceEnable = true;

FASTDB_DLL_ENTRY void dbTrace(char* message, ...) 
{
    if (dbTraceEnable) { 
        va_list args;
        va_start (args, message);
        char buffer[1024];
        vsprintf(buffer, message, args);
        (*dbTraceFunction)(buffer);
        va_end(args);
    }
}


FASTDB_DLL_ENTRY byte* dbMalloc(size_t size)
{
    return (byte*)malloc(size);
}

FASTDB_DLL_ENTRY void  dbFree(void* p)
{
    free(p);
}

END_FASTDB_NAMESPACE
