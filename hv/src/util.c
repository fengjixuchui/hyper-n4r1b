#include <ntddk.h>

#include "..\include\util.h"

UINT64 VadToPhysicalAddr(
    PVOID Vad
)
{
    return MmGetPhysicalAddress(Vad).QuadPart;
}

UINT64 PhysicalAddrToVad(
    UINT64 Pad
)
{
    PHYSICAL_ADDRESS PhysicalAddress;
    PhysicalAddress.QuadPart = Pad;

    return (UINT64) MmGetVirtualForPhysical(PhysicalAddress);
}


INT ipow(
    INT base, 
    INT exp
) 
{
    int result = 1;
    for (;;)
    {
        if (exp & 1)
        {
            result *= base;
        }
        exp >>= 1;
        if (!exp)
        {
            break;
        }
        base *= base;
    }
    return result;
}