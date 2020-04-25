#pragma once

UINT64 
VadToPhysicalAddr(
    PVOID
);

UINT64 
PhysicalAddrToVad(
    ULONG64
);

INT 
ipow(
    INT,
    INT
);

VOID
CaptureContext(
    PCONTEXT
);