#pragma once

#include "selectors.h"

#pragma warning(disable:4131)
#pragma warning(disable:4201)
#pragma warning(disable:4214)

#define MSR_IA32_FEATURE_CONTROL                0x3A
#define MSR_IA32_VMX_BASIC                      0x480
#define MSR_IA32_VMX_PINBASED_CTLS              0x481
#define MSR_IA32_VMX_PROCBASED_CTLS             0x482
#define MSR_IA32_VMX_EXIT_CTLS                  0x483
#define MSR_IA32_VMX_ENTRY_CTLS                 0x484
#define MSR_IA32_VMX_MISC                       0x485
#define MSR_IA32_VMX_CR0_FIXED0                 0x486
#define MSR_IA32_VMX_CR0_FIXED1                 0x487
#define MSR_IA32_VMX_CR4_FIXED0                 0x488
#define MSR_IA32_VMX_CR4_FIXED1                 0x489
#define MSR_IA32_VMX_VMCS_ENUM                  0x48A
#define MSR_IA32_VMX_PROCBASED_CTLS2            0x48B
#define MSR_IA32_VMX_EPT_VPID_CAP               0x48C
#define MSR_IA32_VMX_TRUE_PINBASED_CTLS         0x48D
#define MSR_IA32_VMX_TRUE_PROCBASED_CTLS        0x48E
#define MSR_IA32_VMX_TRUE_EXIT_CTLS             0x48F
#define MSR_IA32_VMX_TRUE_ENTRY_CTLS            0x490
#define MSR_IA32_VMX_VMFUNC                     0x491

typedef enum _ACTIVY_STATE {
    ACTIVITY_STATE_ACTIVE = 0,       
    ACTIVITY_STATE_HLT,          
    ACTIVITY_STATE_SHUTDOWN,
    ACTIVITY_STATE_WAIT_FOR_SIPI
} ACTIVY_STATE;

typedef enum _CONTROL_TYPE {
    PIN_BASED_CTLS_TYPE,
    PROCBASED_CTLS_TYPE,
    PROCBASED_CTLS2_TYPE,
    VM_ENTRY_CTLS_TYPE,
    VM_EXIT_CTLS_TYPE,
} CONTROL_TYPE;

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
        UINT64 RegionSize : 13;
        UINT64 Reserved1 : 3;
        UINT64 SupportedIA64 : 1;
        UINT64 SupportedDualMonitor : 1;
        UINT64 MemoryType : 4;
        UINT64 VmExitReport : 1;
        UINT64 VmxCapabilityHint : 1;
        UINT64 VmEntryCanDeliverHwException : 1;
        UINT64 Reserved2 : 7;
    } Fields;
} IA32_VMX_BASIC, * PIA32_VMX_BASIC;

// 24.6 VM-EXECUTION CONTROL FIELDS

// Table 24-5. Definitions of Pin-Based VM-Execution Controls
typedef union _PIN_BASED_EXECUTION_CONTROL
{
    UINT32 All;
    struct {
        UINT32 ExternalInterruptExiting : 1;
        UINT32 Reserved : 2;
        UINT32 NMIExiting : 1;
        UINT32 Reserved1 : 1;
        UINT32 VirtualNMI : 1;
        UINT32 ActivateVMXPreemptionTimer : 1;
        UINT32 ProcessPostedInterrupts : 1;
    } Fields;
} PIN_BASED_EXECUTION_CONTROL, *PPIN_BASED_EXECUTION_CONTROL;

// Table 24-6. Definitions of Primary Processor-Based VM-Execution Controls
typedef union _PRIMARY_PROC_BASED_EXECUTION_CONTROL
{
    UINT32 All;
    struct {
        UINT32 Reserved : 2;
        UINT32 InterrupWindowsExiting : 1;
        UINT32 UseTSCOffseting : 1;
        UINT32 Reserved1 : 3;
        UINT32 HLTExiting : 1;
        UINT32 Reserved2 : 1;
        UINT32 INVLPGExiting : 1;
        UINT32 MWAITExiting : 1;
        UINT32 RDPMCExiting : 1;
        UINT32 RDTSCExiting : 1;
        UINT32 Reserved3 : 2;
        UINT32 CR3LoadExiting : 1;
        UINT32 CR3StoreExiting : 1;
        UINT32 Reserved4 : 2;
        UINT32 CR8LoadExiting : 1;
        UINT32 CR8StoreExiting : 1;
        UINT32 UseTPRShadow : 1;
        UINT32 NMIWindowExiting : 1;
        UINT32 MOVDRExiting : 1;
        UINT32 UnconditionalIoExiting : 1;
        UINT32 UseIoBitmaps : 1;
        UINT32 Reserved5 : 1;
        UINT32 MonitorTrapFlag : 1;
        UINT32 UseMSRBitmaps : 1;
        UINT32 MONITORExiting : 1;
        UINT32 PAUSEExiting : 1;
        UINT32 ActivateSecondaryControls : 1;
    } Fields;
} PRIMARY_PROC_BASED_EXECUTION_CONTROL, *PPRIMARY_PROC_BASED_EXECUTION_CONTROL;

