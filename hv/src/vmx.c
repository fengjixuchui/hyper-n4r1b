#include <ntifs.h>
#include <intrin.h>

#include "../include/hv.h"
#include "../include/arch/msr.h"

BOOLEAN 
IsVmxSupported()
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

VOID 
EnableVmxOperation()
{
    UINT64 cr4;

    cr4 = __readcr4();
    // 23.7 ENABLING AND ENTERING VMX OPERATION - CR4.VMXE[bit 13] = 1
    cr4 |= 0x2000;
    __writecr4(cr4);

}

NTSTATUS 
AllocAndInitVmxonRegion(
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

    // Bits 44:32 report the number of bytes that software should allocate for the VMXON region and any VMCS
    // region.It is a value greater than 0 and at most 4096 (bit 44 is set if and only if bits 43:32 are clear).
    if (vmxBasic.Fields.RegionSize > PAGE_SIZE) {
        lpBuffer = MmAllocateContiguousMemory(PAGE_SIZE, physicalAddress);

        if (!lpBuffer) {
            HvLogDebug("Error trying to allocate VMXON region\n");
            return STATUS_MEMORY_NOT_ALLOCATED;
        }

        RtlSecureZeroMemory(lpBuffer, PAGE_SIZE);
        Vp->Vmxon = lpBuffer;
    } else {
        lpBuffer = MmAllocateContiguousMemory(vmxBasic.Fields.RegionSize, physicalAddress);

        if (!lpBuffer) {
            HvLogDebug("Error trying to allocate VMXON region\n");
            return STATUS_MEMORY_NOT_ALLOCATED;
        }

        RtlSecureZeroMemory(lpBuffer, vmxBasic.Fields.RegionSize);
        Vp->Vmxon = lpBuffer;
    }

    Vp->VmxonPad = VadToPhysicalAddr(lpBuffer);

    vmxon = Vp->Vmxon;

    vmxon->Header.All = (UINT32) vmxBasic.Fields.RevisionIdentifier;

    status = __vmx_on(&Vp->VmxonPad);

    if (status) {
        HvLogDebug("Error: %d when trying to execute VMXON\n", status);
        return STATUS_ILLEGAL_INSTRUCTION;
    }

    HvLogDebug("Allocated and initialized VMXON region for Virtual Processor %d \n VA: %llx\n PA: %llx\n",
        KeGetCurrentProcessorNumber(),
        Vp->Vmxon,
        Vp->VmxonPad);

    return status;
}

