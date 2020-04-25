#include <ntifs.h>

#include "../include/util.h"
#include "../include/arch/selectors.h"

UINT64 
VadToPhysicalAddr(
    PVOID Vad
)
{
    return MmGetPhysicalAddress(Vad).QuadPart;
}

UINT64 
PhysicalAddrToVad(
    UINT64 Pad
)
{
    PHYSICAL_ADDRESS PhysicalAddress;
    PhysicalAddress.QuadPart = Pad;

    return (UINT64) MmGetVirtualForPhysical(PhysicalAddress);
}


INT 
ipow(
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

NTSTATUS
GetVmxGdtEntry(
    PVOID GdtBase,
    USHORT Selector,
    PVMX_GDTENTRY64 VmxGdtEntry
)
{
    PKGDTENTRY64 gdtEntry = { 0 };

    if (!GdtBase) {
        // LOG
        return STATUS_INVALID_PARAMETER;
    }

    // Check if Table Indicator equals 1, which means is an LDT entry
    if (Selector == 0 || Selector & 0x4) {

        VmxGdtEntry->Base = 0;
        VmxGdtEntry->Selector = 0;
        VmxGdtEntry->Bits.Unusable = TRUE;
        return STATUS_SUCCESS;
    }

    gdtEntry = (PKGDTENTRY64)((char*)GdtBase + (Selector & ~7)); // We remove the Table indicator and the RPL (Last 3 bits)


    VmxGdtEntry->Selector = Selector;
    VmxGdtEntry->Limit = __segmentlimit(Selector);


    VmxGdtEntry->Base = ((gdtEntry->Bytes.BaseHigh << 24) |
        (gdtEntry->Bytes.BaseMiddle << 16) |
        (gdtEntry->BaseLow)) & 0xFFFFFFFF;

    if (gdtEntry->Bits.Type & 0x10) {
        VmxGdtEntry->Base |= ((uintptr_t) gdtEntry->BaseUpper << 32);
    }

    VmxGdtEntry->AccessRights = 0;
    VmxGdtEntry->Bytes.Flags1 = gdtEntry->Bytes.Flags1;
    VmxGdtEntry->Bytes.Flags2 = gdtEntry->Bytes.Flags2;

    VmxGdtEntry->Bits.Reserved = 0;
    VmxGdtEntry->Bits.Unusable = !gdtEntry->Bits.Present;

    return STATUS_SUCCESS;
}

VOID
CaptureContext(
    PCONTEXT Context
)
{
    // Thanks to Alex https://github.com/ionescu007/SimpleVisor/blob/0275e1e184bcf16314fcf789ddb16f0539440ec3/nt/shvos.c#L299
    RtlCaptureContext(Context);
}
