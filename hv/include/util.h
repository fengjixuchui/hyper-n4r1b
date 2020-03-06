#pragma once
#include <ntddk.h>

UINT64 VadToPhysicalAddr(
    PVOID
);

UINT64 PhysicalAddrToVad(
    ULONG64
);

INT ipow(
    INT,
    INT
);