NTSTATUS 
AllocAndInitVmcsRegion(
    PVP_DATA Vp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    PHYSICAL_ADDRESS physicalAddress = { 0 };
    IA32_VMX_BASIC vmxBasic = { 0 };
    PVMCS vmcs;
    PVOID lpBuffer;


    if (!Vp) {
        HvLogDebug("Error initializing VMCS region. No Virtual Processor data provided\n");
        return STATUS_INVALID_PARAMETER;
    }

    physicalAddress.QuadPart = ~0LL;
    vmxBasic.All = __readmsr(MSR_IA32_VMX_BASIC);

    // Bits 44:32 report the number of bytes that software should allocate for the VMXON region and any VMCS
    // region.It is a value greater than 0 and at most 4096 (bit 44 is set ifand only if bits 43:32 are clear).
    if (vmxBasic.Fields.RegionSize > PAGE_SIZE) {
        lpBuffer = MmAllocateContiguousMemory(PAGE_SIZE, physicalAddress);

        if (!lpBuffer) {
            HvLogDebug("Error trying to allocate VMCS region\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlSecureZeroMemory(lpBuffer, PAGE_SIZE);
        Vp->Vmcs = lpBuffer;
    }
    else {
        lpBuffer = MmAllocateContiguousMemory(vmxBasic.Fields.RegionSize, physicalAddress);

        if (!lpBuffer) {
            HvLogDebug("Error trying to allocate VMCS region\n");
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlSecureZeroMemory(lpBuffer, vmxBasic.Fields.RegionSize);
        Vp->Vmcs = lpBuffer;
    }

    Vp->VmcsPad = VadToPhysicalAddr(lpBuffer);

    vmcs = Vp->Vmcs;

    vmcs->Header.All = (UINT32) vmxBasic.Fields.RevisionIdentifier;
    vmcs->Header.Fields.ShadowVmcsIndicator = 0;

    status = __vmx_vmptrld(&Vp->VmcsPad);

    if (status) {
        HvLogDebug("Error: %08xd when trying to execute VMXON\n", status);
        return STATUS_ILLEGAL_INSTRUCTION;
    }

    HvLogDebug("Allocated and initialized VMCS for Virtual Processor %d \n VA: %llx\n PA: %llx\n",
        KeGetCurrentProcessorNumber(),
        Vp->Vmcs,
        Vp->VmcsPad);

    return status;
}

VOID 
ExitVmxOperation()
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

VOID
RestoreAfterLaunch(
    VOID
)
{
    DbgBreakPoint();
}

VOID
AdjustControl(
    UINT32 ControlType,
    PVOID ControlStructure
)
{
    IA32_VMX_BASIC vmxBasic;
    RESERVED_CONTROLS_DEFAULT_SETTINGS reservedControl = { 0 };

    vmxBasic.All = __readmsr(MSR_IA32_VMX_BASIC);

    switch (ControlType) {
    case PIN_BASED_CTLS_TYPE:
        reservedControl.DefaultSettings = vmxBasic.Fields.VmxCapabilityHint ?
            __readmsr(MSR_IA32_VMX_TRUE_PINBASED_CTLS) : __readmsr(MSR_IA32_VMX_PINBASED_CTLS);

        ((PIN_BASED_EXECUTION_CONTROL*)ControlStructure)->All |= reservedControl.Default0;
        ((PIN_BASED_EXECUTION_CONTROL*)ControlStructure)->All &= reservedControl.Default1;
        break;
    case PROCBASED_CTLS_TYPE:
        reservedControl.DefaultSettings = vmxBasic.Fields.VmxCapabilityHint ?
            __readmsr(MSR_IA32_VMX_TRUE_PROCBASED_CTLS) : __readmsr(MSR_IA32_VMX_PROCBASED_CTLS);

        ((PRIMARY_PROC_BASED_EXECUTION_CONTROL*)ControlStructure)->All |= reservedControl.Default0;
        ((PRIMARY_PROC_BASED_EXECUTION_CONTROL*)ControlStructure)->All &= reservedControl.Default1;
        break;
    case PROCBASED_CTLS2_TYPE:
        reservedControl.DefaultSettings = __readmsr(MSR_IA32_VMX_PROCBASED_CTLS2);

        ((SECONDARY_PROC_BASED_EXECUTION_CONTROL*)ControlStructure)->All |= reservedControl.Default0;
        ((SECONDARY_PROC_BASED_EXECUTION_CONTROL*)ControlStructure)->All &= reservedControl.Default1;
        break;
    case VM_ENTRY_CTLS_TYPE:
        reservedControl.DefaultSettings = vmxBasic.Fields.VmxCapabilityHint ?
            __readmsr(MSR_IA32_VMX_TRUE_ENTRY_CTLS) : __readmsr(MSR_IA32_VMX_ENTRY_CTLS);

        ((VM_ENTRY_CONTROL*)ControlStructure)->All |= reservedControl.Default0;
        ((VM_ENTRY_CONTROL*)ControlStructure)->All &= reservedControl.Default1;
        break;
    case VM_EXIT_CTLS_TYPE:
        reservedControl.DefaultSettings = vmxBasic.Fields.VmxCapabilityHint ?
            __readmsr(MSR_IA32_VMX_TRUE_EXIT_CTLS) : __readmsr(MSR_IA32_VMX_EXIT_CTLS);

        ((VM_EXIT_CONTROL*)ControlStructure)->All |= reservedControl.Default0;
        ((VM_EXIT_CONTROL*)ControlStructure)->All &= reservedControl.Default1;
        break;
    default:
        break;
    }
}

VOID
InitGuestStateArea(
    PVP_DATA Vp,
    PVMX_SELECTORS VmxSelectors
)
{

    UNREFERENCED_PARAMETER(Vp);

    // *** 24.4.1 Guest Register State ***

    // Initialize guest Control-Registers
    __vmx_vmwrite(GUEST_CR0, __readcr0());
    __vmx_vmwrite(GUEST_CR3, __readcr3());
    __vmx_vmwrite(GUEST_CR4, __readcr4());

    // Initialize Debug-Register 7
    __vmx_vmwrite(GUEST_DR7, __readdr(7));

    // Initialize Guest RSP, RIP and RFLAGS
    __vmx_vmwrite(GUEST_RIP, (uintptr_t)RestoreAfterLaunch);
    __vmx_vmwrite(GUEST_RSP, 0);
    __vmx_vmwrite(GUEST_RFLAGS, __readeflags());

    // Initialize Selector Registers
    __vmx_vmwrite(GUEST_CS_SELECTOR, VmxSelectors->GdtEntryCS.Selector);
    __vmx_vmwrite(GUEST_SS_SELECTOR, VmxSelectors->GdtEntrySS.Selector);
    __vmx_vmwrite(GUEST_DS_SELECTOR, VmxSelectors->GdtEntryDS.Selector);
    __vmx_vmwrite(GUEST_ES_SELECTOR, VmxSelectors->GdtEntryES.Selector);
    __vmx_vmwrite(GUEST_FS_SELECTOR, VmxSelectors->GdtEntryFS.Selector);
    __vmx_vmwrite(GUEST_GS_SELECTOR, VmxSelectors->GdtEntryGS.Selector);
    __vmx_vmwrite(GUEST_LDTR_SELECTOR, VmxSelectors->GdtEntryLDTR.Selector);
    __vmx_vmwrite(GUEST_TR_SELECTOR, VmxSelectors->GdtEntryTR.Selector);

    // Initialize Selectors Limit
    __vmx_vmwrite(GUEST_CS_LIMIT, VmxSelectors->GdtEntryCS.Limit);
    __vmx_vmwrite(GUEST_SS_LIMIT, VmxSelectors->GdtEntrySS.Limit);
    __vmx_vmwrite(GUEST_DS_LIMIT, VmxSelectors->GdtEntryDS.Limit);
    __vmx_vmwrite(GUEST_ES_LIMIT, VmxSelectors->GdtEntryES.Limit);
    __vmx_vmwrite(GUEST_FS_LIMIT, VmxSelectors->GdtEntryFS.Limit);
    __vmx_vmwrite(GUEST_GS_LIMIT, VmxSelectors->GdtEntryGS.Limit);
    __vmx_vmwrite(GUEST_LDTR_LIMIT, VmxSelectors->GdtEntryLDTR.Limit);
    __vmx_vmwrite(GUEST_TR_LIMIT, VmxSelectors->GdtEntryTR.Limit);


    // Initialize Selectors Base-address
    __vmx_vmwrite(GUEST_CS_BASE, VmxSelectors->GdtEntryCS.Base);
    __vmx_vmwrite(GUEST_SS_BASE, VmxSelectors->GdtEntrySS.Base);
    __vmx_vmwrite(GUEST_DS_BASE, VmxSelectors->GdtEntryDS.Base);
    __vmx_vmwrite(GUEST_ES_BASE, VmxSelectors->GdtEntryES.Base);
    __vmx_vmwrite(GUEST_FS_BASE, __readmsr(MSR_IA32_FS_BASE));
    __vmx_vmwrite(GUEST_GS_BASE, __readmsr(MSR_IA32_GS_BASE));
    __vmx_vmwrite(GUEST_LDTR_BASE, VmxSelectors->GdtEntryLDTR.Base);
    __vmx_vmwrite(GUEST_TR_BASE, VmxSelectors->GdtEntryTR.Base);


    // Initialize Selector Access rights
    __vmx_vmwrite(GUEST_CS_ACCESS_RIGHTS, VmxSelectors->GdtEntryCS.AccessRights);
    __vmx_vmwrite(GUEST_SS_ACCESS_RIGHTS, VmxSelectors->GdtEntrySS.AccessRights);
    __vmx_vmwrite(GUEST_DS_ACCESS_RIGHTS, VmxSelectors->GdtEntryDS.AccessRights);
    __vmx_vmwrite(GUEST_ES_ACCESS_RIGHTS, VmxSelectors->GdtEntryES.AccessRights);
    __vmx_vmwrite(GUEST_FS_ACCESS_RIGHTS, VmxSelectors->GdtEntryFS.AccessRights);
    __vmx_vmwrite(GUEST_GS_ACCESS_RIGHTS, VmxSelectors->GdtEntryGS.AccessRights);
    __vmx_vmwrite(GUEST_LDTR_ACCESS_RIGHTS, VmxSelectors->GdtEntryLDTR.AccessRights);
    __vmx_vmwrite(GUEST_TR_ACCESS_RIGHTS, VmxSelectors->GdtEntryTR.AccessRights);


    // Initialize GDTR and IDTR
    __vmx_vmwrite(GUEST_GDTR_BASE, (uintptr_t)VmxSelectors->Gdtr.Base);
    __vmx_vmwrite(GUEST_IDTR_BASE, (uintptr_t)VmxSelectors->Idtr.Base);
    __vmx_vmwrite(GUEST_GDTR_LIMIT, VmxSelectors->Gdtr.Limit);
    __vmx_vmwrite(GUEST_IDTR_LIMIT, VmxSelectors->Idtr.Limit);


    // Initialize required MSR, for now only the necessary
    __vmx_vmwrite(GUEST_IA32_DEBUGCTL, __readmsr(MSR_IA32_DEBUGCTL) & 0xFFFFFFFF);
    __vmx_vmwrite(GUEST_IA32_DEBUGCTL_HIGH, __readmsr(MSR_IA32_DEBUGCTL) >> 32);
    __vmx_vmwrite(GUEST_IA32_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
    __vmx_vmwrite(GUEST_IA32_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
    __vmx_vmwrite(GUEST_IA32_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));


    // *** 24.4.2 Guest Non-Register State ***
    __vmx_vmwrite(GUEST_INTERRUPTIBILITY_STATE, 0);
    __vmx_vmwrite(GUEST_ACTIVITY_STATE, ACTIVITY_STATE_ACTIVE);

    // Initialize VMCS link pointer - No VMCS shadowing for now so FFFFFFFF_FFFFFFFFH
    __vmx_vmwrite(VMCS_LINK_POINTER, MAXUINT64);
}

VOID
InitHostStateArea(
    PVP_DATA Vp,
    PVMX_SELECTORS VmxSelectors
)
{

    // *** 24.5 HOST-STATE AREA ***

    // Initialize Host Control-Registers
    __vmx_vmwrite(HOST_CR0, __readcr0());
    __vmx_vmwrite(HOST_CR3, __readcr3());
    __vmx_vmwrite(HOST_CR4, __readcr4());

    // Initialize Host RSP and RIP
    __vmx_vmwrite(HOST_RIP, (uintptr_t)VmExitStub);
    __vmx_vmwrite(HOST_RSP, ((uintptr_t) Vp->VmmStack + KERNEL_STACK_SIZE - sizeof(CONTEXT)));

    // Initialize Selector Registers
    __vmx_vmwrite(HOST_CS_SELECTOR, VmxSelectors->GdtEntryCS.Selector & ~7);
    __vmx_vmwrite(HOST_SS_SELECTOR, VmxSelectors->GdtEntrySS.Selector & ~7);
    __vmx_vmwrite(HOST_DS_SELECTOR, VmxSelectors->GdtEntryDS.Selector & ~7);
    __vmx_vmwrite(HOST_ES_SELECTOR, VmxSelectors->GdtEntryES.Selector & ~7);
    __vmx_vmwrite(HOST_FS_SELECTOR, VmxSelectors->GdtEntryFS.Selector & ~7);
    __vmx_vmwrite(HOST_GS_SELECTOR, VmxSelectors->GdtEntryGS.Selector & ~7);
    __vmx_vmwrite(HOST_TR_SELECTOR, VmxSelectors->GdtEntryTR.Selector & ~7);

    // Initialize required MSR, just the necessary for now
    __vmx_vmwrite(HOST_IA32_SYSENTER_CS, __readmsr(MSR_IA32_SYSENTER_CS));
    __vmx_vmwrite(HOST_IA32_SYSENTER_ESP, __readmsr(MSR_IA32_SYSENTER_ESP));
    __vmx_vmwrite(HOST_IA32_SYSENTER_EIP, __readmsr(MSR_IA32_SYSENTER_EIP));

    // Initialize Selector Base-address
    __vmx_vmwrite(HOST_FS_BASE, __readmsr(MSR_IA32_FS_BASE));
    __vmx_vmwrite(HOST_GS_BASE, __readmsr(MSR_IA32_GS_BASE));
    __vmx_vmwrite(HOST_TR_BASE, VmxSelectors->GdtEntryTR.Base);
    __vmx_vmwrite(HOST_GDTR_BASE, (uintptr_t) VmxSelectors->Gdtr.Base);
    __vmx_vmwrite(HOST_IDTR_BASE, (uintptr_t) VmxSelectors->Idtr.Base);
}


VOID
InitVmExecControl(
    VOID
)
{
    // 24.6.1 Pin - Based VM - Execution Controls
    PIN_BASED_EXECUTION_CONTROL pinBasedControl = { 0 };

    AdjustControl(PIN_BASED_CTLS_TYPE, &pinBasedControl);

    __vmx_vmwrite(PIN_BASED_VM_EXECUTION_CTLS, pinBasedControl.All);

    // 24.6.2 Processor-Based VM-Execution Controls
    PRIMARY_PROC_BASED_EXECUTION_CONTROL primaryProcbasedControl = { 0 };

    primaryProcbasedControl.Fields.HLTExiting = TRUE;
    // primaryProcbasedControl.Fields.UseMSRBitmaps = TRUE;
    primaryProcbasedControl.Fields.ActivateSecondaryControls = TRUE;

    AdjustControl(PROCBASED_CTLS_TYPE, &primaryProcbasedControl);

    __vmx_vmwrite(PRIMARY_PROC_BASED_VM_EXECUTION_CTLS, primaryProcbasedControl.All);

    SECONDARY_PROC_BASED_EXECUTION_CONTROL secondaryProcbasedControl = { 0 };

    secondaryProcbasedControl.Fields.EnableRDTSCP = TRUE;
    // secondaryProcbasedControl.Fields.EnableXSAVESAndXRSTORS = TRUE;
    // secondaryProcbasedControl.Fields.EnableINVPCID = TRUE;

    AdjustControl(PROCBASED_CTLS2_TYPE, &primaryProcbasedControl);

    __vmx_vmwrite(SECONDARY_PROC_BASED_VM_EXECUTION_CTLS, secondaryProcbasedControl.All);


    // Control-Register shadows
    __vmx_vmwrite(CR0_READ_SHADOW, __readcr0());
    __vmx_vmwrite(CR4_READ_SHADOW, __readcr4());
}

VOID
InitVmEntryControl(
    VOID
)
{
    VM_ENTRY_CONTROL vmEntryControl = { 0 };

    // Our processor supports Intel 64 architecture
    vmEntryControl.Fields.IA32ModeGuest = TRUE;
    
    AdjustControl(VM_ENTRY_CTLS_TYPE, &vmEntryControl);

    __vmx_vmwrite(VM_ENTRY_CTLS, vmEntryControl.All);
}

VOID
InitVmExitControl(
    VOID
)
{
    VM_EXIT_CONTROL vmExitControl = { 0 };

    vmExitControl.Fields.HostAddressSpaceSize = TRUE;

    AdjustControl(VM_EXIT_CTLS_TYPE, &vmExitControl);

    __vmx_vmwrite(VM_EXIT_CTLS, vmExitControl.All);

}


NTSTATUS 
InitializeVMCS(
    PVP_DATA Vp
)
{
    NTSTATUS status = STATUS_SUCCESS;
    VMX_SELECTORS vmxSelectors = { 0 };

    if ((status = __vmx_vmclear(&Vp->VmcsPad)) != 0 ) {
        HvLogDebug("Error %d when trying to set VMCS to Clear state\n", status);
        return status;
    }

    if ((status = __vmx_vmptrld(&Vp->VmcsPad)) != 0) {
        HvLogDebug("Error %d when trying to set VMCS to Current and Active state\n", status);
        return status;
    }

    HvLogDebug("VMCS %llx successfully set to Current and Active state\n", Vp->Vmcs);

    CaptureContext(&Vp->Context);

    // Initialize selectors

    __sgdt(&vmxSelectors.Gdtr.Limit);
    __sidt(&vmxSelectors.Idtr.Limit);

    GetVmxGdtEntry(vmxSelectors.Gdtr.Base, __getcs(), &vmxSelectors.GdtEntryCS);
    GetVmxGdtEntry(vmxSelectors.Gdtr.Base, __getss(), &vmxSelectors.GdtEntrySS);
    GetVmxGdtEntry(vmxSelectors.Gdtr.Base, __getds(), &vmxSelectors.GdtEntryDS);
    GetVmxGdtEntry(vmxSelectors.Gdtr.Base, __getes(), &vmxSelectors.GdtEntryES);
    GetVmxGdtEntry(vmxSelectors.Gdtr.Base, __getfs(), &vmxSelectors.GdtEntryFS);
    GetVmxGdtEntry(vmxSelectors.Gdtr.Base, __getgs(), &vmxSelectors.GdtEntryGS);
    GetVmxGdtEntry(vmxSelectors.Gdtr.Base, __getldtr(), &vmxSelectors.GdtEntryLDTR);
    GetVmxGdtEntry(vmxSelectors.Gdtr.Base, __gettr(), &vmxSelectors.GdtEntryTR);

    // VMCS Data contains six logical groups
    
    // Guest-state area
    InitGuestStateArea(Vp, &vmxSelectors);

    // Host-state area
    InitHostStateArea(Vp, &vmxSelectors);

    // VM-execution control fields
    InitVmExecControl();

    // VM-exit control fields
    InitVmExitControl();

    // VM-entry control fiedls
    InitVmEntryControl();


    HvLogDebug("VMCS Region initialized\n");

    // VM Launch
    status = __vmx_vmlaunch();
     
    if (status != 0) {
        size_t ErrorCode = 0;
        __vmx_vmread(VM_INSTRUCTION_ERR, &ErrorCode);
        __vmx_off();
        DbgPrint("[*] VMLAUNCH Error : 0x%llx\n", ErrorCode);
        DbgBreakPoint();
    }
    return status;
}
