#include <wdm.h>


#define MSR_IA32_FEATURE_CONTROL 0x3a

// https://tandasat.github.io/HyperPlatform/doxygen/union_ia32_feature_control_msr.html
typedef union _IA32_FEATURE_CONTROL_MSR
{
    ULONG64 All;
    struct
    {
        ULONG64 Lock : 1;
        ULONG64 EnableSMX : 1;
        ULONG64 EnableVmxon : 1;
        ULONG64 Reserved2 : 5;
        ULONG64 EnableLocalSENTER : 7;
        ULONG64 EnableGlobalSENTER : 1;
        ULONG64 Reserved3a : 16;
        ULONG64 Reserved3b : 32;
    } Fields;
} IA32_FEATURE_CONTROL_MSR, * PIA32_FEATURE_CONTROL_MSR;

BOOLEAN IsVmxSupported();
VOID EnableVmxOperation();