#pragma once
#include <ntddk.h>

// Taken from https://revers.engineering/day-3-multiprocessor-initialization-error-handling-the-vmcs/
#define VMCS_COMPONENT_ENCODING(Access, Index, Type, Width)  (UINT32) (( USHORT )(Access) | \
                                                            (( USHORT )(Index) << 1 ) | \
                                                            (( USHORT )(Type) << 10 ) | \
                                                            (( USHORT )(Width) << 13 ) )

#define VMCS_ENCODE_FULL(Index, Type, Width)    VMCS_COMPONENT_ENCODING(Full, Index, Type, Width)
#define VMCS_COMPONENT_16(Index, Type)		    VMCS_ENCODE_FULL(Index, Type, Word)
#define VMCS_COMPONENT_32(Index, Type)		    VMCS_ENCODE_FULL(Index, Type, DWord)
#define VMCS_COMPONENT_64_HIGH(Index, Type)	    VMCS_COMPONENT_ENCODING(High, Index, Type, QWord)
#define VMCS_COMPONENT_64_FULL(Index, Type)		VMCS_ENCODE_FULL(Index, Type, QWord)

typedef enum _ENCODING_ACCESS_TYPE 
{
    Full = 0,
    High
} ENCODING_ACCESS_TYPE;

typedef enum _ENCODING_TYPE
{
    Control = 0,
    VmExitInfo,
    GuestState,
    HostState
} ENCODING_TYPE;

typedef enum _ENCODING_WIDTH
{
    Word = 0,
    QWord,
    DWord,
    Natural
} ENCODING_WIDTH;

