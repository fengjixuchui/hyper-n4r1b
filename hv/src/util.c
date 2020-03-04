#include <ntddk.h>

#include "..\include\util.h"

UINT64 VadToPhysicalAddr(
    PVOID Vad
)
{
    return MmGetPhysicalAddress(Vad).QuadPart;
}

PVOID PhysicalAddrToVad(
    UINT64 Pad
)
{
    PHYSICAL_ADDRESS PhysicalAddress;
    PhysicalAddress.QuadPart = Pad;

    return MmGetVirtualForPhysical(PhysicalAddress);
}