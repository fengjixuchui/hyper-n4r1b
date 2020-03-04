#include <ntddk.h>

ULONG64 VadToPhysicalAddr(
    PVOID
);

PVOID PhysicalAddrToVad(
    ULONG64
);