// Intel manual - Appendix B: FIELD ENCODING IN VMCS 
typedef enum _VMCS_FIELDS {
    // *** 16-BITS FIELDS ***
    // 16-Bit Control Fields
    VirtualProdessorId = VMCS_COMPONENT_16(0, Control), // VPID
    PostedInterruptNotifVector = VMCS_COMPONENT_16(1, Control),
    EPTPIndex = VMCS_COMPONENT_16(2, Control),

    // 16-Bit Guest-State Fields
    GuestSelectorES = VMCS_COMPONENT_16(0, GuestState),
    GuestSelectorCS = VMCS_COMPONENT_16(1, GuestState),
    GuestSelectorSS = VMCS_COMPONENT_16(2, GuestState),
    GuestSelectorDS = VMCS_COMPONENT_16(3, GuestState),
    GuestSelectorFS = VMCS_COMPONENT_16(4, GuestState),
    GuestSelectorGS = VMCS_COMPONENT_16(5, GuestState),
    GuestSelectorLTDR = VMCS_COMPONENT_16(6, GuestState),
    GuestSelectorTR = VMCS_COMPONENT_16(7, GuestState),
    GuestIntStatus = VMCS_COMPONENT_16(8, GuestState),
    PmlIndex = VMCS_COMPONENT_16(9, GuestState),

    // 16-Bit Host-State Fields
    HostSelectorES = VMCS_COMPONENT_16(0, HostState),
    HostSelectorCS = VMCS_COMPONENT_16(1, HostState),
    HostSelectorSS = VMCS_COMPONENT_16(2, HostState),
    HostSelectorDS = VMCS_COMPONENT_16(3, HostState),
    HostSelectorFS = VMCS_COMPONENT_16(4, HostState),
    HostSelectorGS = VMCS_COMPONENT_16(5, HostState),
    HostSelectorTR = VMCS_COMPONENT_16(6, HostState),

    // *** 32-BITS FIELDS ***
    // 32-Bit Control Fields
    PinBasedVMExecControl = VMCS_COMPONENT_32(0, Control),
    PrimaryProcBasedVMExecControl = VMCS_COMPONENT_32(1, Control),
    ExceptionBitmap = VMCS_COMPONENT_32(2, Control),
    PageFaultErrorCodeMask = VMCS_COMPONENT_32(3, Control),
    PageFaultErrorCodeMatch = VMCS_COMPONENT_32(4, Control),
    Cr3TargetCount = VMCS_COMPONENT_32(5, Control),
    VmExitControls = VMCS_COMPONENT_32(6, Control),
    VmExistMSRStoreCount = VMCS_COMPONENT_32(7, Control),
    VmExistMSRLoadCount = VMCS_COMPONENT_32(8, Control),
    VmEntryControls = VMCS_COMPONENT_32(9, Control),
    VmEntryMSRLoadCount = VMCS_COMPONENT_32(10, Control),
    VmEntryIntInfoField = VMCS_COMPONENT_32(11, Control),
    VmEntryExceptionErrCode = VMCS_COMPONENT_32(12, Control),
    VmEntryInstructionLen = VMCS_COMPONENT_32(13, Control),
    TPRThreshold = VMCS_COMPONENT_32(14, Control),
    SecondaryProcBasedVMExecControl = VMCS_COMPONENT_32(15, Control),
    PLEGap = VMCS_COMPONENT_32(16, Control),
    PLEWindow = VMCS_COMPONENT_32(17, Control),

    // 32-Bit Read-Only Data Fields
    VmInstructionError = VMCS_COMPONENT_32(0, VmExitInfo),
    ExitReason = VMCS_COMPONENT_32(1, VmExitInfo),
    VmExitIntInfo = VMCS_COMPONENT_32(2, VmExitInfo),
    VmExitIntErrCode = VMCS_COMPONENT_32(3, VmExitInfo),
    IDTVectoringInfoField = VMCS_COMPONENT_32(4, VmExitInfo),
    IDTVectoringErrCode = VMCS_COMPONENT_32(5, VmExitInfo),
    VmExitInstructionLen = VMCS_COMPONENT_32(6, VmExitInfo),
    VmExistInstructionInfo = VMCS_COMPONENT_32(7, VmExitInfo),

    // 32-Bit Guest-State Fields
    GuestLimitES = VMCS_COMPONENT_32(0, GuestState),
    GuestLimitCS = VMCS_COMPONENT_32(1, GuestState),
    GuestLimitSS = VMCS_COMPONENT_32(2, GuestState),
    GuestLimitDS = VMCS_COMPONENT_32(3, GuestState),
    GuestLimitFS = VMCS_COMPONENT_32(4, GuestState),
    GuestLimitGS = VMCS_COMPONENT_32(5, GuestState),
    GuestLimitLDTR = VMCS_COMPONENT_32(6, GuestState),
    GuestLimitTR = VMCS_COMPONENT_32(7, GuestState),
    GuestLimitGDTR = VMCS_COMPONENT_32(8, GuestState),
    GuestLimitIDTR = VMCS_COMPONENT_32(9, GuestState),
    GuestAccessRightsES = VMCS_COMPONENT_32(10, GuestState),
    GuestAccessRightsCS = VMCS_COMPONENT_32(11, GuestState),
    GuestAccessRightsSS = VMCS_COMPONENT_32(12, GuestState),
    GuestAccessRightsDS = VMCS_COMPONENT_32(13, GuestState),
    GuestAccessRightsFS = VMCS_COMPONENT_32(14, GuestState),
    GuestAccessRightsGS = VMCS_COMPONENT_32(15, GuestState),
    GuestAccessRightsLDTR = VMCS_COMPONENT_32(16, GuestState),
    GuestAccessRightsTR = VMCS_COMPONENT_32(17, GuestState),
    GuestInterruptibilityState = VMCS_COMPONENT_32(18, GuestState),
    GuestActivityState = VMCS_COMPONENT_32(19, GuestState),
    GuestSMBASE = VMCS_COMPONENT_32(20, GuestState),
    GuestIA32SysenterCS = VMCS_COMPONENT_32(21, GuestState),
    VmxPreemptionTimerValue = VMCS_COMPONENT_32(23, GuestState),

    // 32-Bit Host-State Field
    HostIA32SysenterCS = VMCS_COMPONENT_32(0, HostState),

    // *** 64-BITS FIELDS ***
    // 64-Bit Control Fields
    AddrIoBitmapAFull = VMCS_COMPONENT_64_FULL(0, Control),
    AddrIoBitmapAHigh = VMCS_COMPONENT_64_HIGH(0, Control),
    AddrIoBitmapBFull = VMCS_COMPONENT_64_FULL(1, Control),
    AddrIoBitmapBHigh = VMCS_COMPONENT_64_HIGH(1, Control),
    AddrMSRBitmapsFull = VMCS_COMPONENT_64_FULL(2, Control),
    AddrMSRBitmapsHigh = VMCS_COMPONENT_64_HIGH(2, Control),
    VmExitMSRStoreAddrFull = VMCS_COMPONENT_64_FULL(3, Control),
    VmExitMSRStoreAddrHigh = VMCS_COMPONENT_64_HIGH(3, Control),
    VmExitMSRLoadAddrFull = VMCS_COMPONENT_64_FULL(4, Control),
    VmExitMSRLoadAddrHigh = VMCS_COMPONENT_64_HIGH(4, Control),
    VmEntryMSRLoadAddrFull = VMCS_COMPONENT_64_FULL(5, Control),
    VmEntryMSRLoadAddrHigh = VMCS_COMPONENT_64_HIGH(5, Control),
    ExecutiveVMCSPtrFull = VMCS_COMPONENT_64_FULL(6, Control),
    ExecutiveVMCSPtrHigh = VMCS_COMPONENT_64_HIGH(6, Control),
    PMLAddrFull = VMCS_COMPONENT_64_FULL(7, Control),
    PMLAddrHigh = VMCS_COMPONENT_64_HIGH(7, Control),
    TSCOffsetFull = VMCS_COMPONENT_64_FULL(8, Control),
    TSCOffsetHigh = VMCS_COMPONENT_64_HIGH(8, Control),
    VirtualAPICAddrFull = VMCS_COMPONENT_64_FULL(9, Control),
    VirtualAPICAddrHigh = VMCS_COMPONENT_64_HIGH(9, Control),
    APICAccessAddrFull = VMCS_COMPONENT_64_FULL(10, Control),
    APICAccessAddrHigh = VMCS_COMPONENT_64_HIGH(10, Control),
    PostedIntDescriptorAddrFull = VMCS_COMPONENT_64_FULL(11, Control),
    PostedIntDescriptorAddrHigh = VMCS_COMPONENT_64_HIGH(11, Control),
    VmFunctionControlsFull = VMCS_COMPONENT_64_FULL(12, Control),
    VmFunctionControlsHigh = VMCS_COMPONENT_64_HIGH(12, Control),
    EPTPtrFull = VMCS_COMPONENT_64_FULL(13, Control),
    EPTPtrHigh = VMCS_COMPONENT_64_HIGH(13, Control),
    EOIExitBitmap0Full = VMCS_COMPONENT_64_FULL(14, Control),
    EOIExitBitmap0High = VMCS_COMPONENT_64_HIGH(14, Control),
    EOIExitBitmap1Full = VMCS_COMPONENT_64_FULL(15, Control),
    EOIExitBitmap1High = VMCS_COMPONENT_64_HIGH(15, Control),
    EOIExitBitmap2Full = VMCS_COMPONENT_64_FULL(16, Control),
    EOIExitBitmap2High = VMCS_COMPONENT_64_HIGH(16, Control),
    EOIExitBitmap3Full = VMCS_COMPONENT_64_FULL(17, Control),
    EOIExitBitmap3High = VMCS_COMPONENT_64_HIGH(17, Control),
    EPTPListAddrFull = VMCS_COMPONENT_64_FULL(18, Control),
    EPTPListAddrHigh = VMCS_COMPONENT_64_HIGH(18, Control),
    VmReadBitmapAddrFull = VMCS_COMPONENT_64_FULL(19, Control),
    VmReadBitmapAddrHigh = VMCS_COMPONENT_64_HIGH(19, Control),
    VmWriteBitmapAddrFull = VMCS_COMPONENT_64_FULL(20, Control),
    VmWriteBitmapAddrHigh = VMCS_COMPONENT_64_HIGH(20, Control),
    VirtExceptionInfoAddrFull = VMCS_COMPONENT_64_FULL(21, Control),
    VirtExceptionInfoAddrHigh = VMCS_COMPONENT_64_HIGH(21, Control),
    XSSExitingBitmapFull = VMCS_COMPONENT_64_FULL(22, Control),
    XSSExitingBitmapHigh = VMCS_COMPONENT_64_HIGH(22, Control),
    ENCLSExitingBitmapFull = VMCS_COMPONENT_64_FULL(23, Control),
    ENCLSExitingBitmapHigh = VMCS_COMPONENT_64_HIGH(23, Control),
    SubPagePermissionTablePtrFull = VMCS_COMPONENT_64_FULL(24, Control),
    SubPagePermissionTablePtrHigh = VMCS_COMPONENT_64_HIGH(24, Control),
    TSCMultiplierFull = VMCS_COMPONENT_64_FULL(25, Control),
    TSCMultiplierHigh = VMCS_COMPONENT_64_HIGH(25, Control),

    // 64-Bit Read-Only Data Field
    GuestPhysicalAddrFull = VMCS_COMPONENT_64_FULL(0, VmExitInfo),
    GuestPhysicalAddrHigh = VMCS_COMPONENT_64_HIGH(0, VmExitInfo),

    // 64-Bit Guest-State Fields
    VMCSLinkPointerFull = VMCS_COMPONENT_64_FULL(0, GuestState),
    VMCSLinkPointerHigh = VMCS_COMPONENT_64_HIGH(0, GuestState),
    GuestIA32DebugCtlFull = VMCS_COMPONENT_64_FULL(1, GuestState),
    GuestIA32DebugCtlHigh = VMCS_COMPONENT_64_HIGH(1, GuestState),
    GuestIA32PatFull = VMCS_COMPONENT_64_FULL(2, GuestState),
    GuestIA32PatHigh = VMCS_COMPONENT_64_HIGH(2, GuestState),
    GuestIA32EferFull = VMCS_COMPONENT_64_FULL(3, GuestState),
    GuestIA32EferHigh = VMCS_COMPONENT_64_HIGH(3, GuestState),
    GuestIA32PerfGlobalCtrlFull = VMCS_COMPONENT_64_FULL(4, GuestState),
    GuestIA32PerfGlobalCtrlHigh = VMCS_COMPONENT_64_HIGH(4, GuestState),
    GuestPDPTE0Full = VMCS_COMPONENT_64_FULL(5, GuestState),
    GuestPDPTE0High = VMCS_COMPONENT_64_HIGH(5, GuestState),
    GuestPDPTE1Full = VMCS_COMPONENT_64_FULL(6, GuestState),
    GuestPDPTE1High = VMCS_COMPONENT_64_HIGH(6, GuestState),
    GuestPDPTE2Full = VMCS_COMPONENT_64_FULL(7, GuestState),
    GuestPDPTE2High = VMCS_COMPONENT_64_HIGH(7, GuestState),
    GuestPDPTE3Full = VMCS_COMPONENT_64_FULL(8, GuestState),
    GuestPDPTE3High = VMCS_COMPONENT_64_HIGH(8, GuestState),
    GuestIA32BNDCFGSFull = VMCS_COMPONENT_64_FULL(9, GuestState),
    GuestIA32BNDCFGSHigh = VMCS_COMPONENT_64_HIGH(9, GuestState),
    GuestIA32RtitCtlFull = VMCS_COMPONENT_64_FULL(10, GuestState),
    GuestIA32RtitCtlHigh = VMCS_COMPONENT_64_HIGH(10, GuestState),
    
    // 64-Bit Host-State Fields
    HostIA32PatFull = VMCS_COMPONENT_64_FULL(0, HostState),
    HostIA32PatHigh = VMCS_COMPONENT_64_HIGH(0, HostState),
    HostIA32EferFull = VMCS_COMPONENT_64_FULL(1, HostState),
    HostIA32EferHigh = VMCS_COMPONENT_64_HIGH(1, HostState),
    HostIA32PerfGlobalCtrlFull = VMCS_COMPONENT_64_FULL(2, HostState),
    HostIA32PerfGlobalCtrlHigh = VMCS_COMPONENT_64_HIGH(2, HostState),

    // *** NATURAL-WIDTH FIELDS ***
    // Natural-Width Control Fields
    GuestHostMaskCR0 = VMCS_ENCODE_FULL(0, Control, Natural),
    GuestHostMaskCR4 = VMCS_ENCODE_FULL(1, Control, Natural),
    ReadShadowCR0 = VMCS_ENCODE_FULL(2, Control, Natural),
    ReadShadowCR4 = VMCS_ENCODE_FULL(3, Control, Natural),
    TargetValueZeroCR3 = VMCS_ENCODE_FULL(4, Control, Natural),
    TargetValueOneCR3 = VMCS_ENCODE_FULL(5, Control, Natural),
    TargetValueTwoCR3 = VMCS_ENCODE_FULL(5, Control, Natural),
    TargetValueThreeCR3 = VMCS_ENCODE_FULL(5, Control, Natural),

    //Natural-Width Read-Only Data Fields
    ExitQualification = VMCS_ENCODE_FULL(0, VmExitInfo, Natural),
    IoRCX = VMCS_ENCODE_FULL(1, VmExitInfo, Natural),
    IoRSI = VMCS_ENCODE_FULL(2, VmExitInfo, Natural),
    IoRDI = VMCS_ENCODE_FULL(3, VmExitInfo, Natural),
    IoRIP = VMCS_ENCODE_FULL(4, VmExitInfo, Natural),
    GuestLinearAddr = VMCS_ENCODE_FULL(5, VmExitInfo, Natural),

    // Natural-Width Guest-State Fields
    GuestCR0 = VMCS_ENCODE_FULL(0, GuestState, Natural),
    GuestCR3 = VMCS_ENCODE_FULL(1, GuestState, Natural),
    GuestCR4 = VMCS_ENCODE_FULL(2, GuestState, Natural),
    GuestBaseES = VMCS_ENCODE_FULL(3, GuestState, Natural),
    GuestBaseCS = VMCS_ENCODE_FULL(4, GuestState, Natural),
    GuestBaseSS = VMCS_ENCODE_FULL(5, GuestState, Natural),
    GuestBaseDS = VMCS_ENCODE_FULL(6, GuestState, Natural),
    GuestBaseFS = VMCS_ENCODE_FULL(7, GuestState, Natural),
    GuestBaseGS = VMCS_ENCODE_FULL(8, GuestState, Natural),
    GuestBaseLDTR = VMCS_ENCODE_FULL(9, GuestState, Natural),
    GuestBaseTR = VMCS_ENCODE_FULL(10, GuestState, Natural),
    GuestBaseGDTR = VMCS_ENCODE_FULL(11, GuestState, Natural),
    GuestBaseIDTR = VMCS_ENCODE_FULL(12, GuestState, Natural),
    GuestDR7 = VMCS_ENCODE_FULL(13, GuestState, Natural),
    GuestRSP = VMCS_ENCODE_FULL(14, GuestState, Natural),
    GuestRIP = VMCS_ENCODE_FULL(15, GuestState, Natural),
    GuestRFLAGS = VMCS_ENCODE_FULL(16, GuestState, Natural),
    GuestPendingDebugException = VMCS_ENCODE_FULL(17, GuestState, Natural),
    GuestIA32SysenterESP = VMCS_ENCODE_FULL(18, GuestState, Natural),
    GuestIA32SysenterEIP = VMCS_ENCODE_FULL(19, GuestState, Natural),
    GuestIA32CET = VMCS_ENCODE_FULL(20, GuestState, Natural),
    GuestSSP = VMCS_ENCODE_FULL(21, GuestState, Natural),
    GuestIA32InterruptSSPTableAddr = VMCS_ENCODE_FULL(22, GuestState, Natural),

    // Natural-Width Host-State Fields
    HostCR0 = VMCS_ENCODE_FULL(0, HostState, Natural),
    HostCR3 = VMCS_ENCODE_FULL(1, HostState, Natural),
    HostCR4 = VMCS_ENCODE_FULL(2, HostState, Natural),
    HostBaseFS = VMCS_ENCODE_FULL(3, HostState, Natural),
    HostBaseGS = VMCS_ENCODE_FULL(4, HostState, Natural),
    HostBaseTR = VMCS_ENCODE_FULL(5, HostState, Natural),
    HostBaseGDTR = VMCS_ENCODE_FULL(6, HostState, Natural),
    HostBaseIDTR = VMCS_ENCODE_FULL(7, HostState, Natural),
    HostIA32SysenterESP = VMCS_ENCODE_FULL(8, HostState, Natural),
    HostIA32SysenterEIP = VMCS_ENCODE_FULL(9, HostState, Natural),
    HostRSP = VMCS_ENCODE_FULL(10, HostState, Natural),
    HostRIP = VMCS_ENCODE_FULL(11, HostState, Natural),
    HostIA32CET = VMCS_ENCODE_FULL(12, HostState, Natural),
    HostSSP = VMCS_ENCODE_FULL(13, HostState, Natural),
    HostIA32InterruptSSPTableAddr = VMCS_ENCODE_FULL(14, HostState, Natural)
};
