#include <ntifs.h>
#include <stdio.h>
#include <stdarg.h>

VOID HvLogDebug(
    const char* Format,
    ...
)
{
    va_list args;

    va_start(args, Format);

    vDbgPrintExWithPrefix("[hv-n4r1b] ", DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, Format, args);

    va_end(args);
}