// Table 24-7. Definitions of Secondary Processor-Based VM-Execution Controls
typedef union _SECONDARY_PROC_BASED_EXECUTION_CONTROL
{
    UINT32 All;
    struct {
        UINT32 VirtualizeAPICAccesses : 1;
        UINT32 EnableEPT : 1;
        UINT32 DescriptorTableExiting : 1;
        UINT32 EnableRDTSCP : 1;
        UINT32 VirtualizeX2APICMode : 1;
        UINT32 EnableVPID : 1;
        UINT32 WBINVDExiting : 1;
        UINT32 UnrestrictedGuest : 1;
        UINT32 APICRegisterVirtualization : 1;
        UINT32 VirtualInterruptDelivery : 1;
        UINT32 PAUSELoopExiting : 1;
        UINT32 RDRANDExiting : 1;
        UINT32 EnableINVPCID : 1;
        UINT32 EnableVMFunctions : 1;
        UINT32 VMCSShadowing : 1;
        UINT32 EnableENCLSExiting : 1;
        UINT32 RDSEEDExiting : 1;
        UINT32 EnablePML : 1;
        UINT32 EPTViolationVE : 1;
        UINT32 ConcealVMXFromPT : 1;
        UINT32 EnableXSAVESAndXRSTORS : 1;
        UINT32 Reserved : 1;
        UINT32 ModeBasedExecuteControlForEPT : 1;
        UINT32 SubPageWritePermissionForEPT : 1;
        UINT32 IntelPTUsesGuestPA : 1;
        UINT32 UseTSCScaling : 1;
        UINT32 EnableUserWaitAndPause : 1;
        UINT32 Reserved1 : 1;
        UINT32 EnableENCLVExiting : 1;
    } Fields;
} SECONDARY_PROC_BASED_EXECUTION_CONTROL, * PSECONDARY_PROC_BASED_EXECUTION_CONTROL;

// Table 24-11. Definitions of VM-Exit Controls
typedef union _VM_EXIT_CONTROL
{
    UINT32 All;
    struct {
        UINT32 Reserved : 2;
        UINT32 SaveDebugControls : 1;
        UINT32 Reserved1 : 6;
        UINT32 HostAddressSpaceSize : 1;
        UINT32 Reserved2 : 2;
        UINT32 LoadIA32PerfGlobalCtrl : 1;
        UINT32 Reserved3 : 2;
        UINT32 AcknowledgeIntOnExit : 1;
        UINT32 Reserved4 : 2;
        UINT32 SaveIA32PAT : 1;
        UINT32 LoadIA32PAT : 1;
        UINT32 SaveIA32EFER : 1;
        UINT32 LoadIA32EFER : 1;
        UINT32 SaveVMXPreemptionTimerValue : 1;
        UINT32 ClearIA32BNDCFGS : 1;
        UINT32 ConcealVMXFromPT : 1;
        UINT32 ClearIA32RtitCtl : 1;
        UINT32 Reserved6 : 2;
        UINT32 LoadCETState : 1;
    } Fields;
} VM_EXIT_CONTROL, *PVM_EXIT_CONTROL;

// Table 24-13. Definitions of VM-Entry Controls
typedef union _VM_ENTRY_CONTROL
{
    UINT32 All;
    struct {
        UINT32 Reserved : 2;
        UINT32 LoadDebugControls : 1;
        UINT32 Reserved1 : 6;
        UINT32 IA32ModeGuest : 1;
        UINT32 EntryToSMM : 1;
        UINT32 DeactivateDualMonitorTreatment : 1;
        UINT32 Reserved2 : 1;
        UINT32 LoadIA32PerfGlobalCtrl : 1;
        UINT32 LoadIA32PAT : 1;
        UINT32 LoadIA32EFER : 1;
        UINT32 LoadIA32BNDCFGS : 1;
        UINT32 ConcealVMXFromPT : 1;        
        UINT32 LoadIA32RtitCtl : 1;
        UINT32 Reserved3 : 1;
        UINT32 LoadCETState : 1;
    } Fields;
} VM_ENTRY_CONTROL, *PVM_ENTRY_CONTROL;

typedef union _RESERVED_CONTROLS_DEFAULT_SETTINGS
{
    UINT64 DefaultSettings;
    struct {
        UINT32 Default0;
        UINT32 Default1;
    };
} RESERVED_CONTROLS_DEFAULT_SETTINGS, *PRESERVED_CONTROLS_DEFAULT_SETTINGS;

BOOLEAN 
IsVmxSupported(
    VOID
);

VOID 
EnableVmxOperation(
    VOID
);

NTSTATUS 
AllocAndInitVmxonRegion( 
    PVP_DATA 
);

NTSTATUS 
AllocAndInitVmcsRegion( 
    PVP_DATA 
);

VOID 
ExitVmxOperation(
    VOID
);

NTSTATUS
InitializeVMCS(
    PVP_DATA
);