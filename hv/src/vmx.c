#include <ntddk.h>
#include <intrin.h>

#include "../include/hv.h"

BOOLEAN IsVmxSupported()
{
    INT cpuid[4];
    
    __cpuid(cpuid, 1);

    // 23.6 DISCOVERING SUPPORT FOR VMX - CPUID.1:ECX.VMX[bit 5] = 1
    if (!(cpuid[2] & 0x20)) {
        return FALSE;
    }

    IA32_FEATURE_CONTROL vmxControl = { 0 };
    vmxControl.All = __readmsr(MSR_IA32_FEATURE_CONTROL);

    if (vmxControl.Fields.Lock == 0) {
        vmxControl.Fields.Lock = TRUE;
        vmxControl.Fields.EnableVmxon = TRUE;
        __writemsr(MSR_IA32_FEATURE_CONTROL, vmxControl.All);
    }
    else if (vmxControl.Fields.EnableVmxon == FALSE) {
        // Bit 0 is the lock bit. If this bit is clear, VMXON causes a general-protection exception.
        // If the lock bit is set, WRMSR to this MSR causes a general-protection exception
        HvLogDebug("VMX locked in BIOS/UEFI\n");
        return FALSE;
    }

    return TRUE;
}

VOID EnableVmxOperation()
{
    UINT64 cr4;

    cr4 = __readcr4();
    // 23.7 ENABLING AND ENTERING VMX OPERATION - CR4.VMXE[bit 13] = 1
    cr4 |= 0x2000;
    __writecr4(cr4);

}


NTSTATUS AllocAndInitVmxonRegion(
    PVP_DATA Vp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PHYSICAL_ADDRESS physicalAddress = { 0 };
    IA32_VMX_BASIC vmxBasic = { 0 };
    PVMCS vmxon;
    PVOID lpBuffer;


    if (!Vp) {
        HvLogDebug("Error initializing VMXON region. No Virtual Processor data provided\n");
        return STATUS_INVALID_PARAMETER;
    }

    physicalAddress.QuadPart = ~0LL;
    vmxBasic.All = __readmsr(MSR_IA32_VMX_BASIC);

    if (vmxBasic.Fields.RegionSize > PAGE_SIZE) {
        lpBuffer = MmAllocateContiguousMemory(PAGE_SIZE, physicalAddress);

        if (!lpBuffer) {
            HvLogDebug("Error trying to allocate VMXON region\n");
            return STATUS_MEMORY_NOT_ALLOCATED;
        }

        RtlSecureZeroMemory(vmxon, PAGE_SIZE);
        Vp->Vmxon = lpBuffer;
    } else {
        lpBuffer = MmAllocateContiguousMemory(vmxBasic.Fields.RegionSize, physicalAddress);

        if (!lpBuffer) {
            HvLogDebug("Error trying to allocate VMXON region\n");
            return STATUS_MEMORY_NOT_ALLOCATED;
        }

        RtlSecureZeroMemory(vmxon, vmxBasic.Fields.RegionSize);
        Vp->Vmxon = lpBuffer;
    }

    Vp->VmxonPad = VadToPhysicalAddr(lpBuffer);

    vmxon = Vp->Vmxon;

    vmxon->Header.All = vmxBasic.Fields.RevisionIdentifier;

    status = __vmx_on(&Vp->VmxonPad);

    if (status) {
        HvLogDebug("Error: %d when trying to execute VMXON\n", status);
        return STATUS_ILLEGAL_INSTRUCTION;
    }

    HvLogDebug("Allocated and initialized VMXON region for Virtual Processor %d \n VA: %llx\n PA:%llx\n",
        KeGetCurrentProcessorNumber(),
        Vp->Vmxon,
        Vp->VmxonPad);

    return status;
}

NTSTATUS AllocAndInitVmcsRegion(
    PVP_DATA Vp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PHYSICAL_ADDRESS physicalAddress = { 0 };
    IA32_VMX_BASIC vmxBasic = { 0 };
    PVMCS vmcs;
    PVOID lpBuffer;


    if (!Vp) {
        HvLogDebug("Error initializing VMXON region. No Virtual Processor data provided\n");
        return STATUS_INVALID_PARAMETER;
    }

    physicalAddress.QuadPart = ~0LL;
    vmxBasic.All = __readmsr(MSR_IA32_VMX_BASIC);

    if (vmxBasic.Fields.RegionSize > PAGE_SIZE) {
        lpBuffer = MmAllocateContiguousMemory(PAGE_SIZE, physicalAddress);

        if (!lpBuffer) {
            HvLogDebug("Error trying to allocate VMXON region\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlSecureZeroMemory(lpBuffer, PAGE_SIZE);
        Vp->Vmcs = lpBuffer;
    }
    else {
        lpBuffer = MmAllocateContiguousMemory(vmxBasic.Fields.RegionSize, physicalAddress);

        if (!lpBuffer) {
            HvLogDebug("Error trying to allocate VMXON region\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlSecureZeroMemory(lpBuffer, vmxBasic.Fields.RegionSize);
        Vp->Vmcs = lpBuffer;
    }

    Vp->VmcsPad = VadToPhysicalAddr(lpBuffer);

    vmcs = Vp->Vmcs;

    vmcs->Header.All = vmxBasic.Fields.RevisionIdentifier;
    vmcs->Header.Fields.ShadowVmcsIndicator = 0;

    status = __vmx_vmptrld(&Vp->VmcsPad);

    if (status) {
        HvLogDebug("Error: %08xd when trying to execute VMXON\n", status);
        return STATUS_ILLEGAL_INSTRUCTION;
    }

    HvLogDebug("Allocated and initialized VMCS for Virtual Processor %d \n VA: %llx\n PA:%llx\n",
        KeGetCurrentProcessorNumber(),
        Vp->Vmcs,
        Vp->VmcsPad);

    return status;
}

VOID ExitVmxOperation()
{
    KAFFINITY kAffinityMask;
    INT ProcessorsCount = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

    
    for (INT i = 0; i < ProcessorsCount; i++) {

        kAffinityMask = ipow(2, i);
        KeSetSystemAffinityThread(kAffinityMask);

        __vmx_off();
        MmFreeContiguousMemory((PVOID) PhysicalAddrToVad(VmmState[i].VmcsPad));
        MmFreeContiguousMemory((PVOID) PhysicalAddrToVad(VmmState[i].VmxonPad));

    }
}