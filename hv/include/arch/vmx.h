#pragma once
#include <ntddk.h>

#pragma warning(disable:4214)
#pragma warning(disable:4131)

#define MSR_IA32_FEATURE_CONTROL    0x3a
#define MSR_IA32_VMX_BASIC          0x480

// https://tandasat.github.io/HyperPlatform/doxygen/union_ia32_feature_control_msr.html
// Table 6-1. Layout of IA32_FEATURE_CONTROL 
typedef union _IA32_FEATURE_CONTROL
{
    UINT64 All;
    struct
    {
        UINT64 Lock : 1;
        UINT64 EnableSMX : 1;
        UINT64 EnableVmxon : 1;
        UINT64 Reserved : 5;
        UINT64 EnableLocalSENTER : 7;
        UINT64 EnableGlobalSENTER : 1;
        UINT64 Reserved1 : 16;
        UINT64 Reserved2 : 32;
    } Fields;
} IA32_FEATURE_CONTROL, * PIA32_FEATURE_CONTROL;

// A.1 BASIC VMX INFORMATION
// Also https://rayanfam.com/topics/hypervisor-from-scratch-part-3/
typedef union _IA32_VMX_BASIC
{
    UINT64 All;
    struct
    {
        UINT64 RevisionIdentifier : 31;
        UINT64 Reserved : 1;
        UINT64 RegionSize : 12;
        UINT64 RegionClear : 1;
        UINT64 Reserved1 : 3;
        UINT64 SupportedIA64 : 1;
        UINT64 SupportedDualMoniter : 1;
        UINT64 MemoryType : 4;
        UINT64 VmExitReport : 1;
        UINT64 VmxCapabilityHint : 1;
        UINT64 Reserved2 : 8;
    } Fields;
} IA32_VMX_BASIC, * PIA32_VMX_BASIC;

typedef struct _VMCS
{
    union {
        UINT32 All;
        struct {
            UINT32 RevisionId : 31;
            UINT32 ShadowVmcsIndicator : 1;
        } Fields;
    } Header;

    UINT32 AbortIndicator;
    UINT8 Data[PAGE_SIZE - 8];
} VMCS, * PVMCS;

BOOLEAN IsVmxSupported(
    VOID
);

VOID EnableVmxOperation(
    VOID
);

NTSTATUS AllocAndInitVmxonRegion( 
    PVP_DATA 
);

NTSTATUS AllocAndInitVmcsRegion( 
    PVP_DATA 
);

VOID ExitVmxOperation(
    VOID
);
